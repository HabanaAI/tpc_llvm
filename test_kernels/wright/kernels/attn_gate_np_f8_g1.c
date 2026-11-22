// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_gate FP8, no past, 1 KV group (a tensor-parallel rank of Qwen3.8-27B at 4 or 8 ranks): see attn_gate.c
#define NG 1
#define FP8 1
#define NO_PAST 1
#include "attn_gate.c"
