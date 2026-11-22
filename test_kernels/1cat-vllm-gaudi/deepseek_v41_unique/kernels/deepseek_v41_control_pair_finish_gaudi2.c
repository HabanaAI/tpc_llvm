// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
void main(tensor product,tensor scales,tensor channels,tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int rows=get_dim_size(output,1);
    const float64 wh=v_f32_ld_tnsr_partial_b((int5){0,0},channels,23,0);
    const float64 wl=v_f32_ld_tnsr_partial_b((int5){24,0},channels,23,0);
    for(int row=begin[0];row<end[0];++row) {
        const float sh=s_f32_ld_g(gen_addr((int5){0,row},scales));
        const float sl=s_f32_ld_g(gen_addr((int5){0,row+rows},scales));
        float64 value=(v_f32_ld_tnsr_partial_b((int5){0,row},product,23,0)*wh)*sh;
        value+=(v_f32_ld_tnsr_partial_b((int5){24,row},product,23,0)*wl)*sh;
        value+=(v_f32_ld_tnsr_partial_b((int5){0,row+rows},product,23,0)*wh)*sl;
        value+=(v_f32_ld_tnsr_partial_b((int5){24,row+rows},product,23,0)*wl)*sl;
        v_f32_st_tnsr_partial((int5){0,row},output,value,23,0);
    }
}
