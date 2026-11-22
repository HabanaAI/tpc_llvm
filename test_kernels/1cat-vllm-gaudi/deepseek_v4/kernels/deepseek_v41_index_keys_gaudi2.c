// SPDX-License-Identifier: Apache-2.0
// Reuse the C1 UE8M0 index-key codec, but decode each selected key once for
// all MME query heads. No floating point dot product runs in this kernel.
static inline bfloat128 index_key(tensor cache, int row) {
    const uchar256 lanes = V_LANE_ID_8;
    uchar256 raw = v_u8_ld_tnsr_partial_b((int5){0,row}, cache, 63, 0);
    raw = v_u8_mov_dual_group_all_b(raw, 0xffffffff, 0,0,0,0, MkWrA(3,3,3,3), (uchar256){0});
    const uchar256 expanded = v_u8_shuffle_b(raw, (lanes >> 1) | 0x80, 0, (uchar256){0});
    const ushort128 codes = (convert_uchar256_to_ushort256(expanded, SW_LINEAR).v1 >>
                             (((ushort128)V_LANE_ID_16 & 1) << 2)) & 15;
    const ushort128 magnitude = codes & 7;
    ushort128 bits = (magnitude << 6) + 0x3f00;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, 0x3f00, bits);
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits) | ((codes & 8) << 12);
    uchar256 scales = v_u8_ld_tnsr_partial_b((int5){64,row}, cache, 3, 0);
    scales = v_u8_mov_dual_group_all_b(scales, 0xffffffff, 0,0,0,0, MkWrA(3,3,3,3), (uchar256){0});
    scales = v_u8_shuffle_b(scales, (lanes >> 5) | 0x80, 0, (uchar256){0});
    ushort128 scale = convert_uchar256_to_ushort256(scales, SW_LINEAR).v1 << 7;
    // Match the production decode scorer exactly.  The cache producer has
    // already encoded the UE8M0 byte contract; this gather must not reinterpret
    // 255 as NaN or synthesize a signed underflow that the scorer never emits.
    scale = v_u16_sel_eq_u16_b(scale, 0, 0, scale);
    bfloat128 result = v_bf16_mul_b(*((bfloat128*)&bits), *((bfloat128*)&scale));
    return v_bf16_sel_eq_bf16_b(result, (bfloat)0, (bfloat)0, result);
}


void main(tensor cache, tensor pages, tensor rows,
#ifdef DSV41_INDEX_WRITE_COMPLETION
          tensor completion,
#endif
          tensor output,
          int ratio) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    const int page_rows=128/ratio;
    const int page_count=get_dim_size(pages,0), cache_rows=get_dim_size(cache,1);
    for(int batch=begin[1];batch<end[1];++batch) {
        for(int column=begin[0];column<end[0];++column) {
                const int logical=s_i32_ld_g(gen_addr((int5){column,batch},rows));
                bfloat128 key=0;
                if(logical>=0 && logical<page_count*page_rows
#ifdef DSV41_INDEX_WRITE_COMPLETION
                   && s_i32_ld_g(gen_addr((int5){0},completion))>=-1
#endif
                ) {
                    const int request=get_dim_size(pages,1)>1?batch:0;
                    const int page=s_i32_ld_g(gen_addr((int5){logical/page_rows,request},pages));
                    const int offset=logical%page_rows;
                    // Reject invalid page IDs before multiplying signed row coordinates.
                    if(page>=0 && offset<cache_rows && page<=(cache_rows-1-offset)/page_rows) {
                        const int physical=page*page_rows+offset;
                        key=index_key(cache,physical);
                    }
                }
                v_bf16_st_tnsr((int5){0,column,batch},output,key);
        }
    }
}
