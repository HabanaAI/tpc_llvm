// SPDX-License-Identifier: Apache-2.0
// Preserve the qualified row-scale codec; omit the redundant widened KV.
#define DSV41_LOGICAL_MLA_OPERANDS 1
#define DSV41_SELECTED_SCALE_CACHE 1
#define DSV41_MLA_BF16_KV_ONLY 1
#include "deepseek_v41_selected_kv_vector.h"
