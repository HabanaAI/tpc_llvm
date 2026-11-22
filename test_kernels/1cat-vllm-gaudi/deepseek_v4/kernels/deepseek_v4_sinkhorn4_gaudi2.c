// SPDX-License-Identifier: Apache-2.0
// Four independent matrices occupy the four dual groups of one FP32 vector.
static inline uchar256 direction(uint64 index) {
    const uint64 selected = (index & 7) | ((index & 8) << 2) | 0x80;
    uint256 packed;
    packed.v1 = selected;
    packed.v2 = selected;
    packed.v3 = selected;
    packed.v4 = selected;
    return v_convert_u32_to_u8_all_b(packed);
}

void main(tensor input, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(input, 0);
    const uint64 lane = read_lane_id_4b_b();
    const uchar256 r0 = direction(lane & 0xfffffffc);
    const uchar256 r1 = direction((lane & 0xfffffffc) | 1);
    const uchar256 r2 = direction((lane & 0xfffffffc) | 2);
    const uchar256 r3 = direction((lane & 0xfffffffc) | 3);
    const uchar256 c0 = direction(lane & 3);
    const uchar256 c1 = direction((lane & 3) | 4);
    const uchar256 c2 = direction((lane & 3) | 8);
    const uchar256 c3 = direction((lane & 3) | 12);
    for (int block = start[0]; block < end[0]; ++block) {
        const int5 at = {block * 64, 0, 0, 0, 0};
        const int last = s_i32_min(63, width - block * 64 - 1);
        float64 values = v_f32_ld_tnsr_partial_b(at, input, last, 0);
        for (int iteration = 0; iteration < 20; ++iteration) {
            if (iteration != 0) {
                const float64 a = v_f32_shuffle_b(values, r0, 0, 0.0f)
                                + v_f32_shuffle_b(values, r1, 0, 0.0f);
                const float64 b = v_f32_shuffle_b(values, r2, 0, 0.0f)
                                + v_f32_shuffle_b(values, r3, 0, 0.0f);
                values /= (a + b) + 1.0e-6f;
            }
            float64 sum = 0.0f;
            sum += v_f32_shuffle_b(values, c0, 0, 0.0f);
            sum += v_f32_shuffle_b(values, c1, 0, 0.0f);
            sum += v_f32_shuffle_b(values, c2, 0, 0.0f);
            sum += v_f32_shuffle_b(values, c3, 0, 0.0f);
            values /= sum + 1.0e-6f;
        }
        v_f32_st_tnsr_partial(at, output, values, last, 0);
    }
}
