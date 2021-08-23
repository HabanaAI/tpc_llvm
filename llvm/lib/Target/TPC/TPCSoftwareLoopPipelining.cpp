//===---- TPCSoftwareLoopPipelining.cpp - Pipeline loops -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass pipilenes innermost loops
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
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
#include "llvm/IR/DebugLoc.h"
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

class TPCPipeliner  : public MachineFunctionPass {
  MachineLoopInfo            *MLI;
  MachineRegisterInfo        *MRI;
  MachineDominatorTree       *MDT; //Needed?
  const TPCInstrInfo         *TII;
  const TargetRegisterInfo   *TRI;
  MachineFunction            *LMF;

  std::vector<ExThreadParams> ExecThreads;
  AccumChains AccumulateMap;
  std::map<MachineInstr*, int> LinearIVs;

  // For a given phi-def store it's values for (UnrollCount - 1) iterations
  // in case phi executions paths depend on each other
  std::map<unsigned, std::vector<unsigned> > AdjustedPhis;
  std::vector<MachineInstr*> DelayedPhis;

  std::vector<unsigned> InitialCounters;
  std::vector<unsigned> EndCounters;
  std::vector<unsigned> NextCounters;
  std::vector<unsigned> LeftCounters;

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
                   TII(nullptr), TRI(nullptr), LMF(nullptr),
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
  bool findInnermostLoop(MachineLoop* L);
  void replaceVregs(MachineInstr* MI, unsigned ThreadNum);
  unsigned getCounterRegister(MachineLoop* L);
  void decoupleIfPossible(MachineInstr* MI, unsigned ThreadNum);
  bool align(std::vector<unsigned>& Shifts);
  void createPrologCounters(int Counter, MachineBasicBlock* PrologBlock);
  void createEpilogCounters(int Counter, MachineBasicBlock* PrologBlock);
  void createNextCounters(int Counter, MachineBasicBlock* PrologBlock);
  bool isPhiDef(unsigned Reg, unsigned Thread);
  void replaceUses(MachineInstr* MI, unsigned Old, unsigned New);
  void modifyBoundary(unsigned Decrement);
  void calcLoopForm();
  bool isCyclicDependency(MachineInstr* Phi, MachineInstr* MovedInst, MachineOperand& Use);
  void phiPath(MachineInstr* Phi, std::vector<MachineInstr*>& Path, MachineOperand& HeadPhiUse);
  void patchPhiStartValue(MachineInstr* Phi, int UnrollCount, MachineBasicBlock* MBB, MachineBasicBlock::instr_iterator I);
  void replaceWithZero(MachineOperand& ReplaceMO, TPCII::OpType ot);
  void findLinearInduction(unsigned UnrollCount);
  void setPrologInserter();
  void sortDelayedPhis();
  bool feedsPhi(MachineInstr* MI);
  void correctCounters(int Counter);
  void correctIVs(int Counter);
  void correctExit();
  bool checkForProhibitingInstructions();
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
  if (!EnablePipelining) {
    return false;
  }
  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  MDT = &getAnalysis<MachineDominatorTree>();
  TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  TRI = MF.getSubtarget<TPCSubtarget>().getRegisterInfo();
  LMF = &MF;

  bool Changed = false;
  for (auto &L : *MLI) {
    Changed |= findInnermostLoop(L);
  }

  return Changed;
}

static bool isLoop(MachineInstr* MI) {
  return TPCII::isLoopInst(MI->getDesc()) && MI->getOpcode() != TPC::LOOPEND;
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

      for (MachineInstr* ThreadMI : ExecThreads[ThreadNum].ThreadInstrs) {
        for (MachineOperand& TMO : ThreadMI->uses()) {
          if (TMO.isReg() && TMO.getReg() == MO.getReg()) {
            TMO.setReg(v_reg);
          }
        }
      }

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
          for (MachineRegisterInfo::use_iterator
                 RSUse = MRI->use_begin(OrigReg), RSE = MRI->use_end();
               RSUse != RSE; ++RSUse) {

            MachineInstr *RSUseMI = RSUse->getParent();
            if (!CurLoop->contains(RSUseMI)) {
              for (MachineOperand& CMO : RSUseMI->uses()) {
                if (CMO.isReg() && CMO.getReg() == OrigReg) {
                  CMO.setReg(v_reg);
                }
              }
            }
          }
        }
      }
    }
  }
}

void TPCPipeliner::replaceVregs(MachineInstr* MI, unsigned ThreadNum) {
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

    if (ExecThreads[ThreadNum].VRegMap.count(MO.getReg()) > 0) {
      MO.setReg(ExecThreads[ThreadNum].VRegMap[MO.getReg()]);
    }
  }

  decoupleIfPossible(MI, ThreadNum);
}

unsigned TPCPipeliner::getCounterRegister(MachineLoop* L) {
  unsigned Counter = TPC::S32;
  MachineLoop* Parent = L->getParentLoop();
  while(Parent) {
    MachineBasicBlock* MBB = Parent->getHeader();

    MachineBasicBlock* Preheader = nullptr;

    typedef std::vector<MachineBasicBlock*> MBBVector;
    MBBVector Preds(MBB->pred_begin(), MBB->pred_end());
    for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
      MachineBasicBlock *PB = *I;
      if (!Parent->getLoopLatch()) {
        break;
      }
      if (PB != Parent->getLoopLatch()) {
        Preheader = PB;
        break;
      }
    }

    if (Preheader == nullptr) {
      Parent = Parent->getParentLoop();
      continue;
    }


    for (MachineInstr& MI : Preheader->instrs()) {
      if (isLoop(&MI)) {
        Counter++;
        break;
      }
    }

    Parent = Parent->getParentLoop();
    if (Counter == TPC::S35) {
      break;
    }
  }

  return Counter;
}

static unsigned getUnrollCountFromMetadata(const MDNode* LoopMD) {
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

    if (S->getString().equals("llvm.loop.machine.unroll.count")) {
      assert(MD->getNumOperands() == 2 &&
             "Unroll hint metadata should have two operands.");
      unsigned Count =
          mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
      assert(Count >= 1 && "Unroll count must be positive.");
      return Count;
    }
  }

  return 0;
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

  Header = nullptr;
  Latch  = nullptr;
  Exit   = nullptr;
  Prolog = nullptr;

  CurLoop = L;
  LoopCounter = getCounterRegister(CurLoop);
  Header = CurLoop->getHeader();
  Latch = CurLoop->getLoopLatch();

  if (Latch == nullptr) {
    return false;
  }

  MachineInstr& EndInstr = *(--Latch->end());
  
  if (EndInstr.getOpcode() != TPC::LOOPEND) {
    return false;
  }

  typedef std::vector<MachineBasicBlock*> MBBVector;
  MBBVector Preds(Header->pred_begin(), Header->pred_end());
  for (MBBVector::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
    MachineBasicBlock *PB = *I;
    if (PB != Latch) {
      assert(Prolog == nullptr && "Three predecessors for a hardware loop");
      Prolog = PB;
    }
  }

  LoopInst = &(*(--Prolog->end()));
  if (!isLoop(LoopInst)) {
    return false;
  }

  if (checkForProhibitingInstructions()) {
    return false;
  }

  PrologInserter = --Prolog->end();

  const MDNode* LoopMD = nullptr;

  calcLoopForm();

  if(EndInstr.getOperand(EndInstr.getNumOperands() - 5).isMetadata()) {
    LoopMD = EndInstr.getOperand(EndInstr.getNumOperands() - 5).getMetadata();
  }

  // Get iteration count and unroll count from pragmas
  if (LoopMD) {
    unsigned UnrollCount = getUnrollCountFromMetadata(LoopMD);
    bool DoPipeline = getPipelineFromMetadata(LoopMD);
    //unsigned Divide = getMetadataValue(LoopMD, "llvm.loop.unroll.divide");

    if (UnrollCount > 0) {
      if (LoopInst->getOperand(1).isImm() && LoopInst->getOperand(2).isImm()
          && LoopInst->getOperand(0).isImm()) {
        int Start = LoopInst->getOperand(0).getImm();
        int Bound = LoopInst->getOperand(1).getImm();
        int Step = LoopInst->getOperand(2).getImm();

        // We can get all loop parameters in compile time and they don't
        // match with UnrollCount. Nothing we can do here.
        if ((Bound-Start / Step) % UnrollCount != 0) {
          return false;
        }
        if ((Bound-Start) / Step  < (long) (UnrollCount * 2)) {
          DoPipeline = false;
        }
      }
      return unrollAndAlign(UnrollCount, DoPipeline);
    //} else if (Divide > 0) {
    //  return unrollAndAlign(Divide);
    }
  }

  // TODO: better analysis and automatic unrolling withoug pragmas
  return false;
  // Don't have enough pragmas. Still can pipeline if loop parameters are constants
//  MachineBasicBlock::iterator InsertPos = --Prolog->end();

//  if (!isLoop(&(*InsertPos))) {
//    return false;
//  }

//  MachineInstr& LoopInst = *(InsertPos);
//  MachineOperand& Start = LoopInst.getOperand(0);
//  MachineOperand& Boundary = LoopInst.getOperand(1);
//  MachineOperand& Step = LoopInst.getOperand(2);
//  unsigned CmpMode = LoopInst.getOperand(3).getImm();

//  // Can't pipeline without constant step
//  if (!Step.isImm()) {
//    return false;
//  }

//  if (Start.isImm() && Boundary.isImm() && Step.isImm()) {
//    int StartVal = Start.getImm();
//    int EndVal = Boundary.getImm();
//    int StepVal = Step.getImm();
//    unsigned Inclusive = 0;

//    if (CmpMode == TPCII::LoopEQ ||
//        CmpMode == TPCII::LoopLE ||
//        CmpMode == TPCII::LoopGE) {
//      Inclusive = 1;
//    }

//    unsigned IterCount = (abs(EndVal - StartVal) + Inclusive) / abs(StepVal);
//    if ((abs(EndVal - StartVal) + Inclusive) % StepVal) IterCount++;

//    DEBUG(dbgs() << "Iter count " << IterCount << "\n");

//    // Just try several common divisors
//    if (!(IterCount % 4) && IterCount >=8) {
//      return unrollAndAlign(4, L);
//    }
//    if (!(IterCount % 3) && IterCount >=6) {
//      return unrollAndAlign(3, L);
//    }
//    if (!(IterCount % 5) && IterCount >=10) {
//      return unrollAndAlign(5, L);
//    }
//  }

//  return false;
}

void TPCPipeliner::createPrologCounters(int Counter, MachineBasicBlock* PrologBlock) {
  // For the first thread just return start value
  unsigned StartReg;
  //MachineBasicBlock::iterator InsertPos = --PrologBlock->end();
  MachineBasicBlock::iterator InsertPos = PrologInserter;
  assert(isLoop(&(*(--Prolog->end()))) && "Preheader wasn't created during loop construction");
  if (LoopInst->getOperand(0).isReg()) {
    StartReg = LoopInst->getOperand(0).getReg();
  } else {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::MOVsip), v_reg)
        .addImm(LoopInst->getOperand(0).getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(v_reg, RegState::Undef)
        .addReg(TPC::SP0)
        .addImm(0);
    StartReg = v_reg;
  }
  InitialCounters.push_back(StartReg);

  unsigned PrevReg = StartReg;
  for (int i = 1; i < Counter; ++i) {
    unsigned v_reg  = MRI->createVirtualRegister(PrologBlock->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    if (LoopInst->getOperand(2).isReg()) {
      MachineInstr* MI = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), v_reg)
          .addReg(PrevReg)
          .addReg(LoopInst->getOperand(2).getReg())
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(PrevReg, RegState::Undef)
          .addReg(TPC::SP0)
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
          .addReg(TPC::SP0)
          .addImm(0);
      (void) MI;
      LLVM_DEBUG(ExecThreads[i].Dprolog.push_back(MI));
    }
    InitialCounters.push_back(v_reg);

    // Now patch loop counter registers that were moved out of the loop
    for (MachineInstr* MI : ExecThreads[i].CounterInstrs) {
      for (MachineOperand& MO : MI->uses()) {
        if (MO.isReg() && MO.getReg() == LoopCounter) {
          MO.setReg(v_reg);
        }
      }
    }
  }
}

void TPCPipeliner::correctCounters(int Counter) {
  if (ExecThreads[1].CounterInstrs.empty()) {
    return;
  } else {
    createPrologCounters(Counter, Prolog);
  }
}

void TPCPipeliner::correctIVs(int UnrollCount) {
  for (auto IV: LinearIVs) {
    MachineInstr* MI = IV.first;
    unsigned OpNum = IV.second;
    MI->getOperand(OpNum).setImm(MI->getOperand(OpNum).getImm() * UnrollCount);
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
    for (MachineInstr& MI : MBB->instrs()) {
      if (MI.isPHI()) {
        for (MachineOperand& MO : MI.uses()) {
          if (MO.isMBB() && MO.getMBB() == EB) {
            MO.setMBB(Latch);
          }
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
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      // TODO: Need to multiply
      ShiftCounter = BuildMI(*PrologBlock, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), v_reg)
          .addReg(LoopCounter)
          .addReg(LoopInst->getOperand(2).getReg() /** (i + Counter)*/)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(LoopCounter, RegState::Undef)
          .addReg(TPC::SP0)
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
        .addReg(TPC::SP0)
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
          .addReg(TPC::SP0)
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
          .addReg(TPC::SP0)
          .addImm(0);
      LLVM_DEBUG(ExecThreads[Counter - i].Epilog.push_back(MI));
      EpilogInserter = ++MachineBasicBlock::iterator(MI);
    }
    EndCounters.push_back(v_reg);
  }

  std::reverse(EndCounters.begin(), EndCounters.end());
}

void TPCPipeliner::replaceUses(MachineInstr* MI, unsigned Old, unsigned New) {
  for (MachineOperand& MO: MI->uses()) {
    if (MO.isReg() && MO.getReg() == Old) {
      MO.setReg(New);
    }
  }
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

bool TPCPipeliner::align(std::vector<unsigned>& Shifts) {
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
      for (MachineOperand& MO: PrologMI->uses()) {
        if (MO.isReg() && MO.getReg() == ExecThreads[i].VRegMap[LoopCounter]) {
          MO.setReg(InitialCounterReg);
        }
      }

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

      for (MachineOperand& MO : PrologMI->uses()) {
        if (MO.isReg() && PrologMap.count(MO.getReg()) > 0) {
          MO.setReg(PrologMap[MO.getReg()]);
        }
      }

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
      for (MachineOperand& MO : LeftMI->uses()) {
        if (MO.isReg() && MO.getReg() == ExecThreads[i].VRegMap[LoopCounter]) {
          MO.setReg(LeftCounterReg);
        }
      }

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
          replaceUses(EpilogMI, MO.getReg(), TiedReg);
        }
      }

      for (MachineOperand& MO : EpilogMI->uses()) {
        if (MO.isReg() && MO.getReg() == ExecThreads[i].VRegMap[LoopCounter]) {
          MO.setReg(EndCounterReg);
        }
      }

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
    for (unsigned j = 0; j < ExecThreads[i].Phis.size(); ++j) {
      MachineInstr* PhiInst = ExecThreads[i].Phis[j];
      for (MachineOperand& MO: PhiInst->uses()) {
        if(MO.isReg() && EpilogMap.count(MO.getReg()) > 0) {
          MO.setReg(EpilogMap[MO.getReg()]);
        }
      }
    }

    for (MachineInstr& MI : *Header) {
      if (!MI.isPHI()) {
        for (MachineOperand& MO : MI.uses()) {
          if (MO.isReg() && EpilogMap.count(MO.getReg()) > 0) {
            MO.setReg(EpilogMap[MO.getReg()]);
          }
        }
      }
    }

  }
  return true;
}

bool TPCPipeliner::isPhiDef(unsigned Reg, unsigned Thread) {
  for (unsigned i = 0; i < ExecThreads[Thread].Phis.size(); ++i) {
    MachineInstr* Phi = ExecThreads[Thread].Phis[i];
    if ((*Phi->defs().begin()).getReg() == Reg) {
      return true;
    }
  }

  return false;
}

// Unrolls the loop, makes execution threads independent, shifts iterations
bool TPCPipeliner::unrollAndAlign(unsigned UnrollCount, bool DoPipelining) {
  for (unsigned i = 0 ; i < UnrollCount; ++i) {
    ExThreadParams p;
    ExecThreads.push_back(p);
  }

  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (MachineBasicBlock::instr_iterator I = MBB->instr_begin();
         I != MBB->instr_end(); ++I) {
      MachineInstr* MI = &(*I);

      if (MI->isBranch()) continue;
      if (MI->isPHI()) {
        ExecThreads[0].Phis.push_back(MI);
      } else {
        ExecThreads[0].ThreadInstrs.push_back(MI);
      }
    }
  }

  // Before pipelining we need to find all induction variables that are left
  // after hw transformation and fix increments
  findLinearInduction(UnrollCount);

  ExecThreads[0].VRegMap[LoopCounter] = LoopCounter;

  // Duplicate counter register for every execution thread
  auto InsertPos = Header->begin();
  while(InsertPos != Header->end() && (*InsertPos).isPHI()) ++InsertPos;

  unsigned PrevReg = LoopCounter;
  for (unsigned i = 1; i < UnrollCount; ++i) {
    unsigned v_reg  = MRI->createVirtualRegister(Header->getParent()->getSubtarget().getTargetLowering()->getRegClassFor(MVT::i32));
    if (LoopInst->getOperand(2).isReg()) {
      // If increment value is a register, we can't easily make increments in
      // each execution thread independent.
      BuildMI(*Header, InsertPos, DebugLoc(), TII->get(TPC::ADDssp), v_reg)
          .addReg(PrevReg)
          .addReg(LoopInst->getOperand(2).getReg())
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(PrevReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      PrevReg = v_reg;
      InsertPos++;
    } else {
      // If increment value is a constant create independent increment for
      // each execution thread with increment values computed at compile time.
      BuildMI(*Header, InsertPos, DebugLoc(), TII->get(TPC::ADDsip), v_reg)
          .addReg(PrevReg)
          .addImm(LoopInst->getOperand(2).getImm() * i)
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(PrevReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    ExecThreads[i].VRegMap[LoopCounter] = v_reg;
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

      if(std::find(ExecThreads[0].ThreadInstrs.begin(), ExecThreads[0].ThreadInstrs.end(), CurMI) == ExecThreads[0].ThreadInstrs.end()) {
        continue;
      }

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

  MachineInstr* LastInstr = nullptr;
  if (Prolog->size() == 1) {
    LastInstr = &(*(--Prolog->end()));
  } else {
    LastInstr = &(*(--(--Prolog->end())));
  }

  // Now clone and patch phi-nodes
  sortDelayedPhis();

  for (MachineInstr* CurMI : DelayedPhis) {
    assert(CurMI->isPHI() && "Non-phi instruction is in the phi list");
    patchPhiStartValue(CurMI, UnrollCount, CurMI->getParent(), MachineBasicBlock::instr_iterator(CurMI));
  }


  PrologInserter = MachineBasicBlock::iterator(LastInstr);

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
  for (MachineInstr& MI : OldExit->instrs()) {
    if (MI.isPHI()) {
      for (MachineOperand& MO : MI.uses()) {
        if (MO.isMBB() && MO.getMBB() == Latch) {
          MO.setMBB(Accum);
        }
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
    std::vector<unsigned> Chain = It->second;
    unsigned AddOpc;
    const TargetRegisterClass* RC = MRI->getRegClass(ResReg);
    MachineInstr* MI = MRI->getVRegDef(ResReg);
    TPCII::OpType Type = getOpType(*MI);
    AddOpc = getAddOpc(Type, RC);

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
            .addReg(PrevReg, RegState::Undef)
            .addReg(TPC::SP0)
            .addImm(0);
        ChainTerm = TmpVreg;
      }

      unsigned TmpVreg = MRI->createVirtualRegister(MRI->getRegClass(ResReg));
      BuildMI(*Accum, AccumInsertPos, DebugLoc(), TII->get(AddOpc), ResReg)
          .addReg(ChainTerm)
          .addReg(TmpVreg)
          .addImm(Type)
          .addImm(0)
          .addReg(PrevReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      ForceSsa[ResReg] = TmpVreg;
    }
  }

  // To preserve SSA replace original accumulator values with a new reg inside this loop
  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (MachineInstr& MI : MBB->instrs()) {
      for (MachineOperand& MO : MI.operands()) {
        if (MO.isReg()) {
          for (RegMap::iterator It = ForceSsa.begin();
               It != ForceSsa.end(); ++It) {
            if (MO.getReg() == It->first) {
              MO.setReg(It->second);
            }
          }
        }
      }
    }
  }

  for (unsigned i = 0; i < UnrollCount; ++i) {
    LLVM_DEBUG(dbgs() << "\n******** Thread " << i << " *********\n");

    LLVM_DEBUG(dbgs() << "\nPhis\n");
    for (unsigned j = 0; j < ExecThreads[i].Phis.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Phis[j]->dump());
    }
    for (unsigned j = 0; j < ExecThreads[i].Dphi.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Dphi[j]->dump());
    }

    LLVM_DEBUG(dbgs() << "\nLoop body\n");
    for (unsigned j = 0; j < ExecThreads[i].ThreadInstrs.size();++j) {
      LLVM_DEBUG(ExecThreads[i].ThreadInstrs[j]->dump());
    }
  }
  LLVM_DEBUG(dbgs() << "\n");

  // TODO: proper alignment strategy
  std::vector<unsigned> Shifts;
  for (unsigned i = 0; i < UnrollCount; ++i) {
    Shifts.push_back(std::min(ExecThreads[i].ThreadInstrs.size() - 1, (size_t)UnrollCount - i));
  }

  //MF->dump();
  bool Aligned = DoPipelining ? align(Shifts) : false;

  if (!DoPipelining) {
    setPrologInserter();
    correctCounters(UnrollCount);
    correctIVs(UnrollCount);
    correctExit();
  }

  // Change the increment value of the loop
  if (LoopInst->getOperand(2).isImm()) {
    LoopInst->getOperand(2).setImm(LoopInst->getOperand(2).getImm()*UnrollCount);

    if (LoopInst->getOperand(2).getImm() > 0 && LoopInst->getOperand(3).getImm() == 5) {
      LoopInst->getOperand(3).setImm(2);
    }
  } else {
    llvm_unreachable("");
  }
  
  if (Aligned) {
    PrologInserter = --Prolog->end();
    modifyBoundary(UnrollCount);
  }

  for (unsigned i = 0; i < UnrollCount; ++i) {
    LLVM_DEBUG(dbgs() << "\n******** Thread " << i << " *********\n");

    LLVM_DEBUG(dbgs() << "\nPrologue\n");
    for (unsigned j = 0; j < ExecThreads[i].Dprolog.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Dprolog[j]->dump());
    }

    LLVM_DEBUG(dbgs() << "\nPhis\n");
    for (unsigned j = 0; j < ExecThreads[i].Phis.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Phis[j]->dump());
    }
    for (unsigned j = 0; j < ExecThreads[i].Dphi.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Dphi[j]->dump());
    }

    LLVM_DEBUG(dbgs() << "\nLoop body\n");
    for (unsigned j = 0; j < ExecThreads[i].Dloop.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Dloop[j]->dump());
    }
    for (unsigned j = 0; j < ExecThreads[i].Dshift.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Dshift[j]->dump());
    }

    LLVM_DEBUG(dbgs() << "\nEpilogue\n");
    for (unsigned j = 0; j < ExecThreads[i].Epilog.size();++j) {
      LLVM_DEBUG(ExecThreads[i].Epilog[j]->dump());
    }
  }
  LLVM_DEBUG(dbgs() << "\n");

  return true;
}

void TPCPipeliner::calcLoopForm() {
  int CmpMode = LoopInst->getOperand(3).getImm();
  if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopEQ) {
    Inclusive = true;
  } else {
    Inclusive = false;
  }

  if (LoopInst->getOperand(2).isImm()) {
    Ascending = (LoopInst->getOperand(2).getImm() > 0);
    return;
  }

  if (LoopInst->getOperand(0).isImm() && LoopInst->getOperand(1).isImm()) {
    Ascending = (LoopInst->getOperand(0).getImm() < LoopInst->getOperand(1).getImm());
    return;
  }

  // TODO: Can't say, should prohibit pipelining
}

void TPCPipeliner::modifyBoundary(unsigned Decrement) {
  if (LoopInst->getOperand(1).isImm()) {
    if (LoopInst->getOperand(2).isImm()) {
      int Step = LoopInst->getOperand(2).getImm();
      if (Step > 0) {
        LoopInst->getOperand(1).setImm(LoopInst->getOperand(1).getImm() - Decrement);
      } else {
        LoopInst->getOperand(1).setImm(LoopInst->getOperand(1).getImm() + Decrement);
      }
    } else {
      int CmpMode = LoopInst->getOperand(3).getImm();
      if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopLT) {
        LoopInst->getOperand(1).setImm(LoopInst->getOperand(1).getImm() - Decrement);
      } else if (CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopGT) {
        LoopInst->getOperand(1).setImm(LoopInst->getOperand(1).getImm() + Decrement);
      } else {
        // Can't infer ascending or descending  nature of the loop. Have to create
        // a runtime check.
        // TODO: SSA + predicated execution is tricky
        llvm_unreachable("");
      }
    }
  } else {
    unsigned NewBoundary = MRI->createVirtualRegister(MRI->getRegClass(LoopInst->getOperand(1).getReg()));
    if (LoopInst->getOperand(2).isImm()) {
      int Step = LoopInst->getOperand(2).getImm();
      if (Step > 0) {
        unsigned LoopCounter = LoopInst->getOperand(1).getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::SUBsip), NewBoundary)
            .addReg(LoopCounter)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0) // Switch
            .addReg(LoopCounter, RegState::Undef)
            .addReg(TPC::SP0)
            .addImm(0);
        LoopInst->getOperand(1).setReg(NewBoundary);
      } else {
        unsigned LoopReg = LoopInst->getOperand(1).getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::ADDsip), NewBoundary)
            .addReg(LoopReg)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopReg, RegState::Undef)
            .addReg(TPC::SP0)
            .addImm(0);
        LoopInst->getOperand(1).setReg(NewBoundary);
      }
    } else {
      int CmpMode = LoopInst->getOperand(3).getImm();
      if (CmpMode == TPCII::LoopLE || CmpMode == TPCII::LoopLT) {
        unsigned LoopCounter = LoopInst->getOperand(1).getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::SUBsip), NewBoundary)
            .addReg(LoopCounter)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopCounter, RegState::Undef)
            .addReg(TPC::SP0)
            .addImm(0);
        LoopInst->getOperand(1).setReg(NewBoundary);
      } else if (CmpMode == TPCII::LoopGE || CmpMode == TPCII::LoopGT) {
        unsigned LoopReg = LoopInst->getOperand(1).getReg();
        BuildMI(*Prolog, PrologInserter, DebugLoc(), TII->get(TPC::ADDsip), NewBoundary)
            .addReg(LoopReg)
            .addImm(Decrement)
            .addImm(TPCII::OpType::INT32)
            .addImm(0)
            .addReg(LoopReg, RegState::Undef)
            .addReg(TPC::SP0)
            .addImm(0);
        LoopInst->getOperand(1).setReg(NewBoundary);
      } else {
        // Can't infer ascending or descending  nature of the loop. Have to create
        // a runtime check.
        llvm_unreachable("");
      }
    }
  }
}

void TPCPipeliner::replaceWithZero(MachineOperand& ReplaceMO, TPCII::OpType ot) {
  MachineBasicBlock::iterator ZeroInsertPos = Prolog->begin();
  while(ZeroInsertPos != Prolog->end() && (*ZeroInsertPos).isPHI()) ++ZeroInsertPos;
  if (ReplaceMO.isFPImm() || ReplaceMO.isImm()) {
    // Even if the immediate is a floating point number, MOV uses integer bit
    // representation.
    ReplaceMO.setImm(0);
  } else if (ReplaceMO.isReg()) {
    const auto AddMovZero = [&](unsigned Opcode, unsigned Reg) {
      BuildMI(*Prolog, ZeroInsertPos++, DebugLoc(), TII->get(Opcode), Reg)
         .addImm(0)
         .addImm(ot)
         .addImm(0)
         .addReg(Reg, RegState::Undef)
         .addReg(TPC::SP0)
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

// Some cloned phi-nodes must start with the same values (for accumulators)
// the others must adjust out-of-loop uses and create instructions in the preheader
void TPCPipeliner::patchPhiStartValue(MachineInstr* Phi, int UnrollCount,
                                      MachineBasicBlock* MBB,
                                      MachineBasicBlock::instr_iterator I) {
  std::vector<MachineInstr*> Path;
  unsigned PhiDef = Phi->getOperand(0).getReg();

  int OutOfLoopUse = -1;
  int LoopUse = -1;

  int counter = 0;
  for (MachineOperand& MO : Phi->uses()) {
    if (MO.isMBB() && MO.getMBB() == Prolog) {
      OutOfLoopUse = counter;
    }
    if (MO.isMBB() && MO.getMBB() == Latch) {
      LoopUse = counter;
    }
    ++counter;
  }

  phiPath(Phi, Path, Phi->getOperand(OutOfLoopUse));
  std::reverse(Path.begin(), Path.end());

  // TODO: what if not a reg
  unsigned FinalReg = Phi->getOperand(OutOfLoopUse).getReg();
  unsigned FirstReg = FinalReg;

  std::vector<unsigned> UseVec;
  std::vector<unsigned> DefVec;

  for (int i = 1; i < UnrollCount; ++i) {
    MachineInstr* ClonedPhi = MBB->getParent()->CloneMachineInstr(&(*I));
    ExecThreads[i].Phis.push_back(ClonedPhi);
    replaceVregs(ClonedPhi, i);
    MBB->insert(I, ClonedPhi);
    RegMap RM;

    if (!Path.empty() && TPCII::isMac(Path.back()->getDesc())) {
      MachineOperand& ReplaceMO = ClonedPhi->getOperand(OutOfLoopUse);
      replaceWithZero(ReplaceMO, getOpType(*Path.back()));
      continue;
    }

    RM[PhiDef] = FinalReg;

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
        if (Phi->getOperand(OutOfLoopUse).isReg()) {
          unsigned phi_reg = Phi->getOperand(OutOfLoopUse).getReg();
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
void TPCPipeliner::phiPath(MachineInstr* Phi, std::vector<MachineInstr*>& Path, MachineOperand& HeadPhiUse) {
  std::vector<MachineOperand*> OperandStack;
  OperandStack.push_back(&Phi->getOperand(1));
  OperandStack.push_back(&Phi->getOperand(3));

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
}

// TODO: full implementation of SSA loop search
void TPCPipeliner::findLinearInduction(unsigned UnrollCount) {
  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (MachineBasicBlock::instr_iterator I = MBB->instr_begin();
         I != MBB->instr_end(); ++I) {
      MachineInstr* Phi = &(*I);

      if (!Phi->isPHI()) break;

      unsigned PhiDef = Phi->getOperand(0).getReg();

      int LoopUse = -1;

      int counter = 0;
      for (MachineOperand& MO : Phi->uses()) {
        if (MO.isMBB() && MO.getMBB() == Latch) {
          LoopUse = counter;
        }
        ++counter;
      }

      MachineInstr* DefMI = MRI->getVRegDef(Phi->getOperand(LoopUse).getReg());
      if (DefMI == nullptr) continue;
      for (MachineOperand& MO : DefMI->uses()) {
        if (MO.isReg() && MO.getReg() == PhiDef && TPCII::isAdd(DefMI->getDesc())) {
          //found a linear induction
          if (DefMI->getOperand(2).isImm()) {
            LinearIVs[DefMI] = 2;
          } else if (DefMI->getOperand(1).isImm()) {
            LinearIVs[DefMI] = 1;
          }
        }
      }
    }
  }
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

  PhiDepTree() : Phi(nullptr), Preds(0) {}
};

static void DFS(PhiDepTree* Node, std::list<MachineInstr*>& SortedPhis) {
  for (PhiDepTree* Succ : Node->succs) {
    DFS(Succ, SortedPhis);
  }
  if (std::find(SortedPhis.begin(), SortedPhis.end(), Node->Phi) == SortedPhis.end()) {
    SortedPhis.push_back(Node->Phi);
  }
}

void TPCPipeliner::sortDelayedPhis() {
  std::list<MachineInstr*> SortedPhis;
  std::vector<PhiDepTree*> Tree;

  // Create all nodes
  for (MachineInstr* Phi : DelayedPhis) {
    PhiDepTree* Node = new PhiDepTree();
    Node->Phi = Phi;
    Tree.push_back(Node);
  }

  // Build dep graph
  // TODO: for now just implement naive algorythm. But it's too slow and should be rewritten
  for (unsigned i = 0; i < DelayedPhis.size(); ++i) {
    MachineInstr* Dependant = DelayedPhis[i];
    PhiDepTree* DependantNode = Tree[i];

    int OutOfLoopUse = -1;
    std::vector<MachineInstr*> Path;

    int counter = 0;
    for (MachineOperand& MO : Dependant->uses()) {
      if (MO.isMBB() && MO.getMBB() == Prolog) {
        OutOfLoopUse = counter;
      }
      ++counter;
    }

    phiPath(Dependant, Path, Dependant->getOperand(OutOfLoopUse));

    for (unsigned j = 0; j < DelayedPhis.size(); ++j) {
      MachineInstr* Phi = DelayedPhis[j];
      PhiDepTree* PhiNode = Tree[j];
      unsigned LoopReg = -1;

      if (Phi == Dependant) continue;

      int counter = 0;
      for (MachineOperand& MO : Phi->uses()) {
        if (MO.isMBB() && MO.getMBB() == Latch) {
          LoopReg = Phi->getOperand(counter).getReg();
        }
        ++counter;
      }

      // Find wether the dependant path uses the def of Phi
      for (MachineInstr* MI : Path) {
        for (MachineOperand& MO : MI->uses()) {
          if (MO.isReg() && (MO.getReg() == LoopReg || MO.getReg() == Phi->getOperand(0).getReg())) {
            PhiNode->succs.push_back(DependantNode);
            DependantNode->Preds++;
            break;
          }
        }
      }
    }
  }

  // Topological sort
  for (PhiDepTree* Node : Tree) {
    if (Node->Preds == 0) {
      DFS(Node, SortedPhis);
    }
  }

  SortedPhis.reverse();
  DelayedPhis = std::vector<MachineInstr*>(SortedPhis.begin(), SortedPhis.end());

  for (PhiDepTree* Node : Tree) {
    delete Node;
  }
}

bool TPCPipeliner::checkForProhibitingInstructions() {
  for (MachineBasicBlock* MBB : CurLoop->getBlocks()) {
    for (const MachineInstr& MI : MBB->instrs()) {
      if (MI.getOpcode() == TPC::ASO) {
        return true;
      }
      if (TPCInstrInfo::isMMIOAccess(MI)) {
        return true;
      }

      for (const MachineOperand& MO: MI.defs()) {
        assert(MO.isReg() && "Def can't be a constant");
        if (MO.getReg().isPhysical()) continue;
        const TargetRegisterClass *RC = MRI->getRegClass(MO.getReg());
        if (TPC::HWPCRegClass.hasSubClassEq(RC)) {
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
      .addReg(TPC::SP0)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res1)
      .addReg(Op1, 0, TPC::sub_1)
      .addReg(Op2, 0, TPC::sub_1)
      .addImm(Type)
      .addImm(0)
      .addReg(Res1, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res2)
      .addReg(Op1, 0, TPC::sub_2)
      .addReg(Op2, 0, TPC::sub_2)
      .addImm(Type)
      .addImm(0)
      .addReg(Res2, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res3)
      .addReg(Op1, 0, TPC::sub_3)
      .addReg(Op2, 0, TPC::sub_3)
      .addImm(Type)
      .addImm(0)
      .addReg(Res3, RegState::Undef)
      .addReg(TPC::SP0)
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
      .addReg(TPC::SP0)
      .addImm(0);
  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(AddOpc), Res1)
      .addReg(Op1, 0, TPC::sub_1)
      .addReg(Op2, 0, TPC::sub_1)
      .addImm(Type)
      .addImm(0)
      .addReg(Res1, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);

  BuildMI(*Accum, Accum->end(), DebugLoc(), TII->get(TargetOpcode::REG_SEQUENCE), Res)
      .addReg(Res0)
      .addImm(TPC::sub_0)
      .addReg(Res1)
      .addImm(TPC::sub_1);
}
