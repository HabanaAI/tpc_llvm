//===- TPCModuloScheduling.cpp - TPC support for the MachinePipeliner -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// LLVM's MachinePipeliner modulo-schedules single-block loops (on gaudi2;
// -tpc-modulo-schedule) before they become hardware loops, while the loop is
// still a compare and a branch, and its MVE expander unrolls the kernel
// instead of copying values between iterations. This file holds what it
// needs from TPC:
//
//  - TPCModuloScheduleSetup gives each such loop a preheader, which the
//    pipeliner requires: an inner loop whose entry is guarded by a branch
//    around it has none.
//  - TPCInstrInfo::analyzeLoopForPipelining describes the loop: its exit
//    condition and counter, for the expander's tests of how many iterations
//    are left (made so that the kernel still becomes a hardware loop);
//    whether its iterations access memory independently, from the machine
//    loop dependence analysis, which the pipeliner cannot see through int5
//    coordinates (without it no load of a later iteration could start before
//    an earlier iteration's store); and whether the machine unroller would do
//    better (shouldUseSchedule). It also spreads set_indx over the slots that
//    issue it, as the scheduler later does.
//  - TPCPipelinerLatencyMutation gives the dependence graph the latencies of
//    values read through copy-like pseudos and phis.
//  - TPCModuloScheduleFinish lays out the loop the expander keeps for the
//    iterations left so that it becomes a hardware loop as well;
//    TPCMachineFunctionInfo tells the machine unroller to leave it alone.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCLoopData.h"
#include "TPCMachineFunctionInfo.h"
#include "TPCMachineLoopDependencyAnalysis.h"
#include "TPCMachineLoopLinearIV.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachinePipeliner.h"
#include "llvm/CodeGen/ScheduleDAGMutation.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include <string>

#define DEBUG_TYPE "tpc-msched"

using namespace llvm;

static cl::opt<bool> EnableModuloSchedule(
    "tpc-modulo-schedule", cl::Hidden, cl::init(true),
    cl::desc("Modulo-schedule single-block loops with the MachinePipeliner "
             "before they become hardware loops (by default on gaudi2 only, "
             "where it is measured; given, on every target)"));

bool TPCSubtarget::enableMachinePipeliner() const {
  if (EnableModuloSchedule.getNumOccurrences())
    return EnableModuloSchedule;
  return hasGaudi2ISA();
}

namespace {

class TPCModuloScheduleSetup : public MachineFunctionPass {
public:
  static char ID;
  TPCModuloScheduleSetup() : MachineFunctionPass(ID) {
    initializeTPCModuloScheduleSetupPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "TPC Modulo Schedule Setup";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace

char TPCModuloScheduleSetup::ID = 0;

INITIALIZE_PASS_BEGIN(TPCModuloScheduleSetup, "tpc-msched-setup",
                      "TPC Modulo Schedule Setup", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfoWrapperPass)
INITIALIZE_PASS_END(TPCModuloScheduleSetup, "tpc-msched-setup",
                    "TPC Modulo Schedule Setup", false, false)

FunctionPass *llvm::createTPCModuloScheduleSetup() {
  return new TPCModuloScheduleSetup();
}

static void collectSingleBlockLoops(MachineLoop &L,
                                    SmallVectorImpl<MachineLoop *> &Loops) {
  if (L.isInnermost()) {
    if (L.getNumBlocks() == 1)
      Loops.push_back(&L);
    return;
  }
  for (MachineLoop *Sub : L)
    collectSingleBlockLoops(*Sub, Loops);
}

bool TPCModuloScheduleSetup::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()) ||
      !MF.getSubtarget().enableMachinePipeliner())
    return false;

  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();
  SmallVector<MachineLoop *, 8> Loops;
  for (MachineLoop *L : MLI)
    collectSingleBlockLoops(*L, Loops);

  bool Changed = false;
  for (MachineLoop *L : Loops) {
    if (L->getLoopPreheader())
      continue;
    MachineBasicBlock *Header = L->getHeader();
    MachineBasicBlock *Pred = L->getLoopPredecessor();
    if (!Pred || !Pred->canSplitCriticalEdge(Header, &MLI))
      continue;
    if (MachineBasicBlock *PH = Pred->SplitCriticalEdge(Header, *this)) {
      LLVM_DEBUG(dbgs() << "Preheader " << printMBBReference(*PH) << " for "
                        << printMBBReference(*Header) << "\n");
      Changed = true;
    }
  }
  return Changed;
}

namespace {

/// After the pipeliner: the MVE expander keeps the original loop for the
/// iterations the pipelined loop leaves, with a new exit block placed before
/// it. A hardware loop must fall through to its exit, so the exit block, which
/// only jumps on, is moved after the loop.
class TPCModuloScheduleFinish : public MachineFunctionPass {
public:
  static char ID;
  TPCModuloScheduleFinish() : MachineFunctionPass(ID) {
    initializeTPCModuloScheduleFinishPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "TPC Modulo Schedule Finish";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace

char TPCModuloScheduleFinish::ID = 0;

INITIALIZE_PASS_BEGIN(TPCModuloScheduleFinish, "tpc-msched-finish",
                      "TPC Modulo Schedule Finish", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfoWrapperPass)
INITIALIZE_PASS_END(TPCModuloScheduleFinish, "tpc-msched-finish",
                    "TPC Modulo Schedule Finish", false, false)

FunctionPass *llvm::createTPCModuloScheduleFinish() {
  return new TPCModuloScheduleFinish();
}

bool TPCModuloScheduleFinish::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()) ||
      !MF.getSubtarget().enableMachinePipeliner())
    return false;
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();
  SmallVector<MachineLoop *, 8> Loops;
  for (MachineLoop *L : MLI)
    collectSingleBlockLoops(*L, Loops);

  bool Changed = false;
  for (MachineLoop *L : Loops) {
    MachineBasicBlock *Body = L->getHeader();
    MachineBasicBlock *Exit = L->getExitBlock();
    if (!Exit || Exit == Body || Body->isLayoutSuccessor(Exit) ||
        Exit->getIterator() == MF.begin())
      continue;
    // The exit only merges values (phis) and jumps on, and nothing falls
    // through into it.
    MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
    SmallVector<MachineOperand, 2> Cond;
    if (Exit->succ_size() != 1 || Exit->getFirstNonPHI() == Exit->end() ||
        !Exit->getFirstNonPHI()->isUnconditionalBranch() ||
        TII->analyzeBranch(*Exit, TBB, FBB, Cond) || !TBB || !Cond.empty())
      continue;
    MachineBasicBlock *Prev = &*std::prev(Exit->getIterator());
    if (Prev->isSuccessor(Exit)) {
      MachineBasicBlock *PT = nullptr, *PF = nullptr;
      SmallVector<MachineOperand, 2> PC;
      if (TII->analyzeBranch(*Prev, PT, PF, PC) || !PT ||
          (!PC.empty() && !PF))
        continue;
    }
    // The loop leaves by a branch, which must stay analyzable.
    MachineBasicBlock *LT = nullptr, *LF = nullptr;
    SmallVector<MachineOperand, 2> LC;
    if (TII->analyzeBranch(*Body, LT, LF, LC) || LC.empty() || !LF)
      continue;
    Exit->moveAfter(Body);
    Changed = true;
  }
  return Changed;
}

namespace {

/// How the loop counts: the compare tests Counter REL Bound, where Counter is
/// the counter's value before (the phi) or after (the update) the iteration's
/// update by Step, and the loop goes on while the relation holds.
struct LoopCounter {
  enum Relation { LT, LE, GT, GE };
  unsigned CounterIdx = 0;  // Comp's operand that holds the counter.
  MachineOperand Bound = MachineOperand::CreateImm(0);
  int64_t Step = 0;
  bool CompOnUpdate = false;
  Register Init;            // The phi's value from the preheader.
  Relation Rel = LT;
  /// The most iterations the loop can run, when the start and the bound
  /// show it (the remainder of an unrolled loop counts down from n & 3).
  std::optional<int64_t> MaxTripCount;
};

class TPCPipelinerLoopInfo : public TargetInstrInfo::PipelinerLoopInfo {
  const TargetInstrInfo &TII;
  MachineRegisterInfo &MRI;
  /// The instruction that computes the branch's predicate.
  MachineInstr *Comp;
  /// The condition on which the loop is left, as insertBranch takes it.
  SmallVector<MachineOperand, 2> ExitCond;
  /// Whether no memory access of an iteration depends on another iteration's.
  bool Independent;
  /// The counter, when the loop counts in a way the MVE expander can test.
  std::optional<LoopCounter> Counter;
  /// The block before the loop, where bounds for the tests are computed.
  MachineBasicBlock *Preheader;
  /// What the machine unroller would make of the loop, if it would unroll it.
  std::optional<AutoUnrollEstimate> Unroll;
  /// Cycles per iteration of the loop as it is, by the unroller's estimate.
  unsigned AsIs;
  /// Instructions moved to another slot for the schedule, with their opcodes
  /// before; moved back unless the loop is pipelined.
  SmallVector<std::pair<MachineInstr *, unsigned>, 8> Reslotted;

public:
  TPCPipelinerLoopInfo(const TargetInstrInfo &TII, MachineRegisterInfo &MRI,
                       MachineInstr *Comp, ArrayRef<MachineOperand> ExitCond,
                       bool Independent, std::optional<LoopCounter> Counter,
                       MachineBasicBlock *Preheader,
                       std::optional<AutoUnrollEstimate> Unroll,
                       unsigned AsIs,
                       ArrayRef<std::pair<MachineInstr *, unsigned>> Reslotted)
      : TII(TII), MRI(MRI), Comp(Comp), ExitCond(ExitCond),
        Independent(Independent), Counter(Counter), Preheader(Preheader),
        Unroll(Unroll), AsIs(AsIs), Reslotted(Reslotted) {}

  ~TPCPipelinerLoopInfo() override { restoreSlots(); }

  void restoreSlots() {
    for (auto [MI, Opc] : Reslotted)
      MI->setDesc(TII.get(Opc));
    Reslotted.clear();
  }

  bool shouldUseSchedule(SwingSchedulerDAG &SSD, SMSchedule &SMS) override;

  // The compare and the instructions it depends on (the counter's update) go
  // into stage 0, so that each prologue's copy of the compare tells whether
  // its iteration is the last, and the kernel's tells it for the newest
  // iteration it starts; the trip count needs no adjusting.
  bool shouldIgnoreForPipelining(const MachineInstr *MI) const override {
    return MI == Comp;
  }

  std::optional<bool>
  createTripCountGreaterCondition(int TC, MachineBasicBlock &MBB,
                                  SmallVectorImpl<MachineOperand> &Cond) override {
    // The branch is inserted as "if (Cond) goto epilogue", and its predicate
    // renamed to the prologue's copy of the compare.
    Cond.assign(ExitCond.begin(), ExitCond.end());
    return std::nullopt;
  }

  void createRemainingIterationsGreaterCondition(
      int TC, MachineBasicBlock &MBB, SmallVectorImpl<MachineOperand> &Cond,
      DenseMap<MachineInstr *, MachineInstr *> &LastStage0Insts) override;

  void adjustTripCount(int TripCountAdjust) override {}
  void setPreheader(MachineBasicBlock *NewPreheader) override {}

  bool isMVEExpanderSupported() override { return Counter.has_value(); }
  bool preferMVEExpander() override { return true; }

  bool mayHaveLoopCarriedMemoryDependences() const override {
    return !Independent;
  }
};

} // namespace

static cl::opt<bool> BalanceSlots(
    "tpc-msched-balance-slots", cl::Hidden, cl::init(true),
    cl::desc("Spread set_indx over the load, scalar and store slots before "
             "modulo scheduling"));

static cl::opt<int> UnrollAdvantage(
    "tpc-msched-unroll-advantage", cl::Hidden, cl::init(0),
    cl::desc("Leave a loop to the machine unroller when its estimate per "
             "iteration is at most the pipelined loop's initiation interval "
             "less this percentage"));

bool TPCPipelinerLoopInfo::shouldUseSchedule(SwingSchedulerDAG &SSD,
                                             SMSchedule &SMS) {
  const unsigned II = SMS.getInitiationInterval();
  const unsigned Stages = SMS.getMaxStageCount() + 1;
  // The copies of the kernel the MVE expander makes (as
  // ModuloScheduleExpanderMVE::calcNumUnroll counts them): one more than the
  // most stages a value lives across.
  unsigned Copies = 1;
  for (SUnit &SU : SSD.SUnits) {
    MachineInstr *MI = SU.getInstr();
    if (!MI || MI->isPHI() || !SMS.isScheduledAtStage(&SU, SMS.stageScheduled(&SU)))
      continue;
    for (const MachineOperand &MO : MI->uses()) {
      if (!MO.isReg() || !MO.getReg().isVirtual())
        continue;
      MachineInstr *Def = MRI.getVRegDef(MO.getReg());
      if (!Def || Def->getParent() != MI->getParent())
        continue;
      unsigned N = 1;
      if (Def->isPHI()) {
        ++N;
        Register LoopVal;
        for (unsigned I = 1; I + 1 < Def->getNumOperands(); I += 2)
          if (Def->getOperand(I + 1).getMBB() == MI->getParent())
            LoopVal = Def->getOperand(I).getReg();
        Def = LoopVal ? MRI.getVRegDef(LoopVal) : nullptr;
        if (!Def || Def->isPHI())
          continue;
      }
      SUnit *DefSU = SSD.getSUnit(Def);
      if (!DefSU)
        continue;
      int Local = int(N) + SMS.stageScheduled(&SU) - SMS.stageScheduled(DefSU);
      if (SMS.cycleScheduled(&SU) <= SMS.cycleScheduled(DefSU))
        --Local;
      Copies = std::max(Copies, unsigned(std::max(Local, 1)));
    }
  }
  // A hardware loop body has at least MinBody bundles.
  const unsigned MinBody = 6;
  const double Pipelined = double(std::max(II * Copies, MinBody)) / Copies;
  const double Rolled = std::max(AsIs, MinBody);
  // The pipeliner does not use a schedule without overlap, nor one with more
  // than -pipeliner-max-stages (3 by default) stages after the first.
  bool Use = Stages > 1 && Stages <= 4;
  // A loop too short to fill the pipeline and run the kernel a few times
  // would only pay for its check.
  if (Counter && Counter->MaxTripCount &&
      *Counter->MaxTripCount < int64_t(2 * Stages))
    Use = false;
  // On a tie the unroller's loop is simpler: no check of the trip count, no
  // prologue and epilogue (and on the card the loops that tie, streaming
  // ones, run at the speed of memory either way).
  if (Unroll && int(Unroll->After) * 100 <= int(II) * (100 - UnrollAdvantage))
    Use = false;
  // Without the unroller, the kernel must beat the loop as it is.
  if (Pipelined >= Rolled)
    Use = false;
  if (Use)
    Reslotted.clear();
  else
    restoreSlots();
  LLVM_DEBUG(dbgs() << "Pipelining " << printMBBReference(*Comp->getParent())
                    << ": II " << II << ", " << Stages << " stages, " << Copies
                    << " copies: " << Pipelined << " vs " << Rolled
                    << " as is; unroller "
                    << (Unroll ? "x" + std::to_string(Unroll->Count) + " " +
                                     std::to_string(Unroll->Before) + " -> " +
                                     std::to_string(Unroll->After)
                               : std::string("does not apply"))
                    << (Counter && Counter->MaxTripCount
                            ? "; at most " +
                                  std::to_string(*Counter->MaxTripCount) +
                                  " iterations"
                            : std::string())
                    << (Use ? "; pipelined" : "; not pipelined")
                    << "\n");
  return Use;
}

static unsigned compareOpcode(LoopCounter::Relation Rel, bool ImmBound) {
  switch (Rel) {
  case LoopCounter::LT:
    return ImmBound ? TPC::CMP_LESSsip : TPC::CMP_LESSssp;
  case LoopCounter::LE:
    return ImmBound ? TPC::CMP_LEQsip : TPC::CMP_LEQssp;
  case LoopCounter::GT:
    return ImmBound ? TPC::CMP_GRTsip : TPC::CMP_GRTssp;
  case LoopCounter::GE:
    return ImmBound ? TPC::CMP_GEQsip : TPC::CMP_GEQssp;
  }
  llvm_unreachable("unknown relation");
}

static Register buildAddImm(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator Pos, Register Src,
                            int64_t Imm, MachineRegisterInfo &MRI,
                            const TargetInstrInfo &TII) {
  const Register Dst = MRI.createVirtualRegister(&TPC::SRFRegClass);
  BuildMI(MBB, Pos, DebugLoc(), TII.get(TPC::ADDsip), Dst)
      .addReg(Src)
      .addImm(Imm)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(Dst, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  return Dst;
}

// Whether at least TC + 1 more iterations start after the one whose compare
// is LastStage0Insts[Comp] (after the first, when that is empty). The
// counters those iterations test are C + Step, ..., C + (TC + 1) * Step for
// the last tested counter C, and all go on while C + TC * Step holds the
// relation with the bound (see analyzeCounter). In a kernel, that is tested
// as C against Bound - TC * Step, computed before the loop, so that the
// kernel still becomes a hardware loop over C.
void TPCPipelinerLoopInfo::createRemainingIterationsGreaterCondition(
    int TC, MachineBasicBlock &MBB, SmallVectorImpl<MachineOperand> &Cond,
    DenseMap<MachineInstr *, MachineInstr *> &LastStage0Insts) {
  assert(Counter && "the loop's counting was not understood");
  const LoopCounter &LC = *Counter;
  // The MVE expander keeps the loop analyzed for the iterations left.
  MachineFunction &MF = *Comp->getMF();
  MF.getInfo<TPCMachineFunctionInfo>()->addPipelineRemainder(
      Comp->getParent());
  Register C;
  int64_t Offset = int64_t(TC) * LC.Step;
  if (LastStage0Insts.empty()) {
    // The first iteration tests Init, or Init + Step after the update.
    C = LC.Init;
    Offset += LC.CompOnUpdate ? LC.Step : 0;
    if (Offset != 0) {
      C = buildAddImm(MBB, MBB.end(), C, Offset, MRI, TII);
      Offset = 0;
    }
  } else {
    C = LastStage0Insts[Comp]->getOperand(LC.CounterIdx).getReg();
  }

  MachineOperand Bound = LC.Bound;
  if (Offset != 0) {
    if (Bound.isImm()) {
      Bound = MachineOperand::CreateImm(Bound.getImm() - Offset);
    } else {
      const Register B =
          buildAddImm(*Preheader, Preheader->getFirstTerminator(),
                      Bound.getReg(), -Offset, MRI, TII);
      Bound = MachineOperand::CreateReg(B, false);
    }
  }

  const Register P = MRI.createVirtualRegister(&TPC::SPRFRegClass);
  MachineInstrBuilder MIB =
      BuildMI(MBB, MBB.end(), Comp->getDebugLoc(),
              TII.get(compareOpcode(LC.Rel, Bound.isImm())), P)
          .addReg(C);
  if (Bound.isImm())
    MIB.addImm(Bound.getImm());
  else
    MIB.addReg(Bound.getReg());
  MIB.addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(P, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  Cond.clear();
  Cond.push_back(MachineOperand::CreateReg(P, false));
  Cond.push_back(MachineOperand::CreateImm(0));
}

static bool isPlainScalarOp(const MachineInstr &MI) {
  // Operands: def, src1, src2, type, switches, income, predicate, polarity.
  return MI.getNumExplicitOperands() == 8 && MI.getOperand(3).isImm() &&
         MI.getOperand(3).getImm() == TPCII::OpType::INT32 &&
         MI.getOperand(4).isImm() && MI.getOperand(4).getImm() == 0 &&
         MI.getOperand(6).isReg() &&
         MI.getOperand(6).getReg() == TPC::SPRF_TRUE &&
         MI.getOperand(7).isImm() && MI.getOperand(7).getImm() == 0;
}

/// The immediate a MOV without a predicate sets Reg to.
static std::optional<int64_t> movedImm(Register Reg,
                                       const MachineRegisterInfo &MRI) {
  const MachineInstr *Def = Reg.isVirtual() ? MRI.getVRegDef(Reg) : nullptr;
  if (!Def || Def->getOpcode() != TPC::MOVsip || !Def->getOperand(1).isImm())
    return std::nullopt;
  const unsigned N = Def->getNumExplicitOperands();
  if (N < 6 || !Def->getOperand(N - 2).isReg() ||
      Def->getOperand(N - 2).getReg() != TPC::SPRF_TRUE)
    return std::nullopt;
  return Def->getOperand(1).getImm();
}

static bool startsAtMultipleUpTo0(const LoopCounter &LC,
                                  const MachineRegisterInfo &MRI) {
  const std::optional<int64_t> I = movedImm(LC.Init, MRI);
  return I && *I <= 0 && *I % LC.Step == 0;
}

/// Whether Bound is n & M (so not negative) with Step dividing M.
static bool isMaskedMultiple(const MachineOperand &Bound, int64_t Step,
                             const MachineRegisterInfo &MRI) {
  if (Bound.isImm())
    return Bound.getImm() >= 0 && Bound.getImm() % Step == 0;
  const MachineInstr *Def = MRI.getVRegDef(Bound.getReg());
  return Def && Def->getOpcode() == TPC::ANDsip && isPlainScalarOp(*Def) &&
         Def->getOperand(2).isImm() && Def->getOperand(2).getImm() >= 0 &&
         Def->getOperand(2).getImm() % Step == 0;
}

/// The loop's counting, when the compare Comp tests a counter that a phi
/// starts and an add of an immediate steps, against a bound that does not
/// change, and the loop goes on while the counter is on one side of the
/// bound (or, stepping by one, while it differs from it). ExitOnTrue: the
/// loop is left when Comp's result is true.
static std::optional<LoopCounter>
analyzeCounter(const MachineInstr &Comp, bool ExitOnTrue,
               const MachineBasicBlock &LoopBB,
               const MachineRegisterInfo &MRI) {
  enum { EQ, NE, LT, LE, GT, GE } Mode;
  switch (Comp.getOpcode()) {
  case TPC::CMP_EQssp: case TPC::CMP_EQsip: Mode = EQ; break;
  case TPC::CMP_NEQssp: case TPC::CMP_NEQsip: Mode = NE; break;
  case TPC::CMP_LESSssp: case TPC::CMP_LESSsip: Mode = LT; break;
  case TPC::CMP_LEQssp: case TPC::CMP_LEQsip: Mode = LE; break;
  case TPC::CMP_GRTssp: case TPC::CMP_GRTsip: Mode = GT; break;
  case TPC::CMP_GEQssp: case TPC::CMP_GEQsip: Mode = GE; break;
  default:
    return std::nullopt;
  }
  if (!isPlainScalarOp(Comp))
    return std::nullopt;

  auto DefinedInLoop = [&](const MachineOperand &MO) {
    return MO.isReg() && MO.getReg().isVirtual() &&
           MRI.getVRegDef(MO.getReg())->getParent() == &LoopBB;
  };
  auto Invariant = [&](const MachineOperand &MO) {
    return MO.isImm() ||
           (MO.isReg() && MO.getReg().isVirtual() && !DefinedInLoop(MO));
  };
  LoopCounter LC;
  const MachineOperand &Op1 = Comp.getOperand(1), &Op2 = Comp.getOperand(2);
  bool Swapped;
  if (DefinedInLoop(Op1) && Invariant(Op2)) {
    LC.CounterIdx = 1;
    Swapped = false;
  } else if (DefinedInLoop(Op2) && Invariant(Op1)) {
    LC.CounterIdx = 2;
    Swapped = true;
  } else {
    return std::nullopt;
  }
  LC.Bound = Comp.getOperand(Swapped ? 1 : 2);
  if (LC.Bound.isImm() && !isInt<32>(LC.Bound.getImm()))
    return std::nullopt;

  // The counter: the phi, or the add of an immediate to the phi that the
  // phi takes from the loop.
  const MachineInstr *CounterDef =
      MRI.getVRegDef(Comp.getOperand(LC.CounterIdx).getReg());
  const MachineInstr *Phi = CounterDef;
  const MachineInstr *Update = nullptr;
  if (!Phi->isPHI()) {
    Update = CounterDef;
    if (Update->getOpcode() != TPC::ADDsip || !isPlainScalarOp(*Update) ||
        !Update->getOperand(1).isReg())
      return std::nullopt;
    Phi = MRI.getVRegDef(Update->getOperand(1).getReg());
    LC.CompOnUpdate = true;
  }
  if (!Phi->isPHI() || Phi->getParent() != &LoopBB ||
      Phi->getNumOperands() != 5)
    return std::nullopt;
  for (unsigned I = 1; I < 5; I += 2) {
    if (Phi->getOperand(I + 1).getMBB() == &LoopBB) {
      const MachineInstr *Next = MRI.getVRegDef(Phi->getOperand(I).getReg());
      if (Update && Next != Update)
        return std::nullopt;
      Update = Next;
    } else {
      LC.Init = Phi->getOperand(I).getReg();
    }
  }
  if (!Update || !LC.Init || Update->getOpcode() != TPC::ADDsip ||
      !isPlainScalarOp(*Update) || !Update->getOperand(1).isReg() ||
      Update->getOperand(1).getReg() != Phi->getOperand(0).getReg())
    return std::nullopt;
  LC.Step = Update->getOperand(2).getImm();
  if (LC.Step == 0 || !isInt<16>(LC.Step))
    return std::nullopt;

  // The relation under which the loop goes on, with the counter on the left.
  if (ExitOnTrue)
    Mode = Mode == EQ ? NE : Mode == NE ? EQ : Mode == LT ? GE
         : Mode == LE ? GT : Mode == GT ? LE : LT;
  if (Swapped)
    Mode = Mode == LT ? GT : Mode == LE ? GE : Mode == GT ? LT
         : Mode == GE ? LE : Mode;
  switch (Mode) {
  case EQ:
    return std::nullopt;
  case NE:
    // Stepping by one, the counter meets the bound before passing it; and
    // stepping up by S from a start of at most 0 that S divides, to a
    // bound n & M where S divides M, as an unrolled loop counts.
    if (LC.Step != 1 && LC.Step != -1 &&
        !(LC.Step > 0 && startsAtMultipleUpTo0(LC, MRI) &&
          isMaskedMultiple(LC.Bound, LC.Step, MRI)))
      return std::nullopt;
    LC.Rel = LC.Step > 0 ? LoopCounter::LT : LoopCounter::GT;
    break;
  case LT: case LE:
    if (LC.Step < 0)
      return std::nullopt;
    LC.Rel = Mode == LT ? LoopCounter::LT : LoopCounter::LE;
    break;
  case GT: case GE:
    if (LC.Step > 0)
      return std::nullopt;
    LC.Rel = Mode == GT ? LoopCounter::GT : LoopCounter::GE;
    break;
  }

  // The range of the start: an immediate, or an and with an immediate mask.
  std::optional<int64_t> Lo, Hi;
  if (const MachineInstr *InitDef = MRI.getVRegDef(LC.Init)) {
    if (const std::optional<int64_t> I = movedImm(LC.Init, MRI)) {
      Lo = Hi = *I;
    } else if (InitDef->getOpcode() == TPC::ANDsip && isPlainScalarOp(*InitDef) &&
               InitDef->getOperand(2).isImm() &&
               InitDef->getOperand(2).getImm() >= 0) {
      Lo = 0;
      Hi = InitDef->getOperand(2).getImm();
    }
  }
  if (Lo && LC.Bound.isImm()) {
    // The counter the first iteration tests, and how far it is from the bound.
    const int64_t First = LC.CompOnUpdate ? LC.Step : 0;
    const int64_t B = LC.Bound.getImm();
    const int64_t Dist = LC.Step > 0 ? B - (*Lo + First) : (*Hi + First) - B;
    const int64_t Inclusive =
        LC.Rel == LoopCounter::LE || LC.Rel == LoopCounter::GE;
    const int64_t AbsStep = LC.Step > 0 ? LC.Step : -LC.Step;
    LC.MaxTripCount =
        Dist < 0 ? 1 : 1 + (Dist + Inclusive + AbsStep - 1) / AbsStep;
  }
  return LC;
}

std::unique_ptr<TargetInstrInfo::PipelinerLoopInfo>
TPCInstrInfo::analyzeLoopForPipelining(MachineBasicBlock *LoopBB) const {
  MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
  SmallVector<MachineOperand, 2> Cond;
  if (analyzeBranch(*LoopBB, TBB, FBB, Cond) || Cond.size() != 2)
    return nullptr;
  if (TBB == FBB || (TBB != LoopBB && FBB != LoopBB))
    return nullptr;
  if (TBB == LoopBB)
    reverseBranchCondition(Cond);

  MachineFunction &MF = *LoopBB->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  for (const MachineInstr &MI : *LoopBB)
    if (MI.isCall())
      return nullptr;
  // A loop the source asks the machine unroller to unroll is left to it.
  if (const BasicBlock *BB = LoopBB->getBasicBlock())
    if (const Instruction *T = BB->getTerminator())
      if (getUnrollCountFromMetadata(T->getMetadata(LLVMContext::MD_loop)))
        return nullptr;
  if (findInstrUnsafeToReorder(*LoopBB, MRI))
    return nullptr;
  if (!Cond[0].isReg() || !Cond[0].getReg().isVirtual())
    return nullptr;
  const Register Pred = Cond[0].getReg();
  MachineInstr *Comp = MRI.getVRegDef(Pred);
  if (!Comp || Comp->isPHI() || Comp->getParent() != LoopBB ||
      !MRI.hasOneNonDBGUse(Pred))
    return nullptr;

  MachineBasicBlock *Preheader = nullptr;
  for (MachineBasicBlock *P : LoopBB->predecessors())
    if (P != LoopBB)
      Preheader = Preheader ? nullptr : P;
  std::optional<LoopCounter> Counter;
  if (Preheader)
    Counter = analyzeCounter(*Comp, Cond[1].getImm() == 0, *LoopBB, MRI);

  // The dependence analysis takes a MachineLoop. The pipeliner may already
  // have changed other loops of the function, so the loops are computed anew.
  MachineDominatorTree MDT(MF);
  MachineLoopInfo MLI(MDT);
  const MachineLoop *L = MLI.getLoopFor(LoopBB);
  if (!L || L->getHeader() != LoopBB || L->getNumBlocks() != 1)
    return nullptr;
  const MLLinearIVsContainer IVs = findLinearIVs(*L, MRI);
  const MLDAReport Deps = runMachineLoopDataDependencyAnalysis(
      *L, MRI, MF.getSubtarget<TPCSubtarget>(), *this, IVs,
      /*IgnoreCheckAgainstStoresToSameTensorId=*/false);
  const bool Independent = Deps.HasLoopDependentMemStLd == false &&
                           Deps.HasLoopDependentMemStSt == false;
  LLVM_DEBUG(dbgs() << "Pipelining " << printMBBReference(*LoopBB)
                    << ": iterations "
                    << (Independent ? "independent" : "may depend")
                    << " in memory, counter "
                    << (Counter ? "understood" : "not understood") << "\n");

  // Whether the machine unroller would take the loop (as
  // TPCPipeliner::pipeline decides for a loop without the pragma): it counts
  // up with '<', each phi is a counter stepped by an immediate or an index
  // register set anew each iteration, no memory access may depend on another,
  // and unrolling saves enough.
  std::optional<AutoUnrollEstimate> Unroll;
  const bool NoMemDeps = Deps.HasLoopDependentMemStLd == false &&
                         Deps.HasLoopIndependentMemStLd == false &&
                         Deps.HasLoopDependentMemStSt == false &&
                         Deps.HasLoopIndependentMemStSt == false;
  auto IsSetEachIteration = [&](const MachineInstr &Phi) {
    const Register PhiReg = Phi.getOperand(0).getReg();
    Register Reg;
    for (unsigned I = 1; I + 1 < Phi.getNumOperands(); I += 2)
      if (Phi.getOperand(I + 1).getMBB() == LoopBB)
        Reg = Phi.getOperand(I).getReg();
    while (Reg && Reg != PhiReg) {
      const MachineInstr *MI = Reg.isVirtual() ? MRI.getVRegDef(Reg) : nullptr;
      if (!MI || MI->getParent() != LoopBB || !isSET_INDX(MI->getOpcode()) ||
          isPredicated(*MI) || !MI->getOperand(1).isReg())
        return false;
      Reg = MI->getOperand(1).getReg();
    }
    return Reg == PhiReg;
  };
  const bool PhisSupported = all_of(LoopBB->phis(), [&](const MachineInstr &Phi) {
    return any_of(IVs, [&](const MLoopLinearIV &IV) {
             return IV.isCustom() && &IV.getCurrDefMI() == &Phi &&
                    IV.getStepOp().isImm();
           }) ||
           IsSetEachIteration(Phi);
  });
  const AutoUnrollEstimate E =
      estimateAutoUnroll(*LoopBB, MRI, *this, getRegisterInfo());
  if (NoMemDeps && PhisSupported && Counter && Counter->Rel == LoopCounter::LT &&
      Counter->Step > 0 && E.Worthwhile)
    Unroll = E;

  // Spread the set_indx instructions, which the load, scalar and store slots
  // all issue, over those slots, as the scheduler does later when the slot
  // it has is taken, so that the pipeliner's resource bound is not that of
  // the load slot alone. (The MVE expander analyzes the loop again; there is
  // nothing left to move then.)
  SmallVector<std::pair<MachineInstr *, unsigned>, 8> Reslotted;
  if (BalanceSlots) {
    enum { Ld, Spu, St, Vpu, None };
    auto SlotOf = [](const MCInstrDesc &D) {
      return TPCII::isLoadInst(D)    ? Ld
             : TPCII::isSPUInst(D)   ? Spu
             : TPCII::isStoreInst(D) ? St
             : TPCII::isVPUInst(D)   ? Vpu
                                     : None;
    };
    unsigned Count[None + 1] = {};
    SmallVector<MachineInstr *, 8> SetIndx;
    for (MachineInstr &MI : *LoopBB) {
      if (MI.isPHI() || MI.isDebugInstr() || MI.isPseudo())
        continue;
      ++Count[SlotOf(MI.getDesc())];
      if (isSET_INDX(MI.getOpcode()))
        SetIndx.push_back(&MI);
    }
    for (MachineInstr *MI : SetIndx) {
      std::vector<unsigned> Alts;
      if (!getOtherSlotOpcodes(MI, Alts))
        continue;
      const unsigned Orig = MI->getOpcode();
      unsigned Best = Orig;
      int BestSlot = SlotOf(MI->getDesc());
      --Count[BestSlot];
      for (unsigned Alt : Alts) {
        const int S = SlotOf(get(Alt));
        if (S != Vpu && S != None && Count[S] < Count[BestSlot]) {
          Best = Alt;
          BestSlot = S;
        }
      }
      ++Count[BestSlot];
      if (Best != Orig) {
        MI->setDesc(get(Best));
        Reslotted.push_back({MI, Orig});
      }
    }
  }

  return std::make_unique<TPCPipelinerLoopInfo>(*this, MRI, Comp, Cond,
                                                Independent, Counter,
                                                Preheader, Unroll, E.Before,
                                                Reslotted);
}

namespace {

/// The pipeliner's dependence graph takes the latency of a value from its
/// producer to the instruction that reads it. Two kinds of reads hide the
/// producer: a copy-like pseudo (REG_SEQUENCE, COPY), which costs nothing and
/// passes the value on, and a phi, through which an iteration reads a value
/// of the previous one: the pipeliner gives the producer -> phi edge latency
/// 1 and the phi -> use edge latency 0, whatever the producer. A load read
/// through either was taken to be ready after a cycle or two instead of 4,
/// and the latency resolver then filled the kernel with NOPs. This mutation
/// gives such edges the producer's latency to the real reader.
class TPCPipelinerLatencyMutation : public ScheduleDAGMutation {
  ScheduleDAGInstrs *DAG = nullptr;

  static bool isCopyLike(const SUnit *SU) {
    if (!SU->isInstr())
      return false;
    const MachineInstr *MI = SU->getInstr();
    return MI->isRegSequence() || MI->isCopy() || MI->isInsertSubreg() ||
           MI->isSubregToReg();
  }

  /// The instructions that read SU's value, through copy-like pseudos, with
  /// the operand they read it in.
  void realReaders(SUnit *SU, SmallVectorImpl<std::pair<SUnit *, int>> &Out,
                   unsigned Depth = 0) const {
    for (const SDep &S : SU->Succs) {
      if (S.getKind() != SDep::Data || !S.getSUnit()->isInstr())
        continue;
      SUnit *U = S.getSUnit();
      if (U->getInstr()->isPHI())
        continue;
      if (isCopyLike(U) && Depth < 4) {
        realReaders(U, Out, Depth + 1);
        continue;
      }
      const MachineInstr *UMI = U->getInstr();
      for (unsigned I = 0, E = UMI->getNumOperands(); I < E; ++I) {
        const MachineOperand &MO = UMI->getOperand(I);
        if (MO.isReg() && MO.isUse() && MO.getReg() == S.getReg()) {
          Out.push_back({U, int(I)});
          break;
        }
      }
    }
  }

  /// The instructions whose values SU passes on (SU itself when it is not
  /// copy-like).
  void realProducers(SUnit *SU, SmallVectorImpl<SUnit *> &Out,
                     unsigned Depth = 0) const {
    if (!isCopyLike(SU) || Depth >= 4) {
      Out.push_back(SU);
      return;
    }
    for (const SDep &P : SU->Preds)
      if (P.getKind() == SDep::Data && P.getSUnit()->isInstr() &&
          !P.getSUnit()->getInstr()->isPHI())
        realProducers(P.getSUnit(), Out, Depth + 1);
  }

  unsigned latency(SUnit *Def, SUnit *Use, int UseIdx) const {
    const MachineInstr *DefMI = Def->getInstr();
    int DefIdx = -1;
    for (unsigned I = 0, E = DefMI->getNumOperands(); I < E; ++I)
      if (DefMI->getOperand(I).isReg() && DefMI->getOperand(I).isDef()) {
        DefIdx = I;
        break;
      }
    if (DefIdx < 0)
      return 0;
    return DAG->getSchedModel()->computeOperandLatency(DefMI, DefIdx,
                                                       Use->getInstr(), UseIdx);
  }

  static void setLatency(SUnit *Src, SUnit *Dst, unsigned Lat) {
    for (SDep &S : Src->Succs)
      if (S.getSUnit() == Dst && S.getKind() == SDep::Data)
        S.setLatency(Lat);
    for (SDep &P : Dst->Preds)
      if (P.getSUnit() == Src && P.getKind() == SDep::Data)
        P.setLatency(Lat);
    Src->setHeightDirty();
    Dst->setDepthDirty();
  }

  static unsigned edgeLatency(const SUnit *Src, const SUnit *Dst) {
    for (const SDep &S : Src->Succs)
      if (S.getSUnit() == Dst && S.getKind() == SDep::Data)
        return S.getLatency();
    return 0;
  }

public:
  void apply(ScheduleDAGInstrs *D) override {
    DAG = D;
    const MachineRegisterInfo &MRI = DAG->MRI;
    const MachineBasicBlock *BB =
        DAG->SUnits.empty() ? nullptr : DAG->SUnits.front().getInstr()->getParent();

    // Edges into copy-like pseudos: the producer's latency to the pseudo's
    // real readers; out of them: none.
    for (SUnit &SU : DAG->SUnits) {
      if (!isCopyLike(&SU))
        continue;
      SmallVector<std::pair<SUnit *, int>, 4> Readers;
      realReaders(&SU, Readers);
      if (Readers.empty())
        continue;
      SmallVector<SUnit *, 4> Preds;
      for (const SDep &P : SU.Preds)
        if (P.getKind() == SDep::Data && P.getSUnit()->isInstr() &&
            !P.getSUnit()->getInstr()->isPHI() && !isCopyLike(P.getSUnit()))
          Preds.push_back(P.getSUnit());
      for (SUnit *P : Preds) {
        unsigned Lat = 0;
        for (auto [U, Idx] : Readers)
          Lat = std::max(Lat, latency(P, U, Idx));
        setLatency(P, &SU, Lat);
      }
      for (const SDep &S : SmallVector<SDep, 4>(SU.Succs))
        if (S.getKind() == SDep::Data && S.getSUnit()->isInstr() &&
            !S.getSUnit()->getInstr()->isPHI())
          setLatency(&SU, S.getSUnit(), 0);
    }

    // The loop counter's phi: the counter becomes the hardware loop's
    // iterator, which the LOOP steps, so its recurrence costs nothing.
    const MachineInstr *CounterPhi = nullptr;
    if (BB && BB->getFirstTerminator() != BB->end()) {
      const MachineInstr &Br = *BB->getFirstTerminator();
      if (Br.getOpcode() == TPC::JMPR && Br.getOperand(1).isReg() &&
          Br.getOperand(1).getReg().isVirtual())
        if (const MachineInstr *Cmp = MRI.getVRegDef(Br.getOperand(1).getReg()))
          for (const MachineOperand &MO : Cmp->uses()) {
            if (!MO.isReg() || !MO.getReg().isVirtual())
              continue;
            const MachineInstr *D = MRI.getVRegDef(MO.getReg());
            if (D && D->getParent() == BB && !D->isPHI() &&
                D->getOpcode() == TPC::ADDsip && D->getOperand(1).isReg())
              D = MRI.getVRegDef(D->getOperand(1).getReg());
            if (D && D->getParent() == BB && D->isPHI())
              CounterPhi = D;
          }
    }

    // Edges out of phis: the latency from the producer of the value the phi
    // takes from the loop, less what the producer -> phi path already has.
    for (SUnit &Phi : DAG->SUnits) {
      if (!Phi.isInstr() || !Phi.getInstr()->isPHI() ||
          Phi.getInstr() == CounterPhi)
        continue;
      const MachineInstr &PhiMI = *Phi.getInstr();
      Register LoopVal;
      for (unsigned I = 1, E = PhiMI.getNumOperands(); I + 1 < E; I += 2)
        if (PhiMI.getOperand(I + 1).getMBB() == BB)
          LoopVal = PhiMI.getOperand(I).getReg();
      if (!LoopVal || !LoopVal.isVirtual())
        continue;
      const MachineInstr *DefMI = MRI.getVRegDef(LoopVal);
      SUnit *Def = DefMI ? DAG->getSUnit(const_cast<MachineInstr *>(DefMI))
                         : nullptr;
      if (!Def || DefMI->isPHI())
        continue;
      SmallVector<SUnit *, 4> Producers;
      realProducers(Def, Producers);
      SmallVector<std::pair<SUnit *, int>, 4> Readers;
      realReaders(&Phi, Readers);
      for (const SDep &S : SmallVector<SDep, 4>(Phi.Succs)) {
        if (S.getKind() != SDep::Data || !S.getSUnit()->isInstr() ||
            S.getSUnit()->getInstr()->isPHI())
          continue;
        SUnit *X = S.getSUnit();
        SmallVector<std::pair<SUnit *, int>, 4> XReaders;
        if (isCopyLike(X)) {
          realReaders(X, XReaders);
        } else {
          for (auto &R : Readers)
            if (R.first == X)
              XReaders.push_back(R);
        }
        unsigned Lat = 0;
        for (SUnit *P : Producers) {
          // Producer -> (pseudo) -> phi: the pipeliner adds 1 for the edge
          // into the phi.
          const unsigned Path = 1 + (P == Def ? 0 : edgeLatency(P, Def));
          for (auto [U, Idx] : XReaders) {
            const unsigned L = latency(P, U, Idx);
            Lat = std::max(Lat, L > Path ? L - Path : 0);
          }
        }
        setLatency(&Phi, X, Lat);
      }
    }
  }
};

} // namespace

void TPCSubtarget::getSMSMutations(
    std::vector<std::unique_ptr<ScheduleDAGMutation>> &Mutations) const {
  Mutations.push_back(std::make_unique<TPCPipelinerLatencyMutation>());
}
