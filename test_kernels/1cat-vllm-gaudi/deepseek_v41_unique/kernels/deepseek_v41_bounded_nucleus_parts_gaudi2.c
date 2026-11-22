// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
static inline float64 sum_all(float64 x) {
    x=v_f32_reduce_add(x);return v_f32_shuffle_b(x,(uchar256)0x80,0,x);
}
static inline int64 count_all(int64 x) {
    x=v_i32_reduce_add(x);return v_i32_shuffle_b(x,(uchar256)0x80,0,x);
}
void main(tensor values,tensor ids,tensor maximum,tensor total,tensor controls,
          tensor omissions,tensor normalized,tensor information) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        const float max=s_f32_ld_g(gen_addr((int5){0,row},maximum));
        const float mass=s_f32_ld_g(gen_addr((int5){0,row},total));
        const float temperature=s_f32_ld_g(gen_addr((int5){0,row},controls));
        const float top_p=s_f32_ld_g(gen_addr((int5){1,row},controls));
        const float uniform=s_f32_ld_g(gen_addr((int5){2,row},controls));
        const float top_k=s_f32_ld_g(gen_addr((int5){3,row},controls));
        const float omitted=s_f32_ld_g(gen_addr((int5){0,row},omissions));
        const float64 scores=v_f32_ld_tnsr_b((int5){0,row},values);
        const int64 selected_ids=v_i32_ld_tnsr_b((int5){0,row},ids);
        const float64 probability=v_exp_cephes_f32(scores-max)/mass;
        float64 cumulative=0,previous=0;
        bool64 ties=v_i32_cmp_less_b(lanes,0);
        // Reductions remain wholly vector-side: TPC has no scalar reads from
        // vector local memory. Small candidate rows never traverse HBM here.
        for(int lane=0;lane<64;++lane) {
            const bool64 at=v_i32_cmp_eq_b(lanes,lane);
            const float64 prefix=sum_all(v_f32_mov_vb(probability,0,(float64)0,
                                                       v_i32_cmp_leq_b(lanes,lane),0));
            cumulative=v_f32_mov_vb(prefix,0,cumulative,at,0);
            const float64 current=sum_all(v_f32_mov_vb(scores,0,(float64)0,at,0));
            if(lane>0)ties=ties|(at&v_f32_cmp_eq_b(current,previous));
            previous=current;
        }
        const bool64 keep=v_f32_cmp_less_b(cumulative-probability,top_p)&
            (v_f32_cmp_leq_b((float64)top_k,0)|v_f32_cmp_less_b(convert_int64_to_float64(lanes,0),top_k));
        const float64 retained=v_f32_mov_vb(probability,0,(float64)0,keep,0);
        const float64 kept_mass=sum_all(retained);
        v_f32_st_tnsr((int5){0,row},normalized,retained/kept_mass);
        const float64 threshold=kept_mass*uniform;
        int64 winner=count_all(v_i32_mov_vb((int64)1,0,(int64)0,v_f32_cmp_less_b(cumulative,threshold),0));
        winner=v_i32_min_b(winner,63);
        int64 token=v_i32_reduce_min(v_i32_mov_vb(selected_ids,0,(int64)2147483647,
                                               v_i32_cmp_eq_b(lanes,winner),0));
        token=v_i32_shuffle_b(token,(uchar256)0x80,0,token);
        const float64 boundary=v_f32_reduce_min(v_f32_mov_vb(scores,0,(float64)(1.0f/0.0f),keep,0));
        const int64 tie_count=count_all(v_i32_mov_vb((int64)1,0,(int64)0,ties&keep,0));
        const float64 candidate_mass=sum_all(probability);
        const bool64 certified=v_f32_cmp_less_b((float64)omitted,boundary)&
            v_f32_cmp_geq_b(candidate_mass,top_p+1e-6f)&v_i32_cmp_eq_b(tie_count,0)&
            v_f32_cmp_grt_b((float64)mass,0)&v_f32_cmp_less_b((float64)mass,(float)(1.0f/0.0f))&
            v_f32_cmp_grt_b((float64)temperature,0)&v_f32_cmp_less_b((float64)top_p,1);
        v_i32_st_tnsr_partial((int5){0,row},information,token,0,0);
        v_i32_st_tnsr_partial((int5){1,row},information,
                             v_i32_mov_vb((int64)1,0,(int64)0,certified,0),0,0);
    }
}
