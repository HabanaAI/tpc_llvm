// SPDX-License-Identifier: Apache-2.0
// Scores have a BF16 boundary. Exact ordered-bit selection needs 16 passes,
// with each pass bounded by current device length, not allocation capacity.
static inline uint64 ordered(float64 values) {
    const uint64 bits = as_uint64(values) >> 16;
    return v_u32_sel_grt_u32_b(bits, 32767, (~bits)&65535, bits^32768);
}
static inline int64 sum_broadcast(int64 value) {
    // The generic reduce loads lookup-table masks, reserving 64 KiB of VLM.
    // Integer shuffle reductions keep that memory available for score keys.
    value += v_i32_mov_dual_group_all_b(
        value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0);
    value += v_i32_mov_dual_group_all_b(
        value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0);
    value += v_i32_mov_group_b(value, 0xffffffff, 63, 0);
    #pragma loop_unroll(3)
    for (int shift = 4; shift > 0; shift >>= 1) {
        const uint64 mask = ((((uint64)V_LANE_ID_32 & 7) ^ shift) | 0x80) * 0x01010101;
        value += v_i32_shuffle_b(value, *((const uchar256*)&mask), 0, 0);
    }
    return value;
}

// The complete 32K BF16 key set fits in 64 KiB of private vector memory.
// Preserve source order explicitly: default narrowing conversions interleave
// lanes, which would corrupt the per-worker prefix counts below.
__local__ ushort128 selection_keys[256];

static inline ushort128 narrow_keys(uint64 first, uint64 second) {
    return convert_uint128_to_ushort128((uint128){first, second}, SW_LINEAR);
}

static inline int64 sum_keys(short128 value) {
    const int128 wide = convert_short128_to_int128(value, SW_LINEAR);
    return sum_broadcast(wide.v1 + wide.v2);
}

static inline uint64 live_keys(tensor scores, int request, int offset, int count) {
    const float64 value = v_f32_ld_tnsr_b((int5){offset, request}, scores);
    const bool64 valid = v_i32_cmp_less_b((int64)V_LANE_ID_32 + offset, count) &
                         v_f32_cmp_grt_b(value, as_float64((int64)0xff800000));
    return v_u32_mov_vb(ordered(value), 0, (uint64)0, valid, 0);
}

// Four ordered 32-bit words describe 128 input rows. Avoid LUT-based
// reductions so the key cache retains the complete private VLM budget.
#include "deepseek_v41_selection_bitmap.h"

static inline void store_bits(tensor metadata, int request, int tile, int word_capacity,
                              ushort128 keys, ushort128 threshold) {
    const uint128 wide = convert_ushort128_to_uint128(keys, SW_LINEAR);
    const uint128 cut = convert_ushort128_to_uint128(threshold, SW_LINEAR);
    const bool64 live0 = v_u32_cmp_grt_b(wide.v1, 0), live1 = v_u32_cmp_grt_b(wide.v2, 0);
    v_u32_st_tnsr_partial((int5){50 + tile * 4, request}, metadata,
        pack_words(pack_half(live0 & v_u32_cmp_grt_b(wide.v1, cut.v1)),
                   pack_half(live1 & v_u32_cmp_grt_b(wide.v2, cut.v2))), 3, 0);
    v_u32_st_tnsr_partial((int5){50 + word_capacity + tile * 4, request}, metadata,
        pack_words(pack_half(live0 & v_u32_cmp_eq_b(wide.v1, cut.v1)),
                   pack_half(live1 & v_u32_cmp_eq_b(wide.v2, cut.v2))), 3, 0);
}

#include "deepseek_v41_index_predicate_pack.h"

static inline void cached_threshold(tensor scores, tensor metadata, int request,
                                    int count, int partition_count, int width, int variant) {
    const int tiles = (count + 127) / 128;
    for (int tile = 0; tile < tiles; ++tile) {
        const int offset = tile * 128;
        selection_keys[tile] = narrow_keys(live_keys(scores, request, offset, count),
                                           live_keys(scores, request, offset + 64, count));
    }
    ushort128 threshold = 0;
    for (int bit = 15; bit >= 0; --bit) {
        const ushort128 trial = threshold | (1u << bit);
        short128 counts0 = 0, counts1 = 0, counts2 = 0, counts3 = 0;
        int tile = 0;
        for (; tile + 3 < tiles; tile += 4) {
            const ushort128 key0 = selection_keys[tile], key1 = selection_keys[tile + 1];
            const ushort128 key2 = selection_keys[tile + 2], key3 = selection_keys[tile + 3];
            counts0 = v_i16_add_vb(counts0, 1, 0, counts0, v_u16_cmp_geq_b(key0, trial), 0);
            counts1 = v_i16_add_vb(counts1, 1, 0, counts1, v_u16_cmp_geq_b(key1, trial), 0);
            counts2 = v_i16_add_vb(counts2, 1, 0, counts2, v_u16_cmp_geq_b(key2, trial), 0);
            counts3 = v_i16_add_vb(counts3, 1, 0, counts3, v_u16_cmp_geq_b(key3, trial), 0);
        }
        short128 counts = (counts0 + counts1) + (counts2 + counts3);
        for (; tile < tiles; ++tile)
            counts = v_i16_add_vb(counts, 1, 0, counts, v_u16_cmp_geq_b(selection_keys[tile], trial), 0);
        // Widen before reducing: 32768 valid keys must not overflow int16.
        const uint64 total = as_uint64(sum_keys(counts));
        const ushort128 total16 = narrow_keys(total, total);
        threshold = v_u16_sel_geq_u16_b(total16, width, trial, threshold);
    }
    const int word_capacity = (get_dim_size(scores, 0) + 127) / 128 * 4;
    if (variant == 1) {
        for (int tile = 0; tile < tiles; ++tile)
            store_bits_packed(metadata, request, tile, word_capacity, selection_keys[tile], threshold);
    } else {
        for (int tile = 0; tile < tiles; ++tile)
            store_bits(metadata, request, tile, word_capacity, selection_keys[tile], threshold);
    }
    const ushort128 lanes = (ushort128)V_LANE_ID_16;
    int64 greater = 0, equal = 0;
    for (int worker = 0; worker < 24; ++worker) {
        v_i32_st_tnsr_partial((int5){2 + worker * 2, request}, metadata, greater, 0, 0);
        v_i32_st_tnsr_partial((int5){3 + worker * 2, request}, metadata, equal, 0, 0);
        const int first = partition_count * worker / 24;
        const int last = partition_count * (worker + 1) / 24;
        const int stop = last < count ? last : count;
        short128 local_greater = 0, local_equal = 0;
        for (int tile = first / 128; tile * 128 < stop; ++tile) {
            const ushort128 key = selection_keys[tile];
            const ushort128 row = lanes + tile * 128;
            const bool128 valid = v_u16_cmp_geq_b(row, first) & v_u16_cmp_less_b(row, stop) &
                                  v_u16_cmp_grt_b(key, 0);
            local_greater += v_i16_mov_vb((short128)1, 0, (short128)0,
                                         valid & v_u16_cmp_grt_b(key, threshold), 0);
            local_equal += v_i16_mov_vb((short128)1, 0, (short128)0,
                                       valid & v_u16_cmp_eq_b(key, threshold), 0);
        }
        greater += sum_keys(local_greater);
        equal += sum_keys(local_equal);
    }
    const uint128 key32 = convert_ushort128_to_uint128(threshold, SW_LINEAR);
    v_u32_st_tnsr_partial((int5){0, request}, metadata, key32.v1, 0, 0);
    v_i32_st_tnsr_partial((int5){1, request}, metadata, greater, 0, 0);
}

void main(tensor scores, tensor positions, tensor metadata, int ratio, int reindex, int blocks_mode, int variant) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int request=begin[1];request<end[1];++request) {
    int valid_count=(s_i32_ld_g(gen_addr((int5){request},positions))+1)/ratio;
    if (valid_count <= 512) continue;
    // Full layers publish at most 2048 eight-row blocks.  When every visible
    // block fits, selection is the causal prefix itself; there is no
    // threshold to find.  Keep this decision device-valued so the same fixed
    // recipe covers short and long histories without recompilation.
    if (blocks_mode && (valid_count + 7) / 8 <= 2048) continue;
    int partition_count=valid_count;
    if (reindex) {
        const int visible_blocks = (valid_count + 7) / 8;
        valid_count = (visible_blocks < 2048 ? visible_blocks : 2048) * 8;
        partition_count = 16384;
    }
    if (blocks_mode) {
        valid_count=(valid_count+7)/8;
        partition_count=(partition_count+7)/8;
    }
    const int width=blocks_mode ? 2048 : 512;
    if (valid_count <= 32768) {
        cached_threshold(scores, metadata, request, valid_count, partition_count, width, variant);
        continue;
    }
    const int64 lanes=(int64)V_LANE_ID_32;
    uint64 threshold=0;
    for (int bit=15;bit>=0;--bit) {
        const uint64 trial=threshold | (1u<<bit);
        int64 total=0;
        for(int offset=0;offset<valid_count;offset+=64) {
            const float64 score=v_f32_ld_tnsr_b((int5){offset,request},scores);
            const bool64 valid=v_i32_cmp_less_b(lanes+offset,valid_count) & v_f32_cmp_grt_b(score,as_float64((int64)0xff800000));
            const bool64 keep=valid & v_u32_cmp_geq_b(ordered(score),trial);
            total += v_i32_mov_vb((int64)1,0,(int64)0,keep,0);
        }
        total=sum_broadcast(total);
        threshold=v_u32_sel_geq_i32_b(total,width,trial,threshold);
    }
    const int word_capacity = (get_dim_size(scores, 0) + 127) / 128 * 4;
    const ushort128 threshold16 = narrow_keys(threshold, threshold);
    for (int offset = 0; offset < valid_count; offset += 128)
        store_bits(metadata, request, offset / 128, word_capacity,
                   narrow_keys(live_keys(scores, request, offset, valid_count),
                               live_keys(scores, request, offset + 64, valid_count)), threshold16);
    // Prefix counts permit disjoint, ordered output emission by 24 workers.
    int64 greater=0,equal=0;
    for(int worker=0;worker<24;++worker) {
        v_i32_st_tnsr_partial((int5){2+worker*2,request},metadata,greater,0,0);
        v_i32_st_tnsr_partial((int5){3+worker*2,request},metadata,equal,0,0);
        const int first=partition_count*worker/24,last=partition_count*(worker+1)/24;
        const int scan_last=last<valid_count?last:valid_count;
        int64 local_greater=0,local_equal=0;
        for(int offset=first;offset<scan_last;offset+=64) {
            const float64 score=v_f32_ld_tnsr_b((int5){offset,request},scores);
            const bool64 valid=v_i32_cmp_less_b(lanes+offset,scan_last) & v_f32_cmp_grt_b(score,as_float64((int64)0xff800000));
            const uint64 key=ordered(score);
            local_greater+=v_i32_mov_vb((int64)1,0,(int64)0,valid & v_u32_cmp_grt_b(key,threshold),0);
            local_equal+=v_i32_mov_vb((int64)1,0,(int64)0,valid & v_u32_cmp_eq_b(key,threshold),0);
        }
        greater+=sum_broadcast(local_greater);equal+=sum_broadcast(local_equal);
    }
    v_u32_st_tnsr_partial((int5){0,request},metadata,threshold,0,0);
    v_i32_st_tnsr_partial((int5){1,request},metadata,greater,0,0);
    }
}
