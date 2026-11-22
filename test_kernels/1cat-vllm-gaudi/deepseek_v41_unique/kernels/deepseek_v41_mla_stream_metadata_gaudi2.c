// SPDX-License-Identifier: Apache-2.0
// Same sorted union with more active address buckets and packed cursor/limit.
// Avoid keeping six independent limit values in the inner merge loop.
static inline int shared_lower_bound(tensor selected,int token,int last,int value) {
    int first=0;
    while(first<last) {
        const int middle=(first+last)>>1;
        const int logical=s_i32_ld_g(gen_addr((int5){middle,token},selected));
        if(logical>=0 && logical<value)first=middle+1;else last=middle;
    }
    return first;
}
void main(tensor selected,tensor positions,tensor lengths,tensor ids,tensor members,tensor counts) {
    const int tokens=get_dim_size(selected,1);
    const int range=65536/(get_dim_size(counts,0)-1);
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int bucket=begin[0];bucket<end[0];++bucket) {
        int written=0;
        if(bucket==0) {
            int window_start[6],window_length[6];
            for(int token=0;token<tokens;++token) {
                window_start[token]=s_i32_ld_g(gen_addr((int5){token},positions))-127;
                window_length[token]=s_i32_min(128,s_i32_max(0,s_i32_ld_g(gen_addr((int5){token},lengths))));
            }
            for(int physical=0;physical<256;++physical) {
                int mask=0;
                for(int token=0;token<tokens;++token) {
                    const int slot=(physical-window_start[token])&255;
                    if(slot<window_length[token] && window_start[token]+slot>=0)mask|=1<<token;
                }
                if(mask) {
                    s_i32_st_g(gen_addr((int5){written,bucket},ids),-2-physical);
                    s_i32_st_g(gen_addr((int5){written,bucket},members),mask);++written;
                }
            }
        } else {
            const int lower=(bucket-1)*range,upper=lower+range;
            int cursor[6],current[6];
            for(int token=0;token<tokens;++token) {
                const int limit=s_i32_min(512,s_i32_max(0,s_i32_ld_g(gen_addr((int5){token},lengths))-128));
                const int first=shared_lower_bound(selected,token,limit,lower);
                cursor[token]=(limit<<16)|first;
                current[token]=first<limit?s_i32_ld_g(gen_addr((int5){first,token},selected)):-1;
            }
            while(1) {
                int minimum=upper;
                for(int token=0;token<tokens;++token)
                    if(current[token]>=lower)minimum=s_i32_min(minimum,current[token]);
                if(minimum>=upper)break;
                int mask=0;
                for(int token=0;token<tokens;++token) {
                    if(current[token]==minimum) {
                        mask|=1<<token;++cursor[token];
                        const int packed=cursor[token];
                        current[token]=(packed&65535)<(packed>>16) ?
                            s_i32_ld_g(gen_addr((int5){packed&65535,token},selected)) : -1;
                    }
                }
                s_i32_st_g(gen_addr((int5){written,bucket},ids),minimum);
                s_i32_st_g(gen_addr((int5){written,bucket},members),mask);++written;
            }
        }
        s_i32_st_g(gen_addr((int5){bucket},counts),written);
    }
}
