//===--- TPC.cpp - Implement TPC target feature support -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements TPC TargetInfo objects.
//
//===----------------------------------------------------------------------===//
#ifdef LLVM_TPC_COMPILER
#include "TPC.h"
#include "Targets.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"

namespace clang {
namespace targets {

const Builtin::Info TPCTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  { #ID, TYPE, ATTRS, nullptr, RC99_LANG, nullptr },
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  { #ID, TYPE, ATTRS, nullptr, RC99_LANG, FEATURE },
#define TPC_BUILTIN(ID, TYPE, ATTRS, FEATURE, DEFARG)                          \
  { #ID, TYPE, ATTRS, nullptr, RC99_LANG, FEATURE, DEFARG },
#include "clang/Basic/BuiltinsTPC.def"
};


static const char * const GCCRegNames[] = {
  // VRF
  "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",
  "v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
  "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
  "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
  "v32", "v33", "v34", "v35", "v36", "v37", "v38", "v39",
  "v40",
  "lfsr", "lfsr_no_change", "v_lane_id_32", "v_lane_id_16", "v_lane_id_8",
  // VPRF
  "vp0", "vp1", "vp2",  "vp3",  "vp4",  "vp5",  "vp6",  "vp7",
  "vp8", "vp9", "vp10", "vp11", "vp12", "vp13", "vp14", "vp15",
  // SRF
  "s0",  "s1",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
  "s8",  "s9",  "s10", "s11", "s12", "s13", "s14", "s15",
  "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",
  "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31",
  "s32", "s33", "s34", "s35", "s36",
  "s_slfr", "s_lsfr_no_change",
  // SPRF
  "sp0", "sp1","sp2","sp3","sp4","sp5","sp6","sp7",
  // IRF
  "i0",  "i1",  "i2",  "i3",  "i4",  "i5",  "i6",  "i7",
  "i8",  "i9",  "i10", "i11", "i12", "i13", "i14", "i15",
  "i16", "i17", "i18", "i19", "i20", "i21", "i22", "i23",
  "i24", "i25", "i26", "i27", "i28", "i29", "i30", "i31",
  // ADRF
  "ad0", "ad1", "ad2", "ad3", "ad4", "ad5", "ad6", "ad7",
  // ARF
  "a0", "a4", "a8", "a12", "a16", "a20", "a24", "a28", "a32", "a36",
  // DRF
  "d0",  "d2",  "a4",  "d6",  "a8",  "d10", "a12", "d14", "a16", "d18",
  "a20", "d22", "a24", "d26", "a28", "d30", "a32", "d34", "a36", "d38"
};


TPCTargetInfo::TPCTargetInfo(const llvm::Triple &Triple, const TargetOptions &TO)
  : TargetInfo(Triple) {
  BigEndian = false;
  TLSSupported = false;
  IntWidth = 32;
  IntAlign = 32;
  LongWidth = 32;
  LongLongWidth = 64;
  LongAlign = LongLongAlign = 32;
  PointerWidth = 32;
  PointerAlign = 32;
  AddrSpaceMap = &TPCAddrSpaceMap;
  UseAddrSpaceMapMangling = true;

  Float8Align = 32;
  Float8Width = 8;
  BFloat16Align = 32;
  BFloat16Width = 16;
  HalfWidth = 16;
  HalfAlign = 16;
  FloatWidth = 32;
  FloatAlign = 32;
  DoubleWidth = 64;
  DoubleAlign = 64;

  SuitableAlign = 32;
  SizeType = UnsignedInt;
  IntMaxType = SignedLongLong;
  IntPtrType = SignedInt;
  PtrDiffType = SignedInt;
  SigAtomicType = SignedLong;
  resetDataLayout(DataLayoutStringTPC);
}

TPCTargetInfo::CPUKind TPCTargetInfo::getCPUKind(StringRef Name) const {
  return llvm::StringSwitch<CPUKind>(Name)
    .Cases("dali", "goya", CPUKind::Goya)
    .Case("gaudi", CPUKind::Gaudi)
    .Default(CPUKind::Generic);
}

bool TPCTargetInfo::checkCPUKind(CPUKind Kind) const {
  // Perform any per-CPU checks necessary to determine if this CPU is
  // acceptable.
  switch (Kind) {
  case CPUKind::Generic:
    // No processor selected!
    return false;
  case CPUKind::Goya:
  case CPUKind::Gaudi:
    return true;
  }
  llvm_unreachable("Unhandled CPU kind");
}

bool TPCTargetInfo::isValidCPUName(StringRef Name) const {
  return checkCPUKind(getCPUKind(Name));
}

void TPCTargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.emplace_back("goya");
  Values.emplace_back("gaudi");
}

bool TPCTargetInfo::setCPU(const std::string &Name) {
  return checkCPUKind(CPU = getCPUKind(Name));
}


void TPCTargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  DefineStd(Builder, "tpc", Opts);
  Builder.defineMacro("__TPC__");
  if (Opts.NumTensors)
    Builder.defineMacro("MAX_TENSORS", Twine(Opts.NumTensors));
  else
    Builder.defineMacro("MAX_TENSORS", Twine(8));

  switch (CPU) {
  case CPUKind::Goya:
    Builder.defineMacro("__dali__");
    Builder.defineMacro("__goya__");
    break;
  case CPUKind::Gaudi:
    Builder.defineMacro("__gaudi__");
    Builder.defineMacro("__gaudi_plus__");
    break;
  default:
    llvm_unreachable("Invalid processor");
  }

  Builder.defineMacro("MAX_VLM", Twine(80*1024));

  switch (CPU) {
    case CPUKind::Goya:
    case CPUKind::Gaudi:
      Builder.defineMacro("MAX_SLM", Twine(1024));
      break;
    default:
      llvm_unreachable("Invalid processor");
  }

  if (Opts.LongIRF)
    Builder.defineMacro("__LONG_IRF__");
}

bool TPCTargetInfo::initFeatureMap(llvm::StringMap<bool> &Features,
                                   DiagnosticsEngine &Diags,
                                   StringRef CPU,
                   const std::vector<std::string> &FeaturesVec) const {
  if (CPU.equals("goya"))
    Features["goya"] = true;
  else if (CPU.equals("dali"))
    Features["goya"] = true;
  else if (CPU.equals("gaudi"))
    Features["gaudi"] = true;

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

bool TPCTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
    .Case("gaudi", CPU == CPUKind::Gaudi)
    .Cases("goya", "dali", CPU == CPUKind::Goya)
    .Default(false);
}

bool TPCTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                         DiagnosticsEngine &Diags) {
  return true;
}

ArrayRef<TargetInfo::GCCRegAlias> TPCTargetInfo::getGCCRegAliases() const {
  // TODO: implement
  return None;
}

const char *TPCTargetInfo::getClobbers() const {
  // TODO: implement
  return "";
}

bool TPCTargetInfo::validateAsmConstraint(const char *&Name,
                                          TargetInfo::ConstraintInfo &Info) const {
  // TODO: implement
  switch (*Name) {
  default:
    return false;
  case 'v': // Vector registers.
  case 's': // Scalar registers.
  case 'S': // Scalar predicate registers.
  case 'V': // Vector predicate registers.
    Info.setAllowsRegister();
    return true;
  }
}

ArrayRef<Builtin::Info> TPCTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo,
                            clang::TPC::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

ArrayRef<const char *> TPCTargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

}
}
#endif
