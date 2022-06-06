//===---- TPCSwapFakePredicate.cpp - pass for process fake registers -----===//
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCSubtarget.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#include "MCTargetDesc/TPCMCTargetDesc.h"

using namespace llvm;

namespace llvm {
  FunctionPass *createTPCSwapFakePredicate();
  void initializeTPCSwapFakePredicatePass(PassRegistry&);
} // end namespace llvm

static const char PassDescription[] = "TPC Swap the fake predicate registers";
static const char PassName[] = "tpc-swap-fake-pred";

class TPCSwapFakePredicate  : public MachineFunctionPass {
public:
  static char ID;
  TPCSwapFakePredicate() : MachineFunctionPass(ID) {
    initializeTPCSwapFakePredicatePass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return PassDescription;
  }
  
private:
  Register CreateMov(const DebugLoc &Loc, bool IsVector);
  
  MachineFunction *Func = nullptr;
  const TPCSubtarget *Subtarget = nullptr;
  const TargetInstrInfo *TII = nullptr;
  MachineRegisterInfo *MRI = nullptr;
};

INITIALIZE_PASS(TPCSwapFakePredicate, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCSwapFakePredicate() {
  return new TPCSwapFakePredicate();
}

char TPCSwapFakePredicate::ID = 0;

bool TPCSwapFakePredicate::runOnMachineFunction(MachineFunction &MF) {
  Func = &MF;
  Subtarget = &MF.getSubtarget<TPCSubtarget>();
  TII = Subtarget->getInstrInfo();
  MRI = &MF.getRegInfo();

  //Scavenger.enterBasicBlock(MF.front());
  
  bool Changed = false;
  
  Register SPReg = TPC::NoRegister;
  Register VPReg = TPC::NoRegister;

  for (MachineBasicBlock& MBB : MF) {
    for (MachineInstr& MI : MBB) {
      for (unsigned I = 0; I < MI.getNumOperands(); ++I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg())
          continue;
        if (!MI.isPseudo() && MI.getDesc().OpInfo[I].OperandType ==
            TPC::OperandType::OPERAND_PREDICATE) {
          assert(I < MI.getNumOperands() - 1);
          MachineOperand PolarityMO = MI.getOperand(I + 1);
          assert(PolarityMO.isImm());
          // !SPRF - is predicate
          if (PolarityMO.getImm() == 0)
            continue;
        }
        
        if (MO.getReg() == TPC::SPRF_TRUE) {
          if (Subtarget->hasDoron1()) {
            if (SPReg == TPC::NoRegister)
              SPReg = CreateMov(MI.getDebugLoc(), false);
            
            MO.setReg(SPReg);
          } else {
            MO.setReg(TPC::SP0);
          }
          Changed = true;
        } else if (MO.getReg() == TPC::VPRF_TRUE) {
          if (Subtarget->hasDoron1()) {
            if (VPReg == TPC::NoRegister)
              VPReg = CreateMov(MI.getDebugLoc(), true);
            
            MO.setReg(VPReg);
          } else {
            MO.setReg(TPC::VP0);
          }
          Changed = true;
        }
      }
    }
  }
  
  return Changed;
}

// Think about copy IMM -> SPRF/VPRF
Register TPCSwapFakePredicate::CreateMov(const DebugLoc &Loc, bool IsVector) {
  assert(Subtarget && Subtarget->hasDoron1());
  assert(Func);
  assert(!Func->empty() && !Func->front().empty());
  assert(MRI);
  Register Income = MRI->createVirtualRegister(IsVector ? &TPC::VPRFRegClass :
                                                        &TPC::SPRFRegClass);
  MachineInstr *ImpDef = BuildMI(Func->front(), Func->front().begin(), Loc,
                                 TII->get(TPC::IMPLICIT_DEF), Income)
      .getInstr();
  unsigned MovOpcode = IsVector ? TPC::MOV_ld_mip : TPC::MOV_ld_pip;
  Register Dest = MRI->createVirtualRegister(IsVector ? &TPC::VPRFRegClass :
                                                        &TPC::SPRFRegClass);
  assert(TII);
  auto MI = BuildMI(Func->front(), std::next(ImpDef->getIterator()), 
    Loc, TII->get(MovOpcode), Dest);
  if (IsVector) {
    MI.addImm(-1)  // src = all-bits-true
      .addImm(8);  // flavor = full broadcast
  } else {
    MI.addImm(1);  // src = true
  }
  MI.addImm(0)     // switch
    .addReg(Income, RegState::Undef)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0);

  return Dest;
}
