#include "TPCMachineLoopDependencyAnalysis.h"

#define GET_INSTRINFO_ENUM
#include "TPCGenInstrInfo.inc"

#define GET_REGINFO_ENUM
#include "TPCGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TPCGenSubtargetInfo.inc"

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPCInstrInfo.h"
#include "TPCMachineInstrTools.h"
#include "TPCSubtarget.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>

#define DEBUG_TYPE "mloop_da"

using namespace llvm;

namespace {
enum class AddressSpaceAccessKind {
  SLM, // Scalar Local Memory
  VLM, // Vector Local Memory
  GMS, // Global Memory (through scalar access)
  GMT, // Global Memory (through tensor access)
};
} // namespace

/// Shortcut to search instruction inside loop by condition.
template <typename Condition>
MachineInstr *findInstr(const MachineLoop &ML, const Condition &Cond) {
  for (MachineBasicBlock *MBB : ML.getBlocks()) {
    for (MachineInstr &MI : MBB->instrs()) {
      if (Cond(MI))
        return &MI;
    }
  }
  return nullptr;
}

static const char *
getAddressSpaceAccessKindName(const AddressSpaceAccessKind Kind) {
  switch (Kind) {
  // clang-format off
  case AddressSpaceAccessKind::SLM: return "SLM";
  case AddressSpaceAccessKind::VLM: return "VLM";
  case AddressSpaceAccessKind::GMS: return "Global";
  case AddressSpaceAccessKind::GMT: return "Global through tensor";
  // clang-format on
  }
  llvm_unreachable("unexpected AddressSpaceAccessKind value");
}

static Optional<AddressSpaceAccessKind>
getAddressSpaceAccessKind(const MachineInstr &MI, const TPCSubtarget &ST) {
  const MCInstrDesc &Desc = MI.getDesc();

  if (TPCII::is_ld_l(Desc) || TPCII::is_st_l(Desc))
    return AddressSpaceAccessKind::SLM;

  if (TPCII::is_ld_l_v_family(Desc) || TPCII::is_st_l_v_family(Desc))
    return AddressSpaceAccessKind::VLM;

  if (TPCII::is_ld_g(Desc) || TPCII::is_st_g(Desc))
    return AddressSpaceAccessKind::GMS;

  if (TPCII::isLdTnsr(Desc, ST.getFeatureBits()[TPC::FeatureGreco]) ||
      TPCII::isStTnsr(Desc))
    return AddressSpaceAccessKind::GMT;

  return None;
}

namespace {

/// Memory usage operations collection.
struct MemUsage {
  struct AddressSpaceUsage {
    using RegToMInstrs = DenseMap<Register, SmallVector<MachineInstr *, 4>>;

    // Mapping: register used as load source to load instructions.
    RegToMInstrs RegSrcToLD;

    // Mapping: register used as store destination to store instructions.
    RegToMInstrs RegDstToST;
  };

  AddressSpaceUsage SLMUsage; // Operations on SLM address space.
  AddressSpaceUsage VLMUsage; // Operations on VLM address space.
  AddressSpaceUsage GMSUsage; // Scalar operations on GM address space.
  AddressSpaceUsage GMTUsage; // Tensor operations on GM address space.
};
} // namespace

/// Whether machine loop is available for dependency analysis.
static bool isDependencyAnalysisSupportedForLoop(const MachineLoop &ML) {
  // Complex loops analysis is not supported.
  MachineBasicBlock *const Latch = ML.getLoopLatch();
  if (!Latch) {
    LLVM_DEBUG(dbgs() << "DA for loops wo Latch block is not implemented.\n");
    return false;
  }
  if (!ML.getSubLoops().empty()) {
    LLVM_DEBUG(dbgs() << "DA supports innermost loops only\n");
    return false;
  }

  // Check loop does not contain instructions which makes dependency analysis
  // hard or even impossible.
  const std::array<unsigned, 3> ProhibitedOpcodes = {
      TPC::ASO,
      TPC::CACHE_FLUSH,
      TPC::CACHE_INVALIDATE,
  };
  const auto IsProhibitedByOpCode = [&](const MachineInstr &MI) {
    return is_contained(ProhibitedOpcodes, MI.getOpcode());
  };
  if (MachineInstr *MI = findInstr(ML, IsProhibitedByOpCode)) {
    LLVM_DEBUG(dbgs() << "DA does not support instr: " << MI);
    return false;
  }

  // Check loop does not contain MMIO access. It makes dependency analysis hard.
  if (MachineInstr *MI = findInstr(ML, TPCInstrInfo::isMMIOAccess)) {
    LLVM_DEBUG(dbgs() << "Dependency analysis does not support MMIO: " << MI);
    return false;
  }

  return true;
}

namespace {
struct MLRegistryDAResult {
  Optional<bool> HasLoopDependentReg;
  SmallPtrSet<MachineInstr *, 8> PhisDefineLoopDependentReg;
};
} // namespace

static bool isMIReallyUsesReg(const MachineInstr &MI, const Register Reg,
                              const MachineRegisterInfo &MRI) {
  return any_of(MI.uses(), [&](const MachineOperand &MO) {
    return MO.isReg() && !MO.isDef() && MO.getReg() == Reg &&
           !isPlaceholderReg(MO, MRI);
  });
}

/// Search for loop-dependent dependencies in registries.
static Optional<MLRegistryDAResult>
analyzeRegDep(const MachineLoop &ML, const MachineRegisterInfo &MRI) {
  MachineBasicBlock *const Latch = ML.getLoopLatch();
  assert(Latch);

  LLVM_DEBUG(dbgs() << "Check against loop-dependent registry deps.\n");

  MLRegistryDAResult Result;

  // Analyze loop dependent dependencies by registries.
  for (MachineInstr &Phi : ML.getHeader()->phis()) {
    // Report dependency exists if there is a phi instruction which accepts
    // non-loop invariant value from Latch block and its result is used in the
    // loop. Use simplified check against loop invariant: either immediate value
    // either registry is defined outside loop.
    const MachineOperand &LatchOp = *getPhiValueForMBB(Phi, *Latch);

    // Immediate -> loop invariant -> no dependency.
    if (LatchOp.isImm() || LatchOp.isFPImm() || LatchOp.isCImm()) {
      LLVM_DEBUG(dbgs() << "Phi latch op is imm. Accept: " << Phi);
      continue;
    }

    // Reject analysis if suspicious phi operand occurs.
    if (!LatchOp.isReg() || LatchOp.getReg().isPhysical()) {
      LLVM_DEBUG(dbgs() << "Phi input is suspicious. Reject: " << Phi);
      return None;
    }

    // Virtual registry outside loop -> loop invariant -> no dependency.
    if (!ML.contains(MRI.getVRegDef(LatchOp.getReg()))) {
      LLVM_DEBUG(dbgs() << "Phi latch op is outside loop. Accept: " << Phi);
      continue;
    }

    // Check if phi definition is used inside loop.
    const Register PhiDefReg = Phi.getOperand(0).getReg();
    MachineInstr *InLoopUseMI = nullptr;
    for (MachineInstr &UseMI : MRI.use_instructions(PhiDefReg)) {
      if (ML.contains(&UseMI) && isMIReallyUsesReg(UseMI, PhiDefReg, MRI)) {
        InLoopUseMI = &UseMI;
        break;
      }
    }
    if (InLoopUseMI) {
      LLVM_DEBUG(
          dbgs()
          << "Phi def is used in loop. Dependency in registers is found:\n"
          << "  Phi: " << Phi << "  Use: " << *InLoopUseMI);
      Result.PhisDefineLoopDependentReg.insert(&Phi);
    }
  }

  Result.HasLoopDependentReg = !Result.PhisDefineLoopDependentReg.empty();
  return Result;
}

/// Collect memory operations inside machine loop.
static Optional<MemUsage> collectMemUsage(const MachineLoop &ML,
                                          const TPCSubtarget &ST) {
  LLVM_DEBUG(dbgs() << "Collect machine loop memory usage instructions.\n");

  MemUsage Result;

  // Shortcut to access address space data by its kind
  const auto AccessASU =
      [&](const AddressSpaceAccessKind Kind) -> MemUsage::AddressSpaceUsage * {
    switch (Kind) {
    // clang-format off
    case AddressSpaceAccessKind::SLM: return &Result.SLMUsage;
    case AddressSpaceAccessKind::VLM: return &Result.VLMUsage;
    case AddressSpaceAccessKind::GMS: return &Result.GMSUsage;
    case AddressSpaceAccessKind::GMT: return &Result.GMTUsage;
      // clang-format on
    }
    llvm_unreachable("unexpected Kind value");
  };

  for (MachineBasicBlock *MBB : ML.getBlocks()) {
    for (MachineInstr &MI : MBB->instrs()) {
      const MCInstrDesc &Desc = MI.getDesc();
      const bool IsLd = TPCII::isLoadInst(Desc);
      const bool IsSt = TPCII::isStoreInst(Desc);

      if (!IsLd && !IsSt)
        continue;

      // Skip known load/store which does not perform actual load and store.
      const unsigned SlotOpCode = TPCII::getSlotOpCode(Desc);
      const bool IsKnownNoActionInstruction =
          IsLd ? (SlotOpCode == TPCII::ldGEN_ADDR ||  //
                  SlotOpCode == TPCII::ldPRMT_INDX || //
                  SlotOpCode == TPCII::ldSET_INDX ||  //
                  SlotOpCode == TPCII::ldMOV ||       //
                  SlotOpCode == TPCII::LOOKUP ||      //
                  SlotOpCode == TPCII::LOOKUP_2C ||   //
                  SlotOpCode == TPCII::LOOKUP_1C ||   //
                  SlotOpCode == TPCII::PREFETCH ||    //
                  SlotOpCode == TPCII::LD_EVENT ||    //
                  SlotOpCode == TPCII::ldNOP)
               : (SlotOpCode == TPCII::stGEN_ADDR ||  //
                  SlotOpCode == TPCII::stPRMT_INDX || //
                  SlotOpCode == TPCII::stSET_INDX ||  //
                  SlotOpCode == TPCII::ST_EVENT ||    //
                  SlotOpCode == TPCII::stNOP);
      if (IsKnownNoActionInstruction)
        continue;

      const Optional<AddressSpaceAccessKind> Kind =
          getAddressSpaceAccessKind(MI, ST);

      if (!Kind.hasValue()) {
        LLVM_DEBUG(dbgs() << "Reject analysis. Unknown mem instr: " << MI);
        assert(false); // Catch unknown instruction during testing and file
                       // improvement JIRA issue.
        return None;
      }
      LLVM_DEBUG(dbgs() << (IsLd ? "Load from " : "Store to  ")
                        << getAddressSpaceAccessKindName(Kind.getValue())
                        << " instr: " << MI);

      if (IsLd) {
        const MachineOperand SrcOp = MI.getOperand(1);
        if (!SrcOp.isReg()) {
          LLVM_DEBUG(
              dbgs()
              << "Mem operation by immediate address. Dependency analysis does "
                 "not support this case. Reject analysis.\n"
              << "  Instr: " << MI);
          return None;
        }

        AccessASU(Kind.getValue())->RegSrcToLD[SrcOp.getReg()].push_back(&MI);
      } else {
        const MachineOperand DstOp = MI.getOperand(0);
        if (!DstOp.isReg()) {
          LLVM_DEBUG(
              dbgs()
              << "Mem operation by immediate address. Dependency analysis does "
                 "not support this case. Reject analysis.\n"
              << "  Instr: " << MI);
          return None;
        }

        AccessASU(Kind.getValue())->RegDstToST[DstOp.getReg()].push_back(&MI);
      }
    }
  }

  return std::move(Result);
}

namespace {
/// Represents expression for memory location construction.
///
/// Currently supports the following pattern:
///
///     MemAccessReg = Origin + Step * LoopCounter + Shift
///
/// Example:
///
///     a[2*i + 3],
///
/// Where:
///     Origin       = a
///     Step         = 2
///     Shift        = 3
///     LoopCounter  = i
///     MemAccessReg = a[2*i + 3]
struct MemAccessExpr {
  Register MemAccessReg;
  Register Origin;
  std::int64_t Shift;
  std::int64_t Step;
};

struct OriginAndShift {
  Register Origin;
  std::int64_t Shift = 0;
};
} // namespace

static Optional<OriginAndShift> squashShift(const MachineOperand Op,
                                            const MachineRegisterInfo &MRI,
                                            const TPCInstrInfo &TII) {
  if (Op.isImm()) {
    OriginAndShift OAS;
    OAS.Shift = Op.getImm();
    return OAS;
  }

  if (!Op.isReg())
    return None;

  std::int64_t Shift = 0;
  Register Reg = Op.getReg();
  do {
    if (!Reg.isVirtual()) {
      OriginAndShift OAS;
      OAS.Origin = Reg;
      OAS.Shift = Shift;
      return OAS;
    }

    const MachineInstr &MI = *MRI.getVRegDef(Reg);

    // Pass through COPY propagation.
    if (MI.isCopy() && MI.getOperand(1).isReg() &&
        MI.getOperand(1).getReg().isVirtual()) {
      Reg = MI.getOperand(1).getReg();
      continue;
    }

    // Something + Constant | Something - Constant.
    if (
        // Scalar reg + imm | reg - imm
        (MI.getOpcode() == TPC::ADDsip || MI.getOpcode() == TPC::SUBsip) &&
        // Can not analyze predicated!
        !TII.isPredicated(MI) &&
        // Operands are integers (seems like memory add)
        MI.getOperand(3).isImm() &&
        (MI.getOperand(3).getImm() == TPCII::OpType::INT32 ||
         MI.getOperand(3).getImm() == TPCII::OpType::UINT32) &&
        // Regular add without fancy switches
        MI.getOperand(4).isImm() && MI.getOperand(4).getImm() == 0) {
      Reg = MI.getOperand(1).getReg();
      Shift += MI.getOpcode() == TPC::ADDsip ? MI.getOperand(2).getImm()
                                             : -MI.getOperand(2).getImm();
      continue;
    }

    OriginAndShift OAS;
    OAS.Origin = Reg;
    OAS.Shift = Shift;
    return OAS;
  } while (true);
}

/// Construct memory access expression for memory location given in MemAccessReg
/// register.
static Optional<MemAccessExpr>
analyzeMemAccessExpr(const Register MemAccessReg,
                     const ArrayRef<MLoopLinearIV> &IVs,
                     const MachineRegisterInfo &MRI, const TPCInstrInfo &TII) {
  // Only mem access for IV is supported for now as the most popular case. It
  // might be improved in the future if required.
  //
  // Parse the following memory access pattern:
  //
  //    a[C1 * i + C0], where a - pointer, C1, C2 - integral constants
  //
  // It should be represented as:
  //
  //    a + C0 + C1 * i
  //
  // Where:
  //
  //    Origin = a
  //    Shift = C0
  //    Step = C1

  const Optional<OriginAndShift> ShiftToIV =
      squashShift(MachineOperand::CreateReg(MemAccessReg, false), MRI, TII);

  if (!ShiftToIV.hasValue() || !ShiftToIV->Origin.isValid()) {
    LLVM_DEBUG(dbgs() << "Failed to squash shifts for "
                      << printReg(MemAccessReg) << "\n");
    return None;
  }

  const Register IVReg = ShiftToIV->Origin;

  for (const MLoopLinearIV &IV : IVs) {
    // Skip non-immediate steps, their analysis is not supported.
    if (!IV.getStepOp().isImm())
      continue;
    const std::int64_t Step = IV.getStepOp().getImm();

    // Check if this IV defines the MemAccessReg
    Optional<std::int64_t> AdditionalShift;
    if (IVReg == IV.getCurrDefReg()) {
      AdditionalShift = 0;
    } else if (IVReg == IV.getNextDefReg()) {
      AdditionalShift = Step;
    }
    if (!AdditionalShift)
      continue;

    // Try parse mem access expression.
    MachineOperand *const StartOp = IV.getStartOp();
    if (!StartOp) {
      LLVM_DEBUG(dbgs() << "Can not find IV start: " << printReg(IVReg)
                        << "\n");
      return None;
    }

    const Optional<OriginAndShift> OAS = squashShift(*StartOp, MRI, TII);
    if (!OAS) {
      LLVM_DEBUG(dbgs() << "Can not locate origin: " << printReg(IVReg)
                        << "\n");
      return None;
    }
    MemAccessExpr E;
    E.MemAccessReg = MemAccessReg;
    E.Origin = OAS->Origin;
    E.Shift = OAS->Shift + AdditionalShift.getValue() + ShiftToIV->Shift;
    E.Step = Step;
    return E;
  }

  // If no IV fits for mem access expression try to make it like Origin + Shift.
  // It covers cases like:
  //
  //    arr[3]
  //
  MemAccessExpr E;
  E.MemAccessReg = MemAccessReg;
  E.Origin = ShiftToIV->Origin;
  E.Shift = ShiftToIV->Shift;
  E.Step = 0;
  return E;
}

/// Construct memory access expressions for each requested memory location in
/// the given address space.
static Optional<DenseMap<Register, MemAccessExpr>>
collectMemAccessExpr(const MemUsage::AddressSpaceUsage &ASU,
                     const ArrayRef<MLoopLinearIV> &IVs,
                     const MachineRegisterInfo &MRI, const TPCInstrInfo &TII) {
  LLVM_DEBUG(dbgs() << "Collect memory access expr for address space.\n");

  DenseMap<Register, MemAccessExpr> RV;

  const auto CollectForMapping =
      [&](const MemUsage::AddressSpaceUsage::RegToMInstrs &Mapping) {
        for (const auto &RegAndMIs : Mapping) {
          const Register MemAccessReg = RegAndMIs.first;

          if (RV.count(MemAccessReg))
            continue;

          const Optional<MemAccessExpr> Expr =
              analyzeMemAccessExpr(MemAccessReg, IVs, MRI, TII);
          if (!Expr)
            return false;

          LLVM_DEBUG(dbgs() << "Memory access expression ready:\n"
                            << "  MemLoc: " << printReg(MemAccessReg) << "\n"
                            << "  Origin: " << printReg(Expr->Origin) << "\n"
                            << "  Shift:  " << Expr->Shift << "\n"
                            << "  Step:   " << Expr->Step << "\n");

          RV[MemAccessReg] = Expr.getValue();
        }
        return true;
      };

  if (!CollectForMapping(ASU.RegDstToST))
    return None;

  if (!CollectForMapping(ASU.RegSrcToLD))
    return None;

  return std::move(RV);
}

/// Whether memory access expressions have common origin.
static bool
hasCommonOrigin(const DenseMap<Register, MemAccessExpr> &MemLocRegToExpr) {
  Optional<Register> CommonOrigin;

  for (const auto &RegAndExpr : MemLocRegToExpr) {
    const Register Origin = RegAndExpr.second.Origin;

    if (!CommonOrigin.hasValue()) {
      CommonOrigin = Origin;
    } else if (CommonOrigin != Origin) {
      LLVM_DEBUG(dbgs() << "Different memory expr origins found:\n"
                        << "  1: " << printReg(CommonOrigin.getValue()) << "\n"
                        << "  2: " << printReg(Origin) << "\n");
      return false;
    }
  }

  return true;
}

/// Check for prohibited registries.
///
/// \details Virtual registers might be analyzed but physical registers are
///     questionable. It is possible to allow hardware loop counter registers
///     here because only the innermost loops are supported. It means that
///     hardware loop counter register value stays unchanged during the loop
///     iteration.
static bool isRegDisallowDependencyAnalysis(const Register R,
                                            const TPCSubtarget &ST) {
  return R.isPhysical() && !ST.isHWLoopReg(R);
}

/// Whether memory operations for the given address space operates on registers
/// which are hard or impossible to analyze for memory dependencies.
static bool
hasRegDisallowDependencyAnalysis(const MemUsage::AddressSpaceUsage &ASU,
                                 const TPCSubtarget &ST) {
  const auto CheckMapping =
      [&ST](const MemUsage::AddressSpaceUsage::RegToMInstrs &Mapping) {
        for (const auto &RegAndMIs : Mapping) {
          if (isRegDisallowDependencyAnalysis(RegAndMIs.first, ST)) {
            LLVM_DEBUG(dbgs() << "Registry disallows Memory DA: "
                              << printReg(RegAndMIs.first) << "\n");
            return true;
          }
        }
        return false;
      };
  return CheckMapping(ASU.RegDstToST) || CheckMapping(ASU.RegSrcToLD);
}

namespace {
struct MemLocDepResult {
  bool HasLoopDependentDep = false;
  bool HasLoopIndependentDep = false;
};
} // namespace

static MemLocDepResult makeLoopDependentMemLocDep() {
  MemLocDepResult RV;
  RV.HasLoopDependentDep = true;
  return RV;
}

static MemLocDepResult makeLoopIndependentMemLocDep() {
  MemLocDepResult RV;
  RV.HasLoopIndependentDep = true;
  return RV;
}

static MemLocDepResult makeNoDependenciesMemLocDep() {
  return MemLocDepResult();
}

static MemLocDepResult makeBothDependenciesMemLocDep() {
  MemLocDepResult RV;
  RV.HasLoopDependentDep = true;
  RV.HasLoopIndependentDep = true;
  return RV;
}

/// Check if two memory locations may alias inside machine loop.
static Optional<MemLocDepResult>
checkMemAccessDependency(const Register MemLocReg1, const Register MemLocReg2,
                         const DenseMap<Register, MemAccessExpr> &MemLocToExpr,
                         const unsigned InterestingIterationsCount) {
  if (MemLocReg1 == MemLocReg2)
    return makeLoopIndependentMemLocDep();

  auto It1 = MemLocToExpr.find(MemLocReg1);
  if (It1 == MemLocToExpr.end()) {
    LLVM_DEBUG(dbgs() << "No expr for " << printReg(MemLocReg1) << "\n");
    return None;
  }

  auto It2 = MemLocToExpr.find(MemLocReg2);
  if (It2 == MemLocToExpr.end()) {
    LLVM_DEBUG(dbgs() << "No expr for " << printReg(MemLocReg2) << "\n");
    return None;
  }

  const MemAccessExpr &E1 = It1->second;
  const MemAccessExpr &E2 = It2->second;

  if (E1.Origin != E2.Origin) {
    LLVM_DEBUG(dbgs() << "Can not analyze 2 mem expr with different origins\n"
                      << "  Expr 1 reg: " << printReg(MemLocReg1) << "\n"
                      << "  Expr 2 reg: " << printReg(MemLocReg2) << "\n"
                      << "  Origin 1: " << printReg(E1.Origin) << "\n"
                      << "  Origin 2: " << printReg(E2.Origin) << "\n");
    return None;
  }

  // Analyze the following linear diophantine equation of (X, Y) for possible
  // solutions:
  //
  //    E1.Step * X + E1.Shift = E2.Step * Y + E2.Shift
  //
  // Let's rewrite the equation for cleaner code:
  //
  //    A * X + B * Y = C
  //
  const std::int64_t A = E1.Step;
  const std::int64_t B = -E2.Step;
  const std::int64_t C = E2.Shift - E1.Shift;

  LLVM_DEBUG(dbgs() << "Analyze mem dep for " << printReg(MemLocReg1) << " and "
                    << printReg(MemLocReg2) << "\n"
                    << "  Diophantine equation: " << A << " * X + " << B
                    << " * Y = " << C << "\n");

  // ZIV-test.
  if (A == 0 && B == 0) {
    LLVM_DEBUG(dbgs() << "ZIV-test applied\n");
    return C == 0 ? makeLoopIndependentMemLocDep()
                  : makeNoDependenciesMemLocDep();
  }

  // Weak SIV-test by X.
  //
  //    A * X = C
  //
  if (B == 0) {
    LLVM_DEBUG(dbgs() << "Weak SIV-test B == 0 applied\n");
    return C % std::abs(A) == 0 ? makeLoopDependentMemLocDep()
                                : makeNoDependenciesMemLocDep();
  }

  // Weak SIV-test by Y.
  //
  //    B * Y = C
  //
  if (A == 0) {
    LLVM_DEBUG(dbgs() << "Weak SIV-test A == 0 applied\n");
    return C % std::abs(B) == 0 ? makeLoopDependentMemLocDep()
                                : makeNoDependenciesMemLocDep();
  }

  // Weak SIV-test.
  //
  //    A * X + A * Y = C
  //
  //  I.e.:
  //
  //    X = C / A - Y
  //
  //  Example:
  //
  //    a[i] = ... = a[N - i]
  //
  if (A == B) {
    LLVM_DEBUG(dbgs() << "Weak SIV-test A == B applied\n");

    // Both dependencies because X == -Y and theoretically there is a case
    // X == 0 and Y == 0.
    if (C == 0)
      return makeBothDependenciesMemLocDep();

    if (C % std::abs(A) == 0)
      return C % std::abs(2 * A) == 0 ? makeBothDependenciesMemLocDep()
                                      : makeLoopDependentMemLocDep();
    return makeNoDependenciesMemLocDep();
  }

  // Strong SIV-test.
  // (Optimized for innermost loops analysis with known interesting iterations
  // count).
  //
  //    A * X - A * Y = C
  //
  //  I.e.:
  //
  //    X = Y + C / A
  //
  // Example:
  //
  //    a[i] = ... = a[i + 2]
  //
  if (A == -B) {
    LLVM_DEBUG(dbgs() << "Strong SIV-test A == -B applied\n");

    if (C == 0)
      return makeLoopIndependentMemLocDep();

    if (C % std::abs(A) != 0)
      return makeNoDependenciesMemLocDep();

    // NOTE: Here we use knowledge that the only innermost loops are analyzed.
    //       It means that X and Y are IVs of the innermost loops, and compiler
    //       may check for interesting iterations count for ranges:
    //
    //    Fixed X: Y in [X, X + InterestingIterationsCount - 1]
    //    Fixed Y: X in [Y, Y + InterestingIterationsCount - 1]
    const std::int64_t Distance = C / A;
    if (InterestingIterationsCount &&
        std::abs(Distance) >= InterestingIterationsCount) {
      LLVM_DEBUG(dbgs() << "  Dependency distance: " << Distance << "\n"
                        << "  Interesting iterations: "
                        << InterestingIterationsCount << "\n"
                        << "  Return <no dependency> result.\n");
      return makeNoDependenciesMemLocDep();
    }

    return makeLoopDependentMemLocDep();
  }

  // Before we fall into the weakest MIV-test try to check
  // InterestingIterationsCount before and after each induction variable.
  //
  // NOTE: Here we use knowledge that the only innermost loops are analyzed.
  //       It means that X and Y are IVs of the innermost loops.
  if (InterestingIterationsCount) {
    // Equation:
    //
    //    A * X + B * Y = C

    LLVM_DEBUG(
        dbgs()
        << "MIV-test with known InterestingIterationsCount is applied\n");

    MemLocDepResult Result;

    // Investigate Ys in: [X, X + InterestingIterationsCount - 1]
    //
    //    A * X + B * (X + Delta) = C
    //
    // I.e.:
    //
    //    (A + B) * X = C - B * Delta
    //
    for (std::int64_t Delta = 0; Delta < InterestingIterationsCount; ++Delta) {
      if ((C - B * Delta) % std::abs(A + B) == 0) {
        LLVM_DEBUG(dbgs() << "  Found dep with Y = X + " << Delta << "\n");
        (Delta == 0 ? Result.HasLoopIndependentDep
                    : Result.HasLoopDependentDep) = true;
      }
    }

    // Investigate Xs in: [Y, Y + InterestingIterationsCount - 1]
    //
    //    A * (Y + Delta) + B * Y = C
    //
    // I.e.:
    //
    //    (A + B) * Y = C - A * Delta
    //
    for (std::int64_t Delta = 0; Delta < InterestingIterationsCount; ++Delta) {
      if ((C - A * Delta) % std::abs(A + B) == 0) {
        LLVM_DEBUG(dbgs() << "  Found dep with X = Y + " << Delta << "\n");
        (Delta == 0 ? Result.HasLoopIndependentDep
                    : Result.HasLoopDependentDep) = true;
      }
    }

    return Result;
  }

  // MIV-test.
  //
  // Fall into GCD test. Immediate boundaries for Banerjee test or more precise
  // tests might be used but are not implemented for simplicity.
  LLVM_DEBUG(dbgs() << "MIV-test is applied\n");
  MemLocDepResult Result;
  Result.HasLoopIndependentDep = C % std::abs(A + B) == 0;
  Result.HasLoopDependentDep =
      C % GreatestCommonDivisor64(std::abs(A), std::abs(B)) == 0;
  return Result;
}

/// Return set of tensor IDs which might be a value for TensorIDMO. Returns
/// empty set on analysis failure.
static SmallSet<int, 4> parseTensorIDFromMO(const MachineOperand &TensorIDMO,
                                            const MachineRegisterInfo &MRI) {
  LLVM_DEBUG(dbgs() << "  Parsing MO for tensor ID: " << TensorIDMO << "\n");

  SmallSet<int, 4> CandidateIDs;

  // Fast return for the most popular case: immediate tensor ID.
  if (TensorIDMO.isImm()) {
    LLVM_DEBUG(dbgs() << "    It is immediate!\n");
    CandidateIDs.insert(TensorIDMO.getImm());
    return CandidateIDs;
  }

  // Case appeared in kernels:
  //    batch_norm_stage2_dynamic_fwd_f32
  //    batch_norm_stage2_fwd_f32
  //
  // Try to detect chain of instructions:
  //   %id_1 = MOVsip id_1, ..., undef, $sp0, 0
  //   %id_2 = MOVsip id_2, ..., %id_1, %predicate, %polarity
  //   ...
  //   %tmp  = COPY %id_2
  //   ST/LD_TNSR %irf, %tmp, ...
  //
  // NOTE: It might be useful to generalize the pattern, but leave it as is for
  //    simplicity for now. Possible improvements:
  //      * consider any chain of COPY or MOVssp instructions, including empty
  //      * consider predicated MOVssp also, traverse all options
  //      * consider any chain of MOVsip id_n ... instructions
  //      * MOVsip might be a more complex expression. It seems like a SCEV.
  if (TensorIDMO.isReg()) {
    const auto IsMOVsipForTensorID = [](const MachineInstr &MI) {
      return MI.getOpcode() == TPC::MOVsip && MI.getOperand(1).isImm() &&
             MI.getOperand(2).isImm() &&
             MI.getOperand(2).getImm() == TPCII::INT8;
    };

    const auto IsMOVsipPredicated = [](const MachineInstr &MI) {
      assert(MI.getOpcode() == TPC::MOVsip);
      return MI.getNumOperands() == 7 &&                              //
             MI.getOperand(4).isReg() && MI.getOperand(4).isTied() && //
             MI.getOperand(5).isReg() && MI.getOperand(5).getReg() != TPC::SP0;
    };

    const Register Reg = TensorIDMO.getReg();

    // Locate %tmp  = COPY %id_2.
    const MachineInstr *RegDefMI = nullptr;
    if (Reg.isVirtual()) {
      RegDefMI = MRI.getVRegDef(Reg);
    } else {
      const MachineInstr *const TensorAccessMI = TensorIDMO.getParent();
      if (TensorAccessMI->getIterator() !=
              TensorAccessMI->getParent()->begin() &&
          (--TensorAccessMI->getIterator())->getOpcode() == TPC::COPY &&
          (--TensorAccessMI->getIterator())->getOperand(0).getReg() == Reg)
        RegDefMI = &*(--TensorAccessMI->getIterator());
    }

    LLVM_DEBUG({
      if (RegDefMI)
        dbgs() << "    Reg def found: " << *RegDefMI;
    });

    // Locate %id_2 = MOVsip id_2, ..., %predicate, %polarity
    Register ID2Reg;
    if (RegDefMI &&                           //
        RegDefMI->getOpcode() == TPC::COPY && //
        RegDefMI->getOperand(1).isReg() &&    //
        RegDefMI->getOperand(1).getReg().isVirtual()) {
      ID2Reg = RegDefMI->getOperand(1).getReg();
    }

    LLVM_DEBUG({
      if (ID2Reg.isValid())
        dbgs() << "    Reg2 found: " << printReg(ID2Reg) << "\n";
    });

    // Parse %id_2 = MOVsip id_2, ..., %predicate, %polarity
    int ID2 = -1;
    Register ID1Reg;
    if (ID2Reg.isValid()) {
      const MachineInstr *const ID2Def = MRI.getVRegDef(ID2Reg);
      LLVM_DEBUG(dbgs() << "    ID2Def: " << *ID2Def);

      if (IsMOVsipForTensorID(*ID2Def)) {
        ID2 = ID2Def->getOperand(1).getImm();

        if (!IsMOVsipPredicated(*ID2Def)) {
          LLVM_DEBUG(dbgs() << "    Returning: " << ID2 << "\n");
          CandidateIDs.insert(ID2);
          return CandidateIDs;
        }

        if (ID2Def->getOperand(4).getReg().isVirtual())
          ID1Reg = ID2Def->getOperand(4).getReg();
      }
    }

    LLVM_DEBUG({
      if (ID1Reg.isValid() && ID1Reg.isVirtual())
        dbgs() << "    Reg1 found: " << printReg(ID1Reg) << "\n";
    });

    // Parse %id_1 = MOVsip id_1, ..., sp0, 0
    if (ID1Reg.isValid() && ID1Reg.isVirtual()) {
      const MachineInstr *const ID1Def = MRI.getVRegDef(ID1Reg);

      if (IsMOVsipForTensorID(*ID1Def) && !IsMOVsipPredicated(*ID1Def)) {
        assert(ID2 >= 0);
        const int ID1 = ID1Def->getOperand(1).getImm();
        LLVM_DEBUG(dbgs() << "    Returning: " << ID1 << " " << ID2 << "\n");
        CandidateIDs.insert(ID1);
        CandidateIDs.insert(ID2);
        return CandidateIDs;
      }
    }
  }

  assert(CandidateIDs.empty());
  LLVM_DEBUG(dbgs() << "    Failed to parse tensor ID " << TensorIDMO
                    << " in: " << *TensorIDMO.getParent());
  return CandidateIDs;
}

/// Having access to tensor memory via LD_G through register Reg try to find
/// tensor ID. Return empty set on parsing failure.
static SmallSet<int, 4>
getTensorIDForAddressReg(const Register Reg, const MachineRegisterInfo &MRI) {
  if (!Reg.isVirtual()) {
    LLVM_DEBUG(dbgs() << "Register is physical: " << printReg(Reg) << "\n");
    return {};
  }

  const MachineInstr *const SrcDef = MRI.getVRegDef(Reg);

  if (SrcDef->getOpcode() != TPC::GEN_ADDR_ld &&
      SrcDef->getOpcode() != TPC::GEN_ADDR_st &&
      SrcDef->getOpcode() != TPC::GEN_ADDR_ldT &&
      SrcDef->getOpcode() != TPC::GEN_ADDR_stT) {
    LLVM_DEBUG(dbgs() << "Unsupported address instruction: " << *SrcDef);
    return {};
  }

  return parseTensorIDFromMO(SrcDef->getOperand(1), MRI);
}

/// Check against load-store memory dependencies in access to tensor memory.
static Optional<bool>
isMemDependencyInTensorMem(const MemUsage &MU, const MachineRegisterInfo &MRI,
                           const bool IgnoreCheckAgainstStoresToSameTensorId) {
  // Check tensor ids used for load and store operations, ensure there is no
  // intersection.
  SmallSet<int, 8> LdTensorIDs;
  SmallSet<int, 8> StTensorIDs;
  for (const auto &RegAndInstructions : MU.GMSUsage.RegSrcToLD) {
    const Register RegSrc = RegAndInstructions.first;

    const SmallSet<int, 4> IDs = getTensorIDForAddressReg(RegSrc, MRI);
    if (IDs.empty())
      return None;

    LdTensorIDs.insert(IDs.begin(), IDs.end());
  }
  for (const auto &RegAndInstructions : MU.GMSUsage.RegDstToST) {
    const Register RegDst = RegAndInstructions.first;

    const SmallSet<int, 4> IDs = getTensorIDForAddressReg(RegDst, MRI);
    if (IDs.empty())
      return None;

    for (const int ID : IDs) {
      if (!IgnoreCheckAgainstStoresToSameTensorId && StTensorIDs.count(ID)) {
        LLVM_DEBUG(dbgs() << ">=2 stores to tensor: " << ID << "\n");
        return None;
      }
      StTensorIDs.insert(ID);
    }
  }
  for (const auto &RegAndInstructions : MU.GMTUsage.RegSrcToLD) {
    for (const MachineInstr *const MI : RegAndInstructions.second) {
      const SmallSet<int, 4> IDs = parseTensorIDFromMO(MI->getOperand(2), MRI);
      if (IDs.empty())
        return None;

      LdTensorIDs.insert(IDs.begin(), IDs.end());
    }
  }
  for (const auto &RegAndInstructions : MU.GMTUsage.RegDstToST) {
    for (const MachineInstr *const MI : RegAndInstructions.second) {
      const SmallSet<int, 4> IDs = parseTensorIDFromMO(MI->getOperand(1), MRI);
      if (IDs.empty())
        return None;

      for (const int ID : IDs) {
        if (!IgnoreCheckAgainstStoresToSameTensorId && StTensorIDs.count(ID)) {
          LLVM_DEBUG(dbgs() << ">=2 writes to tensor: " << ID << "\n");
          return None;
        }
        StTensorIDs.insert(ID);
      }
    }
  }
  LLVM_DEBUG({
    dbgs() << "Detected tensor ID loads:\n";
    for (const int TensorID : LdTensorIDs)
      dbgs() << "  " << TensorID << "\n";

    dbgs() << "Detected tensor ID stores:\n";
    for (const int TensorID : StTensorIDs)
      dbgs() << "  " << TensorID << "\n";
  });

  // Whether intersection is empty?
  for (const int TensorID : LdTensorIDs) {
    if (StTensorIDs.count(TensorID)) {
      LLVM_DEBUG(dbgs() << "Detected mem dep for tensor ID: " << TensorID
                        << "\n");
      return true;
    }
  }

  // No dependency!
  return false;
}

namespace {
struct MLMemoryDAResult {
  Optional<bool> HasLoopDependentMemStLd;
  Optional<bool> HasLoopDependentMemStSt;
  Optional<bool> HasLoopIndependentMemStLd;
  Optional<bool> HasLoopIndependentMemStSt;
};
} // namespace

static MLMemoryDAResult
analyzeMemDep(const MachineLoop &ML, const ArrayRef<MLoopLinearIV> IVs,
              const MachineRegisterInfo &MRI, const TPCSubtarget &ST,
              const TPCInstrInfo &TII,
              const bool IgnoreCheckAgainstStoresToSameTensorId,
              const unsigned InterestingIterationsCount) {
  MLMemoryDAResult Result;

  // Analyze loop dependent dependencies by memory access.
  //
  // Use slightly improved and adapted dependency analysis theory from:
  //
  //    Goff. Kennedy.
  //    Practical Dependence Testing.
  //    Ch 3, 4.
  //
  //    1. Load / Store to tensor memory is allowed without restrictions.
  //
  //       Load / Store to the same tensor memory requires ASO synchronization
  //       instruction, but such loops are already filtered. So, Load and Store
  //       instructions to the same tensor are not allowed to appear here.
  //
  //       But Load / Store to tensor might alias with scalar Load / Store to
  //       global address space.
  //
  //    2. Load / Store to different address spaces are allowed without
  //       restrictions. Restrictions are applied within the single address
  //       space only.
  //
  //    3. For each address space:
  //
  //       Ignore multi indexing array access because it does not work for TPC
  //       now (see GAUDI-2060) and seems like is not required for analysis in
  //       the nearest future.
  //
  //       If only loads exist - there is no dependency.
  //
  //       If loads and stores exist - run ZIV, SIV, MIV ... tests. Note that if
  //       InterestingIterationsCount value is provided, compiler might perform
  //       more precise checks.
  //
  //       Mark dependency type (load-store, store-load or store-store). Types
  //       should be exposed to caller because some algorithms (hardware
  //       unrolling, for example) are tolerant to store-store loop-dependent
  //       dependencies.

  const Optional<MemUsage> MU = collectMemUsage(ML, ST);
  if (!MU.hasValue()) {
    LLVM_DEBUG(dbgs() << "Failed to collect mem usage stat.\n");
    return Result;
  }

  LLVM_DEBUG({
    const auto DumpUsage = [](const MemUsage::AddressSpaceUsage &ASU,
                              const char *ASUName) {
      dbgs() << "Memory usage for " << ASUName << ":\n";
      for (const auto &Item : ASU.RegSrcToLD)
        dbgs() << "  Load from: " << printReg(Item.first) << "\n";
      for (const auto &Item : ASU.RegDstToST)
        dbgs() << "  Store to : " << printReg(Item.first) << "\n";
    };
    DumpUsage(MU->SLMUsage, "scalar local memory (SLM)");
    DumpUsage(MU->VLMUsage, "vector local memory (VLM)");
    DumpUsage(MU->GMSUsage, "scalar global memory (GMS)");
    DumpUsage(MU->GMTUsage, "tensor global memory (GMT)");
  });

  // Reject analysis if there is a potential store-store, store-load, load-store
  // dependency to global memory between scalar and tensor operations.
  if (isMemDependencyInTensorMem(MU.getValue(), MRI,
                                 IgnoreCheckAgainstStoresToSameTensorId) !=
      false) {
    LLVM_DEBUG(dbgs() << "Possible alias in global address space\n");
    return Result;
  }

  bool RejectedStLdMemAnalysis = false;
  bool RejectedStStMemAnalysis = false;
  bool HasLoopDependentMemStLd = false;
  bool HasLoopDependentMemStSt = false;
  bool HasLoopIndependentMemStLd = false;
  bool HasLoopIndependentMemStSt = false;
  for (const MemUsage::AddressSpaceUsage *ASU :
       {&MU->SLMUsage, &MU->VLMUsage, &MU->GMSUsage}) {

    // Fast-return if no store memory access to this address space. No one is
    // interested in load-load dependencies.
    if (ASU->RegDstToST.empty())
      continue;

    if (hasRegDisallowDependencyAnalysis(*ASU, ST)) {
      LLVM_DEBUG(dbgs() << "Memory DA is rejected. Unsupported reg present.\n");
      return Result;
    }

    const Optional<DenseMap<Register, MemAccessExpr>> MemLocRegToExpr =
        collectMemAccessExpr(*ASU, IVs, MRI, TII);

    if (!MemLocRegToExpr.hasValue()) {
      LLVM_DEBUG(dbgs() << "Memory DA is rejected. There are unparsed memory "
                           "access patterns.\n");
      return Result;
    }

    // Ensure origin is common for all registries, otherwise dependency can not
    // be analyzed because there is no restrict/noalias guarantee.
    if (!hasCommonOrigin(MemLocRegToExpr.getValue())) {
      LLVM_DEBUG(dbgs() << "Memory DA is rejected. No noalias guarantee.\n");
      return Result;
    }

    const auto AnalyzeMemDepsBatch =
        [&MemLocRegToExpr, InterestingIterationsCount](
            const MemUsage::AddressSpaceUsage::RegToMInstrs &RegToMIs1,
            const MemUsage::AddressSpaceUsage::RegToMInstrs &RegToMIs2,
            bool &AnalysisRejectedFlag, bool &HasLoopDependentDep,
            bool &HasLoopIndependentDep) {
          for (const auto &RegAndMIs1 : RegToMIs1) {
            for (const auto &RegAndMIs2 : RegToMIs2) {
              // Stop processing if some analysis failed. Can not gain more
              // info.
              if (AnalysisRejectedFlag)
                break;

              // Skip check against the same instance.
              if (&RegAndMIs1 == &RegAndMIs2 &&
                  RegAndMIs1.first == RegAndMIs2.first)
                continue;

              const Optional<MemLocDepResult> Res = checkMemAccessDependency(
                  RegAndMIs1.first, RegAndMIs2.first,
                  MemLocRegToExpr.getValue(), InterestingIterationsCount);

              if (!Res.hasValue())
                AnalysisRejectedFlag = true;

              if (Res.hasValue() && Res->HasLoopDependentDep)
                HasLoopDependentDep = true;

              if (Res.hasValue() && Res->HasLoopIndependentDep)
                HasLoopIndependentDep = true;
            }
          }
        };

    // Check for store-load dependencies.
    AnalyzeMemDepsBatch(ASU->RegDstToST, ASU->RegSrcToLD,
                        RejectedStLdMemAnalysis, HasLoopDependentMemStLd,
                        HasLoopIndependentMemStLd);

    // Check for store-store dependencies.
    //
    // NOTE: Such call always reports loop independent dependencies because the
    //       same container is passed to LHS and RHS, but loop independent
    //       dependencies are not interesting for the analysis.
    AnalyzeMemDepsBatch(ASU->RegDstToST, ASU->RegDstToST,
                        RejectedStStMemAnalysis, HasLoopDependentMemStSt,
                        HasLoopIndependentMemStSt);
  }

  if (!RejectedStLdMemAnalysis) {
    Result.HasLoopDependentMemStLd = HasLoopDependentMemStLd;
    Result.HasLoopIndependentMemStLd = HasLoopIndependentMemStLd;
  }
  if (!RejectedStStMemAnalysis) {
    Result.HasLoopDependentMemStSt = HasLoopDependentMemStSt;
    Result.HasLoopIndependentMemStSt = HasLoopIndependentMemStSt;
  }
  return Result;
}

namespace llvm {

MLDAReport runMachineLoopDataDependencyAnalysis(
    const MachineLoop &ML, const MachineRegisterInfo &MRI,
    const TPCSubtarget &ST, const TPCInstrInfo &TII,
    const MLLinearIVsContainer &LinearIVs,
    const bool IgnoreCheckAgainstStoresToSameTensorId,
    const unsigned InterestingIterationsCount) {
  LLVM_DEBUG({
    dbgs() << "Machine function:\n";
    ML.getHeader()->getParent()->dump();
    dbgs() << "Run Dependency analysis for loop with header "
           << printMBBReference(*ML.getHeader()) << "\n";
  });

  MLDAReport Report;

  if (!isDependencyAnalysisSupportedForLoop(ML))
    return Report;

  if (Optional<MLRegistryDAResult> RegRes = analyzeRegDep(ML, MRI)) {
    Report.HasLoopDependentReg = RegRes->HasLoopDependentReg;
    Report.PhisDefineLoopDependentReg =
        std::move(RegRes->PhisDefineLoopDependentReg);
  }

  if (Optional<MLMemoryDAResult> MemRes = analyzeMemDep(
          ML, LinearIVs, MRI, ST, TII, IgnoreCheckAgainstStoresToSameTensorId,
          InterestingIterationsCount)) {
    Report.HasLoopDependentMemStLd = MemRes->HasLoopDependentMemStLd;
    Report.HasLoopDependentMemStSt = MemRes->HasLoopDependentMemStSt;
    Report.HasLoopIndependentMemStLd = MemRes->HasLoopIndependentMemStLd;
    Report.HasLoopIndependentMemStSt = MemRes->HasLoopIndependentMemStSt;
  }
  return Report;
}

} // namespace llvm
