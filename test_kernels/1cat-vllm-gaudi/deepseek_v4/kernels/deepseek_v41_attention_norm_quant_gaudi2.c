// SPDX-License-Identifier: Apache-2.0
// Share the Q projection's qualified norm and dense quantizer. The normalized
// BF16 row remains available to KV compression and index scoring.
#define DSV41_QNORM_PUBLISH 1
#define DSV41_QNORM_TILES 40
#include "deepseek_v41_qnorm_quant_gaudi2.c"
