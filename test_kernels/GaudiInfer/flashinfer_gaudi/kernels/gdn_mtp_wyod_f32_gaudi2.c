// SPDX-License-Identifier: Apache-2.0
// T=16 build of the WY-form MTP verification core that outputs both core outputs and per-token deltas ds_t
// (used by gdn_state_commit_f32_gaudi2 to update the GDN state in one pass without recomputing gdn_chunk).
#define NO_PHASE2
#define BETA_F32
#define RMAT_KH
#define OUTPUT_DS
#include "gdn_mtp_wy_f32_gaudi2.c"
