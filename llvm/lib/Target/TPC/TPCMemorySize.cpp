//===---- TPCLutCacheCounter.cpp --- Optimizes predicates ----------------===//
//
//===----------------------------------------------------------------------===//
//
// This pass:
// - Generates warning when used LUT size exceeds LUT cache
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "tpc-small-vlm"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCFrameLowering.h"
#include "TPCTargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCInstPrinter.h"
#include <set>
#include <map>
#include <sstream>

using namespace llvm;

namespace llvm {
  FunctionPass *
  createTPCMemorySize();
  void
  initializeTPCMemorySizePass(PassRegistry&);
}

static const char PassDescription[] =
        "Detect Small VLM size usage";
static const char PassName[] = "tpc-small-vlm";


cl::opt<bool> IgnoreMemOverflow("ignore-mem-overflow",
                       cl::desc("Change memory overflow error to warning"),
                       cl::init(false), cl::Hidden);

namespace {

  /// Extracts a numeric type identifier from an MDNode containing type metadata.
  unsigned readIntFromMetadata(MDNode *MD) {
    auto TM = dyn_cast<ValueAsMetadata>(MD->getOperand(0));
    assert(TM);
    auto C = dyn_cast<ConstantInt>(TM->getValue());
    return C->getZExtValue();
  }

  class TPCMemorySize: public MachineFunctionPass {
  public:
    static char ID;
    StringRef getPassName() const override {
      return PassDescription;
    }

    TPCMemorySize() : MachineFunctionPass(ID) {
      initializeTPCMemorySizePass(*PassRegistry::getPassRegistry());
    }
    bool runOnMachineFunction(MachineFunction &MF) override;
  private:
    const TPCSubtarget *Subtarget;
    void setMemorySize(const Module &M);
  };
}

char TPCMemorySize::ID = 0;

INITIALIZE_PASS(TPCMemorySize, PassName, PassDescription, false, false)

FunctionPass * llvm::createTPCMemorySize() {
  return new TPCMemorySize();
}

void TPCMemorySize::setMemorySize(const Module &M) {
  unsigned ScalarSz = 0;

  if (NamedMDNode *N = M.getNamedMetadata("llvm.tpc.scalar_data")) {
    assert(N && "No data about data size");
    assert(N->getNumOperands() == 1 && "Invalid metadata format");
    MDNode *V = N->getOperand(0);
    ScalarSz = readIntFromMetadata(V);
  }

  unsigned VectorSz = 0;
  if (NamedMDNode *N = M.getNamedMetadata("llvm.tpc.vector_data")) {
    assert(N && "No data about data size");
    assert(N->getNumOperands() == 1 && "Invalid metadata format");
    MDNode *V = N->getOperand(0);
    VectorSz = readIntFromMetadata(V);
  }

  unsigned MaxScalarSz = Subtarget->getFrameLowering()->getMaxScalarMemory();
  if (ScalarSz > MaxScalarSz) {
    std::ostringstream Msg;
    Msg << "too much scalar memory is used for statically allocated data: "
        << ScalarSz << " is allocated, but only " << MaxScalarSz << " is available\n";
    if (!IgnoreMemOverflow) {
      report_fatal_error(Msg.str(), false);
    } else {
      errs() << "Warning: " << Msg.str();
    }
  }
  unsigned MaxVectorSz = Subtarget->getFrameLowering()->getMaxVectorMemory();
  if (VectorSz > MaxVectorSz) {
    std::ostringstream Msg;
    Msg << "too much vector memory is used for statically allocated data: "
        << VectorSz << " is allocated, but only " << MaxVectorSz << " is available\n";
    if (!IgnoreMemOverflow) {
      report_fatal_error(Msg.str(), false);
    } else {
      errs() << "Warning: " << Msg.str();
    }
  }

  Subtarget->getFrameLowering()->setScalarDataSize(ScalarSz);
  Subtarget->getFrameLowering()->setVectorDataSize(VectorSz);
}


bool TPCMemorySize::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &MF.getSubtarget<TPCSubtarget>();
  if(!Subtarget->getTargetLowering()->getTargetMachine().Options.LocalVectorMemory) {
    for (auto &BB : MF) {
      for (auto &MI : BB) {
        if (TPCII::isLookup(MI.getDesc())) {
          Subtarget->getFrameLowering()->setMaxVectorMemory(Subtarget->getDefaultSmallVLMSize());
          break;
        }
      }
    }
  }
  setMemorySize(*MF.getFunction().getParent());
  return true;
}
