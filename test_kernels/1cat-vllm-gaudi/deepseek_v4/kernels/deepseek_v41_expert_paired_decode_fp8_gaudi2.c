// SPDX-License-Identifier: Apache-2.0
// One affine workpoint domain covers both matrices of a routed pair.
// W13's N tile and K tile map to W2 with exact checkpoint-derived ratios;
// the compiler can keep paired BatchGemm and slice both outputs in SRAM.
#define DSV41_N256_PREFETCH 16
#define DSV41_N256_SAT_DECODE 1
#define DSV41_N256_PAIRED_FUNCTION 1
#include "deepseek_v41_expert_n256.h"
static inline void decode_short_span(tensor ids,tensor q,tensor planes,tensor lookup,tensor output,
                                    int route,int block,int first_k,int count) {
    const int expert=s_i32_ld_g(gen_addr((int5){route},ids));
    if(expert<0 || expert>=get_dim_size(q,2)) {
        for(int k=first_k;k<first_k+count;++k)
            v_f8_st_tnsr((int5){block*256,k,route},output,(minifloat256){0});
        return;
    }
    const bool compact=get_dim_size(planes,0)==get_dim_size(q,0)/16+128;
    const uchar256 raw=v_u8_ld_tnsr_b((int5){0},lookup);
    const uchar256 table=v_u8_sel_eq_u8_b(raw,127,0,raw+48,SW_MASK_EQ_ZERO);
    const uchar256 channel=v_u8_ld_tnsr_b((int5){get_dim_size(planes,0)-128,block,expert},planes,
                                          0,(uchar256){0},compact);
    const int offset=(first_k/32)*(compact?128:256)+(compact?0:128);
    const uchar256 stored=v_u8_ld_tnsr_b((int5){offset,block,expert},planes);
    const uchar256 delta=compact?(stored-channel)<<3:stored;
    const uchar256 subtract=(uchar256)48-delta;
    uchar256 pending[32];
    #pragma loop_unroll(4)
    for(int i=0;i<count;++i)pending[i]=v_u8_ld_tnsr_b(
        (int5){(first_k+i)*64,block,expert},q,SW_UNPACK|SW_UNPCK_4_TO_8);
    #pragma loop_unroll(4)
    for(int i=0;i<count;++i) {
        const uchar256 encoded=decode_sat_values(pending[i],table,subtract);
        v_f8_st_tnsr((int5){block*256,first_k+i,route},output,*((minifloat256*)&encoded));
    }
}
void main(tensor ids,tensor q13,tensor s13,tensor q2,tensor s2,tensor lookup,tensor w13,tensor w2) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int n13=get_dim_size(q13,1),n2=get_dim_size(q2,1);
    const int n_ratio=n2/n13,k2_tile=(get_dim_size(q2,0)/64)/(get_dim_size(q13,0)/8192);
    for(int tile=start[0];tile<end[0];++tile)for(int k=start[2];k<end[2];++k) {
        for(int route=start[1]*2;route<end[1]*2;++route) {
            const int5 a={tile,route,k};
            decode_paired_half(ids,q13,s13,lookup,w13,a,a+(int5){1,1,1,0,0});
            for(int n=tile*n_ratio;n<(tile+1)*n_ratio;++n)
                decode_short_span(ids,q2,s2,lookup,w2,route,n,k*k2_tile,k2_tile);
        }
    }
}
