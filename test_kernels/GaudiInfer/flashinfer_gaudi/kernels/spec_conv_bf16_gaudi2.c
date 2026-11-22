// SPDX-License-Identifier: Apache-2.0
// Speculative-decode causal conv (width 4) for DFlash/DSpine verification, one launch per GDN layer:
//   history = pool[idx_b, off_b .. off_b + 2] (off_b = clamp(accepted_b, 1, L - 2) - 1, the committed window)
//   y[b*T + t] = silu(sum_tap seq[t + tap] * w[tap]) with seq = [history; x_b] (FP32 math, BF16 out)
//   state[b] = [history[1], history[2], x_b[0 .. T-1]]   (L = T + 2 rows, written to a separate output)
// Same as vllm_gaudi/ops/causal_conv1d_pytorch._hpu_causal_conv1d_spec_update with full_query_valid_state,
// no bias, SiLU, no rounding before the activation.
// Tensors (dim 0 fastest): x [C, B*T] bf16, pool [C, L, S] bf16, w [C, 4] bf16, idx [B] i32, acc [B] i32,
// y [C, B*T] bf16, state [C, L, B] bf16. Index space [C / 128, B].
void main(tensor x, tensor pool, tensor w, tensor idx, tensor acc, tensor y, tensor state)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int rows = get_dim_size(state, 1);
    const int tokens = rows - 2;
    for (int b = start[1]; b < end[1]; ++b) {
        int5 ic = {b, 0, 0, 0, 0};
        const int slot = s_i32_ld_g((__global__ int*)gen_addr(ic, idx));
        int off = s_i32_ld_g((__global__ int*)gen_addr(ic, acc));
        off = off < 1 ? 1 : (off > tokens ? tokens : off);
        off -= 1;
        for (int tile = start[0]; tile < end[0]; ++tile) {
            const int c = tile * 128;
            int5 wc = {c, 0, 0, 0, 0};
            const float64_pair_t w0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));
            wc[1] = 1;
            const float64_pair_t w1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));
            wc[1] = 2;
            const float64_pair_t w2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));
            wc[1] = 3;
            const float64_pair_t w3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));
            int5 pc = {c, off, slot, 0, 0};
            const bfloat128 h0 = v_bf16_ld_tnsr_b(pc, pool);
            pc[1] = off + 1;
            const bfloat128 h1 = v_bf16_ld_tnsr_b(pc, pool);
            pc[1] = off + 2;
            const bfloat128 h2 = v_bf16_ld_tnsr_b(pc, pool);
            int5 sc = {c, 0, b, 0, 0};
            v_bf16_st_tnsr(sc, state, h1);
            sc[1] = 1;
            v_bf16_st_tnsr(sc, state, h2);
            float64_pair_t s0 = v_convert_bf16_to_f32_all_b(h0);
            float64_pair_t s1 = v_convert_bf16_to_f32_all_b(h1);
            float64_pair_t s2 = v_convert_bf16_to_f32_all_b(h2);
            int5 xc = {c, b * tokens, 0, 0, 0};
            bfloat128 next = v_bf16_ld_tnsr_b(xc, x);
            for (int t = 0; t < tokens; ++t) {
                const bfloat128 xv = next;
                xc[1] = b * tokens + (t + 1 < tokens ? t + 1 : t);
                next = v_bf16_ld_tnsr_b(xc, x);
                sc[1] = 2 + t;
                v_bf16_st_tnsr(sc, state, xv);
                const float64_pair_t s3 = v_convert_bf16_to_f32_all_b(xv);
                float64_pair_t o;
                o.v1 = s0.v1 * w0.v1;
                o.v1 = o.v1 + s1.v1 * w1.v1;
                o.v1 = o.v1 + s2.v1 * w2.v1;
                o.v1 = o.v1 + s3.v1 * w3.v1;
                o.v2 = s0.v2 * w0.v2;
                o.v2 = o.v2 + s1.v2 * w1.v2;
                o.v2 = o.v2 + s2.v2 * w2.v2;
                o.v2 = o.v2 + s3.v2 * w3.v2;
                o.v1 = o.v1 * v_sigmoid_f32(o.v1);
                o.v2 = o.v2 * v_sigmoid_f32(o.v2);
                int5 yc = {c, b * tokens + t, 0, 0, 0};
                v_bf16_st_tnsr(yc, y, v_convert_f32_to_bf16_all_b(o));
                s0 = s1; s1 = s2; s2 = s3;
            }
        }
    }
}
