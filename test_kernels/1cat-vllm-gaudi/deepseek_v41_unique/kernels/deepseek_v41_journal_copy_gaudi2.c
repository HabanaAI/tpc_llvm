// SPDX-License-Identifier: Apache-2.0
// Coordinates, bounds and byte-preserving copies share a node. A backup is
// still a distinct allocation; snapshots cannot alias the mutable cache.
void main(tensor source,tensor positions,tensor pages,tensor saved,tensor indices,
          int mode,int parameter_shift,int source_rows,int columns,int element_bytes) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int token=begin[1];token<end[1];++token) {
        const int logical=s_i32_ld_g(gen_addr((int5){token},positions));
        int row=token;
        if(mode==0) row=logical&((1<<parameter_shift)-1);
        else if(mode==1) {
            const int page_id=logical>>7;
            const int page=page_id>=0 && page_id<get_dim_size(pages,0)
                ? s_i32_ld_g(gen_addr((int5){page_id},pages)) : -1;
            row=(page<<(7-parameter_shift))+((logical&127)>>parameter_shift);
        } else if(mode==2) row=logical>>parameter_shift;
        if(row<0 || row>=source_rows)row=0;
        if(begin[0]==0)s_i32_st_g(gen_addr((int5){token},indices),row);
        const int width=256/element_bytes;
        for(int block=begin[0];block<end[0];++block) {
            const int col=block*width;
            const int tail=columns-col<width?columns-col-1:width-1;
            if(element_bytes==4) {
                const int64 bits=v_i32_ld_tnsr_b((int5){col,row},source);
                v_i32_st_tnsr_partial((int5){col,token},saved,bits,tail,0);
            } else if(element_bytes==2) {
                const ushort128 bits=v_u16_ld_tnsr_b((int5){col,row},source);
                v_u16_st_tnsr_partial((int5){col,token},saved,bits,tail,0);
            } else {
                const uchar256 bits=v_u8_ld_tnsr_b((int5){col,row},source);
                v_u8_st_tnsr_partial((int5){col,token},saved,bits,tail,0);
            }
        }
    }
}
