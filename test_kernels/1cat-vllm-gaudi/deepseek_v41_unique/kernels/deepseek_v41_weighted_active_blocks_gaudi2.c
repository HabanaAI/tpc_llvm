// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_selection_bitmap.h"
// Vector loads/reductions produce a real dependency bitmap; no VLM/SLM alias.
void main(tensor bounds,tensor prefixes,tensor active_blocks,int shift) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const uint64 lanes=(uint64)V_LANE_ID_32;
    const unsigned mask=shift==28?0u:0xffffffffu<<(shift+4);
    const int tiles=get_dim_size(bounds,0);
    for(int row=begin[1];row<end[1];++row) {
        const unsigned prefix=s_u32_ld_g(gen_addr((int5){0,row},prefixes));
        for(int part=begin[0];part<end[0];++part) {
            // Store low/high as contiguous planes. A word shuffle cannot
            // freely select across Gaudi2 dual groups.
            const uint64 low=v_u32_ld_tnsr_b((int5){part*32,0,row},bounds);
            const uint64 high=v_u32_ld_tnsr_b((int5){part*32,1,row},bounds);
            const bool64 active=v_u32_cmp_leq_b(low,high)&v_u32_cmp_geq_b(high,prefix)&
                v_u32_cmp_leq_b(low,prefix|~mask)&v_u32_cmp_less_b(lanes,32)&
                v_u32_cmp_less_b(lanes+part*32,tiles);
            const uint64 bitmap=pack_half(active);
            v_u32_st_tnsr_partial((int5){part,row},active_blocks,bitmap,0,0);
        }
    }
}
