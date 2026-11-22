// SPDX-License-Identifier: Apache-2.0
// Sequential head sums match the C1 index scorer, with 128 candidates per SIMD.
static inline float128 bf16_round(float128 value) {
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(value, SW_RHNE));
}

void main(tensor dots, tensor weights, tensor scores) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    for(int request=begin[1];request<end[1];++request) {
        for(int tile=begin[0];tile<end[0];++tile) {
            float128 partial[2]={{0,0},{0,0}};
            for(int shard=0;shard<2;++shard) {
                for(int head=0;head<16;++head) {
                    const int h=shard*16+head;
                    const bfloat128 dot=v_bf16_ld_tnsr_b((int5){tile*128,h,request},dots);
                    const float128 fp=v_convert_bf16_to_f32_all_b(dot);
                    const float w=s_convert_bf16_to_f32(s_bf16_ld_g(gen_addr((int5){h,request},weights)),0);
                    const float128 product=bf16_round((float128){v_f32_max_b(fp.v1,0.f)*w,
                                                               v_f32_max_b(fp.v2,0.f)*w});
                    partial[shard].v1+=product.v1;
                    partial[shard].v2+=product.v2;
                }
                partial[shard]=bf16_round(partial[shard]);
            }
            // Native all-lane conversion splits even/odd BF16 lanes. Restore
            // contiguous candidate order before the two FP32 tensor stores.
            const bfloat128 rounded=v_convert_f32_to_bf16_all_b(
                (float128){partial[0].v1+partial[1].v1,partial[0].v2+partial[1].v2},SW_RHNE);
            const float128 result=convert_bfloat128_to_float128(rounded,SW_LINEAR);
            v_f32_st_tnsr((int5){tile*128,request},scores,result.v1);
            v_f32_st_tnsr((int5){tile*128+64,request},scores,result.v2);
        }
    }
}
