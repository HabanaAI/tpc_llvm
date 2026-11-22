// SPDX-License-Identifier: Apache-2.0
// One M1..6 FP8 matrix per count-ranked owner.  The existing public metadata
// rows and decoder predicates are retained; only plane6 means "owner active".
#define DSV41_GROUPED_RANKED 1
#define DSV41_GROUPED_FUSED_CONTROLS 1
#define DSV41_GROUPED_DECODER_PREDICATES 1
#define DSV41_GROUPED_COMPACT 1
#define DSV41_GROUPED_SIMPLE_FP8 1
#include "route_metadata.h"
