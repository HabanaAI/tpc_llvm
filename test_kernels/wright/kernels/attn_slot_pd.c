// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_slot_p for a DFlash drafting block: every column sees the whole slot below klen (BLOCK_ALL),
// not before kmin (slot field 6), and within its window (field 7); sigma-packed P as attn_slot_p.
#define KMIN 1
#define WINDOW 1
#define BLOCK_ALL 1
#include "attn_slot_p.c"
