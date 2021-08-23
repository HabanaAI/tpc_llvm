//===-TPCIntrinsicUtils.cpp-----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines intrinsics API and helper function to access instruction switches
// from switche table for middle end.It can be extended as need be.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/TPCIntrinsicUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "../lib/Target/TPC/MCTargetDesc/InstructionDB.h"

namespace llvm {

enum ValueType : uint8_t {

  INVALID_VALUE_TYPE = 0,

  Other = 1, // This is a non-standard value
  i1 = 2,    // This is a 1 bit integer value
  i8 = 3,    // This is an 8 bit integer value
  i16 = 4,   // This is a 16 bit integer value
  i32 = 5,   // This is a 32 bit integer value
  i64 = 6,   // This is a 64 bit integer value
  i128 = 7,  // This is a 128 bit integer value

  f16 = 8,  // This is a 16 bit floating point value
  bf16 = 9, // BFloat16
  f32 = 10, // This is a 32 bit floating point value
  f64 = 11  // This is a 64 bit floating point value
};

// POD for defining switch table for convert intrinsics
struct SwitchTbl {
  ValueType SrcType;
  ValueType DstType;
  unsigned SW1;
  unsigned SW2;
};

// Populate swithes for all supported combinations of source and destination
// type.
static SwitchTbl SwTbl[] = {
    // FPTrunc/FPTExt: for specific rounding mode
    {ValueType::f32, ValueType::bf16, TPCII::SW_FP32,
     TPCII::SW_TO_BF16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_SINGLE_LANE},

    {ValueType::f32, ValueType::f16, TPCII::SW_FP32,
     TPCII::SW_TO_FP16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_SINGLE_LANE},

    {ValueType::bf16, ValueType::f32, TPCII::SW_BF16,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::f16, ValueType::f32, TPCII::SW_FP16,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    // FPToSI/FPToUI: for specific rounding mode
    {ValueType::f32, ValueType::i8, TPCII::SW_FP32,
     TPCII::SW_TO_INT8 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::f32, ValueType::i16, TPCII::SW_FP32,
     TPCII::SW_TO_INT16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::f32, ValueType::i32, TPCII::SW_FP32,
     TPCII::SW_TO_INT32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::bf16, ValueType::i16, TPCII::SW_BF16,
     TPCII::SW_TO_INT16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::f16, ValueType::i16, TPCII::SW_FP16,
     TPCII::SW_TO_INT16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    // SIToFP/UIToFP: for specific rounding mode
    {ValueType::i32, ValueType::f32, TPCII::SW_INT32,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i32, ValueType::bf16, TPCII::SW_INT32,
     TPCII::SW_TO_BF16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::i16, ValueType::f32, TPCII::SW_INT16,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i16, ValueType::bf16, TPCII::SW_INT16,
     TPCII::SW_TO_BF16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i16, ValueType::i32, TPCII::SW_INT16,
     TPCII::SW_TO_FP16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i16, ValueType::i8, TPCII::SW_INT16,
     TPCII::SW_TO_INT8 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::i8, ValueType::f32, TPCII::SW_INT8,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i8, ValueType::i32, TPCII::SW_INT8,
     TPCII::SW_TO_INT32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i8, ValueType::i16, TPCII::SW_INT8,
     TPCII::SW_TO_INT16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::i8, ValueType::f32, TPCII::SW_INT8,
     TPCII::SW_TO_FP32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i8, ValueType::i32, TPCII::SW_INT8,
     TPCII::SW_TO_INT32 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},
    {ValueType::i8, ValueType::i16, TPCII::SW_INT8,
     TPCII::SW_TO_INT16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES},

    {ValueType::i16, ValueType::bf16, TPCII::SW_INT16,
     TPCII::SW_TO_BF16 | TPCII::SW_CSR | TPCII::SW_LANE_0 |
         TPCII::SW_ALL_LANES}};

// Get the tpc intrinsic corresponding to \p IDNum and \p FType.
static std::string getTPCIntrinsicName(Intrinsic::ID IDNum,
                                       FunctionType *FType) {
  SmallVector<Intrinsic::IITDescriptor, 8> Table;
  Intrinsic::getIntrinsicInfoTableEntries(IDNum, Table);
  ArrayRef<Intrinsic::IITDescriptor> TableRef = Table;
  (void)TableRef;
  SmallVector<Type *, 4> ArgTys;
  Intrinsic::matchIntrinsicSignature(FType, TableRef, ArgTys);
  return Intrinsic::getName(IDNum, ArgTys);
}

// Lookup API for accessing switches based on \p SrcType and \p DstType.
static const SwitchTbl *SwitchTableLookup(ArrayRef<SwitchTbl> Tbl,
                                          ValueType SrcType,
                                          ValueType DstType) {
  auto I = find_if(Tbl, [=](const SwitchTbl &Entry) {
    return SrcType == Entry.SrcType && DstType == Entry.DstType;
  });
  if (I != Tbl.end())
    return I;

  // Could not find an entry.
  return nullptr;
}

static ValueType getIntegerVT(unsigned BitWidth) {
  switch (BitWidth) {
  default:
    return ValueType::INVALID_VALUE_TYPE;
  case 1:
    return ValueType::i1;
  case 8:
    return ValueType::i8;
  case 16:
    return ValueType::i16;
  case 32:
    return ValueType::i32;
  case 64:
    return ValueType::i64;
  case 128:
    return ValueType::i128;
  }
}

static ValueType getVT(Type *Ty) {
  switch (Ty->getTypeID()) {
  default:
    llvm_unreachable("Unknown type!");
  case Type::IntegerTyID:
    return getIntegerVT(cast<IntegerType>(Ty)->getBitWidth());
  case Type::BFloat16ID:
    return ValueType::bf16;
  case Type::HalfTyID:
    return ValueType::f16;
  case Type::FloatTyID:
    return ValueType::f32;
  case Type::DoubleTyID:
    return ValueType::f64;
  }
}

// Creates the required intrinsic instruction corresponding the instruction \p
// InstrToReplace.\p Switch is used to supply rounding mode switch to be encoded
// in assembly instruction.
CallInst *createConvertIntrinsic(Instruction *InstrToReplace,
                                 unsigned int Switch = 0) {
  auto &Context = InstrToReplace->getParent()->getContext();
  IRBuilder<> Builder(InstrToReplace);
  IntegerType *I1Type = Type::getInt1Ty(Context);
  IntegerType *I8Type = Type::getInt8Ty(Context);
  IntegerType *I32Type = Type::getInt32Ty(Context);

  // Define source and result type
  Value *Operand = InstrToReplace->getOperand(0);
  Type *FromType = Operand->getType();
  Type *ToType = InstrToReplace->getType();

  // Get switch entry corresponding to \p SrcTy and \p DstTy
  ValueType SrcTy = getVT(FromType->getScalarType());
  ValueType DstTy = getVT(ToType->getScalarType());
  auto *Entry = SwitchTableLookup(SwTbl, SrcTy, DstTy);

  // Define intrinsic signature
  SmallVector<Type *, 7> ArgTypes{FromType, I8Type, I32Type,
                                  ToType,   I1Type, I1Type};
  FunctionType *FType = FunctionType::get(ToType, ArgTypes, false);

  // Define intrinsic function
  Function *Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());

  // Create intrinsic call
  auto *Call = Builder.CreateCall(
      Intrinsic,
      {InstrToReplace->getOperand(0), // Src
       llvm::ConstantInt::get(IntegerType::get(Context, 8),
                              Entry->SW1), // Operand Type
       llvm::ConstantInt::get(IntegerType::get(Context, 32),
                              Entry->SW2 | Switch),         // Switches
       UndefValue::get(ToType),                             // Income
       llvm::ConstantInt::get(Type::getInt1Ty(Context), 1), // Predicate
       llvm::ConstantInt::getFalse(Context)});              // Polarity

  return Call;
}

} // namespace llvm