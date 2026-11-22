// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE: SwiGLU on the gate/up batch GEMM output (bf16, rows in sigma order per 128 block):
//   h = silu(gate) * up, bf16 sigma order (the down GEMM operand). F = rows / 2.
// Padding rows (gidx[n] >= n_real: unused page rows) are skipped: the down GEMM is row-independent and
// moe_combine_p never reads them.
// tensors: 0 y1 (2F, N) bf16  1 gidx (N,) i32  2 meta i32 [pos0, n_real] | 3 h (F, N) bf16.
// index space: [N/64] (a member: 64 rows, skipped together when all are padding: one scalar check;
// real rows fill each page from its start, so a 64-row group is almost always all real or all padding)
#include "common.h"

#ifdef LIM
void main(tensor y1, tensor gidx, tensor meta, tensor h, float lim)
#else
void main(tensor y1, tensor gidx, tensor meta, tensor h)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int F = get_dim_size(h, 0), N = get_dim_size(h, 1);
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    for (int grp = off[0]; grp < end[0]; grp++) {
        // real rows are a prefix of each page (pages are 64-row multiples): the group is all padding
        // when its first row is
        int5 gc = {64 * grp, 0, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(gc, gidx)) >= n_real)
            continue;
        const int n1 = 64 * grp + 64 < N ? 64 * grp + 64 : N;
        for (int b = 0; b < F / 128; b++)
            for (int n = 64 * grp; n < n1; n += 4) {     // 4 rows per step, loads first
                float128 g[4], u[4];
                #pragma unroll
                for (int r = 0; r < 4; r++) {
                    int5 c = {128 * b, n + r, 0, 0, 0};
                    g[r] = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c, y1));   // natural halves
                    c[0] = F + 128 * b;
                    u[r] = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c, y1));
                }
                #pragma unroll
                for (int r = 0; r < 4; r++) {
                    int5 c = {128 * b, n + r, 0, 0, 0};
                    v_bf16_st_tnsr(c, h, pack_sigma(nl_silus(CLAMP_G(g[r].v1)) * CLAMP_U(u[r].v1), nl_silus(CLAMP_G(g[r].v2)) * CLAMP_U(u[r].v2)));
                }
            }
    }
}
