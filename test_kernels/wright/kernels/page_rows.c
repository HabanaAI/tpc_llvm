// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Gather 512-byte rows of a large input tensor (the cache arena) through a runtime row table.
// The arena is viewed as (128, PR, NB) i32: row r is (., r % PR, r / PR).
// Rows of -1 are written as zeros.
// tensors: 0 src (128, PR, NB) i32  1 rows (N,) i32 | 2 out (128, N) i32.  index space: [N]
// DEP build: a trailing input tensor dep, unread.
//   tensors: 0 src  1 rows  2 dep | 3 out
#include "common.h"

#ifdef DEP
void main(tensor src, tensor rows, tensor dep, tensor out)
#else
void main(tensor src, tensor rows, tensor out)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(src, 1);
    scalar_cache_reset();
    for (int n = off[0]; n < end[0]; n++) {
        int5 rc = {n, 0, 0, 0, 0};
        const int r = s_i32_ld_g(gen_addr(rc, rows));
        int5 oc = {0, n, 0, 0, 0};
        if (r < 0) {
            v_i32_st_tnsr(oc, out, (int64)0);
            oc[0] = 64;
            v_i32_st_tnsr(oc, out, (int64)0);
            continue;
        }
        int5 sc = {0, r % PR, r / PR, 0, 0};
        const int64 a = v_i32_ld_tnsr_b(sc, src);
        sc[0] = 64;
        const int64 b = v_i32_ld_tnsr_b(sc, src);
        v_i32_st_tnsr(oc, out, a);
        oc[0] = 64;
        v_i32_st_tnsr(oc, out, b);
    }
}
