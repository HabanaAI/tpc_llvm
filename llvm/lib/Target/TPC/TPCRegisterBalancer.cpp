//===---- TPCRegisterBalancer.cpp --- Optimizes predicates ----------------===//
//
//===----------------------------------------------------------------------===//
//
// This pass:
// - Rebalance register pressure reducing that on VRF at the expenses of SRF.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
using namespace llvm;

namespace llvm {
FunctionPass *createTPCRegisterBalancer();
void initializeTPCRegisterBalancerPass(PassRegistry&);
}

static const char PassDescription[] = "TPC register balancer";
static const char PassName[] = "tpc-rbalance";

// Flag to disable register balancing.
static cl::opt<bool>
EnableRegisterBalancer("reg-balancer",
                       cl::desc("Move register pressure from VRF to SRF (default=true)"),
                       cl::init(true), cl::Hidden);

namespace {
class TPCRegisterBalancer : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCRegisterBalancer() : MachineFunctionPass(ID) {
    initializeTPCRegisterBalancerPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

char TPCRegisterBalancer::ID = 0;

INITIALIZE_PASS(TPCRegisterBalancer, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCRegisterBalancer() {
  return new TPCRegisterBalancer();
}

static bool isScalarSplat(const MachineInstr &MI) {
  if (MI.getOpcode() == TPC::MOVvsp) {
    const MCInstrDesc &Desc = MI.getDesc();
    return MI.getOperand(Desc.getNumOperands() - 1).getImm() == 0 &&
           MI.getOperand(Desc.getNumOperands() - 2).getReg() == TPC::SPRF_TRUE;
  }
  return false;
}

static bool isImmSplat(const MachineInstr &MI) {
  if (MI.getOpcode() == TPC::MOVvip) {
    const MCInstrDesc &Desc = MI.getDesc();
    return MI.getOperand(Desc.getNumOperands() - 1).getImm() == 0 &&
           MI.getOperand(Desc.getNumOperands() - 2).getReg() == TPC::SPRF_TRUE;
  }
  return false;
}

static bool hasSplatVariant(const MachineInstr &MI, unsigned &RegVariant, unsigned &ImmVariant) {
  switch (MI.getOpcode()) {
  case TPC::ADDvvp: RegVariant = TPC::ADDvsp; ImmVariant = TPC::ADDvip; return true;
  case TPC::SUBvvp: RegVariant = TPC::SUBvsp; ImmVariant = TPC::SUBvip; return true;
  case TPC::ASHvvp: RegVariant = TPC::ASHvsp; ImmVariant = TPC::ASHvip; return true;
  case TPC::ANDvvp: RegVariant = TPC::ANDvsp; ImmVariant = TPC::ANDvip; return true;
  case TPC::ORvvp:  RegVariant = TPC::ORvsp;  ImmVariant = TPC::ORvip;  return true;
  case TPC::XORvvp: RegVariant = TPC::XORvsp; ImmVariant = TPC::XORvip; return true;
  case TPC::SHRvvp: RegVariant = TPC::SHRvsp; ImmVariant = TPC::SHRvip; return true;
  case TPC::SHLvvp: RegVariant = TPC::SHLvsp; ImmVariant = TPC::SHLvip; return true;
  default:
    return 0;
  }
}


// Number of the first operation argument.
const unsigned OpArg1No = 1;
// Number of the argument that may be vector/scalar/immediate.
const unsigned OpArg2No = 2;
// Number of the source of MOV operation.
const unsigned MovSrcNo = 1;

bool TPCRegisterBalancer::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableRegisterBalancer)
    return false;

  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  NumReplaced = 0;
  for (auto &BB : Func) {
    for (auto IPtr = BB.begin(), EPtr = BB.end(); IPtr != EPtr;) {
      MachineInstr &I = *IPtr++;
      unsigned RegVariant, ImmVariant;
      if (hasSplatVariant(I, RegVariant, ImmVariant)) {
        unsigned ElementRegNo = I.getOperand(OpArg2No).getReg();
        assert(MRI->hasOneDef(ElementRegNo));
        MachineInstr *ElementDef = MRI->getVRegDef(ElementRegNo);
        assert(ElementDef);
        MachineInstr *NewOp = nullptr;
        MachineInstrBuilder MIB;
        unsigned NewValueReg = 0;
        if (isImmSplat(*ElementDef)) {
          assert(ElementDef->getOperand(MovSrcNo).isImm());
          // Create OPvip instread of OPvvv.
          NewValueReg = MRI->createVirtualRegister(&TPC::VRFRegClass);
          MIB = BuildMI(BB, I, I.getDebugLoc(), TII->get(ImmVariant), NewValueReg)
            .addReg(I.getOperand(OpArg1No).getReg(), getRegState(I.getOperand(OpArg1No)))
            .addImm(ElementDef->getOperand(MovSrcNo).getImm());
          NewOp = MIB;
        } else if (isScalarSplat(*ElementDef)) {
          assert(ElementDef->getOperand(MovSrcNo).isReg());
          // Create OPvsp instread of OPvvp.
          NewValueReg = MRI->createVirtualRegister(&TPC::VRFRegClass);
          MIB = BuildMI(BB, I, I.getDebugLoc(), TII->get(RegVariant), NewValueReg)
            .addReg(I.getOperand(OpArg1No).getReg(), getRegState(I.getOperand(OpArg1No)))
            .addReg(ElementDef->getOperand(MovSrcNo).getReg());
          NewOp = MIB;
        }
        if (NewValueReg) {
          // Assume that all supported operations have the same format? like:
          //       ADDvsp $op1, $op2, OpType.INT32, (i8 sw), (IMPLICIT_DEF), SP0, (i1 0)
          assert(I.getOperand(3).isImm());  // OpType
          assert(I.getOperand(4).isImm());  // Switches
          assert(I.getOperand(5).isReg());  // Income
          assert(I.getOperand(6).isReg());  // Predicate
          assert(I.getOperand(7).isImm());  // Polarity

          MIB.addImm(I.getOperand(3).getImm());
          MIB.addImm(I.getOperand(4).getImm());
          MIB.addReg(I.getOperand(5).getReg());
          MIB.addReg(I.getOperand(6).getReg());
          MIB.addImm(I.getOperand(7).getImm());

          MRI->replaceRegWith(I.getOperand(0).getReg(), NewValueReg);
          I.eraseFromParent();
          ++NumReplaced;
        }
      }
    }
  }


  return NumReplaced > 0;
}
