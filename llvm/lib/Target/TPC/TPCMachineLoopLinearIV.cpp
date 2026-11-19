#include "TPCMachineLoopLinearIV.h"

#define GET_REGINFO_ENUM
#include "TPCGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "TPCGenInstrInfo.inc"

#include "TPCMachineInstrTools.h"
#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <optional>

namespace llvm {

MLoopLinearIV::MLoopLinearIV(const MLoopCustomLinearIVInfo Info)
    : ContentFlag(0) {
  Content.CustomIVInfo = Info;
}

MLoopLinearIV::MLoopLinearIV(const HWLoopCounterInfo Info) : ContentFlag(1) {
  Content.CounterInfo = Info;
}

bool MLoopLinearIV::isCustom() const { return ContentFlag == 0; }
bool MLoopLinearIV::isHWCounter() const { return ContentFlag == 1; }

MachineInstr &MLoopLinearIV::getCurrDefMI() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  return ContentFlag == 0 ? *Content.CustomIVInfo.Phi
                          : *Content.CounterInfo.LoopInstr;
}

Register MLoopLinearIV::getCurrDefReg() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  return ContentFlag == 0 ? Content.CustomIVInfo.Phi->getOperand(0).getReg()
                          : Register(Content.CounterInfo.CounterReg);
}

MachineInstr &MLoopLinearIV::getNextDefMI() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  return ContentFlag == 0 ? *Content.CustomIVInfo.DefMI
                          : *Content.CounterInfo.LoopInstr;
}

std::optional<Register> MLoopLinearIV::getNextDefReg() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  if (ContentFlag == 1)
    return std::nullopt; // There is no definition of next iteration value in HWLoop.
  return Content.CustomIVInfo.DefMI->getOperand(0).getReg();
}

unsigned MLoopLinearIV::getStepOpIx() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  return ContentFlag == 0 ? Content.CustomIVInfo.StepOpIx : 2;
}

std::optional<unsigned> MLoopLinearIV::getCurrValOpIxInNextDefMI() const {
  assert(ContentFlag == 0 || ContentFlag == 1);
  if (ContentFlag == 1)
    return std::nullopt;  // There is no definition of next iteration value in HWLoop.

  const Register CurrValReg = getCurrDefReg();
  const MachineInstr &NextDefMI = getNextDefMI();
  for (const auto &I : enumerate(NextDefMI.operands())) {
    if (I.value().isReg() && I.value().getReg() == CurrValReg)
      return I.index();
  }
  llvm_unreachable("Custom IV must use current def reg");
}

MachineOperand &MLoopLinearIV::getStepOp() const {
  return getNextDefMI().getOperand(getStepOpIx());
}

MachineOperand *MLoopLinearIV::getStartOp() const {
  assert(ContentFlag == 0 || ContentFlag == 1);

  if (ContentFlag == 1)
    return &Content.CounterInfo.LoopInstr->getOperand(0);

  MachineInstr &Phi = *Content.CustomIVInfo.Phi;

  // 0 - def, 1 - value, 2 - mbb, 3 - value, 4 - mbb
  if (Phi.getNumOperands() == 5) {
    MachineOperand &Op1 = Phi.getOperand(1);
    MachineOperand &Op3 = Phi.getOperand(3);
    return Op1.isReg() && Op1.getReg() == getNextDefReg() ? &Op3 : &Op1;
  }

  // > 1 start value in Phi. MLoopLinearIV support for such loops is not fully
  // implemented yet because it is not required.
  return nullptr;
}

/// Whether an add adds its operands in every lane each time it runs: it has
/// no predicate, and no saturation or carry switch. A predicated add
/// ('count += 1' where a compare holds) is not a linear induction variable;
/// the unroller took it for one and gave wrong counts.
static bool addsUnconditionally(const MachineInstr &MI) {
  const unsigned N = MI.getNumExplicitOperands();
  if (N < 4 || !MI.getOperand(0).isReg() || !MI.getOperand(0).isTied())
    return false;
  const MachineOperand &Pred = MI.getOperand(N - 2);
  const MachineOperand &Pol = MI.getOperand(N - 1);
  if (!Pred.isReg() || !Pol.isImm() || Pol.getImm() != 0 ||
      (Pred.getReg() != TPC::SPRF_TRUE && Pred.getReg() != TPC::VPRF_TRUE))
    return false;
  // The switches come right before the income, which is tied to the result.
  const unsigned Income = MI.findTiedOperandIdx(0);
  if (Income < 2)
    return false;
  const MachineOperand &Sw = MI.getOperand(Income - 1);
  return Sw.isImm() && (Sw.getImm() & (TPCII::SW_SAT | TPCII::SW_CARRY)) == 0;
}

SmallVector<MLoopLinearIV, 4> findLinearIVs(const MachineLoop &ML,
                                            const MachineRegisterInfo &MRI) {
  SmallVector<MLoopLinearIV, 4> Result;

  // Add HW counter as induction variable.
  if (std::optional<HWLoopCounterInfo> CounterInfo = getLoopCounterInfo(ML))
    Result.emplace_back(CounterInfo.value());

  // Analysis for custom IVs is not implemented for complex loops.
  MachineBasicBlock *const Latch = ML.getLoopLatch();
  if (!Latch)
    return Result;

  for (MachineInstr &Phi : ML.getHeader()->phis()) {
    // Candidate for the current iteration definition.
    const Register PhiDefReg = Phi.getOperand(0).getReg();

    const MachineOperand &LatchValueOp = *getPhiValueForMBB(Phi, *Latch);

    if (!LatchValueOp.isReg())
      continue;

    // Candidate for the next iteration definition.
    MachineInstr *const DefMI = MRI.getVRegDef(LatchValueOp.getReg());

    if (!ML.contains(DefMI))
      continue;

    // Only add instruction for IVs are supported for now.
    if (!TPCII::isAdd(DefMI->getDesc()) || !addsUnconditionally(*DefMI))
      continue;

    // Skip IV if it is not simple (i += step) but complex (i += f(step)).
    // Locate step as another operand.
    std::optional<unsigned> StepOpIx;
    {
      const MachineOperand &Op1 = DefMI->getOperand(1);
      const MachineOperand &Op2 = DefMI->getOperand(2);
      if (Op1.isReg() && Op1.getReg() == PhiDefReg)
        StepOpIx = 2;
      else if (Op2.isReg() && Op2.getReg() == PhiDefReg)
        StepOpIx = 1;
    }
    if (!StepOpIx.has_value())
      continue;

    // In order to guarantee that induction variable is linear, step operand
    // must be a loop invariant. Perform a bit simpler check: step operand is
    // either immediate either defined outside loop.
    {
      const MachineOperand &Op = DefMI->getOperand(StepOpIx.value());
      if (Op.isReg() && (Op.getReg().isPhysical() ||
                         ML.contains(MRI.getVRegDef(Op.getReg()))))
        continue;
      if (!Op.isImm() && !Op.isFPImm() && !Op.isCImm())
        continue;
    }

    // Finally, linear IV is found:
    MLoopCustomLinearIVInfo Info;
    Info.Phi = &Phi;
    Info.DefMI = DefMI;
    Info.StepOpIx = StepOpIx.value();
    Result.emplace_back(Info);
  }
  return Result;
}

raw_ostream &operator<<(raw_ostream &OS, const MLoopCustomLinearIVInfo &Info) {
  return OS << "MLoopCustomLinearIVInfo:\n" //
            << "  Phi:  " << *Info.Phi      //
            << "  Def:  " << *Info.DefMI    //
            << "  Step: " << Info.DefMI->getOperand(Info.StepOpIx) << "\n";
}

raw_ostream &operator<<(raw_ostream &OS, const HWLoopCounterInfo &Info) {
  return OS << "HWLoopCounterInfo:\n"           //
            << "  Loop:    " << *Info.LoopInstr //
            << "  Counter: " << printReg(Info.CounterReg) << "\n";
}

raw_ostream &operator<<(raw_ostream &OS, const MLoopLinearIV &Info) {
  assert(Info.ContentFlag == 0 || Info.ContentFlag == 1);
  OS << "MLoopLinearIV::";
  return Info.ContentFlag == 0 ? OS << Info.Content.CustomIVInfo
                               : OS << Info.Content.CounterInfo;
}

} // namespace llvm
