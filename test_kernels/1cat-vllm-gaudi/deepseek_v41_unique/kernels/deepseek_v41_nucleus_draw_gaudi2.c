// SPDX-License-Identifier: Apache-2.0
// Match descending-probability inverse CDF with the same request uniform.
// Small nuclei use a bounded local list; large nuclei use weighted radix
// selection, never a full-vocabulary sort or a host round trip.
#pragma clang fp contract(off)
// Vector stores followed by global reads require an explicit TPC fence.
static inline void workspace_visible(void) {
    aso(SW_INC|SW_VPU);
    aso(SW_DEC|SW_SPU);
}

static inline float64 sum_row(float64 x) {
    x=v_f32_reduce_add(x);return v_f32_shuffle_b(x,(uchar256)0x80,0,x);
}
static inline int64 count_row(int64 x) {
    x=v_i32_reduce_add(x);return v_i32_shuffle_b(x,(uchar256)0x80,0,x);
}
void main(tensor probability,tensor masks,tensor info,tensor controls,tensor result,tensor fine_flags) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int n=get_dim_size(probability,0);
    float candidates[64];int ids[64];
    int active_tiles[2020];
    for(int row=begin[0];row<end[0];++row) {
        const int count=s_i32_ld_g(gen_addr((int5){0,row},info));
        int certified=s_i32_ld_g(gen_addr((int5){1,row},info));
        const float draw=s_f32_ld_g(gen_addr((int5){2,row},controls));
        if(count<=64) {
            int used=0;
            for(int word=0;word<n/32 && used<count;++word) {
                unsigned active=s_u32_ld_g(gen_addr((int5){word,row},masks));
                while(active && used<count) {
                    const int index=word*32+s_u32_find_first(active,SW_FIND_ONE|SW_LSB);
                    const float value=s_f32_ld_g(gen_addr((int5){index,row},probability));
                    int place=used;
                    while(place>0 && candidates[place-1]<value) {
                        candidates[place]=candidates[place-1];ids[place]=ids[place-1];--place;
                    }
                    candidates[place]=value;ids[place]=index;++used;active&=active-1;
                }
            }
            float total=0;for(int i=0;i<used;++i)total+=candidates[i];
            const float target=draw*total;
            float cumulative=0,previous=0;int chosen=used-1;
            for(int i=0;i<used;++i) {
                previous=cumulative;cumulative+=candidates[i];
                if(cumulative>=target){chosen=i;break;}
            }
            int token=0;
            if(used>0) {
                token=ids[chosen];
                if((chosen>0 && candidates[chosen]==candidates[chosen-1]) ||
                   (chosen+1<used && candidates[chosen]==candidates[chosen+1]))certified=0;
                // Close CDF boundaries need the reference's exact association.
                if((chosen+1<used && s_f32_abs(cumulative-target)<1e-6f*total) ||
                   (chosen>0 && s_f32_abs(previous-target)<1e-6f*total))certified=0;
            } else certified=0;
            s_i32_st_g(gen_addr((int5){0,row},result),token);
            s_i32_st_g(gen_addr((int5){1,row},result),certified);
            for(int col=0;col<n/64;col+=64)
                v_i32_st_tnsr_partial((int5){col,row},fine_flags,(int64)0,
                                     n/64-col<64 ? n/64-col-1 : 63,0);
            continue;
        }
        int nonzero_count=0;
        for(int tile=0;tile<n/64;++tile)
            if(s_u32_ld_g(gen_addr((int5){tile*2,row},masks))|
               s_u32_ld_g(gen_addr((int5){tile*2+1,row},masks)))active_tiles[nonzero_count++]=tile*64;
        float64 total=0;
        for(int col=0;col<n;col+=64)total+=v_f32_ld_tnsr_b((int5){col,row},probability);
        total=sum_row(total);const float64 desired=total*draw;uint64 boundary=0;
        for(int bit=31;bit>=16;--bit) {
            const uint64 trial=boundary|(1u<<bit);float64 mass=0;
            for(int tile=0;tile<nonzero_count;++tile) {
                const int col=active_tiles[tile];
                const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);
                mass+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_geq_b(as_uint64(x),trial),0);
            }
            boundary=v_u32_sel_geq_f32_b(sum_row(mass),desired,trial,boundary);
        }
        float64 above=0;const uint64 high=boundary|65535u;
        for(int col=0;col<n;col+=64) {
            const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);const uint64 key=as_uint64(x);
            above+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_grt_b(key,high),0);
            const bool64 member=v_u32_cmp_geq_b(key,boundary)&v_u32_cmp_leq_b(key,high)&v_f32_cmp_grt_b(x,0);
            const int64 hits=count_row(v_i32_mov_vb((int64)1,0,(int64)0,member,0));
            v_i32_st_tnsr_partial((int5){col/64,row},fine_flags,v_i32_sel_grt_i32_b(hits,0,(int64)1,(int64)0),0,0);
        }
        workspace_visible();
        above=sum_row(above);
        int fine_count=0;
        for(int tile=0;tile<n/64;++tile)
            if(s_i32_ld_g(gen_addr((int5){tile,row},fine_flags)))active_tiles[fine_count++]=tile*64;
        for(int bit=15;bit>=0;--bit) {
            const uint64 trial=boundary|(1u<<bit);float64 mass=0;
            for(int tile=0;tile<fine_count;++tile) {
                const int col=active_tiles[tile];
                const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);const uint64 key=as_uint64(x);
                mass+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_geq_b(key,trial)&v_u32_cmp_leq_b(key,high),0);
            }
            boundary=v_u32_sel_geq_f32_b(above+sum_row(mass),desired,trial,boundary);
        }
        int64 equal=0,winner=n;
        for(int col=0;col<n;col+=64) {
            const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);
            const bool64 selected=v_u32_cmp_eq_b(as_uint64(x),boundary)&v_f32_cmp_grt_b(x,0);
            equal+=v_i32_mov_vb((int64)1,0,(int64)0,selected,0);
            winner=v_i32_min_b(winner,v_i32_mov_vb((int64)V_LANE_ID_32+col,0,(int64)n,selected,0));
        }
        equal=count_row(equal);winner=v_i32_reduce_min(winner);winner=v_i32_shuffle_b(winner,(uchar256)0x80,0,winner);
        v_i32_st_tnsr_partial((int5){0,row},result,winner,0,0);
        const int64 valid=v_i32_sel_eq_i32_b(equal,1,(int64)certified,(int64)0);
        v_i32_st_tnsr_partial((int5){1,row},result,valid,0,0);
    }
}
