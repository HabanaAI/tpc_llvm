// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DFlash 2 candidate path selector, greedy: for each row b, walk the block's draft columns k = 1 .. Tq - 1
// (column c = block b + k) choosing among the K candidates of the column the one with the largest
//   score_j = unary_j + <pred[prev] * hp[:, c], succ[cand_j]>
// prev: the row's anchor, then the previous column's choice. Writes next[:, b] = [anchor, choice_1 ..]
// (Tq rows: the anchor and the drafts the next step verifies, at most the block).
// tensors: 0 top_v (K, T) f32  1 top_i (K, T) i32  2 hp (R, T) f32  3 anchors (B) i32
//          4 pred (R, V) bf16  5 succ (R, V) bf16 | 6 next (Tq, B) i32
// params: block. R = 256, K <= 16. index space: [B]
#include "common.h"
#define NINF (-3.0e38f)
#define KMAX 16
__local__ float64 PR[KMAX * 4];  // the candidates' predecessor rows: the next column's choices
static inline void load_row(tensor tab, int id, float64 v[4])
{
    int5 c = {0, id, 0, 0, 0};
    const float128 lo = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, tab), SW_LINEAR);
    c[0] = 128;
    const float128 hi = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, tab), SW_LINEAR);
    v[0] = lo.v1;
    v[1] = lo.v2;
    v[2] = hi.v1;
    v[3] = hi.v2;
}
void main(tensor top_v, tensor top_i, tensor hp, tensor anchors, tensor pred, tensor succ, tensor next, int block)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(top_v, 0), Tq = get_dim_size(next, 0);
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++) {
        int5 ac = {b, 0, 0, 0, 0};
        const int anchor = s_i32_ld_g(gen_addr(ac, anchors));
        float64 pe[4];
        load_row(pred, anchor, pe);
        int5 nc = {0, b, 0, 0, 0};
        v_i32_st_tnsr_partial(nc, next, (int64)anchor, 0, 0);
        for (int k = 1; k < Tq; k++) {
            const int col = block * b + k;
            float64 w[4];
            #pragma unroll
            for (int i = 0; i < 4; i++) {
                int5 hc = {64 * i, col, 0, 0, 0};
                w[i] = pe[i] * v_f32_ld_tnsr_b(hc, hp);
            }
            int5 tc = {0, col, 0, 0, 0};
            const float64 unary = v_f32_sel_less_f32_b(lanef, (float)K, v_f32_ld_tnsr_b(tc, top_v), NINF);
            float64 score = NINF;
            int64 ids = 0;
            for (int j = 0; j < K; j++) {
                int5 ic = {j, col, 0, 0, 0};
                const int id = s_i32_ld_g(gen_addr(ic, top_i));
                float64 sr[4], pr[4];
                load_row(succ, id, sr);
                load_row(pred, id, pr);
                #pragma unroll
                for (int i = 0; i < 4; i++)
                    PR[4 * j + i] = pr[i];
                float64 dot = sr[0] * w[0];
                dot = v_f32_mac_b(sr[1], w[1], dot);
                dot = v_f32_mac_b(sr[2], w[2], dot);
                dot = v_f32_mac_b(sr[3], w[3], dot);
                float64 s;
                NL_REDUCE_ADD(dot, IDX, s);
                score = v_f32_sel_eq_f32_b(lanef, (float)j, s, score);
                ids = v_i32_sel_eq_i32_b(lane, (int64)j, (int64)id, ids);
            }
            score = v_f32_sel_less_f32_b(lanef, (float)K, score + unary, NINF);
            float64 m;
            NL_REDUCE_MAX(score, IDX, m);
            float64 first;  // the lowest lane holding the maximum
            NL_REDUCE_(v_f32_sel_eq_f32_b(score, m, lanef, 3.0e38f), IDX, first, v_f32_min_b);
            int64 pick;
            NL_REDUCE_I32(v_i32_sel_eq_f32_b(lanef, first, ids, (int64)0), IDX, pick, v_i32_add_b);
            nc[0] = k;
            v_i32_st_tnsr_partial(nc, next, pick, 0, 0);
            #pragma unroll
            for (int i = 0; i < 4; i++) {
                float64 acc = 0.0f;
                for (int j = 0; j < K; j++)
                    acc = v_f32_sel_eq_f32_b(first, (float)j, PR[4 * j + i], acc);
                pe[i] = acc;
            }
        }
    }
}
