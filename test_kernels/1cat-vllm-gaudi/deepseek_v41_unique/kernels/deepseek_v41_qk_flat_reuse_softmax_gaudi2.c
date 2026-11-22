// SPDX-License-Identifier: Apache-2.0
// Same FP32 softmax, reading only each query diagonal directly.
#define DSV41_MLA_FLAT_QK 1
#define DSV41_MLA_FLAT_QK_SPLIT 1
#include "../../deepseek_v4/kernels/deepseek_v41_selected_mla_softmax_gaudi2.c"
