// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot attention, accumulate (Mistral-Large-4; attn_slot_acc for any head width D): merge
// one launch's slot results into the running attention output of each query column. Per column c of group g the accumulator holds
// the normalized output acc_o, the running max acc_m and the running sum acc_l; slot s (covering
// columns qcol0 .. qcol0 + M - 1 of its group) contributes its unnormalized P.V result O, max mx
// and sum l:
//   n = max(acc_m, mx),  wa = exp(acc_m - n) acc_l,  ws = exp(mx - n),
//   acc_o = (wa acc_o + ws O) / (wa + ws l),  acc_m = n,  acc_l = wa + ws l
// A member owns a 64-column block of one group.
// tensors: 0 O (>= D, M, S) f32 (rows past D ignored)  1 mx (Mp, S) f32  2 l (Mp, S) f32
//          3 slots (8, S) i32  4 acc_o (D, W, G) f32  5 acc_m (W, G) f32  6 acc_l (W, G) f32
//        | 7 sink (64, 1) i32
// index space: [ceil(W / 64), G]; member (0, 0) writes the sink.
#include "common.h"

// lane k of v broadcast to all lanes
#define GET_LANE(v, k, out)                                                                  \
    { const float lo_ = (float)(k) - 0.5f, hi_ = (float)(k) + 0.5f;                          \
      float64 t_ = v_f32_sel_less_f32_b(lanef, lo_, 0.0f, v_f32_sel_less_f32_b(lanef, hi_, v, 0.0f)); \
      NL_REDUCE_ADD(t_, IDX, out) }

void main(tensor O, tensor mx, tensor l, tensor slots, tensor acc_o, tensor acc_m, tensor acc_l,
          tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(O, 1), S = get_dim_size(O, 2), W = get_dim_size(acc_o, 1);
    const int D = get_dim_size(acc_o, 0);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int g = off[1]; g < end[1]; g++) {
        for (int cb = off[0]; cb < end[0]; cb++) {
            const int c0 = cb * 64, c1 = c0 + 64 < W ? c0 + 64 : W;
            int5 ac = {c0, g, 0, 0, 0};
            float64 am = v_f32_ld_tnsr_b(ac, acc_m), al = v_f32_ld_tnsr_b(ac, acc_l);
            for (int s = 0; s < S; s++) {
                int5 fc = {0, s, 0, 0, 0};
                if (s_i32_ld_g(gen_addr(fc, slots)) != g)
                    continue;
                fc[0] = 1;
                const int q0 = s_i32_ld_g(gen_addr(fc, slots));
                const int lo = c0 > q0 ? c0 : q0, hi = c1 < q0 + M ? c1 : q0 + M;
                for (int c = lo; c < hi; c++) {
                    const int m = c - q0;
                    int5 sc = {m & ~63, s, 0, 0, 0};
                    float64 ms, ls, ma, la;
                    GET_LANE(v_f32_ld_tnsr_b(sc, mx), m & 63, ms)
                    GET_LANE(v_f32_ld_tnsr_b(sc, l), m & 63, ls)
                    GET_LANE(am, c - c0, ma)
                    GET_LANE(al, c - c0, la)
                    const float64 n = v_f32_max_b(ma, ms);
                    const float64 wa = nl_exp(ma - n) * la, ws = nl_exp(ms - n);
                    const float64 den = v_f32_mac_b(ws, ls, wa);
                    const float64 inv = nl_recip(v_f32_max_b(den, 1.0e-30f));
                    int5 oc = {0, c, g, 0, 0}, pc = {0, m, s, 0, 0};
                    for (int d = 0; d < D; d += 64) {
                        oc[0] = d;
                        pc[0] = d;
                        const float64 acc = v_f32_ld_tnsr_b(oc, acc_o) * wa;
                        v_f32_st_tnsr(oc, acc_o, v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), ws, acc) * inv);
                    }
                    const float lo_ = (float)(c - c0) - 0.5f, hi_ = (float)(c - c0) + 0.5f;
                    am = v_f32_sel_less_f32_b(lanef, lo_, am, v_f32_sel_less_f32_b(lanef, hi_, n, am));
                    al = v_f32_sel_less_f32_b(lanef, lo_, al, v_f32_sel_less_f32_b(lanef, hi_, den, al));
                }
            }
            v_f32_st_tnsr(ac, acc_m, am);
            v_f32_st_tnsr(ac, acc_l, al);
        }
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
