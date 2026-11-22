// SPDX-License-Identifier: Apache-2.0

#define DSV4_IDENTITY_HIDDEN 4096
#define DSV4_IDENTITY_VECTOR_WIDTH 128

// An opaque BF16 materialization point. The compiler may otherwise let a
// following graph node consume the FP32 producer behind a BF16 cast, delaying
// the observable round. A custom TPC load/store fixes that semantic boundary.
void main(tensor input, tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    for (int block = index_start[0]; block < index_end[0]; ++block) {
        int5 coords = {
            block * DSV4_IDENTITY_VECTOR_WIDTH, 0, 0, 0, 0};
        const bfloat128 value = v_bf16_ld_tnsr_b(coords, input);
        v_bf16_st_tnsr(coords, output, value);
    }
}
