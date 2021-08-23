//===---- TPCRegisterCounter.cpp --- Optimizes predicates ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "tpc-rcount"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCFrameLowering.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCInstPrinter.h"
#include <set>
#include <sstream>

using namespace llvm;

namespace llvm {
FunctionPass *createTPCRegisterCounter();
void initializeTPCRegisterCounterPass(PassRegistry&);
}

static const char PassDescription[] = "TPC register counter";
static const char PassName[] = "tpc-rcount";

// Flag to enable spill counter.
static cl::opt<bool>
EnableSpillCounter("spill-count",
                       cl::desc("Count number of registers,local memory and local memory for spills used (default=false)"),
                       cl::init(false), cl::Hidden);

namespace {

static void printRegisterSet(std::set<unsigned> Registers, StringRef RegType) {
    StringRef OutputStr = " registers used: ";
    if (Registers.size() == 0) {
        return;
    }
    if (Registers.size() == 1) {
        OutputStr = " register used: ";
    }
    errs() <<  Registers.size() << " " << RegType<< OutputStr;
    for (auto Elem : Registers){
        errs() << "%" <<TPCInstPrinter::getRegisterName(Elem) << " ";
    }
    errs() << "\n";
}
class TPCRegisterCounter : public MachineFunctionPass {
public:
  static char ID;
  StringRef getPassName() const override { return PassDescription; }

  TPCRegisterCounter() : MachineFunctionPass(ID) {
    initializeTPCRegisterCounterPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

char TPCRegisterCounter::ID = 0;

INITIALIZE_PASS(TPCRegisterCounter, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCRegisterCounter() {
  return new TPCRegisterCounter();
}

bool TPCRegisterCounter::runOnMachineFunction(MachineFunction &MF) {
  TPCFrameLowering &FL = *const_cast<TPCFrameLowering *>(
      MF.getSubtarget<TPCSubtarget>().getFrameLowering());
  unsigned ScalarSz = FL.getScalarDataSize();
  unsigned VectorSz = FL.getVectorDataSize();
  unsigned SpillScalarSz = FL.getSpillScalarDataSize();
  unsigned SpillVectorSz = FL.getSpillVectorDataSize();

  unsigned MaxSpillVlm = MF.getSubtarget<TPCSubtarget>().getTargetLowering()->getTargetMachine().Options.SpillVlm;
  if (MaxSpillVlm) {
    if (SpillVectorSz > MaxSpillVlm) {
      std::ostringstream Msg;
      Msg << "Too much vector memory is used for vector spills: "
          << SpillVectorSz << " is used, but only " << MaxSpillVlm << " is available\n";
      report_fatal_error(Msg.str(), false);
    }
  }

  bool RegMemCounter = MF.getSubtarget<TPCSubtarget>().getTargetLowering()->getTargetMachine().Options.RegMemCount;
  if (!RegMemCounter && !EnableSpillCounter) {
    return false;
  }

  // If Vector memory is not used, the initial offset is not taken into account.
  if (VectorSz == 256) {
      VectorSz = 0;
  }

  const TPCInstrInfo *TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  const TPCRegisterInfo &RI = TII->getRegisterInfo();

  std::set<unsigned> SRF;
  std::set<unsigned> VRF;
  std::set<unsigned> IRF;
  std::set<unsigned> SPRF;
  std::set<unsigned> VPRF;
  std::set<unsigned> ADRF;

  for (auto &BB : MF) {
    for (auto &MI : BB) {
      for(auto &MO : MI.operands()){
        if (!MO.isReg())
          continue;
        if (!MO.getReg().isPhysical())
          continue;
        unsigned R = MO.getReg();
        if (R) {
          const TargetRegisterClass *RegClass =  TII->getClassOfPhysicalRegister(R, RI);
          if (RegClass == &TPC::VRFRegClass) {
            VRF.insert(R);
          } else if (RegClass == &TPC::SRFRegClass) {
            SRF.insert(R);
          } else if (RegClass == &TPC::IRFRegClass) {
            IRF.insert(R);
          } else if (RegClass == &TPC::VPRFRegClass) {
            VPRF.insert(R);
          } else if (RegClass == &TPC::SPRFRegClass) {
            SPRF.insert(R);
          } else if (RegClass == &TPC::ADRFRegClass) {
            ADRF.insert(R);
          }
        }
      }
    }
  }
  if (RegMemCounter) {
    errs() << "Total SLM used: " << ScalarSz + SpillScalarSz << " bytes\n";
    errs() << "Total VLM used: " << VectorSz + SpillVectorSz << " bytes\n";
  }
  if (EnableSpillCounter) {
      errs() << "SLM used: " << ScalarSz << " bytes\n";
      errs() << "Spill SLM used: " << SpillScalarSz << " bytes\n";
      errs() << "VLM used: " << VectorSz << " bytes\n";
      errs() << "Spill VLM used: " << SpillVectorSz << " bytes\n";
  }

  printRegisterSet(VRF, "VRF");
  printRegisterSet(SRF, "SRF");
  printRegisterSet(VPRF,"VPRF");
  printRegisterSet(SPRF,"SPRF");
  printRegisterSet(IRF, "IRF");
  printRegisterSet(ADRF,"ADRF");
  return true;
}
