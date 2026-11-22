// SPDX-License-Identifier: Apache-2.0
// Private ISA/capability prototype, absent from serving. The checkpoint's
// exactly decoded BF16 weights have four zero low bits. Pack the high byte
// separately from the four low mantissa bits; the activation stays BF16.
// Low-plane packing must match the qualified LSU/conversion lane permutation.
void main(tensor high, tensor low, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int block_n = start[1]; block_n < end[1]; ++block_n) {
        for (int block_k = start[0]; block_k < end[0]; ++block_k) {
            for (int row = 0; row < 32; ++row) {
                const int n = block_n * 32 + row;
                // Only the four payload bits may enter the mantissa. Mask
                // extension bits before widening, irrespective of tensor
                // signedness or the LSU's byte-return representation.
                const uchar256 nibbles = v_u8_ld_tnsr_b(
                    (int5){block_k * 128, n}, low, SW_UNPACK | SW_UNPCK_4_TO_8) & 15;
                // Native lane order deliberately avoids SW_LINEAR shuffles.
                // Offline weight packing supplies the matching permutation.
                const ushort256 tails = convert_uchar256_to_ushort256(nibbles, 0);
                const ushort128 first = v_u16_ld_tnsr_b(
                    (int5){block_k * 256, n}, high, SW_UNPACK | SW_UNPCK_8_TO_16);
                const ushort128 second = v_u16_ld_tnsr_b(
                    (int5){block_k * 256 + 128, n}, high, SW_UNPACK | SW_UNPCK_8_TO_16);
                const ushort128 bits0 = (first << 8) | (tails.v1 << 4);
                const ushort128 bits1 = (second << 8) | (tails.v2 << 4);
                v_bf16_st_tnsr((int5){block_k * 256, n}, output, *((bfloat128*)&bits0));
                v_bf16_st_tnsr((int5){block_k * 256 + 128, n}, output, *((bfloat128*)&bits1));
            }
        }
    }
}
