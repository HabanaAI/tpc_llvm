// SPDX-License-Identifier: Apache-2.0
// Preserve C1's normalize -> BF16 -> inverse RoPE -> BF16 boundaries in one node.
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_INVERSE 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "../../deepseek_v4/include/deepseek_v4_qnorm_rope_kv_pack_bf16.h"

void main(tensor product,
#ifdef DSV41_STREAM_EXP_SPLIT
          tensor second_product,
#endif
          tensor inverse_denominator,tensor positions,tensor phase,tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int token=begin[2];token<end[2];++token) {
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        for(int head=begin[1];head<end[1];++head) {
            const float inverse=s_f32_ld_g(gen_addr((int5){0,head,token},inverse_denominator));
            for(int tile=begin[0];tile<end[0];++tile) {
                const int n=tile*128;
                float128 values;
                values.v1=v_f32_ld_tnsr_b((int5){n,head,token},product);
                values.v2=v_f32_ld_tnsr_b((int5){n+64,head,token},product);
#ifdef DSV41_STREAM_EXP_SPLIT
                values.v1+=v_f32_ld_tnsr_b((int5){n,head,token},second_product);
                values.v2+=v_f32_ld_tnsr_b((int5){n+64,head,token},second_product);
#endif
                values.v1*=inverse;values.v2*=inverse;
                const bfloat128 rounded=convert_float128_to_bfloat128(values,SW_RHNE|SW_LINEAR);
                if(n==384) {
                    float128 rotated=convert_bfloat128_to_float128(rounded,SW_LINEAR);
                    rotated.v2=dsv4_qkv_apply_pairwise_rope_f32(rotated.v2,phase,position);
                    v_bf16_st_tnsr((int5){n,head,token},output,
                        convert_float128_to_bfloat128(rotated,SW_LINEAR));
                } else v_bf16_st_tnsr((int5){n,head,token},output,rounded);
            }
        }
    }
}
