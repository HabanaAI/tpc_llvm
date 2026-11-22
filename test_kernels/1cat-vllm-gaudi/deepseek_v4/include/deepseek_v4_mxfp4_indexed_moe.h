/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#ifndef DSV4_MXFP4_INDEXED_FC1
#define DSV4_MXFP4_INDEXED_FC1 0
#endif
#ifndef DSV4_MXFP4_INDEXED_FC2
#define DSV4_MXFP4_INDEXED_FC2 0
#endif

#define DSV4_MXFP4_DEQUANT_HELPERS_ONLY 1
#include "deepseek_v4_mxfp4_dequant_fp8.h"
#undef DSV4_MXFP4_DEQUANT_HELPERS_ONLY

#define DSV4_MXFP4_INDEXED_TOPK 6
#define DSV4_MXFP4_INDEXED_HIDDEN 4096
#define DSV4_MXFP4_INDEXED_INTERMEDIATE 1024
#define DSV4_MXFP4_INDEXED_W13_PACKED_COLS 2048
#define DSV4_MXFP4_INDEXED_W2_PACKED_COLS 512
#define DSV4_MXFP4_INDEXED_PACKED_VECTOR 256
#define DSV4_MXFP4_INDEXED_SCALE_VECTOR 16

typedef struct {
    bfloat128 values[4];
} dsv4_mxfp4_bf16_512;

static inline dsv4_mxfp4_bf16_512 dsv4_mxfp4_load_bf16_512(
    tensor packed,
    tensor scales,
    int packed_offset,
    int scale_offset,
    int row,
    int expert)
{
    int5 source = {packed_offset, row, expert, 0, 0};
    const uchar256 packed_values = v_u8_ld_tnsr_b(source, packed);
    const bfloat256 scale_values = dsv4_mxfp4_scale_vector(
        scales, scale_offset, row, expert);
#if DSV4_MXFP4_INDEXED_FC1
    const uchar256 low = dsv4_mxfp4_nibbles_to_fp8(
        v_u8_and_b(packed_values, 0x0f), scale_values);
    const uchar256 high = dsv4_mxfp4_nibbles_to_fp8(
        v_u8_shr_b(packed_values, 4), scale_values);
    const ushort256 low_wide = convert_uchar256_to_ushort256(
        low, SW_LINEAR | SW_RHNE);
    const ushort256 high_wide = convert_uchar256_to_ushort256(
        high, SW_LINEAR | SW_RHNE);
    const ushort128 interleaved_low = v_u16_or_b(
        low_wide.v1, v_u16_shl_b(high_wide.v1, 8));
    const ushort128 interleaved_high = v_u16_or_b(
        low_wide.v2, v_u16_shl_b(high_wide.v2, 8));
    const uchar256 output_low = as_uchar256(interleaved_low);
    const uchar256 output_high = as_uchar256(interleaved_high);
    const minifloat256 low_fp8 = *((minifloat256*)&output_low);
    const minifloat256 high_fp8 = *((minifloat256*)&output_high);
    const bfloat256 low_bf16 = v_convert_f8_to_bf16_all_b(
        low_fp8, SW_LINEAR | SW_FP8_BIAS7);
    const bfloat256 high_bf16 = v_convert_f8_to_bf16_all_b(
        high_fp8, SW_LINEAR | SW_FP8_BIAS7);

    dsv4_mxfp4_bf16_512 result;
    result.values[0] = low_bf16.v1 * 0.5f;
    result.values[1] = low_bf16.v2 * 0.5f;
    result.values[2] = high_bf16.v1 * 0.5f;
    result.values[3] = high_bf16.v2 * 0.5f;
    return result;
#else
    const bfloat256 low = dsv4_mxfp4_nibbles_to_bf16(
        v_u8_and_b(packed_values, 0x0f), scale_values);
    const bfloat256 high = dsv4_mxfp4_nibbles_to_bf16(
        v_u8_shr_b(packed_values, 4), scale_values);

    const uint128 low_first = convert_ushort128_to_uint128(
        *((ushort128*)&low.v1), SW_LINEAR);
    const uint128 low_second = convert_ushort128_to_uint128(
        *((ushort128*)&low.v2), SW_LINEAR);
    const uint128 high_first = convert_ushort128_to_uint128(
        *((ushort128*)&high.v1), SW_LINEAR);
    const uint128 high_second = convert_ushort128_to_uint128(
        *((ushort128*)&high.v2), SW_LINEAR);
    const uint64 interleaved[4] = {
        v_u32_or_b(low_first.v1, v_u32_shl_b(high_first.v1, 16)),
        v_u32_or_b(low_first.v2, v_u32_shl_b(high_first.v2, 16)),
        v_u32_or_b(low_second.v1, v_u32_shl_b(high_second.v1, 16)),
        v_u32_or_b(low_second.v2, v_u32_shl_b(high_second.v2, 16)),
    };

    dsv4_mxfp4_bf16_512 result;
    result.values[0] = *((bfloat128*)&interleaved[0]);
    result.values[1] = *((bfloat128*)&interleaved[1]);
    result.values[2] = *((bfloat128*)&interleaved[2]);
    result.values[3] = *((bfloat128*)&interleaved[3]);
    // The shared scale helper emits twice the checkpoint scale for the FP8
    // bridge. Cancel that factor in this direct W4A16 path.
    result.values[0] *= 0.5f;
    result.values[1] *= 0.5f;
    result.values[2] *= 0.5f;
    result.values[3] *= 0.5f;
    return result;
#endif
}

static inline float64 dsv4_mxfp4_reduce_dot(float128 accum)
{
    float64 reduced = v_f32_reduce_add(accum.v1 + accum.v2);
    return v_f32_shuffle_b(reduced, 0x80, 0, reduced);
}

static inline void dsv4_mxfp4_store_bf16_scalar(
    int5 coords,
    tensor output,
    float64 value)
{
    float128 lanes = {0};
    lanes.v1 = value;
    const bfloat128 converted = v_convert_f32_to_bf16_all_b(
        lanes, SW_RHNE);
    v_bf16_st_tnsr_partial(coords, output, converted, 0, 0);
}

#if DSV4_MXFP4_INDEXED_FC1

// One program computes a gate/up pair for one selected expert and one
// intermediate channel. Packed MXFP4 weights are addressed with the runtime
// expert ID, so no selected-weight materialization is needed.
void main(
    tensor hidden_states,
    tensor expert_ids,
    tensor w13,
    tensor w13_scale,
    tensor intermediate)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int selected = index_start[1]; selected < index_end[1]; ++selected) {
        int5 id_coords = {selected, 0, 0, 0, 0};
        const int source_expert = s_i32_ld_g(
            gen_addr(id_coords, expert_ids));
        for (int channel = index_start[0];
             channel < index_end[0];
             ++channel) {
            float128 gate_accum = {0};
            float128 up_accum = {0};
            #pragma unroll (8)
            for (int packed_chunk = 0; packed_chunk < 8; ++packed_chunk) {
                const int packed_offset =
                    packed_chunk * DSV4_MXFP4_INDEXED_PACKED_VECTOR;
                const int scale_offset =
                    packed_chunk * DSV4_MXFP4_INDEXED_SCALE_VECTOR;
                const dsv4_mxfp4_bf16_512 gate_weights =
                    dsv4_mxfp4_load_bf16_512(
                        w13,
                        w13_scale,
                        packed_offset,
                        scale_offset,
                        channel,
                        source_expert);
                const dsv4_mxfp4_bf16_512 up_weights =
                    dsv4_mxfp4_load_bf16_512(
                        w13,
                        w13_scale,
                        packed_offset,
                        scale_offset,
                        channel + DSV4_MXFP4_INDEXED_INTERMEDIATE,
                        source_expert);
                #pragma unroll (4)
                for (int vector = 0; vector < 4; ++vector) {
                    int5 hidden_coords = {
                        packed_chunk * 512 + vector * 128,
                        0,
                        0,
                        0,
                        0};
                    const bfloat128 hidden = v_bf16_ld_tnsr_b(
                        hidden_coords, hidden_states);
                    gate_accum = v_bf16_mac_acc32_b(
                        hidden,
                        gate_weights.values[vector],
                        gate_accum,
                        0);
                    up_accum = v_bf16_mac_acc32_b(
                        hidden,
                        up_weights.values[vector],
                        up_accum,
                        0);
                }
            }

            const float64 gate = dsv4_mxfp4_reduce_dot(gate_accum);
            const float64 up = dsv4_mxfp4_reduce_dot(up_accum);
            const float64 sigmoid = v_reciprocal_f32(
                1.0f + v_exp_cephes_f32(-gate));
            const float64 activated = gate * sigmoid * up;
            int5 output_coords = {channel, selected, 0, 0, 0};
            dsv4_mxfp4_store_bf16_scalar(
                output_coords, intermediate, activated);
        }
    }
}

#elif DSV4_MXFP4_INDEXED_FC2

// One program computes one hidden output channel and accumulates all six
// selected experts with their router weights in FP32.
void main(
    tensor intermediate,
    tensor expert_ids,
    tensor router_weights,
    tensor w2,
    tensor w2_scale,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int output_channel = index_start[0];
         output_channel < index_end[0];
         ++output_channel) {
        float64 routed_sum = 0.0f;
        #pragma unroll (DSV4_MXFP4_INDEXED_TOPK)
        for (int selected = 0;
             selected < DSV4_MXFP4_INDEXED_TOPK;
             ++selected) {
            int5 id_coords = {selected, 0, 0, 0, 0};
            const int source_expert = s_i32_ld_g(
                gen_addr(id_coords, expert_ids));
            float128 dot_accum = {0};
            #pragma unroll (2)
            for (int packed_chunk = 0; packed_chunk < 2; ++packed_chunk) {
                const dsv4_mxfp4_bf16_512 weights =
                    dsv4_mxfp4_load_bf16_512(
                        w2,
                        w2_scale,
                        packed_chunk * DSV4_MXFP4_INDEXED_PACKED_VECTOR,
                        packed_chunk * DSV4_MXFP4_INDEXED_SCALE_VECTOR,
                        output_channel,
                        source_expert);
                #pragma unroll (4)
                for (int vector = 0; vector < 4; ++vector) {
                    int5 intermediate_coords = {
                        packed_chunk * 512 + vector * 128,
                        selected,
                        0,
                        0,
                        0};
                    const bfloat128 hidden = v_bf16_ld_tnsr_b(
                        intermediate_coords, intermediate);
                    dot_accum = v_bf16_mac_acc32_b(
                        hidden,
                        weights.values[vector],
                        dot_accum,
                        0);
                }
            }
            int5 router_coords = {selected, 0, 0, 0, 0};
            const bf16 router_bf16 = s_bf16_ld_g(
                gen_addr(router_coords, router_weights));
            const float router_weight = s_convert_bf16_to_f32(router_bf16);
            routed_sum += dsv4_mxfp4_reduce_dot(dot_accum) * router_weight;
        }
        int5 output_coords = {output_channel, 0, 0, 0, 0};
        dsv4_mxfp4_store_bf16_scalar(output_coords, output, routed_sum);
    }
}

#endif
