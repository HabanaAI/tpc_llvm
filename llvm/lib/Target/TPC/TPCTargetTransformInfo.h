//===-- TPCTargetTransformInfo.h - TPC specific TTI -------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
/// \file
/// This file contains declaration of a TargetTransformInfo object specific to
/// the TPC target machine. It should help to tune target-independent passes to
/// make their job more TPC friendly.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_TPC_TPCTARGETTRANSFORMINFO_H

#include "TPC.h"
#include "TPCMapCompoundInst.h"
#include "TPCTargetMachine.h"
#include "latencies.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include <algorithm>

namespace llvm {

class TPCTTIImpl : public BasicTTIImplBase<TPCTTIImpl> {

  using VpuSpuSlot = std::pair<uint64_t, uint64_t>;
  typedef std::map<Intrinsic::ID, VpuSpuSlot> IntrinToSlot;
  typedef std::map<int, VpuSpuSlot> InstToSlot;

  typedef BasicTTIImplBase<TPCTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const TPCSubtarget *ST;
  const TPCTargetLowering *TLI;
  //  Maps : Intrinsic/Instruction => <vpu, spu> H/W opcodes
  IntrinToSlot IntrinToSlotMap;
  InstToSlot InstToSlotMap;
  CompoundInstToIntrinsMap CIIMap;

public:
  explicit TPCTTIImpl(const TPCTargetMachine *TM, const Function &F)
    : BaseT(TM, F.getParent()->getDataLayout()),
      ST(TM->getSubtargetImpl(F)),
      TLI(ST->getTargetLowering()) {
  }
  static const unsigned DEFAULT_LATENCY;
  static const unsigned ZERO_LATENCY;

  const TPCSubtarget *getST() const { return ST; }
  const TPCTargetLowering *getTLI() const { return TLI; }

  unsigned getNumberOfRegisters(bool Vector) const;
  //unsigned getRegisterBitWidth(bool Vector) const;
  bool isSupportedConstant(const Constant *C) const;

  IntrinToSlot &getIntrinToSlotMap() { return IntrinToSlotMap; }
  InstToSlot &getInstToSlotMap() { return InstToSlotMap; }
  CompoundInstToIntrinsMap &getCiiMap() { return CIIMap; }

  const IntrinToSlot &getIntrinToSlotMap() const { return IntrinToSlotMap; }
  const InstToSlot &getInstToSlotMap() const { return InstToSlotMap; }
  const CompoundInstToIntrinsMap &getCiiMap() const { return CIIMap; }

  bool shouldExpandReduction(const IntrinsicInst *II) const { return false; }

  int getInstructionLatency(const Instruction *I);
  int getIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                            TTI::TargetCostKind CostKind);
  int getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy,
                            ArrayRef<Value *> Args, FastMathFlags FMF,
                            unsigned VF = 1);

  int getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy, ArrayRef<Type *> Tys,
                            FastMathFlags FMF,
                            unsigned ScalarizationCostPassed = UINT_MAX) const {

    assert(false && "TPC does not support costs based purely on Types. Use the "
                    "variant with Values.");
    return -1;
  }

  int getShuffleCost(TTI::ShuffleKind Kind, VectorType *Tp, int Index,
                     VectorType *SubTp);

  static bool getTypeMatrix(const Instruction *I, Type::TypeID Ty[]) {
    Ty[0] = I->getType()->getTypeID();
    if (I->getType()->getTypeID() == Type::FixedVectorTyID) {
      Ty[1] = cast<VectorType>(I->getType())->getElementType()->getTypeID();
    }
    return true;
  }

  bool isCompoundInst(const Instruction *I) const {
    return getCiiMap().find(I->getOpcode()) != getCiiMap().end();
  }
  bool extractAndPopulate(
      const Instruction *I,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &SrcIld) const;
  bool extractAndPopulate(
      const Intrinsic::ID ID, Type *RetTy, ArrayRef<const Value *> Args,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &inst) const;
  bool extractAndPopulate(
      const IntrinsicInfo,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &inst) const;
  bool populateDestination(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const;
  unsigned getLatency(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &dest,
      bool DestPresent);
  bool initLatenciesDB(void) const;
  bool initIldMap(void);

  bool initBE(void) {
    initIldMap();
    initLatenciesDB();
    return true;
  }

  // We get latency by deriving the difference between the source
  // (TPCLatencyEvaluation::e_src_a) and destination
  // (TPCLatencyEvaluation::e_dst) of an instruction A query function (to
  // latencesDB) that basically just does a dest-srcA+1(forward the data). So we
  // only take source ILD to derive the latency.
  //
  // This model is simpler than fashioning a producer and a consumer instruction
  // and provides a reasonable degree of accuracy.
  unsigned
  getLatency(TPCLatencyEvaluation::InstructionForLatencyDetermination &src);

  static bool
  getDefaultILD(TPCLatencyEvaluation::InstructionForLatencyDetermination &);
  static bool getFloatInfo(
      const Type::TypeID Input1,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &target);

  static bool getVectorScalarInfo(
      Type::TypeID Input,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &target);

  // Use Instruction::Opcode or Intrinic::ID to lookup H/W opCodes
  template <class MapType, class KeyType>
  bool
  getOpcodeSlot(KeyType Id, const Type::TypeID Ty,
                TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild,
                const MapType &Map) const;
  static bool getLoadStoreConfig(
      TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild,
      TPCLatencyEvaluation::_IssueSlot Islot,
      TPCLatencyEvaluation::_RegisterFile Rf, uint32_t);
  bool populateDestinationSTSlot(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const;
  bool populateDestinationLDSlot(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const;
  bool populateDestinationVPU(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const;
  bool populateDestinationSPU(
      const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
      TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const;
};
}
#endif

