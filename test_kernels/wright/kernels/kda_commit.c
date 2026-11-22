// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Speculative commit: the accepted tokens of a kda_verify step, applied to each sequence's arena state. counts[bl]
// tokens (from the first) of entry bl = (layer, sequence) are replayed from the recorded updates,
// S = diag(e_t) S + k_t d_t^T per head, read from ssm_src and stored at ssm_dst; the conv state rolls forward over the tokens'
// recorded inputs (conv_src -> conv_dst). A count of 0 leaves the entry alone.
// tensors: 0 arena (128, PR, NB) f32  1 pend (128, 3, nh, Tq, BL) f32 [k | d | e]  2 pconv (3C, Tq, BL) f32
//          3 bases (4, BL) i32 [conv_src, ssm_src, conv_dst, ssm_dst]  4 counts (BL) i32 | 5 sink (64, 1) i32
// index space: [nh, BL]; member (0, 0) writes the sink.
#include "common.h"

#define DK 128

__local__ float64 S[2 * DK];
__local__ float64 K4[8];
__local__ float64 E4[8];

// one token's replay over rows i .. i + 3 of each 16-row group: x from LOAD, r to STORE
#define LOAD_VLM                                                                       \
    x[0] = S[2 * i]; x[1] = S[2 * i + 1]; x[2] = S[2 * i + 2]; x[3] = S[2 * i + 3];    \
    x[4] = S[2 * i + 4]; x[5] = S[2 * i + 5]; x[6] = S[2 * i + 6]; x[7] = S[2 * i + 7];
#define LOAD_ARENA                                                                     \
    lc[1] = srow + i;                                                                  \
    lc[0] = 0; x[0] = v_f32_ld_tnsr_b(lc, arena); lc[0] = 64; x[1] = v_f32_ld_tnsr_b(lc, arena); lc[1]++; \
    lc[0] = 0; x[2] = v_f32_ld_tnsr_b(lc, arena); lc[0] = 64; x[3] = v_f32_ld_tnsr_b(lc, arena); lc[1]++; \
    lc[0] = 0; x[4] = v_f32_ld_tnsr_b(lc, arena); lc[0] = 64; x[5] = v_f32_ld_tnsr_b(lc, arena); lc[1]++; \
    lc[0] = 0; x[6] = v_f32_ld_tnsr_b(lc, arena); lc[0] = 64; x[7] = v_f32_ld_tnsr_b(lc, arena);
#define STORE_VLM                                                                      \
    S[2 * i] = r[0]; S[2 * i + 1] = r[1]; S[2 * i + 2] = r[2]; S[2 * i + 3] = r[3];    \
    S[2 * i + 4] = r[4]; S[2 * i + 5] = r[5]; S[2 * i + 6] = r[6]; S[2 * i + 7] = r[7];
#define STORE_ARENA                                                                    \
    sc[1] = drow + i;                                                                  \
    sc[0] = 0; v_f32_st_tnsr(sc, arena, r[0]); sc[0] = 64; v_f32_st_tnsr(sc, arena, r[1]); sc[1]++; \
    sc[0] = 0; v_f32_st_tnsr(sc, arena, r[2]); sc[0] = 64; v_f32_st_tnsr(sc, arena, r[3]); sc[1]++; \
    sc[0] = 0; v_f32_st_tnsr(sc, arena, r[4]); sc[0] = 64; v_f32_st_tnsr(sc, arena, r[5]); sc[1]++; \
    sc[0] = 0; v_f32_st_tnsr(sc, arena, r[6]); sc[0] = 64; v_f32_st_tnsr(sc, arena, r[7]);
#define REPLAY(LOAD, STORE)                                                            \
    for (int g = 0; g < 8; g++) {                                                      \
        const float64 k4 = K4[g], e4 = E4[g];                                          \
        _Pragma("unroll")                                                              \
        for (int e = 0; e < 16; e += 4) {                                              \
            const int i = g * 16 + e;                                                  \
            float64 x[8], r[8];                                                        \
            LOAD                                                                       \
            const float64 kb0 = v_f32_shuffle_b(k4, IDXL[e], 0, k4), eg0 = v_f32_shuffle_b(e4, IDXL[e], 0, e4);             \
            const float64 kb1 = v_f32_shuffle_b(k4, IDXL[e + 1], 0, k4), eg1 = v_f32_shuffle_b(e4, IDXL[e + 1], 0, e4);     \
            const float64 kb2 = v_f32_shuffle_b(k4, IDXL[e + 2], 0, k4), eg2 = v_f32_shuffle_b(e4, IDXL[e + 2], 0, e4);     \
            const float64 kb3 = v_f32_shuffle_b(k4, IDXL[e + 3], 0, k4), eg3 = v_f32_shuffle_b(e4, IDXL[e + 3], 0, e4);     \
            r[0] = v_f32_mac_b(kb0, d0, x[0] * eg0); r[1] = v_f32_mac_b(kb0, d1, x[1] * eg0); \
            r[2] = v_f32_mac_b(kb1, d0, x[2] * eg1); r[3] = v_f32_mac_b(kb1, d1, x[3] * eg1); \
            r[4] = v_f32_mac_b(kb2, d0, x[4] * eg2); r[5] = v_f32_mac_b(kb2, d1, x[5] * eg2); \
            r[6] = v_f32_mac_b(kb3, d0, x[6] * eg3); r[7] = v_f32_mac_b(kb3, d1, x[7] * eg3); \
            STORE                                                                      \
        }                                                                              \
    }

void main(tensor arena, tensor pend, tensor pconv, tensor bases, tensor counts, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int C3 = get_dim_size(pconv, 0), C = C3 / 3;
    scalar_cache_reset();
    uchar256 IDXL[16];
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);
    const float64 zero = 0.0f;

    for (int bl = off[1]; bl < end[1]; bl++) {
        int5 nc = {bl, 0, 0, 0, 0};
        const int count = s_i32_ld_g(gen_addr(nc, counts));
        if (count <= 0)
            continue;
        int5 bc = {0, bl, 0, 0, 0};
        const int conv_src = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 1; const int ssm_src = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 2; const int conv_dst = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 3; const int ssm_dst = s_i32_ld_g(gen_addr(bc, bases));
        const int srow0 = ssm_src % PR, sblk0 = ssm_src / PR;
        const int drow0 = ssm_dst % PR, dblk0 = ssm_dst / PR;
        const int csrow = conv_src % PR, csblk = conv_src / PR;
        const int cdrow = conv_dst % PR, cdblk = conv_dst / PR;
        for (int h = off[0]; h < end[0]; h++) {
            int srow = srow0 + h * DK, sblk = sblk0, drow = drow0 + h * DK, dblk = dblk0;
            while (srow >= PR) { srow -= PR; sblk++; }
            while (drow >= PR) { drow -= PR; dblk++; }
            if (ssm_src < 0) {   // a fresh sequence: the first token reads zeros
                #pragma unroll 4
                for (int i = 0; i < 2 * DK; i++) S[i] = zero;
            }
            for (int t = 0; t < count; t++) {
                int5 rc = {0, 0, h, t, bl};
                const float64 k0 = v_f32_ld_tnsr_b(rc, pend);
                rc[0] = 64; const float64 k1 = v_f32_ld_tnsr_b(rc, pend);
                rc[0] = 0; rc[1] = 1; const float64 d0 = v_f32_ld_tnsr_b(rc, pend);
                rc[0] = 64; const float64 d1 = v_f32_ld_tnsr_b(rc, pend);
                rc[0] = 0; rc[1] = 2; const float64 ea = v_f32_ld_tnsr_b(rc, pend);
                rc[0] = 64; const float64 eb = v_f32_ld_tnsr_b(rc, pend);
                K4[0] = BCAST_DG(k0, 0); K4[1] = BCAST_DG(k0, 1); K4[2] = BCAST_DG(k0, 2); K4[3] = BCAST_DG(k0, 3);
                K4[4] = BCAST_DG(k1, 0); K4[5] = BCAST_DG(k1, 1); K4[6] = BCAST_DG(k1, 2); K4[7] = BCAST_DG(k1, 3);
                E4[0] = BCAST_DG(ea, 0); E4[1] = BCAST_DG(ea, 1); E4[2] = BCAST_DG(ea, 2); E4[3] = BCAST_DG(ea, 3);
                E4[4] = BCAST_DG(eb, 0); E4[5] = BCAST_DG(eb, 1); E4[6] = BCAST_DG(eb, 2); E4[7] = BCAST_DG(eb, 3);
                const bool from_arena = t == 0 && ssm_src >= 0;
                const bool to_arena = t == count - 1 && ssm_dst >= 0;
                int5 lc = {0, srow, sblk, 0, 0}, sc = {0, drow, dblk, 0, 0};
                if (from_arena && to_arena) {
                    REPLAY(LOAD_ARENA, STORE_ARENA)
                } else if (from_arena) {
                    REPLAY(LOAD_ARENA, STORE_VLM)
                } else if (to_arena) {
                    REPLAY(LOAD_VLM, STORE_ARENA)
                } else {
                    REPLAY(LOAD_VLM, STORE_VLM)
                }
            }
            // ---- conv state: the last 3 of [old 3 inputs, the tokens' inputs]
            if (conv_dst < 0)
                continue;
            #pragma unroll 1
            for (int c = 0; c < 6; c++) {
                const int chan = (c >> 1) * C + h * DK + 64 * (c & 1);
                float64 v[3];
                #pragma unroll
                for (int j = 0; j < 3; j++) {
                    const int seq = count - 3 + j;  // index into the tokens; < 0: the old state's column 3 + seq
                    if (seq >= 0) {
                        int5 pc = {chan, seq, bl, 0, 0};
                        v[j] = v_f32_ld_tnsr_b(pc, pconv);
                    } else if (conv_src >= 0) {
                        int r = csrow + ((3 + seq) * C3 + chan) / DK, q = csblk;
                        if (r >= PR) { r -= PR; q++; }
                        int5 sc = {chan % DK, r, q, 0, 0};
                        v[j] = v_f32_ld_tnsr_b(sc, arena);
                    } else {
                        v[j] = zero;
                    }
                }
                #pragma unroll
                for (int j = 0; j < 3; j++) {
                    int r = cdrow + (j * C3 + chan) / DK, q = cdblk;
                    if (r >= PR) { r -= PR; q++; }
                    int5 oc = {chan % DK, r, q, 0, 0};
                    v_f32_st_tnsr(oc, arena, v[j]);
                }
            }
        }
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
