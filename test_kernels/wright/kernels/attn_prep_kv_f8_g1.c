// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// attn_prep FP8, kv part, 1 KV group (a tensor-parallel rank of Qwen3.8-27B at 4 or 8 ranks): see attn_prep.c
#define NG 1
#define FP8 1
#define PART_KV 1
#include "attn_prep.c"
