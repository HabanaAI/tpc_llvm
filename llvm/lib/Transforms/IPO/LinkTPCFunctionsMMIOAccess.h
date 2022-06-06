#ifndef LLVM_TRANSFORMS_IPO_LINK_TPC_FUNCTIONS_MMIO_ACCESS_H
#define LLVM_TRANSFORMS_IPO_LINK_TPC_FUNCTIONS_MMIO_ACCESS_H

#include "llvm/Transforms/IPO/LinkTPCHeaders.h"

#include <string>

namespace llvm {

/// Return ll code for get_dim_size LTO intrinsic.
std::string getLLCodeForGetDimSize(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_dim_size LTO intrinsic.
std::string getLLCodeForSetDimSize(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_dim_stride LTO intrinsic.
std::string getLLCodeForGetDimStride(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_dim_stride LTO intrinsic.
std::string getLLCodeForSetDimStride(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_tensor_hwpref_stride LTO intrinsic.
std::string getLLCodeForGetTensorHWPrefStride(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_tensor_hwpref_stride LTO intrinsic.
std::string getLLCodeForSetTensorHWPrefStride(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_pad_value_float LTO intrinsic.
std::string getLLCodeForGetPadValueF32(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_pad_value_float LTO intrinsic.
std::string getLLCodeForSetPadValueF32(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_pad_value_bf16 LTO intrinsic.
std::string getLLCodeForGetPadValueBF16(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_pad_value_bf16 LTO intrinsic.
std::string getLLCodeForSetPadValueBF16(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_pad_value_i32 LTO intrinsic.
std::string getLLCodeForGetPadValueI32(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_pad_value_i32 LTO intrinsic.
std::string getLLCodeForSetPadValueI32(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_pad_value_i16 LTO intrinsic.
std::string getLLCodeForGetPadValueI16(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_pad_value_i16 LTO intrinsic.
std::string getLLCodeForSetPadValueI16(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_pad_value_i8 LTO intrinsic.
std::string getLLCodeForGetPadValueI8(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_pad_value_i8 LTO intrinsic.
std::string getLLCodeForSetPadValueI8(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_semaphore_value LTO intrinsic.
std::string getLLCodeForGetSemaphoreValue(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_semaphore_value LTO intrinsic.
std::string getLLCodeForSetSemaphoreValue(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_csr_value LTO intrinsic.
std::string getLLCodeForGetCSRValue(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_csr_value LTO intrinsic.
std::string getLLCodeForSetCSRValue(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for get_convert_csr_value LTO intrinsic.
std::string getLLCodeForGetConvertCSRValue(LinkTPCHeadersPass::ArchTy Arch);

/// Return ll code for set_convert_csr_value LTO intrinsic.
std::string getLLCodeForSetConvertCSRValue(LinkTPCHeadersPass::ArchTy Arch);

}  // namespace llvm

#endif  // LLVM_TRANSFORMS_IPO_LINK_TPC_FUNCTIONS_MMIO_ACCESS_H
