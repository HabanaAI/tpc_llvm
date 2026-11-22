// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked gated delta rule, stage 2: per (head h,
// chunk c), from KK[t][s] = k_t.k_s and QK[t][s] = q_t.k_s (row-major, lanes = s):
//   A[t][s] = beta_t exp(G_t - G_s) KK[t][s]  (s < t),   P[t][s] = exp(G_t - G_s) QK[t][s]  (s <= t)
//   T = (I + A)^-1 by forward substitution:  T_t = e_t - sum_{s<t} A[t][s] T_s
// and the gate scalings folded into T (gamma = exp(G), kt_t = exp(G_63 - G_t)):
//   Tb = T diag(beta)  Tbg = T diag(beta gamma)  Tkb = diag(kt) Tb  Tkbg = diag(kt) Tbg
// tensors: 0 KK (64,64,nk,NC) f32 (per q/k head: value head h reads h % nk)  1 QK  2 GB (64,2,nv,NC)
//          [G | beta] | 3 P  4 Tb  5 Tbg  6 Tkb  7 Tkbg (64,64,nv,NC)
// index space: [nv, NC]
#include "common.h"

#define CH 64

__local__ float64 TV[CH];          // rows of T
__local__ float64 AV[CH];          // rows of A


void main(tensor KK, tensor QK, tensor GB, tensor P, tensor Tb, tensor Tbg, tensor Tkb, tensor Tkbg)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    const int nk = get_dim_size(KK, 2);
    for (int h = off[0]; h < end[0]; h++) {
        int kh = h;                                    // q/k head
        while (kh >= nk) kh -= nk;
        for (int c = off[1]; c < end[1]; c++) {
            int5 gc = {0, 0, h, c, 0};
            const float64 G = v_f32_ld_tnsr_b(gc, GB);
            gc[1] = 1;
            const float64 Bt = v_f32_ld_tnsr_b(gc, GB);
            const float64 BG = Bt * nl_exp(G);
            const float64 G63 = v_f32_shuffle_b(BCAST_DG(G, 3), IDX[15], 0, BCAST_DG(G, 3));
            const float64 KT = nl_exp(G63 - G);
            #pragma unroll 8
            for (int s2 = 0; s2 < CH; s2++) TV[s2] = 0.0f;
// acc -= sum over s in dual group DS of A[t][s] T_s, for the blocks up to the diagonal. A[t][s] = 0 for
// s >= t and the rows of T not computed yet are 0 (TV cleared per chunk), so the diagonal block
// needs no special case.
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
// phase 1: A rows, P rows out
#define AROWS(DT) {                                                                         \
            const float64 Gd = BCAST_DG(G, DT), Bd = BCAST_DG(Bt, DT);                       \
            _Pragma("unroll 2")                                                              \
            for (int et = 0; et < 16; et++) {                                                \
                const int t = (DT) * 16 + et;                                                \
                const float lo = (float)t - 0.5f, hi = (float)t + 0.5f;                      \
                const float64 Gt = v_f32_shuffle_b(Gd, IDX[et], 0, Gd);                      \
                const float64 bt = v_f32_shuffle_b(Bd, IDX[et], 0, Bd);                      \
                const float64 dec = nl_exp(Gt - G);           /* clamped for s > t, masked */ \
                int5 rc = {0, t, h, c, 0}, kc = {0, t, kh, c, 0};                            \
                AV[t] = v_f32_sel_less_f32_b(lanef, lo, bt * dec * v_f32_ld_tnsr_b(kc, KK), 0.0f); \
                v_f32_st_tnsr(rc, P, v_f32_sel_less_f32_b(lanef, hi, dec * v_f32_ld_tnsr_b(kc, QK), 0.0f)); \
            } }
            AROWS(0) AROWS(1) AROWS(2) AROWS(3)
#undef AROWS
// phase 2: forward substitution + scaled outputs
#define ROWS(DT) {                                                                          \
            const float64 Kd = BCAST_DG(KT, DT);                                             \
            for (int et = 0; et < 16; et++) {                                                \
                const int t = (DT) * 16 + et;                                                \
                const float lo = (float)t - 0.5f, hi = (float)t + 0.5f;                      \
                const float64 ktt = v_f32_shuffle_b(Kd, IDX[et], 0, Kd);                     \
                const float64 arow = AV[t];                                                  \
                float64 acc = v_f32_sel_less_f32_b(lanef, lo, 0.0f, v_f32_sel_less_f32_b(lanef, hi, 1.0f, 0.0f)); \
                COLS(0) COLS(1) COLS(2) COLS(3)                                              \
                TV[t] = acc;                                                                 \
                int5 rc = {0, t, h, c, 0};                                                   \
                const float64 rb = acc * Bt, rbg = acc * BG;                                 \
                v_f32_st_tnsr(rc, Tb, rb);                                                   \
                v_f32_st_tnsr(rc, Tbg, rbg);                                                 \
                v_f32_st_tnsr(rc, Tkb, rb * ktt);                                            \
                v_f32_st_tnsr(rc, Tkbg, rbg * ktt);                                          \
            } }
            ROWS(0) ROWS(1) ROWS(2) ROWS(3)
#undef ROWS
#undef COLS
        }
    }
}
