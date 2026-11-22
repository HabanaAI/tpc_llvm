// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sampling, pass 5 of 5 (draw): one member per row. A sampled row draws the token where the
// cumulative kept mass, in vocabulary order, first exceeds uniform * total (searchsorted
// right): the chunk from samp_mass's chunk masses, then the vector and lane inside it. Rounding
// that leaves the target past every kept token picks the last kept token. A greedy row takes
// samp_select's greedy token. Both report the token's raw logprob x - lse.
// Weights are samp_scan's per-token weights rescaled to the row maximum.
// tensors: 0 logits (V, B) f32  1 wkey (Vp, B) i32  2 wt (Vp, B) f32  3 stats (see samp_scan)
//          4 fin_i (see samp_full)  5 sel_f (see samp_select)  6 cmass (1, NC, B) f32
//        | 7 tok (1, B) i32  8 lp (1, B) f32
// index space: [B]
#include "samp.h"

void main(tensor logits, tensor wkey, tensor wt, tensor stats, tensor fin_i, tensor sel_f,
          tensor cmass, tensor tok, tensor lp)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NC = get_dim_size(cmass, 1);
    const int nvec = get_dim_size(wkey, 0) / 64;
    const int per = (nvec + NC - 1) / NC;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++) {
        int5 pc = {0, b, 0, 0, 0};
        const int theta = s_i32_ld_g(gen_addr(pc, fin_i));
        const float m = s_f32_ld_g(gen_addr(pc, sel_f));
        pc[0] = 1; const int sample = s_i32_ld_g(gen_addr(pc, fin_i));
        const float lse = s_f32_ld_g(gen_addr(pc, sel_f));
        pc[0] = 2; const int greedy = s_i32_ld_g(gen_addr(pc, fin_i));
        pc[0] = 3; const float invT = s_f32_ld_g(gen_addr(pc, sel_f));
        pc[0] = 4; const float uniform = s_f32_ld_g(gen_addr(pc, sel_f));
        int5 oc = {0, b, 0, 0, 0};
        if (!sample) {
            int5 gc = {greedy, b, 0, 0, 0};
            const float xg = s_f32_ld_g(gen_addr(gc, logits));
            v_i32_st_tnsr(oc, tok, (int64)greedy);
            v_f32_st_tnsr(oc, lp, (float64)(xg - lse));
            continue;
        }
        // the chunk: the first whose cumulative mass exceeds the target
        float total = 0.0f;
        for (int c = 0; c < NC; c++) {
            int5 cc = {0, c, b, 0, 0};
            total += s_f32_ld_g(gen_addr(cc, cmass));
        }
        const float target = uniform * total;
        int chosen = -1, last = 0;
        float before = 0.0f, acc = 0.0f, last_before = 0.0f, last_mass = 0.0f;
        for (int c = 0; c < NC; c++) {
            int5 cc = {0, c, b, 0, 0};
            const float mc = s_f32_ld_g(gen_addr(cc, cmass));
            if (mc > 0.0f) { last = c; last_before = acc; last_mass = mc; }
            if (chosen < 0 && target < acc + mc) { chosen = c; before = acc; }
            acc += mc;
        }
        float rest = target - before;
        if (chosen < 0) { chosen = last; rest = last_mass; }   // rounding: the last kept token
        // the vector: the first whose cumulative kept mass in the chunk exceeds rest
        const int v0 = chosen * per, v1 = v0 + per < nvec ? v0 + per : nvec;
        float64 run = 0.0f, wsel = 0.0f, xsel = 0.0f, vsel = -1.0f, rsel = 0.0f;
        float64 wlast = 0.0f, xlast = 0.0f, vlast = (float)v0, rlast = 0.0f;
        const float64 restv = rest;
        int5 sc = {0, chosen, b, 0, 0};
        const float mc = s_f32_ld_g(gen_addr(sc, stats));
        const float64 scale = weight((float64)((mc - m) * invT));
        int5 xc = {0, b, 0, 0, 0};
        for (int v = v0; v < v1; v++) {
            xc[0] = v * 64;
            const float64 x = v_f32_ld_tnsr_b(xc, logits);
            const float64 w = v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), (int64)theta,
                                                  v_f32_ld_tnsr_b(xc, wt), 0.0f) * scale;
            float64 vs;
            NL_REDUCE_ADD(w, IDX, vs)
            const float64 hit = v_f32_sel_less_f32_b(vsel, 0.0f,
                v_f32_sel_grt_f32_b(run + vs, restv, 1.0f, 0.0f), 0.0f);
            wsel = v_f32_sel_grt_f32_b(hit, 0.5f, w, wsel);
            xsel = v_f32_sel_grt_f32_b(hit, 0.5f, x, xsel);
            rsel = v_f32_sel_grt_f32_b(hit, 0.5f, restv - run, rsel);
            vsel = v_f32_sel_grt_f32_b(hit, 0.5f, (float64)(float)v, vsel);
            const float64 any = v_f32_sel_grt_f32_b(vs, 0.0f, 1.0f, 0.0f);
            wlast = v_f32_sel_grt_f32_b(any, 0.5f, w, wlast);
            xlast = v_f32_sel_grt_f32_b(any, 0.5f, x, xlast);
            rlast = v_f32_sel_grt_f32_b(any, 0.5f, vs, rlast);
            vlast = v_f32_sel_grt_f32_b(any, 0.5f, (float64)(float)v, vlast);
            run += vs;
        }
        const float64 miss = v_f32_sel_less_f32_b(vsel, 0.0f, 1.0f, 0.0f);
        wsel = v_f32_sel_grt_f32_b(miss, 0.5f, wlast, wsel);
        xsel = v_f32_sel_grt_f32_b(miss, 0.5f, xlast, xsel);
        rsel = v_f32_sel_grt_f32_b(miss, 0.5f, rlast, rsel);
        vsel = v_f32_sel_grt_f32_b(miss, 0.5f, vlast, vsel);
        // the lane: the first whose inclusive lane sum exceeds rsel (the last kept lane when
        // rounding leaves rsel at the vector's sum)
        float64 lo = 0.0f, hi = 63.0f;
        for (int it = 0; it < 6; it++) {
            const float64 mid = v_convert_i32_to_f32_b(
                v_convert_f32_to_i32_b((lo + hi) * 0.5f, SW_RZ, 0, 1, 0), SW_RHNE, 0, 1, 0);
            float64 s;
            NL_REDUCE_ADD(v_f32_sel_less_f32_b(mid, lanef, 0.0f, wsel), IDX, s)
            const float64 active = v_f32_sel_less_f32_b(lo, hi, 1.0f, 0.0f);
            const float64 up = v_f32_sel_grt_f32_b(s, rsel, active, 0.0f);   // crossing <= mid
            const float64 down = active - up;                                  // crossing > mid
            hi = v_f32_sel_grt_f32_b(up, 0.5f, mid, hi);
            lo = v_f32_sel_grt_f32_b(down, 0.5f, mid + 1.0f, lo);
        }
        // a lane past the last kept one (rounding): step back to the last positive weight
        float64 lastk;
        NL_REDUCE_MAX(v_f32_sel_grt_f32_b(wsel, 0.0f, lanef, -1.0f), IDX, lastk)
        float64 lane = v_f32_sel_grt_f32_b(lo, lastk, lastk, lo);
        float64 xv;
        NL_REDUCE_ADD(v_f32_sel_eq_f32_b(lanef, lane, xsel, 0.0f), IDX, xv)
        const float64 t = vsel * 64.0f + lane;
        v_i32_st_tnsr(oc, tok, v_convert_f32_to_i32_b(t, SW_RHNE, 0, 1, 0));
        v_f32_st_tnsr(oc, lp, xv - lse);
    }
}
