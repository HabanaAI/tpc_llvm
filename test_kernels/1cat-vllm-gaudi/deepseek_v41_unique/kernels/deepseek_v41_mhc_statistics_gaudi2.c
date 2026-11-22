// SPDX-License-Identifier: Apache-2.0
// Feature-parallel BF16 statistics; 40 tiles per model row, four vectors per tile.
#pragma clang fp contract(off)
void main(tensor residual, tensor partials) {
    const int5 begin=get_index_space_offset();
    const int5 end=begin+get_index_space_size();
    for(int row=begin[1];row<end[1];++row) {
        for(int tile=begin[0];tile<end[0];++tile) {
            float128 square={0};
            #pragma unroll(4)
            for(int vector=0;vector<4;++vector) {
                float128 x=v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b((int5){tile*512+vector*128,row},residual));
                square.v1+=x.v1*x.v1;
                square.v2+=x.v2*x.v2;
            }
            float64 sum=v_f32_reduce_add(square.v1+square.v2);
            v_f32_st_tnsr_partial((int5){tile,row},partials,sum,0,0);
        }
    }
}
