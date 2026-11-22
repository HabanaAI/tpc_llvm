// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_slot_pd (the bidirectional drafting block) with one member per column (index space [M, S]).
#define KMIN 1
#define WINDOW 1
#define BLOCK_ALL 1
#define PER_COLUMN
#include "attn_slot_p.c"
