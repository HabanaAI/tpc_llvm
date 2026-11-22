// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Store a chunk's new K and V rows into their cache pages. Token n's
// row of tensor j (K groups 0..3, then V groups 0..3) goes to arena row rows[j, n]; -1 skips it.
// K rows are in natural token order; V staging rows are sigma_inv-permuted within 128-token
// blocks (attn_prep), so token n's V row is read at sigma_inv(n). Destination rows are
// in the page format: K at page row pos % 128, V at sigma_inv(pos % 128).
// The arena is viewed as (256, PR, NB) bf16: row r is (., r % PR, r / PR).
// tensors: 0 arena  1 rows (8, N) i32  2..5 k0..k3 (256, T) bf16  6..9 v0..v3 (256, Tv) bf16
//        | 10 sink (64, 1) i32
// index space: [N]; member 0 writes the sink.
#include "common.h"

#define COPY(src, srow)                                                                      \
    { int5 sc_ = {0, (srow), 0, 0, 0};                                                       \
      const bfloat128 lo_ = v_bf16_ld_tnsr_b(sc_, src);                                      \
      sc_[0] = 128;                                                                          \
      const bfloat128 hi_ = v_bf16_ld_tnsr_b(sc_, src);                                      \
      int5 dc_ = {0, dst % PR, dst / PR, 0, 0};                                              \
      v_bf16_st_tnsr(dc_, arena, lo_);                                                       \
      dc_[0] = 128;                                                                          \
      v_bf16_st_tnsr(dc_, arena, hi_); }

void main(tensor arena, tensor rows, tensor k0, tensor k1, tensor k2, tensor k3, tensor v0,
          tensor v1, tensor v2, tensor v3, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    scalar_cache_reset();
    for (int n = off[0]; n < end[0]; n++) {
        const int vn = sigma_inv(n);
        for (int j = 0; j < 8; j++) {
            int5 rc = {j, n, 0, 0, 0};
            const int dst = s_i32_ld_g(gen_addr(rc, rows));
            if (dst < 0)
                continue;
            switch (j) {
            case 0: COPY(k0, n) break;
            case 1: COPY(k1, n) break;
            case 2: COPY(k2, n) break;
            case 3: COPY(k3, n) break;
            case 4: COPY(v0, vn) break;
            case 5: COPY(v1, vn) break;
            case 6: COPY(v2, vn) break;
            default: COPY(v3, vn)
            }
        }
    }
    if (off[0] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
