// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// ix_list over compact entries (DeepSeek-V4.1 candidate blocks, sparse scoring): each listed entry is written as
// its position, the lane of cpos (KC, T) (ix_cgather) at the entry; tensor 5 is cpos and the outputs follow it.
// KC <= 16384 and cpos < 2^18: an entry is keyed as (index << 18) | cpos.
#define MAP
#include "ix_list.c"
