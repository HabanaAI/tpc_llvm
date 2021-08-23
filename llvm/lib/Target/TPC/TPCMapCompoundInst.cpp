//===-- TPCMapCompoundInst.cpp - TPC Map Compound Inst -------------*- C++
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file contains code that maps compound Instruction to the corresponding
/// set of Intrinsics.
/// This code is the property of Habana.
//===----------------------------------------------------------------------===//

#include "TPCMapCompoundInst.h"

IntrinsicVector MapTruncate::downConvertVec(const Instruction *I,
                                            const TPCSubtarget *ST) {
  unsigned SrcWidth = 0, DestWidth = 0;
  assert(I->getType()->getTypeID() == Type::VectorTyID);
  SrcWidth =
      I->getOperand(0)->getType()->getScalarType()->getPrimitiveSizeInBits();
  DestWidth = I->getType()->getScalarType()->getPrimitiveSizeInBits();
  IntrinsicVector Append;
  if (SrcWidth == 32) {
    if (DestWidth == 16) {
      Append = getTrunc32_16(ST);
    } else if (DestWidth == 8) {
      Append = getTrunc32_8();
    }
  } else if (SrcWidth == 16 && DestWidth == 8) {
    Append = getTrunc16_8();
  }
  return Append;
}

IntrinsicVector MapTruncate::getMapping(const Instruction *I,
                                        const TPCSubtarget *ST) {
  IntrinsicVector Vec, Append;
  if (I->getType()->getTypeID() == Type::VectorTyID) {
    switch (I->getType()->getVectorNumElements()) {
    case 256:
      Vec = {{Intrinsic::tpc_mov_irf_dim, 4}, {Intrinsic::tpc_convert, 4}};
      break;
    case 128:
      Vec = {{Intrinsic::tpc_mov_irf_dim, 2}, {Intrinsic::tpc_convert, 2}};
      break;
    default:
      break;
    }
    Append = downConvertVec(I, ST);
  } else {
    Vec = {{Intrinsic::tpc_convert, 1}};
  }
  if (Append.size()) {
    Vec.insert(Vec.end(), Append.begin(), Append.end());
  }
  return Vec;
}

IntrinsicVector MapExtend::upConvertVec(const Instruction *I,
                                        const TPCSubtarget *ST) {
  unsigned SrcWidth = 0, DestWidth = 0;
  IntrinsicVector Append;
  assert(I->getType()->getTypeID() == Type::VectorTyID);
  SrcWidth =
      I->getOperand(0)->getType()->getScalarType()->getPrimitiveSizeInBits();
  DestWidth = I->getType()->getScalarType()->getPrimitiveSizeInBits();
  if (SrcWidth == 8) {
    if (DestWidth == 16) {
      Append = getExt8_16(ST);
    } else if (DestWidth == 32) {
      Append = getExt8_32(ST);
    }
  } else if (SrcWidth == 16 && DestWidth == 32) {
    Append = getExt16_32(ST);
  }
  return Append;
}

IntrinsicVector MapExtend::getMapping(const Instruction *I,
                                      const TPCSubtarget *ST) {
  IntrinsicVector Vec;
  if (I->getType()->getTypeID() == Type::VectorTyID) {
    Vec = upConvertVec(I, ST);
  } else if (I->getOpcode() == Instruction::FPExt) {
    Vec = {{Intrinsic::tpc_convert, 1}};
  }
  return Vec;
}

IntrinsicVector MapExtendTruncate::getMapping(const Instruction *I,
                                              const TPCSubtarget *ST) {

  unsigned SrcWidth, DestWidth;
  IntrinsicVector Vec;
  assert(I->getNumOperands() > 0);
  SrcWidth =
      I->getOperand(0)->getType()->getScalarType()->getPrimitiveSizeInBits();
  DestWidth = I->getType()->getScalarType()->getPrimitiveSizeInBits();
  if (SrcWidth == DestWidth) {
    Vec = {{Intrinsic::tpc_convert, 1}};
  } else {
    if (SrcWidth < DestWidth) {
      Vec = MapExtend::getMapping(I, ST);
    } else if (SrcWidth > DestWidth) {
      Vec = MapTruncate::getMapping(I, ST);
    }
    Vec.insert(Vec.end(), {{Intrinsic::tpc_convert, 1}});
  }
  return Vec;
}

IntrinsicVector MapSelect::getMapping(const Instruction *I,
                                      const TPCSubtarget *ST) {
  if (I->getType()->getTypeID() != Type::VectorTyID) {
    if (auto *CI = dyn_cast<CmpInst>(I->getOperand(0))) {
      switch (CI->getPredicate()) {
      case CmpInst::Predicate::ICMP_SGT:
      case CmpInst::Predicate::ICMP_UGT:
        return getMax();
      case CmpInst::Predicate::ICMP_SLT:
      case CmpInst::Predicate::ICMP_ULT:
        return getMin();
      default:
        break;
      }
    }
  }
  return getSelect_Mov();
}

IntrinsicVector MapDivide::getMapping(const Instruction *I,
                                      const TPCSubtarget *ST) {
  IntrinsicVector Vec;
  assert(!I->getType()->isVectorTy() &&
         "Vector integer division not supported.");

  Vec.append(getBase().begin(), getBase().end());
  if (!this->Unsigned) {
    Vec.append(And.begin(), And.end());
  }
  Vec.insert(Vec.end(), {Intrinsic::tpc_udiv, 1});
  return Vec;
}

IntrinsicVector MapSignedDivide::getMapping(const Instruction *I,
                                            const TPCSubtarget *ST) {
  IntrinsicVector Vec = MapDivide::getMapping(I, ST);
  Vec.insert(Vec.end(), getSBase().begin(), getSBase().end());
  return Vec;
}

IntrinsicVector MapFRem::getMapping(const Instruction *I,
                                    const TPCSubtarget *ST) {
  IntrinsicVector Result, FDiv;
  FDiv = MapFDivide::getMapping(I, ST);
  Result.append(getBase().begin(), getBase().end());
  Result.append(FDiv.begin(), FDiv.end());
  return Result;
}
