// SPDX-License-Identifier: Apache-2.0
// Stable first-occurrence expert grouping. One tiny control work point owns
// all route slots; matrix kernels consume its fixed-capacity device outputs.
#ifndef DSV41_GROUPED_RANKED
#define DSV41_GROUPED_RANKED 0
#endif
#ifndef DSV41_GROUPED_FUSED_CONTROLS
#define DSV41_GROUPED_FUSED_CONTROLS 0
#endif
#ifndef DSV41_GROUPED_DECODER_PREDICATES
#define DSV41_GROUPED_DECODER_PREDICATES 0
#endif
#ifndef DSV41_GROUPED_COMPACT
#define DSV41_GROUPED_COMPACT 0
#endif
#ifndef DSV41_GROUPED_SIMPLE_FP8
#define DSV41_GROUPED_SIMPLE_FP8 0
#endif
#include "route_offsets.h"
void main(tensor ids, tensor generation, tensor metadata, tensor maps,
          int num_experts) {
    const int tokens = get_dim_size(ids, 1);
    const int slots = tokens * 6;
    int experts[36];
    int group_ids[36];
    int counts[36];
    int cursors[36];
    int tile_starts[36];
    int group_offsets[36];
    int groups = 0;
    int valid = 0;
    int tiles = 0;
    // Assemble rows in vector registers/VLM and publish each row once. TPC
    // scalar and vector local memory are distinct: do not reinterpret a
    // scalar scratch array as a vector. Scalar results enter lanes by select.
    const int64 lanes = (int64)read_lane_id_4b_b();
    const short128 short_lanes = (short128)read_lane_id_2b_b();
    int64 out0=0, out1=-1, out2=-1, out3=-1, out4=-1;
    int64 out5=-1, out6=-1, out7=-1, out8=-1, out9=-1;
#if DSV41_GROUPED_DECODER_PREDICATES
    // Header words9..44 carry one device-prepared address per original
    // route; word45 versions this optional extension. The public nine-word
    // header and all existing table rows remain unchanged.
    out0 = v_i32_mov_vb(-1, 0, out0, v_i32_cmp_geq_b(lanes, 9) & v_i32_cmp_less_b(lanes, 45));
    out0 = v_i32_sel_eq_i32_b(lanes, 45, DSV41_GROUPED_COMPACT ? 0x47525033 : 0x47525032, out0);
#endif
#if !DSV41_GROUPED_FUSED_CONTROLS
    short128 map_rows[6];
    for (int m = 0; m < 6; ++m) map_rows[m] = (short128)-1;
#endif
    for (int slot = 0; slot < slots; ++slot) {
        int5 pos = {slot % 6, slot / 6, 0, 0, 0};
        int expert = s_i32_ld_g(gen_addr(pos, ids));
        group_ids[slot] = -1;
        if (expert < 0 || expert >= num_experts) continue;
        int group = 0;
        while (group < groups && experts[group] != expert) ++group;
        if (group == groups) {
            experts[group] = expert;
            counts[group] = 0;
            ++groups;
        }
        group_ids[slot] = group;
        ++counts[group];
        ++valid;
    }
#if DSV41_GROUPED_RANKED
    // Stable descending count order bounds owner g by floor(36/(g+1)).
    // This changes only matrix scheduling, not token order within an expert
    // or the original route-slot map used by weighted BF16 restoration.
    int original_groups[36], inverse_groups[36];
    for (int g = 0; g < groups; ++g) original_groups[g] = g;
    for (int g = 1; g < groups; ++g) {
        const int expert = experts[g], count = counts[g], original = original_groups[g];
        int before = g;
        while (before > 0 && counts[before - 1] < count) {
            experts[before] = experts[before - 1];
            counts[before] = counts[before - 1];
            original_groups[before] = original_groups[before - 1];
            --before;
        }
        experts[before] = expert; counts[before] = count; original_groups[before] = original;
    }
    for (int g = 0; g < groups; ++g) inverse_groups[original_groups[g]] = g;
    for (int slot = 0; slot < slots; ++slot)
        if (group_ids[slot] >= 0) group_ids[slot] = inverse_groups[group_ids[slot]];
#endif
    int offset = 0;
    for (int group = 0; group < groups; ++group) {
        cursors[group] = offset;
        group_offsets[group] = offset;
        tile_starts[group] = tiles;
        out1 = v_i32_sel_eq_i32_b(lanes, group, experts[group], out1);
        out2 = v_i32_sel_eq_i32_b(lanes, group, offset, out2);
        out3 = v_i32_sel_eq_i32_b(lanes, group, counts[group], out3);
        for (int first = 0; first < counts[group]; first += 6) {
            int remaining = counts[group] - first;
            int m = remaining < 6 ? remaining : 6;
            out6 = v_i32_sel_eq_i32_b(lanes, tiles, group, out6);
            out7 = v_i32_sel_eq_i32_b(lanes, tiles, offset + first, out7);
            out8 = v_i32_sel_eq_i32_b(lanes, tiles, m, out8);
#if !DSV41_GROUPED_FUSED_CONTROLS
            map_rows[m-1] = v_i16_sel_eq_i16_b(short_lanes, (short)tiles,
                                             (short)experts[group], map_rows[m-1]);
#endif
            ++tiles;
        }
        offset += counts[group];
    }
    for (int slot = 0; slot < slots; ++slot) {
        int group = group_ids[slot];
        if (group < 0) continue;
        int row = cursors[group]++;
        out4 = v_i32_sel_eq_i32_b(lanes, row, slot, out4);
        out5 = v_i32_sel_eq_i32_b(lanes, slot, row, out5);
        out9 = v_i32_sel_eq_i32_b(lanes, slot, tile_starts[group] * 6 + row - group_offsets[group], out9);
#if DSV41_GROUPED_DECODER_PREDICATES
        const int local = row - group_offsets[group], remainder = counts[group] % 6;
        const int full = counts[group] - remainder;
        const bool tail = local >= full;
        const int matrix_row = (DSV41_GROUPED_COMPACT ? dsv41_compact_offset(group) : dsv41_ranked_offset(group)) + (tail ?
            dsv41_ranked_full_capacity(group) + (DSV41_GROUPED_COMPACT ? 0 : remainder*(remainder-1)/2) + local-full : local);
        const int packed_row = tail ? group*6 + local-full : group*36 + local;
        const int address = matrix_row | (packed_row << 9) | ((int)tail << 20);
        out0 = v_i32_sel_eq_i32_b(lanes, slot+9, address, out0);
#endif
    }
    int header[9];
    header[0] = 1; header[1] = tokens; header[2] = slots;
    header[3] = valid; header[4] = groups; header[5] = tiles;
    int5 gen_pos = {0, 0, 0, 0, 0};
    header[6] = s_i32_ld_g(gen_addr(gen_pos, generation));
    gen_pos[0] = 1;
    header[7] = s_i32_ld_g(gen_addr(gen_pos, generation));
    header[8] = slots - valid;
    for (int word = 0; word < 9; ++word) {
        out0 = v_i32_sel_eq_i32_b(lanes, word, header[word], out0);
    }
    v_i32_st_tnsr((int5){0,0,0,0,0}, metadata, out0);
    v_i32_st_tnsr((int5){0,1,0,0,0}, metadata, out1);
    v_i32_st_tnsr((int5){0,2,0,0,0}, metadata, out2);
    v_i32_st_tnsr((int5){0,3,0,0,0}, metadata, out3);
    v_i32_st_tnsr((int5){0,4,0,0,0}, metadata, out4);
    v_i32_st_tnsr((int5){0,5,0,0,0}, metadata, out5);
    v_i32_st_tnsr((int5){0,6,0,0,0}, metadata, out6);
    v_i32_st_tnsr((int5){0,7,0,0,0}, metadata, out7);
    v_i32_st_tnsr((int5){0,8,0,0,0}, metadata, out8);
    v_i32_st_tnsr((int5){0,9,0,0,0}, metadata, out9);
#if DSV41_GROUPED_FUSED_CONTROLS
    // The same owner counts directly form the firmware predicates. They
    // remain scalar-local here: no second kernel reloads the published table.
    // Preserve all 704 int16 entries, including inactive padding.
    for (int row = 0; row < 11; ++row) {
        const short128 flat = short_lanes + row * 64;
        short128 values = -1;
#if DSV41_GROUPED_SIMPLE_FP8
        // FP8 grouped C6 has one physical M1..6 matrix per nonempty owner.
        // Plane6 is its active-owner predicate; the compact row program
        // patches that matrix to the exact device count.
        for (int group = 0; group < groups; ++group)
            values = v_i16_sel_eq_i16_b(flat, (short)(group * 11 + 6), 0, values);
#else
        const int last = (row * 64 + 63) / 11;
        for (int group = row * 64 / 11; group <= last && group < groups; ++group) {
            const int count = counts[group];
            const short128 plane = flat - group * 11;
            const bool128 full = v_i16_cmp_geq_b(plane, 0) & v_i16_cmp_less_b(plane, count / 6);
            const bool128 tail = v_i16_cmp_eq_b(plane, DSV41_GROUPED_COMPACT ? 6 : 5 + count % 6) & (bool128)(count % 6 != 0);
            values = v_i16_mov_vb(0, 0, values, full | tail);
        }
#endif
#if DSV41_GROUPED_DECODER_PREDICATES
        // Indices396..431 occupy only control row6. Group owners are compact,
        // so g<groups is exactly the nonempty decoder predicate.
        if (row == 6)
            for (int group = 0; group < groups; ++group)
                values = v_i16_sel_eq_i16_b(flat, (short)(396 + group), 0, values);
#endif
        v_i16_st_tnsr_partial((int5){0,row,0,0,0}, maps, values, 63, 0);
    }
#else
    for (int m = 0; m < 6; ++m)
        v_i16_st_tnsr_partial((int5){0,m,0,0,0}, maps, map_rows[m], 63, 0);
#endif
}
