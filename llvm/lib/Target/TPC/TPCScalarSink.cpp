//===---- TPCScalarSink.cpp - Convert vector operands into scalar operands -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The intrinsics that can have auto-broadcasted scalars as arguments are represented as
// intr(v1, splat(v2))
// in front end. LICM can hoist splat part out of the loop so later during
// the selection phase the scalar variant of the command can not be formed.
// This pass finds broadcast moves and tries to sink them inside the loop
// and create the scalar version of the command. This saves VRF usage.
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

#define DEBUG_TYPE "tpc-scalar-sink"

static cl::opt<bool>
    EnableSink("enable-scalar-sink", cl::Hidden,
                 cl::desc("Enable scalar sink"), cl::init(true));

namespace llvm {
  FunctionPass *createTPCScalarSink();
  void initializeTPCScalarSinkPass(PassRegistry&);
} // end namespace llvm


class TPCScalarSink : public MachineFunctionPass {
  MachineLoopInfo *MLI;
  MachineRegisterInfo* MRI;
  const TPCInstrInfo * TII;

public:
  static char ID;
  TPCScalarSink() : MachineFunctionPass(ID), MLI(nullptr), TII(nullptr) {
    initializeTPCScalarSinkPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

INITIALIZE_PASS_BEGIN(TPCScalarSink, "movscalarsink",
                      "TPC Scalar Sink", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCScalarSink, "movscalarsink",
                    "TPC Scalar Sink", false, false)

FunctionPass *llvm::createTPCScalarSink() {
  return new TPCScalarSink();
}

char TPCScalarSink::ID = 0;

bool TPCScalarSink::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableSink) {
    return false;
  };

  std::map<unsigned, unsigned> V2S;
  std::map<unsigned, unsigned> V2I;
  // TODO: only basic variants of instructions are here
  //       adding all variants in that way is ridiculous.
  //       Think of something better
  V2S[TPC::MACf32vvp] = TPC::MACf32vsp; V2I[TPC::MACf32vvp] = TPC::MACf32vip;
  V2S[TPC::MACf32vvm] = TPC::MACf32vsm; V2I[TPC::MACf32vvm] = TPC::MACf32vim;
  V2S[TPC::MACi16vvp] = TPC::MACi16vsp; V2I[TPC::MACi16vvp] = TPC::MACi16vip;
  V2S[TPC::MACi16vvm] = TPC::MACi16vsm; V2I[TPC::MACi16vvm] = TPC::MACi16vim;
  V2S[TPC::MACu16vvp] = TPC::MACu16vsp; V2I[TPC::MACu16vvp] = TPC::MACu16vip;
  V2S[TPC::MACu16vvm] = TPC::MACu16vsm; V2I[TPC::MACu16vvm] = TPC::MACu16vim;
  V2S[TPC::MACi8vvp] = TPC::MACi8vsp; V2I[TPC::MACi8vvp] = TPC::MACi8vip;
  V2S[TPC::MACi8vvm] = TPC::MACi8vsm; V2I[TPC::MACi8vvm] = TPC::MACi8vim;
  V2S[TPC::MACu8vvp] = TPC::MACu8vsp; V2I[TPC::MACu8vvp] = TPC::MACu8vip;
  V2S[TPC::MACu8vvm] = TPC::MACu8vsm; V2I[TPC::MACu8vvm] = TPC::MACu8vim;
  V2S[TPC::MACAbf16vvp] = TPC::MACAbf16vsp; V2I[TPC::MACAbf16vvp] = TPC::MACAbf16vip;
  V2S[TPC::MACAbf16vvm] = TPC::MACAbf16vsm; V2I[TPC::MACAbf16vvm] = TPC::MACAbf16vim;

  V2S[TPC::MULf32vvp] = TPC::MULf32vsp; V2I[TPC::MULf32vvp] = TPC::MULf32vip;
  V2S[TPC::MULf32vvm] = TPC::MULf32vsm; V2I[TPC::MULf32vvm] = TPC::MULf32vim;
  V2S[TPC::MULi32vvp] = TPC::MULi32vsp; V2I[TPC::MULi32vvp] = TPC::MULi32vip;
  V2S[TPC::MULi32vvm] = TPC::MULi32vsm; V2I[TPC::MULi32vvm] = TPC::MULi32vim;
  V2S[TPC::MULu32vvp] = TPC::MULu32vsp; V2I[TPC::MULu32vvp] = TPC::MULu32vip;
  V2S[TPC::MULu32vvm] = TPC::MULu32vsm; V2I[TPC::MULu32vvm] = TPC::MULu32vim;
  V2S[TPC::MULi16vvp] = TPC::MULi16vsp; V2I[TPC::MULi16vvp] = TPC::MULi16vip;
  V2S[TPC::MULi16vvm] = TPC::MULi16vsm; V2I[TPC::MULi16vvm] = TPC::MULi16vim;
  V2S[TPC::MULu16vvp] = TPC::MULu16vsp; V2I[TPC::MULu16vvp] = TPC::MULu16vip;
  V2S[TPC::MULu16vvm] = TPC::MULu16vsm; V2I[TPC::MULu16vvm] = TPC::MULu16vim;
  V2S[TPC::MULi8vvp] = TPC::MULi8vsp; V2I[TPC::MULi8vvp] = TPC::MULi8vip;
  V2S[TPC::MULi8vvm] = TPC::MULi8vsm; V2I[TPC::MULi8vvm] = TPC::MULi8vim;
  V2S[TPC::MULu8vvp] = TPC::MULu8vsp; V2I[TPC::MULu8vvp] = TPC::MULu8vip;
  V2S[TPC::MULu8vvm] = TPC::MULu8vsm; V2I[TPC::MULu8vvm] = TPC::MULu8vim;
  V2S[TPC::MULAbf16vvp] = TPC::MULAbf16vsp; V2I[TPC::MULAbf16vvp] = TPC::MULAbf16vip;
  V2S[TPC::MULAbf16vvm] = TPC::MULAbf16vsm; V2I[TPC::MULAbf16vvm] = TPC::MULAbf16vim;
  V2S[TPC::MULAi32vvp] = TPC::MULAi32vsp; V2I[TPC::MULAi32vvp] = TPC::MULAi32vip;
  V2S[TPC::MULAi32vvm] = TPC::MULAi32vsm; V2I[TPC::MULAi32vvm] = TPC::MULAi32vim;
  V2S[TPC::MULAu32vvp] = TPC::MULAu32vsp; V2I[TPC::MULAu32vvp] = TPC::MULAu32vip;
  V2S[TPC::MULAu32vvm] = TPC::MULAu32vsm; V2I[TPC::MULAu32vvm] = TPC::MULAu32vim;

  V2S[TPC::ADDvvp] = TPC::ADDvsp; V2I[TPC::ADDvvp] = TPC::ADDvip;
  V2S[TPC::ADDvvm] = TPC::ADDvsm; V2I[TPC::ADDvvm] = TPC::ADDvim;
  V2S[TPC::SUBvvp] = TPC::SUBvsp; V2I[TPC::SUBvvp] = TPC::SUBvip;
  V2S[TPC::SUBvvm] = TPC::SUBvsm; V2I[TPC::SUBvvm] = TPC::SUBvim;
  V2S[TPC::MAXvvp] = TPC::MAXvsp; V2I[TPC::MAXvvp] = TPC::MAXvip;
  V2S[TPC::MAXvvm] = TPC::MAXvsm; V2I[TPC::MAXvvm] = TPC::MAXvim;
  V2S[TPC::MINvvp] = TPC::MINvsp; V2I[TPC::MINvvp] = TPC::MINvip;
  V2S[TPC::MINvvm] = TPC::MINvsm; V2I[TPC::MINvvm] = TPC::MINvim;
  V2S[TPC::ANDvvp] = TPC::ANDvsp; V2I[TPC::ANDvvp] = TPC::ANDvip;
  V2S[TPC::ANDvvm] = TPC::ANDvsm; V2I[TPC::ANDvvm] = TPC::ANDvim;
  V2S[TPC::ORvvp] = TPC::ORvsp; V2I[TPC::ORvvp] = TPC::ORvip;
  V2S[TPC::ORvvm] = TPC::ORvsm; V2I[TPC::ORvvm] = TPC::ORvim;
  V2S[TPC::XORvvp] = TPC::XORvsp; V2I[TPC::XORvvp] = TPC::XORvip;
  V2S[TPC::XORvvm] = TPC::XORvsm; V2I[TPC::XORvvm] = TPC::XORvim;
  V2S[TPC::ASHvvp] = TPC::ASHvsp; V2I[TPC::ASHvvp] = TPC::ASHvip;
  V2S[TPC::ASHvvm] = TPC::ASHvsm; V2I[TPC::ASHvvm] = TPC::ASHvim;
  V2S[TPC::SHRvvp] = TPC::SHRvsp; V2I[TPC::SHRvvp] = TPC::SHRvip;
  V2S[TPC::SHRvvm] = TPC::SHRvsm; V2I[TPC::SHRvvm] = TPC::SHRvim;
  V2S[TPC::SHLvvp] = TPC::SHLvsp; V2I[TPC::SHLvvp] = TPC::SHLvip;
  V2S[TPC::SHLvvm] = TPC::SHLvsm; V2I[TPC::SHLvvm] = TPC::SHLvim;
  V2S[TPC::CMP_EQvvp] = TPC::CMP_EQvsp; V2I[TPC::CMP_EQvvp] = TPC::CMP_EQvip;
  V2S[TPC::CMP_EQvvm] = TPC::CMP_EQvsm; V2I[TPC::CMP_EQvvm] = TPC::CMP_EQvim;
  V2S[TPC::CMP_NEQvvp] = TPC::CMP_NEQvsp; V2I[TPC::CMP_NEQvvp] = TPC::CMP_NEQvip;
  V2S[TPC::CMP_NEQvvm] = TPC::CMP_NEQvsm; V2I[TPC::CMP_NEQvvm] = TPC::CMP_NEQvim;
  V2S[TPC::CMP_LESSvvp] = TPC::CMP_LESSvsp; V2I[TPC::CMP_LESSvvp] = TPC::CMP_LESSvip;
  V2S[TPC::CMP_LESSvvm] = TPC::CMP_LESSvsm; V2I[TPC::CMP_LESSvvm] = TPC::CMP_LESSvim;
  V2S[TPC::CMP_LEQvvp] = TPC::CMP_LEQvsp; V2I[TPC::CMP_LEQvvp] = TPC::CMP_LEQvip;
  V2S[TPC::CMP_LEQvvm] = TPC::CMP_LEQvsm; V2I[TPC::CMP_LEQvvm] = TPC::CMP_LEQvim;
  V2S[TPC::CMP_GRTvvp] = TPC::CMP_GRTvsp; V2I[TPC::CMP_GRTvvp] = TPC::CMP_GRTvip;
  V2S[TPC::CMP_GRTvvm] = TPC::CMP_GRTvsm; V2I[TPC::CMP_GRTvvm] = TPC::CMP_GRTvim;
  V2S[TPC::CMP_GEQvvp] = TPC::CMP_GEQvsp; V2I[TPC::CMP_GEQvvp] = TPC::CMP_GEQvip;
  V2S[TPC::CMP_GEQvvm] = TPC::CMP_GEQvsm; V2I[TPC::CMP_GEQvvm] = TPC::CMP_GEQvim;

  bool Changed = false;
  MRI = &MF.getRegInfo();
  TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();

  for (auto MBB = MF.begin(), E = MF.end(); MBB != E; ++MBB) {
    for (MachineInstr& Inst : MBB->instrs()) {
      if (V2S.count(Inst.getOpcode()) > 0) {
        if (Inst.getOperand(2).isReg()) {
          const TargetRegisterClass *RC = MRI->getRegClass(Inst.getOperand(2).getReg());
          if (TPC::VRFRegClass.hasSubClassEq(RC)) {
            MachineInstr* defI = MRI->getVRegDef(Inst.getOperand(2).getReg());
            if (defI->getOpcode() == TPC::MOV_ld_vsp) {
              if (defI->getOperand(1).isReg()) {
                Inst.getOperand(2).setReg(defI->getOperand(1).getReg());
                Inst.setDesc(TII->get(V2S[Inst.getOpcode()]));
                Changed = true;
              }
            } else if (defI->getOpcode() == TPC::MOV_ld_vsp) {
              // TODO: how to change register operand to constant?
            }
          }
        }
      }
    }
  }

  return Changed;
}
