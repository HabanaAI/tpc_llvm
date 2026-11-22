// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// PLE n-gram rows on the device (decode, one token): the hashed n-gram table rows of the transformers
// Qwen4ExpTextNGramEmbedding, bit for bit.
//   ctx: t0 = the token, t1 / t2 = the previous two (t2 reads as EOS when t1 is EOS; the history
//   starts as EOS, EOS). mixed_2 = A0[t0] ^ A1[t1], mixed_3 = mixed_2 ^ A2[t2] (64-bit; A_j[t] = t * m_j
//   precomputed on the host as (hi, lo) u32 pairs), head h (lanes 0..NH-1) uses mixed_2 for h < NH/2.
//   row_h = mixed mod p_h + off_h, as a local table row (other ranks' rows -> the zero row nloc).
// 64-bit mod p (p < 2^25): hi mod p (f32 quotient estimate + exact integer correction), then
// (hi mod p) * (2^32 mod p) + lo < 2^51 reduced the same way (u32 x u32 -> 64-bit products).
// tensors: 0 tok (T,) i32 (token = element 0)  1 hin (64,) i32 [t1, t2]  2 A (8, V) i32 [hi0 lo0 hi1 lo1 hi2 lo2]
//          3 HP (64, 4) i32 per head lane: p, off - table_lo, 2^32 mod p, f32 bits of 1/p
//        | 4 rows (NH,) i32  5 hout (64,) i32 [t0, t1]
// params: eos, nloc. index space: [1]
#include "common.h"

#define U2F(x) v_convert_u32_to_f32_b(x, SW_RHNE, 0, 1, 0)

static inline int64 fixr(int64 r, int64 p)          // r in (-2p, 2p) -> [0, p)
{
    r = v_i32_sel_less_i32_b(r, (int64)0, r + p, r);
    r = v_i32_sel_less_i32_b(r, (int64)0, r + p, r);
    r = v_i32_sel_less_i32_b(r, p, r, r - p);
    r = v_i32_sel_less_i32_b(r, p, r, r - p);
    return r;
}

static inline int64 mod64(uint64 hi, uint64 lo, int64 p, uint64 pu, uint64 C, float64 invp)
{
    // hi mod p
    uint64 q = v_convert_f32_to_u32_b(U2F(hi) * invp, SW_RZ, 0, 1, 0);
    int64 r = as_int64(hi - v_u32_mul_b(q, pu).v1);
    const uint64 hr = as_uint64(fixr(r, p));
    // x = hr * C + lo  (< 2^51)
    const uint128 m = v_u32_mul_b(hr, C);
    const uint64 l3 = m.v1 + lo;
    const uint64 carry = v_u32_sel_less_u32_b(l3, m.v1, (uint64)1, (uint64)0);
    const uint64 h3 = m.v2 + carry;
    const float64 fx = U2F(h3) * 4294967296.0f + U2F(l3);
    q = v_convert_f32_to_u32_b(fx * invp, SW_RZ, 0, 1, 0);
    const int64 r0 = as_int64(l3 - v_u32_mul_b(q, pu).v1);            // |r0| < 2^30
    const int64 q2 = v_convert_f32_to_i32_b(v_convert_i32_to_f32_b(r0, SW_RHNE, 0, 1, 0) * invp, SW_RZ, 0, 1, 0);
    return fixr(r0 - as_int64(v_u32_mul_b(as_uint64(q2), pu).v1), p);
}

// T tokens per launch (decode 1, speculative steps 1 + drafts): token t's context is the previous tokens of
// the launch, then the history. Rows (NH T,): token t at NH t. hout = the last two tokens; -DMID: hmid
// (64, T-1) = the history after token t at column t (the rollback points).
// -DARR (pipelined speculative step): one more input meta (after HP); hin (64, S) read at column meta[2]; hout
// (64, T) = the history after token t at column t (every t).
#ifdef ARR
void main(tensor tok, tensor hin, tensor A, tensor HP, tensor meta, tensor rows, tensor hout, int eos, int nloc)
#elif defined(MID)
void main(tensor tok, tensor hin, tensor A, tensor HP, tensor rows, tensor hout, tensor hmid, int eos, int nloc)
#else
void main(tensor tok, tensor hin, tensor A, tensor HP, tensor rows, tensor hout, int eos, int nloc)
#endif
{
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int64 lane = (int64)read_lane_id_4b_b();
    const int T = get_dim_size(tok, 0);
    const int NH = get_dim_size(rows, 0) / T;
#ifdef ARR
    const int sl = meta_i32(meta, 2);
#else
    const int sl = 0;
#endif
    int5 z = {0, sl, 0, 0, 0}, z1 = {1, sl, 0, 0, 0};
    int hp1 = s_i32_ld_g(gen_addr(z, hin)), hp2 = s_i32_ld_g(gen_addr(z1, hin));   // t-1, t-2
    int5 hc = {0, 0, 0, 0, 0};
    const int64 p = v_i32_ld_tnsr_b(hc, HP);
    hc[1] = 1; const int64 off = v_i32_ld_tnsr_b(hc, HP);
    hc[1] = 2; const uint64 C = as_uint64(v_i32_ld_tnsr_b(hc, HP));
    hc[1] = 3; const float64 invp = as_float64(v_i32_ld_tnsr_b(hc, HP));
    const int64 is2 = v_i32_sel_less_i32_b(lane, (int64)(NH / 2), (int64)1, (int64)0);
    for (int t = 0; t < T; t++) {
        int5 tc = {t, 0, 0, 0, 0};
        const int t0 = s_i32_ld_g(gen_addr(tc, tok));
        const int t1 = hp1;
        const int t2 = t1 == eos ? eos : hp2;
        int5 c0 = {0, t0, 0, 0, 0}, c1 = {0, t1, 0, 0, 0}, c2 = {0, t2, 0, 0, 0};
        const float64 a0 = BCAST_DG(as_float64(v_i32_ld_tnsr_b(c0, A)), 0);
        const float64 a1 = BCAST_DG(as_float64(v_i32_ld_tnsr_b(c1, A)), 0);
        const float64 a2 = BCAST_DG(as_float64(v_i32_ld_tnsr_b(c2, A)), 0);
#define LANE(v, e) as_uint64(v_f32_shuffle_b(v, IDX[e], 0, v))
        const uint64 hi2 = LANE(a0, 0) ^ LANE(a1, 2), lo2 = LANE(a0, 1) ^ LANE(a1, 3);
        const uint64 hi3 = hi2 ^ LANE(a2, 4), lo3 = lo2 ^ LANE(a2, 5);
        const uint64 hi = as_uint64(v_i32_sel_eq_i32_b(is2, (int64)1, as_int64(hi2), as_int64(hi3)));
        const uint64 lo = as_uint64(v_i32_sel_eq_i32_b(is2, (int64)1, as_int64(lo2), as_int64(lo3)));
        const int64 r = mod64(hi, lo, p, as_uint64(p), C, invp) + off;
        const int64 loc = v_i32_sel_less_i32_b(r, (int64)0, (int64)nloc, v_i32_sel_less_i32_b(r, (int64)nloc, r, (int64)nloc));
        int5 oc = {NH * t, 0, 0, 0, 0};
        v_i32_st_tnsr(oc, rows, loc);             // lanes beyond NH are overwritten by the next token
        hp2 = hp1; hp1 = t0;
#if defined(MID) || defined(ARR)
#ifdef ARR
#define hmid hout
        if (1) {
#else
        if (t < T - 1) {
#endif
            int5 mz = {0, t, 0, 0, 0};
            v_i32_st_tnsr(mz, hmid, v_i32_sel_eq_i32_b(lane, (int64)0, (int64)hp1, v_i32_sel_eq_i32_b(lane, (int64)1, (int64)hp2, (int64)0)));
        }
#endif
    }
#ifndef ARR
    v_i32_st_tnsr(z, hout, v_i32_sel_eq_i32_b(lane, (int64)0, (int64)hp1, v_i32_sel_eq_i32_b(lane, (int64)1, (int64)hp2, (int64)0)));
#endif
}
