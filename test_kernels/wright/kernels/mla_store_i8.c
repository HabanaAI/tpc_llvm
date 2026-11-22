// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mla_store with power-of-two token scales and FP8 index keys (DeepSeek-V4.1; see mla_store.c): tensor 3 is the
// index key (128, T) f32, stored as FP8 E4M3 with a power-of-two scale (max |key| / 240 rounded up) in the
// 128-byte unit rows[2, n] (unit u = row u >> 2, bytes (u & 3) * 128: four keys per row), its scale as f32 at
// byte 4 rows[6, n] of row rows[5, n] (an index page is 32 rows of 128 keys and one row of their scales).
#define POW2
#define IX8
#include "mla_store.c"
