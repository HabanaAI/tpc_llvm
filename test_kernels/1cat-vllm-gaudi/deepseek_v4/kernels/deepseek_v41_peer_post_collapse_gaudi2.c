// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "../include/deepseek_v41_peer_post_math.h"

void main(tensor value, tensor residual, tensor post, tensor comb, tensor next_pre,
          tensor residual_out, tensor collapsed_out) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        const PostWeights weights = load_post_weights(post, comb, next_pre, row);
        for (int tile = begin[0]; tile < end[0]; ++tile)
            post_tile(value, residual, weights, residual_out, collapsed_out, row, tile, 1);
    }
}
