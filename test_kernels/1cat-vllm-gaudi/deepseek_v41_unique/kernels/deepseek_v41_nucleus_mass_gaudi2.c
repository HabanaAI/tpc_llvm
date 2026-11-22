// SPDX-License-Identifier: Apache-2.0
// Read original logits once; subsequent radix passes consume this node's
// private probability workspace. All arithmetic remains FP32.
#pragma clang fp contract(off)
// Vector stores followed by global reads require an explicit TPC fence.
static inline void workspace_visible(void) {
    aso(SW_INC|SW_VPU);
    aso(SW_DEC|SW_SPU);
}

#include "deepseek_v41_selection_bitmap.h"
static inline float64 sum_row(float64 x) {
    x=v_f32_reduce_add(x);return v_f32_shuffle_b(x,(uchar256)0x80,0,x);
}
static inline int64 count_row(int64 x) {
    x=v_i32_reduce_add(x);return v_i32_shuffle_b(x,(uchar256)0x80,0,x);
}
void main(tensor logits,tensor controls,tensor probability,tensor masks,tensor info) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int n=get_dim_size(logits,0);
    int active_tiles[2020];
    for(int row=begin[0];row<end[0];++row) {
        const float temperature=s_f32_ld_g(gen_addr((int5){0,row},controls));
        const float top_p=s_f32_ld_g(gen_addr((int5){1,row},controls));
        const float top_k=s_f32_ld_g(gen_addr((int5){3,row},controls));
        float64 maximum=-3.402823466e38f;
        for(int col=0;col<n;col+=64) {
            float64 x=v_f32_ld_tnsr_b((int5){col,row},logits);
            if(temperature!=0) x=x/temperature;
            maximum=v_f32_max_b(maximum,x);
            v_f32_st_tnsr((int5){col,row},probability,x);
        }
        workspace_visible();
        maximum=v_f32_reduce_max(maximum);
        maximum=v_f32_shuffle_b(maximum,(uchar256)0x80,0,maximum);
        if(temperature==0) {
            int64 winner=n;
            for(int col=0;col<n;col+=64) {
                const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);
                winner=v_i32_min_b(winner,v_i32_sel_eq_f32_b(x,maximum,(int64)V_LANE_ID_32+col,(int64)n));
            }
            winner=v_i32_reduce_min(winner);winner=v_i32_shuffle_b(winner,(uchar256)0x80,0,winner);
            for(int col=0;col<n;col+=128) {
                const bool64 a=v_i32_cmp_eq_b((int64)V_LANE_ID_32+col,winner);
                const bool64 b=v_i32_cmp_eq_b((int64)V_LANE_ID_32+col+64,winner);
                v_f32_st_tnsr((int5){col,row},probability,v_f32_mov_vb((float64)1,0,(float64)0,a,0));
                v_f32_st_tnsr((int5){col+64,row},probability,v_f32_mov_vb((float64)1,0,(float64)0,b,0));
                v_u32_st_tnsr_partial((int5){col/32,row},masks,pack_words(pack_half(a),pack_half(b)),3,0);
            }
            v_i32_st_tnsr_partial((int5){0,row},info,(int64)1,0,0);
            v_i32_st_tnsr_partial((int5){1,row},info,(int64)1,0,0);
            continue;
        }
        float64 total=0;
        for(int col=0;col<n;col+=64) {
            const float64 x=v_exp_cephes_f32(v_f32_ld_tnsr_b((int5){col,row},probability)-maximum);
            v_f32_st_tnsr((int5){col,row},probability,x);total+=x;
        }
        workspace_visible();
        total=sum_row(total);
        const float64 desired=total*top_p;
        uint64 boundary=0;
        if(top_p<1) {
            for(int bit=31;bit>=16;--bit) {
                const uint64 trial=boundary|(1u<<bit);float64 mass=0;
                for(int col=0;col<n;col+=64) {
                    const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);
                    mass+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_geq_b(as_uint64(x),trial),0);
                }
                boundary=v_u32_sel_geq_f32_b(sum_row(mass),desired,trial,boundary);
            }
            float64 above=0;
            const uint64 high=boundary|65535u;
            for(int col=0;col<n;col+=64) {
                const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);const uint64 key=as_uint64(x);
                above+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_grt_b(key,high),0);
                const bool64 member=v_u32_cmp_geq_b(key,boundary)&v_u32_cmp_leq_b(key,high);
                const int64 count=count_row(v_i32_mov_vb((int64)1,0,(int64)0,member,0));
                v_i32_st_tnsr_partial((int5){col/64,row},masks,v_i32_sel_grt_i32_b(count,0,(int64)1,(int64)0),0,0);
            }
            workspace_visible();
            above=sum_row(above);
            int active_count=0;
            for(int tile=0;tile<n/64;++tile)
                if(s_i32_ld_g(gen_addr((int5){tile,row},masks)))active_tiles[active_count++]=tile*64;
            for(int bit=15;bit>=0;--bit) {
                const uint64 trial=boundary|(1u<<bit);float64 mass=0;
                for(int tile=0;tile<active_count;++tile) {
                    const int col=active_tiles[tile];
                    const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);const uint64 key=as_uint64(x);
                    mass+=v_f32_mov_vb(x,0,(float64)0,v_u32_cmp_geq_b(key,trial)&v_u32_cmp_leq_b(key,high),0);
                }
                boundary=v_u32_sel_geq_f32_b(above+sum_row(mass),desired,trial,boundary);
            }
        }
        float64 retained=0;int64 equal=0,count=0;
        for(int col=0;col<n;col+=64) {
            const float64 x=v_f32_ld_tnsr_b((int5){col,row},probability);const uint64 key=as_uint64(x);
            const bool64 keep=v_u32_cmp_geq_b(key,boundary)&v_f32_cmp_grt_b(x,0);
            retained+=v_f32_mov_vb(x,0,(float64)0,keep,0);
            count+=v_i32_mov_vb((int64)1,0,(int64)0,keep,0);
            equal+=v_i32_mov_vb((int64)1,0,(int64)0,v_u32_cmp_eq_b(key,boundary),0);
        }
        retained=sum_row(retained);equal=count_row(equal);count=count_row(count);
        const bool64 whole_boundary=v_f32_cmp_less_b(retained-as_float64(boundary),desired);
        bool64 certificate=(v_i32_cmp_leq_b(equal,1)|whole_boundary)
            &v_f32_cmp_grt_b(retained,0)&v_u32_cmp_less_b(as_uint64(retained),0x7f800000u);
        if(top_k>0 || top_p<=0 || top_p>1) certificate=v_i32_cmp_eq_b((int64)0,(int64)1);
        float64 inverse=v_reciprocal_f32(retained);inverse=inverse*(2.0f-retained*inverse);
        for(int col=0;col<n;col+=128) {
            const float64 a=v_f32_ld_tnsr_b((int5){col,row},probability);
            const float64 b=v_f32_ld_tnsr_b((int5){col+64,row},probability);
            const bool64 ka=v_u32_cmp_geq_b(as_uint64(a),boundary)&v_f32_cmp_grt_b(a,0);
            const bool64 kb=v_u32_cmp_geq_b(as_uint64(b),boundary)&v_f32_cmp_grt_b(b,0);
            v_f32_st_tnsr((int5){col,row},probability,v_f32_mov_vb(a*inverse,0,(float64)0,ka,0));
            v_f32_st_tnsr((int5){col+64,row},probability,v_f32_mov_vb(b*inverse,0,(float64)0,kb,0));
            v_u32_st_tnsr_partial((int5){col/32,row},masks,pack_words(pack_half(ka),pack_half(kb)),3,0);
        }
        v_i32_st_tnsr_partial((int5){0,row},info,count,0,0);
        v_i32_st_tnsr_partial((int5){1,row},info,v_i32_mov_vb((int64)1,0,(int64)0,certificate,0),0,0);
    }
}
