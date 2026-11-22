// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked gated delta rule, chunk-to-chunk state step (one node per chunk, after the product
// X = KW_c S_c):   S_{c+1} = gamma_C S_c - X + B_c,   gamma_C = exp(G_63) of chunk c (per head).
// tensors: 0 S (128,128,nv,1) f32  1 X  2 B  3 GB (64,2,nv,1) | 4 Snew (128,128,nv,1)
// index space: [nv, 16] (8 state rows per member)
#include "common.h"

void main(tensor S, tensor X, tensor B, tensor GB, tensor Sn)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int h = off[0]; h < end[0]; h++) {
        int5 gc = {0, 0, h, 0, 0};
        float64 GCv = v_f32_sel_less_f32_b(lanef, 62.5f, 0.0f, v_f32_ld_tnsr_b(gc, GB));
        float64 GC;
        NL_REDUCE_ADD(GCv, IDX, GC)
        const float64 gam = nl_exp(GC);
        for (int rb = off[1]; rb < end[1]; rb++) {
            for (int i = rb * 8; i < rb * 8 + 8; i++) {
                int5 c = {0, i, h, 0, 0};
                v_f32_st_tnsr(c, Sn, v_f32_mac_b(gam, v_f32_ld_tnsr_b(c, S), v_f32_ld_tnsr_b(c, B) - v_f32_ld_tnsr_b(c, X)));
                c[0] = 64;
                v_f32_st_tnsr(c, Sn, v_f32_mac_b(gam, v_f32_ld_tnsr_b(c, S), v_f32_ld_tnsr_b(c, B) - v_f32_ld_tnsr_b(c, X)));
            }
        }
    }
}
