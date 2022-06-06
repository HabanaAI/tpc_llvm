//===---- TPCLutCacheCounter.cpp --- Optimizes predicates ----------------===//
//                     The LLVM Compiler Infrastructure:
//
//              2021 - This pass is a property of Habana labs
//
// Author: Michael Zuckerman
//===----------------------------------------------------------------------===//
//
// This pass:
// - Generates warning when used LUT size exceeds LUT cache
// 1) The pass-run over each machine basic block and check his location in the
//    loop structure.
// 2) If the basic block is in a nested loop that his depth is greater than one
//    continue to the next check.
// 3) If one of the machine instruction is a lookup instruction and violate one 
//    of the following condition produce a warning:
// a) The number of tables is exceeded the allowed number.
// b) Different interval groups in the same VLM
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "tpc-lut-count"
#include "MCTargetDesc/TPCInstPrinter.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCFrameLowering.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"
#include <map>
#include <set>

using namespace llvm;

namespace llvm {
FunctionPass *
createTPCLutCacheCounter();
void
initializeTPCLutCacheCounterPass(PassRegistry&);
}

static const char PassDescription[] =
    "Print warning if used LUT size exceeds LUT cache";
static const char PassName[] = "tpc-lut-count";

//static cl::opt<bool>
//EnableLutCounter("lut-mem-count",
//                       cl::desc("Prints warning if used LUT size exceeds LUT cache(default=true)"),
//                       cl::init(true), cl::Hidden);

namespace {

enum LookupDT {
  BV32,
  BV16,
  BV8_0,
  BV8_1
};

static std::map<unsigned, unsigned> FuncIdToTableSize {
  { 0, 256 },
  { 1, 128 },
  { 2, 64 },
  { 3, 64 }
};


class TPCLutCacheCounter: public MachineFunctionPass {
public:
  static char ID;
  StringRef getPassName() const override {
    return PassDescription;
  }

  TPCLutCacheCounter() : MachineFunctionPass(ID) {
    initializeTPCLutCacheCounterPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
private:
  std::set<unsigned> loadedFunctionsDali;
  std::map<unsigned, unsigned> loadedFunctionsGaudi;
  unsigned maxSize = 384;
  unsigned usedSize = 0;
  bool isCommomLookupUsed = false;
  bool processLookupDali(const MachineInstr &I);
  bool processLookupGen2Plus(const MachineInstr &I);
  bool isLookUpInADeepLoop(MachineBasicBlock *MBB, int &deep);
};
}

char TPCLutCacheCounter::ID = 0;

INITIALIZE_PASS(TPCLutCacheCounter, PassName, PassDescription, false, false)

FunctionPass * llvm::createTPCLutCacheCounter() {
  return new TPCLutCacheCounter();
}

bool TPCLutCacheCounter::processLookupGen2Plus(const MachineInstr &MI) {
  const MCInstrDesc &MCID = MI.getDesc();
  if (TPCII::isLookup(MCID)) {
    if (!TPCII::isLookupC(MCID)) {
      isCommomLookupUsed = true;
    }
    if (isCommomLookupUsed && TPCII::isLookupC(MCID)) {
      errs() << "Warning: mixing generic and special lookup may cause LUT cache misses and/or LUT misbehaviour.\n";
      return false;
    }
    unsigned funcId    = MI.getOperand(2).getImm();
    unsigned lookupDt  = MI.getOperand(3).getImm() & 0x7;
    unsigned tableSize = FuncIdToTableSize[(funcId >> 7)];

    if(!loadedFunctionsGaudi.emplace(funcId,lookupDt).second &&
        loadedFunctionsGaudi[funcId] == lookupDt) {
        return true;
    }
    usedSize += tableSize;
    if (lookupDt == BV32) {
      maxSize = 256;
    }
    if (usedSize > maxSize) {
      errs() << "Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Gen2+ architecture, this will cause LUT misses.\n";
      return false;
    }
  }
  return true;
}



bool TPCLutCacheCounter::processLookupDali(const MachineInstr &MI) {
  if (TPCII::isLookup(MI.getDesc())) {
    unsigned funcId = MI.getOperand(2).getImm();
    unsigned tableSize = FuncIdToTableSize[(funcId >> 4)];
    if (!loadedFunctionsDali.insert(funcId).second) {
      return true;
    }
    for (auto usedFuncId : loadedFunctionsDali) {
      unsigned usedFuncIdTableSize = FuncIdToTableSize[(usedFuncId >> 4)];
      if (tableSize != usedFuncIdTableSize) {
        errs() << "Performance warning: encountered special function IDs from different interval groups, this will cause LUT misses.\n";
        return false;
      }
    }
    unsigned numberOfTables = loadedFunctionsDali.size();
    if ((tableSize == 256 && numberOfTables > 1)
        || (tableSize == 128 && numberOfTables > 2)
        || (tableSize == 64 && numberOfTables > 4)) {
      errs() << "Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Goya architecture,"
          " this will cause LUT misses. The cache can hold 1 special function with 256 intervals or 2 special functions,"
          " each with 128 intervals or 4 special functions, each with 64 intervals.\n";
      return false;
    }
  }
  return true;
}

/**
 * @brief The isLookUpInADeepLoop function tune the LTU warning by checking the
 * Location of the basic block in the struct of the loop.
 * 
 * @param MBB machine basic block
 * @param depth of the basic block in the loop.
 * @return true if the depth is greater than 1
 * @return false if the depth of the basic block is less than 1
 */
bool TPCLutCacheCounter::isLookUpInADeepLoop(MachineBasicBlock *MBB,
                                             int &depth) {
  auto &MLI = getAnalysis<MachineLoopInfo>();
  MachineLoop *loop = MLI.getLoopFor(MBB);
  if (loop) {
    depth = loop->getLoopDepth();
    // The basic block is part of a loop. Locate the loop depth and if grather
    // then 1 return true.
    if (loop->getLoopDepth() <= 1)
      return false;
    else
      return true;
  }
  // The basic block is not part of any loop.
  return false;
}

bool TPCLutCacheCounter::runOnMachineFunction(MachineFunction &MF) {
  auto ST = &MF.getSubtarget<TPCSubtarget>();
  bool PrintlutWarn =  ST->getTargetLowering()->getTargetMachine().Options.LutWarn;
  if (!PrintlutWarn) {
    return false;
  }

  for (auto &BB : MF) {
    int deep = 0;
    if (!isLookUpInADeepLoop(&BB, deep))
      continue;
    for (auto &MI : BB) {
      if(ST->hasGoyaISA()) {
        if(!processLookupDali(MI)) {
          return false;
        }
      } else {
        if(!processLookupGen2Plus(MI)) {
          return false;
        }
      }
    }
  }
  return true;
}
