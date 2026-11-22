// SPDX-License-Identifier: Apache-2.0
// T=8 build of the WY-form MTP verification core that outputs both core outputs and per-token deltas ds_t.
#define TOKENS 8
#define NO_PHASE2
#define BETA_F32
#define RMAT_KH
#define OUTPUT_DS
#include "gdn_mtp_wy_f32_gaudi2.c"
