//===-- TPCInstrInfo.td ------ TPC Instruction Definition --*- tablegen -*-===//
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
def gloadIp  :  SDNode<"TPCISD::LD_GI", SDTypeProfile<1, 6, []>,
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

def truncstore_f16 : PatFrag<(ops node:$val, node:$ptr),
                             (truncstore node:$val, node:$ptr),
                             [{
    const auto *St = cast<StoreSDNode>(N);
    return St->getMemoryVT() == MVT::f16 &&
           St->getAddressSpace() == 1;
}]> {
  let IsStore = 1;
  let MemoryVT = f16;
}

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

def truncstore_f8_143 : PatFrag<(ops node:$val, node:$ptr),
                                (truncstore node:$val, node:$ptr),
                                [{
    const auto *St = cast<StoreSDNode>(N);
    return St->getMemoryVT() == MVT::f8_143 &&
           St->getAddressSpace() == 1;
}]> {
  let IsStore = 1;
  let MemoryVT = f8_143;
}

def truncstore_f8_152 : PatFrag<(ops node:$val, node:$ptr),
                                (truncstore node:$val, node:$ptr),
                                [{
    const auto *St = cast<StoreSDNode>(N);
    return St->getMemoryVT() == MVT::f8_152 &&
           St->getAddressSpace() == 1;
}]> {
  let IsStore = 1;
  let MemoryVT = f8_152;
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

def UpperLaneSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() | TPCII::SW_NEARBY_LANES, SDLoc(N), MVT::i32);
}]>;

def LkpPart1Switch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() | TPCII::SW_LKP_PART1, SDLoc(N), MVT::i32);
}]>;

def LkpPart2Switch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() | TPCII::SW_LKP_PART2, SDLoc(N), MVT::i32);
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
                   !if(!eq(!cast<string>(T), "f8_143"), v256f8_143,
                   !if(!eq(!cast<string>(T), "f8_152"), v256f8_152,
                   !if(!eq(!cast<string>(T), "bf16"),   v128bf16,
                   !if(!eq(!cast<string>(T), "f16"),    v128f16,
                   !if(!eq(!cast<string>(T), "f32"),    v64f32,
                   !if(!eq(!cast<string>(T), "i8"),     v256i8,
                   !if(!eq(!cast<string>(T), "i16"),    v128i16,
                   !if(!eq(!cast<string>(T), "i32"),    v64i32,
                   isVoid)))))))));
}

class VectorElement<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "v256i1"),   i1,
                   !if(!eq(!cast<string>(T), "v128i1"),   i1,
                   !if(!eq(!cast<string>(T), "v64i1"),    i1,
                   !if(!eq(!cast<string>(T), "v512f8_143"), f8_143,
                   !if(!eq(!cast<string>(T), "v512f8_152"), f8_152,
                   !if(!eq(!cast<string>(T), "v256f8_143"), f8_143,
                   !if(!eq(!cast<string>(T), "v256f8_152"), f8_152,
                   !if(!eq(!cast<string>(T), "v128bf16"), bf16,
                   !if(!eq(!cast<string>(T), "v256bf16"), bf16,
                   !if(!eq(!cast<string>(T), "v128f16"),  f16,
                   !if(!eq(!cast<string>(T), "v256f16"),  f16,
                   !if(!eq(!cast<string>(T), "v64f32"),   f32,
                   !if(!eq(!cast<string>(T), "v128f32"),  f32,
                   !if(!eq(!cast<string>(T), "v256f32"),  f32,
                   !if(!eq(!cast<string>(T), "v256i8"),   i8,
                   !if(!eq(!cast<string>(T), "v128i16"),  i16,
                   !if(!eq(!cast<string>(T), "v256i16"),  i16,
                   !if(!eq(!cast<string>(T), "v64i32"),   i32,
                   !if(!eq(!cast<string>(T), "v128i32"),  i32,
                   !if(!eq(!cast<string>(T), "v256i32"),  i32,
                   isVoid))))))))))))))))))));
}

class DoubleVector<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "v256i1"),     v512i1,
                   !if(!eq(!cast<string>(T), "v256f8_143"), v512f8_143,
                   !if(!eq(!cast<string>(T), "v256f8_152"), v512f8_152,
                   !if(!eq(!cast<string>(T), "v128bf16"),   v256bf16,
                   !if(!eq(!cast<string>(T), "v128f16"),    v256f16,
                   !if(!eq(!cast<string>(T), "v64f32"),     v128f32,
                   !if(!eq(!cast<string>(T), "v256i8"),     v512i8,
                   !if(!eq(!cast<string>(T), "v128i16"),    v256i16,
                   !if(!eq(!cast<string>(T), "v64i32"),     v128i32,
                   isVoid)))))))));
}

class FloatToInteger<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "f32"),        i32,
                   !if(!eq(!cast<string>(T), "bf16"),       i16,
                   !if(!eq(!cast<string>(T), "f16"),        i16,
                   !if(!eq(!cast<string>(T), "f8_143"),     i8,
                   !if(!eq(!cast<string>(T), "f8_152"),     i8,
                   !if(!eq(!cast<string>(T), "v64f32"),     v64i32,
                   !if(!eq(!cast<string>(T), "v128bf16"),   v128i16,
                   !if(!eq(!cast<string>(T), "v128f16"),    v128i16,
                   !if(!eq(!cast<string>(T), "v256f8_143"), v256i8,
                   !if(!eq(!cast<string>(T), "v256f8_152"), v256i8,
                   T))))))))));
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
                  !if(!eq(OT, OpType.FP16),   "f16",
                  !if(!eq(OT, OpType.FP8_143), "f8_143",
                  !if(!eq(OT, OpType.FP8_152), "f8_152",
                  ""))))))))))))));
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
                   !if(!eq(OT, OpType.FP16),   f16,
                   !if(!eq(OT, OpType.FP8_143), f8_143,
                   !if(!eq(OT, OpType.FP8_152), f8_152,
                   isVoid))))))))))))));
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
                   !if(!eq(OT, OpType.FP16),    v128f16,
                   !if(!eq(OT, OpType.FP8_143), v256f8_143,
                   !if(!eq(OT, OpType.FP8_152), v256f8_152,
                   isVoid))))))))))))));
}

class OpTypeForType<ValueType T> {
  bits<4> V = !if(!eq(!cast<string>(T), "f32"),       OpType.FP32,
              !if(!eq(!cast<string>(T), "bf16"),      OpType.BF16,
              !if(!eq(!cast<string>(T), "f16"),       OpType.FP16,
              !if(!eq(!cast<string>(T), "f8_143"),    OpType.FP8_143,
              !if(!eq(!cast<string>(T), "f8_152"),    OpType.FP8_152,
              !if(!eq(!cast<string>(T), "i32"),       OpType.INT32,
              !if(!eq(!cast<string>(T), "i16"),       OpType.INT16,
              !if(!eq(!cast<string>(T), "i8"),        OpType.INT8,
              !if(!eq(!cast<string>(T), "i1"),        OpType.BOOL,
              !if(!eq(!cast<string>(T), "v64f32"),    OpType.FP32,
              !if(!eq(!cast<string>(T), "v128f32"),   OpType.FP32,
              !if(!eq(!cast<string>(T), "v256f32"),   OpType.FP32,
              !if(!eq(!cast<string>(T), "v128bf16"),  OpType.BF16,
              !if(!eq(!cast<string>(T), "v256bf16"),  OpType.BF16,
              !if(!eq(!cast<string>(T), "v128f16"),   OpType.FP16,
              !if(!eq(!cast<string>(T), "v256f16"),   OpType.FP16,
              !if(!eq(!cast<string>(T), "v256f8_143"),  OpType.FP8_143,
              !if(!eq(!cast<string>(T), "v512f8_143"),  OpType.FP8_143,
              !if(!eq(!cast<string>(T), "v256f8_152"),  OpType.FP8_152,
              !if(!eq(!cast<string>(T), "v512f8_153"),  OpType.FP8_152,
              !if(!eq(!cast<string>(T), "v64i32"),    OpType.INT32,
              !if(!eq(!cast<string>(T), "v128i32"),   OpType.INT32,
              !if(!eq(!cast<string>(T), "v256i32"),   OpType.INT32,
              !if(!eq(!cast<string>(T), "v128i16"),   OpType.INT16,
              !if(!eq(!cast<string>(T), "v256i16"),   OpType.INT16,
              !if(!eq(!cast<string>(T), "v256i8"),    OpType.INT8,
              !if(!eq(!cast<string>(T), "v256i1"),    OpType.BOOL,
              !if(!eq(!cast<string>(T), "v128i1"),    OpType.BOOL,
              !if(!eq(!cast<string>(T), "v64i1"),     OpType.BOOL,
              0b1111)))))))))))))))))))))))))))));
}


class UnsignedOpTypeForType<ValueType T> {
  bits<4> V = !if(!eq(!cast<string>(T), "i32"),       OpType.UINT32,
              !if(!eq(!cast<string>(T), "i16"),       OpType.UINT16,
              !if(!eq(!cast<string>(T), "i8"),        OpType.UINT8,
              !if(!eq(!cast<string>(T), "v64i32"),    OpType.UINT32,
              !if(!eq(!cast<string>(T), "v128i32"),   OpType.UINT32,
              !if(!eq(!cast<string>(T), "v256i32"),   OpType.UINT32,
              !if(!eq(!cast<string>(T), "v128i16"),   OpType.UINT16,
              !if(!eq(!cast<string>(T), "v256i16"),   OpType.UINT16,
              !if(!eq(!cast<string>(T), "v256i8"),    OpType.UINT8,
              0b1111)))))))));
}

class SwitchValTypeForType<ValueType T> {
  int S = !if(!eq(!cast<string>(T), "f32"),       SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "bf16"),      SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "f16"),       SwitchVal.TO_FP16,
          !if(!eq(!cast<string>(T), "f8_143"),    SwitchVal.TO_FP8_143,
          !if(!eq(!cast<string>(T), "f8_152"),    SwitchVal.TO_FP8_152,
          !if(!eq(!cast<string>(T), "i32"),       SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "i16"),       SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "i8"),        SwitchVal.TO_INT8,
          !if(!eq(!cast<string>(T), "v64f32"),    SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v128f32"),   SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v256f32"),   SwitchVal.TO_FP32,
          !if(!eq(!cast<string>(T), "v128bf16"),  SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "v256bf16"),  SwitchVal.TO_BF16,
          !if(!eq(!cast<string>(T), "v128f16"),   SwitchVal.TO_FP16,
          !if(!eq(!cast<string>(T), "v256f16"),   SwitchVal.TO_FP16,
          !if(!eq(!cast<string>(T), "v256f8_143"),SwitchVal.TO_FP8_143,
          !if(!eq(!cast<string>(T), "v512f8_143"),SwitchVal.TO_FP8_143,
          !if(!eq(!cast<string>(T), "v256f8_152"),SwitchVal.TO_FP8_152,
          !if(!eq(!cast<string>(T), "v512f8_152"),SwitchVal.TO_FP8_152,
          !if(!eq(!cast<string>(T), "v64i32"),    SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v128i32"),   SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v256i32"),   SwitchVal.TO_INT32,
          !if(!eq(!cast<string>(T), "v128i16"),   SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "v256i16"),   SwitchVal.TO_INT16,
          !if(!eq(!cast<string>(T), "v256i8"),    SwitchVal.TO_INT8,
          0x7fffffff)))))))))))))))))))))))));
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
                  !if(!eq(!cast<string>(T), "v128i1"),     "m",
                  !if(!eq(!cast<string>(T), "v64i1"),      "m",
                  !if(!eq(!cast<string>(T), "v256i8"),     "v",
                  !if(!eq(!cast<string>(T), "v512i8"),     "d",
                  !if(!eq(!cast<string>(T), "v128i16"),    "v",
                  !if(!eq(!cast<string>(T), "v256i16"),    "d",
                  !if(!eq(!cast<string>(T), "v256f8_143"), "v",
                  !if(!eq(!cast<string>(T), "v512f8_143"), "d",
                  !if(!eq(!cast<string>(T), "v256f8_152"), "v",
                  !if(!eq(!cast<string>(T), "v512f8_152"), "d",
                  !if(!eq(!cast<string>(T), "v128f16"),    "v",
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
                  !if(!eq(!cast<string>(T), "f16"),        "s",
                  !if(!eq(!cast<string>(T), "f8_143"),     "s",
                  !if(!eq(!cast<string>(T), "f8_152"),     "s",
                  !if(!eq(!cast<string>(T), "f32"),        "s",
                  !if(!eq(!cast<string>(T), "i1"),         "p",
                  !if(!eq(!cast<string>(T), "i8"),         "s",
                  !if(!eq(!cast<string>(T), "i16"),        "s",
                  !if(!eq(!cast<string>(T), "i32"),        "s", "")))))))))))))))))))))))))))))))));
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
                       !if(!eq(!cast<string>(T), "v128i16"),   [v128bf16, v128f16],
                       !if(!eq(!cast<string>(T), "v256i16"),   [v256bf16, v256f16],
                       !if(!eq(!cast<string>(T), "f32"),       [i32],
                       !if(!eq(!cast<string>(T), "bf16"),      [i16],
                       !if(!eq(!cast<string>(T), "f16"),       [i16],
                       !if(!eq(!cast<string>(T), "i32"),       [f32],
                       !if(!eq(!cast<string>(T), "i16"),       [bf16, f16],
                       [])))))))))))))));
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
  string suffix_g3 = !if(!and(!eq(suffix_T1, "d"), !eq(suffix_T2, "d")), "g3",
                     !if(!or(!eq(suffix_T1, "a"), !eq(suffix_T2, "a")),  "g3",
                     ""));
  string Name = !strconcat("CONVERT", suffix_g3, suffix_T1, suffix_T2, TypeMap<pred>.Suffix);
  Instruction Insn = !cast<Instruction>(Name);
}

class VectorPredicate<ValueType T> {
  ValueType Type = !if(!eq(!cast<string>(T), "v256i1"),     v256i1,
                   !if(!eq(!cast<string>(T), "v256f8_143"), v256i1,
                   !if(!eq(!cast<string>(T), "v256f8_152"), v256i1,
                   !if(!eq(!cast<string>(T), "v256i8"),     v256i1,
                   !if(!eq(!cast<string>(T), "v128bf16"),   v128i1,
                   !if(!eq(!cast<string>(T), "v128f16"),    v128i1,
                   !if(!eq(!cast<string>(T), "v128i16"),    v128i1,
                   !if(!eq(!cast<string>(T), "v64f32"),     v64i1,
                   !if(!eq(!cast<string>(T), "v64i32"),     v64i1,
                   isVoid)))))))));
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

def vsplatf16 : PatFrag<(ops node:$e0),
                        (v128f16 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
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

def vsplatf8_143  : PatFrag<(ops node:$e0),
                    (v256f8_143 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
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

def vsplatf8_152  : PatFrag<(ops node:$e0),
                    (v256f8_152 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
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

def vsplatb256  : PatFrag<(ops node:$e0),
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

def vsplatb128 : PatFrag<(ops node:$e0),
                         (v128i1 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
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

def vsplatb64  : PatFrag<(ops node:$e0),
                        (v64i1 (build_vector node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0, node:$e0,
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
              !if(!eq(OT, OpType.FP16),    vsplatf16,
              !if(!eq(OT, OpType.FP8_143), vsplatf8_143,
              !if(!eq(OT, OpType.FP8_152), vsplatf8_152,
              !if(!eq(OT, OpType.INT32),   vsplati32,
              !if(!eq(OT, OpType.UINT32),  vsplati32,
              !if(!eq(OT, OpType.INT8),    vsplati8,
              !if(!eq(OT, OpType.UINT8),   vsplati8,
              !if(!eq(OT, OpType.BOOL),    vsplatb256,
              !if(!eq(OT, OpType.BOOL),    vsplatb128,
              !if(!eq(OT, OpType.BOOL),    vsplatb64,
              !if(!eq(OT, OpType.INT16),   vsplati16,
              !if(!eq(OT, OpType.UINT16),  vsplati16, vsplati32))))))))))))));
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
  PatFrag Splat = !if(!eq(!cast<string>(T), "v256i1"),     vsplatb256,
                  !if(!eq(!cast<string>(T), "v128i1"),     vsplatb128,
                  !if(!eq(!cast<string>(T), "v64i1"),      vsplatb64,
                  !if(!eq(!cast<string>(T), "v128bf16"),   vsplatbf16,
                  !if(!eq(!cast<string>(T), "v128f16"),    vsplatf16,
                  !if(!eq(!cast<string>(T), "v256f8_143"), vsplatf8_143,
                  !if(!eq(!cast<string>(T), "v256f8_152"), vsplatf8_152,
                  !if(!eq(!cast<string>(T), "v64f32"),     vsplatf32,
                  !if(!eq(!cast<string>(T), "v256i8"),     vsplati8,
                  !if(!eq(!cast<string>(T), "v128i16"),    vsplati16,
                  !if(!eq(!cast<string>(T), "v64i32"),     vsplati32, vsplati32)))))))))));
}

class ImmNode<ValueType T> {
  SDNode Node = !if(!eq(!cast<string>(T), "f32"),        fpimm,
                !if(!eq(!cast<string>(T), "f16"),        fpimm,
                !if(!eq(!cast<string>(T), "f8_143"),     fpimm,
                !if(!eq(!cast<string>(T), "f8_152"),     fpimm,
                !if(!eq(!cast<string>(T), "bf16"),       fpimm,
                !if(!eq(!cast<string>(T), "v64f32"),     fpimm,
                !if(!eq(!cast<string>(T), "v128bf16"),   fpimm,
                !if(!eq(!cast<string>(T), "v128f16"),    fpimm,
                !if(!eq(!cast<string>(T), "v256f8_143"), fpimm,
                !if(!eq(!cast<string>(T), "v256f8_152"), fpimm,
                imm))))))))));
}

class CastOp<ValueType T> {
  SDNodeXForm Cast = !if(!eq(!cast<string>(T), "v64f32"),     FPToImm32,
                     !if(!eq(!cast<string>(T), "v128bf16"),   FPToImm32,
                     !if(!eq(!cast<string>(T), "v128f16"),    FPToImm32,
                     !if(!eq(!cast<string>(T), "v256f8_143"), FPToImm32,
                     !if(!eq(!cast<string>(T), "v256f8_152"), FPToImm32,
                     !if(!eq(!cast<string>(T), "f32"),        FPToImm32,
                     !if(!eq(!cast<string>(T), "f16"),        FPToImm32,
                     !if(!eq(!cast<string>(T), "bf16"),       FPToImm32,
                     !if(!eq(!cast<string>(T), "f8_143"),     FPToImm32,
                     !if(!eq(!cast<string>(T), "f8_152"),     FPToImm32,
                     CastToImm32))))))))));
}

class SelInstructionForPredicate<PatFrag P> {
  string Pred = !cast<string>(P);
  string Name = !if(!eq(Pred, "seteq"), "SEL_EQ",
                !if(!eq(Pred, "setne"), "SEL_NEQ",
                !if(!eq(Pred, "setlt"), "SEL_LESS",
                !if(!eq(Pred, "setgt"), "SEL_GRT",
                !if(!eq(Pred, "setle"), "SEL_LEQ",
                !if(!eq(Pred, "setge"), "SEL_GEQ",
                !if(!eq(Pred, "setult"), "SEL_LESS",
                !if(!eq(Pred, "setule"), "SEL_LEQ",
                !if(!eq(Pred, "setugt"), "SEL_GRT",
                !if(!eq(Pred, "setuge"), "SEL_GEQ",
                !if(!eq(Pred, "setoeq"), "SEL_EQ",
                !if(!eq(Pred, "setone"), "SEL_NEQ",
                !if(!eq(Pred, "setolt"), "SEL_LESS",
                !if(!eq(Pred, "setogt"), "SEL_GRT",
                !if(!eq(Pred, "setole"), "SEL_LEQ",
                !if(!eq(Pred, "setoge"), "SEL_GEQ",
                !strconcat("__", Pred, "__")))))))))))))))));
  Instruction InstrVVVV = !cast<Instruction>(!strconcat(Name, "vvvvp"));
  Instruction InstrVSVV = !cast<Instruction>(!strconcat(Name, "vsvvp"));
  Instruction InstrVIVV = !cast<Instruction>(!strconcat(Name, "vivvp"));
  Instruction InstrVVVS = !cast<Instruction>(!strconcat(Name, "vvvsp"));
  Instruction InstrVVVI = !cast<Instruction>(!strconcat(Name, "vvvip"));
}

// Represents 1-bit constant 'true'.
def i1true : PatLeaf<(i1 imm:$val) ,[{
  return cast<ConstantSDNode>(N)->getZExtValue() == 1;
}]>;

//------------------------------------------------------------------------------

let isNotUsedInDisasm = 0, isBranch = 1, isTerminator = 1, hasCtrlDep = 1 in {
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

let isBranch = 1, hasCtrlDep = 1, isTerminator = 1, isPseudo = 1,
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
defm SPILL_ADRF : SPILL_REG<ADRF>, Requires<[ADRFMov]>;

def TPC_EOF : TPCInstPseudo<(outs), (ins), "EOF">;

let isCodeGenOnly = 1 in {
  // PriorDoron1
  def ReadLFSRp   : ReadLFSR   <"READ\t$dest, LFSR, $pred", SPred>, Requires<[PriorDoron1]>;
  def ReadLFSRm   : ReadLFSR   <"READ\t$dest, LFSR, $pred", VPred>, Requires<[PriorDoron1]>;
  def ReadLFSRNCp : ReadLFSRNC <"READ\t$dest, LFSR_NO_CHANGE, $pred", SPred>, Requires<[PriorDoron1]>;
  def ReadLFSRNCm : ReadLFSRNC <"READ\t$dest, LFSR_NO_CHANGE, $pred", VPred>, Requires<[PriorDoron1]>;

  def ReadSLFSR   : ReadSLFSR  <"READ\t$dest, S_LFSR, $pred">, Requires<[PriorDoron1]>;
  def ReadSLFSRNC : ReadSLFSRNC<"READ\t$dest, S_LFSR_NO_CHANGE, $pred">, Requires<[PriorDoron1]>;

  def WriteLFSRp  : WriteLFSR <"WRITE\tLFSR, $src, $pred", SPred>, Requires<[PriorDoron1]>;
  def WriteLFSRm  : WriteLFSR <"WRITE\tLFSR, $src, $pred", VPred>, Requires<[PriorDoron1]>;

  def WriteSLFSR  : WriteSLFSR<"WRITE\tS_LFSR, $src, $pred">, Requires<[PriorDoron1]>;
  // Doron1
  def ReadLFSRG5Pp   : ReadVPU_LFSR  <"READ\t$dest, VPU_LFSR, $pred", SPred>, Requires<[Doron1]>;
  def ReadLFSRG5Pm   : ReadVPU_LFSR  <"READ\t$dest, VPU_LFSR, $pred", VPred>, Requires<[Doron1]>;
  def ReadLFSRNCG5Pp : ReadVPU_LFSRNC<"READ\t$dest, VPU_LFSR_RO, $pred", SPred>, Requires<[Doron1]>;
  def ReadLFSRNCG5Pm : ReadVPU_LFSRNC<"READ\t$dest, VPU_LFSR_RO, $pred", VPred>, Requires<[Doron1]>;

  def ReadSLFSRG5P   : ReadSPU_LFSR  <"READ\t$dest, SPU_LFSR, $pred">, Requires<[Doron1]>;
  def ReadSLFSRNCG5P : ReadSPU_LFSRNC<"READ\t$dest, SPU_LFSR_RO, $pred">, Requires<[Doron1]>;

  def WriteLFSRG5Pvp : WriteVPU_LFSR<VRF, "WRITE\tVPU_LFSR, $src, $pred", SPred>, Requires<[Doron1]>;
  def WriteLFSRG5Pvm : WriteVPU_LFSR<VRF, "WRITE\tVPU_LFSR, $src, $pred", VPred>, Requires<[Doron1]>;
  def WriteLFSRG5Psp : WriteVPU_LFSR<SRF, "WRITE\tVPU_LFSR, $src, $pred", SPred>, Requires<[Doron1]>;
  def WriteLFSRG5Psm : WriteVPU_LFSR<SRF, "WRITE\tVPU_LFSR, $src, $pred", VPred>, Requires<[Doron1]>;
  def WriteLFSRG5Pip : WriteVPU_LFSR<TPCImm<i32imm>, "WRITE\tVPU_LFSR, $src, $pred", SPred>, Requires<[Doron1]>;
  def WriteLFSRG5Pim : WriteVPU_LFSR<TPCImm<i32imm>, "WRITE\tVPU_LFSR, $src, $pred", VPred>, Requires<[Doron1]>;

  def WriteSLFSRG5Ps : WriteSPU_LFSR<SRF, "WRITE\tSPU_LFSR, $src, $pred">, Requires<[Doron1]>;
  def WriteSLFSRG5Pi : WriteSPU_LFSR<TPCImm<i32imm>, "WRITE\tSPU_LFSR, $src, $pred">, Requires<[Doron1]>;
}

foreach pred = [i1, v256i1] in {
  // PriorDoron1
  def : Pat<(v256i8 (int_tpc_read_lfsr v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadLFSR", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
  def : Pat<(v256i8 (int_tpc_read_lfsrnc v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadLFSRNC", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
            
  def : Pat<(int_tpc_write_lfsr v256i8:$src, pred:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"WriteLFSR", pred>.Insn VRF:$src, (i32 0), pred:$pred, (i1 imm:$polarity))>, Requires<[PriorDoron1]>;
  // Doron1
  def : Pat<(v256i8 (int_tpc_read_lfsr v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadLFSRG5P", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
  def : Pat<(v256i8 (int_tpc_read_lfsrnc v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadLFSRNCG5P", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
            
  def : Pat<(int_tpc_write_lfsr v256i8:$src, pred:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"WriteLFSRG5P", v256i8, pred>.Insn VRF:$src, (i32 0), pred:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;
  def : Pat<(int_tpc_write_lfsr (v256i8 (vsplati8 i8:$src)), pred:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"WriteLFSRG5P", i8, pred>.Insn SRF:$src, (i32 0), pred:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;
  def : Pat<(int_tpc_write_lfsr (v256i8 (vsplati8 (i8 imm:$src))), pred:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"WriteLFSRG5P", iAny, pred>.Insn (CastToImm32 $src), (i32 0), pred:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;
}
// PriorDoron1
def : Pat<(i32 (int_tpc_s_read_lfsr i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (ReadSLFSR (i32 0), i32:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
def : Pat<(i32 (int_tpc_s_read_lfsrnc i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (ReadSLFSRNC (i32 0), i32:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;

def : Pat<(int_tpc_write_lfsr_s i32:$src, i1:$pred, (i1 imm:$polarity)),
          (WriteSLFSR SRF:$src, (i32 0), i1:$pred, (i1 imm:$polarity))>, Requires<[PriorDoron1]>;
// Doron1
def : Pat<(i32 (int_tpc_s_read_lfsr i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (ReadSLFSRG5P (i32 0), i32:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
def : Pat<(i32 (int_tpc_s_read_lfsrnc i32:$income, i1:$pred, (i1 imm:$polarity))),
          (i32 (ReadSLFSRNCG5P (i32 0), i32:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;

def : Pat<(int_tpc_write_lfsr_s i32:$src, i1:$pred, (i1 imm:$polarity)),
          (WriteSLFSRG5Ps SRF:$src, (i32 0), i1:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;
def : Pat<(int_tpc_write_lfsr_s (i32 imm:$src), i1:$pred, (i1 imm:$polarity)),
          (WriteSLFSRG5Pi (CastToImm32 $src), (i32 0), i1:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;

let isCodeGenOnly = 1 in {
  let src = 42, Uses = [V_LANE_ID_32] in {
    def ReadVLaneId32p : ReadVLaneId<"READ\t$dest, V_LANE_ID_32, $pred", SPred>, Requires<[PriorDoron1]>;
    def ReadVLaneId32m : ReadVLaneId<"READ\t$dest, V_LANE_ID_32, $pred", VPred>, Requires<[PriorDoron1]>;
  }

  let src = 43, Uses = [V_LANE_ID_16] in {
    def ReadVLaneId16p : ReadVLaneId<"READ\t$dest, V_LANE_ID_16, $pred", SPred>, Requires<[PriorDoron1]>;
    def ReadVLaneId16m : ReadVLaneId<"READ\t$dest, V_LANE_ID_16, $pred", VPred>, Requires<[PriorDoron1]>;
  }

  let src = 20, Uses = [LANE_ID_4B] in {
    def ReadLaneId4Bp : ReadLaneId<"READ\t$dest, LANE_ID_4B, $pred", SPred>, Requires<[Doron1]>;
    def ReadLaneId4Bm : ReadLaneId<"READ\t$dest, LANE_ID_4B, $pred", VPred>, Requires<[Doron1]>;
  }

  let src = 21, Uses = [LANE_ID_2B] in {
    def ReadLaneId2Bp : ReadLaneId<"READ\t$dest, LANE_ID_2B, $pred", SPred>, Requires<[Doron1]>;
    def ReadLaneId2Bm : ReadLaneId<"READ\t$dest, LANE_ID_2B, $pred", VPred>, Requires<[Doron1]>;
  }

  let src = 22, Uses = [LANE_ID_1B] in {
    def ReadLaneId1Bp : ReadLaneId<"READ\t$dest, LANE_ID_1B, $pred", SPred>, Requires<[Doron1]>;
    def ReadLaneId1Bm : ReadLaneId<"READ\t$dest, LANE_ID_1B, $pred", VPred>, Requires<[Doron1]>;
  }

  let src = 44, Uses = [V_LANE_ID_8] in {
    def ReadVLaneId8p : ReadVLaneId<"READ\t$dest, V_LANE_ID_8, $pred", SPred>, Requires<[PriorDoron1]>;
    def ReadVLaneId8m : ReadVLaneId<"READ\t$dest, V_LANE_ID_8, $pred", VPred>, Requires<[PriorDoron1]>;
  }
}

def : Pat<(v64i32 (int_tpc_read_lane_id (v64i32 undef), (i1 1), (i1 0))),
          (COPY (v64i32 V_LANE_ID_32))>, Requires<[PriorDoron1]>;
def : Pat<(v128i16 (int_tpc_read_lane_id (v128i16 undef), (i1 1), (i1 0))),
          (COPY (v128i16 V_LANE_ID_16))>, Requires<[PriorDoron1]>;
def : Pat<(v256i8 (int_tpc_read_lane_id (v256i8 undef), (i1 1), (i1 0))),
          (COPY (v256i8 V_LANE_ID_32))>, Requires<[PriorDoron1]>;

foreach pred = [i1, v256i1] in {
  def : Pat<(v64i32 (int_tpc_read_lane_id v64i32:$income, pred:$pred, (i1 imm:$polarity))),
            (v64i32 (ConstructInsn<"ReadVLaneId32", pred>.Insn (i32 0), v64i32:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
  def : Pat<(v128i16 (int_tpc_read_lane_id v128i16:$income, pred:$pred, (i1 imm:$polarity))),
            (v128i16 (ConstructInsn<"ReadVLaneId16", pred>.Insn (i32 0), v128i16:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
  def : Pat<(v256i8 (int_tpc_read_lane_id v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadVLaneId8", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
  
  def : Pat<(v64i32 (int_tpc_read_lane_id v64i32:$income, pred:$pred, (i1 imm:$polarity))),
            (v64i32 (ConstructInsn<"ReadLaneId4B", pred>.Insn (i32 0), v64i32:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
  def : Pat<(v128i16 (int_tpc_read_lane_id v128i16:$income, pred:$pred, (i1 imm:$polarity))),
            (v128i16 (ConstructInsn<"ReadLaneId2B", pred>.Insn (i32 0), v128i16:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
  def : Pat<(v256i8 (int_tpc_read_lane_id v256i8:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i8 (ConstructInsn<"ReadLaneId1B", pred>.Insn (i32 0), v256i8:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
}
  
let hasSideEffects = 1 in
  def MOVnodce : SPUInstRI<0b001000, SRF, i32imm, "mov.i32\t$dst, $src", [], OpType.INT32>;

let isCodeGenOnly = 1, hasSideEffects = 1 in
  // It needs to perform the dummy lookup before the CACHE_INVALIDATE.RST_LU instruction
  // due to the GAUDI2_0010 restriction (CACHE_INVALIDATE.RST_LU is not resetting LUT$ pLRU).
  // In common case the LoadInst_LOOKUP_GEN2 format is used for Gen2Plus however this restriction only for Gen4.
  def LOOKUPnodce : LoadInst_LOOKUP_GEN2<0b000110, VRF, "lookup $sw $dest, $src, $fid, $pred">, Requires<[Gen4Plus]>;
 
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
                      (v256i8   (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _v128i16  : Pat<(v128i16  (LdOp Addr)),
                      (v128i16  (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _v64i32   : Pat<(v64i32   (LdOp Addr)),
                      (v64i32   (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _v64f32   : Pat<(v64f32   (LdOp Addr)),
                      (v64f32   (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _v128bf16 : Pat<(v128bf16 (LdOp Addr)),
                      (v128bf16 (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}

multiclass LoadMapSRF<PatFrag LdOp, dag Addr, SDNode Insn> {
  def _i8   : Pat<(i8   (LdOp Addr)),
                  (i8   (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _i16  : Pat<(i16  (LdOp Addr)),
                  (i16  (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _i32  : Pat<(i32  (LdOp Addr)),
                  (i32  (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _f32  : Pat<(f32  (LdOp Addr)),
                  (f32  (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def _bf16 : Pat<(bf16 (LdOp Addr)),
                  (bf16 (Insn Addr, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}

// Mappin of unary operation on integer scalar.
multiclass UnaryIntegerScalar<PatFrag Op, SDNode InstrS> {
  def i32s  : Pat<(Op i32:$op), (InstrS $op, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16s  : Pat<(Op i16:$op), (InstrS $op, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8s   : Pat<(Op i8:$op),  (InstrS $op, OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of unary operation on float scalar.
multiclass UnaryFloatScalar<PatFrag Op, SDNode InstrS> {
  def f32s  : Pat<(Op f32:$op),  (InstrS $op, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def bf16s : Pat<(Op bf16:$op), (InstrS $op, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
  def f16s  : Pat<(Op f16:$op),  (InstrS $op, OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
  def f8_143s : Pat<(Op f8_143:$op), (InstrS $op, OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
  def f8_152s : Pat<(Op f8_152:$op), (InstrS $op, OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
}

// Mapping of binary operation on integer signed scalars.
multiclass BinarySignedScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def i32s  : Pat<(Op i32:$op1, i32:$op2), (InstrSS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32i  : Pat<(Op i32:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16s  : Pat<(Op i16:$op1, i16:$op2), (InstrSS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16i  : Pat<(Op i16:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8s   : Pat<(Op i8:$op1,  i8:$op2),  (InstrSS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8i   : Pat<(Op i8:$op1,  imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of binary operation on integer signed scalars for shift operations.
// In this case the second operand is always i32.
multiclass BinaryShiftScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def i32s  : Pat<(Op i32:$op1, i32:$op2),       (InstrSS $op1, $op2,           OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32i  : Pat<(Op i32:$op1, (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16s  : Pat<(Op i16:$op1, i32:$op2),       (InstrSS $op1, $op2,           OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16i  : Pat<(Op i16:$op1, (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8s   : Pat<(Op i8:$op1,  i32:$op2),       (InstrSS $op1, $op2,           OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8i   : Pat<(Op i8:$op1,  (i32 imm:$op2)), (InstrSI $op1, (i32 imm:$op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of binary operation on integer unsigned scalars.
multiclass BinaryUnsignedScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def u32s  : Pat<(Op i32:$op1, i32:$op2), (InstrSS $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def u32i  : Pat<(Op i32:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def u16s  : Pat<(Op i16:$op1, i16:$op2), (InstrSS $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def u16i  : Pat<(Op i16:$op1, imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def u8s   : Pat<(Op i8:$op1,  i8:$op2),  (InstrSS $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def u8i   : Pat<(Op i8:$op1,  imm:$op2), (InstrSI $op1, (CastToImm32 $op2), OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of binary operation on float scalars.
multiclass BinaryFloatScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def f32s : Pat<(Op f32:$op1, f32:$op2),         (InstrSS $op1, $op2,             OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32i : Pat<(Op f32:$op1, (f32 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  let Predicates = [BF16] in {
    def bf16s : Pat<(Op bf16:$op1, bf16:$op2),         (InstrSS $op1, $op2,             OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def bf16i : Pat<(Op bf16:$op1, (bf16 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
  let Predicates = [FP16] in {
    def f16s : Pat<(Op f16:$op1, f16:$op2),         (InstrSS $op1, $op2,             OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def f16i : Pat<(Op f16:$op1, (f16 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
  let Predicates = [FP8] in {
    def f8_143s : Pat<(Op f8_143:$op1, f8_143:$op2),         (InstrSS $op1, $op2,             OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def f8_143i : Pat<(Op f8_143:$op1, (f8_143 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def f8_152s : Pat<(Op f8_152:$op1, f8_152:$op2),         (InstrSS $op1, $op2,             OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def f8_152i : Pat<(Op f8_152:$op1, (f8_152 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
}

def AddDnormSupport : SDNodeXForm<imm, [{
  bool IsDnormSupport = CurDAG->getSubtarget().getTargetLowering()->getTargetMachine().Options.TpcDnorm;
  uint64_t SwitchVal = N->getZExtValue();
  if (IsDnormSupport)
    SwitchVal |= TPCII::SW_DNORM;
  return CurDAG->getTargetConstant(SwitchVal, SDLoc(N), MVT::i32);
}]>;

multiclass BinaryFloatArithScalar<PatFrag Op, SDNode InstrSS, SDNode InstrSI> {
  def f32s : Pat<(Op f32:$op1, f32:$op2),         (InstrSS $op1, $op2,             OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32i : Pat<(Op f32:$op1, (f32 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  let Predicates = [BF16] in {
    def bf16s : Pat<(Op bf16:$op1, bf16:$op2),         (InstrSS $op1, $op2,             OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def bf16i : Pat<(Op bf16:$op1, (bf16 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
  let Predicates = [FP16] in {
    def f16s : Pat<(Op f16:$op1, f16:$op2),         (InstrSS $op1, $op2,             OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def f16i : Pat<(Op f16:$op1, (f16 fpimm:$op2)), (InstrSI $op1, (FPToImm32 $op2), OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
}

// Mapping of binary operation on index values. Scalar variants have scalar
// argument as the second operand (SHL/SHR).
multiclass BinaryIndexScalar<PatFrag Op, SDNode InstrNS, SDNode InstrNI> {
  def s : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 i32:$op2))),
              (InstrNS $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 (i32 imm:$op2)))),
              (InstrNI $op1, (CastToImm32 $op2), (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of binary operation on index values. Scalar variants have immediate
// scalar argument as the second operand (CMP_*).
multiclass CmpIndexOp<PatFrag Op, SDNode InstrNI> {
  def i : Pat<(Op v5i32:$op1, (v5i32 (vsplati5 (i32 imm:$op2)))),
              (InstrNI $op1, (CastToImm32 $op2), (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of binary operation on index values. Scalar variants have scalar
// argument as the first operand (as ADD, SUB etc).
multiclass BinaryIndexOp1<PatFrag Op, SDNode InstrNN, SDNode InstrNS, SDNode InstrNI> {
  def I : Pat<(Op v5i32:$op1, v5i32:$op2),
              (InstrNN $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def s : Pat<(Op (v5i32 (vsplati5 i32:$op1)), v5i32:$op2),
              (InstrNS $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i : Pat<(Op (v5i32 (vsplati5 (i32 imm:$op1))), v5i32:$op2),
              (InstrNI (CastToImm32 $op1), $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of unary operation on integer vector.
multiclass UnaryIntegerVector<PatFrag Op, SDNode InstrV> {
  def i32v : Pat<(Op v64i32:$op),  (InstrV $op, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16v : Pat<(Op v128i16:$op), (InstrV $op, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8v  : Pat<(Op v256i8:$op),  (InstrV $op, OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// Mapping of unary operation on float vector.
multiclass UnaryFloatVector<PatFrag Op, SDNode InstrV> {
  def f32v  : Pat<(Op v64f32:$op),   (InstrV $op, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def bf16v : Pat<(Op v128bf16:$op), (InstrV $op, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
  def fp16v : Pat<(Op v128f16:$op),  (InstrV $op, OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
}

// Mapping of binary operation on integer vectors.
multiclass BinaryIntegerVector<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(Op v64i32:$op1,  v64i32:$op2                         ), (InstrVV $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vs  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       ), (InstrVS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vi  : Pat<(Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vv  : Pat<(Op v128i16:$op1, v128i16:$op2                        ), (InstrVV $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vs  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      ), (InstrVS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vi  : Pat<(Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2)))), (InstrVI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vv   : Pat<(Op v256i8:$op1,  v256i8:$op2                         ), (InstrVV $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vs   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         ), (InstrVS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vi   : Pat<(Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   ), (InstrVI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

multiclass CmpOpVect<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(v64i1  (Op v64i32:$op1,  v64i32:$op2                         )), (InstrVV $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vs  : Pat<(v64i1  (Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       )), (InstrVS $op1, $op2,               OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vi  : Pat<(v64i1  (Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) )), (InstrVI $op1, (CastToImm32 $op2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vv  : Pat<(v128i1 (Op v128i16:$op1, v128i16:$op2                        )), (InstrVV $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vs  : Pat<(v128i1 (Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      )), (InstrVS $op1, $op2,               OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vi  : Pat<(v128i1 (Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2))))), (InstrVI $op1, (CastToImm32 $op2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vv   : Pat<(v256i1 (Op v256i8:$op1,  v256i8:$op2                         )), (InstrVV $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vs   : Pat<(v256i1 (Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         )), (InstrVS $op1, $op2,               OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vi   : Pat<(v256i1 (Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   )), (InstrVI $op1, (CastToImm32 $op2), OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

multiclass FCmpOpVect<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def f32vv  : Pat<(v64i1  (Op v64f32:$op1,     v64f32:$op2                              )), (InstrVV $op1, $op2,               OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32vs  : Pat<(v64i1  (Op v64f32:$op1,     (v64f32 (vsplatf32 f32:$op2))            )), (InstrVS $op1, $op2,               OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32vi  : Pat<(v64i1  (Op v64f32:$op1,     (v64f32 (vsplatf32 (f32 fpimm:$op2)))    )), (InstrVI $op1, (CastToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def bf16vv : Pat<(v128i1 (Op v128bf16:$op1,   v128bf16:$op2                            )), (InstrVV $op1, $op2,               OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def bf16vs : Pat<(v128i1 (Op v128bf16:$op1,   (v128bf16 (vsplatbf16 bf16:$op2))        )), (InstrVS $op1, $op2,               OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def bf16vi : Pat<(v128i1 (Op v128bf16:$op1,   (v128bf16 (vsplatbf16 (bf16 fpimm:$op2))))), (InstrVI $op1, (CastToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f16vv  : Pat<(v128i1 (Op v128f16:$op1,    v128f16:$op2                             )), (InstrVV $op1, $op2,               OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f16vs  : Pat<(v128i1 (Op v128f16:$op1,    (v128f16 (vsplatf16 f16:$op2))           )), (InstrVS $op1, $op2,               OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f16vi  : Pat<(v128i1 (Op v128f16:$op1,    (v128f16 (vsplatf16 (f16 fpimm:$op2)))   )), (InstrVI $op1, (CastToImm32 $op2), OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f843vv : Pat<(v256i1 (Op v256f8_143:$op1, v256f8_143:$op2                          )), (InstrVV $op1, $op2,               OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f843vs : Pat<(v256i1 (Op v256f8_143:$op1, (v256f8_143 (vsplatf8_143 f8_143:$op2))  )), (InstrVS $op1, $op2,               OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f843vi : Pat<(v256i1 (Op v256f8_143:$op1, (v256f8_143 (vsplatf8_143 (f8_143 fpimm:$op2))))), (InstrVI $op1, (CastToImm32 $op2), OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f852vv : Pat<(v256i1 (Op v256f8_152:$op1, v256f8_152:$op2                          )), (InstrVV $op1, $op2,               OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f852vs : Pat<(v256i1 (Op v256f8_152:$op1, (v256f8_152 (vsplatf8_152 f8_152:$op2))  )), (InstrVS $op1, $op2,               OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f852vi : Pat<(v256i1 (Op v256f8_152:$op1, (v256f8_152 (vsplatf8_152 (f8_152 fpimm:$op2))))), (InstrVI $op1, (CastToImm32 $op2), OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

multiclass CmpOpUVect<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def i32vv  : Pat<(v64i1  (Op v64i32:$op1,  v64i32:$op2                         )), (InstrVV $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vs  : Pat<(v64i1  (Op v64i32:$op1,  (v64i32 (vsplati32 i32:$op2))       )), (InstrVS $op1, $op2,               OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i32vi  : Pat<(v64i1  (Op v64i32:$op1,  (v64i32 (vsplati32 (i32 imm:$op2))) )), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vv  : Pat<(v128i1 (Op v128i16:$op1, v128i16:$op2                        )), (InstrVV $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vs  : Pat<(v128i1 (Op v128i16:$op1, (v128i16 (vsplati16 i16:$op2))      )), (InstrVS $op1, $op2,               OpType.UINT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i16vi  : Pat<(v128i1 (Op v128i16:$op1, (v128i16 (vsplati16 (i16 imm:$op2))))), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vv   : Pat<(v256i1 (Op v256i8:$op1,  v256i8:$op2                         )), (InstrVV $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vs   : Pat<(v256i1 (Op v256i8:$op1,  (v256i8 (vsplati8 i8:$op2))         )), (InstrVS $op1, $op2,               OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def i8vi   : Pat<(v256i1 (Op v256i8:$op1,  (v256i8 (vsplati8 (i8 imm:$op2)))   )), (InstrVI $op1, (CastToImm32 $op2), OpType.UINT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}


// Mapping of binary operation on float vectors.
multiclass BinaryFloatVector<PatFrag Op, SDNode InstrVV, SDNode InstrVS, SDNode InstrVI> {
  def f32vv  : Pat<(Op v64f32:$op1,   v64f32:$op2                          ), (InstrVV $op1, $op2,             OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32vs  : Pat<(Op v64f32:$op1,   (v64f32 (vsplatf32 f32:$op2))        ), (InstrVS $op1, $op2,             OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def f32vi  : Pat<(Op v64f32:$op1,   (v64f32 (vsplatf32 (f32 fpimm:$op2)))), (InstrVI $op1, (FPToImm32 $op2), OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  let Predicates = [BF16] in {
    def bf16vv  : Pat<(Op v128bf16:$op1, v128bf16:$op2                            ), (InstrVV $op1, $op2,             OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def bf16vs  : Pat<(Op v128bf16:$op1, (v128bf16 (vsplatbf16 bf16:$op2))        ), (InstrVS $op1, $op2,             OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def bf16vi  : Pat<(Op v128bf16:$op1, (v128bf16 (vsplatbf16 (bf16 fpimm:$op2)))), (InstrVI $op1, (FPToImm32 $op2), OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  }
  let Predicates = [FP16] in {
    def fp16vv  : Pat<(Op v128f16:$op1, v128f16:$op2                          ), (InstrVV $op1, $op2,             OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def fp16vs  : Pat<(Op v128f16:$op1, (v128f16 (vsplatf16 f16:$op2))        ), (InstrVS $op1, $op2,             OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
    def fp16vi  : Pat<(Op v128f16:$op1, (v128f16 (vsplatf16 (f16 fpimm:$op2)))), (InstrVI $op1, (FPToImm32 $op2), OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
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

  def svm   : StoreInst_ST_Vrr<opc, VRF,  asm_string, VPred>, Requires<[Addr1, VectStMask]>;
  def ivm   : StoreInst_ST_Vir<opc, VRF,  asm_string, VPred>, Requires<[Addr1, VectStMask]>;
  def smm   : StoreInst_ST_Vrr<opc, VPRF, asm_string, VPred>, Requires<[Addr1, VectStMask]>;
  def imm   : StoreInst_ST_Vir<opc, VPRF, asm_string, VPred>, Requires<[Addr1, VectStMask]>;
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

  let Predicates = [DimMaskR] in {
    def wII: Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), v5i32, v5i32, i1>.Insn
                      v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wsI: Pat<(RT (Intrin (v5i32 (vsplati5 i32:$x0)), v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), i32, v5i32, i1>.Insn
                      i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wiI: Pat<(RT (Intrin (v5i32 (vsplati5 (i32 imm:$x0))), v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), iAny, v5i32, i1>.Insn
                      (CastToImm32 $x0), v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wsI2: Pat<(RT (Intrin i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), i32, v5i32, i1>.Insn
                      i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wiI2: Pat<(RT (Intrin (i32 imm:$x0), v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), iAny, v5i32, i1>.Insn
                      (CastToImm32 $x0), v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  }
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

  let Predicates = [DimMaskR] in {
    def wIs: Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 i32:$x1)), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), i32, v5i32, i1>.Insn
                      i32:$x1, v5i32:$x0, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIi: Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), iAny, v5i32, i1>.Insn
                      (CastToImm32 $x1), v5i32:$x0, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIs2: Pat<(RT (Intrin v5i32:$x0, i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), i32, v5i32, i1>.Insn
                      i32:$x1, v5i32:$x0, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIi2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (ConstructInsn<!strconcat(IName, "w"), iAny, v5i32, i1>.Insn
                      (CastToImm32 $x1), v5i32:$x0, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  }
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

  let Predicates = [DimMaskR] in {
    def wII: Pat<(RT (Intrin v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (!cast<Instruction>(!strconcat(IName, "wIIp"))
                      v5i32:$x0, v5i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIs: Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 i32:$x1)), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (!cast<Instruction>(!strconcat(IName, "wIsp"))
                      v5i32:$x0, i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIi: Pat<(RT (Intrin v5i32:$x0, (v5i32 (vsplati5 (i32 imm:$x1))), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (!cast<Instruction>(!strconcat(IName, "wIip"))
                      v5i32:$x0, (CastToImm32 $x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIs2: Pat<(RT (Intrin v5i32:$x0, i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (!cast<Instruction>(!strconcat(IName, "wIsp"))
                      v5i32:$x0, i32:$x1, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
    def wIi2: Pat<(RT (Intrin v5i32:$x0, (i32 imm:$x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
                 (RT (!cast<Instruction>(!strconcat(IName, "wIip"))
                      v5i32:$x0, (CastToImm32 $x1), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  }
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
  // Mask in register
  def : Pat<(v5i32 (Intrin v5i32:$src, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<!strconcat(IName, "w"), v5i32, v5i32, i1>.Insn
                    IRF:$src, MRF:$mask, (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v5i32 (Intrin (v5i32 (vsplati5 i32:$src)), i32:$mask, (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<!strconcat(IName, "w"), v5i32, i32, i1>.Insn
                    SRF:$src, MRF:$mask, (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v5i32 (Intrin i32:$src, i32:$mask, (i8 imm:$dt), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
            (v5i32 (ConstructInsn<!strconcat(IName, "w"), v5i32, i32, i1>.Insn
                    SRF:$src, MRF:$mask, (i8 imm:$dt), (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

multiclass BinaryScalarIntrinsics<SDNode Intrin, string IName, ValueType T, ValueType RT = T> {
  def : Pat<(RT (Intrin T:$x0, T:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, T, i1>.Insn T:$x0, T:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin T:$x0, (T ImmNode<T>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, i1>.Insn T:$x0, (CastOp<T>.Cast $x1), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
}

multiclass BinaryScalarIntrinsicsDnorm<SDNode Intrin, string IName, ValueType T, ValueType RT = T> {
  def : Pat<(RT (Intrin T:$x0, T:$x1, (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, T, i1>.Insn T:$x0, T:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin T:$x0, (T ImmNode<T>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), RT:$income, i1:$pred, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T, iAny, i1>.Insn T:$x0, (CastOp<T>.Cast $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), RT:$income, i1:$pred, (i1 imm:$polarity)))>;
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

multiclass BinaryVectorIntrinsics<SDNode Intrin, string IName, ValueType T1, ValueType T2, ValueType P, ValueType RT = T1> {
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

multiclass BinaryVectorIntrinsicsDnorm<SDNode Intrin, string IName, ValueType T1, ValueType T2, ValueType P, ValueType RT = T1> {
  def : Pat<(RT (Intrin
                 T1:$op1,
                 T2:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, T2, P>.Insn
                 T1:$op1,
                 T2:$op2,
                 (i8 imm:$optype), (AddDnormSupport (i32 imm:$sw)), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (T2 (SplatNode<T2>.Splat VectorElement<T2>.Type:$op2)),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, VectorElement<T2>.Type, P>.Insn
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (AddDnormSupport (i32 imm:$sw)), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, VectorElement<T2>.Type, P>.Insn
                 T1:$op1,
                 VectorElement<T2>.Type:$op2,
                 (i8 imm:$optype), (AddDnormSupport (i32 imm:$sw)), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (T2 (SplatNode<T2>.Splat (VectorElement<T2>.Type ImmNode<T2>.Node:$op2))),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, iAny, P>.Insn
                 T1:$op1,
                 (CastOp<T2>.Cast $op2),
                 (i8 imm:$optype), (AddDnormSupport (i32 imm:$sw)), RT:$income, P:$P, (i1 imm:$polarity)))>;
  def : Pat<(RT (Intrin
                 T1:$op1,
                 (VectorElement<T2>.Type ImmNode<T2>.Node:$op2),
                 (i8 imm:$optype), (i32 imm:$sw), RT:$income, P:$P, (i1 imm:$polarity))),
            (RT (ConstructInsn<IName, T1, iAny, P>.Insn
                 T1:$op1,
                 (CastOp<T2>.Cast $op2),
                 (i8 imm:$optype), (AddDnormSupport (i32 imm:$sw)), RT:$income, P:$P, (i1 imm:$polarity)))>;
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
def MOV_ld_mvp : LoadInst_Mov <0b000101, VPRF, VRF,    "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmvp">, Requires<[VRF2VPRF]>;
def MOV_ld_mvm : LoadInst_Mov <0b000101, VPRF, VRF,    "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmvm">, Requires<[VRF2VPRF]>;
def MOV_ld_vmp : LoadInst_Mov <0b000101, VRF,  VPRF,   "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvmp">, Requires<[VRF2VPRF]>;
def MOV_ld_vmm : LoadInst_Mov <0b000101, VRF,  VPRF,   "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvmm">, Requires<[VRF2VPRF]>;
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

def MOV_ld_aap  : LoadInst_Mov     <0b000101, ADRF, ADRF,   "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVaap">, Requires<[ADRFMov]>;

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
  def MOV_ld_gphp : LoadInst_MovFromHWRegG<0b000101, SPRF, "mov.from_hw_reg $sw $dest, $reg, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_gvhp : LoadInst_MovFromHWRegG<0b000101, VRF,  "mov.from_hw_reg $sw $dest, $reg, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_gvhm : LoadInst_MovFromHWRegG<0b000101, VRF,  "mov.from_hw_reg $sw $dest, $reg, $pred", VPred>, Requires<[HWMov]>;
  def MOV_ld_gmhp : LoadInst_MovFromHWRegG<0b000101, VPRF, "mov.from_hw_reg $sw $dest, $reg, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_gmhm : LoadInst_MovFromHWRegG<0b000101, VPRF, "mov.from_hw_reg $sw $dest, $reg, $pred", VPred>, Requires<[HWMov]>;

  def MOV_ld_ghsp : LoadInst_MovToHWRegG<0b000101, SRF,  "mov.to_hw_reg $sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_ghpp : LoadInst_MovToHWRegG<0b000101, SPRF, "mov.to_hw_reg $sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_ghvp : LoadInst_MovToHWRegG<0b000101, VRF,  "mov.to_hw_reg $sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_ghvm : LoadInst_MovToHWRegG<0b000101, VRF,  "mov.to_hw_reg $sw $reg, $src, $pred", VPred>, Requires<[HWMov]>;
  def MOV_ld_ghmp : LoadInst_MovToHWRegG<0b000101, VPRF, "mov.to_hw_reg $sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOV_ld_ghmm : LoadInst_MovToHWRegG<0b000101, VPRF, "mov.to_hw_reg $sw $reg, $src, $pred", VPred>, Requires<[HWMov]>;
  def MOV_ld_ghip : LoadInst_MovToHWRegG<0b000101, TPCImm<i32imm>,  "mov.to_hw_reg $sw $reg, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOV_ld_ghim : LoadInst_MovToHWRegG<0b000101, TPCImm<i32imm>,  "mov.to_hw_reg $sw $reg, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

// Normal MOV instructions.
// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOV_ld_shp : LoadInst_MovFromHWReg<0b000101, SRF,  HSRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVshp">, Requires<[GetHSRF]>;
}
let isAsmParserOnly = 1 in {
  def MOV_ld_gshpPriorDoron1 : LoadInst_MovFromHWReg<0b000101, SRF, HSRFPriorDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[GetHSRF, PriorDoron1]>;
  def MOV_ld_gshpDoron1 : LoadInst_MovFromHWReg<0b000101, SRF, HSRFDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[GetHSRF, Doron1]>;
}

def MOV_ld_php : LoadInst_MovFromHWReg<0b000101, SPRF, HSPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVphp">, Requires<[HWMov]>;

// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOV_ld_vhp : LoadInst_MovFromHWReg<0b000101, VRF, HVRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvhp">, Requires<[GetHSRF]>;
  def MOV_ld_vhm : LoadInst_MovFromHWReg<0b000101, VRF, HVRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvhm">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOV_ld_gvhpPriorDoron1 : LoadInst_MovFromHWReg<0b000101, VRF, HVRFPriorDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[GetHSRF, PriorDoron1]>;
  def MOV_ld_gvhmPriorDoron1 : LoadInst_MovFromHWReg<0b000101, VRF, HVRFPriorDoron1, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, PriorDoron1]>;
  def MOV_ld_gvhpDoron1 : LoadInst_MovFromHWReg<0b000101, VRF, HVRFDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[GetHSRF, Doron1]>;
  def MOV_ld_gvhmDoron1 : LoadInst_MovFromHWReg<0b000101, VRF, HVRFDoron1, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

def MOV_ld_mhp : LoadInst_MovFromHWReg<0b000101, VPRF, HVPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmhp">, Requires<[HWMov]>;
def MOV_ld_mhm : LoadInst_MovFromHWReg<0b000101, VPRF, HVPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmhm">, Requires<[HWMov]>;

def MOV_ld_spcp : LoadInst_MovFromHWReg<0b000101, SRF, HWPC, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVspcp">, Requires<[Doron1]>;

// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOV_ld_hsp : LoadInst_MovToHWReg<0b000101, HSRF, SRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhsp">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOV_ld_ghspPriorDoron1 : LoadInst_MovToHWReg<0b000101, HSRFPriorDoron1, SRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, PriorDoron1]>;
  def MOV_ld_ghspDoron1 : LoadInst_MovToHWReg<0b000101, HSRFDoron1RW, SRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
}

def MOV_ld_hsip : LoadInst_MovToHWReg<0b000101, HSRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhsip">, Requires<[HWMov, Doron1]>;

def MOV_ld_hpp : LoadInst_MovToHWReg<0b000101, HSPRF, SPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhpp">, Requires<[HWMov]>;

// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOV_ld_hvp : LoadInst_MovToHWReg<0b000101, HVRF, VRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvp">, Requires<[HWMov]>;
  def MOV_ld_hvm : LoadInst_MovToHWReg<0b000101, HVRF, VRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvm">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOV_ld_ghvpPriorDoron1 : LoadInst_MovToHWReg<0b000101, HVRFPriorDoron1, VRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, PriorDoron1]>;
  def MOV_ld_ghvmPriorDoron1 : LoadInst_MovToHWReg<0b000101, HVRFPriorDoron1, VRF, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, PriorDoron1]>;
  def MOV_ld_ghvpDoron1 : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, VRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOV_ld_ghvmDoron1 : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, VRF, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

def MOV_ld_hvsp : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, SRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvsp">, Requires<[HWMov, Doron1]>;
def MOV_ld_hvsm : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, SRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvsm">, Requires<[HWMov, Doron1]>;
def MOV_ld_hvip : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvip">, Requires<[HWMov, Doron1]>;
def MOV_ld_hvim : LoadInst_MovToHWReg<0b000101, HVRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvim">, Requires<[HWMov, Doron1]>;

def MOV_ld_hmp : LoadInst_MovToHWReg<0b000101, HVPRF, VPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhmp">, Requires<[HWMov]>;
def MOV_ld_hmm : LoadInst_MovToHWReg<0b000101, HVPRF, VPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhmm">, Requires<[HWMov]>;

// SPU slot.

def MOVssp  : SpuInst_Mov     <0b001000, SRF,  SRF,    "mov$optype $sw $dest, $src, $pred">, MultiSlot<"MOVssp">;
def MOVsip  : SpuInst_MovI    <0b001000, SRF,  i32imm, "mov$optype $sw $dest, $src, $pred">, MultiSlot<"MOVsip">;
def MOVpsp  : SpuInst_MovType <0b001000, SPRF, SRF,    "mov $sw $dest, $src, $pred", OpType.BOOL>, MultiSlot<"MOVpsp">;
def MOVpip  : SpuInst_MovTypeI<0b001000, SPRF, i32imm, "mov $sw $dest, $src, $pred", OpType.BOOL>, MultiSlot<"MOVpip">;
def MOVppp  : SpuInst_MovType <0b001000, SPRF, SPRF,   "mov $sw $dest, $src, $pred", OpType.BOOL>;

def MOVIsp  : SpuInst_MovMask <0b001000, IRF,  SRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIsp">;
def MOVIip  : SpuInst_MovMaskI<0b001000, IRF,  i32imm, "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIip">;
def MOVIIp  : SpuInst_MovMask <0b001000, IRF,  IRF,    "mov $sw $mask $dest, $src, $pred">, MultiSlot<"MOVIIp">;
def MOVwIsp : SpuInst_MovMReg <0b001000, IRF,  SRF,    "mov $sw $mask $dest, $src, $pred">, Requires<[DimMaskR]>;
def MOVwIip : SpuInst_MovMRegI<0b001000, IRF,  i32imm, "mov $sw $mask $dest, $src, $pred">, Requires<[DimMaskR]>;
def MOVwIIp : SpuInst_MovMReg <0b001000, IRF,  IRF,    "mov $sw $mask $dest, $src, $pred">, Requires<[DimMaskR]>;

def MOVaap  : SpuInst_MovType <0b001000, ADRF, ADRF,   "mov $sw $dest, $src, $pred", OpType.UINT32>, MultiSlot<"MOVaap">, Requires<[ADRFMov]>;
def MOVazp  : SpuInst_MovType <0b001000, ADRF, ZRF,    "mov $sw $dest, $src, $pred", OpType.UINT32>, Requires<[ADRFMov]>;
def MOVzap  : SpuInst_MovType <0b001000, ZRF,  ADRF,   "mov $sw $dest, $src, $pred", OpType.UINT32>, Requires<[ADRFMov]>;

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
  def MOVgphp : SpuInst_MovFromHWRegG<0b001000, SPRF, "mov.from_hw_reg $sw $dest, $reg, $pred">, Requires<[HWMov]>;
  def MOVghsp : SpuInst_MovToHWRegG  <0b001000, SRF,  "mov.to_hw_reg $sw $reg, $src, $pred">, Requires<[HWMov]>;
  def MOVghpp : SpuInst_MovToHWRegG  <0b001000, SPRF, "mov.to_hw_reg $sw $reg, $src, $pred">, Requires<[HWMov]>;
  def MOVgship : SpuInst_MovToHWRegG<0b001000, TPCImm<i32imm>, "mov.to_hw_reg $sw $reg, $src, $pred">, Requires<[HWMov, Doron1]>;
}

let isBarrier = 1, hasSideEffects = 1 in {
  def MOVsqz : SpuInst_MovToHWRegG  <0b001000, SRF,  "mov.to_hw_reg $sw $reg, $src, $pred">, Requires<[HWMov, PriorDoron1]>;
}

// Normal MOV instructions.
// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOVshp : SpuInst_MovFromHWReg<0b001000, SRF,  HSRF,  "mov $sw $dest, $src, $pred">, MultiSlot<"MOVshp">, Requires<[GetHSRF]>;
}
let isAsmParserOnly = 1 in {
  def MOVgshpPriorDoron1 : SpuInst_MovFromHWReg<0b001000, SRF, HSRFPriorDoron1, "mov $sw $dest, $src, $pred">, Requires<[GetHSRF, PriorDoron1]>;
  def MOVgshpDoron1 : SpuInst_MovFromHWReg<0b001000, SRF, HSRFDoron1, "mov $sw $dest, $src, $pred">, Requires<[GetHSRF, Doron1]>;
}
def MOVphp : SpuInst_MovFromHWReg<0b001000, SPRF, HSPRF, "mov $sw $dest, $src, $pred">, MultiSlot<"MOVphp">, Requires<[HWMov]>;

// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOVhsp : SpuInst_MovToHWReg<0b001000, HSRF, SRF, "mov $sw $dest, $src, $pred">, MultiSlot<"MOVhsp">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOVghspPriorDoron1 : SpuInst_MovToHWReg<0b001000, HSRFPriorDoron1, SRF, "mov $sw $dest, $src, $pred">, Requires<[HWMov, PriorDoron1]>;
  def MOVghspDoron1 : SpuInst_MovToHWReg<0b001000, HSRFDoron1RW, SRF, "mov $sw $dest, $src, $pred">, Requires<[HWMov, Doron1]>;
}

def MOVhsip : SpuInst_MovToHWReg<0b001000, HSRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred">, MultiSlot<"MOVhsip">, Requires<[HWMov, Doron1]>;

def MOVhpp : SpuInst_MovToHWReg<0b001000, HSPRF, SPRF, "mov $sw $dest, $src, $pred">, MultiSlot<"MOVhpp">, Requires<[HWMov]>;

// VPU slot.

def MOVvvp  : VpuInst_Mov<0b001000, VRF,  VRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvvp">;
def MOVvvm  : VpuInst_Mov<0b001000, VRF,  VRF,  "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvvm">;

def MOVmvp  : VpuInst_MovBit<0b001000, VPRF, VRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmvp">, Requires<[VRF2VPRF]>;
def MOVmvm  : VpuInst_MovBit<0b001000, VPRF, VRF,  "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmvm">, Requires<[VRF2VPRF]>;
def MOVvmp  : VpuInst_MovBit<0b001000, VRF,  VPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvmp">, Requires<[VRF2VPRF]>;
def MOVvmm  : VpuInst_MovBit<0b001000, VRF,  VPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvmm">, Requires<[VRF2VPRF]>;
def MOVmmp  : VpuInst_MovBit<0b001000, VPRF, VPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmmp">;
def MOVmmm  : VpuInst_MovBit<0b001000, VPRF, VPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmmm">;
def MOVmpp  : VpuInst_MovBit<0b001000, VPRF, SPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmpp">;
def MOVmpm  : VpuInst_MovBit<0b001000, VPRF, SPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmpm">;

let Switches = 0b0000100 in { // SW_X2_MOV
  def MOVddp  : VpuInst_Mov<0b001000, DRF,  DRF,  "mov $sw $dest, $src, $pred", SPred>, Requires<[Gen3Plus]>;
  def MOVddm  : VpuInst_Mov<0b001000, DRF,  DRF,  "mov $sw $dest, $src, $pred", VPred>, Requires<[Gen3Plus]>;
}

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

// HW register moves.

// Generic mov_hwreg instructions. HW registers in them are specified by
// immediate. Such instructions are for assembler only, they are useless
// for code generator and not interesting for disassembler.
let isAsmParserOnly = 1 in {
  def MOVgvhp : VpuInst_MovFromHWRegG<0b001000, VRF,  "mov.from_hw_reg$sw $dest, $reg, $pred", SPred>, Requires<[HWMov]>;
  def MOVgvhm : VpuInst_MovFromHWRegG<0b001000, VRF,  "mov.from_hw_reg$sw $dest, $reg, $pred", VPred>, Requires<[HWMov]>;
  def MOVgmhp : VpuInst_MovFromHWRegG<0b001000, VPRF, "mov.from_hw_reg$sw $dest, $reg, $pred", SPred>, Requires<[HWMov]>;
  def MOVgmhm : VpuInst_MovFromHWRegG<0b001000, VPRF, "mov.from_hw_reg$sw $dest, $reg, $pred", VPred>, Requires<[HWMov]>;

  def MOVghvp : VpuInst_MovToHWRegG<0b001000, VRF,  "mov.to_hw_reg$sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOVghvm : VpuInst_MovToHWRegG<0b001000, VRF,  "mov.to_hw_reg$sw $reg, $src, $pred", VPred>, Requires<[HWMov]>;
  def MOVghmp : VpuInst_MovToHWRegG<0b001000, VPRF, "mov.to_hw_reg$sw $reg, $src, $pred", SPred>, Requires<[HWMov]>;
  def MOVghmm : VpuInst_MovToHWRegG<0b001000, VPRF, "mov.to_hw_reg$sw $reg, $src, $pred", VPred>, Requires<[HWMov]>;
  def MOVgvhip : VpuInst_MovToHWRegG<0b001000, TPCImm<i32imm>, "mov.to_hw_reg$sw $reg, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOVgvhim : VpuInst_MovToHWRegG<0b001000, TPCImm<i32imm>, "mov.to_hw_reg$sw $reg, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
  def MOVgvhsp : VpuInst_MovToHWRegG<0b001000, SRF, "mov.to_hw_reg$sw $reg, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOVgvhsm : VpuInst_MovToHWRegG<0b001000, SRF, "mov.to_hw_reg$sw $reg, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

// Normal MOV instructions.
// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOVvhp : VpuInst_MovFromHWReg<0b001000, VRF, HVRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVvhp">, Requires<[HWMov]>;
  def MOVvhm : VpuInst_MovFromHWReg<0b001000, VRF, HVRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVvhm">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOVgvhpPriorDoron1 : VpuInst_MovFromHWReg<0b001000, VRF, HVRFPriorDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, PriorDoron1]>;
  def MOVgvhmPriorDoron1 : VpuInst_MovFromHWReg<0b001000, VRF, HVRFPriorDoron1, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, PriorDoron1]>;
  def MOVgvhpDoron1 : VpuInst_MovFromHWReg<0b001000, VRF, HVRFDoron1, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOVgvhmDoron1 : VpuInst_MovFromHWReg<0b001000, VRF, HVRFDoron1, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

def MOVmhp : VpuInst_MovFromHWReg<0b001000, VPRF, HVPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVmhp">, Requires<[HWMov]>;
def MOVmhm : VpuInst_MovFromHWReg<0b001000, VPRF, HVPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVmhm">, Requires<[HWMov]>;

// A register may be inexistent for some architectures.
// Codegen is not checking it.
let isCodeGenOnly = 1 in {
  def MOVhvp : VpuInst_MovToHWReg<0b001000, HVRF,  VRF,  "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvp">, Requires<[HWMov]>;
  def MOVhvm : VpuInst_MovToHWReg<0b001000, HVRF,  VRF,  "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvm">, Requires<[HWMov]>;
}
let isAsmParserOnly = 1 in {
  def MOVghvpPriorDoron1 : VpuInst_MovToHWReg<0b001000, HVRFPriorDoron1, VRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, PriorDoron1]>;
  def MOVghvmPriorDoron1 : VpuInst_MovToHWReg<0b001000, HVRFPriorDoron1, VRF, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, PriorDoron1]>;
  def MOVghvpDoron1 : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, VRF, "mov $sw $dest, $src, $pred", SPred>, Requires<[HWMov, Doron1]>;
  def MOVghvmDoron1 : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, VRF, "mov $sw $dest, $src, $pred", VPred>, Requires<[HWMov, Doron1]>;
}

def MOVhvsp : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, SRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvsp">, Requires<[HWMov, Doron1]>;
def MOVhvsm : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, SRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvsm">, Requires<[HWMov, Doron1]>;
def MOVhvip : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhvip">, Requires<[HWMov, Doron1]>;
def MOVhvim : VpuInst_MovToHWReg<0b001000, HVRFDoron1RW, TPCImm<i32imm>, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhvim">, Requires<[HWMov, Doron1]>;

def MOVhmp : VpuInst_MovToHWReg<0b001000, HVPRF, VPRF, "mov $sw $dest, $src, $pred", SPred>, MultiSlot<"MOVhmp">, Requires<[HWMov]>;
def MOVhmm : VpuInst_MovToHWReg<0b001000, HVPRF, VPRF, "mov $sw $dest, $src, $pred", VPred>, MultiSlot<"MOVhmm">, Requires<[HWMov]>;

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

def : Pat<(int_tpc_mov_mask v5i32:$src, i32:$mask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVwIIp IRF:$src, MRF:$mask, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[DimMaskR]>;
def : Pat<(int_tpc_mov_mask i32:$src, i32:$mask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVwIsp SRF:$src, MRF:$mask, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[DimMaskR]>;
def : Pat<(int_tpc_mov_mask (i32 imm:$src), i32:$mask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVwIip (CastToImm32 $src), MRF:$mask, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[DimMaskR]>;
def : Pat<(int_tpc_mov_mask (v5i32 (vsplati5 i32:$src)), i32:$mask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVwIsp SRF:$src, MRF:$mask, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[DimMaskR]>;
def : Pat<(int_tpc_mov_mask (v5i32 (vsplati5 (i32 imm:$src))), i32:$mask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (MOVwIip (CastToImm32 $src), MRF:$mask, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[DimMaskR]>;

foreach type = [i1] in {
  def : Pat<(type (int_tpc_mov type:$src, (i8 imm), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"MOV", type, type, i1>.Insn
                   type:$src, (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_mov (type imm:$src), (i8 imm), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"MOV", type, iAny, i1>.Insn
                   (CastToImm32 $src), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

foreach pred = [i1, v256i1] in {
  foreach T = [v256i1, v128i1, v64i1] in {
    def : Pat<(T (int_tpc_mov T:$src, (i8 imm), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, T, pred>.Insn
                       VPRF:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov (T (SplatNode<T>.Splat i1:$src)), (i8 imm), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, i1, pred>.Insn
                       i1:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(T (int_tpc_mov i1:$src, (i8 imm), (i32 imm:$sw), T:$income, pred:$pred, (i1 imm:$polarity))),
              (T (ConstructInsn<"MOV", T, i1, pred>.Insn
                       i1:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  }

  def : Pat<(v256i1 (int_tpc_mov v64i32:$src, (i8 imm), (i32 imm:$sw), v256i1:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i1 (ConstructInsn<"MOV", v256i1, v64i32, pred>.Insn
                     VRF:$src, (i32 imm:$sw), VPRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v64i32 (int_tpc_mov v256i1:$src, (i8 imm), (i32 imm:$sw), v64i32:$income, pred:$pred, (i1 imm:$polarity))),
            (v64i32 (ConstructInsn<"MOV", v64i32, v256i1, pred>.Insn
                     VPRF:$src, (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
}

foreach type = [i32, i16, i8] in {
  def : Pat<(i1 (int_tpc_mov (i1 (trunc type:$src)), (i8 imm), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
            (i1 (MOVpsp SRF:$src, (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(i1 (int_tpc_mov (i1 (trunc (type imm:$src))), (i8 imm), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
            (i1 (MOVpip (CastToImm32 $src), (i32 imm:$sw), SPRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  def : Pat<(type (int_tpc_mov type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVssp SRF:$src, (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

foreach type = [f32, bf16, f16, f8_143, f8_152] in
  def : Pat<(type (int_tpc_mov (type fpimm:$src), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVsip (FPToImm32 $src), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

foreach type = [i32, i16, i8] in
  def : Pat<(type (int_tpc_mov (type imm:$src), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MOVsip (CastToImm32 $src), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


foreach T = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
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

foreach type = [v128f32, v256bf16, v256f16, v128i32, v256i16, v512i8, v512f8_143, v512f8_152] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_mov type:$src, (i8 imm), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"MOV", type, type, pred>.Insn
                     DRF:$src, (i32 imm:$sw), DRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  }

def : Pat<(i1 (i1 imm:$src)), (MOVpip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(i1 (trunc i32:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i1 (trunc i16:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i1 (trunc  i8:$src)), (i1 (MOVpsp SRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def : Pat<(v64f32 (vsplatf32 (f32 fpimm:$src))),
          (v64f32 (MOV_ld_vip (FPToImm32 $src), OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128bf16 (vsplatbf16 (bf16 fpimm:$src))),
          (v128bf16 (MOV_ld_vip (FPToImm32 $src), OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128f16 (vsplatf16 (f16 fpimm:$src))),
          (v128f16 (MOV_ld_vip (FPToImm32 $src), OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256f8_143 (vsplatf8_143 (f8_143 fpimm:$src))),
          (v256f8_143 (MOV_ld_vip (FPToImm32 $src), OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256f8_152 (vsplatf8_152 (f8_152 fpimm:$src))),
          (v256f8_152 (MOV_ld_vip (FPToImm32 $src), OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 (i32 imm:$src))),
          (v64i32 (MOV_ld_vip (CastToImm32 $src), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 (i16 imm:$src))),
          (v128i16 (MOV_ld_vip (CastToImm32 $src), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 (i8 imm:$src))),
          (v256i8 (MOV_ld_vip (CastToImm32 $src), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def PropToTimm32 : SDNodeXForm<timm, [{
  return CurDAG->getTargetConstant(N->getZExtValue(), SDLoc(N), MVT::i32);
}]>;

// https://gaudi.unipro.ru/jira/browse/GAUDI-1895
// https://gaudi.unipro.ru/jira/browse/GAUDI-1896
// After faced with BUILD_VECTOR TargetConstant...
def : Pat<(v256i1 (vsplatb256 (i1 timm:$src))),
          (v256i1 (MOVB_ld_mip (PropToTimm32 $src), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 (i32 timm:$src))),
          (v64i32 (MOV_ld_vip $src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 (i16 timm:$src))),
          (v128i16 (MOV_ld_vip (PropToTimm32 $src), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 (i8 timm:$src))),
          (v256i8 (MOV_ld_vip (PropToTimm32 $src), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def : Pat<(v64f32 (vsplatf32 f32:$src)),
          (v64f32 (MOV_ld_vsp SRF:$src, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128bf16 (vsplatbf16 bf16:$src)),
          (v128bf16 (MOV_ld_vsp SRF:$src, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128f16 (vsplatf16 f16:$src)),
          (v128f16 (MOV_ld_vsp SRF:$src, OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256f8_143 (vsplatf8_143 f8_143:$src)),
          (v256f8_143 (MOV_ld_vsp SRF:$src, OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256f8_152 (vsplatf8_152 f8_152:$src)),
          (v256f8_152 (MOV_ld_vsp SRF:$src, OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64i32 (vsplati32 i32:$src)),
          (v64i32 (MOV_ld_vsp SRF:$src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128i16 (vsplati16 i16:$src)),
          (v128i16 (MOV_ld_vsp SRF:$src, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v256i8 (vsplati8 i8:$src)),
          (v256i8 (MOV_ld_vsp SRF:$src, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

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

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(type (cmove type:$src, i1:$pred)),
            (type (MOVssp type:$src, OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmove (type ImmNode<type>.Node:$src), i1:$pred)),
            (type (MOVsip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), (IMPLICIT_DEF), i1:$pred, (i1 0)))>;
  def : Pat<(type (cmoveinv type:$src, i1:$pred, type:$income)),
            (type (MOVssp type:$src, OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
  def : Pat<(type (cmoveinv (type ImmNode<type>.Node:$src), i1:$pred, type:$income)),
            (type (MOVsip (CastOp<type>.Cast $src), OpTypeForType<type>.V, (i32 0), $income, i1:$pred, (i1 1)))>;
}

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
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
foreach T = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  def : Pat<(T ImmNode<T>.Node:$x), (T (MOVsip (CastOp<T>.Cast $x), OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def : Pat<(i64 (int_tpc_update_addr i64:$income, v2i32:$src, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))),
          (i64 (MOVazp ZRF:$src, (i32 imm:$sw), ADRF:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[ADRFMov]>;
def : Pat<(v2i32 (int_tpc_get_addr i64:$src, (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (v2i32 (MOVzap ADRF:$src, (i32 imm:$sw), ZRF:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[ADRFMov]>;
               

//------ Load slot -------------------------------------------------------------

//------ GEN_ADDR --------------------------------------------------------------

def GEN_ADDR_ld  : LoadInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.DimMask>, MultiSlot<"GEN_ADDR">;
def GEN_ADDR_ldT : LoadInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.Reg>, Requires<[TnsrInReg]>, MultiSlot<"GEN_ADDRT">;

//------ EVENT -----------------------------------------------------------------
def EVENT_ld_sp  : LoadInst_EVENT<0b011000, SRF,            "event $sw $src, $pred">, Requires<[Gen4Plus]>;
let isNotUsedInDisasm = 1 in
def EVENT_ld_ip  : LoadInst_EVENT<0b011000, TPCImm<i32imm>, "event $sw $src, $pred">, Requires<[Gen4Plus]>;

def : Pat<(int_tpc_event i16:$data, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (EVENT_ld_sp SRF:$data, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_event (i16 imm:$data), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (EVENT_ld_ip (CastToImm32 $data), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;


//------ PRMT_INDX -------------------------------------------------------------

def PRMT_INDX_ld : LoadInstrPRMT_INDX<0b000011, "prmt_indx $sw $dest, $src1, $src2, $pred">, MultiSlot<"PRMT_INDX">;


//------ SET_INDX --------------------------------------------------------------

def SET_INDX_ld_rp  : LoadInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_rp">;
def SET_INDX_ld_ip  : LoadInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_ip">;
def SET_INDX_ld_rwp : LoadInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_rwp">, Requires<[DimMaskR]>;
def SET_INDX_ld_iwp : LoadInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_iwp">, Requires<[DimMaskR]>;

def : Pat<(int_tpc_set_indx v5i32:$income, (i32 imm:$dimmask), i32:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_rp IRF:$income, SRF:$value, (i32 imm:$dimmask), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_set_indx v5i32:$income, (i32 imm:$dimmask), (i32 imm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_ip IRF:$income, (CastToImm32 $value), (i32 imm:$dimmask), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_set_indx v5i32:$income, i32:$dimmask, i32:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_rwp IRF:$income, SRF:$value, MRF:$dimmask, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_set_indx v5i32:$income, i32:$dimmask, (i32 imm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (SET_INDX_ld_iwp IRF:$income, (CastToImm32 $value), MRF:$dimmask, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;


//------ LD_L ------------------------------------------------------------------

def LD_Lpsp  : LoadInst_LD_L <0b001011, SPRF, "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lpip  : LoadInst_LD_LI<0b001011, SPRF, "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lssp  : LoadInst_LD_L <0b001011, SRF,  "ld_l $sw $dest, $addr, $pred", []>;
def LD_Lsip  : LoadInst_LD_LI<0b001011, SRF,  "ld_l $sw $dest, $addr, $pred", []>;

def : Pat<(i1 (lsload (i32 imm:$addr))), (LD_Lpip (CastToImm32 $addr), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i1 (lsload i32:$addr)),       (LD_Lpsp $addr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(type (lsload (i32 imm:$addr))), (LD_Lsip (CastToImm32 $addr), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def : Pat<(type (lsload i32:$addr)),       (LD_Lssp $addr,               (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

def : Pat<(i1 (int_tpc_ld_l (i32 imm:$addr), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
          (i1 (LD_Lpip (CastToImm32 $addr), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity)))>;
def : Pat<(i1 (int_tpc_ld_l i32:$addr, (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
          (i1 (LD_Lpsp $addr, (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity)))>;
foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(type (int_tpc_ld_l (i32 imm:$addr), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (LD_Lsip (CastToImm32 $addr), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_l i32:$addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (LD_Lssp $addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
}


//------ LD_G ------------------------------------------------------------------

def LD_Gsap  : LoadInst_LD_G   <0b001100, SRF,  "ld_g $sw $dest, $addr, $pred", SPred>;
def LD_Gzap  : LoadInst_LD_G64 <0b001100, ZRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Doron1]>;
def LD_Gpap  : LoadInst_LD_G   <0b001100, SPRF, "ld_g $sw $dest, $addr, $pred", SPred>;
def LD_Gvap  : LoadInst_LD_G   <0b001100, VRF,  "ld_g $sw $dest, $addr, $pred", SPred>;
def LD_GIap  : LoadInst_LD_GI  <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Imm>, Requires<[DimMaskR]>;
def LD_GIwap : LoadInst_LD_GI  <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Reg>, Requires<[DimMaskR]>;

def LD_G_INCsap : LoadInst_LD_G_INC  <0b001100, SRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen3]>;
def LD_G_INCpap : LoadInst_LD_G_INC  <0b001100, SPRF, "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen3]>;
def LD_G_INCIap : LoadInst_LD_GI_INC <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Imm>, Requires<[Gen3]>;
def LD_G_INCIwap : LoadInst_LD_GI_INC<0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Reg>, Requires<[Gen3]>;

def LD_Gg4sap  : LoadInst_LD_Gg4  <0b001100, SRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_Gg4pap  : LoadInst_LD_Gg4  <0b001100, SPRF, "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_Gg4vap  : LoadInst_LD_Gg4  <0b001100, VRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_Gg4vam  : LoadInst_LD_Gg4  <0b001100, VRF,  "ld_g $sw $dest, $addr, $pred", VPred>, Requires<[Gen4Plus]>;
def LD_Gg4map  : LoadInst_LD_Gg4  <0b001100, VPRF, "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_Gg4mam  : LoadInst_LD_Gg4  <0b001100, VPRF, "ld_g $sw $dest, $addr, $pred", VPred>, Requires<[Gen4Plus]>;
def LD_Gg4Iap  : LoadInst_LD_GIg4 <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Imm>, Requires<[DimMaskR, Gen4Plus]>;
def LD_Gg4Iwap : LoadInst_LD_GIg4 <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Reg>, Requires<[DimMaskR, Gen4Plus]>;

def LD_Gg4_INCsap  : LoadInst_LD_Gg4_INC  <0b001100, SRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_Gg5_INCzap  : LoadInst_LD_G64_INC  <0b001100, ZRF,  "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Doron1]>;
def LD_Gg4_INCpap  : LoadInst_LD_Gg4_INC  <0b001100, SPRF, "ld_g $sw $dest, $addr, $pred", SPred>, Requires<[Gen4Plus]>;
def LD_G_INCg4Iap  : LoadInst_LD_GI_INCg4 <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Imm>, Requires<[Gen4Plus]>;
def LD_G_INCg4Iwap : LoadInst_LD_GI_INCg4 <0b001100, IRF,  "ld_g $sw $dimmask $dest, $addr, $pred", DMask.Reg>, Requires<[Gen4Plus]>;

def LD_G_Pvap : LoadInst_LD_G_P<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>, Requires<[Partial, Gen3]>;
def LD_G_Pvam : LoadInst_LD_G_P<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>, Requires<[Partial, Gen3]>;

def LD_G_Pg4vap : LoadInst_LD_G_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>, Requires<[Partial, Gen4Plus]>;
def LD_G_Pg4vam : LoadInst_LD_G_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>, Requires<[Partial, Gen4Plus]>;

def LD_G_P_INCvap : LoadInst_LD_G_P_INC<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>, Requires<[Partial, Gen3]>;
def LD_G_P_INCvam : LoadInst_LD_G_P_INC<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>, Requires<[Partial, Gen3]>;

def LD_G_P_INCg4vap : LoadInst_LD_G_P_INC_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>, Requires<[Partial, Gen4Plus]>;
def LD_G_P_INCg4vam : LoadInst_LD_G_P_INC_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>, Requires<[Partial, Gen4Plus]>;

foreach type = [i1, i8, i16, i32, f32, bf16, f16, f8_143, f8_152] in
  def : Pat<(type (gload i64:$addr)),
            (type (ConstructInsn<"LD_G", type, i64, i1>.Insn
                   ADRF:$addr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

// Patterns for intrinsics 'int_tpc_ld_g'. See getTgtMemIntrinsic() and LowerINTRINSIC_W_CHAIN()
// in TPCISelLowering.cpp for how the node 'gloadp' is mapped to 'int_tpc_ld_g'
foreach type = [i1, i8, i16, i32, f32, bf16, f16, f8_143, f8_152, v64i32, v128i16, v256i8, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def : Pat<(type (gloadp i64:$addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_G", type, i64, i1>.Insn
                   ADRF:$addr, (i32 imm:$sw), (type $income), SPRF:$pred, (i1 imm:$polarity)))>, Requires<[PriorGen4]>;
}

def : Pat<(v2i32 (gloadp   i64:$addr, (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (v2i32 (LD_Gzap ADRF:$addr, (i32 imm:$sw),   ZRF:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;

foreach type = [i1, i8, i16, i32, f32, bf16, f16, f8_143, f8_152, v64i32, v128i16, v256i8, v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def : Pat<(type (gloadp i64:$addr, (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_Gg4", type, i64, i1>.Insn
                   ADRF:$addr, (i32 imm:$sw), (type $income), SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus]>;
}

foreach type = [v64i32, v128i16, v256i8, v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def : Pat<(type (gloadp i64:$addr, (i32 imm:$sw), type:$income, v256i1:$pred, (i1 imm:$polarity))),
    (type (ConstructInsn<"LD_Gg4", type, i64, v256i1>.Insn
           ADRF:$addr, (i32 imm:$sw), type:$income, VPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus]>;
}

def : Pat<(v5i32 (gloadIp i64:$addr, (i32 imm:$dimmask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
  (v5i32 (LD_GIap  ADRF:$addr, (i32 imm:$dimmask), (i32 imm:$sw), v5i32:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen3]>;
def : Pat<(v5i32 (gloadIp i64:$addr, i32:$dimmask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
  (v5i32 (LD_GIwap ADRF:$addr, MRF:$dimmask, (i32 imm:$sw), v5i32:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen3, DimMaskR]>;

def : Pat<(v5i32 (gloadIp i64:$addr, (i32 imm:$dimmask), (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
  (v5i32 (LD_Gg4Iap  ADRF:$addr, (i32 imm:$dimmask), (i32 imm:$sw), v5i32:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus]>;
def : Pat<(v5i32 (gloadIp i64:$addr, i32:$dimmask, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity))),
  (v5i32 (LD_Gg4Iwap ADRF:$addr, MRF:$dimmask, (i32 imm:$sw), v5i32:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus, DimMaskR]>;

foreach type = [v64i32, v128i16, v256i8, v64f32, v128bf16, v128f16] in {
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_g_partial i64:$addr, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
      (type (ConstructInsn<"LD_G_P", type, i64, pred>.Insn
         ADRF:$addr, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Partial, Gen3]>;
  }
}

foreach type = [v64i32, v128i16, v256i8, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_g_partial i64:$addr, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
      (type (ConstructInsn<"LD_G_Pg4", type, i64, pred>.Insn
         ADRF:$addr, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Partial, Gen4Plus]>;
  }
}

//------ PREFETCH --------------------------------------------------------------

def PREFETCH : LoadInstr_PREFETCH<0b001101, "prefetch $sw $addr, $pred">, Requires<[Gen2Plus]> {
  let hasSideEffects = 1;
}

def : Pat<(int_tpc_prefetch i64:$addr, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (PREFETCH ADRF:$addr, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus]>;


//------ LD_L_V ----------------------------------------------------------------

defm LD_L_V : LoadInst_LD_L_V<0b001110, "ld_l_v $sw $dest, $addr, $pred">;

foreach type = [v256i1, v128i1, v64i1, v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def : Pat<(type (lvload (i32 imm:$addr))),
            (type (ConstructInsn<"LD_L_V", type, iAny, i1>.Insn
                   (i32 imm:$addr), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Addr1]>;
  def : Pat<(type (lvload (i32 i32:$addr))),
            (type (ConstructInsn<"LD_L_V", type, i32, i1>.Insn
                   (i32 $addr), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Addr1]>;
  def : Pat<(type (lvload (i32 imm:$addr))),
            (type (ConstructInsn<"LD_L_V", type, i32, iAny, i1>.Insn
                   (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                   (i32 0), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Addr2]>;
  def : Pat<(type (lvload i32:$addr)),
            (type (ConstructInsn<"LD_L_V", type, i32, iAny, i1>.Insn
                   (i32 $addr), (i32 0), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Addr2]>;
}

foreach type = [v256i1, v128i1, v64i1, v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
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

foreach type = [v256i1, v128i1, v64i1, v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v_low (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_LOW", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
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

foreach type = [v256i1, v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [v256i1, i1] in {
    def : Pat<(type (int_tpc_ld_l_v_high (i32 imm:$addr), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_L_V_HIGH", type, i32, iAny, pred>.Insn
                     (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
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
def LOOKUP_3    : LoadInst_LOOKUP_GEN3<0b000110, VRF, "lookup      $sw $dest, $src, $fid, $pred">, Requires<[Gen3Plus]>;
def LOOKUP_C0   : LoadInst_LOOKUP_GEN1<0b001001, VRF, "lookup_c0   $sw $dest, $src, $fid, $pred">, Requires<[Gen1]>;
def LOOKUP_C1C2 : LoadInst_LOOKUP_GEN1<0b000111, DRF, "lookup_c1c2 $sw $dest, $src, $fid, $pred">, Requires<[Gen1]>;
def LOOKUP_1Ci  : LoadInst_LOOKUP_GEN2<0b001001, VRF, "lookup_1c   $sw $dest, $src, $fid, $pred">, Requires<[Gen2Plus]>;
def LOOKUP_1Cs  : LoadInst_LOOKUP_GEN3<0b001001, VRF, "lookup_1c   $sw $dest, $src, $fid, $pred">, Requires<[Gen3Plus]>;
def LOOKUP_2Ci   : LoadInst_LOOKUP_GEN2<0b000111, DRF, "lookup_2c   $sw $dest, $src, $fid, $pred">, Requires<[Gen2Plus]>;
def LOOKUP_2Cs  : LoadInst_LOOKUP_GEN3<0b000111, DRF, "lookup_2c   $sw $dest, $src, $fid, $pred">, Requires<[Gen3Plus]>;

foreach T1 = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
    foreach T2 = [v256i8, v128i16, v64i32] in {
      def : Pat<(T1 (int_tpc_lookup T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(T1 (int_tpc_lookup T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen2Plus]>;
      def : Pat<(T1 (int_tpc_lookup T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_3 T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen3Plus]>;
      def : Pat<(T1 (int_tpc_lookup_c0 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_C0 T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_c1c2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_C1C2 T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen1]>;
      def : Pat<(T1 (int_tpc_lookup_1c T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1Ci T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
      def : Pat<(T1 (int_tpc_lookup_1c T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1Cs T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_2c T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_2Ci T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_2c T2:$src, i32:$src1, (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_2Cs T2:$src, i32:$src1, (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[Gen3TillDoron1]>;

      def : Pat<(T1 (int_tpc_lookup_1c T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1Ci T2:$src, (i32 imm:$fid), (LkpPart2Switch $sw),
                (T1 (LOOKUP_1Ci T2:$src, (i32 imm:$fid), (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))), i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
      def : Pat<(T1 (int_tpc_lookup_1c T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))),
                (T1 (LOOKUP_1Cs T2:$src, i32:$src1, (LkpPart2Switch $sw),
                (T1 (LOOKUP_1Cs T2:$src, i32:$src1, (i32 imm:$sw), T1:$income, i1:$pred, (i1 imm:$polarity))), i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;

      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_2c T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_2Ci T2:$src, (i32 imm:$fid), (LkpPart2Switch $sw),
                (DoubleVector<T1>.Type (LOOKUP_2Ci T2:$src, (i32 imm:$fid), (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))), i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
      def : Pat<(DoubleVector<T1>.Type (int_tpc_lookup_2c T2:$src, i32:$src1, (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))),
                (DoubleVector<T1>.Type (LOOKUP_2Cs T2:$src, i32:$src1, (LkpPart2Switch $sw),
                (DoubleVector<T1>.Type (LOOKUP_2Cs T2:$src, i32:$src1, (i32 imm:$sw), DoubleVector<T1>.Type:$income, i1:$pred, (i1 imm:$polarity))), i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
    }


//------ LD_TNSR ---------------------------------------------------------------

multiclass Ld_Tnsr<bits<6> OpCode, string op_name, string asmstr> {
  def vp   : LoadInst_LD_TNSR   <OpCode, VRF,  asmstr, SPred>, MultiSlotGen3<op_name#"vp">;
  def vm   : LoadInst_LD_TNSR   <OpCode, VRF,  asmstr, VPred>, MultiSlotGen3<op_name#"vm">, Requires<[LdVectMask]>;
  def mp   : LoadInst_LD_TNSR   <OpCode, VPRF, asmstr, SPred>, MultiSlotGen3<op_name#"mp">;
  def mm   : LoadInst_LD_TNSR   <OpCode, VPRF, asmstr, VPred>, MultiSlotGen3<op_name#"mm">, Requires<[LdVectMask]>;

  // Instructions where tensor is specified in register S27. We don't use the
  // 'FeatureTnsrInReg' for them, as this feature assumes the tensor in HW
  // register. S27 is used for this purpose only in Gen2, so there is no
  // dedicated feature.
  def Gen2Tvp  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, SPred, 0>, Requires<[Gen2]>;
  def Gen2Tvm  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, VPred, 0>, Requires<[Gen2, LdVectMask]>;
  def Gen2Tmp  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, SPred, 0>, Requires<[Gen2]>;
  def Gen2Tmm  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, VPred, 0>, Requires<[Gen2, LdVectMask]>;

  def Tvp  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, SPred, 1>, MultiSlotGen3<op_name#"Tvp">, Requires<[TnsrInReg]>;
  def Tvm  : LoadInst_LD_TNSR_R <OpCode, VRF,  asmstr, VPred, 1>, MultiSlotGen3<op_name#"Tvm">, Requires<[TnsrInReg, LdVectMask]>;
  def Tmp  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, SPred, 1>, MultiSlotGen3<op_name#"Tmp">, Requires<[TnsrInReg]>;
  def Tmm  : LoadInst_LD_TNSR_R <OpCode, VPRF, asmstr, VPred, 1>, MultiSlotGen3<op_name#"Tmm">, Requires<[TnsrInReg, LdVectMask]>;
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

def LD_TNSR_Pvp   : LoadInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_Pvm   : LoadInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;
def LD_TNSR_Pmp   : LoadInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_Pmm   : LoadInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;
def LD_TNSR_PTvp  : LoadInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial, TnsrInReg]>;
def LD_TNSR_PTvm  : LoadInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, TnsrInReg, LdVectMask]>;
def LD_TNSR_PTmp  : LoadInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial, TnsrInReg]>;
def LD_TNSR_PTmm  : LoadInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, TnsrInReg, LdVectMask]>;

def LD_TNSR_Dvp   : LoadInst_LD_TNSR_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_Dvm   : LoadInst_LD_TNSR_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;
def LD_TNSR_Dmp   : LoadInst_LD_TNSR_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_Dmm   : LoadInst_LD_TNSR_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;

def LD_TNSR_PDvp   : LoadInst_LD_TNSR_PD <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_PDvm   : LoadInst_LD_TNSR_PD <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;
def LD_TNSR_PDmp   : LoadInst_LD_TNSR_PD <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_PDmm   : LoadInst_LD_TNSR_PD <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr v5i32:$ndx, S27, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSRGen2T", type, pred>.Insn v5i32:$ndx,
                   S27, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
  def : Pat<(type (int_tpc_ld_tnsr v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSRT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[TnsrInReg]>;
  def : Pat<(type (int_tpc_ld_tnsr_direct i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_D", type, pred>.Insn i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Direct]>;
}

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PGen2", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Gen2]>;
    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, i8:$tensor, i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, i32:$offsize, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Gen2]>;

    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_P", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial]>;
    def : Pat<(type (int_tpc_ld_tnsr_partial v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, TnsrInReg]>;
    def : Pat<(type (int_tpc_ld_tnsr_partial_direct i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_PD", type, pred>.Insn i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, Direct]>;
  }

//------ LD_TNSR_LOW -----------------------------------------------------------

defm LD_TNSR_LOW : Ld_Tnsr<0b010010, "LD_TNSR_LOW", "ld_tnsr_low $sw $dst, $tensor, $coords, $pred">;

def LD_TNSR_LOW_Pvp   : LoadInst_LD_TNSR_P <0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_LOW_Pvm   : LoadInst_LD_TNSR_P <0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;
def LD_TNSR_LOW_Pmp   : LoadInst_LD_TNSR_P <0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_LOW_Pmm   : LoadInst_LD_TNSR_P <0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;

def LD_TNSR_LOW_PTvp  : LoadInst_LD_TNSR_PR<0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[TnsrInReg]>;
def LD_TNSR_LOW_PTvm  : LoadInst_LD_TNSR_PR<0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[TnsrInReg]>;
def LD_TNSR_LOW_PTmp  : LoadInst_LD_TNSR_PR<0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[TnsrInReg]>;
def LD_TNSR_LOW_PTmm  : LoadInst_LD_TNSR_PR<0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[TnsrInReg]>;

def LD_TNSR_LOW_Dvp   : LoadInst_LD_TNSR_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_LOW_Dvm   : LoadInst_LD_TNSR_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;
def LD_TNSR_LOW_Dmp   : LoadInst_LD_TNSR_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_LOW_Dmm   : LoadInst_LD_TNSR_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;

def LD_TNSR_LOW_PDvp  : LoadInst_LD_TNSR_PD <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_LOW_PDvm  : LoadInst_LD_TNSR_PD <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;
def LD_TNSR_LOW_PDmp  : LoadInst_LD_TNSR_PD <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_LOW_PDmm  : LoadInst_LD_TNSR_PD <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr_low v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOW", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr_low v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOWGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
  def : Pat<(type (int_tpc_ld_tnsr_low v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOWT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[TnsrInReg]>;
  def : Pat<(type (int_tpc_ld_tnsr_low_direct i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_LOW_D", type, pred>.Insn i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Direct]>;
  }

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ld_tnsr_low_partial v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_LOW_P", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial]>;
    def : Pat<(type (int_tpc_ld_tnsr_low_partial v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_LOW_PT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, TnsrInReg]>;
    def : Pat<(type (int_tpc_ld_tnsr_low_partial_direct i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_LOW_PD", type, pred>.Insn i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, Direct]>;
  }


//------ LD_TNSR_HIGH ----------------------------------------------------------

defm LD_TNSR_HIGH : Ld_Tnsr<0b010011, "LD_TNSR_HIGH", "ld_tnsr_high $sw $dst, $tensor, $coords, $pred">;
def LD_TNSR_HIGH_Pvp   : LoadInst_LD_TNSR_P <0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_HIGH_Pvm   : LoadInst_LD_TNSR_P <0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;
def LD_TNSR_HIGH_Pmp   : LoadInst_LD_TNSR_P <0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial]>;
def LD_TNSR_HIGH_Pmm   : LoadInst_LD_TNSR_P <0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask]>;

def LD_TNSR_HIGH_PTvp  : LoadInst_LD_TNSR_PR<0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial, TnsrInReg]>;
def LD_TNSR_HIGH_PTvm  : LoadInst_LD_TNSR_PR<0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask, TnsrInReg]>;
def LD_TNSR_HIGH_PTmp  : LoadInst_LD_TNSR_PR<0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred, 1>, Requires<[Partial, TnsrInReg]>;
def LD_TNSR_HIGH_PTmm  : LoadInst_LD_TNSR_PR<0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred, 1>, Requires<[Partial, LdVectMask, TnsrInReg]>;

def LD_TNSR_HIGH_Dvp   : LoadInst_LD_TNSR_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_HIGH_Dvm   : LoadInst_LD_TNSR_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;
def LD_TNSR_HIGH_Dmp   : LoadInst_LD_TNSR_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct]>;
def LD_TNSR_HIGH_Dmm   : LoadInst_LD_TNSR_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdVectMask]>;

def LD_TNSR_HIGH_PDvp  : LoadInst_LD_TNSR_PD <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_HIGH_PDvm  : LoadInst_LD_TNSR_PD <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;
def LD_TNSR_HIGH_PDmp  : LoadInst_LD_TNSR_PD <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", SPred, 1>, Requires<[Direct, Partial]>;
def LD_TNSR_HIGH_PDmm  : LoadInst_LD_TNSR_PD <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", VPred, 1>, Requires<[Direct, Partial, LdVectMask]>;

foreach type = [v256i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
  def : Pat<(type (int_tpc_ld_tnsr_high v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGH", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_ld_tnsr_high v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGHGen2T", type, pred>.Insn v5i32:$ndx, i8:$tensor, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Gen2]>;
  def : Pat<(type (int_tpc_ld_tnsr_high v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGHT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[TnsrInReg]>;
  def : Pat<(type (int_tpc_ld_tnsr_high_direct i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"LD_TNSR_HIGH_D", type, pred>.Insn i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
        Requires<[Direct]>;
  }

foreach type = [v256i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ld_tnsr_high_partial v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_HIGH_P", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial]>;
    def : Pat<(type (int_tpc_ld_tnsr_high_partial v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_HIGH_PT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, TnsrInReg]>;
    def : Pat<(type (int_tpc_ld_tnsr_high_partial_direct i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_HIGH_PD", type, pred>.Insn i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
          Requires<[Partial, Direct]>;
  }

//------ FCLASS ----------------------------------------------------------------

def LD_FCLASSvvp : LoadInst_FCLASS<0b010101, VRF, VRF, "fclass$optype $sw $dest, $src, $pred", SPred>, MultiSlotDoron1<"LD_FCLASSvvp">, Requires<[Doron1]>;
def LD_FCLASSvvm : LoadInst_FCLASS<0b010101, VRF, VRF, "fclass$optype $sw $dest, $src, $pred", VPred>, MultiSlotDoron1<"LD_FCLASSvvm">, Requires<[Doron1]>;
def LD_FCLASSvsp : LoadInst_FCLASS<0b010101, VRF, SRF, "fclass$optype $sw $dest, $src, $pred", SPred>, MultiSlotDoron1<"LD_FCLASSvsp">, Requires<[Doron1]>;
def LD_FCLASSvsm : LoadInst_FCLASS<0b010101, VRF, SRF, "fclass$optype $sw $dest, $src, $pred", VPred>, MultiSlotDoron1<"LD_FCLASSvsm">, Requires<[Doron1]>;

//------ CALC_FP_SPECIAL -------------------------------------------------------

def LD_CALC_FP_SPECIALOneArgvvp : LoadInst_CALC_FP_SPECIAL<0b010110, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", SPred>, MultiSlot<"LD_CALC_FP_SPECIALOneArgvvp">, Requires<[Doron1]>;
def LD_CALC_FP_SPECIALOneArgvvm : LoadInst_CALC_FP_SPECIAL<0b010110, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", VPred>, MultiSlot<"LD_CALC_FP_SPECIALOneArgvvm">, Requires<[Doron1]>;

//------ LD_TNSR_CNVRT ---------------------------------------------------------

def LD_TNSR_CNVRTvp : LoadInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRTvp">, Requires<[Doron1]>;
def LD_TNSR_CNVRTvm : LoadInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRTvm">, Requires<[Doron1]>;

def LD_TNSR_CNVRTTvp : LoadInst_LD_TNSR_CNVRTR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRTTvp">, Requires<[Doron1, TnsrInReg]>;
def LD_TNSR_CNVRTTvm : LoadInst_LD_TNSR_CNVRTR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRTTvm">, Requires<[Doron1, TnsrInReg]>;

def LD_TNSR_CNVRT_Pvp : LoadInst_LD_TNSR_CNVRT_P<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_Pvp">, Requires<[Doron1, Partial]>;
def LD_TNSR_CNVRT_Pvm : LoadInst_LD_TNSR_CNVRT_P<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_Pvm">, Requires<[Doron1, Partial]>;

def LD_TNSR_CNVRT_PTvp : LoadInst_LD_TNSR_CNVRT_PR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_PTvp">, Requires<[Doron1, Partial, TnsrInReg]>;
def LD_TNSR_CNVRT_PTvm : LoadInst_LD_TNSR_CNVRT_PR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_PTvm">, Requires<[Doron1, Partial, TnsrInReg]>;

def LD_TNSR_CNVRT_Dvp : LoadInst_LD_TNSR_CNVRT_D<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_Dvp">, Requires<[Doron1, Direct]>;
def LD_TNSR_CNVRT_Dvm : LoadInst_LD_TNSR_CNVRT_D<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_Dvm">, Requires<[Doron1, Direct]>;

def LD_TNSR_CNVRT_PDvp : LoadInst_LD_TNSR_CNVRT_PD<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_PDvp">, Requires<[Doron1,  Partial, Direct]>;
def LD_TNSR_CNVRT_PDvm : LoadInst_LD_TNSR_CNVRT_PD<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_PDvm">, Requires<[Doron1, Partial, Direct]>;

foreach type = [v128bf16, v128f16] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRT", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1]>;
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRTT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1, TnsrInReg]>;
                    
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt_partial v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRT_P", type, pred>.Insn v5i32:$ndx, (i8 imm:$tensor), LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1, Partial]>;
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt_partial v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRT_PT", type, pred>.Insn v5i32:$ndx, LD_TNSR_ID_REG, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1, Partial, TnsrInReg]>;
                    
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt_direct i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRT_D", type, pred>.Insn i32:$addr1, i32:$addr2, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1, Direct]>;
    def : Pat<(type (int_tpc_ld_tnsr_cnvrt_partial_direct i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"LD_TNSR_CNVRT_PD", type, pred>.Insn i32:$addr1, i32:$addr2, LD_PARTIAL_REG, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity)))>,
              Requires<[Doron1, Partial, Direct]>;
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
          (f32 (MACssp f32:$x0, f32:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
def : Pat<(f32 (int_tpc_mac f32:$x0, (f32 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
          (f32 (MACsip f32:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;

let Predicates = [BF16] in {
  def : Pat<(bf16 (int_tpc_mac bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity))),
            (bf16 (MACssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), bf16:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(bf16 (int_tpc_mac bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), bf16:$income, i1:$pred, (i1 imm:$polarity))),
            (bf16 (MACsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), bf16:$income, i1:$pred, (i1 imm:$polarity)))>;
}

let Predicates = [FP16] in {
  def : Pat<(f16 (int_tpc_mac f16:$x0, f16:$x1, (i8 imm:$dt), (i32 imm:$sw), f16:$income, i1:$pred, (i1 imm:$polarity))),
            (f16 (MACssp f16:$x0,f16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f16:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f16 (int_tpc_mac f16:$x0, (f16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f16:$income, i1:$pred, (i1 imm:$polarity))),
            (f16 (MACsip f16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f16:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// With ACC_I16.
let Predicates = [AccI16] in {
  def : Pat<(i16 (int_tpc_mac i8:$x0, i8:$x1, (i8 imm:$dt), (i32 imm:$sw), i16:$income, i1:$pred, (i1 imm:$polarity))),
            (i16 (MACssp i8:$x0, i8:$x1, (i8 imm:$dt), (i32 imm:$sw), i16:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(i16 (int_tpc_mac i8:$x0, (i8 imm:$x1), (i8 imm:$dt), (i32 imm:$sw), i16:$income, i1:$pred, (i1 imm:$polarity))),
            (i16 (MACsip i8:$x0, (CastToImm32 $x1), (i8 imm:$dt), (i32 imm:$sw), i16:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// With ACC_FP32.
let Predicates = [BF16] in {
  def : Pat<(f32 (int_tpc_mac bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// With ACC_FP32.
let Predicates = [FP16] in {
  def : Pat<(f32 (int_tpc_mac f16:$x0, f16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACssp f16:$x0, f16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac f16:$x0, (f16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACsip f16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

// With ACC_FP32.
let Predicates = [FP8] in {
  def : Pat<(f32 (int_tpc_mac f8_143:$x0, f8_143:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACssp f8_143:$x0, f8_143:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac f8_143:$x0, (f8_143 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACsip f8_143:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac f8_152:$x0, f8_152:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACssp f8_152:$x0, f8_152:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mac f8_152:$x0, (f8_152 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MACsip f8_152:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}


//------ MUL -------------------------------------------------------------------

def MULssp  : SpuInst_Binary      <0b000001, SRF, SRF,    SRF,    "mul$optype $sw $dest, $op1, $op2, $pred">;
def MULsip  : SpuInst_BinaryI     <0b000001, SRF, SRF,    i32imm, "mul$optype $sw $dest, $op1, $op2, $pred">;
def MULsIp  : SpuInst_BinaryMask  <0b000001, IRF, SRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;
def MULiIp  : SpuInst_BinaryMaskIR<0b000001, IRF, i32imm, IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;
def MULIIp  : SpuInst_BinaryMask  <0b000001, IRF, IRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">;
def MULwsIp : SpuInst_BinaryMReg  <0b000001, IRF, SRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MULwiIp : SpuInst_BinaryMRegIR<0b000001, IRF, i32imm, IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MULwIIp : SpuInst_BinaryMReg  <0b000001, IRF, IRF,    IRF,    "mul$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinarySignedScalar<mul,  MULssp, MULsip>;

def : Pat<(fmul f32:$op1, f32:$op2),         (MULssp $op1, $op2,             OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fmul f32:$op1, (f32 fpimm:$op2)), (MULsip $op1, (FPToImm32 $op2), OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
let Predicates = [BF16] in {
  def : Pat<(fmul bf16:$op1, bf16:$op2),         (MULssp $op1, $op2,             OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def : Pat<(fmul bf16:$op1, (bf16 fpimm:$op2)), (MULsip $op1, (FPToImm32 $op2), OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}
let Predicates = [FP16] in {
  def : Pat<(fmul f16:$op1, f16:$op2),         (MULssp $op1, $op2,             OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def : Pat<(fmul f16:$op1, (f16 fpimm:$op2)), (MULsip $op1, (FPToImm32 $op2), OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

defm : BinaryIndexOp1<mul, MULIIp, MULsIp, MULiIp>;

// ACC_FP32
def : Pat<(f32 (fmul (fpextend f8_143:$x0), (fpextend f8_143:$x1))),
          (f32 (MULssp f8_143:$x0, f8_143:$x1, OpType.FP8_143, (AddDnormSupport (i32 4)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(f32 (fmul (fpextend f8_152:$x0), (fpextend f8_152:$x1))),
          (f32 (MULssp f8_152:$x0, f8_152:$x1, OpType.FP8_152, (AddDnormSupport (i32 4)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

foreach type = [f32, bf16, f16] in {
  def : Pat<(type (int_tpc_mul type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULssp type:$x0, type:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), type:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_mul type:$x0, (type ImmNode<type>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULsip type:$x0, (CastOp<type>.Cast $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), type:$income, i1:$pred, (i1 imm:$polarity)))>;
}
foreach type = [i32] in {
  def : Pat<(type (int_tpc_mul type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULssp type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_mul type:$x0, (type ImmNode<type>.Node:$x1), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (MULsip type:$x0, (CastOp<type>.Cast $x1), (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity)))>;
}
let Predicates = [BF16] in {
  def : Pat<(f32 (int_tpc_mul bf16:$x0, bf16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULssp bf16:$x0, bf16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mul bf16:$x0, (bf16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULsip bf16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport  (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

let Predicates = [FP16] in {
  def : Pat<(f32 (int_tpc_mul f16:$x0, f16:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULssp f16:$x0, f16:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mul f16:$x0, (f16 fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULsip f16:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}

let Predicates = [FP8] in {
  foreach type = [f8_143, f8_152] in {
    def : Pat<(f32 (int_tpc_mul type:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
              (f32 (MULssp type:$x0, type:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_mul type:$x0, (type fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (MULsip type:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  }
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
def ADDwsIp : SpuInst_BinaryMReg  <0b000010, IRF, SRF,    IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ADDwiIp : SpuInst_BinaryMRegIR<0b000010, IRF, i32imm, IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ADDwIIp : SpuInst_BinaryMReg  <0b000010, IRF, IRF,    IRF,    "add$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
}

defm : BinarySignedScalar<add,  ADDssp, ADDsip>;
defm : BinaryFloatArithScalar<fadd, ADDssp, ADDsip>;
def : Pat<(fadd f32:$x0, (fpextend f8_143:$x1)),
          (ADDssp f32:$x0, f8_143:$x1, (i8 OpType.FP8_143), (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fadd f32:$x0, (fpextend f8_152:$x1)),
          (ADDssp f32:$x0, f8_152:$x1, (i8 OpType.FP8_152), (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
defm : BinaryIndexOp1<add, ADDIIp, ADDsIp, ADDiIp>;

foreach type = [f32, bf16, f16] in
  defm : BinaryScalarIntrinsicsDnorm<int_tpc_add, "ADD", type>;
foreach type = [i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_add, "ADD", type>;
foreach type = [f8_143, f8_152] in {
  def : Pat<(f32 (int_tpc_add f32:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (ADDssp f32:$x0, type:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_add f32:$x0, (type fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (ADDsip f32:$x0, (FPToImm32 type:$x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}
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
def SUBwsIp : SpuInst_BinaryMReg  <0b000011, IRF, SRF,    IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SUBwiIp : SpuInst_BinaryMRegIR<0b000011, IRF, i32imm, IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SUBwIIp : SpuInst_BinaryMReg  <0b000011, IRF, IRF,    IRF,    "sub$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
}

defm : BinarySignedScalar<sub,  SUBssp, SUBsip>;
defm : BinaryFloatArithScalar<fsub, SUBssp, SUBsip>;
def : Pat<(fsub f32:$x0, (fpextend f8_143:$x1)),
          (SUBssp f32:$x0, f8_143:$x1, (i8 OpType.FP8_143), (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fsub f32:$x0, (fpextend f8_152:$x1)),
          (SUBssp f32:$x0, f8_152:$x1, (i8 OpType.FP8_152), (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
defm : BinaryIndexOp1<sub, SUBIIp, SUBsIp, SUBiIp>;

foreach type = [f32, bf16, f16] in
  defm : BinaryScalarIntrinsicsDnorm<int_tpc_sub, "SUB", type>;
foreach type = [i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_sub, "SUB", type>;
foreach type = [f8_143, f8_152] in {
  def : Pat<(f32 (int_tpc_sub f32:$x0, type:$x1, (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (SUBssp f32:$x0, type:$x1, (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
  def : Pat<(f32 (int_tpc_sub f32:$x0, (type fpimm:$x1), (i8 imm:$dt), (i32 imm:$sw), f32:$income, i1:$pred, (i1 imm:$polarity))),
            (f32 (SUBsip f32:$x0, (FPToImm32 $x1), (i8 imm:$dt), (AddDnormSupport (i32 imm:$sw)), f32:$income, i1:$pred, (i1 imm:$polarity)))>;
}
defm : BinaryIndexIntrinsicsCompat<int_tpc_sub, "SUB">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_sub_mask, "SUB">;

//------ MAX -------------------------------------------------------------------

def MAXssp  : SpuInst_Binary      <0b000101, SRF, SRF,    SRF,    "max$optype $sw $dest, $op1, $op2, $pred">;
def MAXsip  : SpuInst_BinaryI     <0b000101, SRF, SRF,    i32imm, "max$optype $sw $dest, $op1, $op2, $pred">;
def MAXIIp  : SpuInst_BinaryMask  <0b000101, IRF, IRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;
def MAXiIp  : SpuInst_BinaryMaskIR<0b000101, IRF, i32imm, IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;
def MAXsIp  : SpuInst_BinaryMask  <0b000101, IRF, SRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">;
def MAXwIIp : SpuInst_BinaryMReg  <0b000101, IRF, IRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MAXwiIp : SpuInst_BinaryMRegIR<0b000101, IRF, i32imm, IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MAXwsIp : SpuInst_BinaryMReg  <0b000101, IRF, SRF,    IRF,    "max$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
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
def MINwIIp : SpuInst_BinaryMReg  <0b000110, IRF, IRF,    IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MINwiIp : SpuInst_BinaryMRegIR<0b000110, IRF, i32imm, IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def MINwsIp : SpuInst_BinaryMReg  <0b000110, IRF, SRF,    IRF,    "min$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_min, "MIN", type>;
defm : BinaryIndexIntrinsicsCompat<int_tpc_min, "MIN">;
defm : BinaryIndexIntrinsicsOp1<int_tpc_min_mask, "MIN">;
defm : BinaryIndexIntrinsicsOp1Commutative<int_tpc_min_mask, "MIN">;


//------ ABS -------------------------------------------------------------------

def ABSssp  : SpuInst_Unary    <0b000111, SRF, SRF, "abs$optype $sw $dest, $op, $pred">;
def ABSIsp  : SpuInst_UnaryMask<0b000111, IRF, SRF, "abs$optype $sw $mask $dest, $op, $pred">;
def ABSIIp  : SpuInst_UnaryMask<0b000111, IRF, IRF, "abs$optype $sw $mask $dest, $op, $pred">;
def ABSwIsp : SpuInst_UnaryMReg<0b000111, IRF, SRF, "abs$optype $sw $mask $dest, $op, $pred">, Requires<[DimMaskR]>;
def ABSwIIp : SpuInst_UnaryMReg<0b000111, IRF, IRF, "abs$optype $sw $mask $dest, $op, $pred">, Requires<[DimMaskR]>;

defm : UnaryIntegerScalar<abs,  ABSssp>;
defm : UnaryIndexIntrinsics<int_tpc_abs_mask, "ABS">;

foreach type = [i32, i16, i8] in
  def : Pat<(type (int_tpc_abs type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ABSssp SRF:$src, (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

//------ CMP_HIGH32 ------------------------------------------------------------

multiclass Cmp64Instrinsic<SDNode Intrin, string IName> {
  def : Pat<(i1 (Intrin v2i32:$src1, v2i32:$src2, (i8 imm:$dt), (i32 imm:$sw), i1:$income, i1:$pred, (i1 imm:$polarity))),
          (i1 (!cast<Instruction>(!strconcat(IName, "_HIGH32"))
            (i32 (EXTRACT_SUBREG v2i32:$src1, sub_s1)), (i32 (EXTRACT_SUBREG v2i32:$src2, sub_s1)), (i8 imm:$dt), (i32 imm:$sw),
            (i1 (!cast<Instruction>(!strconcat(IName, "ssp")) (i32 (EXTRACT_SUBREG v2i32:$src1, sub_s0)), (i32 (EXTRACT_SUBREG v2i32:$src2, sub_s0)), OpType.UINT32, (i32 imm:$sw), i1:$income,  i1:$pred, (i1 imm:$polarity))),
            i1:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
}

//------ CMP_EQ ----------------------------------------------------------------

def CMP_EQssp  : SpuInst_Binary     <0b001001, SPRF, SRF,    SRF, "cmp_eq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_EQsip  : SpuInst_BinaryI    <0b001001, SPRF, SRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_EQIIp  : SpuInst_BinaryMask <0b001001, SPRF, IRF,    IRF, "cmp_eq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_EQIip  : SpuInst_BinaryMaskI<0b001001, SPRF, IRF, i32imm, "cmp_eq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_EQwIIp : SpuInst_BinaryMReg <0b001001, SPRF, IRF,    IRF, "cmp_eq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_EQwIip : SpuInst_BinaryMRegI<0b001001, SPRF, IRF, i32imm, "cmp_eq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_EQ_HIGH32 : CMPInstHigh32 <0b001001, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<seteq,  CMP_EQssp, CMP_EQsip>;
defm : BinaryFloatScalar<setoeq, CMP_EQssp, CMP_EQsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_eq, "CMP_EQ", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_eq_mask, "CMP_EQ", i1>;
  defm : CmpIndexOp<seteq, CMP_EQIip>;
}

foreach T = [i32, i16, i8] in {
  def : Pat<(seteq (and T:$x0, T:$x1), 0),
            (ConstructInsn<"CMP_EQ", T, T, i1>.Insn T:$x0, T:$x1, OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen2Plus]>;
  def : Pat<(seteq (and T:$x0, (T ImmNode<T>.Node:$x1)), 0),
            (ConstructInsn<"CMP_EQ", T, iAny, i1>.Insn T:$x0, (CastToImm32 $x1), OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen2Plus]>;
}

def : Pat<(i1 (seteq (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_EQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETEQ)),
          (i1 (CMP_EQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_eq, "CMP_EQ">;

//------ CMP_NEQ ---------------------------------------------------------------

def CMP_NEQssp  : SpuInst_Binary     <0b001010, SPRF, SRF,    SRF, "cmp_neq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_NEQsip  : SpuInst_BinaryI    <0b001010, SPRF, SRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_NEQIIp  : SpuInst_BinaryMask <0b001010, SPRF, IRF,    IRF, "cmp_neq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_NEQIip  : SpuInst_BinaryMaskI<0b001010, SPRF, IRF, i32imm, "cmp_neq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_NEQwIIp : SpuInst_BinaryMReg <0b001010, SPRF, IRF,    IRF, "cmp_neq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_NEQwIip : SpuInst_BinaryMRegI<0b001010, SPRF, IRF, i32imm, "cmp_neq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_NEQ_HIGH32 : CMPInstHigh32 <0b001010, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<setne,  CMP_NEQssp, CMP_NEQsip>;
defm : BinaryFloatScalar<setone, CMP_NEQssp, CMP_NEQsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_neq, "CMP_NEQ", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_neq_mask, "CMP_NEQ", i1>;
  defm : CmpIndexOp<setne, CMP_NEQIip>;
}

def : Pat<(i1 (setne (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_NEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETNE)),
          (i1 (CMP_NEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_neq, "CMP_NEQ">;

//------ CMP_LESS --------------------------------------------------------------

def CMP_LESSssp  : SpuInst_Binary     <0b001011, SPRF, SRF,    SRF, "cmp_less$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LESSsip  : SpuInst_BinaryI    <0b001011, SPRF, SRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LESSIIp  : SpuInst_BinaryMask <0b001011, SPRF, IRF,    IRF, "cmp_less$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LESSIip  : SpuInst_BinaryMaskI<0b001011, SPRF, IRF, i32imm, "cmp_less$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LESSwIIp : SpuInst_BinaryMReg <0b001011, SPRF, IRF,    IRF, "cmp_less$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LESSwIip : SpuInst_BinaryMRegI<0b001011, SPRF, IRF, i32imm, "cmp_less$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_LESS_HIGH32 : CMPInstHigh32 <0b001011, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<setlt,  CMP_LESSssp, CMP_LESSsip>;
defm : BinaryUnsignedScalar<setult, CMP_LESSssp, CMP_LESSsip>;
defm : BinaryFloatScalar<setolt, CMP_LESSssp, CMP_LESSsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_less, "CMP_LESS", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_less_mask, "CMP_LESS", i1>;
  defm : CmpIndexOp<setlt, CMP_LESSIip>;
}

def : Pat<(i1 (setlt (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_LESSIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETLT)),
          (i1 (CMP_LESSIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_less, "CMP_LESS">;

//------ CMP_LEQ ---------------------------------------------------------------

def CMP_LEQssp  : SpuInst_Binary     <0b001100, SPRF, SRF,    SRF, "cmp_leq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LEQsip  : SpuInst_BinaryI    <0b001100, SPRF, SRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_LEQIIp  : SpuInst_BinaryMask <0b001100, SPRF, IRF,    IRF, "cmp_leq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LEQIip  : SpuInst_BinaryMaskI<0b001100, SPRF, IRF, i32imm, "cmp_leq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LEQwIIp : SpuInst_BinaryMReg <0b001100, SPRF, IRF,    IRF, "cmp_leq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_LEQwIip : SpuInst_BinaryMRegI<0b001100, SPRF, IRF, i32imm, "cmp_leq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_LEQ_HIGH32 : CMPInstHigh32 <0b001100, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<setle,  CMP_LEQssp, CMP_LEQsip>;
defm : BinaryUnsignedScalar<setule, CMP_LEQssp, CMP_LEQsip>;
defm : BinaryFloatScalar<setole, CMP_LEQssp, CMP_LEQsip>;

defm : BinaryFloatScalar<setule, CMP_LEQssp, CMP_LEQsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_leq, "CMP_LEQ", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_leq_mask, "CMP_LEQ", i1>;
  defm : CmpIndexOp<setle, CMP_LEQIip>;
}

def : Pat<(i1 (setle (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_LEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETLE)),
          (i1 (CMP_LEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_leq, "CMP_LEQ">;

//------ CMP_GRT ---------------------------------------------------------------

def CMP_GRTssp  : SpuInst_Binary     <0b001101, SPRF, SRF,    SRF, "cmp_grt$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GRTsip  : SpuInst_BinaryI    <0b001101, SPRF, SRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GRTIIp  : SpuInst_BinaryMask <0b001101, SPRF, IRF,    IRF, "cmp_grt$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GRTIip  : SpuInst_BinaryMaskI<0b001101, SPRF, IRF, i32imm, "cmp_grt$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GRTwIIp : SpuInst_BinaryMReg <0b001101, SPRF, IRF,    IRF, "cmp_grt$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GRTwIip : SpuInst_BinaryMRegI<0b001101, SPRF, IRF, i32imm, "cmp_grt$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_GRT_HIGH32 : CMPInstHigh32 <0b001101, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<setgt,  CMP_GRTssp, CMP_GRTsip>;
defm : BinaryUnsignedScalar<setugt, CMP_GRTssp, CMP_GRTsip>;
defm : BinaryFloatScalar<setogt, CMP_GRTssp, CMP_GRTsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_grt, "CMP_GRT", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_grt_mask, "CMP_GRT", i1>;
  defm : CmpIndexOp<setgt, CMP_GRTIip>;
}

def : Pat<(i1 (setgt (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_GRTIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETGT)),
          (i1 (CMP_GRTIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_grt, "CMP_GRT">;

//------ CMP_GEQ ---------------------------------------------------------------

def CMP_GEQssp  : SpuInst_Binary     <0b001110, SPRF, SRF,    SRF, "cmp_geq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GEQsip  : SpuInst_BinaryI    <0b001110, SPRF, SRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred">;
def CMP_GEQIIp  : SpuInst_BinaryMask <0b001110, SPRF, IRF,    IRF, "cmp_geq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GEQIip  : SpuInst_BinaryMaskI<0b001110, SPRF, IRF, i32imm, "cmp_geq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GEQwIIp : SpuInst_BinaryMReg <0b001110, SPRF, IRF,    IRF, "cmp_geq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;
def CMP_GEQwIip : SpuInst_BinaryMRegI<0b001110, SPRF, IRF, i32imm, "cmp_geq$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[Gen3Plus]>;

def CMP_GEQ_HIGH32 : CMPInstHigh32 <0b001110, SPRF, SRF, SRF, "cmp_eq$optype high32 $sw $dest, $op1, $op2, $pred">, Requires<[Doron1]>;

defm : BinarySignedScalar<setge,  CMP_GEQssp, CMP_GEQsip>;
defm : BinaryUnsignedScalar<setuge, CMP_GEQssp, CMP_GEQsip>;
defm : BinaryFloatScalar<setoge, CMP_GEQssp, CMP_GEQsip>;
defm : BinaryFloatScalar<setuge, CMP_GEQssp, CMP_GEQsip>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
  defm : BinaryScalarIntrinsics<int_tpc_cmp_geq, "CMP_GEQ", type, i1>;
let Predicates = [Gen3Plus] in {
  defm : IndexCmpIntrinsic<int_tpc_cmp_geq_mask, "CMP_GEQ", i1>;
  defm : CmpIndexOp<setge, CMP_GEQIip>;
}

def : Pat<(i1 (setge (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim))),
          (i1 (CMP_GEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;
def : Pat<(i1 (setcc (extractelt v5i32:$vec1, imm:$dim), (extractelt v5i32:$vec2, imm:$dim), SETGE)),
          (i1 (CMP_GEQIIp v5i32:$vec1, v5i32:$vec2, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>, Requires<[Gen3Plus]>;

defm : Cmp64Instrinsic<int_tpc_cmp_geq, "CMP_GEQ">;

//------ OR --------------------------------------------------------------------

def ORssp  : SpuInst_Binary      <0b001111, SRF, SRF,    SRF,    "or$optype $sw $dest, $op1, $op2, $pred">;
def ORsip  : SpuInst_BinaryI     <0b001111, SRF, SRF,    i32imm, "or$optype $sw $dest, $op1, $op2, $pred">;
def ORppp  : SpuInst_Binary      <0b001111, SPRF, SPRF,  SPRF,   "or$optype $sw $dest, $op1, $op2, $pred">;
def ORIIp  : SpuInst_BinaryMask  <0b001111, IRF, IRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ORiIp  : SpuInst_BinaryMaskIR<0b001111, IRF, i32imm, IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ORsIp  : SpuInst_BinaryMask  <0b001111, IRF, SRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">;
def ORwIIp : SpuInst_BinaryMReg  <0b001111, IRF, IRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ORwiIp : SpuInst_BinaryMRegIR<0b001111, IRF, i32imm, IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ORwsIp : SpuInst_BinaryMReg  <0b001111, IRF, SRF,    IRF,    "or$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinarySignedScalar<or, ORssp, ORsip>;
defm : BinaryFloatScalar<fpor, ORssp, ORsip>;
defm : BinaryIndexOp1<or, ORIIp, ORsIp, ORiIp>;
def : Pat<(or i1:$op1, i1:$op2), (ORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
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
def ANDwIIp : SpuInst_BinaryMReg  <0b010000, IRF, IRF,    IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ANDwiIp : SpuInst_BinaryMRegIR<0b010000, IRF, i32imm, IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def ANDwsIp : SpuInst_BinaryMReg  <0b010000, IRF, SRF,    IRF,    "and$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinarySignedScalar<and, ANDssp, ANDsip>;
defm : BinaryFloatScalar<fpand, ANDssp, ANDsip>;
defm : BinaryIndexOp1<and, ANDIIp, ANDsIp, ANDiIp>;
def : Pat<(and i1:$op1, i1:$op2), (ANDppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
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

def : Pat<(f16 (int_tpc_abs f16:$src, (i8 imm:$dt), (i32 imm:$sw), f16:$income, i1:$pred, (i1 imm:$polarity))),
          (f16 (ANDsip SRF:$src, (i32 0x7fff), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

def : Pat<(f8_143 (int_tpc_abs f8_143:$src, (i8 imm:$dt), (i32 imm:$sw), f8_143:$income, i1:$pred, (i1 imm:$polarity))),
          (f8_143 (ANDsip SRF:$src, (i32 0x7f), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

def : Pat<(f8_152 (int_tpc_abs f8_152:$src, (i8 imm:$dt), (i32 imm:$sw), f8_152:$income, i1:$pred, (i1 imm:$polarity))),
          (f8_152 (ANDsip SRF:$src, (i32 0x7f), (i8 imm:$dt), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

// llvm.fabs
def : Pat<(fabs f32:$src),
          (f32 (ANDsip $src, (i32 0x7fffffff), OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(fabs bf16:$src),
          (bf16 (ANDsip $src, (i32 0x7fff), OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(fabs f16:$src),
          (f16 (ANDsip $src, (i32 0x7fff), OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(fabs f8_143:$src),
          (f8_143 (ANDsip $src, (i32 0x7f), OpType.FP8_143, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(fabs f8_152:$src),
          (f8_152 (ANDsip $src, (i32 0x7f), OpType.FP8_152, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ XOR -------------------------------------------------------------------

def XORssp  : SpuInst_Binary      <0b010001, SRF, SRF,    SRF,    "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORsip  : SpuInst_BinaryI     <0b010001, SRF, SRF,    i32imm, "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORppp  : SpuInst_Binary      <0b010001, SPRF, SPRF,  SPRF,   "xor$optype $sw $dest, $op1, $op2, $pred">;
def XORIIp  : SpuInst_BinaryMask  <0b010001, IRF, IRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;
def XORiIp  : SpuInst_BinaryMaskIR<0b010001, IRF, i32imm, IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;
def XORsIp  : SpuInst_BinaryMask  <0b010001, IRF, SRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">;
def XORwIIp : SpuInst_BinaryMReg  <0b010001, IRF, IRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def XORwiIp : SpuInst_BinaryMRegIR<0b010001, IRF, i32imm, IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def XORwsIp : SpuInst_BinaryMReg  <0b010001, IRF, SRF,    IRF,    "xor$optype $sw $mask  $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinarySignedScalar<xor, XORssp, XORsip>;
defm : BinaryFloatScalar<fpxor, XORssp, XORsip>;
defm : BinaryIndexOp1<xor, XORIIp, XORsIp, XORiIp>;

def : Pat<(xor i1:$op1, i1:$op2),      (XORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in
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
def NOTwIsp : SpuInst_UnaryMReg<0b010010, IRF,  SRF,  "not$optype $sw $mask $dest, $op, $pred">, Requires<[DimMaskR]>;
def NOTwIIp : SpuInst_UnaryMReg<0b010010, IRF,  IRF,  "not$optype $sw $mask $dest, $op, $pred">, Requires<[DimMaskR]>;

defm : UnaryIntegerScalar<inot,  NOTssp>;
defm : UnaryFloatScalar<fpnot, NOTssp>;
defm : UnaryIndexIntrinsics<int_tpc_not_mask, "NOT">;

def : Pat<(inot i1:$op), (NOTppp $op, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8, i1] in
  def : Pat<(type (int_tpc_not type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (ConstructInsn<"NOT", type, type, i1>.Insn
                   type:$src, (i8 imm:$dt), (i32 imm:$sw), type:$income, SPRF:$pred, (i1 imm:$polarity)))>;


//------ SHR -------------------------------------------------------------------

def SHRssp  : SpuInst_Binary     <0b010011, SRF, SRF,    SRF,    "shr$optype $sw $dest, $op1, $op2, $pred">;
def SHRsip  : SpuInst_BinaryI    <0b010011, SRF, SRF,    i32imm, "shr$optype $sw $dest, $op1, $op2, $pred">;
def SHRIsp  : SpuInst_BinaryMask <0b010011, IRF, IRF,    SRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHRIip  : SpuInst_BinaryMaskI<0b010011, IRF, IRF,    i32imm, "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHRIIp  : SpuInst_BinaryMask <0b010011, IRF, IRF,    IRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;
def SHRwIsp : SpuInst_BinaryMReg <0b010011, IRF, IRF,    SRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SHRwIip : SpuInst_BinaryMRegI<0b010011, IRF, IRF,    i32imm, "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SHRwIIp : SpuInst_BinaryMReg <0b010011, IRF, IRF,    IRF,    "shr$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinaryShiftScalar<srl, SHRssp, SHRsip>;
defm : BinaryIndexScalar<srl, SHRIsp, SHRIip>;
def  : Pat<(srl v5i32:$op1, v5i32:$op2),
           (SHRIIp $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
defm : BinaryIndexShiftIntrinsics<int_tpc_shr_mask, "SHR">;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
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
def SHLwIsp : SpuInst_BinaryMReg <0b010100, IRF, IRF,    SRF,    "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SHLwIip : SpuInst_BinaryMRegI<0b010100, IRF, IRF,    i32imm, "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;
def SHLwIIp : SpuInst_BinaryMReg <0b010100, IRF, IRF,    IRF,    "shl$optype $sw $mask $dest, $op1, $op2, $pred">, Requires<[DimMaskR]>;

defm : BinaryShiftScalar<shl, SHLssp, SHLsip>;
defm : BinaryIndexScalar<shl, SHLIsp, SHLIip>;
def  : Pat<(shl v5i32:$op1, v5i32:$op2),
           (SHLIIp $op1, $op2, (i32 31), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
defm : BinaryIndexShiftIntrinsics<int_tpc_shl_mask, "SHL">;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
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

foreach fromType = [f32, bf16, f16, f8_143, f8_152, i8, i16, i32] in
  foreach toType = [f32, bf16, f16, f8_143, f8_152, i8, i16, i32] in
    def : Pat<(toType (int_tpc_convert fromType:$src, (i8 imm:$optype), (i32 imm:$sw), toType:$income,  i1:$pred, (i1 imm:$polarity))),
              (toType (CONVERTssp SRF:$src, (i8 imm:$optype), (CastToImm32 $sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


// INT64[src1,src2] => INT32
def CONVERTINT64ssp : SpuInst_ConvertINT64<0b010110, "convert.i64 $sw $dst, $op1, $op2, $pred">, Requires<[Gen4Plus]>;

def : Pat<(i32 (int_tpc_convert_int64 i32:$src1, i32:$src2, (i32 imm:$sw), i32:$income,  i1:$pred,   (i1 imm:$polarity))),
              (i32 (CONVERTINT64ssp   SRF:$src1, SRF:$src2, (CastToImm32 $sw), i32:$income,  SPRF:$pred, (i1 imm:$polarity)))>;


def : Pat<(i32 (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i8  (fp_to_sint f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT8, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(i32 (fp_to_sint bf16:$src)), (CONVERTssp (CONVERTssp $src, OpType.BF16, SwitchVal.TO_FP32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i32 (fp_to_sint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (fp_to_sint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(i8  (fp_to_sint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;

def : Pat<(i32 (fp_to_sint f16:$src)),  (CONVERTssp (CONVERTssp $src, OpType.FP16, SwitchVal.TO_FP32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3]>;
def : Pat<(i32 (fp_to_sint f16:$src)),  (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (fp_to_sint f16:$src)),  (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(i8  (fp_to_sint f16:$src)),  (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_INT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;

def : Pat<(i32 (fp_to_sint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i16 (fp_to_sint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i8  (fp_to_sint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_INT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(i32 (fp_to_sint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i16 (fp_to_sint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i8  (fp_to_sint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_INT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(i32 (fp_to_uint f32:$src)), (CONVERTssp (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                   OpType.INT32, SwitchVal.TO_UINT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i32 (fp_to_uint f32:$src)), (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_UINT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (fp_to_uint f32:$src)), (CONVERTssp
                                           (CONVERTssp
                                               (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                               OpType.INT32, SwitchVal.TO_INT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                           OpType.INT16, SwitchVal.TO_UINT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i16 (fp_to_uint f32:$src)), (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i8  (fp_to_uint f32:$src)), (CONVERTssp (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                   OpType.INT32, SwitchVal.TO_UINT8, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i8  (fp_to_uint f32:$src)), (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_UINT8, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;

def : Pat<(i32 (fp_to_uint bf16:$src)), (CONVERTssp
                                            (CONVERTssp
                                                (CONVERTssp $src, OpType.BF16, SwitchVal.TO_FP32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                            OpType.INT32, SwitchVal.TO_UINT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i32 (fp_to_uint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_UINT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (fp_to_uint bf16:$src)), (CONVERTssp (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.INT16, SwitchVal.TO_UINT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i16 (fp_to_uint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i8  (fp_to_uint bf16:$src)), (CONVERTssp (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.INT16, SwitchVal.TO_UINT8, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i8  (fp_to_uint bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_UINT8, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;

def : Pat<(i32 (fp_to_uint f16:$src)), (CONVERTssp
                                           (CONVERTssp
                                               (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                               OpType.FP32, !or(SwitchVal.TO_INT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                           OpType.INT32, SwitchVal.TO_UINT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i32 (fp_to_uint f16:$src)), (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_UINT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (fp_to_uint f16:$src)), (CONVERTssp (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                   OpType.INT16, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i16 (fp_to_uint f16:$src)), (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(i8  (fp_to_uint f16:$src)), (CONVERTssp (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                   OpType.INT16, !or(SwitchVal.TO_UINT8, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(i8  (fp_to_uint f16:$src)), (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_UINT8, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;

def : Pat<(i32 (fp_to_uint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_UINT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i16 (fp_to_uint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i8  (fp_to_uint f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_UINT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(i32 (fp_to_uint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_UINT32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i16 (fp_to_uint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_UINT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(i8  (fp_to_uint f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_UINT8,  SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(f32  (fpextend bf16:$src)),   (CONVERTssp $src, OpType.BF16,    !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(f32  (fpextend f16:$src)),    (CONVERTssp $src, OpType.FP16,    !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f32  (fpextend f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f32  (fpextend f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(bf16 (fpextend f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(bf16 (fpextend f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f16  (fpextend f8_143:$src)), (CONVERTssp $src, OpType.FP8_143, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f16  (fpextend f8_152:$src)), (CONVERTssp $src, OpType.FP8_152, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(bf16   (fpround f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(f16    (fpround f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_FP16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (fpround f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (fpround f32:$src)),  (CONVERTssp $src, OpType.FP32, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_143 (fpround f16:$src)),  (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (fpround f16:$src)),  (CONVERTssp $src, OpType.FP16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_143 (fpround bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (fpround bf16:$src)), (CONVERTssp $src, OpType.BF16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(f32    (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(f16    (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (sint_to_fp i32:$src)), (CONVERTssp $src, OpType.INT32, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f32    (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(f16    (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (sint_to_fp i16:$src)), (CONVERTssp $src, OpType.INT16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f32    (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(bf16   (sint_to_fp i8:$src)),  (CONVERTssp (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_INT16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                      OpType.INT16,  !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen2]>;
def : Pat<(bf16   (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3Plus]>;
def : Pat<(f16    (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (sint_to_fp i8:$src)),  (CONVERTssp $src, OpType.INT8,  !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

// There are no conversions from UINT32 to any other types on all architectures
// prior to Gen4, so define for them a kludge that works only for lower half of
// unsigned type value range. Full-fledged conversions are available only for Gen4+.
def : Pat<(f32 (uint_to_fp i32:$src)),  (CONVERTssp (ANDsip i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(bf16 (uint_to_fp i32:$src)), (CONVERTssp (ANDsip i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16, PriorGen4]>;
def : Pat<(f16 (uint_to_fp i32:$src)),  (CONVERTssp (ANDsip i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                    OpType.INT32, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16, PriorGen4]>;
def : Pat<(f32    (uint_to_fp i32:$src)), (CONVERTssp $src, OpType.UINT32, !or(SwitchVal.TO_FP32,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(bf16   (uint_to_fp i32:$src)), (CONVERTssp $src, OpType.UINT32, !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(f16    (uint_to_fp i32:$src)), (CONVERTssp $src, OpType.UINT32, !or(SwitchVal.TO_FP16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(f8_143 (uint_to_fp i32:$src)), (CONVERTssp $src, OpType.UINT32, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (uint_to_fp i32:$src)), (CONVERTssp $src, OpType.UINT32, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
// Short unsigned integer types may be promoted to longer signed types.
def : Pat<(f32    (uint_to_fp i16:$src)), (CONVERTssp (ANDsip SRF:$src, (i32 0x0ffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen4]>;
def : Pat<(f32    (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_FP32,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
def : Pat<(bf16   (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[BF16]>;
def : Pat<(f16    (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_FP16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (uint_to_fp i16:$src)), (CONVERTssp $src, OpType.UINT16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(f32    (uint_to_fp i8:$src)),  (CONVERTssp (ANDsip SRF:$src, (i32 0x0ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen3]>;
def : Pat<(f32    (uint_to_fp i8:$src)),  (CONVERTssp $src, OpType.UINT8,  !or(SwitchVal.TO_FP32,   SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3Plus]>;
def : Pat<(bf16   (uint_to_fp i8:$src)),  (CONVERTssp (ANDsip SRF:$src, (i32 0x0ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                                      OpType.INT32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[PriorGen3]>;
def : Pat<(bf16   (uint_to_fp i8:$src)),  (CONVERTssp $src, OpType.UINT8,  !or(SwitchVal.TO_BF16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3Plus]>;
def : Pat<(f16    (uint_to_fp i8:$src)),  (CONVERTssp $src, OpType.UINT8,  !or(SwitchVal.TO_FP16,    SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP16]>;
def : Pat<(f8_143 (uint_to_fp i8:$src)),  (CONVERTssp $src, OpType.UINT8,  !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;
def : Pat<(f8_152 (uint_to_fp i8:$src)),  (CONVERTssp $src, OpType.UINT8,  !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[FP8]>;

def : Pat<(i32 (sext i16:$src)), (CONVERTssp $src, OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (sext i8:$src)),  (CONVERTssp $src, OpType.INT8,  SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (sext i8:$src)),  (CONVERTssp $src, OpType.INT8,  SwitchVal.TO_INT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;


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


//------ CONVERT_INT8 ----------------------------------------------------------

def CONVERT_INT8ssp  : SpuInstConvertIntGen3<0b110101, "convert_int8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[I4CVT]>;
def CONVERT_INT8sip  : SpuInstConvertIntGen3<0b110101, "convert_int8 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[I4CVT]>;

def : Pat<(i8 (int_tpc_convert_int i8:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT8ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[I4CVT]>;
def : Pat<(i8 (int_tpc_convert_int i8:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_INT8sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[I4CVT]>;


//------ CONVERT_UINT8 ---------------------------------------------------------

def CONVERT_UINT8ssp : SpuInstConvertIntGen3<0b110110, "convert_uint8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[I4CVT]>;
def CONVERT_UINT8sip : SpuInstConvertIntGen3<0b110110, "convert_uint8 $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[I4CVT]>;

def : Pat<(i8 (int_tpc_convert_uint i8:$src1, i32:$src2, (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT8ssp SRF:$src1, SRF:$src2, (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[I4CVT]>;
def : Pat<(i8 (int_tpc_convert_uint i8:$src1, (i32 imm:$src2), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (i8 (CONVERT_UINT8sip SRF:$src1, (CastToImm32 $src2), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[I4CVT]>;

//------ CONVERT_FP_FLEX -------------------------------------------------------

def CONVERT_FP_FLEXsssp : SpuInst_CONVERT_FP_FLEX<0b111000, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXssip : SpuInst_CONVERT_FP_FLEX<0b111000, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", 1 /*hasImm*/>, Requires<[ConvertFpFlex]>;

multiclass SpuConvertFpFlex<ValueType ToType, ValueType FromType> {
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1, i16:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, i1:$pred, (i1 imm:$polarity))),
            (ToType (CONVERT_FP_FLEXsssp FromType:$src1, i16:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1, (i16 imm:$src2), (i8 imm:$optype), (i32 imm:$sw), ToType:$income, i1:$pred, (i1 imm:$polarity))),
            (ToType (CONVERT_FP_FLEXssip FromType:$src1, (CastToImm32 $src2), (i8 imm:$optype), (i32 imm:$sw), ToType:$income, i1:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
}

defm : SpuConvertFpFlex<f32, f16>;
defm : SpuConvertFpFlex<f16, f32>;

//------ FCLASS ----------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def FCLASSsp : SpuInst_UnarySw<0b011110, SRF, SRF, "fclass$optype $sw $dest, $op, $pred">;

  foreach type = [f32, bf16, f8_143, f8_152] in {
    def : Pat<(type (int_tpc_fclass type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, i1:$pred, (i1 imm:$polarity))),
              (type (FCLASSsp SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  }

  def : Pat<(f16 (int_tpc_fclass f16:$src, (i8 imm:$optype), (i32 imm:$sw), f16:$income, i1:$pred, (i1 imm:$polarity))),
            (f16 (FCLASSsp SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Gen3Plus]>;
}


//------ POPCNT ----------------------------------------------------------------

def POPCNTsp : SpuInst_UnarySw<0b011001, SRF, SRF, "popcnt$optype $sw $dest, $op, $pred">;

foreach type = [i8, i16, i32, f32, bf16, f16, f8_143, f8_152] in {
  def : Pat<(i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw),  i8:$income,   i1:$pred, (i1 imm:$polarity))),
            (i8 (POPCNTsp        SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}

// Lowering of llvm.ctpop
def : Pat<(ctpop i8:$src),
          (i8 (POPCNTsp i8:$src,  OpType.INT8,  SW.SET, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctpop i16:$src)))),
          (i8 (POPCNTsp i16:$src, OpType.INT16, SW.SET, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctpop i32:$src)))),
          (i8 (POPCNTsp i32:$src, OpType.INT32, SW.SET, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ FIND_FIRST ------------------------------------------------------------

def FIND_FIRSTsp : SpuInst_UnarySw<0b011010, SRF, SRF, "find_first$optype $sw $dest, $op, $pred">;

foreach type = [i8, i16, i32, f32, bf16, f16, f8_143, f8_152] in
  def : Pat<(i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw),  i8:$income,   i1:$pred, (i1 imm:$polarity))),
            (i8 (FIND_FIRSTsp  SRF:$src,  (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;

// Lowering of llvm.ctlz
def : Pat<(ctlz i8:$src),
          (i8 (FIND_FIRSTsp i8:$src,  OpType.INT8,  SW.MSB, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctlz i16:$src)))),
          (i8 (FIND_FIRSTsp i16:$src, OpType.INT16, SW.MSB, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctlz i32:$src)))),
          (i8 (FIND_FIRSTsp i32:$src, OpType.INT32, SW.MSB, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

// Lowering of llvm.cttz
def : Pat<(ctlz i8:$src),
          (i8 (FIND_FIRSTsp i8:$src,  OpType.INT8,  (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i16 (ctlz i16:$src)))),
          (i8 (FIND_FIRSTsp i16:$src, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(i8 (trunc (i32 (ctlz i32:$src)))),
          (i8 (FIND_FIRSTsp i32:$src, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ EXTRACT_EXP -----------------------------------------------------------

def EXTRACT_EXPssp : SpuInst_UnarySw <0b011101, SRF, SRF,    "extract_exp$optype $sw $dest, $op, $pred">;
def EXTRACT_EXPsip : SpuInst_UnarySwI<0b011101, SRF, i32imm, "extract_exp$optype $sw $dest, $op, $pred">;

foreach type = [f32, bf16, f16, f8_143, f8_152] in {
  def : Pat<(FloatToInteger<type>.Type (int_tpc_extract_exp         type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income,   i1:$pred, (i1 imm:$polarity))),
            (FloatToInteger<type>.Type (EXTRACT_EXPssp               SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(FloatToInteger<type>.Type (int_tpc_extract_exp (type fpimm:$src), (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income,   i1:$pred, (i1 imm:$polarity))),
            (FloatToInteger<type>.Type (EXTRACT_EXPsip       (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
}


//------ NEARBYINT -------------------------------------------------------------

def NEARBYINTsp : SpuInst_NEARBYINT<0b011011, SRF, SRF, "nearbyint$optype $sw $dest, $op, $pred">;

foreach type = [f32, bf16, f16, f8_143, f8_152] in
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income,   i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTsp        SRF:$src, (i8 imm:$optype), (i32 imm:$sw),  SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;


//------ BREV ------------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def BREVssp : SpuInst_UnarySw<0b011111, SRF, SRF, "brev$optype $sw $dest, $op, $pred">;

  foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
    def : Pat<(FloatToInteger<type>.Type (int_tpc_brev type:$src, (i8 imm:$optype), (i32 imm:$sw), FloatToInteger<type>.Type:$income, i1:$pred, (i1 imm:$polarity))),
              (FloatToInteger<type>.Type (BREVssp SRF:$src, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity)))>;
  }
}


//------ JMPA, JMPR ------------------------------------------------------------

let isBranch = 1, isTerminator = 1 in {

let isNotUsedInDisasm = 1 in {
// Generic predicable instruction.
def JMPR   : SpuInstJmp <0b100100, "jmpr $target, $pred">;
def JMPA   : SpuInstJmp <0b100101, "jmpa $target, $pred">;

// Unconditional jump. Need to be a barrier.
let isBarrier = 1 in
def JMPR_u : SpuInstJmpU<0b100100, "jmpr $target", [(br bb:$target)]>;
}

// Register variants. Always indirect.
let isNotUsedInDisasm = 0, isIndirectBranch = 1 in {
def JMPRr  : SpuInstJmpR<0b100100, "jmpr $target, $pred">;
def JMPAr  : SpuInstJmpR<0b100101, "jmpa $target, $pred">;
}

}

def : Pat<(brind SRF:$target), (JMPRr SRF:$target, SPRF_TRUE, (i1 0))>;
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
def SET_INDX_spu_rwp : SpuInst_SET_INDX <0b100111, "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_rwp">, Requires<[DimMaskR]>;
def SET_INDX_spu_iwp : SpuInst_SET_INDXI<0b100111, "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_iwp">, Requires<[DimMaskR]>;


//------ UDIV_STEP -------------------------------------------------------------

def UDIV_STEP   : SpuInst_UDIV_STEP<0b101000, "udiv_step$optype $sw $step $dest, $src, $pred", 0>, Requires<[Gen1]>;
def UDIV_4STEP  : SpuInst_UDIV_STEP<0b101000, "udiv_4step$optype $sw $step $dest, $src, $pred", 0>, Requires<[Gen2Plus, PriorGen4]>;
def UDIV_4STEPT : SpuInst_UDIV_STEP<0b101000, "udiv_4step$optype $sw $step $dest, $src, $pred", 1>, Requires<[Gen3]>;

def : Pat<(v2i32 (int_tpc_udiv_step i32:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;
def : Pat<(v2i32 (int_tpc_udiv_step i16:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;
def : Pat<(v2i32 (int_tpc_udiv_step i8:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw),  v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen1]>;

def : Pat<(v2i32 (int_tpc_udiv_step i32:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus, PriorGen4]>;
def : Pat<(v2i32 (int_tpc_udiv_step i16:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus, PriorGen4]>;
def : Pat<(v2i32 (int_tpc_udiv_step i8:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEP SRF:$src, (i32 imm:$step), (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen2Plus, PriorGen4]>;


def : Pat<(v2i32 (int_tpc_udiv_step i32:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEPT SRF:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen3]>;
def : Pat<(v2i32 (int_tpc_udiv_step i16:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEPT SRF:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen3]>;
def : Pat<(v2i32 (int_tpc_udiv_step i8:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw),  v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_4STEPT SRF:$src, i32:$step, (i8 imm:$optype), (i32 imm:$sw), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen3]>;


//------ UDIV ------------------------------------------------------------------

def UDIV : SpuInst_UDIV<0b101000, "udiv$optype $sw $dest, $src1, $src2, $pred", 0>, Requires<[Gen4Plus]>;
def UDIV_BOTH : SpuInst_UDIV<0b101000, "udiv$optype $sw$bothdiv $dest, $src1, $src2, $pred", 1>, Requires<[Gen4Plus]>;

def : Pat<(v2i32 (int_tpc_udiv i32:$src1, i32:$src2, (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_BOTH SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), (i8 0), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(v2i32 (int_tpc_udiv i16:$src1, i16:$src2, (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_BOTH SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), (i8 0), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(v2i32 (int_tpc_udiv i8:$src1, i8:$src2, (i8 imm:$optype), (i32 imm:$sw), v2i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV_BOTH SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), (i8 0), ZRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;

def : Pat<(i32 (int_tpc_udiv i32:$src1, i32:$src2, (i8 imm:$optype), (i32 imm:$sw), i32:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(i16 (int_tpc_udiv i16:$src1, i16:$src2, (i8 imm:$optype), (i32 imm:$sw), i16:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(i8 (int_tpc_udiv i8:$src1, i8:$src2, (i8 imm:$optype), (i32 imm:$sw), i8:$income, i1:$pred, (i1 imm:$polarity))),
          (UDIV SRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), SRF:$income, SPRF:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;

//------ CALC_FP_SPECIAL -------------------------------------------------------

def CALC_FP_SPECIALssp : SpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred">, Requires<[Gen2Plus]>;

foreach type = [f32, bf16, f16] in
  def : Pat<(type (int_tpc_calc_fp_special type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity))),
            (type (CALC_FP_SPECIALssp type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity)))>;

// Variants for one argument
foreach func = [0, 1, 2, 3, 4, 5] in // funcId (RECIP, RSQRT, SQRT, LOG, EXP, TANH)
  foreach type = [f32, bf16, f16] in
    def : Pat<(type (int_tpc_calc_fp_special type:$src1, (type undef), (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity))),
              (type (CALC_FP_SPECIALssp type:$src1, type:$src1, (i8 imm:$optype), (i32 imm:$func), type:$income, i1:$pred, (i1 imm:$polarity)))>;

//------ THREAD_SYNC -----------------------------------------------------------
def THREAD_SYNC : SpuInst_THREAD_SYNC<0b111000, "thread_sync">, Requires<[Doron1]>;
def : Pat<(int_tpc_thread_sync), (THREAD_SYNC)>;

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

multiclass MacMulX2Instr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvvp : VpuInst_MAC_X2   <OpCode, Rdst, VRF, VRF,    0/*acc*/, asmstr, Type, SPred>;
  def vsvvp : VpuInst_MAC_X2   <OpCode, Rdst, VRF, SRF,    0/*acc*/, asmstr, Type, SPred>;
  def vivvp : VpuInst_MAC_X2_I <OpCode, Rdst, VRF, i32imm, 0/*acc*/, asmstr, Type, SPred>;
  def vvvvm : VpuInst_MAC_X2   <OpCode, Rdst, VRF, VRF,    0/*acc*/, asmstr, Type, VPred>;
  def vsvvm : VpuInst_MAC_X2   <OpCode, Rdst, VRF, SRF,    0/*acc*/, asmstr, Type, VPred>;
  def vivvm : VpuInst_MAC_X2_I <OpCode, Rdst, VRF, i32imm, 0/*acc*/, asmstr, Type, VPred>;
}

multiclass MacZPInstr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvp : VpuInst_MAC_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsp : VpuInst_MAC_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vip : VpuInst_MAC_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvm : VpuInst_MAC_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsm : VpuInst_MAC_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vim : VpuInst_MAC_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MacZPInstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvp : VpuInst_MACA_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsp : VpuInst_MACA_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vip : VpuInst_MACA_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvm : VpuInst_MACA_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsm : VpuInst_MACA_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vim : VpuInst_MACA_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MacX2InstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvvp : VpuInst_MAC_X2   <OpCode, Rdst, VRF, VRF,    1/*acc*/, asmstr, Type, SPred>;
  def vsvvp : VpuInst_MAC_X2   <OpCode, Rdst, VRF, SRF,    1/*acc*/, asmstr, Type, SPred>;
  def vivvp : VpuInst_MAC_X2_I <OpCode, Rdst, VRF, i32imm, 1/*acc*/, asmstr, Type, SPred>;
  def vvvvm : VpuInst_MAC_X2   <OpCode, Rdst, VRF, VRF,    1/*acc*/, asmstr, Type, VPred>;
  def vsvvm : VpuInst_MAC_X2   <OpCode, Rdst, VRF, SRF,    1/*acc*/, asmstr, Type, VPred>;
  def vivvm : VpuInst_MAC_X2_I <OpCode, Rdst, VRF, i32imm, 1/*acc*/, asmstr, Type, VPred>;
}

multiclass MacX2ZPInstr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvvp : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, VRF,    0/*acc*/, asmstr, Type, SPred>;
  def vsvvp : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, SRF,    0/*acc*/, asmstr, Type, SPred>;
  def vivvp : VpuInst_MAC_X2_ZP_I <OpCode, Rdst, VRF, i32imm, 0/*acc*/, asmstr, Type, SPred>;
  def vvvvm : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, VRF,    0/*acc*/, asmstr, Type, VPred>;
  def vsvvm : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, SRF,    0/*acc*/, asmstr, Type, VPred>;
  def vivvm : VpuInst_MAC_X2_ZP_I <OpCode, Rdst, VRF, i32imm, 0/*acc*/, asmstr, Type, VPred>;
}

multiclass MacX2ZPInstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvvp : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, VRF,    1/*acc*/, asmstr, Type, SPred>;
  def vsvvp : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, SRF,    1/*acc*/, asmstr, Type, SPred>;
  def vivvp : VpuInst_MAC_X2_ZP_I <OpCode, Rdst, VRF, i32imm, 1/*acc*/, asmstr, Type, SPred>;
  def vvvvm : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, VRF,    1/*acc*/, asmstr, Type, VPred>;
  def vsvvm : VpuInst_MAC_X2_ZP   <OpCode, Rdst, VRF, SRF,    1/*acc*/, asmstr, Type, VPred>;
  def vivvm : VpuInst_MAC_X2_ZP_I <OpCode, Rdst, VRF, i32imm, 1/*acc*/, asmstr, Type, VPred>;
}

multiclass MacMulX2InstrGen4<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvp : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            VRF,            asmstr, Type, SPred>;
  def viip : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, TPCImm<i32imm>, asmstr, Type, SPred>;
  def vssp : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            SRF,            SRF,            asmstr, Type, SPred>;
  def vivp : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, VRF,            asmstr, Type, SPred>;
  def vsvp : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            SRF,            VRF,            asmstr, Type, SPred>;
  def vvip : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            TPCImm<i32imm>, asmstr, Type, SPred>;
  def vvsp : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            SRF,            asmstr, Type, SPred>;
  def ivvp : VpuInst_MAC_MUL_X2<OpCode, Rdst, TPCImm<i32imm>, VRF,            VRF,            asmstr, Type, SPred>;
  def svvp : VpuInst_MAC_MUL_X2<OpCode, Rdst, SRF,            VRF,            VRF,            asmstr, Type, SPred>;

  def vvvm : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            VRF,            asmstr, Type, VPred>;
  def viim : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, TPCImm<i32imm>, asmstr, Type, VPred>;
  def vssm : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            SRF,            SRF,            asmstr, Type, VPred>;
  def vivm : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, VRF,            asmstr, Type, VPred>;
  def vsvm : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            SRF,            VRF,            asmstr, Type, VPred>;
  def vvim : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            TPCImm<i32imm>, asmstr, Type, VPred>;
  def vvsm : VpuInst_MAC_MUL_X2<OpCode, Rdst, DRF,            VRF,            SRF,            asmstr, Type, VPred>;
  def ivvm : VpuInst_MAC_MUL_X2<OpCode, Rdst, TPCImm<i32imm>, VRF,            VRF,            asmstr, Type, VPred>;
  def svvm : VpuInst_MAC_MUL_X2<OpCode, Rdst, SRF,            VRF,            VRF,            asmstr, Type, VPred>;
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

multiclass MacMulIntrinsicsDnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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

multiclass MacMulIntrinsicsAccDnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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

multiclass MacMulX2IntrinsicsDnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
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
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
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

multiclass MacMulX2IntrinsicsGen4<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (OpTypeProps<T>.Scal ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Scal ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        (OpTypeProps<T>.Scal ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        (OpTypeProps<T>.Cast ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MacMulX2IntrinsicsGen4Dnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        (OpTypeProps<T>.Scal ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Scal ImmNode:$x2),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        (OpTypeProps<T>.Scal ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        (OpTypeProps<T>.Cast ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Arg:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

//------ MAC -------------------------------------------------------------------

defm MACf32  : MacMulInstr<0b000000, VRF, OpType.FP32,   "mac.f32  $dt$sw $dest, $op1, $op2, $pred">;
defm MACbf16 : MacMulInstr<0b000000, VRF, OpType.BF16,   "mac.bf16 $dt$sw $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MACf16  : MacMulInstr<0b000000, VRF, OpType.FP16,   "mac.f16  $dt$sw $dest, $op1, $op2, $pred">, Requires<[FP16]>;
defm MACi16  : MacMulInstr<0b000000, DRF, OpType.INT16,  "mac.i16  $dt$sw $dest, $op1, $op2, $pred">;
defm MACu16  : MacMulInstr<0b000000, DRF, OpType.UINT16, "mac.u16  $dt$sw $dest, $op1, $op2, $pred">;
defm MACi8   : MacMulInstr<0b000000, ARF, OpType.INT8,   "mac.i8   $dt$sw $dest, $op1, $op2, $pred">, Requires<[MULI8]>;
defm MACu8   : MacMulInstr<0b000000, ARF, OpType.UINT8,  "mac.u8   $dt$sw $dest, $op1, $op2, $pred">, Requires<[MULI8]>;

// ACC_FP32
defm MACAbf16  : MacMulInstrAcc<0b000000, DRF, OpType.BF16,    "mac.bf16 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MACAf16   : MacMulInstrAcc<0b000000, DRF, OpType.FP16,    "mac.f16 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[FP16]>;
defm MACAf8_143: MacMulInstrAcc<0b000000, ARF, OpType.FP8_143, "mac.f8_143 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[FP8]>;
defm MACAf8_152: MacMulInstrAcc<0b000000, ARF, OpType.FP8_152, "mac.f8_152 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[FP8]>;

// acc - ACC_16
defm MACAi8    : MacMulInstrAcc<0b000000, DRF, OpType.INT8,   "mac.i8 $dt$sw$acc $dest, $op1, $op2, $pred">,  Requires<[AccI16]>;
defm MACAu8    : MacMulInstrAcc<0b000000, DRF, OpType.UINT8,  "mac.u8 $dt$sw$acc $dest, $op1, $op2, $pred">,  Requires<[AccI16]>;
// acc - ACC_i32
defm MACA32u8  : MacMulInstrAcc<0b000000, ARF, OpType.UINT8,  "mac.u8 $dt$sw$acc $dest, $op1, $op2, $pred">,  Requires<[AccI32]>;
defm MACA32u16 : MacMulInstrAcc<0b000000, DRF, OpType.UINT16, "mac.u16 $dt$sw$acc $dest, $op1, $op2, $pred">, Requires<[AccI32]>;

//ZP
defm MACzpi8 : MacZPInstr<0b000000, ARF, OpType.INT8,  "mac.i8 $dt$sw $zp $dest, $op1, $op2, $pred">, Requires<[ZP]>;
defm MACzpu8 : MacZPInstr<0b000000, ARF, OpType.UINT8, "mac.u8 $dt$sw $zp $dest, $op1, $op2, $pred">, Requires<[ZP]>;

// acc - ACC_16
defm MACAzpi8   : MacZPInstrAcc<0b000000, DRF, OpType.INT8,  "mac.i8 $dt$sw$acc $zp $dest, $op1, $op2, $pred">, Requires<[AccI16, ZP]>;
defm MACAzpu8   : MacZPInstrAcc<0b000000, DRF, OpType.UINT8, "mac.u8 $dt$sw$acc $zp $dest, $op1, $op2, $pred">, Requires<[AccI16, ZP]>;
// acc - ACC_i32
defm MACA32zpu8 : MacZPInstrAcc<0b000000, ARF, OpType.UINT8, "mac.u8 $dt$sw$acc $zp $dest, $op1, $op2, $pred">, Requires<[AccI32, ZP]>;

//x2
defm MACx2u8    : MacMulX2Instr    <0b000000, ARF, OpType.UINT8, "mac.u8 $dt$sw $dest, $op1, $op2, $op3, $op4, $pred">,  Requires<[X2Mac]>;
defm MACx2i8    : MacMulX2Instr    <0b000000, ARF, OpType.INT8,  "mac.i8 $dt$sw $dest, $op1, $op2, $op3, $op4, $pred">,  Requires<[X2Mac]>;
defm MACx2f32   : MacMulX2InstrGen4<0b000000, DRF, OpType.FP32,  "mac.f32 $dt$sw $dest, $op1, $op2, $op3, $pred">,       Requires<[Gen4Plus, X2Mac]>;

// acc - ACC_16
defm MACAx2u8   : MacX2InstrAcc<0b000000, DRF, OpType.UINT8,  "mac.u8 $dt$sw$acc $dest, $op1, $op2, $op3, $op4, $pred">,  Requires<[AccI16, X2Mac]>;
defm MACAx2i8   : MacX2InstrAcc<0b000000, DRF, OpType.INT8,   "mac.i8 $dt$sw$acc $dest, $op1, $op2, $op3, $op4, $pred">,  Requires<[AccI16, X2Mac]>;
// acc - ACC_i32
defm MACA32x2u8 : MacX2InstrAcc<0b000000, ARF, OpType.UINT8,  "mac.u8 $dt$sw$acc $dest, $op1, $op2, $op3, $op4, $pred">,  Requires<[AccI32, X2Mac]>;

//X2 + ZP
defm MACx2zpu8  : MacX2ZPInstr<0b000000, ARF, OpType.UINT8, "mac.u8 $dt$sw $zp $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[X2Mac, ZP]>;
defm MACx2zpi8  : MacX2ZPInstr<0b000000, ARF, OpType.INT8,  "mac.i8 $dt$sw $zp $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[X2Mac, ZP]>;

// acc - ACC_16
defm MACAx2zpu8   : MacX2ZPInstrAcc<0b000000, DRF, OpType.UINT8, "mac.u8 $dt$sw$acc $zp $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[AccI16, X2Mac, ZP]>;
defm MACAx2zpi8   : MacX2ZPInstrAcc<0b000000, DRF, OpType.INT8,  "mac.i8 $dt$sw$acc $zp $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[AccI16, X2Mac, ZP]>;
// acc - ACC_i32
defm MACA32x2zpu8 : MacX2ZPInstrAcc<0b000000, ARF, OpType.UINT8, "mac.u8 $dt$sw$acc $zp $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[AccI32, X2Mac, ZP]>;

defm : MacMulIntrinsicsDnorm<int_tpc_mac, OpType.FP32,   v64f32,   "MAC", fpimm>;
defm : MacMulIntrinsicsDnorm<int_tpc_mac, OpType.BF16,   v128bf16, "MAC", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsicsDnorm<int_tpc_mac, OpType.FP16,   v128f16,  "MAC", fpimm>, Requires<[FP16]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.INT8,   v256i32,  "MAC", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.UINT8,  v256i32,  "MAC", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.INT16,  v128i32,  "MAC", imm>;
defm : MacMulIntrinsics<int_tpc_mac, OpType.UINT16, v128i32,  "MAC", imm>;

defm : MacMulIntrinsicsAccDnorm<int_tpc_mac, OpType.BF16, v128f32, "MACA", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mac, OpType.FP16, v128f32, "MACA", fpimm>, Requires<[FP16]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mac, OpType.FP8_143, v256f32, "MACA", fpimm>, Requires<[FP8]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mac, OpType.FP8_152, v256f32, "MACA", fpimm>, Requires<[FP8]>;

// acc - ACC_16
defm : MacMulIntrinsicsAcc<int_tpc_mac, OpType.INT8,   v256i16, "MACA",   imm>, Requires<[AccI16]>;
defm : MacMulIntrinsicsAcc<int_tpc_mac, OpType.UINT8,  v256i16, "MACA",   imm>, Requires<[AccI16]>;
// acc - ACC_i32
defm : MacMulIntrinsicsAcc<int_tpc_mac, OpType.UINT8,  v256i32, "MACA32", imm>, Requires<[AccI32]>;
defm : MacMulIntrinsicsAcc<int_tpc_mac, OpType.UINT16, v128i32, "MACA32", imm>, Requires<[AccI32]>;

// zp
defm : MacZPIntrinsics<int_tpc_mac_zp, OpType.INT8,  v256i32, "MACzp", imm>, Requires<[ZP]>;
defm : MacZPIntrinsics<int_tpc_mac_zp, OpType.UINT8, v256i32, "MACzp", imm>, Requires<[ZP]>;

// acc - ACC_16
defm : MacZPIntrinsicsAcc<int_tpc_mac_zp, OpType.INT8,  v256i16, "MACAzp",   imm>, Requires<[AccI16, ZP]>;
defm : MacZPIntrinsicsAcc<int_tpc_mac_zp, OpType.UINT8, v256i16, "MACAzp",   imm>, Requires<[AccI16, ZP]>;
// acc - ACC_i32
defm : MacZPIntrinsicsAcc<int_tpc_mac_zp, OpType.UINT8, v256i32, "MACA32zp", imm>, Requires<[AccI32, ZP]>;

// X2
defm : MacMulX2Intrinsics    <int_tpc_mac_x2,     OpType.INT8,  v256i32,                   "MACx2", imm>,   Requires<[X2Mac]>;
defm : MacMulX2Intrinsics    <int_tpc_mac_x2,     OpType.UINT8, v256i32,                   "MACx2", imm>,   Requires<[X2Mac]>;
defm : MacMulX2IntrinsicsGen4Dnorm<int_tpc_mac_x2_f32, OpType.FP32,  DoubleVector<v64f32>.Type, "MACx2", fpimm>, Requires<[Gen4Plus]>;
// X2 + ZP
defm : MacX2ZPIntrinsics<int_tpc_mac_x2_zp, OpType.INT8, v256i32,  "MACx2zp">,   Requires<[X2Mac, ZP]>;
defm : MacX2ZPIntrinsics<int_tpc_mac_x2_zp, OpType.UINT8, v256i32,  "MACx2zp">,   Requires<[X2Mac, ZP]>;

// acc - ACC_16
defm : MacX2IntrinsicsAcc<int_tpc_mac_x2, OpType.INT8,  v256i16, "MACAx2">,   Requires<[AccI16, X2Mac]>;
defm : MacX2IntrinsicsAcc<int_tpc_mac_x2, OpType.UINT8, v256i16, "MACAx2">,   Requires<[AccI16, X2Mac]>;
// acc - ACC_i32
defm : MacX2IntrinsicsAcc<int_tpc_mac_x2, OpType.UINT8, v256i32, "MACA32x2">, Requires<[AccI32, X2Mac]>;

// acc - ACC_16
defm : MacX2ZPIntrinsicsAcc<int_tpc_mac_x2_zp, OpType.INT8,  v256i16, "MACAx2zp">,   Requires<[AccI16, X2Mac, ZP]>;
defm : MacX2ZPIntrinsicsAcc<int_tpc_mac_x2_zp, OpType.UINT8, v256i16, "MACAx2zp">,   Requires<[AccI16, X2Mac, ZP]>;
// acc - ACC_i32
defm : MacX2ZPIntrinsicsAcc<int_tpc_mac_x2_zp, OpType.UINT8, v256i32, "MACA32x2zp">, Requires<[AccI32, X2Mac, ZP]>;

//------ MUL -------------------------------------------------------------------

defm MULf32  : MacMulInstr<0b000001, VRF, OpType.FP32,   "mul.f32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULbf16 : MacMulInstr<0b000001, VRF, OpType.BF16,   "mul.bf16 $dt $sw $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MULf16  : MacMulInstr<0b000001, VRF, OpType.FP16,   "mul.f16 $dt $sw $dest, $op1, $op2, $pred">, Requires<[FP16]>;
defm MULi32  : MacMulInstr<0b000001, DRF, OpType.INT32,  "mul.i32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULu32  : MacMulInstr<0b000001, DRF, OpType.UINT32, "mul.u32 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULi16  : MacMulInstr<0b000001, DRF, OpType.INT16,  "mul.i16 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULu16  : MacMulInstr<0b000001, DRF, OpType.UINT16, "mul.u16 $dt $sw $dest, $op1, $op2, $pred" >;
defm MULi8   : MacMulInstr<0b000001, ARF, OpType.INT8,   "mul.i8 $dt $sw $dest, $op1, $op2, $pred"  >, Requires<[MULI8]>;
defm MULu8   : MacMulInstr<0b000001, ARF, OpType.UINT8,  "mul.u8 $dt $sw $dest, $op1, $op2, $pred"  >, Requires<[MULI8]>;

// Variants with DOUBLE_AND_ROUND32 (i32/u32) and ACC_FP32 (bf16, f8_143, f8_152).
defm MULAbf16 : MacMulInstrAcc<0b000001, DRF, OpType.BF16,   "mul.bf16 $dt $sw$acc $dest, $op1, $op2, $pred">, Requires<[BF16]>;
defm MULAf16  : MacMulInstrAcc<0b000001, DRF, OpType.FP16,   "mul.f16 $dt $sw$acc $dest, $op1, $op2, $pred">, Requires<[FP16]>;
defm MULAf8_143 : MacMulInstrAcc<0b000001, ARF, OpType.FP8_143, "mul.f8_143 $dt $sw$acc $dest, $op1, $op2, $pred">, Requires<[FP8]>;
defm MULAf8_152 : MacMulInstrAcc<0b000001, ARF, OpType.FP8_152, "mul.f8_152 $dt $sw$acc $dest, $op1, $op2, $pred">, Requires<[FP8]>;
defm MULAi32  : MacMulInstrAcc<0b000001, VRF, OpType.INT32,  "mul.i32 $dt $sw$acc $dest, $op1, $op2, $pred" >;
defm MULAu32  : MacMulInstrAcc<0b000001, VRF, OpType.UINT32, "mul.u32 $dt $sw$acc $dest, $op1, $op2, $pred" >;

// Variants with X2
defm MULx2f32 : MacMulX2InstrGen4<0b000001, DRF, OpType.FP32,  "mul.f32 $dt$sw $dest, $op1, $op2, $op3, $pred">, Requires<[Gen4Plus]>;


def : Pat<(v64f32 (fmul v64f32:$op1, v64f32:$op2)),
        (v64f32 (MULf32vvp  v64f32:$op1, v64f32:$op2, OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64f32 (fmul v64f32:$op1, (v64f32 (vsplatf32 f32:$op2)))),
        (v64f32 (MULf32vsp  v64f32:$op1, f32:$op2, OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64f32 (fmul v64f32:$op1, (v64f32 (vsplatf32 (f32 fpimm:$op2))))),
        (v64f32 (MULf32vip  v64f32:$op1, (FPToImm32 $op2), OpType.FP32, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
let Predicates = [BF16] in {
  def : Pat<(v128bf16 (fmul v128bf16:$op1, v128bf16:$op2)),
            (v128bf16 (MULbf16vvp v128bf16:$op1, v128bf16:$op2, OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(v128bf16 (fmul v128bf16:$op1, (v128bf16 (vsplatbf16 bf16:$op2)))),
            (v128bf16 (MULbf16vsp v128bf16:$op1, bf16:$op2, OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(v128bf16 (fmul v128bf16:$op1, (v128bf16 (vsplatbf16 (bf16 fpimm:$op2))))),
            (v128bf16 (MULbf16vip v128bf16:$op1, (FPToImm32 $op2), OpType.BF16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}
let Predicates = [FP16] in {
  def : Pat<(v128f16 (fmul v128f16:$op1, v128f16:$op2)),
            (v128f16 (MULf16vvp v128f16:$op1, v128f16:$op2, OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(v128f16 (fmul v128f16:$op1, (v128f16 (vsplatf16 f16:$op2)))),
            (v128f16 (MULf16vsp v128f16:$op1, f16:$op2, OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(v128f16 (fmul v128f16:$op1, (v128f16 (vsplatf16 (f16 fpimm:$op2))))),
            (v128f16 (MULf16vip v128f16:$op1, (FPToImm32 $op2), OpType.FP16, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}


defm : MacMulIntrinsicsDnorm<int_tpc_mul, OpType.FP32,   v64f32,   "MUL", fpimm>;
defm : MacMulIntrinsicsDnorm<int_tpc_mul, OpType.BF16,   v128bf16, "MUL", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsicsDnorm<int_tpc_mul, OpType.FP16,   v128f16,  "MUL", fpimm>, Requires<[FP16]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT8,   v256i32,  "MUL", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT8,  v256i32,  "MUL", imm>,   Requires<[MULI8]>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT16,  v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT16, v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.INT32,  v128i32,  "MUL", imm>;
defm : MacMulIntrinsics<int_tpc_mul, OpType.UINT32, v128i32,  "MUL", imm>;

defm : MacMulIntrinsicsAccDnorm<int_tpc_mul, OpType.BF16,   v128f32, "MULA", fpimm>, Requires<[BF16]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mul, OpType.FP16,   v128f32, "MULA", fpimm>, Requires<[FP16]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mul, OpType.FP8_143, v256f32, "MULA", fpimm>, Requires<[FP8]>;
defm : MacMulIntrinsicsAccDnorm<int_tpc_mul, OpType.FP8_152, v256f32, "MULA", fpimm>, Requires<[FP8]>;
defm : MacMulIntrinsicsAcc<int_tpc_mul, OpType.INT32,  v64i32,  "MULA", imm>;
defm : MacMulIntrinsicsAcc<int_tpc_mul, OpType.UINT32, v64i32,  "MULA", imm>;

defm : MacMulX2IntrinsicsGen4Dnorm<int_tpc_mul_x2_f32, OpType.FP32, DoubleVector<v64f32>.Type, "MULx2", fpimm>, Requires<[Gen4Plus]>;

//------ MADD ------------------------------------------------------------------
// MADD and MUL share many common features, especially combinations of input and
// output register classes. Present them in the classes defined below.

multiclass MaddInstr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvp : VpuInst_MADD  <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsvp : VpuInst_MADD  <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vivp : VpuInst_MADDI <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvvm : VpuInst_MADD  <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsvm : VpuInst_MADD  <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vivm : VpuInst_MADDI <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MaddInstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvp : VpuInst_MADDA  <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsvp : VpuInst_MADDA  <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vivp : VpuInst_MADDAI <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvvm : VpuInst_MADDA  <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsvm : VpuInst_MADDA  <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vivm : VpuInst_MADDAI <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MaddZPInstr<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvp : VpuInst_MADD_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsvp : VpuInst_MADD_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vivp : VpuInst_MADD_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvvm : VpuInst_MADD_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsvm : VpuInst_MADD_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vivm : VpuInst_MADD_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MaddZPInstrAcc<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvp : VpuInst_MADDA_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, SPred>;
  def vsvp : VpuInst_MADDA_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, SPred>;
  def vivp : VpuInst_MADDA_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, SPred>;
  def vvvm : VpuInst_MADDA_ZP   <OpCode, Rdst, VRF, VRF,    asmstr, Type, VPred>;
  def vsvm : VpuInst_MADDA_ZP   <OpCode, Rdst, VRF, SRF,    asmstr, Type, VPred>;
  def vivm : VpuInst_MADDA_ZP_I <OpCode, Rdst, VRF, i32imm, asmstr, Type, VPred>;
}

multiclass MaddInstrX2<bits<6> OpCode, RegisterClass Rdst, bits<4> Type, string asmstr> {
  def vvvvp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            VRF,            asmstr, Type, SPred>;
  def vivip : VpuInst_MADD_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, DRF,            TPCImm<i32imm>, asmstr, Type, SPred>;
  def vsvsp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            SRF,            DRF,            SRF,            asmstr, Type, SPred>;
  def vivvp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, DRF,            VRF,            asmstr, Type, SPred>;
  def vsvvp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            SRF,            DRF,            VRF,            asmstr, Type, SPred>;
  def vvvip : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            TPCImm<i32imm>, asmstr, Type, SPred>;
  def vvvsp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            SRF,            asmstr, Type, SPred>;
  def vvivp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            TPCImm<i32imm>, VRF,            asmstr, Type, SPred>;
  def vvsvp : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            SRF,            VRF,            asmstr, Type, SPred>;
  def ivvvp : VpuInst_MADD_X2<OpCode, Rdst, TPCImm<i32imm>, VRF,            DRF,            VRF,            asmstr, Type, SPred>;
  def svvvp : VpuInst_MADD_X2<OpCode, Rdst, SRF,            VRF,            DRF,            VRF,            asmstr, Type, SPred>;

  def vvvvm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            VRF,            asmstr, Type, VPred>;
  def vivim : VpuInst_MADD_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, DRF,            TPCImm<i32imm>, asmstr, Type, VPred>;
  def vsvsm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            SRF,            DRF,            SRF,            asmstr, Type, VPred>;
  def vivvm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            TPCImm<i32imm>, DRF,            VRF,            asmstr, Type, VPred>;
  def vsvvm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            SRF,            DRF,            VRF,            asmstr, Type, VPred>;
  def vvvim : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            TPCImm<i32imm>, asmstr, Type, VPred>;
  def vvvsm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            DRF,            SRF,            asmstr, Type, VPred>;
  def vvivm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            TPCImm<i32imm>, VRF,            asmstr, Type, VPred>;
  def vvsvm : VpuInst_MADD_X2<OpCode, Rdst, DRF,            VRF,            SRF,            VRF,            asmstr, Type, VPred>;
  def ivvvm : VpuInst_MADD_X2<OpCode, Rdst, TPCImm<i32imm>, VRF,            DRF,            VRF,            asmstr, Type, VPred>;
  def svvvm : VpuInst_MADD_X2<OpCode, Rdst, SRF,            VRF,            DRF,            VRF,            asmstr, Type, VPred>;
}

multiclass MaddIntrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddIntrinsicsDnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddIntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddIntrinsicsAccDnorm<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddZPIntrinsics<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddZPIntrinsicsAcc<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg,  OpTypeProps<T>.Arg,pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Arg:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        ResTy:$x2,
                        OpTypeProps<T>.Arg:$zp,
                        (i8 T), (i32 imm:$sw), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

multiclass MaddIntrinsicsX2<SDNode Intrinsic, bits<4> T, ValueType ResTy, string I, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x3))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Cast ImmNode:$x3),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Scal ImmNode:$x3),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Cast ImmNode:$x3),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x3)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Scal ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Cast ImmNode:$x1),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Scal:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x3))),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Cast ImmNode:$x3),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Scal ImmNode:$x3),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Cast ImmNode:$x3),
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x3)),
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Scal:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Scal ImmNode:$x2),
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        (OpTypeProps<T>.Cast ImmNode:$x2),
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        OpTypeProps<T>.Scal:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        (OpTypeProps<T>.Scal ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, iAny, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        (OpTypeProps<T>.Cast ImmNode:$x0),
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy (Intrinsic
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy (ConstructInsn<ConstructName<T, I>.IName, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                        OpTypeProps<T>.Scal:$x0,
                        OpTypeProps<T>.Arg:$x1,
                        DoubleVector<OpTypeProps<T>.Arg>.Type:$x2,
                        OpTypeProps<T>.Arg:$x3,
                        (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

defm MADDf32  : MaddInstr<0b110111, VRF, OpType.FP32,   "madd.f32 $dt$sw $dest, $op1, $op2, $op3, $pred" >, Requires<[MADD]>;
defm MADDbf16 : MaddInstr<0b110111, VRF, OpType.BF16,   "madd.bf16 $dt$sw $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;
defm MADDf16  : MaddInstr<0b110111, VRF, OpType.FP16,   "madd.f16 $dt$sw $dest, $op1, $op2, $op3, $pred" >, Requires<[MADD]>;
defm MADDi16  : MaddInstr<0b110111, DRF, OpType.INT16,  "madd.i16 $dt$sw $dest, $op1, $op2, $op3, $pred" >, Requires<[MADD]>;
defm MADDu16  : MaddInstr<0b110111, DRF, OpType.UINT16, "madd.u16 $dt$sw $dest, $op1, $op2, $op3, $pred" >, Requires<[MADD]>;
defm MADDi8   : MaddInstr<0b110111, ARF, OpType.INT8,   "madd.i8 $dt$sw $dest, $op1, $op2, $op3, $pred"  >, Requires<[MADD]>;
defm MADDu8   : MaddInstr<0b110111, ARF, OpType.UINT8,  "madd.u8 $dt$sw $dest, $op1, $op2, $op3, $pred"  >, Requires<[MADD]>;

defm MADDAbf16 : MaddInstrAcc<0b110111, DRF, OpType.BF16,  "madd.bf16 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">,  Requires<[MADD]>;
defm MADDAf16  : MaddInstrAcc<0b110111, DRF, OpType.FP16,  "madd.f16 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">,  Requires<[MADD]>;
defm MADDAf8_143 : MaddInstrAcc<0b110111, ARF, OpType.FP8_143,  "madd.f8_143 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">,  Requires<[MADD, FP8]>;
defm MADDAf8_152 : MaddInstrAcc<0b110111, ARF, OpType.FP8_152,  "madd.f8_152 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">,  Requires<[MADD, FP8]>;
defm MADDAi8   : MaddInstrAcc<0b110111, DRF, OpType.INT8,  "madd.i8 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">,  Requires<[MADD]>;
defm MADDAu8   : MaddInstrAcc<0b110111, DRF, OpType.UINT8, "madd.u8 $dt$sw$acc $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;

//ZP
defm MADDzpi8   : MaddZPInstr<0b110111, ARF, OpType.INT8,  "madd.i8 $dt$sw $zp $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;
defm MADDzpu8   : MaddZPInstr<0b110111, ARF, OpType.UINT8, "madd.u8 $dt$sw $zp $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;

defm MADDAzpi8   : MaddZPInstrAcc<0b110111, DRF, OpType.INT8,  "madd.i8 $dt$sw$acc $zp $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;
defm MADDAzpu8   : MaddZPInstrAcc<0b110111, DRF, OpType.UINT8, "madd.u8 $dt$sw$acc $zp $dest, $op1, $op2, $op3, $pred">, Requires<[MADD]>;

//X2
defm MADDx2f32 : MaddInstrX2<0b110111, DRF, OpType.FP32, "madd.f32 $dt$sw$x2 $dest, $op1, $op2, $op3, $op4, $pred">, Requires<[Gen4Plus]>;

defm : MaddIntrinsicsDnorm<int_tpc_madd, OpType.FP32,   v64f32,   "MADD", fpimm>, Requires<[MADD]>;
defm : MaddIntrinsicsDnorm<int_tpc_madd, OpType.BF16,   v128bf16, "MADD", fpimm>, Requires<[MADD]>;
defm : MaddIntrinsicsDnorm<int_tpc_madd, OpType.FP16,   v128f16,  "MADD", fpimm>, Requires<[MADD]>;
defm : MaddIntrinsics<int_tpc_madd, OpType.INT8,   v256i32,  "MADD", imm>, Requires<[MADD]>;
defm : MaddIntrinsics<int_tpc_madd, OpType.UINT8,  v256i32,  "MADD", imm>, Requires<[MADD]>;
defm : MaddIntrinsics<int_tpc_madd, OpType.INT16,  v128i32,  "MADD", imm>, Requires<[MADD]>;
defm : MaddIntrinsics<int_tpc_madd, OpType.UINT16, v128i32,  "MADD", imm>, Requires<[MADD]>;

defm : MaddIntrinsicsAccDnorm<int_tpc_madd, OpType.BF16,  v128f32, "MADDA", fpimm>, Requires<[MADD]>;
defm : MaddIntrinsicsAccDnorm<int_tpc_madd, OpType.FP16,  v128f32, "MADDA", fpimm>, Requires<[MADD]>;
defm : MaddIntrinsicsAccDnorm<int_tpc_madd, OpType.FP8_143, v256f32, "MADDA", fpimm>, Requires<[MADD, FP8]>;
defm : MaddIntrinsicsAccDnorm<int_tpc_madd, OpType.FP8_152, v256f32, "MADDA", fpimm>, Requires<[MADD, FP8]>;
defm : MaddIntrinsicsAcc<int_tpc_madd, OpType.INT8,  v256i16, "MADDA", imm>,   Requires<[MADD]>;
defm : MaddIntrinsicsAcc<int_tpc_madd, OpType.UINT8, v256i16, "MADDA", imm>,   Requires<[MADD]>;

defm : MaddZPIntrinsics<int_tpc_madd_zp, OpType.INT8,   v256i32,  "MADDzp", imm>,   Requires<[MADD]>;
defm : MaddZPIntrinsics<int_tpc_madd_zp, OpType.UINT8,  v256i32,  "MADDzp", imm>,   Requires<[MADD]>;

defm : MaddZPIntrinsicsAcc<int_tpc_madd_zp, OpType.INT8,  v256i16, "MADDAzp", imm>,   Requires<[MADD]>;
defm : MaddZPIntrinsicsAcc<int_tpc_madd_zp, OpType.UINT8, v256i16, "MADDAzp", imm>,   Requires<[MADD]>;

defm : MaddIntrinsicsX2<int_tpc_madd_x2, OpType.FP32, DoubleVector<v64f32>.Type, "MADDx2", fpimm>, Requires<[Gen4Plus]>;

//------ ADD -------------------------------------------------------------------

def ADDvvp : VpuInstBinSw  <0b000010, VRF, VRF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvsp : VpuInstBinSw  <0b000010, VRF, VRF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvip : VpuInstBinSwI <0b000010, VRF, VRF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDvvm : VpuInstBinSw  <0b000010, VRF, VRF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDvsm : VpuInstBinSw  <0b000010, VRF, VRF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDvim : VpuInstBinSwI <0b000010, VRF, VRF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", VPred>;

let isNotUsedInDisasm = 1 in {
def ADDavp : VpuInstBinSw  <0b000010, ARF, ARF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDasp : VpuInstBinSw  <0b000010, ARF, ARF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDaip : VpuInstBinSwI <0b000010, ARF, ARF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ADDavm : VpuInstBinSw  <0b000010, ARF, ARF, VRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDasm : VpuInstBinSw  <0b000010, ARF, ARF, SRF,    "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
def ADDaim : VpuInstBinSwI <0b000010, ARF, ARF, i32imm, "add$optype $sw $dest, $op1, $op2, $pred", VPred>;
}

multiclass AddSubX2F32Instr<bits<6> OpCode, string asmstr> {
  def vvvp : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            VRF,            asmstr, SPred>;
  def vsvp : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, SRF,            VRF,            asmstr, SPred>;
  def vvsp : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            SRF,            asmstr, SPred>;
  def vssp : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, SRF,            SRF,            asmstr, SPred>;
  def vivp : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, TPCImm<i32imm>, VRF,            asmstr, SPred>;
  def vvip : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            TPCImm<i32imm>, asmstr, SPred>;
  def viip : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, TPCImm<i32imm>, TPCImm<i32imm>, asmstr, SPred>;
  def vvvm : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            VRF,            asmstr, VPred>;
  def vsvm : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, SRF,            VRF,            asmstr, VPred>;
  def vvsm : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            SRF,            asmstr, VPred>;
  def vssm : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, SRF,            SRF,            asmstr, VPred>;
  def vivm : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, TPCImm<i32imm>, VRF,            asmstr, VPred>;
  def vvim : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, VRF,            TPCImm<i32imm>, asmstr, VPred>;
  def viim : VpuInst_ADD_SUB_X2<OpCode, DRF, DRF, TPCImm<i32imm>, TPCImm<i32imm>, asmstr, VPred>;
}

defm ADDx2 : AddSubX2F32Instr<0b000010, "add$optype $sw$x2 $dest, $op1, $op2, $op3, $pred">, Requires<[Gen4Plus]>;

defm : BinaryIntegerVector<add,  ADDvvp, ADDvsp, ADDvip>;
defm : BinaryFloatVector<fadd, ADDvvp, ADDvsp, ADDvip>;

foreach pred = [i1, v256i1] in {
  foreach type = [v256i8, v128i16, v64i32] in
    defm : BinaryVectorIntrinsics<int_tpc_add, "ADD", type, type, pred>;
  foreach type = [v64f32, v128bf16, v128f16] in
    defm : BinaryVectorIntrinsicsDnorm<int_tpc_add, "ADD", type, type, pred>;

  defm : BinaryVectorIntrinsicsDnorm<int_tpc_add, "ADD", v256f32, v256f8_143, pred>;
  defm : BinaryVectorIntrinsicsDnorm<int_tpc_add, "ADD", v256f32, v256f8_152, pred>;
}

multiclass AddSubX2IntrinsicsGen4<SDNode Intrinsic, bits<4> T, ValueType ResTy, string Name, SDNode ImmNode> {
  foreach pred = [i1, v256i1] in {
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
                (ResTy
                  (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Arg, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                  (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x1)),
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat OpTypeProps<T>.Scal:$x2)),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Scal, OpTypeProps<T>.Scal, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Scal:$x1,
                  OpTypeProps<T>.Scal:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Scal ImmNode:$x1),
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Cast ImmNode:$x1),
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, iAny, OpTypeProps<T>.Arg, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Cast ImmNode:$x1),
                  OpTypeProps<T>.Arg:$x2,
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  (OpTypeProps<T>.Scal ImmNode:$x2),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  (OpTypeProps<T>.Cast ImmNode:$x2),
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, OpTypeProps<T>.Arg, iAny, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  OpTypeProps<T>.Arg:$x1,
                  (OpTypeProps<T>.Cast ImmNode:$x2),
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Scal ImmNode:$x1),
                  (OpTypeProps<T>.Scal ImmNode:$x2),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Cast ImmNode:$x1),
                  (OpTypeProps<T>.Cast ImmNode:$x2),
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(ResTy
                (Intrinsic
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x1))),
                  (OpTypeProps<T>.Arg (OpTypeProps<T>.Splat (OpTypeProps<T>.Scal ImmNode:$x2))),
                  (i8 T), (i32 imm:$sw), ResTy:$income, pred:$pred, (i1 imm:$polarity))),
              (ResTy
                (ConstructInsn<Name, OpTypeProps<T>.Arg, iAny, iAny, pred>.Insn
                  DoubleVector<OpTypeProps<T>.Arg>.Type:$x0,
                  (OpTypeProps<T>.Cast ImmNode:$x1),
                  (OpTypeProps<T>.Cast ImmNode:$x2),
                  (i8 T), (AddDnormSupport (i32 imm:$sw)), (i8 0), ResTy:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

defm : AddSubX2IntrinsicsGen4<int_tpc_add_x2, OpType.FP32, DoubleVector<v64f32>.Type, "ADDx2", fpimm>, Requires<[Gen4Plus]>;

//------ SUB -------------------------------------------------------------------

def SUBvvp : VpuInstBinSw  <0b000011, VRF, VRF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvsp : VpuInstBinSw  <0b000011, VRF, VRF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvip : VpuInstBinSwI <0b000011, VRF, VRF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBvvm : VpuInstBinSw  <0b000011, VRF, VRF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBvsm : VpuInstBinSw  <0b000011, VRF, VRF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBvim : VpuInstBinSwI <0b000011, VRF, VRF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;

let isNotUsedInDisasm = 1 in {
def SUBavp : VpuInstBinSw  <0b000011, ARF, ARF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBasp : VpuInstBinSw  <0b000011, ARF, ARF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBaip : VpuInstBinSwI <0b000011, ARF, ARF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SUBavm : VpuInstBinSw  <0b000011, ARF, ARF, VRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBasm : VpuInstBinSw  <0b000011, ARF, ARF, SRF,    "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SUBaim : VpuInstBinSwI <0b000011, ARF, ARF, i32imm, "sub$optype $sw $dest, $op1, $op2, $pred", VPred>;
}

defm SUBx2 : AddSubX2F32Instr<0b000011, "sub$optype $sw$x2 $dest, $op1, $op2, $op3, $pred">, Requires<[Gen4Plus]>;

defm : BinaryIntegerVector<sub,  SUBvvp, SUBvsp, SUBvip>;
defm : BinaryFloatVector<fsub, SUBvvp, SUBvsp, SUBvip>;

foreach pred = [i1, v256i1] in {
  foreach type = [v256i8, v128i16, v64i32] in
    defm : BinaryVectorIntrinsics<int_tpc_sub, "SUB", type, type, pred>;
  foreach type = [v64f32, v128bf16, v128f16] in
    defm : BinaryVectorIntrinsicsDnorm<int_tpc_sub, "SUB", type, type, pred>;
  
  defm : BinaryVectorIntrinsicsDnorm<int_tpc_sub, "SUB", v256f32, v256f8_143, pred>;
  defm : BinaryVectorIntrinsicsDnorm<int_tpc_sub, "SUB", v256f32, v256f8_152, pred>;
}

defm : AddSubX2IntrinsicsGen4<int_tpc_sub_x2, OpType.FP32, DoubleVector<v64f32>.Type, "SUBx2", fpimm>, Requires<[Gen4Plus]>;

//------ MAX -------------------------------------------------------------------

def MAXvvp  : VpuInstBinSw  <0b000101, VRF, VRF, VRF,    "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvsp  : VpuInstBinSw  <0b000101, VRF, VRF, SRF,    "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvip  : VpuInstBinSwI <0b000101, VRF, VRF, i32imm, "max$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MAXvvm  : VpuInstBinSw  <0b000101, VRF, VRF, VRF,    "max$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MAXvsm  : VpuInstBinSw  <0b000101, VRF, VRF, SRF,    "max$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MAXvim  : VpuInstBinSwI <0b000101, VRF, VRF, i32imm, "max$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_max, "MAX", type, type, pred>;


//------ MIN -------------------------------------------------------------------

def MINvvp  : VpuInstBinSw  <0b000110, VRF, VRF, VRF,    "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvsp  : VpuInstBinSw  <0b000110, VRF, VRF, SRF,    "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvip  : VpuInstBinSwI <0b000110, VRF, VRF, i32imm, "min$optype $sw $dest, $op1, $op2, $pred", SPred>;
def MINvvm  : VpuInstBinSw  <0b000110, VRF, VRF, VRF,    "min$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MINvsm  : VpuInstBinSw  <0b000110, VRF, VRF, SRF,    "min$optype $sw $dest, $op1, $op2, $pred", VPred>;
def MINvim  : VpuInstBinSwI <0b000110, VRF, VRF, i32imm, "min$optype $sw $dest, $op1, $op2, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_min, "MIN", type, type, pred>;


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
def : Pat<(and v256i1:$op1, v256i1:$op2), (ANDmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(and v128i1:$op1, v128i1:$op2), (ANDmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(and v64i1:$op1,  v64i1:$op2),  (ANDmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_and, "AND", type, type, pred>;

foreach T = [v256i1, v128i1, v64i1] in {
  def : Pat<(T (int_tpc_and    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income,     i1:$pred, (i1 imm:$polarity))),
            (T (ANDmmp      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(T (int_tpc_and    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income,    v256i1:$pred, (i1 imm:$polarity))),
            (T (ANDmmm      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}


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
def : Pat<(or v256i1:$op1, v256i1:$op2), (ORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(or v128i1:$op1, v128i1:$op2), (ORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(or v64i1:$op1, v64i1:$op2), (ORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_or, "OR", type, type, pred>;

foreach T = [v256i1, v128i1, v64i1] in {
  def : Pat<(T (int_tpc_or    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income,     i1:$pred, (i1 imm:$polarity))),
            (T (ORmmp      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(T (int_tpc_or    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income, v256i1:$pred, (i1 imm:$polarity))),
            (T (ORmmm      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}


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
def : Pat<(xor v256i1:$op1, v256i1:$op2), (XORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(xor v128i1:$op1, v128i1:$op2), (XORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(xor v64i1:$op1, v64i1:$op2), (XORmmp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : BinaryVectorIntrinsics<int_tpc_xor, "XOR", type, type, pred>;

foreach T = [v256i1, v128i1, v64i1] in {
  def : Pat<(T (int_tpc_xor    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income,     i1:$pred, (i1 imm:$polarity))),
            (T (XORmmp      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(T (int_tpc_xor    T:$op1,    T:$op2, (i8 imm:$optype), (i32 imm:$sw),    T:$income,    v256i1:$pred, (i1 imm:$polarity))),
            (T (XORmmm      VPRF:$op1, VPRF:$op2, (i8 imm:$optype), (i32 imm:$sw), VPRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}


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

def ASHvvpRhaz : VpuInstBinSwD <0b010101, VRF, DRF, VRF, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def ASHvvmRhaz : VpuInstBinSwD <0b010101, VRF, DRF, VRF, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def ASHvspRhaz : VpuInstBinSwD <0b010101, VRF, DRF, SRF, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def ASHvsmRhaz : VpuInstBinSwD <0b010101, VRF, DRF, SRF, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def ASHvipRhaz : VpuInstBinSwDI <0b010101, VRF, DRF, i8imm, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def ASHvimRhaz : VpuInstBinSwDI <0b010101, VRF, DRF, i8imm, "ash$optype $sw $rhaz $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;

class ConstructAsh<string suffix, ValueType T> {
  string Name = !strconcat("ASH",
                           suffix,
                           TypeMap<T>.Suffix,
                           "Rhaz");
  Instruction Insn = !cast<Instruction>(Name);
}

foreach type = [v256i8, v128i16, v64i32] in
  foreach pred = [i1, v256i1] in
    defm : ShiftVectorIntrinsics<int_tpc_ash, "ASH", type, v256i8, pred>;

foreach type = [v256i8, v128i16, v64i32] in
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_ash DoubleVector<type>.Type:$op1, v256i8:$op2, (i8 imm:$optype), (i32 imm:$sw),
                                 type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructAsh<"vv", pred>.Insn DRF:$op1, VRF:$op2, (i8 imm:$optype),
                                 (i32 imm:$sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

    def : Pat<(type (int_tpc_ash DoubleVector<type>.Type:$op1, i8:$op2, (i8 imm:$optype), (i32 imm:$sw),
                                 type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructAsh<"vs", pred>.Insn DRF:$op1, SRF:$op2, (i8 imm:$optype),
                                 (i32 imm:$sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

   def : Pat<(type (int_tpc_ash DoubleVector<type>.Type:$op1,
                                (v256i8 (SplatNode<v256i8>.Splat i8:$op2)), (i8 imm:$optype), (i32 imm:$sw),
                                type:$income, pred:$pred, (i1 imm:$polarity))),
             (type (ConstructAsh<"vs", pred>.Insn DRF:$op1, SRF:$op2, (i8 imm:$optype),
                                (i32 imm:$sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

    def : Pat<(type (int_tpc_ash DoubleVector<type>.Type:$op1, (i8 imm:$op2), (i8 imm:$optype), (i32 imm:$sw),
                                 type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructAsh<"vi", pred>.Insn DRF:$op1, (i8 imm:$op2), (i8 imm:$optype),
                                 (i32 imm:$sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

    def : Pat<(type (int_tpc_ash DoubleVector<type>.Type:$op1,
                                 (v256i8 (SplatNode<v256i8>.Splat (i8 imm:$op2))), (i8 imm:$optype), (i32 imm:$sw),
                                 type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructAsh<"vi", pred>.Insn DRF:$op1, (i8 imm:$op2), (i8 imm:$optype),
                                 (i32 imm:$sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  }


//------ NOT -------------------------------------------------------------------

def NOTvvp  : VpuInst_UnarySw<0b010010, VRF,  VRF,  "not$optype $sw $dest, $op, $pred", SPred>;
def NOTvvm  : VpuInst_UnarySw<0b010010, VRF,  VRF,  "not$optype $sw $dest, $op, $pred", VPred>;
def NOTmmp  : VpuInst_UnarySw<0b010010, VPRF, VPRF, "not$optype $sw $dest, $op, $pred", SPred>;
def NOTmmm  : VpuInst_UnarySw<0b010010, VPRF, VPRF, "not$optype $sw $dest, $op, $pred", VPred>;

defm : UnaryIntegerVector<inot,  NOTvvp>;
defm : UnaryFloatVector<fpnot, NOTvvp>;

def : Pat<(xor v256i1:$src, (vsplatb256 (i1 -1))), (NOTmmp VPRF:$src, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(xor v128i1:$src, (vsplatb128 (i1 -1))), (NOTmmp VPRF:$src, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(xor v64i1:$src,  (vsplatb64  (i1 -1))), (NOTmmp VPRF:$src, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8, v256i1, v128i1, v64i1] in
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

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
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

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    defm : ShiftVectorIntrinsics<int_tpc_shl, "SHL", type, FloatToInteger<type>.Type, pred>;


//------ CMP_EQ ----------------------------------------------------------------

def CMP_EQvvp : VpuInstBinSw <0b001001, VPRF, VRF, VRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvip : VpuInstBinSwI<0b001001, VPRF, VRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvsp : VpuInstBinSw <0b001001, VPRF, VRF, SRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_EQvvm : VpuInstBinSw <0b001001, VPRF, VRF, VRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_EQvim : VpuInstBinSwI<0b001001, VPRF, VRF, i32imm, "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_EQvsm : VpuInstBinSw <0b001001, VPRF, VRF, SRF,    "cmp_eq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<seteq, CMP_EQvvp, CMP_EQvsp, CMP_EQvip>;
defm : FCmpOpVect<setoeq, CMP_EQvvp, CMP_EQvsp, CMP_EQvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_eq, "CMP_EQ", type, type, pred, v256i1>;
  }
foreach T = [v256i8, v128i16, v64i32] in {
  def : Pat<(VectorPredicate<T>.Type (seteq (and T:$x0, T:$x1), (T (SplatNode<T>.Splat (VectorElement<T>.Type 0))))),
            (ConstructInsn<"CMP_EQ", T, T, i1>.Insn T:$x0, T:$x1, OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen2Plus]>;
  def : Pat<(VectorPredicate<T>.Type (seteq (and T:$x0, (T ImmNode<T>.Node:$x1)), (T (SplatNode<T>.Splat (VectorElement<T>.Type 0))))),
            (ConstructInsn<"CMP_EQ", T, iAny, i1>.Insn T:$x0, (CastToImm32 $x1), OpTypeForType<T>.V, (i32 SW.MASK_EQ_ZERO), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen2Plus]>;
}


//------ CMP_NEQ ---------------------------------------------------------------

def CMP_NEQvvp : VpuInstBinSw <0b001010, VPRF, VRF, VRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvip : VpuInstBinSwI<0b001010, VPRF, VRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvsp : VpuInstBinSw <0b001010, VPRF, VRF, SRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_NEQvvm : VpuInstBinSw <0b001010, VPRF, VRF, VRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_NEQvim : VpuInstBinSwI<0b001010, VPRF, VRF, i32imm, "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_NEQvsm : VpuInstBinSw <0b001010, VPRF, VRF, SRF,    "cmp_neq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<setne, CMP_NEQvvp, CMP_NEQvsp, CMP_NEQvip>;
defm : FCmpOpVect<setone, CMP_NEQvvp, CMP_NEQvsp, CMP_NEQvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_neq, "CMP_NEQ", type, type, pred, v256i1>;
  }


//------ CMP_LESS --------------------------------------------------------------

def CMP_LESSvvp : VpuInstBinSw <0b001011, VPRF, VRF, VRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvip : VpuInstBinSwI<0b001011, VPRF, VRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvsp : VpuInstBinSw <0b001011, VPRF, VRF, SRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LESSvvm : VpuInstBinSw <0b001011, VPRF, VRF, VRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LESSvim : VpuInstBinSwI<0b001011, VPRF, VRF, i32imm, "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LESSvsm : VpuInstBinSw <0b001011, VPRF, VRF, SRF,    "cmp_less$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<setlt, CMP_LESSvvp, CMP_LESSvsp, CMP_LESSvip>;
defm : CmpOpUVect<setult, CMP_LESSvvp, CMP_LESSvsp, CMP_LESSvip>;
defm : FCmpOpVect<setolt, CMP_LESSvvp, CMP_LESSvsp, CMP_LESSvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_less, "CMP_LESS", type, type, pred, v256i1>;
  }


//------ CMP_LEQ ---------------------------------------------------------------

def CMP_LEQvvp : VpuInstBinSw <0b001100, VPRF, VRF, VRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvip : VpuInstBinSwI<0b001100, VPRF, VRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvsp : VpuInstBinSw <0b001100, VPRF, VRF, SRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_LEQvvm : VpuInstBinSw <0b001100, VPRF, VRF, VRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LEQvim : VpuInstBinSwI<0b001100, VPRF, VRF, i32imm, "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_LEQvsm : VpuInstBinSw <0b001100, VPRF, VRF, SRF,    "cmp_leq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<setle, CMP_LEQvvp, CMP_LEQvsp, CMP_LEQvip>;
defm : CmpOpUVect<setule, CMP_LEQvvp, CMP_LEQvsp, CMP_LEQvip>;
defm : FCmpOpVect<setle, CMP_LEQvvp, CMP_LEQvsp, CMP_LEQvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_leq, "CMP_LEQ", type, type, pred, v256i1>;
  }


//------ CMP_GRT ---------------------------------------------------------------

def CMP_GRTvvp : VpuInstBinSw <0b001101, VPRF, VRF, VRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvip : VpuInstBinSwI<0b001101, VPRF, VRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvsp : VpuInstBinSw <0b001101, VPRF, VRF, SRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GRTvvm : VpuInstBinSw <0b001101, VPRF, VRF, VRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GRTvim : VpuInstBinSwI<0b001101, VPRF, VRF, i32imm, "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GRTvsm : VpuInstBinSw <0b001101, VPRF, VRF, SRF,    "cmp_grt$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<setgt, CMP_GRTvvp, CMP_GRTvsp, CMP_GRTvip>;
defm : CmpOpUVect<setugt, CMP_GRTvvp, CMP_GRTvsp, CMP_GRTvip>;
defm : FCmpOpVect<setogt, CMP_GRTvvp, CMP_GRTvsp, CMP_GRTvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_grt, "CMP_GRT", type, type, pred, v256i1>;
  }


//------ CMP_GEQ ---------------------------------------------------------------

def CMP_GEQvvp : VpuInstBinSw <0b001110, VPRF, VRF, VRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvip : VpuInstBinSwI<0b001110, VPRF, VRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvsp : VpuInstBinSw <0b001110, VPRF, VRF, SRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", SPred>;
def CMP_GEQvvm : VpuInstBinSw <0b001110, VPRF, VRF, VRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GEQvim : VpuInstBinSwI<0b001110, VPRF, VRF, i32imm, "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;
def CMP_GEQvsm : VpuInstBinSw <0b001110, VPRF, VRF, SRF,    "cmp_geq$optype $sw $dest, $op1, $op2, $pred", VPred>;

defm : CmpOpVect<setge, CMP_GEQvvp, CMP_GEQvsp, CMP_GEQvip>;
defm : CmpOpUVect<setuge, CMP_GEQvvp, CMP_GEQvsp, CMP_GEQvip>;
defm : FCmpOpVect<setoge, CMP_GEQvvp, CMP_GEQvsp, CMP_GEQvip>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in {
    defm : BinaryVectorIntrinsics<int_tpc_cmp_geq, "CMP_GEQ", type, type, pred, v256i1>;
  }


//------ CONVERT ---------------------------------------------------------------

def CONVERTvvp : VpuInstConvert<0b010110, VRF, VRF, "convert$optype $sw $dst, $src, $pred", SPred>;
def CONVERTvvm : VpuInstConvert<0b010110, VRF, VRF, "convert$optype $sw $dst, $src, $pred", VPred>;

def CONVERTvdp : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen2]>;
def CONVERTvdm : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen2]>;

def CONVERTdvp : VpuInstConvert<0b010110, DRF, VRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen2Plus]>;
def CONVERTdvm : VpuInstConvert<0b010110, DRF, VRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen2Plus]>;

def CONVERTg2bvdp : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen2B]>;
def CONVERTg2bvdm : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen2B]>;

def CONVERTg3vdp : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def CONVERTg3vdm : VpuInstConvert<0b010110, VRF, DRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;
def CONVERTg3vap : VpuInstConvert<0b010110, VRF, ARF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def CONVERTg3vam : VpuInstConvert<0b010110, VRF, ARF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;
def CONVERTg3avp : VpuInstConvert<0b010110, ARF, VRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def CONVERTg3avm : VpuInstConvert<0b010110, ARF, VRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;
def CONVERTg3ddp : VpuInstConvert<0b010110, DRF, DRF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def CONVERTg3ddm : VpuInstConvert<0b010110, DRF, DRF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;
def CONVERTg3aap : VpuInstConvert<0b010110, ARF, ARF, "convert$optype $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def CONVERTg3aam : VpuInstConvert<0b010110, ARF, ARF, "convert$optype $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;

def : Pat<(v64f32 (sint_to_fp v64i32:$src)),
          (CONVERTvvp $src, OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT),  (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(v64f32 (uint_to_fp v64i32:$src)),
/*FIXME???*/          (CONVERTvvp (ANDvip v64i32:$src, (i32 0x7fffffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT32, !or(SwitchVal.TO_FP32, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

/* Float to UInt/Int */

foreach type = [v64f32, v128f32, v256f32, v256bf16] in
 foreach res_type = ConvertCompatibleTypes<type>.Ts in {
   /* Convert to Signed Integer */
   def : Pat<(res_type (fp_to_sint type:$src)),
             (ConstructConvertInsn<res_type, type, i1>.Insn
             $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

   /* Convert to Unsigned Integer */
   def : Pat<(res_type (fp_to_uint type:$src)),
             (ConstructConvertInsn<res_type, res_type, i1>.Insn
               (ConstructConvertInsn<res_type, type, i1>.Insn
               $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
             OpTypeForType<res_type>.V, UnsignedSwitchValForType<res_type>.U, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

// FP16 for Gen3
foreach type = [v256f16] in
 foreach res_type = ConvertCompatibleTypes<type>.Ts in {
   /* Convert to Signed Integer */
   def : Pat<(res_type (fp_to_sint type:$src)),
             (ConstructConvertInsn<res_type, type, i1>.Insn
             $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3]>;

   /* Convert to Unsigned Integer */
   def : Pat<(res_type (fp_to_uint type:$src)),
             (ConstructConvertInsn<res_type, res_type, i1>.Insn
               (ConstructConvertInsn<res_type, type, i1>.Insn
               $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
             OpTypeForType<res_type>.V, UnsignedSwitchValForType<res_type>.U, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3]>;
}

// SINT/UINT To FP16
// FP16 for Gen3
foreach type = [v256i16] in
 foreach res_type = ConvertCompatibleTypes<type>.Ts in {
   /* Convert to Signed Integer */
   def : Pat<(res_type (sint_to_fp type:$src)),
             (ConstructConvertInsn<res_type, type, i1>.Insn
             $src, OpTypeForType<type>.V, !or(SwitchValTypeForType<res_type>.S, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>, Requires<[Gen3]>;
}

//------ FCLASS ----------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  let isNotUsedInDisasm = 1 in {
    def FCLASSvsp : VpuInst_UnarySw<0b011110, VRF, SRF, "fclass$optype $sw $dest, $op, $pred", SPred>, MultiSlotDoron1<"LD_FCLASSvsp">;
    def FCLASSvsm : VpuInst_UnarySw<0b011110, VRF, SRF, "fclass$optype $sw $dest, $op, $pred", VPred>, MultiSlotDoron1<"LD_FCLASSvsm">;
    def FCLASSvvp : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", SPred>, MultiSlotDoron1<"LD_FCLASSvvp">;
    def FCLASSvvm : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", VPred>, MultiSlotDoron1<"LD_FCLASSvvm">;
  }
}

foreach type = [v64f32, v128bf16, v128f16, v128f16, v256f8_143, v256f8_152] in {
  foreach pred = [i1, v256i1] in {
    def : Pat<(type (int_tpc_fclass type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"FCLASSvv", pred>.Insn VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(type (int_tpc_fclass VectorElement<type>.Type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"FCLASSvs", pred>.Insn SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat<(type (int_tpc_fclass (type (SplatNode<type>.Splat VectorElement<type>.Type:$src)), (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"FCLASSvs", pred>.Insn SRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
  }
}

//------ FCLASS_LIMIT ----------------------------------------------------------

let Predicates = [Gen3Plus] in {
  def FCLASS_LIMITvvvvp : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
  def FCLASS_LIMITvsvvp : VpuInst_FCLASS_LIMIT<0b011110, VRF, SRF, VRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
  def FCLASS_LIMITvvsvp : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, SRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
  def FCLASS_LIMITvvvsp : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, SRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
  def FCLASS_LIMITvvvvm : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
  def FCLASS_LIMITvsvvm : VpuInst_FCLASS_LIMIT<0b011110, VRF, SRF, VRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
  def FCLASS_LIMITvvsvm : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, SRF, VRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
  def FCLASS_LIMITvvvsm : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, SRF, "fclass$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;

  foreach Type = [v64f32, v128bf16, v128f16, v128f16, v256f8_143, v256f8_152] in {
    foreach Pred = [i1, v256i1] in {
      def : Pat<(Type (int_tpc_fclass_limit Type:$src1, Type:$src2, Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvvvv", Pred>.Insn VRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;

      def : Pat<(Type (int_tpc_fclass_limit VectorElement<Type>.Type:$src1, Type:$src2, Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvsvv", Pred>.Insn SRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(Type (int_tpc_fclass_limit Type:$src1, VectorElement<Type>.Type:$src2, Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvvsv", Pred>.Insn VRF:$src1, SRF:$src2, VRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(Type (int_tpc_fclass_limit Type:$src1, Type:$src2, VectorElement<Type>.Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvvvs", Pred>.Insn VRF:$src1, VRF:$src2, SRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;

      def : Pat<(Type (int_tpc_fclass_limit (Type (SplatNode<Type>.Splat VectorElement<Type>.Type:$src1)),
                  Type:$src2, Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvsvv", Pred>.Insn SRF:$src1, VRF:$src2, VRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(Type (int_tpc_fclass_limit Type:$src1, (Type (SplatNode<Type>.Splat VectorElement<Type>.Type:$src2)),
                  Type:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvvsv", Pred>.Insn VRF:$src1, SRF:$src2, VRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;
      def : Pat<(Type (int_tpc_fclass_limit Type:$src1, Type:$src2, (Type (SplatNode<Type>.Splat VectorElement<Type>.Type:$src3)),
                  (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity))),
                (Type (ConstructInsn<"FCLASS_LIMITvvvs", Pred>.Insn VRF:$src1, VRF:$src2, SRF:$src3, (i8 imm:$optype), (i32 imm:$sw), Type:$income, Pred:$pred, (i1 imm:$polarity)))>;
    }
  }

}


//------ POPCNT ----------------------------------------------------------------

def POPCNTvp : VpuInst_UnarySw<0b011001, VRF, VRF, "popcnt$optype $sw $dest, $op, $pred", SPred>;
def POPCNTvm : VpuInst_UnarySw<0b011001, VRF, VRF, "popcnt$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v128f16, v256f8_143,v256f8_152] in {
  def : Pat<(v256i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income,     i1:$pred, (i1 imm:$polarity))),
            (v256i8 (POPCNTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i8 (int_tpc_popcnt type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income, v256i1:$pred, (i1 imm:$polarity))),
            (v256i8 (POPCNTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}

//llvm.ctpop
  def : Pat<(ctpop v256i8:$src),
            (v256i8 (POPCNTvp $src, (i8 4), (i32 1), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ FIND_FIRST ------------------------------------------------------------

def FIND_FIRSTvp : VpuInst_UnarySw<0b011010, VRF, VRF, "find_first$optype $sw $dest, $op, $pred", SPred>;
def FIND_FIRSTvm : VpuInst_UnarySw<0b011010, VRF, VRF, "find_first$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v128f16, v256f8_143,v256f8_152] in {
  def : Pat<(v256i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income,     i1:$pred, (i1 imm:$polarity))),
            (v256i8 (FIND_FIRSTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i8 (int_tpc_find_first type:$src, (i8 imm:$optype), (i32 imm:$sw), v256i8:$income, v256i1:$pred, (i1 imm:$polarity))),
            (v256i8 (FIND_FIRSTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),    VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}
  //llvm ctlz
  def : Pat<(ctlz v256i8:$src),
            (v256i8 (FIND_FIRSTvp $src, (i8 4), (i32 1), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  //llvm cttz
  def : Pat<(cttz v256i8:$src),
            (v256i8 (FIND_FIRSTvp $src, (i8 4), (i32 3), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


 // llvm fminnum
def : Pat<(fminnum v64f32:$v0, v64f32:$v1),
      (v64f32(MINvvp $v0,  $v1, (i8 0), (i32 0),(IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
 // llvm fmaxnum
def : Pat<(fmaxnum v64f32:$v0, v64f32:$v1),
      (v64f32(MAXvvp $v0,  $v1, (i8 0), (i32 0),(IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ NEARBYINT -------------------------------------------------------------

def NEARBYINTvp : VpuInst_NEARBYINT<0b011011, VRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", SPred>;
def NEARBYINTvm : VpuInst_NEARBYINT<0b011011, VRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", VPred>;

def NEARBYINT_CNVRTvp : VpuInst_NEARBYINT<0b011011, DRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", SPred>;
def NEARBYINT_CNVRTvm : VpuInst_NEARBYINT<0b011011, DRF, VRF, "nearbyint$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income,     i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTvp        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),  VRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (type (NEARBYINTvm        VRF:$src, (i8 imm:$optype), (i32 imm:$sw),  VRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}

foreach type = [v64f32, v128bf16, v128f16] in {
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income,     i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvp  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>;
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvm  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>;
}

foreach type = [v256f8_143, v256f8_152] in {
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income,     i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvp  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   SPRF:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvm  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   VPRF:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
}

foreach type = [v256f8_143, v256f8_152] in {
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income,     i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvp  VRF:$src, (i8 imm:$optype), (UpperLaneSwitch $sw),
            (NEARBYINT_CNVRTvp  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   SPRF:$pred, (i1 imm:$polarity)),   SPRF:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
  def : Pat<(DoubleVector<type>.Type (int_tpc_nearbyint type:$src, (i8 imm:$optype), (i32 imm:$sw), DoubleVector<type>.Type:$income, v256i1:$pred, (i1 imm:$polarity))),
            (DoubleVector<type>.Type (NEARBYINT_CNVRTvm  VRF:$src, (i8 imm:$optype), (UpperLaneSwitch $sw),
            (NEARBYINT_CNVRTvm  VRF:$src, (i8 imm:$optype), (i32 imm:$sw), DRF:$income,   VPRF:$pred, (i1 imm:$polarity)),   VPRF:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
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
let Predicates = [FP16] in
  foreach ptype = [i1, v256i1] in {
    def : Pat<(v128i16 (int_tpc_extract_exp v128f16:$src, (i8 imm:$optype), (i32 imm:$biased), v128i16:$income, ptype:$pred, (i1 imm:$polarity))),
              (v128i16 (ConstructInsn<"EXTRACT_EXPvv", ptype>.Insn
                        VRF:$src, (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
    def : Pat<(v128i16 (int_tpc_extract_exp (v128f16 (vsplatf16 (f16 fpimm:$src))), (i8 imm:$optype), (i32 imm:$biased), v128i16:$income, ptype:$pred, (i1 imm:$polarity))),
              (v128i16 (ConstructInsn<"EXTRACT_EXPvi", ptype>.Insn
                        (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
  }
let Predicates = [FP8] in
  foreach ptype = [i1, v256i1] in {
    def : Pat<(v256i8 (int_tpc_extract_exp v256f8_143:$src, (i8 imm:$optype), (i32 imm:$biased), v256i8:$income, ptype:$pred, (i1 imm:$polarity))),
              (v256i8 (ConstructInsn<"EXTRACT_EXPvv", ptype>.Insn
                       VRF:$src, (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
    def : Pat<(v256i8 (int_tpc_extract_exp (v256f8_143 (vsplatf8_143 (f8_143 fpimm:$src))), (i8 imm:$optype), (i32 imm:$biased), v256i8:$income, ptype:$pred, (i1 imm:$polarity))),
              (v256i8 (ConstructInsn<"EXTRACT_EXPvi", ptype>.Insn
                       (FPToImm32 $src), (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
    def : Pat<(v256i8 (int_tpc_extract_exp v256f8_152:$src, (i8 imm:$optype), (i32 imm:$biased), v256i8:$income, ptype:$pred, (i1 imm:$polarity))),
              (v256i8 (ConstructInsn<"EXTRACT_EXPvv", ptype>.Insn
                       VRF:$src, (i8 imm:$optype), (i32 imm:$biased), VRF:$income, ptype:$pred, (i1 imm:$polarity)))>;
    def : Pat<(v256i8 (int_tpc_extract_exp (v256f8_152 (vsplatf8_152 (f8_152 fpimm:$src))), (i8 imm:$optype), (i32 imm:$biased), v256i8:$income, ptype:$pred, (i1 imm:$polarity))),
              (v256i8 (ConstructInsn<"EXTRACT_EXPvi", ptype>.Insn
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
                       !if(!eq(!cast<string>(T), "v128i16"),    [v128i16, v128bf16, v128f16],
                       !if(!eq(!cast<string>(T), "v128bf16"),   [v128bf16, v128i16, v128f16],
                       !if(!eq(!cast<string>(T), "v128f16"),    [v128f16, v128i16, v128bf16],
                       !if(!eq(!cast<string>(T), "v256i8"),     [v256i8, v256f8_143, v256f8_152],
                       !if(!eq(!cast<string>(T), "v256f8_143"), [v256i8, v256f8_143, v256f8_152],
                       !if(!eq(!cast<string>(T), "v256f8_152"), [v256i8, v256f8_143, v256f8_152],
                       []))))))));
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

foreach T1 = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
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
foreach type = [v256i8, v128i16, v64i32, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
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

// For doron1 it is needed other patterns(lower/upper lanes split), so here patterns were removed
// Implemented by any-any way besides i8->i4

// foreach pred = [i1, v256i1] in {
//   foreach res_type = [v256i8, v128i16] in {
//     defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32",   res_type,  v64i32, v256i8, pred>, Requires<[Gen1]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen2]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen3Plus]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g2", res_type,  v64i32, v64i32, pred>, Requires<[Gen3Plus]>;
//   }
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g3i16", v128i16, v128i32, v128i16, pred>, Requires<[Gen3Plus]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT32g3i8",  v256i8,  v256i32, v256i8,  pred>, Requires<[Gen3Plus]>;
// }

defm CONVERT_UINT32      : CONVERT_INTG1<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g2    : CONVERT_INTG2<0b011000, VRF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g3i16 : CONVERT_INTG2<0b011000, DRF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT32g3i8  : CONVERT_INTG2<0b011000, ARF, "convert_uint32 $sw $dest, $op1, $op2, $pred">;

// foreach pred = [i1, v256i1] in {
//   foreach res_type = [v256i8, v128i16] in {
//     defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32",   res_type,  v64i32, v256i8, pred>, Requires<[Gen1]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen2]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g2", res_type,  v64i32, v64i32, pred>, Requires<[Gen3Plus]>;
//     defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g2", res_type,  v64i32, v256i8, pred>, Requires<[Gen3Plus]>;
//   }
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g3i16", v128i16, v128i32, v128i16, pred>, Requires<[Gen3Plus]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT32g3i8",  v256i8,  v256i32, v256i8,  pred>, Requires<[Gen3Plus]>;
// }


defm CONVERT_INT16    : CONVERT_INTG1<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT16g2  : CONVERT_INTG2<0b000100, VRF, "convert_int16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_INT16g3A : CONVERT_INTG2<0b000100, DRF, "convert_int16 $sw $dest, $op1, $op2, $pred">;

// foreach pred = [i1, v256i1] in {
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16",    v256i8, v128i16, v256i8,  pred>, Requires<[Gen1]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16g2",  v256i8, v128i16, v256i8,  pred>, Requires<[Gen2]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16g2",  v256i8, v128i16, v128i16, pred>, Requires<[Gen3Plus]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16g2",  v256i8, v128i16, v256i8,  pred>, Requires<[Gen3Plus]>;  //formed in preshaper
//   defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT16g3A", v256i8, v256i16, v256i8 , pred>, Requires<[Gen3Plus]>;
// }
 

defm CONVERT_UINT16    : CONVERT_INTG1<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT16g2  : CONVERT_INTG2<0b011100, VRF, "convert_uint16 $sw $dest, $op1, $op2, $pred">;
defm CONVERT_UINT16g3A : CONVERT_INTG2<0b011100, DRF, "convert_uint16 $sw $dest, $op1, $op2, $pred">;

// foreach pred = [i1, v256i1] in {
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16",    v256i8, v128i16, v256i8,  pred>, Requires<[Gen1]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16g2",  v256i8, v128i16, v256i8,  pred>, Requires<[Gen2]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16g2",  v256i8, v128i16, v128i16, pred>, Requires<[Gen3Plus]>;
//   defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT16g3A", v256i8, v256i16, v256i8,  pred>, Requires<[Gen3Plus]>;
// }

defm CONVERT_INT8   : CONVERT_INTG2<0b110101, DRF, "convert_int8 $sw $dest, $op1, $op2, $pred">;
 
foreach pred = [i1, v256i1] in {
  defm : ConvertIntIntrinsics<int_tpc_convert_int, "CONVERT_INT8", v256i8, v512i8, v256i8, pred>, Requires<[Gen3Plus]>;
}

defm CONVERT_UINT8   : CONVERT_INTG2<0b110110, DRF, "convert_uint8 $sw $dest, $op1, $op2, $pred">;

foreach pred = [i1, v256i1] in {
  defm : ConvertIntIntrinsics<int_tpc_convert_uint, "CONVERT_UINT8", v256i8, v512i8, v256i8, pred>, Requires<[Gen3Plus]>;
}

//------ CONVERT_FP_FLEX -------------------------------------------------------

def CONVERT_FP_FLEXvvvp  : VpuInst_CONVERT_FP_FLEX<0b111000, VRF, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", SPred>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXvvsp  : VpuInst_CONVERT_FP_FLEX<0b111000, SRF, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", SPred>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXvvip  : VpuInst_CONVERT_FP_FLEX<0b111000, TPCImm<i32imm>, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", SPred>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXvvvm  : VpuInst_CONVERT_FP_FLEX<0b111000, VRF, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", VPred>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXvvsm  : VpuInst_CONVERT_FP_FLEX<0b111000, SRF, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", VPred>, Requires<[ConvertFpFlex]>;
def CONVERT_FP_FLEXvvim  : VpuInst_CONVERT_FP_FLEX<0b111000, TPCImm<i32imm>, "convert_fp_flex$optype $sw $dest, $op1, $op2, $pred", VPred>, Requires<[ConvertFpFlex]>;

multiclass VpuConvertFpFlex<ValueType ToType, ValueType FromType, ValueType ControlType, ValueType Predicate> {
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1, ControlType:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity))),
            (ToType (ConstructInsn<"CONVERT_FP_FLEXvvv", Predicate>.Insn
             FromType:$src1, VRF:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1,
             VectorElement<ControlType>.Type:$src2,
             (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity))),
            (ToType (ConstructInsn<"CONVERT_FP_FLEXvvs", Predicate>.Insn
             FromType:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1,
             (ControlType (SplatNode<ControlType>.Splat VectorElement<ControlType>.Type:$src2)),
             (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity))),
            (ToType (ConstructInsn<"CONVERT_FP_FLEXvvs", Predicate>.Insn
             FromType:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1,
             (VectorElement<ControlType>.Type imm:$src2),
             (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity))),
            (ToType (ConstructInsn<"CONVERT_FP_FLEXvvi", Predicate>.Insn
             FromType:$src1, (CastToImm32 $src2), (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
  def : Pat<(ToType (int_tpc_convert_fp_flex FromType:$src1,
             (SplatNode<ControlType>.Splat (VectorElement<ControlType>.Type imm:$src2)),
             (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity))),
            (ToType (ConstructInsn<"CONVERT_FP_FLEXvvi", Predicate>.Insn
             FromType:$src1, (CastToImm32 $src2), (i8 imm:$optype), (i32 imm:$sw), ToType:$income, Predicate:$pred, (i1 imm:$polarity)))>, Requires<[ConvertFpFlex]>;
}

foreach pred = [i1, v256i1] in {
    defm : VpuConvertFpFlex<v64f32, v128f16, v128i16, pred>;
    defm : VpuConvertFpFlex<v128f16, v64f32, v128i16, pred>;
}


//------ BREV ------------------------------------------------------------------

let Predicates = [Gen2Plus] in {
  def BREVvvp : VpuInst_UnarySw<0b011111, VRF, VRF, "brev$optype $sw $dest, $op, $pred", SPred>;
  def BREVvvm : VpuInst_UnarySw<0b011111, VRF, VRF, "brev$optype $sw $dest, $op, $pred", VPred>;
  def BREVvsp : VpuInst_UnarySw<0b011111, VRF, SRF, "brev$optype $sw $dest, $op, $pred", SPred>;
  def BREVvsm : VpuInst_UnarySw<0b011111, VRF, SRF, "brev$optype $sw $dest, $op, $pred", VPred>;

  foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
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

foreach type = [v128bf16, v128f16, v128i16, v256i8, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    def : Pat <(type (int_tpc_pack type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"PACK", pred>.Insn VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;


//------ UNPACK ----------------------------------------------------------------

def UNPACKp : VpuInst_PACK<0b101110, VRF, VRF, "unpack$optype $sw $dest, $op, $pred", SPred>;
def UNPACKm : VpuInst_PACK<0b101110, VRF, VRF, "unpack$optype $sw $dest, $op, $pred", VPred>;

foreach type = [v128bf16, v128f16, v128i16, v256i8, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    def : Pat <(type (int_tpc_unpack type:$src, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"UNPACK", pred>.Insn VRF:$src, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;


//------ SHUFFLE ---------------------------------------------------------------

def SHUFFLEvvp : VpuInstBinSw<0b101100, VRF, VRF, VRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", SPred>;
def SHUFFLEvvm : VpuInstBinSw<0b101100, VRF, VRF, VRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", VPred>;
def SHUFFLEvsp : VpuInstBinSw<0b101100, VRF, VRF, SRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", SPred>, Requires<[Gen4Plus]>;
def SHUFFLEvsm : VpuInstBinSw<0b101100, VRF, VRF, SRF,            "shuffle$optype $sw $dest, $op1, $op2, $pred", VPred>, Requires<[Gen4Plus]>;
def SHUFFLEvip : VpuInstBinSw<0b101100, VRF, VRF, TPCImm<i32imm>, "shuffle$optype $sw $dest, $op1, $op2, $pred", SPred>, Requires<[Gen4Plus]>;
def SHUFFLEvim : VpuInstBinSw<0b101100, VRF, VRF, TPCImm<i32imm>, "shuffle$optype $sw $dest, $op1, $op2, $pred", VPred>, Requires<[Gen4Plus]>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    def : Pat <(type (int_tpc_shuffle type:$src1, v256i8:$src2, (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"SHUFFLEvv", pred>.Insn VRF:$src1, VRF:$src2, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;
    def : Pat <(type (int_tpc_shuffle type:$src1, (v256i8 (vsplati8 i8:$src2)),  (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"SHUFFLEvs", pred>.Insn VRF:$src1, SRF:$src2, (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus]>;
    def : Pat <(type (int_tpc_shuffle type:$src1, (v256i8 (vsplati8 (i8 imm:$src2))), (i8 imm:$optype), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"SHUFFLEvi", pred>.Insn VRF:$src1, (CastToImm32 $src2), (i8 imm:$optype), (i32 imm:$sw), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen4Plus]>;
}

//------ GET_LUT_ENTRY_AND_INTERVAL_START --------------------------------------

def GET_LUT_ENTRYp: VpuInst_GET_LUT<0b101111, "get_lut_entry_and_interval_start$optype $sw $dest, $src, $shift, $pred", [], SPred>;
def GET_LUT_ENTRYm: VpuInst_GET_LUT<0b101111, "get_lut_entry_and_interval_start$optype $sw $dest, $src, $shift, $pred", [], VPred>;

foreach pred = [i1, v256i1] in {
  def : Pat<(v128i32 (int_tpc_get_lut_entry v64f32:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v128i32:$income, pred:$pred, (i1 imm:$polarity))),
            (v128i32 (ConstructInsn<"GET_LUT_ENTRY", pred>.Insn VRF:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v128i32:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i16 (int_tpc_get_lut_entry v128bf16:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i16 (ConstructInsn<"GET_LUT_ENTRY", pred>.Insn VRF:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity)))>;
  def : Pat<(v256i16 (int_tpc_get_lut_entry v128f16:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity))),
            (v256i16 (ConstructInsn<"GET_LUT_ENTRY", pred>.Insn VRF:$src, (i8 imm:$shift), (i8 imm:$dt), (i32 imm:$sw), v256i16:$income, pred:$pred, (i1 imm:$polarity)))>;
}


//------ FORM_FP_NUMBER --------------------------------------------------------

def FORM_FP_NUMvvvp  : VpuInst_FORM_FP_NUM<0b110000, VRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMvvvm  : VpuInst_FORM_FP_NUM<0b110000, VRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
def FORM_FP_NUMsvvp  : VpuInst_FORM_FP_NUM<0b110000, SRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMsvvm  : VpuInst_FORM_FP_NUM<0b110000, SRF,            "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;
def FORM_FP_NUMivvp  : VpuInst_FORM_FP_NUM<0b110000, TPCImm<i32imm>, "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def FORM_FP_NUMivvm  : VpuInst_FORM_FP_NUM<0b110000, TPCImm<i32imm>, "form_fp_num$optype $sw $dest, $op1, $op2, $op3, $pred", VPred>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
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

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_abs type:$src, (i8 imm:$dt), (i32 imm), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"FORM_FP_NUMvvv", pred>.Insn
                     VRF:$src, VRF:$src, VRF:$src, (i8 imm:$dt), (i32 SW.FORCE_SIGN0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  def : Pat<(type (fabs type:$src)),
            (type (FORM_FP_NUMvvvp $src, $src, $src, (i8 OpTypeForType<type>.V), (i32 SW.FORCE_SIGN0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;


//------ MOV_DUAL_GROUP --------------------------------------------------------

def MOV_DUAL_GROUPp : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", SPred>;
def MOV_DUAL_GROUPm : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", VPred>;

def MOV_DUAL_GROUP_CTRL_REGp : VpuInst_MOV_DUAL_GROUP_CTRL_REG<0b110001, "mov_dg $sw $dst, $src1, $src2, $imm, $pred", SPred>, Requires<[Doron1]>;
def MOV_DUAL_GROUP_CTRL_REGm : VpuInst_MOV_DUAL_GROUP_CTRL_REG<0b110001, "mov_dg $sw $dst, $src1, $src2, $imm, $pred", VPred>, Requires<[Doron1]>;

def MOV_DUAL_GROUP_CTRL_REG_ALLp : VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src1, $src2, $imm, $pred", SPred>, Requires<[Doron1]>;
def MOV_DUAL_GROUP_CTRL_REG_ALLm : VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src1, $src2, $imm, $pred", VPred>, Requires<[Doron1]>;

def MOV_DUAL_GROUP_ALLp : VpuInst_MOV_DUAL_GROUP_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src, $imm, $pred", SPred>, Requires<[Gen2Plus]>;
def MOV_DUAL_GROUP_ALLm : VpuInst_MOV_DUAL_GROUP_ALL<0b110001, "mov_dg $movdgall $sw $dst, $src, $imm, $pred", VPred>, Requires<[Gen2Plus]>;

def MOV_DUAL_GROUP_PACKp : VpuInst_MOV_DUAL_GROUP_PACK<0b110001, "mov_dg $movdgpack $sw $dst, $src, $pred", SPred>, Requires<[Gen3Plus]>;
def MOV_DUAL_GROUP_PACKm : VpuInst_MOV_DUAL_GROUP_PACK<0b110001, "mov_dg $movdgpack $sw $dst, $src, $pred", VPred>, Requires<[Gen3Plus]>;

def MOV_DUAL_GROUP_UNPACKp : VpuInst_MOV_DUAL_GROUP_UNPACK<0b110001, "mov_dg $movdgunpack $sw $dst, $src, $imm, $pred", SPred>, Requires<[MovDGUnpack]>;
def MOV_DUAL_GROUP_UNPACKm : VpuInst_MOV_DUAL_GROUP_UNPACK<0b110001, "mov_dg $movdgunpack $sw $dst, $src, $imm, $pred", VPred>, Requires<[MovDGUnpack]>;

def AddDualGroupPackSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() & ~TPCII::SW_MDG_TYPE_MASK) | TPCII::SW_MDG_TYPE_PACK, SDLoc(N), MVT::i32);
}]>;

def AddDualGroupUnPackSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() & ~TPCII::SW_MDG_TYPE_MASK) | TPCII::SW_MDG_TYPE_UNPACK, SDLoc(N), MVT::i32);
}]>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in
  foreach pred = [i1, v256i1] in {
    // single and all cases have custom lowering
    def : Pat <(type (int_tpc_mov_dual_group_pack type:$src, (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"MOV_DUAL_GROUP_PACK", pred>.Insn
                      VRF:$src, (AddDualGroupPackSwitch $sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Gen3Plus]>;
    def : Pat <(type (int_tpc_mov_dual_group_unpack type:$src, (i32 imm:$wren), (i32 imm:$sw), type:$income, pred:$pred, (i1 imm:$polarity))),
               (type (ConstructInsn<"MOV_DUAL_GROUP_UNPACK", pred>.Insn
                      VRF:$src, (i32 imm:$wren), (AddDualGroupUnPackSwitch $sw), (i8 0), VRF:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[MovDGUnpack]>;
  }


//------ CALC_FP_SPECIAL -------------------------------------------------------

def CALC_FP_SPECIALvvp : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", SPred>, Requires<[Gen2Plus]>;
def CALC_FP_SPECIALvvm : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", VPred>, Requires<[Gen2Plus]>;

// Separate definitions to support multi-slot scheduling for single-source variant on Doron1+.
// Despite the name, reused instruction class with 2 sources (op2 is to be duplicated from op1).
// If we bother to introduce a separate class, beware to update TPCHWWAGeneral::goya2HwWA also.
let isCodeGenOnly = 1 in {
  def CALC_FP_SPECIALOneArgvvp : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", SPred>, MultiSlot<"LD_CALC_FP_SPECIALOneArgvvp">, Requires<[Doron1]>;
  def CALC_FP_SPECIALOneArgvvm : VpuInst_CALC_FP_SPECIAL<0b110100, "calc_fp_special$optype $funcId $dest, $op1, $op2, $pred", VPred>, MultiSlot<"LD_CALC_FP_SPECIALOneArgvvm">, Requires<[Doron1]>;
}

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach pred = [i1, v256i1] in
    def : Pat<(type (int_tpc_calc_fp_special type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity))),
              (type (ConstructInsn<"CALC_FP_SPECIALvv", pred>.Insn
                     type:$src1, type:$src2, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity)))>;

// Variants for one argument
foreach func = [0, 1, 2, 3, 4, 5] in // funcId (RECIP, RSQRT, SQRT, LOG, EXP, TANH)
  foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
    foreach pred = [i1, v256i1] in {
      def : Pat<(type (int_tpc_calc_fp_special type:$src1, (type undef), (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"CALC_FP_SPECIALvv", pred>.Insn
                       type:$src1, type:$src1, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[PriorDoron1]>;
      def : Pat<(type (int_tpc_calc_fp_special type:$src1, (type undef), (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity))),
                (type (ConstructInsn<"CALC_FP_SPECIALOneArgvv", pred>.Insn
                       type:$src1, type:$src1, (i8 imm:$optype), (i32 imm:$func), type:$income, pred:$pred, (i1 imm:$polarity)))>, Requires<[Doron1]>;
    }

//====== Store slot ============================================================

multiclass StoreIntrinsics<SDNode IntrinA1, SDNode IntrinA2, string IName> {
  let Predicates = [Addr2] in
    foreach pred = [i1] in
      foreach type = [v256i1, v256i8, v128i16, v64i32,
                      v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
        def : Pat<(IntrinA2 i32:$base, i32:$ofs, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, i32, i32, type, pred>.Insn
                     SRF:$base, SRF:$ofs, type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
        def : Pat<(IntrinA2 i32:$base, (i32 imm:$ofs), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity)),
                  (ConstructInsn<IName, i32, iAny, type, pred>.Insn
                     SRF:$base, (i32 imm:$ofs), type:$src, (i32 imm:$sw), pred:$pred, (i1 imm:$polarity))>;
      }
  let Predicates = [Addr1] in
    foreach pred = [i1, v256i1] in
      foreach type = [v256i1, v256i8, v128i16, v64i32,
                      v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
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
                   (MOVsip (i32 imm:$addr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                   (i32 0), type:$src, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
      def : Pat<(IntrinA1 i32:$addr, type:$src, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
                (ConstructInsn<IName, i32, iAny, type, i1>.Insn
                   SRF:$addr, (i32 0), type:$src, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
    }
}

//------ GEN_ADDR --------------------------------------------------------------

def GEN_ADDR_st  : StoreInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.DimMask>, MultiSlot <"GEN_ADDR">;
def GEN_ADDR_stT : StoreInst_GEN_ADDR<0b000000, "gen_addr $sw $dest, $tensor, $coords, $pred", TensorSrc.Reg>, Requires<[TnsrInReg]>, MultiSlot<"GEN_ADDRT">;

def : Pat<(int_tpc_gen_addr v5i32:$coord, (i8 imm:$tensor), (i32 imm:$sw), i64:$income, i1:$pred, (i1 imm:$polarity)),
          (GEN_ADDR_st (i8 imm:$tensor), IRF:$coord, (i32 imm:$sw), $income, $pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_gen_addr v5i32:$coord, ST_TNSR_ID_REG, (i32 imm:$sw), i64:$income, i1:$pred, (i1 imm:$polarity)),
          (GEN_ADDR_stT ST_TNSR_ID_REG, $coord, (i32 imm:$sw), $income, $pred, (i1 imm:$polarity))>, Requires<[TnsrInReg]>;

//------ EVENT -----------------------------------------------------------------
def EVENT_st_sp  : StoreInst_EVENT<0b011000, SRF,            "event $sw $src, $pred">, Requires<[Gen4Plus]>;
let isNotUsedInDisasm = 1 in
def EVENT_st_ip  : StoreInst_EVENT<0b011000, TPCImm<i32imm>, "event $sw $src, $pred">, Requires<[Gen4Plus]>;

def : Pat<(int_tpc_event i16:$data, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (EVENT_st_sp SRF:$data, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_event (i16 imm:$data), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (EVENT_st_ip (CastToImm32 $data), (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;

//------ PRMT_INDX -------------------------------------------------------------

def PRMT_INDX_st : StoreInstrPRMT_INDX<0b000011, "prmt_indx $sw $dest, $src1, $src2, $pred">, MultiSlot<"PRMT_INDX">;

def : Pat<(int_tpc_prmt_indx v5i32:$src1, i32:$src2, (i32 imm:$sw), v5i32:$income, i1:$pred, (i1 imm:$polarity)),
          (PRMT_INDX_st IRF:$src1, SRF:$src2, (i32 imm:$sw), IRF:$income, SPRF:$pred, (i1 imm:$polarity))>;


//------ SET_INDX --------------------------------------------------------------

def SET_INDX_st_rp  : StoreInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_rp">;
def SET_INDX_st_ip  : StoreInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Imm>, MultiSlot<"SET_INDX_ip">;
def SET_INDX_st_rwp : StoreInst_SET_INDX <0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_rwp">, Requires<[DimMaskR]>;
def SET_INDX_st_iwp : StoreInst_SET_INDXI<0b000100,  "set_indx $sw $dst, $mask, $src, $pred", DMask.Reg>, MultiSlot<"SET_INDX_iwp">, Requires<[DimMaskR]>;


//------ ST_L ------------------------------------------------------------------

def ST_Lssp : StoreInst_ST_L <0b000101, SRF,  "st_l $sw $addr, $value, $pred">;
def ST_Lisp : StoreInst_ST_LI<0b000101, SRF,  "st_l $sw $addr, $value, $pred">;
def ST_Lspp : StoreInst_ST_L <0b000101, SPRF, "st_l $sw $addr, $value, $pred">;
def ST_Lipp : StoreInst_ST_LI<0b000101, SPRF, "st_l $sw $addr, $value, $pred">;

def : Pat<(lsstore i1:$value, (i32 imm:$addr)), (ST_Lipp (CastToImm32 $addr), (i1 $value), (i32 0), SPRF_TRUE, (i1 0))>;
def : Pat<(lsstore i1:$value,       i32:$addr), (ST_Lspp $addr,               (i1 $value), (i32 0), SPRF_TRUE, (i1 0))>;
foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(lsstore type:$value, (i32 imm:$addr)), (ST_Lisp (CastToImm32 $addr), type:$value, (i32 0), SPRF_TRUE, (i1 0))>;
  def : Pat<(lsstore type:$value, i32:$addr      ), (ST_Lssp $addr,               type:$value, (i32 0), SPRF_TRUE, (i1 0))>;
}

def : Pat<(int_tpc_st_l (i32 imm:$addr), i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_Lipp (CastToImm32 $addr), i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
def : Pat<(int_tpc_st_l i32:$addr, i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_Lspp $addr, i1:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;

foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(int_tpc_st_l (i32 imm:$addr), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_Lisp (CastToImm32 $addr), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_l i32:$addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_Lssp $addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
}


//------ ST_G ------------------------------------------------------------------

def ST_Gs : StoreInst_ST_G   <0b000110, SRF,  "st_g $sw $addr, $value, $pred">;
def ST_Gz : StoreInst_ST_G64 <0b000110, ZRF,  "st_g $sw $addr, $value, $pred">, Requires<[Doron1]>;
def ST_Gp : StoreInst_ST_G   <0b000110, SPRF, "st_g $sw $addr, $value, $pred">;

def ST_G_INCs : StoreInst_ST_G_INC   <0b000110, SRF,  "st_g $sw $addr, $value, $pred">, Requires<[Gen4Plus]>;
def ST_G_INCz : StoreInst_ST_G64_INC <0b000110, ZRF,  "st_g $sw $addr, $value, $pred">, Requires<[Doron1]>;
def ST_G_INCp : StoreInst_ST_G_INC   <0b000110, SPRF, "st_g $sw $addr, $value, $pred">, Requires<[Gen4Plus]>;

foreach type = [i1, i8, i16, i32, f32, bf16, f16, f8_143, f8_152] in {
  def : Pat<(gstore type:$value, i64:$addr),
            (ConstructInsn<"ST_G", type>.Insn i64:$addr, type:$value, (i32 0), SPRF_TRUE, (i1 0))>;
  def : Pat<(gpost_store type:$value, i64:$addr, (i64 imm:$ofs)),
            (ConstructInsn<"ST_G_INC", type>.Insn i64:$addr, type:$value, (GetGPtrIncSwitch (i64 imm:$ofs)), SPRF_TRUE, (i1 0))>, Requires<[Gen4Plus]>;
}

// Patterns for intrinsics 'int_tpc_st_g_b'. See getTgtMemIntrinsic() and LowerINTRINSIC_W_CHAIN()
// in TPCISelLowering.cpp for how the node 'gstorep' is mapped to 'int_tpc_st_g_b'
foreach type = [i1, i8, i16, i32, f32, bf16, f16, f8_143, f8_152] in {
  def : Pat<(gstorep i64:$addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_G", type>.Insn ADRF:$addr, type:$value, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
  def : Pat<(gstore_incp i64:$addr, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_G_INC", type>.Insn ADRF:$addr, type:$value, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;
}

def : Pat<(gstorep i64:$addr, v2i32:$value, (i32 imm:$sw),  i1:$pred, (i1 imm:$polarity)),
          (ST_Gz ADRF:$addr, ZRF:$value, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;

def : Pat<(gstore_incp i64:$addr, v2i32:$value, (i32 imm:$sw),  i1:$pred, (i1 imm:$polarity)),
          (ST_G_INCz ADRF:$addr, ZRF:$value, (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;

//------ ST_L_V ----------------------------------------------------------------

defm ST_L_V : StoreInst_ST_L_V<0b000111, "st_l_v $sw $addr, $src, $pred">;

let Predicates = [Addr1] in
  foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152,
                  v64i32, v128i16, v256i8, v256i1, v128i1, v64i1] in {
    def : Pat<(lvstore type:$src, i32:$addr),
              (ConstructInsn<"ST_L_V", i32, type, i1>.Insn
                 i32:$addr, type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
    def : Pat<(lvstore type:$src, (i32 imm:$addr)),
              (ConstructInsn<"ST_L_V", iAny, type, i1>.Insn
                 (i32 imm:$addr), type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
  }
let Predicates = [Gen1] in
  foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152,
                  v64i32, v128i16, v256i8, v256i1, v128i1, v64i1] in {
    def : Pat<(lvstore type:$src, (i32 AddrRR:$addr)),
              (ConstructInsn<"ST_L_V", i32, i32, type, i1>.Insn
                 AddrRR:$addr, type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
    def : Pat<(lvstore type:$src, (i32 AddrRI:$addr)),
              (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                 AddrRI:$addr, type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
  }

// Helper for Gen1, which has no 1-component address.
let Predicates = [Gen1] in
  foreach type = [v64f32, v64i32, v128i16, v256i8, v256i1, v128i1, v64i1] in {
    def : Pat<(lvstore type:$src, (i32 imm:$ptr)),
               (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                  (MOVsip (i32 imm:$ptr), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  (i32 0), type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
    def : Pat<(lvstore type:$src, i32:$ptr),
               (ConstructInsn<"ST_L_V", i32, iAny, type, i1>.Insn
                  SRF:$ptr, (i32 0), type:$src, (i32 0), SPRF_TRUE, (i1 0))>;
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
  def Tvp      : StoreInst_ST_TNSR_T   <OpCode, VRF,  asmstr, 1>, Requires<[TnsrInReg]>;
  def Tmp      : StoreInst_ST_TNSR_T   <OpCode, VPRF, asmstr, 1>, Requires<[TnsrInReg]>;
}

multiclass St_Tnsr_RMW<bits<6> OpCode, string asmstr> {
  def Gen2vp  : StoreInst_ST_TNSR_RMW     <OpCode, VRF,  asmstr, 0>, Requires<[Gen2]>;
  def Gen2Tvp : StoreInst_ST_TNSR_RMW_T   <OpCode, VRF,  asmstr, 0>, Requires<[Gen2]>;
  def vp      : StoreInst_ST_TNSR_RMW     <OpCode, VRF,  asmstr, 1>, Requires<[RMW]>;
  def Tvp     : StoreInst_ST_TNSR_RMW_T   <OpCode, VRF,  asmstr, 1>, Requires<[TnsrInReg]>;
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
def ST_TNSR_PGen2RTvp : StoreInst_ST_TNSR_P_RMW_T <0b001011, VRF,  "st_tnsr $sw $tensor, $coords, $value, $rmw, $offsize, $pred", 0>, Requires<[Gen2]>;

def ST_TNSR_Pvp    : StoreInst_ST_TNSR_P         <0b001011, VRF,  "st_tnsr $sw $tensor,  $coords, $value, $offsize, $pred", 1>, Requires<[Partial]>;
def ST_TNSR_Pmp    : StoreInst_ST_TNSR_P         <0b001011, VPRF, "st_tnsr $sw $tensor,  $coords, $value, $offsize, $pred", 1>, Requires<[Partial]>;
def ST_TNSR_PTmp   : StoreInst_ST_TNSR_P_T       <0b001011, VPRF, "st_tnsr $sw $tensor,  $coords, $value, $offsize, $pred", 1>, Requires<[Partial, TnsrInReg]>;
def ST_TNSR_PTvp   : StoreInst_ST_TNSR_P_T       <0b001011, VRF,  "st_tnsr $sw $tensor,  $coords, $value, $offsize, $pred", 1>, Requires<[Partial, TnsrInReg]>;
def ST_TNSR_PDvp   : StoreInst_ST_TNSR_P_D       <0b001011, VRF,  "st_tnsr $sw $coords1, $coords2, $value, $offsize, $pred", 1>, Requires<[Direct, Partial]>;
def ST_TNSR_PDmp   : StoreInst_ST_TNSR_P_D       <0b001011, VPRF, "st_tnsr $sw $coords1, $coords2, $value, $offsize, $pred", 1>, Requires<[Direct, Partial]>;

def ST_TNSR_PRvp   : StoreInst_ST_TNSR_P_RMW     <0b001011, VRF,  "st_tnsr $sw $tensor,  $coords, $value, $rmw, $offsize, $pred", 1>, Requires<[Partial, RMW]>;
def ST_TNSR_PRTvp  : StoreInst_ST_TNSR_P_RMW_T   <0b001011, VRF,  "st_tnsr $sw $tensor,  $coords, $value, $rmw, $offsize, $pred", 1>, Requires<[Partial, RMW, TnsrInReg]>;
def ST_TNSR_PDRvp  : StoreInst_ST_TNSR_P_D_RMW   <0b001011, VRF,  "st_tnsr $sw $coords1, $coords2, $value, $rmw, $offsize, $pred", 1>, Requires<[Partial, RMW, Direct]>;

def ST_TNSR_Dvp   : StoreInst_ST_TNSR_D     <0b001011, VRF,  "st_tnsr $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;
def ST_TNSR_Dmp   : StoreInst_ST_TNSR_D     <0b001011, VPRF, "st_tnsr $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;
def ST_TNSR_RDvp  : StoreInst_ST_TNSR_RMW_D <0b001011, VRF,  "st_tnsr $sw $coords1, $coords2, $value, $rmw, $pred", 1>, Requires<[RMW, Direct]>;

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;

  def : Pat<(int_tpc_st_tnsr v5i32:$ndx, S28, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSRGen2T", type, i1>.Insn v5i32:$ndx,
             S28, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, S28, type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PGen2T", type, i1>.Insn v5i32:$ndx, S28, type:$value, i32:$offsize, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;

  def : Pat<(int_tpc_st_tnsr v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSRT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, (i8 imm:$tensor), type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_P", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial]>;
  def : Pat<(int_tpc_st_tnsr_partial v5i32:$ndx, ST_TNSR_ID_REG, type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial, TnsrInReg]>;

  def : Pat<(int_tpc_st_tnsr_direct i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_D", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Direct]>;
  def : Pat<(int_tpc_st_tnsr_partial_direct i32:$addr1, i32:$addr2, type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PD", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial, Direct]>;
}

// No RMW for v256f8_143 and VPRF
foreach type = [v64f32, v128bf16, v128f16, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
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
  def : Pat<(int_tpc_st_tnsr_rmw v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_partial_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PR", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial, RMW]>;
  def : Pat<(int_tpc_st_tnsr_partial_rmw v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PRT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial, RMW, TnsrInReg]>;

  def : Pat<(int_tpc_st_tnsr_rmw_direct i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_RD", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, Direct]>;
  def : Pat<(int_tpc_st_tnsr_partial_rmw_direct i32:$addr1, i32:$addr2, type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_PDR", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, i32:$rmw, ST_PARTIAL_REG, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Partial, RMW, Direct]>;
}

//------ ST_TNSR_LOW -----------------------------------------------------------

defm ST_TNSR_LOW   : St_Tnsr    <0b001100, "st_tnsr_low $sw $tensor, $coords, $value, $pred">;
defm ST_TNSR_LOW_R : St_Tnsr_RMW<0b001100, "st_tnsr_low $sw $tensor, $coords, $value, $rmw, $pred">;

def ST_TNSR_LOW_Dvp   : StoreInst_ST_TNSR_D   <0b001100, VRF,  "st_tnsr_low $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;
def ST_TNSR_LOW_Dmp   : StoreInst_ST_TNSR_D   <0b001100, VPRF, "st_tnsr_low $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;

def ST_TNSR_LOW_RDvp  : StoreInst_ST_TNSR_RMW_D <0b001100, VRF,  "st_tnsr_low $sw $coords1, $coords2, $value, $rmw, $pred", 1>, Requires<[RMW, Direct]>;

foreach type = [v256i1, v128i1, v64i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_low v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_tnsr_low v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOWGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_low v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOWT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_low_direct i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_D", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Direct]>;
}

foreach type = [v64f32, v128bf16, v128f16, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
            Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_low_rmw_direct i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_LOW_RD", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, Direct]>;
}

//------ ST_TNSR_HIGH ----------------------------------------------------------

defm ST_TNSR_HIGH   : St_Tnsr    <0b001101, "st_tnsr_high $sw $tensor, $coords, $value, $pred">;
defm ST_TNSR_HIGH_R : St_Tnsr_RMW<0b001101, "st_tnsr_high $sw $tensor, $coords, $value, $rmw, $pred">;

def ST_TNSR_HIGH_Dvp   : StoreInst_ST_TNSR_D   <0b001101, VRF,  "st_tnsr_high $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;
def ST_TNSR_HIGH_Dmp   : StoreInst_ST_TNSR_D   <0b001101, VPRF, "st_tnsr_high $sw $coords1, $coords2, $value, $pred">, Requires<[Direct]>;

def ST_TNSR_HIGH_RDvp  : StoreInst_ST_TNSR_RMW_D <0b001101, VRF,  "st_tnsr_high $sw $coords1, $coords2, $value, $rmw, $pred", 1>, Requires<[RMW, Direct]>;

foreach type = [v256i1, v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_high v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>;
  def : Pat<(int_tpc_st_tnsr_high v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGHGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_high v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGHT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_high_direct i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_D", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Direct]>;
}

foreach type = [v64f32, v128bf16, v128f16, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RGen2", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_R", type, i1>.Insn v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RGen2T", type, i1>.Insn v5i32:$ndx, i8:$tensor, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[Gen2]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RT", type, i1>.Insn v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, TnsrInReg]>;
  def : Pat<(int_tpc_st_tnsr_high_rmw_direct i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ConstructInsn<"ST_TNSR_HIGH_RD", type, i1>.Insn i32:$addr1, i32:$addr2, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>,
        Requires<[RMW, Direct]>;
}

//------ ST_TNSR_SQZ -----------------------------------------------------------

def ST_TNSR_SQZ     : StoreInst_ST_TNSR_SQZ      <0b010110, "st_tnsr_sqz $sw $tensor, $squeeze, $coords, $value, $pred">, Requires<[Gen4Plus]>;
def ST_TNSR_SQZ_T   : StoreInst_ST_TNSR_SQZ_T    <0b010110, "st_tnsr_sqz $sw $tensor, $squeeze, $coords, $value, $pred">, Requires<[Gen4Plus]>;
def ST_TNSR_SQZ_R   : StoreInst_ST_TNSR_SQZ_RMW  <0b010110, "st_tnsr_sqz $sw $tensor, $squeeze, $coords, $value, $rmw, $pred">, Requires<[Gen4Plus]>;
def ST_TNSR_SQZ_R_T : StoreInst_ST_TNSR_SQZ_RMW_T<0b010110, "st_tnsr_sqz $sw $tensor, $squeeze, $coords, $value, $rmw, $pred">, Requires<[Gen4Plus]>;

//Asm only
def ST_TNSR_SQZ_CNT_ONLY : StoreInst_ST_TNSR_SQZ_COUNT <0b010110, "st_tnsr_sqz $sw $squeeze, $pred">, Requires<[Doron1]>;
def ST_TNSR_SQZ_FLUSH    : StoreInst_ST_TNSR_SQZ_FLUSH <0b010110, "st_tnsr_sqz $sw $squeeze">, Requires<[Doron1]>;

foreach type = [v64f32, v64i32] in {
  def : Pat<(int_tpc_st_tnsr_sqz v5i32:$ndx, (i8 imm:$tensor), (i8 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_SQZ v5i32:$ndx, (i8 imm:$tensor), (i8 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_sqz v5i32:$ndx, ST_TNSR_ID_REG, (i8 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_SQZ_T v5i32:$ndx, ST_TNSR_ID_REG, (i8 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;

  def : Pat<(int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, (i8 imm:$tensor), (i8 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_SQZ_R v5i32:$ndx, (i8 imm:$tensor), (i8 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, ST_TNSR_ID_REG, (i8 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_SQZ_R_T v5i32:$ndx, ST_TNSR_ID_REG, (i8 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}

// Rewrite after switching to genuine i64 squeeze
// Now ST_TNSR_SQZ* have custom lowering. See TPCTargetLowering::LowerINTRINSIC_W_CHAIN.
// For avoid:
// $sqz_cntr0_lo = MOVhsp killed $s2, 0, undef $sqz_cntr0_lo(tied-def 0), $SPRF_TRUE, 0
// $sqz_cntr0_hi = COPY $sqz_cntr0_lo
/*def InitSQZ : OutPatFrag<
  (ops node:$value),
  (REG_SEQUENCE HWSqzCntr,
    (MOVhsp (MOVsip (i32 $value), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
    sub_0,
    (MOVhsp (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0), (i32 (IMPLICIT_DEF)), SPRF_TRUE, (i1 0)),
    sub_1)>;

foreach type = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(i32 (int_tpc_st_tnsr_sqz v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ v5i32:$ndx, (i8 imm:$tensor), (InitSQZ imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
  def : Pat<(i32 (int_tpc_st_tnsr_sqz v5i32:$ndx, ST_TNSR_ID_REG, (i32 imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_T v5i32:$ndx, ST_TNSR_ID_REG, (InitSQZ imm:$squeeze), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
            
  def : Pat<(i32 (int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_R v5i32:$ndx, (i8 imm:$tensor), (InitSQZ imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
  def : Pat<(i32 (int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, ST_TNSR_ID_REG, (i32 imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_R_T v5i32:$ndx, ST_TNSR_ID_REG, (InitSQZ imm:$squeeze), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
            
  def : Pat<(i32 (int_tpc_st_tnsr_sqz v5i32:$ndx, (i8 imm:$tensor), i32:$squeeze, type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ v5i32:$ndx, (i8 imm:$tensor), (i64 (COPY $squeeze)), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
  def : Pat<(i32 (int_tpc_st_tnsr_sqz v5i32:$ndx, ST_TNSR_ID_REG, i32:$squeeze, type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_T v5i32:$ndx, ST_TNSR_ID_REG, (i64 (COPY $squeeze)), type:$value, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;

  def : Pat<(i32 (int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, (i8 imm:$tensor), i32:$squeeze, type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_R v5i32:$ndx, (i8 imm:$tensor), (i64 (COPY $squeeze)), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
  def : Pat<(i32 (int_tpc_st_tnsr_sqz_rmw v5i32:$ndx, ST_TNSR_ID_REG, i32:$squeeze, type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))),
            (i32 (COPY (ST_TNSR_SQZ_R_T v5i32:$ndx, ST_TNSR_ID_REG, (i64 (COPY $squeeze)), type:$value, i32:$rmw, (i32 imm:$sw), v256i1:$pred, (i1 imm:$polarity))))>, Requires<[Gen4Plus]>;
}*/

//------ ST_TNSR_S -----------------------------------------------------------

def ST_TNSR_Ssr      : StoreInst_ST_TNSR_S<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_Sir      : StoreInst_ST_TNSR_S<0b010111, TPCImm<i32imm>, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rsr    : StoreInst_ST_TNSR_S_RMW<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rir    : StoreInst_ST_TNSR_S_RMW<0b010111, TPCImm<i32imm>, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWRr   : StoreInst_ST_TNSR_S_HWR<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWR_Rr : StoreInst_ST_TNSR_S_HWR_RMW<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $rmw, $pred", 1>, Requires<[Gen4Plus]>;

def ST_TNSR_Ssi      : StoreInst_ST_TNSR_S<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_Sii      : StoreInst_ST_TNSR_S<0b010111, TPCImm<i32imm>, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rsi    : StoreInst_ST_TNSR_S_RMW<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rii    : StoreInst_ST_TNSR_S_RMW<0b010111, TPCImm<i32imm>, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWRi   : StoreInst_ST_TNSR_S_HWR<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWR_Ri : StoreInst_ST_TNSR_S_HWR_RMW<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $rmw, $pred", 0>, Requires<[Gen4Plus]>;

foreach type = [i32, i16, i8] in {
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, ST_TNSR_ID_REG, (type imm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Sir v5i32:$ndx, ST_TNSR_ID_REG, (CastToImm32 $value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, (i8 imm:$tensor), (type imm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Sii v5i32:$ndx, (i8 imm:$tensor), (CastToImm32 $value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, ST_TNSR_ID_REG, (type imm:$value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rir v5i32:$ndx, ST_TNSR_ID_REG, (CastToImm32 $value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, (i8 imm:$tensor), (type imm:$value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rii v5i32:$ndx, (i8 imm:$tensor), (CastToImm32 $value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}
foreach type = [f32, bf16, f16, f8_143, f8_152] in {
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, ST_TNSR_ID_REG, (type fpimm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Sir v5i32:$ndx, ST_TNSR_ID_REG, (FPToImm32 $value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, (i8 imm:$tensor), (type fpimm:$value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Sii v5i32:$ndx, (i8 imm:$tensor), (FPToImm32 $value), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}
foreach type = [f32, bf16, f16, f8_143, f8_152, i32, i16, i8] in {
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Ssr v5i32:$ndx, ST_TNSR_ID_REG, type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_Ssi v5i32:$ndx, (i8 imm:$tensor), type:$value, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}
// No RMW for f8_143 !!!
foreach type = [f32, bf16, f16, f8_152] in {
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, ST_TNSR_ID_REG, (type fpimm:$value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rir v5i32:$ndx, ST_TNSR_ID_REG, (FPToImm32 $value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, (i8 imm:$tensor), (type fpimm:$value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rii v5i32:$ndx, (i8 imm:$tensor), (FPToImm32 $value), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}
foreach type = [f32, bf16, f16, f8_152, i32, i16, i8] in {
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rsr v5i32:$ndx, ST_TNSR_ID_REG, type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
  def : Pat<(int_tpc_st_tnsr_s_rmw v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
            (ST_TNSR_S_Rsi v5i32:$ndx, (i8 imm:$tensor), type:$value, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
}

// Rewrite after switching to genuine i64 squeeze
/*def : Pat<(int_tpc_st_tnsr_s_hwr v5i32:$ndx, ST_TNSR_ID_REG, (i32 imm:$hw_reg_sel), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWRr v5i32:$ndx, ST_TNSR_ID_REG, (InitSQZ imm:$hw_reg_sel), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr_rmw v5i32:$ndx, ST_TNSR_ID_REG, (i32 imm:$hw_reg_sel), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWR_Rr v5i32:$ndx, ST_TNSR_ID_REG, (InitSQZ imm:$hw_reg_sel), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$hw_reg_sel), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWRi v5i32:$ndx, (i8 imm:$tensor), (InitSQZ imm:$hw_reg_sel), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr_rmw v5i32:$ndx, (i8 imm:$tensor), (i32 imm:$hw_reg_sel), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWR_Ri v5i32:$ndx, (i8 imm:$tensor), (InitSQZ imm:$hw_reg_sel), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;

def : Pat<(int_tpc_st_tnsr_s_hwr v5i32:$ndx, ST_TNSR_ID_REG, i32:$hw_reg_sel, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWRr v5i32:$ndx, ST_TNSR_ID_REG, (i64 (COPY $hw_reg_sel)), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr_rmw v5i32:$ndx, ST_TNSR_ID_REG, i32:$hw_reg_sel, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWR_Rr v5i32:$ndx, ST_TNSR_ID_REG, (i64 (COPY $hw_reg_sel)), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr v5i32:$ndx, (i8 imm:$tensor), i32:$hw_reg_sel, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWRi v5i32:$ndx, (i8 imm:$tensor), (i64 (COPY $hw_reg_sel)), (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;
def : Pat<(int_tpc_st_tnsr_s_hwr_rmw v5i32:$ndx, (i8 imm:$tensor), i32:$hw_reg_sel, i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (ST_TNSR_S_HWR_Ri v5i32:$ndx, (i8 imm:$tensor), (i64 (COPY $hw_reg_sel)), i32:$rmw, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity))>, Requires<[Gen4Plus]>;*/

//------ LD_TNSR ---------------------------------------------------------------

multiclass Ld_Tnsr_St<bits<6> OpCode, string op_name, string asmstr> {
  def vp   : StoreInst_LD_TNSR   <OpCode, VRF,  asmstr, SPred>, MultiSlotGen4<op_name#"vp">, Requires<[LdInStore]>;
  def vm   : StoreInst_LD_TNSR   <OpCode, VRF,  asmstr, VPred>, MultiSlotGen4<op_name#"vm">, Requires<[LdInStore, LdVectMask]>;
  def mp   : StoreInst_LD_TNSR   <OpCode, VPRF, asmstr, SPred>, MultiSlotGen4<op_name#"mp">, Requires<[LdInStore]>;
  def mm   : StoreInst_LD_TNSR   <OpCode, VPRF, asmstr, VPred>, MultiSlotGen4<op_name#"mm">, Requires<[LdInStore, LdVectMask]>;

  def Dvp   : StoreInst_LD_TNSR   <OpCode, VRF,  asmstr, SPred>, MultiSlotGen4<op_name#"Dvp">, Requires<[LdInStore]>;
  def Dvm   : StoreInst_LD_TNSR   <OpCode, VRF,  asmstr, VPred>, MultiSlotGen4<op_name#"Dvm">, Requires<[LdInStore, LdVectMask]>;
  def Dmp   : StoreInst_LD_TNSR   <OpCode, VPRF, asmstr, SPred>, MultiSlotGen4<op_name#"Dmp">, Requires<[LdInStore]>;
  def Dmm   : StoreInst_LD_TNSR   <OpCode, VPRF, asmstr, VPred>, MultiSlotGen4<op_name#"Dmm">, Requires<[LdInStore, LdVectMask]>;

  def Tvp  : StoreInst_LD_TNSR_R <OpCode, VRF,  asmstr, SPred>, MultiSlotGen4<op_name#"Tvp">, Requires<[LdInStore, TnsrInReg]>;
  def Tvm  : StoreInst_LD_TNSR_R <OpCode, VRF,  asmstr, VPred>, MultiSlotGen4<op_name#"Tvm">, Requires<[LdInStore, TnsrInReg, LdVectMask]>;
  def Tmp  : StoreInst_LD_TNSR_R <OpCode, VPRF, asmstr, SPred>, MultiSlotGen4<op_name#"Tmp">, Requires<[LdInStore, TnsrInReg]>;
  def Tmm  : StoreInst_LD_TNSR_R <OpCode, VPRF, asmstr, VPred>, MultiSlotGen4<op_name#"Tmm">, Requires<[LdInStore, TnsrInReg, LdVectMask]>;
}

defm LD_TNSR_ST : Ld_Tnsr_St<0b010001, "LD_TNSR", "ld_tnsr $sw $dst, $tensor, $coords, $pred">;
def LD_TNSR_ST_Pvp   : StoreInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_ST_Pvm   : StoreInst_LD_TNSR_P <0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_ST_Pmp   : StoreInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_ST_Pmm   : StoreInst_LD_TNSR_P <0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_ST_PTvp  : StoreInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_ST_PTvm  : StoreInst_LD_TNSR_PR<0b010001, VRF,  "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_ST_PTmp  : StoreInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_ST_PTmm  : StoreInst_LD_TNSR_PR<0b010001, VPRF, "ld_tnsr $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;

def LD_TNSR_ST_Dvp   : StoreInst_LD_TNSR_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_ST_Dvm   : StoreInst_LD_TNSR_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;
def LD_TNSR_ST_Dmp   : StoreInst_LD_TNSR_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_ST_Dmm   : StoreInst_LD_TNSR_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;

def LD_TNSR_ST_PDvp   : StoreInst_LD_TNSR_P_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_ST_PDvm   : StoreInst_LD_TNSR_P_D <0b010001, VRF,  "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;
def LD_TNSR_ST_PDmp   : StoreInst_LD_TNSR_P_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_ST_PDmm   : StoreInst_LD_TNSR_P_D <0b010001, VPRF, "ld_tnsr $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;


//------ LD_TNSR_LOW -----------------------------------------------------------

defm LD_TNSR_LOW_ST : Ld_Tnsr_St<0b010010, "LD_TNSR_LOW", "ld_tnsr_low $sw $dst, $tensor, $coords, $pred">;

def LD_TNSR_LOW_ST_Pvp  : StoreInst_LD_TNSR_P <0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_LOW_ST_Pvm  : StoreInst_LD_TNSR_P <0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_LOW_ST_Pmp  : StoreInst_LD_TNSR_P <0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_LOW_ST_Pmm  : StoreInst_LD_TNSR_P <0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_LOW_ST_PTvp : StoreInst_LD_TNSR_PR<0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_LOW_ST_PTvm : StoreInst_LD_TNSR_PR<0b010010, VRF,  "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_LOW_ST_PTmp : StoreInst_LD_TNSR_PR<0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_LOW_ST_PTmm : StoreInst_LD_TNSR_PR<0b010010, VPRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;

def LD_TNSR_LOW_ST_Dvp  : StoreInst_LD_TNSR_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_LOW_ST_Dvm  : StoreInst_LD_TNSR_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;
def LD_TNSR_LOW_ST_Dmp  : StoreInst_LD_TNSR_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_LOW_ST_Dmm  : StoreInst_LD_TNSR_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;

def LD_TNSR_LOW_ST_PDvp  : StoreInst_LD_TNSR_P_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_LOW_ST_PDvm  : StoreInst_LD_TNSR_P_D <0b010010, VRF,  "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;
def LD_TNSR_LOW_ST_PDmp  : StoreInst_LD_TNSR_P_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_LOW_ST_PDmm  : StoreInst_LD_TNSR_P_D <0b010010, VPRF, "ld_tnsr_low $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;

//------ LD_TNSR_HIGH ----------------------------------------------------------

defm LD_TNSR_HIGH_ST : Ld_Tnsr_St<0b010011, "LD_TNSR_HIGH", "ld_tnsr_high $sw $dst, $tensor, $coords, $pred">;
def LD_TNSR_HIGH_ST_Pvp     : StoreInst_LD_TNSR_P <0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_Pvm     : StoreInst_LD_TNSR_P <0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_HIGH_ST_Pmp     : StoreInst_LD_TNSR_P <0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_Pmm     : StoreInst_LD_TNSR_P <0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_HIGH_ST_PTvp    : StoreInst_LD_TNSR_PR<0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_PTvm    : StoreInst_LD_TNSR_PR<0b010011, VRF,  "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;
def LD_TNSR_HIGH_ST_PTmp    : StoreInst_LD_TNSR_PR<0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, Requires<[LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_PTmm    : StoreInst_LD_TNSR_PR<0b010011, VPRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, Requires<[LdInStore, Partial, LdVectMask]>;

def LD_TNSR_HIGH_ST_Dvp     : StoreInst_LD_TNSR_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_HIGH_ST_Dvm     : StoreInst_LD_TNSR_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;
def LD_TNSR_HIGH_ST_Dmp     : StoreInst_LD_TNSR_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", SPred>, Requires<[Direct, LdInStore]>;
def LD_TNSR_HIGH_ST_Dmm     : StoreInst_LD_TNSR_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $pred", VPred>, Requires<[Direct, LdInStore, LdVectMask]>;

def LD_TNSR_HIGH_ST_PDvp     : StoreInst_LD_TNSR_P_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_PDvm     : StoreInst_LD_TNSR_P_D <0b010011, VRF,  "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;
def LD_TNSR_HIGH_ST_PDmp     : StoreInst_LD_TNSR_P_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, Requires<[Direct, LdInStore, Partial]>;
def LD_TNSR_HIGH_ST_PDmm     : StoreInst_LD_TNSR_P_D <0b010011, VPRF, "ld_tnsr_high $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, Requires<[Direct, LdInStore, Partial, LdVectMask]>;

//------ CACHE_FLUSH -----------------------------------------------------------

let hasSideEffects = 1 in {
  def CACHE_FLUSH      : StoreInstPred<0b010100, "cache_flush $sw $pred">;
  def CACHE_FLUSH_ADDR : StoreInstPredAddr<0b010100, "cache_flush $sw $addr, $pred", 1>, Requires<[Doron1]>;
}

def : Pat<(int_tpc_cache_flush (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_FLUSH (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;

def CacheFlushAddrSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() | TPCII::SW_CL, SDLoc(N), MVT::i32);
}]>;

def : Pat<(int_tpc_cache_flush_addr i64:$addr, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_FLUSH_ADDR ADRF:$addr, (CacheFlushAddrSwitch $sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;

//------ CACHE_INVALIDATE ------------------------------------------------------

let hasSideEffects = 1 in {
  def CACHE_INVALIDATE      : StoreInstPred<0b010101, "cache_invalidate $sw $pred">;
  def CACHE_INVALIDATE_ADDR : StoreInstPredAddr<0b010101, "cache_invalidate $sw $addr, $pred", 0>, Requires<[Doron1]>;
}

def : Pat<(int_tpc_cache_invalidate (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_INVALIDATE (i32 imm:$sw), SPRF:$pred, (i1 imm:$polarity))>;

def CacheInvAddrSwitch : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() | TPCII::SW_INV_CL | TPCII::SW_D, SDLoc(N), MVT::i32);
}]>;

def : Pat<(int_tpc_cache_invalidate_addr i64:$addr, (i32 imm:$sw), i1:$pred, (i1 imm:$polarity)),
          (CACHE_INVALIDATE_ADDR ADRF:$addr, (CacheInvAddrSwitch $sw), SPRF:$pred, (i1 imm:$polarity))>, Requires<[Doron1]>;

//------ LD_TNSR_CNVRT ---------------------------------------------------------

def LD_TNSR_CNVRT_ST_vp : StoreInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRTvp">, Requires<[Doron1]>;
def LD_TNSR_CNVRT_ST_vm : StoreInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRTvm">, Requires<[Doron1]>;

def LD_TNSR_CNVRTT_ST_vp : StoreInst_LD_TNSR_CNVRTR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRTTvp">, Requires<[Doron1, TnsrInReg]>;
def LD_TNSR_CNVRTT_ST_vm : StoreInst_LD_TNSR_CNVRTR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRTTvm">, Requires<[Doron1, TnsrInReg]>;

def LD_TNSR_CNVRT_P_ST_vp : StoreInst_LD_TNSR_CNVRT_P<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_Pvp">, Requires<[Doron1, Partial]>;
def LD_TNSR_CNVRT_P_ST_vm : StoreInst_LD_TNSR_CNVRT_P<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_Pvm">, Requires<[Doron1, Partial]>;

def LD_TNSR_CNVRT_PT_ST_vp : StoreInst_LD_TNSR_CNVRT_PR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_PTvp">, Requires<[Doron1, Partial, TnsrInReg]>;
def LD_TNSR_CNVRT_PT_ST_vm : StoreInst_LD_TNSR_CNVRT_PR<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_PTvm">, Requires<[Doron1, Partial, TnsrInReg]>;

def LD_TNSR_CNVRT_D_ST_vp : StoreInst_LD_TNSR_CNVRT_D<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_Dvp">, Requires<[Doron1, Direct]>;
def LD_TNSR_CNVRT_D_ST_vm : StoreInst_LD_TNSR_CNVRT_D<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_Dvm">, Requires<[Doron1, Direct]>;

def LD_TNSR_CNVRT_PD_ST_vp : StoreInst_LD_TNSR_CNVRT_PD<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $offsize, $pred", SPred>, MultiSlot<"LD_TNSR_CNVRT_PDvp">, Requires<[Doron1,  Partial, Direct]>;
def LD_TNSR_CNVRT_PD_ST_vm : StoreInst_LD_TNSR_CNVRT_PD<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $coords1, $coords2, $offsize, $pred", VPred>, MultiSlot<"LD_TNSR_CNVRT_PDvm">, Requires<[Doron1, Partial, Direct]>;

//------------------------------------------------------------------------------

// Predicate manipulations.

def : Pat<(i1 i1true), (COPY SPRF_TRUE)>;
def : Pat<(v256i1 (vsplatb256 i1true)), (COPY (v256i1 VPRF_TRUE))>;
def : Pat<(v128i1 (vsplatb128 i1true)), (COPY (v128i1 VPRF_TRUE))>;
def : Pat<(v64i1  (vsplatb64  i1true)), (COPY (v64i1  VPRF_TRUE))>;

def : Pat<(v256i1 (vsplatb256 i1:$src)), (v256i1 (MOV_ld_mpp SPRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128i1 (vsplatb128 i1:$src)), (v128i1 (MOV_ld_mpp SPRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64i1  (vsplatb64  i1:$src)), (v64i1  (MOV_ld_mpp SPRF:$src, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def : Pat<(v256i1 (vsplatb256 (i1 imm:$src))),
          (v256i1 (MOVB_ld_mip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v128i1 (vsplatb128 (i1 imm:$src))),
          (v128i1 (MOVB_ld_mip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
def : Pat<(v64i1  (vsplatb64 (i1 imm:$src))),
          (v64i1  (MOVB_ld_mip (CastToImm32 $src), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

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

def : Pat<(v64i32 (zext v64i1:$x)),
          (v64i32 (MOVvim (i32 0), OpType.INT32, (i32 0),
                          (MOVvim (i32 1), OpType.INT32, (i32 0), (IMPLICIT_DEF), v64i1:$x, (i1 0)),
                          v64i1:$x, (i1 1)))>;
def : Pat<(v128i16 (zext v128i1:$x)),
          (v128i16 (MOVvim (i32 0), OpType.INT16, (i32 0),
                          (MOVvim (i32 1), OpType.INT16, (i32 0), (IMPLICIT_DEF), v128i1:$x, (i1 0)),
                          v128i1:$x, (i1 1)))>;
def : Pat<(v256i8 (zext v256i1:$x)),
          (v256i8 (MOVvim (i32 0), OpType.INT8, (i32 0),
                          (MOVvim (i32 1), OpType.INT8, (i32 0), (IMPLICIT_DEF), v256i1:$x, (i1 0)),
                          v256i1:$x, (i1 1)))>;

// Select instruction.
foreach T = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152, v64i32, v128i16, v256i8] in {
  def : Pat<(vselect VectorPredicate<T>.Type:$p, T:$vtrue, T:$vfalse),
            (T (MOVvvm T:$vtrue, (i32 0), T:$vfalse, VectorPredicate<T>.Type:$p, (i1 0)))>;
  def : Pat<(vselect VectorPredicate<T>.Type:$p, (T (SplatNode<T>.Splat VectorElement<T>.Type:$vtrue)), T:$vfalse),
            (T (MOVvsm VectorElement<T>.Type:$vtrue, OpTypeForType<T>.V, (i32 0), T:$vfalse, VectorPredicate<T>.Type:$p, (i1 0)))>;
  def : Pat<(vselect VectorPredicate<T>.Type:$p, T:$vtrue, (T (SplatNode<T>.Splat VectorElement<T>.Type:$vfalse))),
            (T (MOVvsm VectorElement<T>.Type:$vfalse, OpTypeForType<T>.V, (i32 0), T:$vtrue, VectorPredicate<T>.Type:$p, (i1 1)))>;
  def : Pat<(vselect VectorPredicate<T>.Type:$p, (T (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$vtrue))), T:$vfalse),
            (T (MOVvim (CastOp<T>.Cast $vtrue), OpTypeForType<T>.V, (i32 0), T:$vfalse, VectorPredicate<T>.Type:$p, (i1 0)))>;
  def : Pat<(vselect VectorPredicate<T>.Type:$p, T:$vtrue, (T (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$vfalse)))),
            (T (MOVvim (CastOp<T>.Cast $vfalse), OpTypeForType<T>.V, (i32 0), T:$vtrue, VectorPredicate<T>.Type:$p, (i1 1)))>;
}

// Select folded with compare.

multiclass SelPatterns<PatFrag Pred, ValueType TC, ValueType TD, bits<4> OpType> {
  def : Pat<(TD (vselect (VectorPredicate<TC>.Type (Pred TC:$c1, TC:$c2)), TD:$d1, TD:$d2)),
            (TD (SelInstructionForPredicate<Pred>.InstrVVVV TC:$c1, TC:$c2, TD:$d1, TD:$d2, OpType, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(TD (vselect (VectorPredicate<TC>.Type (Pred TC:$c1, (TC (SplatNode<TC>.Splat VectorElement<TC>.Type:$c2)))), TD:$d1, TD:$d2)),
            (TD (SelInstructionForPredicate<Pred>.InstrVSVV TC:$c1, VectorElement<TC>.Type:$c2, TD:$d1, TD:$d2, OpType, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(TD (vselect (VectorPredicate<TC>.Type (Pred TC:$c1, (TC (SplatNode<TC>.Splat (VectorElement<TC>.Type ImmNode<TC>.Node:$c2))))), TD:$d1, TD:$d2)),
            (TD (SelInstructionForPredicate<Pred>.InstrVIVV TC:$c1, (CastOp<TC>.Cast VectorElement<TC>.Type:$c2), TD:$d1, TD:$d2, OpType, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(TD (vselect (VectorPredicate<TC>.Type (Pred TC:$c1, TC:$c2)), TD:$d1, (TD (SplatNode<TD>.Splat VectorElement<TD>.Type:$d2)))),
            (TD (SelInstructionForPredicate<Pred>.InstrVVVS TC:$c1, TC:$c2, TD:$d1, VectorElement<TD>.Type:$d2, OpType, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(TD (vselect (VectorPredicate<TC>.Type (Pred TC:$c1, TC:$c2)), TD:$d1, (TD (SplatNode<TD>.Splat (VectorElement<TD>.Type ImmNode<TD>.Node:$d2))))),
            (TD (SelInstructionForPredicate<Pred>.InstrVVVI TC:$c1, TC:$c2, TD:$d1, (CastOp<TD>.Cast VectorElement<TD>.Type:$d2), OpType, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}

foreach TC = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in
  foreach TD = SelCompatibleTypes<TC>.Ts in
    foreach Pred = [setoeq, setone, setolt, setole, setogt, setoge] in
      defm : SelPatterns<Pred, TC, TD, OpTypeForType<TC>.V>;

foreach TC = [v64i32, v128i16, v256i8] in
  foreach TD = SelCompatibleTypes<TC>.Ts in
    foreach Pred = [seteq, setne, setlt, setle, setgt, setge] in
      defm : SelPatterns<Pred, TC, TD, OpTypeForType<TC>.V>;

foreach TC = [v64i32, v128i16, v256i8] in
  foreach TD = SelCompatibleTypes<TC>.Ts in
    foreach Pred = [setult, setule, setugt, setuge] in
      defm : SelPatterns<Pred, TC, TD, UnsignedOpTypeForType<TC>.V>;


// Compare instructions.
foreach T = [v64f32, v128bf16, v128f16, v256f8_143, v256f8_152] in {
  def: Pat<(VectorPredicate<T>.Type (setcc T:$a, T:$a, SETUO)),
            (NOTmmp (CMP_EQvvp VRF:$a, VRF:$a, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                    OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def: Pat<(VectorPredicate<T>.Type (setcc T:$a, T:$b, SETUO)),
           (ORmmp (NOTmmp (CMP_EQvvp VRF:$a, VRF:$a, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  (NOTmmp (CMP_EQvvp VRF:$b, VRF:$b, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def: Pat<(VectorPredicate<T>.Type (setcc T:$a, T:$a, SETO)),
           (CMP_EQvvp VRF:$a, VRF:$a, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
  def: Pat<(VectorPredicate<T>.Type (setcc T:$a, T:$b, SETO)),
           (ANDmmp (CMP_EQvvp VRF:$a, VRF:$a, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                   (CMP_EQvvp VRF:$b, VRF:$b, OpTypeForType<T>.V, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                   OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
}

def : Pat<(i1 (setne i1:$op1, i1:$op2)),
          (XORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i1 (seteq i1:$op1, i1:$op2)),
          (XORppp (XORppp $op1, $op2, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  SPRF_TRUE, OpType.BOOL, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

// Convert patterns
// TODO: patterns for unsigned truncation

// TODO: should we use CONVERT instead?
def : Pat<(i32 (zext i16:$src)), (ANDsip i16:$src, (i32 0x0000ffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (zext i8:$src)), (ANDsip i8:$src, (i32 0x000000ff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (zext i8:$src)), (ANDsip i8:$src, (i32 0x000000ff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;


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
           ,(i32  (ABSssp i32:$src1, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)
             ))
        >;

def srac16  : PatFrag<(ops node:$src), (i16 (sra node:$src,(i32 15))) >;
def addsra16 :PatFrag<(ops node:$src), (i16 (add node:$src,(srac16 node:$src)))>;
def : Pat<( xor (addsra16 i16:$src1), (srac16 i16:$src1) 
           )
           ,(i16  (ABSssp i16:$src1, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)
             ))
        >;

def srac8  : PatFrag<(ops node:$src), (i8 (sra node:$src,(i32 7))) >;
def addsra8 :PatFrag<(ops node:$src), (i8 (add node:$src,(srac8 node:$src)))>;
def : Pat<( xor (addsra8 i8:$src1), (srac8 i8:$src1) 
           )
           ,(i8  (ABSssp i8:$src1, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)
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
          (MAXsip i32:$src1,imm:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)
          )
         >;
def : Pat< (select   
                  (i1 (setcc i32:$src1, i32:$src2, SETGT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MAXssp i32:$src1,i32:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, imm:$src2, SETLT)),
                   i32:$src1,
                   imm:$src2
            ),
          (MINsip i32:$src1,imm:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, i32:$src2, SETLT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MINssp i32:$src1,i32:$src2,OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i32:$src1, (i32 imm :$src2), SETGT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MAXsip i32:$src1,(i32 imm :$src2),OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, (i32 imm:$src2), SETLT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MINsip i32:$src1,(i32 imm :$src2),OpType.INT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////
//int16
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETGT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MAXssp i16:$src1,i16:$src2,OpType.INT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETLT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MINssp i16:$src1,i16:$src2,OpType.INT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i16:$src1, (i16 imm :$src2), SETGT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MAXsip i16:$src1,(CastToImm32 $src2),OpType.INT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i16:$src1, (i16 imm:$src2), SETLT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MINsip i16:$src1,(CastToImm32 $src2),OpType.INT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//int8
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETGT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MAXssp i8:$src1,i8:$src2,OpType.INT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETLT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MINssp i8:$src1,i8:$src2,OpType.INT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i8:$src1, (i8 imm :$src2), SETGT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MAXsip i8:$src1,(CastToImm32 $src2),OpType.INT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i8:$src1, (i8 imm:$src2), SETLT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MINsip i8:$src1,(CastToImm32 $src2),OpType.INT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
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
          (MAXssp i32:$src1,i32:$src2,OpType.UINT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, i32:$src2, SETULT)),
                   i32:$src1, 
                   i32:$src2
            ),
          (MINssp i32:$src1,i32:$src2,OpType.UINT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i32:$src1, (i32 imm :$src2), SETUGT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MAXsip i32:$src1,(i32 imm :$src2),OpType.UINT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i32:$src1, (i32 imm:$src2), SETULT)),
                   i32:$src1, 
                   (i32 imm :$src2)
            ),
          (MINsip i32:$src1,(i32 imm :$src2),OpType.UINT32,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//uint16
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETUGT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MAXssp i16:$src1,i16:$src2,OpType.UINT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i16:$src1, i16:$src2, SETULT)),
                   i16:$src1, 
                   i16:$src2
            ),
          (MINssp i16:$src1,i16:$src2,OpType.UINT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////// with imm ////////////////////////////////
def : Pat< (select   
                  (i1 (setcc i16:$src1, (i16 imm :$src2), SETUGT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MAXsip i16:$src1,(CastToImm32 $src2),OpType.UINT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i16:$src1, (i16 imm:$src2), SETULT)),
                   i16:$src1, 
                   (i16 imm :$src2)
            ),
          (MINsip i16:$src1,(CastToImm32 $src2),OpType.UINT16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

//int8
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETUGT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MAXssp i8:$src1,i8:$src2,OpType.UINT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select   
                  (i1 (setcc i8:$src1, i8:$src2, SETULT)),
                   i8:$src1, 
                   i8:$src2
            ),
          (MINssp i8:$src1,i8:$src2,OpType.UINT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
////////////////// with imm ////////////////////////////////*
def : Pat< (select   
                  (i1 (setcc i8:$src1, (i8 imm :$src2), SETUGT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MAXsip i8:$src1,(CastToImm32 $src2),OpType.UINT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
def : Pat< (select
                  (i1 (setcc i8:$src1, (i8 imm:$src2), SETULT)),
                   i8:$src1, 
                   (i8 imm :$src2)
            ),
          (MINsip i8:$src1,(CastToImm32 $src2),OpType.UINT8,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
          )
         >;
///////////////////// end of imm ///////////////////////

def : Pat<(bf16 (selectcc bf16:$tval, bf16:$fval, bf16:$tval, bf16:$fval, SETOGT)),
          (MAXssp bf16:$tval,bf16:$fval,OpType.BF16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(bf16 (selectcc bf16:$tval, bf16:$fval, bf16:$tval, bf16:$fval, SETOLT)),
          (MINssp bf16:$tval,bf16:$fval,OpType.BF16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(f16 (selectcc f16:$tval, f16:$fval, f16:$tval, f16:$fval, SETOGT)),
          (MAXssp f16:$tval,f16:$fval,OpType.FP16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(f16 (selectcc f16:$tval, f16:$fval, f16:$tval, f16:$fval, SETOLT)),
          (MINssp f16:$tval,f16:$fval,OpType.FP16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;


def : Pat<(bf16 (selectcc bf16:$tval, (bf16 fpimm:$fval), bf16:$tval, (bf16 fpimm:$fval), SETOGT)),
          (MAXsip bf16:$tval,(FPToImm32 $fval),OpType.BF16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(bf16 (selectcc bf16:$tval, (bf16 fpimm:$fval), bf16:$tval, (bf16 fpimm:$fval), SETOLT)),
          (MINsip bf16:$tval,(FPToImm32 $fval),OpType.BF16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(f16 (selectcc f16:$tval, (f16 fpimm:$fval), f16:$tval, (f16 fpimm:$fval), SETOGT)),
          (MAXsip f16:$tval,(FPToImm32 $fval),OpType.FP16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;

def : Pat<(f16 (selectcc f16:$tval, (f16 fpimm:$fval), f16:$tval, (f16 fpimm:$fval), SETOLT)),
          (MINsip f16:$tval,(FPToImm32 $fval),OpType.FP16,(i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0) 
               )
         >;


// Bit extend patterns.
//def : Pat<(select i1:$cond, i32:$tvalue, i32:$fvalue),
//          (CMOVIi32si $tvalue, $cond, (CMOVi32si $fvalue, $cond))>;


// Bitcast patterns
def : Pat<(v64f32  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v128f16:$src)), (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v64i32:$src)),  (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v128i16:$src)), (COPY VRF:$src)>;
def : Pat<(v64f32  (bitconvert v256i8:$src)),  (COPY VRF:$src)>;

def : Pat<(v128bf16 (bitconvert v64f32:$src)),  (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v128f16:$src)), (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v64i32:$src)),  (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v128i16:$src)), (COPY VRF:$src)>;
def : Pat<(v128bf16 (bitconvert v256i8:$src)),  (COPY VRF:$src)>;

def : Pat<(v128f16 (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128f16 (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v128f16 (bitconvert v64i32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128f16 (bitconvert v128i16:$src)),  (COPY VRF:$src)>;
def : Pat<(v128f16 (bitconvert v256i8:$src)),   (COPY VRF:$src)>;

def : Pat<(v64i32  (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v128f16:$src)),  (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v128i16:$src)),  (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v256i8:$src)),   (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v256f8_143:$src)), (COPY VRF:$src)>;
def : Pat<(v64i32  (bitconvert v256f8_152:$src)), (COPY VRF:$src)>;

def : Pat<(v128i16 (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v128f16:$src)),  (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v64i32:$src)),   (COPY VRF:$src)>;
def : Pat<(v128i16 (bitconvert v256i8:$src)),   (COPY VRF:$src)>;

def : Pat<(v256i8  (bitconvert v64f32:$src)),   (COPY VRF:$src)>;
def : Pat<(v256i8  (bitconvert v128bf16:$src)), (COPY VRF:$src)>;
def : Pat<(v256i8  (bitconvert v128f16:$src)),  (COPY VRF:$src)>;
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

def : Pat<(v256f8_143 (bitconvert v64i32:$src)),   (COPY VRF:$src)>;
def : Pat<(v256f8_152 (bitconvert v64i32:$src)),   (COPY VRF:$src)>;


def : Pat<(i32 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(f16 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert f32:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert f16:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert f16:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert f16:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert f16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert bf16:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(f16 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert i32:$src)), (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(f16 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i8  (bitconvert i16:$src)), (COPY SRF:$src)>;
def : Pat<(i32 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(i16 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(f32 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(f16 (bitconvert i8:$src)),  (COPY SRF:$src)>;
def : Pat<(bf16 (bitconvert i8:$src)),  (COPY SRF:$src)>;

// Arithmetics.
def : Pat<(fadd v256f8_143:$op1, v256f8_143:$op2),
          (CONVERTg3vap (ADDavp (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                v256f8_143:$op2, OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fadd v256f8_143:$op1, (v256f8_143 (vsplatf8_143 f8_143:$op2))),
          (CONVERTg3vap (ADDasp (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                f8_143:$op2, OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fadd v256f8_143:$op1, (v256f8_143 (vsplatf8_143 (f8_143 fpimm:$op2)))),
          (CONVERTg3vap (ADDaip (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                (FPToImm32 $op2), OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(fadd v256f8_152:$op1, v256f8_152:$op2),
          (CONVERTg3vap (ADDavp (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                v256f8_152:$op2, OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fadd v256f8_152:$op1, (v256f8_152 (vsplatf8_152 f8_152:$op2))),
          (CONVERTg3vap (ADDasp (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                f8_152:$op2, OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fadd v256f8_152:$op1, (v256f8_152 (vsplatf8_152 (f8_152 fpimm:$op2)))),
          (CONVERTg3vap (ADDaip (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                (FPToImm32 $op2), OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(fsub v256f8_143:$op1, v256f8_143:$op2),
          (CONVERTg3vap (SUBavp (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                v256f8_143:$op2, OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fsub v256f8_143:$op1, (v256f8_143 (vsplatf8_143 f8_143:$op2))),
          (CONVERTg3vap (SUBasp (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                f8_143:$op2, OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fsub v256f8_143:$op1, (v256f8_143 (vsplatf8_143 (f8_143 fpimm:$op2)))),
          (CONVERTg3vap (SUBaip (CONVERTg3avp v256f8_143:$op1, OpType.FP8_143, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                (FPToImm32 $op2), OpType.FP8_143, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_143), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(fsub v256f8_152:$op1, v256f8_152:$op2),
          (CONVERTg3vap (SUBavp (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                v256f8_152:$op2, OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fsub v256f8_152:$op1, (v256f8_152 (vsplatf8_152 f8_152:$op2))),
          (CONVERTg3vap (SUBasp (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                f8_152:$op2, OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fsub v256f8_152:$op1, (v256f8_152 (vsplatf8_152 (f8_152 fpimm:$op2)))),
          (CONVERTg3vap (SUBaip (CONVERTg3avp v256f8_152:$op1, OpType.FP8_152, (i32 SwitchVal.TO_FP32), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                (FPToImm32 $op2), OpType.FP8_152, (AddDnormSupport (i32 0)), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        OpType.FP32, (i32 SwitchVal.TO_FP8_152), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(i16 (mul i16:$src1, i16:$src2)),       (MULssp SRF:$src1, SRF:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, 0)>;
def : Pat<(i8  (mul i8:$src1, i8:$src2)),         (MULssp SRF:$src1, SRF:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, 0)>;


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
                 (SUBssp (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          SRF:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                 OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra i32:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         imm:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                 OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(sra i16:$src1, i16:$src2),
          (ASHssp SRF:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                        i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                 OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra i16:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (ANDsip (SUBsip (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                 (i32 imm:$src2), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(sra i8:$src1, i8:$src2),
          (ASHssp SRF:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                           SRF:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra i8:$src1, (i32 imm:$src2)),
          (ASHssp SRF:$src1,
                 (ANDsip (SUBsip (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                                 (i32 imm:$src2), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(sra v64i32:$src1, v64i32:$src2),
          (ASHvvp v64i32:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         v64i32:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v64i32:$src1, (v64i32 (vsplati32 i32:$src2))),
          (ASHvsp v64i32:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          i32:$src2, OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v64i32:$src1, (v64i32 (vsplati32 (i32 imm:$src2)))),
          (ASHvsp v64i32:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         (i32 imm:$src2), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(sra v128i16:$src1, v128i16:$src2),
          (ASHvvp v128i16:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          v128i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v128i16:$src1, (v128i16 (vsplati16 i16:$src2))),
          (ASHvsp v128i16:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          i16:$src2, OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v128i16:$src1, (v128i16 (vsplati16 (i16 imm:$src2)))),
          (ASHvsp v128i16:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         (i32 (CastToImm32 $src2)), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(sra v256i8:$src1, v256i8:$src2),
          (ASHvvp v256i8:$src1,
                 (SUBvvp (MOVvip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          v256i8:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v256i8:$src1, (v256i8 (vsplati8 i8:$src2))),
          (ASHvsp v256i8:$src1,
                 (SUBssp (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                          i8:$src2, OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(sra v256i8:$src1, (v256i8 (vsplati8 (i8 imm:$src2)))),
          (ASHvsp v256i8:$src1,
                 (SUBsip (MOVsip (i32 0), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                         (i32 (CastToImm32 $src2)), OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                  OpType.INT8, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(fneg f32:$e), (XORsip f32:$e, 0x80000000, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fneg bf16:$e), (XORsip bf16:$e, 0x8000, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fneg f16:$e), (XORsip f16:$e, 0x8000, OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fneg v64f32:$e), (XORvip v64f32:$e, 0x80000000, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fneg v128bf16:$e), (XORvip v128bf16:$e, 0x8000, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(fneg v128f16:$e), (XORvip v128f16:$e, 0x8000, OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

// NaN check
def : Pat<(seto f32:$f,  f32:$f),  (CMP_EQssp f32:$f,  f32:$f,  OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(seto bf16:$f, bf16:$f), (CMP_EQssp bf16:$f, bf16:$f, OpType.BF16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(seto f16:$f,  f16:$f),  (CMP_EQssp f16:$f,  f16:$f,  OpType.FP16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

// build_vector
def : Pat<(v5i32 (build_vector i32:$a1, i32:$a2, i32:$a3, i32:$a4, i32:$a5)),
          (v5i32 (SET_INDX_ld_rp
                   (SET_INDX_ld_rp
                     (SET_INDX_ld_rp
                       (SET_INDX_ld_rp
                         (SET_INDX_ld_rp (v5i32 (IMPLICIT_DEF)),
        i32:$a1, (i32 1), (i32 0), SPRF_TRUE, (i1 0)),
        i32:$a2, (i32 2), (i32 0), SPRF_TRUE, (i1 0)),
        i32:$a3, (i32 4), (i32 0), SPRF_TRUE, (i1 0)),
        i32:$a4, (i32 8), (i32 0), SPRF_TRUE, (i1 0)),
        i32:$a5, (i32 16), (i32 0), SPRF_TRUE, (i1 0)))>;

def : Pat<(v5i32 (build_vector (i32 imm:$a1), (i32 imm:$a2), (i32 imm:$a3), (i32 imm:$a4), (i32 imm:$a5))),
          (v5i32 (SET_INDX_ld_ip
                   (SET_INDX_ld_ip
                     (SET_INDX_ld_ip
                       (SET_INDX_ld_ip
                         (SET_INDX_ld_ip (v5i32 (IMPLICIT_DEF)),
        (CastToImm32 $a1), (i32 1), (i32 0), SPRF_TRUE, (i1 0)),
        (CastToImm32 $a2), (i32 2), (i32 0), SPRF_TRUE, (i1 0)),
        (CastToImm32 $a3), (i32 4), (i32 0), SPRF_TRUE, (i1 0)),
        (CastToImm32 $a4), (i32 8), (i32 0), SPRF_TRUE, (i1 0)),
        (CastToImm32 $a5), (i32 16), (i32 0), SPRF_TRUE, (i1 0)))>;

def : Pat<(v5i32 (build_vector i32:$a, i32:$a, i32:$a, i32:$a, i32:$a)),
          (v5i32 (SET_INDX_ld_rp  (v5i32 (IMPLICIT_DEF)), i32:$a, (i32 31), (i32 0), SPRF_TRUE, (i1 0)))>;

def : Pat<(v5i32 (build_vector (i32 imm:$a), (i32 imm:$a), (i32 imm:$a), (i32 imm:$a), (i32 imm:$a))),
          (v5i32 (SET_INDX_ld_ip  (v5i32 (IMPLICIT_DEF)), (CastToImm32 $a), (i32 31), (i32 0), SPRF_TRUE, (i1 0)))>;


// insertelt
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 0))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 1), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 1))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec,i32:$e, (i32 2), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 2))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 4), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 3))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec,i32:$e, (i32 8), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, i32:$e, (i32 4))),
          (v5i32 (SET_INDX_ld_rp IRF:$vec, i32:$e, (i32 16), (i32 0), SPRF_TRUE, (i1 0)))>;

def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 0))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 1), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 1))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 2), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 2))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 4), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 3))),
          (v5i32 (SET_INDX_ld_ip IRF:$vec, (CastToImm32 $e), (i32 8), (i32 0), SPRF_TRUE, (i1 0)))>;
def : Pat<(v5i32 (insertelt v5i32:$vec, (i32 imm:$e), (i32 4))),
          (v5i32 (SET_INDX_ld_ip  IRF:$vec, (CastToImm32 $e), (i32 16), (i32 0), SPRF_TRUE, (i1 0)))>;


def : Pat<(i32 (extractelt v5i32:$vec, imm:$dim)),
          (i32 (MOV_IRF_DIM $vec, (CastToImm32 $dim), (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

def : Pat<(insertelt v5i32:$vec,
                     (add (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ADDsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (add (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ADDiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (SUBsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (SUBiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub i32:$item,
                      (extractelt v5i32:$vec, (i32 imm:$dim))),
                     (i32 imm:$dim)),
          (SUBsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (sub (i32 imm:$item),
                      (extractelt v5i32:$vec, (i32 imm:$dim))),
                     (i32 imm:$dim)),
          (SUBiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (mul (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (MULsIp i32:$item, v5i32:$vec, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (mul (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (MULiIp (CastToImm32 $item), v5i32:$vec, (DimensionToMask $dim), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (or (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ORsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (or (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ORiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (and (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (ANDsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (and (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (ANDiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (xor (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          i32:$item),
                     (i32 imm:$dim)),
          (XORsIp $item, $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(insertelt v5i32:$vec,
                     (xor (extractelt v5i32:$vec,
                                      (i32 imm:$dim)),
                          (i32 imm:$item)),
                     (i32 imm:$dim)),
          (XORiIp (CastToImm32 $item), $vec, (DimensionToMask $dim), OpType.INT32, (i32 0), $vec, SPRF_TRUE, (i1 0))>;

def : Pat<(v5i32 (int_tpc_get_index_space_offset)), (COPY I0)>;
def : Pat<(v5i32 (int_tpc_get_index_space_size)), (COPY I1)>;

// Extendind loads
def : Pat<(i32 (l_load_sext8 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT8, SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_sext8 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT8, SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_zext8 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_zext8 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (l_load_sext8 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT8, SwitchVal.TO_INT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (l_load_sext8 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT8, SwitchVal.TO_INT16, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (l_load_zext8 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xffff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i16 (l_load_zext8 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xffff), OpType.INT16, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_sext16 i32:$ptr)), (CONVERTssp  (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_sext16 (i32 imm:$ptr))), (CONVERTssp  (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), OpType.INT16, SwitchVal.TO_INT32, (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_zext16 i32:$ptr)), (ANDsip (LD_Lssp SRF:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;
def : Pat<(i32 (l_load_zext16 (i32 imm:$ptr))), (ANDsip (LD_Lsip imm:$ptr, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)), (i32 0xffff), OpType.INT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0))>;

// Truncating stores
def : Pat<(truncstore_f16 f32:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP32, !or(SwitchVal.TO_FP16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP16]>;
def : Pat<(truncstore_bf16 f32:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP32, !or(SwitchVal.TO_BF16, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[BF16]>;
def : Pat<(truncstore_f8_143 f32:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP32, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;
def : Pat<(truncstore_f8_152 f32:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP32, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;
def : Pat<(truncstore_f8_143 bf16:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.BF16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;
def : Pat<(truncstore_f8_152 bf16:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.BF16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;
def : Pat<(truncstore_f8_143 f16:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP16, !or(SwitchVal.TO_FP8_143, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;
def : Pat<(truncstore_f8_152 f16:$val, i32:$ptr),
          (ST_Lssp SRF:$ptr,
              (CONVERTssp $val, OpType.FP16, !or(SwitchVal.TO_FP8_152, SwitchVal.RM_DEFAULT), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
              (i32 0), SPRF_TRUE, (i1 0))>,
      Requires<[FP8]>;

// IndVarSimplifyPass when it recognizes arithmetical progression:
//
// for(i..upper) {s=+i;} ==> s=upper*(upper-1)/2;
//
def : Pat<(i32 (trunc (srl (mul (zext i32:$x), (zext i32:$y)), (i32 1)))),
          (i32 (SHRsip (MULssp i32:$x, i32:$y, OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)),
                       (i32 1), OpType.UINT32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;

// Check that is not NAN, special case for ternary_cond.c G-403
def f32cc_uo : Pat<(i1(setcc f32:$op1, f32:$op1, SETUO)),
                   (i1 (CMP_NEQssp $op1, $op1, OpType.FP32, (i32 0), (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))
           >;


// The instruction with maximum number of arguments in asm string does not
// tell vector predicates form scalar predicates.
// So as workaround we created dummy instruction with a lot of arguments
def DUMMY : VPUInstDummy<0b000000, "dummy $op1, $op2, $op3, $op4, $op5, $op6, $op7,"
                                          "$op8, $op9, $op10, $op11, $op12, $op13,"
                                          "$op14, $op15, $op16, $op17, $op18, $op19, $op20">;

// Saturation arithmetic intrinsics.

multiclass SatScalar<ValueType T, SDNode Intrinsic, SDNode Iss, SDNode Isi, bits<4> OType> {
  def : Pat<(T (Intrinsic T:$x1, T:$x2)),
            (T (Iss SRF:$x1, SRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (T imm:$x2))),
            (T (Isi SRF:$x1, (CastToImm32 $x2), OType, SW.ST, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
}
multiclass SatVector<ValueType T, SDNode Intrinsic, SDNode Ivv, SDNode Ivs, SDNode Ivi, bits<4> OType> {
  def : Pat<(T (Intrinsic T:$x1, T:$x2)),
            (T (Ivv VRF:$x1, VRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (SplatNode<T>.Splat VectorElement<T>.Type:$x2))),
            (T (Ivs VRF:$x1, SRF:$x2, OType, SW.ST, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
  def : Pat<(T (Intrinsic T:$x1, (SplatNode<T>.Splat (VectorElement<T>.Type ImmNode<T>.Node:$x2)))),
            (T (Ivi VRF:$x1, (CastToImm32 $x2), OType, SW.ST, (IMPLICIT_DEF), SPRF_TRUE, (i1 0)))>;
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
