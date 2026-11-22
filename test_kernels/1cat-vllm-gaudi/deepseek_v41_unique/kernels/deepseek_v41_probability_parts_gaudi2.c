// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
void main(tensor probability,tensor partials) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(probability,0);
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[1];row<end[1];++row)
    for(int part=begin[0];part<end[0];++part) {
        float64 mass=0;
        const int last=part*2048+2048<columns?part*2048+2048:columns;
        for(int col=part*2048;col<last;col+=64) {
            const float64 value=v_f32_ld_tnsr_b((int5){col,row},probability);
            mass+=v_f32_mov_vb(value,0,(float64)0,v_i32_cmp_less_b(lanes+col,columns),0);
        }
        mass=v_f32_reduce_add(mass);
        v_f32_st_tnsr_partial((int5){part,row},partials,mass,0,0);
    }
}
