// SPDX-License-Identifier: Apache-2.0
// Fuse the BF16 ReLU/weight/two-shard reduction after the index MME.
// The two explicit BF16 boundaries match the checkpoint scorer.
static inline float64 round_bf16(float64 value) {
    const bfloat128 rounded = v_convert_f32_to_bf16_all_b((float128){value, value}, SW_RHNE);
    return v_convert_bf16_to_f32_all_b(rounded).v1;
}

void main(tensor raw_scores, tensor weights, tensor positions, tensor output, int ratio, int local_heads) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int64 lanes = (int64)V_LANE_ID_32;
    for (int request = begin[1]; request < end[1]; ++request) {
        const int valid = (s_i32_ld_g(gen_addr((int5){request}, positions)) + 1) / ratio;
        for (int block = begin[0]; block < end[0]; ++block) {
            const int offset = block * 64;
            const int5 output_at = {offset, request, 0, 0, 0};
            if (offset >= valid) {
                v_f32_st_tnsr_partial(output_at, output, as_float64((int64)0xff800000), 63, 0);
                continue;
            }
            float64 total = 0;
            for (int first_head = 0; first_head < 32; first_head += local_heads) {
                float64 partial = 0;
                #pragma unroll (4)
                for (int local_head = 0; local_head < local_heads; ++local_head) {
                    const int head = first_head + local_head;
                    const bfloat128 packed = v_bf16_ld_tnsr_partial_b(
                        (int5){offset, head, request, 0, 0}, raw_scores, 63, 0);
                    float64 score = convert_bfloat128_to_float128(packed, SW_LINEAR).v1;
                    score = v_f32_max_b(score, 0.0f);
                    const bfloat weight = s_bf16_ld_g(gen_addr((int5){head, request}, weights));
                    partial += round_bf16(score * s_convert_bf16_to_f32(weight, 0));
                }
                total += round_bf16(partial);
            }
            float64 reduced = round_bf16(total);
            reduced = v_f32_mov_vb(reduced, 0, as_float64((int64)0xff800000),
                                    v_i32_cmp_less_b(lanes + offset, valid), 0);
            v_f32_st_tnsr_partial(output_at, output, reduced, 63, 0);
        }
    }
}
