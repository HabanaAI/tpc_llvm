// SPDX-License-Identifier: Apache-2.0
// Exact ordered-key bounds of positive mass, once per 64-score tile.
void main(tensor scores,tensor weights,tensor bounds) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[1];row<end[1];++row)for(int tile=begin[0];tile<end[0];++tile) {
        const int col=tile*64;
        const float64 score=v_f32_ld_tnsr_b((int5){col,row},scores);
        const float64 weight=v_f32_ld_tnsr_b((int5){col,row},weights);
        const uint64 bits=*((uint64*)&score);
        const uint64 keys=v_u32_sel_grt_u32_b(bits,0x7fffffffu,~bits,bits^0x80000000u);
        const bool64 active=v_f32_cmp_grt_b(weight,0);
        const uint64 low_keys=v_u32_mov_vb(keys,0,(uint64)0xffffffffu,active,0)^0x80000000u;
        const uint64 high_keys=v_u32_mov_vb(keys,0,(uint64)0,active,0)^0x80000000u;
        const uint64 low=(uint64)v_i32_reduce_min((int64)low_keys)^0x80000000u;
        const uint64 high=(uint64)v_i32_reduce_max((int64)high_keys)^0x80000000u;
        v_u32_st_tnsr_partial((int5){tile,0,row},bounds,low,0,0);
        v_u32_st_tnsr_partial((int5){tile,1,row},bounds,high,0,0);
    }
}
