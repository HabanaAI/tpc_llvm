// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot attention, accumulate, for a prefill chunk (Mistral-Large-4; mla_slot_acc's merge for one
// slot per launch holding every column of the query block, so columns map one to one):
//   n = max(acc_m, mx),  wa = exp(acc_m - n) acc_l,  ws = exp(mx - n),  den = wa + ws l,
//   acc_o = (ws O + wa acc_o) / den,  acc_m = n,  acc_l = den
// tensors: 0 O (>= D, W, 1) f32 (rows past D ignored)  1 mx (Wp, 1) f32  2 l (Wp, 1) f32
//          3 slots (8, 1) i32  4 acc_o (D, W, 1) f32  5 acc_m (W, 1) f32  6 acc_l (W, 1) f32
//        | 7 sink (64, 1) i32
// index space: [ceil(W / 64), 1]; member (0, 0) writes the sink.
#include "common.h"

// the columns of dual group dg (16 lanes) of a block: ka and ks broadcast in the group first, then
// element e of it per column
#define MERGE_GROUP(dg)                                                                          {                                                                                                const float64 ga = BCAST_DG(ka, dg), gs = BCAST_DG(ks, dg);                                  const int g0 = c0 + 16 * (dg), g1 = g0 + 16 < c1 ? g0 + 16 : c1;                             for (int c = g0; c < g1; c++) {                                                                  const int e = c - g0;                                                                        const uchar256 ctl = (unsigned char)(0x80 | ((e >> 3) << 5) | (e & 7));                      const float64 ba = v_f32_shuffle_b(ga, ctl, 0, ga);                                          const float64 bs = v_f32_shuffle_b(gs, ctl, 0, gs);                                          merge_column(O, acc_o, c, D, ba, bs);                                                    }                                                                                        }

static inline void merge_column(tensor O, tensor acc_o, int c, int D, float64 ba, float64 bs)
{
    int5 oc = {0, c, 0, 0, 0}, pc = {0, c, 0, 0, 0};
    for (int d = 0; d < D; d += 256) {
        oc[0] = d;        pc[0] = d;
        const float64 a0 = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), bs, v_f32_ld_tnsr_b(oc, acc_o) * ba);
        v_f32_st_tnsr(oc, acc_o, a0);
        oc[0] = d + 64;   pc[0] = d + 64;
        const float64 a1 = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), bs, v_f32_ld_tnsr_b(oc, acc_o) * ba);
        v_f32_st_tnsr(oc, acc_o, a1);
        oc[0] = d + 128;  pc[0] = d + 128;
        const float64 a2 = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), bs, v_f32_ld_tnsr_b(oc, acc_o) * ba);
        v_f32_st_tnsr(oc, acc_o, a2);
        oc[0] = d + 192;  pc[0] = d + 192;
        const float64 a3 = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), bs, v_f32_ld_tnsr_b(oc, acc_o) * ba);
        v_f32_st_tnsr(oc, acc_o, a3);
    }
}

void main(tensor O, tensor mx, tensor l, tensor slots, tensor acc_o, tensor acc_m, tensor acc_l,
          tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int W = get_dim_size(acc_o, 1), D = get_dim_size(acc_o, 0);
    scalar_cache_reset();
    for (int cb = off[0]; cb < end[0]; cb++) {
        const int c0 = cb * 64, c1 = c0 + 64 < W ? c0 + 64 : W;
        int5 ac = {c0, 0, 0, 0, 0};
        const float64 am = v_f32_ld_tnsr_b(ac, acc_m), al = v_f32_ld_tnsr_b(ac, acc_l);
        const float64 ms = v_f32_ld_tnsr_b(ac, mx), ls = v_f32_ld_tnsr_b(ac, l);
        const float64 n = v_f32_max_b(am, ms);
        const float64 wa = nl_exp(am - n) * al, ws = nl_exp(ms - n);
        const float64 den = v_f32_mac_b(ws, ls, wa);
        const float64 inv = nl_recip(v_f32_max_b(den, 1.0e-30f));
        const float64 ka = wa * inv, ks = ws * inv;
        v_f32_st_tnsr(ac, acc_m, n);
        v_f32_st_tnsr(ac, acc_l, den);
        MERGE_GROUP(0)
        MERGE_GROUP(1)
        MERGE_GROUP(2)
        MERGE_GROUP(3)
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
