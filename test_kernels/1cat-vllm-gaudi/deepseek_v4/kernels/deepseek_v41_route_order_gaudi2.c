// SPDX-License-Identifier: Apache-2.0
// Count lexicographically smaller (expert, original route) keys in registers.
void main(tensor ids, tensor inverse)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int routes = get_dim_size(ids, 0);
    const int64 lanes = as_int64(read_lane_id_4b_b());
    for (int original = start[0]; original < end[0]; ++original) {
        const int expert = s_i32_ld_g(gen_addr((int5){original}, ids));
        int64 count = 0;
        for (int first = 0; first < routes; first += 64) {
            const int64 other = v_i32_ld_tnsr_b((int5){first}, ids);
            const int64 earlier = v_i32_sel_less_i32_b(lanes + first, original, 1, 0);
            int64 before = v_i32_sel_less_i32_b(other, expert, 1, 0);
            before = v_i32_sel_eq_i32_b(other, expert, earlier, before);
            count += v_i32_sel_less_i32_b(lanes + first, routes, before, 0);
        }
        count = v_i32_reduce_add(count);
        v_i32_st_tnsr_partial((int5){original}, inverse, count, 0, 0);
    }
}
