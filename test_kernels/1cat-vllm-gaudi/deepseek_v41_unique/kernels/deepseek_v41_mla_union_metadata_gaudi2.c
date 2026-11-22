// SPDX-License-Identifier: Apache-2.0
// Merge ascending logical selections, preserving every original query slot.
static inline int source(tensor selected, int row, int token) {
    return s_i32_ld_g(gen_addr((int5){row,token,0,0,0},selected));
}
static inline void link(tensor next, int member, int previous) {
    s_i32_st_g(gen_addr((int5){member%640,member/640,0,0,0},next),previous);
}
void main(tensor selected,tensor positions,tensor lengths,tensor rows,
          tensor heads,tensor next,tensor counts) {
    const int tokens=get_dim_size(selected,1);
    int prefix_heads[257],cursor[6],limit[6],current[6];
    for(int i=0;i<257;++i) prefix_heads[i]=-1;
    bool sorted=1;
    int invalid=0,active=0;
    for(int token=0;token<tokens;++token) {
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        limit[token]=s_i32_min(512,s_i32_max(0,length-128));
        for(int slot=0;slot<128;++slot) {
            const int absolute=position-127+slot;
            const int owner=(slot<length && absolute>=0) ? 1+(absolute&255) : 0;
            const int member=token*640+slot;
            link(next,member,prefix_heads[owner]);prefix_heads[owner]=member;
            if(owner==0) ++invalid;else ++active;
        }
        int previous=-1;
        for(int slot=0;slot<512;++slot) {
            const int logical=source(selected,slot,token);
            if(logical<0 || slot>=limit[token]) {
                const int member=token*640+128+slot;
                link(next,member,prefix_heads[0]);prefix_heads[0]=member;++invalid;
            } else {
                if(logical<previous) sorted=0;
                previous=logical;++active;
            }
        }
        cursor[token]=0;
    }
    int unique=0;
    if(!sorted) {
        // Correct device fallback: retain original slots without deduplication.
        for(int token=0;token<tokens;++token) for(int slot=0;slot<limit[token];++slot) {
            const int logical=source(selected,slot,token);
            if(logical<0) continue;
            const int member=token*640+128+slot;
            s_i32_st_g(gen_addr((int5){unique},rows),logical);
            s_i32_st_g(gen_addr((int5){257+unique},heads),member);
            link(next,member,-1);++unique;
        }
    } else {
        for(int token=0;token<tokens;++token) {
            current[token]=-1;
            while(cursor[token]<limit[token] && current[token]<0) {
                current[token]=source(selected,cursor[token],token);
                if(current[token]<0) ++cursor[token];
            }
        }
        while(1) {
            bool present=0;int minimum=0x7fffffff;
            for(int token=0;token<tokens;++token) if(current[token]>=0) {
                present=1;minimum=s_i32_min(minimum,current[token]);
            }
            if(!present) break;
            int head=-1;
            for(int token=0;token<tokens;++token) {
                while(current[token]==minimum && cursor[token]<limit[token]) {
                    const int member=token*640+128+cursor[token];
                    link(next,member,head);head=member;++cursor[token];
                    current[token]=-1;
                    while(cursor[token]<limit[token] && current[token]<0) {
                        current[token]=source(selected,cursor[token],token);
                        if(current[token]<0) ++cursor[token];
                    }
                }
            }
            s_i32_st_g(gen_addr((int5){unique},rows),minimum);
            s_i32_st_g(gen_addr((int5){257+unique},heads),head);++unique;
        }
    }
    for(int row=0;row<257;++row) s_i32_st_g(gen_addr((int5){row},heads),prefix_heads[row]);
    s_i32_st_g(gen_addr((int5){0},counts),unique);
    s_i32_st_g(gen_addr((int5){1},counts),invalid);
    s_i32_st_g(gen_addr((int5){2},counts),!sorted);
    s_i32_st_g(gen_addr((int5){3},counts),active);
}
