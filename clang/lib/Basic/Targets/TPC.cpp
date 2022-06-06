//===--- TPC.cpp - Implement TPC target feature support -------------------===//
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

static LangASMap TPCAddrSpaceMap = {
    0, // Default
    1, // opencl_global
    0, // opencl_local
    0, // opencl_constant
    0, // opencl_private
    0, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    1, // cuda_shared
    0, // ptr32_sptr
    0, // ptr32_uptr
    0  // ptr64
};

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
  VLASupported = false;
  NoAsmVariants = true;
  HasLegalHalfType = true;
  HasFloat128 = false;
  HasFloat16 = true;
  HasBFloat16 = true;
  HasStrictFP = false;

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
    .Case("gaudib", CPUKind::GaudiB)
    .Cases("goya2", "greco", CPUKind::Greco)
    .Case("gaudi2", CPUKind::Gaudi2)
    .Case("doron1", CPUKind::Doron1)
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
  case CPUKind::GaudiB:
  case CPUKind::Greco:
  case CPUKind::Gaudi2:
  case CPUKind::Doron1:
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
  Values.emplace_back("gaudib");
  Values.emplace_back("greco");
  Values.emplace_back("gaudi2");
  Values.emplace_back("doron1");
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
  case CPUKind::GaudiB:
    Builder.defineMacro("__gaudib__");
    Builder.defineMacro("__gaudi_plus__");
    break;
  case CPUKind::Greco:
    Builder.defineMacro("__goya2__");
    Builder.defineMacro("__greco__");
    Builder.defineMacro("__gaudi_plus__");
    Builder.defineMacro("__greco_plus__");
    break;
  case CPUKind::Gaudi2:
    Builder.defineMacro("__gaudi2__");
    Builder.defineMacro("__gaudi_plus__");
    Builder.defineMacro("__greco_plus__");
    Builder.defineMacro("__gaudi2_plus__");
    break;
  case CPUKind::Doron1:
    Builder.defineMacro("__doron1__");
    Builder.defineMacro("__gaudi_plus__");
    Builder.defineMacro("__greco_plus__");
    Builder.defineMacro("__gaudi2_plus__");
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
    case CPUKind::GaudiB:
      Builder.defineMacro("MAX_SLM", Twine(1024));
      break;
    case CPUKind::Greco:
      Builder.defineMacro("MAX_SLM", Twine(2*1024));
      break;
    case CPUKind::Gaudi2:
    case CPUKind::Doron1:
      Builder.defineMacro("MAX_SLM", Twine(16*1024));
      break;
    default:
      llvm_unreachable("Invalid processor");
  }

  if (Opts.LongIRF)
    Builder.defineMacro("__LONG_IRF__");
  if (Opts.CompileForLibrary)
    Builder.defineMacro("__TPC_IR_LIBRARY__");
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
  else if (CPU.equals("gaudib"))
    Features["gaudib"] = true;
  else if (CPU.equals("greco"))
    Features["greco"] = true;
  else if (CPU.equals("goya2"))
    Features["greco"] = true;
  else if (CPU.equals("gaudi2"))
    Features["gaudi2"] = true;
  else if (CPU.equals("doron1"))
    Features["doron1"] = true;

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

bool TPCTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
    .Case("gaudi", CPU == CPUKind::Gaudi)
    .Case("gaudib", CPU == CPUKind::GaudiB)
    .Cases("goya", "dali", CPU == CPUKind::Goya)
    .Cases("greco", "goya2", CPU == CPUKind::Greco)
    .Case("gaudi2", CPU == CPUKind::Gaudi2)
    .Case("doron1", CPU == CPUKind::Doron1)
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

TargetInfo::BuiltinVaListKind TPCTargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::CharPtrBuiltinVaList;
}

const char *TPCTargetInfo::getBFloat16Mangling() const {
  return "DH";
}

}
}
#endif
