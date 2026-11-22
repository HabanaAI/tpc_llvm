// SPDX-License-Identifier: Apache-2.0
// Two adjacent output vectors share each route's scalar ID and activation
// scale. Projection scaling, BF16 route rounding and summation stay ordered.
static inline float64 round_route(float64 value) {
    // Qualified C1 packing convention: the two halves are identical, so no
    // linear-layout shuffle is needed to return the first FP32 half.
    float128 repeated;
    repeated.v1 = value;
    repeated.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(repeated, SW_RHNE)).v1;
}

static inline float128 route_values(tensor product, tensor channel, int n,
                                    int row, int expert, float sx, bool valid, int token, int slot) {
    const bfloat128 packed = v_bf16_ld_tnsr_b((int5){n % 256, n / 256, expert},
                                            channel, 0, (bfloat128)0, valid);
    const float128 weight = convert_bfloat128_to_float128(packed, SW_LINEAR);
    float128 scaled;
#if defined(DSV41_W2_TOKEN_BATCH) && DSV41_W2_TOKEN_BATCH
    scaled.v1 = (v_f32_ld_tnsr_b((int5){n%256,0,slot,n/256,token}, product) * weight.v1) * sx;
    scaled.v2 = (v_f32_ld_tnsr_b((int5){(n+64)%256,0,slot,n/256,token}, product) * weight.v2) * sx;
#elif defined(DSV41_W2_CHANNEL_BATCH) && DSV41_W2_CHANNEL_BATCH
    scaled.v1 = (v_f32_ld_tnsr_b((int5){n%256, 0, row, n/256}, product) * weight.v1) * sx;
    scaled.v2 = (v_f32_ld_tnsr_b((int5){(n+64)%256, 0, row, n/256}, product) * weight.v2) * sx;
#else
    scaled.v1 = (v_f32_ld_tnsr_b((int5){n, 0, row}, product) * weight.v1) * sx;
    scaled.v2 = (v_f32_ld_tnsr_b((int5){n + 64, 0, row}, product) * weight.v2) * sx;
#endif
    scaled.v1 = round_route(scaled.v1);
    scaled.v2 = round_route(scaled.v2);
    return scaled;
}

#ifndef DSV41_W2_SHARED_SCALE
#define DSV41_W2_SHARED_SCALE 0
#endif
void main(tensor product, tensor ids, tensor activation_scale, tensor channel,
#if DSV41_W2_SHARED_SCALE
          tensor shared, tensor shared_scale, tensor shared_channel,
#endif
          tensor output) {
    const int5 start = get_index_space_offset(), end = start + get_index_space_size();
    const int experts = get_dim_size(channel, 2);
    for (int token = start[1]; token < end[1]; ++token) {
        for (int block = start[0]; block < end[0]; ++block) {
            const int n = block * 256;
            float128 first = {0}, second = {0};
            #pragma loop_unroll(6)
            for (int slot = 0; slot < 6; ++slot) {
                const int row = token * 6 + slot;
                const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
                const float sx = s_f32_ld_g(gen_addr((int5){0, row}, activation_scale));
                const bool valid = expert >= 0 && expert < experts;
                const float128 a = route_values(product, channel, n, row, expert, sx, valid, token, slot);
                const float128 b = route_values(product, channel, n + 128, row, expert, sx, valid, token, slot);
                first.v1 += a.v1;
                first.v2 += a.v2;
                second.v1 += b.v1;
                second.v2 += b.v2;
            }
#if DSV41_W2_SHARED_SCALE
            // Reuse decode's deferred shared scaling (95cee5ed). Both scale
            // producers emit powers of two; retain the rounded routed row
            // before adding the separately rounded shared row.
            const float sx_shared=s_f32_ld_g(gen_addr((int5){0,token},shared_scale));
            for(int part=0;part<2;++part) {
                const int col=n+part*128;
                float128 factor;
                factor.v1=v_f32_ld_tnsr_b((int5){col},shared_channel)*sx_shared;
                factor.v2=v_f32_ld_tnsr_b((int5){col+64},shared_channel)*sx_shared;
                const bfloat128 shared_row=v_bf16_ld_tnsr_b((int5){col,token},shared)*
                    convert_float128_to_bfloat128(factor,SW_RHNE|SW_LINEAR);
                const float128 shared_value=convert_bfloat128_to_float128(shared_row,SW_LINEAR);
                const float128 routed=part==0?first:second;
                float128 combined=convert_bfloat128_to_float128(
                    convert_float128_to_bfloat128(routed,SW_RHNE|SW_LINEAR),SW_LINEAR);
                combined.v1+=shared_value.v1;combined.v2+=shared_value.v2;
                v_bf16_st_tnsr((int5){col,0,token},output,
                    convert_float128_to_bfloat128(combined,SW_RHNE|SW_LINEAR));
            }
#else
            v_bf16_st_tnsr((int5){n, 0, token}, output, convert_float128_to_bfloat128(first, SW_RHNE | SW_LINEAR));
            v_bf16_st_tnsr((int5){n + 128, 0, token}, output, convert_float128_to_bfloat128(second, SW_RHNE | SW_LINEAR));
#endif
        }
    }
}
