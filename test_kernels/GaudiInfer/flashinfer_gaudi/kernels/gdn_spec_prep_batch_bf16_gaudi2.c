// SPDX-License-Identifier: Apache-2.0
// Batched 3D variant of gdn_spec_prep: seq [C, T+3, B], w [C, 4] -> X [128, 2T, 16, B], V [C-4096, T, B].
#define BATCHED
#include "gdn_spec_prep_bf16_gaudi2.c"
