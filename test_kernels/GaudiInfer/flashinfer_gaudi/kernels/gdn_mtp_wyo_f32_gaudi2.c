// SPDX-License-Identifier: Apache-2.0
// T=16 build of the WY-form MTP verification core that only produces the outputs (no state checkpoints):
// the g2e engine recomputes the accepted state in its commit recipe.
#define NO_PHASE2
#define BETA_F32  // beta F32 [48, T, B]
#define RMAT_KH   // rmat F32 [384, 2T, 16, B]; no source / kvals inputs
#include "gdn_mtp_wy_f32_gaudi2.c"
