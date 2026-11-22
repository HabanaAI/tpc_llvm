// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_slot_p for a causal drafting block (DSpine): column m sees the keys of the slot up to its own
// position and below klen, not before kmin (slot field 6), and within its window (field 7); sigma-packed
// P as attn_slot_p.
#define KMIN 1
#define WINDOW 1
#include "attn_slot_p.c"
