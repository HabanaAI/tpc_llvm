// SPDX-License-Identifier: Apache-2.0
// Scatter the once-per-token FP8 activation and its scalar scale into owner
// rows.  Every real top-6 token has distinct experts, so owner M is at most6.
void main(tensor x, tensor sx, tensor routing, tensor metadata,
          tensor packed, tensor packed_sx, tensor packed_routing) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    const int chunks=get_dim_size(x,0)/256;
    for(int slot=begin[0];slot<end[0];++slot) {
        const int address=s_i32_ld_g(gen_addr((int5){slot+9,0,0,0,0},metadata));
        if(address<0) continue;
        const bool tail=(address&(1<<20))!=0;
        const int destination=(address>>9)&2047;
        const int owner=tail?destination/6:destination/36;
        const int row=tail?destination%6:destination%36;
        if(owner<0 || owner>=36 || row<0 || row>=6) continue;
        const int token=slot/6, route=slot%6;
        s_f32_st_g(gen_addr((int5){row,owner,0,0,0},packed_sx),
                   s_f32_ld_g(gen_addr((int5){0,token,0,0,0},sx)));
        s_f32_st_g(gen_addr((int5){row,owner,0,0,0},packed_routing),
                   s_f32_ld_g(gen_addr((int5){route,token,0,0,0},routing)));
        for(int chunk=0;chunk<chunks;++chunk) {
            const minifloat256 value=v_f8_ld_tnsr_b((int5){chunk*256,token,0,0,0},x);
            v_f8_st_tnsr((int5){chunk*256,row,owner,0,0},packed,value);
        }
    }
}
