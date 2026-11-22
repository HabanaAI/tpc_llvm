// SPDX-License-Identifier: Apache-2.0
// Cold BF16 weights, BF16 products with FP32 accumulation. This is a numerical
// candidate, qualified against the upstream mHC tolerance rather than bit equality.
#define DSV41_CONTROL_BF16_WEIGHT
#include "deepseek_v41_control_rrms_parallel_bf16_gaudi2.c"
