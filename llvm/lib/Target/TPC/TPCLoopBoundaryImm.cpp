//===- TPCLoopBoundaryImm.cpp - LOOP boundaries the gaudi2 decoder takes --===//
//
// On gaudi2 the LOOP instruction's immediate boundary is encoded from bit 38
// of the VLIW (TPCMCCodeEmitter::encodeLoop), so the boundary's bits 12:7
// fall on the VPU slot's opcode field (bits 45-50). The decoder checks that
// field in a LOOP bundle too: when those six bits are an opcode gaudi2 does
// not take, the TPC stops with tpc_illegal_instruction and the driver resets
// the card. Measured on a gaudi2 card (2026-11-23, loops of NOPs): 0x21 and
// 0x38-0x3C stop it; 0x00, 0x10, 0x18, 0x28, 0x2F, 0x30, 0x33, 0x37 and
// 0x3D-0x3F do not. Neither does the start or the step, nor the trip count;
// only the boundary's value. The simulator does not check it. Habana's
// assembler encodes the boundary the same way; its compiler emitted no such
// boundary in the kernels tried.
//
// A boundary like that (7168, 15360 and 64512 in test_kernels, where it
// stopped three kernels) is given to the LOOP in a scalar register instead,
// set by a MOV right before the LOOP. The pass runs after the last pass that
// makes or changes LOOPs, before register allocation.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-loop-boundary-imm"

STATISTIC(NumBoundaries, "LOOP boundaries moved to a register");

static cl::opt<bool> EnableLoopBoundaryImm(
    "tpc-loop-boundary-to-reg", cl::Hidden, cl::init(true),
    cl::desc("On gaudi2, give a LOOP whose immediate boundary the decoder "
             "rejects the boundary in a register"));

namespace llvm {
FunctionPass *createTPCLoopBoundaryImm();
void initializeTPCLoopBoundaryImmPass(PassRegistry &);
} // namespace llvm

namespace {

class TPCLoopBoundaryImm : public MachineFunctionPass {
public:
  static char ID;
  TPCLoopBoundaryImm() : MachineFunctionPass(ID) {
    initializeTPCLoopBoundaryImmPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override {
    return "TPC LOOP boundaries the decoder takes";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // namespace

char TPCLoopBoundaryImm::ID = 0;

INITIALIZE_PASS(TPCLoopBoundaryImm, DEBUG_TYPE,
                "TPC LOOP boundaries the decoder takes", false, false)

FunctionPass *llvm::createTPCLoopBoundaryImm() {
  return new TPCLoopBoundaryImm();
}

/// Whether gaudi2 stops at a LOOP with this immediate boundary.
static bool isRejectedBoundary(int64_t Imm) {
  const unsigned VpuOpcode = (static_cast<uint32_t>(Imm) >> 7) & 0x3f;
  return VpuOpcode == 0x21 || (VpuOpcode >= 0x38 && VpuOpcode <= 0x3c);
}

/// The LOOP opcode with the boundary in a register, for one with an
/// immediate boundary; 0 for any other opcode.
static unsigned withRegisterBoundary(unsigned Opc) {
  switch (Opc) {
  case TPC::LOOPsis:   return TPC::LOOPsss;
  case TPC::LOOPiis:   return TPC::LOOPiss;
  case TPC::LOOPsii:   return TPC::LOOPssi;
  case TPC::LOOPiii:   return TPC::LOOPisi;
  case TPC::LOOPsisp:  return TPC::LOOPsssp;
  case TPC::LOOPiisp:  return TPC::LOOPissp;
  case TPC::LOOPsiip:  return TPC::LOOPssip;
  case TPC::LOOPiiip:  return TPC::LOOPisip;
  case TPC::LOOP1sisp: return TPC::LOOP1sssp;
  case TPC::LOOP1iisp: return TPC::LOOP1issp;
  case TPC::LOOP1siip: return TPC::LOOP1ssip;
  case TPC::LOOP1iiip: return TPC::LOOP1isip;
  default:             return 0;
  }
}

bool TPCLoopBoundaryImm::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableLoopBoundaryImm || !MF.getSubtarget<TPCSubtarget>().hasGaudi2ISA())
    return false;
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      const unsigned NewOpc = withRegisterBoundary(MI.getOpcode());
      if (!NewOpc)
        continue;
      // LOOP operands: start, boundary, step, comparison, exit block[, ...].
      MachineOperand &Boundary = MI.getOperand(1);
      assert(Boundary.isImm() && "an immediate-boundary LOOP");
      if (!isRejectedBoundary(Boundary.getImm()))
        continue;
      LLVM_DEBUG(dbgs() << "Boundary to a register: " << MI);
      const Register Reg = MRI.createVirtualRegister(&TPC::SRFRegClass);
      BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(TPC::MOVsip), Reg)
          .addImm(Boundary.getImm())
          .addImm(TPCII::OpType::INT32)
          .addImm(0)
          .addReg(Reg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      MI.setDesc(TII->get(NewOpc));
      Boundary.ChangeToRegister(Reg, /*isDef=*/false);
      ++NumBoundaries;
      Changed = true;
    }
  }
  return Changed;
}
