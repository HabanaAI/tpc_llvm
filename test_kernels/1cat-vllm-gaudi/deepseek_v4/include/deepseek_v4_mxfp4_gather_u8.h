/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_MXFP4_TOPK 6
#define DSV4_MXFP4_EXPERTS 256
#define DSV4_MXFP4_W13_ROWS 2048
#define DSV4_MXFP4_W13_COLS 2048
#define DSV4_MXFP4_W2_ROWS 4096
#define DSV4_MXFP4_W2_COLS 512
#define DSV4_MXFP4_S13_COLS 128
#define DSV4_MXFP4_S2_COLS 32

void main(
    tensor expert_ids,
    tensor w13,
    tensor w2,
    tensor w13_scale,
    tensor w2_scale,
    tensor gathered_w13,
    tensor gathered_w2,
    tensor gathered_w13_scale,
    tensor gathered_w2_scale)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int selected = index_start[1]; selected < index_end[1]; ++selected) {
        int5 id_coords = {selected, 0, 0, 0, 0};
        const int source_expert =
            s_i32_ld_g(gen_addr(id_coords, expert_ids));

        for (int row = index_start[0]; row < index_end[0]; ++row) {
            #pragma unroll (8)
            for (int chunk = 0; chunk < 8; ++chunk) {
                int5 source = {
                    chunk * 256, row, source_expert, 0, 0};
                int5 destination = {
                    chunk * 256, row, selected, 0, 0};
                const uchar256 values = v_u8_ld_tnsr_b(source, w13);
                v_u8_st_tnsr(destination, gathered_w13, values);
            }

            int5 source_scale = {0, row, source_expert, 0, 0};
            int5 destination_scale = {0, row, selected, 0, 0};
            const uchar256 scales =
                v_u8_ld_tnsr_b(source_scale, w13_scale);
            v_u8_st_tnsr_partial(
                destination_scale,
                gathered_w13_scale,
                scales,
                DSV4_MXFP4_S13_COLS - 1,
                0);

            #pragma unroll (2)
            for (int row_half = 0; row_half < 2; ++row_half) {
                const int w2_row = row + row_half * DSV4_MXFP4_W13_ROWS;
                #pragma unroll (2)
                for (int chunk = 0; chunk < 2; ++chunk) {
                    int5 source = {
                        chunk * 256, w2_row, source_expert, 0, 0};
                    int5 destination = {
                        chunk * 256, w2_row, selected, 0, 0};
                    const uchar256 values = v_u8_ld_tnsr_b(source, w2);
                    v_u8_st_tnsr(destination, gathered_w2, values);
                }

                source_scale[1] = w2_row;
                destination_scale[1] = w2_row;
                const uchar256 w2_scales =
                    v_u8_ld_tnsr_b(source_scale, w2_scale);
                v_u8_st_tnsr_partial(
                    destination_scale,
                    gathered_w2_scale,
                    w2_scales,
                    DSV4_MXFP4_S2_COLS - 1,
                    0);
            }
        }
    }
}
