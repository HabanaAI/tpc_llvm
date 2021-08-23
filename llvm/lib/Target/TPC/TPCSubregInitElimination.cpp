//===---- TPCSubregInitElimination.cpp -Eliminates Dead Subregister Initialization -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------------===//
//
//===-------------------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#define DEBUG_TYPE "subregelim"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCSubregInitElimination();
void initializeTPCSubregInitEliminationPass(PassRegistry&);
}

static const char PassDescription[] = "TPC Eliminate subregister initialization";
static const char PassName[] = "tpc-subreg";

static cl::opt<bool>
EnableTPCSubregInitElimination("dead-subreg-elimination",
                   cl::desc(PassDescription),
                   cl::init(true), cl::Hidden);

namespace {
class TPCSubregInitElimination : public MachineFunctionPass {
  MachineFunction *MF;
  MachineRegisterInfo *MRI;
  const TargetRegisterInfo *TRI;
  const TargetInstrInfo *TII;
  SmallSet<MachineInstr *, 4> SubregInstrs;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCSubregInitElimination() : MachineFunctionPass(ID) {
	  initializeTPCSubregInitEliminationPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool removeInitialization(MachineInstr *Instr,unsigned SubregCount);
  bool hasUses(MachineInstr *Instr, unsigned DefRegNo, unsigned SubregCount);
  void replaceRegister(unsigned DefRegNo, unsigned NewRegNo);
};
}

char TPCSubregInitElimination::ID = 0;

INITIALIZE_PASS(TPCSubregInitElimination, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCSubregInitElimination() {
  return new TPCSubregInitElimination();
}


void TPCSubregInitElimination::replaceRegister(unsigned DefRegNo, unsigned NewRegNo) {
  assert(DefRegNo != NewRegNo && "Cannot replace a reg with itself");

  for (auto I = MRI->use_begin(DefRegNo), E = MRI->use_end(); I != E; ) {
	MachineOperand &O = *I;
	++I;
	if (SubregInstrs.count(O.getParent()) || !O.isTied())
	  continue;
	assert(O.isReg());
	assert(O.getReg() == DefRegNo);
	assert(!O.isDef());
	O.setReg(NewRegNo);
  }
}

bool TPCSubregInitElimination::hasUses(MachineInstr *FirstInstr, unsigned DefRegNo, unsigned SubregCount) {
  MachineBasicBlock *BB = FirstInstr->getParent();
  MachineBasicBlock::iterator I = FirstInstr;
  I++;
  for(MachineBasicBlock::iterator End = BB->end(); I != End;) {
    if (SubregCount == 0) {
        return false;
    }
    MachineInstr *Instr = &(*I++);
    for(auto MO : Instr->uses()) {
      if(!MO.isReg() || MO.getReg() != DefRegNo) continue;
      if (Instr->getOpcode() == FirstInstr->getOpcode()) {
        SubregCount--;
    	unsigned UseOpIdx = Instr->findRegisterUseOperandIdx(MO.getReg());
        DefRegNo = Instr->getOperand(Instr->findTiedOperandIdx(UseOpIdx)).getReg();
        SubregInstrs.insert(Instr);
      } else {
        return true;
	  }
	}
  }
  return SubregCount != 0;
}


bool TPCSubregInitElimination::removeInitialization(MachineInstr *FirstInstr, unsigned SubregCount) {
  SubregInstrs.clear();
  for(unsigned i = 0; i < FirstInstr->getNumOperands(); i++) {
    MachineOperand MO = FirstInstr->getOperand(i);
    if(!MO.isReg() || !MO.isUse() || !MO.isTied()) continue;
    unsigned DefRegNo = FirstInstr->getOperand(FirstInstr->findTiedOperandIdx(i)).getReg();
    if (!hasUses(FirstInstr, DefRegNo, (SubregCount-1))) {
      MachineBasicBlock::iterator I = FirstInstr;
      unsigned v_reg = MRI->createVirtualRegister(MRI->getRegClass(MO.getReg()));
      BuildMI(*(FirstInstr->getParent()), --I, DebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), v_reg);
      replaceRegister(MO.getReg(), v_reg);
      return true;
    }
  }
  return false;
}


bool TPCSubregInitElimination::runOnMachineFunction(MachineFunction &Func) {
  if (!EnableTPCSubregInitElimination)
    return false;

  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  TRI = MF->getSubtarget().getRegisterInfo();
  SubregInstrs.clear();
  bool Changed = false;
  for (auto &BB : Func) {
    for (auto &I : BB) {
      if (unsigned SubregCount = TPCII::getLanesCount(I.getDesc())) {
          if (removeInitialization(&I, SubregCount))
            Changed = true;
        }
    }
  }
  return Changed;
}
