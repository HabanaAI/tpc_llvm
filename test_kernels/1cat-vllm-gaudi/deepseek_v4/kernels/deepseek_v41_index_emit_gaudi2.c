// SPDX-License-Identifier: Apache-2.0
void main(tensor scores,tensor positions,tensor candidates,tensor metadata,tensor output,
          int ratio,int reindex,int blocks_mode) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int request=begin[1];request<end[1];++request) {
    const int visible=(s_i32_ld_g(gen_addr((int5){request},positions))+1)/ratio;
    const int width=blocks_mode?2048:512;
    const int64 lanes=(int64)V_LANE_ID_32;
    const int direct_limit=blocks_mode?(visible+7)/8:visible;
    if(visible<=512 || (blocks_mode && direct_limit<=2048)) {
        for(int worker=begin[0];worker<end[0];++worker)
            for(int offset=worker*64;offset<width;offset+=24*64) {
                int64 ids=v_i32_sel_less_i32_b(lanes+offset,direct_limit,lanes+offset,-1);
                v_i32_st_tnsr((int5){offset,request},output,ids);
            }
        continue;
    }
    int valid_count=visible;
    int partition_count=visible;
    if(reindex) {
        const int visible_blocks=(visible+7)/8;
        valid_count=(visible_blocks<2048?visible_blocks:2048)*8;
        partition_count=16384;
    }
    if(blocks_mode) {
        valid_count=(valid_count+7)/8;
        partition_count=(partition_count+7)/8;
    }
    const int word_capacity=(get_dim_size(scores,0)+127)/128*4;
    const int total_greater=s_i32_ld_g(gen_addr((int5){1,request},metadata));
    const int equal_budget=width-total_greater;
    for(int worker=begin[0];worker<end[0];++worker) {
        int greater=s_i32_ld_g(gen_addr((int5){2+worker*2,request},metadata));
        int equal=s_i32_ld_g(gen_addr((int5){3+worker*2,request},metadata));
        int written=greater+(equal<equal_budget?equal:equal_budget);
        int buffered=0;
        int64 rows=0;
        const int first=partition_count*worker/24,last=partition_count*(worker+1)/24;
        const int scan_last=last<valid_count?last:valid_count;
        for(int word=first/32;word*32<scan_last;++word) {
            unsigned greater_bits=s_u32_ld_g(gen_addr((int5){50+word,request},metadata));
            unsigned equal_bits=s_u32_ld_g(gen_addr((int5){50+word_capacity+word,request},metadata));
            unsigned active=0xffffffff;
            if(word==first/32)active<<=first%32;
            if((word+1)*32>scan_last)active&=0xffffffffu>>(32-scan_last%32);
            unsigned keep=greater_bits&active;
            equal_bits&=active;
            while(equal_bits && equal<equal_budget) {
                keep|=equal_bits & -equal_bits;
                equal_bits&=equal_bits-1;
                ++equal;
            }
            while(keep && written<width) {
                const int offset=word*32+s_u32_find_first(keep,SW_FIND_ONE|SW_LSB);
                int logical=offset;
                if(reindex && !blocks_mode)logical=s_i32_ld_g(gen_addr((int5){offset/8,request},candidates))*8+offset%8;
                rows=v_i32_mov_vb(logical,0,rows,v_i32_cmp_eq_b(lanes,buffered),0);
                ++written;
                if(++buffered==64) {
                    v_i32_st_tnsr((int5){written-64,request},output,rows);
                    buffered=0;
                }
                keep&=keep-1;
            }
        }
        if(buffered)v_i32_st_tnsr_partial((int5){written-buffered,request},output,rows,buffered-1,0);
        if(worker==23)for(;written<width;written+=64)v_i32_st_tnsr((int5){written,request},output,(int64)-1);
    }
    }
}
