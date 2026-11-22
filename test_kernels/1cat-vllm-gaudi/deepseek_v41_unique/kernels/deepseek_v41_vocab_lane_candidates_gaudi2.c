// SPDX-License-Identifier: Apache-2.0
// One vector scan, two retained scores per lane. The maximum third score is
// an upper bound on every omission; the nucleus certificate must pass before
// these candidates are committed. No scalar score reloads or local spills.
static inline float64 max_all(float64 value) {
    value=v_f32_max_b(value,v_f32_mov_dual_group_all_b(value,0xffffffff,1,0,3,2,MkWrA(3,3,3,3),0));
    value=v_f32_max_b(value,v_f32_mov_dual_group_all_b(value,0xffffffff,2,3,0,1,MkWrA(3,3,3,3),0));
    value=v_f32_max_b(value,v_f32_mov_group_b(value,0xffffffff,63,0));
    #pragma loop_unroll(3)
    for(int shift=4;shift>0;shift>>=1) {
        const uint64 mask=((((uint64)V_LANE_ID_32&7)^shift)|0x80)*0x01010101;
        value=v_f32_max_b(value,v_f32_shuffle_b(value,*((const uchar256*)&mask),0,value));
    }
    return value;
}
void main(tensor scores,tensor values,tensor ids,tensor omissions,int columns) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        float64 first=(float64)(-1.0f/0.0f),second=first,third=first;
        int64 first_id=0,second_id=0;
        for(int col=0;col<columns;col+=64) {
            const float64 x=v_f32_ld_tnsr_b((int5){col,row},scores);
            const bool64 before_first=v_f32_cmp_grt_b(x,first);
            const float64 rest=v_f32_mov_vb(first,0,x,before_first,0);
            const int64 rest_id=v_i32_mov_vb(first_id,0,lanes+col,before_first,0);
            const bool64 before_second=v_f32_cmp_grt_b(rest,second);
            third=v_f32_max_b(third,v_f32_mov_vb(second,0,rest,before_second,0));
            second=v_f32_mov_vb(rest,0,second,before_second,0);
            second_id=v_i32_mov_vb(rest_id,0,second_id,before_second,0);
            first=v_f32_mov_vb(x,0,first,before_first,0);
            first_id=v_i32_mov_vb(lanes+col,0,first_id,before_first,0);
        }
        v_f32_st_tnsr((int5){0,row},values,first);
        v_f32_st_tnsr((int5){64,row},values,second);
        v_i32_st_tnsr((int5){0,row},ids,first_id);
        v_i32_st_tnsr((int5){64,row},ids,second_id);
        v_f32_st_tnsr_partial((int5){0,row},omissions,max_all(third),0,0);
    }
}
