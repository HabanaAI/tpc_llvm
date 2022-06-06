// ExpandSpecialFunction --
/*
Expand special function definition for TPC
*/

#include "llvm/Transforms/Scalar/EvalSpecialFunctionPass.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/TPCIntrinsicUtils.h"

char EvalSpecialFunctionPass::ID = 0;
static cl::opt<bool> EvalSpclFunc("eval-special-function",
                                  cl::desc("Evaluate Special Funtion IR"),
                                  cl::init(true), cl::ZeroOrMore, cl::Hidden);

Constant *EvalSpecialFunctionPass::getBfloatValue(double V) {
  APFloat APF(V);
  bool unused;
  APF.convert(APFloat::BFloat(), APFloat::rmNearestTiesToEven, &unused);
  return ConstantFP::get(BF16Type->getContext(), APF);
}

#if 1 // MERGE
void EvalSpecialFunctionPass::replaceBF16ReciprocalWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw11 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x bfloat> @llvm.tpc.fclass
  // <128 x bfloat> %2, i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false
  SmallVector<Type *, 6> Types = {Bfloat128Type, I8Type, I32Type,
                                  Bfloat128Type, I1Type, I1Type};
  auto FType = FunctionType::get(Bfloat128Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto FClass = Builder.CreateCall(Intrinsic, {Operand, Sw11, Sw2,
                                               UndefValue::get(Bfloat128Type),
                                               Predicate, Polarity});

  // %4 = bitcast <128 x bfloat> %2 to <128 x i16>
  auto BitCast = Builder.CreateBitCast(Operand, Short128Type);

  // %5 = tail call <128 x i16> @llvm.tpc.and
  // (<128 x i16> %4, i16 32640, i8 7, i32 0, <128 x i16> undef, i1 true, i1
  // false
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto And = Builder.CreateCall(
      Intrinsic,
      {BitCast, ConstantInt::get(I16Type, 32640), ConstantInt::get(I8Type, 7),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = tail call <128 x bfloat> @llvm.tpc.form.fp.num
  // (<128 x bfloat> <bfloat> <128 x bfloat> %2, <128 x bfloat> %2, i8 1, i32
  // 512, <128 x bfloat> undef, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP = Builder.CreateCall(
      Intrinsic, {ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(1.0)), Operand,
                  Operand, Sw11, ConstantInt::get(I32Type, 512),
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %7 = tail call <256 x i16> @llvm.tpc.get.lut.entry
  // <128 x bfloat> %6, i8 5, i8 1, i32 0, <256 x i16> undef, i1 true, i1 false
  Types = {Bfloat128Type, I8Type, I8Type, I32Type,
           Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry = Builder.CreateCall(
      Intrinsic, {FormFP, ConstantInt::get(I8Type, 5),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %8 = shufflevector <256 x i16> %7 0...127
  auto Mask = createSequentialMask(0, 128, 0);
  auto Shuffle1 = Builder.CreateShuffleVector(
      LutEntry, UndefValue::get(Short256Type), Mask);

  // %9 = shufflevector <256 x i16> %7 128...255
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle2 = Builder.CreateShuffleVector(
      LutEntry, UndefValue::get(Short256Type), Mask);

  // %10 = bitcast <128 x i16> %9 to <128 x bfloat>
  auto BitCast1 = Builder.CreateBitCast(Shuffle2, Bfloat128Type);

  // %11 = fsub <128 x bfloat> %6, %10
  auto FSub = Builder.CreateFSub(FormFP, BitCast1);

  // %12 = tail call <128 x bfloat> @llvm.tpc.lookup.1c
  // <128 x i16> %8, i32 405, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1
  // false
  Types = {Short128Type, I32Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, ConstantInt::get(I32Type, 405), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat128Type), Predicate, Polarity});

  // %13 = tail call <256 x bfloat> @llvm.tpc.lookup.2c
  // <128 x i16> %8, i32 402, i32 1, <256 x bfloat> zeroinitializer, i1 true, i1
  // false
  Types = {Short128Type, I32Type, I32Type, Bfloat256Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, ConstantInt::get(I32Type, 402), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C1C2.sroa.0.256.vec.extract.i = shufflevector <256 x bfloat> %13,
  // 128...255
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle3 = Builder.CreateShuffleVector(
      Lookup2, UndefValue::get(Bfloat256Type), Mask);

  // %C1C2.sroa.0.0.vec.extract.i = shufflevector <256 x bfloat> %13 0...128
  Mask = createSequentialMask(0, 128, 0);
  auto Shuffle4 = Builder.CreateShuffleVector(
      Lookup2, UndefValue::get(Bfloat256Type), Mask);

  // %14 = tail call <128 x bfloat> @llvm.tpc.mac
  // <128 x bfloat> %C1C2.sroa.0.256.vec.extract.i, <128 x bfloat> %11, i8 1,
  // i32 0, <128 x bfloat> %C1C2.sroa.0.0.vec.extract.i, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle3, FSub, Sw11, Sw2, Shuffle4, Predicate, Polarity});

  // %15 = tail call <128 x bfloat> @llvm.tpc.mac
  // (<128 x bfloat> %14, <128 x bfloat> %11, i8 1, i32 0, <128 x bfloat> %12,
  // i1 true, i1 false)
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, FSub, Sw11, Sw2, Lookup1, Predicate, Polarity});

  //  %16 = bitcast <128 x bfloat> %15 to <128 x i16>
  auto BitCast2 = Builder.CreateBitCast(Mac2, Short128Type);

  // %17 = tail call <128 x i16> @llvm.tpc.and
  // (<128 x i16> %16, i16 32640, i8 8, i32 0, <128 x i16> undef, i1 true, i1
  // false
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto And2 = Builder.CreateCall(
      Intrinsic,
      {BitCast2, ConstantInt::get(I16Type, 32640), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %18 = tail call <128 x i16> @llvm.tpc.add
  // (<128 x i16> %17, i16 16256, i8 8, i32 0, <128 x i16> undef, i1 true, i1
  // false
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_add, FType), FType)
                         .getCallee());
  auto Add1 = Builder.CreateCall(
      Intrinsic,
      {And2, ConstantInt::get(I16Type, 16256), ConstantInt::get(I8Type, 8), Sw2,
       UndefValue::get(Short128Type), Predicate, Polarity});

  // %19 = tail call <128 x i16> @llvm.tpc.sub
  // <128 x i16> %18, <128 x i16> %5, i8 8, i32 1, <128 x i16> undef, i1 true,
  // i1 false
  Types = {Short128Type, Short128Type, I8Type, I32Type,
           Short128Type, I1Type,       I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_sub, FType), FType)
                         .getCallee());
  auto Sub1 = Builder.CreateCall(
      Intrinsic,
      {Add1, And, ConstantInt::get(I8Type, 8), ConstantInt::get(I32Type, 1),
       UndefValue::get(Short128Type), Predicate, Polarity});

  // %20 = bitcast <128 x i16> %19 to <128 x bfloat>
  auto BitCast3 = Builder.CreateBitCast(Sub1, Bfloat128Type);

  // %21 = tail call <128 x bfloat> @llvm.tpc.form.fp.num
  // <128 x bfloat> %20, <128 x bfloat> %2, <128 x bfloat> %15, i8 1, i32 0,
  // <128 x bfloat> undef, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic, {BitCast3, Operand, Mac2, Sw11, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %22 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special
  // <128 x bfloat> %3, <128 x bfloat> undef, i8 1, i32 0, <128 x bfloat> %21,
  // i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto CalcFP = Builder.CreateCall(
      Intrinsic,
      {FClass, UndefValue::get(Bfloat128Type), ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 0), FormFP1, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(CalcFP);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceReciprocalWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw10 = ConstantInt::get(I8Type, 0);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = bitcast <64 x float> %2 to <64 x i32>
  auto BitCast = Builder.CreateBitCast(Operand, Int64Type);

  // %4 = tail call <64 x float> @llvm.tpc.form.fp.num
  // (<256 x i8> zeroinitializer, <64 x float> %2, <64 x float> %2, i8 0, i32
  // 2816, <64 x float> undef, i1 true, i1 false)
  SmallVector<Type *, 6> Types = {I8256Type, Float64Type, Float64Type, I8Type,
                                  I32Type,   Float64Type, I1Type,      I1Type};
  auto FType = FunctionType::get(Float64Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP = Builder.CreateCall(
      Intrinsic, {Constant::getNullValue(I8256Type), Operand, Operand, Sw10,
                  ConstantInt::get(I32Type, 2816), UndefValue::get(Float64Type),
                  Predicate, Polarity});

  // %5 = tail call <128 x i32> @llvm.tpc.get.lut.entry
  // (<64 x float> %4, i8 16, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  Types = {Float64Type, I8Type, I8Type, I32Type, Int128Type, I1Type, I1Type};
  FType = FunctionType::get(Int128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry = Builder.CreateCall(
      Intrinsic, {FormFP, ConstantInt::get(I8Type, 16), Sw10, Sw2,
                  UndefValue::get(Int128Type), Predicate, Polarity});

  //  %6 = shufflevector <128 x i32> %5, 0...63
  auto Mask = createSequentialMask(0, 64, 0);
  auto Shuffle1 =
      Builder.CreateShuffleVector(LutEntry, UndefValue::get(Int128Type), Mask);

  // %7 = shufflevector <128 x i32> %5, 64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle2 =
      Builder.CreateShuffleVector(LutEntry, UndefValue::get(Int128Type), Mask);

  // %8 = bitcast <64 x i32> %7 to <64 x float>
  auto BitCast1 = Builder.CreateBitCast(Shuffle2, Float64Type);

  // %sub.i.i = fsub <64 x float> %4, %8
  auto Subii = Builder.CreateFSub(FormFP, BitCast1);

  // %9 = tail call <64 x float> @llvm.tpc.lookup.1c
  // (<64 x i32> %6, i32 129, i32 0, <64 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic, {Shuffle1, ConstantInt::get(I32Type, 129), Sw2,
                  UndefValue::get(Float64Type), Predicate, Polarity});

  // %10 = tail call <128 x float> @llvm.tpc.lookup.2c
  // (<64 x i32> %6, i32 17, i32 0, <128 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type, I32Type, Float128Type, I1Type, I1Type};
  FType = FunctionType::get(Float128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic, {Shuffle1, ConstantInt::get(I32Type, 129), Sw2,
                  UndefValue::get(Float128Type), Predicate, Polarity});

  // %11 = shufflevector <128 x float> %10, 0...63
  Mask = createSequentialMask(0, 64, 0);
  auto Shuffle3 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %12 = shufflevector <128 x float> %10, 64...128
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle4 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %13 = tail call <64 x float> @llvm.tpc.mac
  // (<64 x float> %12, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %11, i1
  // true, i1 false)
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle4, Subii, Sw10, Sw2, Shuffle3, Predicate, Polarity});

  // %14 = tail call <64 x float> @llvm.tpc.mac
  // (<64 x float> %13, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %9, i1
  // true, i1 false)
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, Subii, Sw10, Sw2, Lookup1, Predicate, Polarity});

  // %15 = bitcast <64 x float> %14 to <64 x i32>
  auto BitCast2 = Builder.CreateBitCast(Mac2, Int64Type);

  // %and.i.i = and <64 x i32> %15, <i32 2139095040...
  auto Andii = Builder.CreateAnd(
      BitCast2,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 2139095040)));

  // %and4.i.i = and <64 x i32> %3, <i32 2139095040,
  auto And4ii = Builder.CreateAnd(
      BitCast,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 2139095040)));

  // %sub5.i.i = sub nsw <64 x i32> %and.i.i, %and4.i.i
  auto Sub5ii = Builder.CreateNSWSub(Andii, And4ii);

  // %16 = bitcast <64 x i32> %sub5.i.i to <64 x float>
  auto BitCast3 = Builder.CreateBitCast(Sub5ii, Float64Type);

  // %17 = tail call <64 x float> @llvm.tpc.form.fp.num
  // (<64 x float> %16, <64 x float> %2, <64 x float> %14, i8 0, i32 256,
  // <64 x float> undef, i1 true, i1 false)
  Types = {Float64Type, Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic, {BitCast3, Operand, Mac2, Sw10, ConstantInt::get(I32Type, 256),
                  UndefValue::get(Float64Type), Predicate, Polarity});

  // %18 = tail call <64 x float> @llvm.tpc.sel.less
  // <64 x float> %2, float 0.000000e+00, <64 x float> <float -0.000000e+00
  // <64 x float> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1
  // false
  Types = {Float64Type, F32Type,     Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_less, FType),
                            FType)
          .getCallee());
  auto SelLess = Builder.CreateCall(
      Intrinsic,
      {Operand, ConstantFP::get(F32Type, 0.000000e+00),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -0.000000e+00)),
       Constant::getNullValue(Float64Type), Sw10, Sw2,
       UndefValue::get(Float64Type), Predicate, Polarity});

  //  %19 = tail call <64 x float> @llvm.tpc.sel.leq
  // (<64 x i32> %sub5.i.i, i32 -1065353216, <64 x float> %18, <64 x float> %17,
  // i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type,     Float64Type, Float64Type, I8Type,
           I32Type,   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_leq, FType),
                            FType)
          .getCallee());
  auto SelLeq1 = Builder.CreateCall(
      Intrinsic, {Sub5ii, ConstantInt::get(I32Type, -1065353216), SelLess,
                  FormFP1, ConstantInt::get(I8Type, 2), Sw2,
                  UndefValue::get(Float64Type), Predicate, Polarity});

  //  %20 = tail call <64 x float> @llvm.tpc.fclass.
  // <64 x float> %2, i8 0, i32 0, <64 x float> undef, i1 true, i1 false
  Types = {Float64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto FClass = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %21 = tail call <64 x float> @llvm.tpc.calc.fp.special
  // <64 x float> %20, <64 x float> undef, i8 0, i32 0, <64 x float> %19, i1
  // true, i1 false
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto CalcFP =
      Builder.CreateCall(Intrinsic, {FClass, UndefValue::get(Float64Type), Sw10,
                                     Sw2, SelLeq1, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(CalcFP);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceTanhWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw10 = ConstantInt::get(I8Type, 0);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %2)
  auto FType = FunctionType::get(Float64Type, {Operand->getType()}, false);
  auto Intrinsic =
      cast<Function>(InstrToReplace->getModule()
                         ->getOrInsertFunction(
                             getTPCIntrinsicName(Intrinsic::fabs, FType), FType)
                         .getCallee());
  auto Fabs = Builder.CreateCall(Intrinsic, {Operand});

  // %4 = bitcast <64 x float> %3 to <64 x i32>
  auto BitCast = Builder.CreateBitCast(Fabs, Int64Type);

  // %5 = tail call <64 x i32> @llvm.tpc.extract.exp
  // (<64 x float> %2, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  SmallVector<Type *, 6> Types = {Float64Type, I8Type, I32Type,
                                  Int64Type,   I1Type, I1Type};
  FType = FunctionType::get(Int64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Int64Type), Predicate, Polarity});

  // %6 = tail call <256 x i1> @llvm.tpc.cmp.less
  // (<64 x i32> %5, i32 0, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type, I8Type, I32Type, Char256Type, I1Type, I1Type};
  FType = FunctionType::get(Char256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_less, FType),
                            FType)
          .getCallee());
  auto CmpLess = Builder.CreateCall(
      Intrinsic, {ExtractExp, Sw2, ConstantInt::get(I8Type, 2), Sw2,
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %7 = tail call <128 x i32> @llvm.tpc.get.lut.entry
  // (<64 x float> %3, i8 17, i8 0, i32 8192, <128 x i32> undef, i1 true, i1
  // false)
  Types = {Float64Type, I8Type, I8Type, I32Type, Int128Type, I1Type, I1Type};
  FType = FunctionType::get(Int128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry = Builder.CreateCall(
      Intrinsic, {Fabs, ConstantInt::get(I8Type, 17), Sw10,
                  ConstantInt::get(I32Type, 8192), UndefValue::get(Int128Type),
                  Predicate, Polarity});

  // %8 = shufflevector <128 x i32> %7, 0...63
  auto Mask = createSequentialMask(0, 64, 0);
  auto Shuffle1 =
      Builder.CreateShuffleVector(LutEntry, UndefValue::get(Int128Type), Mask);

  // %9 = shufflevector <128 x i32> %7, 64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle2 =
      Builder.CreateShuffleVector(LutEntry, UndefValue::get(Int128Type), Mask);

  // %10 = bitcast <64 x i32> %9 to <64 x float>
  auto BitCast1 = Builder.CreateBitCast(Shuffle2, Float64Type);

  // %sub.i.i = fsub <64 x float> %3, %10
  auto Subii = Builder.CreateFSub(Fabs, BitCast1);

  // %11 = tail call <64 x float> @llvm.tpc.lookup.1c
  // (<64 x i32> %8, i32 0, i32 0, <64 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, Sw2, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %12 = tail call <128 x float> @llvm.tpc.lookup.2c
  // (<64 x i32> %8, i32 0, i32 0, <128 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type, I32Type, Float128Type, I1Type, I1Type};
  FType = FunctionType::get(Float128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, Sw2, Sw2, UndefValue::get(Float128Type), Predicate, Polarity});

  // %13 = shufflevector <128 x float> %12 0...63
  Mask = createSequentialMask(0, 64, 0);
  auto Shuffle3 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %14 = shufflevector <128 x float> %12 64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle4 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %15 = tail call <64 x float> @llvm.tpc.mac
  // (<64 x float> %14, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %13, i1
  // true, i1 false)
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle4, Subii, Sw10, Sw2, Shuffle3, Predicate, Polarity});

  // %16 = tail call <64 x float> @llvm.tpc.mac
  // (<64 x float> %15, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %11, i1
  // true, i1 false)
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, Subii, Sw10, Sw2, Lookup1, Predicate, Polarity});

  // %17 = tail call <64 x float> @llvm.tpc.mul
  // (<64 x float> %16, <64 x float> %3, i8 0, i32 0, <64 x float> %16, <256 x
  // i1> %6, i1 false)
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, Char256Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mul, FType), FType)
                         .getCallee());
  auto Mul1 = Builder.CreateCall(
      Intrinsic, {Mac2, Fabs, Sw10, Sw2, Mac2, CmpLess, Polarity});

  // %18 = tail call <256 x i1> @llvm.tpc.cmp.geq
  // (<64 x float> %3, float 8.000000e+00, i8 0, i32 0, <256 x i1> undef, i1
  // true, i1 false)
  Types = {Float64Type, F32Type, I8Type, I32Type, Char256Type, I1Type, I1Type};
  FType = FunctionType::get(Char256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_geq, FType),
                            FType)
          .getCallee());
  auto CmpGeq = Builder.CreateCall(
      Intrinsic, {Fabs, ConstantFP::get(F32Type, 8.000000e+00), Sw10, Sw2,
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %19 = tail call <64 x float> @llvm.tpc.sel.less
  // (<64 x float> %3, float 9.000000e+00, <64 x float>(0.999999881f) <64 x
  // float> %17, i8 0, i32 0, <64 x float> %17, <256 x i1> %18, i1 false)
  Types = {Float64Type, F32Type,     Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, Char256Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_less, FType),
                            FType)
          .getCallee());
  auto SelLess = Builder.CreateCall(
      Intrinsic,
      {Fabs, ConstantFP::get(F32Type, 9.000000e+00),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 0.999999881f)),
       Mul1, Sw10, Sw2, Mul1, CmpGeq, Polarity});

  //  %20 = tail call <64 x float> @llvm.tpc.sel.geq
  // (<64 x float> %3, float 9.000000e+00, <64 x float>(1.000000e+00), <64 x
  // float> %19, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  Types = {Float64Type, F32Type,     Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_geq, FType),
                            FType)
          .getCallee());
  auto SelGeq = Builder.CreateCall(
      Intrinsic,
      {Fabs, ConstantFP::get(F32Type, 9.000000e+00),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.000000e+00)),
       SelLess, Sw10, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %21 = tail call <64 x float> @llvm.tpc.form.fp.num
  // (<64 x float> %20, <64 x float> %2, <64 x float> %20, i8 0, i32 0, <64 x
  // float> undef, i1 true, i1 false)
  Types = {Float64Type, Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP = Builder.CreateCall(
      Intrinsic, {SelGeq, Operand, SelGeq, Sw10, Sw2,
                  UndefValue::get(Float64Type), Predicate, Polarity});

  // %22 = tail call <64 x float> @llvm.tpc.sel.grt
  // (<64 x i32> %4, i32 2139095040, <64 x float>(2147483647f), <64 x float>
  // %21, i8 3, i32 0, <64 x float> undef, i1 true, i1 false)
  Types = {Int64Type, I32Type,     Float64Type, Float64Type, I8Type,
           I32Type,   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt = Builder.CreateCall(
      Intrinsic,
      {BitCast, ConstantInt::get(I32Type, 2139095040),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 2147483647)),
       FormFP, ConstantInt::get(I8Type, 3), Sw2, UndefValue::get(Float64Type),
       Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(SelGrt);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceBF16TanhWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw11 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x bfloat> @llvm.tpc.fclass
  // (<128 x bfloat> %2, i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  SmallVector<Type *, 10> Types{Bfloat128Type, I8Type, I32Type,
                                Bfloat128Type, I1Type, I1Type};
  auto FType = FunctionType::get(Bfloat128Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto FClass = Builder.CreateCall(Intrinsic, {Operand, Sw11, Sw2,
                                               UndefValue::get(Bfloat128Type),
                                               Predicate, Polarity});

  // %4 = tail call <128 x bfloat> @llvm.tpc.abs
  // (<128 x bfloat> %2, i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  Types = {Bfloat128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_abs, FType), FType)
                         .getCallee());
  auto Fabs = Builder.CreateCall(Intrinsic, {Operand, Sw11, Sw2,
                                             UndefValue::get(Bfloat128Type),
                                             Predicate, Polarity});

  // %5 = tail call <128 x i16> @llvm.tpc.extract.exp
  // (<128 x bfloat> %2, i8 1, i32 0, <128 x i16> undef, i1 true, i1 false)
  Types = {Bfloat128Type, I8Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw11, Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = tail call <256 x i1> @llvm.tpc.cmp.less
  // (<128 x i16> %5, i16 0, i8 7, i32 0, <256 x i1> undef, i1 true, i1 false)
  Types = {Short128Type, I16Type, I8Type, I32Type, Char256Type, I1Type, I1Type};
  FType = FunctionType::get(Char256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_less, FType),
                            FType)
          .getCallee());
  auto CmpLess = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I16Type, 0), ConstantInt::get(I8Type, 7),
       Sw2, UndefValue::get(Char256Type), Predicate, Polarity});

  // %7 = tail call <256 x i16> @llvm.tpc.get.lut.entry
  // (<128 x bfloat> %4, i8 4, i8 1, i32 8192, <256 x i16> undef, i1 true, i1
  // false)
  Types = {Bfloat128Type, I8Type, I8Type, I32Type,
           Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry = Builder.CreateCall(
      Intrinsic, {Fabs, ConstantInt::get(I8Type, 4),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 8192),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %8 = shufflevector <256 x i16> %7 0...127
  auto Mask = createSequentialMask(0, 128, 0);
  auto Shuffle1 = Builder.CreateShuffleVector(
      LutEntry, UndefValue::get(Short256Type), Mask);

  // %9 = shufflevector <256 x i16> %7 128...255
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle2 = Builder.CreateShuffleVector(
      LutEntry, UndefValue::get(Short256Type), Mask);

  // %10 = bitcast <128 x i16> %9 to <128 x bfloat>
  auto BitCast = Builder.CreateBitCast(Shuffle2, Bfloat128Type);

  // %11 = fsub <128 x bfloat> %4, %10
  auto FSub = Builder.CreateFSub(Fabs, BitCast);

  // %12 = tail call <128 x bfloat> @llvm.tpc.lookup.1c
  // (<128 x i16> %8, i32 425, i32 1, <128 x bfloat> zeroinitializer, i1 true,
  // i1 false)
  Types = {Short128Type, I32Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, ConstantInt::get(I32Type, 425), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat128Type), Predicate, Polarity});

  // %13 = tail call <256 x bfloat> @llvm.tpc.lookup.2c
  // (<128 x i16> %8, i32 422, i32 1, <256 x bfloat> zeroinitializer, i1 true,
  // i1 false)
  Types = {Short128Type, I32Type, I32Type, Bfloat256Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, ConstantInt::get(I32Type, 422), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C1C2.sroa.0.256.vec.extract.i = shufflevector <256 x bfloat> %13,
  // 128...255
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle3 = Builder.CreateShuffleVector(
      Lookup2, UndefValue::get(Bfloat256Type), Mask);

  // %C1C2.sroa.0.0.vec.extract.i = shufflevector <256 x bfloat> %13 0...128
  Mask = createSequentialMask(0, 128, 0);
  auto Shuffle4 = Builder.CreateShuffleVector(
      Lookup2, UndefValue::get(Bfloat256Type), Mask);

  // %14 = tail call <128 x bfloat> @llvm.tpc.mac
  // (<128 x bfloat> %C1C2.sroa.0.256.vec.extract.i, <128 x bfloat> %11, i8 1,
  // i32 0, <128 x bfloat> %C1C2.sroa.0.0.vec.extract.i, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle3, FSub, Sw11, Sw2, Shuffle4, Predicate, Polarity});

  // %15 = tail call <128 x bfloat> @llvm.tpc.mac
  // (<128 x bfloat> %14, <128 x bfloat> %11, i8 1, i32 0, <128 x bfloat> %12,
  // i1 true, i1 false)
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, FSub, Sw11, Sw2, Lookup1, Predicate, Polarity});

  // %16 = tail call <128 x bfloat> @llvm.tpc.mul
  // (<128 x bfloat> %15, <128 x bfloat> %4, i8 1, i32 0, <128 x bfloat> %15,
  // <256 x i1> %6, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, Char256Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mul, FType), FType)
                         .getCallee());
  auto Mul1 = Builder.CreateCall(
      Intrinsic, {Mac2, Fabs, Sw11, Sw2, Mac2, CmpLess, Polarity});

  // %17 = tail call <128 x bfloat> @llvm.tpc.sel.grt.
  // <128 x i16> %5, i16 1, <128 x bfloat> <bfloat 0xH3F80..>, 128 x bfloat>
  // %16, i8 7, i32 0, <128 x bfloat> undef, i1 true, i1 false
  Types = {Short128Type, I16Type,       Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,      Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt = Builder.CreateCall(
      Intrinsic, {ExtractExp, ConstantInt::get(I16Type, 1),
                  ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(1.0)), Mul1,
                  ConstantInt::get(I8Type, 7), Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %18 = tail call <128 x bfloat> @llvm.tpc.form.fp.num
  // (<128 x bfloat> %17, <128 x bfloat> %2, <128 x bfloat> %17, i8 1, i32 0,
  // <128 x bfloat> undef, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP = Builder.CreateCall(
      Intrinsic, {SelGrt, Operand, SelGrt, Sw11, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %19 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special.
  // (<128 x bfloat> %3, <128 x bfloat> undef, i8 1, i32 5, <128 x bfloat> %18,
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto CalcFP = Builder.CreateCall(
      Intrinsic,
      {FClass, UndefValue::get(Bfloat128Type), ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 5), FormFP, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(CalcFP);
  InstrToReplace->eraseFromParent();
}
#endif // MERGE

void EvalSpecialFunctionPass::replaceExpWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);
  auto Ty = Operand->getType();

  SmallVector<Type *, 7> TypesMac0{Ty,          Float64Type, I8Type, I32Type,
                                   Float64Type, I1Type,      I1Type};
  FunctionType *FType = FunctionType::get(Ty, TypesMac0, false);
  Function *Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  auto ConstLog2E = ConstantFP::get(F32Type, 1.44269502e0);
  auto ConstResult = ConstantFP::get(F32Type, 5.000000e-01);
  auto Mac0Res = Builder.CreateCall(
      Intrinsic, {Operand, ConstantVector::getSplat(ElementCount::getFixed(64), ConstLog2E),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  ConstantVector::getSplat(ElementCount::getFixed(64), ConstResult),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 6> TypesNearbyInt0{Float64Type, I8Type, I32Type,
                                         Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, TypesNearbyInt0, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_nearbyint, FType), FType)
          .getCallee());
  auto NearbyInt0Res = Builder.CreateCall(
      Intrinsic,
      {Mac0Res, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 196608),
       UndefValue::get(Float64Type), ConstantInt::get(I1Type, 1),
       ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 7> TypesMac1{Float64Type, Float64Type, I8Type, I32Type,
                                   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, TypesMac1, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  auto ConstLn21 = ConstantFP::get(F32Type, 0.693359375);
  auto Mac1Res = Builder.CreateCall(
      Intrinsic,
      {NearbyInt0Res, ConstantVector::getSplat(ElementCount::getFixed(64), ConstLn21),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 2), Operand,
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstLn22 = ConstantFP::get(F32Type, -2.12194440e-4);
  auto Mac2Res = Builder.CreateCall(
      Intrinsic,
      {NearbyInt0Res, ConstantVector::getSplat(ElementCount::getFixed(64), ConstLn22),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 2), Mac1Res,
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstC3 = ConstantFP::get(F32Type, 8.380148765026943e-3);
  auto ConstC4 = ConstantFP::get(F32Type, 4.191878872870153e-2);
  auto Mac3Res = Builder.CreateCall(
      Intrinsic, {Mac2Res, ConstantVector::getSplat(ElementCount::getFixed(64), ConstC3),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  ConstantVector::getSplat(ElementCount::getFixed(64), ConstC4),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstC5 = ConstantFP::get(F32Type, 0.1666634537038239);
  auto Mac4Res = Builder.CreateCall(
      Intrinsic,
      {Mac3Res, Mac2Res, ConstantInt::get(I8Type, 0),
       ConstantInt::get(I32Type, 0), ConstantVector::getSplat(ElementCount::getFixed(64), ConstC5),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstC6 = ConstantFP::get(F32Type, 0.49998858346161135);
  auto Mac5Res = Builder.CreateCall(
      Intrinsic,
      {Mac4Res, Mac2Res, ConstantInt::get(I8Type, 0),
       ConstantInt::get(I32Type, 0), ConstantVector::getSplat(ElementCount::getFixed(64), ConstC6),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstXPlus1 = ConstantFP::get(F32Type, 1.0);
  auto Mac6Res = Builder.CreateCall(
      Intrinsic,
      {Mac5Res, Mac2Res, ConstantInt::get(I8Type, 0),
       ConstantInt::get(I32Type, 0), ConstantVector::getSplat(ElementCount::getFixed(64), ConstXPlus1),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto Mac7Res = Builder.CreateCall(
      Intrinsic,
      {Mac6Res, Mac2Res, ConstantInt::get(I8Type, 0),
       ConstantInt::get(I32Type, 0), ConstantVector::getSplat(ElementCount::getFixed(64), ConstXPlus1),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto BitCast0Res = Builder.CreateBitCast(Mac7Res, Int64Type);

  SmallVector<Type *, 6> TypesConvert0{Float64Type, I8Type, I32Type,
                                       Int64Type,   I1Type, I1Type};
  FType = FunctionType::get(Int64Type, TypesConvert0, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());
  auto Convert0Res = Builder.CreateCall(
      Intrinsic, {NearbyInt0Res, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 197120), UndefValue::get(Int64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  //%11 = tail call <64 x i32> @llvm.tpc.shl.v64i32.i32.i1(<64 x i32> %10, i32
  // 23, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false) #3
  SmallVector<Type *, 6> TypesTpcShl0{Int64Type, I32Type, I8Type, I32Type,
                                      Int64Type, I1Type,  I1Type};
  FType = FunctionType::get(Int64Type, TypesTpcShl0, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_shl, FType),
                                FType)
          .getCallee());
  auto TpcShl0Res = Builder.CreateCall(
      Intrinsic,
      {Convert0Res, ConstantInt::get(I32Type, 23), ConstantInt::get(I8Type, 2),
       ConstantInt::get(I32Type, 0), UndefValue::get(Int64Type),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto Add0Res = Builder.CreateAdd(TpcShl0Res, BitCast0Res);

  auto Bitcast1Res = Builder.CreateBitCast(Add0Res, Float64Type);

  SmallVector<Type *, 9> TypesSelLeq0{Float64Type, Float64Type, Float64Type,
                                      Float64Type, I8Type,      I32Type,
                                      Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, TypesSelLeq0, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_leq, FType), FType)
          .getCallee());
  auto ConstExpLower = ConstantFP::get(F32Type, -87.336);
  auto TpcSelLeq0Res = Builder.CreateCall(
      Intrinsic, {Operand, ConstantVector::getSplat(ElementCount::getFixed(64), ConstExpLower),
                  ConstantAggregateZero::get(Float64Type), Bitcast1Res,
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Float64Type), ConstantInt::get(I1Type, 1),
                  ConstantInt::get(I1Type, 0)});

  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_geq, FType), FType)
          .getCallee());
  auto ConstExpUpper = ConstantFP::get(F32Type, 88.722);
  auto ConstPlusInfFP32 = ConstantFP::getInfinity(F32Type, false);
  auto TpcSelGeq0Res = Builder.CreateCall(
      Intrinsic, {Operand, ConstantVector::getSplat(ElementCount::getFixed(64), ConstExpUpper),
                  ConstantVector::getSplat(ElementCount::getFixed(64), ConstPlusInfFP32), TpcSelLeq0Res,
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Float64Type), ConstantInt::get(I1Type, 1),
                  ConstantInt::get(I1Type, 0)});

  auto BitCast2Res = Builder.CreateBitCast(Operand, Int64Type);

  auto ConstNan = ConstantInt::get(I32Type, 2147483647);
  auto And0Res =
      Builder.CreateAnd(BitCast2Res, ConstantVector::getSplat(ElementCount::getFixed(64), ConstNan));

  SmallVector<Type *, 9> TypesSelGrt0{Int64Type,   I32Type, Float64Type,
                                      Float64Type, I8Type,  I32Type,
                                      Float64Type, I1Type,  I1Type};
  FType = FunctionType::get(Float64Type, TypesSelGrt0, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType), FType)
          .getCallee());
  auto SelGrt0Res = Builder.CreateCall(
      Intrinsic, {And0Res, ConstantInt::get(I32Type, 2139095040), Operand,
                  TpcSelGeq0Res, ConstantInt::get(I8Type, 3),
                  ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  InstrToReplace->replaceAllUsesWith(SelGrt0Res);
  InstrToReplace->eraseFromParent();
}

#if 1 // MERGE
void EvalSpecialFunctionPass::replaceBF16SinWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw1 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // Begin instruction sequence.
  // %3 = bitcast <128 x bfloat16> %2 to <128 x i16>
  auto BitCast = Builder.CreateBitCast(Operand, /*Dest Type*/ Short128Type);

  // %and.i = and <128 x i16> %3, <i16 32767, ...
  auto BitCastAnd = Builder.CreateAnd(
      BitCast, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)));

  // %4 = bitcast <128 x i16> %and.i to <128 x bfloat16>
  auto BitCastToBF16 = Builder.CreateBitCast(BitCastAnd, Bfloat128Type);

  // %5 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1
  // (<128 x bfloat16> %4, i8 1, i32 0, <128 x float> undef, i1 true,
  // i1 false)
  SmallVector<Type *, 6> ConvertTypes{Bfloat128Type, I8Type, I32Type,
                                      Float128Type,  I1Type, I1Type};
  auto FType = FunctionType::get(Float128Type, ConvertTypes, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertToFloat128 = Builder.CreateCall(
      Intrinsic, {BitCastToBF16, Sw1, Sw2, UndefValue::get(Float128Type),
                  Predicate, Polarity});

  // %X = shufflevector <128 x float> %5, <128 x float> undef, <64 x
  // i32>(0...63)
  SmallVector<uint32_t, 64> Vec0, Vec1;
  for (int i = 0; i < 64; i++) {
    Vec0.push_back(i);
    Vec1.push_back(64 + i);
  }

  auto FirstHalf = Builder.CreateShuffleVector(
      ConvertToFloat128, UndefValue::get(Float128Type), Vec0);
  // %mul.i = fmul <64 x float> %X, <float 1.27323949f ....>
  auto FirstMul = Builder.CreateFMul(
      FirstHalf,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.27323949f)));
  // %Y = shufflevector <128 x float> %5, <128 x float> undef, <64 x
  // i32>(64...128)
  auto SecondHalf = Builder.CreateShuffleVector(
      ConvertToFloat128, UndefValue::get(Float128Type), Vec1);
  // %mul15.i = fmul <64 x float> %Y, <float 1.27323949f ....>
  auto SecondMul = Builder.CreateFMul(
      SecondHalf,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.27323949f)));

  //  %6 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float>
  //  %mul.i...
  ConvertTypes = {Float64Type, I8Type, I32Type, Int64Type, I1Type, I1Type};
  FType = FunctionType::get(Int64Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertToInt641 = Builder.CreateCall(
      Intrinsic,
      {FirstMul, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 66048),
       UndefValue::get(Int64Type), Predicate, Polarity});

  //  %7 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float>
  //  %mul.i...
  auto ConvertToInt642 = Builder.CreateCall(
      Intrinsic,
      {SecondMul, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 66048),
       UndefValue::get(Int64Type), Predicate, Polarity});

  // %and17.i = and <64 x i32> %6, <i32 1, i32 1,...
  auto And1 = Builder.CreateAnd(
      ConvertToInt641,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %add.i = add <64 x i32> %and17.i, %6
  auto Add1 = Builder.CreateAdd(And1, ConvertToInt641);

  // %and22.i = and <64 x i32> %7, <i32 1, i32 1,...
  auto And2 = Builder.CreateAnd(
      ConvertToInt642,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %add25.i = add <64 x i32> %and22.i, %7
  auto Add2 = Builder.CreateAdd(And2, ConvertToInt642);

  // %8 = tail call <128 x i16> @llvm.tpc.convert.int.v128i16.v64i32.v256i8.i1
  // <64 x i32> %add.i, <256 x i8> zeroinitializer, i32 720896
  ConvertTypes = {Int64Type, I8256Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_convert_int, FType), FType)
          .getCallee());
  auto ConvertToShort1281 = Builder.CreateCall(
      Intrinsic,
      {Add1, ConstantVector::getSplat(ElementCount::getFixed(256), ConstantInt::get(I8Type, 0)),
       ConstantInt::get(I32Type, 720896),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 0)), Predicate,
       Polarity});

  // %9 = tail call <128 x i16>
  // @llvm.tpc.convert.int.v128i16.v64i32.v256i8.i1(<64 x i32> %add25.i,...
  auto ConvertToShort1282 = Builder.CreateCall(
      Intrinsic,
      {Add2, ConstantVector::getSplat(ElementCount::getFixed(256), ConstantInt::get(I8Type, 0)),
       ConstantInt::get(I32Type, 720897), ConvertToShort1281, Predicate,
       Polarity});

  // %10 = tail call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32>
  // %add.i...
  ConvertTypes = {Int64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConverttoFloat641 = Builder.CreateCall(
      Intrinsic,
      {Add1, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %11 = tail call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32>
  // %add25.i,...
  auto ConverttoFloat642 = Builder.CreateCall(
      Intrinsic,
      {Add2, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float64Type), Predicate, Polarity});

  SmallVector<Type *, 6> MacTypes = {Float64Type, Float64Type, I8Type, I32Type,
                                     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, MacTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());

  // %12 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %10, <64 x float>...%X...
  auto Mac1 = Builder.CreateCall(
      Intrinsic,
      {ConverttoFloat641,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -7.85156250e-01f)),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), FirstHalf,
       Predicate, Polarity});

  // %13 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11,..%Y...
  auto Mac2 = Builder.CreateCall(
      Intrinsic,
      {ConverttoFloat642,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -7.85156250e-01f)),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), SecondHalf,
       Predicate, Polarity});

  // %14 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %10,...Mac1..
  auto Mac3 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat641,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -2.41875648498e-4f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac1, Predicate, Polarity});

  // %15 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11,..Mac2..
  auto Mac4 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat642,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -2.41875648498e-4f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac2, Predicate, Polarity});

  // %16 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %10,..Mac3...
  auto Mac5 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat641,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -3.7748949774e-8f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac3, Predicate, Polarity});

  // %17 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11,...Mac5...
  auto Mac6 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat642,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -3.7748949774e-8f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac4, Predicate, Polarity});

  // %fl.sroa.0.0.vec.expand.i = shufflevector <64 x float> %16, <64 x float>
  // undef, <128 x i32>...
  auto Mask1 = createSequentialMask(0, 64, 64);
  auto ShuffleFloat641 =
      Builder.CreateShuffleVector(Mac5, UndefValue::get(Float64Type), Mask1);

  // %fl.sroa.0.256.vec.expand195.i = shufflevector <64 x float> %17, <64 x
  // float> undef, <128 x i32> <i32 undef,...
  SmallVector<Constant *, 16> Mask2;
  Constant *Undef = UndefValue::get(Builder.getInt32Ty());
  for (unsigned i = 0; i < 64; i++)
    Mask2.push_back(Undef);
  for (unsigned i = 0; i < 64; i++)
    Mask2.push_back(Builder.getInt32(i));

  auto ShuffleFloat642 = Builder.CreateShuffleVector(
      Mac6, UndefValue::get(Float64Type), ConstantVector::get(Mask2));

  // %fl.sroa.0.256.vecblend196.i = shufflevector <128 x float>
  // %fl.sroa.0.0.vec.expand.i, <128 x float> %fl.sroa.0.256.vec.expand195.i,
  // <128 x i32>
  SmallVector<Constant *, 16> Mask3;
  for (unsigned i = 0; i < 64; i++)
    Mask3.push_back(Builder.getInt32(i));
  for (unsigned i = 192; i < 256; i++)
    Mask3.push_back(Builder.getInt32(i));

  auto ShuffleFloat128 = Builder.CreateShuffleVector(
      ShuffleFloat641, ShuffleFloat642, ConstantVector::get(Mask3));

  // %18 = tail call <128 x bfloat16> @llvm.tpc.convert.v128bf16.v128f32.i1(<128
  // x float> %fl.sroa.0.256.vecblend196.i,...
  ConvertTypes = {Float128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertBfloat128 = Builder.CreateCall(
      Intrinsic, {ShuffleFloat128, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 393472),
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %19 = tail call <256 x i1> @llvm.tpc.cmp.grt.v256i1.v128bf16.bf16.i1(<128 x
  // bfloat16> %18, bfloat16 0xR0000,...
  ConvertTypes = {Bfloat128Type, BF16Type, I8Type, I32Type,
                  Char256Type,   I1Type,   I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt = Builder.CreateCall(
      Intrinsic, {ConvertBfloat128, Constant::getNullValue(BF16Type),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %20 = bitcast <128 x bfloat16> %18 to <128 x i16>
  auto BitCastShort128 = Builder.CreateBitCast(ConvertBfloat128, Short128Type);

  // %and75.i = and <128 x i16> %20, <i16 32767,...
  auto AndShort128 = Builder.CreateAnd(
      BitCastShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)));

  //  %21 = bitcast <128 x i16> %and75.i to <128 x bfloat16>
  auto BitCastBfloat128 = Builder.CreateBitCast(AndShort128, Bfloat128Type);

  // %22 = tail call <128 x i16> @llvm.tpc.shr.v128i16.i16.i1(<128 x i16> %3,...
  ConvertTypes = {Short128Type, I16Type, I8Type, I32Type,
                  Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_shr, FType), FType)
                         .getCallee());
  auto SHRShort128 = Builder.CreateCall(
      Intrinsic, {BitCast, ConstantInt::get(I16Type, 15),
                  ConstantInt::get(I8Type, 8), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %23 = lshr <128 x i16> %9, <i16 1, i16 1,...
  auto LSHRShort128 = Builder.CreateLShr(
      ConvertToShort1282,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 1)));

  //  %and76.i = and <128 x i16> %23, <i16 3, i16 3...
  auto And76i = Builder.CreateAnd(
      LSHRShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 3)));

  // %and77.i = and <128 x i16> %23, <i16 2, i16 2,...
  auto And77i = Builder.CreateAnd(
      LSHRShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 2)));

  // %24 = lshr exact <128 x i16> %and77.i, <i16 1, i16 1,...
  auto LSHRShort1281 = Builder.CreateLShr(
      And77i, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 1)), "",
      true);

  // %xor.i = xor <128 x i16> %22, %24
  auto Xori = Builder.CreateXor(SHRShort128, LSHRShort1281);

  // %sub.i = sub nsw <128 x i16> %and76.i, %and77.i
  auto Subi = Builder.CreateNSWSub(And76i, And77i);

  // %25 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v128i16.i16.i1(<128 x
  // i16> %sub.i,...
  ConvertTypes = {Short128Type, I16Type, I8Type, I32Type,
                  Char256Type,  I1Type,  I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType),
                            FType)
          .getCallee());
  auto CmpEqShort128 = Builder.CreateCall(
      Intrinsic, {Subi, ConstantInt::get(I16Type, 1),
                  ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %26 = or <256 x i1> %25, %19
  auto OrShort1281 = Builder.CreateOr(CmpEqShort128, SelGrt);

  // %27 = tail call <128 x i16> @llvm.tpc.xor.v128i16.v128i16.i16.v256i1(<128 x
  // i16> %xor.i,...
  ConvertTypes = {Short128Type, I16Type,     I8Type, I32Type,
                  Short128Type, Char256Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_xor, FType), FType)
                         .getCallee());
  auto Xor27 = Builder.CreateCall(
      Intrinsic,
      {Xori, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 7),
       ConstantInt::get(I32Type, 0), Xori, OrShort1281, Predicate});

  // %28 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v128i16.i16.i1(<128 x
  // i16> %27,
  ConvertTypes = {Short128Type, I16Type, I8Type, I32Type,
                  Char256Type,  I1Type,  I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType),
                            FType)
          .getCallee());
  auto CmpEqShort1281 = Builder.CreateCall(
      Intrinsic, {Xor27, ConstantInt::get(I16Type, 1),
                  ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %29 = tail call <256 x i16>
  // @llvm.tpc.get.lut.entry.v256i16.v128bf16.i1(<128 x bfloat16> %21,...
  ConvertTypes = {Bfloat128Type, I8Type, I8Type, I32Type,
                  Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {BitCastBfloat128, ConstantInt::get(I8Type, 3),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 24576),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %30 = shufflevector <256 x i16> %29, <256 x i16> undef, <128 x i32> <i32
  // 0,...127
  auto Mask4 = createSequentialMask(0, 128, 0);
  auto ShuffleShort2561 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask4);

  // %31 = shufflevector <256 x i16> %29, <256 x i16> undef, <128 x i32> <i32
  // 128, i32 129...255
  Mask4 = createSequentialMask(128, 128, 0);
  auto ShuffleShort2562 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask4);

  // %32 = bitcast <128 x i16> %31 to <128 x bfloat16>
  auto BitCastBfloat1281 =
      Builder.CreateBitCast(ShuffleShort2562, Bfloat128Type);

  // %shl.i = shl nsw <128 x i16> %sub.i, <i16 4, i16 4...
  auto Shli = Builder.CreateShl(
      Subi, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 4)));

  // %33 = or <128 x i16> %30, %shl.i
  auto OrShort1282 = Builder.CreateOr(ShuffleShort2561, Shli);

  // %34 = fsub <128 x bfloat16> %21, %32
  auto FSubBfloat1281 = Builder.CreateFSub(BitCastBfloat128, BitCastBfloat1281);

  // %35 = tail call <256 x bfloat16> @llvm.tpc.lookup.2c.v256bf16.v128i16(<128
  // x i16> %33,...
  ConvertTypes = {Short128Type,  I32Type, I32Type,
                  Bfloat256Type, I1Type,  I1Type};
  FType = FunctionType::get(Bfloat256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto LookupBfloat2561 = Builder.CreateCall(
      Intrinsic, {OrShort1282, ConstantInt::get(I32Type, 429),
                  ConstantInt::get(I32Type, 1),
                  Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C0C1.sroa.0.256.vec.extract.i = shufflevector <256 x bfloat16> %35,...
  Mask4 = createSequentialMask(128, 128, 0);
  auto ShuffleBfloat2561 = Builder.CreateShuffleVector(
      LookupBfloat2561, UndefValue::get(Bfloat256Type), Mask4);

  // %C0C1.sroa.0.0.vec.extract.i = shufflevector <256 x bfloat16> %35, <256 x
  // bfloat16> undef, <128 x i32> <i32 0,...
  Mask4 = createSequentialMask(0, 128, 0);
  auto ShuffleBfloat2562 = Builder.CreateShuffleVector(
      LookupBfloat2561, UndefValue::get(Bfloat256Type), Mask4);

  // %36 = tail call <128 x bfloat16> @llvm.tpc.mac.v128bf16.v128bf16.i1(<128 x
  // bfloat16> %C0C1.sroa.0.256.vec.extract.i, <128 x bfloat16> %34, i8 1, i32
  // 0, <128 x bfloat16> %C0C1.sroa.0.0.vec.extract.i,
  ConvertTypes = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                  Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto MacBfloat1281 = Builder.CreateCall(
      Intrinsic,
      {ShuffleBfloat2561, FSubBfloat1281, ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 0), ShuffleBfloat2562, Predicate, Polarity});

  // %37 = tail call <128 x bfloat16>
  // @llvm.tpc.mul.v128bf16.v128bf16.v128bf16.v256i1(<128 x bfloat16> %21, <128
  // x bfloat16> %36, i8 1, i32 0, <128 x bfloat16> %36, <256 x i1> %25, i1
  // true)
  ConvertTypes = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                  Bfloat128Type, Char256Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mul, FType), FType)
                         .getCallee());
  auto MacBfloat1282 = Builder.CreateCall(
      Intrinsic, {BitCastBfloat128, MacBfloat1281, ConstantInt::get(I8Type, 1),
                  ConstantInt::get(I32Type, 0), MacBfloat1281, CmpEqShort128,
                  /*Inverted predicate*/ Predicate});

  // %38 = bitcast <128 x bfloat16> %37 to <128 x i16>
  auto BitCastShort1281 = Builder.CreateBitCast(MacBfloat1282, Short128Type);

  // %39 = tail call <128 x i16> @llvm.tpc.xor.v128i16.v128i16.i16.v256i1(<128 x
  // i16> %38, i16 -32768...
  //  <256 x i1> %28, i1 false
  ConvertTypes = {Short128Type, I16Type,     I8Type, I32Type,
                  Short128Type, Char256Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_xor, FType), FType)
                         .getCallee());
  auto XorShort1281 = Builder.CreateCall(
      Intrinsic, {BitCastShort1281, ConstantInt::get(I16Type, -32768),
                  ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 0),
                  BitCastShort1281, CmpEqShort1281, Polarity});

  // %40 = tail call <128 x i16> @llvm.tpc.sel.grt..(<128 x i16> %and.i, i16
  // 17920, <128 x i16> <i16 32767..>) <128 x i16> %39, i8 8, i32 0, <128 x i16>
  // undef, i1 true, i1 false)
  ConvertTypes = {Short128Type, I16Type, Short128Type,
                  Short128Type, I8Type,  I32Type,
                  Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt1 = Builder.CreateCall(
      Intrinsic,
      {BitCastAnd, ConstantInt::get(I16Type, 17920),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)),
       XorShort1281, ConstantInt::get(I8Type, 8), ConstantInt::get(I32Type, 0),
       UndefValue::get(Short128Type), Predicate, Polarity});

  // %41 = bitcast <128 x i16> %40 to <128 x bfloat16>
  auto BitCastBfloat1282 = Builder.CreateBitCast(SelGrt1, Bfloat128Type);

  InstrToReplace->replaceAllUsesWith(BitCastBfloat1282);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceBF16CosWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {
  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw1 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // Begin instruction sequence.
  // %4 = bitcast <128 x bfloat16> %3 to <128 x i16>
  auto BitCast = Builder.CreateBitCast(Operand, /*Dest Type*/ Short128Type);

  // %and.i = and <128 x i16> %4, <i16 32767, ...
  auto BitCastAnd = Builder.CreateAnd(
      BitCast, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)));

  // %5 = bitcast <128 x i16> %and.i to <128 x bfloat16>
  auto BitCastToBF16 = Builder.CreateBitCast(BitCastAnd, Bfloat128Type);

  // %6 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1
  // (<128 x bfloat16> %5, i8 1, i32 0, <128 x float> undef, i1 true,
  // i1 false)
  SmallVector<Type *, 6> ConvertTypes{Bfloat128Type, I8Type, I32Type,
                                      Float128Type,  I1Type, I1Type};
  auto FType = FunctionType::get(Float128Type, ConvertTypes, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertToFloat128 = Builder.CreateCall(
      Intrinsic, {BitCastToBF16, Sw1, Sw2, UndefValue::get(Float128Type),
                  Predicate, Polarity});

  // %X = shufflevector <128 x float> %6, <128 x float> undef, <64 x
  // i32>(0...63)
  SmallVector<uint32_t, 64> Vec0, Vec1;
  for (int i = 0; i < 64; i++) {
    Vec0.push_back(i);
    Vec1.push_back(64 + i);
  }

  auto FirstHalf = Builder.CreateShuffleVector(
      ConvertToFloat128, UndefValue::get(Float128Type), Vec0);
  // %mul.i = fmul <64 x float> %X, <float 1.27323949f ....>
  auto FirstMul = Builder.CreateFMul(
      FirstHalf,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.27323949f)));
  // %Y = shufflevector <128 x float> %5, <128 x float> undef, <64 x
  // i32>(64...128)
  auto SecondHalf = Builder.CreateShuffleVector(
      ConvertToFloat128, UndefValue::get(Float128Type), Vec1);
  // %mul15.i = fmul <64 x float> %Y, <float 1.27323949f ....>
  auto SecondMul = Builder.CreateFMul(
      SecondHalf,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.27323949f)));

  //  %7 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float>
  //  %mul.i...
  ConvertTypes = {Float64Type, I8Type, I32Type, Int64Type, I1Type, I1Type};
  FType = FunctionType::get(Int64Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertToInt641 = Builder.CreateCall(
      Intrinsic,
      {FirstMul, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 66112),
       UndefValue::get(Int64Type), Predicate, Polarity});

  //  %8 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float>
  //  %mul.i...
  auto ConvertToInt642 = Builder.CreateCall(
      Intrinsic,
      {SecondMul, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 66112),
       UndefValue::get(Int64Type), Predicate, Polarity});

  // %and17.i = and <64 x i32> %7, <i32 1, i32 1,...
  auto And1 = Builder.CreateAnd(
      ConvertToInt641,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %add.i = add <64 x i32> %and17.i, %7
  auto Add1 = Builder.CreateAdd(And1, ConvertToInt641);

  // %and22.i = and <64 x i32> %8, <i32 1, i32 1,...
  auto And2 = Builder.CreateAnd(
      ConvertToInt642,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %add25.i = add <64 x i32> %and22.i, %8
  auto Add2 = Builder.CreateAdd(And2, ConvertToInt642);

  // %9 = tail call <128 x i16> @llvm.tpc.convert.int.v128i16.v64i32.v256i8.i1
  // <64 x i32> %add.i, <256 x i8> zeroinitializer, i32 720896
  ConvertTypes = {Int64Type, I8256Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_convert_int, FType), FType)
          .getCallee());
  auto ConvertToShort1281 = Builder.CreateCall(
      Intrinsic,
      {Add1, ConstantVector::getSplat(ElementCount::getFixed(256), ConstantInt::get(I8Type, 0)),
       ConstantInt::get(I32Type, 720900),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 0)), Predicate,
       Polarity});

  // %10 = tail call <128 x i16>
  // @llvm.tpc.convert.int.v128i16.v64i32.v256i8.i1(<64 x i32> %add25.i,...
  auto ConvertToShort1282 = Builder.CreateCall(
      Intrinsic,
      {Add2, ConstantVector::getSplat(ElementCount::getFixed(256), ConstantInt::get(I8Type, 0)),
       ConstantInt::get(I32Type, 720901), ConvertToShort1281, Predicate,
       Polarity});

  // %11 = tail call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32>
  // %add.i...
  ConvertTypes = {Int64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConverttoFloat641 = Builder.CreateCall(
      Intrinsic,
      {Add1, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 64),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %12 = tail call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32>
  // %add25.i,...
  auto ConverttoFloat642 = Builder.CreateCall(
      Intrinsic,
      {Add2, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 64),
       UndefValue::get(Float64Type), Predicate, Polarity});

  SmallVector<Type *, 6> MacTypes = {Float64Type, Float64Type, I8Type, I32Type,
                                     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, MacTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());

  // %13 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11, <64 x float>...%X...
  auto Mac1 = Builder.CreateCall(
      Intrinsic,
      {ConverttoFloat641,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -7.85156250e-01f)),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), FirstHalf,
       Predicate, Polarity});

  // %14 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %12,..%Y...
  auto Mac2 = Builder.CreateCall(
      Intrinsic,
      {ConverttoFloat642,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -7.85156250e-01f)),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), SecondHalf,
       Predicate, Polarity});

  // %15 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11,...Mac1..
  auto Mac3 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat641,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -2.41875648498e-4f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac1, Predicate, Polarity});

  // %16 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %12,..Mac2..
  auto Mac4 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat642,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -2.41875648498e-4f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac2, Predicate, Polarity});

  // %17 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %11,..Mac3...
  auto Mac5 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat641,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -3.7748949774e-8f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac3, Predicate, Polarity});

  // %18 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>
  // %12,...Mac5...
  auto Mac6 = Builder.CreateCall(
      Intrinsic, {ConverttoFloat642,
                  ConstantVector::getSplat(
                      ElementCount::getFixed(64), ConstantFP::get(F32Type, -3.7748949774e-8f)),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
                  Mac4, Predicate, Polarity});

  // %fl.sroa.0.0.vec.expand.i = shufflevector <64 x float> %16, <64 x float>
  // undef, <128 x i32>...
  auto Mask1 = createSequentialMask(0, 64, 64);
  auto ShuffleFloat641 =
      Builder.CreateShuffleVector(Mac5, UndefValue::get(Float64Type), Mask1);

  // %fl.sroa.0.256.vec.expand195.i = shufflevector <64 x float> %17, <64 x
  // float> undef, <128 x i32> <i32 undef,...
  SmallVector<Constant *, 16> Mask2;
  Constant *Undef = UndefValue::get(Builder.getInt32Ty());
  for (unsigned i = 0; i < 64; i++)
    Mask2.push_back(Undef);
  for (unsigned i = 0; i < 64; i++)
    Mask2.push_back(Builder.getInt32(i));

  auto ShuffleFloat642 = Builder.CreateShuffleVector(
      Mac6, UndefValue::get(Float64Type), ConstantVector::get(Mask2));

  // %fl.sroa.0.256.vecblend196.i = shufflevector <128 x float>
  // %fl.sroa.0.0.vec.expand.i, <128 x float> %fl.sroa.0.256.vec.expand195.i,
  // <128 x i32>
  SmallVector<Constant *, 16> Mask3;
  for (unsigned i = 0; i < 64; i++)
    Mask3.push_back(Builder.getInt32(i));
  for (unsigned i = 192; i < 256; i++)
    Mask3.push_back(Builder.getInt32(i));

  auto ShuffleFloat128 = Builder.CreateShuffleVector(
      ShuffleFloat641, ShuffleFloat642, ConstantVector::get(Mask3));

  // %19 = tail call <128 x bfloat16> @llvm.tpc.convert.v128bf16.v128f32.i1(<128
  // x float> %fl.sroa.0.256.vecblend196.i,...
  ConvertTypes = {Float128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto ConvertBfloat128 = Builder.CreateCall(
      Intrinsic, {ShuffleFloat128, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 393472),
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %20 = tail call <256 x i1> @llvm.tpc.cmp.grt.v256i1.v128bf16.bf16.i1(<128 x
  // bfloat16> %19, bfloat16 0xR0000,...
  ConvertTypes = {Bfloat128Type, BF16Type, I8Type, I32Type,
                  Char256Type,   I1Type,   I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_less, FType),
                            FType)
          .getCallee());
  auto SelGrt = Builder.CreateCall(
      Intrinsic, {ConvertBfloat128, Constant::getNullValue(BF16Type),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %21 = bitcast <128 x bfloat16> %19 to <128 x i16>
  auto BitCastShort128 = Builder.CreateBitCast(ConvertBfloat128, Short128Type);

  // %and75.i = and <128 x i16> %21, <i16 32767,...
  auto AndShort128 = Builder.CreateAnd(
      BitCastShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)));

  // %22 = bitcast <128 x i16> %and75.i to <128 x bfloat16>
  auto BitCastBfloat128 = Builder.CreateBitCast(AndShort128, Bfloat128Type);

  // %23 = lshr <128 x i16> %10, <i16 1, i16 1,...
  auto LSHRShort128 = Builder.CreateLShr(
      ConvertToShort1282,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 1)));

  //  %and76.i = and <128 x i16> %23, <i16 3, i16 3...
  auto And76i = Builder.CreateAnd(
      LSHRShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 3)));

  // %and77.i = and <128 x i16> %23, <i16 2, i16 2,...
  auto And77i = Builder.CreateAnd(
      LSHRShort128,
      ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 2)));

  // %24 = lshr exact <128 x i16> %and77.i, <i16 1, i16 1,...
  auto LSHRShort1281 = Builder.CreateLShr(
      And77i, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 1)), "",
      true);

  // %Mov = tail call <128 x i16> @llvm.tpc.mov.v128i16.i16.i1(i16 0, i8 7, i32
  // 0,
  SmallVector<Type *, 6> Types{I16Type,      I8Type, I32Type,
                               Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mov, FType), FType)
                         .getCallee());
  auto Mov = Builder.CreateCall(
      Intrinsic, {ConstantInt::get(I16Type, 0), ConstantInt::get(I8Type, 7),
                  Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %xor.i = xor <128 x i16> Zero, %24
  auto Xori = Builder.CreateXor(Mov, LSHRShort1281);

  // %sub.i = sub nsw <128 x i16> %and76.i, %and77.i
  auto Subi = Builder.CreateNSWSub(And76i, And77i);

  // %25 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v128i16.i16.i1(<128 x
  // i16> %sub.i,...
  ConvertTypes = {Short128Type, I16Type, I8Type, I32Type,
                  Char256Type,  I1Type,  I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType),
                            FType)
          .getCallee());
  auto CmpEqShort128 = Builder.CreateCall(
      Intrinsic, {Subi, ConstantInt::get(I16Type, 0),
                  ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %26 = or <256 x i1> %25, %20
  auto OrShort1281 = Builder.CreateOr(CmpEqShort128, SelGrt);

  // %27 = tail call <128 x i16> @llvm.tpc.xor.v128i16.v128i16.i16.v256i1(<128 x
  // i16> %xor.i,...
  ConvertTypes = {Short128Type, I16Type,     I8Type, I32Type,
                  Short128Type, Char256Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_xor, FType), FType)
                         .getCallee());
  auto Xor27 = Builder.CreateCall(
      Intrinsic,
      {Xori, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 7),
       ConstantInt::get(I32Type, 0), Xori, OrShort1281, Predicate});

  // %28 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v128i16.i16.i1(<128 x
  // i16> %27,
  ConvertTypes = {Short128Type, I16Type, I8Type, I32Type,
                  Char256Type,  I1Type,  I1Type};
  FType = FunctionType::get(Char256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType),
                            FType)
          .getCallee());
  auto CmpEqShort1281 = Builder.CreateCall(
      Intrinsic, {Xor27, ConstantInt::get(I16Type, 1),
                  ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Char256Type), Predicate, Polarity});

  // %29 = tail call <256 x i16>
  // @llvm.tpc.get.lut.entry.v256i16.v128bf16.i1(<128 x bfloat16> %22,...
  ConvertTypes = {Bfloat128Type, I8Type, I8Type, I32Type,
                  Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {BitCastBfloat128, ConstantInt::get(I8Type, 3),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 24576),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %30 = shufflevector <256 x i16> %29, <256 x i16> undef, <128 x i32> <i32
  // 0,...127
  auto Mask4 = createSequentialMask(0, 128, 0);
  auto ShuffleShort2561 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask4);

  // %31 = shufflevector <256 x i16> %29, <256 x i16> undef, <128 x i32> <i32
  // 128, i32 129...255
  Mask4 = createSequentialMask(128, 128, 0);
  auto ShuffleShort2562 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask4);

  // %32 = bitcast <128 x i16> %31 to <128 x bfloat16>
  auto BitCastBfloat1281 =
      Builder.CreateBitCast(ShuffleShort2562, Bfloat128Type);

  // %shl.i = shl nsw <128 x i16> %sub.i, <i16 4, i16 4...
  auto Shli = Builder.CreateShl(
      Subi, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 4)));

  // %33 =  <128 x i16> @llvm.tpc.or..
  // (<128 x i16> ShuffleShort2561, <128 x i16> %shl.i, i8 8, i32 0,
  // <128 x i16> ShuffleShort2561, <256 x i1> CmpEqShort128, i1 false)
  ConvertTypes = {Short128Type, Short128Type, I8Type, I32Type,
                  Short128Type, Char256Type,  I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_or, FType), FType)
                         .getCallee());
  auto OrShort1282 = Builder.CreateCall(
      Intrinsic, {ShuffleShort2561, Shli, ConstantInt::get(I8Type, 8),
                  ConstantInt::get(I32Type, 0), ShuffleShort2561, CmpEqShort128,
                  Polarity});

  // %34 = tail call <128 x i16> @llvm.tpc.add...()
  // <128 x i16> %33, i16 16, i8 8, i32 0, <128 x i16> %33, <256 x i1> %25, i1
  // false
  ConvertTypes = {Short128Type, I16Type,     I8Type, I32Type,
                  Short128Type, Char256Type, I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_add, FType), FType)
                         .getCallee());
  auto Add34 = Builder.CreateCall(
      Intrinsic,
      {OrShort1282, ConstantInt::get(I16Type, 16), ConstantInt::get(I8Type, 8),
       ConstantInt::get(I32Type, 0), OrShort1282, CmpEqShort128, Polarity});

  // %35 = fsub <128 x bfloat16> %22, %32
  auto FSubBfloat1281 = Builder.CreateFSub(BitCastBfloat128, BitCastBfloat1281);

  // %36 = tail call <128 x bfloat> @llvm.tpc.lookup.1c
  // <128 x i16> %34, i32 435, i32 1, <128 x bfloat> zeroinitializer, i1 true,
  // i1 false
  ConvertTypes = {Short128Type,  I32Type, I32Type,
                  Bfloat128Type, I1Type,  I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto LookupBfloat1281 = Builder.CreateCall(
      Intrinsic,
      {Add34, ConstantInt::get(I32Type, 435), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat128Type), Predicate, Polarity});

  // %37 = tail call <256 x bfloat16> @llvm.tpc.lookup.2c.v256bf16.v128i16(<128
  // x i16> %34,...
  ConvertTypes = {Short128Type,  I32Type, I32Type,
                  Bfloat256Type, I1Type,  I1Type};
  FType = FunctionType::get(Bfloat256Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto LookupBfloat2561 = Builder.CreateCall(
      Intrinsic,
      {Add34, ConstantInt::get(I32Type, 434), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C0C1.sroa.0.256.vec.extract.i = shufflevector <256 x bfloat16> %37,...
  Mask4 = createSequentialMask(128, 128, 0);
  auto ShuffleBfloat2561 = Builder.CreateShuffleVector(
      LookupBfloat2561, UndefValue::get(Bfloat256Type), Mask4);

  // %C0C1.sroa.0.0.vec.extract.i = shufflevector <256 x bfloat16> %37, <256 x
  // bfloat16> undef, <128 x i32> <i32 0,...
  Mask4 = createSequentialMask(0, 128, 0);
  auto ShuffleBfloat2562 = Builder.CreateShuffleVector(
      LookupBfloat2561, UndefValue::get(Bfloat256Type), Mask4);

  // %38 = tail call <128 x bfloat16> @llvm.tpc.mac.v128bf16.v128bf16.i1(<128 x
  // bfloat16> %C0C1.sroa.0.256.vec.extract.i, <128 x bfloat16> %35, i8 1, i32
  // 0, <128 x bfloat16> %C0C1.sroa.0.0.vec.extract.i,
  ConvertTypes = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                  Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto MacBfloat1281 = Builder.CreateCall(
      Intrinsic,
      {ShuffleBfloat2561, FSubBfloat1281, ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 0), ShuffleBfloat2562, Predicate, Polarity});

  // %39 = tail call <128 x bfloat> @llvm.tpc.mac...
  // <128 x bfloat> %38, <128 x bfloat> %35, i8 1, i32 0, <128 x bfloat> %36
  ConvertTypes = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                  Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto MacBfloat1282 = Builder.CreateCall(
      Intrinsic,
      {MacBfloat1281, FSubBfloat1281, ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 0), LookupBfloat1281, Predicate, Polarity});

  // %40 = tail call <128 x bfloat16>
  // @llvm.tpc.mul.v128bf16.v128bf16.v128bf16.v256i1(<128 x bfloat16> %22, <128
  // x bfloat16> %39, i8 1, i32 0, <128 x bfloat16> %39, <256 x i1> %25, i1
  // true)
  ConvertTypes = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                  Bfloat128Type, Char256Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mul, FType), FType)
                         .getCallee());
  auto MulBfloat1282 = Builder.CreateCall(
      Intrinsic, {BitCastBfloat128, MacBfloat1282, ConstantInt::get(I8Type, 1),
                  ConstantInt::get(I32Type, 0), MacBfloat1282, CmpEqShort128,
                  /*Inverted pred*/ Predicate});

  // %41 = tail call <128 x bfloat> @llvm.tpc.sub...
  // <128 x bfloat> %40, bfloat 0xH0000, i8 1, i32 2, <128 x bfloat> %40, <256 x
  // i1> %28, i1 false
  ConvertTypes = {Bfloat128Type, BF16Type,    I8Type, I32Type,
                  Bfloat128Type, Char256Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, ConvertTypes, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_sub, FType), FType)
                         .getCallee());
  auto Sub41 = Builder.CreateCall(
      Intrinsic,
      {MulBfloat1282, getBfloatValue(0.0), ConstantInt::get(I8Type, 1),
       ConstantInt::get(I32Type, 2), MulBfloat1282, CmpEqShort1281, Polarity});

  // %42 = bitcast <128 x bfloat16> %41 to <128 x i16>
  auto BitCastShort1281 = Builder.CreateBitCast(Sub41, Short128Type);

  // %43 = tail call <128 x i16> @llvm.tpc.sel.grt..(<128 x i16> %and.i, i16
  // 17920, <128 x i16> <i16 32767..>) <128 x i16> %42, i8 8, i32 0, <128 x i16>
  // undef, i1 true, i1 false)
  ConvertTypes = {Short128Type, I16Type, Short128Type,
                  Short128Type, I8Type,  I32Type,
                  Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, ConvertTypes, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt1 = Builder.CreateCall(
      Intrinsic,
      {BitCastAnd, ConstantInt::get(I16Type, 17920),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 32767)),
       BitCastShort1281, ConstantInt::get(I8Type, 8),
       ConstantInt::get(I32Type, 0), UndefValue::get(Short128Type), Predicate,
       Polarity});

  // %44 = bitcast <128 x i16> %43 to <128 x bfloat16>
  auto BitCastBfloat1282 = Builder.CreateBitCast(SelGrt1, Bfloat128Type);

  InstrToReplace->replaceAllUsesWith(BitCastBfloat1282);
  InstrToReplace->eraseFromParent();
}
#endif // MERGE

void EvalSpecialFunctionPass::replaceLogWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw10 = ConstantInt::get(I8Type, 0);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <64 x i32> @llvm.tpc.extract.exp...<64 x float> %2, i8 0,
  // i32 0..
  SmallVector<Type *, 6> Types{Float64Type, I8Type, I32Type,
                               Int64Type,   I1Type, I1Type};
  auto FType = FunctionType::get(Int64Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Int64Type), Predicate, Polarity});

  // add.i.i = add <64 x i32> %3, <i32 1, i32 1...
  auto Addii = Builder.CreateAdd(
      ExtractExp, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %4 = tail call <64 x float> @llvm.tpc.form.fp.num...(<256 x i8> {126...},
  // <64 x float> %2, <64 x float> %2, i8 0, i32 2048)
  Types = {I8256Type, Float64Type, Float64Type, I8Type,
           I32Type,   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP = Builder.CreateCall(
      Intrinsic, {ConstantVector::getSplat(ElementCount::getFixed(256), ConstantInt::get(I8Type, 126)),
                  Operand, Operand, Sw10, ConstantInt::get(I32Type, 2048),
                  UndefValue::get(Float64Type), Predicate, Polarity});

  // %sub.i.i = fadd <64 x float> %4, <float -0.70710677
  auto Subii = Builder.CreateFAdd(
      FormFP,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -0.70710677)));

  // %5 = bitcast <64 x float> %sub.i.i to <64 x i32>
  auto BitCast1 = Builder.CreateBitCast(Subii, Int64Type);

  // %6 = tail call <64 x i32> @llvm.tpc.shr...<64 x i32> %5, i32 31, i8 2, i32
  // 0
  Types = {Int64Type, I32Type, I8Type, I32Type, Int64Type, I1Type, I1Type};
  FType = FunctionType::get(Int64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_shr, FType), FType)
                         .getCallee());
  auto Shr1 = Builder.CreateCall(
      Intrinsic, {BitCast1, ConstantInt::get(I32Type, 31),
                  ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Int64Type), Predicate, Polarity});

  // %sub1.i.i = sub <64 x i32> %add.i.i, %6
  auto Sub1ii = Builder.CreateSub(Addii, Shr1);

  // %7 = tail call <64 x float> @llvm.tpc.convert.v64f32...<64 x i32> %6, i8 2,
  // i32 64
  Types = {Int64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto Convert1 = Builder.CreateCall(
      Intrinsic,
      {Shr1, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 64),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %mul.i.i = fmul <64 x float> %4, %7
  auto Mulii = Builder.CreateFMul(FormFP, Convert1);

  // %sub2.i.i = fadd <64 x float> %mul.i.i, <float -1...
  auto Sub2ii = Builder.CreateFAdd(
      Mulii, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -1)));

  // %add3.i.i = fadd <64 x float> %4, %sub2.i.i
  auto Add3ii = Builder.CreateFAdd(FormFP, Sub2ii);

  // %8 = tail call <64 x float> @llvm.tpc.mac...<64 x float> <float >,
  // <64 x float> %add3.i.i, i8 0, i32 0, <64 x float> <float >
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic,
      {ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 7.0376836292e-2)),
       Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -1.1514610310e-1)),
       Predicate, Polarity});

  auto Mac2 = Builder.CreateCall(
      Intrinsic,
      {Mac1, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.1676998740E-1)),
       Predicate, Polarity});

  auto Mac3 = Builder.CreateCall(
      Intrinsic,
      {Mac2, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -1.2420140846E-1)),
       Predicate, Polarity});

  auto Mac4 = Builder.CreateCall(
      Intrinsic,
      {Mac3, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 1.4249322787E-1)),
       Predicate, Polarity});

  auto Mac5 = Builder.CreateCall(
      Intrinsic,
      {Mac4, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -1.6668057665E-1)),
       Predicate, Polarity});

  auto Mac6 = Builder.CreateCall(
      Intrinsic,
      {Mac5, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 2.0000714765E-1)),
       Predicate, Polarity});

  auto Mac7 = Builder.CreateCall(
      Intrinsic,
      {Mac6, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, -2.4999993993E-1)),
       Predicate, Polarity});

  auto Mac8 = Builder.CreateCall(
      Intrinsic,
      {Mac7, Add3ii, Sw10, Sw2,
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 3.3333331174E-1)),
       Predicate, Polarity});

  // %16 = tail call <64 x float> @llvm.tpc.convert...<64 x i32> %sub1.i.i, i8
  // 2, i32 64
  Types = {Int64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto Convert2 = Builder.CreateCall(
      Intrinsic,
      {Sub1ii, ConstantInt::get(I8Type, 2), ConstantInt::get(I32Type, 64),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %mul4.i.i = fmul <64 x float> %add3.i.i, %add3.i.i
  auto Mul4ii = Builder.CreateFMul(Add3ii, Add3ii);

  // %mul8.i.i = fmul <64 x float> %mul4.i.i, %15
  auto Mul8ii = Builder.CreateFMul(Mul4ii, Mac8);

  // %mul9.i.i = fmul <64 x float> %add3.i.i, %mul8.i.i
  auto Mul9ii = Builder.CreateFMul(Add3ii, Mul8ii);

  // %mul10.i.i = fmul <64 x float> %mul4.i.i, <float 5.000000
  auto Mul10ii = Builder.CreateFMul(
      Mul4ii,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 5.000000e-01)));

  // %sub11.i.i = fsub <64 x float> %mul9.i.i, %mul10.i.i
  auto Sub11ii = Builder.CreateFSub(Mul9ii, Mul10ii);

  // %mul12.i.i = fmul <64 x float> %sub11.i.i, <float
  // 0.44269504088896340735992..
  auto Mul12ii = Builder.CreateFMul(
      Sub11ii, ConstantVector::getSplat(
        ElementCount::getFixed(64), ConstantFP::get(F32Type, 0.44269504088896340735992)));

  // %mul13.i.i = fmul <64 x float> %add3.i.i, <float
  // 0.44269504088896340735992..
  auto Mul13ii = Builder.CreateFMul(
      Add3ii, ConstantVector::getSplat(
        ElementCount::getFixed(64), ConstantFP::get(F32Type, 0.44269504088896340735992)));

  // %add14.i.i = fadd <64 x float> %mul13.i.i, %mul12.i.i
  auto Add14ii = Builder.CreateFAdd(Mul13ii, Mul12ii);

  // %add15.i.i = fadd <64 x float> %sub11.i.i, %add14.i.i
  auto Add15ii = Builder.CreateFAdd(Sub11ii, Add14ii);

  // %add16.i.i = fadd <64 x float> %add3.i.i, %add15.i.i
  auto Add16ii = Builder.CreateFAdd(Add3ii, Add15ii);

  // %add17.i.i = fadd <64 x float> %16, %add16.i.i
  auto Add17ii = Builder.CreateFAdd(Convert2, Add16ii);

  // %mul18.i.i = fmul <64 x float> %add17.i.i, <float 0.69314718056...
  auto Mul18ii = Builder.CreateFMul(
      Add17ii,
      ConstantVector::getSplat(ElementCount::getFixed(64), ConstantFP::get(F32Type, 0.69314718056)));

  // %17 = tail call <64 x float> @llvm.tpc.fclass...(<64 x float> %2, i8 0, i32
  // 0,
  Types = {Float64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass1 = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %18 = tail call <64 x float> @llvm.tpc.calc.fp.special
  // <64 x float> %17, <64 x float> undef, i8 0, i32 3, <64 x float> %mul18.i.i
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass1, UndefValue::get(Float64Type), Sw10,
                  ConstantInt::get(I32Type, 3), Mul18ii, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}

#if 1 // MERGE
void EvalSpecialFunctionPass::replaceReciprocalSqrtWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw10 = ConstantInt::get(I8Type, 0);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  //   %3 = tail call <64 x i32> @llvm.tpc.extract.exp.v64i32.v64f32.i1(<64 x
  //   float> %2, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false) #3
  SmallVector<Type *, 6> Types{Float64Type, I8Type, I32Type,
                               Int64Type,   I1Type, I1Type};
  auto FType = FunctionType::get(Int64Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Int64Type), Predicate, Polarity});

  // %and.i.i = and <64 x i32> %3, <i32 1, i32 1...
  auto Andii = Builder.CreateAnd(
      ExtractExp, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %4 = bitcast <64 x i32> %and.i.i to <256 x i8>
  auto BitCastI8256 = Builder.CreateBitCast(Andii, I8256Type);

  // %5 = tail call <64 x float> @llvm.tpc.form.fp.num...
  // <256 x i8> %4, <64 x float> %2, <64 x float> %2, i8 0, i32 2304,
  Types = {I8256Type, Float64Type, Float64Type, I8Type,
           I32Type,   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic,
      {BitCastI8256, Operand, Operand, Sw10, ConstantInt::get(I32Type, 2304),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %6 = tail call <128 x i32> @llvm.tpc.get.lut.entry.v128i32.v64f32.i1(<64 x
  // float> %5, i8 16, i8 0, i32 16384, <128 x i32> undef, i1 true, i1   false)
  // #3
  Types = {Float64Type, I8Type, I8Type, I32Type, Int128Type, I1Type, I1Type};
  FType = FunctionType::get(Int128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {FormFP1, ConstantInt::get(I8Type, 16),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 16384),
                  UndefValue::get(Int128Type), Predicate, Polarity});

  // %7 = shufflevector <128 x i32> %6...0...63
  auto Mask = createSequentialMask(0, 64, 0);
  auto Shuffe1 =
      Builder.CreateShuffleVector(LutEntry1, UndefValue::get(Int128Type), Mask);

  // %8 = shufflevector <128 x i32> %6...64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffe2 =
      Builder.CreateShuffleVector(LutEntry1, UndefValue::get(Int128Type), Mask);

  // %9 = bitcast <64 x i32> %8 to <64 x float>
  auto BitCast1 = Builder.CreateBitCast(Shuffe2, Float64Type);

  // %sub.i.i = fsub <64 x float> %5, %9
  auto Subii = Builder.CreateFSub(FormFP1, BitCast1);

  // %10 = tail call <64 x float> @llvm.tpc.lookup.1c.v64f32.v64i32(<64 x i32>
  // %7, i32 1, i32 0, <64 x float> undef, i1 true, i1 false) #3
  Types = {Int64Type, I32Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 1), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %11 = tail call <128 x float> @llvm.tpc.lookup.2c...
  // <64 x i32> %7, i32 128, i32 0, <128 x float> undef
  Types = {Int64Type, I32Type, I32Type, Float128Type, I1Type, I1Type};
  FType = FunctionType::get(Float128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 1), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float128Type), Predicate, Polarity});

  // %12 = shufflevector <128 x float> %11, 0...63
  Mask = createSequentialMask(0, 64, 0);
  auto Shuffle3 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %13 = shufflevector <128 x float> %11, 64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle4 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %14 = tail call <64 x float> @llvm.tpc.mac...
  // (<64 x float> %13, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %12
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle4, Subii, Sw10, Sw2, Shuffle3, Predicate, Polarity});

  // %15 = tail call <64 x float> @llvm.tpc.mac...
  // <64 x float> %14, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %10
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, Subii, Sw10, Sw2, Lookup1, Predicate, Polarity});

  // %16 = bitcast <64 x float> %15 to <64 x i32>
  auto BitCast2 = Builder.CreateBitCast(Mac2, Int64Type);

  // %17 = lshr <64 x i32> %3, <i32 1...
  auto Lshr = Builder.CreateLShr(
      ExtractExp, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %shl.i.i = shl <64 x i32> %17, <i32 23,...
  auto Shlii = Builder.CreateShl(
      Lshr, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 23)));

  // %sub4.i.i = sub <64 x i32> %16, %shl.i.i
  auto Sub4ii = Builder.CreateSub(BitCast2, Shlii);

  // %18 = bitcast <64 x i32> %sub4.i.i to <64 x float>
  auto BitCast3 = Builder.CreateBitCast(Sub4ii, Float64Type);

  // %19 = tail call <64 x float> @llvm.tpc.fclass...64 x float> %2, i8 0, i32
  // 0,
  Types = {Float64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass1 = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %20 = tail call <64 x float> @llvm.tpc.calc.fp.special...
  // <64 x float> %19, <64 x float> undef, i8 0, i32 1, <64 x float> %18,
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass1, UndefValue::get(Float64Type), Sw10,
                  ConstantInt::get(I32Type, 1), BitCast3, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceSqrtWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw10 = ConstantInt::get(I8Type, 0);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <64 x i32> @llvm.tpc.extract.exp..<64 x float> %2, i8 0, i32
  // 0,
  SmallVector<Type *, 6> Types{Float64Type, I8Type, I32Type,
                               Int64Type,   I1Type, I1Type};
  auto FType = FunctionType::get(Int64Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Int64Type), Predicate, Polarity});

  // %and.i.i = and <64 x i32> %3, <i32 1, i32 1...
  auto Andii = Builder.CreateAnd(
      ExtractExp, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %4 = bitcast <64 x i32> %and.i.i to <256 x i8>
  auto BitCastI8256 = Builder.CreateBitCast(Andii, I8256Type);

  // %5 = tail call <64 x float> @llvm.tpc.form.fp.num...
  // <256 x i8> %4, <64 x float> %2, <64 x float> %2, i8 0, i32 2304,
  Types = {I8256Type, Float64Type, Float64Type, I8Type,
           I32Type,   Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic,
      {BitCastI8256, Operand, Operand, Sw10, ConstantInt::get(I32Type, 2304),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %6 = tail call <128 x i32> @llvm.tpc.get.lut.entry...
  // <64 x float> %5, i8 17, i8 0, i32 16384, <128 x i32> undef, i1 true, i1
  // false
  Types = {Float64Type, I8Type, I8Type, I32Type, Int128Type, I1Type, I1Type};
  FType = FunctionType::get(Int128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {FormFP1, ConstantInt::get(I8Type, 17),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 16384),
                  UndefValue::get(Int128Type), Predicate, Polarity});

  // %7 = shufflevector <128 x i32> %6...0...63
  auto Mask = createSequentialMask(0, 64, 0);
  auto Shuffe1 =
      Builder.CreateShuffleVector(LutEntry1, UndefValue::get(Int128Type), Mask);

  // %8 = shufflevector <128 x i32> %6...64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffe2 =
      Builder.CreateShuffleVector(LutEntry1, UndefValue::get(Int128Type), Mask);

  // %9 = bitcast <64 x i32> %8 to <64 x float>
  auto BitCast1 = Builder.CreateBitCast(Shuffe2, Float64Type);

  // %sub.i.i = fsub <64 x float> %5, %9
  auto Subii = Builder.CreateFSub(FormFP1, BitCast1);

  // %10 = tail call <64 x float> @llvm.tpc.lookup.1c
  // (<64 x i32> %7, i32 128, i32 0...
  Types = {Int64Type, I32Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 128), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float64Type), Predicate, Polarity});

  // %11 = tail call <128 x float> @llvm.tpc.lookup.2c...
  // <64 x i32> %7, i32 128, i32 0, <128 x float> undef
  Types = {Int64Type, I32Type, I32Type, Float128Type, I1Type, I1Type};
  FType = FunctionType::get(Float128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup2 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 128), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float128Type), Predicate, Polarity});

  // %12 = shufflevector <128 x float> %11, 0...63
  Mask = createSequentialMask(0, 64, 0);
  auto Shuffle3 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %13 = shufflevector <128 x float> %11, 64...127
  Mask = createSequentialMask(64, 64, 0);
  auto Shuffle4 =
      Builder.CreateShuffleVector(Lookup2, UndefValue::get(Float128Type), Mask);

  // %14 = tail call <64 x float> @llvm.tpc.mac...
  // (<64 x float> %13, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %12
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle4, Subii, Sw10, Sw2, Shuffle3, Predicate, Polarity});

  // %15 = tail call <64 x float> @llvm.tpc.mac...
  // <64 x float> %14, <64 x float> %sub.i.i, i8 0, i32 0, <64 x float> %10
  auto Mac2 = Builder.CreateCall(
      Intrinsic, {Mac1, Subii, Sw10, Sw2, Lookup1, Predicate, Polarity});

  // %16 = bitcast <64 x float> %15 to <64 x i32>
  auto BitCast2 = Builder.CreateBitCast(Mac2, Int64Type);

  // %17 = lshr <64 x i32> %3, <i32 1...
  auto Lshr = Builder.CreateLShr(
      ExtractExp, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 1)));

  // %shl.i.i = shl <64 x i32> %17, <i32 23,...
  auto Shlii = Builder.CreateShl(
      Lshr, ConstantVector::getSplat(ElementCount::getFixed(64), ConstantInt::get(I32Type, 23)));

  // %add.i.i = add <64 x i32> %shl.i.i, %16
  auto Addii = Builder.CreateAdd(Shlii, BitCast2);

  // %18 = bitcast <64 x i32> %add.i.i to <64 x float>
  auto BitCast3 = Builder.CreateBitCast(Addii, Float64Type);

  // %19 = tail call <64 x float> @llvm.tpc.sel.eq...
  // <64 x float> %2, float 0.000000e+00, <64 x float> zeroinitializer, <64 x
  // float> %18, i8 0, i32 0,
  Types = {Float64Type, F32Type,     Float64Type, Float64Type, I8Type,
           I32Type,     Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_eq, FType),
                            FType)
          .getCallee());
  auto SelEq = Builder.CreateCall(
      Intrinsic, {Operand, ConstantFP::get(F32Type, 0.0),
                  Constant::getNullValue(Float64Type), BitCast3, Sw10, Sw2,
                  UndefValue::get(Float64Type), Predicate, Polarity});

  // %20 = tail call <64 x float> @llvm.tpc.fclass...64 x float> %2, i8 0, i32
  // 0,
  Types = {Float64Type, I8Type, I32Type, Float64Type, I1Type, I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass1 = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw10, Sw2, UndefValue::get(Float64Type), Predicate, Polarity});

  // %21 = tail call <64 x float> @llvm.tpc.calc.fp.special...
  // <64 x float> %20, <64 x float> undef, i8 0, i32 2, <64 x float> %19,
  Types = {Float64Type, Float64Type, I8Type, I32Type,
           Float64Type, I1Type,      I1Type};
  FType = FunctionType::get(Float64Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass1, UndefValue::get(Float64Type), Sw10,
                  ConstantInt::get(I32Type, 2), SelEq, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceBF16LogWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw1 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x
  // bfloat> %2,...
  SmallVector<Type *, 6> Types{Bfloat128Type, I8Type, I32Type,
                               Short128Type,  I1Type, I1Type};
  auto FType = FunctionType::get(Short128Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %4 = bitcast <128 x bfloat> %2 to <128 x i16>
  auto BitCastShort128 = Builder.CreateBitCast(Operand, Short128Type);

  // %5 = tail call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16>
  // %4, i16 -4,...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto AndShort128 = Builder.CreateCall(
      Intrinsic, {BitCastShort128, ConstantInt::get(I16Type, -4),
                  ConstantInt::get(I8Type, 8), Sw2,
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = bitcast <128 x i16> %5 to <128 x bfloat>
  auto BitCastBfloat128 = Builder.CreateBitCast(AndShort128, Bfloat128Type);

  // %7 = tail call <256 x i16> @llvm.tpc.get.lut.entry.v256i16.v128bf16.i1(<128
  // x bfloat> %6...
  Types = {Bfloat128Type, I8Type, I8Type, I32Type,
           Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {BitCastBfloat128, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 32768),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %8 = shufflevector <256 x i16> %7, <256 x i16> undef, <128 x i32> <i32 0,
  // i32 1,...
  auto Mask = createSequentialMask(0, 128, 0);
  auto Shuffe1 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask);

  // %9 = shufflevector <256 x i16> %7, <256 x i16> undef, <128 x i32> <i32
  // 128...
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffe2 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask);

  // %10 = bitcast <128 x i16> %9 to <128 x bfloat>
  auto BitCastBfloat1282 = Builder.CreateBitCast(Shuffe2, Bfloat128Type);

  // %11 = tail call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x
  // i16> %8, i16 3...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto AndShort1281 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I16Type, 3), ConstantInt::get(I8Type, 8), Sw2,
       UndefValue::get(Short128Type), Predicate, Polarity});

  // %12 = tail call <128 x bfloat>
  // @llvm.tpc.form.fp.num.v128bf16.v128bf16.i1(<128 x bfloat>... <128 x bfloat>
  // %10, <128 x bfloat> %10...
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic, {ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(1.0)),
                  BitCastBfloat1282, BitCastBfloat1282, Sw1, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %13 = tail call <128 x bfloat> @llvm.tpc.sel.eq..(<128 x i16> %11, i16 2,
  // <128 x bfloat> %10, <128 x bfloat> %12...)
  Types = {Short128Type, I16Type,       Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,      Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_eq, FType),
                            FType)
          .getCallee());
  auto SelEq = Builder.CreateCall(
      Intrinsic, {AndShort1281, ConstantInt::get(I16Type, 2), BitCastBfloat1282,
                  FormFP1, ConstantInt::get(I8Type, 8), Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %14 = tail call <128 x bfloat> @llvm.tpc.form.fp.num...(<128 x bfloat> %13,
  // <128 x bfloat> %2, <128 x bfloat> %2...
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP2 = Builder.CreateCall(
      Intrinsic, {SelEq, Operand, Operand, Sw1, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %15 = fsub <128 x bfloat> %14, %13
  auto FSub1 = Builder.CreateFSub(FormFP2, SelEq);

  // %16 = tail call <256 x bfloat> @llvm.tpc.lookup.2c...(<128 x i16> %8, i32
  // 136, i32 1, <256 x bfloat> zeroinitializer,)
  Types = {Short128Type, I32Type, I32Type, Bfloat256Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 136), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C0C1.sroa.0.256.vec.extract.i.i = shufflevector <256 x bfloat>
  // %16...128...255
  Mask = createSequentialMask(128, 128, 0);
  auto ShuffleBfloat2561 = Builder.CreateShuffleVector(
      Lookup1, UndefValue::get(Bfloat256Type), Mask);

  // %C0C1.sroa.0.0.vec.extract.i.i = shufflevector <256 x bfloat> %16...0...128
  Mask = createSequentialMask(0, 128, 0);
  auto ShuffleBfloat2562 = Builder.CreateShuffleVector(
      Lookup1, UndefValue::get(Bfloat256Type), Mask);

  // %17 = tail call <128 x bfloat> @llvm.tpc.mac...(<128 x bfloat>
  // %C0C1.sroa.0.256.vec.extract.i.i, <128 x bfloat> %15, i8 1, i32 0,  <128 x
  // bfloat> %C0C1.sroa.0.0.vec.extract.i.i)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 =
      Builder.CreateCall(Intrinsic, {ShuffleBfloat2561, FSub1, Sw1, Sw2,
                                     ShuffleBfloat2562, Predicate, Polarity});

  // %18 = tail call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128i16..(<128 x
  // i16> %3,...)
  Types = {Short128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto Convert1 = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I8Type, 7), ConstantInt::get(I32Type, 256),
       UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %19 = tail call <256 x i1> @llvm.tpc.cmp.eq...(<128 x i16> %11, i16 0,...)
  Types = {Short128Type, I16Type, I8Type, I32Type, Char256Type, I1Type, I1Type};
  FType = FunctionType::get(Char256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType),
                            FType)
          .getCallee());
  auto CmpEq1 = Builder.CreateCall(
      Intrinsic,
      {AndShort1281, ConstantInt::get(I16Type, 0), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Char256Type), Predicate, Polarity});

  // %20 = tail call <128 x bfloat> @llvm.tpc.sub.v128bf16...(<128 x bfloat>
  // %14, bfloat 0xH3F80,)
  Types = {Bfloat128Type, BF16Type, I8Type, I32Type,
           Bfloat128Type, I1Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_sub, FType), FType)
                         .getCallee());
  auto Sub1 = Builder.CreateCall(
      Intrinsic, {FormFP2, getBfloatValue(1.0), Sw1, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %21 = tail call <128 x bfloat> @llvm.tpc.mul...(<128 x bfloat> %17, <128 x
  // bfloat> %20, i8 1, i32 0, <128 x bfloat> %17,)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, Char256Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mul, FType), FType)
                         .getCallee());
  auto Mul1 = Builder.CreateCall(
      Intrinsic, {Mac1, Sub1, Sw1, Sw2, Mac1, CmpEq1,
                  /*Inverted Pred*/ ConstantInt::getTrue(M.getContext())});

  // %22 = tail call <128 x bfloat> @llvm.tpc.add..(<128 x bfloat> %21, <128 x
  // bfloat> %18, i8 1, i32 0, <128 x bfloat> %21)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, Char256Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_add, FType), FType)
                         .getCallee());
  auto Add11 = Builder.CreateCall(
      Intrinsic, {Mul1, Convert1, Sw1, Sw2, Mul1, CmpEq1, Polarity});

  // %23 = tail call <128 x bfloat> @llvm.tpc.fclass...(<128 x bfloat> %2,)
  Types = {Bfloat128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass1 = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %24 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special...((<128 x bfloat>
  // %23, <128 x bfloat> undef, i8 1, i32 3, <128 x bfloat> %22,)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass1, UndefValue::get(Bfloat128Type), Sw1,
                  ConstantInt::get(I32Type, 3), Add11, Predicate, Polarity});

  // %25 = fmul <128 x bfloat> %24, <bfloat 0xH3F31,...
  auto Final = Builder.CreateFMul(
      FPSpecial, ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(0.69314718)));
  InstrToReplace->replaceAllUsesWith(Final);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceBF16ReciprocalSqrtWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  // %2 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1...
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw1 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x
  // bfloat> %2,...
  SmallVector<Type *, 8> Types = {Bfloat128Type, I8Type, I32Type,
                                  Short128Type,  I1Type, I1Type};
  auto FType = FunctionType::get(Short128Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %4 = tail call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16>
  // %3, i16 1, i8 8,...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto AndShort128 = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %5 = tail call <128 x i16> @llvm.tpc.shr.v128i16.i16.i1(<128 x i16> %3, i16
  // 1, i8 8,...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_shr, FType), FType)
                         .getCallee());
  auto ShrShort128 = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = bitcast <128 x i16> %4 to <128 x bfloat>
  auto BitCastShort128 = Builder.CreateBitCast(AndShort128, Bfloat128Type);

  // %7 = tail call <128 x bfloat>
  // @llvm.tpc.form.fp.num.v128bf16.v128bf16.i1(<128 x bfloat> %6, <128 x
  // bfloat> %2, <128 x bfloat> %2, i8 1,...
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic,
      {BitCastShort128, Operand, Operand, Sw1, ConstantInt::get(I32Type, 2304),
       UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %8 = tail call <256 x i16> @llvm.tpc.get.lut.entry.v256i16.v128bf16.i1(<128
  // x bfloat> %7, i8 4, i8 1, i32 16384, <256 x i16> undef, i1 true, i1 false)
  Types = {Bfloat128Type, I8Type, I8Type, I32Type,
           Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {FormFP1, ConstantInt::get(I8Type, 4),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 16384),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %9 = shufflevector <256 x i16> %8, <256 x i16> undef, <128 x i32> <i32
  // 0..128
  auto Mask = createSequentialMask(0, 128, 0);
  auto Shuffle1 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask);

  // %10 = shufflevector <256 x i16> %8, <256 x i16> undef, <128 x i32> <i32
  // 128..255
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle2 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask);

  // %11 = bitcast <128 x i16> %10 to <128 x bfloat>
  auto BitCast128ShortToFloat = Builder.CreateBitCast(Shuffle2, Bfloat128Type);

  // %12 = fsub <128 x bfloat> %7, %11
  auto Sub1 = Builder.CreateFSub(FormFP1, BitCast128ShortToFloat);

  // %13 = tail call <256 x bfloat> @llvm.tpc.lookup.2c.v256bf16.v128i16(<128 x
  // i16> %9, i32 384, i32 1, <256 x bfloat> zeroinitializer, i1 true, i1 false)
  Types = {Short128Type, I32Type, I32Type, Bfloat256Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffle1, ConstantInt::get(I32Type, 384), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat256Type), Predicate, Polarity});

  // %C1C2.sroa.0.256.vec.extract.i = shufflevector <256 x bfloat> %13, <256 x
  // bfloat> undef, <128 x i32>
  Mask = createSequentialMask(128, 128, 0);
  auto Shuffle3 = Builder.CreateShuffleVector(
      Lookup1, UndefValue::get(Bfloat256Type), Mask);

  // %C1C2.sroa.0.0.vec.extract.i = shufflevector <256 x bfloat> %13, <256 x
  // bfloat> undef, <128 x i32>
  Mask = createSequentialMask(0, 128, 0);
  auto Shuffle4 = Builder.CreateShuffleVector(
      Lookup1, UndefValue::get(Bfloat256Type), Mask);

  // %14 = tail call <128 x bfloat> @llvm.tpc.mac.v128bf16.v128bf16.i1(<128 x
  // bfloat> %C1C2.sroa.0.256.vec.extract.i, <128 x bfloat> %12, i8 1, i32 0,
  // <128 x bfloat> %C1C2.sroa.0.0.vec.extract.i, i1 true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic, {Shuffle3, Sub1, ConstantInt::get(I8Type, 1),
                  ConstantInt::get(I32Type, 0), Shuffle4, Predicate, Polarity});

  // %15 = tail call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128
  // x bfloat> %14, i8 1, i32 1, <128 x i16> undef, i1 true, i1 false)
  Types = {Bfloat128Type, I8Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp1 = Builder.CreateCall(
      Intrinsic, {Mac1, Sw1, ConstantInt::get(I32Type, 1),
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %16 = sub <128 x i16> %15, %5
  auto Sub2 = Builder.CreateSub(ExtractExp1, ShrShort128);

  // %17 = bitcast <128 x i16> %16 to <128 x bfloat>
  BitCastShort128 = Builder.CreateBitCast(Sub2, Bfloat128Type);

  // %18 = tail call <128 x bfloat>
  // @llvm.tpc.form.fp.num.v128bf16.v128bf16.i1(<128 x bfloat> %17, <128 x
  // bfloat> %14, <128 x bfloat> %14, i8 1, i32 2560, <128 x bfloat> undef, i1
  // true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP2 = Builder.CreateCall(
      Intrinsic,
      {BitCastShort128, Mac1, Mac1, Sw1, ConstantInt::get(I32Type, 2560),
       UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %19 = tail call <128 x bfloat> @llvm.tpc.fclass.v128bf16.i1(<128 x bfloat>
  // %2, i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  Types = {Bfloat128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %20 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special.v128bf16.i1(<128 x
  // bfloat> %19, <128 x bfloat> undef, i8 1, i32 1, <128 x bfloat> %18, i1
  // true, i1 false)
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass, UndefValue::get(Bfloat128Type), Sw1,
                  ConstantInt::get(I32Type, 1), FormFP2, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceBF16SqrtWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw1 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x bfloat> @llvm.tpc.fclass.v128bf16...<128 x bfloat>
  // %2,
  SmallVector<Type *, 6> Types{Bfloat128Type, I8Type, I32Type,
                               Bfloat128Type, I1Type, I1Type};
  auto FType = FunctionType::get(Bfloat128Type, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %4 = tail call <128 x i16> @llvm.tpc.extract.exp...<128 x bfloat> %2,...
  Types = {Bfloat128Type, I8Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp = Builder.CreateCall(
      Intrinsic,
      {Operand, Sw1, Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %5 = tail call <128 x i16> @llvm.tpc.and.v128i16...<128 x i16> %4, i16 1,
  // i8 8,...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_and, FType), FType)
                         .getCallee());
  auto AndShort128 = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = tail call <128 x i16> @llvm.tpc.shr...<128 x i16> %4, i16 1, i8 8,...
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_shr, FType), FType)
                         .getCallee());
  auto ShrShort128 = Builder.CreateCall(
      Intrinsic,
      {ExtractExp, ConstantInt::get(I16Type, 1), ConstantInt::get(I8Type, 8),
       Sw2, UndefValue::get(Short128Type), Predicate, Polarity});

  // %7 = bitcast <128 x i16> %5 to <128 x bfloat>
  auto BitCastShort128 = Builder.CreateBitCast(AndShort128, Bfloat128Type);

  // %8 = tail call <128 x bfloat> @llvm.tpc.form.fp..(<128 x bfloat> %7, <128 x
  // bfloat> %2, <128 x bfloat> %2, i8 1,
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP1 = Builder.CreateCall(
      Intrinsic,
      {BitCastShort128, Operand, Operand, Sw1, ConstantInt::get(I32Type, 2304),
       UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %9 = tail call <256 x i16> @llvm.tpc.get.lut.entry..(<128 x bfloat> %8, i8
  // 1, i8 1, i32 16384)
  Types = {Bfloat128Type, I8Type, I8Type, I32Type,
           Short256Type,  I1Type, I1Type};
  FType = FunctionType::get(Short256Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto LutEntry1 = Builder.CreateCall(
      Intrinsic, {FormFP1, ConstantInt::get(I8Type, 1),
                  ConstantInt::get(I8Type, 1), ConstantInt::get(I32Type, 16384),
                  UndefValue::get(Short256Type), Predicate, Polarity});

  // %10 = shufflevector <256 x i16> %9, <256 x i16> undef, <128 x i32> <i32
  // 0..128
  auto Mask = createSequentialMask(0, 128, 0);
  auto Shuffe1 = Builder.CreateShuffleVector(
      LutEntry1, UndefValue::get(Short256Type), Mask);

  // %11 = tail call <128 x bfloat> @llvm.tpc.lookup.1c...<128 x i16> %10, i32
  // 276, i32 1,
  Types = {Short128Type, I32Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Shuffe1, ConstantInt::get(I32Type, 276), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat128Type), Predicate, Polarity});

  // %12 = tail call <128 x i16> @llvm.tpc.extract.exp...<128 x bfloat> %11, i8
  // 1, i32 1
  Types = {Bfloat128Type, I8Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_extract_exp, FType), FType)
          .getCallee());
  auto ExtractExp1 = Builder.CreateCall(
      Intrinsic, {Lookup1, Sw1, ConstantInt::get(I32Type, 1),
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %13 = add <128 x i16> %12, %6
  auto Add1 = Builder.CreateAdd(ExtractExp1, ShrShort128);

  // %14 = bitcast <128 x i16> %13 to <128 x bfloat>
  auto BitCastShort1281 = Builder.CreateBitCast(Add1, Bfloat128Type);

  // %15 = tail call <128 x bfloat> @llvm.tpc.form.fp.num...<128 x bfloat> %14,
  // <128 x bfloat> %11, <128 x bfloat> %11,
  Types = {Bfloat128Type, Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_form_fp_num, FType), FType)
          .getCallee());
  auto FormFP2 = Builder.CreateCall(
      Intrinsic,
      {BitCastShort1281, Lookup1, Lookup1, Sw1, ConstantInt::get(I32Type, 2560),
       UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %16 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special...
  // <128 x bfloat> %3, <128 x bfloat> undef, i8 1, i32 2, <128 x bfloat> %15,
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass, UndefValue::get(Bfloat128Type), Sw1,
                  ConstantInt::get(I32Type, 2), FormFP2, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}
#endif // MERGE

void EvalSpecialFunctionPass::replaceBF16ExpWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);

  // Helper Values.
  auto Sw11 = ConstantInt::get(I8Type, 1);
  auto Sw2 = ConstantInt::get(I32Type, 0);
  auto Predicate = ConstantInt::get(I1Type, 1);
  auto Polarity = ConstantInt::get(I1Type, 0);

  // %3 = tail call <128 x bfloat> @llvm.tpc.mac...<128 x bfloat> %2, <128 x
  // bfloat> <bfloat 1.4453125... <128 x bfloat> <bfloat 0.5...
  SmallVector<Type *, 6> Types{Bfloat128Type, Bfloat128Type, I8Type, I32Type,
                               Bfloat128Type, I1Type,        I1Type};
  auto FType = FunctionType::get(Bfloat128Type, Types, false);
  auto Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac1 = Builder.CreateCall(
      Intrinsic,
      {Operand, ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(1.4453125)), Sw11,
       Sw2, ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(0.5)), Predicate,
       Polarity});

  // %4 = tail call <128 x bfloat> @llvm.tpc.nearbyint...<128 x bfloat> %3, i8
  // 1, i32 198400
  Types = {Bfloat128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_nearbyint, FType), FType)
          .getCallee());
  auto Nearby = Builder.CreateCall(
      Intrinsic, {Mac1, Sw11, ConstantInt::get(I32Type, 196608),
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %5 = tail call <128 x i16> @llvm.tpc.convert...<128 x bfloat> %4, i8 1, i32
  // 198464,
  Types = {Bfloat128Type, I8Type, I32Type, Short128Type, I1Type, I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                            FType)
          .getCallee());
  auto Convert1 = Builder.CreateCall(
      Intrinsic, {Nearby, Sw11, ConstantInt::get(I32Type, 198400),
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %6 = tail call <128 x bfloat> @llvm.tpc.mac...<128 x bfloat> %4, <128 x
  // bfloat> <bfloat -0.6875..> <128 x bfloat> %2
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac2 = Builder.CreateCall(
      Intrinsic,
      {Nearby, ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(-0.6875)), Sw11,
       Sw2, Operand, Predicate, Polarity});

  // %7 = tail call <128 x bfloat> @llvm.tpc.mac...<128 x bfloat> %4, <128 x
  // bfloat> <bfloat -0.00564575195> <128 x bfloat> %6
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_mac, FType), FType)
                         .getCallee());
  auto Mac3 = Builder.CreateCall(
      Intrinsic,
      {Nearby, ConstantVector::getSplat(ElementCount::getFixed(128), getBfloatValue(-0.00564575195)),
       Sw11, Sw2, Mac2, Predicate, Polarity});

  // %8 = tail call <128 x bfloat> @llvm.tpc.sel.leq...(<128 x bfloat> %2,
  // bfloat 0xc2af, <128 x bfloat> <bfloat 0xff80...> <128 x bfloat> %2,
  Types = {Bfloat128Type, BF16Type,      Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_leq, FType),
                            FType)
          .getCallee());
  auto SelLeq = Builder.CreateCall(
      Intrinsic,
      {Operand, getBfloatValue(-87.3365479),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantFP::getInfinity(BF16Type, true)),
       Operand, Sw11, Sw2, UndefValue::get(Bfloat128Type), Predicate,
       Polarity});

  // %9 = tail call <128 x bfloat> @llvm.tpc.sel.gr...(<128 x bfloat> %8, bfloat
  // 0x42B1, <128 x bfloat> <bfloat 0x7F80>) <128 x bfloat> %8
  Types = {Bfloat128Type, BF16Type,      Bfloat128Type, Bfloat128Type, I8Type,
           I32Type,       Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType),
                            FType)
          .getCallee());
  auto SelGrt = Builder.CreateCall(
      Intrinsic,
      {SelLeq, getBfloatValue(88.7228394),
       ConstantVector::getSplat(ElementCount::getFixed(128), ConstantFP::getInfinity(BF16Type, false)),
       SelLeq, Sw11, Sw2, UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %10 = tail call <128 x bfloat> @llvm.tpc.add...<128 x bfloat> %7, bfloat
  Types = {Bfloat128Type, BF16Type, I8Type, I32Type,
           Bfloat128Type, I1Type,   I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_add, FType), FType)
                         .getCallee());
  auto Add1 = Builder.CreateCall(
      Intrinsic, {Mac3, getBfloatValue(1.5), Sw11, Sw2,
                  UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %11 = bitcast <128 x bfloat> %10 to <128 x i16>
  auto BitCast1 = Builder.CreateBitCast(Add1, Short128Type);

  // %12 = tail call <128 x i16> @llvm.tpc.sub...<128 x i16> %11, i16 16199, i8
  // 8, i32 1
  Types = {Short128Type, I16Type, I8Type, I32Type,
           Short128Type, I1Type,  I1Type};
  FType = FunctionType::get(Short128Type, Types, false);
  Intrinsic =
      cast<Function>(M.getOrInsertFunction(
                          getTPCIntrinsicName(Intrinsic::tpc_sub, FType), FType)
                         .getCallee());
  auto Sub1 = Builder.CreateCall(
      Intrinsic, {BitCast1, ConstantInt::get(I16Type, 16199),
                  ConstantInt::get(I8Type, 8), ConstantInt::get(I32Type, 1),
                  UndefValue::get(Short128Type), Predicate, Polarity});

  // %13 = tail call <128 x bfloat> @llvm.tpc.lookup.1c...<128 x i16> %12, i32
  // 138, i32 1, <128 x bfloat> zeroinitializer
  Types = {Short128Type, I32Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto Lookup1 = Builder.CreateCall(
      Intrinsic,
      {Sub1, ConstantInt::get(I32Type, 138), ConstantInt::get(I32Type, 1),
       Constant::getNullValue(Bfloat128Type), Predicate, Polarity});

  // %14 = shl <128 x i16> %5, <i16 7,...
  auto Shl1 = Builder.CreateShl(
      Convert1, ConstantVector::getSplat(ElementCount::getFixed(128), ConstantInt::get(I16Type, 7)));

  // %15 = bitcast <128 x bfloat> %13 to <128 x i16>
  auto BitCast2 = Builder.CreateBitCast(Lookup1, Short128Type);

  // %16 = add <128 x i16> %14, %15
  auto Add2 = Builder.CreateAdd(Shl1, BitCast2);

  // %17 = bitcast <128 x i16> %16 to <128 x bfloat>
  auto BitCast3 = Builder.CreateBitCast(Add2, Bfloat128Type);

  // %18 = tail call <128 x bfloat> @llvm.tpc.fclass...<128 x bfloat> %9, i8 1,
  // i32 0,
  Types = {Bfloat128Type, I8Type, I32Type, Bfloat128Type, I1Type, I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_fclass, FType),
                            FType)
          .getCallee());
  auto Fclass = Builder.CreateCall(
      Intrinsic,
      {SelGrt, Sw11, Sw2, UndefValue::get(Bfloat128Type), Predicate, Polarity});

  // %19 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special...
  // <128 x bfloat> %18, <128 x bfloat> undef, i8 1, i32 4, <128 x bfloat> %17
  Types = {Bfloat128Type, Bfloat128Type, I8Type, I32Type,
           Bfloat128Type, I1Type,        I1Type};
  FType = FunctionType::get(Bfloat128Type, Types, false);
  Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_calc_fp_special, FType), FType)
          .getCallee());
  auto FPSpecial = Builder.CreateCall(
      Intrinsic, {Fclass, UndefValue::get(Bfloat128Type), Sw11,
                  ConstantInt::get(I32Type, 4), BitCast3, Predicate, Polarity});

  InstrToReplace->replaceAllUsesWith(FPSpecial);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::replaceSinCosWithTPCIntrinsics(
    Module &M, Instruction *InstrToReplace, int SinCond) {

  IRBuilder<> Builder(InstrToReplace);
  Value *Operand = InstrToReplace->getOperand(0);
  auto Ty = Operand->getType();
  SmallVector<Type *, 9> Types{Ty,
                               F32Type,
                               Ty,
                               Ty,
                               IntegerType::get(M.getContext(), 8),
                               IntegerType::get(M.getContext(), 32),
                               Ty,
                               I1Type,
                               I1Type};
  FunctionType *FType = FunctionType::get(Ty, Types, false);
  Function *Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType), FType)
          .getCallee());
  auto ConstOne = ConstantFP::get(F32Type, 1.0);
  auto ConstNegOne = ConstantFP::get(F32Type, -1.0);
  auto SelGrtRes = Builder.CreateCall(
      Intrinsic,
      {Operand, ConstantFP::get(F32Type, 0.0),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstOne),
       ConstantVector::getSplat(ElementCount::getFixed(64), ConstNegOne),
       llvm::ConstantInt::get(IntegerType::get(M.getContext(), 8), 0),
       llvm::ConstantInt::get(IntegerType::get(M.getContext(), 32), 0),
       UndefValue::get(Float64Type), llvm::ConstantInt::get(I1Type, 1),
       llvm::ConstantInt::getFalse(M.getContext())});

  // Begin SIN_COS_CALC(0)
  auto FType0 = FunctionType::get(Float64Type, {Operand->getType()}, false);
  auto Intrinsic0 = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::fabs, FType0),
                                FType0)
          .getCallee());
  auto FabsRes = Builder.CreateCall(Intrinsic0, {Operand});

  auto op0Mul = FabsRes;
  auto op1Mul = ConstantFP::get(Operand->getType(), 1.27323949);
  auto MulRes = Builder.CreateFMul(op0Mul, op1Mul);

  SmallVector<Type *, 6> TypesConvert{MulRes->getType(),
                                      IntegerType::get(M.getContext(), 8),
                                      IntegerType::get(M.getContext(), 32),
                                      Int64Type,
                                      I1Type,
                                      I1Type};
  FType = FunctionType::get(Int64Type, TypesConvert, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());
  auto ConvertRes = Builder.CreateCall(
      Intrinsic, {MulRes, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 197120), UndefValue::get(Int64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  auto ConstOneInt = ConstantInt::get(I32Type, 1);
  auto Op0And = ConstantVector::getSplat(ElementCount::getFixed(64), ConstOneInt);
  auto AndRes = Builder.CreateAnd(ConvertRes, Op0And);

  auto AddRes = Builder.CreateAdd(ConvertRes, AndRes);

  SmallVector<Type *, 6> TypesConvert2{AddRes->getType(),
                                       IntegerType::get(M.getContext(), 8),
                                       IntegerType::get(M.getContext(), 32),
                                       Float64Type,
                                       I1Type,
                                       I1Type};
  FType = FunctionType::get(Float64Type, TypesConvert2, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());
  ConvertRes = Builder.CreateCall(
      Intrinsic, {AddRes, ConstantInt::get(I8Type, 2),
                  ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 6> TypesMac{ConvertRes->getType(),
                                  Float64Type,
                                  IntegerType::get(M.getContext(), 8),
                                  IntegerType::get(M.getContext(), 32),
                                  FabsRes->getType(),
                                  I1Type,
                                  I1Type};
  FType = FunctionType::get(Float64Type, TypesMac, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  auto Const0Mac = ConstantFP::get(F32Type, 7.85156250e-01);
  auto MacRes = Builder.CreateCall(
      Intrinsic,
      {ConvertRes, ConstantVector::getSplat(ElementCount::getFixed(64), Const0Mac),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 2), FabsRes,
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 6> Types2Mac{ConvertRes->getType(),
                                   Float64Type,
                                   IntegerType::get(M.getContext(), 8),
                                   IntegerType::get(M.getContext(), 32),
                                   MacRes->getType(),
                                   I1Type,
                                   I1Type};
  FType = FunctionType::get(Float64Type, Types2Mac, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  Const0Mac = ConstantFP::get(F32Type, 2.41875648498e-4);
  auto Mac2Res = Builder.CreateCall(
      Intrinsic,
      {ConvertRes, ConstantVector::getSplat(ElementCount::getFixed(64), Const0Mac),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 2), MacRes,
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 6> Types3Mac{ConvertRes->getType(),
                                   Float64Type,
                                   IntegerType::get(M.getContext(), 8),
                                   IntegerType::get(M.getContext(), 32),
                                   Mac2Res->getType(),
                                   I1Type,
                                   I1Type};
  FType = FunctionType::get(Float64Type, Types3Mac, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  Const0Mac = ConstantFP::get(F32Type, 3.7748949774e-8);
  auto Mac3Res = Builder.CreateCall(
      Intrinsic,
      {ConvertRes, ConstantVector::getSplat(ElementCount::getFixed(64), Const0Mac),
       ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 2), Mac2Res,
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  FType = FunctionType::get(Float64Type, {Mac3Res->getType()}, false);
  Intrinsic =
      cast<Function>(InstrToReplace->getModule()
                         ->getOrInsertFunction(
                             getTPCIntrinsicName(Intrinsic::fabs, FType), FType)
                         .getCallee());
  auto FabsRes2 = Builder.CreateCall(Intrinsic, {Mac3Res});

  auto Op0Lshr = ConstantInt::get(Int64Type, 1);
  auto LshrRes = Builder.CreateLShr(AddRes, Op0Lshr);

  auto Op0And2 = ConstantInt::get(I32Type, 3);
  auto And2Res =
      Builder.CreateAnd(LshrRes, ConstantVector::getSplat(ElementCount::getFixed(64), Op0And2));

  auto Op0And3 = ConstantInt::get(I32Type, 2);
  auto And3Res =
      Builder.CreateAnd(LshrRes, ConstantVector::getSplat(ElementCount::getFixed(64), Op0And3));

  FType = FunctionType::get(Float64Type, TypesConvert2, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());
  ConvertRes = Builder.CreateCall(
      Intrinsic, {And3Res, ConstantInt::get(I8Type, 2),
                  ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  Value *SubRes;
  if (!SinCond) {
    MulRes = Builder.CreateFMul(SelGrtRes, ConvertRes);
    SubRes = Builder.CreateFSub(SelGrtRes, MulRes);
  } else {
    auto ConstOne0 = ConstantFP::get(F32Type, 1.000000e+00);
    auto Op0Sub4 = ConstantVector::getSplat(ElementCount::getFixed(64), ConstOne0);
    MulRes = Builder.CreateFSub(Op0Sub4, ConvertRes);
  }
  auto Sub2Res = Builder.CreateNSWSub(And2Res, And3Res);

  SmallVector<Type *, 7> TypesCmpEq{Sub2Res->getType(),
                                    IntegerType::get(M.getContext(), 32),
                                    IntegerType::get(M.getContext(), 8),
                                    IntegerType::get(M.getContext(), 32),
                                    Char256Type,
                                    I1Type,
                                    I1Type};
  FType = FunctionType::get(Char256Type, TypesCmpEq, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_cmp_eq, FType), FType)
          .getCallee());
  auto CmpEqRes = Builder.CreateCall(
      Intrinsic,
      {Sub2Res, ConstantInt::get(I32Type, SinCond), ConstantInt::get(I8Type, 2),
       ConstantInt::get(I32Type, 0), UndefValue::get(Char256Type),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 7> TypesGetLutEntry{FabsRes2->getType(),
                                          IntegerType::get(M.getContext(), 8),
                                          IntegerType::get(M.getContext(), 8),
                                          IntegerType::get(M.getContext(), 32),
                                          FixedVectorType::get(I32Type, 128),
                                          I1Type,
                                          I1Type};
  FType =
      FunctionType::get(FixedVectorType::get(I32Type, 128), TypesGetLutEntry, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_get_lut_entry, FType), FType)
          .getCallee());
  auto GetLutEntryRes = Builder.CreateCall(
      Intrinsic, {FabsRes2, ConstantInt::get(I8Type, 17),
                  ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 24576),
                  UndefValue::get(FixedVectorType::get(I32Type, 128)),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  SmallVector<uint32_t, 64> Vec0;
  SmallVector<uint32_t, 64> Vec1;
  for (int i = 0; i < 64; i++) {
    Vec0.push_back(i);
    Vec1.push_back(64 + i);
  }

  auto FirstHalf = Builder.CreateShuffleVector(
      GetLutEntryRes, UndefValue::get(FixedVectorType::get(I32Type, 128)), Vec0);
  auto SecondHalf = Builder.CreateShuffleVector(
      GetLutEntryRes, UndefValue::get(FixedVectorType::get(I32Type, 128)), Vec1);

  auto DestType = Float64Type;
  auto BitCastRes = Builder.CreateBitCast(SecondHalf, DestType);

  SmallVector<Type *, 7> TypesAdd4{Int64Type, I32Type,   I8Type,
                                   I32Type,   Int64Type, CmpEqRes->getType(),
                                   I1Type};
  FType = FunctionType::get(Int64Type, TypesAdd4, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_add, FType),
                                FType)
          .getCallee());
  auto Add4Res = Builder.CreateCall(
      Intrinsic, {FirstHalf, ConstantInt::get(I32Type, 64),
                  ConstantInt::get(I8Type, 3), ConstantInt::get(I32Type, 0),
                  FirstHalf, CmpEqRes, ConstantInt::get(I1Type, 1)});

  auto Sub3Res = Builder.CreateFSub(FabsRes2, BitCastRes);

  SmallVector<Type *, 6> TypesLookUp1c{Int64Type,   I32Type, I32Type,
                                       Float64Type, I1Type,  I1Type};
  FType = FunctionType::get(Float64Type, TypesLookUp1c, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_lookup_1c, FType), FType)
          .getCallee());
  auto LookUp1cRes = Builder.CreateCall(
      Intrinsic,
      {Add4Res, ConstantInt::get(I32Type, 130), ConstantInt::get(I32Type, 0),
       ConstantAggregateZero::get(Float64Type), ConstantInt::get(I1Type, 1),
       ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 6> TypesLookUp2c{Int64Type, I32Type,
                                       I32Type,   FixedVectorType::get(F32Type, 128),
                                       I1Type,    I1Type};
  FType =
      FunctionType::get(FixedVectorType::get(F32Type, 128), TypesLookUp2c, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_lookup_2c, FType), FType)
          .getCallee());
  auto LookUp2cRes = Builder.CreateCall(
      Intrinsic,
      {Add4Res, ConstantInt::get(I32Type, 130), ConstantInt::get(I32Type, 0),
       UndefValue::get(FixedVectorType::get(F32Type, 128)),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  FirstHalf = Builder.CreateShuffleVector(
      LookUp2cRes, UndefValue::get(FixedVectorType::get(F32Type, 128)), Vec0);
  SecondHalf = Builder.CreateShuffleVector(
      LookUp2cRes, UndefValue::get(FixedVectorType::get(F32Type, 128)), Vec1);

  FType = FunctionType::get(Float64Type, Types3Mac, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mac, FType),
                                FType)
          .getCallee());
  auto Mac4Res = Builder.CreateCall(
      Intrinsic, {SecondHalf, Sub3Res, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 0), FirstHalf,
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  auto Mac5Res = Builder.CreateCall(
      Intrinsic, {Mac4Res, Sub3Res, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 0), LookUp1cRes,
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});

  SmallVector<Type *, 7> TypesTPCMul{
      Float64Type, Mac5Res->getType(), I8Type, I32Type,
      Float64Type, Char256Type,        I1Type};
  FType = FunctionType::get(Float64Type, TypesTPCMul, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_mul, FType),
                                FType)
          .getCallee());
  auto TPCMulRes = Builder.CreateCall(
      Intrinsic, {FabsRes2, Mac5Res, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 0), Mac5Res, CmpEqRes,
                  ConstantInt::get(I1Type, 0)});
  Value *TPCSelLessRes, *TPCSelGrtRes0;
  if (!SinCond) {
    auto Sub4Res = Builder.CreateFNeg(SubRes);
    SmallVector<Type *, 9> TypesSelLess{Float64Type, F32Type,     Float64Type,
                                        Float64Type, I8Type,      I32Type,
                                        Float64Type, Char256Type, I1Type};
    FType = FunctionType::get(Float64Type, TypesSelLess, false);
    Intrinsic = cast<Function>(
        InstrToReplace->getModule()
            ->getOrInsertFunction(
                getTPCIntrinsicName(Intrinsic::tpc_sel_less, FType), FType)
            .getCallee());
    TPCSelLessRes = Builder.CreateCall(
        Intrinsic,
        {Mac3Res, ConstantFP::get(F32Type, 0.000000e+00), Sub4Res, SubRes,
         ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), SubRes,
         CmpEqRes, ConstantInt::get(I1Type, 0)});
  } else {
    auto Sub0Res = Builder.CreateFNeg(MulRes);
    SmallVector<Type *, 9> TypesSelGrt0{
        Mac3Res->getType(), F32Type,     Sub0Res->getType(),
        MulRes->getType(),  I8Type,      I32Type,
        Float64Type,        Char256Type, I1Type};
    FType = FunctionType::get(Float64Type, TypesSelGrt0, false);
    Intrinsic = cast<Function>(
        InstrToReplace->getModule()
            ->getOrInsertFunction(
                getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType), FType)
            .getCallee());
    TPCSelGrtRes0 = Builder.CreateCall(
        Intrinsic,
        {Mac3Res, ConstantFP::get(F32Type, 0.000000e+00), Sub0Res, MulRes,
         ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0), MulRes,
         CmpEqRes, ConstantInt::get(I1Type, 0)});
  }

  auto Sub5Res = Builder.CreateFNeg(TPCMulRes);

  SmallVector<Type *, 9> TypesSelLess2{Float64Type, F32Type, Float64Type,
                                       Float64Type, I8Type,  I32Type,
                                       Float64Type, I1Type,  I1Type};
  FType = FunctionType::get(Float64Type, TypesSelLess2, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_less, FType), FType)
          .getCallee());

  Value *TPCSelLess2Res;
  if (!SinCond) {
    TPCSelLess2Res = Builder.CreateCall(
        Intrinsic, {TPCSelLessRes, ConstantFP::get(F32Type, 0.000000e+00),
                    Sub5Res, TPCMulRes, ConstantInt::get(I8Type, 0),
                    ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                    ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  } else {
    TPCSelLess2Res = Builder.CreateCall(
        Intrinsic, {TPCSelGrtRes0, ConstantFP::get(F32Type, 0.000000e+00),
                    Sub5Res, TPCMulRes, ConstantInt::get(I8Type, 0),
                    ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                    ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  }
  SmallVector<Type *, 6> TypesConvert3{I32Type, I8Type, I32Type,
                                       F32Type, I1Type, I1Type};
  FType = FunctionType::get(F32Type, TypesConvert3, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_convert, FType), FType)
          .getCallee());
  auto ConvertRes2 = Builder.CreateCall(
      Intrinsic,
      {ConstantInt::get(I32Type, 16777215), ConstantInt::get(I8Type, 2),
       ConstantInt::get(I32Type, 0), ConstantFP::get(F32Type, 0.000000e+00),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  auto ConvertRes3 = Builder.CreateCall(
      Intrinsic,
      {ConstantInt::get(I32Type, 8192), ConstantInt::get(I8Type, 2),
       ConstantInt::get(I32Type, 0), ConstantFP::get(F32Type, 0.000000e+00),
       ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  SmallVector<Type *, 9> TypesSelGrt{FabsRes->getType(), F32Type, Float64Type,
                                     Float64Type,        I8Type,  I32Type,
                                     Float64Type,        I1Type,  I1Type};
  FType = FunctionType::get(Float64Type, TypesSelGrt, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_grt, FType), FType)
          .getCallee());
  auto TPCSelGrtRes = Builder.CreateCall(
      Intrinsic, {FabsRes, ConvertRes3, ConstantAggregateZero::get(Float64Type),
                  TPCSelLess2Res, ConstantInt::get(I8Type, 0),
                  ConstantInt::get(I32Type, 0), UndefValue::get(Float64Type),
                  ConstantInt::get(I1Type, 1), ConstantInt::get(I1Type, 0)});
  auto ConstNanFP32 = ConstantFP::getNaN(F32Type);
  auto TPCSelGrt2Res = Builder.CreateCall(
      Intrinsic,
      {FabsRes, ConvertRes2, ConstantVector::getSplat(ElementCount::getFixed(64), ConstNanFP32),
       TPCSelGrtRes, ConstantInt::get(I8Type, 0), ConstantInt::get(I32Type, 0),
       UndefValue::get(Float64Type), ConstantInt::get(I1Type, 1),
       ConstantInt::get(I1Type, 0)});

  auto DestType2 = Int64Type;
  auto BitCastRes2 = Builder.CreateBitCast(FabsRes, DestType2);

  SmallVector<Type *, 9> TypesSelGeq{DestType2,   I32Type, Float64Type,
                                     Float64Type, I8Type,  I32Type,
                                     Float64Type, I1Type,  I1Type};
  FType = FunctionType::get(Float64Type, TypesSelGeq, false);
  Intrinsic = cast<Function>(
      InstrToReplace->getModule()
          ->getOrInsertFunction(
              getTPCIntrinsicName(Intrinsic::tpc_sel_geq, FType), FType)
          .getCallee());
  auto TPCSelGeqRes = Builder.CreateCall(
      Intrinsic, {BitCastRes2, ConstantInt::get(I32Type, 2139095040),
                  ConstantVector::getSplat(ElementCount::getFixed(64), ConstNanFP32), TPCSelGrt2Res,
                  ConstantInt::get(I8Type, 3), ConstantInt::get(I32Type, 0),
                  UndefValue::get(Float64Type), ConstantInt::get(I1Type, 1),
                  ConstantInt::get(I1Type, 0)});
  InstrToReplace->replaceAllUsesWith(TPCSelGeqRes);
  InstrToReplace->eraseFromParent();
}

void EvalSpecialFunctionPass::expandSpecialFunction(Module &M) {
  for (auto &FuncIt : M) {
    Function *F = &FuncIt;
    for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
      inst_iterator PrevIt;
      bool bFirst = false;
      if (It == inst_begin(F)) {
        bFirst = true;
      } else {
        PrevIt = It;
        --PrevIt;
      }
      Instruction *CurrI = &(*It);
      if (!dyn_cast<IntrinsicInst>(CurrI))
        continue;
      auto IntrInst = dyn_cast<IntrinsicInst>(CurrI);
      auto IntrID = IntrInst->getIntrinsicID();
      auto Ty = IntrInst->getType();

      bool Change = true;
      if (Ty == Float64Type) {
        if (IntrID == Intrinsic::sin)
          replaceSinCosWithTPCIntrinsics(M, CurrI, 0);
        else if (IntrID == Intrinsic::cos)
          replaceSinCosWithTPCIntrinsics(M, CurrI, 1);
        else if (IntrID == Intrinsic::exp)
          replaceExpWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::log)
          replaceLogWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::sqrt)
          replaceSqrtWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_rsqrt)
          replaceReciprocalSqrtWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_tanh)
          replaceTanhWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_reciprocal)
          replaceReciprocalWithTPCIntrinsics(M, CurrI);
        else
          Change = false;
      } else if (Ty == Bfloat128Type) {
        if (IntrID == Intrinsic::sin)
          replaceBF16SinWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::cos)
          replaceBF16CosWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::exp)
          replaceBF16ExpWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::log)
          replaceBF16LogWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::sqrt)
          replaceBF16SqrtWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_rsqrt)
          replaceBF16ReciprocalSqrtWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_tanh)
          replaceBF16TanhWithTPCIntrinsics(M, CurrI);
        else if (IntrID == Intrinsic::tpc_reciprocal)
          replaceBF16ReciprocalWithTPCIntrinsics(M, CurrI);
        else
          Change = false;
      } else {
        Change = false;
      }

      if (Change) {
        if (!bFirst) {
          It = ++PrevIt;
        } else {
          It = ++inst_begin(F);
        }
      }
    }
  }
}

void EvalSpecialFunctionPass::expandFDiv(Module &M, Instruction *I) {
  IRBuilder<> Builder(I);
  Type *Ty = I->getType();
  Value *Numerator = I->getOperand(0), *Denominator = I->getOperand(1);
  SmallVector<Type *, 1> Types = {Ty};
  auto FType = FunctionType::get(Ty, Types, false);
  auto Intrinsic = cast<Function>(
      M.getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_reciprocal, FType), FType)
          .getCallee());
  auto Recip = Builder.CreateCall(Intrinsic, Denominator);
  auto FMul = Builder.CreateFMul(Numerator, Recip);
  I->replaceAllUsesWith(FMul);
}

void EvalSpecialFunctionPass::expandSpecialCaseLLVMIR(Module &M) {
  SmallVector<Instruction *, 8> EraseList;
  for (auto &FIt : M)
    for (inst_iterator It = inst_begin(&FIt), E = inst_end(&FIt); It != E;
         ++It) {
      Instruction *I = &*It;
      // Handle vector FDIV case.
      if (I->getType()->isVectorTy() && I->getOpcode() == Instruction::FDiv 
          /*&& cast<FixedVectorType>(I->getType())->getElementType()->getTypeID() != Type::FloatTyID*/) {
        expandFDiv(M, I);
        EraseList.push_back(I);
      }
    }

  for (Instruction *I : EraseList)
    I->eraseFromParent();
}

bool EvalSpecialFunctionPass::runOnModule(Module &M) {

  if (!EvalSpclFunc)
    return false;

  I32Type = Type::getInt32Ty(M.getContext());
  I1Type = Type::getInt1Ty(M.getContext());
  I8Type = Type::getInt8Ty(M.getContext());
  I16Type = Type::getInt16Ty(M.getContext());
  F32Type = Type::getFloatTy(M.getContext());
  BF16Type = Type::getBFloatTy(M.getContext());
  Int64Type = FixedVectorType::get(I32Type, 64);
  Int128Type = FixedVectorType::get(I32Type, 128);
  Float64Type = FixedVectorType::get(F32Type, 64);
  Float128Type = FixedVectorType::get(F32Type, 128);
  Short128Type = FixedVectorType::get(I16Type, 128);
  Short256Type = FixedVectorType::get(I16Type, 256);
  Bfloat128Type = FixedVectorType::get(BF16Type, 128);
  Bfloat256Type = FixedVectorType::get(BF16Type, 256);
  Char256Type = FixedVectorType::get(I1Type, 256);
  I8256Type = FixedVectorType::get(I8Type, 256);

  expandSpecialCaseLLVMIR(M);
  expandSpecialFunction(M);
  return false;
}

ModulePass *llvm::createEvalSpecialFunctionPass() {
  return new EvalSpecialFunctionPass();
}
