// SPDX-License-Identifier: Apache-2.0
// masked_softmax_bf16 writing FP8 E4M3 probabilities scaled by 128 directly (and row sums scaled by 128),
// eliminating the post-softmax BF16->FP8 conversion pass when G2E_KV_FP8=1.
#define OUT_FP8
#include "masked_softmax_bf16_gaudi2.c"
