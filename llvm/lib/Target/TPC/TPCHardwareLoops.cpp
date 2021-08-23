//===---- TPCHardwareLoops.cpp - Identify and generate hardware loops -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass identifies loops where we can generate the TPC hardware
// loop instruction.
//
// Criteria for hardware loops:
//  - Countable loops (w/ ind. var for a trip count)
//  - Try inner-most loops first
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
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
#include "llvm/IR/DebugLoc.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "hwloops"

static cl::opt<bool>
    RemoveJumps("remove-jumps", cl::Hidden,
                    cl::desc("Remove jumps before HW loops"), cl::init(false));

namespace llvm {

  FunctionPass *createTPCHardwareLoops();
  void initializeTPCHardwareLoopsPass(PassRegistry&);

} // end namespace llvm
static const char PassDescription[] = "TPC Hardware Loops";
static const char PassName[] = "tpc-hwloops";
static cl::opt<bool>
EnableTPCHardwareLoops(PassName,
  cl::Hidden,
  cl::init(true));

namespace {
struct LoopVars {
  union {
    unsigned Reg;
    unsigned ImmVal;
    float    FpVal;
  } Start;

  union {
    unsigned Reg;
    unsigned ImmVal;
    float    FpVal;
  } Boundary;

  union {
    unsigned Reg;
    unsigned ImmVal;
    float    FpVal;
  } Step;

  bool StartSel;
  bool BoundarySel;
  bool StepSel;

  unsigned PredReg;
  unsigned Polarity;
  bool IsPredicated;
  bool IsTaken;

  TPCII::CmpMode Mode;

  bool isStart(MachineOperand& MO)  const{
    if (StartSel) {
      if (!MO.isImm()) return false;
      return MO.getImm() == Start.ImmVal;
    } else {
      if (!MO.isReg()) return false;
      return MO.getReg() == Start.Reg;
    }
  }

  bool isBoundary(MachineOperand& MO)  const{
    if (BoundarySel) {
      if (!MO.isImm()) return false;
      return MO.getImm() == Boundary.ImmVal;
    } else {
      if (!MO.isReg()) return false;
      return MO.getReg() == Boundary.Reg;
    }
  }

  bool isStep(MachineOperand& MO)  const{
    if (StepSel) {
      if (!MO.isImm()) return false;
      return MO.getImm() == Step.ImmVal;
    } else {
      if (!MO.isReg()) return false;
      return MO.getReg() == Step.Reg;
    }
  }

  void dump() {
    dbgs() << "Start is ";
    if (StartSel) {
      dbgs() << Start.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Start.Reg << "\n";
    }

    dbgs() << "Boundary is ";
    if (BoundarySel) {
      dbgs() << Boundary.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Boundary.Reg << "\n";
    }

    dbgs() << "Step is ";
    if (StepSel) {
      dbgs() << Step.ImmVal << "\n";
    } else {
      dbgs() << "%vreg" << Step.Reg << "\n";
    }

    dbgs() << "Cmp mode is " << Mode << "\n";
  }
};

struct LoopStats {
  unsigned Phi;
  unsigned Iter;
  unsigned CounterReg;
  union {
    unsigned Reg;
    unsigned ImmVal;
  } Step;
  bool ImmStep;
  MachineLoop* L;

  LoopStats(unsigned Phi_, unsigned Iter_, MachineLoop* L_) : Phi(Phi_),
    Iter(Iter_), CounterReg(0), ImmStep(false), L(L_) {}
};

class TPCHardwareLoops : public MachineFunctionPass {
  MachineLoopInfo            *MLI;
  MachineRegisterInfo        *MRI;
  MachineDominatorTree       *MDT; //Needed?
  const TPCInstrInfo         *TII;

  SmallVector<MachineOperand, 8> Iterators;
  //SmallVector<MachineInstr, 4> Loops;
  std::map<MachineLoop*, LoopStats*> LoopOrder;
  std::map<unsigned, SmallVector<LoopStats*, 4>> RegToLoop;

public:
  static char ID;
  TPCHardwareLoops() : MachineFunctionPass(ID), MLI(nullptr), MRI(nullptr),
                       MDT(nullptr), TII(nullptr) {
    initializeTPCHardwareLoopsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool convertToHardwareLoop(MachineLoop *L, int& Layer);
  void setCounters(MachineLoop* L, int& Layer);
  LoopVars* calculateLoop(MachineLoop *L);

  StringRef getPassName() const override { return "TPC Hardware Loops"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  bool checkLayout(MachineLoop *L);
  TPCII::CmpMode  getCmpMode(unsigned Opcode) const;
  TPCII::CmpMode  invertCond(TPCII::CmpMode mode) const;
  TPCII::CmpMode  swapCond(TPCII::CmpMode mode) const;
  bool     appropriateInst(unsigned Opcode) const;
  unsigned chooseLoopOpcode(LoopVars* lv) const;
  MachineBasicBlock* stripEntranceCheck(MachineBasicBlock* Preheader, MachineBasicBlock* Header, MachineBasicBlock* Exit, LoopVars* Lv);
  bool predicateLoop(MachineBasicBlock* Preheader, MachineBasicBlock* Exit, MachineBasicBlock* PredTarget, LoopVars* Lv);
};
}

INITIALIZE_PASS_BEGIN(TPCHardwareLoops, PassName,
                      PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCHardwareLoops, PassName,
                    PassDescription, false, false)

FunctionPass *llvm::createTPCHardwareLoops() {
  return new TPCHardwareLoops();
}

bool TPCHardwareLoops::checkLayout(MachineLoop *L) {
  MachineBasicBlock* Latch = L->getLoopLatch();
  MachineBasicBlock* Exit = L->getExitBlock();

  assert(Exit && "We should've checked earlier that the loop has only one exit");

  // There's a pass that set numbers to blocks in order they will be emitted.
  // But it's launched much later. In theory we can bring it before loop conversion
  // by adding it to the dependency list. But for now it looks like we don't need
  // such a strict check
//  int StartNum = Start->getNumber();
//  int EndNum = Latch->getNumber();
//  int ExitNum = Exit->getNumber();
//
//  if (ExitNum < StartNum)
//    return false;
//
//  if (EndNum < StartNum)
//    return false;
//
//  for (int i = StartNum; i < EndNum; ++i) {
//    MachineBasicBlock* NextLayout = Start->getParent()->getBlockNumbered(i);
//    // The loop should be layouted contiguously.
//    if (!L->contains(NextLayout)) {
//      return false;
//    }
//  }

  // The loop exit should be layouted just after the loop
  if (!Latch->isLayoutSuccessor(Exit)) {
    return false;
  }

  int OnlyOneExit = 0;
  for (auto B : L->getBlocks()) {
    if (L->isLoopExiting(B)) OnlyOneExit++;
    if (OnlyOneExit > 1) return false;
  }

  return true;
}

LoopVars* TPCHardwareLoops::calculateLoop(MachineLoop *L) {
  MachineBasicBlock *Latch = L->getLoopLatch();
  MachineBasicBlock *ExitingBlock = L->findLoopControlBlock();
  MachineBasicBlock* ExitBlock = L->getExitBlock();

  LLVM_DEBUG(dbgs() << "Calculate loop\n");

  // Loops with several backedges can't be converted
  if (!Latch)
    return nullptr;

  // Don't generate hw loop if the loop has more than one exit.
  if (!ExitingBlock)
    return nullptr;

  if (!ExitBlock)
    return nullptr;

  if (!checkLayout(L))
    return nullptr;

  typedef MachineBasicBlock::reverse_instr_iterator rev_interator;

  MachineOperand* Pred = nullptr;

  rev_interator I = Latch->instr_rbegin();

  bool InvertCond = false;
  MachineInstr* Terminator = nullptr;

  for (; I != Latch->instr_rend(); ++I) {
    Terminator = &*I;

    if (Terminator->getOpcode() == TPC::JMPR) {
      //This is the backedge that we need. Extract predicate from it.
      Pred = &Terminator->getOperand(1);
      MachineBasicBlock* Block1 = Terminator->getOperand(0).getMBB();
      if (Terminator->getOperand(2).getImm() != 0) {
        if (L->contains(Block1))
          InvertCond = true;
      } else {
        if (!L->contains(Block1))
          InvertCond = true;
      }
      break;
    }
  }

  // If there's no conditional jump inside the loop, it's a precondition loop
  if (!Pred)
    return nullptr;

  MachineInstr* Condition = MRI->getVRegDef(Pred->getReg());
  assert(Condition && "Multidef of a predicate in SSA form");
  if (Condition->isCopy()) {
    Condition = MRI->getVRegDef(Condition->getOperand(1).getReg());
  }
  if (!L->contains(Condition)) {
    return nullptr;
  }
  assert(L->contains(Condition) && "Condition for backedge is not in the loop");

  LLVM_DEBUG(dbgs() << "Found condition:\n");
  LLVM_DEBUG(Condition->dump());
  LLVM_DEBUG(dbgs() << "\n");

  LoopVars* Lv = new LoopVars();
  Lv->IsPredicated = false;
  Lv->Mode = getCmpMode(Condition->getOpcode());

  // Sometimes predicate is calculated as AND of several conditions, not much we can do here.
  if (Lv->Mode == TPCII::LoopErr) {
    delete Lv;
    return nullptr;
  }
  if (InvertCond) Lv->Mode = invertCond(Lv->Mode);

  // Get iterator and boundary from CMP instruction.
  MachineOperand& Op1 = Condition->getOperand(1);
  MachineOperand& Op2 = Condition->getOperand(2);

  MachineOperand* Iterator;
  MachineOperand* Boundary;

  if (!Op1.isReg()) {
    Boundary = &Op1;
    Iterator = &Op2;
    if (Lv->Mode != TPCII::LoopNE) { // != is processed separately
      Lv->Mode = swapCond(Lv->Mode); // Boundary COND Iterator, invert this to Iterator COND Boundary
    }
    if (Op1.isFPImm()) {
      Lv->Boundary.FpVal = Op1.getFPImm()->getValueAPF().convertToFloat();
    } else {
      Lv->Boundary.ImmVal = Op1.getImm();
    }
    Lv->BoundarySel = true;
  } else if (!Op2.isReg()) {
    Boundary = &Op2;
    Iterator = &Op1;
    if (Op2.isFPImm()) {
      Lv->Boundary.FpVal = Op2.getFPImm()->getValueAPF().convertToFloat();
    } else {
      Lv->Boundary.ImmVal = Op2.getImm();
    }
    Lv->BoundarySel = true;
  } else if (Op1.isReg() && Op2.isReg()) {
    // Shared iterator between loops
    if (!MRI->getVRegDef(Op1.getReg()) || ! MRI->getVRegDef(Op2.getReg())) {
      delete Lv;
      return nullptr;
    }
    if (!L->contains(MRI->getVRegDef(Op1.getReg()))) {
      if (!L->contains(MRI->getVRegDef(Op2.getReg()))) {
        delete Lv;
        return nullptr;
      }
      Iterator = &Op2;
      Boundary = &Op1;
      if (Lv->Mode != TPCII::LoopNE) { // != is processed separately
        Lv->Mode = swapCond(Lv->Mode); // Boundary COND Iterator, invert this to Iterator COND Boundary
      }
    } else if (!L->contains(MRI->getVRegDef(Op2.getReg()))) {
      if (!L->contains(MRI->getVRegDef(Op1.getReg()))) {
        delete Lv;
        return nullptr;
      }
      Iterator = &Op1;
      Boundary = &Op2;
    } else {
      delete Lv;
      return nullptr;
    }

    Lv->Boundary.Reg = Boundary->getReg();
    Lv->BoundarySel = false;

    MachineInstr* BoundaryDef = MRI->getVRegDef(Boundary->getReg());
    if (L->contains(BoundaryDef)) {
      // TODO: If at this point loop invariants are not moved out of the loop
      // we should check wether boundary def is a real invariant and move it
      // ourselfs. If it's not possible, the loop can't be converted.
      delete Lv;
      return nullptr;
    }
  } else {
    assert(false && "This loop should be optimized out");
  }

  LLVM_DEBUG(dbgs() << "Boundary is ");
  LLVM_DEBUG(Boundary->dump());
  LLVM_DEBUG(dbgs() << "\n");

  // Search for the iterator increment. I think it should be in the latch block.
  // But maybe not.
  MachineInstr* IteratorStep = MRI->getVRegDef(Iterator->getReg());
  LLVM_DEBUG(dbgs() << "Processing iterator:\n");
  LLVM_DEBUG(IteratorStep->dump());
  LLVM_DEBUG(dbgs() << "\n");

  if (!appropriateInst(IteratorStep->getOpcode())) {
    delete Lv;
    LLVM_DEBUG(dbgs() << "No approptiate increment\n");
    return nullptr;
  }

  assert(L->contains(IteratorStep) && "Iterator increment is not in the loop");

  // TODO: Below I assume that step is the second operand and iterator is the first one.
  //       This is not necessary true.
  // Find the step
  MachineOperand& Step = IteratorStep->getOperand(2);
  MachineOperand& StartPhi = IteratorStep->getOperand(1);
  if (Step.isReg()) {
    MachineInstr* StepDef = MRI->getVRegDef(Step.getReg());

    if (L->contains(StepDef)) {
      // TODO: again, there should be no definitions of step in the loop
      delete Lv;
      return nullptr;
    }
    Lv->Step.Reg = Step.getReg();
    Lv->StepSel = false;
  } else {
    // Step is immediate, nothing to do except write it down
    if (Step.isFPImm()) {
      Lv->Step.FpVal = Step.getFPImm()->getValueAPF().convertToFloat();
    } else {
      Lv->Step.ImmVal = Step.getImm();
    }
    Lv->StepSel = true;
  }

  LLVM_DEBUG(dbgs() << "Step is ");
  LLVM_DEBUG(Step.dump());
  LLVM_DEBUG(dbgs() << "\n");

  // Find the start
  assert(StartPhi.isReg() && "Iterator can't be a constant");
  MachineInstr* Phi = MRI->getVRegDef(StartPhi.getReg());

  // More complex increment. Try to make it into hardware loop anyway
  // by finding a chain of adds with constant increments
  if (!Phi->isPHI()) {
    if (!Lv->StepSel) {
      delete Lv;
      return nullptr;
    }

    while(!Phi->isPHI()) {
      if (!TPCII::isAdd(Phi->getDesc())) {
        delete Lv;
        return nullptr;
      }

      if (Phi->getOperand(2).isImm()) {
        Lv->Step.ImmVal += Phi->getOperand(2).getImm();
        Phi = MRI->getVRegDef(Phi->getOperand(1).getReg());
      } else if (Phi->getOperand(1).isImm()) {
        Lv->Step.ImmVal += Phi->getOperand(1).getImm();
        Phi = MRI->getVRegDef(Phi->getOperand(2).getReg());
      } else {
        delete Lv;
        return nullptr;
      }
    }
  }

  assert(Phi->isPHI() && "Iterator should be a phi node");
  assert(L->contains(Phi) && "Phi node for iterator should be inside the loop");

  bool IVinPhi = false;
  for (MachineOperand& MUse : Phi->uses()) {
    if (MUse.isIdenticalTo(*Iterator)) {
      IVinPhi = true;
      break;
    }
  }

  // Some sort of double inductive variable. We can't link iterator increment
  // with initial increment value through a phi node.
  // TODO: it still may be possible to form hw loop, just more clever
  //       analysis is required
  if (!IVinPhi) {
    delete Lv;
    return nullptr;
  }

  LLVM_DEBUG(dbgs() << "Phi node:\n");
  LLVM_DEBUG(Phi->dump());
  LLVM_DEBUG(dbgs() << "\n");

  MachineOperand PhiOp1 = Phi->getOperand(1);
  MachineOperand PhiOp2 = Phi->getOperand(3);

  MachineBasicBlock* PhiBlock1 = Phi->getOperand(2).getMBB();

  MachineOperand Start = L->contains(PhiBlock1) ? PhiOp2 : PhiOp1;

  if (Start.isReg()) {
    MachineInstr* StartDef = MRI->getVRegDef(Start.getReg());
    if (StartDef->getOpcode() == TPC::MOVsip && !TII->isPredicated(*StartDef)) {
      // TODO: it would be good to remove the mov if it's not used.
      // But that leaves the vreg without any usage. LLVM crashes because of that.
      // I didn't find a way to delete a vreg yet. It looks like DCE and other
      // passes that delete instructions operate on the higher level IR,
      // not on this representation.
//      bool MultiUse = false;
//      for (MachineInstr& StartUser : MRI->use_nodbg_instructions(Start.getReg())) {
//        if (&StartUser != Phi) {
//          MultiUse = true;
//          break;
//        }
//      }

//      if (!MultiUse) {
//        StartDef->removeFromParent();
//      }
      Start = StartDef->getOperand(1);
    }
  }


  if (Start.isReg()) {
    MachineInstr* StartDef = MRI->getVRegDef(Start.getReg());
    (void) StartDef;
    assert(L->contains(StartDef) == false &&
           "Both inputs for phi are inside the loop, that's ridiculous");
    // Start is a register, write it down as such
    Lv->Start.Reg = Start.getReg();
    Lv->StartSel = false;
  } else {
    // Start is an immidiate, write it down as such
    if (Start.isFPImm()) {
      Lv->Start.FpVal = Start.getFPImm()->getValueAPF().convertToFloat();
    } else {
      Lv->Start.ImmVal = Start.getImm();
    }
    Lv->StartSel = true;
  }

  // Try to get rid of != condition, it's to specific and prevents futher optimizations
  if (Lv->Mode == TPCII::LoopNE) {
    if (Lv->StepSel) {
      int StepImm = Lv->Step.ImmVal;
      if (StepImm > 0) Lv->Mode = TPCII::LoopLT;
      else             Lv->Mode = TPCII::LoopGT;
    }
  }

  LLVM_DEBUG(dbgs() << "Start is ");
  LLVM_DEBUG(Start.dump());
  LLVM_DEBUG(dbgs() << "\n");

  LLVM_DEBUG(dbgs() << "Return LV\n");

  // Now that we're definetely going to convert this loop, remove all useless instructions
  bool ConditionReuse = false;
  LLVM_DEBUG(dbgs() << "Trying to remove "); LLVM_DEBUG(Condition->dump(););
  LLVM_DEBUG(Terminator->dump());
  int num_uses = 0;
  for (MachineInstr &UseMI : MRI->use_instructions(Condition->getOperand(0).getReg())) {
    (void) UseMI;
    LLVM_DEBUG(dbgs() << "UseMI: ");
    LLVM_DEBUG(UseMI.dump());
    ++num_uses;
    if (num_uses > 1) {
      ConditionReuse = true;
    }
  }
  if (!ConditionReuse) {
    LLVM_DEBUG(dbgs() << "Removing "); LLVM_DEBUG(Condition->dump(););
    Condition->removeFromParent();
  }

  bool IteratorReuse = false;
  for (MachineInstr &UseMI : MRI->use_instructions(Iterator->getReg())) {
    if ((L->contains(&UseMI) && !UseMI.isPHI())) {
      IteratorReuse = true;
      break;
    }
  }

  if (!IteratorReuse) {
    LLVM_DEBUG(dbgs() << "Removing "); LLVM_DEBUG(IteratorStep->dump(););
    IteratorStep->removeFromParent();
  }
  Phi->removeFromParent();

  // Replace all occurances of iterator with an SRF register
  LoopStats* Stats = new LoopStats(StartPhi.getReg(), Iterator->getReg(), L);
  if (Lv->StepSel) {
    Stats->ImmStep = true;
    Stats->Step.ImmVal = Lv->Step.ImmVal;
  } else {
    Stats->Step.Reg = Lv->Step.Reg;
  }

  LoopOrder[L] = Stats;
  LLVM_DEBUG(dbgs() << "Push phi and iterator:\n ");
  LLVM_DEBUG(StartPhi.dump());
  LLVM_DEBUG(Iterator->dump());

  return Lv;
}

void TPCHardwareLoops::setCounters(MachineLoop* L, int& Layer) {
  if (LoopOrder.count(L) > 0) {
    ++Layer;
    LoopOrder[L]->CounterReg = TPC::S32 + Layer;
    RegToLoop[TPC::S32 + Layer].push_back(LoopOrder[L]);
  }

  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    setCounters(*I, Layer);
  }

  if (LoopOrder.count(L) > 0) {
    --Layer;
  }
}

bool TPCHardwareLoops::runOnMachineFunction(MachineFunction &MF) {
  if(!EnableTPCHardwareLoops)
    return false;
  LLVM_DEBUG(dbgs() << "********* TPC Hardware Loops *********\n");
  LLVM_DEBUG(MF.dump());
  bool Changed = false;

  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  MDT = &getAnalysis<MachineDominatorTree>();
  TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();

  for (auto &L : *MLI) {
    if (!L->getParentLoop()) {
      int Layer = 0;
      Changed |= convertToHardwareLoop(L, Layer);
    }
  }

  // Now set all counter registers
  for (auto &L : *MLI) {
    if (!L->getParentLoop()) {
      int Layer = -1;
      setCounters(L, Layer);
    }
  }

  // Replace iterators with special registers.
  for (auto const& MapEntry: LoopOrder) {
    LoopStats* Stats = MapEntry.second;

    if (!MRI->getVRegDef(Stats->Iter)) {
      MRI->replaceRegWith(Stats->Iter, Stats->CounterReg);
    }
    MRI->replaceRegWith(Stats->Phi, Stats->CounterReg);
  }

  // Fix any phi-node and tied registers that we broke
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock* Block = &*I;
    for (MachineInstr& MI : Block->instrs()) {

      if (MI.isPHI()) {
        int Idx = 1;
        for (MachineOperand& MO : MI.uses()) {
          if (MO.isReg() && MO.getReg().isPhysical()) {
            MachineBasicBlock* Origin = MI.getOperand(Idx + 1).getMBB();

            MachineBasicBlock::iterator InsertPos = Origin->end();
            MachineBasicBlock::iterator LastInst = --(Origin->end());
            while(InsertPos != Origin->begin() && (*LastInst).isTerminator()) {
              --InsertPos;
              --LastInst;
            }
            auto LSVector = RegToLoop[MO.getReg()];
            bool PhiFromLoop = false;
            LoopStats* MatchedLoop = nullptr;
            for (LoopStats* LS : LSVector) {
              if (LS->L->contains(Origin)) {
                PhiFromLoop = true;
                MatchedLoop = LS;
                break;
              }
            }
            unsigned SafeReg =  MRI->createVirtualRegister(MF.getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
            if (PhiFromLoop && !MatchedLoop->L->contains(&MI)) {
              if (MatchedLoop->ImmStep) {
                BuildMI(*Origin, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), SafeReg)
                    .addReg(MO.getReg())
                    .addImm(MatchedLoop->Step.ImmVal)
                    .addImm(TPCII::OpType::INT32)
                    .addImm(0)
                    .addReg(MO.getReg(), RegState::Undef)
                    .addReg(TPC::SP0)
                    .addImm(0);
              } else {
                BuildMI(*Origin, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), SafeReg)
                    .addReg(MO.getReg())
                    .addReg(MatchedLoop->Step.Reg)
                    .addImm(TPCII::OpType::INT32)
                    .addImm(0)
                    .addReg(MO.getReg(), RegState::Undef)
                    .addReg(TPC::SP0)
                    .addImm(0);
              }
            } else {
              BuildMI(*Origin, InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY), SafeReg).addReg(MO.getReg());
            }
            MO.setReg(SafeReg);
          }
          ++Idx;
        }
      } else {
        for (MachineOperand& MO: MI.uses()) {
          if (MO.isReg() && MO.isTied() && MO.getReg().isPhysical()) {
            unsigned SafeReg =  MRI->createVirtualRegister(MF.getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
            MachineBasicBlock::iterator InsertPos(MI);
            BuildMI(*MI.getParent(), InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY), SafeReg).addReg(MO.getReg());
            MO.setReg(SafeReg);
            LLVM_DEBUG(dbgs() << "Doing something with tied operands ");
            LLVM_DEBUG(MI.dump());
          }
        }
      }
    }
  }


  LLVM_DEBUG(dbgs() << "After convertation\n");
  LLVM_DEBUG(MF.dump());

  for (auto const &MapEntry : RegToLoop) {
    for(LoopStats* LS : MapEntry.second) {
      delete LS;
    }
  }

  return Changed;
}

bool TPCHardwareLoops::convertToHardwareLoop(MachineLoop *L,
                                             int &Layer) {
  LLVM_DEBUG(dbgs() << "Convert loop\n");
  LLVM_DEBUG(L->dump());

  // This is just for sanity.
  assert(L->getHeader() && "Loop without a header?");

  bool Changed = false;

  // Process nested loops first.
  int MaxLayer = Layer;
  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    int TempLayer = Layer;
    Changed |= convertToHardwareLoop(*I, TempLayer);
    if (TempLayer > MaxLayer) {
      MaxLayer = TempLayer;
    }
  }
  Layer = MaxLayer;

  // Only 4 innermost loops can be transformed.
  if (Layer >= 4)
    return Changed;

  LoopVars* Lv = calculateLoop(L);
  if (!Lv) {
    return false;
  }

  MachineBasicBlock *Header = L->getHeader();
  MachineBasicBlock *Latch = L->getLoopLatch();

  MDNode* LoopMD = Latch->getBasicBlock()->getTerminator()->getMetadata(LLVMContext::MD_loop);
  Lv->IsTaken = false;

  if (LoopMD) {
    MDNode *MD = nullptr;

    for (unsigned i = 1, e = LoopMD->getNumOperands(); i < e; ++i) {
      MD = dyn_cast<MDNode>(LoopMD->getOperand(i));
      if (!MD)
        continue;

      MDString *S = dyn_cast<MDString>(MD->getOperand(0));
      if (!S)
        continue;

      if (S->getString().equals("llvm.loop.taken")) {
        assert(MD->getNumOperands() == 2 &&
               "Taken hint metadata should have two operands.");
        Lv->IsTaken =
            !(mdconst::extract<ConstantInt>(MD->getOperand(1))->isZero());
      }
    }
  }


  LLVM_DEBUG(Lv->dump());

  typedef MachineBasicBlock::reverse_instr_iterator rev_interator;
  rev_interator I = Latch->instr_rbegin();
  MachineInstr *Terminator = &*I;

  // Remove terminators
  if (Terminator->getOpcode() == TPC::JMPR_u) {
    Latch->remove(Terminator);

    I = Latch->instr_rbegin();
    Terminator = &*I;

    assert((Terminator->getOpcode() == TPC::JMPR) &&
           "Regular loop should end with a conditional branch");

    Latch->remove(Terminator);
  } else {
    assert((Terminator->getOpcode() == TPC::JMPR) &&
           "Regular loop should end with a conditional branch");

    Latch->remove(Terminator);
  }

  MachineBasicBlock::iterator InsertPos;
  MachineBasicBlock* LoopBlock = nullptr;

  MachineBasicBlock* Preheader = nullptr;
  typedef std::vector<MachineBasicBlock*> MBBVector;
  MBBVector Preds(Header->pred_begin(), Header->pred_end());

  for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
    MachineBasicBlock *PB = *I;
    if (PB != Latch) {
      Preheader = PB;
    }
  }
  assert(Preheader && "Unreachable loop");
 
  MachineBasicBlock* NewPreheader = stripEntranceCheck(Preheader, Header, L->getExitBlock(), Lv);
  if (NewPreheader) {
    InsertPos = NewPreheader->end();
    LoopBlock = NewPreheader;
  } else if (Preheader->succ_size() == 1) {
    InsertPos = Preheader->end();
    LoopBlock = Preheader;
  } else {
    MachineFunction* MF = Header->getParent();
    MachineBasicBlock* Prolog = MF->CreateMachineBasicBlock();
    MF->insert(Header->getIterator(), Prolog);

    // Insert loop pre-header for the prolog and rewire CFG.
    for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
      MachineBasicBlock *PB = *I;
      if (PB != Latch) {
        //Patch Phis
        for (MachineInstr& MI : Header->instrs()) {
          if (MI.isPHI()) {
            for (MachineOperand& MO: MI.uses()) {
              if (MO.isMBB() && MO.getMBB() == PB) {
                MO.setMBB(Prolog);
              }
            }
          }
        }

        PB->ReplaceUsesOfBlockWith(Header, Prolog);
      }
    }

    Prolog->addSuccessor(Header, BranchProbability::getOne());
    MachineLoop *ParentLoop = L->getParentLoop();
    if (ParentLoop && !MLI->getLoopFor(Prolog)) {
      ParentLoop->addBasicBlockToLoop(Prolog, MLI->getBase());
    }

    InsertPos = Prolog->end();
    LoopBlock = Prolog;
  }

  //DebugLoc DL = Prolog->begin()->getDebugLoc();

  unsigned LOOP_opc = chooseLoopOpcode(Lv);
  const MachineInstrBuilder& MIB = BuildMI(*LoopBlock, InsertPos, DebugLoc(), TII->get(LOOP_opc));

  if (Lv->StartSel) {
    MIB.addImm(Lv->Start.ImmVal);
  } else {
    MIB.addReg(Lv->Start.Reg);
  }

  if (Lv->BoundarySel) {
    MIB.addImm(Lv->Boundary.ImmVal);
  } else {
    MIB.addReg(Lv->Boundary.Reg);
  }

  if (Lv->StepSel) {
    MIB.addImm(Lv->Step.ImmVal);
  } else {
    MIB.addReg(Lv->Step.Reg);
  }

  MIB.addImm(Lv->Mode);
  MachineBasicBlock* Block = L->getExitBlock();
  assert(Block);
  MIB.addMBB(Block);
  Block->setHasAddressTaken();

  if (Lv->IsPredicated) {
    MIB.addReg(Lv->PredReg);
    MIB.addImm(Lv->Polarity);
  }
  InsertPos = Latch->end();
 //DL = InsertPos->getDebugLoc();
  const MachineInstrBuilder& MEndLoop = BuildMI(*Latch, InsertPos, DebugLoc(), TII->get(TPC::LOOPEND));
  MEndLoop.addMBB(Block);
  MEndLoop.addMBB(Header);
// Below issue is done for G-1819
// it is temporaly hidden due to new exposed problem G-1926
 #if 0 
  // need for unhardware to add upper,step
  if (!Lv->BoundarySel) {
    MEndLoop.addReg(Lv->Boundary.Reg, RegState::Implicit);
  }
  
  if (!Lv->StepSel) {
    MEndLoop.addReg(Lv->Step.Reg, RegState::Implicit);
  } 
#endif
  if (LoopMD) {
    MEndLoop.addMetadata(LoopMD);
  }
 
  ++Layer;
  delete Lv;
  return true;
}

bool TPCHardwareLoops::predicateLoop(MachineBasicBlock* Preheader, MachineBasicBlock* Exit, MachineBasicBlock* PredTarget, LoopVars* Lv) {
  MachineInstr *Terminator = &(*(--Preheader->end()));

  if (Terminator->getOpcode() == TPC::JMPR) {
    if (Terminator->getOperand(0).getMBB() != Exit) {
      return false;
    }
    Lv->PredReg = Terminator->getOperand(1).getReg();
    Lv->Polarity = !Terminator->getOperand(2).getImm();
    Lv->IsPredicated = true;
  }
  // There's a preheader block that's not part of the loop, but under jump guard, make every instruction there predicated
//  if (PredTarget != nullptr) {
//    for (MachineInstr& MI : PredTarget->instrs()) {
//      unsigned PredPos = MI.getNumOperands() - 2; // All instructions except pseudos have predicate at the penultimate position
//      if(MI.getNumOperands() > 3) { // pred_link, pred, polarity
//        MI.getOperand(PredPos).setReg(Terminator->getOperand(1).getReg());
//      }
//    }
//  }
  Terminator->removeFromParent();
  return true;
}

// Attempts to remove conditions and jumps that guard a post-condition loop
MachineBasicBlock* TPCHardwareLoops::stripEntranceCheck(MachineBasicBlock* Preheader, MachineBasicBlock* Header, MachineBasicBlock* Exit, LoopVars* Lv) {
  bool ShouldMerge = false;
  bool Predicated  = false;

  MachineBasicBlock* LoopEntrance = Header;
  MachineBasicBlock* PredTarget = nullptr;
  if (Preheader->empty()) {
    // TODO: guarding condition is in another block. We should attempt to find it anyway.
    //       This single if might not be enough
    if (Preheader->pred_size() == 1) {
      LoopEntrance = Preheader;
      Preheader = *(Preheader->predecessors().begin());
      ShouldMerge = true;
    } else {
      return nullptr;
    }
  }

  MachineInstr *Terminator = &(*(--Preheader->end()));
  // Guarding jump may be in predecessor of preheader if preheader has some code hoisted to it
  if (Terminator->getOpcode() != TPC::JMPR) {
    // We remove jumps around loop only if there's loop_taken pragma meaning that
    // the loop is executed at least once. We can do the optimization for any loop
    // but that requires altering instructions:
    // In one scenario guarding jump falls through right into the loop body and LOOP
    // instruction will perform the check. In the other scenario there's some
    // preparation code bedore loop body that is not a part of the loop and guarding
    // jump falls through to that. If we remove jump and the loop is never executed
    // we also should not execute that prolog. In order to do this we have to
    // predicate everything in the prolog. There're two problems with that:
    //   1. Load and Store slot are currently can't share predicates and will not
    //      be bundled together
    //   2. We can't predicate pseudo instructions and delay-predicate phi nodes.
    //      This problem causes some tests to fail because when later phi-nodes
    //      are eliminated resulted MOVs should be predicated but LLVM can't do that.
    //      As a result some values are overwritten.
    if (!Lv->IsTaken) {
      return nullptr;
    }

    // Preheader must be just a connector block to a loop body
    if (Preheader->succ_size() == 1 && Preheader->pred_size() == 1) {
      LoopEntrance = Preheader;
      PredTarget = Preheader;
      Preheader = *(Preheader->predecessors().begin());
      Terminator = &(*(--Preheader->end()));
      ShouldMerge = true;
    }
  }

  // TODO: there are some weird layouts where preheader jumps to the successor of the exit
  if (!Preheader->isSuccessor(Exit)) {
    return nullptr;
  }

  if (Terminator->getOpcode() == TPC::JMPR) {
    MachineInstr* Condition = MRI->getVRegDef(Terminator->getOperand(1).getReg());
    if (Condition->isCopy() || Condition->getOpcode() == TPC::MOVpsp) {
      // Try looking further
      // TODO: What if it's still not a cmp
      Condition = MRI->getVRegDef(Condition->getOperand(1).getReg());
    }

    //LLVM_DEBUG(dbgs() << "Condition: "; Condition->dump());
    LLVM_DEBUG(dbgs() << "Terminator: "; Terminator->dump());
    LLVM_DEBUG(Preheader->getParent()->dump());

    MachineOperand& Op1 = Condition->getOperand(1);
    MachineOperand& Op2 = Condition->getOperand(2);

    // Check that preheader actually jumps to the exit
    typedef std::vector<MachineBasicBlock*> MBBVector;
    MBBVector Succs(Preheader->succ_begin(), Preheader->succ_end());

    if (Succs.size() != 2) {
      return nullptr;
    }

    if (Succs[0] == LoopEntrance) {
      if (Succs[1] != Exit) {
        return nullptr;
      }
    } else if (Succs[0] == Exit) {
      if (Succs[1] != LoopEntrance) {
        return nullptr;
      }
    } else {
      //assert(false && "One of the successors of the preheader must be the header");
    }
    LLVM_DEBUG(dbgs() << "Checked CFG\n");

    // Check that cmp instruction involves both start and boundary of the loop
    if (!RemoveJumps) {
      LLVM_DEBUG(dbgs() << "Op1 "; Op1.dump());
      LLVM_DEBUG(dbgs() << "Op2 "; Op2.dump());
      if (Lv->isStart(Op1)) {
        if (!Lv->isBoundary(Op2)) {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
        }
      } else if (Lv->isBoundary(Op1)) {
        if (!Lv->isStart(Op2)) {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
        }
      } else {
          if (predicateLoop(Preheader, Exit,  PredTarget, Lv)) {
            Predicated = true;
          } else {
            return nullptr;
          }
      }
    }
  } else {
    return nullptr;
  }

  if (!Predicated) {
    LLVM_DEBUG(dbgs() << "Checked operands\n");
    // If we are here, we can safely eliminate jump and merge blocks
    MachineInstr* Condition = MRI->getVRegDef(Terminator->getOperand(1).getReg());
    bool ConditionReuse = false;
    LLVM_DEBUG(dbgs() << "Trying to remove "); LLVM_DEBUG(Condition->dump(););
    LLVM_DEBUG(Terminator->dump());

    int num_uses = 0;
    for (MachineInstr &UseMI : MRI->use_instructions(Condition->getOperand(0).getReg())) {
    (void) UseMI;
      LLVM_DEBUG(dbgs() << "UseMI: ");
      LLVM_DEBUG(UseMI.dump());
      ++num_uses;
      if (num_uses > 1) {
        ConditionReuse = true;
      }
    }
    if (!ConditionReuse) {
      LLVM_DEBUG(dbgs() << "Removing "); LLVM_DEBUG(Condition->dump(););
      Condition->removeFromParent();
    }
    Terminator->removeFromParent();
  }

  if (ShouldMerge) {
    assert((LoopEntrance->empty() || !(*LoopEntrance->begin()).isPHI()) && "Connector block can't have phis");
    MachineBasicBlock* LoopBody = *LoopEntrance->succ_begin();
    Preheader->splice(Preheader->end(), LoopEntrance, LoopEntrance->begin(), LoopEntrance->end());
    Preheader->removeSuccessor(LoopEntrance);
    Preheader->removeSuccessor(Exit);
    Preheader->addSuccessor(*LoopEntrance->succ_begin());
    LoopEntrance->removeSuccessor(LoopEntrance->succ_begin());

    for (MachineInstr& MI : LoopBody->instrs()) {
      if (!MI.isPHI()) {
        break;
      }

      for (MachineOperand& MO : MI.uses()) {
        if (MO.isMBB() && MO.getMBB() == LoopEntrance) {
          MO.setMBB(Preheader);
        }
      }
    }

    LoopEntrance->eraseFromParent();

    // Fix phis because loop is always taken, can't get to exit from Preheader
    auto InsertPos = Exit->begin();
    while(InsertPos != Exit->end() && (*InsertPos).isPHI()) ++InsertPos;

    SmallVector<MachineInstr*, 4> Removal;
    for (MachineInstr& MI : *Exit) {
      if (!MI.isPHI()) {
        break;
      }

      bool Processed = true;

      // We can add phi-nodes inside this loop. Need to check whether this is
      // new or old phi. This is bad, but I don't know a better solution.
      for (unsigned i = 1; i < MI.getNumOperands(); ++i) {
        if (MI.getOperand(i).isMBB() && MI.getOperand(i).getMBB() == Preheader) {
          Processed = false;
          break;
        }
      }

      if (Processed) {
        continue;
      }

      // 2-way merge, replace phi with copy
      if (MI.getNumOperands() == 5) {
        unsigned UseReg = 0;
        unsigned DefReg = MI.getOperand(0).getReg();
        if (MI.getOperand(2).getMBB() == Preheader) {
          UseReg = MI.getOperand(3).getReg();
        } else {
          UseReg = MI.getOperand(1).getReg();
        }
        Removal.push_back(&MI);
        BuildMI(*Exit, InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY), DefReg)
            .addReg(UseReg);
      } else { // Exit has some other predecessors, remove Preheader case from phi
        unsigned DefReg = MI.getOperand(0).getReg();
        for (unsigned i = 1; i < MI.getNumOperands(); ++i) {
          if (MI.getOperand(i).isMBB() && MI.getOperand(i).getMBB() == Preheader) {
            MachineInstrBuilder BMI = BuildMI(*Exit, InsertPos, DebugLoc(), TII->get(TargetOpcode::PHI), DefReg);
            for (unsigned j = 1; j < MI.getNumOperands(); ++j) {
              if (j == i - 1) {
                ++j;
                continue;
              }
              BMI.addReg(MI.getOperand(j++).getReg());
              BMI.addMBB(MI.getOperand(j).getMBB());
            }
            Removal.push_back(&MI);
            break;
          }
        }
      }
    }

    for (MachineInstr* MI : Removal) {
      MI->removeFromParent();
    }

  }

  return Preheader;
}

char TPCHardwareLoops::ID = 0;

TPCII::CmpMode TPCHardwareLoops::getCmpMode(unsigned Opcode) const {
  switch (Opcode) {
    case TPC::CMP_EQssp:
    case TPC::CMP_EQsip:
      return TPCII::LoopEQ;
    case TPC::CMP_NEQssp:
    case TPC::CMP_NEQsip:
      return TPCII::LoopNE;
    case TPC::CMP_LESSssp:
    case TPC::CMP_LESSsip:
      return TPCII::LoopLT;
    case TPC::CMP_LEQssp:
    case TPC::CMP_LEQsip:
      return TPCII::LoopLE;
    case TPC::CMP_GRTssp:
    case TPC::CMP_GRTsip:
      return TPCII::LoopGT;
    case TPC::CMP_GEQssp:
    case TPC::CMP_GEQsip:
      return TPCII::LoopGE;
    default:
      return TPCII::LoopErr;
  }
}

TPCII::CmpMode TPCHardwareLoops::invertCond(TPCII::CmpMode mode) const {
  switch(mode) {
    case TPCII::LoopEQ: return TPCII::LoopNE;
    case TPCII::LoopNE: return TPCII::LoopEQ;
    case TPCII::LoopLT: return TPCII::LoopGE;
    case TPCII::LoopLE: return TPCII::LoopGT;
    case TPCII::LoopGT: return TPCII::LoopLE;
    case TPCII::LoopGE: return TPCII::LoopLT;
    default: llvm_unreachable("Incorrect cmp mode");
  }
}

TPCII::CmpMode TPCHardwareLoops::swapCond(TPCII::CmpMode mode) const {
  switch(mode) {
    case TPCII::LoopEQ: return TPCII::LoopEQ;
    case TPCII::LoopNE: return TPCII::LoopNE;
    case TPCII::LoopLT: return TPCII::LoopGT;
    case TPCII::LoopLE: return TPCII::LoopGE;
    case TPCII::LoopGT: return TPCII::LoopLT;
    case TPCII::LoopGE: return TPCII::LoopLE;
    default: llvm_unreachable("Incorrect cmp mode");
  }
}

// TODO: this is too cumbersome. Plus there is a possibility that if-conversion
// already happened, so we need to take into account predicated instructions.
// Also intrinsics.
bool TPCHardwareLoops::appropriateInst(unsigned Opcode) const {
  switch (Opcode) {
    case TPC::ADDssp:
    case TPC::ADDsip:
      return true;
    default:
      return false;
  }
}

unsigned TPCHardwareLoops::chooseLoopOpcode(LoopVars* lv) const {
  unsigned mask = 0;
  if (lv->StartSel)     mask |= 1;
  if (lv->BoundarySel)  mask |= 2;
  if (lv->StepSel)      mask |= 4;
  if (lv->IsPredicated) mask |= 8;

  switch (mask) {
    case 0: return TPC::LOOPsss;
    case 1: return TPC::LOOPiss;
    case 2: return TPC::LOOPsis;
    case 3: return TPC::LOOPiis;
    case 4: return TPC::LOOPssi;
    case 5: return TPC::LOOPisi;
    case 6: return TPC::LOOPsii;
    case 7: return TPC::LOOPiii;
    case 8: return TPC::LOOPsssp;
    case 9: return TPC::LOOPissp;
    case 10: return TPC::LOOPsisp;
    case 11: return TPC::LOOPiisp;
    case 12: return TPC::LOOPssip;
    case 13: return TPC::LOOPisip;
    case 14: return TPC::LOOPsiip;
    case 15: return TPC::LOOPiiip;
  }

  llvm_unreachable("Sanity check");
}
