// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, accumulate a launch's normalized slot results (attn_slot_acc's merge). Slot s
// holds, per head h and token t of its Tq query tokens (chunk tokens qtok0 .. qtok0 + Tq - 1), the
// normalized output O, the row maximum mx and the inverse weight sum linv. Token t of head h is
// accumulator column h + H t of the slot's group:
//   n = max(acc_m, mx),  wa = exp(acc_m - n) acc_l,  ws = exp(mx - n) / linv,
//   acc_o = (wa acc_o + ws O) / (wa + ws),  acc_m = n,  acc_l = wa + ws
// acc_m and acc_l are token-major per head here: element (t + T h, g).
// A member owns 64 query tokens of the slots (tokens 64 b .. 64 b + 63 of each slot's block) of one
// head and group, so every member of a launch has work; padding slots (group -1) are skipped.
// Query blocks start at and span multiples of 64 tokens. A launch's slots of one query block and group are
// consecutive among the group's slots (Rust slot_tables puts the groups innermost).
// The member keeps the block's 64 tokens of acc_o in VLM (AO) while it merges the block's slots: it loads them when
// the block starts and stores them when the block ends. The kernel never loads an acc_o value that it stored.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, 0, 0]
// tensors: 0 O (256, Tq, H, S) f32  1 mx (Tq, H, S) f32  2 linv (Tq, H, S) f32  3 slots (8, S)
//          4 acc_o (256, W, G) f32  5 acc_m (W, G) f32  6 acc_l (W, G) f32 | 7 sink (64, 1) i32
// params: T (chunk tokens, W / H). index space: [ceil(Tq / 64), H, G]; member (0, 0, 0) writes
// the sink.
#include "common.h"

__local__ float64 AO[64 * 4];  // the block's acc_o: token t0 + r, dims 64 d .. 64 d + 63 at AO[4 r + d]

// the block's accumulators back: nt tokens of AO from column c (token stride H), am and al at ac
static inline void store_block(tensor acc_o, tensor acc_m, tensor acc_l, int5 ac, float64 am, float64 al, int c,
                               int H, int g, int nt)
{
    int5 oc = {0, c, g, 0, 0};
    for (int r = 0; r < nt; r++) {
        #pragma unroll
        for (int d = 0; d < 4; d++) {
            oc[0] = 64 * d;
            v_f32_st_tnsr(oc, acc_o, AO[4 * r + d]);
        }
        oc[1] += H;
    }
    v_f32_st_tnsr(ac, acc_m, am);
    v_f32_st_tnsr(ac, acc_l, al);
}

void main(tensor O, tensor mx, tensor linv, tensor slots, tensor acc_o, tensor acc_m,
          tensor acc_l, tensor sink, int T)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int Tq = get_dim_size(O, 1), H = get_dim_size(O, 2), S = get_dim_size(O, 3);
    scalar_cache_reset();
    for (int g = off[2]; g < end[2]; g++) {
        for (int h = off[1]; h < end[1]; h++) {
            for (int tb = off[0]; tb < end[0]; tb++) {
                const int tl = tb * 64;                      // the member's first token in a slot
                int t0 = -1, nt = 0;                         // the chunk token and token count of AO, am, al
                int5 ac = {0, g, 0, 0, 0};
                float64 am = 0.0f, al = 0.0f;
                for (int s = 0; s < S; s++) {
                    int5 fc = {0, s, 0, 0, 0};
                    if (s_i32_ld_g(gen_addr(fc, slots)) != g)
                        continue;
                    fc[0] = 5;
                    const int q = s_i32_ld_g(gen_addr(fc, slots)) + tl;
                    if (q != t0) {                           // another query block: its accumulators
                        if (t0 >= 0)
                            store_block(acc_o, acc_m, acc_l, ac, am, al, h + H * t0, H, g, nt);
                        t0 = q;
                        nt = t0 + 64 < T ? 64 : T - t0;
                        ac[0] = T * h + t0;
                        am = v_f32_ld_tnsr_b(ac, acc_m);
                        al = v_f32_ld_tnsr_b(ac, acc_l);
                        int5 oc = {0, h + H * t0, g, 0, 0};
                        for (int r = 0; r < nt; r++) {
                            #pragma unroll
                            for (int d = 0; d < 4; d++) {
                                oc[0] = 64 * d;
                                AO[4 * r + d] = v_f32_ld_tnsr_b(oc, acc_o);
                            }
                            oc[1] += H;
                        }
                    }
                    int5 sc = {tl, h, s, 0, 0};
                    const float64 ms = v_f32_ld_tnsr_b(sc, mx), li = v_f32_ld_tnsr_b(sc, linv);
                    const float64 n = v_f32_max_b(am, ms);
                    const float64 wa = nl_exp(am - n) * al;
                    const float64 ws = nl_exp(ms - n) * nl_recip(v_f32_max_b(li, 1.0e-30f));
                    const float64 den = wa + ws;
                    const float64 inv = nl_recip(v_f32_max_b(den, 1.0e-30f));
                    const float64 a = wa * inv, b = ws * inv;
                    int5 pc = {0, tl, h, s, 0};
                    for (int r = 0; r < nt; r++) {
                        const float64 ar = lane_bcast(a, r), br = lane_bcast(b, r);
                        #pragma unroll
                        for (int d = 0; d < 4; d++) {
                            pc[0] = 64 * d;
                            AO[4 * r + d] = v_f32_mac_b(v_f32_ld_tnsr_b(pc, O), br, AO[4 * r + d] * ar);
                        }
                        pc[1]++;
                    }
                    am = n;
                    al = den;
                }
                if (t0 >= 0)
                    store_block(acc_o, acc_m, acc_l, ac, am, al, h + H * t0, H, g, nt);
            }
        }
    }
    if (off[0] == 0 && off[1] == 0 && off[2] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
