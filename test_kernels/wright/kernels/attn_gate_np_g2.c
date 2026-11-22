// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_gate bf16, no past partial, 2 KV groups: see attn_gate.c
#define NG 2
#define NO_PAST 1
#include "attn_gate.c"
