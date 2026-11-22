// SPDX-License-Identifier: Apache-2.0
// Four sources, eight direct outputs: fifteen tensor descriptors, no packed output slices.
static inline void copy_rows(tensor source, tensor positions, tensor pages,
                             tensor config, tensor saved, tensor indices,
                             int element_bytes, int buffer, int first_token, int last_token,
                             int first_block, int last_block) {
    const int mode=s_i32_ld_g(gen_addr((int5){0,buffer},config));
    if(mode<0)return;
    const int shift=s_i32_ld_g(gen_addr((int5){1,buffer},config));
    const int columns=get_dim_size(source,0),source_rows=get_dim_size(source,1);
    for(int token=first_token;token<last_token;++token) {
        const int logical=s_i32_ld_g(gen_addr((int5){token},positions));
        int row=token;
        if(mode==0)row=logical&((1<<shift)-1);
        else if(mode==1) {
            const int page_id=logical>>7;
            const int page=page_id>=0 && page_id<get_dim_size(pages,0)
                ?s_i32_ld_g(gen_addr((int5){page_id},pages)):-1;
            row=(page<<(7-shift))+((logical&127)>>shift);
        } else if(mode==2)row=logical>>shift;
        if(row<0 || row>=source_rows)row=0;
        if(first_block==0)s_i32_st_g(gen_addr((int5){token},indices),row);
        for(int block=first_block;block<last_block;++block) {
            const int width=256/element_bytes;
            const int column=block*width;
            if(column>=columns)continue;
            const int tail=s_i32_min(width-1,columns-column-1);
            if(element_bytes==4) {
                const int64 value=v_i32_ld_tnsr_b((int5){column,row},source);
                v_i32_st_tnsr_partial((int5){column,token},saved,value,tail,0);
            } else if(element_bytes==2) {
                const ushort128 value=v_u16_ld_tnsr_b((int5){column,row},source);
                v_u16_st_tnsr_partial((int5){column,token},saved,value,tail,0);
            } else {
                const uchar256 value=v_u8_ld_tnsr_b((int5){column,row},source);
                v_u8_st_tnsr_partial((int5){column,token},saved,value,tail,0);
            }
        }
    }
}
void main(tensor a,tensor b,tensor c,tensor d,tensor positions,tensor pages,tensor config,
          tensor sa,tensor sb,tensor sc,tensor sd,tensor ia,tensor ib,tensor ic,tensor id,
          int byte_codes) {
    const int5 first=get_index_space_offset(),last=first+get_index_space_size();
    for(int buffer=first[0];buffer<last[0];++buffer) {
        const int bytes=1<<((byte_codes>>(buffer*2))&3);
        switch(buffer) {
        case 0:copy_rows(a,positions,pages,config,sa,ia,bytes,buffer,first[1],last[1],first[2],last[2]);break;
        case 1:copy_rows(b,positions,pages,config,sb,ib,bytes,buffer,first[1],last[1],first[2],last[2]);break;
        case 2:copy_rows(c,positions,pages,config,sc,ic,bytes,buffer,first[1],last[1],first[2],last[2]);break;
        case 3:copy_rows(d,positions,pages,config,sd,id,bytes,buffer,first[1],last[1],first[2],last[2]);break;
        }
    }
}
