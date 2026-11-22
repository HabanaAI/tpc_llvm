// SPDX-License-Identifier: Apache-2.0
// Same control/RRMS arithmetic, one weight scan for the small query batch.
#define DSV41_CONTROL_BATCH_REUSE 1
#include "../../deepseek_v4/kernels/deepseek_v41_control_gemv_rrms_bf16_gaudi2.c"
