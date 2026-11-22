// SPDX-License-Identifier: Apache-2.0
// Immutable mode/shift records select eight independent byte-preserving
// snapshots in one dispatch. Coordinate computation and byte copies share
// the same workpoint; there are no device-to-host decisions.
static inline void copy_rows(tensor source, tensor positions, tensor pages,
                             tensor config, tensor saved, tensor indices,
                             int buffer, int first_token, int last_token,
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
        if(first_block==0)s_i32_st_g(gen_addr((int5){token,buffer},indices),row);
        for(int block=first_block;block<last_block;++block) {
            const int column=block*256;
            if(column>=columns)continue;
            const int tail=s_i32_min(255,columns-column-1);
            const uchar256 value=v_u8_ld_tnsr_b((int5){column,row},source);
            v_u8_st_tnsr_partial((int5){column,token,buffer},saved,value,tail,0);
        }
    }
}
void main(tensor a,tensor b,tensor c,tensor d,tensor e,tensor f,tensor g,tensor h,
          tensor positions,tensor pages,tensor config,tensor saved,tensor indices) {
    const int5 first=get_index_space_offset(),last=first+get_index_space_size();
    for(int buffer=first[0];buffer<last[0];++buffer) {
        switch(buffer) {
        case 0:copy_rows(a,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 1:copy_rows(b,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 2:copy_rows(c,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 3:copy_rows(d,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 4:copy_rows(e,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 5:copy_rows(f,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 6:copy_rows(g,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        case 7:copy_rows(h,positions,pages,config,saved,indices,buffer,first[1],last[1],first[2],last[2]);break;
        }
    }
}
