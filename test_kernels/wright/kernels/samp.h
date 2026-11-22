// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Shared definitions of the sampling kernels (samp_*.c).
// Order keys order logits as signed i32: bits for x >= 0, bits ^ 0x7FFFFFFF for x < 0; -0.0 keys
// as +0.0, NaN as KEY_MAX (torch.argmax counts NaN as the maximum); absent tokens are KEY_MIN.
// Weights exp(d) (d <= 0) below f32's normal range count as 0.
#pragma once
#include "common.h"

#define KEY_MIN ((int)0x80000000)
#define KEY_MAX 0x7FFFFFFF
#define INF_BITS 0x7F800000
#define NO_MAX (-3.0e38f)  // a chunk or row maximum when there is no finite logit
#define NC_MAX 64          // vocabulary chunks per row (the scan's index space dim 0)
#define DEPTH 8            // candidates per lane and chunk
#define NMAX 64            // top-logprob list length

static inline int64 order_key(float64 x)
{
    int64 b = as_int64(x);
    b = v_i32_sel_eq_i32_b(b, (int64)KEY_MIN, (int64)0, b);                 // -0.0 -> +0.0
    const int64 k = v_i32_sel_less_i32_b(b, (int64)0, b ^ (int64)KEY_MAX, b);
    const int64 mag = b & (int64)KEY_MAX;
    return v_i32_sel_grt_i32_b(mag, (int64)INF_BITS, (int64)KEY_MAX, k);  // NaN
}

static inline float64 key_value(int64 k)
{
    return as_float64(v_i32_sel_less_i32_b(k, (int64)0, k ^ (int64)KEY_MAX, k));
}

static inline float64 weight(float64 d)
{
    return v_f32_sel_grt_f32_b(d, -87.0f, nl_exp(v_f32_max_b(d, -87.0f)), 0.0f);
}

// lane k of v, broadcast (needs lanef and IDX)
#define SAMP_LANE(v, k, out)                                                                 \
    { float64 t_ = v_f32_sel_eq_f32_b(lanef, (float)(k), (v), 0.0f); NL_REDUCE_ADD(t_, IDX, out) }
