// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MXFP4 -> FP8 (E4M3 bias 7, +-240), natural element order.
//   out = FP8(code * 2^(S - 127 - k)): the tensor scale is 2^k (x = out * 2^k), applied by the caller.
//   Exact while every block is at most 13 exponents below the block that sets k (k = max S - 127 - 5);
//   larger values saturate at +-240.
// tensors: 0 codes (ceil(K/2), M) u8, low nibble first | 1 scales (ceil(K/32), M) u8 E8M0 | 2 out (K, M) FP8
//   With SLOTS (mx4_decs.c): 2 ids i32  3 ktab (E) i32 | 4 out (K, M): row m of out decodes row e of codes and
//   scales with k = ktab[e], where e = ids[m % R, m / R] for ids (64, T) when R > 0, or e = ids[m] for ids (M) when
//   R = 0 (e < 0: row m is not written).
// params: k (SLOTS: R, u0: the slots start u0 units (of 2048 elements) into each row of codes and scales). index space: [ceil(K/2048), M]; one member = 2048 elements = 64 blocks. A row's tail is written past
// K, and the store of the tail is capped at the tensor's dim 0. Rows of K >= 8192 use the 32-vector loop.
#include "common.h"
#include "mx4_common.h"
#pragma clang diagnostic ignored "-Wuninitialized"  // undef8: lanes whose contents do not matter
// the 32-vector loop from 4 members up
#ifndef BIG_MIN_UNITS
#define BIG_MIN_UNITS 4
#endif
#define BCAST_Q(x, q) v_u8_mov_dual_group_all_b((x), 0xFFFFFFFF, q, q, q, q, MkWrA(0b11, 0b11, 0b11, 0b11), (x))
#ifdef SLOTS
void main(tensor codes, tensor scales, tensor ids, tensor ktab, tensor out, int R, int u0)
#else
void main(tensor codes, tensor scales, tensor out, int k)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const uchar256 lane = read_lane_id_1b_b();
    const uchar256 e = v_u8_and_b(lane, MASK32);
    const uchar256 T0 = build_T0(e);
    const uchar256 T1 = build_T1(e);
    const uchar256 bf = build_basef2(e);
    const bool256 isf = v_u8_cmp_eq_b(v_u8_and_b(bf, (uchar256)0x20), (uchar256)0x20);
    // v = sat_add(base, d) with base = 0xE0 + E(m): every e' >= 31 lands on 0xFF, a saturated table entry.
    // k once per launch: bs = BS + k; bs in [0, 255]: d = sat_sub(S, bs); bs > 255: 255 (all values are zero);
    // bs < 0: -bs moves into base (saturating) and d = S.
#ifdef SLOTS
    uchar256 bsv = 0, base = 0;  // per row, from k = ktab[row]
#else
    const int bsi = BS + k;
    const int offi = bsi < 0 ? (-bsi < 31 ? -bsi : 31) : 0;
    const int bsc = bsi < 0 ? 0 : (bsi > 255 ? 255 : bsi);
    const uchar256 bsv = (uchar256)(unsigned char)bsc;
    const uchar256 base = v_u8_add_b(v_u8_or_b(v_u8_and_b(bf, (uchar256)0x1F), (uchar256)0xE0), (uchar256)(unsigned char)offi, SW_SAT);
#endif
    // code 1..7: clear the preset sign; code 9..15: keep it; codes 0 and 8: the output is 0x00 whatever T gives
    const uchar256 e15 = v_u8_and_b(e, (uchar256)15);
    const uchar256 mask = v_u8_sel_eq_u8_b(v_u8_and_b(e15, (uchar256)7), (uchar256)0, (uchar256)0,
                          v_u8_sel_grt_u8_b(e15, (uchar256)8, (uchar256)0xFF, (uchar256)0x7F));
    const uchar256 sel0 = v_u8_or_b(v_u8_and_b(lane, (uchar256)0x20), (uchar256)0x80);
    const uchar256 sel1 = v_u8_or_b(sel0, (uchar256)0x10);
    // scale index per lane: group g, half h: block 8 h + g of the pair
    const uchar256 idx0 = v_u8_add_b(v_u8_or_b(v_u8_shr_b(lane, (char256)5), (uchar256)0x80), v_u8_shr_b(v_u8_and_b(lane, (uchar256)16), (char256)1));
    const uchar256 idx1 = v_u8_add_b(idx0, (uchar256)16);
    const uchar256 idx2 = v_u8_add_b(idx1, (uchar256)16);
    const uchar256 idx3 = v_u8_add_b(idx2, (uchar256)16);
    uchar256 undef8;
#define TABLE(TB, AX, IDX) { const uchar256 vv = v_u8_add_b(base, v_u8_shuffle_b(AX, IDX, 0, undef8), SW_SAT); \
    uchar256 tb0_ = v_u8_shuffle_b(T0, vv, 0, undef8); \
    tb0_ = v_u8_shuffle_vb(T1, vv, 0, tb0_, isf); TB = v_u8_and_b(tb0_, mask); }
#define FINAL(TB, SL, PRED) v_u8_st_tnsr(oc, out, v_u8_shuffle_b(TB, SL, 0, undef8), 0, PRED, 0); oc[0] += 256;
#ifdef SLOTS
    scalar_cache_reset();
#endif
    for (int m = off[1]; m < end[1]; m++) {
#ifdef SLOTS
        int5 ic = {R ? m % R : m, R ? m / R : 0, 0, 0, 0};
        const int row = s_i32_ld_g(gen_addr(ic, ids));
        if (row < 0) continue;
        int5 kc = {row, 0, 0, 0, 0};
        const int k = s_i32_ld_g(gen_addr(kc, ktab));
        {
            const int bsi = BS + k;
            const int offi = bsi < 0 ? (-bsi < 31 ? -bsi : 31) : 0;
            const int bsc = bsi < 0 ? 0 : (bsi > 255 ? 255 : bsi);
            bsv = (uchar256)(unsigned char)bsc;
            base = v_u8_add_b(v_u8_or_b(v_u8_and_b(bf, (uchar256)0x1F), (uchar256)0xE0), (uchar256)(unsigned char)offi, SW_SAT);
        }
        int5 cc = {(off[0] + u0) * 1024, row, 0, 0, 0};
        int5 sc = {(off[0] + u0) * 64, row, 0, 0, 0};
#else
        int5 cc = {off[0] * 1024, m, 0, 0, 0};
        int5 sc = {off[0] * 64, m, 0, 0, 0};
#endif
        int v = off[0];

        if (end[0] - v >= BIG_MIN_UNITS) {
        int5 oc = {off[0] * 2048 - 3 * 512, m, 0, 0, 0};
        uchar256 R = v_u8_sub_b(v_u8_ld_tnsr_b(sc, scales), bsv, SW_SAT); sc[0] += 256;
        uchar256 Ac = BCAST_Q(R, 0);
        uchar256 ptb0 = undef8, pa0 = undef8, pb0 = undef8;
        uchar256 ptb1 = undef8, pa1 = undef8, pb1 = undef8;
        uchar256 ptb2 = undef8, pa2 = undef8, pb2 = undef8;
        const int v0 = v;
        for (; v + 4 <= end[0]; v += 4) {
            const bool stp = v != v0;
            const uchar256 Rn = v_u8_sub_b(v_u8_ld_tnsr_b(sc, scales), bsv, SW_SAT); sc[0] += 256;
            FINAL(ptb0, pa0, stp) FINAL(ptb0, pb0, stp)
            FINAL(ptb1, pa1, stp) FINAL(ptb1, pb1, stp)
            FINAL(ptb2, pa2, stp) FINAL(ptb2, pb2, stp)
            const uchar256 A1 = BCAST_Q(R, 1), A2 = BCAST_Q(R, 2), A3 = BCAST_Q(R, 3);
            uchar256 tb0; TABLE(tb0, Ac, idx0)
            uchar256 tb1; TABLE(tb1, Ac, idx1)
            uchar256 tb2; TABLE(tb2, Ac, idx2)
            uchar256 tb3; TABLE(tb3, Ac, idx3)
            uchar256 tb4; TABLE(tb4, A1, idx0)
            uchar256 tb5; TABLE(tb5, A1, idx1)
            uchar256 tb6; TABLE(tb6, A1, idx2)
            uchar256 tb7; TABLE(tb7, A1, idx3)
            uchar256 tb8; TABLE(tb8, A2, idx0)
            uchar256 tb9; TABLE(tb9, A2, idx1)
            uchar256 tb10; TABLE(tb10, A2, idx2)
            uchar256 tb11; TABLE(tb11, A2, idx3)
            uchar256 tb12; TABLE(tb12, A3, idx0)
            const uchar512 u0 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca0 = v_u8_or_b(u0.v1, sel0), cb0 = v_u8_or_b(u0.v2, sel1);
            const uchar512 u1 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca1 = v_u8_or_b(u1.v1, sel0), cb1 = v_u8_or_b(u1.v2, sel1);
            const uchar512 u2 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca2 = v_u8_or_b(u2.v1, sel0), cb2 = v_u8_or_b(u2.v2, sel1);
            const uchar512 u3 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca3 = v_u8_or_b(u3.v1, sel0), cb3 = v_u8_or_b(u3.v2, sel1);
            const uchar512 u4 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca4 = v_u8_or_b(u4.v1, sel0), cb4 = v_u8_or_b(u4.v2, sel1);
            const uchar512 u5 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca5 = v_u8_or_b(u5.v1, sel0), cb5 = v_u8_or_b(u5.v2, sel1);
            const uchar512 u6 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca6 = v_u8_or_b(u6.v1, sel0), cb6 = v_u8_or_b(u6.v2, sel1);
            const uchar512 u7 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca7 = v_u8_or_b(u7.v1, sel0), cb7 = v_u8_or_b(u7.v2, sel1);
            const uchar512 u8 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca8 = v_u8_or_b(u8.v1, sel0), cb8 = v_u8_or_b(u8.v2, sel1);
            const uchar512 u9 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca9 = v_u8_or_b(u9.v1, sel0), cb9 = v_u8_or_b(u9.v2, sel1);
            const uchar512 u10 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca10 = v_u8_or_b(u10.v1, sel0), cb10 = v_u8_or_b(u10.v2, sel1);
            const uchar512 u11 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca11 = v_u8_or_b(u11.v1, sel0), cb11 = v_u8_or_b(u11.v2, sel1);
            const uchar512 u12 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca12 = v_u8_or_b(u12.v1, sel0), cb12 = v_u8_or_b(u12.v2, sel1);
            const uchar512 u13 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            pa0 = v_u8_or_b(u13.v1, sel0); pb0 = v_u8_or_b(u13.v2, sel1);
            const uchar512 u14 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            pa1 = v_u8_or_b(u14.v1, sel0); pb1 = v_u8_or_b(u14.v2, sel1);
            const uchar512 u15 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            pa2 = v_u8_or_b(u15.v1, sel0); pb2 = v_u8_or_b(u15.v2, sel1);
            FINAL(tb0, ca0, 1) FINAL(tb0, cb0, 1)
            FINAL(tb1, ca1, 1) FINAL(tb1, cb1, 1)
            FINAL(tb2, ca2, 1) FINAL(tb2, cb2, 1)
            FINAL(tb3, ca3, 1) FINAL(tb3, cb3, 1)
            FINAL(tb4, ca4, 1) FINAL(tb4, cb4, 1)
            FINAL(tb5, ca5, 1) FINAL(tb5, cb5, 1)
            FINAL(tb6, ca6, 1) FINAL(tb6, cb6, 1)
            FINAL(tb7, ca7, 1) FINAL(tb7, cb7, 1)
            FINAL(tb8, ca8, 1) FINAL(tb8, cb8, 1)
            FINAL(tb9, ca9, 1) FINAL(tb9, cb9, 1)
            FINAL(tb10, ca10, 1) FINAL(tb10, cb10, 1)
            FINAL(tb11, ca11, 1) FINAL(tb11, cb11, 1)
            FINAL(tb12, ca12, 1) FINAL(tb12, cb12, 1)
            TABLE(ptb0, A3, idx1)
            TABLE(ptb1, A3, idx2)
            TABLE(ptb2, A3, idx3)
            R = Rn; Ac = BCAST_Q(Rn, 0);
        }
        FINAL(ptb0, pa0, 1) FINAL(ptb0, pb0, 1)
        FINAL(ptb1, pa1, 1) FINAL(ptb1, pb1, 1)
        FINAL(ptb2, pa2, 1) FINAL(ptb2, pb2, 1)
#ifdef SLOTS
        sc[0] = (v + u0) * 64;
#else
        sc[0] = v * 64;
#endif
        }
        if (v < end[0]) {
        int5 oc = {v * 2048 - 3 * 512, m, 0, 0, 0};
        uchar256 A = BCAST_DG0(v_u8_sub_b(v_u8_ld_tnsr_b(sc, scales), bsv, SW_SAT)); sc[0] += 64;
        uchar256 qtb0 = undef8, qa0 = undef8, qb0 = undef8;
        uchar256 qtb1 = undef8, qa1 = undef8, qb1 = undef8;
        uchar256 qtb2 = undef8, qa2 = undef8, qb2 = undef8;
        const int v1 = v;
        for (; v < end[0]; v++) {
            const bool stp = v != v1;
            const uchar256 An = BCAST_DG0(v_u8_sub_b(v_u8_ld_tnsr_b(sc, scales), bsv, SW_SAT)); sc[0] += 64;
            FINAL(qtb0, qa0, stp) FINAL(qtb0, qb0, stp)
            FINAL(qtb1, qa1, stp) FINAL(qtb1, qb1, stp)
            FINAL(qtb2, qa2, stp) FINAL(qtb2, qb2, stp)
            uchar256 tb0; TABLE(tb0, A, idx0)
            const uchar512 u0 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            const uchar256 ca0 = v_u8_or_b(u0.v1, sel0), cb0 = v_u8_or_b(u0.v2, sel1);
            const uchar512 u1 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            qa0 = v_u8_or_b(u1.v1, sel0); qb0 = v_u8_or_b(u1.v2, sel1);
            const uchar512 u2 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            qa1 = v_u8_or_b(u2.v1, sel0); qb1 = v_u8_or_b(u2.v2, sel1);
            const uchar512 u3 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cc, codes, SW_UNPACK | SW_UNPCK_4_TO_8)); cc[0] += 256;
            qa2 = v_u8_or_b(u3.v1, sel0); qb2 = v_u8_or_b(u3.v2, sel1);
            FINAL(tb0, ca0, 1) FINAL(tb0, cb0, 1)
            TABLE(qtb0, A, idx1)
            TABLE(qtb1, A, idx2)
            TABLE(qtb2, A, idx3)
            A = An;
        }
        FINAL(qtb0, qa0, 1) FINAL(qtb0, qb0, 1)
        FINAL(qtb1, qa1, 1) FINAL(qtb1, qb1, 1)
        FINAL(qtb2, qa2, 1) FINAL(qtb2, qb2, 1)
        }
    }
}
