// SPDX-License-Identifier: Apache-2.0
// DSpine fused Code Bridge Stage D (100% LUT-free, 4-way VPU-interleaved):
//   if (row & 15) == 0: out2 = out
//   else:               out2 = bf16(f32(out) + f32(delta) * rms[row])
//   x_in = bf16((f32(out2) * rsqrt(mean(out2^2) + eps)) * f32(next_w))
// Tensors (dim 0 fastest):
//   0 out    BF16 [H, BT]
//   1 delta  BF16 [H, BT]
//   2 rms    F32  [1, BT]
//   3 next_w BF16 [H]
//   4 out2   BF16 [H, BT]
//   5 x_in   BF16 [H, BT]
// Index space: dim 0 = row [0..BT).

#include "norm_math_gaudi2.h"

void main(tensor out, tensor delta, tensor rms_in, tensor next_w, tensor out2, tensor x_in,
          float inv_h, float eps) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(out, 0);
    const int tiles = width / 128;  // H=5120 -> 40 (multiple of 4)
    bfloat128 cached[64];

    for (int row = start[0]; row < end[0]; ++row) {
        const int is_anchor = ((row & 15) == 0);
        float128 sq0 = {0}, sq1 = {0}, sq2 = {0}, sq3 = {0};

        if (is_anchor) {
            for (int tile = 0; tile < tiles; tile += 4) {
                const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
                int5 ic0 = {c0, row, 0, 0, 0}, ic1 = {c1, row, 0, 0, 0};
                int5 ic2 = {c2, row, 0, 0, 0}, ic3 = {c3, row, 0, 0, 0};

                const bfloat128 o0 = v_bf16_ld_tnsr_b(ic0, out);
                const bfloat128 o1 = v_bf16_ld_tnsr_b(ic1, out);
                const bfloat128 o2 = v_bf16_ld_tnsr_b(ic2, out);
                const bfloat128 o3 = v_bf16_ld_tnsr_b(ic3, out);

                v_bf16_st_tnsr(ic0, out2, o0);
                v_bf16_st_tnsr(ic1, out2, o1);
                v_bf16_st_tnsr(ic2, out2, o2);
                v_bf16_st_tnsr(ic3, out2, o3);

                cached[tile]     = o0;
                cached[tile + 1] = o1;
                cached[tile + 2] = o2;
                cached[tile + 3] = o3;

                sq0 = v_bf16_mac_acc32_b(o0, o0, sq0, (e_no_negation) << 1);
                sq1 = v_bf16_mac_acc32_b(o1, o1, sq1, (e_no_negation) << 1);
                sq2 = v_bf16_mac_acc32_b(o2, o2, sq2, (e_no_negation) << 1);
                sq3 = v_bf16_mac_acc32_b(o3, o3, sq3, (e_no_negation) << 1);
            }
        } else {
            int5 sc = {0, row, 0, 0, 0};
            const float64 r = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, rms_in), 0);

            for (int tile = 0; tile < tiles; tile += 4) {
                const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
                int5 ic0 = {c0, row, 0, 0, 0}, ic1 = {c1, row, 0, 0, 0};
                int5 ic2 = {c2, row, 0, 0, 0}, ic3 = {c3, row, 0, 0, 0};

                const float64_pair_t f0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic0, out));
                const float64_pair_t f1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic1, out));
                const float64_pair_t f2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic2, out));
                const float64_pair_t f3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic3, out));

                const float64_pair_t d0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic0, delta));
                const float64_pair_t d1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic1, delta));
                const float64_pair_t d2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic2, delta));
                const float64_pair_t d3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic3, delta));

                float64_pair_t u0 = {f0.v1 + d0.v1 * r, f0.v2 + d0.v2 * r};
                float64_pair_t u1 = {f1.v1 + d1.v1 * r, f1.v2 + d1.v2 * r};
                float64_pair_t u2 = {f2.v1 + d2.v1 * r, f2.v2 + d2.v2 * r};
                float64_pair_t u3 = {f3.v1 + d3.v1 * r, f3.v2 + d3.v2 * r};

                const bfloat128 o0 = v_convert_f32_to_bf16_all_b(u0);
                const bfloat128 o1 = v_convert_f32_to_bf16_all_b(u1);
                const bfloat128 o2 = v_convert_f32_to_bf16_all_b(u2);
                const bfloat128 o3 = v_convert_f32_to_bf16_all_b(u3);

                v_bf16_st_tnsr(ic0, out2, o0);
                v_bf16_st_tnsr(ic1, out2, o1);
                v_bf16_st_tnsr(ic2, out2, o2);
                v_bf16_st_tnsr(ic3, out2, o3);

                cached[tile]     = o0;
                cached[tile + 1] = o1;
                cached[tile + 2] = o2;
                cached[tile + 3] = o3;

                sq0 = v_bf16_mac_acc32_b(o0, o0, sq0, (e_no_negation) << 1);
                sq1 = v_bf16_mac_acc32_b(o1, o1, sq1, (e_no_negation) << 1);
                sq2 = v_bf16_mac_acc32_b(o2, o2, sq2, (e_no_negation) << 1);
                sq3 = v_bf16_mac_acc32_b(o3, o3, sq3, (e_no_negation) << 1);
            }
        }

        const float64 sum_all = ((sq0.v1 + sq1.v1) + (sq2.v1 + sq3.v1)) +
                                ((sq0.v2 + sq1.v2) + (sq2.v2 + sq3.v2));
        const float64 rrms = positive_rsqrt(row_sum(sum_all) * inv_h + eps);

        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 wc0 = {c0, 0, 0, 0, 0}, wc1 = {c1, 0, 0, 0, 0};
            int5 wc2 = {c2, 0, 0, 0, 0}, wc3 = {c3, 0, 0, 0, 0};

            float64_pair_t v0 = v_convert_bf16_to_f32_all_b(cached[tile]);
            float64_pair_t v1 = v_convert_bf16_to_f32_all_b(cached[tile + 1]);
            float64_pair_t v2 = v_convert_bf16_to_f32_all_b(cached[tile + 2]);
            float64_pair_t v3 = v_convert_bf16_to_f32_all_b(cached[tile + 3]);

            const float64_pair_t w0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc0, next_w));
            const float64_pair_t w1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc1, next_w));
            const float64_pair_t w2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc2, next_w));
            const float64_pair_t w3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc3, next_w));

            v0.v1 = (v0.v1 * rrms) * w0.v1; v0.v2 = (v0.v2 * rrms) * w0.v2;
            v1.v1 = (v1.v1 * rrms) * w1.v1; v1.v2 = (v1.v2 * rrms) * w1.v2;
            v2.v1 = (v2.v1 * rrms) * w2.v1; v2.v2 = (v2.v2 * rrms) * w2.v2;
            v3.v1 = (v3.v1 * rrms) * w3.v1; v3.v2 = (v3.v2 * rrms) * w3.v2;

            int5 oc0 = {c0, row, 0, 0, 0}, oc1 = {c1, row, 0, 0, 0};
            int5 oc2 = {c2, row, 0, 0, 0}, oc3 = {c3, row, 0, 0, 0};
            v_bf16_st_tnsr(oc0, x_in, v_convert_f32_to_bf16_all_b(v0));
            v_bf16_st_tnsr(oc1, x_in, v_convert_f32_to_bf16_all_b(v1));
            v_bf16_st_tnsr(oc2, x_in, v_convert_f32_to_bf16_all_b(v2));
            v_bf16_st_tnsr(oc3, x_in, v_convert_f32_to_bf16_all_b(v3));
        }
    }
}
