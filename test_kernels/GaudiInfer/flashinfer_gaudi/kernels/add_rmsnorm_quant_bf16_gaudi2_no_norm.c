// SPDX-License-Identifier: Apache-2.0
// 3-output variant (no normalized output written) for widths <= 8192.
#define FLASHINFER_NORM_CACHE_TILES 64
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#define NO_NORMALIZED_OUT
#include "add_rmsnorm_quant_bf16_gaudi2.c"
