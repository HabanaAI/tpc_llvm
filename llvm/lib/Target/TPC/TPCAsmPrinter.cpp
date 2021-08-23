//===-- TPCAsmPrinter.cpp - Convert TPC LLVM code to AT&T assembly --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to TPC machine code.
//
//===----------------------------------------------------------------------===//

#include "TPCAsmPrinter.h"
#include "TPCTargetMachine.h"
#include "MCTargetDesc/TPCInstPrinter.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MachineValueType.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern Target TheTPCTarget;

// Force static initialization.
extern "C" void LLVMInitializeTPCAsmPrinter() {
  RegisterAsmPrinter<TPCAsmPrinter> X(TheTPCTarget);
}

void TPCAsmPrinter::EmitInstruction(const MachineInstr* MI) {
  if (MI->isBundle()) {
    MCInst MCB = TPCMCInstrInfo::createBundle();
    const MachineBasicBlock* MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    unsigned IgnoreCount = 0;

    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII)
    {
      MCInst *instb = new (OutContext) MCInst;

      if (MII->getOpcode() == TargetOpcode::DBG_VALUE ||
          MII->getOpcode() == TargetOpcode::IMPLICIT_DEF ||
          MII->getOpcode() == TPC::LOOPEND)
        ++IgnoreCount;
      else
      {
        instb->setOpcode(MII->getOpcode());
        for (unsigned i = 0; i < MII->getNumOperands(); ++i) {
          MachineOperand sOp = MII->getOperand(i);
          if (sOp.isImm()) {
            instb->addOperand(MCOperand::createImm(sOp.getImm()));
          } else if (sOp.isFPImm()) {
             // TODO: Think about a better way than turn floats to ints
             APFloat Val = sOp.getFPImm()->getValueAPF();
             instb->addOperand(MCOperand::createImm(Val.bitcastToAPInt().getZExtValue()));
          } else if (sOp.isReg()) {
             instb->addOperand(MCOperand::createReg(sOp.getReg()));
          } else if (sOp.isMBB()) {
             MCOperand MCOp = MCOperand::createExpr(MCSymbolRefExpr::create(
                                	   sOp.getMBB()->getSymbol(), this->OutContext));
             instb->addOperand(MCOp);
           } else if (sOp.isBlockAddress()) {
             MCSymbol *Sym = GetBlockAddressSymbol(sOp.getBlockAddress());
             MCOperand MCOp = MCOperand::createExpr(
                 MCSymbolRefExpr::create(Sym, this->OutContext));
             instb->addOperand(MCOp);
           } else {
             llvm_unreachable(
                 "Operand other than register, blockaddress or immediate");
           }
        }
        MCB.addOperand(MCOperand::createInst(instb));
     }
   }
   OutStreamer->EmitInstruction(MCB, getSubtargetInfo());
   return;
  }

  MCInst inst;
  switch(MI->getOpcode()) {
  case TPC::LOOPEND: return;
  default:
    inst.setOpcode(MI->getOpcode());
    for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
      MachineOperand sOp = MI->getOperand(i);
      if (sOp.isImm()) {
        inst.addOperand(MCOperand::createImm(sOp.getImm()));
      } else if (sOp.isFPImm()) {
        // TODO: Think about a better way than turn floats to ints
        APFloat Val = sOp.getFPImm()->getValueAPF();
        inst.addOperand(MCOperand::createImm(Val.bitcastToAPInt().getZExtValue()));
      } else if (sOp.isReg()) {
        inst.addOperand(MCOperand::createReg(sOp.getReg()));
      } else if (sOp.isMBB()) {
        MCOperand MCOp = MCOperand::createExpr(MCSymbolRefExpr::create(
                                      sOp.getMBB()->getSymbol(), this->OutContext));
        inst.addOperand(MCOp);
      } else if (sOp.isBlockAddress()) {
        MCSymbol *Sym = GetBlockAddressSymbol(sOp.getBlockAddress());
        MCOperand MCOp = MCOperand::createExpr(
            MCSymbolRefExpr::create(Sym, this->OutContext));
        inst.addOperand(MCOp);
      } else {
        llvm_unreachable(
            "Operand other than register, blockaddress or immediate");
      }
    }
    break;
  }

  OutStreamer->EmitInstruction(inst, getSubtargetInfo());
}

static void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNo);
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << TPCInstPrinter::getRegisterName(MO.getReg());
    break;
  default:
    llvm_unreachable("not implemented");
  }
}

bool TPCAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                    const char *ExtraCode, raw_ostream &OS) {
  // Print the operand if there is no operand modifier.
  if (!ExtraCode || !ExtraCode[0]) {
    printOperand(MI, OpNo, OS);
    return false;
  }

  // Otherwise fallback on the default implementation.
  return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, OS);
}

// This method ensures that blocks that are only linked via LOOP instruction
// are not counted as fallthrough blocks. We need it because fallthrough
// blocks do not have an address and therefore can't be referenced in a
// relocation.
bool TPCAsmPrinter::isBlockOnlyReachableByFallthrough(
  const MachineBasicBlock *MBB) const {
  if (!AsmPrinter::isBlockOnlyReachableByFallthrough(MBB)) {
    return false;
  }

  assert(MBB->pred_size() <= 1 && "Fallthrough blocks can have only one predecessor");

  // TODO: This is an overkill. We can have maximum of 4 LOOP instructions
  //       in a method. We don't need to iterate over everything.
  for (MachineFunction::const_iterator I = MF->begin(), E = MF->end(); I != E; ++I) {
    const MachineBasicBlock *Block = &*I;
    for (const MachineInstr& MI : Block->instrs()) {
      if (isLoop(MI)) {
        int idx = TPCII::getIsPredicated(MI.getDesc()) ? MI.getNumOperands() - 7 : MI.getNumOperands() - 5;
        const MachineOperand& MO = MI.getOperand(idx);

        assert(MO.isMBB() && "Last operand in a LOOP instruction should be a block");

        if (MO.getMBB() == MBB) return false;
      }
    }
  }

  return true;
}

bool TPCAsmPrinter::isLoop(const MachineInstr& MI) const {
  unsigned Opc = MI.getOpcode();
  return TPCII::isLoopInst(MI.getDesc()) && Opc != TPC::LOOPEND;
}

