// SPDX-License-Identifier: Apache-2.0
// Combine ownership lookup, two history writes and four pair row reads.
// Keep the existing framework softmax and arithmetic as the consumer.
void main(tensor kv_history, tensor score_history, tensor kv, tensor score,
          tensor positions, tensor slots, tensor pairs) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int capacity = get_dim_size(kv_history, 1) / 8;
    for (int request = begin[1]; request < end[1]; ++request) {
        const int logical = s_i32_ld_g(gen_addr((int5){request}, positions));
        const int owner = s_i32_ld_g(gen_addr((int5){request}, slots));
        const bool active = logical >= 0 && owner >= 0 && owner < capacity;
        const int ring = owner * 8 + (logical & 7);
        const int first = logical & 1;
        const int other = owner * 8 + ((logical ^ 1) & 7);
        for (int block = begin[0]; block < end[0]; ++block) {
            const int feature = block * 64;
            float64 current_value = 0, current_score = 0, other_value = 0, other_score = 0;
            if (active) {
                current_value = v_f32_ld_tnsr_b((int5){feature, request}, kv);
                current_score = v_f32_ld_tnsr_b((int5){feature, request}, score);
                other_value = v_f32_ld_tnsr_b((int5){feature, other}, kv_history);
                other_score = v_f32_ld_tnsr_b((int5){feature, other}, score_history);
                v_f32_st_tnsr((int5){feature, ring}, kv_history, current_value);
                v_f32_st_tnsr((int5){feature, ring}, score_history, current_score);
            }
            v_f32_st_tnsr((int5){feature, first, request}, pairs, current_value);
            v_f32_st_tnsr((int5){feature, first ^ 1, request}, pairs, other_value);
            v_f32_st_tnsr((int5){feature, 2 + first, request}, pairs, current_score);
            v_f32_st_tnsr((int5){feature, 2 + (first ^ 1), request}, pairs, other_score);
        }
    }
}
