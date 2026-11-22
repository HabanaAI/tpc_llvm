// SPDX-License-Identifier: Apache-2.0
// Preserve C1 arithmetic while removing the dynamic row-sized local cache.
#define DSV41_QNORM_REMATERIALIZE 1
#define DSV41_QNORM_PUBLISH 1
#define DSV41_QNORM_TILES 40
#include "../../deepseek_v4/kernels/deepseek_v41_qnorm_quant_gaudi2.c"
