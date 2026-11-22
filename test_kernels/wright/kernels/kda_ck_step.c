// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked KDA, stage 4 (sequential over chunks): S' = diag(e^G_63) S - X + B per head, X = KW S
// (KW = kL^T W), B = kL^T U. S rows are key channels (dim 1), columns value channels (dim 0).
// tensors: 0 S (128,128,nh,1) f32  1 X  2 B  3 cs (192, nh, 1) f32 (rows 0..127 = e^G_63) | 4 S' (128,128,nh,1)
// index space: [nh]
#include "common.h"

void main(tensor S, tensor X, tensor B, tensor cs, tensor So)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    for (int h = off[0]; h < end[0]; h++) {
        int5 ec = {0, h, 0, 0, 0};
        const float64 e0 = v_f32_ld_tnsr_b(ec, cs);
        ec[0] = 64;
        const float64 e1 = v_f32_ld_tnsr_b(ec, cs);
#define ROWS(E, R0, DG) {                                                                   \
            const float64 Ed = BCAST_DG(E, DG);                                              \
            _Pragma("unroll 4")                                                              \
            for (int l = 0; l < 16; l++) {                                                   \
                const float64 d = v_f32_shuffle_b(Ed, IDX[l], 0, Ed);                        \
                const int r = (R0) + 16 * (DG) + l;                                          \
                int5 c = {0, r, h, 0, 0};                                                    \
                float64 a = v_f32_ld_tnsr_b(c, S), x = v_f32_ld_tnsr_b(c, X), b = v_f32_ld_tnsr_b(c, B); \
                c[0] = 64;                                                                   \
                float64 a2 = v_f32_ld_tnsr_b(c, S), x2 = v_f32_ld_tnsr_b(c, X), b2 = v_f32_ld_tnsr_b(c, B); \
                v_f32_st_tnsr(c, So, v_f32_mac_b(a2, d, b2 - x2));                           \
                c[0] = 0;                                                                    \
                v_f32_st_tnsr(c, So, v_f32_mac_b(a, d, b - x));                              \
            } }
        ROWS(e0, 0, 0) ROWS(e0, 0, 1) ROWS(e0, 0, 2) ROWS(e0, 0, 3)
        ROWS(e1, 64, 0) ROWS(e1, 64, 1) ROWS(e1, 64, 2) ROWS(e1, 64, 3)
#undef ROWS
    }
}
