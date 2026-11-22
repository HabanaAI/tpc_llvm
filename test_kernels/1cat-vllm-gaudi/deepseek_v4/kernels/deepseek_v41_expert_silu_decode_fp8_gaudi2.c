// SPDX-License-Identifier: Apache-2.0
// Keep the complete SwiGLU numerical boundary with the W2 weight producer.
// Only the N/K origin writes the small activation row; all other index tiles
// emit disjoint decoded weights. No tile reads another tile's new output.
#define DSV41_N256_PREFETCH 16
#define DSV41_N256_SAT_DECODE 1
#define DSV41_N256_SLOT_TILE (get_dim_size(ids,0)==3?3:2)
#define DSV41_N256_PAIRED_FUNCTION 1
#include "deepseek_v41_expert_n256.h"
#define DSV41_SILU_QUANT_FUNCTION 1
#include "deepseek_v41_expert_n256_silu_quant_gaudi2.c"
void main(tensor ids, tensor q16, tensor planes, tensor lookup, tensor product,
          tensor activation_scale, tensor channel, tensor router,
          tensor weights, tensor activation, tensor scales) {
    const int5 begin=get_index_space_offset();
    const int5 end=begin+get_index_space_size();
    decode_paired_half(ids,q16,planes,lookup,weights,begin,end);
    if (begin[0]==0 && begin[2]==0) {
        const int route_tile=get_dim_size(ids,0)==3?3:2;
        const int last=s_i32_min(end[1]*route_tile,get_dim_size(ids,0));
        silu_quant_rows(product,ids,activation_scale,channel,router,activation,scales,
                        (int5){begin[1]*route_tile},(int5){last});
    }
}
