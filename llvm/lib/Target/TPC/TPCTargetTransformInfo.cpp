//===-- TPCTargetTransformInfo.cpp - TPC specific TTI pass ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file contains implementation of a TargetTransformInfo object specific
/// to the TPC target machine. It should help to tune target-independent passes
/// to make their job more TPC friendly.
///
//===----------------------------------------------------------------------===//
#undef _MSC_EXTENSIONS // Turn off iso646 mapping
#include "TPCTargetTransformInfo.h"
#include "MCTargetDesc/InstructionDB.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpctti"

unsigned TPCTTIImpl::getNumberOfRegisters(bool Vector) const {
  if (Vector)
    return 40;
  return 32;
}

// TPC default latency measured in cycles
const unsigned TPCTTIImpl::DEFAULT_LATENCY = 4;
const unsigned TPCTTIImpl::ZERO_LATENCY = 0;

//unsigned TPCTTIImpl::getRegisterBitWidth(bool Vector) const {
//  if (Vector)
//    return 256 * 8; // 2048
//  return 4 * 8;     // 32
//}

bool TPCTTIImpl::isSupportedConstant(const Constant *C) const {
  Type *CTy = C->getType();

  // No restrictions for scalar constants.
  if (!CTy->isVectorTy())
    return true;

  VectorType *CVTy = cast<VectorType>(CTy);
  unsigned EltSize = CVTy->getElementType()->getScalarSizeInBits();
  bool IsElementFloat = !CVTy->getElementType()->isIntegerTy();

  // Assume we can create any constant of type int5. It may be not so cheap,
  // but still no more that 5 instructions.
  if (CVTy->getNumElements() == 5) {
    assert(CVTy->getElementType()->isIntegerTy());
    assert(CVTy->getElementType()->getScalarSizeInBits() == 32);
    return true;
  }

  // Is this a valid type for VLM? If not, report this constant as supported,
  // probably some backend transform can melt it into something more usual.
  bool VLMType = false;
  if (CVTy->getNumElements() == 64 && EltSize == 32)
    VLMType = true;
  else if (CVTy->getNumElements() == 128 && EltSize == 16)
    VLMType = true;
  else if (CVTy->getNumElements() == 256 && EltSize == 8)
    VLMType = true;
  if (!VLMType)
    return true;

  if (isa<ConstantAggregateZero>(C))
    return true;

  if (isa<UndefValue>(C))
    return true;

  if (auto *D = dyn_cast<ConstantDataVector>(C)) {
    if (D->isSplat())
      return true;

    // No float vectors are supported but splats.
    if (IsElementFloat)
      return false;

    // If the vector contains sequence like 0, 1, 2, ..., it can be represented
    // by VRF42, VRF43 or VRF44.
    bool Supported = true;
    if (CVTy->getElementType()->isIntegerTy())
      for (unsigned I = 0, E = D->getNumElements(); I < E; ++I)
        if (I != D->getElementAsInteger(I)) {
          Supported = false;
          break;
        }
    if (Supported)
      return true;

    // Here we can check for other synthesisable constants.
    return false;
  }

  if (auto *D = dyn_cast<ConstantVector>(C)) {
    Constant *V0 = D->getOperand(0);
    bool IsSplat = true;
    for (auto Cur = D->op_begin() + 1, End = D->op_end(); Cur != End; ++Cur)
      if (*Cur != V0) {
        IsSplat = false;
        break;
      }
    if (IsSplat)
      return true;

    // No float vectors are supported but splats.
    if (IsElementFloat)
      return false;

    unsigned Counter = 0;
    for (auto Cur = D->op_begin(), End = D->op_end(); Cur != End; ++Cur, ++Counter) {
      auto *Elt = cast<ConstantInt>(*Cur);
      if (Elt->getLimitedValue() != Counter)
        return false;
    }
    return true;
  }

  llvm_unreachable("Unexpected constant element");
}

bool TPCTTIImpl::getDefaultILD(
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild) {

  Ild.the_opCode = 0;
  Ild.the_slotID = TPCLatencyEvaluation::e_issue_slot_load;
  Ild.the_operandID = TPCLatencyEvaluation::e_src_a;
  Ild.the_idxDst0 = false;
  Ild.the_isOpTypeFloat = false;
  Ild.the_isIRFDest = false;
  Ild.the_isVectorPipe = false;
  Ild.the_isLFSRImplicitDst = false;
  Ild.the_isAccFp32 = false;
  Ild.the_is2SrfDst = false;
  Ild.the_is2xLookupAddSub = false;
  Ild.the_registerFile = TPCLatencyEvaluation::e_rf_a;
  Ild.the_isFp16 = false;
  return true;
}

bool TPCTTIImpl::extractAndPopulate(
    const Instruction *I,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &SrcIld) const {
  bool Success = true;
  Type::TypeID Ty = I->getType()->getTypeID();
  getDefaultILD(SrcIld);

  if (getOpcodeSlot<InstToSlot, int>(I->getOpcode(), Ty, SrcIld,
                                     getInstToSlotMap())) {
    getVectorScalarInfo(Ty, SrcIld);
    getFloatInfo(Ty, SrcIld);
    return true;
  }

  // Handle special cases and others
  switch (I->getOpcode()) {
  case Instruction::ExtractElement:
    SrcIld.the_opCode = TPCII::spuMOV_IRF_DIM;
    SrcIld.the_registerFile = TPCLatencyEvaluation::e_rf_i;
    SrcIld.the_slotID = TPCLatencyEvaluation::e_issue_slot_spu;
    break;
  case Instruction::InsertElement:
    SrcIld.the_opCode = TPCII::ldSET_INDX;
    SrcIld.the_isIRFDest = true;
    SrcIld.the_registerFile = TPCLatencyEvaluation::e_rf_s;
    SrcIld.the_slotID = TPCLatencyEvaluation::e_issue_slot_load;
    break;
  case Instruction::Load:
    if (Ty == Type::VectorTyID) {
      getLoadStoreConfig(SrcIld, TPCLatencyEvaluation::e_issue_slot_load,
                         TPCLatencyEvaluation::e_rf_i, TPCII::LD_TNSR);
      SrcIld.the_isVectorPipe = true;
    } else {
      getLoadStoreConfig(SrcIld, TPCLatencyEvaluation::e_issue_slot_load,
                         TPCLatencyEvaluation::e_rf_s, TPCII::LD_L);
    }
    break;
  case Instruction::Store:
    if (Ty == Type::VectorTyID) {
      getLoadStoreConfig(SrcIld, TPCLatencyEvaluation::e_issue_slot_store,
                         TPCLatencyEvaluation::e_rf_i, TPCII::ST_TNSR);
    } else {
      getLoadStoreConfig(SrcIld, TPCLatencyEvaluation::e_issue_slot_store,
                         TPCLatencyEvaluation::e_rf_s, TPCII::ST_L);
    }
    break;
  default:
    Success = false;
  }

  return Success;
}

bool TPCTTIImpl::extractAndPopulate(
    const IntrinsicInfo II,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild) const {
  bool Success = true;
  TPCTTIImpl::getDefaultILD(Ild);

  if (getOpcodeSlot<IntrinToSlot, Intrinsic::ID>(II.Id, II.Ty[_PRIMARY_TYPE],
                                                 Ild, getIntrinToSlotMap())) {
    getVectorScalarInfo(II.Ty[_PRIMARY_TYPE], Ild);
    if (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID) {
      getFloatInfo(II.Ty[_VECTOR_TYPE], Ild);
    } else {
      getFloatInfo(II.Ty[_PRIMARY_TYPE], Ild);
    }
    return Success;
  }
  LLVM_DEBUG(dbgs() << "\nRetTy of intrinsic is " << II.Ty[_PRIMARY_TYPE]
                    << "\n");
  switch (II.Id) {
    // Multi-issue slot instructions are currently hardcoded to load slot
  case llvm::Intrinsic::tpc_gen_addr:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_i, TPCII::ldGEN_ADDR);
    break;
  case llvm::Intrinsic::tpc_prmt_indx:
    Ild.the_isIRFDest = true;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_i, TPCII::ldPRMT_INDX);
    break;
  case llvm::Intrinsic::tpc_set_indx:
    Ild.the_isIRFDest = true;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_s, TPCII::ldSET_INDX);
    break;
  case llvm::Intrinsic::tpc_prefetch:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_a, TPCII::PREFETCH);
    break;
  case llvm::Intrinsic::tpc_ld_l_v:
    Ild.the_isVectorPipe = true;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_s, TPCII::LD_L_V);
    break;
  case llvm::Intrinsic::tpc_ld_l_v_low:
    Ild.the_isVectorPipe = true;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_s, TPCII::LD_L_V_LOW);
    break;
  case llvm::Intrinsic::tpc_ld_l_v_high:
    Ild.the_isVectorPipe = true;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_s, TPCII::LD_L_V_HIGH);
    break;
  case llvm::Intrinsic::tpc_ld_l:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_s, TPCII::LD_L);
    break;
  case llvm::Intrinsic::tpc_ld_g:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_a, TPCII::LD_G);
    break;
  case llvm::Intrinsic::tpc_ld_tnsr_low:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_i, TPCII::LD_TNSR_LOW);
    break;
  case llvm::Intrinsic::tpc_ld_tnsr_high:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_i, TPCII::LD_TNSR_HIGH);
    break;
  case llvm::Intrinsic::tpc_ld_tnsr:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_i, TPCII::LD_TNSR);
    break;
  case llvm::Intrinsic::tpc_st_tnsr:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_i, TPCII::ST_TNSR);
    break;
  case llvm::Intrinsic::tpc_st_tnsr_high:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_i, TPCII::ST_TNSR_HIGH);
    break;
  case llvm::Intrinsic::tpc_st_tnsr_low:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_i, TPCII::ST_TNSR_LOW);
    break;
  case llvm::Intrinsic::tpc_st_l_v:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID)
                           ? TPCLatencyEvaluation::e_rf_v
                           : TPCLatencyEvaluation::e_rf_s,
                       TPCII::ST_L_V);
    break;
  case llvm::Intrinsic::tpc_st_l_v_high:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID)
                           ? TPCLatencyEvaluation::e_rf_v
                           : TPCLatencyEvaluation::e_rf_s,
                       TPCII::ST_L_V_HIGH);
    break;
  case llvm::Intrinsic::tpc_st_l_v_low:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID)
                           ? TPCLatencyEvaluation::e_rf_v
                           : TPCLatencyEvaluation::e_rf_s,
                       TPCII::ST_L_V_LOW);
    break;
  case llvm::Intrinsic::tpc_aso:
    Ild.the_isVectorPipe =
        (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID) ? true : false;
    Ild.the_operandID = TPCLatencyEvaluation::e_src_p;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_sp, TPCII::ASO);
    break;
  case llvm::Intrinsic::tpc_cache_flush:
    Ild.the_operandID = TPCLatencyEvaluation::e_src_p;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_sp, TPCII::CACHE_FLUSH);
    break;
  case llvm::Intrinsic::tpc_cache_invalidate:
    Ild.the_operandID = TPCLatencyEvaluation::e_src_p;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_store,
                       TPCLatencyEvaluation::e_rf_sp, TPCII::CACHE_INVALIDATE);
    break;
  case llvm::Intrinsic::tpc_lookup:
    Ild.the_isVectorPipe =
        (II.Ty[_PRIMARY_TYPE] == Type::VectorTyID) ? true : false;
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_load,
                       TPCLatencyEvaluation::e_rf_v, TPCII::LOOKUP);
    break;
  case llvm::Intrinsic::tpc_mov_irf_dim:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_spu,
                       TPCLatencyEvaluation::e_rf_i, TPCII::spuMOV_IRF_DIM);
    break;
  case llvm::Intrinsic::tpc_udiv_step:
    getLoadStoreConfig(Ild, TPCLatencyEvaluation::e_issue_slot_spu,
                       TPCLatencyEvaluation::e_rf_s, TPCII::spuUDIV_STEP);
    break;
  default:
    Success = false;
    LLVM_DEBUG(dbgs() << "\nIntrinsic ID " << II.Id << " not handled \n");
  }
  return Success;
}

bool TPCTTIImpl::extractAndPopulate(
    const Intrinsic::ID ID, Type *RetTy, ArrayRef<Value *> Args,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild) const {
  Type::TypeID Ty = RetTy->getTypeID();
  IntrinsicInfo II{ID,
                   {Ty, (Ty == Type::VectorTyID)
                            ? RetTy->getVectorElementType()->getTypeID()
                            : Type::VoidTyID}};
  return extractAndPopulate(II, Ild);
}

bool TPCTTIImpl::getLoadStoreConfig(
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild,
    TPCLatencyEvaluation::_IssueSlot Islot,
    TPCLatencyEvaluation::_RegisterFile Rf, uint32_t Opcode) {
  Ild.the_opCode = Opcode;
  Ild.the_registerFile = Rf;
  Ild.the_slotID = Islot;
  return true;
}

int TPCTTIImpl::getInstructionLatency(const Instruction *I) {
  unsigned latency = TPCTTIImpl::DEFAULT_LATENCY;
  TPCLatencyEvaluation::InstructionForLatencyDetermination Src;

  // Handle cases that does not require the latenciesDB
  // ShuffleVector devolves into a set of extract/insert subregs which in turn
  // becomes copies that are optimized out.
  if (I->getOpcode() == Instruction::BitCast ||
      I->getOpcode() == Instruction::ShuffleVector) {
    return TPCTTIImpl::ZERO_LATENCY;
  }

  TPCTTIImpl::initBE();

  // Handle intrinsics using the dedicated API
  if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
    SmallVector<Value *, 4> Args(II->arg_operands());
    FastMathFlags FMF;
    return getIntrinsicInstrCost(II->getIntrinsicID(), II->getType(), Args,
                                 FMF);
  }

  // 1-1 mapped instruction
  if (extractAndPopulate(I, Src)) {
    latency = getLatency(Src);
  } else if (isCompoundInst(I)) { // 1-to-Many mapped instruction
    LLVM_DEBUG(dbgs() << "Doing compound instruction\n");
    IntrinsicVector IVec = getCiiMap()[I->getOpcode()]->getMapping(I, getST());
    latency = std::accumulate(
        IVec.begin(), IVec.end(), 0,
        [&Src, &I, this](unsigned SumAcc, IntrinsicBunch IB) {
          unsigned int Latency = 0;
          Type::TypeID Ty[TYPE_MATRIX];
          getTypeMatrix(I, Ty);
          if (extractAndPopulate({IB.Id, {Ty[_PRIMARY_TYPE], Ty[_VECTOR_TYPE]}},
                                 Src)) {
            Latency = getLatency(Src);
          } else {
            LLVM_DEBUG(dbgs() << "Unable to find Latency for Intrinsic "
                              << IB.Id << "\n");
            Latency = TPCTTIImpl::DEFAULT_LATENCY;
          }
          LLVM_DEBUG(dbgs() << "Latency for Intrinsic " << IB.Id << " is "
                            << Latency << " Count is " << IB.count << "\n");
          return SumAcc + (Latency * IB.count);
        });
    if (!latency) {
      latency = TPCTTIImpl::DEFAULT_LATENCY;
    }
    LLVM_DEBUG(dbgs() << "Many to one latency for instr " << I->getName()
                      << " is " << latency << "\n");
  } else {
    LLVM_DEBUG(dbgs() << "\nInstruction  " << I->getOpcodeName()
                      << " not handled \n");
  }

  return (int)latency;
}

unsigned TPCTTIImpl::getLatency(
    TPCLatencyEvaluation::InstructionForLatencyDetermination &src) {
  TPCLatencyEvaluation::InstructionForLatencyDetermination dest;
  bool Success = populateDestination(
      const_cast<const TPCLatencyEvaluation::InstructionForLatencyDetermination
                     &>(src),
      dest);
  return getLatency(
      const_cast<const TPCLatencyEvaluation::InstructionForLatencyDetermination
                     &>(src),
      const_cast<const TPCLatencyEvaluation::InstructionForLatencyDetermination
                     &>(dest),
      Success);
}

unsigned TPCTTIImpl::getLatency(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &dest,
    bool DestPresent) {
  int latency = TPCTTIImpl::DEFAULT_LATENCY;

  if (TPCLatencyEvaluation::latenciesDB.find(src) !=
      TPCLatencyEvaluation::latenciesDB.end()) {

    if (DestPresent) {
      if (TPCLatencyEvaluation::latenciesDB.find(dest) !=
          TPCLatencyEvaluation::latenciesDB.end()) {
        latency = TPCLatencyEvaluation::latenciesDB[dest].first -
                  TPCLatencyEvaluation::latenciesDB[src].first + 1;
      } else {
        LLVM_DEBUG(dbgs() << "Choosing default latency of 4 as  destination "
                             "ILD lookup has failed \n");
      }
    } else {
      latency = TPCLatencyEvaluation::latenciesDB[src].first -
                TPCLatencyEvaluation::e_stage_d2 + 1;
    }
    assert(latency > 0 && "Error in calculating latency");

    // Debug dump of src,dest and latency
    LLVM_DEBUG(dbgs() << " Source ILD " << src.str() << "\n");
    if (DestPresent) {
      LLVM_DEBUG(dbgs() << " Destination ILD " << dest.str() << "\n");
    }
    LLVM_DEBUG(dbgs() << "Calculated latency is " << latency << "\n");
  } else {
    LLVM_DEBUG(
        dbgs() << "\nChoosing default latency of 4 as  src ILD not found. \n");
  }

  return (unsigned)latency;
}

int TPCTTIImpl::getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy,
                                      ArrayRef<Value *> Args, FastMathFlags FMF,
                                      unsigned int VF) {
  TPCLatencyEvaluation::InstructionForLatencyDetermination src;
  TPCTTIImpl::initBE();
  if (extractAndPopulate(ID, RetTy, Args, src)) {
    return getLatency(src);
  }
  // return default cycle
  LLVM_DEBUG(dbgs() << " Intrinsic cannot be located in the latenciesDB. "
                       "Returning default cycle 4 \n");
  return TPCTTIImpl::DEFAULT_LATENCY;
}

bool TPCTTIImpl::populateDestinationSPU(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const {
  bool Success = true;

  switch (src.the_opCode) {
  case TPCII::spuMOV_IRF_DIM:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_s;
    break;
  case TPCII::spuCMP_EQ:
  case TPCII::spuCMP_GEQ:
  case TPCII::spuCMP_LEQ:
  case TPCII::spuCMP_NEQ:
  case TPCII::spuCMP_LESS:
  case TPCII::spuCMP_GRT:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_sp;
    break;
  case TPCII::spuJMPA:
  case TPCII::spuJMPR:
    Success = false;
    break;
  }
  return Success;
}

bool TPCTTIImpl::populateDestinationVPU(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const {
  bool Success = true;
  switch (src.the_opCode) {
  case TPCII::vpuCMP_EQ:
  case TPCII::vpuCMP_GEQ:
  case TPCII::vpuCMP_LEQ:
  case TPCII::vpuCMP_NEQ:
  case TPCII::vpuCMP_LESS:
  case TPCII::vpuCMP_GRT:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_vp;
    break;
  }
  return Success;
}

bool TPCTTIImpl::populateDestinationLDSlot(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const {
  bool Success = true;

  switch (src.the_opCode) {
  case TPCII::LD_TNSR:
  case TPCII::LD_TNSR_LOW:
  case TPCII::LD_TNSR_HIGH:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_v;
    break;
  case TPCII::ldGEN_ADDR:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_a;
    break;
  case TPCII::ldSET_INDX:
    dest.the_registerFile = TPCLatencyEvaluation::e_rf_i;
    break;
  case TPCII::PREFETCH:
    Success = false;
    break;
  }
  return Success;
}

bool TPCTTIImpl::populateDestinationSTSlot(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const {
  bool Success = true;

  switch (src.the_opCode) {
  case TPCII::ASO:
  case TPCII::ST_TNSR:
  case TPCII::ST_TNSR_LOW:
  case TPCII::ST_TNSR_HIGH:
  case TPCII::ST_L_V:
  case TPCII::ST_L_V_HIGH:
  case TPCII::ST_L_V_LOW:
  case TPCII::CACHE_INVALIDATE:
  case TPCII::CACHE_FLUSH:
    Success = false;
    break;
  }
  return Success;
}

bool TPCTTIImpl::populateDestination(
    const TPCLatencyEvaluation::InstructionForLatencyDetermination &src,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &dest) const {
  bool Success = true;
  dest = src;
  dest.the_operandID = TPCLatencyEvaluation::e_dst;
  dest.the_isVectorPipe = false;

  assert((src.the_slotID == TPCLatencyEvaluation::e_issue_slot_spu) ||
         (src.the_slotID == TPCLatencyEvaluation::e_issue_slot_vpu) ||
         (src.the_slotID == TPCLatencyEvaluation::e_issue_slot_load) ||
         (src.the_slotID == TPCLatencyEvaluation::e_issue_slot_store) &&
             "Unsupported slot\n");
  // Examine src to configure the destination.
  switch (src.the_slotID) {
  case TPCLatencyEvaluation::e_issue_slot_spu:
    Success = populateDestinationSPU(src, dest);
    break;
  case TPCLatencyEvaluation::e_issue_slot_vpu:
    Success = populateDestinationVPU(src, dest);
    break;
  case TPCLatencyEvaluation::e_issue_slot_load:
    Success = populateDestinationLDSlot(src, dest);
    break;
  case TPCLatencyEvaluation::e_issue_slot_store:
    Success = populateDestinationSTSlot(src, dest);
    break;
  }
  if (!Success) {
    LLVM_DEBUG(dbgs() << "Cannot populateDestination ILD " << Success << "\n");
  }
  return Success;
}

bool TPCTTIImpl::getFloatInfo(
    const Type::TypeID Input,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Target) {

  switch (Input) {
  case Type::FloatTyID:
    Target.the_isOpTypeFloat = true;
    break;
  case Type::HalfTyID:
    Target.the_isFp16 = true;
    break;
  default:
    LLVM_DEBUG(dbgs() << "Non-float type id supplied : " << Input << " \n");
  }
  return true;
}

bool TPCTTIImpl::initLatenciesDB() const {
  if (TPCLatencyEvaluation::latenciesDB.empty()) {
    if (ST->hasGaudiISA()) {
      TPCLatencyEvaluation::gaudi_buildInstructionLatenciesDB();
    }
    else {
      TPCLatencyEvaluation::dali_buildInstructionLatenciesDB();
    }
  }
  return true;
}

bool TPCTTIImpl::getVectorScalarInfo(
    Type::TypeID Input,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &target) {
  if (Input == Type::VectorTyID) {
    target.the_registerFile = TPCLatencyEvaluation::e_rf_v;
    target.the_isVectorPipe = true;
    target.the_slotID = TPCLatencyEvaluation::e_issue_slot_vpu;
  } else {
    target.the_slotID = TPCLatencyEvaluation::e_issue_slot_spu;
    target.the_registerFile = TPCLatencyEvaluation::e_rf_s;
  }
  return true;
}

bool TPCTTIImpl::initIldMap(void) {

  if (!getIntrinToSlotMap().empty()) {
    return true;
  }

#define MAP_COMPOUND_INST(INSTRUCTION, COMPOUND_CLASS)                         \
  getCiiMap().insert(std::make_pair(Instruction::INSTRUCTION,                  \
                                    std::make_shared<COMPOUND_CLASS>()));

#define MAP_INST(INSTRUCTION, SLOT)                                            \
  getInstToSlotMap().insert(std::pair<int, VpuSpuSlot>(                        \
      Instruction::INSTRUCTION,                                                \
      VpuSpuSlot(TPCII::vpu##SLOT, TPCII::spu##SLOT)));

#define MAP_INST_SPU(INSTRUCTION, SLOT)                                        \
  getInstToSlotMap().insert(std::pair<int, VpuSpuSlot>(                        \
      Instruction::INSTRUCTION, VpuSpuSlot(-1, TPCII::spu##SLOT)));

#define MAP_INTRIN(INTRINSIC, SLOT)                                            \
  getIntrinToSlotMap().insert(std::pair<Intrinsic::ID, VpuSpuSlot>(            \
      Intrinsic::tpc_##INTRINSIC,                                              \
      VpuSpuSlot(TPCII::vpu##SLOT, TPCII::spu##SLOT)));

#define MAP_III(INTRINSIC, INSTRUCTION, SLOT)                                  \
  MAP_INTRIN(INTRINSIC, SLOT)                                                  \
  MAP_INST(INSTRUCTION, SLOT)

#define MAP_INTRIN_VPU(INTRINSIC, SLOT)                                        \
  getIntrinToSlotMap().insert(std::pair<Intrinsic::ID, VpuSpuSlot>(            \
      Intrinsic::tpc_##INTRINSIC, VpuSpuSlot(TPCII::vpu##SLOT, -1)));

#include "Ild_mapper.def"
#undef MAP_III
#undef MAP_COMPOUND_INST
#undef MAP_INST
#undef MAP_INTRIN
#undef MAP_INTRIN_VPU

  LLVM_DEBUG(dbgs() << "Intrinsic to H/W slot\n");
  for (auto Elem : getIntrinToSlotMap()) {
    LLVM_DEBUG(dbgs() << "Intrinsic " << Elem.first << " => "
                      << "Vector slot " << Elem.second.first << "Scalar slot "
                      << Elem.second.second << "\n");
  }
  LLVM_DEBUG(dbgs() << "Instruction to H/W slot\n");
  for (auto Elem : getInstToSlotMap()) {
    LLVM_DEBUG(dbgs() << "Instruction " << Elem.first << " => "
                      << "Vector slot " << Elem.second.first << "Scalar slot "
                      << Elem.second.second << "\n");
  }

  return true;
}

template <class MapType, class KeyType>
bool TPCTTIImpl::getOpcodeSlot(
    const KeyType Id, const Type::TypeID Ty,
    TPCLatencyEvaluation::InstructionForLatencyDetermination &Ild,
    const MapType &Map) const {
  bool Success = false;

  if (Map.find(Id) != Map.end()) {
    Ild.the_opCode =
        (Ty == Type::VectorTyID) ? Map.at(Id).first : Map.at(Id).second;
    LLVM_DEBUG(dbgs() << "\nOpcode found! Ild.the_opCode = " << Ild.the_opCode
                      << "\n");
    Success = true;
  }
  return Success;
}
