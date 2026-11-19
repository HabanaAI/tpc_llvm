//===- TPCMulAddCombine.cpp - Fewer vector ops for products ---------------===//
//
// Two rewrites of vector products, both giving the same values:
//
//  * A MAC whose accumulator starts at +0 (v_bf16_mac_acc32_b(a, b, 0), the
//    usual way to get a product in f32) becomes a MADD with a +0 addend.
//    MAC accumulates into its destination, so the zero had to be written
//    again before each MAC, two MOVs for a register pair; a MADD reads the
//    addend from another register, so one zero, made in the preheader of the
//    innermost loop around the MADD, serves every iteration (TPC's LICM does
//    not hoist vector immediates; the register allocator may still
//    rematerialize it). Both compute a*b + 0 in one rounding.
//    Only MACs without a predicate and without switches other than acc_fp32
//    are rewritten (MAC and MADD do not read their NEG switch the same way).
//
//  * On Gen4+, two f32 MULs of the halves of a register pair whose results
//    form a pair again ('p.v1 *= r; p.v2 *= r;') become one MUL x2, which
//    multiplies both halves in one instruction, lane by lane as the two MULs
//    did.
//
// The pass runs in SSA form before machine LICM and CSE.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-mul-add-combine"

STATISTIC(NumMacToMadd, "MACs from zero rewritten as MADDs");
STATISTIC(NumMulX2, "Pairs of MULs rewritten as MUL x2");

static cl::opt<bool> EnableMacToMadd(
    "tpc-mac-zero-to-madd", cl::Hidden, cl::init(true),
    cl::desc("Rewrite a MAC whose accumulator starts at zero as a MADD"));

static cl::opt<bool> EnableMulX2(
    "tpc-mul-x2-combine", cl::Hidden, cl::init(true),
    cl::desc("Rewrite two f32 MULs of the halves of a pair as one MUL x2"));

namespace llvm {
FunctionPass *createTPCMulAddCombine();
void initializeTPCMulAddCombinePass(PassRegistry &);
} // namespace llvm

namespace {

class TPCMulAddCombine : public MachineFunctionPass {
public:
  static char ID;
  TPCMulAddCombine() : MachineFunctionPass(ID) {
    initializeTPCMulAddCombinePass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return "TPC MAC and MUL combines"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<MachineLoopInfoWrapperPass>();
    AU.addPreserved<MachineLoopInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineRegisterInfo *MRI = nullptr;
  const TPCInstrInfo *TII = nullptr;
  MachineLoopInfo *MLI = nullptr;
  /// The zero made for each preheader and register class.
  DenseMap<std::pair<MachineBasicBlock *, const TargetRegisterClass *>,
           Register>
      Zeros;

  Register zeroFor(MachineInstr &MI, Register Zero);

  Register lookThroughCopies(Register R) const;
  bool isUnpredicated(const MachineInstr &MI) const;
  bool isPositiveZero(Register R, unsigned Depth = 0) const;
  bool macToMadd(MachineInstr &MI);
  bool mulPairToX2(MachineInstr &RegSeq);
};

} // namespace

char TPCMulAddCombine::ID = 0;

INITIALIZE_PASS_BEGIN(TPCMulAddCombine, DEBUG_TYPE, "TPC MAC and MUL combines",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfoWrapperPass)
INITIALIZE_PASS_END(TPCMulAddCombine, DEBUG_TYPE, "TPC MAC and MUL combines",
                    false, false)

FunctionPass *llvm::createTPCMulAddCombine() { return new TPCMulAddCombine(); }

Register TPCMulAddCombine::lookThroughCopies(Register R) const {
  while (R.isVirtual()) {
    const MachineInstr *Def = MRI->getUniqueVRegDef(R);
    if (!Def || !Def->isFullCopy())
      break;
    R = Def->getOperand(1).getReg();
  }
  return R;
}

// The last two explicit operands of a predicated TPC instruction are the
// predicate and its polarity. Before TPCPredicateOptimizer the predicate of an
// intrinsic is often a COPY of SPRF_TRUE.
bool TPCMulAddCombine::isUnpredicated(const MachineInstr &MI) const {
  const unsigned N = MI.getNumExplicitOperands();
  if (N < 2)
    return false;
  const MachineOperand &Pred = MI.getOperand(N - 2);
  const MachineOperand &Pol = MI.getOperand(N - 1);
  return Pred.isReg() && Pol.isImm() && Pol.getImm() == 0 &&
         lookThroughCopies(Pred.getReg()) == TPC::SPRF_TRUE;
}

/// Whether every lane of R is +0.0 (all bits clear).
bool TPCMulAddCombine::isPositiveZero(Register R, unsigned Depth) const {
  R = lookThroughCopies(R);
  if (!R.isVirtual() || Depth > 2)
    return false;
  const MachineInstr *Def = MRI->getUniqueVRegDef(R);
  if (!Def)
    return false;
  if (Def->isRegSequence()) {
    for (unsigned I = 1, E = Def->getNumOperands(); I < E; I += 2)
      if (!Def->getOperand(I).isReg() || Def->getOperand(I).getSubReg() ||
          !isPositiveZero(Def->getOperand(I).getReg(), Depth + 1))
        return false;
    return true;
  }
  // MOV_ld_vip dest, imm, type, switches, income, pred, polarity.
  return (Def->getOpcode() == TPC::MOV_ld_vip ||
          Def->getOpcode() == TPC::MOVvip) &&
         Def->getOperand(1).isImm() && Def->getOperand(1).getImm() == 0 &&
         Def->getOperand(3).isImm() && Def->getOperand(3).getImm() == 0 &&
         isUnpredicated(*Def);
}

static unsigned getMaddOpcode(unsigned MacOpc) {
  switch (MacOpc) {
  case TPC::MACf32vvp:   return TPC::MADDf32vvvp;
  case TPC::MACf32vsp:   return TPC::MADDf32vsvp;
  case TPC::MACf32vip:   return TPC::MADDf32vivp;
  case TPC::MACbf16vvp:  return TPC::MADDbf16vvvp;
  case TPC::MACbf16vsp:  return TPC::MADDbf16vsvp;
  case TPC::MACbf16vip:  return TPC::MADDbf16vivp;
  case TPC::MACf16vvp:   return TPC::MADDf16vvvp;
  case TPC::MACf16vsp:   return TPC::MADDf16vsvp;
  case TPC::MACf16vip:   return TPC::MADDf16vivp;
  case TPC::MACAbf16vvp: return TPC::MADDAbf16vvvp;
  case TPC::MACAbf16vsp: return TPC::MADDAbf16vsvp;
  case TPC::MACAbf16vip: return TPC::MADDAbf16vivp;
  case TPC::MACAf16vvp:  return TPC::MADDAf16vvvp;
  case TPC::MACAf16vsp:  return TPC::MADDAf16vsvp;
  case TPC::MACAf16vip:  return TPC::MADDAf16vivp;
  }
  return 0;
}

/// A zero of Zero's class for the MADD replacing MI: one made in the
/// preheader of the innermost loop around MI, or Zero itself outside loops.
Register TPCMulAddCombine::zeroFor(MachineInstr &MI, Register Zero) {
  MachineLoop *L = MLI->getLoopFor(MI.getParent());
  MachineBasicBlock *Pre = L ? L->getLoopPreheader() : nullptr;
  if (!Pre)
    return Zero;
  const TargetRegisterClass *RC = MRI->getRegClass(Zero);
  Register &R = Zeros[{Pre, RC}];
  if (R)
    return R;
  const DebugLoc DL;
  const MachineBasicBlock::iterator At = Pre->getFirstTerminator();
  auto MakeV = [&]() {
    const Register Undef = MRI->createVirtualRegister(&TPC::VRFRegClass);
    BuildMI(*Pre, At, DL, TII->get(TargetOpcode::IMPLICIT_DEF), Undef);
    const Register V = MRI->createVirtualRegister(&TPC::VRFRegClass);
    // MOV_ld_vip dest, imm, type, switches, income, pred, polarity.
    BuildMI(*Pre, At, DL, TII->get(TPC::MOV_ld_vip), V)
        .addImm(0)
        .addImm(TPCII::OpType::FP32)
        .addImm(0)
        .addReg(Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    return V;
  };
  if (TPC::VRFRegClass.hasSubClassEq(RC)) {
    R = MakeV();
  } else if (TPC::DRFRegClass.hasSubClassEq(RC)) {
    const Register V = MakeV();
    R = MRI->createVirtualRegister(RC);
    BuildMI(*Pre, At, DL, TII->get(TargetOpcode::REG_SEQUENCE), R)
        .addReg(V)
        .addImm(TPC::sub_0)
        .addReg(V)
        .addImm(TPC::sub_1);
  } else {
    R = Zero;
  }
  return R;
}

// MAC:  dest, op1, op2, type, sw, [acc,] income (the accumulator), pred, pol
// MADD: dest, op1, op2, op3 (the addend), type, sw, [acc,] income, pred, pol
bool TPCMulAddCombine::macToMadd(MachineInstr &MI) {
  const unsigned MaddOpc = getMaddOpcode(MI.getOpcode());
  if (!MaddOpc || !isUnpredicated(MI))
    return false;
  const unsigned Income = MI.findTiedOperandIdx(0);
  const MachineOperand &Sw = MI.getOperand(4);
  if (!Sw.isImm() || (Sw.getImm() & ~int64_t(TPCII::SW_ACC_FP32)) != 0)
    return false;
  if (!isPositiveZero(MI.getOperand(Income).getReg()))
    return false;
  const Register Acc = zeroFor(MI, MI.getOperand(Income).getReg());

  const Register Dest = MI.getOperand(0).getReg();
  const Register Undef = MRI->createVirtualRegister(MRI->getRegClass(Dest));
  BuildMI(*MI.getParent(), MI, MI.getDebugLoc(),
          TII->get(TargetOpcode::IMPLICIT_DEF), Undef);
  MachineInstrBuilder MIB =
      BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), TII->get(MaddOpc), Dest);
  MIB.add(MI.getOperand(1)).add(MI.getOperand(2)).addReg(Acc);
  for (unsigned I = 3; I < Income; ++I)
    MIB.add(MI.getOperand(I));
  MIB.addReg(Undef);
  for (unsigned I = Income + 1, E = MI.getNumExplicitOperands(); I < E; ++I)
    MIB.add(MI.getOperand(I));
  MRI->clearKillFlags(Acc);
  LLVM_DEBUG(dbgs() << "MAC from zero: " << MI << "        as " << *MIB);
  MI.eraseFromParent();
  ++NumMacToMadd;
  return true;
}

namespace {
/// One half of a MUL x2: a MUL.f32 of half Half of Pair by Other.
struct HalfMul {
  MachineInstr *MI = nullptr;
  Register Pair;
  unsigned Half = 0;
  const MachineOperand *Other = nullptr;
};
} // namespace

/// The MUL x2 opcode for the kinds of its two second operands.
static unsigned getMulX2Opcode(const MachineOperand &Lo,
                               const MachineOperand &Hi,
                               const MachineRegisterInfo &MRI) {
  auto Kind = [&](const MachineOperand &MO) {
    if (MO.isImm())
      return 'i';
    return TPC::SRFRegClass.hasSubClassEq(MRI.getRegClass(MO.getReg())) ? 's'
                                                                         : 'v';
  };
  const char K[2] = {Kind(Lo), Kind(Hi)};
  if (K[0] == 'v' && K[1] == 'v') return TPC::MULx2f32vvvp;
  if (K[0] == 's' && K[1] == 's') return TPC::MULx2f32vssp;
  // An instruction has one immediate field: two immediates must be equal.
  if (K[0] == 'i' && K[1] == 'i')
    return Lo.getImm() == Hi.getImm() ? TPC::MULx2f32viip : 0;
  if (K[0] == 'i' && K[1] == 'v') return TPC::MULx2f32vivp;
  if (K[0] == 's' && K[1] == 'v') return TPC::MULx2f32vsvp;
  if (K[0] == 'v' && K[1] == 'i') return TPC::MULx2f32vvip;
  if (K[0] == 'v' && K[1] == 's') return TPC::MULx2f32vvsp;
  return 0;
}

/// %r = REG_SEQUENCE (MUL.f32 %p.sub_0, %b), sub_0, (MUL.f32 %p.sub_1, %c),
/// sub_1  =>  %r = MUL.f32 x2 %p, %b, %c.
bool TPCMulAddCombine::mulPairToX2(MachineInstr &RegSeq) {
  if (RegSeq.getNumOperands() != 5)
    return false;
  const Register Dest = RegSeq.getOperand(0).getReg();
  if (!TPC::DRFRegClass.hasSubClassEq(MRI->getRegClass(Dest)))
    return false;

  // Which half of which pair a MUL operand reads.
  auto HalfOf = [&](const MachineOperand &MO, Register &Pair, unsigned &Idx) {
    if (!MO.isReg() || !MO.getReg().isVirtual())
      return false;
    Register R = MO.getReg();
    unsigned Sub = MO.getSubReg();
    if (!Sub) {
      const MachineInstr *Def = MRI->getUniqueVRegDef(R);
      if (!Def || !Def->isCopy() || !Def->getOperand(1).getReg().isVirtual())
        return false;
      R = Def->getOperand(1).getReg();
      Sub = Def->getOperand(1).getSubReg();
    }
    if ((Sub != TPC::sub_0 && Sub != TPC::sub_1) ||
        !TPC::DRFRegClass.hasSubClassEq(MRI->getRegClass(R)))
      return false;
    Pair = R;
    Idx = Sub == TPC::sub_0 ? 0 : 1;
    return true;
  };

  HalfMul Muls[2];
  for (unsigned I = 1; I < 5; I += 2) {
    const MachineOperand &In = RegSeq.getOperand(I);
    const unsigned Sub = RegSeq.getOperand(I + 1).getImm();
    if (!In.isReg() || In.getSubReg() || !In.getReg().isVirtual() ||
        (Sub != TPC::sub_0 && Sub != TPC::sub_1) ||
        !MRI->hasOneNonDBGUse(In.getReg()))
      return false;
    MachineInstr *Mul = MRI->getUniqueVRegDef(In.getReg());
    if (!Mul || Mul->getParent() != RegSeq.getParent() ||
        (Mul->getOpcode() != TPC::MULf32vvp &&
         Mul->getOpcode() != TPC::MULf32vsp &&
         Mul->getOpcode() != TPC::MULf32vip) ||
        !isUnpredicated(*Mul))
      return false;
    // MUL: dest, op1, op2, type, sw, income, pred, pol. Only a vector-vector
    // MUL can have its half in the second operand.
    HalfMul H;
    H.MI = Mul;
    if (HalfOf(Mul->getOperand(1), H.Pair, H.Half))
      H.Other = &Mul->getOperand(2);
    else if (Mul->getOpcode() == TPC::MULf32vvp &&
             HalfOf(Mul->getOperand(2), H.Pair, H.Half))
      H.Other = &Mul->getOperand(1);
    else
      return false;
    if (H.Half != (Sub == TPC::sub_0 ? 0u : 1u))
      return false;
    Muls[H.Half] = H;
  }
  if (!Muls[0].MI || !Muls[1].MI || Muls[0].Pair != Muls[1].Pair)
    return false;
  const MachineInstr &Lo = *Muls[0].MI, &Hi = *Muls[1].MI;
  if (!Lo.getOperand(3).isIdenticalTo(Hi.getOperand(3)) ||
      !Lo.getOperand(4).isIdenticalTo(Hi.getOperand(4)) ||
      !Lo.getOperand(4).isImm())
    return false;
  if (Muls[0].Other->isReg() && Muls[0].Other->getSubReg())
    return false;
  if (Muls[1].Other->isReg() && Muls[1].Other->getSubReg())
    return false;
  const unsigned Opc = getMulX2Opcode(*Muls[0].Other, *Muls[1].Other, *MRI);
  if (!Opc)
    return false;

  // MUL x2: dest, op1 (the pair), op2 (for the low half), op3 (for the high
  // half), type, sw, income, pred, pol.
  MachineBasicBlock &MBB = *RegSeq.getParent();
  const Register Undef = MRI->createVirtualRegister(MRI->getRegClass(Dest));
  BuildMI(MBB, RegSeq, RegSeq.getDebugLoc(),
          TII->get(TargetOpcode::IMPLICIT_DEF), Undef);
  MachineInstrBuilder MIB =
      BuildMI(MBB, RegSeq, RegSeq.getDebugLoc(), TII->get(Opc), Dest)
          .addReg(Muls[0].Pair)
          .add(*Muls[0].Other)
          .add(*Muls[1].Other)
          .add(Lo.getOperand(3))
          // The encoder sets the x2 bit from the opcode, the printer from the
          // switches; the intrinsic sets it in both.
          .addImm(Lo.getOperand(4).getImm() | TPCII::SW_X2_ARITHMETIC)
          .addReg(Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
  for (MachineOperand &MO : MIB->uses())
    if (MO.isReg())
      MO.setIsKill(false);
  MRI->clearKillFlags(Muls[0].Pair);
  for (const HalfMul &H : Muls)
    if (H.Other->isReg())
      MRI->clearKillFlags(H.Other->getReg());
  LLVM_DEBUG(dbgs() << "MUL pair: " << Lo << "          " << Hi
                    << "      as " << *MIB);
  RegSeq.eraseFromParent();
  Muls[0].MI->eraseFromParent();
  Muls[1].MI->eraseFromParent();
  ++NumMulX2;
  return true;
}

bool TPCMulAddCombine::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;
  const auto &ST = MF.getSubtarget<TPCSubtarget>();
  MRI = &MF.getRegInfo();
  TII = ST.getInstrInfo();
  MLI = &getAnalysis<MachineLoopInfoWrapperPass>().getLI();
  Zeros.clear();
  const bool DoMadd = EnableMacToMadd && ST.hasMADD();
  const bool DoX2 = EnableMulX2 && ST.hasGen4Plus();
  if (!DoMadd && !DoX2)
    return false;

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : make_early_inc_range(MBB)) {
      if (DoMadd && getMaddOpcode(MI.getOpcode()))
        Changed |= macToMadd(MI);
      else if (DoX2 && MI.isRegSequence())
        Changed |= mulPairToX2(MI);
    }
  }
  return Changed;
}
