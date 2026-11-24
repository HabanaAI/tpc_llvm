// SPDX-License-Identifier: Apache-2.0
// A matrix-vector product in f32: the 24 rows of the mHC (hyper-connection)
// control projection of DeepSeek-V4.1 decoding, each against 20480-wide
// activations, for 8 tokens. One output element per member, over the whole
// K = 20480: 320 vector loads of each operand into one MAC chain, then a lane
// reduction and a one-element partial store.
// From 1cat-vllm-gaudi, test_kernels/1cat-vllm-gaudi/deepseek_v4/kernels/deepseek_v41_control_gemv_f32_gaudi2.c; Apache-2.0.
//
// BENCH: arch gaudi2
// BENCH: in  f32 20480 8 range=-2,2
// BENCH: in  f32 20480 24 range=-0.02,0.02
// BENCH: out f32 24 8
// BENCH: index-space 24 8

// Each TPC work point owns a complete output row. No split-K or atomic sum.
void main(tensor activation, tensor weight, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = get_index_space_size() + start;
    for (int token = start[1]; token < end[1]; ++token) {
        for (int row = start[0]; row < end[0]; ++row) {
            float64 accumulator = 0.0f;
            for (int k = 0; k < 20480; k += 64) {
                int5 ac = {k, token, 0, 0, 0};
                int5 wc = {k, row, 0, 0, 0};
                float64 a = v_f32_ld_tnsr_b(ac, activation);
                float64 w = v_f32_ld_tnsr_b(wc, weight);
                accumulator = v_f32_mac_b(a, w, accumulator);
            }
            int5 oc = {row, token, 0, 0, 0};
            v_f32_st_tnsr_partial(oc, output, v_f32_reduce_add(accumulator), 0, 0);
        }
    }
}
