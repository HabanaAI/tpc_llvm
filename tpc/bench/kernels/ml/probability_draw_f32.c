// SPDX-License-Identifier: Apache-2.0
// Sampling a token per row from a probability vector by inverse CDF in
// vocabulary order, from DeepSeek-V4.1 decoding: target = u * sum(partials); a
// scalar scan over the partial sums of 2048-column blocks picks a block, and a
// vector scan of that block (a 64-lane prefix sum in 6 shift-and-add steps, a
// lane sum carried from vector to vector) finds the first column whose
// cumulative mass reaches the target, with a flag for a target safely away
// from the column boundaries. f32 without contraction; vocabulary 131072 (64
// blocks), 8 rows, one row per member, u in controls[2]. The probabilities
// are uniform with mean 1/131072 and every partial is 1/64, so the partials
// match the block sums to about 1%; the drawn column is almost always found.
// From 1cat-vllm-gaudi (https://github.com/1CatAI/1cat-vllm-gaudi, commit e2266e0e),
// csrc/deepseek_v41_unique/kernels/deepseek_v41_probability_draw_gaudi2.c; Apache-2.0.
//
// BENCH: arch gaudi2
// BENCH: in  f32 131072 8 range=0,0.0000152587890625
// BENCH: in  f32 64 8 range=0.015625,0.015625
// BENCH: in  f32 3 8 range=0,1
// BENCH: out i32 2 8
// BENCH: index-space 8

// A vocabulary-order inverse CDF. Its distribution is the supplied q, not
// a bounded approximation. Main and exact repair share this same draw map.
#pragma clang fp contract(off)
void main(tensor probability,tensor partials,tensor controls,tensor information) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(probability,0),parts=get_dim_size(partials,0);
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        float total=0;
        for(int part=0;part<parts;++part)total+=s_f32_ld_g(gen_addr((int5){part,row},partials));
        const float target=s_f32_ld_g(gen_addr((int5){2,row},controls))*total;
        float preceding=0;
        int selected_part=parts-1;
        for(int part=0;part<parts;++part) {
            const float mass=s_f32_ld_g(gen_addr((int5){part,row},partials));
            if(preceding+mass>=target && mass>0) {selected_part=part;break;}
            preceding+=mass;
        }
        float64 previous=preceding,margin=1.0e30f;
        int64 selected=columns;
        const int first=selected_part*2048,last=first+2048<columns?first+2048:columns;
        for(int col=first;col<last;col+=64) {
            float64 value=v_f32_ld_tnsr_b((int5){col,row},probability);
            const bool64 valid=v_i32_cmp_less_b(lanes+col,columns);
            value=v_f32_mov_vb(value,0,(float64)0,valid,0);
            float64 prefix=value;
            #pragma unroll
            for(int shift=1;shift<64;shift*=2) {
                const float64 moved=v_element_shift_up_f32(prefix,shift);
                prefix+=v_f32_mov_vb(moved,0,(float64)0,v_i32_cmp_geq_b(lanes,shift),0);
            }
            const float64 cumulative=previous+prefix;
            const bool64 positive=valid&v_f32_cmp_grt_b(value,0.0f);
            selected=v_i32_min_b(selected,v_i32_mov_vb(lanes+col,0,(int64)columns,
                                            positive&v_f32_cmp_geq_b(cumulative,target),0));
            margin=v_f32_min_b(margin,v_f32_mov_vb(v_f32_abs_b(cumulative-target),0,
                                                (float64)1.0e30f,positive,0));
            float64 mass=v_f32_reduce_add(value);
            previous+=v_f32_shuffle_b(mass,(uchar256)0x80,0,mass);
        }
        selected=v_i32_reduce_min(selected);
        margin=v_f32_reduce_min(margin);
        const int64 safe=v_i32_mov_vb((int64)1,0,(int64)0,
            v_f32_cmp_grt_b(margin,2.0e-6f*total)&v_i32_cmp_less_b(selected,columns)&
            v_f32_cmp_grt_b((float64)total,0.0f),0);
        v_i32_st_tnsr_partial((int5){0,row},information,selected,0,0);
        v_i32_st_tnsr_partial((int5){1,row},information,safe,0,0);
    }
}
