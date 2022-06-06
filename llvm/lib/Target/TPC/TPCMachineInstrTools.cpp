#include "TPCMachineInstrTools.h"

#define GET_INSTRINFO_ENUM
#include "TPCGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TPCGenSubtargetInfo.inc"

#define GET_REGINFO_ENUM
#define GET_REGINFO_HEADER
#include "TPCGenRegisterInfo.inc"

#include "MCTargetDesc/InstructionDB.h"

#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

#include <cassert>

using namespace llvm;

static bool hasHardwareUDiv(const TargetSubtargetInfo &STI) {
  return STI.getFeatureBits()[TPC::FeatureGen4Plus];
}

static MachineOperand evalHWUDiv(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator InsertPos,
                                 const Register OpLHS, const std::int32_t OpRHS,
                                 MachineRegisterInfo &MRI,
                                 const TargetInstrInfo &TII) {
  assert(MBB.getParent());
  assert(hasHardwareUDiv(MBB.getParent()->getSubtarget()));

  // UDIV accepts SRF only even if division on imm value is required.
  const Register RHSReg = MRI.createVirtualRegister(&TPC::SRFRegClass);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::MOVsip), RHSReg)
      .addImm(OpRHS)
      .addImm(TPCII::OpType::UINT32)
      .addImm(0)
      .addReg(RHSReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  const Register ResultReg = MRI.createVirtualRegister(&TPC::SRFRegClass);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::UDIV), ResultReg)
      .addReg(OpLHS)
      .addReg(RHSReg)
      .addImm(TPCII::OpType::UINT32)
      .addImm(TPCII::SW_DIV_MODE_MOD)
      .addReg(ResultReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  return MachineOperand::CreateReg(ResultReg, false);
}

static Register insertCMPLess(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator InsertPos,
                              const Register OpLHS, const std::int32_t OpRHS,
                              MachineRegisterInfo &MRI,
                              const TargetInstrInfo &TII) {
  const Register Reg = MRI.createVirtualRegister(&TPC::SPRFRegClass);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::CMP_LESSsip), Reg)
      .addReg(OpLHS)
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(Reg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  return Reg;
}

namespace llvm {

unsigned getLoopMachineInstrOpcode(const bool IsStartImm, const bool IsBoundImm,
                                   const bool IsStepImm,
                                   const bool IsPredicated) {
  const unsigned Mask = (IsStartImm ? 1 : 0) | (IsBoundImm ? 2 : 0) |
                        (IsStepImm ? 4 : 0) | (IsPredicated ? 8 : 0);

  // clang-format off
  switch (Mask) {
  case 0:  return TPC::LOOPsss;
  case 1:  return TPC::LOOPiss;
  case 2:  return TPC::LOOPsis;
  case 3:  return TPC::LOOPiis;
  case 4:  return TPC::LOOPssi;
  case 5:  return TPC::LOOPisi;
  case 6:  return TPC::LOOPsii;
  case 7:  return TPC::LOOPiii;
  case 8:  return TPC::LOOPsssp;
  case 9:  return TPC::LOOPissp;
  case 10: return TPC::LOOPsisp;
  case 11: return TPC::LOOPiisp;
  case 12: return TPC::LOOPssip;
  case 13: return TPC::LOOPisip;
  case 14: return TPC::LOOPsiip;
  case 15: return TPC::LOOPiiip;
  }
  // clang-format on

  llvm_unreachable("Sanity check");
}

MachineOperand evalSubI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII) {
  assert(MBB.getParent());
  assert(!OpLHS.isImm() || isIntN(32, OpLHS.getImm()));
  assert(!OpRHS.isImm() || isIntN(32, OpRHS.getImm()));

  // Case: value is known at compile time.
  if (OpLHS.isImm() && OpRHS.isImm()) {
    const int64_t Result = OpLHS.getImm() - OpRHS.getImm();
    assert(isIntN(32, Result)); // operation must be applicable.
    return MachineOperand::CreateImm(Result);
  }

  if (OpRHS.isImm() && OpRHS.getImm() == 0)
    return OpLHS;

  // Case: instruction generation is required.
  const Register DstReg = MRI.createVirtualRegister(
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32));
  if (OpLHS.isReg() && OpRHS.isReg()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SUBssp), DstReg)
        .addReg(OpLHS.getReg())
        .addReg(OpRHS.getReg())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else if (OpLHS.isReg()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SUBsip), DstReg)
        .addReg(OpLHS.getReg())
        .addImm(OpRHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SUBsip), DstReg)
        .addReg(OpRHS.getReg())
        .addImm(OpLHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(TPCII::SW_NEG)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  }
  return MachineOperand::CreateReg(DstReg, false);
}

MachineOperand evalAddI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII) {
  assert(MBB.getParent());
  assert(!OpLHS.isImm() || isIntN(32, OpLHS.getImm()));
  assert(!OpRHS.isImm() || isIntN(32, OpRHS.getImm()));

  // Case: value is known at compile time.
  if (OpLHS.isImm() && OpRHS.isImm()) {
    const int64_t Result = OpLHS.getImm() + OpRHS.getImm();
    assert(isIntN(32, Result)); // operation must be applicable
    return MachineOperand::CreateImm(Result);
  }

  if (OpLHS.isImm() && OpLHS.getImm() == 0)
    return OpRHS;

  if (OpRHS.isImm() && OpRHS.getImm() == 0)
    return OpLHS;

  // Case: instruction generation is required.
  const Register DstReg = MRI.createVirtualRegister(
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32));
  if (OpLHS.isReg() && OpRHS.isReg()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ADDssp), DstReg)
        .addReg(OpLHS.getReg())
        .addReg(OpRHS.getReg())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else if (OpLHS.isReg()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ADDsip), DstReg)
        .addReg(OpLHS.getReg())
        .addImm(OpRHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ADDsip), DstReg)
        .addReg(OpRHS.getReg())
        .addImm(OpLHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(DstReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  }
  return MachineOperand::CreateReg(DstReg, false);
}

namespace {
struct MagicsForI32Div {
  std::int32_t M;
  std::int32_t P;
};
} // namespace

/// Return magic numbers for integer division on the given constant >= 2.
///
/// See "Hacker's delight" Ch. 10 for details.
static MagicsForI32Div getMagicsForI32Div(const std::int32_t Den) {
  assert(Den >= 2);

  const std::int64_t Nc = (((1LL << 31) - 1) / Den) * Den - 1;

  std::int64_t P = 32;
  while (true) {
    const std::int64_t PPower2 = 1LL << P;
    if (PPower2 > Nc * (Den - PPower2 % Den))
      break;

    ++P;
  }
  assert(P <= 62); // See "Hacker's delight" Proof.

  const std::int64_t PPower2 = 1LL << P;
  const std::int64_t MSmall = (PPower2 + Den - PPower2 % Den) / Den;
  assert(0 <= MSmall && MSmall <= (1LL << 32)); // See Proof.

  const std::int64_t M = MSmall < (1LL << 31) ? MSmall : MSmall - (1LL << 32);
  assert(isIntN(32, M));

  // Sanity check for special cases (see proof).
  assert(Den != 3 || M == ((1LL << 32) + 2) / 3);
  assert(Den != 5 || M == ((1LL << 33) + 3) / 5);
  assert(Den != 7 || M == ((1LL << 34) + 5) / 7 - (1LL << 32));

  MagicsForI32Div Result;
  Result.M = M;
  Result.P = P;
  return Result;
}

MachineOperand evalShrI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS, const std::int32_t OpRHS,
                          MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII) {
  assert(0 <= OpRHS && OpRHS < 32);

  if (OpRHS == 0)
    return OpLHS;

  const std::uint32_t Magic = 1 << (32 - OpRHS - 1);

  if (OpLHS.isImm()) {
    // Operator >> is implementation-defined for negative
    // integrals until C++20.
    // https://en.cppreference.com/w/cpp/language/operator_arithmetic
    return OpLHS.getImm() >= 0
               ? MachineOperand::CreateImm(OpLHS.getImm() >> OpRHS)
               : MachineOperand::CreateImm(((-OpLHS.getImm()) >> OpRHS) |
                                           Magic);
  }

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  const Register ResultReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SHRsip), ResultReg)
      .addReg(OpLHS.getReg())
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(ResultReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  return MachineOperand::CreateReg(ResultReg, false);
}

MachineOperand evalShlI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS, const std::int32_t OpRHS,
                          MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII) {
  assert(0 <= OpRHS && OpRHS < 32);

  if (OpRHS == 0)
    return OpLHS;

  if (OpLHS.isImm())
    return MachineOperand::CreateImm(OpLHS.getImm() << OpRHS);

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  const Register ResultReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SHLsip), ResultReg)
      .addReg(OpLHS.getReg())
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(ResultReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  return MachineOperand::CreateReg(ResultReg, false);
}

MachineOperand evalAndI32(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator InsertPos,
                          const MachineOperand &OpLHS,
                          const MachineOperand &OpRHS, MachineRegisterInfo &MRI,
                          const TargetInstrInfo &TII) {
  if (OpLHS.isImm() && OpRHS.isImm())
    return MachineOperand::CreateImm(OpLHS.getImm() & OpRHS.getImm());

  if (OpLHS.isImm() && OpLHS.getImm() == 0)
    return MachineOperand::CreateImm(0);

  if (OpRHS.isImm() && OpRHS.getImm() == 0)
    return MachineOperand::CreateImm(0);

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  const Register ResultReg = MRI.createVirtualRegister(TRCi32);
  if (OpLHS.isReg() && OpRHS.isReg()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ANDssp), ResultReg)
        .addReg(OpLHS.getReg())
        .addReg(OpRHS.getReg())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(ResultReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else if (OpRHS.isImm()) {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ANDsip), ResultReg)
        .addReg(OpLHS.getReg())
        .addImm(OpRHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(ResultReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  } else {
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ANDsip), ResultReg)
        .addReg(OpRHS.getReg())
        .addImm(OpLHS.getImm())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(ResultReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
  }
  return MachineOperand::CreateReg(ResultReg, false);
}

MachineOperand evalShrSI32(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator InsertPos,
                           const MachineOperand &OpLHS,
                           const std::int32_t OpRHS, MachineRegisterInfo &MRI,
                           const TargetInstrInfo &TII) {
  assert(0 <= OpRHS && OpRHS < 32);

  if (OpRHS == 0)
    return OpLHS;

  std::uint32_t Magic = 0;
  for (int Shift = 1; Shift <= OpRHS + 1; ++Shift)
    Magic += 1 << (32 - Shift);

  if (OpLHS.isImm()) {
    // Operator >> is implementation-defined for negative integrals until C++20.
    // https://en.cppreference.com/w/cpp/language/operator_arithmetic
    return OpLHS.getImm() >= 0
               ? MachineOperand::CreateImm(OpLHS.getImm() >> OpRHS)
               : MachineOperand::CreateImm(((-OpLHS.getImm()) >> OpRHS) |
                                           Magic);
  }

  // TPC has no instruction for signed shift right. Need to emulate.

  // CMP = OpLHS < 0. (Latency = 2)
  const Register CMPDefReg =
      insertCMPLess(MBB, InsertPos, OpLHS.getReg(), 0, MRI, TII);

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  // TMP = OpLHS >> OpRHS (unsigned shift)
  const Register TMPDstReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SHRsip), TMPDstReg)
      .addReg(OpLHS.getReg())
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(TMPDstReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  // Result = OpLHS < 0 ? TMP | Magic : TMP
  const Register ResultReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ORsip), ResultReg)
      .addReg(TMPDstReg)
      .addImm(Magic)
      .addImm(TPCII::OpType::UINT32)
      .addImm(0)
      .addReg(TMPDstReg)
      .addReg(CMPDefReg)
      .addImm(0);

  return MachineOperand::CreateReg(ResultReg, false);
}

MachineOperand evalRemPosI32(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator InsertPos,
                             const MachineOperand &OpLHS,
                             const std::int32_t OpRHS, MachineRegisterInfo &MRI,
                             const TargetInstrInfo &TII) {
  assert(MBB.getParent());
  assert(!OpLHS.isImm() || isIntN(32, OpLHS.getImm()));
  assert(OpRHS > 0);

  if (OpLHS.isImm())
    return MachineOperand::CreateImm(OpLHS.getImm() % OpRHS);

  if (OpRHS == 1)
    return MachineOperand::CreateImm(0);

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  if (isPowerOf2_32(OpRHS)) {
    // Use trick for the case when SHRS instruction is not available.
    // T1 = OpLHS >> 31 (unsigned shift)
    // T2 = T1 << K
    // T3 = T2 - T1 (2**k - 1 for n < 0 and 0 for n >= 0)
    // T4 = OpLHS + T3
    // T5 = T4 & (-2**k)
    // R = N - T5
    const std::uint32_t K = Log2_32(OpRHS);
    const MachineOperand T1 = evalShrI32(MBB, InsertPos, OpLHS, 31, MRI, TII);
    const MachineOperand T2 = evalShlI32(MBB, InsertPos, T1, K, MRI, TII);
    const MachineOperand T3 = evalSubI32(MBB, InsertPos, T2, T1, MRI, TII);
    const MachineOperand T4 = evalAddI32(MBB, InsertPos, OpLHS, T3, MRI, TII);
    const MachineOperand T5 = evalAndI32(
        MBB, InsertPos, T4, MachineOperand::CreateImm(-(1 << K)), MRI, TII);
    return evalSubI32(MBB, InsertPos, OpLHS, T5, MRI, TII);
  }

  if (hasHardwareUDiv(MBB.getParent()->getSubtarget())) {
    // T1 = OpLHS < 0
    // T2 = ABS(OpLHS)
    // T3 = UDIV(T2, OpRHS)
    // T4 = OpLHS < 0 ? -T3 : T3
    const Register T1 =
        insertCMPLess(MBB, InsertPos, OpLHS.getReg(), 0, MRI, TII);
    const Register T2 = MRI.createVirtualRegister(&TPC::SRFRegClass);
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ABSssp), T2)
        .addReg(OpLHS.getReg())
        .addImm(TPCII::OpType::INT32)
        .addImm(0)
        .addReg(T2, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    const Register T3 =
        evalHWUDiv(MBB, InsertPos, T2, OpRHS, MRI, TII).getReg();
    const Register T4 = MRI.createVirtualRegister(&TPC::SRFRegClass);
    BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::SUBsip), T4)
        .addReg(T3)
        .addImm(0)
        .addImm(TPCII::OpType::INT32)
        .addImm(TPCII::SW_NEG)
        .addReg(T3)
        .addReg(T1)
        .addImm(0);
    return MachineOperand::CreateReg(T4, false);
  }

  const MagicsForI32Div Magics = getMagicsForI32Div(OpRHS);

  // CMP = OpLHS < 0. (Latency = 2)
  const Register CMPDefReg =
      insertCMPLess(MBB, InsertPos, OpLHS.getReg(), 0, MRI, TII);

  // T1 = OpLHS * M / 2**32.
  const Register T1DstReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::MULsip), T1DstReg)
      .addReg(OpLHS.getReg())
      .addImm(Magics.M)
      .addImm(TPCII::OpType::INT32)
      .addImm(TPCII::SW_UPPER32)
      .addReg(T1DstReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  // T2 = OpLHS * M / 2**P = T1 >>(signed) (2**(P-32)).
  assert(32 <= Magics.P && Magics.P <= 62); // According to construction.
  const Register T2DstReg =
      evalShrSI32(MBB, InsertPos, MachineOperand::CreateReg(T1DstReg, false),
                  Magics.P - 32, MRI, TII)
          .getReg();

  // Quotient = OpLHS < 0 ? T2 + 1 : T2
  const Register QuotientDefReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::ADDsip), QuotientDefReg)
      .addReg(T2DstReg)
      .addImm(1)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(T2DstReg)
      .addReg(CMPDefReg)
      .addImm(0);

  // T3 = Quotient * OpRHS
  const Register T3DefReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::MULsip), T3DefReg)
      .addReg(QuotientDefReg)
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(T3DefReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  // Rem = OpLHS - T3
  return evalSubI32(MBB, InsertPos, OpLHS,
                    MachineOperand::CreateReg(T3DefReg, false), MRI, TII);
}

MachineOperand evalRemPosToPosI32(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator InsertPos,
                                  const MachineOperand &OpLHS,
                                  const std::int32_t OpRHS,
                                  MachineRegisterInfo &MRI,
                                  const TargetInstrInfo &TII) {
  assert(MBB.getParent());
  assert(!OpLHS.isImm() || isIntN(32, OpLHS.getImm()));
  assert(OpRHS > 0);

  if (OpLHS.isImm())
    return MachineOperand::CreateImm(OpLHS.getImm() % OpRHS);

  if (OpRHS == 1)
    return MachineOperand::CreateImm(0);

  // The fastest way to calculate remainder in one instruction.
  if (isPowerOf2_32(OpRHS)) {
    // R = N & (2**k - 1).
    return evalAndI32(MBB, InsertPos, OpLHS,
                      MachineOperand::CreateImm(OpRHS - 1), MRI, TII);
  }

  // Gaudi2+ supports UDIV instruction.
  if (hasHardwareUDiv(MBB.getParent()->getSubtarget()))
    return evalHWUDiv(MBB, InsertPos, OpLHS.getReg(), OpRHS, MRI, TII);

  const TargetRegisterClass *const TRCi32 =
      MBB.getParent()->getSubtarget().getTargetLowering()->getRegClassFor(
          MVT::i32);

  const MagicsForI32Div Magics = getMagicsForI32Div(OpRHS);

  // T1 = OpLHS * M / 2**32.
  const Register T1DstReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::MULsip), T1DstReg)
      .addReg(OpLHS.getReg())
      .addImm(Magics.M)
      .addImm(TPCII::OpType::INT32)
      .addImm(TPCII::SW_UPPER32)
      .addReg(T1DstReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  // Quotient = OpLHS * M / 2**P = T1 >>(signed) (2**(P-32)).
  assert(32 <= Magics.P && Magics.P <= 62); // According to construction.
  const Register QuotientDefReg =
      evalShrSI32(MBB, InsertPos, MachineOperand::CreateReg(T1DstReg, false),
                  Magics.P - 32, MRI, TII)
          .getReg();

  // T2 = Quotient * OpRHS
  const Register T3DefReg = MRI.createVirtualRegister(TRCi32);
  BuildMI(MBB, InsertPos, DebugLoc(), TII.get(TPC::MULsip), T3DefReg)
      .addReg(QuotientDefReg)
      .addImm(OpRHS)
      .addImm(TPCII::OpType::INT32)
      .addImm(0)
      .addReg(T3DefReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

  // Rem = OpLHS - T2
  return evalSubI32(MBB, InsertPos, OpLHS,
                    MachineOperand::CreateReg(T3DefReg, false), MRI, TII);
}

Register insertImplicitDef(const TargetRegisterClass &RC,
                           MachineBasicBlock &MBB,
                           const MachineBasicBlock::iterator Before,
                           MachineRegisterInfo &MRI,
                           const TargetInstrInfo &TII) {
  const Register R = MRI.createVirtualRegister(&RC);
  BuildMI(MBB, Before, DebugLoc(), TII.get(TargetOpcode::IMPLICIT_DEF), R);
  return R;
}

Register insertVRegCOPY(const Register SrcReg, MachineBasicBlock &MBB,
                        const MachineBasicBlock::iterator Before,
                        MachineRegisterInfo &MRI, const TargetInstrInfo &TII) {
  assert(SrcReg.isVirtual());

  const Register Result = MRI.cloneVirtualRegister(SrcReg);
  BuildMI(MBB, Before, DebugLoc(), TII.get(TargetOpcode::COPY), Result)
      .addReg(SrcReg);
  return Result;
}

SmallVector<PhiIP, 4> getAllPhiIPs(MachineInstr &Phi) {
  assert(Phi.isPHI());
  const unsigned OpCount = Phi.getNumOperands();

  SmallVector<PhiIP, 4> RV;
  RV.reserve((OpCount - 1) / 2);
  for (unsigned Ix = 1; Ix < OpCount; Ix += 2)
    RV.emplace_back(PhiIP{&Phi.getOperand(Ix), &Phi.getOperand(Ix + 1)});
  return RV;
}

template <typename MOT, typename MIT>
MOT *getPhiValueForMBBT(MIT &Phi, const MachineBasicBlock &MBB) {
  const Optional<unsigned> Ix = getIndexOfPhiValOp(Phi, MBB);
  return Ix.hasValue() ? &Phi.getOperand(Ix.getValue()) : nullptr;
}

MachineOperand *getPhiValueForMBB(MachineInstr &Phi,
                                  const MachineBasicBlock &MBB) {
  return getPhiValueForMBBT<MachineOperand>(Phi, MBB);
}

const MachineOperand *getPhiValueForMBB(const MachineInstr &Phi,
                                        const MachineBasicBlock &MBB) {
  return getPhiValueForMBBT<const MachineOperand>(Phi, MBB);
}

Optional<PhiIP> getPhiIPForMBB(MachineInstr &Phi,
                               const MachineBasicBlock &MBB) {
  const Optional<unsigned> MBBIx = getIndexOfPhiMBBOp(Phi, MBB);
  const Optional<unsigned> ValIx = getIndexOfPhiValOp(Phi, MBB);
  assert(MBBIx.hasValue() == ValIx.hasValue());
  return MBBIx.hasValue()
             ? Optional<PhiIP>(PhiIP{&Phi.getOperand(ValIx.getValue()),
                                     &Phi.getOperand(MBBIx.getValue())})
             : Optional<PhiIP>();
}

bool isPhiInputs(const MachineInstr &Phi,
                 const ArrayRef<const PhiIP *> Inputs) {
  return Phi.getNumOperands() == Inputs.size() * 2 + 1 &&
         all_of(Inputs, [&](const PhiIP *IP) {
           auto *Val = getPhiValueForMBB(Phi, *IP->MBBOp->getMBB());
           return Val && Val->isIdenticalTo(*IP->ValOp);
         });
}

bool removePhiInputFromMBB(MachineInstr &Phi, const MachineBasicBlock &MBB) {
  const Optional<unsigned> MBBIx = getIndexOfPhiMBBOp(Phi, MBB);
  const Optional<unsigned> ValIx = getIndexOfPhiValOp(Phi, MBB);
  assert(MBBIx.hasValue() == ValIx.hasValue());

  if (!MBBIx.hasValue())
    return false;

  Phi.RemoveOperand(MBBIx.getValue()); // MBB (remove first: MBBIx > ValIx)
  Phi.RemoveOperand(ValIx.getValue()); // Value
  return true;
}

Optional<unsigned> getIndexOfPhiMBBOp(const MachineInstr &Phi,
                                      const MachineBasicBlock &MBB) {
  assert(Phi.isPHI());
  for (auto &Item : enumerate(Phi.operands())) {
    if (Item.value().isMBB() && Item.value().getMBB() == &MBB)
      return Item.index();
  }
  return None;
}

Optional<unsigned> getIndexOfPhiValOp(const MachineInstr &Phi,
                                      const MachineBasicBlock &MBB) {
  const Optional<unsigned> MBBIx = getIndexOfPhiMBBOp(Phi, MBB);
  return MBBIx.hasValue() ? Optional<unsigned>(MBBIx.getValue() - 1)
                          : Optional<unsigned>();
}

MachineInstr &insertPHI(const TargetRegisterClass &RC,
                        ArrayRef<const PhiIP *> IPs, MachineBasicBlock &MBB,
                        const MachineBasicBlock::iterator Before,
                        MachineRegisterInfo &MRI, const TargetInstrInfo &TII) {
  const Register Reg = MRI.createVirtualRegister(&RC);
  const MachineInstrBuilder MIB =
      BuildMI(MBB, Before, DebugLoc(), TII.get(TargetOpcode::PHI), Reg);
  for (const PhiIP *IP : IPs) {
    MIB.add(*IP->ValOp);
    MIB.addMBB(IP->MBBOp->getMBB());
  }
  return *MIB.getInstr();
}

bool isLikelyTied(const MachineOperand &MO, const MachineRegisterInfo &MRI) {
  assert(MO.isReg());

  // Tricky knowledge:
  // * Undef tied-def does not lead to registry collapse.
  // * Tied-def with IMPLICIT_DEF result does not lead
  if (MO.isTied() && !MO.isUndef()) {
    if (MO.getReg().isPhysical())
      return true;
    return !MRI.getVRegDef(MO.getReg())->isImplicitDef();
  }
  return false;
}

bool isPlaceholderReg(const MachineOperand &MO,
                      const MachineRegisterInfo &MRI) {
  return MO.isReg() &&
         (MO.isUndef() || (MO.isTied() && !isLikelyTied(MO, MRI)));
}

} // namespace llvm
