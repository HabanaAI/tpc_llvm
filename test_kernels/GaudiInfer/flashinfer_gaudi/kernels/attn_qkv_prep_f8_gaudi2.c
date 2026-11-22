// SPDX-License-Identifier: Apache-2.0
// attn_qkv_prep writing k_out and v_out directly in FP8 E4M3 (unit scale) for G2E_KV_FP8=1.
#define KV_FP8
#include "attn_qkv_prep_bf16_gaudi2.c"
