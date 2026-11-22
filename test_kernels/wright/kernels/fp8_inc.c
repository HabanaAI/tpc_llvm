// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// e4m3 weight bytes -> fp8_inc, load time: the value halves (the block scales double on the host),
// bytes equal to checkpoint.HALVE: mag >= 16 drops one exponent; below that the subnormals halve
// rounding to even; 0x7f (NaN) becomes 0. Byte positions are kept.
// tensors: 0 w (256, N) u8 | 1 out (256, N) u8.  index space: [N]
#include "common.h"

void main(tensor w, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    for (int n = off[0]; n < end[0]; n++) {
        int5 c = {0, n, 0, 0, 0};
        const uchar256 b = v_u8_ld_tnsr_b(c, w);
        const uchar256 mag = b & (unsigned char)0x7F;
        const uchar256 lo3 = v_u8_sel_eq_u8_b(mag & (unsigned char)3, (unsigned char)3, (uchar256)(unsigned char)1, (uchar256)(unsigned char)0);
        const uchar256 sub = v_u8_shr_b(mag + lo3, (unsigned char)1);
        const uchar256 v = v_u8_sel_less_u8_b(mag, (unsigned char)16, sub, mag - (unsigned char)8);
        const uchar256 signed_ = v | (b & (unsigned char)0x80);
        const uchar256 r = v_u8_sel_eq_u8_b(mag, (unsigned char)0x7F, (uchar256)(unsigned char)0, signed_);
        v_u8_st_tnsr(c, out, r);
    }
}
