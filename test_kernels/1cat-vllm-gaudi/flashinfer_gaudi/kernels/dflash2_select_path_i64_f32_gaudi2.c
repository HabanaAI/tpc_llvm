// SPDX-License-Identifier: Apache-2.0
/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

// Greedy Qwen3.8 DFlash2 lattice walk for Gaudi2. One index-space point
// owns a request and resolves all seven dependent top-16 choices in one TPC
// launch. Strict greater-than preserves torch.argmax's first-index tie rule.
void main(tensor candidate_ids, tensor scores, tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int num_steps = get_dim_size(candidate_ids, 1);
    const int top_k = get_dim_size(candidate_ids, 0);

    for (int batch = index_start[0]; batch < index_end[0]; ++batch) {
        int previous = 0;
        for (int step = 0; step < num_steps; ++step) {
            int5 score_coords = {0, previous, step, batch, 0};
            float best_score = s_f32_ld_g(gen_addr(score_coords, scores));
            int best_index = 0;
            for (int candidate = 1; candidate < top_k; ++candidate) {
                score_coords[0] = candidate;
                const float candidate_score = s_f32_ld_g(gen_addr(score_coords, scores));
                if (candidate_score > best_score) {
                    best_score = candidate_score;
                    best_index = candidate;
                }
            }

            int5 candidate_coords = {best_index, step, batch, 0, 0};
            const int token = s_i32_ld_g(gen_addr(candidate_coords, candidate_ids));
            int5 output_coords = {step, batch, 0, 0, 0};
            s_i32_st_g(gen_addr(output_coords, output), token);
            previous = best_index;
        }
    }
}
