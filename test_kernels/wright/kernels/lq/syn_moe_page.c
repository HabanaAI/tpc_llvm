// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Routed matrix product by pages (many tokens): token-expert pairs grouped by expert into pages of PS = 2^psh rows, one
// batch_gemm over the pages (B = the pages' experts dequantized by syn_qdqp_*). pair = t * nu + s; the tokens
// are split into chunks of tc tokens (chunk m: tokens [m tc, (m + 1) tc)). Ep is E rounded up to 64, and
// tc nu is a multiple of 32.
//  -DCNT : cnt[m][e] = pairs of chunk m routed to e. item = chunk. tensors: 0 ids | 1 cnt [NC Ep]
//  -DTOT : tot[e] = sum over chunks of cnt[m][e] (vectors). item = 64 experts. tensors: 0 cnt | 1 tot [Ep]
//  -DPLAN: one member. pe[p] = the expert of page p (-1: unused), pgb[e] = the first page row of expert e.
//          tensors: 0 tot | 1 pe [P]  2 pgb [Ep]
//  -DBASE: base[m][e] = the first page row of chunk m's pairs of expert e (rows in token order; vectors).
//          item = 64 experts. tensors: 0 cnt  1 pgb | 2 base [NC Ep]
//  -DASG : pairrow[pair] = the pair's page row (-1: expert out of range). item = chunk. tensors: 0 ids  1 base |
//          2 pairrow  3 cur [NC Ep] (unused)
//  -DGX  : xp[:, r % PS, r / PS] (bf16 [K, PS, P]) = x[:, s or 0, t] for r = pairrow[pair]; rows no pair maps to
//          are left as they are. item = pair; bcast:
//          1 one x row per token, 2 the same and item = token (nu <= 32).
//          tensors: 0 x  1 pairrow | 2 xp
//  -DGLU : xp[k, i, p] (bf16 [ff, PS, P]) = silu(a[k, row]) * b[k, row], row = p PS + i, for the used pages
//          (pe[p] >= 0): the SwiGLU of the gate / up outputs read as page rows (flat f32, row r at
//          off + r s), written as the next product's page input. Rows no
//          pair maps to are computed too. The f32 -> bf16 is GX's. item = (page, row). tensors: 0 a  1 b  2 pe | 3 xp
//  else  : out[:, s, t] (f32 [M, nu, T]) = y[:, r % PS, r / PS] (f32 [M, PS, P]) for r = pairrow[pair], zeros
//          for -1. item = (pair, 64 rows). tensors: 0 y  1 pairrow | 2 out
#include "syn_common.h"

static inline int ldi(tensor t, int i)
{
    int5 c = {i, 0, 0, 0, 0};
    return s_i32_ld_g((__global int *) gen_addr(c, t));
}

static inline void sti(tensor t, int i, int v)
{
    int5 c = {i, 0, 0, 0, 0};
    s_i32_st_g((__global int *) gen_addr(c, t), v);
}

#define MAXE 512   // per-expert state (E <= MAXE)

#if defined(CNT)
void main(tensor ids, tensor dep, tensor cnt, int nmem, int nu, int T, int E, int Ep, int tc, int offi, int si0,
          int si1)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    scalar_cache_reset();
    const int64 lane = (int64) read_lane_id_4b_b();
    int c[MAXE];
    for (int m = ioff[0]; m < iend[0]; m++) {
        for (int e = 0; e < E; e++) {
            c[e] = 0;
        }
        const int t1 = (m + 1) * tc < T ? (m + 1) * tc : T;
        for (int t = m * tc; t < t1; t++) {
            for (int s = 0; s < nu; s++) {
                const int e = ldi(ids, offi + s * si0 + t * si1);
                if (e >= 0 && e < E) {
                    c[e]++;
                }
            }
        }
        // the row as vectors (counts of experts >= E: 0)
        for (int q = 0; q < Ep; q += 64) {
            int64 v = 0;
            for (int l = 0; l < 64 && q + l < E; l++) {
                v = v_i32_sel_eq_i32_b(lane, (int64) l, (int64) c[q + l], v);
            }
            int5 co = {m * Ep + q, 0, 0, 0, 0};
            v_i32_st_tnsr(co, cnt, v);
        }
    }
}
#elif defined(TOT)
void main(tensor cnt, tensor dep, tensor tot, int nmem, int Ep, int NC)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    for (int q = ioff[0]; q < iend[0]; q++) {
        int64 acc = 0;
        for (int m = 0; m < NC; m++) {
            int5 c = {m * Ep + q * 64, 0, 0, 0, 0};
            acc += v_i32_ld_tnsr_b(c, cnt);
        }
        int5 co = {q * 64, 0, 0, 0, 0};
        v_i32_st_tnsr(co, tot, acc);
    }
}
#elif defined(PLAN)
void main(tensor tot, tensor dep, tensor pe, tensor pgb, int nmem, int E, int psh, int P)
{
    const int5 ioff = get_index_space_offset();
    if (ioff[0] != 0) {
        return;
    }
    scalar_cache_reset();
    const int PS = 1 << psh;
    int pg = 0;
    for (int e = 0; e < E; e++) {
        const int c = ldi(tot, e);
        sti(pgb, e, pg << psh);
        for (int q = 0; q < c && pg < P; q += PS) {
            sti(pe, pg, e);
            pg++;
        }
    }
    for (; pg < P; pg++) {
        sti(pe, pg, -1);
    }
}
#elif defined(BASE)
void main(tensor cnt, tensor pgb, tensor dep, tensor base, int nmem, int Ep, int NC)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    for (int q = ioff[0]; q < iend[0]; q++) {
        int5 c = {q * 64, 0, 0, 0, 0};
        int64 run = v_i32_ld_tnsr_b(c, pgb);
        for (int m = 0; m < NC; m++) {
            c[0] = m * Ep + q * 64;
            v_i32_st_tnsr(c, base, run);
            run += v_i32_ld_tnsr_b(c, cnt);
        }
    }
}
#elif defined(ASG)
// pairrow[a, b) from the lanes [a - vb, b - vb) of v (vb: the 64-pair block of a)
#define FLUSH(vb, a, b) \
    { \
        int5 co_ = {(a), 0, 0, 0, 0}; \
        v_i32_st_tnsr_partial(co_, pairrow, v, (b) - (a) - 1, (a) - (vb)); \
    }

void main(tensor ids, tensor base, tensor dep, tensor pairrow, tensor cur, int nmem, int nu, int T, int E, int Ep,
          int tc, int offi, int si0, int si1)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    scalar_cache_reset();
    const int64 lane = (int64) read_lane_id_4b_b();
    int c[MAXE];
    for (int m = ioff[0]; m < iend[0]; m++) {
        const int row = m * Ep;
        for (int e = 0; e < E; e++) {
            c[e] = ldi(base, row + e);
        }
        const int t0 = m * tc, t1 = (m + 1) * tc < T ? (m + 1) * tc : T;
        if (t0 >= t1) {
            continue;
        }
        // the chunk's pair rows, 64 at a time (pairs [vb, vb + 64))
        int vb = (t0 * nu) & ~63, va = t0 * nu;
        int64 v = -1;
        for (int t = t0; t < t1; t++) {
            for (int s = 0; s < nu; s++) {
                const int e = ldi(ids, offi + s * si0 + t * si1);
                int r = -1;
                if (e >= 0 && e < E) {
                    r = c[e];
                    c[e] = r + 1;
                }
                const int p = t * nu + s;
                v = v_i32_sel_eq_i32_b(lane, (int64) (p - vb), (int64) r, v);
                if (p - vb == 63) {
                    FLUSH(vb, va, p + 1)
                    vb += 64;
                    va = vb;
                    v = -1;
                }
            }
        }
        if (va < t1 * nu) {
            FLUSH(vb, va, t1 * nu)
        }
    }
}
#elif defined(GX)
void main(tensor x, tensor pairrow, tensor dep, tensor xp, int nmem, int K, int nu, int T, int psh, int bcast,
          int offx, int sx1, int sx2)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    scalar_cache_reset();
    const int PS = 1 << psh;
    if (bcast == 2) {
        // one row of x per token: converted once, stored to each of the token's page rows (nu <= 32)
        int rr[32], rp[32];
        for (int m = ioff[0]; m < iend[0]; m++) {
            int t0, t1;
            SPLIT_RANGE(T, nmem, m, t0, t1)
            for (int t = t0; t < t1; t++) {
                int n = 0;
                for (int s = 0; s < nu; s++) {
                    const int r = ldi(pairrow, t * nu + s);
                    rr[n] = r & (PS - 1);
                    rp[n] = r >> psh;
                    n += r >= 0 ? 1 : 0;
                }
                const int px = offx + t * sx2;
                for (int k = 0; k < K; k += 128) {
                    const bfloat128 v = cvt_bf16(ld_f32(x, px + k), ld_f32(x, px + k + 64));
                    const int left = K - k;
                    for (int j = 0; j < n; j++) {
                        int5 co = {k, rr[j], rp[j], 0, 0};
                        if (left >= 128) v_bf16_st_tnsr(co, xp, v); else v_bf16_st_tnsr_partial(co, xp, v, left - 1, 0);
                    }
                }
            }
        }
        return;
    }
    const int nitems = T * nu;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int pair = i0; pair < i1; pair++) {
            const int r = ldi(pairrow, pair);
            if (r < 0) {
                continue;
            }
            const int t = pair / nu, s = pair - t * nu;
            const int px = offx + (bcast ? 0 : s * sx1) + t * sx2;
            for (int k = 0; k < K; k += 128) {
                const bfloat128 v = cvt_bf16(ld_f32(x, px + k), ld_f32(x, px + k + 64));
                int5 co = {k, r & (PS - 1), r >> psh, 0, 0};
                const int left = K - k;
                if (left >= 128) v_bf16_st_tnsr(co, xp, v); else v_bf16_st_tnsr_partial(co, xp, v, left - 1, 0);
            }
        }
    }
}
#elif defined(GLU)
void main(tensor ya, tensor yb, tensor pe, tensor dep, tensor xp, int nmem, int ff, int psh, int P, int offa, int sa,
          int offb, int sb)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    scalar_cache_reset();
    const int PS = 1 << psh;
    const int nitems = P * PS;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        int pcur = -1;
        bool used = false;
        for (int it = i0; it < i1; it++) {
            const int p = it >> psh;
            if (p != pcur) {
                pcur = p;
                used = ldi(pe, p) >= 0;
            }
            if (!used) {
                continue;
            }
            const int ra = offa + it * sa, rb = offb + it * sb;   // row (p, i)
            int k = 0;
            // 256 columns per step
            for (; k + 256 <= ff; k += 256) {
                const float64 a0 = ld_f32(ya, ra + k), a1 = ld_f32(ya, ra + k + 64);
                const float64 a2 = ld_f32(ya, ra + k + 128), a3 = ld_f32(ya, ra + k + 192);
                const float64 b0 = ld_f32(yb, rb + k), b1 = ld_f32(yb, rb + k + 64);
                const float64 b2 = ld_f32(yb, rb + k + 128), b3 = ld_f32(yb, rb + k + 192);
                const bfloat128 v0 = cvt_bf16(nl_silu_acc(a0) * b0, nl_silu_acc(a1) * b1);
                const bfloat128 v1 = cvt_bf16(nl_silu_acc(a2) * b2, nl_silu_acc(a3) * b3);
                int5 co = {k, it & (PS - 1), p, 0, 0};
                v_bf16_st_tnsr(co, xp, v0);
                co[0] = k + 128;
                v_bf16_st_tnsr(co, xp, v1);
            }
            for (; k < ff; k += 128) {
                const float64 a0 = ld_f32(ya, ra + k), a1 = ld_f32(ya, ra + k + 64);
                const float64 b0 = ld_f32(yb, rb + k), b1 = ld_f32(yb, rb + k + 64);
                const bfloat128 v = cvt_bf16(nl_silu_acc(a0) * b0, nl_silu_acc(a1) * b1);
                int5 co = {k, it & (PS - 1), p, 0, 0};
                const int left = ff - k;
                if (left >= 128) v_bf16_st_tnsr(co, xp, v); else v_bf16_st_tnsr_partial(co, xp, v, left - 1, 0);
            }
        }
    }
}
#else
void main(tensor y, tensor pairrow, tensor dep, tensor out, int nmem, int M, int nu, int T, int psh)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    scalar_cache_reset();
    const int PS = 1 << psh;
    const int nk = (M + 63) >> 6;
    const int nitems = T * nu * nk;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        int pcur = -1, r = -1;
        for (int it = i0; it < i1; it++) {
            const int pair = it / nk, k = (it - pair * nk) << 6;
            if (pair != pcur) {
                pcur = pair;
                r = ldi(pairrow, pair);
            }
            float64 v = 0.0f;
            if (r >= 0) {
                int5 cy = {k, r & (PS - 1), r >> psh, 0, 0};
                v = v_f32_ld_tnsr_b(cy, y);
            }
            ST_F32(out, pair * M + k, v, M - k);
        }
    }
}
#endif
