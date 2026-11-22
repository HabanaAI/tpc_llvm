// SPDX-License-Identifier: Apache-2.0
// g2e Direct MME+SRAM GDN conv state commit for one slot:
//   Copies 3 rows (rows[0..2]) of seq [C, T+3, NG] BF16 -> conv_out [C, 3, NG] BF16.
// Tensors (dim 0 fastest):
//   0 seq      BF16 [C, T+3, NG]
//   1 rows     I32  [3]
//   2 conv_out BF16 [C, 3, NG]
// Index space: dim 1 = layer in [0..NG) (48 layers -> 2 layers/TPC across 24 TPCs).

void main(tensor seq, tensor rows, tensor conv_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int C = get_dim_size(seq, 0);
    int5 rc0 = {0, 0, 0, 0, 0}, rc1 = {1, 0, 0, 0, 0}, rc2 = {2, 0, 0, 0, 0};
    const int r0 = s_i32_ld_g((__global__ int*)gen_addr(rc0, rows));
    const int r1 = s_i32_ld_g((__global__ int*)gen_addr(rc1, rows));
    const int r2 = s_i32_ld_g((__global__ int*)gen_addr(rc2, rows));

    for (int lyr = start[1]; lyr < end[1]; ++lyr) {
        for (int col = 0; col < C; col += 256) {
            int5 s0a = {col, r0, lyr, 0, 0}, s0b = {col + 128, r0, lyr, 0, 0};
            int5 s1a = {col, r1, lyr, 0, 0}, s1b = {col + 128, r1, lyr, 0, 0};
            int5 s2a = {col, r2, lyr, 0, 0}, s2b = {col + 128, r2, lyr, 0, 0};
            const bfloat128 v0a = v_bf16_ld_tnsr_b(s0a, seq);
            const bfloat128 v0b = v_bf16_ld_tnsr_b(s0b, seq);
            const bfloat128 v1a = v_bf16_ld_tnsr_b(s1a, seq);
            const bfloat128 v1b = v_bf16_ld_tnsr_b(s1b, seq);
            const bfloat128 v2a = v_bf16_ld_tnsr_b(s2a, seq);
            const bfloat128 v2b = v_bf16_ld_tnsr_b(s2b, seq);
            int5 d0a = {col, 0, lyr, 0, 0}, d0b = {col + 128, 0, lyr, 0, 0};
            int5 d1a = {col, 1, lyr, 0, 0}, d1b = {col + 128, 1, lyr, 0, 0};
            int5 d2a = {col, 2, lyr, 0, 0}, d2b = {col + 128, 2, lyr, 0, 0};
            v_bf16_st_tnsr(d0a, conv_out, v0a);
            v_bf16_st_tnsr(d0b, conv_out, v0b);
            v_bf16_st_tnsr(d1a, conv_out, v1a);
            v_bf16_st_tnsr(d1b, conv_out, v1b);
            v_bf16_st_tnsr(d2a, conv_out, v2a);
            v_bf16_st_tnsr(d2b, conv_out, v2b);
        }
    }
}
