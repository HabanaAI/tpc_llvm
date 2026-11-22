// SPDX-License-Identifier: Apache-2.0
// g2e Direct MME+SRAM Q transpose/pack helper:
//   q_in  BF16 [256, 24, 16, 8]  (Synapse apre_q_ layout: [B=8, T=16, NH=24, D=256])
//   q_out BF16/F8 [256, 96, 4, 8] (DirectAttnCore SRAM layout: [B=8, KV=4, G=6, T=16, D=256])
// Index space: dim 1 = row in [0..384) (for each row, processes all 8 slots -> 24 TPCs x 16 rows).

#ifdef OUT_FP8
static inline minifloat256 f32_pair_to_f8(float64 v1, float64 v2) {
    minifloat256 packed = 0;
    packed = v_convert_f32_to_f8_b(v1, 0, SW_CLIP_FP, packed);
    packed = v_convert_f32_to_f8_b(v2, 2, SW_CLIP_FP, packed);
    const minifloat256 sparse = packed;
    packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
    return v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
}
#endif

void main(tensor q_in, tensor q_out) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int B = get_dim_size(q_out, 3);
    for (int row = start[1]; row < end[1]; ++row) {
        const int h = row / 96;
        const int rem = row - h * 96;
        const int g = rem >> 4;
        const int t = rem & 15;
        const int nh = h * 6 + g;
        for (int b = 0; b < B; ++b) {
            int5 ic0 = {0, nh, t, b, 0}, ic1 = {128, nh, t, b, 0};
            const bfloat128 x0 = v_bf16_ld_tnsr_b(ic0, q_in);
            const bfloat128 x1 = v_bf16_ld_tnsr_b(ic1, q_in);
            int5 oc0 = {0, rem, h, b, 0};
#ifdef OUT_FP8
            const float128 f0 = v_convert_bf16_to_f32_all_b(x0);
            const float128 f1 = v_convert_bf16_to_f32_all_b(x1);
            const minifloat256 q0 = f32_pair_to_f8(f0.v1, f0.v2);
            const minifloat256 q1 = f32_pair_to_f8(f1.v1, f1.v2);
            minifloat256 pk = v_f8_mov_dual_group_b(q1, 0xffffffff, 0, 2, MkWr(1, 1), q0);
            pk = v_f8_mov_dual_group_b(q1, 0xffffffff, 1, 3, MkWr(1, 1), pk);
            v_f8_st_tnsr(oc0, q_out, pk);
#else
            int5 oc1 = {128, rem, h, b, 0};
            v_bf16_st_tnsr(oc0, q_out, x0);
            v_bf16_st_tnsr(oc1, q_out, x1);
#endif
        }
    }
}
