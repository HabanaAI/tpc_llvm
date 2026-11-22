// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DFlash 2 grouped dynamic two-tap convolution over the columns of a drafting block (one side of a
// sublayer: side 0 convolves its input, side 1 its output). Channel c of column t:
//   in(c, t) = x[c, t]  (* xs[t] in the SCALED build: x is an FP8-GEMM output);
//   prev(c, t) = in(c, t - 1), or 0 at the first column of a block (t % block == 0);
//   coef_k(c, t) = base[k, c] + dyn[k GR + c / 16, t] * ds[t],  k = 2 side + tap, GR = H / 16;
//   out(c, t) = coef_0 in + coef_1 prev.
// Q8 build (-DQ8): out is emitted as per-token FP8 (tau order) + scale, the next FP8 GEMM's operand.
// tensors: 0 x (H,T) f32  [SCALED: 1 xs (1,T) f32]  dyn (4 GR, T) f32  ds (1,T) f32  base (H, 4) f32
//        | out (H,T) f32,  or Q8: q (H,T) fp8  qs (1,T) f32
// params: side, block. index space: [T]. H a multiple of 1024 (GR of 64), at most 8192.
#include "common.h"
#ifdef Q8
__local__ float64 OUT[128];
#endif
// lanes 16 i .. 16 i + 15 <- lane l0 + i of d (i = 0 .. 3): the coefficients of 4 consecutive groups
static inline float64 expand4(float64 d, int l0, float64 lanef)
{
    const float64 c0 = lane_bcast(d, l0), c1 = lane_bcast(d, l0 + 1);
    const float64 c2 = lane_bcast(d, l0 + 2), c3 = lane_bcast(d, l0 + 3);
    return v_f32_sel_less_f32_b(lanef, 16.0f, c0,
           v_f32_sel_less_f32_b(lanef, 32.0f, c1, v_f32_sel_less_f32_b(lanef, 48.0f, c2, c3)));
}
#ifdef SCALED
#define XS_ARG tensor xs,
#else
#define XS_ARG
#endif
#ifdef Q8
void main(tensor x, XS_ARG tensor dyn, tensor ds, tensor base, tensor q, tensor qs, int side, int block)
#else
void main(tensor x, XS_ARG tensor dyn, tensor ds, tensor base, tensor out, int side, int block)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const int GR = H / 16, NV = H / 64, ND = GR / 64;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    float64 D0[8], D1[8];  // the two taps' group coefficients of one column (GR <= 512)
    for (int t = off[0]; t < end[0]; t++) {
        const int first = (t % block) == 0;
        const float64 dst = TOKEN_SCALE(ds, t, IDX[0]);
#ifdef SCALED
        const float64 sx = TOKEN_SCALE(xs, t, IDX[0]);
        const float64 sp = first ? (float64)0.0f : TOKEN_SCALE(xs, t - 1, IDX[0]);
#endif
        const int k0 = 2 * side;
        for (int j = 0; j < ND; j++) {
            int5 dc = {k0 * GR + 64 * j, t, 0, 0, 0};
            D0[j] = v_f32_ld_tnsr_b(dc, dyn) * dst;
            dc[0] += GR;
            D1[j] = v_f32_ld_tnsr_b(dc, dyn) * dst;
        }
#ifdef Q8
        float64 amax = 0.0f;
#endif
        for (int v = 0; v < NV; v++) {
            const int j = v >> 4, l0 = (v & 15) * 4;
            int5 bc = {64 * v, k0, 0, 0, 0};
            const float64 c0 = v_f32_ld_tnsr_b(bc, base) + expand4(D0[j], l0, lanef);
            bc[1] = k0 + 1;
            const float64 c1 = v_f32_ld_tnsr_b(bc, base) + expand4(D1[j], l0, lanef);
            int5 xc = {64 * v, t, 0, 0, 0};
            float64 cur = v_f32_ld_tnsr_b(xc, x);
            float64 prev = 0.0f;
            if (!first) {
                xc[1] = t - 1;
                prev = v_f32_ld_tnsr_b(xc, x);
            }
#ifdef SCALED
            cur = cur * sx;
            prev = prev * sp;
#endif
            const float64 o = v_f32_mac_b(c1, prev, c0 * cur);
#ifdef Q8
            OUT[v] = o;
            amax = v_f32_max_b(amax, v_f32_abs_b(o));
#else
            int5 oc = {64 * v, t, 0, 0, 0};
            v_f32_st_tnsr(oc, out, o);
#endif
        }
#ifdef Q8
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        for (int v = 0; v < NV; v += 4) {
            int5 qc = {64 * v, t, 0, 0, 0};
            v_f8_st_tnsr(qc, q, pack_tau(OUT[v] * inv, OUT[v + 1] * inv, OUT[v + 2] * inv, OUT[v + 3] * inv));
        }
        STORE_TOKEN_SCALE(qs, t, sc)
#endif
    }
}
