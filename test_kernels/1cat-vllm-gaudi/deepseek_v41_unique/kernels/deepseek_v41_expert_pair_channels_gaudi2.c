// SPDX-License-Identifier: Apache-2.0
// Preserve the existing horizontal SAT axes and original expert channel scales.
void main(tensor ids,tensor channel,tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int width=get_dim_size(output,0)/2,experts=get_dim_size(channel,2);
    for(int pair=begin[1];pair<end[1];++pair)for(int tile=begin[0];tile<end[0];++tile) {
        const int column=tile*128,role=column/width,n=column%width;
        const int expert=s_i32_ld_g(gen_addr((int5){pair*2+role,0},ids));
        const bfloat128 value=v_bf16_ld_tnsr_b((int5){n%256,n/256,expert},channel,0,(bfloat128)0,
                                            expert>=0 && expert<experts);
        const float128 converted=convert_bfloat128_to_float128(value,SW_LINEAR);
        v_f32_st_tnsr((int5){column,0,pair},output,converted.v1);
        v_f32_st_tnsr((int5){column+64,0,pair},output,converted.v2);
    }
}
