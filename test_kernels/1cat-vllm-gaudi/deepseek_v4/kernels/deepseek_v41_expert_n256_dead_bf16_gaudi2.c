// SPDX-License-Identifier: Apache-2.0
// Invalid expert groups have no live route writer. Their weight/output bytes
// are unspecified and must never be exposed through the general decode API.
#define DSV41_N256_FP8 0
#define DSV41_N256_DEAD_PADDING 1
#include "deepseek_v41_prefill_expert_n256.h"
