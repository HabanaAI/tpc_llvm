// SPDX-License-Identifier: Apache-2.0
void main(tensor swa,tensor main_result,tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int token=begin[2];token<end[2];++token)
        for(int head=begin[1];head<end[1];++head)
            for(int tile=begin[0];tile<end[0];++tile) {
                const int offset=tile*128;
                float128 sum;
                sum.v1=(v_f32_ld_tnsr_b((int5){offset,head*2,token},swa)+
                        v_f32_ld_tnsr_b((int5){offset,head*2+1,token},swa))+
                        v_f32_ld_tnsr_b((int5){offset,head,token},main_result);
                sum.v2=(v_f32_ld_tnsr_b((int5){offset+64,head*2,token},swa)+
                        v_f32_ld_tnsr_b((int5){offset+64,head*2+1,token},swa))+
                        v_f32_ld_tnsr_b((int5){offset+64,head,token},main_result);
                v_bf16_st_tnsr((int5){offset,head,token},output,
                    convert_float128_to_bfloat128(sum,SW_RHNE|SW_LINEAR));
            }
}
