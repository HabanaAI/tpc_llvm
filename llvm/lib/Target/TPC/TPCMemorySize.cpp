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
#include "llvm/IR/Module.h"
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
      report_fatal_error(Twine(Msg.str()), false);
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
      report_fatal_error(Twine(Msg.str()), false);
    } else {
      errs() << "Warning: " << Msg.str();
    }
  }

  Subtarget->getFrameLowering()->setScalarDataSize(ScalarSz);
  Subtarget->getFrameLowering()->setVectorDataSize(VectorSz);
}


/// The ends of the SLM and VLM a function reaches at constant addresses
/// (ld_l/st_l other than MMIO, ld_l_v/st_l_v), which the spill slots must
/// not overlap: they are placed after the statically allocated data, and
/// these accesses are not part of it. Runs before register allocation, so
/// none of the accesses is a spill. A vector is 256 bytes and is reached at
/// its address rounded down to 256 bytes.
static std::pair<unsigned, unsigned>
getConstantAddressExtents(const MachineFunction &MF) {
  unsigned SLMEnd = 0, VLMEnd = 0;
  for (const MachineBasicBlock &MBB : MF) {
    for (const MachineInstr &MI : MBB) {
      const MCInstrDesc &Desc = MI.getDesc();
      const bool IsSLM = TPCII::is_ld_l(Desc) || TPCII::is_st_l(Desc);
      const bool IsVLM =
          TPCII::is_ld_l_v_family(Desc) || TPCII::is_st_l_v_family(Desc);
      if (!IsSLM && !IsVLM)
        continue;
      if (IsSLM && TPCInstrInfo::isMMIOAccess(MI))
        continue;
      const MachineOperand &Addr = MI.getOperand(TPCII::isLoadInst(Desc) ? 1 : 0);
      if (!Addr.isImm() || Addr.getImm() < 0)
        continue;
      const uint64_t A = Addr.getImm();
      if (IsSLM)
        SLMEnd = std::max<uint64_t>(SLMEnd, alignTo(A + 4, 4));
      else
        VLMEnd = std::max<uint64_t>(VLMEnd, alignTo(A + 1, 256));
    }
  }
  return {SLMEnd, VLMEnd};
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
  const TPCFrameLowering &FL = *Subtarget->getFrameLowering();
  const auto [SLMEnd, VLMEnd] = getConstantAddressExtents(MF);
  FL.setScalarDataSize(std::max(FL.getScalarDataSize(), SLMEnd));
  FL.setVectorDataSize(std::max(FL.getVectorDataSize(), VLMEnd));
  return true;
}
