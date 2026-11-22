// SPDX-License-Identifier: Apache-2.0
// T=8 build of the output-only WY-form MTP verification core (the batched verification at width 8): as
// gdn_mtp_wyo_f32_gaudi2.c.
#define NO_PHASE2
#define BETA_F32  // beta F32 [48, T, B]
#define RMAT_KH   // rmat F32 [384, 2T, 16, B]; no source / kvals inputs
#define TOKENS 8
#include "gdn_mtp_wy_f32_gaudi2.c"
