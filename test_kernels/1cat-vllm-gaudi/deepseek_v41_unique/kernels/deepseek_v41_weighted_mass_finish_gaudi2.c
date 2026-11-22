// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
void main(tensor scores,tensor probability,tensor prefixes,tensor retained,tensor counts,tensor ids
#ifdef DSV41_WEIGHTED_FINISH_ACTIVE_MASK
          ,tensor active_blocks
#endif
          ) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(scores,0);
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[1];row<end[1];++row) {
        const unsigned int prefix=(unsigned int)s_i32_ld_g(gen_addr((int5){0,row},prefixes));
        for(int part=begin[0];part<end[0];++part) {
            const int first=part*2048,last=first+2048<columns?first+2048:columns;
            int64 count=0,winner=-1;
#ifdef DSV41_WEIGHTED_FINISH_ACTIVE_MASK
            uint64 active_mask=0;
#endif
            for(int col=first;col<last;col+=64) {
                const float64 value=v_f32_ld_tnsr_b((int5){col,row},scores);
                const float64 weight=v_f32_ld_tnsr_b((int5){col,row},probability);
                const uint64 bits=*((uint64*)&value);
                const uint64 key=v_u32_sel_grt_u32_b(bits,0x7fffffffu,~bits,bits^0x80000000u);
                const bool64 valid=v_i32_cmp_less_b(lanes+col,columns);
                const bool64 boundary=valid&v_u32_cmp_eq_b(key,prefix);
                count+=v_i32_mov_vb((int64)1,0,(int64)0,boundary,0);
                winner=v_i32_max_b(winner,v_i32_mov_vb(lanes+col,0,(int64)-1,boundary,0));
                const float64 kept=v_f32_mov_vb(weight,0,(float64)0,valid&v_u32_cmp_geq_b(key,prefix),0);
                v_f32_st_tnsr((int5){col,row},retained,kept);
#ifdef DSV41_WEIGHTED_FINISH_ACTIVE_MASK
                const int64 any=v_i32_reduce_max(v_i32_mov_vb((int64)1,0,(int64)0,
                                                            v_f32_cmp_grt_b(kept,0.0f),0));
                active_mask |= v_u32_sel_grt_i32_b(any,0,(uint64)(1u<<((col-first)/64)),(uint64)0);
#endif
            }
            count=v_i32_reduce_add(count);winner=v_i32_reduce_max(winner);
            v_i32_st_tnsr_partial((int5){part,row},counts,count,0,0);
            v_i32_st_tnsr_partial((int5){part,row},ids,winner,0,0);
#ifdef DSV41_WEIGHTED_FINISH_ACTIVE_MASK
            v_u32_st_tnsr_partial((int5){part,row},active_blocks,active_mask,0,0);
#endif
        }
    }
}
