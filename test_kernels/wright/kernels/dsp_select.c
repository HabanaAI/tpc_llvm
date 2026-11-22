// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSpine serial refinement, greedy: for each row b, walk the block's draft columns k = 1 .. Tq - 1
// (column c = block b + k) choosing among the K candidates of the column the one whose logit is largest
// after the column's last code write is redone from the exact code of the token chosen before it (the
// row's anchor, then the previous choice):
//   score_j = rms_o[c] / ws[c] * logit_j + <tg[cand_j], m - mp>
//   m  = sigmoid(q + tsv_g[prev] + bias) * tsv_v[prev],  mp = sigmoid(q + s[c - 1] + bias) * v[c - 1]
// (q, s, v: the columns' packed rows; mp the message the forward pass wrote; rms_o the final norm's RMS of
// x[:, c], ws[c] the write's scale; dividing by ws keeps the argmax). Writes next[:, b] = [anchor,
// choice_1 ..] (Tq rows: the anchor and the drafts the next step verifies, at most the block).
// tensors: 0 top_v (K, T) f32  1 top_i (K, T) i32  2 x (H, T) f32  3 ws (1, T) f32  4 pk (3R, T) f32
//          5 bias (R) f32  6 anchors (B) i32  7 tg (R, V) bf16  8 tsv (2R, V) bf16 [gate | value]
//        | 9 next (Tq, B) i32
// tg[v] = (head[v] * norm) . write_last, tsv[v] = [predecessor_gate | prev] . code(v) (natural order).
// params: block, eps. R = 512, K <= 16. index space: [B]
#include "common.h"
#define NINF (-3.0e38f)
#define RV 8   // R / 64
static inline void load_rows(tensor tab, int id, int c0, float64 v[RV])
{
    int5 c = {c0, id, 0, 0, 0};
    #pragma unroll
    for (int i = 0; i < RV / 2; i++) {
        c[0] = c0 + 128 * i;
        const float128 w = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, tab), SW_LINEAR);
        v[2 * i] = w.v1;
        v[2 * i + 1] = w.v2;
    }
}
__local__ float64 ML[16 * RV];  // per candidate: the next column's redone message from its exact code

// m = sigmoid(q + tsv_g[id] + bias) * tsv_v[id] with qb = q + bias
static inline void message(tensor tsv, int id, const float64 qb[RV], float64 m[RV])
{
    float64 g[RV], val[RV];
    load_rows(tsv, id, 0, g);
    load_rows(tsv, id, 64 * RV, val);
    #pragma unroll
    for (int i = 0; i < RV; i++)
        m[i] = nl_sigm(qb[i] + g[i]) * val[i];
}

static inline void query(tensor pk, int col, const float64 gb[RV], float64 qb[RV])
{
    int5 qc = {0, col, 0, 0, 0};
    #pragma unroll
    for (int i = 0; i < RV; i++) {
        qc[0] = 64 * i;
        qb[i] = v_f32_ld_tnsr_b(qc, pk) + gb[i];
    }
}

void main(tensor top_v, tensor top_i, tensor x, tensor ws, tensor pk, tensor bias, tensor anchors,
          tensor tg, tensor tsv, tensor next, int block, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(top_v, 0), Tq = get_dim_size(next, 0);
    const int H = get_dim_size(x, 0);
    const int R = 64 * RV;
    const float inv_h = 1.0f / (float)H;
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    float64 gb[RV];
    {
        int5 bc = {0, 0, 0, 0, 0};
        #pragma unroll
        for (int i = 0; i < RV; i++) {
            bc[0] = 64 * i;
            gb[i] = v_f32_ld_tnsr_b(bc, bias);
        }
    }
    for (int b = off[0]; b < end[0]; b++) {
        int5 ac = {b, 0, 0, 0, 0};
        const int anchor = s_i32_ld_g(gen_addr(ac, anchors));
        int5 nc = {0, b, 0, 0, 0};
        v_i32_st_tnsr_partial(nc, next, (int64)anchor, 0, 0);
        float64 qb[RV], cur[RV];
        query(pk, block * b + 1, gb, qb);
        message(tsv, anchor, qb, cur);  // column 1's sender: the anchor
        for (int k = 1; k < Tq; k++) {
            const int col = block * b + k;
            // the final norm's RMS over the write's scale
            int5 c = {0, col, 0, 0, 0};
            float64 acc = 0.0f;
            for (int i = 0; i < H; i += 64) {
                c[0] = i;
                const float64 v = v_f32_ld_tnsr_b(c, x);
                acc = v_f32_mac_b(v, v, acc);
            }
            float64 ss;
            NL_REDUCE_ADD(acc, IDX, ss);
            const float64 ms = ss * inv_h + eps;
            const float64 ratio = ms * nl_rsqrt(ms) * nl_recip(v_f32_max_b(TOKEN_SCALE(ws, col, IDX[0]), 1e-30f));
            // the redone write's message less the forward pass's (qb: this column's query)
            float64 dm[RV];
            int5 pc = {0, col - 1, 0, 0, 0};
            #pragma unroll
            for (int i = 0; i < RV; i++) {
                pc[0] = R + 64 * i;
                const float64 s = v_f32_ld_tnsr_b(pc, pk);
                pc[0] = 2 * R + 64 * i;
                dm[i] = cur[i] - nl_sigm(qb[i] + s) * v_f32_ld_tnsr_b(pc, pk);
            }
            const bool more = k + 1 < Tq;
            if (more)
                query(pk, col + 1, gb, qb);
            int5 tc = {0, col, 0, 0, 0};
            const float64 unary = v_f32_ld_tnsr_b(tc, top_v) * ratio;
            float64 score = NINF;
            int64 ids = 0;
            for (int j = 0; j < K; j++) {
                int5 ic = {j, col, 0, 0, 0};
                const int id = s_i32_ld_g(gen_addr(ic, top_i));
                float64 t[RV];
                load_rows(tg, id, 0, t);
                float64 dot = t[0] * dm[0];
                #pragma unroll
                for (int i = 1; i < RV; i++)
                    dot = v_f32_mac_b(t[i], dm[i], dot);
                float64 s;
                NL_REDUCE_ADD(dot, IDX, s);
                score = v_f32_sel_eq_f32_b(lanef, (float)j, s, score);
                ids = v_i32_sel_eq_i32_b(lane, (int64)j, (int64)id, ids);
                if (more) {
                    float64 mj[RV];
                    message(tsv, id, qb, mj);
                    #pragma unroll
                    for (int i = 0; i < RV; i++)
                        ML[RV * j + i] = mj[i];
                }
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
            if (more) {
                #pragma unroll
                for (int i = 0; i < RV; i++) {
                    float64 a = 0.0f;
                    for (int j = 0; j < K; j++)
                        a = v_f32_sel_eq_f32_b(first, (float)j, ML[RV * j + i], a);
                    cur[i] = a;
                }
            }
        }
    }
}
