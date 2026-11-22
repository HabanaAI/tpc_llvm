// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, accumulate (after attn_dec_comb): merge one launch's slot results into the
// running attention output of each query column. Per column c of group g the accumulator holds
// the normalized output acc_o, the running max acc_m and the running sum acc_l; slot s (covering
// columns qcol0 .. qcol0 + M - 1 of its group) contributes its unnormalized P.V result O, max mx
// and sum l:
//   n = max(acc_m, mx),  wa = exp(acc_m - n) acc_l,  ws = exp(mx - n),
//   acc_o = (wa acc_o + ws O) / (wa + ws l),  acc_m = n,  acc_l = wa + ws l
// A slot in which the column sees no key (l = 0: a verify step's or a block's query block reaches into a key
// chunk past the column's position) leaves the column's accumulator exactly as it is.
// A member owns a 64-column block of one group. Per slot it takes n, wa, ws and the reciprocal
// for the whole block at once (lane r: column c0 + r) and then broadcasts each column's lanes.
// The member keeps the block's acc_o in VLM (AO) for the whole launch: it loads the block once, merges every slot
// there and stores the block once. The kernel never loads an acc_o value that it stored.
// tensors: 0 O (256, M, S) f32  1 mx (Mp, S) f32  2 l (Mp, S) f32  3 slots (8, S) i32
//          4 acc_o (256, W, G) f32  5 acc_m (W, G) f32  6 acc_l (W, G) f32 | 7 sink (64, 1) i32
// index space: [ceil(W / 64), G]; member (0, 0) writes the sink.
#include "common.h"

__local__ float64 AO[64 * 4];  // the block's acc_o: column c0 + r, dims 64 d .. 64 d + 63 at AO[4 r + d]

void main(tensor O, tensor mx, tensor l, tensor slots, tensor acc_o, tensor acc_m, tensor acc_l,
          tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(O, 1), S = get_dim_size(O, 2), W = get_dim_size(acc_o, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int g = off[1]; g < end[1]; g++) {
        for (int cb = off[0]; cb < end[0]; cb++) {
            const int c0 = cb * 64, c1 = c0 + 64 < W ? c0 + 64 : W;
            for (int c = c0; c < c1; c++) {
                int5 oc = {0, c, g, 0, 0};
                #pragma unroll
                for (int d = 0; d < 4; d++) {
                    oc[0] = 64 * d;
                    AO[4 * (c - c0) + d] = v_f32_ld_tnsr_b(oc, acc_o);
                }
            }
            int5 ac = {c0, g, 0, 0, 0};
            float64 am = v_f32_ld_tnsr_b(ac, acc_m), al = v_f32_ld_tnsr_b(ac, acc_l);
            for (int s = 0; s < S; s++) {
                int5 fc = {0, s, 0, 0, 0};
                if (s_i32_ld_g(gen_addr(fc, slots)) != g)
                    continue;
                fc[0] = 1;
                const int q0 = s_i32_ld_g(gen_addr(fc, slots));
                const int lo = c0 > q0 ? c0 : q0, hi = c1 < q0 + M ? c1 : q0 + M;
                if (lo >= hi)
                    continue;
                // the slot's statistics in the block's lanes: lane r is slot column c0 + r - q0
                int5 sc = {c0 - q0, s, 0, 0, 0};
                const float64 ms = v_f32_ld_tnsr_b(sc, mx), ls = v_f32_ld_tnsr_b(sc, l);
                // l >= 1 when the column sees a key (its max key's term is 1), 0 when it sees none
                const float64 n = v_f32_sel_less_f32_b(ls, 0.5f, am, v_f32_max_b(am, ms));
                const float64 wa = nl_exp(am - n) * al, ws = nl_exp(ms - n);
                const float64 den = v_f32_sel_less_f32_b(ls, 0.5f, al, v_f32_mac_b(ws, ls, wa));
                const float64 inv = nl_recip(v_f32_max_b(den, 1.0e-30f));
                for (int c = lo; c < hi; c++) {
                    const int r = c - c0;
                    const float64 wr = lane_bcast(wa, r), sr = lane_bcast(ws, r), ir = lane_bcast(inv, r);
                    const float64 lr = lane_bcast(ls, r);
                    int5 pc = {0, c - q0, s, 0, 0};
                    #pragma unroll
                    for (int d = 0; d < 4; d++) {
                        pc[0] = 64 * d;
                        const float64 old = AO[4 * r + d];
                        const float64 merged = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), sr, old * wr) * ir;
                        AO[4 * r + d] = v_f32_sel_less_f32_b(lr, 0.5f, old, merged);
                    }
                }
                // the slot's columns take the new max and sum
                const float lof = (float)(lo - c0) - 0.5f, hif = (float)(hi - c0) - 0.5f;
                am = v_f32_sel_less_f32_b(lanef, lof, am, v_f32_sel_less_f32_b(lanef, hif, n, am));
                al = v_f32_sel_less_f32_b(lanef, lof, al, v_f32_sel_less_f32_b(lanef, hif, den, al));
            }
            for (int c = c0; c < c1; c++) {
                int5 oc = {0, c, g, 0, 0};
                #pragma unroll
                for (int d = 0; d < 4; d++) {
                    oc[0] = 64 * d;
                    v_f32_st_tnsr(oc, acc_o, AO[4 * (c - c0) + d]);
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
