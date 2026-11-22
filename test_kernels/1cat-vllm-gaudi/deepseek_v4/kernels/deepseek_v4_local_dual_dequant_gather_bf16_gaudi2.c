/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_DEQUANT_HELPERS_ONLY
#include "deepseek_v4_dequant_gather_bf16.h"

void main(
    tensor first_storage_u8,
    tensor first_geometry,
    tensor first_shape_buffer,
    tensor token_to_req_indices,
    tensor block_table,
    tensor is_valid_token,
    tensor seq_lens,
    tensor second_storage_u8,
    tensor second_geometry,
    tensor second_indices,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int first_count = get_dim_size(first_shape_buffer, 0);

    int5 scalar_coords = {0, 0, 0, 0, 0};
    const int request_index = s_i32_ld_g(
        gen_addr(scalar_coords, token_to_req_indices));
    const int valid_token = s_i32_ld_g(
        gen_addr(scalar_coords, is_valid_token));
    scalar_coords[0] = request_index;
    const int sequence_length = s_i32_ld_g(
        gen_addr(scalar_coords, seq_lens));
    int topk_count = sequence_length / 4;
    topk_count = topk_count < first_count ? topk_count : first_count;
    topk_count = valid_token != 0 ? topk_count : 0;

    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int first_base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, first_geometry));
    const int second_base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, second_geometry));
    geometry_coords[0] = 1;
    const int first_block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, first_geometry));
    const int second_block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, second_geometry));
    geometry_coords[0] = 2;
    const int first_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, first_geometry));
    const int second_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, second_geometry));
    geometry_coords[0] = 3;
    const int first_block_count =
        s_i32_ld_g(gen_addr(geometry_coords, first_geometry));
    const int second_block_count =
        s_i32_ld_g(gen_addr(geometry_coords, second_geometry));

    for (int output_slot = index_start[1];
         output_slot < index_end[1];
         ++output_slot) {
        if (output_slot < first_count) {
            int source_slot = -1;
            if (output_slot < topk_count) {
                const int logical_block = output_slot / first_block_size;
                const int block_offset =
                    output_slot - logical_block * first_block_size;
                int5 block_coords = {
                    logical_block, request_index, 0, 0, 0};
                const int physical_block = s_i32_ld_g(
                    gen_addr(block_coords, block_table));
                source_slot =
                    physical_block * first_block_size + block_offset;
            }
            dsv4_dequant_gather_slot(
                first_storage_u8,
                source_slot,
                output_slot,
                first_base_offset,
                first_block_stride,
                first_block_size,
                first_block_count * first_block_size,
                output);
        } else {
            const int local_slot = output_slot - first_count;
            int5 index_coords = {local_slot, 0, 0, 0, 0};
            const int source_slot =
                s_i32_ld_g(gen_addr(index_coords, second_indices));
            dsv4_dequant_gather_slot(
                second_storage_u8,
                source_slot,
                output_slot,
                second_base_offset,
                second_block_stride,
                second_block_size,
                second_block_count * second_block_size,
                output);
        }
    }
}
