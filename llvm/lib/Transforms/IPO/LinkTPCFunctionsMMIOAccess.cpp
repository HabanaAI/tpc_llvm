#include "LinkTPCFunctionsMMIOAccess.h"

#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

using ArchTy = LinkTPCHeadersPass::ArchTy;

static const unsigned DimStrideOffset = 4;

// Tensor descriptor data:
//  * continuous memory block for goya ... gaudi2
//
//    Layout in memory:
//      tensor_0_data_start ... tensor_0_data_end
//      tensor_1_data_start ... tensor_1_data_end
//      ...
//      tensor_n_data_start ... tensor_n_data_end
//
//    Implementation details:
//      getDescBaseThreadPrivate == tensor_0_data_start
//      getDescBaseThreadShared  == tensor_0_data_start
//      getDescStrideThreadPrivate == tensor_1_data_start - tensor_0_data_start
//      getDescStrideThreadShared  == tensor_1_data_start - tensor_0_data_start
//
//  * separate continuous memory blocks for thread private and thread shared
//    values for doron1
//
//    Layout in memory:
//      tensor_0_thread_private_data_start ... tensor_0_thread_private_data_end
//      tensor_1_thread_private_data_start ... tensor_1_thread_private_data_end
//      ...
//      tensor_n_thread_private_data_start ... tensor_n_thread_private_data_end
//
//      tensor_0_thread_shared_data_start ... tensor_0_thread_shared_data_end
//      tensor_1_thread_shared_data_start ... tensor_1_thread_shared_data_end
//      ...
//      tensor_n_thread_shared_data_start ... tensor_n_thread_shared_data_end
//
//    Implementation details:
//      getDescBaseThreadPrivate == tensor_0_thread_private_data_start
//      getDescBaseThreadShared  == tensor_0_thread_shared_data_start
//      getDescStrideThreadPrivate == tensor_1_thread_private_data_start -
//                                    tensor_0_thread_private_data_start
//      getDescStrideThreadShared  == tensor_1_thread_shared_data_start -
//                                    tensor_0_thread_shared_data_start

static unsigned getDescBaseThreadPrivate(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
    return 0x400U;
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
    return 0;
  case ArchTy::Doron1:
    return 0x1000U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getDescBaseThreadShared(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
    return 0x400U;
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
    return 0;
  case ArchTy::Doron1:
    return 0x2000U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getDescStrideThreadPrivate(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0x4cU;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
    return 0x38U;
  case ArchTy::Gaudi2:
    return 0x50U;
  case ArchTy::Doron1:
    return 0x10U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getDescStrideThreadShared(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0x4cU;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
    return 0x38U;
  case ArchTy::Gaudi2:
    return 0x50U;
  case ArchTy::Doron1:
    return 0x48U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getSizeStrideArrOffset(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
    return 0x10U;
  case ArchTy::Doron1:
    return 0x8U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getAdrPadValueOffset(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
    return 0x8U;
  case ArchTy::Doron1:
    return 0;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getAdrPrefStrideOffset(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
    return 0x38U;
  case ArchTy::Doron1:
    return 0x8U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getSizeStrideElSize(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0xcU;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
  case ArchTy::Gaudi2:
  case ArchTy::Doron1:
    return 0x8U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getSemaphoreAddress(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0x808U;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
    return 0x908;
  case ArchTy::Greco:
    return 0xDB4U;
  case ArchTy::Gaudi2:
    return 0xD74U;
  case ArchTy::Doron1:
    return 0x1160U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getCSRAddress(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0x7fcU;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
  case ArchTy::Greco:
    return 0x8fcU;
  case ArchTy::Gaudi2:
    return 0xd68U;
  case ArchTy::Doron1:
    return 0x1170U;
  }
  llvm_unreachable("unexpected arch");
}

static unsigned getConvertCSRAddress(const ArchTy Arch) {
  switch (Arch) {
  case ArchTy::Goya:
    return 0x7fcU;
  case ArchTy::Gaudi:
  case ArchTy::GaudiB:
    return 0x8fcU;
  case ArchTy::Greco:
    return 0x7f8U;
  case ArchTy::Gaudi2:
    return 0xca8U;
  case ArchTy::Doron1:
    return 0x1158U;
  }
  llvm_unreachable("unexpected arch");
}

namespace PushImpl {

template <typename T> std::string pushFormat(const T &Arg) { return Arg; }

std::string pushFormat(const unsigned Num) { return std::to_string(Num); }
std::string pushFormat(const int Num) { return std::to_string(Num); }

static void pushN(std::string &Out) {}

template <typename T, typename... ArgsT>
static void pushN(std::string &Out, const T &Arg, ArgsT &&...Args) {
  Out += pushFormat(Arg);
  pushN(Out, Args...);
}

} // namespace PushImpl

template <typename... ArgsT> void push(std::string &Out, ArgsT &&...Args) {
  PushImpl::pushN(Out, Args...);
}

/// Return result registry number.
static unsigned pushInlineLLForDimSizeOffset(std::string &S, const ArchTy Arch,
                                             const std::string &Tensor,
                                             const std::string &Dim,
                                             unsigned FreeRegNum) {
  // return c_tensors_base +
  //        c_tensor_desc_size * tensor +
  //        c_tensor_size_stride_arr_offset +
  //        c_tensor_size_stride_element_size * dim;
  //
  // Example:
  //   %op_2 = mul i32 76, %tensor
  //   %op_4 = mul i32 12, %dim
  //   %sum_1 = add i32 1040, %op_2
  //   %sum_2 = add i32 %op_4, %sum_1
  const unsigned AbsOffset =
      getDescBaseThreadShared(Arch) + getSizeStrideArrOffset(Arch);
  const unsigned DescSize = getDescStrideThreadShared(Arch);
  const unsigned SSElSize = getSizeStrideElSize(Arch);
  const unsigned ROp2 = FreeRegNum++;
  const unsigned ROp4 = FreeRegNum++;
  const unsigned RSum = FreeRegNum++;
  const unsigned RRes = FreeRegNum;
  push(S, "  %", ROp2, " = mul i32 ", DescSize, ", ", Tensor, "\n");
  push(S, "  %", ROp4, " = mul i32 ", SSElSize, ", ", Dim, "\n");
  push(S, "  %", RSum, " = add i32 ", AbsOffset, ", %", ROp2, "\n");
  push(S, "  %", RRes, " = add i32 %", ROp4, ", %", RSum, "\n");
  return RRes;
}

/// Return result registry number.
static unsigned pushInlineLLForDimStrideOffset(std::string &S,
                                               const ArchTy Arch,
                                               const std::string &Tensor,
                                               const std::string &Dim,
                                               unsigned FreeRegNum) {
  // return c_tensors_base +
  //        c_tensor_desc_size * tensor +
  //        c_tensor_size_stride_arr_offset +
  //        c_tensor_size_stride_element_size * dim +
  //        c_dim_stride_offset;
  const unsigned AbsOffset = getDescBaseThreadShared(Arch) +
                             getSizeStrideArrOffset(Arch) + DimStrideOffset;
  const unsigned DescSize = getDescStrideThreadShared(Arch);
  const unsigned SSElSize = getSizeStrideElSize(Arch);
  const unsigned ROp2 = FreeRegNum++;
  const unsigned ROp4 = FreeRegNum++;
  const unsigned RSum = FreeRegNum++;
  const unsigned RRes = FreeRegNum;
  push(S, "  %", ROp2, " = mul i32 ", DescSize, ", ", Tensor, "\n");
  push(S, "  %", ROp4, " = mul i32 ", SSElSize, ", ", Dim, "\n");
  push(S, "  %", RSum, " = add i32 ", AbsOffset, ", %", ROp2, "\n");
  push(S, "  %", RRes, " = add i32 %", ROp4, ", %", RSum, "\n");
  return RRes;
}

/// Return result registry number.
static unsigned pushInlineLLForTensorPadOffset(std::string &S,
                                               const ArchTy Arch,
                                               const std::string &Tensor,
                                               unsigned FreeRegNum) {
  // return c_tensors_base +
  //        c_tensor_desc_size * tensor +
  //        c_tensor_addr_pad_offset;
  //
  // Example:
  //   %tpo_op2 = mul i32 76, %a
  //   %tpo_sum = add i32 1032, %op_2
  const unsigned AbsOffset =
      getDescBaseThreadShared(Arch) + getAdrPadValueOffset(Arch);
  const unsigned DescSize = getDescStrideThreadShared(Arch);
  const unsigned ROp2 = FreeRegNum++;
  const unsigned RRes = FreeRegNum;
  push(S, "  %", ROp2, " = mul i32 ", DescSize, ", ", Tensor, "\n");
  push(S, "  %", RRes, " = add i32 ", AbsOffset, ", %", ROp2, "\n");
  return RRes;
}

/// Return result registry number.
static unsigned pushInlineLLForTensorStrideOffset(std::string &S,
                                                  const ArchTy Arch,
                                                  const std::string &Tensor,
                                                  unsigned FreeRegNum) {
  // return c_tensors_base +
  //        c_tensor_addr_pref_stride_offset +
  //        c_tensor_desc_size * tensor;
  const unsigned AbsOffset =
      getDescBaseThreadPrivate(Arch) + getAdrPrefStrideOffset(Arch);
  const unsigned DescSize = getDescStrideThreadPrivate(Arch);
  const unsigned ROp3 = FreeRegNum++;
  const unsigned RRes = FreeRegNum;
  push(S, "  %", ROp3, " = mul i32 ", DescSize, ", ", Tensor, "\n");
  push(S, "  %", RRes, " = add i32 ", AbsOffset, ", %", ROp3, "\n");
  return RRes;
}

static std::string getLLCodeForGenericGetValueFunction(const char *FunctionName,
                                                       const char *Ty,
                                                       const unsigned Address) {
  std::string S;
  S.reserve(2048);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S, "define dso_local ", Ty, " @", FunctionName,
       "() local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  push(S, "  %result = call ", Ty, " @llvm.tpc.ld.l.", Ty, "(i32 ", Address,
       ", i32 1, ", Ty, " undef, i1 true, i1 false)\n");
  push(S, "  ret ", Ty, " %result\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare ", Ty, " @llvm.tpc.ld.l.", Ty, "(i32, i32, ", Ty,
       ", i1, i1) #2\n");
  S += "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind readonly }\n";
  return S;
}

static std::string getLLCodeForGenericSetValueFunction(const char *FunctionName,
                                                       const char *Ty,
                                                       const unsigned Address) {
  std::string S;
  S.reserve(2048);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S, "define dso_local void @", FunctionName, "(", Ty,
       " %value) local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  push(S, "  tail call void @llvm.tpc.st.l.", Ty, "(i32 ", Address, ", ", Ty,
       " %value, i32 1, i1 true, i1 false)\n");
  push(S, "ret void\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare void @llvm.tpc.st.l.", Ty, "(i32, ", Ty,
       ", i32, i1, i1) #2\n");
  S += "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind writeonly }";
  return S;
}

static std::string getLLCodeForGetPadValue(const ArchTy Arch,
                                           const std::string &FuncName,
                                           const std::string &LdTy,
                                           const std::string &Ty) {
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n";
  push(S, "define dso_local ", Ty, " ", FuncName,
       "(i32 %a) local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  const unsigned AdrRegN = pushInlineLLForTensorPadOffset(S, Arch, "%a", 0);
  const unsigned ResRegN = AdrRegN + 1;
  push(S, "  %", ResRegN, " = call ", Ty, " @llvm.tpc.ld.l.", LdTy, "(i32 %",
       AdrRegN, ", i32 1, ", Ty, " undef, i1 true, i1 false)\n");
  push(S, "  ret ", Ty, " %", ResRegN, "\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare ", Ty, " @llvm.tpc.ld.l.", LdTy, "(i32, i32, ", Ty,
       ", i1, i1) #2\n");
  S += "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind readonly }\n";
  return S;
}

static std::string getLLCodeForSetPadValue(const ArchTy Arch,
                                           const std::string &FuncName,
                                           const std::string &StTy,
                                           const std::string &Ty) {
  std::string S;
  S.reserve(2048);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S, "define dso_local void ", FuncName, "(i32 %a, ", Ty,
       " %value) local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  const unsigned AdrRegN = pushInlineLLForTensorPadOffset(S, Arch, "%a", 0);
  push(S, "  tail call void @llvm.tpc.st.l.", StTy, "(i32 %", AdrRegN, ", ", Ty,
       " %value, i32 1, i1 true, i1 false)\n");
  S += "  ret void\n"
       "}\n"
       "\n";
  push(S, "declare void @llvm.tpc.st.l.", StTy, "(i32, ", Ty,
       ", i32, i1, i1) #2\n");
  S += "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind writeonly }\n";
  return S;
}

static std::string getLLCodeForSetPadValueTyI16(const ArchTy Arch,
                                                const std::string &FuncName) {
  std::string S;
  S.reserve(2048);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S, "define dso_local void ", FuncName,
       "(i32 %a, i16 %value) local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  push(S, "  %0 = zext i16 %value to i32\n");
  push(S, "  %1 = shl nuw i32 %0, 16\n");
  push(S, "  %2 = or i32 %1, %0\n");
  const unsigned AdrRegNum = pushInlineLLForTensorPadOffset(S, Arch, "%a", 3);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegNum,
       ", i32 %2, "
       "i32 1, i1 true, i1 false)\n");
  push(S, "  ret void\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n");
  push(S, "\n");
  push(S, "attributes #1 = { alwaysinline nounwind }\n");
  push(S, "attributes #2 = { nounwind writeonly }\n");
  return S;
}

static std::string getLLCodeForSetPadValueTyI8(const ArchTy Arch,
                                               const std::string &FuncName) {
  std::string S;
  S.reserve(2048);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S, "define dso_local void ", FuncName,
       "(i32 %a, i8 %value) local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  push(S, "  %0 = zext i8 %value to i32\n");
  push(S, "  %1 = shl nuw nsw i32 %0, 8\n");
  push(S, "  %2 = or i32 %1, %0\n");
  push(S, "  %3 = shl nuw i32 %2, 16\n");
  push(S, "  %4 = or i32 %3, %2\n");
  const unsigned AdrRegN = pushInlineLLForTensorPadOffset(S, Arch, "%a", 5);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegN,
       ", i32 %4, i32 1, i1 true, i1 false)\n");
  push(S, "  ret void\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n");
  push(S, "\n");
  push(S, "attributes #1 = { alwaysinline nounwind }\n");
  push(S, "attributes #2 = { nounwind writeonly }\n");
  return S;
}

namespace llvm {

std::string getLLCodeForGetDimSize(const ArchTy Arch) {
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n"
       "define dso_local i32 @tpc_get_dim_size(i32 %a, i32 %dim) "
       "local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForDimSizeOffset(S, Arch, "%a", "%dim", 0);
  const unsigned ResRegN = AdrRegN + 1;
  push(S, "  %", ResRegN, " = call i32 @llvm.tpc.ld.l.i32(i32 %", AdrRegN,
       ", i32 1, i32 undef, i1 true, i1 false)\n");
  push(S, "  ret i32 %", ResRegN, "\n");
  S += "}\n"
       "\n"
       "declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind readonly }\n";
  return S;
}

std::string getLLCodeForSetDimSize(const ArchTy Arch) {
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n"
       "define dso_local void @tpc_set_dim_size(i32 %a, i32 %dim, i32 %value) "
       "local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForDimSizeOffset(S, Arch, "%a", "%dim", 0);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegN,
       ", i32 %value, i32 1, i1 true, i1 false)\n");
  S += "  ret void\n"
       "}\n"
       "\n"
       "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind writeonly }\n";
  return S;
}

std::string getLLCodeForGetDimStride(const ArchTy Arch) {
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n"
       "define dso_local i32 @tpc_get_dim_stride(i32 %a, i32 %dim) "
       "local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForDimStrideOffset(S, Arch, "%a", "%dim", 0);
  const unsigned ResRegN = AdrRegN + 1;
  push(S, "  %", ResRegN, " = call i32 @llvm.tpc.ld.l.i32(i32 %", AdrRegN,
       ", i32 1, i32 undef, i1 true, i1 false)\n");
  push(S, "  ret i32 %", ResRegN, "\n");
  S += "}\n"
       "\n"
       "declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind readonly }\n";
  return S;
}

std::string getLLCodeForSetDimStride(const ArchTy Arch) {
  std::string S;
  S +=
      "; Function Attrs: alwaysinline nounwind\n"
      "define dso_local void @tpc_set_dim_stride(i32 %a, i32 %dim, i32 %value) "
      "local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForDimStrideOffset(S, Arch, "%a", "%dim", 0);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegN,
       ", i32 %value, i32 1, i1 true, i1 false)\n");
  S += "  ret void\n"
       "}\n"
       "\n"
       "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind writeonly }\n";
  return S;
}

std::string getLLCodeForGetTensorHWPrefStride(const ArchTy Arch) {
  // unsigned res = 0;
  // unsigned address = get_tensor_stride_offset_internal(tensor);
  // return s_u32_ld_l(address, SW_MMIO, res, 1, 0);
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n"
       "define dso_local i32 @tpc_get_tensor_hwpref_stride(i32 %tensor) "
       "local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForTensorStrideOffset(S, Arch, "%tensor", 0);
  const unsigned ResRegN = AdrRegN + 1;
  push(S, "  %", ResRegN, " = call i32 @llvm.tpc.ld.l.i32(i32 %", AdrRegN,
       ", i32 1, i32 undef, i1 true, i1 false)\n");
  push(S, "  ret i32 %", ResRegN, "\n");
  S += "}\n"
       "\n"
       "declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind readonly }\n";
  return S;
}

std::string getLLCodeForSetTensorHWPrefStride(const ArchTy Arch) {
  // unsigned address = get_tensor_stride_offset_internal(tensor);
  // s_u32_st_l(address, stride, SW_MMIO, 1, 0);
  std::string S;
  S.reserve(2048);
  S += "; Function Attrs: alwaysinline nounwind\n"
       "define dso_local void @tpc_set_tensor_hwpref_stride(i32 %tensor, i32 "
       "%value) local_unnamed_addr #1 {\n";
  push(S, "entry:\n");
  const unsigned AdrRegN =
      pushInlineLLForTensorStrideOffset(S, Arch, "%tensor", 0);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegN,
       ", i32 %value, i32 1, i1 true, i1 false)\n");
  S += "  ret void\n"
       "}\n"
       "\n"
       "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n"
       "\n"
       "attributes #1 = { alwaysinline nounwind }\n"
       "attributes #2 = { nounwind writeonly }\n";
  return S;
}

std::string getLLCodeForGetPadValueF32(const ArchTy Arch) {
  return getLLCodeForGetPadValue(Arch, "@tpc_get_pad_value_float", "f32",
                                 "float");
}

std::string getLLCodeForSetPadValueF32(const ArchTy Arch) {
  return getLLCodeForSetPadValue(Arch, "@tpc_set_pad_value_float", "f32",
                                 "float");
}

std::string getLLCodeForGetPadValueBF16(const ArchTy Arch) {
  return getLLCodeForGetPadValue(Arch, "@tpc_get_pad_value_bf16", "bf16",
                                 "bfloat");
}

std::string getLLCodeForSetPadValueBF16(const ArchTy Arch) {
  std::string S;
  S.reserve(1024);
  push(S, "; Function Attrs: alwaysinline nounwind\n");
  push(S,
       "define dso_local void @tpc_set_pad_value_bf16(i32 %a, bfloat %value) "
       "local_unnamed_addr #1 {\n");
  push(S, "entry:\n");
  push(S, "  %0 = bitcast bfloat %value to i16\n");
  push(S, "  %1 = zext i16 %0 to i32\n");
  push(S, "  %2 = and i32 %1, 65535\n");
  push(S, "  %3 = shl i32 %1, 16\n");
  push(S, "  %4 = or i32 %3, %2\n");
  const unsigned AdrRegN = pushInlineLLForTensorPadOffset(S, Arch, "%a", 5);
  push(S, "  tail call void @llvm.tpc.st.l.i32(i32 %", AdrRegN,
       ", i32 %3, i32 1, i1 true, i1 false)\n");
  push(S, "  ret void\n");
  push(S, "}\n");
  push(S, "\n");
  push(S, "declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2\n");
  push(S, "\n");
  push(S, "attributes #1 = { alwaysinline nounwind }\n");
  push(S, "attributes #2 = { nounwind writeonly }\n");
  return S;
}

std::string getLLCodeForGetPadValueI32(const ArchTy Arch) {
  return getLLCodeForGetPadValue(Arch, "@tpc_get_pad_value_i32", "i32", "i32");
}

std::string getLLCodeForSetPadValueI32(const ArchTy Arch) {
  return getLLCodeForSetPadValue(Arch, "@tpc_set_pad_value_i32", "i32", "i32");
}

std::string getLLCodeForGetPadValueI16(const ArchTy Arch) {
  return getLLCodeForGetPadValue(Arch, "@tpc_get_pad_value_i16", "i16", "i16");
}

std::string getLLCodeForSetPadValueI16(const ArchTy Arch) {
  return getLLCodeForSetPadValueTyI16(Arch, "@tpc_set_pad_value_i16");
}

std::string getLLCodeForGetPadValueI8(const ArchTy Arch) {
  return getLLCodeForGetPadValue(Arch, "@tpc_get_pad_value_i8", "i8", "i8");
}

std::string getLLCodeForSetPadValueI8(const ArchTy Arch) {
  return getLLCodeForSetPadValueTyI8(Arch, "@tpc_set_pad_value_i8");
}

std::string getLLCodeForGetSemaphoreValue(const ArchTy Arch) {
  return getLLCodeForGenericGetValueFunction("tpc_get_semaphore_value", "i32",
                                             getSemaphoreAddress(Arch));
}

std::string getLLCodeForSetSemaphoreValue(const ArchTy Arch) {
  return getLLCodeForGenericSetValueFunction("tpc_set_semaphore_value", "i32",
                                             getSemaphoreAddress(Arch));
}

std::string getLLCodeForGetCSRValue(ArchTy Arch) {
  return getLLCodeForGenericGetValueFunction("tpc_get_csr_value", "i32",
                                             getCSRAddress(Arch));
}

std::string getLLCodeForSetCSRValue(ArchTy Arch) {
  return getLLCodeForGenericSetValueFunction("tpc_set_csr_value", "i32",
                                             getCSRAddress(Arch));
}

std::string getLLCodeForGetConvertCSRValue(ArchTy Arch) {
  return getLLCodeForGenericGetValueFunction("tpc_get_convert_csr_value", "i32",
                                             getConvertCSRAddress(Arch));
}

std::string getLLCodeForSetConvertCSRValue(ArchTy Arch) {
  return getLLCodeForGenericSetValueFunction("tpc_set_convert_csr_value", "i32",
                                             getConvertCSRAddress(Arch));
}

} // namespace llvm
