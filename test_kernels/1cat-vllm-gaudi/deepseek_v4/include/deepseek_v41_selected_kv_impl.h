// SPDX-License-Identifier: Apache-2.0

#if defined(DSV41_DIRECT_MLA_KV) && !defined(DSV41_PACKED_MLA_GATHER)
#define DSV41_PACKED_MLA_GATHER 1
#endif
// Decode selected SWA/CSA2 rows once, preserving selection order and duplicates.
#ifdef DSV41_VECTOR_KV_CODEC
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

#ifdef DSV41_LOGICAL_MLA_OPERANDS
void main(tensor swa, tensor main_cache, tensor selection, tensor positions, tensor pages,
          tensor lengths, tensor rows, tensor values, tensor mask,
#ifdef DSV41_LOGICAL_MLA_EXPORT_MAIN
          tensor shared_rows,
#endif
          int ratio) {
#else
void main(tensor swa, tensor main_cache, tensor indices,
#ifdef DSV41_PACKED_MLA_GATHER
          tensor attention_indices, tensor lengths,
#endif
#ifdef DSV41_KV_WRITE_DEPENDENCY
          tensor completion,
#endif
#ifdef DSV41_COMPRESS_WRITE_DEPENDENCY
          tensor compressed_completion,
#endif
          tensor rows,
#ifdef DSV41_PACKED_MLA_GATHER
          tensor values, tensor mask) {
#else
          tensor local_indices) {
#endif
#endif
#ifdef DSV41_KV_WRITE_DEPENDENCY
    const bool swa_ready = s_i32_ld_g(gen_addr((int5){0}, completion)) >= 0;
#else
    const bool swa_ready = 1;
#endif
#ifdef DSV41_COMPRESS_WRITE_DEPENDENCY
    const bool ready = swa_ready && s_i32_ld_g(gen_addr((int5){0}, compressed_completion)) >= 0;
#else
    const bool ready = swa_ready;
#endif
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int swa_length = get_dim_size(swa, 1);
    const int main_length = get_dim_size(main_cache, 0) == 288 ? get_dim_size(main_cache, 1) : 0;
#ifndef DSV41_VECTOR_KV_CODEC
    const uint64 lanes = V_LANE_ID_32;
    uint256 wide_directions = {0};
    wide_directions.v1 = (lanes >> 1) | 0x80;
    const uchar256 directions = convert_uint256_to_uchar256(wide_directions, SW_LINEAR);
#ifdef DSV41_VECTOR_KV_SCALES
    uint256 scale_directions = {0};
    scale_directions.v1 = (lanes >> 5) | 0x80;
    const uchar256 swa_scale_directions = convert_uint256_to_uchar256(scale_directions, SW_LINEAR);
    scale_directions.v1 = (lanes >> 4) | 0x80;
    const uchar256 main_scale_directions = convert_uint256_to_uchar256(scale_directions, SW_LINEAR);
#endif
#endif
#if defined(DSV41_PACKED_MLA_GATHER) || defined(DSV41_LOGICAL_MLA_OPERANDS)
    const int first_token = begin[1], token_end = end[1];
#else
    const int first_token = 0, token_end = 1;
#endif
#ifdef DSV41_LOGICAL_MLA_OPERANDS
#if defined(DSV41_VECTOR_MAIN_MASK) || defined(DSV41_TENSOR_MAIN_MASK)
    // The logical gather host contract accepts ratios1/2 only.
    const int page_shift = ratio == 2 ? 6 : 7;
    const int page_width = 1 << page_shift;
#else
    const int page_width = 128 / ratio;
#endif
    const int page_count = get_dim_size(pages, 0);
#endif
    for (int token = first_token; token < token_end; ++token) {
#if defined(DSV41_PACKED_MLA_GATHER) || defined(DSV41_LOGICAL_MLA_OPERANDS)
        const int length = s_i32_ld_g(gen_addr((int5){token}, lengths));
#endif
#ifdef DSV41_PACKED_MLA_GATHER
        const int selected_rows = get_dim_size(indices, 0);
#endif
#if defined(DSV41_VECTOR_MAIN_MASK) || defined(DSV41_TENSOR_MAIN_MASK)
        const int position = s_i32_ld_g(gen_addr((int5){token}, positions));
#endif
#ifdef DSV41_VECTOR_MAIN_MASK
        if (begin[0] == 0) {
            for (int chunk = 0; chunk < 10; ++chunk) {
                const int64 slots = (int64)V_LANE_ID_32 + chunk * 64;
                float64 enabled;
                if (chunk < 2)
                    enabled = v_f32_sel_grt_i32_b(slots + position - 127, -1, 1.0f, 0.0f);
                else {
                    const int64 logical = v_i32_ld_tnsr_b((int5){(chunk - 2) * 64, token}, selection);
                    enabled = v_f32_sel_grt_i32_b(logical, -1, 1.0f, 0.0f);
                }
                enabled = v_f32_sel_less_i32_b(slots, length, enabled, 0.0f);
                v_f32_st_tnsr((int5){chunk * 64, token}, mask, enabled);
            }
        }
#endif
        for (int slot = begin[0]; slot < end[0]; ++slot) {
#ifdef DSV41_LOGICAL_MLA_OPERANDS
        bool local_valid = slot < length;
        int index = -1;
        if (slot < 128) {
#if !defined(DSV41_VECTOR_MAIN_MASK) && !defined(DSV41_TENSOR_MAIN_MASK)
            const int position = s_i32_ld_g(gen_addr((int5){token}, positions));
#endif
            const int absolute = position - 127 + slot;
            local_valid = local_valid && absolute >= 0;
            if (local_valid) index = absolute & 255;
        } else {
            const int logical = s_i32_ld_g(gen_addr((int5){slot - 128, token}, selection));
            local_valid = local_valid && logical >= 0;
#if defined(DSV41_VECTOR_MAIN_MASK) || defined(DSV41_TENSOR_MAIN_MASK)
            const int logical_page = logical >> page_shift;
#else
            const int logical_page = logical / page_width;
#endif
            if (local_valid && logical_page < page_count) {
                const int page = s_i32_ld_g(gen_addr((int5){logical_page}, pages));
                const int physical = page * page_width + (logical & (page_width - 1));
                // Preserve the parent physical clamp and its independent
                // local validity mask, including a negative page mapping.
                index = (physical < 0 ? 0 : physical) + swa_length;
            }
        }
#ifdef DSV41_TENSOR_MAIN_MASK
        // Retain per-row ownership and compiler slicing. A one-lane tensor
        // store avoids the global-address path without a serial whole-row owner.
        v_f32_st_tnsr_partial((int5){slot, token}, mask,
                             (float64)(local_valid ? 1.0f : 0.0f), 0, 0);
#elif !defined(DSV41_VECTOR_MAIN_MASK)
        s_f32_st_g(gen_addr((int5){slot, token}, mask), local_valid ? 1.0f : 0.0f);
#endif
#elif defined(DSV41_PACKED_MLA_GATHER)
        const int selected = s_i32_ld_g(gen_addr((int5){slot, token}, attention_indices));
        const bool consumed = slot < length && selected >= 0 && selected < selected_rows;
        int index = -1;
        if (consumed) index = s_i32_ld_g(gen_addr((int5){selected}, indices));
        // The original gather accepts an in-range selected row even when its
        // physical source was invalid and the decoder filled it with zeros.
        s_f32_st_g(gen_addr((int5){slot, token}, mask), consumed ? 1.0f : 0.0f);
#else
        const int index = s_i32_ld_g(gen_addr((int5){slot, 0, 0, 0, 0}, indices));
#endif
        const bool valid = ready && index >= 0 && index < swa_length + main_length;
#if !defined(DSV41_PACKED_MLA_GATHER) && !defined(DSV41_LOGICAL_MLA_OPERANDS)
        s_i32_st_g(gen_addr((int5){slot, 0, 0, 0, 0}, local_indices), valid ? slot : -1);
#endif
#ifdef DSV41_SELECTED_VALID_ONLY
        // The only consumer rejects -1 before loading a row. Invalid rows are
        // internal unspecified storage; the public gather still writes zeros.
        if (!valid) continue;
#endif
#ifdef DSV41_VECTOR_KV_SCALES
        // A whole row's scale bytes fit within one shuffle dual group.
        // Load them once, replacing 16/32 dependent scalar global loads.
        uchar256 scale_bytes = {0};
        if (valid && index < swa_length) {
            scale_bytes = v_u8_ld_tnsr_partial_b((int5){512, index, 0, 0, 0}, swa, 15, 0);
        } else if (valid) {
            scale_bytes = v_u8_ld_tnsr_partial_b(
                (int5){256, index - swa_length, 0, 0, 0}, main_cache, 31, 0);
        }
#endif
#ifdef DSV41_VECTOR_KV_CODEC
        const uchar256 lanes128 = V_LANE_ID_8;
        const uchar256 row_scales = v_u8_mov_dual_group_all_b(
            scale_bytes, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
        const uchar256 fp4_directions = (lanes128 >> 1) | 0x80;
        const ushort128 shifts = ((ushort128)V_LANE_ID_16 & 1) << 2;
        for (int chunk = 0; chunk < 4; ++chunk) {
            bfloat128 output = {0};
            if (valid && index < swa_length) {
                const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 128, index}, swa, 127, 0);
                #ifndef DSV41_NATIVE_KV_CODEC
                const ushort128 code = convert_uchar256_to_ushort256(bytes, SW_LINEAR).v1;
#endif
                const uchar256 d = ((lanes128 >> 5) + chunk * 4) | 0x80;
                const uchar256 scales8 = v_u8_shuffle_b(row_scales, d, 0, (uchar256){0});
                const ushort128 scales16 = convert_uchar256_to_ushort256(scales8, SW_LINEAR).v1;
                #ifdef DSV41_NATIVE_KV_CODEC
                output = v_bf16_mul_b(selected_e4m3fn_bytes(bytes), selected_ue8m0(scales16));
#else
                output = v_bf16_mul_b(selected_e4m3fn(code), selected_ue8m0(scales16));
#endif
            } else if (valid) {
                const int row = index - swa_length;
                const uchar256 packed = v_u8_ld_tnsr_partial_b((int5){chunk * 64, row}, main_cache, 63, 0);
                const uchar256 bytes = v_u8_mov_dual_group_all_b(
                    packed, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), (uchar256){0});
                const uchar256 expanded = v_u8_shuffle_b(bytes, fp4_directions, 0, (uchar256){0});
                const ushort128 code = (convert_uchar256_to_ushort256(expanded, SW_LINEAR).v1 >> shifts) & 15;
                const uchar256 d = ((lanes128 >> 4) + chunk * 8) | 0x80;
                const uchar256 scales8 = v_u8_shuffle_b(row_scales, d, 0, (uchar256){0});
                #ifndef DSV41_NATIVE_KV_CODEC
                const ushort128 scales16 = convert_uchar256_to_ushort256(scales8, SW_LINEAR).v1;
#endif
                #ifdef DSV41_NATIVE_KV_CODEC
                output = v_bf16_mul_b(selected_fp4(code), selected_e4m3fn_bytes(scales8));
#else
                output = v_bf16_mul_b(selected_fp4(code), selected_e4m3fn(scales16));
#endif
                output = v_bf16_sel_eq_bf16_b(output, (bfloat)0, (bfloat)0, output);
            }
            v_bf16_st_tnsr((int5){chunk * 128, slot, token}, rows, output);
            v_bf16_st_tnsr((int5){chunk * 128, slot, token}, shared_rows, output);
            const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
            v_f32_st_tnsr((int5){chunk * 128, slot, token}, values, restored.v1);
            v_f32_st_tnsr((int5){chunk * 128 + 64, slot, token}, values, restored.v2);
        }
#else
        for (int chunk = 0; chunk < 8; ++chunk) {
            float64 value = 0;
            if (valid && index < swa_length) {
                const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 64, index, 0, 0, 0}, swa, 63, 0);
                const uint64 code = convert_uchar256_to_uint256(bytes, SW_LINEAR).v1;
#ifdef DSV41_VECTOR_KV_SCALES
                const uchar256 selected_scales = v_u8_shuffle_b(
                    scale_bytes, swa_scale_directions + (uchar256)(chunk * 2), 0, scale_bytes);
                const uint64 scales = convert_uchar256_to_uint256(selected_scales, SW_LINEAR).v1;
#else
                const unsigned s0 = s_u8_ld_g(gen_addr((int5){512 + chunk * 2, index, 0, 0, 0}, swa));
                const unsigned s1 = s_u8_ld_g(gen_addr((int5){513 + chunk * 2, index, 0, 0, 0}, swa));
                const uint64 scales = v_u32_sel_less_u32_b(lanes, 32, s0, s1);
#endif
                value = e4m3fn(code) * ue8m0(scales);
            } else if (valid) {
                const int row = index - swa_length;
                const uchar256 bytes = v_u8_ld_tnsr_partial_b((int5){chunk * 32, row, 0, 0, 0}, main_cache, 31, 0);
                const uchar256 expanded = v_u8_shuffle_b(bytes, directions, 0, (uchar256){0});
                const uint64 code = (convert_uchar256_to_uint256(expanded, SW_LINEAR).v1 >> ((lanes & 1) * 4)) & 15;
                const uint64 magnitude = code & 7;
                const float64 number = convert_uint64_to_float64(magnitude, 0);
                value = v_f32_sel_less_u32_b(magnitude, 6, number - 2.0f, number * 2.0f - 8.0f);
                value = v_f32_sel_less_u32_b(magnitude, 4, number * 0.5f, value);
                value = as_float64(as_uint64(value) | ((code & 8) << 28));
#ifdef DSV41_VECTOR_KV_SCALES
                const uchar256 selected_scales = v_u8_shuffle_b(
                    scale_bytes, main_scale_directions + (uchar256)(chunk * 4), 0, scale_bytes);
                const uint64 scales = convert_uchar256_to_uint256(selected_scales, SW_LINEAR).v1;
#else
                const unsigned s0 = s_u8_ld_g(gen_addr((int5){256 + chunk * 4, row, 0, 0, 0}, main_cache));
                const unsigned s1 = s_u8_ld_g(gen_addr((int5){257 + chunk * 4, row, 0, 0, 0}, main_cache));
                const unsigned s2 = s_u8_ld_g(gen_addr((int5){258 + chunk * 4, row, 0, 0, 0}, main_cache));
                const unsigned s3 = s_u8_ld_g(gen_addr((int5){259 + chunk * 4, row, 0, 0, 0}, main_cache));
                uint64 scales = v_u32_sel_less_u32_b(lanes, 16, s0, s1);
                scales = v_u32_sel_geq_u32_b(lanes, 32, s2, scales);
                scales = v_u32_sel_geq_u32_b(lanes, 48, s3, scales);
#endif
                value *= e4m3fn(scales);
                value = v_f32_sel_eq_f32_b(value, 0.0f, 0.0f, value);
            }
            float128 wide = {0};
            wide.v1 = value;
            const bfloat128 output = convert_float128_to_bfloat128(wide, SW_RHNE | SW_LINEAR);
            v_bf16_st_tnsr_partial((int5){chunk * 64, slot, token, 0, 0}, rows, output, 63, 0);
#ifdef DSV41_LOGICAL_MLA_EXPORT_MAIN
            // A separate output preserves the QK operand's internal SRAM
            // placement. Later layers consume only slots 128..639.
            v_bf16_st_tnsr_partial((int5){chunk * 64, slot, token, 0, 0}, shared_rows, output, 63, 0);
#endif
#if defined(DSV41_PACKED_MLA_GATHER) || defined(DSV41_LOGICAL_MLA_OPERANDS)
            // PV must consume the rounded BF16 key, exactly as the separate
            // gather did, including signed zeros and nonfinite values.
            const float128 restored = convert_bfloat128_to_float128(output, SW_LINEAR);
            v_f32_st_tnsr((int5){chunk * 64, slot, token, 0, 0}, values, restored.v1);
#endif
        }
#endif
        }
    }
}
