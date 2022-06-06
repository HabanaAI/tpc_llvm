//===---- TPCAddrOpt.cpp --- Optimizes  st/ld_l_v  instructions -----------===//

/* This optimization is relevant only for dali, so it became deprecated and excluded from passes
 * It could be found useful if before ld_l_v will be found asignment to some SRF s=imm
 */

#ifdef DEPRECATED_TPC_ADDR_OPT
//===----------------------------------------------------------------------===//
//
// This pass:
// - Swap base and offset in instruction
//  mov.i32 %S0, 0x1d00           ===>   mov.i32 %S0, 0x00
//  ld_l_v %V0, %S0, 0x0, %SP0           ld_l_v %V0, %S0, 0x0, %SP0
//  Swapping  base and offset in the 'ld_l_v' address will permit 
//  eliminate all those 'mov' instructions, 
//  and in some cases save a number of scalar registers.
//===----------------------------------------------------------------------===//
#include "llvm/ADT/SmallSet.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
using namespace llvm;

namespace llvm {
FunctionPass *createTPCAddrOpt();
void initializeTPCAddrOptPass(PassRegistry&);
}

static const char PassDescription[] = "TPC address optimization";
static const char PassName[] = "tpc-addr-opt";

// Flag to disable register balancing.
static cl::opt<bool>
EnableTPCAddrOpt(PassName,
             cl::desc("swap base and offset in ld/st_l_v instruction"),
             cl::init(true), cl::Hidden);

namespace {
class TPCAddrOpt : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCAddrOpt() : MachineFunctionPass(ID) {
  initializeTPCAddrOptPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

char  TPCAddrOpt::ID = 0;

INITIALIZE_PASS(TPCAddrOpt, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCAddrOpt() {
  return new TPCAddrOpt();
}

SmallSet<MachineInstr *, 4> PreventLooping;
static bool  extract_imm(int64_t* imm, MachineInstr *ElementDef, MachineFunction *MF)
{
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  int opc = ElementDef->getOpcode();

  if (PreventLooping.find(ElementDef) != PreventLooping.end()) {
    // already there  -means looping
    return false;
  }
  else {
    PreventLooping.insert(ElementDef);
  }
  if (opc == TPC::MOVsip) {
    MachineOperand opnd1 = ElementDef->getOperand(1);
    int nopnd = ElementDef->getNumOperands();
    if (nopnd > 3) {
      MachineOperand opndP  = ElementDef->getOperand(nopnd-2);
      MachineOperand opndPP = ElementDef->getOperand(nopnd-1);

      if (opnd1.isImm()  &&
          opndP.isReg()  && (opndP.getReg() == TPC::SPRF_TRUE) &&
	  opndPP.isImm() && (opndPP.getImm() == 0)) {
        *imm = opnd1.getImm();
        PreventLooping.clear();
        return true;
      }
    }
  }
  else if (ElementDef->isCopy()) {
    MachineOperand opnd1 = ElementDef->getOperand(1);
    if (opnd1.isReg()) {
      unsigned int reg1 = opnd1.getReg();
      if (MRI->hasOneDef(reg1)) {
        // this is still SSA, but may be no def at all (arg)
        if (extract_imm(imm, MRI->getVRegDef(reg1), MF)) {
          PreventLooping.clear();
          return true;
        }
      }
    }
  }
  else if (ElementDef->isPHI()) {
    int nopnd = ElementDef->getNumOperands();
    int64_t imm1;
    bool im_init = false;
    for (int i = 1; i < nopnd; i += 2) {
      MachineOperand opnd = ElementDef->getOperand(i);
      int64_t imm2;
      if (opnd.isReg()) {
        unsigned int reg = opnd.getReg();
        if (MRI->hasOneDef(reg)) {
          MachineInstr * phidef = MRI->getVRegDef(reg);
          if (extract_imm(&imm2, phidef, MF)) {
            if (i == 1) {
              imm1 = imm2;
              im_init = true;
            }
            else {
              if (imm2 != imm1) {
                return false;
              }
            }
          }
          else {
            return false;
          }
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }
    if (im_init) {
      *imm = imm1;
      PreventLooping.clear();
      return true;
    }
  }
  else if (opc == TPC::ORsip) {
    MachineOperand opnd1 = ElementDef->getOperand(1);
    MachineOperand opnd2 = ElementDef->getOperand(2);
    int64_t immo2;
    MachineOperand opnd3 = ElementDef->getOperand(3);
    MachineOperand opnd4 = ElementDef->getOperand(4);
    MachineOperand opnd5 = ElementDef->getOperand(5);
    MachineOperand opnd6 = ElementDef->getOperand(6);
    MachineOperand opnd7 = ElementDef->getOperand(7);
    if (!(opnd7.isImm() && opnd7.getImm() == 0)) {
      return false;
    }
    if (opnd6.isReg()) {
      unsigned reg6 = opnd6.getReg();
      if (reg6 != TPC::SPRF_TRUE) {
        return false;
      }
    }
    if (opnd5.isReg() && opnd5.isTied()) {
      unsigned regd = opnd5.getReg();
      MachineInstr * MID = MRI->getVRegDef(regd);
      if (MID->getOpcode() != TPC::IMPLICIT_DEF) {
        return false;
      }
    }

    if (!(opnd4.isImm() && opnd4.getImm() == 0)) {
      return 0;
    }
    if (!(opnd3.isImm() && opnd3.getImm() == 2)) {
      return 0;
    }
    if (opnd2.isImm()) {
      immo2 = opnd2.getImm();
    }
    else {
      return false;
    }
    if (opnd1.isReg()) {
      unsigned int reg = opnd1.getReg();
      if (MRI->hasOneDef(reg)) {
        int64_t imm1;
        if (extract_imm(&imm1, MRI->getVRegDef(reg), MF)) {
          *imm = imm1 | immo2;
          PreventLooping.clear();
          return true;
        }
      }
    }
  }
  return false;
}


bool TPCAddrOpt::runOnMachineFunction(MachineFunction &Func) 
{
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableTPCAddrOpt)
    return false;
  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  auto Features = MF->getSubtarget().getFeatureBits();
  bool is_dali= Features[TPC::FeatureGoya];
  if (!is_dali) {
    return false;
  }

  NumReplaced = 0;
  MachineBasicBlock *MBB;
  unsigned zerreg = 0;

  for (MachineFunction::iterator MBBI = MF->begin(), MBBE = MF->end();
    MBBI != MBBE; ++MBBI) {
    MBB = &*MBBI;
    for (MachineBasicBlock::iterator mi = MBB->begin(), me = MBB->end();
      mi != me; ) {
      MachineBasicBlock::iterator nmi = std::next(mi);
      MachineInstr *MI = &*mi;
      if (MI->getOpcode()==TPC::MOVnodce) {
        zerreg = MI->getOperand(0).getReg();
      }
      bool isld = TPCII::is_ld_l_v(MI->getDesc());
      bool isst = TPCII::is_st_l_v(MI->getDesc());

      if (isld || isst) {
        PreventLooping.clear();
        int opndcnt = (isld) ? 1 : 0;
        MachineOperand opnd1 = MI->getOperand(opndcnt);
        MachineOperand opnd2 = MI->getOperand(opndcnt+1);
        if (opnd2.isImm() && opnd1.isReg() && opnd2.getImm() == 0) {
          unsigned int reg1 = opnd1.getReg();
          if (!MRI->hasOneDef(reg1) || zerreg == 0) { // if arg - no def
            mi = nmi;
            continue;
          }
          MachineInstr *ElementDef = MRI->getVRegDef(reg1);
          int64_t imm; 
          if (extract_imm(&imm, ElementDef, MF)) {
            MachineInstrBuilder MIB;
            unsigned int i;
            if (isld) {
              MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), MI->getDesc(), MI->getOperand(0).getReg());
              MIB.addReg(zerreg);
              MIB.addImm(imm);
              for (i = 3; i< MI->getNumOperands(); i++)
                MIB.add(MI->getOperand(i));
              if (!MI->memoperands_empty()) {
                for (auto mo : MI->memoperands()) {
                  MIB.addMemOperand(mo);
                }
              }
            }
            else {
              MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), MI->getDesc());
              MIB.addReg(zerreg);
              MIB.addImm(imm);
              for (i = 2; i< MI->getNumOperands(); i++)
                MIB.add(MI->getOperand(i));
              if (!MI->memoperands_empty()) {
                for (auto mo : MI->memoperands()) {
                  MIB.addMemOperand(mo);
                }
              }
            }
            MI->removeFromParent();
            ++NumReplaced;
          }
        }
      }
      mi = nmi;
    }
  }
 
  return NumReplaced > 0;
}
#endif