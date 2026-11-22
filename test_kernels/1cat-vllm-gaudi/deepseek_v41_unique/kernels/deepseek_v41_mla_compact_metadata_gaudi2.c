// SPDX-License-Identifier: Apache-2.0
// Parallel merge of sorted query lists in disjoint logical address ranges.
// Search each query once per range, rather than six binary searches per KV.
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
            const int lower=(bucket-1)*1024,upper=lower+1024;
            int cursor[6],limit[6],current[6];
            for(int token=0;token<tokens;++token) {
                limit[token]=s_i32_min(512,s_i32_max(0,s_i32_ld_g(gen_addr((int5){token},lengths))-128));
                cursor[token]=shared_lower_bound(selected,token,limit[token],lower);
                current[token]=cursor[token]<limit[token] ?
                    s_i32_ld_g(gen_addr((int5){cursor[token],token},selected)) : -1;
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
                        current[token]=cursor[token]<limit[token] ?
                            s_i32_ld_g(gen_addr((int5){cursor[token],token},selected)) : -1;
                    }
                }
                s_i32_st_g(gen_addr((int5){written,bucket},ids),minimum);
                s_i32_st_g(gen_addr((int5){written,bucket},members),mask);++written;
            }
        }
        s_i32_st_g(gen_addr((int5){bucket},counts),written);
    }
}
