// SPDX-License-Identifier: Apache-2.0
// Reuses FlashInfer-Gaudi's row statistics and FP8 packing, with V4.1
// FP32 clamp/SwiGLU/routing and explicit BF16 projection/activation boundaries.
#ifndef DSV41_SILU_SCALAR_CACHE5
#define DSV41_SILU_SCALAR_CACHE5 0
#endif
#ifndef DSV41_SILU_PHYSICAL_PAIR
#define DSV41_SILU_PHYSICAL_PAIR 0
#endif
#ifndef DSV41_SILU_PHYSICAL_ROLE
#define DSV41_SILU_PHYSICAL_ROLE 0
#endif
#ifndef DSV41_SILU_FIXED_WIDTH
#define DSV41_SILU_FIXED_WIDTH 0
#endif
#ifndef DSV41_SILU_UNROLL
#define DSV41_SILU_UNROLL 2
#endif
#ifndef DSV41_SILU_CHANNEL_VECTOR
#define DSV41_SILU_CHANNEL_VECTOR 0
#endif

static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

#ifndef DSV41_SHARED_RNE
static inline float64 reciprocal_without_lookup(float64 value) {
    // Positive normal scales are bounded below by epsilon/240 and above
    // by BF16_MAX/240. Three Newton steps preserve the BF16-rounded result.
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}
#endif

static inline float64 row_max_without_lookup(float64 value) {
    // Fold the four dual groups and their two groups. Unlike the generic
    // reduction helper, explicit lane broadcasts do not reserve the LUT cache.
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

static inline float64 projected(tensor product, tensor channel, int n, int row,
                                int expert, float sx, bool valid, int row_width) {
#if defined(DSV41_SILU_SCALED_BF16) && DSV41_SILU_SCALED_BF16
    const uint64 raw=v_u32_ld_tnsr_b((int5){n,0,row},product,SW_UNPACK|SW_UNPCK_16_TO_32)<<16;
    return as_float64(raw);
#else
#if defined(DSV41_SILU_FLAT_PRODUCT)
    const int width = get_dim_size(channel, 1) * 256;
    const float64 acc = v_f32_ld_tnsr_b((int5){n + row * width,0,0},product);
#elif DSV41_SILU_PHYSICAL_PAIR
    const float64 acc = v_f32_ld_tnsr_b((int5){n+(row&1)*row_width*2,0,row/2},product);
#else
    const float64 acc = v_f32_ld_tnsr_b((int5){n, 0, row}, product);
#endif
    const uint64 bits = v_u32_ld_tnsr_b((int5){n % 256, n / 256, expert}, channel,
        SW_UNPACK | SW_UNPCK_16_TO_32, (uint64){0}, valid) << 16;
    return round_bf16(v_f32_mul_b(v_f32_mul_b(acc, *((float64*)&bits)), sx));
#endif
}

static inline float64 activated_values(float64 gate,float64 up,float route) {
    gate = v_f32_min_b(gate, 10.0f);
    up = v_f32_max_b(v_f32_min_b(up, 10.0f), -10.0f);
    const float64 silu = v_f32_mul_b(gate, v_sigmoid_f32(gate));
    return v_f32_mul_b(v_f32_mul_b(silu, up), route);
}

static inline float64 activated_half(tensor product, tensor channel, int n, int row,
                                     int expert, int width, float sx, float route, bool valid) {
    return activated_values(projected(product,channel,n,row,expert,sx,valid,width),
                            projected(product,channel,n+width,row,expert,sx,valid,width),route);
}

#if DSV41_SILU_CHANNEL_VECTOR
static inline float128 activated_vector(tensor product,tensor channel,int n,int row,
                                        int expert,int width,float sx,float route,bool valid) {
    // Each aligned BF16 load fills all 128 lanes. Parent issues four
    // 64-channel unpacking loads and rounds each half independently.
    const bfloat128 gate_channel=v_bf16_ld_tnsr_b((int5){n%256,n/256,expert},channel,0,(bfloat128)0,valid);
    const int up_n=n+width;
    const bfloat128 up_channel=v_bf16_ld_tnsr_b((int5){up_n%256,up_n/256,expert},channel,0,(bfloat128)0,valid);
    const float128 gc=convert_bfloat128_to_float128(gate_channel,SW_LINEAR);
    const float128 uc=convert_bfloat128_to_float128(up_channel,SW_LINEAR);
    float128 gate,up;
    gate.v1=v_f32_mul_b(v_f32_mul_b(v_f32_ld_tnsr_b((int5){n,0,row},product),gc.v1),sx);
    gate.v2=v_f32_mul_b(v_f32_mul_b(v_f32_ld_tnsr_b((int5){n+64,0,row},product),gc.v2),sx);
    up.v1=v_f32_mul_b(v_f32_mul_b(v_f32_ld_tnsr_b((int5){up_n,0,row},product),uc.v1),sx);
    up.v2=v_f32_mul_b(v_f32_mul_b(v_f32_ld_tnsr_b((int5){up_n+64,0,row},product),uc.v2),sx);
    gate=convert_bfloat128_to_float128(convert_float128_to_bfloat128(gate,SW_RHNE|SW_LINEAR),SW_LINEAR);
    up=convert_bfloat128_to_float128(convert_float128_to_bfloat128(up,SW_RHNE|SW_LINEAR),SW_LINEAR);
    float128 result;
    result.v1=activated_values(gate.v1,up.v1,route);
    result.v2=activated_values(gate.v2,up.v2,route);
    return result;
}
#endif

#ifdef DSV41_SILU_QUANT_FUNCTION
static inline void silu_quant_rows(tensor product,tensor ids,tensor activation_scale,tensor channel,tensor router,
                                  tensor output,tensor scales,const int5 start,const int5 end) {
#else
void main(tensor product, tensor ids, tensor activation_scale, tensor channel, tensor router,
          tensor output, tensor scales
#if DSV41_SILU_PHYSICAL_PAIR
          , int configured_width
#endif
          ) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
#endif
#if defined(DSV41_SILU_PADDED_INPUT_WIDTH)
    const int width=get_dim_size(product,0)/2;
#elif DSV41_SILU_PHYSICAL_PAIR
    const int width=configured_width;
#else
    const int width=DSV41_SILU_FIXED_WIDTH ? DSV41_SILU_FIXED_WIDTH : get_dim_size(output,0);
#endif
    const int experts = get_dim_size(channel, 2);
#if !DSV41_SILU_PHYSICAL_PAIR
    const int scale_rows = get_dim_size(activation_scale, 1);
#endif
#if DSV41_SILU_SCALAR_CACHE5
    bfloat128 cache0=0,cache1=0,cache2=0,cache3=0,cache4=0;
#else
    bfloat128 activated[DSV41_SILU_FIXED_WIDTH ? DSV41_SILU_FIXED_WIDTH / 128 : 20];
#endif
#if DSV41_SILU_PHYSICAL_PAIR
    for(int pair=start[0];pair<end[0];++pair)
#if DSV41_SILU_PHYSICAL_ROLE
    for(volatile int role_cursor=start[1];role_cursor<end[1];++role_cursor) {
        const int role=role_cursor;
#else
    for(volatile int role_cursor=0;role_cursor<2;++role_cursor) {
        const int role=role_cursor;
#endif
        const int row=pair*2+role;
#if defined(DSV41_SILU_PHYSICAL_CANONICAL) && DSV41_SILU_PHYSICAL_CANONICAL
        const float sx=s_f32_ld_g(gen_addr((int5){0,row},activation_scale));
        const int expert=s_i32_ld_g(gen_addr((int5){row,0},ids));
        const float route=s_f32_ld_g(gen_addr((int5){row,0},router));
#else
        const float sx=s_f32_ld_g(gen_addr((int5){role,pair},activation_scale));
        const int expert=s_i32_ld_g(gen_addr((int5){role,pair},ids));
        const float route=s_f32_ld_g(gen_addr((int5){role,pair},router));
#endif
        const bool valid=expert>=0 && expert<experts;
#else
    for (int row = start[0] *
#ifdef DSV41_SILU_QUANT_ROW_TILE
         DSV41_SILU_QUANT_ROW_TILE; row < end[0] * DSV41_SILU_QUANT_ROW_TILE;
#else
         1; row < end[0];
#endif
         ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, scale_rows == 1 ? 0 : row}, activation_scale));
        const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
        const bool valid = expert >= 0 && expert < experts;
        const float route = s_f32_ld_g(gen_addr((int5){row}, router));
#endif
        float64 maximum = 0;
#if DSV41_SILU_SCALAR_CACHE5
        #pragma nounroll
#elif defined(DSV41_SILU_FULL_UNROLL)
        #pragma unroll
#else
        #pragma loop_unroll(DSV41_SILU_UNROLL)
#endif
        for (int tile = 0; tile < width / 128; ++tile) {
#if DSV41_SILU_CHANNEL_VECTOR
            const float128 pair=activated_vector(product,channel,tile*128,row,expert,width,sx,route,valid);
#else
            float64_pair_t pair;
            pair.v1 = activated_half(product, channel, tile * 128, row, expert, width, sx, route, valid);
            pair.v2 = activated_half(product, channel, tile * 128 + 64, row, expert, width, sx, route, valid);
#endif
            // Projection loads are two contiguous 64-element halves, unlike
            // the interleaved pair produced by convert_bf16_to_f32_all.
            const bfloat128 value = convert_float128_to_bfloat128(pair, SW_RHNE | SW_LINEAR);
#if DSV41_SILU_SCALAR_CACHE5
            if(tile==0)cache0=value;
            if(tile==1)cache1=value;
            if(tile==2)cache2=value;
            if(tile==3)cache3=value;
            if(tile==4)cache4=value;
#else
            activated[tile] = value;
#endif
            const float64_pair_t rounded = v_convert_bf16_to_f32_all_b(value);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(rounded.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(rounded.v2));
        }
        maximum = row_max_without_lookup(maximum);
#ifdef DSV41_SHARED_RNE
        const uint64 bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 scale = as_float64((power + 127) << 23);
        const float64 inverse = as_float64((127 - power) << 23);
#else
        const float64 raw_scale = round_bf16(maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
#endif
#if DSV41_SILU_PHYSICAL_PAIR
#if defined(DSV41_SILU_PHYSICAL_CANONICAL) && DSV41_SILU_PHYSICAL_CANONICAL
        v_f32_st_tnsr_partial((int5){0,0,row},scales,scale,0,0);
#else
        v_f32_st_tnsr((int5){0,role,pair},scales,scale);
#endif
#else
        v_f32_st_tnsr((int5){0, 0, row}, scales, scale);
#endif
#if DSV41_SILU_SCALAR_CACHE5
        #pragma nounroll
#elif defined(DSV41_SILU_FULL_UNROLL)
        #pragma unroll
#else
        #pragma loop_unroll(DSV41_SILU_UNROLL)
#endif
        for (int tile = 0; tile < width / 128; ++tile) {
#if DSV41_SILU_SCALAR_CACHE5
            bfloat128 cached=cache0;
            if(tile==1)cached=cache1;
            if(tile==2)cached=cache2;
            if(tile==3)cached=cache3;
            if(tile==4)cached=cache4;
            const float64_pair_t fp32 = v_convert_bf16_to_f32_all_b(cached);
#else
            const float64_pair_t fp32 = v_convert_bf16_to_f32_all_b(activated[tile]);
#endif
            minifloat256 packed = 0;
#ifdef DSV41_SHARED_RNE
            packed = v_convert_f32_to_f8_b(fp32.v1 * inverse, 0, SW_RHNE | SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(fp32.v2 * inverse, 2, SW_RHNE | SW_CLIP_FP, packed);
#else
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v1 * inverse), 0, SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v2 * inverse), 2, SW_CLIP_FP, packed);
#endif
            const minifloat256 sparse = packed;
            packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
#ifdef DSV41_SHARED_RNE
            uchar256 raw = *((uchar256*)&packed);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            packed = *((minifloat256*)&raw);
#endif
#if DSV41_SILU_PHYSICAL_PAIR
#if defined(DSV41_SILU_PHYSICAL_CANONICAL) && DSV41_SILU_PHYSICAL_CANONICAL
            v_f8_st_tnsr_partial((int5){tile*128,0,row},output,packed,127,0);
#else
            v_f8_st_tnsr_partial((int5){tile*128+role*width,0,pair},output,packed,127,0);
#endif
#else
            v_f8_st_tnsr_partial((int5){tile * 128, 0, row}, output, packed, 127, 0);
#endif
        }
    }
}
