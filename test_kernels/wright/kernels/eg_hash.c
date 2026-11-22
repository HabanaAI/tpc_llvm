// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 Engram hash rows of one layer, for columns whose newest token is known only on the device.
// Column t's n-gram of size n (lane l = 8 (n - 2) + head, 24 lanes) hashes
//   h = A0[tok[t]] ^ X[l, t],  row = h mod p_l + offset_l
// where A0[v] = compressed(v) * multiplier_0 (64-bit wrap) and X holds the XOR of the n - 1 older tokens' products,
// both as (hi, lo) u32 halves. 64-bit h mod p (p < 2^31) from 32-bit products and a f32 reciprocal, corrected.
// tensors: 0 tok (T,) i32  1 A0 (2, V) i32 [hi, lo]  2 xh (64, T) i32  3 xl (64, T) i32
//          4 hp (64, 4) i32 per lane [p, offset, 2^32 mod p, f32 bits of 1 / p] | 5 rows (24, T) i32
// index space: [T]
#include "common.h"

#define U2F(x) v_convert_u32_to_f32_b(x, SW_RHNE, 0, 1, 0)

static inline int64 fixr(int64 r, int64 p)  // r in (-2p, 2p) -> [0, p)
{
    r = v_i32_sel_less_i32_b(r, (int64)0, r + p, r);
    r = v_i32_sel_less_i32_b(r, (int64)0, r + p, r);
    r = v_i32_sel_less_i32_b(r, p, r, r - p);
    r = v_i32_sel_less_i32_b(r, p, r, r - p);
    return r;
}

static inline int64 mod64(uint64 hi, uint64 lo, int64 p, uint64 C, float64 invp)
{
    const uint64 pu = as_uint64(p);
    uint64 q = v_convert_f32_to_u32_b(U2F(hi) * invp, SW_RZ, 0, 1, 0);
    const uint64 hr = as_uint64(fixr(as_int64(hi - v_u32_mul_b(q, pu).v1), p));  // hi mod p
    const uint128 m = v_u32_mul_b(hr, C);                                         // (hi mod p) 2^32 mod p, 64-bit
    const uint64 lo3 = m.v1 + lo;
    const uint64 hi3 = m.v2 + v_u32_sel_less_u32_b(lo3, m.v1, (uint64)1, (uint64)0);
    const float64 fx = U2F(hi3) * 4294967296.0f + U2F(lo3);
    q = v_convert_f32_to_u32_b(fx * invp, SW_RZ, 0, 1, 0);
    const int64 r0 = as_int64(lo3 - v_u32_mul_b(q, pu).v1);
    const int64 q2 = v_convert_f32_to_i32_b(v_convert_i32_to_f32_b(r0, SW_RHNE, 0, 1, 0) * invp, SW_RZ, 0, 1, 0);
    return fixr(r0 - as_int64(v_u32_mul_b(as_uint64(q2), pu).v1), p);
}

void main(tensor tok, tensor A0, tensor xh, tensor xl, tensor hp, tensor rows)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    int5 hc = {0, 0, 0, 0, 0};
    const int64 p = v_i32_ld_tnsr_b(hc, hp);
    hc[1] = 1; const int64 offset = v_i32_ld_tnsr_b(hc, hp);
    hc[1] = 2; const uint64 C = as_uint64(v_i32_ld_tnsr_b(hc, hp));
    hc[1] = 3; const float64 invp = as_float64(v_i32_ld_tnsr_b(hc, hp));
    for (int t = off[0]; t < end[0]; t++) {
        int5 tc = {t, 0, 0, 0, 0};
        const int v = s_i32_ld_g(gen_addr(tc, tok));
        int5 ac = {0, v, 0, 0, 0};
        const unsigned ahi = s_i32_ld_g(gen_addr(ac, A0));
        ac[0] = 1;
        const unsigned alo = s_i32_ld_g(gen_addr(ac, A0));
        int5 xc = {0, t, 0, 0, 0};
        const uint64 hi = as_uint64(v_i32_ld_tnsr_b(xc, xh)) ^ (uint64)ahi;
        const uint64 lo = as_uint64(v_i32_ld_tnsr_b(xc, xl)) ^ (uint64)alo;
        v_i32_st_tnsr(xc, rows, mod64(hi, lo, p, C, invp) + offset);
    }
}
