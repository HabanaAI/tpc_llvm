// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Split-K flash decode, pass 2: merge the per-chunk P.V results of attn_dec_p + batch GEMM and
// the current token's own key/value into the normalized attention output of row r
// (group j = r / hpg, head h = r % hpg):
//   M = max(max_c m[c,r], sn[r]*scale),  w_c = exp(m[c,r] - M),  w_n = exp(sn[r]*scale - M)
//   o_j[:, h] = (sum_c w_c O_j[:, h, c] + w_n vn_j) / (sum_c w_c l[c,r] + w_n)
// tensors: O_0..O_{NG-1} (256, hpg, S) f32, vn_0..vn_{NG-1} (256, 1) f32, stm, stl (64, S),
//          sn (1, R) f32 | o_0..o_{NG-1} (256, hpg) f32
// params: scale, hpg. index space: [R]
#include "common.h"

#ifndef NG
#define NG 2
#endif

// lane r of v broadcast to all lanes
#define GET_LANE(v, out) \
    { float64 t_ = v_f32_sel_less_f32_b(lanef, rlo, 0.0f, v_f32_sel_less_f32_b(lanef, rhi, v, 0.0f)); \
      NL_REDUCE_ADD(t_, IDX, out) }

// stats columns are loaded as vectors (lane = row)
#define ACC(Ot, vt, ot) {                                                                   \
    float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f, lv = 0.0f;                            \
    int5 oc = {0, h, 0, 0, 0}, sc = {0, 0, 0, 0, 0};                                         \
    for (int ci = 0; ci < S; ci++) {                                                         \
        sc[1] = ci;                                                                          \
        const float64 wv = nl_exp(v_f32_ld_tnsr_b(sc, stm) - M);                             \
        lv = v_f32_mac_b(wv, v_f32_ld_tnsr_b(sc, stl), lv);                                  \
        float64 w;                                                                           \
        GET_LANE(wv, w)                                                                      \
        oc[2] = ci;                                                                          \
        oc[0] = 0;   a0 = v_f32_mac_b(w, v_f32_ld_tnsr_b(oc, Ot), a0);                        \
        oc[0] = 64;  a1 = v_f32_mac_b(w, v_f32_ld_tnsr_b(oc, Ot), a1);                        \
        oc[0] = 128; a2 = v_f32_mac_b(w, v_f32_ld_tnsr_b(oc, Ot), a2);                        \
        oc[0] = 192; a3 = v_f32_mac_b(w, v_f32_ld_tnsr_b(oc, Ot), a3);                        \
    }                                                                                        \
    float64 lsum;                                                                            \
    GET_LANE(lv, lsum)                                                                       \
    const float64 wn = nl_exp(sns - M);                                                      \
    const float64 inv = nl_recip(lsum + wn);                                                 \
    int5 vc = {0, 0, 0, 0, 0}, dc = {0, h, 0, 0, 0};                                         \
    vc[0] = 0;   dc[0] = 0;   v_f32_st_tnsr(dc, ot, v_f32_mac_b(wn, v_f32_ld_tnsr_b(vc, vt), a0) * inv); \
    vc[0] = 64;  dc[0] = 64;  v_f32_st_tnsr(dc, ot, v_f32_mac_b(wn, v_f32_ld_tnsr_b(vc, vt), a1) * inv); \
    vc[0] = 128; dc[0] = 128; v_f32_st_tnsr(dc, ot, v_f32_mac_b(wn, v_f32_ld_tnsr_b(vc, vt), a2) * inv); \
    vc[0] = 192; dc[0] = 192; v_f32_st_tnsr(dc, ot, v_f32_mac_b(wn, v_f32_ld_tnsr_b(vc, vt), a3) * inv); }

#if NG == 1
void main(tensor O0, tensor vn0, tensor stm, tensor stl, tensor sn, tensor o0, float scale, int hpg)
#elif NG == 2
void main(tensor O0, tensor O1, tensor vn0, tensor vn1, tensor stm, tensor stl, tensor sn,
          tensor o0, tensor o1, float scale, int hpg)
#else
void main(tensor O0, tensor O1, tensor O2, tensor O3, tensor vn0, tensor vn1, tensor vn2, tensor vn3,
          tensor stm, tensor stl, tensor sn, tensor o0, tensor o1, tensor o2, tensor o3, float scale, int hpg)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int S = get_dim_size(stm, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    // column-wise max over chunks (lane = row), once for all rows of this member
    float64 Mv = -3.0e38f;
    {
        int5 sc = {0, 0, 0, 0, 0};
        for (int ci = 0; ci < S; ci++) {
            sc[1] = ci;
            Mv = v_f32_max_b(Mv, v_f32_ld_tnsr_b(sc, stm));
        }
    }
    // group / head of the first row
    int j = 0, h = off[0];
    while (h >= hpg) { h -= hpg; j++; }
    for (int r = off[0]; r < end[0]; r++) {
        const float rlo = (float)r - 0.5f, rhi = (float)r + 0.5f;
        int5 nc = {0, r, 0, 0, 0};
        const float64 sns = s_f32_ld_g(gen_addr(nc, sn)) * scale;
        float64 M;
        GET_LANE(Mv, M)
        M = v_f32_max_b(M, sns);
#if NG == 1
        ACC(O0, vn0, o0)
#elif NG == 2
        if (j == 0) ACC(O0, vn0, o0) else ACC(O1, vn1, o1)
#else
        switch (j) {
        case 0: ACC(O0, vn0, o0) break;
        case 1: ACC(O1, vn1, o1) break;
        case 2: ACC(O2, vn2, o2) break;
        default: ACC(O3, vn3, o3)
        }
#endif
        if (++h == hpg) { h = 0; j++; }
    }
}
