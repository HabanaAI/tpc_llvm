// SPDX-License-Identifier: Apache-2.0
// Reuse decode23551208's eight-row candidate coordinate mapping, consuming
// the decoded BF16 index mirror directly. Each block ID is loaded once for
// its eight rows; no I64 coordinates or gathered byte intermediates exist.
void main(tensor keys,tensor blocks,tensor logical,tensor selected) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int capacity=get_dim_size(keys,1);
    const int64 lanes=(int64)read_lane_id_4b_b();
    for(int query=begin[1];query<end[1];++query)
    for(int tile=begin[0];tile<end[0];++tile) {
        int64 ids=-1;
        for(int j=0;j<8;++j) {
            const int block=s_i32_ld_g(gen_addr((int5){tile*8+j,query},blocks));
            const int first=(int)((unsigned)block<<3);
            const int64 current=block>=0 ? (lanes&7)+first : (int64)-1;
            ids=v_i32_mov_vb(current,0,ids,v_i32_cmp_eq_b(lanes>>3,j),0);
            for(int row=0;row<8;++row) {
                const int index=block>=0 ? first+row : -1;
                const bool valid=index>=0 && index<capacity;
                const int safe=valid ? index : 0;
                const bfloat128 value=v_bf16_ld_tnsr_b((int5){0,safe},keys,0,(bfloat128)0,valid,0);
                v_bf16_st_tnsr((int5){0,tile*64+j*8+row,query},selected,value);
            }
        }
        v_i32_st_tnsr((int5){tile*64,query},logical,ids);
    }
}
