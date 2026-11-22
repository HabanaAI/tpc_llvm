// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: finalize block outputs (delta_out = dequant(raw), res_out = res_in, optional feat_out = res_in + delta_out).
// Tensors (dim 0 fastest):
//   0 raw       BF16 [H, R]  unscaled down MME output in SRAM
//   1 sa        F32  [1, R]  row activation scale in SRAM
//   2 swb       F32  [H]     pre-interleaved weight scales [even 64, odd 64] per 128 cols
//   3 res_in    BF16 [H, R]  residual in SRAM
//   4 res_out   BF16 [H, R]  residual out in HBM
//   5 delta_out BF16 [H, R]  dequantized delta out in HBM
//   6 feat_out  BF16 [H, R]  optional captured feature (res_out + delta_out) in HBM
// Scalar SRF 0: int write_feat (0 or 1).
// Index space: dim 1 = row [0..R).

#define FIN_TILE(TIDX) do {                                                                 \
    const int col_ = (TIDX) * 128;                                                          \
    int5 c_ = {col_, row, 0, 0, 0};                                                         \
    int5 wc_ = {col_, 0, 0, 0, 0};                                                          \
    float64_pair_t r_ = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c_, raw));             \
    const float64 w1_ = v_f32_ld_tnsr_b(wc_, swb);                                          \
    wc_[0] = col_ + 64;                                                                     \
    const float64 w2_ = v_f32_ld_tnsr_b(wc_, swb);                                          \
    r_.v1 = r_.v1 * (w1_ * a);                                                              \
    r_.v2 = r_.v2 * (w2_ * a);                                                              \
    const bfloat128 d_ = v_convert_f32_to_bf16_all_b(r_);                                   \
    const bfloat128 s_ = v_bf16_ld_tnsr_b(c_, res_in);                                      \
    v_bf16_st_tnsr(c_, delta_out, d_);                                                      \
    v_bf16_st_tnsr(c_, res_out, s_);                                                        \
    if (write_feat) v_bf16_st_tnsr(c_, feat_out, s_ + d_);                                  \
} while (0)

void main(tensor raw, tensor sa, tensor swb, tensor res_in, tensor res_out, tensor delta_out,
          tensor feat_out, int write_feat) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tiles = get_dim_size(raw, 0) / 128;
    for (int row = start[1]; row < end[1]; ++row) {
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        for (int tile = 0; tile < tiles; tile += 4) {
            FIN_TILE(tile);
            FIN_TILE(tile + 1);
            FIN_TILE(tile + 2);
            FIN_TILE(tile + 3);
        }
    }
}
