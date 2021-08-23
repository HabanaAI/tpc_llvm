//===---- TPCSetIndxCoalescer.cpp ------ Optimizes SET_INDX ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
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
FunctionPass *createTPCSetIndxCoalescer();
void initializeTPCSetIndxCoalescerPass(PassRegistry&);
}

static const char PassDescription[] = "TPC SET_INDX Coalescer";
static const char PassName[] = "tpc-int5";

const unsigned NMovType = 2;        // OpType of MOVsip
const unsigned NMovPredicate = 5;   // Predicate of MOVsip
const unsigned NMovPolarity = 6;    // Polarity of MOVsip
const unsigned NInt5 = 1;
const unsigned NElement = 2;
const unsigned NMask = 3;

// Flag to disable SET_INDX coalescing.
static cl::opt<bool>
EnableSetIndxCoalescer("set_indx-coalescer",
                   cl::desc("Coalesce SET_INDX instructions (default=true)"),
                   cl::init(true), cl::Hidden);


namespace {
class TPCSetIndxCoalescer : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  unsigned NumReplaced = 0;
  SmallSet<MachineInstr *, 32> RemovedInstrs;
  SmallSet<MachineInstr *, 32> AddedInstrs;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCSetIndxCoalescer() : MachineFunctionPass(ID) {
    initializeTPCSetIndxCoalescerPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool processSetIndx(MachineInstr *Instr);
  MachineInstr *skipRegisterLoad(MachineInstr *I);
  MachineInstr *skipUndefinedOperand(MachineInstr *I);
  void replaceRegister(unsigned FromReg, unsigned ToReg);
};
}

char TPCSetIndxCoalescer::ID = 0;

INITIALIZE_PASS(TPCSetIndxCoalescer, PassName, PassDescription, false, false)


FunctionPass *llvm::createTPCSetIndxCoalescer() {
  return new TPCSetIndxCoalescer();
}


// Checks if the given register is used only in SET_INDX instructions.
//
// Returns number of uses if the register is used only in SET_INDX instructions
// or 0 if some uses are not SET_INDX.
//
static unsigned isRegUsedOnlyInSetIndx(MachineRegisterInfo *MRI, unsigned RegNo) {
  unsigned NumUses = 0;
  for (MachineInstr &U : MRI->use_nodbg_instructions(RegNo)) {
    if (U.definesRegister(RegNo))
      continue;
    if (!isSET_INDX(U.getOpcode()))
      return 0;
    ++NumUses;
  }
  return NumUses;
}


static bool isUnpredicatedMOV(const MachineInstr *MI) {
  if (MI->getOpcode() != TPC::MOVsip)
    return false;
  if (MI->getOperand(NMovType).getImm() != TPCII::OpType::INT32)
    return false;
  if (MI->getOperand(NMovPredicate).getReg() != TPC::SP0)
    return false;
  if (MI->getOperand(NMovPolarity).getImm() != 0)
    return false;
  return true;
}

void TPCSetIndxCoalescer::replaceRegister(unsigned FromReg, unsigned ToReg) {
  assert(FromReg != ToReg && "Cannot replace a reg with itself");

  for (auto I = MRI->use_begin(FromReg), E = MRI->use_end(); I != E; ) {
    MachineOperand &O = *I;
    ++I;
    if (RemovedInstrs.count(O.getParent()))
      continue;
    assert(O.isReg());
    assert(O.getReg() == FromReg);
    assert(!O.isDef());
    O.setReg(ToReg);
  }
}


/// Returns true if the specified instruction sets element to value, which is
/// implicitly defined.
MachineInstr *TPCSetIndxCoalescer::skipUndefinedOperand(MachineInstr *I) {
  if (!isSET_INDX(I->getOpcode()))
    return nullptr;

  MachineOperand &ElementOp = I->getOperand(NElement);
  if (!ElementOp.isReg())
    return nullptr;
  unsigned ElementRegNo = ElementOp.getReg();
  MachineInstr *ElementDef = MRI->getVRegDef(ElementRegNo);
  assert(ElementDef);
  if (ElementDef->getOpcode() != TPC::IMPLICIT_DEF)
    return nullptr;

  RemovedInstrs.insert(I);

  // All users of the current instruction must use register defined by
  // its income argument.
  unsigned ReplacedReg = I->getOperand(0).getReg();
  unsigned ReplacingReg = I->getOperand(1).getReg();
  replaceRegister(ReplacedReg, ReplacingReg);

  return MRI->getVRegDef(ReplacingReg);
}


MachineInstr *TPCSetIndxCoalescer::skipRegisterLoad(MachineInstr *I) {
  assert(isSET_INDX(I->getOpcode()));

  switch (I->getOpcode()) {
  case TPC::SET_INDX_ld_ip:
  case TPC::SET_INDX_spu_ip:
  case TPC::SET_INDX_st_ip:
    // The instruction already uses immediate.
    return nullptr;
  default:
    break;
  }

  // If the element is not defined by an instruction "move immediate to register",
  // we cannot optimize it.
  MachineOperand &ElementOp = I->getOperand(NElement);
  unsigned ElementRegNo = ElementOp.getReg();
  MachineInstr *ElementDef = MRI->getVRegDef(ElementRegNo);
  assert(ElementDef);
  if (!isUnpredicatedMOV(ElementDef))
    return nullptr;

  // Do not replace register use with immediate operand if we cannot remove
  // MOV instruction.
  unsigned NumUses = isRegUsedOnlyInSetIndx(MRI, ElementRegNo);
  if (!NumUses)
    return nullptr;

  MachineOperand &Int5Op = I->getOperand(NInt5);
  int64_t OpVal = ElementDef->getOperand(1).getImm();

  // Create new SET_INDX instruction, which uses immediate instead of register.
  unsigned NewOpCode;
  switch (I->getOpcode()) {
    case TPC::SET_INDX_ld_rp:
      NewOpCode = TPC::SET_INDX_ld_ip;
      break;
    case TPC::SET_INDX_spu_rp:
      NewOpCode = TPC::SET_INDX_spu_ip;
      break;
    case TPC::SET_INDX_st_rp:
      NewOpCode = TPC::SET_INDX_st_ip;
      break;
    default:
      llvm_unreachable("Unexpected code");
  }
  unsigned NewValueReg = MRI->createVirtualRegister(&TPC::IRFRegClass);
  MachineInstr *NewSetIndx = BuildMI(*I->getParent(), I, I->getDebugLoc(),
                                     TII->get(NewOpCode), NewValueReg)
                               .addReg(Int5Op.getReg(), getRegState(Int5Op))
                               .addImm(OpVal)
                               .addImm(I->getOperand(NMask).getImm())
                               .addImm(0)
                               .addReg(TPC::SP0)
                               .addImm(0);
      
  assert(NewSetIndx->getNumOperands() == I->getNumOperands());

  // Replace all users of original SET_INDX with just created SET_INDX instruction.
  unsigned OldResult = I->getOperand(0).getReg();
  SmallVector<MachineOperand *, 8> OperandToUpdate;
  for (MachineOperand &Use : MRI->use_operands(OldResult)) {
    assert(Use.isReg());
    assert(Use.getReg() == OldResult);
    assert(!Use.isDef());
    OperandToUpdate.push_back(&Use);
  }
  for (MachineOperand *Op : OperandToUpdate)
    Op->setReg(NewValueReg);

  // Insert instruction into proper sets. Note, we put MOVi32si into the set of
  // removed instructions although it still may be used. We know however that
  // this instruction is used only in SET_INDX and all of them must be processed.
  RemovedInstrs.insert(I);
  RemovedInstrs.insert(ElementDef);
  AddedInstrs.insert(NewSetIndx);

  return NewSetIndx;
}


bool TPCSetIndxCoalescer::processSetIndx(MachineInstr *Instr) {
  // If this instruction sets undefined value to a dimension, skip it.
  if (skipUndefinedOperand(Instr))
    return false;

  // Keep initial instruction. If it will be replaced by the variant with
  // immediate, we still need original instruction for checks if the
  // written value is the same.
  MachineInstr *OldInstr = Instr;
  MachineOperand &OldElement = OldInstr->getOperand(NElement);
  if (MachineInstr * R = skipRegisterLoad(Instr))
    Instr = R;

  MachineOperand &Element = Instr->getOperand(NElement);
  MachineOperand &Predicate = Instr->getOperand(NMovPredicate);
  MachineOperand &Polarity = Instr->getOperand(NMovPolarity);
  MachineInstr *MI = Instr;
  MachineInstr *CoalescedDef = MI;
  MachineInstr *ResultInstr = MI;
  unsigned NumCoalesced = 0;
  unsigned Mask = Instr->getOperand(NMask).getImm();

  // Iterate the chain of SET_INDX.
  while(true) {
    // Get register defined by the current instruction.
    assert(MI->getDesc().getNumDefs() == 1);
    MachineOperand &DefOperand = MI->getOperand(0);
    assert(DefOperand.isReg());
    assert(DefOperand.isDef());
    int DefReg = DefOperand.getReg();

    // Find the next instruction to coalesce. If the next suitable instruction
    // has more than one use, we must finish coalescing.
    MachineInstr *Next = nullptr;
    for (MachineInstr &User : MRI->use_instructions(DefReg))
      if (&User != MI) {
        if (RemovedInstrs.count(&User))
          continue;
        if (Next) {
          Next = nullptr;
          break;
        }
        Next = &User;
      }

    if (!Next)
      break;

    if (MachineInstr *N = skipUndefinedOperand(Next)) {
      MI = N;
      continue;
    }

    if (!isSET_INDX(Next->getOpcode()))
      break;

    // If this instruction has different element, it cannot be coalesced with
    // the previous instructions.
    bool ElementIsSame = false;
    if (MachineInstr *R = skipRegisterLoad(Next))
      Next = R;
    MachineOperand &CurrElement = Next->getOperand(NElement);
    if (CurrElement.isImm() && Element.isImm())
      ElementIsSame = (CurrElement.getImm() == Element.getImm());
    else if (CurrElement.isReg() && Element.isReg())
      ElementIsSame = (CurrElement.getReg() == Element.getReg());
    else if (CurrElement.isReg() && OldElement.isReg())
      ElementIsSame = (CurrElement.getReg() == OldElement.getReg());
    MachineOperand &CurrPredicate = Next->getOperand(NMovPredicate);
    MachineOperand &CurrPolarity = Next->getOperand(NMovPolarity);
    assert(Predicate.isReg() && CurrPredicate.isReg());
    bool predicate_same = Predicate.getReg() == CurrPredicate.getReg();
    assert(Polarity.isImm() && CurrPolarity.isImm());
    bool polarity_same = Polarity.getImm() == CurrPolarity.getImm();
    if (ElementIsSame && predicate_same && polarity_same) {
      // The current instruction may be coalesced with 'CoalescedDef'.
      Mask |= Next->getOperand(NMask).getImm();
      CoalescedDef->getOperand(NMask).setImm(Mask);
      RemovedInstrs.insert(Next);
      ++NumCoalesced;
    } else {
      // Even if the current instruction cannot be coalesced with 'CoalescedDef',
      // maybe next instruction(s) can:
      //
      //    SET_INDX  %I2, %S1, 0x3
      //    SET_INDX  %I2, %S0, 0x4   <- Argument is different from the previous
      //    SET_INDX  %I2, %S1, 0x8   <- Can be coalesced with the first insn
      //    SET_INDX  %I2, %S0, 0x10  <- Can be coalesced with the second insn
      //
      // So continue looking forward if there is instruction to coalesce. If
      // such is found, coalesce it with 'CoalescedDef' and properly fix affected
      // users.
      MI = Next;
      ResultInstr = Next;
      continue;
    }

    // All users of the next instruction must use register defined by
    // 'ResultInstr'.
    unsigned ReplacedReg = Next->getOperand(0).getReg();
    unsigned ReplacingReg = ResultInstr->getOperand(0).getReg();
    replaceRegister(ReplacedReg, ReplacingReg);

    MI = ResultInstr;
  }

  NumReplaced += NumCoalesced;
  return NumCoalesced != 0;
}


bool TPCSetIndxCoalescer::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnableSetIndxCoalescer)
    return false;

  MF = &Func;
  MRI = &MF->getRegInfo();
  TII = MF->getSubtarget().getInstrInfo();
  NumReplaced = 0;
  RemovedInstrs.clear();
  AddedInstrs.clear();
  bool Changed = false;

  for (auto &BB : Func) {
    for (auto &I : BB) {
      if (RemovedInstrs.count(&I) == 0)
        if (isSET_INDX(I.getOpcode())) {
          // If single user of the result is also SET_INDX with the same second argument,
          // we may coalesce these instructions.
          if (processSetIndx(&I))
            Changed = true;
        }
    }
  }

  for (MachineInstr *MI : RemovedInstrs)
    MI->eraseFromParent();

  return Changed;
}
