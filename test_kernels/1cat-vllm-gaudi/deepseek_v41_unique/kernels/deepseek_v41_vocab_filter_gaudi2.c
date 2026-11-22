// SPDX-License-Identifier: Apache-2.0
// Scan each score once into a compact bitmap. No scalar/vector local-memory
// interchange: the real producer output carries the emission dependency.
#include "deepseek_v41_selection_bitmap.h"
void main(tensor scores,tensor cutoffs,tensor masks,int columns) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[1];row<end[1];++row) {
        const float cutoff=s_f32_ld_g(gen_addr((int5){0,row/8},cutoffs));
        for(int block=begin[0];block<end[0];++block) {
            const int col=block*128;
            const float64 a=v_f32_ld_tnsr_b((int5){col,row},scores);
            const float64 b=v_f32_ld_tnsr_b((int5){col+64,row},scores);
            const uint64 bits=pack_words(pack_half(v_f32_cmp_grt_b(a,cutoff)&v_i32_cmp_less_b(lanes+col,columns)),
                pack_half(v_f32_cmp_grt_b(b,cutoff)&v_i32_cmp_less_b(lanes+col+64,columns)));
            v_u32_st_tnsr_partial((int5){block*4,row},masks,bits,3,0);
        }
    }
}
