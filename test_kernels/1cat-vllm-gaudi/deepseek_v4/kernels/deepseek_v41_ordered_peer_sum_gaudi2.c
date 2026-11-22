// SPDX-License-Identifier: Apache-2.0
// Fixed-rank FP32 addition followed by exactly one BF16 rounding.
void main(tensor shards, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int ranks = get_dim_size(shards, 1);
    for (int block = begin[0]; block < end[0]; ++block) {
        const int offset = block * 128;
        const bfloat128 first = v_bf16_ld_tnsr_b((int5){offset, 0}, shards);
        float128 total = v_convert_bf16_to_f32_all_b(first);
        for (int rank = 1; rank < ranks; ++rank) {
            const bfloat128 packed = v_bf16_ld_tnsr_b((int5){offset, rank}, shards);
            const float128 value = v_convert_bf16_to_f32_all_b(packed);
            total.v1 = total.v1 + value.v1;
            total.v2 = total.v2 + value.v2;
        }
        v_bf16_st_tnsr((int5){offset, 0}, output, v_convert_f32_to_bf16_all_b(total, SW_RHNE));
    }
}
