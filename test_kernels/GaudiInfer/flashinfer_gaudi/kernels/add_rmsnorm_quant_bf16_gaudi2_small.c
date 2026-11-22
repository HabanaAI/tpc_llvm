// SPDX-License-Identifier: Apache-2.0
// Lookup rsqrt leaves 16 KiB VLM: instantiate only for widths <= 8192.
#define FLASHINFER_NORM_CACHE_TILES 64
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "add_rmsnorm_quant_bf16_gaudi2.c"
