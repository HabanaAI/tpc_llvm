//===---------------- TPCSetSpillBase.cpp - initializes spill base register ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass initializes spill base register if there were vector spills 
// and target has feature Addr2.
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
#include "llvm/CodeGen/TargetRegisterInfo.h"
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
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "setspillbase"

namespace llvm {

  FunctionPass *createTPCSetSpillBase();
  void initializeTPCSetSpillBasePass(PassRegistry&);

} // end namespace llvm

class TPCSetSpillBase  : public MachineFunctionPass {
public:
  static char ID;
  TPCSetSpillBase() : MachineFunctionPass(ID) {
    initializeTPCSetSpillBasePass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "TPC Set Spill Base"; }
};

INITIALIZE_PASS_BEGIN(TPCSetSpillBase, "setspillbase",
                      "TPC Set Spill Base", false, false)
INITIALIZE_PASS_END(TPCSetSpillBase, "setspillbase",
                    "TPC Set Spill Base", false, false)

FunctionPass *llvm::createTPCSetSpillBase() {
  return new TPCSetSpillBase();
}

char TPCSetSpillBase::ID = 0;

bool TPCSetSpillBase::runOnMachineFunction(MachineFunction &MF) {
  const TPCSubtarget &Subtarget = MF.getSubtarget<TPCSubtarget>();
  TPCFrameLowering &FL = *const_cast<TPCFrameLowering *>(Subtarget.getFrameLowering());
  unsigned SpillVectorSz = FL.getSpillVectorDataSize();
  if (SpillVectorSz && Subtarget.hasAddr2()) {
    MachineBasicBlock &MBB = *MF.begin();
    DebugLoc DL = MBB.findDebugLoc(MBB.instr_begin());
    const MCInstrDesc &InstrDesc = Subtarget.getInstrInfo()->get(TPC::MOV_ld_sip /*MOVnodce*/);
    const TPCTargetLowering &TL = *Subtarget.getTargetLowering();
    unsigned ZR = TL.getZeroReg();
    BuildMI(MBB, MBB.instr_begin(), DL, InstrDesc, ZR).addImm(0) 
      .addImm(TPCII::OpType::INT32) // Data type
      .addImm(0)                    // Switch
      .addReg(ZR, RegState::Undef)  // income
      .addReg(TPC::SPRF_TRUE)             // Pred
      .addImm(0);                   // Polarity
    return true;
  }

  return false;
}
