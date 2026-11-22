// SPDX-License-Identifier: Apache-2.0
// Retain BF16 head products and both TP partial-sum boundaries while keeping
// the relu, products and head accumulators in registers.
static inline float128 sum_heads(tensor scores, bfloat128 weights, int token,
                                 int tile, int first_head, int local_heads) {
    float128 total;
    total.v1 = 0;
    total.v2 = 0;
    // Fixed head groups remove per-head predication. Unrolling exposes the
    // independent SRAM loads while preserving the ordered FP32 recurrence.
    #pragma unroll (4)
    for (int offset = 0; offset < local_heads; ++offset) {
        const int head = first_head + offset;
        const int5 score_at = {tile * 128, head, token, 0, 0};
        // SHUFFLE directions select bytes within each replicated dual group.
        const uchar256 direction = ((uchar256)V_LANE_ID_8 & 1) | (head * 2) | 0x80;
        const uchar256 gain_bytes = v_u8_shuffle_b(*((uchar256*)&weights), direction, 0,
                                                  *((uchar256*)&weights));
        const bfloat128 gain = *((bfloat128*)&gain_bytes);
        const bfloat128 positive = v_bf16_max_b(v_bf16_ld_tnsr_b(score_at, scores), (bfloat)0);
        const bfloat128 product = v_bf16_mul_b(positive, gain);
        const float128 value = convert_bfloat128_to_float128(product, 0);
        total.v1 = v_f32_add_b(total.v1, value.v1);
        total.v2 = v_f32_add_b(total.v2, value.v2);
    }
    return total;
}

void main(tensor scores, tensor weights, tensor positions, tensor rows,
          tensor output, int ratio, int local_heads) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int token = begin[1]; token < end[1]; ++token) {
        const int5 position_at = {token, 0, 0, 0, 0};
        const int count = (s_i32_ld_g(gen_addr(position_at, positions)) + 1) / ratio;
        // Reuse one bounded weight load across this core's source tiles.
        bfloat128 gains = v_bf16_ld_tnsr_partial_b((int5){0, token}, weights, 31, 0);
        gains = v_bf16_mov_dual_group_all_b(gains, 0xffffffff, 0, 0, 0, 0,
                                           MkWrA(3, 3, 3, 3), gains);
        for (int tile = begin[0]; tile < end[0]; ++tile) {
            float128 total = {0};
            for (int head = 0; head < 32; head += local_heads) {
                const bfloat128 partial = convert_float128_to_bfloat128(
                    sum_heads(scores, gains, token, tile, head, local_heads), SW_RHNE);
                // Invert the packed BF16 conversion in the same lane layout.
                // Linearize only once, when storing the final F32 tensor.
                const float128 rounded = convert_bfloat128_to_float128(partial, 0);
                total.v1 = v_f32_add_b(total.v1, rounded.v1);
                total.v2 = v_f32_add_b(total.v2, rounded.v2);
            }
            const bfloat128 combined = convert_float128_to_bfloat128(total, SW_RHNE);
            const float128 reduced = convert_bfloat128_to_float128(combined, SW_LINEAR);
            const int owner = get_dim_size(rows, 1) > 1 ? token : 0;
            const int5 low_row = {tile * 128, owner, 0, 0, 0};
            const int5 high_row = {tile * 128 + 64, owner, 0, 0, 0};
            const int64 lo = v_i32_ld_tnsr_b(low_row, rows);
            const int64 hi = v_i32_ld_tnsr_b(high_row, rows);
            const float64 invalid = as_float64((uint64)0xff800000);
            float64 low = v_f32_sel_less_i32_b(lo, 0, invalid, reduced.v1);
            float64 high = v_f32_sel_less_i32_b(hi, 0, invalid, reduced.v2);
            low = v_f32_sel_geq_i32_b(lo, count, invalid, low);
            high = v_f32_sel_geq_i32_b(hi, count, invalid, high);
            const int5 low_out = {tile * 128, token, 0, 0, 0};
            const int5 high_out = {tile * 128 + 64, token, 0, 0, 0};
            v_f32_st_tnsr(low_out, output, low);
            v_f32_st_tnsr(high_out, output, high);
        }
    }
}
