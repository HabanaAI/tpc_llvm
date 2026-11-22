// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// ds_q8 with power-of-two token scales, clamped SwiGLU times the token scale only (see ds_q8.c).
#define POW2
#define MODE 5
#include "ds_q8.c"
