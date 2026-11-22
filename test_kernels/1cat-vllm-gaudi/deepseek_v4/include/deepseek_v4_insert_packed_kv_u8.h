/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_INSERT_DATA_BYTES 576
#define DSV4_INSERT_SCALE_BYTES 8
#define DSV4_INSERT_PAYLOAD_BYTES 584

void main(
    tensor cache_storage_u8,
    tensor cache_geometry,
    tensor packed,
    tensor slots,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 1;
    const int block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 2;
    const int block_size =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 3;
    const int block_count =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    const int slot_count = block_size * block_count;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        int5 token_coords = {token, 0, 0, 0, 0};
        const int slot = s_i32_ld_g(gen_addr(token_coords, slots));
        unsigned char wrote = 0;
        if (slot >= 0 && slot < slot_count) {
            const int block = slot / block_size;
            const int position = slot - block * block_size;
            const int block_offset = base_offset + block * block_stride;
            const int data_offset =
                block_offset + position * DSV4_INSERT_DATA_BYTES;
            const int scale_offset =
                block_offset
                + block_size * DSV4_INSERT_DATA_BYTES
                + position * DSV4_INSERT_SCALE_BYTES;

            int5 packed_coords = {0, token, 0, 0, 0};
            int5 cache_coords = {data_offset, 0, 0, 0, 0};
            const uchar256 data_low =
                v_u8_ld_tnsr_b(packed_coords, packed);
            v_u8_st_tnsr(cache_coords, cache_storage_u8, data_low);

            packed_coords[0] = 256;
            cache_coords[0] = data_offset + 256;
            const uchar256 data_high =
                v_u8_ld_tnsr_b(packed_coords, packed);
            v_u8_st_tnsr(cache_coords, cache_storage_u8, data_high);

            packed_coords[0] = 512;
            cache_coords[0] = data_offset + 512;
            const uchar256 data_tail =
                v_u8_ld_tnsr_b(packed_coords, packed);
            v_u8_st_tnsr_partial(
                cache_coords,
                cache_storage_u8,
                data_tail,
                63,
                0);

            packed_coords[0] = DSV4_INSERT_DATA_BYTES;
            cache_coords[0] = scale_offset;
            const uchar256 scales =
                v_u8_ld_tnsr_b(packed_coords, packed);
            v_u8_st_tnsr_partial(
                cache_coords,
                cache_storage_u8,
                scales,
                DSV4_INSERT_SCALE_BYTES - 1,
                0);
            wrote = 1;
        }
        s_u8_st_g(gen_addr(token_coords, output), wrote);
    }
}
