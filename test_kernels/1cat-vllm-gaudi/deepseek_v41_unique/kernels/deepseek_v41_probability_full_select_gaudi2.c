// SPDX-License-Identifier: Apache-2.0
// Repair only: repeat exactly the same inclusive scan for total and selection.
// This prevents a block-total reduction from disagreeing with its local scan.
#pragma clang fp contract(off)
void main(tensor probability,tensor controls,tensor output) {
    const int5 first=get_index_space_offset(),last=first+get_index_space_size();
    const int columns=get_dim_size(probability,0);
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=first[0];row<last[0];++row) {
        float64 total=0,target=0;
        int64 selected=columns,last_positive=0;
        for(int pass=0;pass<2;++pass) {
            float64 preceding=0;
            for(int col=0;col<columns;col+=64) {
                const float64 value=v_f32_ld_tnsr_b((int5){col,row},probability);
                float64 prefix=value;
                #pragma unroll
                for(int shift=1;shift<64;shift*=2) {
                    const float64 moved=v_element_shift_up_f32(prefix,shift);
                    prefix+=v_f32_mov_vb(moved,0,(float64)0,v_i32_cmp_geq_b(lanes,shift),0);
                }
                const float64 cumulative=preceding+prefix;
                if(pass) {
                    const bool64 positive=v_f32_cmp_grt_b(value,0.0f);
                    selected=v_i32_min_b(selected,v_i32_mov_vb(lanes+col,0,(int64)columns,
                        positive&v_f32_cmp_geq_b(cumulative,target),0));
                    last_positive=v_i32_max_b(last_positive,v_i32_mov_vb(lanes+col,0,(int64)0,positive,0));
                }
                preceding=v_broadcast_element_f32(cumulative,63);
            }
            if(!pass) {
                total=preceding;
                target=s_f32_ld_g(gen_addr((int5){2,row},controls))*total;
            }
        }
        selected=v_i32_reduce_min(selected);
        last_positive=v_i32_reduce_max(last_positive);
        selected=v_i32_mov_vb(last_positive,0,selected,v_i32_cmp_geq_b(selected,columns),0);
        v_i32_st_tnsr_partial((int5){row},output,selected,0,0);
    }
}
