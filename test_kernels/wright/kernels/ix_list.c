// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA key lists (Mistral-Large-4): per column c, the positions of its kept keys (sparse decode
// attention): first the keys whose mkey(score) is above the threshold (ix_thr), then the ties
// (equal), each in ascending position, at most KS in all (a column that sees at most KS keys
// lists all of them: its threshold 1 is below every key). ix_bcnt's block counts give every
// member its list offsets: member m takes blocks
// [m nb / NM, (m + 1) nb / NM) of nb = ceil(vis / 64).
// Also writes column c's slot record for the sparse attention launch:
//   slots[:, c] = [group (0, or -1 with no visible keys), H c, KS, 0, count, c, 0, cut]
// cut (one member per column, plain lists): the ties listed are those at positions <= cut (0x7FFFFFFF: every
// tie; -1: none).
// (qcol0 = H c: query block 1; qpos0 = KS: every listed key is visible; key0 = 0).
// tensors: 0 score (K, T) f32  1 vis (T) i32  2 thr (64, T) i32  3 cnt (K / 64, T) i32
//          4 gsum (K / 4096, T) i32 (ix_bcnt) | 5 list (KS, T) i32  6 slots (8, T) i32
// params: NM (members per column), KS, H. index space: [NM, T]
#include "common.h"

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

// entries [a, b) of the list vector whose first entry is vb, from lanes [a - vb, b - vb) of v (keyed values)
#ifdef MAP
#define DECODE(v) (((v) ^ (int64)BIAS) & (int64)0x3FFFF)
#else
#define DECODE(v) ((v) ^ (int64)BIAS)
#endif
#define FLUSH(vb, a, b, v) \
    { \
        int5 co_ = {(a), c, 0, 0, 0}; \
        v_i32_st_tnsr_partial(co_, list, DECODE(v), (b) - (a) - 1, (a) - (vb)); \
    }

#define BIAS 0x80000000
// min / max over the 64 lanes into every lane: dual groups, then a butterfly within a dual group
#define RED(x, out, OP) \
    { \
        int64 s_ = OP((x), v_i32_mov_dual_group_all_b((x), 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, (x))); \
        s_ = OP(s_, v_i32_mov_dual_group_all_b(s_, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, s_)); \
        s_ = OP(s_, v_i32_shuffle_b(s_, BF1, 0, s_)); \
        s_ = OP(s_, v_i32_shuffle_b(s_, BF2, 0, s_)); \
        s_ = OP(s_, v_i32_shuffle_b(s_, BF4, 0, s_)); \
        out = OP(s_, v_i32_shuffle_b(s_, BF8, 0, s_)); \
    }

// lane l <- element e (0..15) of its dual group
static inline uchar256 ctl_of(int64 e)
{
    const int64 c = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    return __builtin_bit_cast(uchar256, c);
}

#ifdef MAP
void main(tensor score, tensor vis, tensor thr, tensor cnt, tensor gsum, tensor cpos, tensor list, tensor slots,
          int NM, int KS, int H)
#else
void main(tensor score, tensor vis, tensor thr, tensor cnt, tensor gsum, tensor list, tensor slots,
          int NM, int KS, int H)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e16 = lane & 15;
    const uchar256 BF1 = ctl_of(e16 ^ 1), BF2 = ctl_of(e16 ^ 2), BF4 = ctl_of(e16 ^ 4), BF8 = ctl_of(e16 ^ 8);
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        const int nb = (lim + 63) >> 6;
        int5 tc = {0, c, 0, 0, 0};
        const uint64 th = v_u32_ld_tnsr_b(tc, thr);
        for (int m = off[0]; m < end[0]; m++) {
            const int b0 = m * nb / NM, b1 = (m + 1) * nb / NM;
            // list offsets: strict keys before b0, all strict keys, ties before b0, all ties
            // (group sums of 64 blocks, then the blocks of b0's group before it)
            int s0 = 0, st = 0, t0 = 0, tt = 0;
            for (int q = 0; q < (nb + 63) >> 6; q++) {
                int5 gc = {q, c, 0, 0, 0};
                const int x = s_i32_ld_g(gen_addr(gc, gsum));
                const int xs = x & 0xFFFF, xt = x >> 16;
                if (q < b0 >> 6) {
                    s0 += xs;
                    t0 += xt;
                }
                st += xs;
                tt += xt;
            }
            for (int b = b0 & ~63; b < b0; b++) {
                int5 cc = {b, c, 0, 0, 0};
                const int x = s_i32_ld_g(gen_addr(cc, cnt));
                s0 += x & 0xFFFF;
                t0 += x >> 16;
            }
            if (m == 0) {  // the slot record
                const int count = st + tt < KS ? st + tt : KS;
                int64 rec = 0;
                rec = v_i32_sel_eq_i32_b(lane, (int64)0, (int64)(lim > 0 ? 0 : -1), rec);
                rec = v_i32_sel_eq_i32_b(lane, (int64)1, (int64)(H * c), rec);
                rec = v_i32_sel_eq_i32_b(lane, (int64)2, (int64)KS, rec);
                rec = v_i32_sel_eq_i32_b(lane, (int64)4, (int64)count, rec);
                rec = v_i32_sel_eq_i32_b(lane, (int64)5, (int64)c, rec);
                int5 rc = {0, c, 0, 0, 0};
                v_i32_st_tnsr_partial(rc, slots, rec, 7, 0);
            }
            // pass 0: strict keys at s0..; pass 1: ties at st + t0.. A block's n entries take list slots
            // g .. g + n - 1, filled from both ends at once (the lowest and the highest remaining position) while
            // all n fit under KS, else from the low end only. Values are keyed (position, then cpos with MAP) as
            // biased signed ints so that one min or max finds both; vA and vB hold list slots vb .. vb + 127.
            int64 tw = (int64)BIAS;  // the largest tie listed (keyed)
            for (int pass = 0; pass < 2; pass++) {
                int g = pass == 0 ? s0 : st + t0;  // the next entry's list index
                int vb = g & ~63, va = g;
                int64 vA = 0, vB = 0;
                for (int b = b0; b < b1 && g < KS; b++) {
                    int5 cc = {b, c, 0, 0, 0};
                    const int x = s_i32_ld_g(gen_addr(cc, cnt));
                    const int n = pass == 0 ? (x & 0xFFFF) : (x >> 16);
                    if (n == 0)
                        continue;
                    int5 sc = {b * 64, c, 0, 0, 0};
                    const uint64 k = mkey(v_f32_ld_tnsr_b(sc, score));
                    const int64 pos = lane + (int64)(b * 64);
#ifdef MAP
                    const int64 key = ((pos << 18) | v_i32_ld_tnsr_b(sc, cpos)) ^ (int64)BIAS;
#else
                    const int64 key = pos ^ (int64)BIAS;
#endif
                    // candidate lanes: visible and (above | equal to) the threshold
                    const uint64 none = (uint64)0x7FFFFFFF, ukey = as_uint64(key);
                    int64 lo = as_int64(pass == 0 ? v_u32_sel_less_u32_b(th, k, ukey, none)
                                                  : v_u32_sel_eq_u32_b(k, th, ukey, none));
                    lo = v_i32_sel_less_i32_b(pos, (int64)lim, lo, (int64)0x7FFFFFFF);
                    int64 hi = v_i32_sel_eq_i32_b(lo, (int64)0x7FFFFFFF, (int64)BIAS, lo);
                    const int take = n < KS - g ? n : KS - g;
                    int a = g, z = g + take - 1;
                    if (take == n)
                        for (; a < z; a++, z--) {
                            int64 l, h;
                            RED(lo, l, v_i32_min_b);
                            RED(hi, h, v_i32_max_b);
                            vA = v_i32_sel_eq_i32_b(lane, (int64)(a - vb), l, vA);
                            vB = v_i32_sel_eq_i32_b(lane, (int64)(a - vb - 64), l, vB);
                            vA = v_i32_sel_eq_i32_b(lane, (int64)(z - vb), h, vA);
                            vB = v_i32_sel_eq_i32_b(lane, (int64)(z - vb - 64), h, vB);
                            if (pass == 1)
                                tw = v_i32_max_b(tw, h);
                            lo = v_i32_sel_eq_i32_b(lo, l, (int64)0x7FFFFFFF, lo);
                            lo = v_i32_sel_eq_i32_b(lo, h, (int64)0x7FFFFFFF, lo);
                            hi = v_i32_sel_eq_i32_b(hi, h, (int64)BIAS, hi);
                            hi = v_i32_sel_eq_i32_b(hi, l, (int64)BIAS, hi);
                        }
                    for (; a <= z; a++) {
                        int64 l;
                        RED(lo, l, v_i32_min_b);
                        vA = v_i32_sel_eq_i32_b(lane, (int64)(a - vb), l, vA);
                        vB = v_i32_sel_eq_i32_b(lane, (int64)(a - vb - 64), l, vB);
                        if (pass == 1)
                            tw = v_i32_max_b(tw, l);
                        lo = v_i32_sel_eq_i32_b(lo, l, (int64)0x7FFFFFFF, lo);
                    }
                    g += take;
                    if (g - vb >= 64) {
                        FLUSH(vb, va, vb + 64, vA)
                        vA = vB;
                        vB = 0;
                        vb += 64;
                        va = vb;
                    }
                }
                if (g > va)
                    FLUSH(vb, va, g, vA)
            }
#ifndef MAP
            if (NM == 1) {  // slot lane 7: the tie cut (ties at or before it are listed)
                const int cut = st + tt <= KS ? 0x7FFFFFFF : st >= KS ? -1 : 0;
                int64 cv = (int64)cut;
                if (cut == 0) {
                    int64 twr;
                    RED(tw, twr, v_i32_max_b);
                    cv = DECODE(twr);
                }
                int5 rc = {7, c, 0, 0, 0};
                v_i32_st_tnsr_partial(rc, slots, cv, 0, 0);
            }
#endif
        }
    }
}
