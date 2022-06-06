#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#define DEBUG_TYPE "hwwa"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCExpandHWRegCopies();
void initializeTPCExpandHWRegCopiesPass(PassRegistry&);
}

static const char PassDescription[] = "TPC Expand Copy HWReg with MOV";
static const char PassName[] = "tpc-expand-copy-hwreg";

static cl::opt<bool>
EnableTPCExpandHWRegCopies(PassName,
              cl::desc(PassDescription),
              cl::init(true), cl::Hidden);

namespace {
class TPCExpandHWRegCopies : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCExpandHWRegCopies() : MachineFunctionPass(ID) {
  initializeTPCExpandHWRegCopiesPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

char TPCExpandHWRegCopies::ID = 0;

INITIALIZE_PASS(TPCExpandHWRegCopies, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCExpandHWRegCopies() {
  return new TPCExpandHWRegCopies();
}


bool TPCExpandHWRegCopies::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;
  if (!EnableTPCExpandHWRegCopies)
    return false;
  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  NumReplaced = 0;
  MachineBasicBlock *MBB;
  MachineInstrBuilder MIB;
  typedef enum {none, zpreg} TCopyKind;
  for (MachineFunction::iterator MBBI = MF->begin(), MBBE = MF->end();
       MBBI != MBBE; ++MBBI) {
    MBB = &*MBBI;
    for (MachineBasicBlock::iterator mi = MBB->begin(), me = MBB->end();
         mi != me;) {
      MachineBasicBlock::iterator nmi = std::next(mi);
      MachineInstr *MI = &*mi;
      auto opc = MI->getOpcode();
      if (opc == TargetOpcode::COPY) {
        MachineOperand des = MI->getOperand(0);
        if (des.isReg()) {
          Register destreg = des.getReg();
          TCopyKind CopyKind = TCopyKind(none);
          if (destreg.isPhysical()) {
            if (destreg == TPC::ZP_REG)
              CopyKind = TCopyKind(zpreg);
          } else {
            const TargetRegisterClass *RC;
            const MachineRegisterInfo &MRI =
                MI->getParent()->getParent()->getRegInfo();
            RC = MRI.getRegClass(destreg);
            if (TPC::HWZPRegRegClass.hasSubClassEq(RC)) {
              CopyKind = TCopyKind(zpreg);
            }
          }
          if (CopyKind == TCopyKind(zpreg)) {
            MIB = BuildMI(*MBB, mi, MI->getDebugLoc(),TII->get(TPC::MOV_ld_hvp),destreg);
            MIB.add(MI->getOperand(1));
            MIB.addImm(1);
            MIB.addReg(destreg, RegState::Undef);
            MIB.addReg(TPC::SPRF_TRUE);
            MIB.addImm(0);

            MI->removeFromParent();
            NumReplaced++;
          }
        }
      }
      mi = nmi;
    }
  }
  return NumReplaced > 0;
}
