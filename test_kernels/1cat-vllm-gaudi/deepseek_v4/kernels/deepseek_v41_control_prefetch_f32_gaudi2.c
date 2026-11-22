// SPDX-License-Identifier: Apache-2.0
// Load four vectors ahead while retaining the original ordered FP32 MACs.
// A work point owns a complete row; the fixed width has no partial K tile.
void main(tensor activation, tensor weight, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = get_index_space_size() + start;
    for (int token = start[1]; token < end[1]; ++token) {
        for (int row = start[0]; row < end[0]; ++row) {
            float64 accumulator = 0.0f;
            for (int k = 0; k < 20480; k += 256) {
                int5 ac = {k, token, 0, 0, 0};
                int5 wc = {k, row, 0, 0, 0};
                float64 a0 = v_f32_ld_tnsr_b(ac, activation);
                float64 w0 = v_f32_ld_tnsr_b(wc, weight);
                ac[0] += 64;
                wc[0] += 64;
                float64 a1 = v_f32_ld_tnsr_b(ac, activation);
                float64 w1 = v_f32_ld_tnsr_b(wc, weight);
                ac[0] += 64;
                wc[0] += 64;
                float64 a2 = v_f32_ld_tnsr_b(ac, activation);
                float64 w2 = v_f32_ld_tnsr_b(wc, weight);
                ac[0] += 64;
                wc[0] += 64;
                float64 a3 = v_f32_ld_tnsr_b(ac, activation);
                float64 w3 = v_f32_ld_tnsr_b(wc, weight);
                accumulator = v_f32_mac_b(a0, w0, accumulator);
                accumulator = v_f32_mac_b(a1, w1, accumulator);
                accumulator = v_f32_mac_b(a2, w2, accumulator);
                accumulator = v_f32_mac_b(a3, w3, accumulator);
            }
            int5 oc = {row, token, 0, 0, 0};
            v_f32_st_tnsr_partial(oc, output, v_f32_reduce_add(accumulator), 0, 0);
        }
    }
}
