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
    tensor first_indices,
    tensor second_storage_u8,
    tensor second_geometry,
    tensor second_indices,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int first_count = get_dim_size(first_indices, 0);

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
            int5 index_coords = {output_slot, 0, 0, 0, 0};
            const int source_slot =
                s_i32_ld_g(gen_addr(index_coords, first_indices));
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
