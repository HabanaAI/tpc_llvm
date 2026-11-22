// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "deepseek_v41_sampling_reduce.h"
// Preserve the original two compiled FP32 cumsums and all exp/divide math.
// Merge only retention, boundary/tie certificate and zeroing of discarded mass.
void main(tensor values,tensor probabilities,tensor cumulative,tensor cuts,
          tensor controls,tensor total,tensor filtered,tensor coverage,
          int columns,int ranks,int width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        const float temperature=s_f32_ld_g(gen_addr((int5){0,row},controls));
        const float top_p=s_f32_ld_g(gen_addr((int5){1,row},controls));
        const float top_k=s_f32_ld_g(gen_addr((int5){3,row},controls));
        const float z=s_f32_ld_g(gen_addr((int5){0,row},total));
        const unsigned zbits=*((unsigned*)&z);
        const bool64 unbounded=v_f32_cmp_leq_b((float64)top_k,0);
        int64 retained=0,ties=0;
        for(int col=0;col<columns;col+=64) {
            const int5 at={col,row};
            const float64 probability=v_f32_ld_tnsr_b(at,probabilities);
            const float64 cdf=v_f32_ld_tnsr_b(at,cumulative);
            const bool64 keep=v_f32_cmp_less_b(cdf-probability,top_p) &
                (unbounded | v_f32_cmp_less_b(convert_int64_to_float64(lanes+col,0),top_k));
            retained+=v_i32_mov_vb(1,0,0,keep,0);
            v_f32_st_tnsr(at,filtered,v_f32_mov_vb(probability,0,0,keep,0));
            // Shifted readonly loads avoid negative addresses and preserve
            // exactly the parent's adjacent-value tie test at tile boundaries.
            const int count=columns-col-1<64?columns-col-1:64;
            if(count>0) {
                const int5 next={col+1,row};
                const float64 next_probability=v_f32_ld_tnsr_partial_b(next,probabilities,count-1,0);
                const float64 next_cdf=v_f32_ld_tnsr_partial_b(next,cumulative,count-1,0);
                const float64 next_value=v_f32_ld_tnsr_partial_b(next,values,count-1,0);
                const float64 previous=v_f32_ld_tnsr_b(at,values);
                const bool64 next_keep=v_f32_cmp_less_b(next_cdf-next_probability,top_p) &
                    (unbounded | v_f32_cmp_less_b(convert_int64_to_float64(lanes+col+1,0),top_k));
                const bool64 equal=v_f32_cmp_eq_b(next_value,previous) & next_keep &
                    v_i32_cmp_less_b(lanes,count);
                ties+=v_i32_mov_vb(1,0,0,equal,0);
            }
        }
        retained=v_i32_max_b(sampling_sum(retained),1);
        uint64 boundary_bits=0;
        for(int col=0;col<columns;col+=64) {
            const uint64 bits=as_uint64(v_f32_ld_tnsr_b((int5){col,row},values));
            boundary_bits|=v_u32_mov_vb(bits,0,0,v_i32_cmp_eq_b(lanes+col,retained-1),0);
        }
        const float64 boundary=as_float64(sampling_sum(as_int64(boundary_bits)));
        bool64 covered=v_i32_cmp_eq_b(sampling_sum(ties),0);
        for(int rank=0;rank<ranks;++rank) {
            const float cut=s_f32_ld_g(gen_addr((int5){rank,row},cuts));
            covered=covered & v_f32_cmp_grt_b(boundary,cut);
        }
        covered=covered & v_u32_cmp_neq_b((uint64)(zbits&0x7f800000),0x7f800000) &
            v_f32_cmp_grt_b((float64)z,0) &
            (v_f32_cmp_less_b((float64)top_p,1) | v_f32_cmp_grt_b((float64)top_k,0));
        if(temperature==0)covered=v_i32_cmp_eq_b((int64)0,0);
        v_i32_st_tnsr_partial((int5){0,row},coverage,v_i32_mov_vb(1,0,0,covered,0),0,0);
    }
}
