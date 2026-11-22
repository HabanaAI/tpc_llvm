// SPDX-License-Identifier: Apache-2.0
// 5-input (folds FP8 GEMM dequantization) + 3-output (no normalized output written) variant for widths <= 8192.
#define FLASHINFER_NORM_CACHE_TILES 64
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#define NO_NORMALIZED_OUT
#define FOLD_DEQUANT
#include "add_rmsnorm_quant_bf16_gaudi2.c"
