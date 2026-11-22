// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_score_order.h"
#if DSV41_SCORE_KEY_BITS == 32
#include "deepseek_v41_selection_bitmap.h"
#endif
static inline int64 broadcast_sum(int64 value) {
    value = v_i32_reduce_add(value);
    return v_i32_shuffle_b(value, (uchar256)0x80, 0, value);
}
void main(tensor scores, tensor metadata, int columns, int width) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        if (width == columns) continue;
        uint64 threshold = 0;
        for (int bit = DSV41_SCORE_KEY_BITS - 1; bit >= 0; --bit) {
            const uint64 trial = threshold | (1u << bit);
            int64 count = 0;
            for (int col = 0; col < columns; col += 64) {
                const uint64 key = dsv41_score_key(v_f32_ld_tnsr_b((int5){col,row}, scores));
                count += v_i32_mov_vb((int64)1, 0, (int64)0, v_u32_cmp_geq_b(key,trial), 0);
            }
            threshold = v_u32_sel_geq_i32_b(broadcast_sum(count), width, trial, threshold);
        }
        int64 greater = 0, equal = 0;
#if DSV41_SCORE_KEY_BITS == 32
        const int word_capacity = (columns + 127) / 128 * 4;
        for (int col = 0; col < columns; col += 128) {
            const uint64 key0 = dsv41_score_key(v_f32_ld_tnsr_b((int5){col,row},scores));
            const uint64 key1 = dsv41_score_key(v_f32_ld_tnsr_b((int5){col+64,row},scores));
            const bool64 valid0 = v_i32_cmp_less_b((int64)V_LANE_ID_32 + col,columns);
            const bool64 valid1 = v_i32_cmp_less_b((int64)V_LANE_ID_32 + col+64,columns);
            v_u32_st_tnsr_partial((int5){18+col/32,row},metadata,
                pack_words(pack_half(valid0 & v_u32_cmp_grt_b(key0,threshold)),
                           pack_half(valid1 & v_u32_cmp_grt_b(key1,threshold))),3,0);
            v_u32_st_tnsr_partial((int5){18+word_capacity+col/32,row},metadata,
                pack_words(pack_half(valid0 & v_u32_cmp_eq_b(key0,threshold)),
                           pack_half(valid1 & v_u32_cmp_eq_b(key1,threshold))),3,0);
        }
#endif
        // Eight disjoint input partitions emit in source order. Prefix counts
        // avoid atomics and keep tie handling deterministic across TPC cores.
        const int64 lanes = (int64)V_LANE_ID_32;
        for (int worker = 0; worker < 8; ++worker) {
            v_i32_st_tnsr_partial((int5){2 + worker * 2,row}, metadata, greater, 0, 0);
            v_i32_st_tnsr_partial((int5){3 + worker * 2,row}, metadata, equal, 0, 0);
            const int first = columns * worker / 8, last = columns * (worker + 1) / 8;
            int64 local_greater = 0, local_equal = 0;
            for (int col = first; col < last; col += 64) {
                const int count = last - col < 64 ? last - col : 64;
                const uint64 key = dsv41_score_key(v_f32_ld_tnsr_partial_b((int5){col,row}, scores, count - 1, 0));
                const bool64 valid = v_i32_cmp_less_b(lanes + col,last);
                local_greater += v_i32_mov_vb((int64)1,0,(int64)0,valid & v_u32_cmp_grt_b(key,threshold),0);
                local_equal += v_i32_mov_vb((int64)1,0,(int64)0,valid & v_u32_cmp_eq_b(key,threshold),0);
            }
            greater += broadcast_sum(local_greater);
            equal += broadcast_sum(local_equal);
        }
        v_u32_st_tnsr_partial((int5){0,row}, metadata, threshold, 0, 0);
        v_i32_st_tnsr_partial((int5){1,row}, metadata, greater, 0, 0);
    }
}
