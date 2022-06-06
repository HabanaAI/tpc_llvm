//===---- TPCSoftwareLoopPipelining.cpp - Pipeline loops -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass pipilenes innermost loops
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCLoopData.h"
#include "TPCMachineInstrTools.h"
#include "TPCMachineLoopDependencyAnalysis.h"
#include "TPCMachineLoopLinearIV.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include <cassert>
#include <cstdlib>
#include <map>
#include <set>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "pipelining"

static cl::opt<bool>
    EnablePipelining("pipeline-loops", cl::Hidden,
                     cl::desc("Enable software pipelining"), cl::init(true));

namespace llvm {

  FunctionPass *createTPCPipeliner();
  void initializeTPCPipelinerPass(PassRegistry&);

} // end namespace llvm

typedef std::map<unsigned, std::vector<unsigned>> AccumChains;
typedef std::map<unsigned, unsigned> RegMap;

struct ExThreadParams {
  RegMap VRegMap;
  std::vector<MachineInstr*> ThreadInstrs;
  std::vector<MachineInstr*> Phis;
  std::vector<MachineInstr*> CounterInstrs;

  std::vector<MachineInstr*> Dprolog;
  std::vector<MachineInstr*> Dphi;
  std::vector<MachineInstr*> Dloop;
  std::vector<MachineInstr*> Dshift;
  std::vector<MachineInstr*> Epilog;
};

/// Return collection of machine instruction which uses the given register.
///
/// \details Function is useful when MRI.use_instructions is not applicable.
///     For example, machine operand or machine instruction manipulations might
///     invalidate iterator over MRI.use_instructions range:
///
/// \code
///     for (MachineInstruction &MI : MRI.use_instructions(Reg))
///         MI.getOperand(3).setReg(..)  // Might invalidate iterator!
///
///     // Prefer:
///     for (MachineInstruction &MI : collectUses(Reg, MRI))
///         MI.getOperand(3).setReg(..)  // Safe
SmallVector<MachineInstr *, 8> collectUses(const Register Reg,
                                           const MachineRegisterInfo &MRI) {
  SmallVector<MachineInstr *, 8> Uses;
  for (MachineInstr &MI : MRI.use_instructions(Reg))
    Uses.push_back(&MI);
  return Uses;
}

/// Shortcut to change operand TargetOp with ResultOp reg/imm content having
/// other operand flags left untouched.
static void updateRegOrImmOperand(MachineOperand &TargetOp,
                                  const MachineOperand &ResultOp) {
  assert(TargetOp.isImm() || TargetOp.isReg());
  assert(ResultOp.isImm() || ResultOp.isReg());
  assert(!TargetOp.isReg() || !TargetOp.isDef());

  if (TargetOp.isImm() && ResultOp.isImm()) {
    TargetOp.setImm(ResultOp.getImm());
    return;
  }

  if (TargetOp.isReg() && ResultOp.isReg()) {
    TargetOp.setReg(ResultOp.getReg());
    return;
  }

  if (TargetOp.isImm()) {
    TargetOp.ChangeToRegister(ResultOp.getReg(), false);
    return;
  }

  TargetOp.ChangeToImmediate(ResultOp.getImm());
}

/// Replace registers of all uses operands for the given machine instruction
/// from OldReg to NewReg.
///
/// \note Use registers which are actually definitions (see LOOP instruction for
///     example) are not replaced.
static void replaceUses(MachineInstr &MI, const Register OldReg,
                        const Register NewReg) {
  for (MachineOperand &MO : MI.uses()) {
    if (MO.isReg() && MO.getReg() == OldReg)
      MO.setReg(NewReg);
  }
}

/// Replace register of all uses operands for all machine instructions of the
/// given machine basic block with the new register.
///
/// \note Use registers which are actually definitions (see LOOP instruction for
///     example) are not replaced.
///
/// \param MBB machine basic block which instructions should be updated.
/// \param OldReg registry number to be replaced.
/// \param NewReg registry number to be substituted.
/// \param ProcessPhi whether to process phi instructions.
static void replaceUsesInBB(MachineBasicBlock &MBB, const Register OldReg,
                            const Register NewReg, const bool ProcessPhi) {
  for (MachineInstr &MI : MBB) {
    if (ProcessPhi || !MI.isPHI())
      replaceUses(MI, OldReg, NewReg);
  }
}

// Replace use register operands according to mapping for register ids in the
// given machine instruction inplace.
static void replaceUseRegistersByMapping(MachineInstr &MI, const RegMap &Map) {
  for (MachineOperand& MO : MI.uses()) {
    if (!MO.isReg())
      continue;

    auto It = Map.find(MO.getReg());
    if (It != Map.end())
      MO.setReg(It->second);
  }
}

/// Build mapping: next IV definition -> index of step operand in this
/// instruction.
static DenseMap<MachineInstr *, int>
makeLinearIVNextValDefToStepOpIxMapping(const MachineLoop &ML,
                                        const MachineRegisterInfo &MRI,
                                        const MLLinearIVsContainer &LinearIVs) {
  DenseMap<MachineInstr *, int> RV;
  for (const MLoopLinearIV &Info : LinearIVs) {
    // HWCounter is not interesting for our case.
    if (Info.isHWCounter())
      continue;

    // Only immediate steps are supported.
    if (!Info.getStepOp().isImm())
      continue;

    RV[&Info.getNextDefMI()] = Info.getStepOpIx();
  }

  return RV;
}

class TPCPipeliner  : public MachineFunctionPass {
  MachineLoopInfo            *MLI;
  MachineRegisterInfo        *MRI;
  MachineDominatorTree       *MDT; //Needed?
  const TPCInstrInfo         *TII;
  const TargetRegisterInfo   *TRI;

  MachineFunction *CurrMF;

  std::vector<ExThreadParams> ExecThreads;
  AccumChains AccumulateMap;
  DenseMap<MachineInstr *, int> LinearIVs;

  // For a given phi-def store it's values for (UnrollCount - 1) iterations
  // in case phi executions paths depend on each other
  std::map<unsigned, std::vector<unsigned> > AdjustedPhis;
  std::vector<MachineInstr*> DelayedPhis;

  std::vector<unsigned> InitialCounters;
  std::vector<unsigned> EndCounters;
  std::vector<unsigned> NextCounters;
  std::vector<unsigned> LeftCounters;

  MLLinearIVsContainer MLLinearIVs;

  MachineBasicBlock* Header;
  MachineBasicBlock* Latch;
  MachineBasicBlock* Exit;
  MachineBasicBlock* Prolog;

  MachineLoop* CurLoop;
  MachineInstr* LoopInst;
  unsigned LoopCounter;

  bool Ascending; // Indicates whether the counter is ascending or descending
  bool Inclusive; // Indicates whether the boundary in inclusive or exclusive

  // Auxiliary iterator for tracking the epilog of the pipeliened loop
  // just so we don't need to create different blocks for accumulators and epilog
  MachineBasicBlock::iterator EpilogInserter;
  MachineBasicBlock::iterator PrologInserter;

public:
  static char ID;
  TPCPipeliner() : MachineFunctionPass(ID), MLI(nullptr), MRI(nullptr), MDT(nullptr),
                   TII(nullptr), TRI(nullptr), CurrMF(nullptr),
                   Header(nullptr), Latch(nullptr), Exit(nullptr), Prolog(nullptr),
                   CurLoop(nullptr), LoopInst(nullptr), LoopCounter(-1),
                   Ascending(false), Inclusive(false) {
    initializeTPCPipelinerPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "TPC Software Loop Pipelining"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  bool pipeline(MachineLoop* L);
  bool unrollAndAlign(unsigned UnrollCount, bool DoPipelining);
  void fixUnrolledLoopBoundary(unsigned UnrollCount);
  bool findInnermostLoop(MachineLoop* L);
  void recalculateAnalysis();
  void replaceVregs(MachineInstr* MI, unsigned ThreadNum);
  void decoupleIfPossible(MachineInstr* MI, unsigned ThreadNum);
  bool align(const std::vector<unsigned> &Shifts);
  void createPrologCounters(unsigned UnrollCount, MachineBasicBlock* PrologBlock);
  void createEpilogCounters(int Counter, MachineBasicBlock* PrologBlock);
  void createNextCounters(int Counter, MachineBasicBlock* PrologBlock);
  bool isPhiDef(unsigned Reg, unsigned Thread);
  void modifyBoundary(unsigned Decrement);
  bool calcLoopForm();
  bool isCyclicDependency(MachineInstr* Phi, MachineInstr* MovedInst, MachineOperand& Use);
  std::vector<MachineInstr*> phiPath(MachineInstr *Phi, MachineOperand &HeadPhiUse);
  void patchPhiStartValue(MachineInstr &Phi, int UnrollCount);
  void replaceWithZero(MachineOperand& ReplaceMO, TPCII::OpType ot);
  void setPrologInserter();
  void sortDelayedPhis();
  bool feedsPhi(MachineInstr* MI);
  void correctCounters(unsigned UnrollCount);
  void correctIVs(int Counter);
  void correctExit();
  bool checkForProhibitingInstructions();

  /// Add epilogue loop after the source loop to be unrolled.
  ///
  /// Source loop params should be changed so that its iterations count fits for
  /// unrolling. Method must be called before source loop unrolling because
  /// it prepares source loop for unrolling.
  void generateEpilogueLoop(unsigned UnrollCount, bool HasLoopTakenGuarantee);

  void addDoubleRegs(MachineBasicBlock* Accum, unsigned Res, unsigned Op1,
                     unsigned Op2, unsigned AddOpc, TPCII::OpType Type,
                     const TargetRegisterClass* RC);
  void addQuadroRegs(MachineBasicBlock* Accum, unsigned Res, unsigned Op1,
                     unsigned Op2, unsigned AddOpc, TPCII::OpType Type,
                     const TargetRegisterClass* RC);
};

INITIALIZE_PASS_BEGIN(TPCPipeliner, "pipelining",
                      "TPC Software Loop Pipelining", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCPipeliner, "pipelining",
                    "TPC Software Loop Pipelining", false, false)

FunctionPass *llvm::createTPCPipeliner() {
  return new TPCPipeliner();
}

char TPCPipeliner::ID = 0;

bool TPCPipeliner::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  if (!EnablePipelining) {
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "********* TPC Software Loop Pipelining *********\n";
    MF.dump();
  });

  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  MDT = &getAnalysis<MachineDominatorTree>();
  TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  TRI = MF.getSubtarget<TPCSubtarget>().getRegisterInfo();
  CurrMF = &MF;

  bool Changed = false;
  for (auto &L : *MLI) {
    const bool Unrolled = findInnermostLoop(L);

    if (Unrolled) {
      Changed = true;
      recalculateAnalysis();
    }
  }

  LLVM_DEBUG({
    if (Changed) {
      dbgs() << "MF at the end of pipelining pass\n";
      MF.dump();
    }
  });

  return Changed;
}

static bool isLoop(const MachineInstr &MI) {
  return TPCII::isLoopInst(MI.getDesc()) && MI.getOpcode() != TPC::LOOPEND;
}

static unsigned getAddOpc(TPCII::OpType Type, const TargetRegisterClass* RC) {
  if (TPC::SRFRegClass.hasSubClassEq(RC))
    return TPC::ADDssp;
  if (TPC::VRFRegClass.hasSubClassEq(RC))
    return TPC::ADDvvp;
  if(TPC::ARFRegClass.hasSubClassEq(RC))
    return TPC::ADDvvp;
  if(TPC::DRFRegClass.hasSubClassEq(RC))
    return TPC::ADDvvp;
  if(TPC::ZRFRegClass.hasSubClassEq(RC))
    return TPC::ADDssp;
  if(TPC::IRFRegClass.hasSubClassEq(RC)) {
    assert(false && "No accumulators for IRFs");
  } else {
    llvm_unreachable("Unimplemented reg class as accumulator");
  }
  return 0;
}

bool TPCPipeliner::findInnermostLoop(MachineLoop* L) {
  if (L->begin() == L->end()) {
    return pipeline(L);
  }

  bool Changed = false;
  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    Changed |= findInnermostLoop(*I);
  }
  return Changed;
}

static bool isAccumulator(MachineInstr* MI) {
  if (TPCII::getSlotOpCode(MI->getDesc()) == TPCII::vpuMAC/* ||
      TPCII::getSlotOpCode(MI->getDesc()) == TPCII::spuvpuADD*/) {
    return true;
  }
  return false;
}

bool TPCPipeliner::feedsPhi(MachineInstr *MI) {
  for (MachineOperand& MO : MI->defs()) {
    for (MachineInstr &UseMI : MRI->use_instructions(MO.getReg())) {
      if (UseMI.isPHI() && CurLoop->contains(&UseMI)) {
        return true;
      }
    }
  }

  return false;
}

void TPCPipeliner::recalculateAnalysis() {
  MDT->calculate(*CurrMF);
}

// For effective pipelining we need not to just unroll, but make
// execution threads independent. In order to do that we need to create new
// virtual registers for cloned instructions. But we can't do it
// for everything. If the def of an instructions is used after loop
// then decoupling this instruction creates data that's going to be lost.
// In that case we need to gather results from all threads of execution
// in one register. For now we can only do that with addition, i.e.
// if instruction is accumulator.
void TPCPipeliner::decoupleIfPossible(MachineInstr* MI, unsigned ThreadNum) {
  bool SurvivesLoop = false;
  bool InLoopUse = false;
  for (MachineOperand& MO : MI->defs()) {
    for (MachineInstr &UseMI : MRI->use_instructions(MO.getReg())) {
      //UseMI.dump();
      if (!CurLoop->contains(&UseMI)) {
        SurvivesLoop = true;
        break;
      } else if (!MI->isPHI()) {
        InLoopUse = true;
      }
      if (SurvivesLoop) break;
    }
  }

  // For phi we need also to patch all it's uses
  if (MI->isPHI()) {
    //MI->dump();
    assert(!SurvivesLoop && "Phi should not be used outside a loop");

    for (MachineOperand& MO : MI->defs()) {
      unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
      ExecThreads[ThreadNum].VRegMap[MO.getReg()] = v_reg;

      for (MachineInstr *ThreadMI : ExecThreads[ThreadNum].ThreadInstrs)
        replaceUses(*ThreadMI, MO.getReg(), v_reg);

      MO.setReg(v_reg);
    }
    return;
  }

  if (!SurvivesLoop || LinearIVs.count(MI) > 0) {
    for (MachineOperand& MO : MI->defs()) {
      if (MO.getReg().isPhysical()) {
        continue;
      }
      unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
      ExecThreads[ThreadNum].VRegMap[MO.getReg()] = v_reg;
      MO.setReg(v_reg);
    }
  } else {
    // TODO: there must be more complex analysis, we need to check
    // how this surviving value uses values of the loop
    if (isAccumulator(MI) && feedsPhi(MI) && !InLoopUse) {
      for (MachineOperand& MO : MI->defs()) {
        unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
        ExecThreads[ThreadNum].VRegMap[MO.getReg()] = v_reg;
        AccumulateMap[MO.getReg()].push_back(v_reg);
        MO.setReg(v_reg);
      }
    } else {
      // If it's not an accumulator, then the value of the last iteration
      // should be used. Patch every use outside of the loop to the
      // value of the last execution thread
      for (MachineOperand& MO : MI->defs()) {

        if (MO.getReg().isPhysical())
          continue;

        unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
        unsigned OrigReg = MO.getReg();
        ExecThreads[ThreadNum].VRegMap[OrigReg] = v_reg;
        MO.setReg(v_reg);

        if (ThreadNum == ExecThreads.size() - 1) {
          for (MachineOperand &RSUse: MRI->use_operands(OrigReg)) {
            MachineInstr *RSUseMI = RSUse.getParent();
            if (!CurLoop->contains(RSUseMI))
              replaceUses(*RSUseMI, OrigReg, v_reg);
          }
        }
      }
    }
  }
}

void TPCPipeliner::replaceVregs(MachineInstr* MI, unsigned ThreadNum) {
  replaceUseRegistersByMapping(*MI, ExecThreads[ThreadNum].VRegMap);

  for (MachineOperand& MO : MI->uses()) {
    if (!MO.isReg()) continue;
    // Tied register that was not defined in the current execution thread
    if (MO.isTied() && ExecThreads[ThreadNum].VRegMap.count(MO.getReg()) == 0) {
      unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
      MachineInstr* TiedDef = MRI->getVRegDef(MO.getReg());
      MachineBasicBlock::iterator it = ++MachineBasicBlock::iterator(TiedDef);
      if (!TiedDef->isPHI()) {
        BuildMI(*TiedDef->getParent(), it, DebugLoc(), TII->get(TargetOpcode::COPY), v_reg)
                .addReg(MO.getReg());
        MO.setReg(v_reg);
      }
    }
  }

  decoupleIfPossible(MI, ThreadNum);
}

static bool getPipelineFromMetadata(const MDNode* LoopMD) {
  // First operand should refer to the loop id itself.
  assert(LoopMD->getNumOperands() > 0 && "requires at least one operand");
//  assert(LoopMD->getOperand(0) == LoopMD && "invalid loop id");

  MDNode *MD = nullptr;

  for (unsigned i = 1, e = LoopMD->getNumOperands(); i < e; ++i) {
    MD = dyn_cast<MDNode>(LoopMD->getOperand(i));
    if (!MD)
      continue;

    MDString *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    if (S->getString().equals("llvm.loop.pipelined")) {
      assert(MD->getNumOperands() == 2 &&
             "Pipeline hint metadata should have two operands.");
      bool DoPipelining =
          !(mdconst::extract<ConstantInt>(MD->getOperand(1))->isZero());
      return DoPipelining;
    }
  }

  return 0;
}

/// Return iterations count for loops like:
///   for (int i = Start; i Cmp Bound; i += Step)
///
/// if Start, Bound and Step are immediate values.
static llvm::Optional<uint64_t> getIterationsCount(const int64_t Start,
                                                   const int64_t Bound,
                                                   const int64_t Step,
                                                   const int64_t Cmp) {
  assert(Cmp == TPCII::LoopLT || Cmp == TPCII::LoopLE || Cmp == TPCII::LoopEQ ||
         Cmp == TPCII::LoopNE || Cmp == TPCII::LoopGE || Cmp == TPCII::LoopGT);

  // Process Start == Bound boundary case here in order not to ignore it below
  if (Start == Bound) {
    if (Cmp == TPCII::LoopLT || Cmp == TPCII::LoopNE || Cmp == TPCII::LoopGT)
      return 0;  // loop is never taken

    if (Step == 0)
      return llvm::None;  // infinite loop

    const int CmpToTake = Step < 0 ? TPCII::LoopGE : TPCII::LoopLE;
    return Cmp == TPCII::LoopEQ || Cmp == CmpToTake
        ? llvm::Optional<uint64_t>(1) // loop is taken once
        : llvm::Optional<uint64_t>(); // infinite loop
  }

  // Process Step == 0 boundary case and cases when Step sign does not
  // correspond to relation between Start and Bound
  if (Step == 0 || (Step < 0 && Start < Bound) || (Step > 0 && Bound < Start)) {
    const int CmpToHang1 = Start < Bound ? TPCII::LoopLT : TPCII::LoopGT;
    const int CmpToHang2 = Start < Bound ? TPCII::LoopLE : TPCII::LoopGE;
    return Cmp == TPCII::LoopNE || Cmp == CmpToHang1 || Cmp == CmpToHang2
        ? llvm::Optional<uint64_t>()  // infinite loop
        : llvm::Optional<uint64_t>(0); // loop is never taken
  }

  assert((Step < 0 && Bound < Start) || (Step > 0 && Start < Bound));

  // Check against never taken loops
  const int CmpToSkip1 = Start < Bound ? TPCII::LoopGT : TPCII::LoopLT;
  const int CmpToSkip2 = Start < Bound ? TPCII::LoopGE : TPCII::LoopGT;
  if (Cmp == TPCII::LoopEQ || Cmp == CmpToSkip1 || Cmp == CmpToSkip2)
    return 0;

  // Bound and Start must fit I32 type according to TPC architecture.
  assert(isIntN(32, Bound));  // otherwise, Bound - Start might overflow I64
  assert(isIntN(32, Start));  // otherwise, Bound - Start might overflow I64
  const uint64_t Distance = Start < Bound ? Bound - Start : Start - Bound;

  const uint64_t AbsStep = Step > 0 ? Step : -Step;

  // Special case of loop with != compare
  if (Cmp == TPCII::LoopNE)
    return Distance % AbsStep == 0
        ? llvm::Optional<uint64_t>(Distance / AbsStep)
        : llvm::Optional<uint64_t>();  // infinite loop

  // Finally process regular ranges
  assert((Step < 0 && (Cmp == TPCII::LoopGT || Cmp == TPCII::LoopGE)) ||
         (Step > 0 && (Cmp == TPCII::LoopLT || Cmp == TPCII::LoopLE)));
  const int CmpToHitExtraIteration = Step < 0
      ? Cmp == TPCII::LoopGE
      : Cmp == TPCII::LoopLE;
  return Distance / AbsStep +
         (Cmp == CmpToHitExtraIteration && Distance % AbsStep == 0 ? 1 : 0);
}

/// Return whether iterations count for LOOP instruction might be calculated at
/// compile time.
static bool isIterationsCountKnownAtCompileTime(const MachineInstr &LoopInstr) {
  assert(isLoop(LoopInstr));
  const MachineOperand &Start = LoopInstr.getOperand(0);
  const MachineOperand &Bound = LoopInstr.getOperand(1);
  const MachineOperand &Step = LoopInstr.getOperand(2);
  const MachineOperand &Comp = LoopInstr.getOperand(3);
  return Start.isImm() && Bound.isImm() && Step.isImm() && Comp.isImm();
}

/// Return iterations count for LOOP instruction if it is known at compile time.
/// Might return llvm::None if START, STEP, BOUNDARY are not known at compile
/// time or loop is infinite.
static llvm::Optional<uint64_t>
getCompileTimeIterationsCount(const MachineInstr &LoopInstr) {
  assert(isLoop(LoopInstr));
  if (!isIterationsCountKnownAtCompileTime(LoopInstr))
    return llvm::None;

  const MachineOperand &Start = LoopInstr.getOperand(0);
  const MachineOperand &Bound = LoopInstr.getOperand(1);
  const MachineOperand &Step = LoopInstr.getOperand(2);
  const MachineOperand &Comp = LoopInstr.getOperand(3);
  return getIterationsCount(Start.getImm(), Bound.getImm(), Step.getImm(),
                            Comp.getImm());
}

/// Shortcut to replace phi instruction inputs using input pair references.
static void setPhiIPInternals(const PhiIP &IP, const Register Reg,
                              MachineBasicBlock &MBB) {
  IP.MBBOp->setMBB(&MBB);
  updateRegOrImmOperand(*IP.ValOp, MachineOperand::CreateReg(Reg, false));
}

/// Shortcut to replace phi instruction inputs using input pair references.
static void setPhiIPInternals(const PhiIP &IP, const RegMap &RegMapping,
                              MachineBasicBlock &MBB) {
  IP.MBBOp->setMBB(&MBB);

  // Nothing to do if ValOp is Imm/FpImm value. Process register case.
  if (!IP.ValOp->isReg())
    return;

  auto It = RegMapping.find(IP.ValOp->getReg());
  if (It == RegMapping.end())
    return;

  IP.ValOp->setReg(It->second);
}

void TPCPipeliner::generateEpilogueLoop(const unsigned UnrollCount,
                                        const bool HasLoopTakenGuarantee) {
  assert(Prolog);
  assert(Prolog->getParent());
  assert(Latch);
  assert(LoopInst);
  assert(Exit);
  assert(MDT && MRI && TII);
  // Previous passes must guarantee the correct layout.
  assert(Prolog->isSuccessor(Exit));
  // Code below splits source loop MIR into two loops. The first one has
  // iterations count divisible to UnrollCount. The second one, the epilogue
  // loop, is for the rest iterations.
  //
  // Source MIR:
  //
  //    BB.prolog:
  //      LOOP ... %BB.exit  ; <- initial iterations count
  //
  //    BB.loop:
  //      LOOPEND %BB.exit, %BB.loop
  //
  //    BB.exit:
  //      ...
  //
  // Should be transformed to:
  //
  //    BB.prolog:
  //      LOOP ... %BB.epilogue_pre_header  ; <- iterations count divisible to
  //                                        ; UnrollCount
  //
  //    BB.loop:
  //      LOOPEND %BB.epilogue_pre_header, %BB.loop
  //
  //    BB.epilogue_pre_header:
  //      LOOP ... %BB.exit  ; <- the rest iterations
  //
  //    BB.epilogue_loop:
  //      LOOPEND ... %BB.exit, %BB.epilogue_loop
  //
  //    BB.exit:
  //      ...

  // Note that operands shortcuts from LoopInst are organized as lambdas because
  // LoopInst might be replaced with another one and deleted during this
  // function, and lambdas allow avoiding dangling references bugs.
  auto StartOp = [&]() -> MachineOperand & { return LoopInst->getOperand(0); };
  auto BoundOp = [&]() -> MachineOperand & { return LoopInst->getOperand(1); };
  auto StepOp = [&]() -> MachineOperand & { return LoopInst->getOperand(2); };
  auto CmpOp = [&]() -> MachineOperand & { return LoopInst->getOperand(3); };
  assert(StepOp().isImm()); // Precondition.

  // Source loop bound may be changed during epilogue loop generation. Save the
  // initial value for future use.
  const MachineOperand SavedSourceBoundOp = BoundOp();

  const ArrayRef<MachineBasicBlock *> SourceBBs = CurLoop->getBlocks();

  // Add instructions to Prolog to calculate iterations count (immediate or run
  // time) for the source loop. If value is known at compile time, its
  // calculation at run time should be omitted.
  //
  // It should be like (simplified):
  //
  //    BB.prolog:
  //      MinDistance = Bound - Start
  //      UnrolledStep = std::abs(Step * UnrollCount)  ; immediate
  //      MinRestDistance = MinDistance % UnrolledStep;
  //      UnrolledDistance = MinDistance - MinRestDistance;
  //      UnrolledBoundary = Start + UnrolledDistance
  //
  // Note than depending on CmpOp() actual distance absolute value might be
  // greater than std::abs(MinDistance) on 1. For example:
  //    for (int i = 0; i <= 100; ++i)
  //      ...
  //    // MinDistance == 100
  //    // Actual distance == 101

  const MachineOperand MinDistanceOp =
      evalSubI32(*Prolog, --Prolog->end(), BoundOp(), StartOp(), *MRI, *TII);

  assert(isIntN(32, StepOp().getImm() * UnrollCount));  // Precondition.
  const std::int32_t UnrolledStep = std::abs(StepOp().getImm() * UnrollCount);

  // If there is guarantee that loop is taken and step > 0, it means that Bound
  // >= Start, i.e. MinDistance >= 0. Cheaper remainder calculation algorithm
  // might be used for positive numerators.
  const MachineOperand MinRestDistanceOp =
      StepOp().getImm() > 0 && HasLoopTakenGuarantee
          ? evalRemPosToPosI32(*Prolog, --Prolog->end(), MinDistanceOp,
                               UnrolledStep, *MRI, *TII)
          : evalRemPosI32(*Prolog, --Prolog->end(), MinDistanceOp, UnrolledStep,
                          *MRI, *TII);

  const MachineOperand UnrolledDistanceVal = evalSubI32(
      *Prolog, --Prolog->end(), MinDistanceOp, MinRestDistanceOp, *MRI, *TII);

  const MachineOperand UnrolledBoundaryVal = evalAddI32(
      *Prolog, --Prolog->end(), StartOp(), UnrolledDistanceVal, *MRI, *TII);

  bool isDoron1Plus = CurrMF->getSubtarget<TPCSubtarget>().hasDoron1();

  // Set the new boundary for the source loop to be unrolled.
  //
  // If operand type is changed, LoopInst type should be updated also. For
  // example:
  //    Source loop (LOOPsii):
  //      for (int i = start; i < 100; ++i)
  //
  //    Loop to be unrolled (LOOPssi):
  //      new_boundary = 100 - start;
  //      for (int i = start; i < new_boundary; ++i)
  if (BoundOp().isImm()) {
    if (UnrolledBoundaryVal.isImm()) {
      BoundOp().setImm(UnrolledBoundaryVal.getImm());
    } else {
      // Loop instruction type is going to be changed. Instruction replacement
      // is required.
      const unsigned NewOpcode = getLoopMachineInstrOpcode(
          StartOp().isImm(),
          false, // Bound is UnrolledLoopBoundOp, it is a register.
          true,  // Step is immediate by precondition.
          TPCMCInstrInfo::hasPredicate(*LoopInst, isDoron1Plus));

      // Exit must already have "address taken" property because it was the
      // target of indirect branch (from source loop) before transformation.
      assert(LoopInst->getOperand(4).getMBB() == Exit);

      const MachineInstrBuilder MIB = BuildMI(*Prolog, LoopInst->getIterator(),
                                              DebugLoc(), TII->get(NewOpcode));
      MIB.add(StartOp());
      MIB.addReg(UnrolledBoundaryVal.getReg());
      MIB.add(StepOp());
      MIB.add(CmpOp());
      MIB.addMBB(Exit);
      if (TPCMCInstrInfo::hasPredicate(*LoopInst, isDoron1Plus)) {
        MIB.add(LoopInst->getOperand(5));  // PredReg
        MIB.add(LoopInst->getOperand(6)); // Polarity
      }
      MIB.addReg(LoopCounter, RegState::Define | RegState::Implicit);

      LLVM_DEBUG(dbgs() << "Loop instruction: " << *LoopInst;
                 dbgs() << "   replaced with: " << *MIB.getInstr());

      LoopInst->eraseFromParent();
      LoopInst = MIB.getInstr();
    }
  } else {
    // It should be impossible to convert source loop with register bound to
    // immediate bound like this:
    //    Source loop:         for (int i = 0; i < bound; ++i)
    //    Loop to be unrolled: for (int i = 0; i < CONST; ++i)
    assert(!UnrolledBoundaryVal.isImm());

    BoundOp().setReg(UnrolledBoundaryVal.getReg());
  }

  // Create epilogue loop pre-header block before the exit block.
  MachineFunction &MF = *Prolog->getParent();
  MachineBasicBlock &EpilogLoopPreHeader =
      *MF.CreateMachineBasicBlock(Prolog->getBasicBlock());
  MF.insert(Exit->getIterator(), &EpilogLoopPreHeader);
  LoopInst->getOperand(4).setMBB(&EpilogLoopPreHeader);
  Latch->removeSuccessor(Exit);
  Latch->addSuccessor(&EpilogLoopPreHeader);
  EpilogLoopPreHeader.setHasAddressTaken(); // has implicit ref from loop instr.

  // Naive clone instructions of loop blocks without registers renewal.
  SmallVector<MachineBasicBlock *, 2> ClonedBBs;
  ClonedBBs.reserve(SourceBBs.size());
  for (MachineBasicBlock *SourceBB : SourceBBs) {
    MachineBasicBlock *ClonedBB = ClonedBBs.emplace_back(
        MF.CreateMachineBasicBlock(SourceBB->getBasicBlock()));
    MF.insert(Exit->getIterator(), ClonedBB);
    for (const MachineInstr &MI : *SourceBB)
      ClonedBB->push_back(MF.CloneMachineInstr(&MI));
  }

  // Shortcut to search cloned BB by its source counterpart. Expected source
  // and cloned BBs count is 1, so linear search is nice here.
  const auto GetRelatedClonedBB = [&](MachineBasicBlock *SourceBB) {
    const auto *const It = llvm::find(SourceBBs, SourceBB);
    return It != SourceBBs.end()
               ? ClonedBBs[std::distance(SourceBBs.begin(), It)]
               : nullptr;
  };

  // Update successors / predecessors for cloned blocks.
  for (MachineBasicBlock *SourceBB : SourceBBs) {
    for (MachineBasicBlock *S : SourceBB->successors()) {
      if (MachineBasicBlock *ClonedS = GetRelatedClonedBB(S)) {
        GetRelatedClonedBB(SourceBB)->addSuccessor(ClonedS);
      } else {
        assert(S == &EpilogLoopPreHeader); // Latch suc is already updated.
      }
    }
  }
  // Update successors for EpilogLoopPreHeader.
  EpilogLoopPreHeader.addSuccessor(ClonedBBs.front());
  EpilogLoopPreHeader.addSuccessor(Exit);  // From the future LOOP instruction.
  // Update successor for cloned Latch block to the Exit
  GetRelatedClonedBB(Latch)->addSuccessor(Exit);
  // Replace Exit block as Prolog successor to EpilogLoopPreHeader
  Prolog->removeSuccessor(Exit);
  Prolog->addSuccessor(&EpilogLoopPreHeader);

  // Create new definitions for cloned instructions and replace its usages.
  RegMap RegMapping;
  for (MachineBasicBlock *ClonedBB : ClonedBBs) {
    for (MachineInstr &ClonedInstr : ClonedBB->instrs()) {
      for (MachineOperand &ClonedOp : ClonedInstr.defs()) {
        const Register PrevDefReg = ClonedOp.getReg();
        if (PrevDefReg.isPhysical())
          continue;

        const TargetRegisterClass *RC = MRI->getRegClass(PrevDefReg);
        const Register DefReg = MRI->createVirtualRegister(RC);
        for (MachineBasicBlock *BB : ClonedBBs)
          replaceUsesInBB(*BB, PrevDefReg, DefReg, false);
        replaceUsesInBB(*Exit, PrevDefReg, DefReg, false);

        RegMapping[PrevDefReg] = DefReg;
        ClonedOp.setReg(DefReg);
      }
    }
  }

  // Update phi nodes in cloned loop.
  //
  // Source MIR:
  //
  //    BB.prolog:
  //      %prolog_value = ...
  //      LOOP ...
  //
  //    BB.loop:
  //      %result = PHI [%prolog_value, BB.prolog], [%loop_value, BB.loop]
  //      %loop_value = ...
  //      LOOPEND ...
  //
  //    BB.exit:
  //      ...
  //
  // Should be transformed to:
  //
  //    BB.prolog:
  //      %prolog_value = ...
  //      LOOP ...
  //
  //    BB.loop:
  //      %result = PHI [%prolog_value, BB.prolog], [%loop_value, BB.loop]
  //      %loop_value = ...
  //      LOOPEND ...
  //
  //    BB.epilogue_pre_header:
  //      %result_eph = PHI [%prolog_value, BB.prolog],   ; CHANGE HERE!
  //                        [%loop_value, BB.loop]        ; CHANGE HERE!
  //      LOOP ...
  //
  //    BB.epilogue_loop:
  //      %epilogue_result = PHI [%result_eph, BB.epilogue_pre_header], ; HERE!
  //                             [%epilogue_value, BB.epilogue_loop]    ; HERE!
  //      %epilogue_value = ...
  //      LOOPEND ...
  //
  //    BB.exit:
  //      ...
  for (MachineBasicBlock *ClonedBB : ClonedBBs) {
    for (MachineInstr &ClonedPhi : ClonedBB->phis()) {
      const PhiIP PrologIP = getPhiIPForMBB(ClonedPhi, *Prolog).getValue();
      const PhiIP LatchIP = getPhiIPForMBB(ClonedPhi, *Latch).getValue();
      const MachineInstr &CounterpartPhi =
          insertPHI(*MRI->getRegClass(ClonedPhi.getOperand(0).getReg()),
                    {&PrologIP, &LatchIP}, EpilogLoopPreHeader,
                    EpilogLoopPreHeader.begin(), *MRI, *TII);
      LLVM_DEBUG(dbgs() << "Epilogue: Created phi counterpart in EPH\n"
                        << "  Cloned phi : " << ClonedPhi
                        << "  Counterpart: " << CounterpartPhi);
      setPhiIPInternals(PrologIP, CounterpartPhi.getOperand(0).getReg(),
                        EpilogLoopPreHeader);
      setPhiIPInternals(LatchIP, RegMapping, *GetRelatedClonedBB(Latch));
    }
  }

  // Update phi nodes in Exit block.
  //
  // Source MIR:
  //
  //    BB.prolog:
  //      %prolog_value = ...
  //      LOOP ...
  //
  //    BB.loop:
  //      %loop_value = ...
  //      LOOPEND ...
  //
  //    BB.exit:
  //      %result = PHI [%prolog_value, BB.prolog],
  //                    [%loop_value, BB.loop]
  //
  // Should be transformed to:
  //
  //    BB.prolog:
  //      %prolog_value = ...
  //      LOOP ...
  //
  //    BB.loop:
  //      %loop_value = ...
  //      LOOPEND ...
  //
  //    BB.epilogue_pre_header:
  //      %result_eph = PHI [%prolog_value, BB.prolog],   ; CHANGE HERE!
  //                        [%loop_value, BB.loop]        ; CHANGE HERE!
  //      LOOP ...
  //
  //    BB.epilogue_loop:
  //      %epilogue_value = ...
  //      LOOPEND ...
  //
  //    BB.exit:
  //      %result = PHI [%result_eph, BB.epilogue_pre_header],  ; CHANGE HERE!
  //                    [%epilogue_value, BB.epilogue_loop]     ; CHANGE HERE!
  //
  // Note that %loop_value and %prolog_value might be immediate, not a registry.
  for (MachineInstr &Phi : Exit->phis()) {
    const PhiIP IPProlog = getPhiIPForMBB(Phi, *Prolog).getValue();
    const PhiIP IPLatch = getPhiIPForMBB(Phi, *Latch).getValue();

    Register EpilogPreHeaderDefReg;

    // Need Phi in epilogue pre header:
    //
    //   EpilogPreHeaderDefReg = PHI [IPProlog->ValOp, Prolog],
    //                               [IPLatch->ValOp, Latch]
    //
    // Find the required phi or create the new one.
    MachineInstr *CounterpartPhi = nullptr;
    for (MachineInstr &EPHPhi : EpilogLoopPreHeader.phis()) {
      if (isPhiInputs(EPHPhi, {&IPProlog, &IPLatch})) {
        CounterpartPhi = &EPHPhi;
        break;
      }
    }
    if (CounterpartPhi) {
      EpilogPreHeaderDefReg = CounterpartPhi->getOperand(0).getReg();

      LLVM_DEBUG(dbgs() << "Epilogue: Found phi counterpart in EPH\n"
                        << "  Counterpart: " << *CounterpartPhi);
    } else {
      // Create counterpart phi instruction in epilogue loop pre header.
      EpilogPreHeaderDefReg = MRI->createVirtualRegister(
          MRI->getRegClass(Phi.getOperand(0).getReg()));
      CounterpartPhi =
          BuildMI(EpilogLoopPreHeader, EpilogLoopPreHeader.begin(), DebugLoc(),
                  TII->get(TargetOpcode::PHI), EpilogPreHeaderDefReg)
              .add(*IPProlog.ValOp)
              .addMBB(Prolog)
              .add(*IPLatch.ValOp)
              .addMBB(Latch)
              .getInstr();

      LLVM_DEBUG(dbgs() << "Epilogue: Created phi counterpart in EPH\n"
                        << "  Exit phi : " << Phi
                        << "  Counterpart: " << *CounterpartPhi);
    }

    // Update phi inputs.
    setPhiIPInternals(IPProlog, EpilogPreHeaderDefReg, EpilogLoopPreHeader);
    setPhiIPInternals(IPLatch, RegMapping, *GetRelatedClonedBB(Latch));
  }

  // Insert LOOP instruction into EpilogLoopPreHeader, LOOPEND instruction in
  // the cloned loop latch block must already present.
  {
    // EpilogLoopPreHeader is going to use hardware register LoopCounter, so it
    // must be enumerated in live ins.
    if (!EpilogLoopPreHeader.isLiveIn(LoopCounter))
      EpilogLoopPreHeader.addLiveIn(LoopCounter);

    const unsigned Opcode = getLoopMachineInstrOpcode(
        false, // Start is used from counter physical register.
        SavedSourceBoundOp.isImm(),
        true, // Step is immediate by precondition.
        TPCMCInstrInfo::hasPredicate(*LoopInst, isDoron1Plus));

    const MachineInstrBuilder MIB =
        BuildMI(EpilogLoopPreHeader, EpilogLoopPreHeader.end(), DebugLoc(),
                TII->get(Opcode));
    MIB.addReg(LoopCounter); // Standard epilogue loop trick from LLVM.
    MIB.add(SavedSourceBoundOp);
    MIB.add(StepOp());
    MIB.add(CmpOp());
    MIB.addMBB(Exit);
    if (TPCMCInstrInfo::hasPredicate(*LoopInst, isDoron1Plus)) {
      MIB.add(LoopInst->getOperand(5));  // PredReg
      MIB.add(LoopInst->getOperand(6)); // Polarity
    }
    MIB.addReg(LoopCounter, RegState::Define | RegState::Implicit);
  }

  // Update LOOPEND instruction params in cloned loop Latch.
  {
    MachineInstr &LoopEndInstr = GetRelatedClonedBB(Latch)->back();
    assert(LoopEndInstr.getOpcode() == TPC::LOOPEND);

    // According to construction algorithm, exit block for epilogue loop LOOPEND
    // instruction should already point to Exit. Nothing to do.
    assert(LoopEndInstr.getOperand(0).getMBB() == Exit);

    // Continuation block should point to cloned loop header.
    LoopEndInstr.getOperand(1).setMBB(ClonedBBs.front());
  }

  // Update LOOPEND instruction params in source loop latch.
  {
    MachineInstr &LoopEndInstr = Latch->back();
    assert(LoopEndInstr.getOpcode() == TPC::LOOPEND);

    // Source loop exit block must point to epilogue loop pre-header.
    LoopEndInstr.getOperand(0).setMBB(&EpilogLoopPreHeader);

    // Source loop continuation block must already point to source loop header.
    assert(LoopEndInstr.getOperand(1).getMBB() == Header);
  }

  // Remove loop_taken pragma for the source loop if exists. There is no more
  // guarantee that source loop is always taken.
  removeLoopTakenMD(Latch->back());

  Exit = &EpilogLoopPreHeader;

  // At this point program has created the new loop and rewired CFG. Unrolling
  // code does not take into account epilogue loop existence, and it should not.
  // Update analysis internal structures for their proper results within
  // unrolling code.
  recalculateAnalysis();
}

// Calculates unroll count and how to shift execution threads
bool TPCPipeliner::pipeline(MachineLoop* L) {
  NextCounters.clear();
  LeftCounters.clear();
  InitialCounters.clear();
  EndCounters.clear();
  ExecThreads.clear();
  DelayedPhis.clear();
  AdjustedPhis.clear();
  AccumulateMap.clear();
  LinearIVs.clear();
  MLLinearIVs.clear();

  Header = nullptr;
  Latch  = nullptr;
  Exit   = nullptr;
  Prolog = nullptr;

  CurLoop = L;
  LoopCounter = getCounterRegister(*CurLoop);
  Header = CurLoop->getHeader();
  Latch = CurLoop->getLoopLatch();
  Exit = CurLoop->getExitBlock();

  LLVM_DEBUG(dbgs() << "Run pipelining for loop with header "
                    << printMBBReference(*Header) << "\n");

  if (!Latch)
    return false;

  if (!Exit)
    return false;

  MachineInstr& EndInstr = Latch->back();

  if (EndInstr.getOpcode() != TPC::LOOPEND)
    return false;

  Prolog = CurLoop->getLoopPredecessor();
  assert(Prolog);

  LoopInst = &Prolog->back();
  if (!isLoop(*LoopInst))
    return false;

  PrologInserter = --Prolog->end();

  // Avoid unrolling non-single MBB loops. Loops with branches are not supported
  // by unrolling algorithm. Theoretically loop might consist of a chain of MBBs
  // without terminators, but the case should not appear, such MBBs should be
  // squashed before this pass.
  if (CurLoop->getBlocks().size() >= 2)
    return false;

  // Avoid unrolling if step is not an immediate value. It is hard to make
  // gain from unrolling in this case.
  const MachineOperand &Step = LoopInst->getOperand(2);
  if (!Step.isImm())
    return false;

  // Get user-specified unroll count from pragma.
  const MDNode *LoopMD = getMachineLoopMDNode(EndInstr);

  const unsigned UserUnrollCount =
      LoopMD ? getUnrollCountFromMetadata(LoopMD).getValueOr(0) : 0;

  if (UserUnrollCount < 2)
    return false;

  if (!calcLoopForm())
    return false;

  if (checkForProhibitingInstructions())
    return false;

  // Report warning on dependencies which might affect unrolling result.
  const bool IgnoreCheckAgainstStoresToSameTensorId = true;
  const MLDAReport DepAnalysisReport = runMachineLoopDataDependencyAnalysis(
      *CurLoop, *MRI, CurrMF->getSubtarget<TPCSubtarget>(), *TII, MLLinearIVs,
      IgnoreCheckAgainstStoresToSameTensorId, UserUnrollCount);
  if (DepAnalysisReport.HasLoopDependentMemStLd == true ||
      DepAnalysisReport.HasLoopIndependentMemStLd == true ||
      DepAnalysisReport.HasLoopDependentMemStSt == true ||
      DepAnalysisReport.HasLoopIndependentMemStSt == true) {
    errs()
        << "warning: TPC backend unrolling is requested in loop_unroll pragma, "
           "but compiler has detected memory dependencies which are affected "
           "by backend unrolling algorithm. TPC backend unrolling optimization "
           "is going to be applied anyway, but code correctness is not "
           "guaranteed. Please, re-check the source code.\n";

    // Try to print source code location from header basic block.
    auto FirstMI = CurLoop->getHeader()->getFirstNonPHI();
    if (FirstMI != CurLoop->getHeader()->end()) {
      if (const DILocation *Loc = FirstMI->getDebugLoc().get()) {
        const StringRef FileName = Loc->getFilename();
        if (!FileName.empty()) {
          errs() << "at: " << FileName << ":" << Loc->getLine() << ":"
                 << Loc->getColumn() << "\n";
        }
        const Optional<StringRef> Source = Loc->getSource();
        if (Source.hasValue())
          errs() << Source.getValue() << "\n";
      }
    }
  }

  // Collect information about linear induction variables.
  MLLinearIVs = findLinearIVs(*CurLoop, *MRI);

  LLVM_DEBUG({
    for (const MLoopLinearIV &IV : MLLinearIVs)
      dbgs() << "Detected linear iv: " << IV.getNextDefMI();
  });

  // Special linear IVs mapping for compatibility with non-refactored algorithm
  // parts.
  LinearIVs =
      makeLinearIVNextValDefToStepOpIxMapping(*CurLoop, *MRI, MLLinearIVs);

  const unsigned UnrollCount = UserUnrollCount;

  bool Pipelined = LoopMD && getPipelineFromMetadata(LoopMD);

  if (isIterationsCountKnownAtCompileTime(*LoopInst)) {
    const llvm::Optional<uint64_t> IterationsCount =
        getCompileTimeIterationsCount(*LoopInst);

    // Avoid unrolling infinite loops.
    if (!IterationsCount.hasValue())
      return false;

    // Avoid unrolling loop if it has no sense.
    if (IterationsCount.getValue() < UnrollCount)
      return false;

    // Avoid pipelining if it has no sense.
    if (IterationsCount.getValue() < UnrollCount * 2)
      Pipelined = false;
  }

  // Avoid unrolling if unrolled step does not fit into i32 type.
  assert(isIntN(32, Step.getImm()));
  if (!isIntN(32, static_cast<int64_t>(Step.getImm()) *
                      static_cast<int64_t>(UnrollCount)))
    return false;

  // If user manually specified loop_unroll, he guarantees that trip count value
  // is divisible at unrolled step even if trip count value is a run time value.
  // It is the loop_unroll pragma precondition. But if user additionally
  // specified unaligned_trip_count, then epilogue should be generated.
  const bool IsEpilogueRequired =
      findOptionMDForLoopMD(*LoopMD, "llvm.loop.unroll.unaligned_trip_count");

  if (IsEpilogueRequired) {
    // Pipelining does not work properly if loop epilogue is generated, so
    // pipelining must be disabled if pass is going to generate epilogue.
    Pipelined = false;

    generateEpilogueLoop(UnrollCount, LoopMD && hasLoopTakenMD(*LoopMD));

    LLVM_DEBUG({
      dbgs() << "MF after epilogue loop generation:\n\n";
      CurrMF->dump();
    });
  } else {
    LLVM_DEBUG(dbgs() << "Epilogue loop generation is not required.\n");
  }

  unrollAndAlign(UnrollCount, Pipelined);

  return true;
}

void TPCPipeliner::createPrologCounters(unsigned UnrollCount, MachineBasicBlock* PrologBlock) {
  // For the first thread just return start value
  unsigned StartReg;
  //MachineBasicBlock::iterator InsertPos = --PrologBlock->end();
  MachineBasicBlock::iterator InsertPos = PrologInserter;
  assert(isLoop(*(--Prolog->end())) && "Preheader wasn't created during loop construction");
  if (LoopInst->getOperand(0).isReg()) {
    StartReg = LoopInst->getOperand(0).getReg();
  } else {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::MOVsip), v_reg)
        .addImm(LoopInst->getOperand(0).getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(v_reg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    StartReg = v_reg;
  }
  InitialCounters.push_back(StartReg);

  unsigned PrevReg = StartReg;
  for (unsigned i = 1; i < UnrollCount; ++i) {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    if (LoopInst->getOperand(2).isReg()) {
      MachineInstr* MI = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), v_reg)
          .addReg(PrevReg)
          .addReg(LoopInst->getOperand(2).getReg())
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(PrevReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      (void) MI;
      PrevReg = v_reg;
      LLVM_DEBUG(ExecThreads[i].Dprolog.push_back(MI));
    } else {
      MachineInstr* MI = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), v_reg)
          .addReg(StartReg)
          .addImm(LoopInst->getOperand(2).getImm() * i)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(StartReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      (void) MI;
      LLVM_DEBUG(ExecThreads[i].Dprolog.push_back(MI));
    }
    InitialCounters.push_back(v_reg);

    // Now patch loop counter registers that were moved out of the loop
    for (MachineInstr *MI : ExecThreads[i].CounterInstrs)
      replaceUses(*MI, LoopCounter, v_reg);
  }
}

void TPCPipeliner::correctCounters(unsigned UnrollCount) {
  if (ExecThreads[1].CounterInstrs.empty()) {
    return;
  } else {
    createPrologCounters(UnrollCount, Prolog);
  }
}

void TPCPipeliner::correctIVs(int UnrollCount) {
  for (auto IV : LinearIVs) {
    MachineInstr *const MI = IV.first;
    const unsigned OpNum = IV.second;

    const Register NextItValDefReg = MI->getOperand(0).getReg();
    const uint64_t NewStepValue = MI->getOperand(OpNum).getImm() * UnrollCount;

    const SmallVector<MachineInstr *, 8> NextItValUses =
        collectUses(NextItValDefReg, *MRI);

    const bool HasUsageInsideLoop =
        any_of(NextItValUses, [this](MachineInstr *UseMI) {
          return !UseMI->isPHI() && CurLoop->contains(UseMI);
        });

    if (HasUsageInsideLoop) {
      MachineInstr *const ClonedMI = Prolog->getParent()->CloneMachineInstr(MI);
      const Register Reg =
          MRI->createVirtualRegister(MRI->getRegClass(NextItValDefReg));
      ClonedMI->getOperand(0).setReg(Reg);

      // Replace IV phi input with unrolled step value.
      for (MachineInstr *UseMI : NextItValUses) {
        if (UseMI->isPHI() && CurLoop->contains(UseMI))
          replaceUses(*UseMI, NextItValDefReg, Reg);
      }

      MI->getParent()->insert(MI, ClonedMI);

      ClonedMI->getOperand(OpNum).setImm(NewStepValue);
    } else {
      MI->getOperand(OpNum).setImm(NewStepValue);
    }
  }
}

void TPCPipeliner::correctExit() {
  MachineBasicBlock* EB = LoopInst->getOperand(4).getMBB();
  if (EB->begin() == EB->end()) {
    MachineBasicBlock* MBB = *(EB->successors().begin());
    MBB->setHasAddressTaken();
    LoopInst->getOperand(4).setMBB(MBB);

    Latch->removeSuccessor(EB);
    Latch->addSuccessor(MBB);

    // Reverse exit rewiring again
    for (MachineInstr &MI : MBB->phis()) {
      for (MachineOperand &MO : MI.uses()) {
        if (MO.isMBB() && MO.getMBB() == EB) {
          MO.setMBB(Latch);
        }
      }
    }
  }
}

// This method creates increments of the counter for shifted iterations.
// Unfortunately we can't handle counter increments as simple instructions of an execution thread
// because they depend on the previous iteration.
// So in this method we ensure that only shifted increments are calculated inside the loop
// and counters for current iteration are all ready either from prolog or from previous iteration.
// Otherwise SPU will ruin the entire pipeline.
void TPCPipeliner::createNextCounters(int Counter, MachineBasicBlock* PrologBlock) {
  MachineBasicBlock::iterator InsertPos = --(PrologBlock->end());
  assert((*InsertPos).getOpcode() == TPC::LOOPEND);
  for (int i = 0; i < Counter; ++i) {
    Register v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));

    MachineInstr* ShiftCounter = nullptr;
    if (LoopInst->getOperand(2).isImm()) {
      ShiftCounter = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), v_reg)
          .addReg(LoopCounter)
          .addImm(LoopInst->getOperand(2).getImm() * Counter)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(LoopCounter, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      // TODO: Need to multiply
      ShiftCounter = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), v_reg)
          .addReg(LoopCounter)
          .addReg(LoopInst->getOperand(2).getReg() /** (i + Counter)*/)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(LoopCounter, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    }
    unsigned ThreadCounter = ExecThreads[i].VRegMap[LoopCounter];
    LLVM_DEBUG(ExecThreads[i].Dloop.push_back(ShiftCounter));

    // Remove original increment
    if (ThreadCounter != LoopCounter) {
      MachineInstr* OriginalCounterInst = MRI->getVRegDef(ThreadCounter);
      OriginalCounterInst->removeFromParent();
    }

    // Create Phi node for the counter
    Register PrologCounter = InitialCounters[i];
    unsigned PhiCounter  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    MachineInstr* Phi = BuildMI(*Header, Header->begin(), DebugLoc(), TII->get(TargetOpcode::PHI), PhiCounter).addReg(PrologCounter).addMBB(Prolog).addReg(v_reg).addMBB(Latch);
    if (PrologCounter.isPhysical()) {
      // For some reason PHI can't have physical register operands, create a proxy COPY in that case
      unsigned PhiWA  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
      MachineBasicBlock::iterator BeforeLoop = --Prolog->end();
      BuildMI(*Prolog, BeforeLoop, DebugLoc(), TII->get(TargetOpcode::COPY), PhiWA).addReg(PrologCounter);
      Phi->getOperand(1).setReg(PhiWA);
    }
    assert(!v_reg.isPhysical() && "Can't have phis with physical registers");
    LLVM_DEBUG(ExecThreads[i].Dphi.push_back(Phi));

    // Replace the base of increment with phi, also replace all usages of the current iteration with phi.
    ShiftCounter->getOperand(1).setReg(PhiCounter);
    LeftCounters.push_back(PhiCounter);

    NextCounters.push_back(v_reg);
  }
}

void TPCPipeliner::createEpilogCounters(int Counter, MachineBasicBlock* PrologBlock) {
  unsigned BoundaryReg;
  EpilogInserter = PrologBlock->begin();
  if (LoopInst->getOperand(1).isReg()) {
    BoundaryReg = LoopInst->getOperand(1).getReg();
  } else {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    MachineInstr* MI = BuildMI(*PrologBlock, EpilogInserter, DebugLoc(), TII->get(TPC::MOVsip), v_reg)
        .addImm(LoopInst->getOperand(1).getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0) // Switch
        .addReg(LoopCounter, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);

    EpilogInserter = ++MachineBasicBlock::iterator(MI);
    BoundaryReg = v_reg;
  }

  unsigned Opcode;
  if (LoopInst->getOperand(2).isReg()) {
    Opcode = TPC::SUBssp;
  } else {
    Opcode = TPC::SUBsip;
  }

  unsigned PrevReg = BoundaryReg;
  for (int i = 1; i <= Counter; ++i) {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    if (LoopInst->getOperand(2).isReg()) {
      unsigned LoopCounter = LoopInst->getOperand(2).getReg();
      MachineInstr* MI = BuildMI(*PrologBlock, EpilogInserter, DebugLoc(), TII->get(Opcode), v_reg)
          .addReg(PrevReg)
          .addReg(LoopCounter)
          .addImm(TPCII::OpType::INT32)
          .addImm(0) // Switch
          .addReg(LoopCounter, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      LLVM_DEBUG(ExecThreads[i].Epilog.push_back(MI));
      EpilogInserter = ++MachineBasicBlock::iterator(MI);
      PrevReg = v_reg;
    } else {
      int Decrement = Inclusive ? i - 1: i;
      MachineInstr* MI = BuildMI(*PrologBlock, EpilogInserter, DebugLoc(), TII->get(Opcode), v_reg)
          .addReg(BoundaryReg)
          .addImm(LoopInst->getOperand(2).getImm() * Decrement)
          .addImm(TPCII::OpType::INT32)
          .addImm(0) // Switch
          .addReg(LoopCounter, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      LLVM_DEBUG(ExecThreads[Counter - i].Epilog.push_back(MI));
      EpilogInserter = ++MachineBasicBlock::iterator(MI);
    }
    EndCounters.push_back(v_reg);
  }

  std::reverse(EndCounters.begin(), EndCounters.end());
}

// Checks wether a MovedInst defines a Phi use
bool TPCPipeliner::isCyclicDependency(MachineInstr* Phi, MachineInstr* MovedInst, MachineOperand& Use) {
  const TargetRegisterClass* RC = MRI->getRegClass(Use.getReg());
  while (true) {
    const TargetRegisterClass* DRC = MRI->getRegClass((*(MovedInst->defs().begin())).getReg());
    if (RC != DRC) {
      return false;
    } else {
      // TODO: walk the entire block until meeting a phi use
      return true;
    }
  }
}

bool TPCPipeliner::align(const std::vector<unsigned> &Shifts) {
  Exit = CurLoop->getExitBlock();

  // Can't pipeline across basic blocks
  if (Header != Latch) {
    return false;
  }

  MachineBasicBlock::iterator BeforeLoopEnd = --Latch->end();
  setPrologInserter();

  // Create a lot of increments. For every execution thread we need:
  // 1. Counter value for the prolog
  // 2. Counter value for a shifted iteration ouside of the current iteration in a loop
  // 3. Counter value for the epilog
  createPrologCounters(Shifts.size(), Prolog);
  createEpilogCounters(Shifts.size(), Exit);
  createNextCounters(Shifts.size(), Latch);

  assert(Shifts.size() == ExecThreads.size() && "Need shifting count for every iteration");
  for (unsigned i = 0; i < Shifts.size(); ++i) {
    unsigned ShiftCount = Shifts[i];

    unsigned InitialCounterReg = InitialCounters[i];
    unsigned EndCounterReg = EndCounters[i];
    unsigned NextCounterReg = NextCounters[i];
    unsigned LeftCounterReg = LeftCounters[i];

    RegMap PrologMap;

    // Shift instructions and create prolog
    for (unsigned j = 0; j < ShiftCount; ++j) {
      // In this loop we process instructions that should be shifted to the next iteration.
      // These instructions should be moved to the prolog of the loop and cloned.
      // Cloned isntructions are placed in the end of the loop and use next iteration's counter. But if an instruction is connected to phi-node
      // Phi node should be fixed to take the def of the moved instruction from the pre-header.
      // And the moved instruction shoul use the original phi-use insted of phi-def
      MachineInstr* PrologMI = ExecThreads[i].ThreadInstrs[j];

      PrologMI->removeFromParent();

      // Replace loop iterator with prolog initial values
      replaceUses(*PrologMI, ExecThreads[i].VRegMap[LoopCounter],
                  InitialCounterReg);

      // Clone instruction and insert it as a shifted iteration at the end of the loop
      MachineInstr* ShiftedMI = Prolog->getParent()->CloneMachineInstr(PrologMI);
      for (MachineOperand& MO: ShiftedMI->uses()) {
        if (MO.isReg() && MO.getReg() == InitialCounterReg) {
          MO.setReg(NextCounterReg);
        }
        // Instruction uses a phi-node, but it's the next iteration already
        // patch to use a phi-use
        if (MO.isReg() && isPhiDef(MO.getReg(), i)) {
          MachineInstr* Phi = MRI->getVRegDef(MO.getReg());
          unsigned counter = 0;
          for (MachineOperand& PhiMO : Phi->uses()) {
            if (PhiMO.isMBB() && PhiMO.getMBB() == Latch) {
              //Phi->dump();
              //ShiftedMI->dump();
              // TODO: potential bugs!! Too complicated
              if (ShiftedMI->getOperand(0).getReg() != Phi->getOperand(counter).getReg()) {
                bool DefinedLater = false;
                for (unsigned k = j + 1; k < ExecThreads[i].ThreadInstrs.size(); k++) {
                  // Make sure that the register is already defined by the previous instruction
                  // Otherwise don't get the valye from the Latch phi-use
                  if (ExecThreads[i].ThreadInstrs[k]->getOperand(0).getReg()
                      == Phi->getOperand(counter).getReg()) {
                    DefinedLater = true;
                    break;
                  }
                }
                if (!DefinedLater) {
                  MO.setReg(Phi->getOperand(counter).getReg());
                }
              }
              break;
            }
            counter++;
          }
        }
      }
      if (LinearIVs.count(PrologMI) > 0) {
        ShiftedMI->getOperand(LinearIVs[PrologMI]).
            setImm(ShiftedMI->getOperand(LinearIVs[PrologMI]).getImm()* Shifts.size());
      }
      Latch->insert(BeforeLoopEnd, ShiftedMI);

      // We have SSA here, create new vregs for everything
      for (MachineOperand& MO : PrologMI->defs()) {
        unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
        PrologMap[MO.getReg()] = v_reg;
        MO.setReg(v_reg);
      }

      replaceUseRegistersByMapping(*PrologMI, PrologMap);

      // If we moved an instruction dependent on a phi-node, remove a phi-node use
      // and replace it with a value from a pre-loop block
      bool PatchPhiUse = false;
      std::map<MachineOperand*, unsigned> PhiPatchMap;
      for (MachineOperand& MO : PrologMI->uses()) {
        if (MO.isReg() && isPhiDef(MO.getReg(), i)) {
          MachineInstr* Phi = MRI->getVRegDef(MO.getReg());
          int OpNum = 1;
          for (MachineOperand& PhiMO : Phi->uses()) {
            if (PhiMO.isMBB() && PhiMO.getMBB() == Prolog) {
              // TODO: what if it's an immidiate
              unsigned PhiHeader = Phi->getOperand(OpNum - 1).getReg();
              // If an operand is tied to the def, several instructions with the same use and different defs would cause problems. Copy use to a separate register.
              if (MO.isTied()) {
                unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
                // TODO: insert after phi-adjusting instructions
                BuildMI(*Prolog, --Prolog->end()/*PrologInserter*/, DebugLoc(), TII->get(TargetOpcode::COPY), v_reg)
                          .addReg(PhiHeader);
                MO.setReg(v_reg);
              } else {
                MO.setReg(Phi->getOperand(OpNum-1).getReg());
              }
              
              // Also patch the phi-node use to take prolog value
              if (isCyclicDependency(Phi, PrologMI, MO)) {
                PatchPhiUse = true;
                PhiPatchMap[&(Phi->getOperand(OpNum-1))] = PrologMI->getOperand(0).getReg();
                //Phi->getOperand(OpNum-1).setReg(PrologMI->getOperand(0).getReg());
              }
              break;
            }
            ++OpNum;
          }
        }
      }

      if (PatchPhiUse) {
        for (auto Pair : PhiPatchMap) {
          Pair.first->setReg(Pair.second);
        }
      }

      // Move instruction from loop to prolog
      MachineBasicBlock::iterator InsertPos = --Prolog->end();
      Prolog->insert(InsertPos, PrologMI);

      LLVM_DEBUG(ExecThreads[i].Dprolog.push_back(PrologMI));
      LLVM_DEBUG(ExecThreads[i].Dshift.push_back(ShiftedMI));
    }
    
    RegMap EpilogMap;

    // Create epilog
    for (unsigned j = ShiftCount; j < ExecThreads[i].ThreadInstrs.size(); ++j) {
      MachineInstr* LeftMI = ExecThreads[i].ThreadInstrs[j];
      MachineInstr* EpilogMI = Latch->getParent()->CloneMachineInstr(ExecThreads[i].ThreadInstrs[j]);

      if (LinearIVs.count(LeftMI) > 0) {
        LeftMI->getOperand(LinearIVs[LeftMI]).
            setImm(LeftMI->getOperand(LinearIVs[LeftMI]).getImm()* Shifts.size());
      }

      // In this loop we process instructions that were not shifted to the next iteration.
      // We need to clone these instructions and insert clones to the epilog.
      // But also we need to check whether they use defs of shifted instructions.
      // If that's the case, then we need to create additinal PHI (PrologDef, PrologMBB, ShiftedDef, LatchMBB)
      // add patch the use with the new virtual register from the PHI
      for (MachineOperand& MO : LeftMI->uses()) {
        if (MO.isReg() && PrologMap.count(MO.getReg()) > 0) {
          unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
          BuildMI(*Header, Header->begin(), DebugLoc(), TII->get(TargetOpcode::PHI), v_reg).addReg(PrologMap[MO.getReg()]).addMBB(Prolog).addReg(MO.getReg()).addMBB(Latch);
          // TODO: It's not really safe, someone else can use this. Probably need to create another map, or flag it in some way
//          PrologMap.erase(MO.getReg());
          assert(!Register(PrologMap[MO.getReg()]).isPhysical() && "Physical register in a phi node");
          assert(!Register(MO.getReg()).isPhysical() && "Physical register in a phi node");
          MO.setReg(v_reg);
          LLVM_DEBUG(ExecThreads[i].Dphi.push_back(&(*Header->begin())));
        }
      }
      replaceUses(*LeftMI, ExecThreads[i].VRegMap[LoopCounter], LeftCounterReg);

      // We have SSA here, create new vregs for everything
      for (MachineOperand& MO : LeftMI->defs()) {
        unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
        EpilogMap[MO.getReg()] = v_reg;

        MO.setReg(v_reg);
      }

      for (MachineOperand& MO : LeftMI->uses()) {
        if (MO.isReg() && EpilogMap.count(MO.getReg()) > 0) {
          MO.setReg(EpilogMap[MO.getReg()]);
        }
        // If def is tied to use, we need to replace epilog's tied use with iteration's def
        if (MO.isReg() && MO.isTied() && CurLoop->contains(MRI->getVRegDef(MO.getReg()))) {
//          unsigned Idx = LeftMI->findRegisterUseOperandIdx(MO.getReg());
//          LeftMI->dump();
//          unsigned TiedReg = LeftMI->getOperand(LeftMI->findTiedOperandIdx(Idx)).getReg();
          unsigned TiedReg = LeftMI->getOperand(LeftMI->findTiedDefIdx(MO)).getReg();
          replaceUses(*EpilogMI, MO.getReg(), TiedReg);
        }
      }

      replaceUses(*EpilogMI, ExecThreads[i].VRegMap[LoopCounter], EndCounterReg);

      Exit->insert(EpilogInserter, EpilogMI);
      EpilogInserter = ++MachineBasicBlock::iterator(EpilogMI);

      LLVM_DEBUG(ExecThreads[i].Dloop.push_back(LeftMI));
      ExecThreads[i].Epilog.push_back(EpilogMI);
    }

    for (MachineInstr* EMI : ExecThreads[i].Epilog) {
      for (MachineOperand&MO: EMI->uses()) {
        // If epilog instruction depends on the phi-def it should be the last calculated value
        if (MO.isReg() && !MO.getReg().isPhysical()) {
          MachineInstr* Phi = MRI->getVRegDef(MO.getReg());
          if (Phi->isPHI()) {
            if (EpilogMap.count(Phi->getOperand(3).getReg()) > 0) {
              MO.setReg(EpilogMap[Phi->getOperand(3).getReg()]);
            } else {
              MachineInstr* PhiUse = MRI->getVRegDef(Phi->getOperand(3).getReg());
              if (CurLoop->contains(PhiUse)) {
                MO.setReg(Phi->getOperand(3).getReg());
              }
            }
          }
        }
      }
    }

    // We created new virtual registers for the regular iteration instructions.
    // If there're phi nodes dependent on these instructions, patch they uses 
    for (MachineInstr *PhiInst: ExecThreads[i].Phis)
      replaceUseRegistersByMapping(*PhiInst, EpilogMap);

    for (MachineInstr& MI : *Header) {
      if (!MI.isPHI())
        replaceUseRegistersByMapping(MI, EpilogMap);
    }
  }
  return true;
}

bool TPCPipeliner::isPhiDef(unsigned Reg, unsigned Thread) {
  return any_of(
      ExecThreads[Thread].Phis,
      [Reg](MachineInstr *Phi){ return Phi->defs().begin()->getReg() == Reg; });
}

void TPCPipeliner::fixUnrolledLoopBoundary(const unsigned UnrollCount) {
  // Fix unrolled loop boundary for <=, >= cases.
  //
  // Initial loop:
  //
  //    for (int i = start; i <= k * unroll_step; i += step)
  //      function(i);
  //
  // Should be transformed to (!note changed boundary here!):
  //
  //    for (int i = start; i <= (k - 1) * unroll_step; i += unroll_step) {
  //      function(i);
  //      ...
  //      function(i + unroll_step - step);
  //    }
  //
  // Note that i == k * unroll_step is not processed in transformed loop. It is
  // an epilogue loop responsibility.
  MachineOperand &BoundOp = LoopInst->getOperand(1);
  const MachineOperand &StepOp = LoopInst->getOperand(2);
  const MachineOperand &CmpOp = LoopInst->getOperand(3);
  assert(StepOp.isImm() && CmpOp.isImm());
  if (CmpOp.getImm() == TPCII::LoopGE || CmpOp.getImm() == TPCII::LoopLE) {
    const MachineOperand FixedBoundOp = evalSubI32(
        *Prolog, --Prolog->end(), BoundOp,
        MachineOperand::CreateImm(UnrollCount * StepOp.getImm()), *MRI, *TII);
    updateRegOrImmOperand(BoundOp, FixedBoundOp);
  }
}

// Unrolls the loop, makes execution threads independent, shifts iterations
bool TPCPipeliner::unrollAndAlign(unsigned UnrollCount, bool DoPipelining) {
  assert(ExecThreads.empty());
  ExecThreads.resize(UnrollCount);

  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (MachineInstr &MI: MBB->instrs()) {
      if (MI.isBranch()) continue;
      if (MI.isPHI()) {
        ExecThreads[0].Phis.push_back(&MI);
      } else {
        ExecThreads[0].ThreadInstrs.push_back(&MI);
      }
    }
  }

  ExecThreads[0].VRegMap[LoopCounter] = LoopCounter;

  // Duplicate counter register for every execution thread
  {
    const auto InsertPos = Header->getFirstNonPHI();
    for (unsigned I = 1; I < UnrollCount; ++I) {
      const Register VReg = MRI->createVirtualRegister(&TPC::SRFRegClass);
      // If increment value is a constant create independent increment for
      // each execution thread with increment values computed at compile time.
      BuildMI(*Header, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), VReg)
          .addReg(LoopCounter)
          .addImm(LoopInst->getOperand(2).getImm() * I)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(VReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      ExecThreads[I].VRegMap[LoopCounter] = VReg;
    }
  }

  // Unroll and make execution threads independent
  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (MachineBasicBlock::instr_iterator I = MBB->instr_begin();
         I != MBB->instr_end(); ++I) {
      MachineInstr* CurMI = &(*I);

      // Fix Phis later
      if (CurMI->isPHI()) {
        DelayedPhis.push_back(CurMI);
        continue;
      }

      if (!is_contained(ExecThreads[0].ThreadInstrs, CurMI))
        continue;

      // Don't clone loops
      if (CurMI->isBranch()) continue;

      for (unsigned i = 1; i < UnrollCount; ++i) {
        MachineInstr* MI = MBB->getParent()->CloneMachineInstr(&(*I));
        if (LinearIVs.count(&(*I)) > 0) {
          LinearIVs[MI] = LinearIVs[&(*I)];
        }
        ExecThreads[i].ThreadInstrs.push_back(MI);
        replaceVregs(MI, i);

        MachineBasicBlock::instr_iterator CorrectPos = I;
        // Insert instructions in iteration order. This should be
        // meaningless since all iterations are independent,
        // but we have artificial tests that depend on it.
        for (unsigned j = 0; j < i; ++j, ++CorrectPos);

        MBB->insert(CorrectPos, MI);
      }
    }
  }

  fixUnrolledLoopBoundary(UnrollCount);

  MachineInstr* LastInstr = nullptr;
  if (Prolog->size() == 1) {
    LastInstr = &(*(--Prolog->end()));
  } else {
    LastInstr = &(*(--(--Prolog->end())));
  }

  // Now clone and patch phi-nodes
  sortDelayedPhis();

  for (MachineInstr* CurMI : DelayedPhis)
    patchPhiStartValue(*CurMI, UnrollCount);

  PrologInserter = MachineBasicBlock::iterator(LastInstr);

  if (!AccumulateMap.empty()) {
    // Merge all accumulator values
    MachineBasicBlock* OldExit = CurLoop->getExitBlock();
    assert(OldExit && "This loop is a hardware loop, it should have only one exit");
    assert(LoopInst->getOperand(4).getMBB() == OldExit && "Loop doesn't end with ExitBlock");

    // We need to merge accumulators just outside the loop.
    // But ExitBlock may contain Phi-nodes depending on accumulated values.
    // So we create another block, insert it before ExitBlock and make
    // it the new exit for the loop
    MachineFunction* MF = OldExit->getParent();
    MachineBasicBlock* Accum = MF->CreateMachineBasicBlock();
    MF->insert(OldExit->getIterator(), Accum);
    assert(LoopInst->getOperand(4).isMBB() && "Incorrect loop instruction");
    LoopInst->getOperand(4).setMBB(Accum);
    Accum->addSuccessor(OldExit, BranchProbability::getOne());
    Latch->removeSuccessor(OldExit);
    Latch->addSuccessor(Accum);

    // Fix Phi-nodes for the former exit block
    for (MachineInstr &MI : OldExit->phis()) {
      for (MachineOperand &MO : MI.uses()) {
        if (MO.isMBB() && MO.getMBB() == Latch) {
          MO.setMBB(Accum);
        }
      }
    }

    // For every entry in AccumulateMap we need to build a chain of ADDs
    // from the last thread to the first. The result of a the merge
    // should be the def of the first accumulator in the chain.
    // All occurences of this def inside the loop should be patched to preserve ssa.
    RegMap ForceSsa;
    MachineBasicBlock::iterator AccumInsertPos = Accum->end();

    for (AccumChains::iterator It = AccumulateMap.begin(); It != AccumulateMap.end(); ++It) {
      unsigned ResReg = It->first;
      const std::vector<unsigned> &Chain = It->second;
      const TargetRegisterClass* RC = MRI->getRegClass(ResReg);
      MachineInstr* MI = MRI->getVRegDef(ResReg);
      TPCII::OpType Type = getOpType(*MI);
      const unsigned AddOpc = getAddOpc(Type, RC);

      unsigned ChainTerm = Chain.back();
      if (TPC::ARFRegClass.hasSubClassEq(RC)) {
        for (int i = Chain.size() - 2; i >= 0; --i) {
           unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
           addQuadroRegs(Accum, TmpVreg, ChainTerm, Chain[i], AddOpc, Type, &TPC::VRFRegClass);
           ChainTerm = TmpVreg;
        }
        unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
        addQuadroRegs(Accum, ResReg, ChainTerm, TmpVreg, AddOpc, Type, &TPC::VRFRegClass);
        ForceSsa[ResReg] = TmpVreg;
      } else if (TPC::DRFRegClass.hasSubClassEq(RC)) {
        for (int i = Chain.size() - 2; i >= 0; --i) {
           unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
           addDoubleRegs(Accum, TmpVreg, ChainTerm, Chain[i], AddOpc, Type, &TPC::VRFRegClass);
           ChainTerm = TmpVreg;
        }
        unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
        addDoubleRegs(Accum, ResReg, ChainTerm, TmpVreg, AddOpc, Type, &TPC::VRFRegClass);
        ForceSsa[ResReg] = TmpVreg;
      } else if (TPC::ZRFRegClass.hasSubClassEq(RC)) {
        for (int i = Chain.size() - 2; i >= 0; --i) {
           unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
           addDoubleRegs(Accum, TmpVreg, ChainTerm, Chain[i], AddOpc, Type, &TPC::SRFRegClass);
           ChainTerm = TmpVreg;
        }
        unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
        addDoubleRegs(Accum, ResReg, ChainTerm, TmpVreg, AddOpc, Type, &TPC::SRFRegClass);
        ForceSsa[ResReg] = TmpVreg;
      } else {
        for (int i = Chain.size() - 2; i >= 0; --i) {
          unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
          BuildMI(*Accum, AccumInsertPos, DebugLoc(), TII->get(AddOpc), TmpVreg)
              .addReg(ChainTerm)
              .addReg(Chain[i])
              .addImm(Type)
              .addImm(0)
              .addReg(TmpVreg, RegState::Undef)
              .addReg(TPC::SPRF_TRUE)
              .addImm(0);
          ChainTerm = TmpVreg;
        }

        unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
        BuildMI(*Accum, AccumInsertPos, DebugLoc(), TII->get(AddOpc), ResReg)
            .addReg(ChainTerm)
            .addReg(TmpVreg)
            .addImm(Type)
            .addImm(0)
            .addReg(ResReg, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        ForceSsa[ResReg] = TmpVreg;
      }
    }

    // To preserve SSA replace original accumulator values with a new reg inside this loop
    for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
      for (MachineInstr& MI : MBB->instrs()) {
        for (MachineOperand& MO : MI.operands()) {
          if (MO.isReg()) {
            auto It = ForceSsa.find(MO.getReg());
            if (It != ForceSsa.end())
              MO.setReg(It->second);
          }
        }
      }
    }
  }

  const auto DumpInstructions = [](const std::vector<MachineInstr*> &InstrVec){
    for (MachineInstr *MI: InstrVec) {
      LLVM_DEBUG(MI->dump());
    }
  };

  for (unsigned i = 0; i < UnrollCount; ++i) {
    LLVM_DEBUG(dbgs() << "\n******** Thread " << i << " *********\n");
    LLVM_DEBUG(dbgs() << "\nPhis\n");
    DumpInstructions(ExecThreads[i].Phis);
    DumpInstructions(ExecThreads[i].Dphi);
    LLVM_DEBUG(dbgs() << "\nLoop body\n");
    DumpInstructions(ExecThreads[i].ThreadInstrs);
  }
  LLVM_DEBUG(dbgs() << "\n");

  // TODO: proper alignment strategy
  bool Aligned = false;
  if (DoPipelining) {
    std::vector<unsigned> Shifts;
    for (unsigned i = 0; i < UnrollCount; ++i)
      Shifts.push_back(std::min(ExecThreads[i].ThreadInstrs.size() - 1, (size_t)UnrollCount - i));

    Aligned = align(Shifts);
  }

  if (!DoPipelining) {
    setPrologInserter();
    correctCounters(UnrollCount);
    correctIVs(UnrollCount);
    correctExit();
  }

  // Change the increment value of the loop
  {
    MachineOperand &StepOp = LoopInst->getOperand(2);
    assert(StepOp.isImm());
    StepOp.setImm(StepOp.getImm() * UnrollCount);

    MachineOperand &CompOp = LoopInst->getOperand(3);
    if (StepOp.getImm() > 0 && CompOp.getImm() == TPCII::LoopNE)
      CompOp.setImm(TPCII::LoopLT);
  }

  if (Aligned) {
    PrologInserter = --Prolog->end();
    modifyBoundary(UnrollCount);
  }

  for (unsigned i = 0; i < UnrollCount; ++i) {
    LLVM_DEBUG(dbgs() << "\n******** Thread " << i << " *********\n");
    LLVM_DEBUG(dbgs() << "\nPrologue\n");
    DumpInstructions(ExecThreads[i].Dprolog);
    LLVM_DEBUG(dbgs() << "\nPhis\n");
    DumpInstructions(ExecThreads[i].Phis);
    DumpInstructions(ExecThreads[i].Dphi);
    LLVM_DEBUG(dbgs() << "\nLoop body\n");
    DumpInstructions(ExecThreads[i].Dloop);
    DumpInstructions(ExecThreads[i].Dshift);
    LLVM_DEBUG(dbgs() << "\nEpilogue\n");
    DumpInstructions(ExecThreads[i].Epilog);
  }
  LLVM_DEBUG(dbgs() << "\n");

  return true;
}

bool TPCPipeliner::calcLoopForm() {
  int CmpMode = LoopInst->getOperand(3).getImm();
  if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopEQ) {
    Inclusive = true;
  } else {
    Inclusive = false;
  }

  if (LoopInst->getOperand(2).isImm()) {
    Ascending = (LoopInst->getOperand(2).getImm() > 0);
    return true;
  }

  if (LoopInst->getOperand(0).isImm() && LoopInst->getOperand(1).isImm()) {
    Ascending = (LoopInst->getOperand(0).getImm() < LoopInst->getOperand(1).getImm());
    return true;
  }

  return false;  // loop form was not detected - prohibit unrolling
}

void TPCPipeliner::modifyBoundary(unsigned Decrement) {
  MachineOperand &BoundOp = LoopInst->getOperand(1);
  MachineOperand &StepOp = LoopInst->getOperand(2);
  MachineOperand &CmpOp = LoopInst->getOperand(3);
  if (BoundOp.isImm()) {
    if (StepOp.isImm()) {
      int Step = StepOp.getImm();
      if (Step > 0) {
        BoundOp.setImm(BoundOp.getImm() - Decrement);
      } else {
        BoundOp.setImm(BoundOp.getImm() + Decrement);
      }
    } else {
      int CmpMode = CmpOp.getImm();
      if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopLT) {
        BoundOp.setImm(BoundOp.getImm() - Decrement);
      } else if (CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopGT) {
        BoundOp.setImm(BoundOp.getImm() + Decrement);
      } else {
        // Can't infer ascending or descending  nature of the loop. Have to create
        // a runtime check.
        // TODO: SSA + predicated execution is tricky
        llvm_unreachable("");
      }
    }
  } else {
    unsigned NewBoundary = MRI->createVirtualRegister(MRI->getRegClass(BoundOp.getReg()));
    if (StepOp.isImm()) {
      int Step = StepOp.getImm();
      if (Step > 0) {
        unsigned LoopCounter = BoundOp.getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::SUBsip), NewBoundary)
            .addReg(LoopCounter)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0) // Switch
            .addReg(LoopCounter, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        BoundOp.setReg(NewBoundary);
      } else {
        unsigned LoopReg = BoundOp.getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::ADDsip), NewBoundary)
            .addReg(LoopReg)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopReg, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        BoundOp.setReg(NewBoundary);
      }
    } else {
      int CmpMode = CmpOp.getImm();
      if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopLT) {
        unsigned LoopCounter = BoundOp.getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::SUBsip), NewBoundary)
            .addReg(LoopCounter)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopCounter, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        BoundOp.setReg(NewBoundary);
      } else if (CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopGT) {
        unsigned LoopReg = BoundOp.getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::ADDsip), NewBoundary)
            .addReg(LoopReg)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopReg, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        BoundOp.setReg(NewBoundary);
      } else {
        // Can't infer ascending or descending  nature of the loop. Have to create
        // a runtime check.
        llvm_unreachable("");
      }
    }
  }
}

void TPCPipeliner::replaceWithZero(MachineOperand& ReplaceMO, TPCII::OpType ot) {
  MachineBasicBlock::iterator ZeroInsertPos = Prolog->getFirstNonPHI();
  if (ReplaceMO.isImm()) {
    ReplaceMO.setImm(0);
  } else if (ReplaceMO.isFPImm()) {
    // Even if the immediate is a floating point number, MOV uses integer bit
    // representation.
    ReplaceMO.ChangeToImmediate(0);
  } else if (ReplaceMO.isReg()) {
    const auto AddMovZero = [&](unsigned Opcode, unsigned Reg) {
      BuildMI(*Prolog, ZeroInsertPos++, DebugLoc(), TII->get(Opcode), Reg)
         .addImm(0)
         .addImm(ot)
         .addImm(0)
         .addReg(Reg, RegState::Undef)
         .addReg(TPC::SPRF_TRUE)
         .addImm(0);
    };

    unsigned zero_reg  = MRI->createVirtualRegister(MRI->getRegClass(ReplaceMO.getReg()));
    const TargetRegisterClass *RC = MRI->getRegClass(ReplaceMO.getReg());
    if (TPC::SRFRegClass.hasSubClassEq(RC)) {
      AddMovZero(TPC::MOVsip, zero_reg);
    } else if (TPC::VRFRegClass.hasSubClassEq(RC)) {
      AddMovZero(TPC::MOVvip, zero_reg);
    } else if (TPC::DRFRegClass.hasSubClassEq(RC)) {
      unsigned zero_sub0  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      unsigned zero_sub1  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      AddMovZero(TPC::MOVvip, zero_sub0);
      AddMovZero(TPC::MOVvip, zero_sub1);
      BuildMI(*Prolog, ZeroInsertPos++,DebugLoc(),
              TII->get(TargetOpcode::REG_SEQUENCE), zero_reg)
          .addReg(zero_sub0).addImm(TPC::sub_0)
          .addReg(zero_sub1).addImm(TPC::sub_1);
    } else if (TPC::ARFRegClass.hasSubClassEq(RC)) {
      unsigned zero_sub0  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      unsigned zero_sub1  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      unsigned zero_sub2  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      unsigned zero_sub3  = MRI->createVirtualRegister(&TPC::VRFRegClass);
      AddMovZero(TPC::MOVvip, zero_sub0);
      AddMovZero(TPC::MOVvip, zero_sub1);
      AddMovZero(TPC::MOVvip, zero_sub2);
      AddMovZero(TPC::MOVvip, zero_sub3);
      BuildMI(*Prolog, ZeroInsertPos++,DebugLoc(),
              TII->get(TargetOpcode::REG_SEQUENCE), zero_reg)
          .addReg(zero_sub0).addImm(TPC::sub_0)
          .addReg(zero_sub1).addImm(TPC::sub_1)
          .addReg(zero_sub2).addImm(TPC::sub_2)
          .addReg(zero_sub3).addImm(TPC::sub_3);

    } else {
      llvm_unreachable("Unsupported reg type in MAC phi-node");
    }

    ReplaceMO.setReg(zero_reg);
  } else {
    llvm_unreachable("Trying to replace wrong operand");
  }
}

// Return index of machine basic block argument in the given phi instruction
static llvm::Optional<unsigned> getBBIndexAsPhiOperand(MachineInstr &Phi,
                                                       MachineBasicBlock &BB) {
  for (auto &Item: enumerate(Phi.uses())) {
    if (Item.value().isMBB() && Item.value().getMBB() == &BB)
      return Item.index();
  }
  return llvm::None;
}

// Some cloned phi-nodes must start with the same values (for accumulators)
// the others must adjust out-of-loop uses and create instructions in the preheader
void TPCPipeliner::patchPhiStartValue(MachineInstr &Phi, int UnrollCount) {
  assert(Phi.isPHI());
  MachineBasicBlock *const MBB = Phi.getParent();
  const unsigned PhiDef = Phi.getOperand(0).getReg();

  const unsigned OutOfLoopUse = getBBIndexAsPhiOperand(Phi, *Prolog).getValue();
  const unsigned LoopUse = getBBIndexAsPhiOperand(Phi, *Latch).getValue();

  std::vector<MachineInstr*> Path = phiPath(&Phi, Phi.getOperand(OutOfLoopUse));
  std::reverse(Path.begin(), Path.end());

  // TODO: what if not a reg
  unsigned FinalReg = Phi.getOperand(OutOfLoopUse).getReg();
  unsigned FirstReg = FinalReg;

  std::vector<unsigned> UseVec;
  std::vector<unsigned> DefVec;

  for (int i = 1; i < UnrollCount; ++i) {
    MachineInstr* ClonedPhi = MBB->getParent()->CloneMachineInstr(&Phi);
    ExecThreads[i].Phis.push_back(ClonedPhi);
    replaceVregs(ClonedPhi, i);
    MBB->insert(MachineBasicBlock::instr_iterator(Phi), ClonedPhi);

    if (!Path.empty() && TPCII::isMac(Path.back()->getDesc())) {
      MachineOperand& ReplaceMO = ClonedPhi->getOperand(OutOfLoopUse);
      replaceWithZero(ReplaceMO, getOpType(*Path.back()));
      continue;
    }

    RegMap RM = {{PhiDef, FinalReg}};

    for (MachineInstr* MI : Path) {
      MachineInstr* Copy = Prolog->getParent()->CloneMachineInstr(MI);

      for (MachineOperand& MO : Copy->defs()) {
        unsigned v_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
        RM[MO.getReg()] = v_reg;
        MO.setReg(v_reg);
        // Last instruction in the path is guaranteed to be the one that generates phi use
        FinalReg = v_reg;
      }

      for (MachineOperand& MO : Copy->uses()) {
        if (!MO.isReg()) continue;
        if (RM.count(MO.getReg()) > 0) {
          MO.setReg(RM[MO.getReg()]);
        }
        if (AdjustedPhis.count(MO.getReg()) > 0) {
          MO.setReg(AdjustedPhis[MO.getReg()][i - 1]);
        }
        if (MO.getReg() == LoopCounter) {
          ExecThreads[i].CounterInstrs.push_back(Copy);
        }
        // Parallel threads shouldn't be tied to the same vreg
        if (Phi.getOperand(OutOfLoopUse).isReg()) {
          unsigned phi_reg = Phi.getOperand(OutOfLoopUse).getReg();
          if (MO.getReg() == phi_reg && MO.isTied()) {
            unsigned copy_reg  = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
            BuildMI(*Prolog, --Prolog->end()/*PrologInserter*/, DebugLoc(), TII->get(TargetOpcode::COPY), copy_reg)
                      .addReg(phi_reg);
            MO.setReg(copy_reg);
          }
        }
      }

      Prolog->insert(--Prolog->end(), Copy);
      LLVM_DEBUG(dbgs() << "Adjust phi with ");
      LLVM_DEBUG(Copy->dump());
    }

    // Finally set phi_use of a cloned phi inst to the new value
    // TODO: what if not a reg
    ClonedPhi->getOperand(OutOfLoopUse).setReg(FinalReg);
    DefVec.push_back(FirstReg);
    UseVec.push_back(FinalReg);
  }
  AdjustedPhis[PhiDef] = DefVec;
  AdjustedPhis[LoopUse] = UseVec;
}

// Find all instructions that participate in a phi-node calculation inside a loop
std::vector<MachineInstr*> TPCPipeliner::phiPath(MachineInstr *Phi,
                                                 MachineOperand &HeadPhiUse) {
  std::vector<MachineOperand*> OperandStack;
  OperandStack.push_back(&Phi->getOperand(1));
  OperandStack.push_back(&Phi->getOperand(3));

  std::vector<MachineInstr*> Path;
  while(!OperandStack.empty()) {
    MachineOperand* MO = OperandStack.back();
    OperandStack.pop_back();

    if (!MO->isReg()) continue;
    if (MO->getReg() == LoopCounter) continue;
    if (MO->getReg().isPhysical()) continue;
    MachineInstr* DefMI = MRI->getVRegDef(MO->getReg());
    if (!CurLoop->contains(DefMI)) continue;
    if (DefMI->isPHI()) continue;

    for (MachineOperand& IMO : DefMI->uses()) {
      if (IMO.isIdenticalTo(HeadPhiUse)) continue;
      if (IMO.isReg() && IMO.getReg() == Phi->getOperand(0).getReg()) continue;
      OperandStack.push_back(&IMO);
    }

    auto Pos = std::find(Path.begin(), Path.end(), DefMI);
    if (Pos != Path.end()) {
      Path.erase(Pos);
    }

    Path.push_back(DefMI);
  }
  return Path;
}

// Find a position for prolog counters. We need to insert them before
// dependent cloned instructions but after actual instructions of the preheader
void TPCPipeliner::setPrologInserter() {
  for (MachineBasicBlock::iterator it = Prolog->begin(); it != Prolog->end(); ++it) {
    MachineInstr* MI = &(*it);
    for (MachineOperand& MO : MI->uses()) {
      if (MO.isReg() && MO.getReg() == LoopCounter) {
        PrologInserter = it;
        return;
      }
    }
  }
}

struct PhiDepTree {
  std::list<PhiDepTree*> succs;
  MachineInstr* Phi;
  int Preds;

  PhiDepTree(MachineInstr *PhiInstr) : Phi(PhiInstr), Preds(0) {}
};

static void DFS(const PhiDepTree &Node,
                std::vector<MachineInstr *> &SortedPhis) {
  for (PhiDepTree *Succ : Node.succs) {
    DFS(*Succ, SortedPhis);
  }
  if (!is_contained(SortedPhis, Node.Phi)) {
    SortedPhis.push_back(Node.Phi);
  }
}

void TPCPipeliner::sortDelayedPhis() {
  // Create all nodes
  std::vector<PhiDepTree> Tree(DelayedPhis.begin(), DelayedPhis.end());

  // Build dep graph
  // TODO: for now just implement naive algorythm. But it's too slow and should be rewritten
  for (unsigned i = 0; i < DelayedPhis.size(); ++i) {
    MachineInstr* Dependant = DelayedPhis[i];
    PhiDepTree &DependantNode = Tree[i];

    const unsigned OutOfLoopUse =
        getBBIndexAsPhiOperand(*Dependant, *Prolog).getValue();

    const std::vector<MachineInstr*> Path =
        phiPath(Dependant, Dependant->getOperand(OutOfLoopUse));

    for (unsigned j = 0; j < DelayedPhis.size(); ++j) {
      MachineInstr* Phi = DelayedPhis[j];
      PhiDepTree &PhiNode = Tree[j];

      if (Phi == Dependant) continue;

      const llvm::Optional<unsigned> LatchOpIx =
          getBBIndexAsPhiOperand(*Phi, *Latch);
      const llvm::Optional<Register> LoopReg = LatchOpIx.map(
          [Phi](unsigned Value){ return Phi->getOperand(Value).getReg(); });

      // Find wether the dependant path uses the def of Phi
      for (MachineInstr* MI : Path) {
        for (MachineOperand& MO : MI->uses()) {
          if (MO.isReg() && (MO.getReg() == LoopReg || MO.getReg() == Phi->getOperand(0).getReg())) {
            PhiNode.succs.push_back(&DependantNode);
            DependantNode.Preds++;
            break;
          }
        }
      }
    }
  }

  // Topological sort
  DelayedPhis.clear();
  for (const PhiDepTree &Node : Tree) {
    if (Node.Preds == 0) {
      DFS(Node, DelayedPhis);
    }
  }
  std::reverse(DelayedPhis.begin(), DelayedPhis.end());
}

bool TPCPipeliner::checkForProhibitingInstructions() {
  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (const MachineInstr& MI : MBB->instrs()) {
      if (MI.getOpcode() == TPC::ASO) {
        LLVM_DEBUG(dbgs() << "ASO detected: " << MI);
        return true;
      }
      if (MI.getOpcode() == TPC::CACHE_FLUSH) {
        LLVM_DEBUG(dbgs() << "CACHE_FLUSH detected: " << MI);
        return true;
      }
      if (MI.getOpcode() == TPC::CACHE_INVALIDATE) {
        LLVM_DEBUG(dbgs() << "CACHE_INVALIDATE detected: " << MI);
        return true;
      }
      if (TPCInstrInfo::isMMIOAccess(MI)) {
        LLVM_DEBUG(dbgs() << "MMIOAccess detected: " << MI);
        return true;
      }

      // Prohibit hardware registries usage which are single in their registry
      // class to avoid their spilling.
      for (const MachineOperand& MO: MI.defs()) {
        assert(MO.isReg() && "Def can't be a constant");
        if (MO.getReg().isPhysical()) continue;
        const TargetRegisterClass *RC = MRI->getRegClass(MO.getReg());
        if (TPC::HWRMWRegRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "RMW-like definition: " << MI);
          return true;
        }
        if (TPC::HWDivStepRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "DivStep-like definition: " << MI);
          return true;
        }
        if (TPC::HWSCarryRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "SCarry-like definition: " << MI);
          return true;
        }
        if (TPC::HWVCarryRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "VCarry-like definition: " << MI);
          return true;
        }
        if (TPC::HWZPRegRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "ZP-like definition: " << MI);
          return true;
        }
        if (TPC::HWTnsrRegLdRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "Tensor load like definition: " << MI);
          return true;
        }
        if (TPC::HWTnsrRegStRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "Tensor store like definition: " << MI);
          return true;
        }
        if (TPC::HWOffsSizeRegLdRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "OffsSize load like definition: " << MI);
          return true;
        }
        if (TPC::HWOffsSizeRegStRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "OffsSize store like definition: " << MI);
          return true;
        }
        if (TPC::HWPCRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "PC-like definition: " << MI);
          return true;
        }
        if (TPC::HVRFRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "HVRF-like definition: " << MI);
          return true;
        }
        if (TPC::HVPRFRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "HVPRF-like definition: " << MI);
          return true;
        }
        if (TPC::HSPRFRegClass.hasSubClassEq(RC)) {
          LLVM_DEBUG(dbgs() << "HSPRF-like definition: " << MI);
          return true;
        }
      }
    }
  }
  return false;
}

void TPCPipeliner::addQuadroRegs(MachineBasicBlock* Accum, unsigned Res, unsigned Op1, unsigned Op2,
                                 unsigned AddOpc, TPCII::OpType Type, const TargetRegisterClass* RC) {
  unsigned Res0 = MRI->createVirtualRegister(RC);
  unsigned Res1 = MRI->createVirtualRegister(RC);
  unsigned Res2 = MRI->createVirtualRegister(RC);
  unsigned Res3 = MRI->createVirtualRegister(RC);

  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res0)
      .addReg(Op1, 0, TPC::sub_0)
      .addReg(Op2, 0, TPC::sub_0)
      .addImm(Type)
      .addImm(0)
      .addReg(Res0, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res1)
      .addReg(Op1, 0, TPC::sub_1)
      .addReg(Op2, 0, TPC::sub_1)
      .addImm(Type)
      .addImm(0)
      .addReg(Res1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res2)
      .addReg(Op1, 0, TPC::sub_2)
      .addReg(Op2, 0, TPC::sub_2)
      .addImm(Type)
      .addImm(0)
      .addReg(Res2, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res3)
      .addReg(Op1, 0, TPC::sub_3)
      .addReg(Op2, 0, TPC::sub_3)
      .addImm(Type)
      .addImm(0)
      .addReg(Res3, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(TargetOpcode::REG_SEQUENCE), Res)
      .addReg(Res0)
      .addImm(TPC::sub_0)
      .addReg(Res1)
      .addImm(TPC::sub_1)
      .addReg(Res2)
      .addImm(TPC::sub_2)
      .addReg(Res3)
      .addImm(TPC::sub_3);
}

void TPCPipeliner::addDoubleRegs(MachineBasicBlock* Accum, unsigned Res, unsigned Op1, unsigned Op2,
                                 unsigned AddOpc, TPCII::OpType Type, const TargetRegisterClass* RC) {
  unsigned Res0 = MRI->createVirtualRegister(RC);
  unsigned Res1 = MRI->createVirtualRegister(RC);

  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res0)
      .addReg(Op1, 0, TPC::sub_0)
      .addReg(Op2, 0, TPC::sub_0)
      .addImm(Type)
      .addImm(0)
      .addReg(Res0, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res1)
      .addReg(Op1, 0, TPC::sub_1)
      .addReg(Op2, 0, TPC::sub_1)
      .addImm(Type)
      .addImm(0)
      .addReg(Res1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(TargetOpcode::REG_SEQUENCE), Res)
      .addReg(Res0)
      .addImm(TPC::sub_0)
      .addReg(Res1)
      .addImm(TPC::sub_1);
}
