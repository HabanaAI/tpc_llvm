// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked KDA, stage 2, per (head h, chunk c): the decay is already folded into the Gram products
// (kda_ck_in's scaled operands), so from KK[t][s] = decayed k_t.k_s and QK[t][s] = decayed q_t.k_s:
//   A[t][s] = beta_t KK[t][s] (s < t)      P[t][s] = QK[t][s] (s <= t)
//   T = (I + A)^-1 by forward substitution: T_t = e_t - sum_{s<t} A[t][s] T_s
// (as gdn_ck_solve without the scalar-gate foldings; T then multiplies beta v and beta k e^G).
// tensors: 0 KK (64,64,nh,NC) f32  1 QK  2 cs (192, nh, NC) f32 (rows 128.. = beta) | 3 P  4 Tm (64,64,nh,NC)
// index space: [nh, NC]
#include "common.h"

#define CH 64

__local__ float64 TV[CH];          // rows of T
__local__ float64 AV[CH];          // rows of A

void main(tensor KK, tensor QK, tensor cs, tensor P, tensor Tm)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int h = off[0]; h < end[0]; h++)
        for (int c = off[1]; c < end[1]; c++) {
            int5 bc = {128, h, c, 0, 0};
            const float64 Bt = v_f32_ld_tnsr_b(bc, cs);
            #pragma unroll 8
            for (int s2 = 0; s2 < CH; s2++) TV[s2] = 0.0f;
#define COLS(DS) if ((DS) * 16 <= t) {                                                      \
                const float64 Ad = BCAST_DG(arow, DS);                                       \
                float64 c0 = 0.0f, c1 = 0.0f, c2 = 0.0f, c3 = 0.0f;                          \
                _Pragma("unroll")                                                            \
                for (int es = 0; es < 16; es += 4) {                                         \
                    c0 = v_f32_mac_b(v_f32_shuffle_b(Ad, IDX[es], 0, Ad), TV[(DS) * 16 + es], c0);         \
                    c1 = v_f32_mac_b(v_f32_shuffle_b(Ad, IDX[es + 1], 0, Ad), TV[(DS) * 16 + es + 1], c1); \
                    c2 = v_f32_mac_b(v_f32_shuffle_b(Ad, IDX[es + 2], 0, Ad), TV[(DS) * 16 + es + 2], c2); \
                    c3 = v_f32_mac_b(v_f32_shuffle_b(Ad, IDX[es + 3], 0, Ad), TV[(DS) * 16 + es + 3], c3); \
                }                                                                            \
                acc = acc - ((c0 + c1) + (c2 + c3));                                         \
            }
#define AROWS(DT) {                                                                         \
            const float64 Bd = BCAST_DG(Bt, DT);                                             \
            _Pragma("unroll 2")                                                              \
            for (int et = 0; et < 16; et++) {                                                \
                const int t = (DT) * 16 + et;                                                \
                const float lo = (float)t - 0.5f, hi = (float)t + 0.5f;                      \
                const float64 bt = v_f32_shuffle_b(Bd, IDX[et], 0, Bd);                      \
                int5 rc = {0, t, h, c, 0};                                                   \
                AV[t] = v_f32_sel_less_f32_b(lanef, lo, bt * v_f32_ld_tnsr_b(rc, KK), 0.0f); \
                v_f32_st_tnsr(rc, P, v_f32_sel_less_f32_b(lanef, hi, v_f32_ld_tnsr_b(rc, QK), 0.0f)); \
            } }
            AROWS(0) AROWS(1) AROWS(2) AROWS(3)
#undef AROWS
#define ROWS(DT) {                                                                          \
            for (int et = 0; et < 16; et++) {                                                \
                const int t = (DT) * 16 + et;                                                \
                const float lo = (float)t - 0.5f, hi = (float)t + 0.5f;                      \
                const float64 arow = AV[t];                                                  \
                float64 acc = v_f32_sel_less_f32_b(lanef, lo, 0.0f, v_f32_sel_less_f32_b(lanef, hi, 1.0f, 0.0f)); \
                COLS(0) COLS(1) COLS(2) COLS(3)                                              \
                TV[t] = acc;                                                                 \
                int5 rc = {0, t, h, c, 0};                                                   \
                v_f32_st_tnsr(rc, Tm, acc);                                                  \
            } }
            ROWS(0) ROWS(1) ROWS(2) ROWS(3)
#undef ROWS
#undef COLS
        }
}
