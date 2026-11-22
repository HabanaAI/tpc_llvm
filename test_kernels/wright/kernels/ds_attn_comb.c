// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 attention over two key sets in one softmax, with the per-head sink: each part gives its normalized
// output o, max m and sum l (ds_slot_ml); per column c (head h = c % hpg)
//   M = max(m1, m2, sink[h]),  a_i = l_i exp(m_i - M),  out = (a_1 o_1 + a_2 o_2) / (a_1 + a_2 + exp(sink[h] - M))
// tensors: 0 o1 (D, W, G) f32  1 m1 (W, G)  2 l1 (W, G)  3 o2 (D, W, G)  4 m2  5 l2  6 sink (hpg) f32
//          | 7 out (D, W, G) f32
// ROPE builds (ds_attn_comb_r.c): then RoPE with tensor 7 cs (128, T) (column c is token c / hpg) on the last 64
// rows, as ml4_rope (e.g. the inverse rotation); out is tensor 8.
// params: hpg. index space: [W, G]
#include "common.h"

#ifdef ROPE
void main(tensor o1, tensor m1, tensor l1, tensor o2, tensor m2, tensor l2, tensor sink, tensor cs, tensor out, int hpg)
#else
void main(tensor o1, tensor m1, tensor l1, tensor o2, tensor m2, tensor l2, tensor sink, tensor out, int hpg)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int D = get_dim_size(out, 0);
    scalar_cache_reset();
#ifdef ROPE
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e = (lane ^ 1) & 15;                                   // pair partner within the dual group
    const int64 ctl = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    const uchar256 SW = __builtin_bit_cast(uchar256, ctl);
#endif
    for (int g = off[1]; g < end[1]; g++) {
        for (int c = off[0]; c < end[0]; c++) {
            int5 mc = {c, g, 0, 0, 0}, hc = {c % hpg, 0, 0, 0, 0};
            const float ma = s_f32_ld_g(gen_addr(mc, m1)), la = s_f32_ld_g(gen_addr(mc, l1));
            const float mb = s_f32_ld_g(gen_addr(mc, m2)), lb = s_f32_ld_g(gen_addr(mc, l2));
            const float sk = s_f32_ld_g(gen_addr(hc, sink));
            float M = ma > mb ? ma : mb;
            M = M > sk ? M : sk;
            const float64 a1 = (float64)la * nl_exp((float64)(ma - M));
            const float64 a2 = (float64)lb * nl_exp((float64)(mb - M));
            const float64 inv = nl_recip(a1 + a2 + nl_exp((float64)(sk - M)));
            const float64 w1 = a1 * inv, w2 = a2 * inv;
#ifdef ROPE
            int5 tc = {0, c / hpg, 0, 0, 0};
            const float64 co = v_f32_ld_tnsr_b(tc, cs);
            tc[0] = 64;
            const float64 si = v_f32_ld_tnsr_b(tc, cs);
#endif
            for (int d = 0; d < D; d += 64) {
                int5 oc = {d, c, g, 0, 0};
                float64 v = v_f32_ld_tnsr_b(oc, o1) * w1 + v_f32_ld_tnsr_b(oc, o2) * w2;
#ifdef ROPE
                if (d == D - 64)
                    v = v_f32_mac_b(v_f32_shuffle_b(v, SW, 0, v), si, v * co);
#endif
                v_f32_st_tnsr(oc, out, v);
            }
        }
    }
}
