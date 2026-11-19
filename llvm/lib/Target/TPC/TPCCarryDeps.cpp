//===- TPCCarryDeps.cpp - Model the carry registers on ADD and SUB --------===//
//
// An integer ADD or SUB writes a carry (S_CARRY on the SPU, V_CARRY on the
// VPU), and one with the CARRY switch (ADD) or the BORROW switch (SUB) reads
// it. This is how 64-bit integers are added: a low add, then a high add with
// CARRY, which must read the carry of that low add and of no other.
//
// The instruction descriptions do not list these registers, so nothing told
// the schedulers that the two adds must stay in order with no other integer
// add between them, and the post-RA scheduler moved them apart (a wrong high
// word in the simulator, on gaudi2).
//
// This pass adds the carry register as an implicit def to every integer ADD
// and SUB, and as an implicit use to those that read it, then marks the defs
// that nothing reads as dead, so that only the adds around a carry read are
// kept in order. Only functions that read a carry are changed. Instructions
// made by later passes do not have these operands, so the pass runs again
// before each scheduler and before the packetizer.
//
// Measured on the simulator (gaudi, gaudi2): i32 and i16 adds write the
// carry; i8 and floating-point adds do not; NO_CARRY_GEN stops the write on
// gaudi2 but not on gaudi; an i32 SUB writes it on gaudi2. To stay safe on
// targets not measured, every integer ADD and SUB counts as a write.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-carry-deps"

STATISTIC(NumCarryReads, "Instructions that read a carry register");

namespace llvm {
FunctionPass *createTPCCarryDeps();
void initializeTPCCarryDepsPass(PassRegistry &);
} // namespace llvm

MCRegister llvm::getCarryRegister(const MachineInstr &MI) {
  MCRegister Reg;
  switch (MI.getOpcode()) {
  case TPC::ADDssp:
  case TPC::ADDsip:
  case TPC::SUBssp:
  case TPC::SUBsip:
    Reg = TPC::S_CARRY;
    break;
  case TPC::ADDvvp:
  case TPC::ADDvsp:
  case TPC::ADDvip:
  case TPC::ADDvvm:
  case TPC::ADDvsm:
  case TPC::ADDvim:
  case TPC::ADDavp:
  case TPC::ADDasp:
  case TPC::ADDaip:
  case TPC::ADDavm:
  case TPC::ADDasm:
  case TPC::ADDaim:
  case TPC::SUBvvp:
  case TPC::SUBvsp:
  case TPC::SUBvip:
  case TPC::SUBvvm:
  case TPC::SUBvsm:
  case TPC::SUBvim:
  case TPC::SUBavp:
  case TPC::SUBasp:
  case TPC::SUBaip:
  case TPC::SUBavm:
  case TPC::SUBasm:
  case TPC::SUBaim:
    Reg = TPC::V_CARRY;
    break;
  default:
    return MCRegister();
  }
  switch (getOpType(MI)) {
  case TPCII::OpType::FP32:
  case TPCII::OpType::BF16:
  case TPCII::OpType::FP16:
  case TPCII::OpType::FP8_152:
  case TPCII::OpType::FP8_143:
    return MCRegister();
  default:
    return Reg;
  }
}

bool llvm::readsCarry(const MachineInstr &MI) {
  if (!getCarryRegister(MI))
    return false;
  const MCInstrDesc &Desc = MI.getDesc();
  for (unsigned I = 0, E = Desc.getNumOperands(); I < E; ++I) {
    if (Desc.operands()[I].OperandType != TPC::OperandType::OPERAND_SWITCH)
      continue;
    const MachineOperand &Sw = MI.getOperand(I);
    // A switch that is not a constant cannot be encoded; take it as a read.
    if (!Sw.isImm())
      return true;
    const unsigned Read =
        TPCII::isAdd(Desc) ? TPCII::SW_CARRY : TPCII::SW_BORROW;
    return (Sw.getImm() & Read) != 0;
  }
  return false;
}

namespace {

class TPCCarryDeps : public MachineFunctionPass {
public:
  static char ID;
  TPCCarryDeps() : MachineFunctionPass(ID) {
    initializeTPCCarryDepsPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override {
    return "TPC carry register dependencies";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

/// The operand of MI that is Reg, implicit, and a def or a use as asked.
MachineOperand *findImplicit(MachineInstr &MI, MCRegister Reg, bool Def) {
  for (MachineOperand &MO : MI.implicit_operands())
    if (MO.isReg() && MO.getReg() == Reg && MO.isDef() == Def)
      return &MO;
  return nullptr;
}

/// Bit 0 for S_CARRY, bit 1 for V_CARRY.
unsigned carryBit(MCRegister Reg) { return Reg == TPC::S_CARRY ? 1 : 2; }

bool isCarryOperand(const MachineOperand &MO) {
  return MO.isReg() &&
         (MO.getReg() == TPC::S_CARRY || MO.getReg() == TPC::V_CARRY);
}

} // namespace

char TPCCarryDeps::ID = 0;

INITIALIZE_PASS(TPCCarryDeps, DEBUG_TYPE, "TPC carry register dependencies",
                false, false)

FunctionPass *llvm::createTPCCarryDeps() { return new TPCCarryDeps(); }

bool TPCCarryDeps::runOnMachineFunction(MachineFunction &MF) {
  // Which carry registers the function reads. One that is never read needs
  // no ordering.
  unsigned ReadBits = 0;
  for (const MachineBasicBlock &MBB : MF)
    for (const MachineInstr &MI : MBB.instrs())
      if (readsCarry(MI))
        ReadBits |= carryBit(getCarryRegister(MI));
  if (!ReadBits)
    return false;

  // Add the operands that are missing.
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB.instrs()) {
      const MCRegister Reg = getCarryRegister(MI);
      if (!Reg || !(ReadBits & carryBit(Reg)))
        continue;
      if (readsCarry(MI) && !findImplicit(MI, Reg, /*Def=*/false)) {
        MI.addOperand(MF, MachineOperand::CreateReg(Reg, /*isDef=*/false,
                                                    /*isImp=*/true));
        ++NumCarryReads;
        Changed = true;
      }
      if (!findImplicit(MI, Reg, /*Def=*/true)) {
        MI.addOperand(MF, MachineOperand::CreateReg(Reg, /*isDef=*/true,
                                                    /*isImp=*/true));
        Changed = true;
      }
    }
  }

  // Liveness of the two carry registers, block by block, then the dead flags.
  // A def that something may read must not be dead: the schedulers keep two
  // defs in order unless both are dead.
  // Walking an instruction backwards: its defs end the live range above it,
  // then its uses start one (an add with CARRY both reads and writes).
  DenseMap<const MachineBasicBlock *, unsigned> LiveIn, LiveOut;
  auto BlockLiveIn = [](const MachineBasicBlock &MBB, unsigned Live) {
    for (const MachineInstr &MI : llvm::reverse(MBB.instrs())) {
      for (const MachineOperand &MO : MI.implicit_operands())
        if (isCarryOperand(MO) && MO.isDef())
          Live &= ~carryBit(MO.getReg());
      for (const MachineOperand &MO : MI.implicit_operands())
        if (isCarryOperand(MO) && MO.isUse())
          Live |= carryBit(MO.getReg());
    }
    return Live;
  };
  for (bool Again = true; Again;) {
    Again = false;
    for (const MachineBasicBlock *MBB : post_order(&MF)) {
      unsigned Out = 0;
      for (const MachineBasicBlock *Succ : MBB->successors())
        Out |= LiveIn.lookup(Succ);
      LiveOut[MBB] = Out;
      const unsigned In = BlockLiveIn(*MBB, Out);
      if (In != LiveIn.lookup(MBB)) {
        LiveIn[MBB] = In;
        Again = true;
      }
    }
  }

  for (MachineBasicBlock &MBB : MF) {
    unsigned Live = LiveOut.lookup(&MBB);
    for (MachineInstr &MI : llvm::reverse(MBB.instrs())) {
      for (MachineOperand &MO : MI.implicit_operands())
        if (isCarryOperand(MO) && MO.isDef()) {
          const bool Dead = !(Live & carryBit(MO.getReg()));
          if (MO.isDead() != Dead) {
            MO.setIsDead(Dead);
            Changed = true;
          }
          Live &= ~carryBit(MO.getReg());
        }
      for (MachineOperand &MO : MI.implicit_operands())
        if (isCarryOperand(MO) && MO.isUse())
          Live |= carryBit(MO.getReg());
    }
  }
  return Changed;
}
