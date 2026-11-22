// SPDX-License-Identifier: Apache-2.0
// One rounded BF16 bank is both QK/PV operand and reusable public cache.
#define DSV41_LOGICAL_MLA_OPERANDS 1
#define DSV41_SELECTED_SCALE_CACHE 1
#define DSV41_MLA_BF16_KV_ONLY 1
#define DSV41_MLA_SINGLE_BANK 1
#define DSV41_MLA_SINGLE_BANK_LOOPED 1
#include "deepseek_v41_selected_kv_vector.h"
