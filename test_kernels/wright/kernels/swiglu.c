// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// SwiGLU: out = bf16( silu(g) * u ), sigma order.   gu = [g (F) | u (F)]
// tensors: 0 gu (2F,T) f32 | 1 out (F,T) bf16.  index space: [F/128, T]
// FP8 build (-DFP8): gu is an FP8-GEMM output with per-token scale gs (1,T); the output is f32
// (natural order), quantized for the down-proj FP8 GEMM by quant_fp8.
//   tensors: 0 gu  1 gs | 2 out (F,T) f32
#include "common.h"

#ifdef FP8
void main(tensor gu, tensor gs, tensor out)
#elif defined(LIM)
void main(tensor gu, tensor out, float lim)
#else
void main(tensor gu, tensor out)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int F = get_dim_size(out, 0);
#ifdef FP8
    DECL_IDX(IDX)
#endif
    for (int t = off[1]; t < end[1]; t++) {
#ifdef FP8
        const float64 s = TOKEN_SCALE(gs, t, IDX[0]);
#endif
        for (int blk = off[0]; blk < end[0]; blk++) {
            const int i = blk * 128;
            int5 c = {i, t, 0, 0, 0};
            const float64 ga = CLAMP_G(v_f32_ld_tnsr_b(c, gu));
            c[0] = i + 64;     const float64 gb = CLAMP_G(v_f32_ld_tnsr_b(c, gu));
            c[0] = F + i;      const float64 ua = CLAMP_U(v_f32_ld_tnsr_b(c, gu));
            c[0] = F + i + 64; const float64 ub = CLAMP_U(v_f32_ld_tnsr_b(c, gu));
            c[0] = i;
#ifdef FP8
            v_f32_st_tnsr(c, out, silu_f32(ga * s) * ua * s);
            c[0] = i + 64;
            v_f32_st_tnsr(c, out, silu_f32(gb * s) * ub * s);
#else
            v_bf16_st_tnsr(c, out, pack_sigma(silu_f32(ga) * ua, silu_f32(gb) * ub));
#endif
        }
    }
}
