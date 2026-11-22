// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE (paged): y = y_ovf + sum over local slots k with a row (bm >= 0) of
// w_k * Y2[:, row % PS, row / PS]  (Y2: down batch GEMM output over pages, bf16 sigma rows).
// tensors: 0 y2 (H, PS, P) bf16 (PS: page rows, a power of 2)  1 yo (H,T) f32  2 ids (64,T) i32  3 bm (64,T) i32  4 wts (64,T) f32
//          5 meta i32 | 6 y (H,T) f32
// params: lo, E (local experts), K. index space: [1, T] (a member: its token's slots once, all H)
#include "common.h"

#define KMAX 16

void main(tensor y2, tensor yo, tensor ids, tensor bm, tensor wts, tensor meta, tensor y, int lo, int E, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    const int PS = get_dim_size(y2, 1), psh = PS == 128 ? 7 : PS == 256 ? 8 : 6;   // 64, 128 or 256 rows
    const int H = get_dim_size(y, 0);
    for (int t = off[1]; t < end[1]; t++) {
        // this token's paged slots: (row, weight)
        int rows[KMAX];
        float ws[KMAX];
        int nk = 0;
        for (int k = 0; k < K && t < n_real; k++) {
            int5 ic = {k, t, 0, 0, 0};
            const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
            const int row = s_i32_ld_g(gen_addr(ic, bm));
            if (e < 0 || e >= E || row < 0)
                continue;
            rows[nk] = row;
            ws[nk] = s_f32_ld_g(gen_addr(ic, wts));
            nk++;
        }
        for (int i = 0; i < H; i += 128) {
            int5 c = {i, t, 0, 0, 0};
            float64 a0 = v_f32_ld_tnsr_b(c, yo);
            c[0] += 64;
            float64 a1 = v_f32_ld_tnsr_b(c, yo);
            for (int j = 0; j < nk; j++) {
                int5 yc = {i, rows[j] & (PS - 1), rows[j] >> psh, 0, 0};
                const float128 v = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(yc, y2));
                a0 = v_f32_mac_b(v.v1, ws[j], a0);
                a1 = v_f32_mac_b(v.v2, ws[j], a1);
            }
            c[0] = i;
            v_f32_st_tnsr(c, y, a0);
            c[0] += 64;
            v_f32_st_tnsr(c, y, a1);
        }
    }
}
