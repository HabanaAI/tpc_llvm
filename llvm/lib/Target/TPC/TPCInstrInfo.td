//===-- TPCInstrInfo.td ------ TPC Instruction Definition --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the TPC instruction set, defining the instructions, and
// properties of the instructions which are needed for code generation, machine
// code emission, and analysis.
//
//===----------------------------------------------------------------------===//
include "TPCInstrFormats.td"

// ------------------------- New nodes -----------------------------------------

def fpor : SDNode<"TPCISD::FPOR", SDTFPBinOp, [SDNPCommutative]>;
def fpand : SDNode<"TPCISD::FPAND", SDTFPBinOp, [SDNPCommutative]>;
def fpxor : SDNode<"TPCISD::FPXOR", SDTFPBinOp, [SDNPCommutative]>;
def fpnot : SDNode<"TPCISD::FPNOT", SDTFPUnaryOp, []>;
def inot : SDNode<"TPCISD::INOT", SDTIntUnaryOp, []>;
def fmac : SDNode<"TPCISD::FMAC", SDTFPBinOp, []>;
def mac : SDNode<"TPCISD::MAC", SDTIntBinOp, []>;
def fmax : SDNode<"TPCISD::FMAX", SDTFPBinOp, [SDNPCommutative]>;
def fmin : SDNode<"TPCISD::FMIN", SDTFPBinOp, [SDNPCommutative]>;
// Doesn't matter what type the second operand of a float shift is,
// it's just a dummy for uniform instruction description
def fsrl : SDNode<"TPCISD::FSRL", SDTFPBinOp, []>;
def fsra : SDNode<"TPCISD::FSRA", SDTFPBinOp, []>;

// Mem nodes - needed to propagate memory operands from intrinsics to
// MachineInstr.
def gloadp   :  SDNode<"TPCISD::LD_G", SDTypeProfile<1, 5, []>,
                        [SDNPMayLoad, SDNPMemOperand, SDNPHasChain]>;
def gstorep  :  SDNode<"TPCISD::ST_G", SDTypeProfile<0, 5, []>,
                        [SDNPMayStore, SDNPMemOperand, SDNPHasChain]>;
def gstore_incp :  SDNode<"TPCISD::ST_G_INC", SDTypeProfile<1, 5, []>,
                        [SDNPMayStore, SDNPMemOperand, SDNPHasChain]>;

def lsstore : PatFrag<(ops node:$val, node:$ptr),
                             (st node:$val, node:$ptr), [{
  const auto *St = cast<StoreSDNode>(N);
  return St->getAddressSpace() == 1 &&
         (!St->isTruncatingStore() || !St->getMemoryVT().isFloatingPoint());
}]>;

def lvstore : PatFrag<(ops node:$val, node:$ptr),
                             (st node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getAddressSpace() == 2;
}]>;

def gstore : PatFrag<(ops node:$val, node:$ptr),
                      (unindexedstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getAddressSpace() == 3;
}]>;

def gpost_store : PatFrag<(ops node:$val, node:$ptr, node:$offset),
                          (post_store node:$val, node:$ptr, node:$offset), [{
  return cast<StoreSDNode>(N)->getAddressSpace() == 3;
}]>;

def lsload : PatFrag<(ops node:$ptr), (unindexedload node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  return cast<LoadSDNode>(N)->getAddressSpace() == 1 &&
  ExtType != ISD::ZEXTLOAD && ExtType != ISD::SEXTLOAD;
}]>;

def l_load_zext8 : PatFrag<(ops node: $ptr), (unindexedload node: $ptr), [{
      LoadSDNode *LD = cast<LoadSDNode>(N);
      ISD::LoadExtType ExtType = LD->getExtensionType();
      return LD->getMemoryVT() == MVT::i8 && ExtType == ISD::ZEXTLOAD &&
             LD->getAddressSpace() == 1;
}]>;

def l_load_sext8 : PatFrag<(ops node: $ptr), (unindexedload node: $ptr), [{
      LoadSDNode *LD = cast<LoadSDNode>(N);
      ISD::LoadExtType ExtType = LD->getExtensionType();
      return LD->getMemoryVT() == MVT::i8 && ExtType == ISD::SEXTLOAD &&
             LD->getAddressSpace() == 1;
}]>;

def l_load_zext16 : PatFrag<(ops node: $ptr), (unindexedload node: $ptr), [{
      LoadSDNode *LD = cast<LoadSDNode>(N);
      ISD::LoadExtType ExtType = LD->getExtensionType();
      return LD->getMemoryVT() == MVT::i16 && ExtType == ISD::ZEXTLOAD &&
             LD->getAddressSpace() == 1;
}]>;

def l_load_sext16 : PatFrag<(ops node: $ptr), (unindexedload node: $ptr), [{
      LoadSDNode *LD = cast<LoadSDNode>(N);
      ISD::LoadExtType ExtType = LD->getExtensionType();
      return LD->getMemoryVT() == MVT::i16 && ExtType == ISD::SEXTLOAD &&
             LD->getAddressSpace() == 1;
}]>;

def truncstore_bf16 : PatFrag<(ops node:$val, node:$ptr),
                              (truncstore node:$val, node:$ptr),
                              [{
    const auto *St = cast<StoreSDNode>(N);
    return St->getMemoryVT() == MVT::bf16 &&
           St->getAddressSpace() == 1;
}]> {
  let IsStore = 1;
  let MemoryVT = bf16;
}

def lvload : PatFrag<(ops node:$ptr), (unindexedload node:$ptr), [{
  return cast<LoadSDNode>(N)->getAddressSpace() == 2;
}]>;

def gload : PatFrag<(ops node:$ptr), (unindexedload node:$ptr), [{
  return cast<LoadSDNode>(N)->getAddressSpace() == 3;
}]>;

def DimensionToMask : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(1LL << (N->getZExtValue()), SDLoc(N), MVT::i8);
}]>;

def CastToImm32 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue(), SDLoc(N), MVT::i32);
}]>;

def FPToImm32 : SDNodeXForm<fpimm, [{
  const APFloat &Val = N->getValueAPF();
  APInt IC = Val.bitcastToAPInt();
  IC = IC.zextOrSelf(32);
  assert(IC.getBitWidth() == 32);
  return CurDAG->getTargetConstant(IC, SDLoc(N), MVT::i32);
}]>;

def AddDualGroupAllSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() & ~TPCII::SW_MDG_TYPE_MASK) | TPCII::SW_MDG_TYPE_ALL, SDLoc(N), MVT::i32);
}]>;

def AddDualGroupPackSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() & ~TPCII::SW_MDG_TYPE_MASK) | TPCII::SW_MDG_TYPE_PACK, SDLoc(N), MVT::i32);
}]>;

def AddDualGroupUnPackSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() & ~TPCII::SW_MDG_TYPE_MASK) | TPCII::SW_MDG_TYPE_UNPACK, SDLoc(N), MVT::i32);
}]>;

def GetGPtrIncSwitch : SDNodeXForm<imm, [{
  unsigned Sz;
  switch (N->getZExtValue()) {
  case 1: Sz = TPCII::SW_INC_1; break;
  case 2: Sz = TPCII::SW_INC_2; break;
  case 4: Sz = TPCII::SW_INC_4; break;
  case 8: Sz = TPCII::SW_INC_8; break;
  default:
    llvm_unreachable("Unsupported size");
    break;
  }
  return CurDAG->getTargetConstant(Sz, SDLoc(N), MVT::i32);
}]>;

def halt : SDNode<"TPCISD::HALT", SDTNone, [SDNPHasChain]>;

def SDTMove : SDTypeProfile<1, 2, [
SDTCisSameAs<0, 1>
]>;

def SDTMoveI : SDTypeProfile<1, 3, [
SDTCisSameAs<0, 1>, SDTCisSameAs<0, 3>
]>;

def cmove : SDNode<"TPCISD::COND_MOV", SDTMove, []>;
def cmoveinv : SDNode<"TPCISD::COND_MOV_INVERT", SDTMoveI, []>;

class ScalarToVector<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "i1"),     v256i1,
                   !if(!eq(!cast<string>(T), "f8_152"), v256f8_152,
                   !if(!eq(!cast<string>(T), "bf16"),   v128bf16,
                   !if(!eq(!cast<string>(T), "f32"),    v64f32,
                   !if(!eq(!cast<string>(T), "i8"),     v256i8,
                   !if(!eq(!cast<string>(T), "i16"),    v128i16,
                   !if(!eq(!cast<string>(T), "i32"),    v64i32,
                   isVoid)))))));
}

class VectorElement<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "v256i1"),   i1,
                   !if(!eq(!cast<string>(T), "v128bf16"), bf16,
                   !if(!eq(!cast<string>(T), "v256bf16"), bf16,
                   !if(!eq(!cast<string>(T), "v64f32"),   f32,
                   !if(!eq(!cast<string>(T), "v128f32"),  f32,
                   !if(!eq(!cast<string>(T), "v256f32"),  f32,
                   !if(!eq(!cast<string>(T), "v256i8"),   i8,
                   !if(!eq(!cast<string>(T), "v128i16"),  i16,
                   !if(!eq(!cast<string>(T), "v256i16"),  i16,
                   !if(!eq(!cast<string>(T), "v64i32"),   i32,
                   !if(!eq(!cast<string>(T), "v128i32"),  i32,
                   !if(!eq(!cast<string>(T), "v256i32"),  i32,
                   isVoid))))))))))));
}

class DoubleVector<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "v256i1"),     v512i1,
                   !if(!eq(!cast<string>(T), "v128bf16"),   v256bf16,
                   !if(!eq(!cast<string>(T), "v64f32"),     v128f32,
                   !if(!eq(!cast<string>(T), "v256i8"),     v512i8,
                   !if(!eq(!cast<string>(T), "v128i16"),    v256i16,
                   !if(!eq(!cast<string>(T), "v64i32"),     v128i32,
                   isVoid))))));
}

class FloatToInteger<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "f32"),        i32,
                   !if(!eq(!cast<string>(T), "bf16"),       i16,
                   !if(!eq(!cast<string>(T), "v64f32"),     v64i32,
                   !if(!eq(!cast<string>(T), "v128bf16"),   v128i16,
                   T))));
}

class OperandType<bits<4> OT> {
  string Suffix = !if(!eq(OT, OpType.FP32),   "f32",
                  !if(!eq(OT, OpType.BF16),   "bf16",
                  !if(!eq(OT, OpType.INT32),  "i32",
                  !if(!eq(OT, OpType.UINT32), "u32",
                  !if(!eq(OT, OpType.INT8),   "i8",
                  !if(!eq(OT, OpType.UINT8),  "u8",
                  !if(!eq(OT, OpType.BOOL),   "b",
                  !if(!eq(OT, OpType.INT16),  "i16",
                  !if(!eq(OT, OpType.UINT16), "u16",
                  !if(!eq(OT, OpType.INT4),   "i4",
                  !if(!eq(OT, OpType.UINT4),  "u4",
                  "")))))))))));
}

class OpTypeToScalar<bits<4> OT> {
  ValueType Type = !if(!eq(OT, OpType.FP32),   f32,
                   !if(!eq(OT, OpType.BF16),   bf16,
                   !if(!eq(OT, OpType.INT32),  i32,
                   !if(!eq(OT, OpType.UINT32), i32,
                   !if(!eq(OT, OpType.INT8),   i8,
                   !if(!eq(OT, OpType.UINT8),  i8,
                   !if(!eq(OT, OpType.BOOL),   i1,
                   !if(!eq(OT, OpType.INT16),  i16,
                   !if(!eq(OT, OpType.UINT16), i16,
                   !if(!eq(OT, OpType.INT4),   i8,
                   !if(!eq(OT, OpType.UINT4),  i8,
                   isVoid)))))))))));
}

class OpTypeToVector<bits<4> OT> {
  ValueType Type = !if(!eq(OT, OpType.FP32),    v64f32,
                   !if(!eq(OT, OpType.BF16),    v128bf16,
                   !if(!eq(OT, OpType.INT32),   v64i32,
                   !if(!eq(OT, OpType.UINT32),  v64i32,
                   !if(!eq(OT, OpType.INT8),    v256i8,
                   !if(!eq(OT, OpType.UINT8),   v256i8,
                   !if(!eq(OT, OpType.BOOL),    v256i1,
                   !if(!eq(OT, OpType.INT16),   v128i16,
                   !if(!eq(OT, OpType.UINT16),  v128i16,
                   !if(!eq(OT, OpType.INT4),    v256i8,
                   !if(!eq(OT, OpType.UINT4),   v256i8,
                   isVoid)))))))))));
}

class OpTypeForType<ValueType T> {
  bits<4> V = !if(!eq(!cast<string>(T), "f32"),       OpType.FP32,
              !if(!eq(!cast<string>(T), "bf16"),      OpType.BF16,
              !if(!eq(!cast<string>(T), "i32"),       OpType.INT32,
              !if(!eq(!cast<string>(T), "i16"),       OpType.INT16,
              !if(!eq(!cast<string>(T), "i8"),        OpType.INT8,
              !if(!eq(!cast<string>(T), "i1"),        OpType.BOOL,
              !if(!eq(!cast<string>(T), "v64f32"),    OpType.FP32,
              !if(!eq(!cast<string>(T), "v128f32"),   OpType.FP32,
              !if(!eq(!cast<string>(T), "v256f32"),   OpType.FP32,
              !if(!eq(!cast<string>(T), "v128bf16"),  OpType.BF16,
              !if(!eq(!cast<string>(T), "v256bf16"),  OpType.BF16,
              !if(!eq(!cast<string>(T), "v256f16"),   OpType.FP16,
              !if(!eq(!cast<string>(T), "v64i32"),    OpType.INT32,
              !if(!eq(!cast<string>(T), "v128i32"),   OpType.INT32,
              !if(!eq(!cast<string>(T), "v256i32"),   OpType.INT32,
              !if(!eq(!cast<string>(T), "v128i16"),   OpType.INT16,
              !if(!eq(!cast<string>(T), "v256i16"),   OpType.INT16,
              !if(!eq(!cast<string>(T), "v256i8"),    OpType.INT8,
              !if(!eq(!cast<string>(T), "v256i1"),    OpType.BOOL,
              0b1111)))))))))))))))))));
}

class SwitchValTypeForType<ValueType T> {
  int S = !if(!eq(!cast<string>(T), "f32"),       SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "bf16"),      SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "i32"),       SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "i16"),       SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "i8"),        SwitchVal.TO_INT8,
          !if(!eq(!cast<string>(T), "v64f32"),    SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v128f32"),   SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v256f32"),   SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v128bf16"),  SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "v256bf16"),  SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "v256f16"),   SwitchVal.TO_FP16,
          !if(!eq(!cast<string>(T), "v64i32"),    SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v128i32"),   SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v256i32"),   SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v128i16"),   SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "v256i16"),   SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "v256i8"),    SwitchVal.TO_INT8,
          0x7fffffff)))))))))))))))));
}

class UnsignedSwitchValForType<ValueType T> {
  int U = !if(!eq(!cast<string>(T), "i32"),       SwitchVal.TO_UINT32,
          !if(!eq(!cast<string>(T), "v64i32"),    SwitchVal.TO_UINT32,
          !if(!eq(!cast<string>(T), "v128i32"),   SwitchVal.TO_UINT32,
          !if(!eq(!cast<string>(T), "v256i32"),   SwitchVal.TO_UINT32,
          !if(!eq(!cast<string>(T), "i16"),       SwitchVal.TO_UINT16,
          !if(!eq(!cast<string>(T), "v128i16"),   SwitchVal.TO_UINT16,
          !if(!eq(!cast<string>(T), "v256i16"),   SwitchVal.TO_UINT16,
          !if(!eq(!cast<string>(T), "i8"),        SwitchVal.TO_UINT8,
          !if(!eq(!cast<string>(T), "v256i8"),    SwitchVal.TO_UINT8,
          0x7fffffff)))))))));
}

class TypeMap<ValueType T> {
  string Suffix = !if(!eq(!cast<string>(T), "v256i1"),     "m",
                  !if(!eq(!cast<string>(T), "v256i8"),     "v",
                  !if(!eq(!cast<string>(T), "v512i8"),     "d",
                  !if(!eq(!cast<string>(T), "v128i16"),    "v",
                  !if(!eq(!cast<string>(T), "v256i16"),    "d",
                  !if(!eq(!cast<string>(T), "v256f16"),    "d",
                  !if(!eq(!cast<string>(T), "v128bf16"),   "v",
                  !if(!eq(!cast<string>(T), "v256bf16"),   "d",
                  !if(!eq(!cast<string>(T), "v64i32"),     "v",
                  !if(!eq(!cast<string>(T), "v128i32"),    "d",
                  !if(!eq(!cast<string>(T), "v256i32"),    "a",
                  !if(!eq(!cast<string>(T), "v64f32"),     "v",
                  !if(!eq(!cast<string>(T), "v128f32"),    "d",
                  !if(!eq(!cast<string>(T), "v256f32"),    "a",
                  !if(!eq(!cast<string>(T), "v5i32"),      "I",
                  !if(!eq(!cast<string>(T), "iAny"),       "i",  // Hack to encode immediate
                  !if(!eq(!cast<string>(T), "i64"),        "a",
                  !if(!eq(!cast<string>(T), "bf16"),       "s",
                  !if(!eq(!cast<string>(T), "f32"),        "s",
                  !if(!eq(!cast<string>(T), "i1"),         "p",
                  !if(!eq(!cast<string>(T), "i8"),         "s",
                  !if(!eq(!cast<string>(T), "i16"),        "s",
                  !if(!eq(!cast<string>(T), "i32"),        "s", "")))))))))))))))))))))));
}

class ConvertCompatibleTypes<ValueType T> {
  list<ValueType> Ts = !if(!eq(!cast<string>(T), "v64f32"),    [v64i32],
                       !if(!eq(!cast<string>(T), "v128f32"),   [v128i32],
                       !if(!eq(!cast<string>(T), "v256f32"),   [v256i32],
                       !if(!eq(!cast<string>(T), "v256bf16"),  [v256i16],
                       !if(!eq(!cast<string>(T), "v256f16"),   [v256i16],
                       !if(!eq(!cast<string>(T), "v64i32"),    [v64f32],
                       !if(!eq(!cast<string>(T), "v128i32"),   [v128f32],
                       !if(!eq(!cast<string>(T), "v256i32"),   [v256f32],
                       !if(!eq(!cast<string>(T), "v128i16"),   [v128bf16],
                       !if(!eq(!cast<string>(T), "v256i16"),   [v256bf16],
                       !if(!eq(!cast<string>(T), "f32"),       [i32],
                       !if(!eq(!cast<string>(T), "bf16"),      [i16],
                       !if(!eq(!cast<string>(T), "i32"),       [f32],
                       !if(!eq(!cast<string>(T), "i16"),       [bf16],
                       []))))))))))))));
}

class PredTypeForPredStr<string Pred> {
  ValueType P = !if(!eq(Pred, "SP0"), i1,
                !if(!eq(Pred, "VP0"), v256i1,
                i1));
}

class ConstructInsn<string BaseName, ValueType T1, ValueType T2 = isVoid, ValueType T3 = isVoid, ValueType T4 = isVoid, ValueType T5 = isVoid> {
  string Name = !strconcat(BaseName,
                           TypeMap<T1>.Suffix,
                           !if(!eq(!cast<string>(T2), "isVoid"), "", TypeMap<T2>.Suffix),
                           !if(!eq(!cast<string>(T3), "isVoid"), "", TypeMap<T3>.Suffix),
                           !if(!eq(!cast<string>(T4), "isVoid"), "", TypeMap<T4>.Suffix),
                           !if(!eq(!cast<string>(T5), "isVoid"), "", TypeMap<T5>.Suffix));
  Instruction Insn = !cast<Instruction>(Name);
}

class ConstructConvertInsn<ValueType T1, ValueType T2, ValueType pred> {
  string suffix_T1 = TypeMap<T1>.Suffix;
  string suffix_T2 = TypeMap<T2>.Suffix;
  string Name = !strconcat("CONVERT", suffix_T1, suffix_T2, TypeMap<pred>.Suffix);
  Instruction Insn = !cast<Instruction>(Name);
}

// ------------------------- Broadcast patterns --------------------------------

def vsplatf32 : PatFrag<(ops node:$e0),
                        (v64f32 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplatbf16 : PatFrag<(ops node:$e0),
                       (v128bf16 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplati32  : PatFrag<(ops node:$e0),
                        (v64i32 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                             node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplati16 : PatFrag<(ops node:$e0),
                        (v128i16 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                               node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplati8  : PatFrag<(ops node:$e0),
                        (v256i8 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplatb  : PatFrag<(ops node:$e0),
                        (v256i1 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
                                              node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

def vsplati5  : PatFrag<(ops node:$e0),
                        (v5i32 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0))>;

class SplatMap<bits<4> OT> {
  PatFrag S = !if(!eq(OT, OpType.FP32),    vsplatf32,
              !if(!eq(OT, OpType.BF16),    vsplatbf16,
              !if(!eq(OT, OpType.INT32),   vsplati32,
              !if(!eq(OT, OpType.UINT32),  vsplati32,
              !if(!eq(OT, OpType.INT8),    vsplati8,
              !if(!eq(OT, OpType.UINT8),   vsplati8,
              !if(!eq(OT, OpType.BOOL),    vsplatb,
              !if(!eq(OT, OpType.INT16),   vsplati16,
              !if(!eq(OT, OpType.UINT16),  vsplati16, vsplati32)))))))));
}

class OpTypeProps<bits<4> Type> {
  ValueType   Arg   = OpTypeToVector<Type>.Type;
  ValueType   Scal  = OpTypeToScalar<Type>.Type;
  SDNodeXForm Cast  = !if(!eq(Type, OpType.FP32), FPToImm32,
                      !if(!eq(Type, OpType.BF16), FPToImm32,
                      !if(!eq(Type, OpType.FP16), FPToImm32,
                      !if(!eq(Type, OpType.FP8_143), FPToImm32,
                      !if(!eq(Type, OpType.FP8_152), FPToImm32,
                      CastToImm32)))));
  PatFrag     Splat = SplatMap<Type>.S;
}

class SplatNode<ValueType T> {
  PatFrag Splat = !if(!eq(!cast<string>(T), "v256i1"),     vsplatb,
                  !if(!eq(!cast<string>(T), "v128bf16"),   vsplatbf16,
                  !if(!eq(!cast<string>(T), "v64f32"),     vsplatf32,
                  !if(!eq(!cast<string>(T), "v256i8"),     vsplati8,
                  !if(!eq(!cast<string>(T), "v128i16"),    vsplati16,
                  !if(!eq(!cast<string>(T), "v64i32"),     vsplati32, vsplati32))))));
}

class ImmNode<ValueType T> {
  SDNode Node = !if(!eq(!cast<string>(T), "f32"),        fpimm,
                !if(!eq(!cast<string>(T), "bf16"),       fpimm,
                !if(!eq(!cast<string>(T), "v64f32"),     fpimm,
                !if(!eq(!cast<string>(T), "v128bf16"),   fpimm,
                imm))));
}

class CastOp<ValueType T> {
  SDNodeXForm Cast = !if(!eq(!cast<string>(T), "v64f32"),     FPToImm32,
                     !if(!eq(!cast<string>(T), "v128bf16"),   FPToImm32,
                     !if(!eq(!cast<string>(T), "f32"),        FPToImm32,
                     !if(!eq(!cast<string>(T), "bf16"),       FPToImm32,
                     CastToImm32))));
}

// Represents 1-bit constant 'true'.
def i1true : PatLeaf<(i1 imm:$val) ,[{
  return cast<ConstantSDNode>(N)->getZExtValue() == 1;
}]>;

//------------------------------------------------------------------------------

let isNotUsedInDisasm = 0, isBranch = 1, isTerminator = 1, hasCtrlDep = 1, Defs = [S32, S33, S34, S35] in {
  def LOOPsss  :  LOOPInstRRR<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPssi  :  LOOPInstRRI<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPsis  :  LOOPInstRIR<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPiss  :  LOOPInstIRR<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPsii  :  LOOPInstRII<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPisi  :  LOOPInstIRI<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPiis  :  LOOPInstIIR<0b100010, "loop $start, $boundary, $step, $cmp, $target">;
  def LOOPiii  :  LOOPInstIII<0b100010, "loop $start, $boundary, $step, $cmp, $target">;

  // Predicated variants
  def LOOPsssp : LoopInstRRRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPssip : LoopInstRRIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPsisp : LoopInstRIRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPissp : LoopInstIRRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPsiip : LoopInstRIIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPisip : LoopInstIRIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPiisp : LoopInstIIRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;
  def LOOPiiip : LoopInstIIIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $pred">;

  // Predicated variants with REPEAT_AT_MOST_ONCE operand
  def LOOP1sssp : LoopInstRRRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1ssip : LoopInstRRIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1sisp : LoopInstRIRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1issp : LoopInstIRRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1siip : LoopInstRIIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1isip : LoopInstIRIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1iisp : LoopInstIIRp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
  def LOOP1iiip : LoopInstIIIp<0b100010, "loop $start, $boundary, $step, $cmp, $target, $rep1, $pred", 1>;
}

let isBranch = 1, hasCtrlDep = 1, isTerminator = 1, isPseudo = 1, Defs = [S32, S33, S34, S35],
  AsmVariantName = "special" in {
  def LOOPEND : TPCInstLOOPEND<0, "LOOPEND">;
}

multiclass SPILL_REG <RegisterClass RC> {
  def _SAVE : TPCInstPseudo<(outs),
                            (ins i32imm:$offset, RC:$reg),
                            "SPILL_SAVE"> {
    let mayStore = 1;
    let mayLoad = 0;
  }
  def _RESTORE : TPCInstPseudo<(outs RC:$reg),
                               (ins i32imm:$offset),
                               "SPILL_RESTORE"> {
    let mayStore = 0;
    let mayLoad = 1;
  }
}

defm SPILL_SRF  : SPILL_REG<SRF>;
defm SPILL_SPRF : SPILL_REG<SPRF>;
defm SPILL_IRF  : SPILL_REG<IRF>;
defm SPILL_VRF  : SPILL_REG<VRF>;
defm SPILL_VPRF : SPILL_REG<VPRF>;
defm SPILL_ZRF  : SPILL_REG<ZRF>;
defm SPILL_DRF  : SPILL_REG<DRF>;
defm SPILL_ARF  : SPILL_REG<ARF>;

def TPC_EOF : TPCInstPseudo<(outs), (ins), "EOF">;


def ReadLFSR : VPUInstReadSpec<0b001000, VRF, "READ\t$dst, LFSR",
                               [(set v256i8:$dst, (int_tpc_read_lfsr))],
                               OpType.UINT8> {
  let hasSideEffects = 1;
  let src = 40; // LFSR
  let Defs = [LFSR];
  let Uses = [LFSR];
}
def ReadLFSRNC : VPUInstReadSpec<0b001000, VRF, "READ\t$dst, LFSR_NO_CHANGE",
                                 [(set v256i8:$dst, (int_tpc_read_lfsrnc))],
                                 OpType.UINT8> {
  let src = 41; // LFSR_NO_CHANGE
  let Uses = [LFSR];
}
def WriteLFSR : VPUInstWriteSpec<0b001000, VRF, "WRITE\tLFSR, $src",
                                 [(int_tpc_write_lfsr v256i8:$src)],
                                 OpType.UINT8> {
  let hasSideEffects = 1;
  let dst = 40; // LFSR
  let Defs = [LFSR];
}

// MOV from S40 on SPU slot does not reseed S40
// Workaround:
// 1. use MOV to S40 on LOAD slot – it will reseed properly.
// 2. use MOV to S40 on SPU slot, followed by fake CONVERT with SR which
//    will cause reseed of S40.
def ReadSLFSR : LoadInstReadSpec<0b000101, SRF, "READ\t$dst, S_LFSR",
                               [(set i32:$dst, (int_tpc_s_read_lfsr))]> {
  let hasSideEffects = 1;
  let src = 104; // S_LFSR
  let Defs = [S_LFSR];
  let Uses = [S_LFSR];
}
def ReadSLFSRNC : LoadInstReadSpec<0b000101, SRF, "READ\t$dst, S_LFSR_NO_CHANGE",
                                 [(set i32:$dst, (int_tpc_s_read_lfsrnc))]> {
  let src = 105; // S_LFSR_NO_CHANGE
  let Uses = [S_LFSR];
}
def WriteSLFSR : LoadInstWriteSpec<0b000101, SRF, "WRITE\tS_LFSR, $src",
                                 [(int_tpc_write_lfsr_s i32:$src)]> {
  let hasSideEffects = 1;
  let dst = 104; // S_LFSR
  let Defs = [S_LFSR];
}

let hasSideEffects = 1 in
  def MOVnodce : SPUInstRI<0b001000, SRF, i32imm, "mov.i32\t$dst, $src", [], OpType.INT32>;

//------ Special instructions --------------------------------------------------

let isReturn = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 1 in {
  def HALTs : SpuInstUncond<0b100000, "halt", [(halt)]>;
  def HALTv : VpuInstUncond<0b100000, "halt">;
}

def DBGs  : SpuInstUncond<0b111110, "dbg">;
def DBGv  : VpuInstUncond<0b111110, "dbg">;

def NOPs  : SpuInstUncond<0b111111, "nop">;
def NOPv  : VpuInstUncond<0b111111, "nop">;
def NOPld : LoadInstUncond<0b011111, "nop">;
def NOPst : StoreInstUncond<0b011111, "nop">;


//------ Helpers ---------------------------------------------------------------

multiclass LoadMapVRF<PatFrag LdOp, dag Addr, SDNode Insn> {
  def _v256i8   : Pat<(v256i8   (LdOp Addr)),
                      (v256i8   (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _v128i16  : Pat<(v128i16  (LdOp Addr)),
                      (v128i16  (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _v64i32   : Pat<(v64i32   (LdOp Addr)),
                      (v64i32   (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _v64f32   : Pat<(v64f32   (LdOp Addr)),
                      (v64f32   (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _v128bf16 : Pat<(v128bf16 (LdOp Addr)),
                      (v128bf16 (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
}

multiclass LoadMapSRF<PatFrag LdOp, dag Addr, SDNode Insn> {
  def _i8   : Pat<(i8   (LdOp Addr)),
                  (i8   (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _i16  : Pat<(i16  (LdOp Addr)),
                  (i16  (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _i32  : Pat<(i32  (LdOp Addr)),
                  (i32  (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _f32  : Pat<(f32  (LdOp Addr)),
                  (f32  (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def _bf16 : Pat<(bf16 (LdOp Addr)),
                  (bf16 (Insn Addr, (IMPLICIT_DEF), SP0, (i1 0)))>;
}

// Mappin of unary operation on integer scalar.
multiclass UnaryIntegerScalar<PatFrag Op, SDNode InstrS> {
  def i32s  : Pat<(Op i32:$op), (InstrS $op, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16s  : Pat<(Op i16:$op), (InstrS $op, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8s   : Pat<(Op i8:$op),  (InstrS $op, OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of unary operation on float scalar.
multiclass UnaryFloatScalar<PatFrag Op, SDNode InstrS> {
  def f32s  : Pat<(Op f32:$op),  (InstrS $op, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def bf16s : Pat<(Op bf16:$op), (InstrS $op, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
}

// Mapping of binary operation on integer signed scalars.
multiclass BinarySignedScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def i32s  : Pat<(Op i32:$op1, i32:$op2), (InstrSS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i32i  : Pat<(Op i32:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16s  : Pat<(Op i16:$op1, i16:$op2), (InstrSS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16i  : Pat<(Op i16:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8s   : Pat<(Op i8:$op1,  i8:$op2),  (InstrSS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8i   : Pat<(Op i8:$op1,  imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of binary operation on integer signed scalars for shift operations.
// In this case the second operand is always i32.
multiclass BinaryShiftScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def i32s  : Pat<(Op i32:$op1, i32:$op2),       (InstrSS $op1, $op2,           OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i32i  : Pat<(Op i32:$op1, (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16s  : Pat<(Op i16:$op1, i32:$op2),       (InstrSS $op1, $op2,           OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16i  : Pat<(Op i16:$op1, (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8s   : Pat<(Op i8:$op1,  i32:$op2),       (InstrSS $op1, $op2,           OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8i   : Pat<(Op i8:$op1,  (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of binary operation on integer unsigned scalars.
multiclass BinaryUnsignedScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def u32s  : Pat<(Op i32:$op1, i32:$op2), (InstrSS $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def u32i  : Pat<(Op i32:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def u16s  : Pat<(Op i16:$op1, i16:$op2), (InstrSS $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def u16i  : Pat<(Op i16:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def u8s   : Pat<(Op i8:$op1,  i8:$op2),  (InstrSS $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def u8i   : Pat<(Op i8:$op1,  imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of binary operation on float scalars.
multiclass BinaryFloatScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def f32s : Pat<(Op f32:$op1, f32:$op2),         (InstrSS $op1, $op2,             OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def f32i : Pat<(Op f32:$op1, (f32 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  let Predicates = [BF16] in {
    def bf16s : Pat<(Op bf16:$op1, bf16:$op2),         (InstrSS $op1, $op2,             OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
    def bf16i : Pat<(Op bf16:$op1, (bf16 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  }
}

// Mapping of binary operation on index values. Scalar variants have scalar
// argument as the second operand (SHL/SHR).
multiclass BinaryIndexScalar<PatFrag Op, SDNode InstrNS, SDNode InstrNI> {
  def s : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 i32:$op2))),
              (InstrNS $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 (i32 imm:$op2)))),
              (InstrNI $op1, (CastToImm32 $op2), (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of binary operation on index values. Scalar variants have immediate
// scalar argument as the second operand (CMP_*).
multiclass CmpIndexOp<PatFrag Op, SDNode InstrNI> {
  def i : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 (i32 imm:$op2)))),
              (InstrNI $op1, (CastToImm32 $op2), (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of binary operation on index values. Scalar variants have scalar
// argument as the first operand (as ADD, SUB etc).
multiclass BinaryIndexOp1<PatFrag Op, SDNode InstrNN, SDNode InstrNS, SDNode InstrNI> {
  def I : Pat<(Op v5i32:$op1, v5i32:$op2),
              (InstrNN $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def s : Pat<(Op (v5i32 (vsplati5 i32:$op1)), v5i32:$op2),
              (InstrNS $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i : Pat<(Op (v5i32 (vsplati5 (i32 imm:$op1))), v5i32:$op2),
              (InstrNI (CastToImm32 $op1), $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of unary operation on integer vector.
multiclass UnaryIntegerVector<PatFrag Op, SDNode InstrV> {
  def i32v : Pat<(Op v64i32:$op),  (InstrV $op, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16v : Pat<(Op v128i16:$op), (InstrV $op, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8v  : Pat<(Op v256i8:$op),  (InstrV $op, OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

// Mapping of unary operation on float vector.
multiclass UnaryFloatVector<PatFrag Op, SDNode InstrV> {
  def f32v  : Pat<(Op v64f32:$op),   (InstrV $op, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def bf16v : Pat<(Op v128bf16:$op), (InstrV $op, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
}

// Mapping of binary operation on integer vectors.
multiclass BinaryIntegerVector<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(Op v64i32:$op1,  v64i32:$op2                         ), (InstrVV $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i32vs  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       ), (InstrVS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i32vi  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16vv  : Pat<(Op v128i16:$op1, v128i16:$op2                        ), (InstrVV $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16vs  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      ), (InstrVS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i16vi  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2)))), (InstrVI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8vv   : Pat<(Op v256i8:$op1,  v256i8:$op2                         ), (InstrVV $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8vs   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         ), (InstrVS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def i8vi   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

multiclass BinOpSwSVectVP0<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(Op v64i32:$op1,  v64i32:$op2                         ), (InstrVV $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i32vs  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       ), (InstrVS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i32vi  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vv  : Pat<(Op v128i16:$op1, v128i16:$op2                        ), (InstrVV $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vs  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      ), (InstrVS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vi  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2)))), (InstrVI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vv   : Pat<(Op v256i8:$op1,  v256i8:$op2                         ), (InstrVV $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vs   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         ), (InstrVS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vi   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
}

multiclass BinOpSwUVectVP0<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(Op v64i32:$op1,  v64i32:$op2                         ), (InstrVV $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i32vs  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       ), (InstrVS $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i32vi  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) ), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT32, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vv  : Pat<(Op v128i16:$op1, v128i16:$op2                        ), (InstrVV $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vs  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      ), (InstrVS $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i16vi  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2)))), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT16, (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vv   : Pat<(Op v256i8:$op1,  v256i8:$op2                         ), (InstrVV $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vs   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         ), (InstrVS $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
  def i8vi   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   ), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT8,  (i32 0), (IMPLICIT_DEF), VP0, (i1 0))>;
}


// Mapping of binary operation on float vectors.
multiclass BinaryFloatVector<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def f32vv  : Pat<(Op v64f32:$op1,   v64f32:$op2                          ), (InstrVV $op1, $op2,             OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def f32vs  : Pat<(Op v64f32:$op1,   (v64f32 (vsplatf32 f32:$op2))        ), (InstrVS $op1, $op2,             OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def f32vi  : Pat<(Op v64f32:$op1,   (v64f32 (vsplatf32 (f32 fpimm:$op2)))), (InstrVI $op1, (FPToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  let Predicates = [BF16] in {
    def bf16vv  : Pat<(Op v128bf16:$op1, v128bf16:$op2                            ), (InstrVV $op1, $op2,             OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
    def bf16vs  : Pat<(Op v128bf16:$op1, (v128bf16 (vsplatbf16 bf16:$op2))        ), (InstrVS $op1, $op2,             OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
    def bf16vi  : Pat<(Op v128bf16:$op1, (v128bf16 (vsplatbf16 (bf16 fpimm:$op2)))), (InstrVI $op1, (FPToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  }
}

multiclass LoadInst_LD_L_V<bits<6> opc, string asm_string> {
  def vssp  : LoadInst_LD_Vrr<opc, VRF,  asm_string, SPred>, Requires<[Addr2]>;
  def vssm  : LoadInst_LD_Vrr<opc, VRF,  asm_string, VPred>, Requires<[Addr2]>;
  def vsip  : LoadInst_LD_Vri<opc, VRF,  asm_string, SPred>, Requires<[Addr2]>;
  def vsim  : LoadInst_LD_Vri<opc, VRF,  asm_string, VPred>, Requires<[Addr2]>;
  def mssp  : LoadInst_LD_Vrr<opc, VPRF, asm_string, SPred>, Requires<[Addr2]>;
  def mssm  : LoadInst_LD_Vrr<opc, VPRF, asm_string, VPred>, Requires<[Addr2]>;
  def msip  : LoadInst_LD_Vri<opc, VPRF, asm_string, SPred>, Requires<[Addr2]>;
  def msim  : LoadInst_LD_Vri<opc, VPRF, asm_string, VPred>, Requires<[Addr2]>;

  def vsp   : LoadInst_LD_Vr<opc, VRF,  asm_string, SPred>, Requires<[Addr1]>;
  def vsm   : LoadInst_LD_Vr<opc, VRF,  asm_string, VPred>, Requires<[Addr1]>;
  def vip   : LoadInst_LD_Vi<opc, VRF,  asm_string, SPred>, Requires<[Addr1]>;
  def vim   : LoadInst_LD_Vi<opc, VRF,  asm_string, VPred>, Requires<[Addr1]>;
  def msp   : LoadInst_LD_Vr<opc, VPRF, asm_string, SPred>, Requires<[Addr1]>;
  def msm   : LoadInst_LD_Vr<opc, VPRF, asm_string, VPred>, Requires<[Addr1]>;
  def mip   : LoadInst_LD_Vi<opc, VPRF, asm_string, SPred>, Requires<[Addr1]>;
  def mim   : LoadInst_LD_Vi<opc, VPRF, asm_string, VPred>, Requires<[Addr1]>;
}

multiclass StoreInst_ST_L_V<bits<6> opc, string asm_string> {
  def ssvp  : StoreInst_ST_Vrrr<opc, VRF,  asm_string, SPred>, Requires<[Addr2]>;
  def sivp  : StoreInst_ST_Vrir<opc, VRF,  asm_string, SPred>, Requires<[Addr2]>;
  def ssmp  : StoreInst_ST_Vrrr<opc, VPRF, asm_string, SPred>, Requires<[Addr2]>;
  def simp  : StoreInst_ST_Vrir<opc, VPRF, asm_string, SPred>, Requires<[Addr2]>;

  def svp   : StoreInst_ST_Vrr<opc, VRF,  asm_string, SPred>, Requires<[Addr1]>;
  def ivp   : StoreInst_ST_Vir<opc, VRF,  asm_string, SPred>, Requires<[Addr1]>;
  def smp   : StoreInst_ST_Vrr<opc, VPRF, asm_string, SPred>, Requires<[Addr1]>;
  def imp   : StoreInst_ST_Vir<opc, VPRF, asm_string, SPred>, Requires<[Addr1]>;
}

// Mapping of intrinsics of binary operation on index vectors. Scalar variants
// have scalar argument as the first operand (as ADD, SUB etc).
multiclass BinaryIndexIntrinsicsOp1<SDNode Intrin, string IName, ValueType RT = v5i32> {
  def II:  Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, v5i32, v5i32, i1>.Insn
                    v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def sI:  Pat<(RT (Intrin (v5i32 (vsplati5 i32:$x0)), v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def iI:  Pat<(RT (Intrin (v5i32 (vsplati5 (i32 imm:$x0))), v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (CastToImm32 $x0), v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def sI2: Pat<(RT (Intrin i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def iI2: Pat<(RT (Intrin (i32 imm:$x0), v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (CastToImm32 $x0), v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;

}

// Additional mapping intrinsics of commutative binary operation on index vectors.
// The instructions corresponding to these intrinsics expect scalar argument to
// be the first one. In this mappings it is allowed to be the second argument.
multiclass BinaryIndexIntrinsicsOp1Commutative<SDNode Intrin, string IName, ValueType RT = v5i32> {
  def Is:  Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 i32:$x1)), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x1, v5i32:$x0, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii:  Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (CastToImm32 $x1), v5i32:$x0, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Is2: Pat<(RT (Intrin v5i32:$x0, i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x1, v5i32:$x0, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (CastToImm32 $x1), v5i32:$x0, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;

}

// Mapping of intrinsics of binary operation on index vectors. Scalar variants
// have scalar argument as the second operand (SHL/SHR).
multiclass BinaryIndexShiftIntrinsics<SDNode Intrin, string IName, ValueType RT = v5i32> {
  def II:  Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "IIp"))
                    v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Is:  Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 i32:$x1)), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Isp"))
                    v5i32:$x0, i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii:  Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Iip"))
                    v5i32:$x0, (CastToImm32 $x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Is2: Pat<(RT (Intrin v5i32:$x0, i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Isp"))
                    v5i32:$x0, i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Iip"))
                    v5i32:$x0, (CastToImm32 $x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// Mapping of intrinsics of binary operation on index vectors. Scalar variants
// have immediate scalar argument as the second operand (CMP_*).
multiclass IndexCmpIntrinsic<SDNode Intrin, string IName, ValueType RT = v5i32> {
  def II:  Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "IIp"))
                    v5i32:$x0, v5i32:$x1, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii:  Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Iip"))
                    v5i32:$x0, (CastToImm32 $x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def Ii2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (!cast<Instruction>(!strconcat(IName, "Iip"))
                    v5i32:$x0, (CastToImm32 $x1), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;

  def wII: Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                (RT (!cast<Instruction>(!strconcat(IName, "wIIp"))
                    v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def wIi: Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                (RT (!cast<Instruction>(!strconcat(IName, "wIip"))
                    v5i32:$x0, (CastToImm32 $x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def wIi2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                (RT (!cast<Instruction>(!strconcat(IName, "wIip"))
                    v5i32:$x0, (CastToImm32 $x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// Mapping of intrinsics of unary operation on index vectors.
multiclass UnaryIndexIntrinsics<SDNode Intrin, string IName, ValueType RT = v5i32> {
  // Constant mask
  def : Pat<(v5i32 (Intrin v5i32:$src, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<IName, v5i32, v5i32, i1>.Insn
                    IRF:$src, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v5i32 (Intrin (v5i32 (vsplati5 i32:$src)), (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<IName, v5i32, i32, i1>.Insn
                    SRF:$src, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v5i32 (Intrin i32:$src, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<IName, v5i32, i32, i1>.Insn
                    SRF:$src, (i32 imm:$mask), (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

multiclass BinaryScalarIntrinsics<SDNode Intrin, string IName, ValueType T, ValueType RT = T> {
  def : Pat<(RT (Intrin T:$x0, T:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, T, i1>.Insn T:$x0, T:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin T:$x0, (T ImmNode<T>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, i1>.Insn T:$x0, (CastOp<T>.Cast $x1), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// Process intrinsics like 'tpc_add' operation on v5i32 type. For this type we
// should use another intrinsic, like 'tpc_add_mask', but the former usage is
// still in use. In such case mask is represented by 'switchset' argument and
// may be only a constant, in contrast to 'tpc_add_mask'.
multiclass BinaryIndexIntrinsicsCompat<SDNode Intrin, string IName, ValueType RT = v5i32> {
  def II: Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
              (RT (ConstructInsn<IName, v5i32, v5i32, i1>.Insn
                   v5i32:$x0, v5i32:$x1, (i32 imm:$sw), (i8 imm:$dt), (i32 0), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def sI: Pat<(RT (Intrin (v5i32 (vsplati5 i32:$x0)), v5i32:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x0, v5i32:$x1, (i32 imm:$sw), (i8 imm:$dt), (i32 0), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def iI:  Pat<(RT (Intrin (v5i32 (vsplati5 (i32 imm:$x0))), v5i32:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (CastToImm32 $x0), v5i32:$x1, (i32 imm:$sw), (i8 imm:$dt), (i32 0), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def sI2: Pat<(RT (Intrin i32:$x0, v5i32:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, i32, v5i32, i1>.Insn
                    i32:$x0, v5i32:$x1, (i32 imm:$sw), (i8 imm:$dt), (i32 0), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def iI2: Pat<(RT (Intrin (i32 imm:$x0), v5i32:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
               (RT (ConstructInsn<IName, iAny, v5i32, i1>.Insn
                    (i32 imm:$x0), v5i32:$x1, (i32 imm:$sw), (i8 imm:$dt), (i32 0), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
}

multiclass BinaryVectorIntrinsics<SDNode Intrin, string IName, ValueType T, ValueType P, ValueType RT = T> {
  def : Pat<(RT (Intrin
                 T:$op1,
                 T:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, T, P>.Insn
                 T:$op1,
                 T:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T:$op1,
                 (T (SplatNode<T>.Splat VectorElement<T>.Type:$op2)),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, VectorElement<T>.Type, P>.Insn
                 T:$op1,
                 VectorElement<T>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T:$op1,
                 VectorElement<T>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, VectorElement<T>.Type, P>.Insn
                 T:$op1,
                 VectorElement<T>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T:$op1,
                 (T (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$op2))),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, P>.Insn
                 T:$op1,
                 (CastOp<T>.Cast $op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T:$op1,
                 (VectorElement<T>.Type ImmNode<T>.Node:$op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, P>.Insn
                 T:$op1,
                 (CastOp<T>.Cast $op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 (VectorElement<T>.Type ImmNode<T>.Node:$op2),
                 T:$op1,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, P>.Insn
                 T:$op1,
                 (CastOp<T>.Cast $op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
}

multiclass ShiftVectorIntrinsics<SDNode Intrin, string IName, ValueType T1, ValueType T2, ValueType P, ValueType RT = T1> {
  def : Pat<(RT (Intrin
                 T1:$op1,
                 T2:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, T2, P>.Insn
                 T1:$op1,
                 T2:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (T2 (SplatNode<T2>.Splat VectorElement<T2>.Type:$op2)),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, VectorElement<T2>.Type, P>.Insn
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, VectorElement<T2>.Type, P>.Insn
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (T2 (SplatNode<T2>.Splat (VectorElement<T2>.Type ImmNode<T2>.Node:$op2))),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, iAny, P>.Insn
                 T1:$op1,
                 (CastOp<T2>.Cast $op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (VectorElement<T2>.Type ImmNode<T2>.Node:$op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, iAny, P>.Insn
                 T1:$op1,
                 (CastOp<T2>.Cast $op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
}

//------ Move instructions -----------------------------------------------------

// Load slot.

def MOV_ld_vvp : LoadInst_Mov <0b000101, VRF,  VRF,    "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvvp">;
def MOV_ld_vvm : LoadInst_Mov <0b000101, VRF,  VRF,    "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvvm">;
def MOV_ld_mmp : LoadInst_Mov <0b000101, VPRF, VPRF,   "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmmp">;
def MOV_ld_mmm : LoadInst_Mov <0b000101, VPRF, VPRF,   "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmmm">;
def MOV_ld_mpp : LoadInst_Mov <0b000101, VPRF, SPRF,   "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmpp">;
def MOV_ld_mpm : LoadInst_Mov <0b000101, VPRF, SPRF,   "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmpm">;

def MOV_ld_msp  : LoadInst_MovFlavor<0b000101, VPRF, SRF,            "mov $sw $flavor $dest, $src, $pred", SPred>, MultiSlot<"MOVmsp">;
def MOV_ld_msm  : LoadInst_MovFlavor<0b000101, VPRF, SRF,            "mov $sw $flavor $dest, $src, $pred", VPred>, MultiSlot<"MOVmsm">;
def MOV_ld_mip  : LoadInst_MovFlavor<0b000101, VPRF, TPCImm<i32imm>, "mov $sw $flavor $dest, $src, $pred", SPred>, MultiSlot<"MOVmip">;
def MOV_ld_mim  : LoadInst_MovFlavor<0b000101, VPRF, TPCImm<i32imm>, "mov $sw $flavor $dest, $src, $pred", VPred>, MultiSlot<"MOVmim">;

def MOVB_ld_msp : LoadInst_MovFlavorBroadcast<0b000101, VPRF, SRF,            "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVBmsp">;
def MOVB_ld_msm : LoadInst_MovFlavorBroadcast<0b000101, VPRF, SRF,            "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVBmsm">;
def MOVB_ld_mip : LoadInst_MovFlavorBroadcast<0b000101, VPRF, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVBmip">;
def MOVB_ld_mim : LoadInst_MovFlavorBroadcast<0b000101, VPRF, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVBmim">;

def MOV_ld_vsp : LoadInst_MovType <0b000101, VRF, SRF,    "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvsp">;
def MOV_ld_vsm : LoadInst_MovType <0b000101, VRF, SRF,    "mov$optype $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvsm">;
def MOV_ld_vip : LoadInst_MovTypeI<0b000101, VRF, i32imm, "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvip">;
def MOV_ld_vim : LoadInst_MovTypeI<0b000101, VRF, i32imm, "mov$optype $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvim">;

def MOV_ld_Isp  : LoadInst_MovMask <0b000101, IRF,  SRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIsp">;
def MOV_ld_Iip  : LoadInst_MovMaskI<0b000101, IRF,  i32imm, "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIip">;
def MOV_ld_IIp  : LoadInst_MovMask <0b000101, IRF,  IRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIIp">;

def MOV_ld_ssp  : LoadInst_MovType <0b000101, SRF,  SRF,    "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVssp">;
def MOV_ld_sip  : LoadInst_MovTypeI<0b000101, SRF,  i32imm, "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVsip">;
def MOV_ld_psp  : LoadInst_Mov     <0b000101, SPRF, SRF,    "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVpsp">;
def MOV_ld_pip  : LoadInst_MovI    <0b000101, SPRF, i32imm, "mov $sw $dest, $src, $pred">, MultiSlot<"MOVpip">;

// Compatibility instructions.
let isAsmParserOnly = 1 in {
  def MOV_ld_tvvp : LoadInst_MOVnotype<0b000101, VRF,  VRF,  "mov$optype $sw $dest, $src, $pred", SPred>;
  def MOV_ld_tvvm : LoadInst_MOVnotype<0b000101, VRF,  VRF,  "mov$optype $sw $dest, $src, $pred", VPred>;
  def MOV_ld_tmmp : LoadInst_MOVnotype<0b000101, VPRF, VPRF, "mov$optype $sw $dest, $src, $pred", SPred>;
  def MOV_ld_tmmm : LoadInst_MOVnotype<0b000101, VPRF, VPRF, "mov$optype $sw $dest, $src, $pred", VPred>;
  def MOV_ld_tmpp : LoadInst_MOVnotype<0b000101, VPRF, SPRF, "mov$optype $sw $dest, $src, $pred", SPred>;
  def MOV_ld_tmpm : LoadInst_MOVnotype<0b000101, VPRF, SPRF, "mov$optype $sw $dest, $src, $pred", VPred>;

  def MOV_ld_tmsp : LoadInst_MovFlavorT<0b000101, VPRF, SRF,    "mov$optype $sw $flavor $dest, $src, $pred", SPred>;
  def MOV_ld_tmsm : LoadInst_MovFlavorT<0b000101, VPRF, SRF,    "mov$optype $sw $flavor $dest, $src, $pred", VPred>;
  def MOV_ld_tmip : LoadInst_MovFlavorT<0b000101, VPRF, TPCImm<i32imm>, "mov$optype $sw $flavor $dest, $src, $pred", SPred>;
  def MOV_ld_tmim : LoadInst_MovFlavorT<0b000101, VPRF, TPCImm<i32imm>, "mov$optype $sw $flavor $dest, $src, $pred", VPred>;

  def MOV_ld_nIIp : LoadInst_MovIRF <0b000101, IRF,  IRF, "mov $sw $dest, $src, $pred">;
  def MOV_ld_tIIp : LoadInst_MovIRFT<0b000101, IRF,  IRF, "mov$optype $sw $dest, $src, $pred">;

  def MOV_ldc_psp : LoadInst_MovType <0b000101, SPRF, SRF,    "mov$optype $sw $dest, $src, $pred", SPred>;
  def MOV_ldc_pip : LoadInst_MovTypeI<0b000101, SPRF, i32imm, "mov$optype $sw $dest, $src, $pred", SPred>;
}

// HW register moves.

// Generic mov_hwreg instructions. HW registers in them are specified by
// immediate. Such instructions are for assembler only, they are useless
// for code generator and not interesting for disassembler.
let isAsmParserOnly = 1 in {
  def MOV_ld_gshp : LoadInst_MovFromHWRegG<0b000101, SRF,  "mov.from_hw_reg $sw $dest, $reg, $pred", SPred>, Requires<[GetHSRF]>;
}

// Normal MOV instructions.
def MOV_ld_shp : LoadInst_MovFromHWReg<0b000101, SRF,  HSRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVshp">, Requires<[GetHSRF]>;

// SPU slot.

def MOVssp  : SpuInst_Mov     <0b001000, SRF,  SRF,    "mov$optype $sw $dest, $src, $pred">, MultiSlot<"MOVssp">;
def MOVsip  : SpuInst_MovI    <0b001000, SRF,  i32imm, "mov$optype $sw $dest, $src, $pred">, MultiSlot<"MOVsip">;
def MOVpsp  : SpuInst_MovType <0b001000, SPRF, SRF,    "mov $sw $dest, $src, $pred", OpType.BOOL>, MultiSlot<"MOVpsp">;
def MOVpip  : SpuInst_MovTypeI<0b001000, SPRF, i32imm, "mov $sw $dest, $src, $pred", OpType.BOOL>, MultiSlot<"MOVpip">;
def MOVppp  : SpuInst_MovType <0b001000, SPRF, SPRF,   "mov $sw $dest, $src, $pred", OpType.BOOL>;

def MOVIsp  : SpuInst_MovMask <0b001000, IRF,  SRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIsp">;
def MOVIip  : SpuInst_MovMaskI<0b001000, IRF,  i32imm, "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIip">;
def MOVIIp  : SpuInst_MovMask <0b001000, IRF,  IRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIIp">;

// Compatibility instructions.
let isAsmParserOnly = 1 in {
  def MOVtIsp : SpuInst_MovMaskT <0b001000, IRF,  SRF,    "mov$optype $sw $mask $dest, $src, $pred">;
  def MOVtIip : SpuInst_MovMaskIT<0b001000, IRF,  i32imm, "mov$optype $sw $mask $dest, $src, $pred">;
  def MOVtIIp : SpuInst_MovMaskT <0b001000, IRF,  IRF,    "mov$optype $sw $mask $dest, $src, $pred">;
  def MOVtpip : SpuInst_MovI     <0b001000, SPRF, i32imm, "mov$optype $sw $dest, $src, $pred">;
  def MOVtppp : SpuInst_Mov      <0b001000, SPRF, SPRF,   "mov$optype $sw $dest, $src, $pred">;
}

// HW register moves.

let isAsmParserOnly = 1 in {
  def MOVgshp : SpuInst_MovFromHWRegG<0b001000, SRF,  "mov.from_hw_reg $sw $dest, $reg, $pred">, Requires<[GetHSRF]>;
}

// Normal MOV instructions.
def MOVshp : SpuInst_MovFromHWReg<0b001000, SRF,  HSRF,  "mov $sw $dest, $src, $pred">, MultiSlot<"MOVshp">, Requires<[GetHSRF]>;


// VPU slot.

def MOVvvp  : VpuInst_Mov<0b001000, VRF,  VRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvvp">;
def MOVvvm  : VpuInst_Mov<0b001000, VRF,  VRF,  "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvvm">;

def MOVmmp  : VpuInst_MovBit<0b001000, VPRF, VPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmmp">;
def MOVmmm  : VpuInst_MovBit<0b001000, VPRF, VPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmmm">;
def MOVmpp  : VpuInst_MovBit<0b001000, VPRF, SPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmpp">;
def MOVmpm  : VpuInst_MovBit<0b001000, VPRF, SPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmpm">;

def MOVmsp : VpuInst_MovFlavor<0b001000, VPRF, SRF,            "mov $sw $flavor $dest, $src, $pred", SPred>, MultiSlot<"MOVmsp">;
def MOVmsm : VpuInst_MovFlavor<0b001000, VPRF, SRF,            "mov $sw $flavor $dest, $src, $pred", VPred>, MultiSlot<"MOVmsm">;
def MOVmip : VpuInst_MovFlavor<0b001000, VPRF, TPCImm<i32imm>, "mov $sw $flavor $dest, $src, $pred", SPred>, MultiSlot<"MOVmip">;
def MOVmim : VpuInst_MovFlavor<0b001000, VPRF, TPCImm<i32imm>, "mov $sw $flavor $dest, $src, $pred", VPred>, MultiSlot<"MOVmim">;

def MOVBmsp : VpuInst_MovFlavorBroadcast<0b001000, VPRF, SRF,            "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVBmsp">;
def MOVBmsm : VpuInst_MovFlavorBroadcast<0b001000, VPRF, SRF,            "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVBmsm">;
def MOVBmip : VpuInst_MovFlavorBroadcast<0b001000, VPRF, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVBmip">;
def MOVBmim : VpuInst_MovFlavorBroadcast<0b001000, VPRF, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVBmim">;

// Moves with required datatype.
def MOVvsp : VpuInst_MovType <0b001000, VRF, SRF,    "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvsp">;
def MOVvsm : VpuInst_MovType <0b001000, VRF, SRF,    "mov$optype $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvsm">;
def MOVvip : VpuInst_MovIType<0b001000, VRF, i32imm, "mov$optype $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvip">;
def MOVvim : VpuInst_MovIType<0b001000, VRF, i32imm, "mov$optype $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvim">;

// Compatibility instructions.
let isAsmParserOnly = 1 in {
  def MOVtvvp : VpuInst_MovType<0b001000, VRF,  VRF,  "mov$optype$sw $dest, $src, $pred", SPred>;
  def MOVtvvm : VpuInst_MovType<0b001000, VRF,  VRF,  "mov$optype$sw $dest, $src, $pred", VPred>;
  def MOVtmmp : VpuInst_MovType<0b001000, VPRF, VPRF, "mov$optype$sw $dest, $src, $pred", SPred>;
  def MOVtmmm : VpuInst_MovType<0b001000, VPRF, VPRF, "mov$optype$sw $dest, $src, $pred", VPred>;
  def MOVtmpp : VpuInst_MovType<0b001000, VPRF, SPRF, "mov$optype$sw $dest, $src, $pred", SPred>;
  def MOVtmpm : VpuInst_MovType<0b001000, VPRF, SPRF, "mov$optype$sw $dest, $src, $pred", VPred>;

  def MOVtmsp : VpuInst_MovFlavorT<0b001000, VPRF, SRF,            "mov$optype$sw $flavor $dest, $src, $pred", SPred>;
  def MOVtmsm : VpuInst_MovFlavorT<0b001000, VPRF, SRF,            "mov$optype$sw $flavor $dest, $src, $pred", VPred>;
  def MOVtmip : VpuInst_MovFlavorT<0b001000, VPRF, TPCImm<i32imm>, "mov$optype$sw $flavor $dest, $src, $pred", SPred>;
  def MOVtmim : VpuInst_MovFlavorT<0b001000, VPRF, TPCImm<i32imm>, "mov$optype$sw $flavor $dest, $src, $pred", VPred>;
}


def : Pat<(v256i1 (vsplatb i1:$src)), (v256i1 (MOV_ld_mpp SPRF:$src, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def : Pat<(int_tpc_mov_flavor i32:$src, (i32 8), (i32 imm:$sw), v256i1:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVB_ld_msp SRF:$src, (i32 imm:$sw), VPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor i32:$src, (i32 8), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity)),
          (MOVB_ld_msm SRF:$src, (i32 imm:$sw), VPRF:$income, VPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor (i32 imm:$src), (i32 8), (i32 imm:$sw), v256i1:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVB_ld_mip (CastToImm32 $src), (i32 imm:$sw), VPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor (i32 imm:$src), (i32 8), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity)),
          (MOVB_ld_mim (CastToImm32 $src), (i32 imm:$sw), VPRF:$income, VPRF:$pred, (i1 imm:$polarity))>;

def : Pat<(int_tpc_mov_flavor i32:$src, (i32 imm:$flavor), (i32 imm:$sw), v256i1:$income, i1:$pred, (i1 imm:$polarity)),
          (MOV_ld_msp SRF:$src, (i32 imm:$flavor), (i32 imm:$sw), VPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor i32:$src, (i32 imm:$flavor), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity)),
          (MOV_ld_msm SRF:$src, (i32 imm:$flavor), (i32 imm:$sw), VPRF:$income, VPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor (i32 imm:$src), (i32 imm:$flavor), (i32 imm:$sw), v256i1:$income, i1:$pred, (i1 imm:$polarity)),
          (MOV_ld_mip (CastToImm32 $src), (i32 imm:$flavor), (i32 imm:$sw), VPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_flavor (i32 imm:$src), (i32 imm:$flavor), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity)),
          (MOV_ld_mim (CastToImm32 $src), (i32 imm:$flavor), (i32 imm:$sw), VPRF:$income, VPRF:$pred, (i1 imm:$polarity))>;

def : Pat<(int_tpc_mov_mask v5i32:$src, (i32 imm:$mask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVIIp IRF:$src, (i32 imm:$mask), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_mask i32:$src, (i32 imm:$mask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVIsp SRF:$src, (i32 imm:$mask), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_mask (i32 imm:$src), (i32 imm:$mask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVIip (CastToImm32 $src), (i32 imm:$mask), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_mask (v5i32 (vsplati5 i32:$src)), (i32 imm:$mask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVIsp SRF:$src, (i32 imm:$mask), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_mov_mask (v5i32 (vsplati5 (i32 imm:$src))), (i32 imm:$mask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVIip (CastToImm32 $src), (i32 imm:$mask), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;

foreach type = [i1] in {
  def : Pat<(type (int_tpc_mov type:$src, (i8 imm), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"MOV", type, type, i1>.Insn
                   type:$src, (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_mov (type imm:$src), (i8 imm), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"MOV", type, iAny, i1>.Insn
                   (CastToImm32 $src), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

foreach pred = [i1, v256i1] in {
  def : Pat<(v256i1 (int_tpc_mov v256i1:$src, (i8 imm), (i32 imm:$sw), v256i1:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i1 (ConstructInsn<"MOV", v256i1, v256i1, pred>.Insn
                     VPRF:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i1 (int_tpc_mov (v256i1 (vsplatb i1:$src)), (i8 imm), (i32 imm:$sw), v256i1:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i1 (ConstructInsn<"MOV", v256i1, i1, pred>.Insn
                     i1:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i1 (int_tpc_mov i1:$src, (i8 imm), (i32 imm:$sw), v256i1:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i1 (ConstructInsn<"MOV", v256i1, i1, pred>.Insn
                     i1:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
}

foreach type = [i32, i16, i8] in {
  def : Pat<(i1 (int_tpc_mov (i1 (trunc type:$src)), (i8 imm), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
            (i1 (MOVpsp SRF:$src, (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(i1 (int_tpc_mov (i1 (trunc (type imm:$src))), (i8 imm), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
            (i1 (MOVpip (CastToImm32 $src), (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

foreach type = [f32, bf16, i32, i16, i8] in
  def : Pat<(type (int_tpc_mov type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVssp SRF:$src, (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

foreach type = [f32, bf16] in
  def : Pat<(type (int_tpc_mov (type fpimm:$src), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVsip (FPToImm32 $src), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

foreach type = [i32, i16, i8] in
  def : Pat<(type (int_tpc_mov (type imm:$src), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVsip (CastToImm32 $src), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


foreach T = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(T (int_tpc_mov T:$src, (i8 imm), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, T, pred>.Insn
                     VRF:$src, (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov (T (SplatNode<T>.Splat VectorElement<T>.Type:$src)), (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, VectorElement<T>.Type, pred>.Insn
                     SRF:$src, (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov (T (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$src))), (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, iAny, pred>.Insn
                     (CastOp<T>.Cast $src), (i8 imm:$dt), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov VectorElement<T>.Type:$src, (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, VectorElement<T>.Type, pred>.Insn
                     SRF:$src, (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov (VectorElement<T>.Type ImmNode<T>.Node:$src), (i8 imm:$dt), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, iAny, pred>.Insn
                     (CastOp<T>.Cast $src), (i8 imm:$dt), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  }

def : Pat<(i1 (i1 imm:$src)), (MOVpip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(i1 (trunc i32:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i1 (trunc i16:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i1 (trunc  i8:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def : Pat<(v256i1 (vsplatb (i1 imm:$src))),
          (v256i1 (MOVB_ld_mip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def : Pat<(v64f32 (vsplatf32 (f32 fpimm:$src))),
          (v64f32 (MOV_ld_vip (FPToImm32 $src), OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v128bf16 (vsplatbf16 (bf16 fpimm:$src))),
          (v128bf16 (MOV_ld_vip (FPToImm32 $src), OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 (i32 imm:$src))),
          (v64i32 (MOV_ld_vip (CastToImm32 $src), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 (i16 imm:$src))),
          (v128i16 (MOV_ld_vip (CastToImm32 $src), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 (i8 imm:$src))),
          (v256i8 (MOV_ld_vip (CastToImm32 $src), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def PropToTimm32 : SDNodeXForm<timm, [{
  return CurDAG->getTargetConstant(N->getZExtValue(), SDLoc(N), MVT::i32);
}]>;

// After faced with BUILD_VECTOR TargetConstant...
def : Pat<(v256i1 (vsplatb (i1 timm:$src))),
          (v256i1 (MOVB_ld_mip (PropToTimm32 $src), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 (i32 timm:$src))),
          (v64i32 (MOV_ld_vip $src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 (i16 timm:$src))),
          (v128i16 (MOV_ld_vip (PropToTimm32 $src), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 (i8 timm:$src))),
          (v256i8 (MOV_ld_vip (PropToTimm32 $src), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def : Pat<(v64f32 (vsplatf32 f32:$src)),
          (v64f32 (MOV_ld_vsp SRF:$src, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v128bf16 (vsplatbf16 bf16:$src)),
          (v128bf16 (MOV_ld_vsp SRF:$src, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 i32:$src)),
          (v64i32 (MOV_ld_vsp SRF:$src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 i16:$src)),
          (v128i16 (MOV_ld_vsp SRF:$src, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 i8:$src)),
          (v256i8 (MOV_ld_vsp SRF:$src, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

// Conditional moves.
def : Pat<(v5i32 (cmove v5i32:$src, i1:$pred)),
          (v5i32 (MOVIIp v5i32:$src, (i32 31), (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
def : Pat<(v5i32 (cmove (v5i32 (vsplati5 i32:$src)), i1:$pred)),
          (v5i32 (MOVIsp i32:$src, (i32 31), (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
def : Pat<(v5i32 (cmove (v5i32 (vsplati5 (i32 imm:$src))), i1:$pred)),
          (v5i32 (MOVIip (CastToImm32 $src), (i32 31), (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;

def : Pat<(v5i32 (cmoveinv v5i32:$src, i1:$pred, v5i32:$income)),
          (v5i32 (MOVIIp v5i32:$src, (i32 31), (i32 0), v5i32:$income, i1:$pred, (i1 1)))>;
def : Pat<(v5i32 (cmoveinv (v5i32 (vsplati5 i32:$src)), i1:$pred, v5i32:$income)),
          (v5i32 (MOVIsp i32:$src, (i32 31), (i32 0), v5i32:$income, i1:$pred, (i1 1)))>;
def : Pat<(v5i32 (cmoveinv (v5i32 (vsplati5 (i32 imm:$src))), i1:$pred, v5i32:$income)),
          (v5i32 (MOVIip (CastToImm32 $src), (i32 31), (i32 0), v5i32:$income, i1:$pred, (i1 1)))>;

def : Pat<(i1 (cmove i1:$src, i1:$pred)),
          (i1 (MOVppp i1:$src, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
def : Pat<(i1 (cmove (i1 imm:$src), i1:$pred)),
          (i1 (MOVpip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;

def : Pat<(i1 (cmoveinv i1:$src, i1:$pred, i1:$income)),
          (i1 (MOVppp i1:$src, (i32 0), $income, i1:$pred, (i1 1)))>;
def : Pat<(i1 (cmoveinv (i1 imm:$src), i1:$pred, i1:$income)),
          (i1 (MOVpip (CastToImm32 $src), (i32 0), $income, i1:$pred, (i1 1)))>;

foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(type (cmove type:$src, i1:$pred)),
            (type (MOVssp type:$src, OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmove (type ImmNode<type>.Node:$src), i1:$pred)),
            (type (MOVsip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmoveinv type:$src, i1:$pred, type:$income)),
            (type (MOVssp type:$src, OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
  def : Pat<(type (cmoveinv (type ImmNode<type>.Node:$src), i1:$pred, type:$income)),
            (type (MOVsip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
}

foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in {
  def : Pat<(type (cmove type:$src, i1:$pred)),
            (type (MOVvvp type:$src, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmove (type (SplatNode<type>.Splat VectorElement<type>.Type:$src)), i1:$pred)),
            (type (MOVvsp $src, OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmove (type (SplatNode<type>.Splat (VectorElement<type>.Type ImmNode<type>.Node:$src))), i1:$pred)),
            (type (MOVvip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmoveinv type:$src, i1:$pred, type:$income)),
            (type (MOVvvp type:$src, (i32 0), $income, i1:$pred, (i1 1)))>;
  def : Pat<(type (cmoveinv (type (SplatNode<type>.Splat VectorElement<type>.Type:$src)), i1:$pred, type:$income)),
            (type (MOVvsp $src, OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
  def : Pat<(type (cmoveinv (type (SplatNode<type>.Splat (VectorElement<type>.Type ImmNode<type>.Node:$src))), i1:$pred, type:$income)),
            (type (MOVvip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
}

// Immediate materialization.
foreach T = [f32, bf16, i32, i16, i8] in
  def : Pat<(T ImmNode<T>.Node:$x), (T (MOVsip (CastOp<T>.Cast $x), OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

foreach type = [i32, i16, i8] in {
  def : Pat<(type (sext i1:$src)),
            (MOVsip (i32 0),   OpTypeForType<type>.V, (i32 0),
            (MOVsip (i32 -1),  OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$src, (i1 0)), $src, (i1 1))>;
  def : Pat<(type (zext i1:$src)),
            (MOVsip (i32 0),   OpTypeForType<type>.V, (i32 0),
            (MOVsip (i32 0x1), OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$src, (i1 0)), $src, (i1 1))>;
  def : Pat<(type (anyext i1:$src)),
            (MOVsip (i32 0),   OpTypeForType<type>.V, (i32 0),
            (MOVsip (i32 0x1), OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$src, (i1 0)), $src, (i1 1))>;
}

//------ Load slot -------------------------------------------------------------

//------ GEN_ADDR --------------------------------------------------------------

def GEN_ADDR_ld  : LoadInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.DimMask>, MultiSlot<"GEN_ADDR">;


//------ PRMT_INDX -------------------------------------------------------------

def PRMT_INDX_ld : LoadInstrPRMT_INDX<0b000011, "prmt_indx $sw $dest, $src1, $src2, $pred">, MultiSlot<"PRMT_INDX">;


//------ SET_INDX --------------------------------------------------------------

def SET_INDX_ld_rp  : LoadInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_rp">;
def SET_INDX_ld_ip  : LoadInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_ip">;

def : Pat<(int_tpc_set_indx v5i32:$income, (i32 imm:$dimmask), i32:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_rp IRF:$income, SRF:$value, (i32 imm:$dimmask), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_set_indx v5i32:$income, (i32 imm:$dimmask), (i32 imm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_ip IRF:$income, (CastToImm32 $value), (i32 imm:$dimmask), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;


//------ LD_L ------------------------------------------------------------------

def LD_Lpsp  : LoadInst_LD_L <0b001011, SPRF, "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lpip  : LoadInst_LD_LI<0b001011, SPRF, "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lssp  : LoadInst_LD_L <0b001011, SRF,  "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lsip  : LoadInst_LD_LI<0b001011, SRF,  "ld_l $sw $dest, $addr, $pred", []>;

def : Pat<(i1 (lsload (i32 imm:$addr))), (LD_Lpip (CastToImm32 $addr), (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i1 (lsload i32:$addr)),       (LD_Lpsp $addr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(type (lsload (i32 imm:$addr))), (LD_Lsip (CastToImm32 $addr), (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
  def : Pat<(type (lsload i32:$addr)),       (LD_Lssp $addr,               (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
}

def : Pat<(i1 (int_tpc_ld_l (i32 imm:$addr), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
          (i1 (LD_Lpip (CastToImm32 $addr), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity)))>;
def : Pat<(i1 (int_tpc_ld_l i32:$addr, (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
          (i1 (LD_Lpsp $addr, (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity)))>;
foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(type (int_tpc_ld_l (i32 imm:$addr), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (LD_Lsip (CastToImm32 $addr), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_l i32:$addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (LD_Lssp $addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
}


//------ LD_G ------------------------------------------------------------------

def LD_Gsap  : LoadInst_LD_G <0b001100, SRF,  "ld_g $sw $dest, $addr, $pred", SPred>;
def LD_Gpap  : LoadInst_LD_G <0b001100, SPRF, "ld_g $sw $dest, $addr, $pred", SPred>;
def LD_Gvap  : LoadInst_LD_G <0b001100, VRF,  "ld_g $sw $dest, $addr, $pred", SPred>;

foreach type = [i1, i8, i16, i32, f32, bf16] in
  def : Pat<(type (gload i64:$addr)),
            (type (ConstructInsn<"LD_G", type, i64, i1>.Insn
                   ADRF:$addr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

// Patterns for intrinsics 'int_tpc_ld_g'. See getTgtMemIntrinsic() and LowerINTRINSIC_W_CHAIN()
// in TPCISelLowering.cpp for how the node 'gloadp' is mapped to 'int_tpc_ld_g'
foreach type = [i1, i8, i16, i32, f32, bf16, v64i32, v128i16, v256i8, v64f32, v128bf16] in {
  def : Pat<(type (gloadp i64:$addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_G", type, i64, i1>.Insn
                   ADRF:$addr, (i32 imm:$sw), (type $income), SPRF:$pred, (i1 imm:$polarity)))>;
}

//------ PREFETCH --------------------------------------------------------------

def PREFETCH : LoadInstr_PREFETCH<0b001101, "prefetch $sw $addr, $pred">, Requires<[Gen2Plus]> {
  let hasSideEffects = 1;
}

def : Pat<(int_tpc_prefetch i64:$addr, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (PREFETCH ADRF:$addr, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus]>;


//------ LD_L_V ----------------------------------------------------------------

defm LD_L_V : LoadInst_LD_L_V<0b001110, "ld_l_v $sw $dest, $addr, $pred">;

foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32, v128bf16] in {
  def : Pat<(type (lvload (i32 imm:$addr))),
            (type (ConstructInsn<"LD_L_V", type, iAny, i1>.Insn
                   (i32 imm:$addr), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>, Requires<[Addr1]>;
  def : Pat<(type (lvload (i32 i32:$addr))),
            (type (ConstructInsn<"LD_L_V", type, i32, i1>.Insn
                   (i32 $addr), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>, Requires<[Addr1]>;
  def : Pat<(type (lvload (i32 imm:$addr))),
            (type (ConstructInsn<"LD_L_V", type, i32, iAny, i1>.Insn
                   (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                   (i32 0), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>, Requires<[Addr2]>;
  def : Pat<(type (lvload i32:$addr)),
            (type (ConstructInsn<"LD_L_V", type, i32, iAny, i1>.Insn
                   (i32 $addr), (i32 0), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>, Requires<[Addr2]>;
}

foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                     (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V", type, i32, iAny, pred>.Insn
                     (i32 $addr), (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V", type, iAny, pred>.Insn
                     (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
    def : Pat<(type (int_tpc_ld_l_v i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V", type, i32, pred>.Insn
                     SRF:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
  }


defm LD_L_V_LOW : LoadInst_LD_L_V<0b001111, "ld_l_v_low $sw $dest, $addr, $pred">;

foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v_low (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_LOW", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                     (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v_low i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_LOW", type, i32, iAny, pred>.Insn
                     (i32 $addr), (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v_low (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_LOW", type, iAny, pred>.Insn
                     (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
    def : Pat<(type (int_tpc_ld_l_v_low i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_LOW", type, i32, pred>.Insn
                     SRF:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
  }


defm LD_L_V_HIGH : LoadInst_LD_L_V<0b010000, "ld_l_v_high $sw $dest, $addr, $pred">;

foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v_high (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_HIGH", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                     (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v_high i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_HIGH", type, i32, iAny, pred>.Insn
                     (i32 $addr), (i32 0), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr2]>;
    def : Pat<(type (int_tpc_ld_l_v_high (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_HIGH", type, iAny, pred>.Insn
                     (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
    def : Pat<(type (int_tpc_ld_l_v_high i32:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_HIGH", type, i32, pred>.Insn
                     SRF:$addr, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Addr1]>;
  }


//------ LOOKUP ---------------------------------------------------------------

// FIXME: Don't let to compile lookup(_c1/_c2) with LUT_PTR and IMM as $fid
def LOOKUP_1    : LoadInst_LOOKUP_GEN1<0b000110, VRF, "lookup      $sw $dest, $src, $fid, $pred">, Requires<[Gen1]>;
def LOOKUP_2    : LoadInst_LOOKUP_GEN2<0b000110, VRF, "lookup      $sw $dest, $src, $fid, $pred">, Requires<[Gen2Plus]>;
def LOOKUP_C0   : LoadInst_LOOKUP_GEN1<0b001001, VRF, "lookup_c0   $sw $dest, $src, $fid, $pred">, Requires<[Gen1]>;
def LOOKUP_C1C2 : LoadInst_LOOKUP_GEN1<0b000111, DRF, "lookup_c1c2 $sw $dest, $src, $fid, $pred">, Requires<[Gen1]>;
def LOOKUP_1C   : LoadInst_LOOKUP_GEN2<0b001001, VRF, "lookup_1c   $sw $dest, $src, $fid, $pred">, Requires<[Gen2Plus]>;
def LOOKUP_2C   : LoadInst_LOOKUP_GEN2<0b000111, DRF, "lookup_2c   $sw $dest, $src, $fid, $pred">, Requires<[Gen2Plus]>;

foreach T1 = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
    foreach T2 = [v256i8, v128i16, v64i32] in {
      def : Pat<(T1 (int_tpc_lookup T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(T1 (int_tpc_lookup T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
      def : Pat<(T1 (int_tpc_lookup_c0 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_C0 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_c1c2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_C1C2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(T1 (int_tpc_lookup_1c T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1C T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_2c T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_2C T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
    }


//------ LD_TNSR ---------------------------------------------------------------

multiclass Ld_Tnsr<bits<6> OpCode, string op_name, string asmstr> {
  def vp   : LoadInst_LD_TNSR   <OpCode, VRF,  asmstr, SPred>;
  def vm   : LoadInst_LD_TNSR   <OpCode, VRF,  asmstr, VPred>, Requires<[LdVectMask]>;
  def mp   : LoadInst_LD_TNSR   <OpCode, VPRF, asmstr, SPred>;
  def mm   : LoadInst_LD_TNSR   <OpCode, VPRF, asmstr, VPred>, Requires<[LdVectMask]>;

  // Instructions where tensor is specified in register S27. We don't use the
  // 'FeatureTnsrInReg' for them, as this feature assumes the tensor in HW
  // register. S27 is used for this purpose only in Gen2, so there is no
  // dedicated feature.
  def Gen2Tvp  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, SPred, 0>, Requires<[Gen2]>;
  def Gen2Tvm  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, VPred, 0>, Requires<[Gen2, LdVectMask]>;
  def Gen2Tmp  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, SPred, 0>, Requires<[Gen2]>;
  def Gen2Tmm  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, VPred, 0>, Requires<[Gen2, LdVectMask]>;
}

defm LD_TNSR : Ld_Tnsr<0b010001, "LD_TNSR", "ld_tnsr $sw $dst, $tensor, $coords, $pred">;

// Partial load operations that use S30 to keep Offset+Size. We don't use
// 'FeaturePartial' for them, as this feature assumes offset+size is specified
// in HW registers. S30 is used for this purpose only in Gen2, so there is no
// dedicated feature.
def LD_TNSR_PGen2vp   : LoadInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2vm   : LoadInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2mp   : LoadInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2mm   : LoadInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2Tvp  : LoadInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2Tvm  : LoadInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2Tmp  : LoadInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 0>, Requires<[Gen2]>;
def LD_TNSR_PGen2Tmm  : LoadInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 0>, Requires<[Gen2]>;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr v5i32:$ndx, S27, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSRGen2T", type, pred>.Insn v5i32:$ndx,
                   S27, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
}

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PGen2", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Gen2]>;
    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, i8:$tensor, i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Gen2]>;
  }

//------ LD_TNSR_LOW -----------------------------------------------------------

defm LD_TNSR_LOW : Ld_Tnsr<0b010010, "LD_TNSR_LOW", "ld_tnsr_low $sw $dst, $tensor, $coords, $pred">;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr_low v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOW", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr_low v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOWGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
  }


//------ LD_TNSR_HIGH ----------------------------------------------------------

defm LD_TNSR_HIGH : Ld_Tnsr<0b010011, "LD_TNSR_HIGH", "ld_tnsr_high $sw $dst, $tensor, $coords, $pred">;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr_high v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGH", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr_high v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGHGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
  }


//------ Scalar slot -----------------------------------------------------------

def MACssp : SpuInstBinarySw  <0b000000, SRF, SRF, SRF,    "mac$optype $sw $dest, $op1, $op2, $pred">;
def MACsip : SpuInstBinarySwI <0b000000, SRF, SRF, i32imm, "mac$optype $sw $dest, $op1, $op2, $pred">;

foreach type = [i8, i16] in {
  def : Pat<(i32 (int_tpc_mac type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
            (i32 (MACssp type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(i32 (int_tpc_mac type:$x0, (type imm:$x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
            (i32 (MACsip type:$x0, (CastToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

def : Pat<(f32 (int_tpc_mac f32:$x0, f32:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
          (f32 (MACssp f32:$x0, f32:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
def : Pat<(f32 (int_tpc_mac f32:$x0, (f32 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
          (f32 (MACsip f32:$x0, (FPToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;

let Predicates = [BF16] in {
  def : Pat<(bf16 (int_tpc_mac bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity))),
            (bf16 (MACssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(bf16 (int_tpc_mac bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity))),
            (bf16 (MACsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// With ACC_FP32.
let Predicates = [BF16] in {
  def : Pat<(f32 (int_tpc_mac bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}


//------ MUL -------------------------------------------------------------------

def MULssp  : SpuInst_Binary      <0b000001, SRF, SRF,    SRF,    "mul$optype $sw $dest, $op1, $op2, $pred">;
def MULsip  : SpuInst_BinaryI     <0b000001, SRF, SRF,    i32imm, "mul$optype $sw $dest, $op1, $op2, $pred">;
def MULsIp  : SpuInst_BinaryMask  <0b000001, IRF, SRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;
def MULiIp  : SpuInst_BinaryMaskIR<0b000001, IRF, i32imm, IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;
def MULIIp  : SpuInst_BinaryMask  <0b000001, IRF, IRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<mul,  MULssp, MULsip>;
defm : BinaryFloatScalar<fmul, MULssp, MULsip>;
defm : BinaryIndexOp1<mul, MULIIp, MULsIp, MULiIp>;

foreach type = [f32, i32, bf16] in {
  def : Pat<(type (int_tpc_mul type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULssp type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_mul type:$x0, (type ImmNode<type>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULsip type:$x0, (CastOp<type>.Cast $x1), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
}
let Predicates = [BF16] in {
  def : Pat<(f32 (int_tpc_mul bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mul bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

let Predicates = [MULI8] in {
  def : Pat<(i32 (int_tpc_mul i8:$x0, i8:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
            (i32 (MULssp i8:$x0, i8:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(i32 (int_tpc_mul i8:$x0, (i8 imm:$x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
            (i32 (MULsip i8:$x0, (CastToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

def : Pat<(i32 (int_tpc_mul i16:$x0, i16:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (MULssp i16:$x0, i16:$x1, (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;
def : Pat<(i32 (int_tpc_mul i16:$x0, (i16 imm:$x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (MULsip i16:$x0, (CastToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;

defm : BinaryIndexIntrinsicsCompat<int_tpc_mul, "MUL">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_mul_mask, "MUL">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_mul_mask, "MUL">;


//------ ADD -------------------------------------------------------------------

// In ADD the flag DIM_MASK_REG is same as CARRY, so to distinguish between
// 'ADD M0 I1, I2, I3' and 'ADD.CARRY S1, S1, S3' we must analyze register
// type.
let isNotUsedInDisasm = 1 in {
def ADDssp  : SpuInst_Binary      <0b000010, SRF, SRF,    SRF,    "add$optype $sw $dest, $op1, $op2, $pred">;
def ADDsip  : SpuInst_BinaryI     <0b000010, SRF, SRF,    i32imm, "add$optype $sw $dest, $op1, $op2, $pred">;
def ADDsIp  : SpuInst_BinaryMask  <0b000010, IRF, SRF,    IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">;
def ADDiIp  : SpuInst_BinaryMaskIR<0b000010, IRF, i32imm, IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">;
def ADDIIp  : SpuInst_BinaryMask  <0b000010, IRF, IRF,    IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">;
}

defm : BinarySignedScalar<add,  ADDssp, ADDsip>;
defm : BinaryFloatScalar<fadd, ADDssp, ADDsip>;
defm : BinaryIndexOp1<add, ADDIIp, ADDsIp, ADDiIp>;
foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_add, "ADD", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_add, "ADD">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_add_mask, "ADD">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_add_mask, "ADD">;


//------ SUB -------------------------------------------------------------------

// In SUB the flag DIM_MASK_REG is same as NEG.
let isNotUsedInDisasm = 1 in {
def SUBssp  : SpuInst_Binary     < 0b000011, SRF, SRF,    SRF,    "sub$optype $sw $dest, $op1, $op2, $pred">;
def SUBsip  : SpuInst_BinaryI     <0b000011, SRF, SRF,    i32imm, "sub$optype $sw $dest, $op1, $op2, $pred">;
def SUBsIp  : SpuInst_BinaryMask  <0b000011, IRF, SRF,    IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">;
def SUBiIp  : SpuInst_BinaryMaskIR<0b000011, IRF, i32imm, IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">;
def SUBIIp  : SpuInst_BinaryMask  <0b000011, IRF, IRF,    IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">;
}

defm : BinarySignedScalar<sub,  SUBssp, SUBsip>;
defm : BinaryFloatScalar<fsub, SUBssp, SUBsip>;
defm : BinaryIndexOp1<sub, SUBIIp, SUBsIp, SUBiIp>;
foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_sub, "SUB", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_sub, "SUB">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_sub_mask, "SUB">;

//------ MAX -------------------------------------------------------------------

def MAXssp  : SpuInst_Binary      <0b000101, SRF, SRF,    SRF,    "max$optype $sw $dest, $op1, $op2, $pred">;
def MAXsip  : SpuInst_BinaryI     <0b000101, SRF, SRF,    i32imm, "max$optype $sw $dest, $op1, $op2, $pred">;
def MAXIIp  : SpuInst_BinaryMask  <0b000101, IRF, IRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;
def MAXiIp  : SpuInst_BinaryMaskIR<0b000101, IRF, i32imm, IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;
def MAXsIp  : SpuInst_BinaryMask  <0b000101, IRF, SRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_max, "MAX", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_max, "MAX">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_max_mask, "MAX">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_max_mask, "MAX">;


//------ MIN -------------------------------------------------------------------

def MINssp  : SpuInst_Binary      <0b000110, SRF, SRF,    SRF,    "min$optype $sw $dest, $op1, $op2, $pred">;
def MINsip  : SpuInst_BinaryI     <0b000110, SRF, SRF,    i32imm, "min$optype $sw $dest, $op1, $op2, $pred">;
def MINIIp  : SpuInst_BinaryMask  <0b000110, IRF, IRF,    IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">;
def MINiIp  : SpuInst_BinaryMaskIR<0b000110, IRF, i32imm, IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">;
def MINsIp  : SpuInst_BinaryMask  <0b000110, IRF, SRF,    IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_min, "MIN", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_min, "MIN">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_min_mask, "MIN">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_min_mask, "MIN">;


//------ ABS -------------------------------------------------------------------

def ABSssp  : SpuInst_Unary    <0b000111, SRF, SRF, "abs$optype $sw $dest, $op, $pred">;
def ABSIsp  : SpuInst_UnaryMask<0b000111, IRF, SRF, "abs$optype $sw $mask $dest, $op, $pred">;
def ABSIIp  : SpuInst_UnaryMask<0b000111, IRF, IRF, "abs$optype $sw $mask $dest, $op, $pred">;

defm : UnaryIntegerScalar<abs,  ABSssp>;
defm : UnaryIndexIntrinsics<int_tpc_abs_mask, "ABS">;

foreach type = [i32, i16, i8] in
  def : Pat<(type (int_tpc_abs type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ABSssp SRF:$src, (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


//------ CMP_EQ ----------------------------------------------------------------

def CMP_EQssp  : SpuInst_Binary     <0b001001, SPRF, SRF,    SRF, "cmp_eq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_EQsip  : SpuInst_BinaryI    <0b001001, SPRF, SRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<seteq,  CMP_EQssp, CMP_EQsip>;
defm : BinaryFloatScalar<setoeq, CMP_EQssp, CMP_EQsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_eq, "CMP_EQ", type, i1>;

foreach T = [i32, i16, i8] in {
  def : Pat<(seteq (and T:$x0, T:$x1), 0),
            (ConstructInsn<"CMP_EQ", T, T, i1>.Insn T:$x0, T:$x1, OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[Gen2Plus]>;
  def : Pat<(seteq (and T:$x0, (T ImmNode<T>.Node:$x1)), 0),
            (ConstructInsn<"CMP_EQ", T, iAny, i1>.Insn T:$x0, (CastToImm32 $x1), OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[Gen2Plus]>;
}


//------ CMP_NEQ ---------------------------------------------------------------

def CMP_NEQssp  : SpuInst_Binary     <0b001010, SPRF, SRF,    SRF, "cmp_neq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_NEQsip  : SpuInst_BinaryI    <0b001010, SPRF, SRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<setne,  CMP_NEQssp, CMP_NEQsip>;
defm : BinaryFloatScalar<setone, CMP_NEQssp, CMP_NEQsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_neq, "CMP_NEQ", type, i1>;


//------ CMP_LESS --------------------------------------------------------------

def CMP_LESSssp  : SpuInst_Binary     <0b001011, SPRF, SRF,    SRF, "cmp_less$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LESSsip  : SpuInst_BinaryI    <0b001011, SPRF, SRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<setlt,  CMP_LESSssp, CMP_LESSsip>;
defm : BinaryUnsignedScalar<setult, CMP_LESSssp, CMP_LESSsip>;
defm : BinaryFloatScalar<setolt, CMP_LESSssp, CMP_LESSsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_less, "CMP_LESS", type, i1>;


//------ CMP_LEQ ---------------------------------------------------------------

def CMP_LEQssp  : SpuInst_Binary     <0b001100, SPRF, SRF,    SRF, "cmp_leq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LEQsip  : SpuInst_BinaryI    <0b001100, SPRF, SRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<setle,  CMP_LEQssp, CMP_LEQsip>;
defm : BinaryUnsignedScalar<setule, CMP_LEQssp, CMP_LEQsip>;
defm : BinaryFloatScalar<setole, CMP_LEQssp, CMP_LEQsip>;

defm : BinaryFloatScalar<setule, CMP_LEQssp, CMP_LEQsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_leq, "CMP_LEQ", type, i1>;


//------ CMP_GRT ---------------------------------------------------------------

def CMP_GRTssp  : SpuInst_Binary     <0b001101, SPRF, SRF,    SRF, "cmp_grt$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GRTsip  : SpuInst_BinaryI    <0b001101, SPRF, SRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<setgt,  CMP_GRTssp, CMP_GRTsip>;
defm : BinaryUnsignedScalar<setugt, CMP_GRTssp, CMP_GRTsip>;
defm : BinaryFloatScalar<setogt, CMP_GRTssp, CMP_GRTsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_grt, "CMP_GRT", type, i1>;


//------ CMP_GEQ ---------------------------------------------------------------

def CMP_GEQssp  : SpuInst_Binary     <0b001110, SPRF, SRF,    SRF, "cmp_geq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GEQsip  : SpuInst_BinaryI    <0b001110, SPRF, SRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<setge,  CMP_GEQssp, CMP_GEQsip>;
defm : BinaryUnsignedScalar<setuge, CMP_GEQssp, CMP_GEQsip>;
defm : BinaryFloatScalar<setoge, CMP_GEQssp, CMP_GEQsip>;
defm : BinaryFloatScalar<setuge, CMP_GEQssp, CMP_GEQsip>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_geq, "CMP_GEQ", type, i1>;


//------ OR --------------------------------------------------------------------

def ORssp  : SpuInst_Binary      <0b001111, SRF, SRF,    SRF,    "or$optype $sw $dest, $op1, $op2, $pred">;
def ORsip  : SpuInst_BinaryI     <0b001111, SRF, SRF,    i32imm, "or$optype $sw $dest, $op1, $op2, $pred">;
def ORppp  : SpuInst_Binary      <0b001111, SPRF, SPRF,  SPRF,   "or$optype $sw $dest, $op1, $op2, $pred">;
def ORIIp  : SpuInst_BinaryMask  <0b001111, IRF, IRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ORiIp  : SpuInst_BinaryMaskIR<0b001111, IRF, i32imm, IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ORsIp  : SpuInst_BinaryMask  <0b001111, IRF, SRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<or, ORssp, ORsip>;
defm : BinaryFloatScalar<fpor, ORssp, ORsip>;
defm : BinaryIndexOp1<or, ORIIp, ORsIp, ORiIp>;
def : Pat<(or i1:$op1, i1:$op2), (ORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_or, "OR", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_or, "OR">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_or_mask, "OR">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_or_mask, "OR">;

def : Pat<(int_tpc_or i1:$op1,   i1:$op2, (i8 imm:$optype), (i32 imm:$sw),   i1:$income,   i1:$pred, (i1 imm:$polarity)),
          (ORppp    SPRF:$op1, SPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;


//------ AND -------------------------------------------------------------------

def ANDssp  : SpuInst_Binary      <0b010000, SRF, SRF,    SRF,    "and$optype $sw $dest, $op1, $op2, $pred">;
def ANDsip  : SpuInst_BinaryI     <0b010000, SRF, SRF,    i32imm, "and$optype $sw $dest, $op1, $op2, $pred">;
def ANDppp  : SpuInst_Binary      <0b010000, SPRF, SPRF,  SPRF,   "and$optype $sw $dest, $op1, $op2, $pred">;
def ANDIIp  : SpuInst_BinaryMask  <0b010000, IRF, IRF,    IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ANDiIp  : SpuInst_BinaryMaskIR<0b010000, IRF, i32imm, IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ANDsIp  : SpuInst_BinaryMask  <0b010000, IRF, SRF,    IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<and, ANDssp, ANDsip>;
defm : BinaryFloatScalar<fpand, ANDssp, ANDsip>;
defm : BinaryIndexOp1<and, ANDIIp, ANDsIp, ANDiIp>;
def : Pat<(and i1:$op1, i1:$op2), (ANDppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_and, "AND", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_and, "AND">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_and_mask, "AND">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_and_mask, "AND">;

def : Pat<(int_tpc_and i1:$op1,   i1:$op2, (i8 imm:$optype), (i32 imm:$sw), i1:$income,   i1:$pred, (i1 imm:$polarity)),
          (ANDppp    SPRF:$op1, SPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;


def : Pat<(f32 (int_tpc_abs f32:$src, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
          (f32 (ANDsip SRF:$src, (i32 0x7fffffff), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

def : Pat<(bf16 (int_tpc_abs bf16:$src, (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity))),
          (bf16 (ANDsip SRF:$src, (i32 0x7fff), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

def : Pat<(f8_152 (int_tpc_abs f8_152:$src, (i8 imm:$dt), (i32 imm:$sw), f8_152:$income, i1:$pred, (i1 imm:$polarity))),
          (f8_152 (ANDsip SRF:$src, (i32 0x7f), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

// llvm.fabs
def : Pat<(fabs f32:$src),
          (f32 (ANDsip $src, (i32 0x7fffffff), OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(fabs bf16:$src),
          (bf16 (ANDsip $src, (i32 0x7fff), OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(fabs f8_152:$src),
          (f8_152 (ANDsip $src, (i32 0x7f), OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;


//------ XOR -------------------------------------------------------------------

def XORssp  : SpuInst_Binary      <0b010001, SRF, SRF,    SRF,    "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORsip  : SpuInst_BinaryI     <0b010001, SRF, SRF,    i32imm, "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORppp  : SpuInst_Binary      <0b010001, SPRF, SPRF,  SPRF,   "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORIIp  : SpuInst_BinaryMask  <0b010001, IRF, IRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;
def XORiIp  : SpuInst_BinaryMaskIR<0b010001, IRF, i32imm, IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;
def XORsIp  : SpuInst_BinaryMask  <0b010001, IRF, SRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;

defm : BinarySignedScalar<xor, XORssp, XORsip>;
defm : BinaryFloatScalar<fpxor, XORssp, XORsip>;
defm : BinaryIndexOp1<xor, XORIIp, XORsIp, XORiIp>;

def : Pat<(xor i1:$op1, i1:$op2),      (XORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i1(setne i1:$op1, i1:$op2)),(XORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [f32, bf16, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_xor, "XOR", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_xor, "XOR">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_xor_mask, "XOR">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_xor_mask, "XOR">;

def : Pat<(int_tpc_xor i1:$op1,   i1:$op2, (i8 imm:$optype), (i32 imm:$sw),   i1:$income,   i1:$pred, (i1 imm:$polarity)),
          (XORppp    SPRF:$op1, SPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity))>;


//------ NOT -------------------------------------------------------------------

def NOTssp  : SpuInst_Unary    <0b010010, SRF,  SRF,  "not$optype $sw $dest, $op, $pred">;
def NOTppp  : SpuInst_Unary    <0b010010, SPRF, SPRF, "not$optype $sw $dest, $op, $pred">;
def NOTIsp  : SpuInst_UnaryMask<0b010010, IRF,  SRF,  "not$optype $sw $mask $dest, $op, $pred">;
def NOTIIp  : SpuInst_UnaryMask<0b010010, IRF,  IRF,  "not$optype $sw $mask $dest, $op, $pred">;

defm : UnaryIntegerScalar<inot,  NOTssp>;
defm : UnaryFloatScalar<fpnot, NOTssp>;
defm : UnaryIndexIntrinsics<int_tpc_not_mask, "NOT">;

def : Pat<(inot i1:$op), (NOTppp $op, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [f32, bf16, i32, i16, i8, i1] in
  def : Pat<(type (int_tpc_not type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"NOT", type, type, i1>.Insn
                   type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;


//------ SHR -------------------------------------------------------------------

def SHRssp  : SpuInst_Binary     <0b010011, SRF, SRF,    SRF,    "shr$optype $sw $dest, $op1, $op2, $pred">;
def SHRsip  : SpuInst_BinaryI    <0b010011, SRF, SRF,    i32imm, "shr$optype $sw $dest, $op1, $op2, $pred">;
def SHRIsp  : SpuInst_BinaryMask <0b010011, IRF, IRF,    SRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHRIip  : SpuInst_BinaryMaskI<0b010011, IRF, IRF,    i32imm, "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHRIIp  : SpuInst_BinaryMask <0b010011, IRF, IRF,    IRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;

defm : BinaryShiftScalar<srl, SHRssp, SHRsip>;
defm : BinaryIndexScalar<srl, SHRIsp, SHRIip>;
def  : Pat<(srl v5i32:$op1, v5i32:$op2),
           (SHRIIp $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
defm : BinaryIndexShiftIntrinsics<int_tpc_shr_mask, "SHR">;

foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(type (int_tpc_shr type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"SHR", type, i8, i1>.Insn
                   type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_shr type:$src1, (i8 imm:$src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"SHR", type, iAny, i1>.Insn
                   type:$src1, (CastToImm32 $src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}


//------ SHL -------------------------------------------------------------------

def SHLssp  : SpuInst_Binary     <0b010100, SRF, SRF,    SRF,    "shl$optype $sw $dest, $op1, $op2, $pred">;
def SHLsip  : SpuInst_BinaryI    <0b010100, SRF, SRF,    i32imm, "shl$optype $sw $dest, $op1, $op2, $pred">;
def SHLIsp  : SpuInst_BinaryMask <0b010100, IRF, IRF,    SRF,    "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHLIip  : SpuInst_BinaryMaskI<0b010100, IRF, IRF,    i32imm, "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHLIIp  : SpuInst_BinaryMask <0b010100, IRF, IRF,    IRF,    "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;

defm : BinaryShiftScalar<shl, SHLssp, SHLsip>;
defm : BinaryIndexScalar<shl, SHLIsp, SHLIip>;
def  : Pat<(shl v5i32:$op1, v5i32:$op2),
           (SHLIIp $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
defm : BinaryIndexShiftIntrinsics<int_tpc_shl_mask, "SHL">;

foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(type (int_tpc_shl type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"SHL", type, i8, i1>.Insn
                   type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_shl type:$src1, (i8 imm:$src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"SHL", type, iAny, i1>.Insn
                   type:$src1, (CastToImm32 $src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}


//------ ASH -------------------------------------------------------------------

def ASHssp : SpuInstBinarySwReserved  <0b010101, SRF, SRF, SRF,    "ash$optype $sw $dest, $op1, $op2, $pred">;
def ASHsip : SpuInstBinarySwIReserved <0b010101, SRF, SRF, i8imm,  "ash$optype $sw $dest, $op1, $op2, $pred">;

foreach type = [i32, i16, i8] in {
  def : Pat<(type (int_tpc_ash type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ASHssp
                   type:$src1, i8:$src2, (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ash type:$src1, (i8 imm:$src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ASHsip
                   type:$src1, (i8 imm:$src2), (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

//------ CONVERT ---------------------------------------------------------------

def CONVERTssp : SpuInst_Convert<0b010110, "convert$optype $sw $dst, $src, $pred">;

foreach fromType = [f32, bf16, i8, i16, i32] in
  foreach toType = [f32, bf16, i8, i16, i32] in
    def : Pat<(toType (int_tpc_convert fromType:$src, (i8 imm:$optype), (i32 imm:$sw), toType:$income,  i1:$pred, (i1 imm:$polarity))),
              (toType (CONVERTssp SRF:$src, (i8 imm:$optype), (CastToImm32 $sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


def : Pat<(i32 (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i8  (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT8, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(i16 (fp_to_sint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
def : Pat<(i8  (fp_to_sint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;

def : Pat<(i16 (fp_to_uint f32:$src)), (CONVERTssp
                                           (CONVERTssp
                                               (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                               OpType.INT32, SwitchVal.TO_INT16, (IMPLICIT_DEF), SP0, (i1 0)),
                                           OpType.INT16, SwitchVal.TO_UINT16, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i8  (fp_to_uint f32:$src)), (CONVERTssp (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                                   OpType.INT32, SwitchVal.TO_UINT8, (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(i32 (fp_to_uint bf16:$src)), (CONVERTssp
                                            (CONVERTssp
                                                (CONVERTssp $src, OpType.BF16, SwitchVal.TO_FP32, (IMPLICIT_DEF), SP0, (i1 0)),
                                                OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                            OpType.INT32, SwitchVal.TO_UINT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (fp_to_uint bf16:$src)), (CONVERTssp (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                                    OpType.INT16, SwitchVal.TO_UINT16, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i8  (fp_to_uint bf16:$src)), (CONVERTssp (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                                    OpType.INT16, SwitchVal.TO_UINT8, (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(f32  (fpextend bf16:$src)),   (CONVERTssp $src, OpType.BF16,    !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;

def : Pat<(bf16   (fpround f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;

def : Pat<(f32    (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
def : Pat<(f32    (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
def : Pat<(f32    (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i8:$src)),  (CONVERTssp (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
                                                      OpType.INT16,  !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[Gen2]>;

def : Pat<(f32 (uint_to_fp i32:$src)),  (CONVERTssp (ANDsip i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                                    OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16 (uint_to_fp i32:$src)), (CONVERTssp (ANDsip i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                                    OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;
// Short unsigned integer types may be promoted to longer signed types.
def : Pat<(f32    (uint_to_fp i16:$src)), (CONVERTssp (ANDsip SRF:$src, (i32 0x0ffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16   (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[BF16]>;

def : Pat<(f32    (uint_to_fp i8:$src)),  (CONVERTssp (ANDsip SRF:$src, (i32 0x0ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(bf16   (uint_to_fp i8:$src)),  (CONVERTssp (ANDsip SRF:$src, (i32 0x0ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(i32 (sext i16:$src)), (CONVERTssp $src, OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (sext i8:$src)),  (CONVERTssp $src, OpType.INT8,  SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (sext i8:$src)),  (CONVERTssp $src, OpType.INT8,  SwitchVal.TO_INT16, (IMPLICIT_DEF), SP0, (i1 0))>;


//------ CONVERT_INT32 ---------------------------------------------------------

def CONVERT_INT32ssp   : SpuInstConvertIntGen1<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_INT32sip   : SpuInstConvertIntGen1<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_INT32g2ssp : SpuInstConvertIntGen2<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen2Plus]>;
def CONVERT_INT32g2sip : SpuInstConvertIntGen2<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen2Plus]>;

foreach res_type = [i8, i16] in {
  def : Pat<(res_type (int_tpc_convert_int i32:$src1, i32:$src2, (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_INT32ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
  def : Pat<(res_type (int_tpc_convert_int i32:$src1, (i32 imm:$src2), (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_INT32sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
  // Gen2+ has different switch encoding.
  def : Pat<(res_type (int_tpc_convert_int i32:$src1, i32:$src2, (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_INT32g2ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
  def : Pat<(res_type (int_tpc_convert_int i32:$src1, (i32 imm:$src2), (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_INT32g2sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
}


//------ CONVERT_UINT32 --------------------------------------------------------

def CONVERT_UINT32ssp   : SpuInstConvertIntGen1<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_UINT32sip   : SpuInstConvertIntGen1<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_UINT32g2ssp : SpuInstConvertIntGen2<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen2Plus]>;
def CONVERT_UINT32g2sip : SpuInstConvertIntGen2<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen2Plus]>;

foreach res_type = [i8, i16] in {
  def : Pat<(res_type (int_tpc_convert_uint i32:$src1, i32:$src2, (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_UINT32ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
  def : Pat<(res_type (int_tpc_convert_uint i32:$src1, (i32 imm:$src2), (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_UINT32sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
  // Gen2 has different switch encoding.
  def : Pat<(res_type (int_tpc_convert_uint i32:$src1, i32:$src2, (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_UINT32g2ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
  def : Pat<(res_type (int_tpc_convert_uint i32:$src1, (i32 imm:$src2), (i32 imm:$sw), res_type:$income, i1:$pred, (i1 imm:$polarity))),
            (res_type (CONVERT_UINT32g2sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
}


//------ CONVERT_INT16 ---------------------------------------------------------

def CONVERT_INT16ssp   : SpuInstConvertIntGen1<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_INT16sip   : SpuInstConvertIntGen1<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_INT16g2ssp : SpuInstConvertIntGen2<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen2Plus]>;
def CONVERT_INT16g2sip : SpuInstConvertIntGen2<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen2Plus]>;

def : Pat<(i8 (int_tpc_convert_int i16:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT16ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
def : Pat<(i8 (int_tpc_convert_int i16:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT16sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
// Gen2 has different switch encoding.
def : Pat<(i8 (int_tpc_convert_int i16:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT16g2ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
def : Pat<(i8 (int_tpc_convert_int i16:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT16g2sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;


//------ CONVERT_UINT16 --------------------------------------------------------

def CONVERT_UINT16ssp   : SpuInstConvertIntGen1<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_UINT16sip   : SpuInstConvertIntGen1<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen1]>;
def CONVERT_UINT16g2ssp : SpuInstConvertIntGen2<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[Gen2Plus]>;
def CONVERT_UINT16g2sip : SpuInstConvertIntGen2<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[Gen2Plus]>;

def : Pat<(i8 (int_tpc_convert_uint i16:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT16ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
def : Pat<(i8 (int_tpc_convert_uint i16:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT16sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
// Gen2 has different switch encoding.
def : Pat<(i8 (int_tpc_convert_uint i16:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT16g2ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
def : Pat<(i8 (int_tpc_convert_uint i16:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT16g2sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;


//------ FCLASS ----------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def FCLASSsp : SpuInst_UnarySw<0b011110, SRF, SRF, "fclass$optype $sw $dest, $op, $pred">;

  foreach type = [f32, bf16] in {
    def : Pat<(type (int_tpc_fclass type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
              (type (FCLASSsp SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  }
}


//------ POPCNT ----------------------------------------------------------------

def POPCNTsp : SpuInst_UnarySw<0b011001, SRF, SRF, "popcnt$optype $sw $dest, $op, $pred">;

foreach type = [i8, i16, i32, f32, bf16] in {
  def : Pat<(i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw),  i8:$income,   i1:$pred, (i1 imm:$polarity))),
            (i8 (POPCNTsp        SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

// Lowering of llvm.ctpop
def : Pat<(ctpop i8:$src),
          (i8 (POPCNTsp i8:$src,  OpType.INT8,  SW.SET, (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctpop i16:$src)))),
          (i8 (POPCNTsp i16:$src, OpType.INT16, SW.SET, (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctpop i32:$src)))),
          (i8 (POPCNTsp i32:$src, OpType.INT32, SW.SET, (IMPLICIT_DEF), SP0, (i1 0)))>;


//------ FIND_FIRST ------------------------------------------------------------

def FIND_FIRSTsp : SpuInst_UnarySw<0b011010, SRF, SRF, "find_first$optype $sw $dest, $op, $pred">;

foreach type = [i8, i16, i32, f32, bf16] in
  def : Pat<(i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw),  i8:$income,   i1:$pred, (i1 imm:$polarity))),
            (i8 (FIND_FIRSTsp  SRF:$src,  (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

// Lowering of llvm.ctlz
def : Pat<(ctlz i8:$src),
          (i8 (FIND_FIRSTsp i8:$src,  OpType.INT8,  SW.MSB, (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctlz i16:$src)))),
          (i8 (FIND_FIRSTsp i16:$src, OpType.INT16, SW.MSB, (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctlz i32:$src)))),
          (i8 (FIND_FIRSTsp i32:$src, OpType.INT32, SW.MSB, (IMPLICIT_DEF), SP0, (i1 0)))>;

// Lowering of llvm.cttz
def : Pat<(ctlz i8:$src),
          (i8 (FIND_FIRSTsp i8:$src,  OpType.INT8,  (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctlz i16:$src)))),
          (i8 (FIND_FIRSTsp i16:$src, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctlz i32:$src)))),
          (i8 (FIND_FIRSTsp i32:$src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;


//------ EXTRACT_EXP -----------------------------------------------------------

def EXTRACT_EXPssp : SpuInst_UnarySw <0b011101, SRF, SRF,    "extract_exp$optype $sw $dest, $op, $pred">;
def EXTRACT_EXPsip : SpuInst_UnarySwI<0b011101, SRF, i32imm, "extract_exp$optype $sw $dest, $op, $pred">;

foreach type = [f32, bf16] in {
  def : Pat<(FloatToInteger<type>.Type (int_tpc_extract_exp         type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income,   i1:$pred, (i1 imm:$polarity))),
            (FloatToInteger<type>.Type (EXTRACT_EXPssp               SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(FloatToInteger<type>.Type (int_tpc_extract_exp (type fpimm:$src), (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income,   i1:$pred, (i1 imm:$polarity))),
            (FloatToInteger<type>.Type (EXTRACT_EXPsip       (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}


//------ NEARBYINT -------------------------------------------------------------

def NEARBYINTsp : SpuInst_NEARBYINT<0b011011, SRF, SRF, "nearbyint$optype $sw $dest, $op, $pred">;

foreach type = [f32, bf16] in
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income,   i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTsp        SRF:$src, (i8 imm:$optype), (i32 imm:$sw),  SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


//------ BREV ------------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def BREVssp : SpuInst_UnarySw<0b011111, SRF, SRF, "brev$optype $sw $dest, $op, $pred">;

  foreach type = [f32, bf16, i32, i16, i8] in {
    def : Pat<(FloatToInteger<type>.Type (int_tpc_brev type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income, i1:$pred, (i1 imm:$polarity))),
              (FloatToInteger<type>.Type (BREVssp SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  }
}


//------ JMPA, JMPR ------------------------------------------------------------

let isNotUsedInDisasm = 0, isBranch = 1, isTerminator = 1 in {

// Generic predicable instruction.
def JMPR   : SpuInstJmp <0b100100, "jmpr $target, $pred">;
def JMPA   : SpuInstJmp <0b100101, "jmpa $target, $pred">;

// Unconditional jump. Need to be a barrier.
let isBarrier = 1 in
def JMPR_u : SpuInstJmpU<0b100100, "jmpr $target", [(br bb:$target)]>;

// Register variants. Always indirect.
let isIndirectBranch = 1 in {
def JMPRr  : SpuInstJmpR<0b100100, "jmpr $target, $pred">;
def JMPAr  : SpuInstJmpR<0b100101, "jmpa $target, $pred">;
}

}

def : Pat<(brind SRF:$target), (JMPRr SRF:$target, SP0, (i1 0))>; 
def : Pat<(brcond i1:$cond, bb:$target), (JMPR $target, (i1 $cond), (i1 0))>;
def : Pat<(brcond (i1 (setne i1:$cond, (i1 -1))), bb:$target), (JMPR $target, (i1 $cond), (i1 1))>;
def : Pat<(brcond (i1 (setne i1:$cond, (i1 0))), bb:$target), (JMPR $target, (i1 $cond), (i1 0))>;
def : Pat<(brcond (i1 (seteq i1:$cond, (i1 -1))), bb:$target), (JMPR $target, (i1 $cond), (i1 0))>;
def : Pat<(brcond (i1 (seteq i1:$cond, (i1 0))), bb:$target), (JMPR $target, (i1 $cond), (i1 1))>;

//------ MOV_IRF_DIM -----------------------------------------------------------

def MOV_IRF_DIM : SpuInst_MovDim<0b100110, SRF, IRF, "mov_irf_dim $sw $dim $dest, $src, $pred">;

def : Pat<(i32 (int_tpc_mov_irf_dim v5i32:$src, (i8 imm:$dim), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (MOV_IRF_DIM v5i32:$src, (i8 imm:$dim), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity)))>;

def MOV_IRF_DIM_BOTH : SpuInst_MovIrfDivBoth<0b100110, ZRF, IRF, "mov_irf_dim $sw $dim $dest, $src, $pred">;

def : Pat<(v2i32 (int_tpc_mov_irf_dim v5i32:$src, (i8 imm:$dim), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (v2i32 (MOV_IRF_DIM_BOTH v5i32:$src, (i8 imm:$dim), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity)))>;

//------ SET_INDX --------------------------------------------------------------

def SET_INDX_spu_rp  : SpuInst_SET_INDX <0b100111, "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_rp">;
def SET_INDX_spu_ip  : SpuInst_SET_INDXI<0b100111, "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_ip">;


//------ UDIV_STEP -------------------------------------------------------------

def UDIV_STEP   : SpuInst_UDIV_STEP<0b101000, "udiv_step$optype $sw $step $dest, $src, $pred", 0>, Requires<[Gen1]>;
def UDIV_4STEP  : SpuInst_UDIV_STEP<0b101000, "udiv_4step$optype $sw $step $dest, $src, $pred", 0>, Requires<[Gen2Plus]>;

def : Pat<(v2i32 (int_tpc_udiv_step i32:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;
def : Pat<(v2i32 (int_tpc_udiv_step i16:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;
def : Pat<(v2i32 (int_tpc_udiv_step i8:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw),  v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;

def : Pat<(v2i32 (int_tpc_udiv_step i32:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus]>;
def : Pat<(v2i32 (int_tpc_udiv_step i16:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus]>;
def : Pat<(v2i32 (int_tpc_udiv_step i8:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus]>;


//------ CALC_FP_SPECIAL -------------------------------------------------------

def CALC_FP_SPECIALssp : SpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;

foreach type = [f32, bf16] in
  def : Pat<(type (int_tpc_calc_fp_special type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (CALC_FP_SPECIALssp type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity)))>;


//------ Vector slot -----------------------------------------------------------

// MAC and MUL share many common features, especially combinations of input and
// output register classes. Present them in the classes defined below.

multiclass MacMulInstr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvp : VpuInstBinSwT  <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsp : VpuInstBinSwT  <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vip : VpuInstBinSwTI <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvm : VpuInstBinSwT  <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsm : VpuInstBinSwT  <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vim : VpuInstBinSwTI <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MacMulInstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvp : VpuInstBinSwTA  <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsp : VpuInstBinSwTA  <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vip : VpuInstBinSwTAI <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvm : VpuInstBinSwTA  <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsm : VpuInstBinSwTA  <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vim : VpuInstBinSwTAI <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

class ConstructName<bits<4> Type, string IBaseName> {
  string IName = !strconcat(IBaseName, OperandType<Type>.Suffix);
}

multiclass MacMulIntrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacMulIntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacMulX2Intrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacX2IntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal imm:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacZPIntrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacZPIntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacX2ZPIntrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal imm:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacX2ZPIntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal imm:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast imm:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

//------ MAC -------------------------------------------------------------------

defm MACf32  : MacMulInstr<0b000000, VRF, OpType.FP32,   "mac.f32  $dt$sw $dest, $op1, $op2, $pred">;
defm MACbf16 : MacMulInstr<0b000000, VRF, OpType.BF16,   "mac.bf16 $dt$sw $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MACi16  : MacMulInstr<0b000000, DRF, OpType.INT16,  "mac.i16  $dt$sw $dest, $op1, $op2, $pred">;
defm MACu16  : MacMulInstr<0b000000, DRF, OpType.UINT16, "mac.u16  $dt$sw $dest, $op1, $op2, $pred">;
defm MACi8   : MacMulInstr<0b000000, ARF, OpType.INT8,   "mac.i8   $dt$sw $dest, $op1, $op2, $pred">, Requires<[MULI8]>;
defm MACu8   : MacMulInstr<0b000000, ARF, OpType.UINT8,  "mac.u8   $dt$sw $dest, $op1, $op2, $pred">, Requires<[MULI8]>;

// ACC_FP32
defm MACAbf16  : MacMulInstrAcc<0b000000, DRF, OpType.BF16,    "mac.bf16 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[BF16]>;

defm : MacMulIntrinsics<int_tpc_mac, OpType.FP32,   v64f32,   "MAC", fpimm>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.BF16,   v128bf16, "MAC", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.INT8,   v256i32,  "MAC", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.UINT8,  v256i32,  "MAC", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.INT16,  v128i32,  "MAC", imm>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.UINT16, v128i32,  "MAC", imm>;

defm : MacMulIntrinsicsAcc<int_tpc_mac, OpType.BF16, v128f32, "MACA", fpimm>, Requires<[BF16]>;

//------ MUL -------------------------------------------------------------------

defm MULf32  : MacMulInstr<0b000001, VRF, OpType.FP32,   "mul.f32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULbf16 : MacMulInstr<0b000001, VRF, OpType.BF16,   "mul.bf16 $dt $sw $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MULi32  : MacMulInstr<0b000001, DRF, OpType.INT32,  "mul.i32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULu32  : MacMulInstr<0b000001, DRF, OpType.UINT32, "mul.u32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULi16  : MacMulInstr<0b000001, DRF, OpType.INT16,  "mul.i16 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULu16  : MacMulInstr<0b000001, DRF, OpType.UINT16, "mul.u16 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULi8   : MacMulInstr<0b000001, ARF, OpType.INT8,   "mul.i8 $dt $sw $dest, $op1, $op2, $pred"  >, Requires<[MULI8]>;
defm MULu8   : MacMulInstr<0b000001, ARF, OpType.UINT8,  "mul.u8 $dt $sw $dest, $op1, $op2, $pred"  >, Requires<[MULI8]>;

// Variants with DOUBLE_AND_ROUND32 (i32/u32) and ACC_FP32 (bf16).
defm MULAbf16 : MacMulInstrAcc<0b000001, DRF, OpType.BF16,   "mul.bf16 $dt $sw$acc $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MULAi32  : MacMulInstrAcc<0b000001, VRF, OpType.INT32,  "mul.i32 $dt $sw$acc $dest, $op1, $op2, $pred" >;
defm MULAu32  : MacMulInstrAcc<0b000001, VRF, OpType.UINT32, "mul.u32 $dt $sw$acc $dest, $op1, $op2, $pred" >;

def : Pat<(v64f32 (fmul v64f32:$op1, v64f32:$op2)),
          (v64f32 (MULf32vvp  v64f32:$op1, v64f32:$op2, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v64f32 (fmul v64f32:$op1, (v64f32 (vsplatf32 f32:$op2)))),
          (v64f32 (MULf32vsp  v64f32:$op1, f32:$op2, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
def : Pat<(v64f32 (fmul v64f32:$op1, (v64f32 (vsplatf32 (f32 fpimm:$op2))))),
          (v64f32 (MULf32vip  v64f32:$op1, (FPToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

let Predicates = [BF16] in {
  def : Pat<(v128bf16 (fmul v128bf16:$op1, v128bf16:$op2)),
            (v128bf16 (MULbf16vvp v128bf16:$op1, v128bf16:$op2, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
  def : Pat<(v128bf16 (fmul v128bf16:$op1, (v128bf16 (vsplatbf16 bf16:$op2)))),
            (v128bf16 (MULbf16vsp v128bf16:$op1, bf16:$op2, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
  def : Pat<(v128bf16 (fmul v128bf16:$op1, (v128bf16 (vsplatbf16 (bf16 fpimm:$op2))))),
            (v128bf16 (MULbf16vip v128bf16:$op1, (FPToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;
}


defm : MacMulIntrinsics<int_tpc_mul, OpType.FP32,   v64f32,   "MUL", fpimm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.BF16,   v128bf16, "MUL", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT8,   v256i32,  "MUL", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT8,  v256i32,  "MUL", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT16,  v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT16, v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT32,  v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT32, v128i32,  "MUL", imm>;

defm : MacMulIntrinsicsAcc<int_tpc_mul, OpType.BF16,   v128f32, "MULA", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsicsAcc<int_tpc_mul, OpType.INT32,  v64i32,  "MULA", imm>;
defm : MacMulIntrinsicsAcc<int_tpc_mul, OpType.UINT32, v64i32,  "MULA", imm>;

//------ ADD -------------------------------------------------------------------

def ADDvvp : VpuInstBinSw  <0b000010, VRF, VRF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvsp : VpuInstBinSw  <0b000010, VRF, VRF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvip : VpuInstBinSwI <0b000010, VRF, VRF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvvm : VpuInstBinSw  <0b000010, VRF, VRF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDvsm : VpuInstBinSw  <0b000010, VRF, VRF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDvim : VpuInstBinSwI <0b000010, VRF, VRF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<add,  ADDvvp, ADDvsp, ADDvip>;
defm : BinaryFloatVector<fadd, ADDvvp, ADDvsp, ADDvip>;
foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_add, "ADD", type, pred>;

//------ SUB -------------------------------------------------------------------

def SUBvvp : VpuInstBinSw  <0b000011, VRF, VRF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvsp : VpuInstBinSw  <0b000011, VRF, VRF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvip : VpuInstBinSwI <0b000011, VRF, VRF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvvm : VpuInstBinSw  <0b000011, VRF, VRF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBvsm : VpuInstBinSw  <0b000011, VRF, VRF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBvim : VpuInstBinSwI <0b000011, VRF, VRF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<sub,  SUBvvp, SUBvsp, SUBvip>;
defm : BinaryFloatVector<fsub, SUBvvp, SUBvsp, SUBvip>;
foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_sub, "SUB", type, pred>;

//------ MAX -------------------------------------------------------------------

def MAXvvp  : VpuInstBinSw  <0b000101, VRF, VRF, VRF,    "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvsp  : VpuInstBinSw  <0b000101, VRF, VRF, SRF,    "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvip  : VpuInstBinSwI <0b000101, VRF, VRF, i32imm, "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvvm  : VpuInstBinSw  <0b000101, VRF, VRF, VRF,    "max$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MAXvsm  : VpuInstBinSw  <0b000101, VRF, VRF, SRF,    "max$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MAXvim  : VpuInstBinSwI <0b000101, VRF, VRF, i32imm, "max$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_max, "MAX", type, pred>;


//------ MIN -------------------------------------------------------------------

def MINvvp  : VpuInstBinSw  <0b000110, VRF, VRF, VRF,    "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvsp  : VpuInstBinSw  <0b000110, VRF, VRF, SRF,    "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvip  : VpuInstBinSwI <0b000110, VRF, VRF, i32imm, "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvvm  : VpuInstBinSw  <0b000110, VRF, VRF, VRF,    "min$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MINvsm  : VpuInstBinSw  <0b000110, VRF, VRF, SRF,    "min$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MINvim  : VpuInstBinSwI <0b000110, VRF, VRF, i32imm, "min$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_min, "MIN", type, pred>;


//------ AND -------------------------------------------------------------------

def ANDvvp  : VpuInstBinSw  <0b010000, VRF, VRF, VRF,    "and$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ANDvsp  : VpuInstBinSw  <0b010000, VRF, VRF, SRF,    "and$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ANDvip  : VpuInstBinSwI <0b010000, VRF, VRF, i32imm, "and$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ANDmmp  : VpuInstBinSw  <0b010000, VPRF, VPRF, VPRF, "and$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ANDvvm  : VpuInstBinSw  <0b010000, VRF, VRF, VRF,    "and$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ANDvsm  : VpuInstBinSw  <0b010000, VRF, VRF, SRF,    "and$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ANDvim  : VpuInstBinSwI <0b010000, VRF, VRF, i32imm, "and$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ANDmmm  : VpuInstBinSw  <0b010000, VPRF, VPRF, VPRF, "and$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<and, ANDvvp, ANDvsp, ANDvip>;
def : Pat<(and v256i1:$op1, v256i1:$op2), (ANDmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_and, "AND", type, pred>;

def : Pat<(v256i1 (int_tpc_and v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income,   i1:$pred, (i1 imm:$polarity))),
          (v256i1 (ANDmmp        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
def : Pat<(v256i1 (int_tpc_and v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity))),
          (v256i1 (ANDmmm        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;


//------ OR -------------------------------------------------------------------

def ORvvp  : VpuInstBinSw  <0b001111, VRF, VRF, VRF,    "or$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ORvsp  : VpuInstBinSw  <0b001111, VRF, VRF, SRF,    "or$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ORvip  : VpuInstBinSwI <0b001111, VRF, VRF, i32imm, "or$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ORmmp  : VpuInstBinSw  <0b001111, VPRF, VPRF, VPRF, "or$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ORvvm  : VpuInstBinSw  <0b001111, VRF, VRF, VRF,    "or$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ORvsm  : VpuInstBinSw  <0b001111, VRF, VRF, SRF,    "or$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ORvim  : VpuInstBinSwI <0b001111, VRF, VRF, i32imm, "or$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ORmmm  : VpuInstBinSw  <0b001111, VPRF, VPRF, VPRF, "or$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<or, ORvvp, ORvsp, ORvip>;
def : Pat<(or v256i1:$op1, v256i1:$op2), (ORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_or, "OR", type, pred>;

def : Pat<(v256i1 (int_tpc_or v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income,   i1:$pred, (i1 imm:$polarity))),
          (v256i1 (ORmmp        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
def : Pat<(v256i1 (int_tpc_or v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity))),
          (v256i1 (ORmmm        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;


//------ XOR -------------------------------------------------------------------

def XORvvp  : VpuInstBinSw  <0b010001, VRF, VRF, VRF,    "xor$optype $sw $dest, $op1, $op2, $pred", SPred>;
def XORvsp  : VpuInstBinSw  <0b010001, VRF, VRF, SRF,    "xor$optype $sw $dest, $op1, $op2, $pred", SPred>;
def XORvip  : VpuInstBinSwI <0b010001, VRF, VRF, i32imm, "xor$optype $sw $dest, $op1, $op2, $pred", SPred>;
def XORmmp  : VpuInstBinSw  <0b010001, VPRF, VPRF, VPRF, "xor$optype $sw $dest, $op1, $op2, $pred", SPred>;
def XORvvm  : VpuInstBinSw  <0b010001, VRF, VRF, VRF,    "xor$optype $sw $dest, $op1, $op2, $pred", VPred>;
def XORvsm  : VpuInstBinSw  <0b010001, VRF, VRF, SRF,    "xor$optype $sw $dest, $op1, $op2, $pred", VPred>;
def XORvim  : VpuInstBinSwI <0b010001, VRF, VRF, i32imm, "xor$optype $sw $dest, $op1, $op2, $pred", VPred>;
def XORmmm  : VpuInstBinSw  <0b010001, VPRF, VPRF, VPRF, "xor$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<xor, XORvvp, XORvsp, XORvip>;
def : Pat<(xor v256i1:$op1, v256i1:$op2), (XORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_xor, "XOR", type, pred>;

def : Pat<(v256i1 (int_tpc_xor v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income,   i1:$pred, (i1 imm:$polarity))),
          (v256i1 (XORmmp        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
def : Pat<(v256i1 (int_tpc_xor v256i1:$op1, v256i1:$op2, (i8 imm:$optype), (i32 imm:$sw), v256i1:$income, v256i1:$pred, (i1 imm:$polarity))),
          (v256i1 (XORmmm        VPRF:$op1,   VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw),   VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;


//------ ABS -------------------------------------------------------------------

def ABSvvp : VpuInst_UnarySw<0b000111, VRF, VRF, "abs$optype $sw $dest, $op, $pred", SPred>;
def ABSvvm : VpuInst_UnarySw<0b000111, VRF, VRF, "abs$optype $sw $dest, $op, $pred", VPred>;

defm : UnaryIntegerVector<abs,  ABSvvp>;
//defm : UnaryFloatVector<fabs, ABSvvp>;

foreach type = [v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_abs type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"ABS", type, type, pred>.Insn
                     VRF:$src, (i8 imm:$dt), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

//------ ASH -------------------------------------------------------------------

def ASHvvp : VpuInstBinSwReserved <0b010101, VRF, VRF, VRF, "ash$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ASHvvm : VpuInstBinSwReserved <0b010101, VRF, VRF, VRF, "ash$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ASHvsp : VpuInstBinSwReserved <0b010101, VRF, VRF, SRF, "ash$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ASHvsm : VpuInstBinSwReserved <0b010101, VRF, VRF, SRF, "ash$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ASHvip : VpuInstBinSwIReserved <0b010101, VRF, VRF, i8imm, "ash$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ASHvim : VpuInstBinSwIReserved <0b010101, VRF, VRF, i8imm, "ash$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32] in
  foreach pred = [i1, v256i1] in
    defm : ShiftVectorIntrinsics<int_tpc_ash, "ASH", type, v256i8, pred>;


//------ NOT -------------------------------------------------------------------

def NOTvvp  : VpuInst_UnarySw<0b010010, VRF,  VRF,  "not$optype $sw $dest, $op, $pred", SPred>;
def NOTvvm  : VpuInst_UnarySw<0b010010, VRF,  VRF,  "not$optype $sw $dest, $op, $pred", VPred>;
def NOTmmp  : VpuInst_UnarySw<0b010010, VPRF, VPRF, "not$optype $sw $dest, $op, $pred", SPred>;
def NOTmmm  : VpuInst_UnarySw<0b010010, VPRF, VPRF, "not$optype $sw $dest, $op, $pred", VPred>;

defm : UnaryIntegerVector<inot,  NOTvvp>;
defm : UnaryFloatVector<fpnot, NOTvvp>;

def : Pat<(xor v256i1:$src, (vsplatb (i1 -1))), (NOTmmp VPRF:$src, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8, v256i1] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_not type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"NOT", type, type, pred>.Insn
                     type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;


//------ SHR -------------------------------------------------------------------

def SHRvvp : VpuInstBinSw <0b010011, VRF, VRF, VRF,    "shr$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHRvvm : VpuInstBinSw <0b010011, VRF, VRF, VRF,    "shr$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SHRvsp : VpuInstBinSw <0b010011, VRF, VRF, SRF,    "shr$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHRvsm : VpuInstBinSw <0b010011, VRF, VRF, SRF,    "shr$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SHRvip : VpuInstBinSwI<0b010011, VRF, VRF, i32imm, "shr$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHRvim : VpuInstBinSwI<0b010011, VRF, VRF, i32imm, "shr$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<srl, SHRvvp, SHRvsp, SHRvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : ShiftVectorIntrinsics<int_tpc_shr, "SHR", type, FloatToInteger<type>.Type, pred>;


//------ SHL -------------------------------------------------------------------

def SHLvvp : VpuInstBinSw <0b010100, VRF, VRF, VRF,    "shl$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHLvvm : VpuInstBinSw <0b010100, VRF, VRF, VRF,    "shl$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SHLvsp : VpuInstBinSw <0b010100, VRF, VRF, SRF,    "shl$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHLvsm : VpuInstBinSw <0b010100, VRF, VRF, SRF,    "shl$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SHLvip : VpuInstBinSwI<0b010100, VRF, VRF, i32imm, "shl$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHLvim : VpuInstBinSwI<0b010100, VRF, VRF, i32imm, "shl$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinaryIntegerVector<shl, SHLvvp, SHLvsp, SHLvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    defm : ShiftVectorIntrinsics<int_tpc_shl, "SHL", type, FloatToInteger<type>.Type, pred>;


//------ CMP_EQ ----------------------------------------------------------------

def CMP_EQvvp : VpuInstBinSw <0b001001, VPRF, VRF, VRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvip : VpuInstBinSwI<0b001001, VPRF, VRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvsp : VpuInstBinSw <0b001001, VPRF, VRF, SRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvvm : VpuInstBinSw <0b001001, VPRF, VRF, VRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_EQvim : VpuInstBinSwI<0b001001, VPRF, VRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_EQvsm : VpuInstBinSw <0b001001, VPRF, VRF, SRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<seteq, CMP_EQvvm, CMP_EQvsm, CMP_EQvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_eq, "CMP_EQ", type, pred, v256i1>;
  }
foreach T = [v256i8, v128i16, v64i32] in {
  def : Pat<(seteq (and T:$x0, T:$x1), (T (SplatNode<T>.Splat (VectorElement<T>.Type 0)))),
            (ConstructInsn<"CMP_EQ", T, T, i1>.Insn T:$x0, T:$x1, OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[Gen2Plus]>;
  def : Pat<(seteq (and T:$x0, (T ImmNode<T>.Node:$x1)), (T (SplatNode<T>.Splat (VectorElement<T>.Type 0)))),
            (ConstructInsn<"CMP_EQ", T, iAny, i1>.Insn T:$x0, (CastToImm32 $x1), OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SP0, (i1 0))>, Requires<[Gen2Plus]>;
}


//------ CMP_NEQ ---------------------------------------------------------------

def CMP_NEQvvp : VpuInstBinSw <0b001010, VPRF, VRF, VRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvip : VpuInstBinSwI<0b001010, VPRF, VRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvsp : VpuInstBinSw <0b001010, VPRF, VRF, SRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvvm : VpuInstBinSw <0b001010, VPRF, VRF, VRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_NEQvim : VpuInstBinSwI<0b001010, VPRF, VRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_NEQvsm : VpuInstBinSw <0b001010, VPRF, VRF, SRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<setne, CMP_NEQvvm, CMP_NEQvsm, CMP_NEQvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_neq, "CMP_NEQ", type, pred, v256i1>;
  }


//------ CMP_LESS --------------------------------------------------------------

def CMP_LESSvvp : VpuInstBinSw <0b001011, VPRF, VRF, VRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvip : VpuInstBinSwI<0b001011, VPRF, VRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvsp : VpuInstBinSw <0b001011, VPRF, VRF, SRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvvm : VpuInstBinSw <0b001011, VPRF, VRF, VRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LESSvim : VpuInstBinSwI<0b001011, VPRF, VRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LESSvsm : VpuInstBinSw <0b001011, VPRF, VRF, SRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<setlt, CMP_LESSvvm, CMP_LESSvsm, CMP_LESSvim>;
defm : BinOpSwUVectVP0<setult, CMP_LESSvvm, CMP_LESSvsm, CMP_LESSvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_less, "CMP_LESS", type, pred, v256i1>;
  }


//------ CMP_LEQ ---------------------------------------------------------------

def CMP_LEQvvp : VpuInstBinSw <0b001100, VPRF, VRF, VRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvip : VpuInstBinSwI<0b001100, VPRF, VRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvsp : VpuInstBinSw <0b001100, VPRF, VRF, SRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvvm : VpuInstBinSw <0b001100, VPRF, VRF, VRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LEQvim : VpuInstBinSwI<0b001100, VPRF, VRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LEQvsm : VpuInstBinSw <0b001100, VPRF, VRF, SRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<setle, CMP_LEQvvm, CMP_LEQvsm, CMP_LEQvim>;
defm : BinOpSwUVectVP0<setule, CMP_LEQvvm, CMP_LEQvsm, CMP_LEQvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_leq, "CMP_LEQ", type, pred, v256i1>;
  }


//------ CMP_GRT ---------------------------------------------------------------

def CMP_GRTvvp : VpuInstBinSw <0b001101, VPRF, VRF, VRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvip : VpuInstBinSwI<0b001101, VPRF, VRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvsp : VpuInstBinSw <0b001101, VPRF, VRF, SRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvvm : VpuInstBinSw <0b001101, VPRF, VRF, VRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GRTvim : VpuInstBinSwI<0b001101, VPRF, VRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GRTvsm : VpuInstBinSw <0b001101, VPRF, VRF, SRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<setgt, CMP_GRTvvm, CMP_GRTvsm, CMP_GRTvim>;
defm : BinOpSwUVectVP0<setugt, CMP_GRTvvm, CMP_GRTvsm, CMP_GRTvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_grt, "CMP_GRT", type, pred, v256i1>;
  }


//------ CMP_GEQ ---------------------------------------------------------------

def CMP_GEQvvp : VpuInstBinSw <0b001110, VPRF, VRF, VRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvip : VpuInstBinSwI<0b001110, VPRF, VRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvsp : VpuInstBinSw <0b001110, VPRF, VRF, SRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvvm : VpuInstBinSw <0b001110, VPRF, VRF, VRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GEQvim : VpuInstBinSwI<0b001110, VPRF, VRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GEQvsm : VpuInstBinSw <0b001110, VPRF, VRF, SRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : BinOpSwSVectVP0<setge, CMP_GEQvvm, CMP_GEQvsm, CMP_GEQvim>;
defm : BinOpSwUVectVP0<setuge, CMP_GEQvvm, CMP_GEQvsm, CMP_GEQvim>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_geq, "CMP_GEQ", type, pred, v256i1>;
  }


//------ CONVERT ---------------------------------------------------------------

def CONVERTvvp : VpuInstConvert<0b010110, VRF, VRF, "convert$optype $sw $dst, $src, $pred", SPred>;
def CONVERTvvm : VpuInstConvert<0b010110, VRF, VRF, "convert$optype $sw $dst, $src, $pred", VPred>;

def CONVERTvdp : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen2]>;
def CONVERTvdm : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen2]>;

def CONVERTdvp : VpuInstConvert<0b010110, DRF, VRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen2Plus]>;
def CONVERTdvm : VpuInstConvert<0b010110, DRF, VRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen2Plus]>;

def : Pat<(v64f32 (sint_to_fp v64i32:$src)),
          (CONVERTvvp $src, OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(v64f32 (uint_to_fp v64i32:$src)),
/*FIXME???*/          (CONVERTvvp (ANDvip v64i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0))>;

/* Float to UInt/Int */

foreach type = [v64f32] in
 foreach res_type = ConvertCompatibleTypes<type>.Ts in
  foreach pred = [SP0, VP0] in {
   /* Convert to Signed Integer */
   def : Pat<(res_type (fp_to_sint type:$src)),
             (ConstructConvertInsn<res_type, type, PredTypeForPredStr<!cast<string>(pred)>.P>.Insn
             $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), pred, (i1 0))>;

   /* Convert to Unsigned Integer */
   def : Pat<(res_type (fp_to_uint type:$src)),
             (ConstructConvertInsn<res_type, res_type, PredTypeForPredStr<!cast<string>(pred)>.P>.Insn
               (ConstructConvertInsn<res_type, type, PredTypeForPredStr<!cast<string>(pred)>.P>.Insn
               $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), pred, (i1 0)),
             OpTypeForType<res_type>.V, UnsignedSwitchValForType<res_type>.U, (IMPLICIT_DEF), pred, (i1 0))>;
}

//CONVERTvv*
foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach res_type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
    foreach pred = [i1, v256i1] in {
      def : Pat<(res_type (int_tpc_convert type:$src, (i8 imm:$optype), (i32 imm:$sw), res_type:$income, pred:$pred, (i1 imm:$polarity))),
                (res_type (ConstructInsn<"CONVERT", res_type, type , pred>.Insn
                          VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    }

foreach pred = [i1, v256i1] in {
  def : Pat<(v128bf16 (int_tpc_convert v128f32:$src, (i8 imm:$optype), (i32 imm:$sw), v128bf16:$income, pred:$pred, (i1 imm:$polarity))),
            (v128bf16 (ConstructInsn<"CONVERT", v128bf16, v128f32, pred>.Insn
                      DRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen2]>;
}

//CONVERTdv*
//// [Gen2] (and [Gen2B]) ////
foreach res_type = [v128f32, v256bf16, v256f16, v128i32, v256i16, v512i8] in
  foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
    foreach pred = [i1, v256i1] in {
      def : Pat<(res_type (int_tpc_convert type:$src, (i8 imm:$optype), (i32 imm:$sw), res_type:$income, pred:$pred, (i1 imm:$polarity))),
                (res_type (ConstructInsn<"CONVERT", res_type, type, pred>.Insn
                          type:$src, (i8 imm:$optype), (i32 imm:$sw), res_type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
    }


//------ FCLASS ----------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  let isNotUsedInDisasm = 1 in {
    def FCLASSvsp : VpuInst_UnarySw<0b011110, VRF, SRF, "fclass$optype $sw $dest, $op, $pred", SPred>;
    def FCLASSvsm : VpuInst_UnarySw<0b011110, VRF, SRF, "fclass$optype $sw $dest, $op, $pred", VPred>;
    def FCLASSvvp : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", SPred>;
    def FCLASSvvm : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", VPred>;
  }

  foreach type = [v64f32, v128bf16] in
    foreach pred = [i1, v256i1] in {
      def : Pat<(type (int_tpc_fclass type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FCLASSvv", pred>.Insn
                         VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_fclass VectorElement<type>.Type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FCLASSvs", pred>.Insn
                         SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_fclass (type (SplatNode<type>.Splat VectorElement<type>.Type:$src)), (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FCLASSvs", pred>.Insn
                         SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    }
}


//------ POPCNT ----------------------------------------------------------------

def POPCNTvp : VpuInst_UnarySw<0b011001, VRF, VRF, "popcnt$optype $sw $dest, $op, $pred", SPred>;
def POPCNTvm : VpuInst_UnarySw<0b011001, VRF, VRF, "popcnt$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in {
  def : Pat<(v256i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income,     i1:$pred, (i1 imm:$polarity))),
            (v256i8 (POPCNTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income, v256i1:$pred, (i1 imm:$polarity))),
            (v256i8 (POPCNTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}

//llvm.ctpop
  def : Pat<(ctpop v256i8:$src),
            (v256i8 (POPCNTvp $src, (i8 4), (i32 1), (IMPLICIT_DEF), SP0, (i1 0)))>;


//------ FIND_FIRST ------------------------------------------------------------

def FIND_FIRSTvp : VpuInst_UnarySw<0b011010, VRF, VRF, "find_first$optype $sw $dest, $op, $pred", SPred>;
def FIND_FIRSTvm : VpuInst_UnarySw<0b011010, VRF, VRF, "find_first$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in {
  def : Pat<(v256i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income,     i1:$pred, (i1 imm:$polarity))),
            (v256i8 (FIND_FIRSTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income, v256i1:$pred, (i1 imm:$polarity))),
            (v256i8 (FIND_FIRSTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}
  //llvm ctlz
  def : Pat<(ctlz v256i8:$src),
            (v256i8 (FIND_FIRSTvp $src, (i8 4), (i32 1), (IMPLICIT_DEF), SP0, (i1 0)))>;
  //llvm cttz
  def : Pat<(cttz v256i8:$src),
            (v256i8 (FIND_FIRSTvp $src, (i8 4), (i32 3), (IMPLICIT_DEF), SP0, (i1 0)))>;


 // llvm fminnum
def : Pat<(fminnum v64f32:$v0, v64f32:$v1),
      (v64f32(MINvvp $v0,  $v1, (i8 0), (i32 0),(IMPLICIT_DEF), SP0, (i1 0)))>;
 // llvm fmaxnum
def : Pat<(fmaxnum v64f32:$v0, v64f32:$v1),
      (v64f32(MAXvvp $v0,  $v1, (i8 0), (i32 0),(IMPLICIT_DEF), SP0, (i1 0)))>;


//------ NEARBYINT -------------------------------------------------------------

def NEARBYINTvp : VpuInst_NEARBYINT<0b011011, VRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", SPred>;
def NEARBYINTvm : VpuInst_NEARBYINT<0b011011, VRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", VPred>;

def NEARBYINT_CNVRTvp : VpuInst_NEARBYINT<0b011011, DRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", SPred>;
def NEARBYINT_CNVRTvm : VpuInst_NEARBYINT<0b011011, DRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v64f32, v128bf16] in {
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income,     i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),  VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),  VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;

  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income,     i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvp  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvm  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}


//------ EXTRACT_EXP -----------------------------------------------------------

def EXTRACT_EXPvvp : VpuInst_UnarySw <0b011101, VRF,    VRF, "extract_exp$optype $sw $dest, $op, $pred", SPred>;
def EXTRACT_EXPvip : VpuInst_UnarySwI<0b011101, VRF, i32imm, "extract_exp$optype $sw $dest, $op, $pred", SPred>;
def EXTRACT_EXPvvm : VpuInst_UnarySw <0b011101, VRF,    VRF, "extract_exp$optype $sw $dest, $op, $pred", VPred>;
def EXTRACT_EXPvim : VpuInst_UnarySwI<0b011101, VRF, i32imm, "extract_exp$optype $sw $dest, $op, $pred", VPred>;

foreach ptype = [i1, v256i1] in {
  def : Pat<(v64i32 (int_tpc_extract_exp v64f32:$src, (i8 imm:$optype), (i32 imm:$biased), v64i32:$income, ptype:$pred, (i1 imm:$polarity))),
            (v64i32 (ConstructInsn<"EXTRACT_EXPvv", ptype>.Insn
                     VRF:$src, (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v64i32 (int_tpc_extract_exp (v64f32 (vsplatf32 (f32 fpimm:$src))), (i8 imm:$optype), (i32 imm:$biased), v64i32:$income, ptype:$pred, (i1 imm:$polarity))),
            (v64i32 (ConstructInsn<"EXTRACT_EXPvi", ptype>.Insn
                     (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
}
let Predicates = [BF16] in
  foreach ptype = [i1, v256i1] in {
    def : Pat<(v128i16 (int_tpc_extract_exp v128bf16:$src, (i8 imm:$optype), (i32 imm:$biased), v128i16:$income, ptype:$pred, (i1 imm:$polarity))),
              (v128i16 (ConstructInsn<"EXTRACT_EXPvv", ptype>.Insn
                        VRF:$src, (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
    def : Pat<(v128i16 (int_tpc_extract_exp (v128bf16 (vsplatbf16 (bf16 fpimm:$src))), (i8 imm:$optype), (i32 imm:$biased), v128i16:$income, ptype:$pred, (i1 imm:$polarity))),
              (v128i16 (ConstructInsn<"EXTRACT_EXPvi", ptype>.Insn
                        (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
  }


//------ SEL* ------------------------------------------------------------------

multiclass VpuSelIntrinsicsBin<SDNode Intrin, string IName, ValueType T1, ValueType T2, ValueType P, ValueType RT = T1> {
  // VRF, VRF, VRF, VRF
  def : Pat<(RT (Intrin
                 T2:$src1, T2:$src2, T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, T2, T1, T1, P>.Insn
                 T2:$src1, T2:$src2, T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;

  // VRF, SRF, VRF, VRF
  def : Pat<(RT (Intrin
                 T2:$src1, VectorElement<T2>.Type:$src2, T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, VectorElement<T2>.Type, T1, T1, P>.Insn
                 T2:$src1, VectorElement<T2>.Type:$src2, T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T2:$src1, (T2 (SplatNode<T2>.Splat VectorElement<T2>.Type:$src2)), T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, VectorElement<T2>.Type, T1, T1, P>.Insn
                 T2:$src1, VectorElement<T2>.Type:$src2, T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;

  // VRF, VRF, VRF, SRF
  def : Pat<(RT (Intrin
                 T2:$src1, T2:$src2, T1:$src3, VectorElement<T1>.Type:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, T2, T1, VectorElement<T1>.Type, P>.Insn
                 T2:$src1, T2:$src2, T1:$src3, VectorElement<T1>.Type:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T2:$src1, T2:$src2, T1:$src3, (T1 (SplatNode<T1>.Splat VectorElement<T1>.Type:$src4)),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, T2, T1, VectorElement<T1>.Type, P>.Insn
                 T2:$src1, T2:$src2, T1:$src3, VectorElement<T1>.Type:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;

  // VRF, IMM, VRF, VRF
  def : Pat<(RT (Intrin
                 T2:$src1, (VectorElement<T2>.Type ImmNode<T2>.Node:$src2), T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, iAny, T1, T1, P>.Insn
                 T2:$src1, (CastOp<T2>.Cast $src2), T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T2:$src1, (T2 (SplatNode<T2>.Splat (VectorElement<T2>.Type ImmNode<T2>.Node:$src2))), T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, iAny, T1, T1, P>.Insn
                 T2:$src1, (CastOp<T2>.Cast $src2), T1:$src3, T1:$src4,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;

  // VRF, VRF, VRF, IMM
  def : Pat<(RT (Intrin
                 T2:$src1, T2:$src2, T1:$src3, (VectorElement<T1>.Type ImmNode<T1>.Node:$src4),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, T2, T1, iAny, P>.Insn
                 T2:$src1, T2:$src2, T1:$src3, (CastOp<T1>.Cast $src4),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T2:$src1, T2:$src2, T1:$src3, (T1 (SplatNode<T1>.Splat (VectorElement<T1>.Type ImmNode<T1>.Node:$src4))),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T2, T2, T1, iAny, P>.Insn
                 T2:$src1, T2:$src2, T1:$src3, (CastOp<T1>.Cast $src4),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity)))>;
}

multiclass SelInstr<bits<6> opc, RegisterClass Rdst, string asmstr> {
  def vvvvp : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, VRF,            SPred, asmstr>;
  def vivvp : VpuInst_Sel<opc, Rdst, VRF, TPCImm<i32imm>, VRF, VRF,            SPred, asmstr>;
  def vvvip : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, TPCImm<i32imm>, SPred, asmstr>;
  def vsvvp : VpuInst_Sel<opc, Rdst, VRF, SRF,            VRF, VRF,            SPred, asmstr>;
  def vvvsp : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, SRF,            SPred, asmstr>;
  def vvvvm : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, VRF,            VPred, asmstr>;
  def vivvm : VpuInst_Sel<opc, Rdst, VRF, TPCImm<i32imm>, VRF, VRF,            VPred, asmstr>;
  def vvvim : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, TPCImm<i32imm>, VPred, asmstr>;
  def vsvvm : VpuInst_Sel<opc, Rdst, VRF, SRF,            VRF, VRF,            VPred, asmstr>;
  def vvvsm : VpuInst_Sel<opc, Rdst, VRF, VRF,            VRF, SRF,            VPred, asmstr>;
}


class SelCompatibleTypes<ValueType T> {
  list<ValueType> Ts = !if(!eq(!cast<string>(T), "v64f32"),     [v64f32, v64i32],
                       !if(!eq(!cast<string>(T), "v64i32"),     [v64i32, v64f32],
                       !if(!eq(!cast<string>(T), "v128i16"),    [v128i16, v128bf16],
                       !if(!eq(!cast<string>(T), "v128bf16"),   [v128bf16, v128i16],
                       !if(!eq(!cast<string>(T), "v256i8"),     [v256i8],
                       [])))));
}

defm SEL_EQ   : SelInstr<0b100010, VRF, "sel_eq$optype     $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL_NEQ  : SelInstr<0b100011, VRF, "sel_neq$optype    $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL_LESS : SelInstr<0b100100, VRF, "sel_less$optype   $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL_LEQ  : SelInstr<0b100101, VRF, "sel_leq$optype    $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL_GRT  : SelInstr<0b100110, VRF, "sel_grt$optype    $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL_GEQ  : SelInstr<0b100111, VRF, "sel_geq$optype    $sw $dest, $op1, $op2, $op3, $op4, $pred">;

defm SEL2_LESS : SelInstr<0b101000, DRF, "sel2_less$optype $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL2_LEQ  : SelInstr<0b101001, DRF, "sel2_leq$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL2_GRT  : SelInstr<0b101010, DRF, "sel2_grt$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">;
defm SEL2_GEQ  : SelInstr<0b101011, DRF, "sel2_geq$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">;

foreach T1 = [v256i8, v128i16, v64i32, v64f32, v128bf16] in
    foreach T2 = SelCompatibleTypes<T1>.Ts in
        foreach pred = [i1, v256i1] in {
            // SEL*
            defm : VpuSelIntrinsicsBin<int_tpc_sel_eq,     "SEL_EQ",   T1, T2, pred>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel_neq,    "SEL_NEQ",  T1, T2, pred>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel_less,   "SEL_LESS", T1, T2, pred>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel_leq,    "SEL_LEQ",  T1, T2, pred>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel_grt,    "SEL_GRT",  T1, T2, pred>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel_geq,    "SEL_GEQ",  T1, T2, pred>;

            // SEL2*
            defm : VpuSelIntrinsicsBin<int_tpc_sel2_less, "SEL2_LESS", T1, T2, pred, DoubleVector<T1>.Type>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel2_leq,  "SEL2_LEQ",  T1, T2, pred, DoubleVector<T1>.Type>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel2_grt,  "SEL2_GRT",  T1, T2, pred, DoubleVector<T1>.Type>;
            defm : VpuSelIntrinsicsBin<int_tpc_sel2_geq,  "SEL2_GEQ",  T1, T2, pred, DoubleVector<T1>.Type>;
        }

//------ MOV_GROUP ------------------------------------------------------------------

def MOV_GROUP_vpu_vp : VpuInst_MOV_GROUP<0b110010, "mov_g $sw $dest, $src, $imm, $pred", SPred>;
def MOV_GROUP_vpu_vm : VpuInst_MOV_GROUP<0b110010, "mov_g $sw $dest, $src, $imm, $pred", VPred>;
foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16] in {
    def : Pat<(type (int_tpc_mov_group  type:$src, (i32 imm:$imm), (i32 imm:$sw), type:$income,     i1:$pred, (i1 imm:$polarity))),
              (type (MOV_GROUP_vpu_vp    VRF:$src, (i32 imm:$imm), (i32 imm:$sw),  VRF:$income,     i1:$pred, (i1 imm:$polarity)))>;
    def : Pat<(type (int_tpc_mov_group  type:$src, (i32 imm:$imm), (i32 imm:$sw), type:$income, v256i1:$pred, (i1 imm:$polarity))),
              (type (MOV_GROUP_vpu_vm    VRF:$src, (i32 imm:$imm), (i32 imm:$sw),  VRF:$income, v256i1:$pred, (i1 imm:$polarity)))>;
}

//------ MSAC ------------------------------------------------------------------

def MSACvvvvp : VpuInst_Msac<0b110011, VRF, VRF, VRF, VRF, SPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvvvvm : VpuInst_Msac<0b110011, VRF, VRF, VRF, VRF, VPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvsvvp : VpuInst_Msac<0b110011, VRF, SRF, VRF, VRF, SPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvsvvm : VpuInst_Msac<0b110011, VRF, SRF, VRF, VRF, VPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvvvsp : VpuInst_Msac<0b110011, VRF, VRF, VRF, SRF, SPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvvvsm : VpuInst_Msac<0b110011, VRF, VRF, VRF, SRF, VPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvivvp : VpuInst_Msac<0b110011, VRF, TPCImm<i32imm>, VRF, VRF, SPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvivvm : VpuInst_Msac<0b110011, VRF, TPCImm<i32imm>, VRF, VRF, VPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvvvip : VpuInst_Msac<0b110011, VRF, VRF, VRF, TPCImm<i32imm>, SPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;
def MSACvvvim : VpuInst_Msac<0b110011, VRF, VRF, VRF, TPCImm<i32imm>, VPred, "msac$optype  $sw $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[MULI8]>;

foreach T = [v256i8, v128i16] in
  foreach pred = [i1, v256i1] in {
    // VRF, VRF, VRF, VRF
    def : Pat<(T (int_tpc_msac
                  T:$src1, T:$src2, v256i8:$src3, v256i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity))),
              (T (ConstructInsn<"MSACvvvv", pred>.Insn
                  T:$src1, T:$src2, v256i8:$src3, v256i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity)))>;
    // VRF, SRF, VRF, VRF
    def : Pat<(T (int_tpc_msac
                  T:$src1, VectorElement<T>.Type:$src2, v256i8:$src3, v256i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity))),
              (T (ConstructInsn<"MSACvsvv", pred>.Insn
                   T:$src1, VectorElement<T>.Type:$src2, v256i8:$src3, v256i8:$src4,
                   (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity)))>;
    // VRF, VRF, VRF, SRF
    def : Pat<(T (int_tpc_msac
                  T:$src1, T:$src2, v256i8:$src3, i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity))),
              (T (ConstructInsn<"MSACvvvs", pred>.Insn
                  T:$src1, T:$src2, v256i8:$src3, VectorElement<v256i8>.Type:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity)))>;
    // VRF, IMM, VRF, VRF
    def : Pat<(T (int_tpc_msac
                  T:$src1, (VectorElement<T>.Type ImmNode<T>.Node:$src2), v256i8:$src3, v256i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity))),
              (T (ConstructInsn<"MSACvivv", pred>.Insn
                  T:$src1, (CastOp<T>.Cast $src2), v256i8:$src3, v256i8:$src4,
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity)))>;
    // VRF, VRF, VRF, IMM
    def : Pat<(T (int_tpc_msac
                  T:$src1, T:$src2, v256i8:$src3, (i8 imm:$src4),
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity))),
              (T (ConstructInsn<"MSACvvvi", pred>.Insn
                  T:$src1, T:$src2, v256i8:$src3, (CastOp<v256i8>.Cast $src4),
                  (i8 imm:$optype), (i32 imm:$sw), T:$income, pred:$P, (i1 imm:$polarity)))>;
  }


//------ CONVERT ------------------------------------------------------------------

multiclass CONVERT_INTG1<bits<6> OpCode, string asmstr> {
  def vvp  : VpuInstConvertIntGen1 <OpCode, VRF, SPred, asmstr, 0 /*hasImm*/>, Requires<[Gen1]>;
  def vsp  : VpuInstConvertIntGen1 <OpCode, SRF, SPred, asmstr, 0 /*hasImm*/>, Requires<[Gen1]>;
  def vip  : VpuInstConvertIntGen1 <OpCode, SRF, SPred, asmstr, 1 /*hasImm*/>, Requires<[Gen1]>;
  def vvm  : VpuInstConvertIntGen1 <OpCode, VRF, VPred, asmstr, 0 /*hasImm*/>, Requires<[Gen1]>;
  def vsm  : VpuInstConvertIntGen1 <OpCode, SRF, VPred, asmstr, 0 /*hasImm*/>, Requires<[Gen1]>;
  def vim  : VpuInstConvertIntGen1 <OpCode, SRF, VPred, asmstr, 1 /*hasImm*/>, Requires<[Gen1]>;
}

multiclass CONVERT_INTG2<bits<6> OpCode, RegisterClass Rsrc1, string asmstr> {
  def vvp  : VpuInstConvertIntGen2 <OpCode, Rsrc1, VRF, SPred, asmstr, 0 /*hasImm*/>, Requires<[Gen2Plus]>;
  def vsp  : VpuInstConvertIntGen2 <OpCode, Rsrc1, SRF, SPred, asmstr, 0 /*hasImm*/>, Requires<[Gen2Plus]>;
  def vip  : VpuInstConvertIntGen2 <OpCode, Rsrc1, SRF, SPred, asmstr, 1 /*hasImm*/>, Requires<[Gen2Plus]>;
  def vvm  : VpuInstConvertIntGen2 <OpCode, Rsrc1, VRF, VPred, asmstr, 0 /*hasImm*/>, Requires<[Gen2Plus]>;
  def vsm  : VpuInstConvertIntGen2 <OpCode, Rsrc1, SRF, VPred, asmstr, 0 /*hasImm*/>, Requires<[Gen2Plus]>;
  def vim  : VpuInstConvertIntGen2 <OpCode, Rsrc1, SRF, VPred, asmstr, 1 /*hasImm*/>, Requires<[Gen2Plus]>;
}

multiclass ConvertIntIntrinsics<SDNode Intrin, string IName, ValueType dt, ValueType st, ValueType sht, ValueType pred> {
  def : Pat<(dt (Intrin st:$src1, sht:$src2, (i32 imm:$sw), dt:$income, pred:$pred, (i1 imm:$polarity))),
            (dt (ConstructInsn<IName, v64i32, v256i8, pred>.Insn
                 st:$src1, VRF:$src2, (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(dt (Intrin st:$src1, VectorElement<sht>.Type:$src2, (i32 imm:$sw), dt:$income, pred:$pred, (i1 imm:$polarity))),
            (dt (ConstructInsn<IName, v64i32, i32, pred>.Insn
                 st:$src1, SRF:$src2, (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(dt (Intrin st:$src1, (sht (SplatNode<sht>.Splat VectorElement<sht>.Type:$src2)), (i32 imm:$sw), dt:$income, pred:$pred, (i1 imm:$polarity))),
            (dt (ConstructInsn<IName, v64i32, i32, pred>.Insn
                 st:$src1, SRF:$src2, (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(dt (Intrin st:$src1, (VectorElement<sht>.Type imm:$src2), (i32 imm:$sw), dt:$income, pred:$pred, (i1 imm:$polarity))),
            (dt (ConstructInsn<IName, v64i32, iAny, pred>.Insn
                 st:$src1, (CastToImm32 $src2), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(dt (Intrin st:$src1, (SplatNode<sht>.Splat (VectorElement<sht>.Type imm:$src2)), (i32 imm:$sw), dt:$income, pred:$pred, (i1 imm:$polarity))),
            (dt (ConstructInsn<IName, v64i32, iAny, pred>.Insn
                 st:$src1, (CastToImm32 $src2), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
}

defm CONVERT_INT32      : CONVERT_INTG1<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT32g2    : CONVERT_INTG2<0b010111, VRF, "convert_int32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT32g3i16 : CONVERT_INTG2<0b010111, DRF, "convert_int32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT32g3i8  : CONVERT_INTG2<0b010111, ARF, "convert_int32 $sw $dest, $op1, $op2, $pred">;

foreach pred = [i1, v256i1] in {
  foreach res_type = [v256i8, v128i16] in {
    defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32",   res_type,  v64i32, v256i8, pred>, Requires<[Gen1]>;
    defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen2]>;
  }
}

defm CONVERT_UINT32      : CONVERT_INTG1<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g2    : CONVERT_INTG2<0b011000, VRF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g3i16 : CONVERT_INTG2<0b011000, DRF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g3i8  : CONVERT_INTG2<0b011000, ARF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;

foreach pred = [i1, v256i1] in {
  foreach res_type = [v256i8, v128i16] in {
    defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32",   res_type,  v64i32, v256i8, pred>, Requires<[Gen1]>;
    defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen2]>;
  }
}


defm CONVERT_INT16    : CONVERT_INTG1<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT16g2  : CONVERT_INTG2<0b000100, VRF, "convert_int16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT16g3A : CONVERT_INTG2<0b000100, DRF, "convert_int16 $sw $dest, $op1, $op2, $pred">;

foreach pred = [i1, v256i1] in {
  defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16",    v256i8, v128i16, v256i8,  pred>, Requires<[Gen1]>;
  defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16g2",  v256i8, v128i16, v256i8,  pred>, Requires<[Gen2]>;
}


defm CONVERT_UINT16    : CONVERT_INTG1<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT16g2  : CONVERT_INTG2<0b011100, VRF, "convert_uint16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT16g3A : CONVERT_INTG2<0b011100, DRF, "convert_uint16 $sw $dest, $op1, $op2, $pred">;

foreach pred = [i1, v256i1] in {
  defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16",    v256i8, v128i16, v256i8,  pred>, Requires<[Gen1]>;
  defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16g2",  v256i8, v128i16, v256i8,  pred>, Requires<[Gen2]>;
}

defm CONVERT_INT8   : CONVERT_INTG2<0b110101, DRF, "convert_int8 $sw $dest, $op1, $op2, $pred">;

defm CONVERT_UINT8   : CONVERT_INTG2<0b110110, DRF, "convert_uint8 $sw $dest, $op1, $op2, $pred">;


//------ BREV ------------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def BREVvvp : VpuInst_UnarySw<0b011111, VRF, VRF, "brev$optype $sw $dest, $op, $pred", SPred>;
  def BREVvvm : VpuInst_UnarySw<0b011111, VRF, VRF, "brev$optype $sw $dest, $op, $pred", VPred>;
  def BREVvsp : VpuInst_UnarySw<0b011111, VRF, SRF, "brev$optype $sw $dest, $op, $pred", SPred>;
  def BREVvsm : VpuInst_UnarySw<0b011111, VRF, SRF, "brev$optype $sw $dest, $op, $pred", VPred>;

  foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
    foreach pred = [i1, v256i1] in {
    def : Pat<(FloatToInteger<type>.Type (int_tpc_brev type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income, pred:$pred, (i1 imm:$polarity))),
              (FloatToInteger<type>.Type (ConstructInsn<"BREV", type, type, pred>.Insn
                                          VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(FloatToInteger<type>.Type (int_tpc_brev VectorElement<type>.Type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income, pred:$pred, (i1 imm:$polarity))),
              (FloatToInteger<type>.Type (ConstructInsn<"BREV", type, VectorElement<type>.Type, pred>.Insn
                                          SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(FloatToInteger<type>.Type (int_tpc_brev
                                          (type (SplatNode<type>.Splat VectorElement<type>.Type:$src)),
                                          (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income, pred:$pred, (i1 imm:$polarity))),
              (FloatToInteger<type>.Type (ConstructInsn<"BREV", type, VectorElement<type>.Type, pred>.Insn
                                          SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    }
}


//------ PACK ------------------------------------------------------------------

def PACKp : VpuInst_PACK<0b101101, VRF, VRF, "pack$optype $sw $dest, $op, $pred", SPred>;
def PACKm : VpuInst_PACK<0b101101, VRF, VRF, "pack$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v128bf16, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in
    def : Pat <(type (int_tpc_pack type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"PACK", pred>.Insn VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;


//------ UNPACK ----------------------------------------------------------------

def UNPACKp : VpuInst_PACK<0b101110, VRF, VRF, "unpack$optype $sw $dest, $op, $pred", SPred>;
def UNPACKm : VpuInst_PACK<0b101110, VRF, VRF, "unpack$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v128bf16, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in
    def : Pat <(type (int_tpc_unpack type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"UNPACK", pred>.Insn VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;


//------ SHUFFLE ---------------------------------------------------------------

def SHUFFLEvvp : VpuInstBinSw<0b101100, VRF, VRF, VRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHUFFLEvvm : VpuInstBinSw<0b101100, VRF, VRF, VRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat <(type (int_tpc_shuffle type:$src1, v256i8:$src2, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"SHUFFLEvv", pred>.Insn VRF:$src1, VRF:$src2, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
}

//------ GET_LUT_ENTRY_AND_INTERVAL_START --------------------------------------

def GET_LUT_ENTRYp: VpuInst_GET_LUT<0b101111, "get_lut_entry_and_interval_start$optype $sw $dest, $src, $shift, $pred", [], SPred>;
def GET_LUT_ENTRYm: VpuInst_GET_LUT<0b101111, "get_lut_entry_and_interval_start$optype $sw $dest, $src, $shift, $pred", [], VPred>;

foreach pred = [i1, v256i1] in {
  def : Pat<(v128i32 (int_tpc_get_lut_entry v64f32:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v128i32:$income, pred:$pred, (i1 imm:$polarity))),
            (v128i32 (ConstructInsn<"GET_LUT_ENTRY", pred>.Insn VRF:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v128i32:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i16 (int_tpc_get_lut_entry v128bf16:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i16 (ConstructInsn<"GET_LUT_ENTRY", pred>.Insn VRF:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity)))>;
}


//------ FORM_FP_NUMBER --------------------------------------------------------

def FORM_FP_NUMvvvp  : VpuInst_FORM_FP_NUM<0b110000, VRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMvvvm  : VpuInst_FORM_FP_NUM<0b110000, VRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
def FORM_FP_NUMsvvp  : VpuInst_FORM_FP_NUM<0b110000, SRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMsvvm  : VpuInst_FORM_FP_NUM<0b110000, SRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
def FORM_FP_NUMivvp  : VpuInst_FORM_FP_NUM<0b110000, TPCImm<i32imm>, "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMivvm  : VpuInst_FORM_FP_NUM<0b110000, TPCImm<i32imm>, "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;

foreach type = [v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    foreach src1t = [type, v256i8] in {
      def : Pat<(type (int_tpc_form_fp_num
                       src1t:$src1, type:$src2, type:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FORM_FP_NUMvvv", pred>.Insn
                       VRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_form_fp_num
                       VectorElement<src1t>.Type:$src1, type:$src2, type:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FORM_FP_NUMsvv", pred>.Insn
                       SRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_form_fp_num
                       (VectorElement<src1t>.Type ImmNode<src1t>.Node:$src1), type:$src2, type:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FORM_FP_NUMivv", pred>.Insn
                       (CastOp<src1t>.Cast $src1), VRF:$src2, VRF:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_form_fp_num
                       (src1t (SplatNode<src1t>.Splat VectorElement<src1t>.Type:$src1)),
                       type:$src2, type:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FORM_FP_NUMsvv", pred>.Insn
                       SRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(type (int_tpc_form_fp_num
                       (src1t (SplatNode<src1t>.Splat (VectorElement<src1t>.Type ImmNode<src1t>.Node:$src1))),
                       type:$src2, type:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"FORM_FP_NUMivv", pred>.Insn
                       (CastOp<src1t>.Cast $src1), VRF:$src2, VRF:$src3, (i8 imm:$dt), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
    }

foreach type = [v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_abs type:$src, (i8 imm:$dt), (i32 imm), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"FORM_FP_NUMvvv", pred>.Insn
                     VRF:$src, VRF:$src, VRF:$src, (i8 imm:$dt), (i32 SW.FORCE_SIGN0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

foreach type = [v64f32, v128bf16] in
  def : Pat<(type (fabs type:$src)),
            (type (FORM_FP_NUMvvvp $src, $src, $src, (i8 0), (i32 SW.FORCE_SIGN0), (IMPLICIT_DEF), SP0, (i1 0)))>;


//------ MOV_DUAL_GROUP --------------------------------------------------------

def MOV_DUAL_GROUPp : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", SPred>;
def MOV_DUAL_GROUPm : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", VPred>;

def MOV_DUAL_GROUP_ALLp : VpuInst_MOV_DUAL_GROUP_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src, $imm, $pred", SPred>, Requires<[Gen2Plus]>;
def MOV_DUAL_GROUP_ALLm : VpuInst_MOV_DUAL_GROUP_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src, $imm, $pred", VPred>, Requires<[Gen2Plus]>;

foreach type = [v64f32, v128bf16, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat <(type (int_tpc_mov_dual_group type:$src, (i32 imm:$wren), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"MOV_DUAL_GROUP", pred>.Insn
                      VRF:$src, (i32 imm:$wren), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat <(type (int_tpc_mov_dual_group_all type:$src, (i32 imm:$wren), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"MOV_DUAL_GROUP_ALL", pred>.Insn
                      VRF:$src, (i32 imm:$wren), (AddDualGroupAllSwitch $sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
  }


//------ CALC_FP_SPECIAL -------------------------------------------------------

def CALC_FP_SPECIALvvp : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", SPred>, Requires<[Gen2Plus]>;
def CALC_FP_SPECIALvvm : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", VPred>, Requires<[Gen2Plus]>;

foreach type = [v64f32, v128bf16] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_calc_fp_special type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"CALC_FP_SPECIALvv", pred>.Insn
                     type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity)))>;


//====== Store slot ============================================================

multiclass StoreIntrinsics<SDNode IntrinA1, SDNode IntrinA2, string IName> {
  let Predicates = [Addr2] in
    foreach pred = [i1] in
      foreach type = [v256i1, v256i8, v128i16, v64i32,
                      v64f32, v128bf16] in {
        def : Pat<(IntrinA2 i32:$base, i32:$ofs, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, i32, i32, type, pred>.Insn
                     SRF:$base, SRF:$ofs, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
        def : Pat<(IntrinA2 i32:$base, (i32 imm:$ofs), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, i32, iAny, type, pred>.Insn
                     SRF:$base, (i32 imm:$ofs), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
      }
  let Predicates = [Addr1] in
    foreach pred = [i1] in
      foreach type = [v256i1, v256i8, v128i16, v64i32,
                      v64f32, v128bf16] in {
        def : Pat<(IntrinA1 (i32 imm:$addr), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, iAny, type, pred>.Insn
                     (i32 imm:$addr), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
        def : Pat<(IntrinA1 i32:$addr, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, i32, type, pred>.Insn
                     SRF:$addr, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
      }
  // Helper for Gen1, which has no 1-component address.
  let Predicates = [Gen1] in
    foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32] in {
      def : Pat<(IntrinA1 (i32 imm:$addr), type:$src, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
                (ConstructInsn<IName, i32, iAny, type, i1>.Insn
                   (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                   (i32 0), type:$src, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
      def : Pat<(IntrinA1 i32:$addr, type:$src, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
                (ConstructInsn<IName, i32, iAny, type, i1>.Insn
                   SRF:$addr, (i32 0), type:$src, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
    }
}

//------ GEN_ADDR --------------------------------------------------------------

def GEN_ADDR_st  : StoreInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.DimMask>, MultiSlot <"GEN_ADDR">;

def : Pat<(int_tpc_gen_addr v5i32:$coord, (i8 imm:$tensor), (i32 imm:$sw), i64:$income, i1:$pred, (i1 imm:$polarity)),
          (GEN_ADDR_st (i8 imm:$tensor), IRF:$coord, (i32 imm:$sw), $income, $pred, (i1 imm:$polarity))>;


//------ PRMT_INDX -------------------------------------------------------------

def PRMT_INDX_st : StoreInstrPRMT_INDX<0b000011, "prmt_indx $sw $dest, $src1, $src2, $pred">, MultiSlot<"PRMT_INDX">;

def : Pat<(int_tpc_prmt_indx v5i32:$src1, i32:$src2, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (PRMT_INDX_st IRF:$src1, SRF:$src2, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;


//------ SET_INDX --------------------------------------------------------------

def SET_INDX_st_rp  : StoreInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_rp">;
def SET_INDX_st_ip  : StoreInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_ip">;


//------ ST_L ------------------------------------------------------------------

def ST_Lssp : StoreInst_ST_L <0b000101, SRF,  "st_l $sw $addr, $value, $pred">;
def ST_Lisp : StoreInst_ST_LI<0b000101, SRF,  "st_l $sw $addr, $value, $pred">;
def ST_Lspp : StoreInst_ST_L <0b000101, SPRF, "st_l $sw $addr, $value, $pred">;
def ST_Lipp : StoreInst_ST_LI<0b000101, SPRF, "st_l $sw $addr, $value, $pred">;

def : Pat<(lsstore i1:$value, (i32 imm:$addr)), (ST_Lipp (CastToImm32 $addr), (i1 $value), (i32 0), SP0, (i1 0))>;
def : Pat<(lsstore i1:$value,       i32:$addr), (ST_Lspp $addr,               (i1 $value), (i32 0), SP0, (i1 0))>;
foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(lsstore type:$value, (i32 imm:$addr)), (ST_Lisp (CastToImm32 $addr), type:$value, (i32 0), SP0, (i1 0))>;
  def : Pat<(lsstore type:$value, i32:$addr      ), (ST_Lssp $addr,               type:$value, (i32 0), SP0, (i1 0))>;
}

def : Pat<(int_tpc_st_l (i32 imm:$addr), i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_Lipp (CastToImm32 $addr), i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_st_l i32:$addr, i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_Lspp $addr, i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;

foreach type = [f32, bf16, i32, i16, i8] in {
  def : Pat<(int_tpc_st_l (i32 imm:$addr), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_Lisp (CastToImm32 $addr), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_l i32:$addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_Lssp $addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
}


//------ ST_G ------------------------------------------------------------------

def ST_Gs : StoreInst_ST_G<0b000110, SRF,  "st_g $sw $addr, $value, $pred">;
def ST_Gp : StoreInst_ST_G<0b000110, SPRF, "st_g $sw $addr, $value, $pred">;

foreach type = [i1, i8, i16, i32, f32, bf16] in {
  def : Pat<(gstore type:$value, i64:$addr),
            (ConstructInsn<"ST_G", type>.Insn i64:$addr, type:$value, (i32 0), SP0, (i1 0))>;
}

// Patterns for intrinsics 'int_tpc_st_g_b'. See getTgtMemIntrinsic() and LowerINTRINSIC_W_CHAIN()
// in TPCISelLowering.cpp for how the node 'gstorep' is mapped to 'int_tpc_st_g_b'
foreach type = [i1, i8, i16, i32, f32, bf16] in {
  def : Pat<(gstorep i64:$addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_G", type>.Insn ADRF:$addr, type:$value, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
}


//------ ST_L_V ----------------------------------------------------------------

defm ST_L_V : StoreInst_ST_L_V<0b000111, "st_l_v $sw $addr, $src, $pred">;

let Predicates = [Addr1] in
  foreach type = [v64f32, v128bf16,
                  v64i32, v128i16, v256i8, v256i1] in {
    def : Pat<(lvstore type:$src, i32:$addr),
              (ConstructInsn<"ST_L_V", i32, type, i1>.Insn
                 i32:$addr, type:$src, (i32 0), SP0, (i1 0))>;
    def : Pat<(lvstore type:$src, (i32 imm:$addr)),
              (ConstructInsn<"ST_L_V", iAny, type, i1>.Insn
                 (i32 imm:$addr), type:$src, (i32 0), SP0, (i1 0))>;
  }
let Predicates = [Gen1] in
  foreach type = [v64f32, v128bf16,
                  v64i32, v128i16, v256i8, v256i1] in {
    def : Pat<(lvstore type:$src, (i32 AddrRR:$addr)),
              (ConstructInsn<"ST_L_V", i32, i32, type, i1>.Insn
                 AddrRR:$addr, type:$src, (i32 0), SP0, (i1 0))>;
    def : Pat<(lvstore type:$src, (i32 AddrRI:$addr)),
              (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                 AddrRI:$addr, type:$src, (i32 0), SP0, (i1 0))>;
  }

// Helper for Gen1, which has no 1-component address.
let Predicates = [Gen1] in
  foreach type = [v64f32, v64i32, v128i16, v256i8, v256i1] in {
    def : Pat<(lvstore type:$src, (i32 imm:$ptr)),
               (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                  (MOVsip (i32 imm:$ptr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  (i32 0), type:$src, (i32 0), SP0, (i1 0))>;
    def : Pat<(lvstore type:$src, i32:$ptr),
               (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                  SRF:$ptr, (i32 0), type:$src, (i32 0), SP0, (i1 0))>;
  }

defm : StoreIntrinsics<int_tpc_st_l_v, int_tpc_st_l_v_ofs, "ST_L_V">;

defm : StoreIntrinsics<int_tpc_st_l_v, int_tpc_st_l_v_ofs, "ST_L_V">;


//------ ST_L_V_LOW ------------------------------------------------------------

//------ ST_L_V_LOW ------------------------------------------------------------

defm ST_L_V_LOW : StoreInst_ST_L_V<0b001000, "st_l_v_low $sw $addr, $src, $pred">;

defm : StoreIntrinsics<int_tpc_st_l_v_low, int_tpc_st_l_v_low_ofs, "ST_L_V_LOW">;


//------ ST_L_V_HIGH -----------------------------------------------------------

defm ST_L_V_HIGH : StoreInst_ST_L_V<0b001001, "st_l_v_high $sw $addr, $src, $pred">;

defm : StoreIntrinsics<int_tpc_st_l_v_high, int_tpc_st_l_v_high_ofs, "ST_L_V_HIGH">;


//------ ASO -------------------------------------------------------------------

def ASO : StoreInstA<0b001010, "aso $sw $pred", []>;

def : Pat<(int_tpc_aso (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ASO (i32 imm:$sw), $pred, (i1 imm:$polarity))>;


//------ ST_TNSR ---------------------------------------------------------------

multiclass St_Tnsr<bits<6> OpCode, string asmstr> {
  def vp       : StoreInst_ST_TNSR     <OpCode, VRF,  asmstr>;
  def mp       : StoreInst_ST_TNSR     <OpCode, VPRF, asmstr>;
  def Gen2Tvp  : StoreInst_ST_TNSR_T   <OpCode, VRF,  asmstr, 0>, Requires<[Gen2]>;
  def Gen2Tmp  : StoreInst_ST_TNSR_T   <OpCode, VPRF, asmstr, 0>, Requires<[Gen2]>;
}

multiclass St_Tnsr_RMW<bits<6> OpCode, string asmstr> {
  def Gen2vp  : StoreInst_ST_TNSR_RMW     <OpCode, VRF,  asmstr, 0>, Requires<[Gen2]>;
  def Gen2mp  : StoreInst_ST_TNSR_RMW     <OpCode, VPRF, asmstr, 0>, Requires<[Gen2]>;
  def Gen2Tvp : StoreInst_ST_TNSR_RMW_T   <OpCode, VRF,  asmstr, 0>, Requires<[Gen2]>;
  def Gen2Tmp : StoreInst_ST_TNSR_RMW_T   <OpCode, VPRF, asmstr, 0>, Requires<[Gen2]>;
  def vp      : StoreInst_ST_TNSR_RMW     <OpCode, VRF,  asmstr, 1>, Requires<[RMW]>;
  def mp      : StoreInst_ST_TNSR_RMW     <OpCode, VPRF, asmstr, 1>, Requires<[RMW]>;
}

defm ST_TNSR   : St_Tnsr    <0b001011, "st_tnsr $sw $tensor, $coords, $value, $pred">;
defm ST_TNSR_R : St_Tnsr_RMW<0b001011, "st_tnsr $sw $tensor, $coords, $value, $rmw, $pred">;

// Partial store operations that use S31 to keep Offset+Size. We don't use
// 'FeaturePartial' for them, as this feature assumes offset+size is specified
// in HW registers. S31 is used for this purpose only in Gen2, so there is no
// dedicated feature.
def ST_TNSR_PGen2vp   : StoreInst_ST_TNSR_P       <0b001011, VRF,  "st_tnsr $sw $tensor, $coords, $value, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2mp   : StoreInst_ST_TNSR_P       <0b001011, VPRF, "st_tnsr $sw $tensor, $coords, $value, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2Tvp  : StoreInst_ST_TNSR_P_T     <0b001011, VRF,  "st_tnsr $sw $tensor, $coords, $value, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2Tmp  : StoreInst_ST_TNSR_P_T     <0b001011, VPRF, "st_tnsr $sw $tensor, $coords, $value, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2Rvp  : StoreInst_ST_TNSR_P_RMW   <0b001011, VRF,  "st_tnsr $sw $tensor, $coords, $value, $rmw, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2Rmp  : StoreInst_ST_TNSR_P_RMW   <0b001011, VPRF, "st_tnsr $sw $tensor, $coords, $value, $rmw, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2RTvp : StoreInst_ST_TNSR_P_RMW_T <0b001011, VRF,  "st_tnsr $sw $tensor, $coords, $value, $rmw, $offsize, $pred", 0>, Requires<[Gen2]>;
def ST_TNSR_PGen2RTmp : StoreInst_ST_TNSR_P_RMW_T <0b001011, VPRF, "st_tnsr $sw $tensor, $coords, $value, $rmw, $offsize, $pred", 0>, Requires<[Gen2]>;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;

  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr v5i32:$ndx, S28, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSRGen2T", type, i1>.Insn v5i32:$ndx,
             S28, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, i8:$tensor, type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2RT", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;

  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW]>;
}


//------ ST_TNSR_LOW -----------------------------------------------------------

defm ST_TNSR_LOW   : St_Tnsr    <0b001100, "st_tnsr_low $sw $tensor, $coords, $value, $pred">;
defm ST_TNSR_LOW_R : St_Tnsr_RMW<0b001100, "st_tnsr_low $sw $tensor, $coords, $value, $rmw, $pred">;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_low v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_tnsr_low v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOWGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
            Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
}


//------ ST_TNSR_HIGH ----------------------------------------------------------

defm ST_TNSR_HIGH   : St_Tnsr    <0b001101, "st_tnsr_high $sw $tensor, $coords, $value, $pred">;
defm ST_TNSR_HIGH_R : St_Tnsr_RMW<0b001101, "st_tnsr_high $sw $tensor, $coords, $value, $rmw, $pred">;

foreach type = [v256i1, v64f32, v128bf16, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_high v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_tnsr_high v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGHGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;

}


//------ CACHE_FLUSH -----------------------------------------------------------

let hasSideEffects = 1 in
  def CACHE_FLUSH : StoreInstPred<0b010100, "cache_flush $sw $pred">;

def : Pat<(int_tpc_cache_flush (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_FLUSH (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;


//------ CACHE_INVALIDATE ------------------------------------------------------

let hasSideEffects = 1 in
  def CACHE_INVALIDATE : StoreInstPred<0b010101, "cache_invalidate $sw $pred">;

def : Pat<(int_tpc_cache_invalidate (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_INVALIDATE (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;


//------------------------------------------------------------------------------

// Convert patterns
// TODO: patterns for unsigned truncation

// TODO: should we use CONVERT instead?
def : Pat<(i32 (zext i16:$src)), (ANDsip i16:$src, (i32 0x0000ffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (zext i8:$src)), (ANDsip i8:$src, (i32 0x000000ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (zext i8:$src)), (ANDsip i8:$src, (i32 0x000000ff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;


def : Pat<(i16 (trunc i32:$src)), (COPY $src)>;
def : Pat<(i8  (trunc i32:$src)), (COPY $src)>;
def : Pat<(i8  (trunc i16:$src)), (COPY $src)>;


def : Pat<(i32 (anyext i16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (anyext i8:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (anyext i8:$src)), (COPY SRF:$src)>;


def srac32  : PatFrag<(ops node:$src), (i32 (sra node:$src,(i32 31))) >;
def addsra32 :PatFrag<(ops node:$src), (i32 (add node:$src,(srac32 node:$src)))>;
def : Pat<( xor (addsra32 i32:$src1), (srac32 i32:$src1) 
           )
           ,(i32  (ABSssp i32:$src1, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)
             ))
        >;

def srac16  : PatFrag<(ops node:$src), (i16 (sra node:$src,(i32 15))) >;
def addsra16 :PatFrag<(ops node:$src), (i16 (add node:$src,(srac16 node:$src)))>;
def : Pat<( xor (addsra16 i16:$src1), (srac16 i16:$src1) 
           )
           ,(i16  (ABSssp i16:$src1, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)
             ))
        >;

def srac8  : PatFrag<(ops node:$src), (i8 (sra node:$src,(i32 7))) >;
def addsra8 :PatFrag<(ops node:$src), (i8 (add node:$src,(srac8 node:$src)))>;
def : Pat<( xor (addsra8 i8:$src1), (srac8 i8:$src1) 
           )
           ,(i8  (ABSssp i8:$src1, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)
             ))
        >;

// trans intr minmax patterns
//min/max pattern to fold after Intrinsic transformation
//int32
def : Pat< (select
                  (i1 (setcc i32:$src1, imm:$src2, SETGT)),
                   i32:$src1,
                   imm:$src2
            ),
          (MAXsip i32:$src1,imm:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0)
          )
         >;
def : Pat< (select   
                  (i1 (setcc i32:$src1, i32:$src2, SETGT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MAXssp i32:$src1,i32:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, imm:$src2, SETLT)),
                   i32:$src1,
                   imm:$src2
            ),
          (MINsip i32:$src1,imm:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0)
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, i32:$src2, SETLT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MINssp i32:$src1,i32:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i32:$src1, (i32 imm :$src2), SETGT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MAXsip i32:$src1,(i32 imm :$src2),OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, (i32 imm:$src2), SETLT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MINsip i32:$src1,(i32 imm :$src2),OpType.INT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////
//int16
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETGT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MAXssp i16:$src1,i16:$src2,OpType.INT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETLT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MINssp i16:$src1,i16:$src2,OpType.INT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i16:$src1, (i16 imm :$src2), SETGT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MAXsip i16:$src1,(CastToImm32 $src2),OpType.INT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i16:$src1, (i16 imm:$src2), SETLT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MINsip i16:$src1,(CastToImm32 $src2),OpType.INT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//int8
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETGT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MAXssp i8:$src1,i8:$src2,OpType.INT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETLT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MINssp i8:$src1,i8:$src2,OpType.INT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i8:$src1, (i8 imm :$src2), SETGT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MAXsip i8:$src1,(CastToImm32 $src2),OpType.INT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i8:$src1, (i8 imm:$src2), SETLT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MINsip i8:$src1,(CastToImm32 $src2),OpType.INT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////
// unsigned min/max
//uint32
def : Pat< (select   
                  (i1 (setcc i32:$src1, i32:$src2, SETUGT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MAXssp i32:$src1,i32:$src2,OpType.UINT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, i32:$src2, SETULT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MINssp i32:$src1,i32:$src2,OpType.UINT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i32:$src1, (i32 imm :$src2), SETUGT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MAXsip i32:$src1,(i32 imm :$src2),OpType.UINT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, (i32 imm:$src2), SETULT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MINsip i32:$src1,(i32 imm :$src2),OpType.UINT32,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//uint16
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETUGT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MAXssp i16:$src1,i16:$src2,OpType.UINT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETULT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MINssp i16:$src1,i16:$src2,OpType.UINT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i16:$src1, (i16 imm :$src2), SETUGT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MAXsip i16:$src1,(CastToImm32 $src2),OpType.UINT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i16:$src1, (i16 imm:$src2), SETULT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MINsip i16:$src1,(CastToImm32 $src2),OpType.UINT16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//int8
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETUGT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MAXssp i8:$src1,i8:$src2,OpType.UINT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETULT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MINssp i8:$src1,i8:$src2,OpType.UINT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////*
def : Pat< (select   
                  (i1 (setcc i8:$src1, (i8 imm :$src2), SETUGT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MAXsip i8:$src1,(CastToImm32 $src2),OpType.UINT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i8:$src1, (i8 imm:$src2), SETULT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MINsip i8:$src1,(CastToImm32 $src2),OpType.UINT8,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

def : Pat<(bf16 (selectcc bf16:$tval, bf16:$fval, bf16:$tval, bf16:$fval, SETOGT)),
          (MAXssp bf16:$tval,bf16:$fval,OpType.BF16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
               )
         >;

def : Pat<(bf16 (selectcc bf16:$tval, bf16:$fval, bf16:$tval, bf16:$fval, SETOLT)),
          (MINssp bf16:$tval,bf16:$fval,OpType.BF16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
               )
         >;

def : Pat<(bf16 (selectcc bf16:$tval, (bf16 fpimm:$fval), bf16:$tval, (bf16 fpimm:$fval), SETOGT)),
          (MAXsip bf16:$tval,(FPToImm32 $fval),OpType.BF16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
               )
         >;

def : Pat<(bf16 (selectcc bf16:$tval, (bf16 fpimm:$fval), bf16:$tval, (bf16 fpimm:$fval), SETOLT)),
          (MINsip bf16:$tval,(FPToImm32 $fval),OpType.BF16,(i32 0), (IMPLICIT_DEF), SP0, (i1 0) 
               )
         >;


// Bit extend patterns.
//def : Pat<(select i1:$cond, i32:$tvalue, i32:$fvalue),
//          (CMOVIi32si $tvalue, $cond, (CMOVi32si $fvalue, $cond))>;


// Bitcast patterns
def : Pat<(v64f32  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v64i32:$src)),  (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v128i16:$src)), (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v256i8:$src)),  (COPY VRF:$src)>;

def : Pat<(v128bf16 (bitconvert v64f32:$src)),  (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v64i32:$src)),  (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v128i16:$src)), (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v256i8:$src)),  (COPY VRF:$src)>;

def : Pat<(v64i32  (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v128i16:$src)),  (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v256i8:$src)),   (COPY VRF:$src)>;

def : Pat<(v128i16 (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v64i32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v256i8:$src)),   (COPY VRF:$src)>;

def : Pat<(v256i8  (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v256i8  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v256i8  (bitconvert v64i32:$src)),   (COPY VRF:$src)>;
def : Pat<(v256i8  (bitconvert v128i16:$src)),  (COPY VRF:$src)>;

def : Pat<(v128f32 (bitconvert v256bf16:$src)), (COPY DRF:$src)>;
def : Pat<(v128f32 (bitconvert v256f16:$src)),  (COPY DRF:$src)>;
def : Pat<(v128f32 (bitconvert v128i32:$src)),  (COPY DRF:$src)>;
def : Pat<(v128f32 (bitconvert v256i16:$src)),  (COPY DRF:$src)>;
def : Pat<(v128f32 (bitconvert v512i8:$src)),   (COPY DRF:$src)>;

def : Pat<(v256bf16 (bitconvert v128f32:$src)), (COPY DRF:$src)>;
def : Pat<(v256bf16 (bitconvert v256i16:$src)), (COPY DRF:$src)>;
def : Pat<(v256bf16 (bitconvert v256f16:$src)), (COPY DRF:$src)>;
def : Pat<(v256bf16 (bitconvert v128i32:$src)), (COPY DRF:$src)>;
def : Pat<(v256bf16 (bitconvert v512i8:$src)),  (COPY DRF:$src)>;

def : Pat<(v256f16 (bitconvert v128f32:$src)),  (COPY DRF:$src)>;
def : Pat<(v256f16 (bitconvert v256bf16:$src)), (COPY DRF:$src)>;
def : Pat<(v256f16 (bitconvert v128i32:$src)),  (COPY DRF:$src)>;
def : Pat<(v256f16 (bitconvert v256i16:$src)),  (COPY DRF:$src)>;
def : Pat<(v256f16 (bitconvert v512i8:$src)),   (COPY DRF:$src)>;

def : Pat<(v128i32 (bitconvert v128f32:$src)),  (COPY DRF:$src)>;
def : Pat<(v128i32 (bitconvert v256bf16:$src)), (COPY DRF:$src)>;
def : Pat<(v128i32 (bitconvert v256f16:$src)),  (COPY DRF:$src)>;
def : Pat<(v128i32 (bitconvert v256i16:$src)),  (COPY DRF:$src)>;
def : Pat<(v128i32 (bitconvert v512i8:$src)),   (COPY DRF:$src)>;

def : Pat<(v256i16 (bitconvert v128f32:$src)),  (COPY DRF:$src)>;
def : Pat<(v256i16 (bitconvert v256bf16:$src)), (COPY DRF:$src)>;
def : Pat<(v256i16 (bitconvert v256f16:$src)),  (COPY DRF:$src)>;
def : Pat<(v256i16 (bitconvert v512i8:$src)),   (COPY DRF:$src)>;

def : Pat<(v512i8 (bitconvert v128f32:$src)),   (COPY DRF:$src)>;
def : Pat<(v512i8 (bitconvert v256bf16:$src)),  (COPY DRF:$src)>;
def : Pat<(v512i8 (bitconvert v256f16:$src)),   (COPY DRF:$src)>;
def : Pat<(v512i8 (bitconvert v128i32:$src)),   (COPY DRF:$src)>;
def : Pat<(v512i8 (bitconvert v256i16:$src)),   (COPY DRF:$src)>;

def : Pat<(v256f32 (bitconvert v256i32:$src)),  (COPY ARF:$src)>;
def : Pat<(v256i32 (bitconvert v256f32:$src)),  (COPY ARF:$src)>;

def : Pat<(i32 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i8:$src)),  (COPY SRF:$src)>;

def : Pat<(i16 (mul i16:$src1, i16:$src2)),       (MULssp SRF:$src1, SRF:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, 0)>;
def : Pat<(i8  (mul i8:$src1, i8:$src2)),         (MULssp SRF:$src1, SRF:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, 0)>;


// ASH instruction performs arithmetic shift of src1: dst = src1 * (2^src2)
// in the case of right shift src2 sould be less than 0. Consequently, we should inverse src2 by subsription from 0.
// Full chain of arithmetic shift right:
// MOV Reg1 0
// SUB Reg2 Reg1 src2
// ASH Reg3 src1 Reg2
// ---------------------------------------------------------
// TODO: change subscription from 0 to using NEG switch in SUB instruction

def : Pat<(sra i32:$src1, i32:$src2),
          (ASHssp SRF:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          SRF:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                 OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra i32:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         imm:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                 OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(sra i16:$src1, i16:$src2),
          (ASHssp SRF:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                        i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                 OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra i16:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (ANDsip (SUBsip (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                 (i32 imm:$src2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(sra i8:$src1, i8:$src2),
          (ASHssp SRF:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                           SRF:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra i8:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (ANDsip (SUBsip (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                                 (i32 imm:$src2), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(sra v64i32:$src1, v64i32:$src2),
          (ASHvvp v64i32:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         v64i32:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v64i32:$src1, (v64i32 (vsplati32 i32:$src2))),
          (ASHvsp v64i32:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          i32:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v64i32:$src1, (v64i32 (vsplati32 (i32 imm:$src2)))),
          (ASHvsp v64i32:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         (i32 imm:$src2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(sra v128i16:$src1, v128i16:$src2),
          (ASHvvp v128i16:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          v128i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v128i16:$src1, (v128i16 (vsplati16 i16:$src2))),
          (ASHvsp v128i16:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v128i16:$src1, (v128i16 (vsplati16 (i16 imm:$src2)))),
          (ASHvsp v128i16:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         (i32 (CastToImm32 $src2)), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(sra v256i8:$src1, v256i8:$src2),
          (ASHvvp v256i8:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          v256i8:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v256i8:$src1, (v256i8 (vsplati8 i8:$src2))),
          (ASHvsp v256i8:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                          i8:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(sra v256i8:$src1, (v256i8 (vsplati8 (i8 imm:$src2)))),
          (ASHvsp v256i8:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                         (i32 (CastToImm32 $src2)), OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(fneg f32:$e), (XORsip f32:$e, 0x80000000, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(fneg bf16:$e), (XORsip bf16:$e, 0x8000, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(fneg v64f32:$e), (XORvip v64f32:$e, 0x80000000, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(fneg v128bf16:$e), (XORvip v128bf16:$e, 0x8000, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

// NaN check
def : Pat<(seto f32:$f,  f32:$f),  (CMP_EQssp f32:$f,  f32:$f,  OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(seto bf16:$f, bf16:$f), (CMP_EQssp bf16:$f, bf16:$f, OpType.BF16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

// build_vector
def : Pat<(v5i32 (build_vector i32:$a1, i32:$a2, i32:$a3, i32:$a4, i32:$a5)),
          (v5i32 (SET_INDX_ld_rp
                   (SET_INDX_ld_rp
                     (SET_INDX_ld_rp
                       (SET_INDX_ld_rp
                         (SET_INDX_ld_rp (v5i32 (IMPLICIT_DEF)),
        i32:$a1, (i32 1), (i32 0), SP0, (i1 0)),
        i32:$a2, (i32 2), (i32 0), SP0, (i1 0)),
        i32:$a3, (i32 4), (i32 0), SP0, (i1 0)),
        i32:$a4, (i32 8), (i32 0), SP0, (i1 0)),
        i32:$a5, (i32 16), (i32 0), SP0, (i1 0)))>;

def : Pat<(v5i32 (build_vector (i32 imm:$a1), (i32 imm:$a2), (i32 imm:$a3), (i32 imm:$a4), (i32 imm:$a5))),
          (v5i32 (SET_INDX_ld_ip
                   (SET_INDX_ld_ip
                     (SET_INDX_ld_ip
                       (SET_INDX_ld_ip
                         (SET_INDX_ld_ip (v5i32 (IMPLICIT_DEF)),
        (CastToImm32 $a1), (i32 1), (i32 0), SP0, (i1 0)),
        (CastToImm32 $a2), (i32 2), (i32 0), SP0, (i1 0)),
        (CastToImm32 $a3), (i32 4), (i32 0), SP0, (i1 0)),
        (CastToImm32 $a4), (i32 8), (i32 0), SP0, (i1 0)),
        (CastToImm32 $a5), (i32 16), (i32 0), SP0, (i1 0)))>;

def : Pat<(v5i32 (build_vector i32:$a, i32:$a, i32:$a, i32:$a, i32:$a)),
          (v5i32 (SET_INDX_ld_rp  (v5i32 (IMPLICIT_DEF)), i32:$a, (i32 31), (i32 0), SP0, (i1 0)))>;

def : Pat<(v5i32 (build_vector (i32 imm:$a), (i32 imm:$a), (i32 imm:$a), (i32 imm:$a), (i32 imm:$a))),
          (v5i32 (SET_INDX_ld_ip  (v5i32 (IMPLICIT_DEF)), (CastToImm32 $a), (i32 31), (i32 0), SP0, (i1 0)))>;


// insertelt
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 0))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 1), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 1))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec,i32:$e, (i32 2), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 2))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 4), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 3))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec,i32:$e, (i32 8), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 4))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 16), (i32 0), SP0, (i1 0)))>;

def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 0))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 1), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 1))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 2), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 2))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 4), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 3))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 8), (i32 0), SP0, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 4))),
          (v5i32 (SET_INDX_ld_ip  IRF:$vec, (CastToImm32 $e), (i32 16), (i32 0), SP0, (i1 0)))>;


def : Pat<(i32 (extractelt v5i32:$vec, imm:$dim)),
          (i32 (MOV_IRF_DIM $vec, (CastToImm32 $dim), (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

def : Pat<(insertelt v5i32:$vec,
                     (add (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ADDsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (add (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ADDiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (SUBsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (SUBiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub i32:$item,
                      (extractelt v5i32:$vec, (i32 imm:$dim))),
                     (i32 imm:$dim)),
          (SUBsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (i32 imm:$item),
                      (extractelt v5i32:$vec, (i32 imm:$dim))),
                     (i32 imm:$dim)),
          (SUBiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (mul (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (MULsIp i32:$item, v5i32:$vec, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (mul (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (MULiIp (CastToImm32 $item), v5i32:$vec, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (or (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ORsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (or (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ORiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (and (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ANDsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (and (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ANDiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (xor (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (XORsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (xor (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (XORiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SP0, (i1 0))>;

def : Pat<(v5i32 (int_tpc_get_index_space_offset)), (COPY I0)>;
def : Pat<(v5i32 (int_tpc_get_index_space_size)), (COPY I1)>;

// Extendind loads
def : Pat<(i32 (l_load_sext8 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT8, SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_sext8 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT8, SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_zext8 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_zext8 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (l_load_sext8 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT8, SwitchVal.TO_INT16, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (l_load_sext8 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT8, SwitchVal.TO_INT16, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (l_load_zext8 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xffff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i16 (l_load_zext8 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xffff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_sext16 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_sext16 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_zext16 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;
def : Pat<(i32 (l_load_zext16 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)), (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))>;

// Truncating stores
def : Pat<(truncstore_bf16 f32:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SP0, (i1 0)),
              (i32 0), SP0, (i1 0))>,
      Requires<[BF16]>;

def : Pat<(i1 i1true), (COPY SP0)>;
def : Pat<(v256i1 (vsplatb i1true)), (COPY VP0)>;

// IndVarSimplifyPass when it recognizes arithmetical progression:
//
// for(i..upper) {s=+i;} ==> s=upper*(upper-1)/2;
//
def : Pat<(i32 (trunc (srl (mul (zext i32:$x), (zext i32:$y)), (i32 1)))),
          (i32 (SHRsip (MULssp i32:$x, i32:$y, OpType.UINT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)),
                       (i32 1), OpType.UINT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))>;

// Check that is not NAN, special case for ternary_cond.c G-403
def f32cc_uo : Pat<(i1(setcc f32:$op1, f32:$op1, SETUO)),
                   (i1 (CMP_NEQssp $op1, $op1, OpType.FP32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0)))
           >;

// workaround for LLVM-1638, TODO more generic solution

// patterns for setcc with integer result instead boolean
def v64i32cc_eq : Pat<(v64i32(setcc v64i32:$src1, v64i32:$src2, SETEQ)),
            ( v64i32 (SEL_EQvvvip
                 v64i32:$src1, v64i32:$src2,
                 (v64i32 (MOV_ld_vip (i32 1), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))),
                 (i32 0),
                 OpType.INT32, (i32 0),(IMPLICIT_DEF), SP0, (i1 0)))>;

def v64i32cc_ugt : Pat<(v64i32(setcc v64i32:$src1, v64i32:$src2, SETUGT)),
            ( v64i32 (SEL_GRTvvvip
                 v64i32:$src1, v64i32:$src2,
                 (v64i32 (MOV_ld_vip (i32 1), OpType.INT32, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))),
                 (i32 0),
                 OpType.INT32, (i32 0),(IMPLICIT_DEF), SP0, (i1 0)))>;

def v128i16cc_ugt : Pat<(v128i16(setcc v128i16:$src1, v128i16:$src2, SETUGT)),
            ( v128i16 (SEL_GRTvvvip
                 v128i16:$src1, v128i16:$src2,
                 (v128i16 (MOV_ld_vip (i32 1), OpType.INT16, (i32 0), (IMPLICIT_DEF), SP0, (i1 0))),
                 (i32 0),
                 OpType.INT16, (i32 0),(IMPLICIT_DEF), SP0, (i1 0)))>;


// The instruction with maximum number of arguments in asm string does not
// tell vector predicates form scalar predicates.
// So as workaround we created dummy instruction with a lot of arguments
def DUMMY : VPUInstDummy<0b000000, "dummy $op1, $op2, $op3, $op4, $op5, $op6, $op7,"
                                          "$op8, $op9, $op10, $op11, $op12, $op13,"
                                          "$op14, $op15, $op16, $op17, $op18, $op19, $op20">;

// Saturation arithmetic intrinsics.

multiclass SatScalar<ValueType T, SDNode Intrinsic, SDNode Iss, SDNode Isi, bits<4> OType> {
  def : Pat<(T (Intrinsic T:$x1, T:$x2)),
            (T (Iss SRF:$x1, SRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (T imm:$x2))),
            (T (Isi SRF:$x1, (CastToImm32 $x2), OType, SW.ST, (IMPLICIT_DEF), SP0, (i1 0)))>;
}
multiclass SatVector<ValueType T, SDNode Intrinsic, SDNode Ivv, SDNode Ivs, SDNode Ivi, bits<4> OType> {
  def : Pat<(T (Intrinsic T:$x1, T:$x2)),
            (T (Ivv VRF:$x1, VRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (SplatNode<T>.Splat VectorElement<T>.Type:$x2))),
            (T (Ivs VRF:$x1, SRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SP0, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$x2)))),
            (T (Ivi VRF:$x1, (CastToImm32 $x2), OType, SW.ST, (IMPLICIT_DEF), SP0, (i1 0)))>;
}

defm : SatScalar<i32, saddsat, ADDssp, ADDsip, OpType.INT32>;
defm : SatScalar<i16, saddsat, ADDssp, ADDsip, OpType.INT16>;
defm : SatScalar<i8 , saddsat, ADDssp, ADDsip, OpType.INT8>;
defm : SatScalar<i32, uaddsat, ADDssp, ADDsip, OpType.UINT32>;
defm : SatScalar<i16, uaddsat, ADDssp, ADDsip, OpType.UINT16>;
defm : SatScalar<i8 , uaddsat, ADDssp, ADDsip, OpType.UINT8>;

defm : SatVector<v64i32,  saddsat, ADDvvp, ADDvsp, ADDvip, OpType.INT32>;
defm : SatVector<v128i16, saddsat, ADDvvp, ADDvsp, ADDvip, OpType.INT16>;
defm : SatVector<v256i8,  saddsat, ADDvvp, ADDvsp, ADDvip, OpType.INT8>;
defm : SatVector<v64i32,  uaddsat, ADDvvp, ADDvsp, ADDvip, OpType.UINT32>;
defm : SatVector<v128i16, uaddsat, ADDvvp, ADDvsp, ADDvip, OpType.UINT16>;
defm : SatVector<v256i8,  uaddsat, ADDvvp, ADDvsp, ADDvip, OpType.UINT8>;

defm : SatScalar<i32, ssubsat, SUBssp, SUBsip, OpType.INT32>;
defm : SatScalar<i16, ssubsat, SUBssp, SUBsip, OpType.INT16>;
defm : SatScalar<i8 , ssubsat, SUBssp, SUBsip, OpType.INT8>;
defm : SatScalar<i32, usubsat, SUBssp, SUBsip, OpType.UINT32>;
defm : SatScalar<i16, usubsat, SUBssp, SUBsip, OpType.UINT16>;
defm : SatScalar<i8 , usubsat, SUBssp, SUBsip, OpType.UINT8>;

defm : SatVector<v64i32,  ssubsat, SUBvvp, SUBvsp, SUBvip, OpType.INT32>;
defm : SatVector<v128i16, ssubsat, SUBvvp, SUBvsp, SUBvip, OpType.INT16>;
defm : SatVector<v256i8,  ssubsat, SUBvvp, SUBvsp, SUBvip, OpType.INT8>;
defm : SatVector<v64i32,  usubsat, SUBvvp, SUBvsp, SUBvip, OpType.UINT32>;
defm : SatVector<v128i16, usubsat, SUBvvp, SUBvsp, SUBvip, OpType.UINT16>;
defm : SatVector<v256i8,  usubsat, SUBvvp, SUBvsp, SUBvip, OpType.UINT8>;


include "TPCDisasmInstrInfo.td"
