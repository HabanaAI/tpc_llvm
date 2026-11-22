// SPDX-License-Identifier: Apache-2.0
// Consume physical [16 bins, partitions, rows] directly. No transpose recipe.
#ifndef DSV41_WEIGHTED_DIRECT_LANE
#define DSV41_WEIGHTED_DIRECT_LANE 0
#endif
#pragma clang fp contract(off)
void main(tensor bins,tensor prefixes,tensor targets,tensor next_prefix,tensor next_target,int shift) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int partitions=get_dim_size(bins,1);
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        const unsigned int prefix=(unsigned int)s_i32_ld_g(gen_addr((int5){0,row},prefixes));
        const float target=s_f32_ld_g(gen_addr((int5){0,row},targets));
        float64 histogram=0;
        for(int part=0;part<partitions;++part)
            histogram+=v_f32_ld_tnsr_b((int5){0,part,row},bins);
        float64 mass=0,remaining=target;
        int64 chosen=0;
        bool64 found=v_i32_cmp_less_b(lanes,0);
        for(int bin=15;bin>=0;--bin) {
#if DSV41_WEIGHTED_DIRECT_LANE
            const float64 weight=v_f32_shuffle_b(histogram,(uchar256)(0x80|bin),0,histogram);
#else
            float64 weight=v_f32_reduce_add(v_f32_mov_vb(histogram,0,(float64)0,
                                                       v_i32_cmp_eq_b(lanes,bin),0));
            weight=v_f32_shuffle_b(weight,(uchar256)0x80,0,weight);
#endif
            const float64 next=mass+weight;
            const bool64 at=(v_f32_cmp_geq_b(next,target)|v_i32_cmp_eq_b((int64)bin,0))&~found;
            chosen=v_i32_mov_vb((int64)bin,0,chosen,at,0);
            remaining=v_f32_mov_vb((float64)target-mass,0,remaining,at,0);
            found=found|at;mass=next;
        }
        const int64 next=chosen<<shift;
        v_i32_st_tnsr_partial((int5){0,row},next_prefix,next|(int)prefix,0,0);
        v_f32_st_tnsr_partial((int5){0,row},next_target,remaining,0,0);
    }
}
