// SPDX-License-Identifier: Apache-2.0
// Independent128-element BF16 tiles share the square/max input scan.
#pragma clang fp contract(off)
void main(tensor input, tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[1];row<end[1];++row)for(int tile=begin[0];tile<end[0];++tile) {
        const float128 x=v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){tile*128,row},input));
        const float64 squares=x.v1*x.v1+x.v2*x.v2;
        const float64 maximum=v_f32_max_b(v_f32_abs_b(x.v1),v_f32_abs_b(x.v2));
        v_f32_st_tnsr_partial((int5){tile,0,row},output,v_f32_reduce_add(squares),0,0);
        v_f32_st_tnsr_partial((int5){tile,1,row},output,v_f32_reduce_max(maximum),0,0);
    }
}
