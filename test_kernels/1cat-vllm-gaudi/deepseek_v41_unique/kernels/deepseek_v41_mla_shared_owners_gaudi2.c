// SPDX-License-Identifier: Apache-2.0
// Sorted selections are produced on device by the common indexer. Keep an
// owner's original slot, so no CPU union, atomic bitmap or compacting copy is
// needed. The MME consumes this shared layout directly, including zero holes.
static inline int lower_bound(tensor selected, int token, int limit, int value) {
    int first=0,last=limit;
    while(first<last) {
        const int middle=(first+last)>>1;
        const int key=s_i32_ld_g(gen_addr((int5){middle,token},selected));
        if(key>=0 && key<value)first=middle+1;else last=middle;
    }
    return first;
}
void main(tensor selected,tensor positions,tensor lengths,tensor ids,tensor members) {
    const int tokens=get_dim_size(selected,1),width=get_dim_size(ids,0);
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    int limits[6],window_start[6],window_length[6];
    for(int token=0;token<tokens;++token) {
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        limits[token]=s_i32_min(512,s_i32_max(0,length-128));
        window_start[token]=s_i32_ld_g(gen_addr((int5){token},positions))-127;
        window_length[token]=s_i32_min(128,s_i32_max(0,length));
    }
    for(int point=begin[0];point<end[0];++point) {
        for(int row=point*16;row<(point+1)*16 && row<width;++row) {
            int id=-1,mask=0;
            if(row<256) {
                for(int token=0;token<tokens;++token) {
                    const int slot=(row-window_start[token])&255;
                    if(slot<window_length[token] && window_start[token]+slot>=0)mask|=1<<token;
                }
                if(mask)id=-2-row;
            } else {
                const int source_token=(row-256)>>9,source_slot=(row-256)&511;
                if(source_slot<limits[source_token]) {
                    const int logical=s_i32_ld_g(gen_addr((int5){source_slot,source_token},selected));
                    if(logical>=0) {
                        int owner_token=-1,owner_slot=-1;
                        for(int token=0;token<tokens;++token) {
                            const int slot=lower_bound(selected,token,limits[token],logical);
                            if(slot<limits[token] &&
                               s_i32_ld_g(gen_addr((int5){slot,token},selected))==logical) {
                                mask|=1<<token;
                                if(owner_token<0){owner_token=token;owner_slot=slot;}
                            }
                        }
                        if(owner_token==source_token && owner_slot==source_slot)id=logical;
                        else mask=0;
                    }
                }
            }
            s_i32_st_g(gen_addr((int5){row},ids),id);
            s_i32_st_g(gen_addr((int5){row},members),mask);
        }
    }
}
