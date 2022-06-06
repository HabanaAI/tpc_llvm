//===- TPCEventProgiler.cpp--Inject event instruction pass------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2021 - This pass is a property of Habana labs
//
// author: Michael Zuckerman
//         mzuckerman@habana.ai
//===----------------------------------------------------------------------===//
// The pass is responsible for injecting event instruction into the machine 
// code. This feature only available for gaudi2 and above. When trigger this 
// pass the pass injects an event instruction according to design in 
// /llvm/docs/TPC/design. The plan in the future to extend this pass to 
// include all kinds of event profiling.
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPC.h"
#include "TPCInstrInfo.h"
#include "TPCRegisterInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "TPCEvent"

static cl::opt<bool> TPCEvent("TPCEvent", cl::Hidden, cl::ZeroOrMore,
                              cl::init(false),
                              cl::desc("tpc event instrumentation"));

namespace llvm {
FunctionPass *createEventProfilerInjectPass();
void initializeTPCEventProfilerPass(PassRegistry &);
} // namespace llvm

namespace {
class TPCEventProfiler : public MachineFunctionPass {
public:
  static char ID;

  TPCEventProfiler() : MachineFunctionPass(ID) {
    initializeTPCEventProfilerPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "TPCEventProfiler"; }
  bool runOnMachineFunction(MachineFunction &Fn) override;
  void injectEvent(unsigned num, MachineInstr *node, MachineFunction &Func);
};

} // namespace

char TPCEventProfiler::ID = 0;
INITIALIZE_PASS_BEGIN(TPCEventProfiler, "TPCEventProfiler",
                      "TPC event instrumentation", false, false)
INITIALIZE_PASS_END(TPCEventProfiler, "TPCEventProfiler",
                    "TPC event instrumentation", false, false)

FunctionPass *llvm::createEventProfilerInject() {
  return new TPCEventProfiler();
}

void TPCEventProfiler::injectEvent(unsigned num, MachineInstr *node,
                                   MachineFunction &Func) {
  auto MF = &Func;
  auto ST = &MF->getSubtarget();
  auto TII = ST->getInstrInfo();
  MachineInstr *Event = MF->CreateMachineInstr(TII->get(TPC::EVENT_ld_ip),
                                               node->getDebugLoc(), false);
  Event->addOperand(*MF, MachineOperand::CreateImm(num));
  Event->addOperand(*MF, MachineOperand::CreateImm(0));
  Event->addOperand(*MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false));
  Event->addOperand(*MF, MachineOperand::CreateImm(0));
  MachineBasicBlock *MBB = node->getParent();
  MBB->insert(node, Event);
}

bool TPCEventProfiler::runOnMachineFunction(MachineFunction &MF) {
  if (!TPCEvent)
    return false;
  for (auto &MBB : MF) {
    MachineInstr *MI = &*MBB.instr_begin();
    injectEvent(MBB.getNumber(), MI, MF);
  }
  return true;
}
