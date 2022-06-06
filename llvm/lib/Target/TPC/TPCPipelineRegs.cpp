//===- TPCPipelineRegs.cpp ---- Pipeline LD/ST on same register -----------===//
//
//===----------------------------------------------------------------------===//
// This pass creates a pipeline based on latency computation.
//
// Initial sequence:
//
// ld_l_v  %V0, 0x4a00, %SP0;      ;       ;       nop
// nop;    ;       ;       nop
// nop;    ;       ;       nop
// nop;    ;       ;       nop
// nop;    ;       ;       st_l_v  %S17, %V0, %SP0
// ld_l_v  %V0, 0x4b00, %SP0;      nop;    nop;    nop
// nop
// nop
// nop
// nop;    nop;    nop;    st_l_v  %S16, %V0, %SP0
//
// Optimized sequence:
//
// ld_l_v  %V0, 0x4a00, %SP0;      ;       ;       nop
// ld_l_v  %V0, 0x4b00, %SP0;      nop;    nop;    nop
// nop;    ;       ;       nop
// nop;    ;       ;       nop
// nop;    ;       ;       st_l_v  %S17, %V0, %SP0
// nop;    nop;    nop;    st_l_v  %S16, %V0, %SP0
//
// It relies on latency of update to the same register to
// generate a pipeline.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCRegisterInfo.h"
#include "TPCSubtarget.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "pipeline-regs"

static cl::opt<bool> TPCEnablePipelineRegs(
    "tpc-enable-pipeline-regs", cl::Hidden, cl::ZeroOrMore, cl::init(true),
    cl::desc("Enable LD/ST pipelining of same register."));

namespace llvm {
FunctionPass *createTPCPipelineRegs();
void initializeTPCPipelineRegsPass(PassRegistry &);
} // namespace llvm

static const char PassDescription[] = "Pipeline for same register";
static const char PassName[] = "TPCPipelineRegs";

namespace {

class TPCPipelineRegs : public MachineFunctionPass {

public:
  static char ID;
  StringRef getPassName() const override { return PassDescription; }
  TPCPipelineRegs() : MachineFunctionPass(ID) {
    initializeTPCPipelineRegsPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char TPCPipelineRegs::ID = 0;

INITIALIZE_PASS(TPCPipelineRegs, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCPipelineRegs() { return new TPCPipelineRegs(); }

//
// isFullNopInstr: returns true if MI is a full NOP instruction
//
static bool isFullNopInstr(const MachineInstr &MI) {
  if (MI.isBundle()) {
    const MachineBasicBlock *MBB = MI.getParent();
    MachineBasicBlock::const_instr_iterator MII = MI.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &BMI = *MII;
      if (!isFullNopInstr(BMI)) {
        return false;
      }
    }
    return true;
  } else {
    if (MI.getOpcode() == TPC::NOPv || MI.getOpcode() == TPC::NOPs ||
        MI.getOpcode() == TPC::NOPld || MI.getOpcode() == TPC::NOPst) {
      return true;
    }
  }
  return false;
}

static bool hasLD_L_V(const MachineInstr &MI) {
  if (isFullNopInstr(MI))
    return false;

  if (MI.isBundle()) {
    const MachineBasicBlock *MBB = MI.getParent();
    auto MII = MI.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &BMI = *MII;
      if (!isFullNopInstr(BMI) && !TPCII::is_ld_l_v(BMI.getDesc())) {
        return false;
      }
    }
    return true;
  }

  return TPCII::is_ld_l_v(MI.getDesc());
}

static bool hasST_L_V(const MachineInstr &MI) {
  if (isFullNopInstr(MI))
    return false;

  if (MI.isBundle()) {
    const MachineBasicBlock *MBB = MI.getParent();
    auto MII = MI.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &BMI = *MII;
      if (!isFullNopInstr(BMI) && !TPCII::is_st_l_v(BMI.getDesc())) {
        return false;
      }
    }
    return true;
  }

  return TPCII::is_st_l_v(MI.getDesc());
}

// Check if FirstI modifies a register that SecondI reads.
static bool hasWriteToReadDep(const MachineInstr &Load,
                              const MachineInstr &Store, unsigned &Reg,
                              const TargetRegisterInfo *TRI) {

  // Get the load store instruction if MI is a bundle.
  const MachineInstr *FirstI = &Load, *SecondI = &Store;
  if (Load.isBundle() && Store.isBundle()) {
    FirstI = nullptr;
    SecondI = nullptr;

    const MachineBasicBlock *MBB = Load.getParent();
    auto MII = Load.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr *MI = &*MII;
      if (TPCII::is_ld_l_v(MI->getDesc())) {
        FirstI = MI;
        break;
      }
    }

    MII = Store.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr *MI = &*MII;
      if (TPCII::is_st_l_v(MI->getDesc())) {
        SecondI = MI;
        break;
      }
    }
  }

  for (auto &MO : FirstI->operands()) {
    if (!MO.isReg() || !MO.isDef())
      continue;
    unsigned R = MO.getReg();
    if (SecondI->readsRegister(R, TRI)) {
      Reg = R;
      return true;
    }
  }
  return false;
}

#ifndef NDEBUG
//
// printMI: debug print of MI instruction. If MI is a bundle prints all
// instructions in that bundle (excluding NOPs)
//
static void printMI(MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock *MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &DefMI = *MII;
      if (!isFullNopInstr(DefMI)) {
        dbgs() << "      " << DefMI;
      }
    }
  } else {
    dbgs() << "    " << *MI;
  }
}
#endif

bool TPCPipelineRegs::runOnMachineFunction(MachineFunction &MF) {

  if (!TPCEnablePipelineRegs)
    return false;

  if (skipFunction(MF.getFunction()))
    return false;

  bool Change = false;

  LLVM_DEBUG(dbgs() << "\nStart pipelining registers...");
  const TPCRegisterInfo *TRI =
      MF.getSubtarget<TPCSubtarget>().getRegisterInfo();

  for (auto &MBB : MF)
    for (auto It = MBB.begin(), End = MBB.end(); It != End;) {
      // TODO: Expand for other types.
      if (!hasLD_L_V(*It)) {
        ++It;
        continue;
      }

      // Get the insert location for load pipeline.
      auto &LoadInst = *It;
      auto LoadInsertIt = std::next(It);

      // Calculate the cycle distance to figure out how many
      // instructions can be pipelined.
      unsigned CycleDist = 0;
      while (++It != MBB.end() && isFullNopInstr(*It))
        ++CycleDist;

      if (It == MBB.end())
        break;

      if (!hasST_L_V(*It)) {
        ++It;
        continue;
      }

      auto &StoreInst = *It;
      unsigned CommonReg = 0;
      if (!hasWriteToReadDep(LoadInst, StoreInst, CommonReg, TRI)) {
        ++It;
        continue;
      }

      auto StoreInsertIt = ++It;
      // Break if this is end of sequence.
      if (It == MBB.end())
        break;

      // We have the cycle distance now. Collect the loads/stores
      // for pipeline generation.
      SmallVector<MachineBasicBlock::iterator, 4> LoadIters, StoreIters;
      LLVM_DEBUG(dbgs() << "\nCycle distance : " << CycleDist);
      unsigned SequenceCount = 0, PipeCount = 0;

      while (SequenceCount++ < CycleDist) {
        if (It == MBB.end())
          break;

        if (!hasLD_L_V(*It)) {
          ++It;
          break;
        }

        auto LoadIt = It;
        unsigned CycleCount = 0;
        SmallVector<MachineBasicBlock::iterator, 4> NOPIters;
        while (++It != MBB.end() && isFullNopInstr(*It)) {
          NOPIters.push_back(It);
          ++CycleCount;
        }

        if (CycleCount != CycleDist || It == MBB.end())
          break;

        if (!hasST_L_V(*It)) {
          ++It;
          break;
        }

        unsigned Reg = 0;
        if (!hasWriteToReadDep(LoadInst, StoreInst, Reg, TRI) ||
            Reg != CommonReg) {
          ++It;
          break;
        }

        for (auto NOPIt : NOPIters)
          MBB.erase(NOPIt);

        LoadIters.push_back(LoadIt);
        StoreIters.push_back(It);
        // Increment the iterator after getting one paired sequence.
        ++It;
        ++PipeCount;
      }

      if ((LoadIters.size() > 0) && (StoreIters.size() > 0)) {
        LLVM_DEBUG(dbgs() << "\nFound LD/ST pipeline sequence...");
        LLVM_DEBUG(dbgs() << "\nPrinting pipelined sequence...\n");

        LLVM_DEBUG(printMI(&LoadInst); {
          for (auto &MIter : LoadIters)
            printMI(&*MIter);
        };
                   printMI(&StoreInst); {
                     for (auto &MIter : StoreIters)
                       printMI(&*MIter);
                   });
        // Now rearrange the sequence of loads/stores to generate
        // pipeline.
        for (auto &MIIter : StoreIters) {
          MBB.splice(StoreInsertIt, &MBB, MIIter);
        }

        for (auto &MIIter : LoadIters) {
          MBB.splice(LoadInsertIt, &MBB, MIIter);
        }

        // Remove the NOPs in between.
        for (std::size_t i = 0; i < PipeCount; ++i) {
          LoadInsertIt = MBB.erase(LoadInsertIt);
        }

        Change = true;
      }
    }

  return Change;
}
