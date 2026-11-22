// SPDX-License-Identifier: Apache-2.0
#if defined(DSV41_MAIN_REUSE_VECTOR128) || defined(DSV41_MAIN_FP16_VALUES) || defined(DSV41_MAIN_SWA_CACHED) || defined(DSV41_MAIN_REUSE_KEYS_ONLY)
#include "deepseek_v41_c6_main_reuse_gather.h"
#else
// SPDX-License-Identifier: Apache-2.0
// Decode selected SWA/CSA2 rows once, preserving selection order and duplicates.
#ifdef DSV41_REUSE_VECTOR
#include "deepseek_v41_selected_kv_codecs.h"
#else
static inline float64 e4m3fn(uint64 code) {
    const uint64 exponent = (code >> 3) & 15;
    const uint64 mantissa = code & 7;
    const uint64 normal_bits = ((exponent + 120) << 23) | (mantissa << 20);
    const float64 tiny = convert_uint64_to_float64(mantissa, 0) * 0.001953125f;
    float64 value = v_f32_sel_eq_u32_b(exponent, 0, tiny, as_float64(normal_bits));
    const uint64 sign = (code & 128) << 24;
    value = as_float64(as_uint64(value) | sign);
    const uint64 nan_bits = 0x7fffffff;
    value = v_f32_sel_eq_u32_b(code & 127, 127, as_float64(nan_bits), value);
    return v_f32_sel_eq_f32_b(value, 0.0f, 0.0f, value);
}

static inline float64 ue8m0(uint64 code) {
    uint64 bits = code << 23;
    bits = v_u32_sel_eq_u32_b(code, 0, 0x00400000, bits);
    bits = v_u32_sel_eq_u32_b(code, 255, 0x7fffffff, bits);
    return as_float64(bits);
}

#endif

void main(tensor swa, tensor shared_rows, tensor shared_mask, tensor positions,
          tensor lengths, tensor rows, tensor values, tensor mask) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#ifndef DSV41_REUSE_VECTOR
    const uint64 lanes = V_LANE_ID_32;
    uint256 scale_directions = {0};
    scale_directions.v1 = (lanes >> 5) | 0x80;
    const uchar256 directions = convert_uint256_to_uchar256(scale_directions, SW_LINEAR);
#endif
    for (int token = begin[1]; token < end[1]; ++token) {
        const int position = s_i32_ld_g(gen_addr((int5){token}, positions));
        const int length = s_i32_ld_g(gen_addr((int5){token}, lengths));
#ifdef DSV41_REUSE_VECTOR_MASK
        // The mask is a contiguous 640-value row. One owner emits it in ten
        // vector stores instead of every row owner issuing scalar traffic.
        if (begin[0] == 0) {
            for (int chunk = 0; chunk < 2; ++chunk) {
                const int64 slots = (int64)V_LANE_ID_32 + chunk * 64;
                const int64 absolute = slots + position - 127;
                float64 enabled = v_f32_sel_grt_i32_b(absolute, -1, 1.0f, 0.0f);
                enabled = v_f32_sel_less_i32_b(slots, length, enabled, 0.0f);
                v_f32_st_tnsr((int5){chunk * 64, token}, mask, enabled);
            }
            for (int chunk = 2; chunk < 10; ++chunk)
                v_f32_st_tnsr((int5){chunk * 64, token}, mask,
                    v_f32_ld_tnsr_b((int5){chunk * 64, token}, shared_mask));
        }
#endif
        for (int slot = begin[0]; slot < end[0]; ++slot) {
            const int absolute = position - 127 + slot;
            const int index = absolute & 255;
            const bool valid_swa = slot < length && absolute >= 0;
#ifndef DSV41_REUSE_VECTOR_MASK
            const float valid = slot < 128 ? (valid_swa ? 1.0f : 0.0f) :
                s_f32_ld_g(gen_addr((int5){slot, token}, shared_mask));
            s_f32_st_g(gen_addr((int5){slot, token}, mask), valid);
#endif
            if (slot >= 128) {
                // Main rows are already BF16. Consume every lane of each
                // vector, then preserve both FP32 halves for the PV consumer.
                for (int chunk = 0; chunk < 4; ++chunk) {
                    const bfloat128 output = v_bf16_ld_tnsr_b(
                        (int5){chunk * 128, slot, token}, shared_rows);
                    v_bf16_st_tnsr((int5){chunk * 128, slot, token}, rows, output);
                    const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
                    v_f32_st_tnsr((int5){chunk * 128, slot, token}, values, restored.v1);
                    v_f32_st_tnsr((int5){chunk * 128 + 64, slot, token}, values, restored.v2);
                }
                continue;
            }
#ifdef DSV41_REUSE_DECODED_SWA
            // The canonical publisher already mirrors the packed-codec result.
            // Read that exact BF16 boundary instead of decoding history again.
            for (int chunk = 0; chunk < 4; ++chunk) {
                const bfloat128 output = valid_swa ? v_bf16_ld_tnsr_b(
                    (int5){chunk * 128, index}, swa) : (bfloat128){0};
                v_bf16_st_tnsr((int5){chunk * 128, slot, token}, rows, output);
                const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
                v_f32_st_tnsr((int5){chunk * 128, slot, token}, values, restored.v1);
                v_f32_st_tnsr((int5){chunk * 128 + 64, slot, token}, values, restored.v2);
            }
#else
            uchar256 scale_bytes = {0};
            if (valid_swa)
                scale_bytes = v_u8_ld_tnsr_partial_b((int5){512, index}, swa, 15, 0);
#ifdef DSV41_REUSE_VECTOR
            const uchar256 expanded_scales = v_u8_mov_dual_group_all_b(
                scale_bytes, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
            for (int chunk = 0; chunk < 4; ++chunk) {
                bfloat128 output = {0};
                if (valid_swa) {
                    const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 128, index}, swa, 127, 0);
#ifndef DSV41_NATIVE_KV_CODEC
                    const ushort128 code = convert_uchar256_to_ushort256(bytes, SW_LINEAR).v1;
#endif
                    const uchar256 directions128 = ((V_LANE_ID_8 >> 5) + chunk * 4) | 0x80;
                    const uchar256 scale_values = v_u8_shuffle_b(expanded_scales, directions128, 0, (uchar256){0});
                    const ushort128 scales = convert_uchar256_to_ushort256(scale_values, SW_LINEAR).v1;
#ifdef DSV41_NATIVE_KV_CODEC
                    output = v_bf16_mul_b(selected_e4m3fn_bytes(bytes), selected_ue8m0(scales));
#else
                    output = v_bf16_mul_b(selected_e4m3fn(code), selected_ue8m0(scales));
#endif
                }
                v_bf16_st_tnsr((int5){chunk * 128, slot, token}, rows, output);
                const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
                v_f32_st_tnsr((int5){chunk * 128, slot, token}, values, restored.v1);
                v_f32_st_tnsr((int5){chunk * 128 + 64, slot, token}, values, restored.v2);
            }
#else
            for (int chunk = 0; chunk < 8; ++chunk) {
                bfloat128 output = {0};
                if (valid_swa) {
                    const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 64, index}, swa, 63, 0);
                    const uint64 code = convert_uchar256_to_uint256(bytes, SW_LINEAR).v1;
                    const uchar256 selected_scales = v_u8_shuffle_b(
                        scale_bytes, directions + (uchar256)(chunk * 2), 0, scale_bytes);
                    const uint64 scales = convert_uchar256_to_uint256(selected_scales, SW_LINEAR).v1;
                    float128 wide = {0};
                    wide.v1 = e4m3fn(code) * ue8m0(scales);
                    output = convert_float128_to_bfloat128(wide, SW_RHNE | SW_LINEAR);
                }
                v_bf16_st_tnsr_partial((int5){chunk * 64, slot, token}, rows, output, 63, 0);
                const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
                v_f32_st_tnsr((int5){chunk * 64, slot, token}, values, restored.v1);
            }
#endif
#endif
        }
    }
}

#endif
