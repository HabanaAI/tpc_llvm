//===- TPCOptimizeHWLoopsGuards.cpp - Drop jumps around hardware loops ----===//
//
// A rotated loop reaches TPCHardwareLoops behind a guard, a conditional jump
// over the loop for the zero-trip case:
//
//   bb.guard:   %p = CMP_GEQ %start, %end ; JMPR %bb.exit, %p
//   bb.pre:     ... ; LOOP %start, %end, step, <, %bb.exit
//
// The LOOP instruction compares start with the boundary before the first
// iteration and goes to %bb.exit when the loop would not run, so a guard that
// tests the same condition only costs the jump and the NOPs that wait for %p.
// TPCHardwareLoops drops it when it is the last instruction before the LOOP
// and compares the LOOP's own start and boundary registers. This pass, which
// runs right after it, drops the rest, as Habana's tpc-optimize-hw-loops-guards
// does:
//
//  * The guard may stand above blocks of preheader code (hoisted loop
//    invariants, setup for inner loops), which then run even when the loop
//    does not. That is allowed when the code has no effect besides its
//    results: no store or other side effect, no load other than from a tensor
//    or local memory, no PHI.
//  * The condition only has to be the LOOP's condition, not the same
//    registers: mov_irf_dim of an IRF dimension equals an IRF compare of that
//    dimension, and 'x < 1' equals 'x <= 0' and '0 < x'.
//  * A guard with another condition (an 'if' around the loop) becomes the
//    LOOP's predicate, when the code it skips has at most
//    -tpc-optimize-hw-loops-guards-max-predicate-block-size instructions.
//  * A predicated LOOP whose predicate is its own condition loses the
//    predicate.
//
// When the loop does not run, control now reaches %bb.exit from the LOOP's
// block instead of the guard's, so the exit PHIs take on that edge the values
// they had on the guard edge. TPCHardwareLoops left IMPLICIT_DEF there, since
// the edge was never taken before.
//
// Habana's pass (1.24.2) also removes a guard over a store, so that
// 'if (n > 0) { store; for (x = 0; x < n; ...) }' stores when n <= 0, as the
// simulator shows. This pass keeps such a guard.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCMachineInstrTools.h"
#include "TPCSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include <climits>
#include <optional>

using namespace llvm;

#define DEBUG_TYPE "tpc-optimize-hw-loops-guards"

STATISTIC(NumRemoved, "Guards removed because the LOOP makes the same check");
STATISTIC(NumPredicated, "Guards turned into a LOOP predicate");
STATISTIC(NumUnpredicated, "LOOP predicates removed as redundant");

static cl::opt<bool> EnableGuardOpt(
    "enable-tpc-optimize-hw-loops-guards", cl::init(true), cl::Hidden,
    cl::desc("Remove the zero-trip jumps around hardware loops"));

static cl::opt<bool> PredicateLoops(
    "tpc-optimize-hw-loops-guards-predicate-loops", cl::init(true), cl::Hidden,
    cl::desc("Turn other jumps around a hardware loop into its predicate"));

static cl::opt<unsigned> MaxPredicateBlockSize(
    "tpc-optimize-hw-loops-guards-max-predicate-block-size", cl::init(10),
    cl::Hidden,
    cl::desc("Most instructions a jump may skip before the LOOP for it to "
             "become the LOOP's predicate"));

namespace llvm {
FunctionPass *createTPCOptimizeHWLoopsGuards();
void initializeTPCOptimizeHWLoopsGuardsPass(PassRegistry &);
} // namespace llvm

namespace {

// LOOP operands: start, boundary, step, cmp, exit, [pred, polarity].
constexpr unsigned LoopStartOp = 0;
constexpr unsigned LoopBoundaryOp = 1;
constexpr unsigned LoopStepOp = 2;
constexpr unsigned LoopCmpOp = 3;
constexpr unsigned LoopExitOp = 4;
constexpr unsigned LoopPredOp = 5;
constexpr unsigned LoopPolarityOp = 6;

/// A compare operand: a register, one dimension of an IRF register, or an
/// immediate.
struct CmpValue {
  enum KindTy { None, Reg, IRFDim, Imm } Kind = None;
  Register R;
  unsigned Dim = 0;
  int64_t ImmVal = 0;

  bool operator==(const CmpValue &O) const {
    if (Kind != O.Kind || Kind == None)
      return false;
    switch (Kind) {
    case Reg: return R == O.R;
    case IRFDim: return R == O.R && Dim == O.Dim;
    case Imm: return ImmVal == O.ImmVal;
    case None: break;
    }
    return false;
  }
};

/// 'L Mode R', a signed 32-bit compare.
struct Cond {
  TPCII::CmpMode Mode = TPCII::LoopErr;
  CmpValue L, R;
};

TPCII::CmpMode invertMode(TPCII::CmpMode M) {
  switch (M) {
  case TPCII::LoopEQ: return TPCII::LoopNE;
  case TPCII::LoopNE: return TPCII::LoopEQ;
  case TPCII::LoopLT: return TPCII::LoopGE;
  case TPCII::LoopLE: return TPCII::LoopGT;
  case TPCII::LoopGT: return TPCII::LoopLE;
  case TPCII::LoopGE: return TPCII::LoopLT;
  default: return TPCII::LoopErr;
  }
}

TPCII::CmpMode swapMode(TPCII::CmpMode M) {
  switch (M) {
  case TPCII::LoopLT: return TPCII::LoopGT;
  case TPCII::LoopLE: return TPCII::LoopGE;
  case TPCII::LoopGT: return TPCII::LoopLT;
  case TPCII::LoopGE: return TPCII::LoopLE;
  default: return M;
  }
}

/// Bring a condition to one form per meaning: an immediate on the right,
/// compared with <=, >=, == or !=, and two registers compared with <, <=, ==
/// or !=. Fails for a compare of two immediates and for one that is constant
/// ('x < INT_MIN').
std::optional<Cond> normalize(Cond C) {
  if (C.Mode == TPCII::LoopErr || C.L.Kind == CmpValue::None ||
      C.R.Kind == CmpValue::None)
    return std::nullopt;
  if (C.L.Kind == CmpValue::Imm && C.R.Kind == CmpValue::Imm)
    return std::nullopt;
  if (C.L.Kind == CmpValue::Imm) {
    std::swap(C.L, C.R);
    C.Mode = swapMode(C.Mode);
  }
  if (C.R.Kind == CmpValue::Imm) {
    if (C.Mode == TPCII::LoopLT) {
      if (C.R.ImmVal == INT32_MIN)
        return std::nullopt;
      C.Mode = TPCII::LoopLE;
      --C.R.ImmVal;
    } else if (C.Mode == TPCII::LoopGT) {
      if (C.R.ImmVal == INT32_MAX)
        return std::nullopt;
      C.Mode = TPCII::LoopGE;
      ++C.R.ImmVal;
    }
  } else if (C.Mode == TPCII::LoopGT || C.Mode == TPCII::LoopGE) {
    std::swap(C.L, C.R);
    C.Mode = swapMode(C.Mode);
  }
  return C;
}

bool sameCond(const Cond &A, const Cond &B) {
  std::optional<Cond> NA = normalize(A), NB = normalize(B);
  if (!NA || !NB || NA->Mode != NB->Mode)
    return false;
  if (NA->L == NB->L && NA->R == NB->R)
    return true;
  bool Symmetric = NA->Mode == TPCII::LoopEQ || NA->Mode == TPCII::LoopNE;
  return Symmetric && NA->L == NB->R && NA->R == NB->L;
}

class TPCOptimizeHWLoopsGuards : public MachineFunctionPass {
public:
  static char ID;

  TPCOptimizeHWLoopsGuards() : MachineFunctionPass(ID) {
    initializeTPCOptimizeHWLoopsGuardsPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "TPC Optimize Hardware Loops Guards";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  MachineRegisterInfo *MRI = nullptr;
  const TPCInstrInfo *TII = nullptr;
  bool IsGen3Plus = false;

  /// A conditional jump to the LOOP's exit, the block it ends and the blocks
  /// it skips on the way to the LOOP (the LOOP's own block last).
  struct Guard {
    MachineBasicBlock *G = nullptr;
    MachineInstr *Jmp = nullptr;
    SmallVector<MachineBasicBlock *, 4> Skipped;
  };

  bool optimizeLoop(MachineInstr *&Loop);
  std::optional<Guard> findGuard(MachineInstr &Loop) const;
  bool skippedCodeIsHarmless(const Guard &Gd, const MachineInstr &Loop,
                             unsigned &Size) const;
  bool isHarmless(const MachineInstr &MI) const;
  bool fixExitPhis(const Guard &Gd, MachineBasicBlock &LoopMBB,
                   MachineBasicBlock &Exit, bool DryRun) const;
  Register loopEntryValue(const MachineInstr &ExitPhi,
                          const MachineBasicBlock &GuardMBB, Register GuardVal,
                          MachineBasicBlock &LoopMBB) const;
  bool computeSame(Register A, Register B) const;
  void removeJump(Guard &Gd, MachineInstr *&Loop);
  MachineInstr *rebuildLoop(MachineInstr &Loop, Register Pred,
                            int64_t Polarity);

  Register lookThroughCopies(Register R) const;
  bool isUnconditional(const MachineInstr &MI) const;
  CmpValue valueOf(const MachineOperand &MO) const;
  std::optional<Cond> condOfPredicate(Register Pred, bool Negate) const;
  Cond loopCond(const MachineInstr &Loop) const;
};

} // namespace

char TPCOptimizeHWLoopsGuards::ID = 0;

INITIALIZE_PASS(TPCOptimizeHWLoopsGuards, DEBUG_TYPE,
                "TPC Optimize Hardware Loops Guards", false, false)

FunctionPass *llvm::createTPCOptimizeHWLoopsGuards() {
  return new TPCOptimizeHWLoopsGuards();
}

static bool isPlainLoop(unsigned Opc) {
  switch (Opc) {
  case TPC::LOOPsss: case TPC::LOOPssi: case TPC::LOOPsis: case TPC::LOOPiss:
  case TPC::LOOPsii: case TPC::LOOPisi: case TPC::LOOPiis: case TPC::LOOPiii:
    return true;
  }
  return false;
}

static bool isPredicatedLoop(unsigned Opc) {
  switch (Opc) {
  case TPC::LOOPsssp: case TPC::LOOPssip: case TPC::LOOPsisp:
  case TPC::LOOPissp: case TPC::LOOPsiip: case TPC::LOOPisip:
  case TPC::LOOPiisp: case TPC::LOOPiiip:
    return true;
  }
  return false;
}

Register TPCOptimizeHWLoopsGuards::lookThroughCopies(Register R) const {
  while (R.isVirtual()) {
    const MachineInstr *Def = MRI->getUniqueVRegDef(R);
    if (!Def || !Def->isFullCopy() || !Def->getOperand(1).getReg().isVirtual())
      break;
    R = Def->getOperand(1).getReg();
  }
  return R;
}

// The last two explicit operands of a predicated TPC instruction are the
// predicate and its polarity.
bool TPCOptimizeHWLoopsGuards::isUnconditional(const MachineInstr &MI) const {
  unsigned N = MI.getNumExplicitOperands();
  if (N < 2)
    return false;
  const MachineOperand &Pred = MI.getOperand(N - 2);
  const MachineOperand &Pol = MI.getOperand(N - 1);
  return Pred.isReg() && Pred.getReg() == TPC::SPRF_TRUE && Pol.isImm() &&
         Pol.getImm() == 0;
}

CmpValue TPCOptimizeHWLoopsGuards::valueOf(const MachineOperand &MO) const {
  CmpValue V;
  if (MO.isImm()) {
    V.Kind = CmpValue::Imm;
    V.ImmVal = static_cast<int32_t>(MO.getImm());
    return V;
  }
  if (!MO.isReg() || !MO.getReg().isVirtual())
    return V;
  Register R = lookThroughCopies(MO.getReg());
  const MachineInstr *Def = MRI->getUniqueVRegDef(R);
  if (Def && isUnconditional(*Def)) {
    // MOV_IRF_DIM dest, src, dim, sw, income, pred, polarity.
    if (Def->getOpcode() == TPC::MOV_IRF_DIM &&
        Def->getOperand(3).getImm() == 0) {
      V.Kind = CmpValue::IRFDim;
      V.R = lookThroughCopies(Def->getOperand(1).getReg());
      V.Dim = Def->getOperand(2).getImm();
      return V;
    }
    // MOVsip dest, imm, optype, sw, income, pred, polarity.
    if (Def->getOpcode() == TPC::MOVsip && Def->getOperand(1).isImm() &&
        Def->getOperand(2).getImm() == TPCII::OpType::INT32 &&
        Def->getOperand(3).getImm() == 0) {
      V.Kind = CmpValue::Imm;
      V.ImmVal = static_cast<int32_t>(Def->getOperand(1).getImm());
      return V;
    }
  }
  V.Kind = CmpValue::Reg;
  V.R = R;
  return V;
}

/// The condition under which predicate Pred is true (false with Negate), if
/// a signed 32-bit compare defines it.
std::optional<Cond>
TPCOptimizeHWLoopsGuards::condOfPredicate(Register Pred, bool Negate) const {
  if (!Pred.isVirtual())
    return std::nullopt;
  const MachineInstr *Def = MRI->getUniqueVRegDef(lookThroughCopies(Pred));
  if (!Def || !isUnconditional(*Def))
    return std::nullopt;

  Cond C;
  bool IRF = false;
  switch (Def->getOpcode()) {
#define CMP_CASES(NAME, MODE)                                                  \
  case TPC::CMP_##NAME##ssp:                                                   \
  case TPC::CMP_##NAME##sip:                                                   \
    C.Mode = TPCII::MODE;                                                      \
    break;                                                                     \
  case TPC::CMP_##NAME##IIp:                                                   \
  case TPC::CMP_##NAME##Iip:                                                   \
    C.Mode = TPCII::MODE;                                                      \
    IRF = true;                                                                \
    break;
    CMP_CASES(EQ, LoopEQ)
    CMP_CASES(NEQ, LoopNE)
    CMP_CASES(LESS, LoopLT)
    CMP_CASES(LEQ, LoopLE)
    CMP_CASES(GRT, LoopGT)
    CMP_CASES(GEQ, LoopGE)
#undef CMP_CASES
  default:
    return std::nullopt;
  }

  // dest, op1, op2, [mask,] optype, sw, income, pred, polarity.
  unsigned TypeOp = IRF ? 4 : 3;
  if (Def->getOperand(TypeOp).getImm() != TPCII::OpType::INT32 ||
      Def->getOperand(TypeOp + 1).getImm() != 0)
    return std::nullopt;

  if (IRF) {
    // An IRF compare of one dimension.
    unsigned Mask = Def->getOperand(3).getImm();
    if (!isPowerOf2_32(Mask) || Mask > 0x10)
      return std::nullopt;
    unsigned Dim = Log2_32(Mask);
    auto DimValue = [&](const MachineOperand &MO) {
      CmpValue V;
      if (MO.isImm()) {
        V.Kind = CmpValue::Imm;
        V.ImmVal = static_cast<int32_t>(MO.getImm());
      } else if (MO.isReg() && MO.getReg().isVirtual()) {
        V.Kind = CmpValue::IRFDim;
        V.R = lookThroughCopies(MO.getReg());
        V.Dim = Dim;
      }
      return V;
    };
    C.L = DimValue(Def->getOperand(1));
    C.R = DimValue(Def->getOperand(2));
  } else {
    C.L = valueOf(Def->getOperand(1));
    C.R = valueOf(Def->getOperand(2));
  }
  if (Negate)
    C.Mode = invertMode(C.Mode);
  return C;
}

/// The condition under which the LOOP runs its first iteration.
Cond TPCOptimizeHWLoopsGuards::loopCond(const MachineInstr &Loop) const {
  Cond C;
  C.Mode = static_cast<TPCII::CmpMode>(Loop.getOperand(LoopCmpOp).getImm());
  C.L = valueOf(Loop.getOperand(LoopStartOp));
  C.R = valueOf(Loop.getOperand(LoopBoundaryOp));
  return C;
}

/// Walk up from the LOOP's block through blocks with one predecessor and one
/// successor to a block that ends in a conditional jump to the LOOP's exit.
std::optional<TPCOptimizeHWLoopsGuards::Guard>
TPCOptimizeHWLoopsGuards::findGuard(MachineInstr &Loop) const {
  MachineBasicBlock *LoopMBB = Loop.getParent();
  MachineBasicBlock *Exit = Loop.getOperand(LoopExitOp).getMBB();
  Guard Gd;

  // The jump may also stand right before the LOOP in its own block.
  if (Loop.getIterator() != LoopMBB->begin()) {
    MachineInstr &Prev = *std::prev(Loop.getIterator());
    if (Prev.getOpcode() == TPC::JMPR && Prev.getOperand(0).isMBB() &&
        Prev.getOperand(0).getMBB() == Exit) {
      Gd.G = LoopMBB;
      Gd.Jmp = &Prev;
      return Gd;
    }
    if (Prev.isTerminator())
      return std::nullopt;
  }

  MachineBasicBlock *B = LoopMBB;
  Gd.Skipped.push_back(B);
  for (unsigned Depth = 0; Depth < 16; ++Depth) {
    if (B->pred_size() != 1)
      return std::nullopt;
    MachineBasicBlock *Q = *B->pred_begin();
    if (Q == B || Q == Exit)
      return std::nullopt;
    if (Q->succ_size() == 1) {
      Gd.Skipped.push_back(Q);
      B = Q;
      continue;
    }
    if (Q->succ_size() != 2 || !Q->isSuccessor(Exit) || !Q->isSuccessor(B))
      return std::nullopt;
    // Q must end in 'JMPR Exit' and go on to B: by falling through, or by a
    // JMPR_u after the JMPR.
    MachineBasicBlock::iterator T = Q->getFirstTerminator();
    if (T == Q->end() || T->getOpcode() != TPC::JMPR ||
        !T->getOperand(0).isMBB() || T->getOperand(0).getMBB() != Exit)
      return std::nullopt;
    MachineInstr *Jmp = &*T;
    ++T;
    if (T != Q->end()) {
      if (T->getOpcode() != TPC::JMPR_u || T->getOperand(0).getMBB() != B ||
          std::next(T) != Q->end())
        return std::nullopt;
    } else if (!Q->isLayoutSuccessor(B)) {
      return std::nullopt;
    }
    Gd.G = Q;
    Gd.Jmp = Jmp;
    std::reverse(Gd.Skipped.begin(), Gd.Skipped.end());
    return Gd;
  }
  return std::nullopt;
}

/// Whether MI may run although the program would have skipped it: it has no
/// effect but its register results.
bool TPCOptimizeHWLoopsGuards::isHarmless(const MachineInstr &MI) const {
  if (MI.isDebugInstr() || MI.isImplicitDef())
    return true;
  if (MI.isPHI() || MI.isCall() || MI.isTerminator() || MI.isInlineAsm() ||
      MI.hasUnmodeledSideEffects() || MI.mayStore())
    return false;
  if (MI.mayLoad()) {
    // Tensor loads are bounds-checked by the hardware; local memory loads
    // only read. Global and MMIO loads are left alone.
    const MCInstrDesc &D = MI.getDesc();
    if (TPCInstrInfo::isMMIOAccess(MI))
      return false;
    if (!TPCII::isLdTnsr(D, IsGen3Plus) && !TPCII::is_ld_l(D) &&
        !TPCII::is_ld_l_v_family(D))
      return false;
  }
  // A physical register def, even a dead one, changes state: an LFSR read
  // has 'implicit-def dead $lfsr' because it advances the LFSR.
  for (const MachineOperand &MO : MI.operands())
    if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
      return false;
  return true;
}

bool TPCOptimizeHWLoopsGuards::skippedCodeIsHarmless(
    const Guard &Gd, const MachineInstr &Loop, unsigned &Size) const {
  Size = 0;
  for (MachineBasicBlock *B : Gd.Skipped) {
    for (const MachineInstr &MI : *B) {
      if (&MI == &Loop)
        break;
      if (MI.getOpcode() == TPC::JMPR_u)
        continue; // To the next skipped block.
      if (!isHarmless(MI)) {
        LLVM_DEBUG(dbgs() << "  guard skips " << MI);
        return false;
      }
      if (!MI.isDebugInstr() && !MI.isImplicitDef() && !MI.isCopy())
        ++Size;
    }
  }
  return true;
}

/// Without the guard, a loop that does not run reaches the exit from the
/// LOOP's block. Give each exit PHI there the value it had from the guard's
/// block, which dominates the LOOP's block. Fails when the LOOP's block
/// already carries another real value.
bool TPCOptimizeHWLoopsGuards::fixExitPhis(const Guard &Gd,
                                           MachineBasicBlock &LoopMBB,
                                           MachineBasicBlock &Exit,
                                           bool DryRun) const {
  if (Gd.G == &LoopMBB)
    return true;
  for (MachineInstr &Phi : Exit.phis()) {
    unsigned GuardIdx = 0, LoopIdx = 0;
    for (unsigned I = 1, E = Phi.getNumOperands(); I < E; I += 2) {
      if (Phi.getOperand(I + 1).getMBB() == Gd.G)
        GuardIdx = I;
      else if (Phi.getOperand(I + 1).getMBB() == &LoopMBB)
        LoopIdx = I;
    }
    if (!GuardIdx || !LoopIdx)
      return false;
    Register GuardVal = Phi.getOperand(GuardIdx).getReg();
    Register LoopVal = Phi.getOperand(LoopIdx).getReg();
    if (DryRun) {
      if (GuardVal == LoopVal)
        continue;
      const MachineInstr *Def = MRI->getUniqueVRegDef(LoopVal);
      if (!Def || !Def->isImplicitDef())
        return false;
      continue;
    }
    Register Val = loopEntryValue(Phi, *Gd.G, GuardVal, LoopMBB);
    MRI->clearKillFlags(Val); // Now live out of the LOOP's block.
    Phi.getOperand(LoopIdx).setReg(Val);
    Phi.removeOperand(GuardIdx + 1);
    Phi.removeOperand(GuardIdx);
  }
  return true;
}

/// GuardVal is usually the initial value of a loop-carried register: a
/// reduction that the exit PHI also receives from the latch. The loop
/// header's PHI for that register then often takes its own copy of the same
/// value from the LOOP's block (each edge materialized the constant). Return
/// that copy, so both PHIs read one register and the other copy dies.
Register TPCOptimizeHWLoopsGuards::loopEntryValue(
    const MachineInstr &ExitPhi, const MachineBasicBlock &GuardMBB,
    Register GuardVal, MachineBasicBlock &LoopMBB) const {
  const MachineBasicBlock *Exit = ExitPhi.getParent();
  MachineBasicBlock *Header = nullptr;
  for (MachineBasicBlock *S : LoopMBB.successors())
    if (S != Exit)
      Header = S;
  // The latch value: the one input from neither the guard nor the LOOP's
  // block.
  Register LatchVal;
  MachineBasicBlock *Latch = nullptr;
  for (unsigned I = 1, E = ExitPhi.getNumOperands(); I < E; I += 2) {
    MachineBasicBlock *From = ExitPhi.getOperand(I + 1).getMBB();
    if (From == &LoopMBB || From == &GuardMBB)
      continue;
    if (Latch)
      return GuardVal;
    Latch = From;
    LatchVal = ExitPhi.getOperand(I).getReg();
  }
  if (!Header || !Latch)
    return GuardVal;
  for (const MachineInstr &HPhi : Header->phis()) {
    Register FromLatch, FromLoopMBB;
    for (unsigned I = 1, E = HPhi.getNumOperands(); I < E; I += 2) {
      if (HPhi.getOperand(I + 1).getMBB() == Latch)
        FromLatch = HPhi.getOperand(I).getReg();
      else if (HPhi.getOperand(I + 1).getMBB() == &LoopMBB)
        FromLoopMBB = HPhi.getOperand(I).getReg();
    }
    if (FromLatch == LatchVal && FromLoopMBB.isValid() &&
        MRI->getRegClass(FromLoopMBB) == MRI->getRegClass(GuardVal) &&
        computeSame(FromLoopMBB, GuardVal))
      return FromLoopMBB;
  }
  return GuardVal;
}

/// Whether A and B are computed by the same side-effect-free instruction
/// from the same operands. Unconditional instructions ignore their income
/// operand, which is a separate IMPLICIT_DEF for each.
bool TPCOptimizeHWLoopsGuards::computeSame(Register A, Register B) const {
  A = lookThroughCopies(A);
  B = lookThroughCopies(B);
  if (A == B)
    return true;
  const MachineInstr *DA = MRI->getUniqueVRegDef(A);
  const MachineInstr *DB = MRI->getUniqueVRegDef(B);
  if (!DA || !DB || DA->getOpcode() != DB->getOpcode() ||
      DA->getNumOperands() != DB->getNumOperands() || DA->mayLoad() ||
      DA->mayStore() || DA->hasUnmodeledSideEffects() ||
      DA->getDesc().getNumDefs() != 1 || !isUnconditional(*DA) ||
      !isUnconditional(*DB))
    return false;
  for (unsigned I = 1, E = DA->getNumOperands(); I < E; ++I) {
    const MachineOperand &OA = DA->getOperand(I), &OB = DB->getOperand(I);
    if (OA.isReg() && OB.isReg() && OA.isTied() && OB.isTied() &&
        OA.getReg().isVirtual() && OB.getReg().isVirtual()) {
      const MachineInstr *IA = MRI->getUniqueVRegDef(OA.getReg());
      const MachineInstr *IB = MRI->getUniqueVRegDef(OB.getReg());
      if (IA && IB && IA->isImplicitDef() && IB->isImplicitDef())
        continue;
    }
    if (!OA.isIdenticalTo(OB))
      return false;
  }
  return true;
}

/// Remove the guard's jump, then merge the guard's block with the next one if
/// nothing else enters it.
void TPCOptimizeHWLoopsGuards::removeJump(Guard &Gd, MachineInstr *&Loop) {
  MachineBasicBlock *G = Gd.G;
  MachineBasicBlock *Exit = Loop->getOperand(LoopExitOp).getMBB();
  fixExitPhis(Gd, *Loop->getParent(), *Exit, /*DryRun=*/false);
  Gd.Jmp->eraseFromParent();
  if (G == Loop->getParent())
    return; // The LOOP's block still goes to the exit.
  G->removeSuccessor(Exit);

  MachineBasicBlock *Next = Gd.Skipped.front();
  if (Next->pred_size() != 1 || G->succ_size() != 1)
    return;
  MachineBasicBlock::iterator T = G->getFirstTerminator();
  if (T != G->end())
    T->eraseFromParent(); // JMPR_u to Next.
  G->splice(G->end(), Next, Next->begin(), Next->end());
  G->removeSuccessor(Next);
  G->transferSuccessorsAndUpdatePHIs(Next);
  Next->eraseFromParent();
}

/// Rebuild the LOOP with predicate Pred, or without a predicate if Pred is 0.
MachineInstr *TPCOptimizeHWLoopsGuards::rebuildLoop(MachineInstr &Loop,
                                                    Register Pred,
                                                    int64_t Polarity) {
  const MachineOperand &Start = Loop.getOperand(LoopStartOp);
  const MachineOperand &Bound = Loop.getOperand(LoopBoundaryOp);
  const MachineOperand &Step = Loop.getOperand(LoopStepOp);
  unsigned Opc = getLoopMachineInstrOpcode(Start.isImm(), Bound.isImm(),
                                           Step.isImm(), Pred.isValid());
  // Copy the implicit operands below rather than take the descriptor's.
  MachineFunction &MF = *Loop.getMF();
  MachineInstr *NewMI =
      MF.CreateMachineInstr(TII->get(Opc), Loop.getDebugLoc(), true);
  Loop.getParent()->insert(Loop.getIterator(), NewMI);
  MachineInstrBuilder MIB(MF, NewMI);
  for (unsigned I = 0; I <= LoopExitOp; ++I)
    MIB.add(Loop.getOperand(I));
  if (Pred.isValid())
    MIB.addReg(Pred).addImm(Polarity);
  // The loop counter and other implicit operands.
  for (unsigned I = Loop.getNumExplicitOperands(), E = Loop.getNumOperands();
       I < E; ++I)
    MIB.add(Loop.getOperand(I));
  MIB->setFlags(Loop.getFlags());
  Loop.eraseFromParent();
  return MIB;
}

bool TPCOptimizeHWLoopsGuards::optimizeLoop(MachineInstr *&Loop) {
  bool Changed = false;
  for (unsigned Iter = 0; Iter < 8; ++Iter) {
    Cond LC = loopCond(*Loop);

    if (isPredicatedLoop(Loop->getOpcode())) {
      const MachineOperand &P = Loop->getOperand(LoopPredOp);
      const int64_t Pol = Loop->getOperand(LoopPolarityOp).getImm();
      std::optional<Cond> PC =
          P.isReg() ? condOfPredicate(P.getReg(), Pol != 0) : std::nullopt;
      if (PC && sameCond(*PC, LC)) {
        LLVM_DEBUG(dbgs() << "Unpredicate " << *Loop);
        Loop = rebuildLoop(*Loop, Register(), 0);
        ++NumUnpredicated;
        Changed = true;
        continue;
      }
    }

    std::optional<Guard> Gd = findGuard(*Loop);
    if (!Gd)
      break;
    LLVM_DEBUG(dbgs() << "Guard " << *Gd->Jmp << "  of " << *Loop);
    unsigned Size;
    MachineBasicBlock &Exit = *Loop->getOperand(LoopExitOp).getMBB();
    if (!skippedCodeIsHarmless(*Gd, *Loop, Size) ||
        !fixExitPhis(*Gd, *Loop->getParent(), Exit, /*DryRun=*/true))
      break;

    // JMPR jumps when its predicate differs from its polarity, so the loop
    // is entered when they are equal.
    const Register JmpPred = Gd->Jmp->getOperand(1).getReg();
    const int64_t JmpPol = Gd->Jmp->getOperand(2).getImm();
    std::optional<Cond> Enter = condOfPredicate(JmpPred, JmpPol == 0);
    if (Enter && sameCond(*Enter, LC)) {
      removeJump(*Gd, Loop);
      ++NumRemoved;
      Changed = true;
      continue;
    }

    if (!PredicateLoops || !isPlainLoop(Loop->getOpcode()) ||
        !JmpPred.isVirtual() || Size > MaxPredicateBlockSize)
      break;
    // The LOOP runs when its predicate differs from its polarity.
    removeJump(*Gd, Loop);
    MRI->clearKillFlags(JmpPred); // Now used by the LOOP.
    Loop = rebuildLoop(*Loop, JmpPred, JmpPol == 0 ? 1 : 0);
    ++NumPredicated;
    Changed = true;
  }
  return Changed;
}

bool TPCOptimizeHWLoopsGuards::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableGuardOpt || skipFunction(MF.getFunction()))
    return false;
  MRI = &MF.getRegInfo();
  const auto &ST = MF.getSubtarget<TPCSubtarget>();
  TII = ST.getInstrInfo();
  IsGen3Plus = ST.hasGen3Plus();

  SmallVector<MachineInstr *, 8> Loops;
  for (MachineBasicBlock &MBB : MF)
    for (MachineInstr &MI : MBB.terminators())
      if (isPlainLoop(MI.getOpcode()) || isPredicatedLoop(MI.getOpcode()))
        Loops.push_back(&MI);

  bool Changed = false;
  for (MachineInstr *Loop : Loops)
    Changed |= optimizeLoop(Loop);
  return Changed;
}
