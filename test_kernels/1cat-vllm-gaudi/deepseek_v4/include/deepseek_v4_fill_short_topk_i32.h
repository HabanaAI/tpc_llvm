/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_SHORT_TOPK_WIDTH 512
#define DSV4_SHORT_TOPK_COMPRESS_RATIO 4

void main(tensor output, tensor positions, tensor valid_counts)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const uint64 lanes = V_LANE_ID_32;

    for (int batch = index_start[0]; batch < index_end[0]; ++batch) {
        int5 position_coords = {batch, 0, 0, 0, 0};
        const int position = s_i32_ld_g(
            gen_addr(position_coords, positions));
        int valid_count =
            (position + 1) / DSV4_SHORT_TOPK_COMPRESS_RATIO;
        valid_count = valid_count < DSV4_SHORT_TOPK_WIDTH
            ? valid_count
            : DSV4_SHORT_TOPK_WIDTH;

        #pragma unroll (8)
        for (int chunk = 0; chunk < 8; ++chunk) {
            const int offset = chunk * 64;
            const uint64 indices = lanes + offset;
            const int64 values = v_i32_sel_less_u32_b(
                indices,
                (unsigned)valid_count,
                (int64)indices,
                -1);
            int5 output_coords = {offset, batch, 0, 0, 0};
            v_i32_st_tnsr(output_coords, output, values);
        }

        v_i32_st_tnsr_partial(
            position_coords, valid_counts, valid_count, 0, 0);
    }
}
