// SPDX-License-Identifier: Apache-2.0
// Four independent request rows reuse each control-weight vector. Every
// request retains the parent's K traversal, F32 MAC and final reduction.
void main(tensor activation, tensor weight, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int tokens = get_dim_size(activation, 1);
    for (int group = begin[1]; group < end[1]; ++group) {
        const int token = group * 4;
        for (int row = begin[0]; row < end[0]; ++row) {
            float64 a0 = 0, a1 = 0, a2 = 0, a3 = 0;
            for (int k = 0; k < 20480; k += 64) {
                const float64 w = v_f32_ld_tnsr_b((int5){k, row}, weight);
                a0 = v_f32_mac_b(v_f32_ld_tnsr_b((int5){k, token}, activation), w, a0);
                if (token + 1 < tokens)
                    a1 = v_f32_mac_b(v_f32_ld_tnsr_b((int5){k, token + 1}, activation), w, a1);
                if (token + 2 < tokens)
                    a2 = v_f32_mac_b(v_f32_ld_tnsr_b((int5){k, token + 2}, activation), w, a2);
                if (token + 3 < tokens)
                    a3 = v_f32_mac_b(v_f32_ld_tnsr_b((int5){k, token + 3}, activation), w, a3);
            }
            v_f32_st_tnsr_partial((int5){row, token}, output, v_f32_reduce_add(a0), 0, 0);
            if (token + 1 < tokens)
                v_f32_st_tnsr_partial((int5){row, token + 1}, output, v_f32_reduce_add(a1), 0, 0);
            if (token + 2 < tokens)
                v_f32_st_tnsr_partial((int5){row, token + 2}, output, v_f32_reduce_add(a2), 0, 0);
            if (token + 3 < tokens)
                v_f32_st_tnsr_partial((int5){row, token + 3}, output, v_f32_reduce_add(a3), 0, 0);
        }
    }
}
