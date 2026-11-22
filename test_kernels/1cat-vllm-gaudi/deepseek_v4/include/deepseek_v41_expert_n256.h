// SPDX-License-Identifier: Apache-2.0
// N256 weights: adjacent N nibbles at each K, one full FP8 vector store.
#ifndef DSV41_N256_N_BLOCKS_PER_POINT
#define DSV41_N256_N_BLOCKS_PER_POINT 1
#endif
#ifndef DSV41_N256_EXTERNAL_CHANNEL_CODE
#define DSV41_N256_EXTERNAL_CHANNEL_CODE 0
#endif
#ifndef DSV41_N256_K_TILE
#define DSV41_N256_K_TILE 128
#endif
#ifndef DSV41_N256_FP8
#define DSV41_N256_FP8 1
#endif
#ifndef DSV41_N256_NORMAL_BF16
#define DSV41_N256_NORMAL_BF16 0
#endif
#ifndef DSV41_N256_ROUTE_TILE
#define DSV41_N256_ROUTE_TILE 0
#endif
#ifndef DSV41_N256_UNROLL_ROWS
#define DSV41_N256_UNROLL_ROWS 0
#endif
#ifndef DSV41_N256_PREFETCH
#define DSV41_N256_PREFETCH 8
#endif
#ifndef DSV41_N256_INTERLEAVE_ALU
#define DSV41_N256_INTERLEAVE_ALU 0
#endif
#ifndef DSV41_N256_AFFINE_ROUTE
#define DSV41_N256_AFFINE_ROUTE 0
#endif
#ifndef DSV41_N256_GROUP_PIPELINE
#define DSV41_N256_GROUP_PIPELINE 0
#endif
#ifndef DSV41_N256_EXPLICIT_STEPS
#define DSV41_N256_EXPLICIT_STEPS 0
#endif
#ifndef DSV41_N256_UNROLL_STEPS
#define DSV41_N256_UNROLL_STEPS 0
#endif
#ifndef DSV41_N256_FUSE_SLOTS
#define DSV41_N256_FUSE_SLOTS 0
#endif
#ifndef DSV41_N256_HORIZONTAL
#define DSV41_N256_HORIZONTAL 0
#endif
#ifndef DSV41_N256_REUSE_SLOTS
#define DSV41_N256_REUSE_SLOTS 0
#endif

#ifndef DSV41_N256_CHANNEL_BATCH
#define DSV41_N256_CHANNEL_BATCH 0
#endif
#ifndef DSV41_N256_TOKEN_CHANNEL_BATCH
#define DSV41_N256_TOKEN_CHANNEL_BATCH 0
#endif
#if DSV41_N256_TOKEN_CHANNEL_BATCH
#define DSV41_N256_OUTPUT_COORD(N,K,S) ((int5){(N)%256,K,local_slot,(N)/256,token})
#elif DSV41_N256_CHANNEL_BATCH
#define DSV41_N256_OUTPUT_COORD(N,K,S) ((int5){(N)%256,K,S,(N)/256,0})
#else
#define DSV41_N256_OUTPUT_COORD(N,K,S) ((int5){N,K,S,0,0})
#endif

#if DSV41_N256_REUSE_SLOTS
#define DSV41_N256_WRITE(ENCODED) do { \
    destination[2] = slot; \
    v_f8_st_tnsr(destination, output, *((minifloat256*)&(ENCODED))); \
    int5 duplicate = destination; \
    duplicate[2] += 1; \
    v_f8_st_tnsr(duplicate, output, *((minifloat256*)&(ENCODED)), 0, repeat_end > slot + 1); \
} while (0)
#else
#define DSV41_N256_WRITE(ENCODED) \
    v_f8_st_tnsr(destination, output, *((minifloat256*)&(ENCODED)))
#endif

#ifndef DSV41_N256_SLOT_TILE
#define DSV41_N256_SLOT_TILE 1
#endif
#ifndef DSV41_N256_SAT_DECODE
#define DSV41_N256_SAT_DECODE 0
#endif
#ifndef DSV41_N256_BOUND_OUTPUT_K
#define DSV41_N256_BOUND_OUTPUT_K 0
#endif
#ifndef DSV41_N256_PACKED_BYTES
#define DSV41_N256_PACKED_BYTES 0
#endif
#define DSV41_N256_Q_WORD_BYTES (DSV41_N256_PACKED_BYTES ? 2 : 1)

#ifndef DSV41_N256_K_FIRST
#define DSV41_N256_K_FIRST 0
#endif

#ifndef DSV41_N256_SAT_DECODE
#define DSV41_N256_SAT_DECODE 0
#endif

static inline ushort128 exact_bf16(ushort128 nibble, ushort128 code)
{
    const ushort128 magnitude = nibble & 7;
    const ushort128 sign = (nibble & 8) << 12;
    ushort128 bits = (code << 7) + (magnitude << 6) - 128;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, bits - 64, bits);
    bits = v_u16_min_b(bits, 0x7f80);
    const ushort128 tiny = v_u16_sel_less_u16_b(magnitude, 4, magnitude << 5, bits);
    bits = v_u16_sel_eq_u16_b(code, 0, tiny, bits);
    const ushort128 half_value = v_u16_sel_eq_u16_b(magnitude, 1, 0x40, bits);
    bits = v_u16_sel_eq_u16_b(code, 1, half_value, bits);
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
    bits = v_u16_sel_eq_u16_b(code, 255, 0x7fc0, bits);
    return bits | sign;
}

#if DSV41_N256_SAT_DECODE
static inline uchar256 decode_sat_values(uchar256 weights, uchar256 table, uchar256 subtract) {
    const uchar256 direction = weights | 0x80;
    const uchar256 base = v_u8_shuffle_b(table, direction, 0, direction);
    return v_u8_sub_b(base, subtract, SW_SAT);
}
// Reuse the C2-C6 SAT decoder: offsets qualified in [-40,48].
#define DSV41_N256_STORE(WEIGHTS) do { \
    const uchar256 direction = (WEIGHTS) | 0x80; \
    const uchar256 base = v_u8_shuffle_b(table, direction, 0, direction); \
    const uchar256 encoded = v_u8_sub_b(base, subtract, SW_SAT); \
    DSV41_N256_WRITE(encoded); \
    destination[1] += 1; \
} while (0)
#else
#define DSV41_N256_STORE(WEIGHTS) do { \
    const uchar256 direction = (WEIGHTS) | 0x80; \
    const uchar256 base = v_u8_shuffle_b(table, direction, 0, direction); \
    const uchar256 encoded = v_u8_sel_eq_u8_b((WEIGHTS), 7, base, base + delta, SW_MASK_EQ_ZERO); \
    DSV41_N256_WRITE(encoded); \
    destination[1] += 1; \
} while (0)
#endif

#ifdef DSV41_N256_PAIRED_FUNCTION
static inline void decode_paired_half(tensor ids,tensor q16,tensor planes,tensor lookup,tensor output,
                                     const int5 start,const int5 end) {
#else
void main(tensor ids, tensor q16, tensor planes, tensor lookup,
#if DSV41_N256_EXTERNAL_CHANNEL_CODE
          tensor channel_codes,
#endif
          tensor output
#if DSV41_N256_BOUND_OUTPUT_K
          , int active_k
#endif
)
{
#if DSV41_N256_N_BLOCKS_PER_POINT > 1
    const int5 raw_start=get_index_space_offset();
    const int5 raw_end=raw_start+get_index_space_size();
    const int5 start={raw_start[0]*DSV41_N256_N_BLOCKS_PER_POINT,raw_start[1],raw_start[2],0,0};
    const int5 end={raw_end[0]*DSV41_N256_N_BLOCKS_PER_POINT,raw_end[1],raw_end[2],0,0};
#elif DSV41_N256_K_FIRST
    const int5 physical_start = get_index_space_offset();
    const int5 physical_end = physical_start + get_index_space_size();
    const int5 start = {physical_start[1], physical_start[2], physical_start[0], 0, 0};
    const int5 end = {physical_end[1], physical_end[2], physical_end[0], 0, 0};
#else
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
#endif
#endif
    const int experts = get_dim_size(q16, 2);
    const bool compact_scales = get_dim_size(planes, 0) == get_dim_size(q16, 0) / (16 * DSV41_N256_Q_WORD_BYTES)
#if !DSV41_N256_EXTERNAL_CHANNEL_CODE
        + 128
#endif
        ;
    const int scale_stride = compact_scales ? 128 : 256;
#if DSV41_N256_FP8 || DSV41_N256_NORMAL_BF16
    const uchar256 raw_table = v_u8_ld_tnsr_b((int5){0}, lookup);
#if DSV41_N256_SAT_DECODE
    const uchar256 table = v_u8_sel_eq_u8_b(raw_table, 127, 0, raw_table + 48, SW_MASK_EQ_ZERO);
#else
    const uchar256 table = raw_table;
#endif
#endif
#if DSV41_N256_FUSE_SLOTS
    const int first_slot = 0;
    const int last_slot = get_dim_size(ids, 0);
    const int first_group = start[1] * (DSV41_N256_K_TILE / 32);
    const int last_group = end[1] * (DSV41_N256_K_TILE / 32);
#if DSV41_N256_FP8
    const int first_k = start[1] * DSV41_N256_K_TILE;
    const int last_k = end[1] * DSV41_N256_K_TILE;
#endif
#else
#if DSV41_N256_REUSE_SLOTS
    const int first_slot = start[1] * DSV41_N256_REUSE_SLOTS;
    const int last_slot = s_i32_min(end[1] * DSV41_N256_REUSE_SLOTS, get_dim_size(ids, 0));
#else
    const int first_slot = start[1] * DSV41_N256_SLOT_TILE;
    const int last_slot = s_i32_min(end[1] * DSV41_N256_SLOT_TILE, get_dim_size(ids, 0));
#endif
    const int first_group = start[2] * (DSV41_N256_K_TILE / 32);
#if DSV41_N256_BOUND_OUTPUT_K
    const int last_group = s_i32_min(end[2] * (DSV41_N256_K_TILE / 32), (active_k + 31) / 32);
#else
    const int last_group =
#ifdef DSV41_N256_ACTIVE_K
        s_i32_min(end[2] * (DSV41_N256_K_TILE / 32), get_dim_size(output, 1) / 32);
#else
        end[2] * (DSV41_N256_K_TILE / 32);
#endif
#endif
#if DSV41_N256_FP8
    const int first_k = start[2] * DSV41_N256_K_TILE;
#if DSV41_N256_BOUND_OUTPUT_K
    const int last_k = s_i32_min(end[2] * DSV41_N256_K_TILE, active_k);
#else
    const int last_k =
#ifdef DSV41_N256_ACTIVE_K
        s_i32_min(end[2] * DSV41_N256_K_TILE, get_dim_size(output, 1));
#else
        end[2] * DSV41_N256_K_TILE;
#endif
#endif
#endif
#endif
#if DSV41_N256_TOKEN_CHANNEL_BATCH
    for(int token=start[3];token<end[3];++token)
        for(int local_slot=first_slot;local_slot<last_slot;++local_slot) {
        const int slot=token*6+local_slot;
        (void)slot;
#else
    for (int slot = first_slot; slot < last_slot; ++slot) {
#endif
#if !DSV41_N256_HORIZONTAL
#if DSV41_N256_TOKEN_CHANNEL_BATCH
        const int expert=s_i32_ld_g(gen_addr((int5){local_slot,token},ids));
#else
        const int expert = s_i32_ld_g(gen_addr((int5){slot}, ids));
#endif
#if DSV41_N256_REUSE_SLOTS
        // Sharing is restricted to one affine batch access tile. Synapse
        // can slice this axis with MME while preserving SRAM ownership.
        if (slot % DSV41_N256_REUSE_SLOTS != 0 && expert >= 0 && expert < experts &&
            expert == s_i32_ld_g(gen_addr((int5){slot - 1}, ids)))
            continue;
        int repeat_end = slot + 1;
        const int tile_end = s_i32_min(last_slot, (slot / DSV41_N256_REUSE_SLOTS + 1) * DSV41_N256_REUSE_SLOTS);
        while (repeat_end < tile_end &&
               expert == s_i32_ld_g(gen_addr((int5){repeat_end}, ids)))
            ++repeat_end;
#endif
#if DSV41_N256_FP8
        if (expert < 0 || expert >= experts) {
            for (int block = start[0]; block < end[0]; ++block) {
                for (int k = first_k; k < last_k; ++k) {
                    v_f8_st_tnsr(DSV41_N256_OUTPUT_COORD(block * 256, k, slot), output, (minifloat256){0});
                }
            }
            continue;
        }
#endif
#endif
        const int blocks_per_tile = DSV41_N256_ROUTE_TILE ? get_dim_size(q16, 1) : 1;
        for (int block = start[0] * blocks_per_tile; block < end[0] * blocks_per_tile; ++block) {
#if DSV41_N256_HORIZONTAL
            const int blocks_per_expert = get_dim_size(q16, 1);
#if DSV41_N256_AFFINE_ROUTE
            // The prepared horizontal packing contains exactly H experts.
            // Bound the quotient to [0,H): avoid runtime scalar udiv/mod for
            // every N256/K128 workpoint without specializing a shard width.
            int route = 0;
            int source_block = block;
            #pragma unroll
            for (int part = 1; part < DSV41_N256_HORIZONTAL; ++part) {
                const bool next = source_block >= blocks_per_expert;
                route += next;
                source_block = next ? source_block - blocks_per_expert : source_block;
            }
#else
            const int route = block / blocks_per_expert;
            const int source_block = block % blocks_per_expert;
#endif
            const int expert = s_i32_ld_g(gen_addr((int5){slot * DSV41_N256_HORIZONTAL + route}, ids));
            if (expert < 0 || expert >= experts) {
                for (int k = first_k; k < last_k; ++k)
                    v_f8_st_tnsr(DSV41_N256_OUTPUT_COORD(block * 256, k, slot), output, (minifloat256){0});
                continue;
            }
#else
            const int source_block = block;
#endif
            const int row = block * 256;
#if DSV41_N256_FP8
            const uchar256 channel_code = v_u8_ld_tnsr_b(
#if DSV41_N256_EXTERNAL_CHANNEL_CODE
                (int5){0, source_block, expert}, channel_codes,
#else
                (int5){get_dim_size(planes, 0) - 128, source_block, expert}, planes,
#endif
                0, (uchar256){0}, compact_scales);
#endif
#if DSV41_N256_GROUP_PIPELINE
#if DSV41_N256_PREFETCH != 8 || !DSV41_N256_FP8 || !DSV41_N256_SAT_DECODE
#error "Group pipeline preserves the existing eight-vector SAT window"
#endif
            // Keep the same eight live weight vectors across scale-group
            // boundaries. Parent restarts eight loads after flushing eight
            // stores at every K32 group; the carried state removes that gap.
            const int initial_scale=first_group*scale_stride+(compact_scales?0:128);
            const uchar256 initial_stored=v_u8_ld_tnsr_b((int5){initial_scale,source_block,expert},planes);
            const uchar256 initial_delta=compact_scales?(initial_stored-channel_code)<<3:initial_stored;
            uchar256 prepared_subtract=(uchar256)48-initial_delta;
            int5 first_source={first_group*2048*DSV41_N256_Q_WORD_BYTES,source_block,expert};
#define DSV41_FIRST_PENDING(N) \
            uchar256 pending##N=v_u8_ld_tnsr_b(first_source,q16,SW_UNPACK | SW_UNPCK_4_TO_8); \
            first_source[0]+=64*DSV41_N256_Q_WORD_BYTES
            DSV41_FIRST_PENDING(0);DSV41_FIRST_PENDING(1);
            DSV41_FIRST_PENDING(2);DSV41_FIRST_PENDING(3);
            DSV41_FIRST_PENDING(4);DSV41_FIRST_PENDING(5);
            DSV41_FIRST_PENDING(6);DSV41_FIRST_PENDING(7);
#undef DSV41_FIRST_PENDING
#endif
            for (int group = first_group; group < last_group; ++group) {
#if DSV41_N256_FP8
#if DSV41_N256_GROUP_PIPELINE
                const uchar256 subtract=prepared_subtract;
#else
                const int scale_offset = group * scale_stride + (compact_scales ? 0 : 128);
                const uchar256 stored = v_u8_ld_tnsr_b((int5){scale_offset, source_block, expert}, planes);
                const uchar256 delta = compact_scales ? (stored - channel_code) << 3 : stored;
#if DSV41_N256_SAT_DECODE
                const uchar256 subtract = (uchar256)48 - delta;
#endif
#endif
#else
                const bool valid = expert >= 0 && expert < experts;
                const uchar256 original = v_u8_ld_tnsr_b(
                    (int5){group * scale_stride, source_block, expert}, planes, 0, (uchar256){0}, valid);
                const ushort256 codes = convert_uchar256_to_ushort256(original, SW_LINEAR);
#if DSV41_N256_NORMAL_BF16
                const ushort128 scale_low_bits = codes.v1 << 7;
                const ushort128 scale_high_bits = codes.v2 << 7;
                const bfloat128 scale_low = *((bfloat128*)&scale_low_bits);
                const bfloat128 scale_high = *((bfloat128*)&scale_high_bits);
                const ushort128 negative_zero_bits = 0x8000;
                const bfloat128 negative_zero = *((bfloat128*)&negative_zero_bits);
#endif
#endif
                int5 source = {group * 2048 * DSV41_N256_Q_WORD_BYTES, source_block, expert};
                int5 destination = DSV41_N256_OUTPUT_COORD(row, group * 32, slot);
#if DSV41_N256_FP8
#if DSV41_N256_GROUP_PIPELINE
                source[0]+=512*DSV41_N256_Q_WORD_BYTES;
#else
                // Keep the reference eight-vector schedule available so the
                // production consumer can compare both binaries in one graph.
                uchar256 pending0 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending1 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending2 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending3 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending4 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending5 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending6 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending7 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
#endif
#if DSV41_N256_EXPLICIT_STEPS
#if DSV41_N256_PREFETCH != 8 || DSV41_N256_GROUP_PIPELINE || DSV41_N256_INTERLEAVE_ALU
#error "Explicit steps retain the unchanged eight-vector SAT schedule"
#endif
#define DSV41_EIGHT_STEP() do { \
    const uchar256 next0 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next1 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next2 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next3 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next4 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next5 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next6 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    const uchar256 next7 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    DSV41_N256_STORE(pending0); \
    DSV41_N256_STORE(pending1); \
    DSV41_N256_STORE(pending2); \
    DSV41_N256_STORE(pending3); \
    DSV41_N256_STORE(pending4); \
    DSV41_N256_STORE(pending5); \
    DSV41_N256_STORE(pending6); \
    DSV41_N256_STORE(pending7); \
    pending0 = next0; \
    pending1 = next1; \
    pending2 = next2; \
    pending3 = next3; \
    pending4 = next4; \
    pending5 = next5; \
    pending6 = next6; \
    pending7 = next7; \
} while (0)
                DSV41_EIGHT_STEP();
                DSV41_EIGHT_STEP();
                DSV41_EIGHT_STEP();
#undef DSV41_EIGHT_STEP
#else
#if DSV41_N256_PREFETCH == 16
                uchar256 pending8 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending9 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending10 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending11 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending12 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending13 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending14 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                uchar256 pending15 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                for (int batch = 0; batch < 1; ++batch) {
#else
#if DSV41_N256_UNROLL_STEPS || DSV41_N256_UNROLL_ROWS
                // The three iterations are fixed by a K32 scale group.
                // Removing their loop-carried register copies gives the
                // load slot back to FP4 loads; the live window remains eight.
                #pragma unroll(3)
#endif
                for (int batch = 0; batch < 3; ++batch) {
#endif
#if DSV41_N256_INTERLEAVE_ALU
                    // Preserve the existing 8/16-vector lookahead and SRAM
                    // geometry. Only interleave the next tensor load with the
                    // three ALU operations/store on its independent old value.
#define DSV41_ADVANCE(PENDING) do { \
    const uchar256 next = v_u8_ld_tnsr_b(source,q16,SW_UNPACK | SW_UNPCK_4_TO_8); \
    source[0] += 64 * DSV41_N256_Q_WORD_BYTES; \
    DSV41_N256_STORE(PENDING); \
    PENDING = next; \
} while (0)
                    DSV41_ADVANCE(pending0);
                    DSV41_ADVANCE(pending1);
                    DSV41_ADVANCE(pending2);
                    DSV41_ADVANCE(pending3);
                    DSV41_ADVANCE(pending4);
                    DSV41_ADVANCE(pending5);
                    DSV41_ADVANCE(pending6);
                    DSV41_ADVANCE(pending7);
#if DSV41_N256_PREFETCH == 16
                    DSV41_ADVANCE(pending8);
                    DSV41_ADVANCE(pending9);
                    DSV41_ADVANCE(pending10);
                    DSV41_ADVANCE(pending11);
                    DSV41_ADVANCE(pending12);
                    DSV41_ADVANCE(pending13);
                    DSV41_ADVANCE(pending14);
                    DSV41_ADVANCE(pending15);
#endif
#undef DSV41_ADVANCE
#else
                    const uchar256 next0 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next1 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next2 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next3 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next4 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next5 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next6 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next7 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
#if DSV41_N256_PREFETCH == 16
                    const uchar256 next8 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next9 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next10 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next11 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next12 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next13 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next14 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    const uchar256 next15 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
#endif
                    DSV41_N256_STORE(pending0);
                    DSV41_N256_STORE(pending1);
                    DSV41_N256_STORE(pending2);
                    DSV41_N256_STORE(pending3);
                    DSV41_N256_STORE(pending4);
                    DSV41_N256_STORE(pending5);
                    DSV41_N256_STORE(pending6);
                    DSV41_N256_STORE(pending7);
#if DSV41_N256_PREFETCH == 16
                    DSV41_N256_STORE(pending8);
                    DSV41_N256_STORE(pending9);
                    DSV41_N256_STORE(pending10);
                    DSV41_N256_STORE(pending11);
                    DSV41_N256_STORE(pending12);
                    DSV41_N256_STORE(pending13);
                    DSV41_N256_STORE(pending14);
                    DSV41_N256_STORE(pending15);
#endif
                    pending0 = next0;
                    pending1 = next1;
                    pending2 = next2;
                    pending3 = next3;
                    pending4 = next4;
                    pending5 = next5;
                    pending6 = next6;
                    pending7 = next7;
#if DSV41_N256_PREFETCH == 16
                    pending8 = next8;
                    pending9 = next9;
                    pending10 = next10;
                    pending11 = next11;
                    pending12 = next12;
                    pending13 = next13;
                    pending14 = next14;
                    pending15 = next15;
#endif
#endif
                }
#endif // DSV41_N256_EXPLICIT_STEPS
#if DSV41_N256_GROUP_PIPELINE
                if(group+1<last_group) {
                    // Fetch and prepare the following scale while the old
                    // group's final eight values still have useful work.
                    const int next_scale=(group+1)*scale_stride+(compact_scales?0:128);
                    const uchar256 next_stored=v_u8_ld_tnsr_b((int5){next_scale,source_block,expert},planes);
                    const uchar256 next_delta=compact_scales?(next_stored-channel_code)<<3:next_stored;
                    const uchar256 next_subtract=(uchar256)48-next_delta;
#define DSV41_CARRY_PENDING(N) do { \
                    const uchar256 next=v_u8_ld_tnsr_b(source,q16,SW_UNPACK | SW_UNPCK_4_TO_8); \
                    source[0]+=64*DSV41_N256_Q_WORD_BYTES; \
                    DSV41_N256_STORE(pending##N); \
                    pending##N=next; \
                } while(0)
                    DSV41_CARRY_PENDING(0);DSV41_CARRY_PENDING(1);
                    DSV41_CARRY_PENDING(2);DSV41_CARRY_PENDING(3);
                    DSV41_CARRY_PENDING(4);DSV41_CARRY_PENDING(5);
                    DSV41_CARRY_PENDING(6);DSV41_CARRY_PENDING(7);
                    prepared_subtract=next_subtract;
#undef DSV41_CARRY_PENDING
                } else {
#endif
                DSV41_N256_STORE(pending0);
                DSV41_N256_STORE(pending1);
                DSV41_N256_STORE(pending2);
                DSV41_N256_STORE(pending3);
                DSV41_N256_STORE(pending4);
                DSV41_N256_STORE(pending5);
                DSV41_N256_STORE(pending6);
                DSV41_N256_STORE(pending7);
#if DSV41_N256_GROUP_PIPELINE
                }
#endif
#if DSV41_N256_PREFETCH == 16
                DSV41_N256_STORE(pending8);
                DSV41_N256_STORE(pending9);
                DSV41_N256_STORE(pending10);
                DSV41_N256_STORE(pending11);
                DSV41_N256_STORE(pending12);
                DSV41_N256_STORE(pending13);
                DSV41_N256_STORE(pending14);
                DSV41_N256_STORE(pending15);
#endif
#else
                #pragma unroll (8)
                for (int part = 0; part < 32; ++part) {
#if DSV41_N256_FP8
                    const uchar256 unpacked = v_u8_ld_tnsr_b(
                        source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    // Every shuffle lane is enabled; its income is never used.
                    // Reuse the direction register instead of clearing a vector.
                    const uchar256 direction = unpacked | 0x80;
                    const uchar256 base = v_u8_shuffle_b(table, direction, 0, direction);
                    const uchar256 encoded = v_u8_sel_eq_u8_b(
                        unpacked, 7, base, base + delta, SW_MASK_EQ_ZERO);
                    v_f8_st_tnsr(destination, output, *((minifloat256*)&encoded));
#else
                    const uchar256 unpacked = v_u8_ld_tnsr_b(
                        source, q16,
                        SW_UNPACK | SW_UNPCK_4_TO_8, (uchar256){0}, valid);
#if DSV41_N256_NORMAL_BF16
                    // This GUID is selected only for qualified UE8M0 codes
                    // 2..254. The tiny/NaN encodings retain exact_bf16 below.
                    const uchar256 directions = unpacked | 0x80;
                    const uchar256 bits = v_u8_shuffle_b(table, directions, 0, unpacked);
                    const bfloat256 base = convert_minifloat256_to_bfloat256(*((minifloat256*)&bits), SW_LINEAR);
                    const bfloat128 low = v_bf16_madd_b(base.v1, scale_low, negative_zero);
                    const bfloat128 high = v_bf16_madd_b(base.v2, scale_high, negative_zero);
                    v_bf16_st_tnsr(destination, output, low);
                    int5 upper = destination;
                    upper[0] += 128;
                    v_bf16_st_tnsr(upper, output, high);
#else
                    const ushort256 nibble = convert_uchar256_to_ushort256(unpacked, SW_LINEAR);
                    const ushort128 low = exact_bf16(nibble.v1, codes.v1);
                    const ushort128 high = exact_bf16(nibble.v2, codes.v2);
                    v_bf16_st_tnsr(destination, output, *((bfloat128*)&low));
                    int5 upper = destination;
                    upper[0] += 128;
                    v_bf16_st_tnsr(upper, output, *((bfloat128*)&high));
#endif
#endif
                    source[0] += 64 * DSV41_N256_Q_WORD_BYTES;
                    destination[1] += 1;
                }
#endif
            }
        }
    }
}

#undef DSV41_N256_STORE
