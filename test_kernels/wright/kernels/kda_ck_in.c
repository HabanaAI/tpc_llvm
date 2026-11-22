// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked KDA (GLM-5.3-Flash linear attention), stage 1, per (head h, chunk c of 64 tokens): the short
// conv + SiLU of q / k / v, l2-norm of q (x dk^-0.5) and k, the forget gate g = lb sigmoid(exp(A_log)
// (f + dt_bias)) (per channel, <= 0), beta = sigmoid(b), and the chunk's decay-scaled operands
// (G = inclusive cumsum of g inside the chunk; r(t) = 16 floor(t / 16) the sub-chunk start):
//   Qr_t = q_t e^(G_t - G_r(t))   Kr_t = k_t e^(G_t - G_r(t))       (rows of the Gram products)
//   Kt[s]_j = k_j e^(min(G_16s - G_j, 80)) for j < 16 s + 16, else 0  (columns, sub-chunk s's reference)
//   qG_t = q_t e^G_t   kbg_t = beta_t k_t e^G_t   vb_t = beta_t v_t   kL_t = k_t e^(G_63 - G_t)
//   cs[0:128] = e^G_63 (state decay over the chunk), cs[128:192] = beta
// Padding tokens (t >= n_real) get g = 0, beta = 0, k = 0: the state passes them unchanged.
// The member of the chunk holding token n_real - 1 writes the conv state (its last 3 raw inputs).
// Semantics: tools/glm_np.py kda_blocks.
// tensors: 0 qkv (3C, T) f32 [q | k | v] (C = nh 128)  1 fg (C, T) f32 f_b(f_a(h))  2 small (>= 256 + nh, T)
//          f32, row 256 + h = b logit of head h  3 conv_w (3C, 4) f32 (tap 3 = current token)
//          4 cst_in (3C, 3) f32 (inputs at positions -3, -2, -1)  5 par (C + nh) f32 [dt_bias | A_log]
//          6 meta i32 [pos0, n_real]
//        | 7 Qr  8 Kr  9 qG  10 kbg  11 kL  12 vb (128, 64, nh, NC) f32  13 Kt (128, 64, 4, nh, NC) f32
//          14 cs (192, nh, NC) f32  15 cst_out (3C, 3) f32
// params: lb. index space: [nh, NC]
#include "common.h"

#define D 128
#define CH 64

__local__ float64 KV_[2 * CH];     // l2-normed k per token (two 64-channel halves)
__local__ float64 GV_[2 * CH];     // cumulative G per token
__local__ float64 WV_[24];         // conv taps: [kind (q, k, v)][half][tap]

void main(tensor qkv, tensor fg, tensor small, tensor conv_w, tensor cst_in, tensor par, tensor meta,
          tensor Qr, tensor Kr, tensor qG, tensor kbg, tensor kL, tensor vb, tensor Kt, tensor cs, tensor cst_out,
          float lb)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NH = get_dim_size(Qr, 2), C = NH * D;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    const float qs = 0.08838834764831845f;              // 128^-0.5
    for (int h = off[0]; h < end[0]; h++) {
        int5 ac = {C + h, 0, 0, 0, 0};
        const float64 al = BCAST_DG(v_f32_ld_tnsr_b(ac, par), 0);
        const float64 aexp = nl_exp(v_f32_shuffle_b(al, IDX[0], 0, al));
        int5 dc = {h * D, 0, 0, 0, 0};
        const float64 dta = v_f32_ld_tnsr_b(dc, par);
        dc[0] += 64;
        const float64 dtb = v_f32_ld_tnsr_b(dc, par);
        #pragma unroll
        for (int kind = 0; kind < 3; kind++)
            #pragma unroll
            for (int hf = 0; hf < 2; hf++)
                #pragma unroll
                for (int tap = 0; tap < 4; tap++) {
                    int5 wc = {kind * C + h * D + 64 * hf, tap, 0, 0, 0};
                    WV_[kind * 8 + hf * 4 + tap] = v_f32_ld_tnsr_b(wc, conv_w);
                }
        for (int c = off[1]; c < end[1]; c++) {
            float64 Ga = 0.0f, Gb = 0.0f, Gra = 0.0f, Grb = 0.0f, bvec = 0.0f;
            for (int t = 0; t < CH; t++) {
                const int p = c * CH + t;
                const float real = p < n_real ? 1.0f : 0.0f;
                float64 x[6];                              // conv outputs: q a/b, k a/b, v a/b
                #pragma unroll
                for (int kind = 0; kind < 3; kind++)
                    #pragma unroll
                    for (int hf = 0; hf < 2; hf++) {
                        const int row = kind * C + h * D + 64 * hf;
                        float64 acc = 0.0f;
                        #pragma unroll
                        for (int tap = 0; tap < 4; tap++) {
                            const int pp = p - 3 + tap;
                            float64 raw;
                            if (pp >= 0) { int5 rc = {row, pp, 0, 0, 0}; raw = v_f32_ld_tnsr_b(rc, qkv); }
                            else { int5 rc = {row, 3 + pp, 0, 0, 0}; raw = v_f32_ld_tnsr_b(rc, cst_in); }
                            acc = v_f32_mac_b(raw, WV_[kind * 8 + hf * 4 + tap], acc);
                        }
                        x[kind * 2 + hf] = nl_silus(acc);
                    }
                float64 sq, sk;
                NL_REDUCE_ADD(v_f32_mac_b(x[0], x[0], x[1] * x[1]), IDX, sq);
                NL_REDUCE_ADD(v_f32_mac_b(x[2], x[2], x[3] * x[3]), IDX, sk);
                const float64 nq = nl_rsqrt(sq + 1e-6f) * qs, nk = nl_rsqrt(sk + 1e-6f) * real;
                const float64 qa = x[0] * nq, qb = x[1] * nq, ka = x[2] * nk, kb = x[3] * nk;
                // gate and beta
                int5 fc = {h * D, p, 0, 0, 0};
                const float64 fa = v_f32_ld_tnsr_b(fc, fg);
                fc[0] += 64;
                const float64 fb = v_f32_ld_tnsr_b(fc, fg);
                const float64 ga = lb * nl_sigm(aexp * (fa + dta)) * real;
                const float64 gb = lb * nl_sigm(aexp * (fb + dtb)) * real;
                int5 bc = {256 + h, p, 0, 0, 0};
                const float64 bl = BCAST_DG(v_f32_ld_tnsr_b(bc, small), 0);
                const float64 beta = nl_sigm(v_f32_shuffle_b(bl, IDX[0], 0, bl)) * real;
                bvec = v_f32_sel_eq_f32_b(lanef, (float)t, beta, bvec);
                Ga += ga; Gb += gb;
                if ((t & 15) == 0) { Gra = Ga; Grb = Gb; }
                KV_[2 * t] = ka; KV_[2 * t + 1] = kb;
                GV_[2 * t] = Ga; GV_[2 * t + 1] = Gb;
                const float64 era = nl_exp(Ga - Gra), erb = nl_exp(Gb - Grb);
                const float64 ea = nl_exp(Ga), eb = nl_exp(Gb);
                int5 oc = {0, t, h, c, 0};
#define ST2(TN, A, B) { oc[0] = 0; v_f32_st_tnsr(oc, TN, A); oc[0] = 64; v_f32_st_tnsr(oc, TN, B); }
                ST2(Qr, qa * era, qb * erb)
                ST2(Kr, ka * era, kb * erb)
                ST2(qG, qa * ea, qb * eb)
                ST2(kbg, ka * ea * beta, kb * eb * beta)
                ST2(vb, x[4] * beta, x[5] * beta)
            }
            // chunk decay, beta
            int5 cc = {0, h, c, 0, 0};
            v_f32_st_tnsr(cc, cs, nl_exp(Ga));
            cc[0] = 64; v_f32_st_tnsr(cc, cs, nl_exp(Gb));
            cc[0] = 128; v_f32_st_tnsr(cc, cs, bvec);
            // kL and the sub-chunk-referenced key columns
            for (int j = 0; j < CH; j++) {
                const float64 ka = KV_[2 * j], kb = KV_[2 * j + 1], gja = GV_[2 * j], gjb = GV_[2 * j + 1];
                int5 oc = {0, j, h, c, 0};
                ST2(kL, ka * nl_exp(Ga - gja), kb * nl_exp(Gb - gjb))
                #pragma unroll
                for (int s = 0; s < 4; s++) {
                    float64 va = 0.0f, vbb = 0.0f;
                    if (j < 16 * s + 16) {
                        va = ka * nl_exp(v_f32_min_b(GV_[32 * s] - gja, 80.0f));
                        vbb = kb * nl_exp(v_f32_min_b(GV_[32 * s + 1] - gjb, 80.0f));
                    }
                    int5 kc = {0, j, s, h, c};
                    v_f32_st_tnsr(kc, Kt, va);
                    kc[0] = 64; v_f32_st_tnsr(kc, Kt, vbb);
                }
            }
            // conv state: the last 3 real inputs of this head's q / k / v channels
            if (c * CH <= n_real - 1 && n_real - 1 < c * CH + CH)
                for (int kind = 0; kind < 3; kind++)
                    for (int hf = 0; hf < 2; hf++) {
                        const int row = kind * C + h * D + 64 * hf;
                        for (int j = 0; j < 3; j++) {
                            const int pp = n_real - 3 + j;
                            float64 raw;
                            if (pp >= 0) { int5 rc = {row, pp, 0, 0, 0}; raw = v_f32_ld_tnsr_b(rc, qkv); }
                            else { int5 rc = {row, 3 + pp, 0, 0, 0}; raw = v_f32_ld_tnsr_b(rc, cst_in); }
                            int5 sc = {row, j, 0, 0, 0};
                            v_f32_st_tnsr(sc, cst_out, raw);
                        }
                    }
        }
    }
}
