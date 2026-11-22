// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// out[i] = sum over s < ns of part[s * n + i] (K-split partial sums of syn_qmm), f32
// tensors: 0 part | 1 out
#include "syn_common.h"

void main(tensor part, tensor dep, tensor out, int nmem, int n, int ns)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nch = (n + 63) >> 6;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int c0, c1;
        SPLIT_RANGE(nch, nmem, m, c0, c1)
        for (int c = c0; c < c1; c++) {
            float64 a = ld_f32(part, c * 64);
            for (int s = 1; s < ns; s++) {
                a += ld_f32(part, s * n + c * 64);
            }
            ST_F32(out, c * 64, a, n - c * 64);
        }
    }
}
