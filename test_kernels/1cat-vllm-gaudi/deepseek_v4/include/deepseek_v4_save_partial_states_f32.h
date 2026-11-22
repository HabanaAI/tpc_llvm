/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

// Fused DeepSeek V4 compressor state writer. One index-space point copies a
// 64-float KV chunk and writes the matching score + APE chunk into paged state.
void main(
    tensor state_storage,
    tensor state_geometry,
    tensor kv,
    tensor score,
    tensor ape,
    tensor positions,
    tensor slot_mapping,
    tensor completion)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 1;
    const int block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 2;
    const int token_stride =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 3;
    const int block_size =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 4;
    const int block_count =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));

    const int state_width = get_dim_size(kv, 0);
    const int compress_ratio = get_dim_size(ape, 1);
    const int slot_count = block_count * block_size;

    for (int token = index_start[1]; token < index_end[1]; ++token) {
        int5 scalar_coords = {token, 0, 0, 0, 0};
        const int position =
            s_i32_ld_g(gen_addr(scalar_coords, positions));
        const int slot =
            s_i32_ld_g(gen_addr(scalar_coords, slot_mapping));
        const bool valid =
            slot >= 0 && slot < slot_count && position >= 0;

        for (int chunk = index_start[0]; chunk < index_end[0]; ++chunk) {
            float64 completion_value = 0.0f;
            if (valid) {
                const int feature = chunk * 64;
                int5 source_coords = {feature, token, 0, 0, 0};
                const float64 kv_value =
                    v_f32_ld_tnsr_b(source_coords, kv);
                const float64 score_value =
                    v_f32_ld_tnsr_b(source_coords, score);

                const int ape_row =
                    position - (position / compress_ratio) * compress_ratio;
                int5 ape_coords = {feature, ape_row, 0, 0, 0};
                const float64 ape_value =
                    v_f32_ld_tnsr_b(ape_coords, ape);

                const int block = slot / block_size;
                const int token_in_block = slot - block * block_size;
                const int state_offset =
                    base_offset + block * block_stride
                    + token_in_block * token_stride + feature;
                int5 state_coords = {state_offset, 0, 0, 0, 0};
                v_f32_st_tnsr(state_coords, state_storage, kv_value);
                state_coords[0] = state_offset + state_width;
                v_f32_st_tnsr(
                    state_coords, state_storage, score_value + ape_value);
                completion_value = 1.0f;
            }

            // A tiny unique output keeps every index-space point observable;
            // state_storage itself is intentionally updated in place.
            int5 completion_coords = {chunk, token, 0, 0, 0};
            v_f32_st_tnsr_partial(
                completion_coords, completion, completion_value, 0, 0);
        }
    }
}
