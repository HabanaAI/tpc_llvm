// SPDX-License-Identifier: Apache-2.0
// Offline ISA prototype. Not registered or selected by any serving path.
// Checkpoint E4M3FN + UE8M0 -> exactly the loader's BF16 weights. Keep the
// source's exponent15/subnormal/negative-zero semantics, not native FP8's.
// Loader must reject nonfinite codes and scales outside [16,239].
#ifndef DSV41_DENSE_SOURCE_UNPACK
#define DSV41_DENSE_SOURCE_UNPACK 0
#endif
#ifndef DSV41_DENSE_SOURCE_INTEGER_TINY
#define DSV41_DENSE_SOURCE_INTEGER_TINY 0
#endif
#ifndef DSV41_DENSE_SOURCE_SIGNED
#define DSV41_DENSE_SOURCE_SIGNED 0
#endif
static inline bfloat128 dense_source_bits(ushort128 code, ushort128 normal_offset,
                                         ushort128 subnormal_offset) {
    const ushort128 magnitude = code & 127;
#if DSV41_DENSE_SOURCE_INTEGER_TINY
    // Integer 0..127 -> BF16 is exact. Only magnitudes 0..7 consume this
    // result; one native conversion replaces the mantissa normalization tree.
    const bfloat128 integer = convert_ushort128_to_bfloat128(magnitude, SW_RHNE);
    ushort128 tiny = *((ushort128*)&integer);
#else
    const ushort128 mantissa = magnitude & 7;
    ushort128 tiny = (mantissa << 5) + 0x3b80;
    tiny = v_u16_sel_less_u16_b(mantissa, 4, (mantissa << 6) + 0x3b00, tiny);
    tiny = v_u16_sel_less_u16_b(mantissa, 2, 0x3b00, tiny);
#endif
    tiny += subnormal_offset;
    ushort128 bits = (magnitude << 4) + normal_offset;
    bits = v_u16_sel_less_u16_b(magnitude, 8, tiny, bits);
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
#if DSV41_DENSE_SOURCE_SIGNED
    bits |= code & 0x8000;
#else
    bits |= (code & 128) << 8;
#endif
    return *((bfloat128*)&bits);
}

void main(tensor weight, tensor scale, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const uchar256 lanes = V_LANE_ID_8;
    const uchar256 direction = (((lanes >> 6) << 1) + (lanes & 1)) | 0x80;
    for (int block_n = begin[1]; block_n < end[1]; ++block_n) {
        for (int block_k = begin[0]; block_k < end[0]; ++block_k) {
            const ushort128 scales = v_u16_ld_tnsr_partial_b(
                (int5){block_k * 4, block_n}, scale, 3, 0);
            const uchar256 repeated = v_u8_shuffle_b(*((uchar256*)&scales), direction,
                                                    0, (uchar256){0});
            const ushort128 codes = *((ushort128*)&repeated);
            const ushort128 normal_offset = (codes - 7) << 7;
            const ushort128 subnormal_offset = (codes -
                (DSV41_DENSE_SOURCE_INTEGER_TINY ? 136 : 127)) << 7;
            for (int row = 0; row < 32; ++row) {
                const int5 coordinate = {block_k * 128, block_n * 32 + row};
#if DSV41_DENSE_SOURCE_SIGNED
                // Sign extension supplies the final BF16 sign bit directly.
                // An actual I8 tensor/8-to-16 LSU binding still needs a device
                // capability check; this file remains an unregistered prototype.
                const short128 signed_source = v_i16_ld_tnsr_b(
                    coordinate, weight, SW_UNPACK | SW_UNPCK_8_TO_16);
                const ushort128 source = *((ushort128*)&signed_source);
#elif DSV41_DENSE_SOURCE_UNPACK
                // ISA-only alternative: expand unsigned bytes in the load
                // pipe instead of the linear conversion's register shuffles.
                // Its actual U8 tensor binding is not device-qualified.
                const ushort128 source = v_u16_ld_tnsr_b(
                    coordinate, weight, SW_UNPACK | SW_UNPCK_8_TO_16);
#else
                const uchar256 raw = v_u8_ld_tnsr_partial_b(coordinate, weight, 127, 0);
                const ushort128 source = convert_uchar256_to_ushort256(raw, SW_LINEAR).v1;
#endif
                const bfloat128 value = dense_source_bits(source, normal_offset, subnormal_offset);
                v_bf16_st_tnsr(coordinate, output, value);
            }
        }
    }
}
