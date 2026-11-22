// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot attention softmax partials over a prefill chunk's DSA mask (Mistral-Large-4; mla_slot_pk
// with its selection computed once per column: ix_mask). A key whose mask is 0 is kept. For each column:
//   mx = max_kept s * scale,  P[i] = exp2-based exp(s[i] * scale - mx) (bf16, natural key order),
//   l = sum_i P[i]
// A column with no kept key in this slot gets mx = NINF * scale, P and l near 0.
// exp: 2^(x / ln 2) with a degree 3 polynomial on the fraction (relative error 1e-4), x clamped at -64.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, 0, 0]
// tensors: 0 sc (C, M, S) f32  1 slots  2 mask (K, T) f32 | 3 P (C, M, S) bf16  4 mx (Mp, S) f32
//          5 l (Mp, S) f32
// params: scale, hpg. index space: [ceil(M / 64), S]
#include "common.h"

#define NINF (-3.0e38f)

static inline float64 exp_lo(float64 x)
{
    const float64 t = v_f32_max_b(x, -64.0f) * 1.4426950408889634f;
    const int64 n = v_convert_f32_to_i32_b(t, SW_RD, 0, 1, 0);
    const float64 r = t - v_convert_i32_to_f32_b(n, SW_RHNE, 0, 1, 0);
    float64 pl = 0.0790219f;
    pl = v_f32_mac_b(pl, r, 0.2248797f);
    pl = v_f32_mac_b(pl, r, 0.6960983f);
    pl = v_f32_mac_b(pl, r, 0.9999252f);
    return pl * as_float64(v_i32_shl_b(n + 127, (int64)23));
}

void main(tensor sc, tensor slots, tensor mask, tensor P, tensor mx, tensor l, float scale, int hpg)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int C = get_dim_size(sc, 0), M = get_dim_size(sc, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
        int5 fc = {2, s, 0, 0, 0};
        const int qpos0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 5;
        const int tok0 = s_i32_ld_g(gen_addr(fc, slots));
        for (int cb = off[0]; cb < end[0]; cb++) {
            float64 mv = NINF * scale, lv = 0.0f;
            int t = 0, h = cb * 64;
            while (h >= hpg) { h -= hpg; t++; }
            for (int r = 0; r < 64 && cb * 64 + r < M; r++) {
                const int m = cb * 64 + r;
                int lim = qpos0 + t + 1 < klen ? qpos0 + t + 1 : klen;
                lim -= key0;
                lim = lim < 0 ? 0 : (lim > C ? C : lim);
                int5 c = {0, m, s, 0, 0}, k = {key0, tok0 + t, 0, 0, 0};
                // pass 1: max over the visible keys
                float64 m0 = NINF, m1 = NINF, m2 = NINF, m3 = NINF;
                for (int i = 0; i < lim; i += 256) {
                    c[0] = i;        k[0] = key0 + i;
                    m0 = v_f32_max_b(m0, v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask));
                    c[0] = i + 64;   k[0] = key0 + i + 64;
                    m1 = v_f32_max_b(m1, v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask));
                    c[0] = i + 128;  k[0] = key0 + i + 128;
                    m2 = v_f32_max_b(m2, v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask));
                    c[0] = i + 192;  k[0] = key0 + i + 192;
                    m3 = v_f32_max_b(m3, v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask));
                }
                float64 mm = v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3));
                NL_REDUCE_MAX(mm, IDX, mm);
                const float64 ms = mm * scale;
                const float64 base = v_f32_sel_less_f32_b(mm, 0.5f * NINF, 1.0e30f, ms);
                // pass 2: unnormalized probabilities and their sum
                float64 s0 = 0.0f, s1 = 0.0f, s2 = 0.0f, s3 = 0.0f;
                int5 pc = {0, m, s, 0, 0};
                int i = 0;
                for (; i < lim; i += 256) {
                    c[0] = i;        k[0] = key0 + i;
                    const float64 xa = v_f32_mac_b(v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask), scale, -base);
                    c[0] = i + 64;   k[0] = key0 + i + 64;
                    const float64 xb = v_f32_mac_b(v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask), scale, -base);
                    c[0] = i + 128;  k[0] = key0 + i + 128;
                    const float64 xc = v_f32_mac_b(v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask), scale, -base);
                    c[0] = i + 192;  k[0] = key0 + i + 192;
                    const float64 xd = v_f32_mac_b(v_f32_ld_tnsr_b(c, sc) + v_f32_ld_tnsr_b(k, mask), scale, -base);
                    const float64 ea = exp_lo(xa), eb = exp_lo(xb), ec = exp_lo(xc), ed = exp_lo(xd);
                    s0 = s0 + ea; s1 = s1 + eb; s2 = s2 + ec; s3 = s3 + ed;
                    pc[0] = i;
                    v_bf16_st_tnsr(pc, P, cvt_bf16_lin(ea, eb));
                    pc[0] = i + 128;
                    v_bf16_st_tnsr(pc, P, cvt_bf16_lin(ec, ed));
                }
                for (; i < C; i += 128) {
                    pc[0] = i;
                    v_bf16_st_tnsr(pc, P, (bfloat128)0);
                }
                float64 tot, sum = (s0 + s1) + (s2 + s3);
                NL_REDUCE_ADD(sum, IDX, tot);
                mv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, mv, ms);
                lv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, lv, tot);
                if (++h == hpg) { h = 0; t++; }
            }
            int5 stc = {cb * 64, s, 0, 0, 0};
            v_f32_st_tnsr(stc, mx, mv);
            v_f32_st_tnsr(stc, l, lv);
        }
    }
}
