// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// PLE n-gram rows: gathered table bytes (e4m3) -> bf16 GEMM operand, per token.
// Row (head h, token t) = e[0..D) at column 16 t + h (D = 160 <= 256; the load pads with zeros).
// A 256-byte load unpacks to v_j[i] = byte 4 i + j; stored as pack_sigma(v1, v2) (bytes 4i, 4i+1)
// then pack_sigma(v3, v4) (bytes 4i+2, 4i+3) in a 256-row block per head. The key/value GEMM
// weight columns are permuted on the host to match (zero columns for bytes >= D).
// tensors: 0 e (D, NH*T) u8 (fp8 bits) | 1 out (256*NH, T) bf16.  index space: [T]
#include "common.h"

void main(tensor e, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NH = get_dim_size(out, 0) / 256;
    for (int t = off[0]; t < end[0]; t++) {
        for (int h = 0; h < NH; h++) {
            int5 c = {0, NH * t + h, 0, 0, 0};
            const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(c, e), 0);
            int5 oc = {256 * h, t, 0, 0, 0};
            v_bf16_st_tnsr(oc, out, pack_sigma(f.v1, f.v2));
            oc[0] += 128;
            v_bf16_st_tnsr(oc, out, pack_sigma(f.v3, f.v4));
        }
    }
}
