// SPDX-License-Identifier: Apache-2.0
// Fixed short-search layout, consuming current selection state and page table.
// DSV41_PREFIX_RATIO is one or two; selection remains an actual graph input.
void main(tensor selected, tensor positions, tensor block_table,
          tensor row_ids, tensor attention_indices, tensor lengths) {
    const int5 begin=get_index_space_offset();
    const int5 end=begin+get_index_space_size();
    const uint64 lanes=V_LANE_ID_32;
    const int page_width=128/DSV41_PREFIX_RATIO;
    const int tokens=get_dim_size(selected,1);
    const int64 pages=v_i32_ld_tnsr_partial_b((int5){0},block_table,7,0);
    const int64 repeated=v_i32_mov_dual_group_all_b(pages,0xffffffff,0,0,0,0,
                                                  MkWrA(3,3,3,3),pages);
    for(int token=begin[2];token<end[2];++token) {
        const int position=s_i32_ld_g(gen_addr((int5){token,0,0,0,0},positions));
        for(int chunk=0;chunk<2;++chunk) {
            const int64 absolute=(int64)lanes+chunk*64+position-127;
            const int64 window=v_i32_sel_geq_i32_b(absolute,0,absolute&255,-1);
            v_i32_st_tnsr((int5){chunk*64,token,0,0,0},attention_indices,window);
        }
        int64 counts=0;
        for(int chunk=0;chunk<8;++chunk) {
            const int64 indices=v_i32_ld_tnsr_b((int5){chunk*64,token,0,0,0},selected);
            const int64 local=v_i32_sel_geq_i32_b(indices,0,indices+256,-1);
            v_i32_st_tnsr((int5){128+chunk*64,token,0,0,0},attention_indices,local);
            counts+=v_i32_sel_geq_i32_b(indices,0,1,0);
            if(token==tokens-1) {
                const int64 safe=v_i32_max_b(indices,0);
                const uint64 page=(uint64)(safe/page_width);
                uint256 wide={0};wide.v1=page|0x80;wide.v2=wide.v1;wide.v3=wide.v1;wide.v4=wide.v1;
                const uchar256 directions=v_convert_u32_to_u8_all_b(wide);
                const int64 blocks=v_i32_shuffle_b(repeated,directions,0,0);
                const int64 physical=blocks*page_width+(safe&(page_width-1))+256;
                const int64 rows=v_i32_sel_geq_i32_b(indices,0,physical,-1);
                v_i32_st_tnsr((int5){256+chunk*64,0,0,0,0},row_ids,rows);
            }
        }
        const int64 count=v_i32_reduce_add(counts)+128;
        v_i32_st_tnsr_partial((int5){token,0,0,0,0},lengths,count,0,0);
        if(token==tokens-1)
            for(int chunk=0;chunk<4;++chunk)
                v_i32_st_tnsr((int5){chunk*64,0,0,0,0},row_ids,(int64)lanes+chunk*64);
    }
}
