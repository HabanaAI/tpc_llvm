// SPDX-License-Identifier: Apache-2.0
// Decode each selected row once, using a full BF16 vector and row scale reuse.
#ifndef DSV41_SELECTED_FULL_ROW_LOAD
#define DSV41_SELECTED_FULL_ROW_LOAD 0
#endif
#ifndef DSV41_SELECTED_FP4_TABLE
#define DSV41_SELECTED_FP4_TABLE 0
#endif
#ifndef DSV41_SELECTED_ROW_CACHE
#define DSV41_SELECTED_ROW_CACHE 0
#endif
#ifndef DSV41_SELECTED_SCALE_CACHE
#define DSV41_SELECTED_SCALE_CACHE 0
#endif
#ifndef DSV41_LOGICAL_MAIN_MIRROR
#define DSV41_LOGICAL_MAIN_MIRROR 0
#endif
#ifndef DSV41_SELECTED_FP8_BRIDGE
#define DSV41_SELECTED_FP8_BRIDGE 0
#endif
#ifndef DSV41_SELECTED_FP8_BITS
#define DSV41_SELECTED_FP8_BITS 0
#endif
#if DSV41_SELECTED_FP8_BRIDGE
static inline bfloat128 selected_e4m3fn_bytes(uchar256 raw) {
    // Reuse the V4 decoder's extended-code correction: Gaudi2 reserves
    // exponent15, so finite E4M3FN120..126 use exponent14 then exact x2.
    // SW_LINEAR retains the original contiguous low128-lane row contract.
    const uchar256 magnitude=raw&127;
    const bool256 extended=v_u8_cmp_geq_b(magnitude,120)&v_u8_cmp_leq_b(magnitude,126);
    const uchar256 adjusted=v_u8_sub_vb(raw,8,0,raw,extended,0);
    const bfloat128 base=convert_minifloat256_to_bfloat256(*((minifloat256*)&adjusted),
                                                        SW_LINEAR|SW_FP8_BIAS7).v1;
    const ushort128 code=convert_uchar256_to_ushort256(magnitude,SW_LINEAR).v1;
    const bool128 extended_low=v_u16_cmp_geq_b(code,120)&v_u16_cmp_leq_b(code,126);
    bfloat128 result=v_bf16_mul_vb(base,2.0f,0,base,extended_low,0);
    ushort128 bits=*((ushort128*)&result);
    bits=v_u16_sel_eq_u16_b(code,0,0,bits);
    bits=v_u16_sel_eq_u16_b(code,127,0x7fff,bits);
    return *((bfloat128*)&bits);
}
#endif
static inline bfloat128 selected_e4m3fn(ushort128 code) {
#if DSV41_SELECTED_FP8_BITS
    const ushort128 magnitude=code&127;
    const ushort128 mantissa=magnitude&7;
    ushort128 tiny=(mantissa<<5)+0x3b80;
    tiny=v_u16_sel_less_u16_b(mantissa,4,(mantissa<<6)+0x3b00,tiny);
    tiny=v_u16_sel_less_u16_b(mantissa,2,0x3b00,tiny);
    // Normal FP8 exponent/mantissa bits are contiguous within BF16.
    // Add the exponent bias once, rather than extract/recombine both fields.
    ushort128 bits=(magnitude<<4)+0x3c00;
    bits=v_u16_sel_less_u16_b(magnitude,8,tiny,bits);
    bits|=(code&128)<<8;
    bits=v_u16_sel_eq_u16_b(magnitude,0,0,bits);
    bits=v_u16_sel_eq_u16_b(magnitude,127,0x7fff,bits);
    return *((bfloat128*)&bits);
#else
    const ushort128 exponent = (code >> 3) & 15;
    const ushort128 mantissa = code & 7;
    ushort128 tiny = (mantissa << 5) + 0x3b80;
    tiny = v_u16_sel_less_u16_b(mantissa, 4, (mantissa << 6) + 0x3b00, tiny);
    tiny = v_u16_sel_eq_u16_b(mantissa, 1, 0x3b00, tiny);
    tiny = v_u16_sel_eq_u16_b(mantissa, 0, 0, tiny);
    ushort128 bits = ((exponent + 120) << 7) | (mantissa << 4);
    bits = v_u16_sel_eq_u16_b(exponent, 0, tiny, bits);
    bits |= (code & 128) << 8;
    bits = v_u16_sel_eq_u16_b(code & 127, 0, 0, bits);
    bits = v_u16_sel_eq_u16_b(code & 127, 127, 0x7fff, bits);
    return *((bfloat128*)&bits);
#endif
}

static inline bfloat128 selected_ue8m0(ushort128 code) {
    ushort128 bits = code << 7;
    // The reference F32 multiply flushes the UE8M0 zero-code subnormal input.
    bits = v_u16_sel_eq_u16_b(code, 0, 0, bits);
    bits = v_u16_sel_eq_u16_b(code, 255, 0x7fff, bits);
    return *((bfloat128*)&bits);
}

static inline bfloat128 selected_fp4(ushort128 code) {
    const ushort128 magnitude = code & 7;
    ushort128 bits = (magnitude << 6) + 0x3f00;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, 0x3f00, bits);
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
    bits |= (code & 8) << 12;
    return *((bfloat128*)&bits);
}

#if !defined(DSV41_SELECTED_CODEC_ONLY)
#if defined(DSV41_DIRECT_MLA_KV) || defined(DSV41_LOGICAL_MLA_OPERANDS)
// Materialize the same rounded BF16 values for QK and widened FP32 values
// for PV. The compiler owns operand placement; inspect the compiled graph.
static inline void selected_store(int chunk, int slot, int token, tensor rows,
                                  tensor values, bfloat128 value) {
    const int offset = chunk * 128;
    v_bf16_st_tnsr((int5){offset, slot, token}, rows, value);
#ifndef DSV41_MLA_BF16_KV_ONLY
#if defined(DSV41_MLA_FP16_VALUES) && DSV41_MLA_FP16_VALUES
    v_f16_st_tnsr((int5){offset, slot, token}, values,
        convert_bfloat128_to_half128(value, SW_RHNE | SW_LINEAR));
#else
    const float128 rounded = convert_bfloat128_to_float128(value, SW_LINEAR);
    v_f32_st_tnsr((int5){offset, slot, token}, values, rounded.v1);
    v_f32_st_tnsr((int5){offset + 64, slot, token}, values, rounded.v2);
#endif
#endif
}
#if DSV41_SELECTED_ROW_CACHE
#define STORE_SELECTED(CHUNK, VALUE) do { \
    const bfloat128 stored_value=(VALUE); \
    if(cache_destination==0 && (CHUNK)==0)cache00=stored_value; \
    if(cache_destination==0 && (CHUNK)==1)cache01=stored_value; \
    if(cache_destination==0 && (CHUNK)==2)cache02=stored_value; \
    if(cache_destination==0 && (CHUNK)==3)cache03=stored_value; \
    if(cache_destination==1 && (CHUNK)==0)cache10=stored_value; \
    if(cache_destination==1 && (CHUNK)==1)cache11=stored_value; \
    if(cache_destination==1 && (CHUNK)==2)cache12=stored_value; \
    if(cache_destination==1 && (CHUNK)==3)cache13=stored_value; \
    if(cache_destination==2 && (CHUNK)==0)cache20=stored_value; \
    if(cache_destination==2 && (CHUNK)==1)cache21=stored_value; \
    if(cache_destination==2 && (CHUNK)==2)cache22=stored_value; \
    if(cache_destination==2 && (CHUNK)==3)cache23=stored_value; \
    if(DSV41_SELECTED_ROW_CACHE>=4 && cache_destination==3 && (CHUNK)==0)cache30=stored_value; \
    if(DSV41_SELECTED_ROW_CACHE>=4 && cache_destination==3 && (CHUNK)==1)cache31=stored_value; \
    if(DSV41_SELECTED_ROW_CACHE>=4 && cache_destination==3 && (CHUNK)==2)cache32=stored_value; \
    if(DSV41_SELECTED_ROW_CACHE>=4 && cache_destination==3 && (CHUNK)==3)cache33=stored_value; \
    selected_store(CHUNK,slot,token,rows,values,stored_value); \
} while(0)
#elif defined(DSV41_LOGICAL_MLA_EXPORT_MAIN)
#define STORE_SELECTED(chunk, value) do { \
    const bfloat128 published_value=(value); \
    selected_store(chunk,slot,token,rows,values,published_value); \
    v_bf16_st_tnsr((int5){(chunk)*128,slot,token},shared_rows,published_value); \
} while(0)
#elif defined(DSV41_MLA_SINGLE_BANK)
#define STORE_SELECTED(chunk, value) selected_store(chunk, slot, token, rows, rows, value)
#else
#define STORE_SELECTED(chunk, value) selected_store(chunk, slot, token, rows, values, value)
#endif
#ifdef DSV41_LOGICAL_MLA_OPERANDS
void main(tensor swa, tensor main_cache, tensor selection, tensor positions,
          tensor pages, tensor lengths,
#if defined(DSV41_DECODED_LOGICAL_MLA) || defined(DSV41_LOGICAL_WRITE_COMPLETION) || defined(DSV41_SELECTED_SWA_CACHED)
          tensor completion,
#endif
          tensor rows,
#ifndef DSV41_MLA_SINGLE_BANK
          tensor values,
#endif
          tensor mask,
#ifdef DSV41_LOGICAL_MLA_EXPORT_MAIN
          tensor shared_rows,
#endif
          int ratio) {
#else
void main(tensor swa, tensor main_cache, tensor indices, tensor selected,
          tensor lengths, tensor rows, tensor values, tensor mask) {
#endif
#else
#define STORE_SELECTED(chunk, value) v_bf16_st_tnsr((int5){chunk * 128, slot, 0, 0, 0}, rows, value)
void main(tensor swa, tensor main_cache, tensor indices,
#ifdef DSV41_KV_WRITE_DEPENDENCY
          tensor completion,
#endif
#ifdef DSV41_COMPRESS_WRITE_DEPENDENCY
          tensor compressed_completion,
#endif
          tensor rows, tensor local_indices) {
#endif
#if !defined(DSV41_DECODED_LOGICAL_MLA) && !defined(DSV41_LOGICAL_WRITE_COMPLETION) && !defined(DSV41_SELECTED_SWA_CACHED)
#ifdef DSV41_KV_WRITE_DEPENDENCY
    const bool swa_ready = s_i32_ld_g(gen_addr((int5){0}, completion)) >= 0;
#else
    const bool swa_ready = 1;
#endif
#endif
#if defined(DSV41_DECODED_LOGICAL_MLA) || defined(DSV41_LOGICAL_WRITE_COMPLETION) || defined(DSV41_SELECTED_SWA_CACHED)
    const bool ready = s_i32_ld_g(gen_addr((int5){0}, completion)) >= -1;
#elif defined(DSV41_COMPRESS_WRITE_DEPENDENCY)
    const bool ready = swa_ready && s_i32_ld_g(gen_addr((int5){0}, compressed_completion)) >= 0;
#else
    const bool ready = swa_ready;
#endif
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if !defined(DSV41_DIRECT_MLA_KV) && !defined(DSV41_LOGICAL_MLA_OPERANDS)
    const int slots = get_dim_size(indices, 0);
#endif
    const int swa_length = get_dim_size(swa, 1);
#if DSV41_LOGICAL_MAIN_MIRROR
    const int main_length = get_dim_size(main_cache, 1);
#elif defined(DSV41_DECODED_LOGICAL_MLA)
    const int main_length = get_dim_size(main_cache, 1);
#else
    const int main_length = get_dim_size(main_cache, 0) == 288 ? get_dim_size(main_cache, 1) : 0;
#endif
#ifndef DSV41_DECODED_LOGICAL_MLA
    const uchar256 lanes = V_LANE_ID_8;
#if !DSV41_LOGICAL_MAIN_MIRROR
#if DSV41_SELECTED_FP4_TABLE
    // Each packed byte becomes four output bytes: the low/high BF16 byte
    // pair of each nibble. The exact 16-entry codec fits each shuffle group.
    const uchar256 fp4_directions = (lanes >> 2) | 0x80;
    const uchar256 nibble_shifts = ((lanes >> 1) & 1) << 2;
    const ushort128 lut_code = (ushort128)V_LANE_ID_16 & 15;
    const bfloat128 lut_values = selected_fp4(lut_code);
    const uchar256 fp4_table = *((uchar256*)&lut_values);
#else
    const uchar256 fp4_directions = (lanes >> 1) | 0x80;
    const ushort128 nibble_shifts = ((ushort128)V_LANE_ID_16 & 1) << 2;
#endif
#endif
#endif
#if defined(DSV41_DIRECT_MLA_KV) || defined(DSV41_LOGICAL_MLA_OPERANDS)
#if defined(DSV41_LOGICAL_MLA_OPERANDS) && defined(DSV41_LOGICAL_COORD_CACHE)
    const int page_shift=ratio==1 ? 7 : 6;
    const int page_width=1<<page_shift;
    const int page_count=get_dim_size(pages,0);
#endif
#if DSV41_SELECTED_ROW_CACHE
    int cache_id0=-1,cache_id1=-1,cache_id2=-1,cache_id3=-1;
    int priority0=-1,priority1=-1,priority2=-1,priority3=-1;
    bfloat128 cache00=0,cache01=0,cache02=0,cache03=0;
    bfloat128 cache10=0,cache11=0,cache12=0,cache13=0;
    bfloat128 cache20=0,cache21=0,cache22=0,cache23=0;
    bfloat128 cache30=0,cache31=0,cache32=0,cache33=0;
    for(int slot=begin[0];slot<end[0];++slot) {
        if(slot==128) {
            cache_id0=cache_id1=cache_id2=cache_id3=-1;
            priority0=priority1=priority2=priority3=-1;
        }
        for(int token=0;token<get_dim_size(selection,1);++token) {
            const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
#else
    for (int token = begin[1]; token < end[1]; ++token) {
        const int length = s_i32_ld_g(gen_addr((int5){token}, lengths));
#if defined(DSV41_LOGICAL_MLA_OPERANDS) && defined(DSV41_LOGICAL_COORD_CACHE)
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        int cached_page=-1,cached_base=0;
#endif
        for (int slot = begin[0]; slot < end[0]; ++slot) {
#endif
#ifdef DSV41_LOGICAL_MLA_OPERANDS
            bool selected_valid = slot < length;
            int index = -1;
            if (slot < 128) {
#ifdef DSV41_LOGICAL_COORD_CACHE
                const int absolute=position-127+slot;
#else
                const int absolute = s_i32_ld_g(gen_addr((int5){token}, positions)) - 127 + slot;
#endif
                selected_valid = selected_valid && absolute >= 0;
                if (selected_valid) index = absolute & 255;
            } else {
                const int logical = s_i32_ld_g(gen_addr((int5){slot - 128, token}, selection));
                selected_valid = selected_valid && logical >= 0;
#if DSV41_LOGICAL_MAIN_MIRROR
                if(selected_valid && logical<main_length) index=logical+swa_length;
#elif defined(DSV41_LOGICAL_COORD_CACHE)
                const int page_id=logical>>page_shift;
                if(selected_valid && page_id<page_count) {
                    if(page_id!=cached_page) {
                        cached_page=page_id;
                        cached_base=s_i32_ld_g(gen_addr((int5){page_id},pages))*page_width;
                    }
                    const int physical=cached_base+(logical&(page_width-1));
                    index=(physical<0?0:physical)+swa_length;
                }
#else
                const int page_width = 128 / ratio;
                if (selected_valid && logical / page_width < get_dim_size(pages, 0)) {
                    const int page = s_i32_ld_g(gen_addr((int5){logical / page_width}, pages));
                    const int physical = page * page_width + (logical & (page_width - 1));
                    index = (physical < 0 ? 0 : physical) + swa_length;
                }
#endif
            }
#else
            const int local = s_i32_ld_g(gen_addr((int5){slot, token}, selected));
            const bool selected_valid = slot < length && local >= 0 && local < get_dim_size(indices, 0);
            const int index = selected_valid ? s_i32_ld_g(gen_addr((int5){local, 0}, indices)) : -1;
#endif
            // Invalid physical addresses still contribute a zero KV row when
            // the local selection is valid, matching the two-gather contract.
            s_f32_st_g(gen_addr((int5){slot, token}, mask), selected_valid ? 1.0f : 0.0f);
#else
    for (int point = begin[0]; point < end[0]; ++point) {
        for (int slot = point; slot < slots; slot += 128) {
            const int index = s_i32_ld_g(gen_addr((int5){slot, 0, 0, 0, 0}, indices));
#endif
            const bool valid = ready && index >= 0 && index < swa_length + main_length;
#if DSV41_SELECTED_ROW_CACHE
            int cache_destination=-1;
            int hit=-1;
            if(valid) {
                if(cache_id0==index)hit=0;
                else if(cache_id1==index)hit=1;
                else if(cache_id2==index)hit=2;
                else if(DSV41_SELECTED_ROW_CACHE>=4 && cache_id3==index)hit=3;
            }
            if(hit>=0) {
                const bfloat128 value0=hit==0?cache00:hit==1?cache10:hit==2?cache20:cache30;
                STORE_SELECTED(0,value0);
                const bfloat128 value1=hit==0?cache01:hit==1?cache11:hit==2?cache21:cache31;
                STORE_SELECTED(1,value1);
                const bfloat128 value2=hit==0?cache02:hit==1?cache12:hit==2?cache22:cache32;
                STORE_SELECTED(2,value2);
                const bfloat128 value3=hit==0?cache03:hit==1?cache13:hit==2?cache23:cache33;
                STORE_SELECTED(3,value3);
                continue;
            }
            if(valid) {
                const int priority=slot<128?s_i32_ld_g(gen_addr((int5){token},positions))-127+slot
                                           :s_i32_ld_g(gen_addr((int5){slot-128,token},selection));
                const int minimum=DSV41_SELECTED_ROW_CACHE>=4
                    ?s_i32_min(s_i32_min(priority0,priority1),s_i32_min(priority2,priority3))
                    :s_i32_min(s_i32_min(priority0,priority1),priority2);
                if(priority>minimum) {
                    if(priority0==minimum){cache_destination=0;priority0=priority;cache_id0=index;}
                    else if(priority1==minimum){cache_destination=1;priority1=priority;cache_id1=index;}
                    else if(priority2==minimum){cache_destination=2;priority2=priority;cache_id2=index;}
                    else if(DSV41_SELECTED_ROW_CACHE>=4){cache_destination=3;priority3=priority;cache_id3=index;}
                }
            }
#endif
#ifdef DSV41_SELECTED_VALID_ONLY
            if (!valid) continue;
#endif
            if (!valid) {
                for (int chunk = 0; chunk < 4; ++chunk)
                    STORE_SELECTED(chunk, (bfloat128){0});
            }
#ifdef DSV41_DECODED_LOGICAL_MLA
            else {
#ifdef DSV41_MLA_SINGLE_BANK_LOOPED
                #pragma nounroll
#endif
                for (int chunk = 0; chunk < 4; ++chunk) {
                    const bfloat128 value = index < swa_length
                        ? v_bf16_ld_tnsr_b((int5){chunk * 128, index}, swa)
                        : v_bf16_ld_tnsr_b((int5){chunk * 128, index - swa_length}, main_cache);
                    STORE_SELECTED(chunk, value);
                }
            }
#else
            else if (index < swa_length) {
#if defined(DSV41_SELECTED_SWA_CACHED) && DSV41_SELECTED_SWA_CACHED
                for (int chunk = 0; chunk < 4; ++chunk)
                    STORE_SELECTED(chunk, v_bf16_ld_tnsr_b((int5){chunk * 128, index}, swa));
#else
                const uchar256 raw_scales = v_u8_ld_tnsr_partial_b((int5){512, index, 0, 0, 0}, swa, 15, 0);
#if !DSV41_SELECTED_SCALE_CACHE
                const uchar256 row_scales = v_u8_mov_dual_group_all_b(
                    raw_scales, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
#endif
#if DSV41_SELECTED_SCALE_CACHE
                const ushort128 scale_codes=convert_uchar256_to_ushort256(raw_scales,SW_LINEAR).v1;
                const bfloat128 decoded_scales=selected_ue8m0(scale_codes);
                const uchar256 scale_bits=v_u8_mov_dual_group_all_b(
                    *((uchar256*)&decoded_scales),0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256){0});
#endif
#ifdef DSV41_MLA_SINGLE_BANK_LOOPED
                #pragma nounroll
#endif
                for (int chunk = 0; chunk < 4; ++chunk) {
                    const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 128, index, 0, 0, 0}, swa, 127, 0);
#if !DSV41_SELECTED_FP8_BRIDGE
                    const ushort128 code = convert_uchar256_to_ushort256(bytes, SW_LINEAR).v1;
#endif
#if DSV41_SELECTED_SCALE_CACHE
                    // One UE8M0 decode per row, then shuffle the two BF16
                    // bytes for each 32-value group. No numerical change.
                    const uchar256 directions=(((lanes>>6)<<1)+(lanes&1)+chunk*8)|0x80;
                    const uchar256 expanded_scale=v_u8_shuffle_b(scale_bits,directions,0,(uchar256){0});
                    const bfloat128 value=v_bf16_mul_b(selected_e4m3fn(code),*((bfloat128*)&expanded_scale));
#else
                    const uchar256 directions = ((lanes >> 5) + chunk * 4) | 0x80;
                    const uchar256 scale_bytes = v_u8_shuffle_b(row_scales, directions, 0, (uchar256){0});
                    const ushort128 scales = convert_uchar256_to_ushort256(scale_bytes, SW_LINEAR).v1;
#if DSV41_SELECTED_FP8_BRIDGE
                    const bfloat128 value=v_bf16_mul_b(selected_e4m3fn_bytes(bytes),selected_ue8m0(scales));
#else
                    const bfloat128 value = v_bf16_mul_b(selected_e4m3fn(code), selected_ue8m0(scales));
#endif
#endif
                    STORE_SELECTED(chunk, value);
                }
#endif
            } else {
                const int row = index - swa_length;
#if DSV41_LOGICAL_MAIN_MIRROR
                for(int chunk=0;chunk<4;++chunk)
                    STORE_SELECTED(chunk,v_bf16_ld_tnsr_b((int5){chunk*128,row},main_cache));
#else
                const uchar256 raw_scales = v_u8_ld_tnsr_partial_b((int5){256, row, 0, 0, 0}, main_cache, 31, 0);
#if !DSV41_SELECTED_SCALE_CACHE
                const uchar256 row_scales = v_u8_mov_dual_group_all_b(
                    raw_scales, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
#endif
#if DSV41_SELECTED_SCALE_CACHE
                const ushort128 scale_codes=convert_uchar256_to_ushort256(raw_scales,SW_LINEAR).v1;
                const bfloat128 decoded_scales=selected_e4m3fn(scale_codes);
                const uchar256 scale_bits=v_u8_mov_dual_group_all_b(
                    *((uchar256*)&decoded_scales),0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256){0});
#endif
#if DSV41_SELECTED_FULL_ROW_LOAD
                const uchar256 packed_row = v_u8_ld_tnsr_b((int5){0,row},main_cache);
                #pragma unroll
#endif
#ifdef DSV41_MLA_SINGLE_BANK_LOOPED
                #pragma nounroll
#endif
                for (int chunk = 0; chunk < 4; ++chunk) {
#if DSV41_SELECTED_FULL_ROW_LOAD
                    uchar256 bytes;
                    if(chunk==0) bytes=v_u8_mov_dual_group_all_b(
                        packed_row,0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256)0);
                    else if(chunk==1) bytes=v_u8_mov_dual_group_all_b(
                        packed_row,0xffffffff,1,1,1,1,MkWrA(3,3,3,3),(uchar256)0);
                    else if(chunk==2) bytes=v_u8_mov_dual_group_all_b(
                        packed_row,0xffffffff,2,2,2,2,MkWrA(3,3,3,3),(uchar256)0);
                    else bytes=v_u8_mov_dual_group_all_b(
                        packed_row,0xffffffff,3,3,3,3,MkWrA(3,3,3,3),(uchar256)0);
#else
                    const uchar256 raw_bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 64, row, 0, 0, 0}, main_cache, 63, 0);
                    const uchar256 bytes = v_u8_mov_dual_group_all_b(
                        raw_bytes, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
#endif
                    const uchar256 expanded = v_u8_shuffle_b(bytes, fp4_directions, 0, (uchar256){0});
#if DSV41_SELECTED_FP4_TABLE
                    const uchar256 code = (expanded >> nibble_shifts) & 15;
                    const uchar256 table_direction = ((code << 1) + (lanes & 1)) | 0x80;
                    const uchar256 fp4_bits = v_u8_shuffle_b(fp4_table,table_direction,0,(uchar256)0);
                    const bfloat128 fp4_values = *((bfloat128*)&fp4_bits);
#else
                    const ushort128 code = (convert_uchar256_to_ushort256(expanded, SW_LINEAR).v1 >> nibble_shifts) & 15;
                    const bfloat128 fp4_values = selected_fp4(code);
#endif
#if DSV41_SELECTED_SCALE_CACHE
                    // Thirty-two FP8 scales are decoded once; each supplies
                    // sixteen FP4 values. Shuffle their exact BF16 bits.
                    const uchar256 directions=(((lanes>>5)<<1)+(lanes&1)+chunk*16)|0x80;
                    const uchar256 expanded_scale=v_u8_shuffle_b(scale_bits,directions,0,(uchar256){0});
                    bfloat128 value=v_bf16_mul_b(fp4_values,*((bfloat128*)&expanded_scale));
#else
                    const uchar256 directions = ((lanes >> 4) + chunk * 8) | 0x80;
                    const uchar256 scale_bytes = v_u8_shuffle_b(row_scales, directions, 0, (uchar256){0});
#if DSV41_SELECTED_FP8_BRIDGE
                    bfloat128 value=v_bf16_mul_b(fp4_values,selected_e4m3fn_bytes(scale_bytes));
#else
                    const ushort128 scales = convert_uchar256_to_ushort256(scale_bytes, SW_LINEAR).v1;
                    bfloat128 value = v_bf16_mul_b(fp4_values, selected_e4m3fn(scales));
#endif
#endif
                    value = v_bf16_sel_eq_bf16_b(value, (bfloat)0, (bfloat)0, value);
                    STORE_SELECTED(chunk, value);
                }
#endif
            }
#endif
        }
    }
}
#undef STORE_SELECTED
#endif  // DSV41_SELECTED_CODEC_ONLY
