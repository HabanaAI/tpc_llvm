// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// ds_ix_slot_acc over FP8 queries and keys (DeepSeek-V4.1; see ix_slot_acc.c): S is the product of the FP8 values,
// and each term takes its query column's scale sq (NH Q, S) and its key's scale sk (C, S) (both > 0, so they
// move past the relu); tensors 4 and 5 are sq and sk, the sink follows.
#define RATIO
#define SCALED
#include "ix_slot_acc.c"
