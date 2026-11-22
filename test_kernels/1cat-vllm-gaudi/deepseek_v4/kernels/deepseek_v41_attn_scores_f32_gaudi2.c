// SPDX-License-Identifier: Apache-2.0
// Preserve the original BF16 dot-product instruction and reduction order.
void main(tensor q, tensor kv, tensor indices, tensor scale, tensor lengths, tensor scores)
{
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int width = get_dim_size(indices, 0);
    const int rows = get_dim_size(kv, 1);
    const int5 zero = {0};
    const float factor = s_f32_ld_g(gen_addr(zero, scale));
    for (int token = begin[2]; token < end[2]; ++token) {
        const int5 lc = {token, 0, 0, 0, 0};
        int count = s_i32_ld_g(gen_addr(lc, lengths));
        count = count < 0 ? 0 : count > width ? width : count;
        for (int tile = begin[1]; tile < end[1]; ++tile) {
            for (int head = begin[0]; head < end[0]; ++head) {
                bfloat128 query[4];
                #pragma unroll (4)
                for (int c = 0; c < 4; ++c) {
                    const int5 qc = {c * 128, head, token, 0, 0};
                    query[c] = v_bf16_ld_tnsr_b(qc, q);
                }
                for (int position = tile * 8; position < (tile + 1) * 8 && position < count; ++position) {
                    const int5 ic = {position, token, 0, 0, 0};
                    const int row = s_i32_ld_g(gen_addr(ic, indices));
                    if (row < 0 || row >= rows) continue;
                    float128 dot = {0};
                    #pragma unroll (4)
                    for (int c = 0; c < 4; ++c) {
                        const int5 kc = {c * 128, row, 0, 0, 0};
                        const bfloat128 key = v_bf16_ld_tnsr_b(kc, kv);
                        dot = v_bf16_mac_acc32_b(query[c], key, dot, (e_no_negation) << 1);
                    }
                    float64 value = dot.v1 + dot.v2;
                    value = v_f32_reduce_add(value);
                    value = v_f32_shuffle_b(value, (uchar256)0x80, 0, value);
                    value *= factor;
                    const int5 out = {head, position, token, 0, 0};
                    v_f32_st_tnsr_partial(out, scores, value, 0, 0);
                }
            }
        }
    }
}
