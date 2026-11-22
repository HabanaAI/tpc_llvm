// SPDX-License-Identifier: Apache-2.0
// Exact predicate packing for the BF16 ordered selection bitmap.
static inline uint64 pack128(bool128 predicate) {
    const uint64 nibble=v_u32_mov_i1_b(from_bool128(predicate));
    const uint64 pair=(nibble&1)|((nibble>>1)&2);
    const uint64 lane=(uint64)V_LANE_ID_32;
    uint64 bits=pair<<((lane&15)*2);
    bits|=v_u32_mov_group_b(bits,0xffffffff,63,0);
    #pragma loop_unroll(3)
    for(int shift=4;shift>0;shift>>=1) {
        const uint64 mask=(((lane&7)^shift)|0x80)*0x01010101;
        bits|=v_u32_shuffle_b(bits,*((const uchar256*)&mask),0,0);
    }
    uint64 words=v_u32_mov_dual_group_b(bits,0x000000f0,1,0,SW_WR_LOWER_GROUP,bits);
    words=v_u32_mov_dual_group_b(bits,0x00000f00,2,0,SW_WR_LOWER_GROUP,words);
    return v_u32_mov_dual_group_b(bits,0x0000f000,3,0,SW_WR_LOWER_GROUP,words);
}
static inline void store_bits_packed(tensor metadata,int request,int tile,int word_capacity,
                              ushort128 keys,ushort128 threshold) {
    const bool128 live=v_u16_cmp_grt_b(keys,0);
    v_u32_st_tnsr_partial((int5){50+tile*4,request},metadata,
                          pack128(live&v_u16_cmp_grt_b(keys,threshold)),3,0);
    v_u32_st_tnsr_partial((int5){50+word_capacity+tile*4,request},metadata,
                          pack128(live&v_u16_cmp_eq_b(keys,threshold)),3,0);
}
