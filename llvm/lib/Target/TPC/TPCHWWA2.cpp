//===---- TPCHWWA2.cpp --- extent shift arg in  convert to all instr -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  need to extend as 0xa => 0x0a0a0a0a 
//
//===----------------------------------------------------------------------===//
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#define DEBUG_TYPE "hwwa2"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCHWWA2();
void initializeTPCHWWA2Pass(PassRegistry&);
}

static const char PassDescription[] = "TPC Hardware Workarounds for converts";
static const char PassName[] = "tpc-hwwa-workaround";

static cl::opt<bool>
EnableMaxConvert("tpc-hwwa-conv-maxint",
  cl::desc(PassDescription),
  cl::init(true), cl::Hidden);

namespace {
class TPCHWWA2 : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCHWWA2() : MachineFunctionPass(ID) {
  initializeTPCHWWA2Pass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

char  TPCHWWA2::ID = 0;

INITIALIZE_PASS(TPCHWWA2, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCHWWA2() {
  return new TPCHWWA2();
}
static int64_t extent_imm(int64_t im,int grp) {
  int wr = im;
  if (grp == 1|| grp==3) {
    wr = (wr << 8) | (wr&0xff);
  }
  wr = (wr << 16) |(wr&0xffff);
  return wr;
}

 static int extend_reg(MachineBasicBlock::iterator mi, MachineFunction& Func,int grp)
{
  auto MF = &Func;
  auto MRI = &MF->getRegInfo();
  auto ST = &MF->getSubtarget();
  auto TII = ST->getInstrInfo();
  MachineInstr *MI = &*mi;
  unsigned rego = MI->getOperand(2).getReg();
  unsigned d_or16 = rego;

  MachineInstrBuilder MIB;

  if (grp == 1 || grp ==3) {
    unsigned shift_and8= MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::ANDsip), shift_and8);
    MIB.addReg(rego);
    MIB.addImm(0xff);
    MIB.addImm(2);
    MIB.addImm(0);
    MIB.addReg(shift_and8, RegState::Undef);
    MIB.addReg(TPC::SP0);                // Pred
    MIB.addImm(0);                       // Polarity

    unsigned d_shl8 = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::SHLsip), d_shl8);
    MIB.addReg(shift_and8);
    MIB.addImm(8);
    MIB.addImm(2);
    MIB.addImm(0);
    MIB.addReg(d_shl8, RegState::Undef);
    MIB.addReg(TPC::SP0);                // Pred
    MIB.addImm(0);                       // Polarity

    d_or16 = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::ORssp), d_or16);
    MIB.addReg(d_shl8);
    MIB.addReg(shift_and8);
    MIB.addImm(2);
    MIB.addImm(0);
    MIB.addReg(d_or16, RegState::Undef);
    MIB.addReg(TPC::SP0);                // Pred
    MIB.addImm(0);                       // Polarity
  }
  else {
    unsigned shift_and16 = MRI->createVirtualRegister(&TPC::SRFRegClass);
    MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::ANDsip), shift_and16);
    MIB.addReg(rego);
    MIB.addImm(0xffff);
    MIB.addImm(2);
    MIB.addImm(0);
    MIB.addReg(shift_and16, RegState::Undef);
    MIB.addReg(TPC::SP0);                // Pred
    MIB.addImm(0);                       // Polarity

    d_or16 = shift_and16;
  }
  unsigned d_shl16 = MRI->createVirtualRegister(&TPC::SRFRegClass);
  MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::SHLsip), d_shl16);
  MIB.addReg(d_or16);
  MIB.addImm(16);
  MIB.addImm(2);
  MIB.addImm(0);
  MIB.addReg(d_shl16, RegState::Undef);
  MIB.addReg(TPC::SP0);                // Pred
  MIB.addImm(0);                       // Polarity

  unsigned d_or32 = MRI->createVirtualRegister(&TPC::SRFRegClass);
  MIB = BuildMI(*(MI->getParent()), mi, MI->getDebugLoc(), TII->get(TPC::ORssp), d_or32);
  MIB.addReg(d_shl16);
  MIB.addReg(d_or16);
  MIB.addImm(2);
  MIB.addImm(0);
  MIB.addReg(d_or32, RegState::Undef);
  MIB.addReg(TPC::SP0);                // Pred
  MIB.addImm(0);                       // Polarity

  return d_or32;
}
bool TPCHWWA2::runOnMachineFunction(MachineFunction &Func) 
{
  if (skipFunction(Func.getFunction()))
    return false;

  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  auto Features = MF->getSubtarget().getFeatureBits();
  bool is_gaudi = Features[TPC::FeatureGaudi];
  NumReplaced = 0;
  MachineBasicBlock *MBB;
  if (!EnableMaxConvert)
    return false;
  for (MachineFunction::iterator MBBI = MF->begin(), MBBE = MF->end();
    MBBI != MBBE; ++MBBI) {
    MBB = &*MBBI;
    for (MachineBasicBlock::iterator mi = MBB->begin(), me = MBB->end();
      mi != me; ) {
      MachineBasicBlock::iterator nmi = std::next(mi);
      MachineInstr *MI = &*mi;
      MachineInstrBuilder MIB;
      auto opc = MI->getOpcode();
// for switch decoding taken from .td-file
#define FP32    0
#define BF16    1
#define INT32   2
#define UINT32  3
#define INT8    4
#define UINT8   5
#define BOOL    6
#define INT16   7
#define UINT16  8

      if (opc == TPC::CONVERTvvp) {
        MachineOperand opnd2 = MI->getOperand(2);
        MachineOperand opnd3 = MI->getOperand(3);
        MachineOperand opnd4 = MI->getOperand(4);

        int64_t destreg = MI->getOperand(0).getReg();
        assert(opnd2.isImm() && opnd3.isImm()&& opnd4.isReg());
        int64_t sw = opnd3.getImm();
        int lane_sel = sw & 3;
        int target_type_to = (sw >> 8) & 0xf;
        if (!(target_type_to == INT8 || target_type_to == INT16)) goto LOOP_BOTTOM;
        int rm = (sw >> 16) & 0xf;
        int target_type_from = opnd2.getImm();
        if (!(target_type_from == FP32)) goto LOOP_BOTTOM;
        int clean_type_to = ~((0xf << 8)|0x7);
        int window = sw & clean_type_to;
        int sw_to_i32 = window | (INT32 << 8);  // sel == 0 for f32->i32   

        unsigned incomreg = MRI->createVirtualRegister(&TPC::VRFRegClass);

        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(),
                      TII->get(TPC::IMPLICIT_DEF), incomreg);

        unsigned dr2 = MRI->createVirtualRegister(&TPC::VRFRegClass);
        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), MI->getDesc(), dr2);
        MIB.add(MI->getOperand(1));
        MIB.addImm(0);
        MIB.addImm(sw_to_i32);
        MIB.addReg(incomreg);
        for (unsigned int i = 5; i < MI->getNumOperands(); i++) {
          MIB.add(MI->getOperand(i));
        }

        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), TII->get(is_gaudi?
                                TPC::CONVERT_INT32g2vip:
                                TPC::CONVERT_INT32vip), destreg);
        MIB.addReg(dr2);
        MIB.addImm(0);
        sw = ((target_type_to&1) <<19)|(rm <<15) | lane_sel;
        MIB.addImm(sw);
        for (unsigned int i = 4; i < MI->getNumOperands(); i++) {
          MIB.add(MI->getOperand(i));
        }
        MI->removeFromParent();
        ++NumReplaced;
      }
      else if (opc == TPC::CONVERTssp) {
        MachineOperand opnd2 = MI->getOperand(2);
        MachineOperand opnd3 = MI->getOperand(3);
        MachineOperand opnd4 = MI->getOperand(4);
        int64_t destreg = MI->getOperand(0).getReg();
        assert(opnd2.isImm() && opnd3.isImm() && opnd4.isReg());
        int64_t sw = opnd3.getImm();
        int lane_sel = sw & 3;
        int target_type_to = (sw >> 8) & 0xf;
        if (!(target_type_to == INT8 || target_type_to == INT16)) goto LOOP_BOTTOM;
        int rm = (sw >> 16) & 0xf;
        int target_type_from = opnd2.getImm();
        if (!(target_type_from == FP32)) goto LOOP_BOTTOM;
        int clean_type_to = ~((0xf << 8) | 0x7);
        int window = sw & clean_type_to;
        int sw_to_i32 = window | (INT32 << 8);  
        unsigned incomreg = MRI->createVirtualRegister(&TPC::SRFRegClass);
        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(),
                      TII->get(TPC::IMPLICIT_DEF), incomreg);
        unsigned dr2 = MRI->createVirtualRegister(&TPC::SRFRegClass);
        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), MI->getDesc(), dr2);
        MIB.add(MI->getOperand(1));
        MIB.addImm(0);
        MIB.addImm(sw_to_i32);
        MIB.addReg(incomreg);
        for (unsigned int i = 5; i < MI->getNumOperands(); i++) {
          MIB.add(MI->getOperand(i));
        }
        MIB = BuildMI(*MBB, mi, MI->getDebugLoc(), TII->get(is_gaudi ?
          TPC::CONVERT_INT32g2sip :
          TPC::CONVERT_INT32sip), destreg);

        MIB.addReg(dr2);
        MIB.addImm(0);
        sw = ((target_type_to&1) << 19) | (rm << 15) | lane_sel;
        MIB.addImm(sw);
        for (unsigned int i = 4; i < MI->getNumOperands(); i++) {
          MIB.add(MI->getOperand(i));
        }
        MI->removeFromParent();
        ++NumReplaced;

      }
      LOOP_BOTTOM:
      mi = nmi;
    }
  }
  return NumReplaced > 0;
}
