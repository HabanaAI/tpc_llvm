//===--------TPCInstrFormats.ts---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//                         TPC Instruction Flags +
//
//                    *** Must match TPCMCInstrInfo.h (TPCBaseInfo.h) ***
//===----------------------------------------------------------------------===//

def AddrRR : ComplexPattern<i32, 2, "SelectAddrRR", [], []>;
def AddrRI : ComplexPattern<i32, 2, "SelectAddrRI", [], []>;

def MEMrr : Operand<i32> {
  let PrintMethod = "printAddrOperand";
  let MIOperandInfo = (ops SRF, SRF);
  let EncoderMethod = "getRrMemoryOpValue";
  let DecoderMethod = "decodeMEMrr";
}

def MMriAsmOperand : AsmOperandClass {
  let Name = "MMri";
  let RenderMethod = "addMemRIOperands";
  let PredicateMethod = "isMemRI";
  let ParserMethod = "parseMemRI";
}

def MEMri : Operand<i32> {
  let ParserMatchClass = MMriAsmOperand;
  let PrintMethod = "printAddrOperand";
  let MIOperandInfo = (ops SRF, i32imm);
}

def brtarget : Operand<OtherVT> {
  let PrintMethod = "printJmpLoopTarget";
}

def SPredicateAsmOperand : AsmOperandClass {
  let Name = "SPredicate";
  let ParserMethod = "parsePredicate";
  let IsOptional = 1;
}

def VPredicateAsmOperand : AsmOperandClass {
  let Name = "VPredicate";
  let ParserMethod = "parsePredicate";
}

def SPred : Operand<i32> {
  let OperandNamespace = "TPC";
  let OperandType = "OPERAND_PREDICATE";
  let PrintMethod = "printSPredicate";
  let ParserMatchClass = SPredicateAsmOperand;
  let EncoderMethod = "encodePredicate";
  let DecoderMethod = "decodeSPredicate";
  let MIOperandInfo = (ops SPRF:$pred, i1imm:$polarity);
}

def VPred : Operand<i32> {
  let OperandNamespace = "TPC";
  let OperandType = "OPERAND_PREDICATE";
  let PrintMethod = "printVPredicate";
  let ParserMatchClass = VPredicateAsmOperand;
  let EncoderMethod = "encodePredicate";
  let DecoderMethod = "decodeVPredicate";
  let MIOperandInfo = (ops VPRF:$vpred, i1imm:$polarity);
}

def LoopStartImm : Operand<i32> {
  let PrintMethod = "printLoopImm";
  let MIOperandInfo = (ops i32imm:$start);
  let DecoderMethod = "decodeLoopStartImm";
}

def LoopBoundaryImm : Operand<i32> {
  let PrintMethod = "printLoopImm";
  let MIOperandInfo = (ops i32imm:$boundary);
  let DecoderMethod = "decodeLoopBoundaryImm";
}

def LoopStepImm : Operand<i32> {
  let PrintMethod = "printLoopImm";
  let MIOperandInfo = (ops i32imm:$step);
  let DecoderMethod = "decodeLoopStepImm";
}

def DataTypeAsmOperand : AsmOperandClass {
  let Name = "DataType";
  let RenderMethod = "addDataTypeOperands";
  let PredicateMethod = "isDataType";
}

def DataType : Operand<i8> {
  let OperandNamespace = "TPC";
  let PrintMethod = "printDataType";
  let ParserMatchClass = DataTypeAsmOperand;
  let OperandType = "OPERAND_DATATYPE";
  let MIOperandInfo = (ops i8imm:$datatype);
}

def DimMaskAsmOperand : AsmOperandClass {
  let Name = "DimMask";
  let RenderMethod = "addDimMaskOperands";
  let PredicateMethod = "isDimMask";
}

def DimMask : Operand<i32> {
  let PrintMethod = "printDimMask";
  let ParserMatchClass = DimMaskAsmOperand;
  let MIOperandInfo = (ops i32imm:$dimmask);
  let OperandNamespace = "TPC";
  let OperandType = "OPERAND_DIMMASK";
}

def SwitchSetAsmOperand : AsmOperandClass {
  let Name = "SwitchSet";
  let RenderMethod = "addSwitchSetOperands";
  let PredicateMethod = "isSwitchSet";
}

def SwitchSet : Operand<i32> {
  let PrintMethod = "printSwitchSet";
  let ParserMatchClass = SwitchSetAsmOperand;
  let MIOperandInfo = (ops i32imm:$sw);
}

def ComparisonAsmOperand : AsmOperandClass {
  let Name = "Comparison";
  let ParserMethod = "parseComparison";
  let RenderMethod = "addComparisonOperands";
  let PredicateMethod = "isComparison";
}

def Comparison : Operand<i8> {
  let PrintMethod = "printComparison";
  let ParserMatchClass = ComparisonAsmOperand;
  let MIOperandInfo = (ops i8imm:$cmp);
  let DecoderMethod = "decodeLoopComparison";
}

def MacAccAsmOperand : AsmOperandClass {
  let Name = "MacAcc";
  let RenderMethod = "addAccumulatorOperands";
  let PredicateMethod = "isAccumulator";
}

def MacAcc : Operand<i8> {
  let PrintMethod = "printAccumulator";
  let ParserMatchClass = MacAccAsmOperand;
  let MIOperandInfo = (ops i8imm:$acc);
}

def RhazRsAsmOperand : AsmOperandClass {
  let Name = "RhazRsOp";
  let RenderMethod = "addRhazRsOperands";
  let PredicateMethod = "isRhazRs";
}

def RhazRsOp : Operand<i8> {
  let PrintMethod = "printRhazRs";
  let ParserMatchClass = RhazRsAsmOperand;
  let MIOperandInfo = (ops i8imm:$rhaz);
}

def SwitchRhuAsmOperand : AsmOperandClass {
  let Name = "Rhu";
  let ParserMethod = "parseRhu";
  let IsOptional = 1;
}

def RhuSwitch : Operand<i8> {
  let PrintMethod = "printRhu";
  let ParserMatchClass = SwitchRhuAsmOperand;
}

def SwitchRhazRsAsmOperand : AsmOperandClass {
  let Name = "RhazRs";
  let ParserMethod = "parseRhazRs";
  let IsOptional = 1;
}

def RhazRsSwitch : Operand<i8> {
  let PrintMethod = "printRhazRs";
  let ParserMatchClass = SwitchRhazRsAsmOperand;
}

def BothDivModAsmOperand : AsmOperandClass {
  let Name = "BothDivModOp";
  let RenderMethod = "addBothDivModOperands";
  let PredicateMethod = "isBothDivMod";
}

def BothDivModOp : Operand<i8> {
  let PrintMethod = "printBothDivMod";
  let ParserMatchClass = BothDivModAsmOperand;
  let MIOperandInfo = (ops i8imm:$bothdiv);
}

def SwitchBothDivModAsmOperand : AsmOperandClass {
  let Name = "BothDivMod";
  let ParserMethod = "parseBothDivMod";
  let IsOptional = 1;
}

def BothDivModSwitch : Operand<i8> {
  let PrintMethod = "printBothDivMod";
  let ParserMatchClass = SwitchBothDivModAsmOperand;
}

def X2AsmOperand : AsmOperandClass {
  let Name = "X2Op";
  let RenderMethod = "addX2Operands";
  let PredicateMethod = "isX2";
}

def X2Op : Operand<i8> {
  let PrintMethod = "printX2";
  let ParserMatchClass = X2AsmOperand;
  let MIOperandInfo = (ops i8imm:$x2);
}

def MovDGAllAsmOperand : AsmOperandClass {
  let Name = "MovDGAll";
  let RenderMethod = "addMovDGAllOperands";
  let PredicateMethod = "isMovDGAll";
}

def MovDGAllOp : Operand<i8> {
  let PrintMethod = "printMovDGAll";
  let ParserMatchClass = MovDGAllAsmOperand;
  let MIOperandInfo = (ops i8imm:$movdgall);
}

def MovDGPackAsmOperand : AsmOperandClass {
  let Name = "MovDGPack";
  let RenderMethod = "addMovDGPackOperands";
  let PredicateMethod = "isMovDGPack";
}

def MovDGPackOp : Operand<i8> {
  let PrintMethod = "printMovDGPack";
  let ParserMatchClass = MovDGPackAsmOperand;
  let MIOperandInfo = (ops i8imm:$movdgpack);
}

def MovDGUnpackAsmOperand : AsmOperandClass {
  let Name = "MovDGUnpack";
  let RenderMethod = "addMovDGUnpackOperands";
  let PredicateMethod = "isMovDGUnpack";
}

def MovDGUnpackOp : Operand<i8> {
  let PrintMethod = "printMovDGUnpack";
  let ParserMatchClass = MovDGUnpackAsmOperand;
  let MIOperandInfo = (ops i8imm:$movdgunpack);
}

let OperandNamespace = "TPC" in {
//Designates operands which should be encoded in the IMM field (possibly as SMALL_IMM)
  class TPCImm<Operand ImmOp> : Operand<ImmOp.Type> {
    let EncoderMethod = "encodeTPCImm";
    let OperandType = "OPERAND_TPC_IMM";
  }
}

class SlotType<bits<3> t> {
  bits<3> Value = t;
}
def PseudoSlot    : SlotType<0>;
def VpuSlot       : SlotType<1>;
def SpuSlot       : SlotType<2>;
def LoadSlot      : SlotType<3>;
def StoreSlot     : SlotType<4>;
def LoopSlot      : SlotType<5>;

// NOTE: Ensure the value of OpType.XXXX to match that of (SwitchVal.TO_XXXX >> 8)
def OpType {
bits<4> FP32       = 0;
bits<4> BF16       = 1;
bits<4> INT32      = 2;
bits<4> UINT32     = 3;
bits<4> INT8       = 4;
bits<4> UINT8      = 5;
bits<4> BOOL       = 6;
bits<4> INT16      = 7;
bits<4> UINT16     = 8;
bits<4> INT4       = 9;
bits<4> UINT4      = 10;
bits<4> FP16       = 11;
bits<4> FP8_152    = 12;
bits<4> FP8_143    = 13;
bits<4> INT64      = 14;
}

def TensorSrc {
  bit DimMask = 0;
  bit Reg = 1;
}

def DMask {
  bit Imm = 0;
  bit Reg = 1;
}

def SwitchVal {
  bits<8> SW_DOUBLE_AND_ROUND32 = 0b00000010;
  bits<8> SW_ACC_FP32           = 0b00000100;
  bits<8> SW_UPPER32            = 0b00000100;
  bits<8> SW_AUTO_INC           = 0b00000001;
  bits<32> SW_FORCE_SIGN0       = 0x00000200;

  // In CONVERT_INT rounding mode is encoded in OperandType.

  // Default rounding mode is the same for all cores.
  int RM_DEFAULT      = 0x050000;

  // Rounding mode in CONVERT_INT, Gen1.
  int RM_G1_RHNE      = 0;
  int RM_G1_RD        = 0x010000;
  int RM_G1_RU        = 0x020000;
  int RM_G1_SR        = 0x030000;   // Not used in CONVERT
  // Additional rounding modes in CONVERT.
  int CVT_G1_RZ       = 0x040000;

  // Rounding mode in CONVERT_INT, Gen2+.
  int RM_RHNE         = 0;
  int RM_RZ           = 0x010000;
  int RM_RU           = 0x020000;
  int RM_RD           = 0x030000;
  int RM_SR           = 0x040000;
  // Additional rounding modes in CONVERT.
  int RM_RHAZ         = 0x060000;

  // NOTE: Ensure the value of SwitchVal.TO_XXXX to match that of (OpType.XXXX << 8)
  // Target data types in CONVERT.
  int TO_FP32         = 0;
  int TO_BF16         = 0x0100;
  int TO_INT32        = 0x0200;
  int TO_UINT32       = 0x0300;
  int TO_INT8         = 0x0400;
  int TO_UINT8        = 0x0500;
  int TO_INT16        = 0x0700;
  int TO_UINT16       = 0x0800;
  int TO_INT4         = 0x0900;
  int TO_UINT4        = 0x0a00;
  int TO_FP16         = 0x0B00;
  int TO_FP8_152      = 0x0C00;
  int TO_FP8_143      = 0x0D00;

  // Optional conversion in NEARBYINT.
  int SW_CNVRT        = 0x1000;
  int SW_CNVRT_INT32  = 0x0400;
  int SW_CNVRT_INT16  = 0x8000;
}

def SW {
  bits<32> ST           = 0x01;
  bits<32> SET          = 0x01;
  bits<32> MSB          = 0x10;
  bits<32> FORCE_SIGN0  = 0x200;
  bits<32> INC_0        = 0;
  bits<32> INC_1        = 0x01;
  bits<32> INC_2        = 0x02;
  bits<32> INC_4        = 0x03;
  bits<32> INC_8        = 0x04;
  bits<32> INC_1_G3     = 0;
  bits<32> INC_2_G3     = 0x01;
  bits<32> INC_4_G3     = 0x02;
  bits<32> INC_8_G3     = 0x03;
  bits<32> MASK_EQ_ZERO = 0x01;
}

//===----------------------------------------------------------------------===//
//                         Instruction Classes Definitions
//===----------------------------------------------------------------------===//

class TPCInstGeneric<bits<6> opc, string asmstr, SlotType slot, InstrItinClass itin>
  : Instruction {
  let Namespace = "TPC";
  let Itinerary = itin;
  let AsmString = asmstr;
  let AsmVariantName = ?;
  let hasSideEffects = 0;
  let isNotUsedInDisasm = 1;    // Turn off disassembler by default

  // Assume maximal instruction size (actually slot size), otherwise the
  // instruction will not be treated by disassembler.
  let Size = 8;

  bits<6> Opcode  = opc;
  bits<4> OType = OpType.INT32; // Bogus value for data type
  bit HasImm = 0;
  bit IsPredicated = 0;
  bit OutOfSlotData = 0;
  bit HasCompositeImm = 0;
  SlotType Slot = slot;
  bits<64> Inst;

  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<64> SoftFail = 0;

  // Target specific information.

  let TSFlags{2-0} = Slot.Value;
  let TSFlags{3} = HasImm;
  let TSFlags{4} = IsPredicated;
  let TSFlags{5} = OutOfSlotData;

  // Immediate helper fields
  bits<1> hasImmField = 0;
  let TSFlags{6} = hasImmField;
  bits<5> startBit = 0;
  let TSFlags{11-7} = startBit;
  bits<5> endBit = 0;
  let TSFlags{16-12} = endBit;
  bits<4> immOpNum = 0;
  let TSFlags{20-17} = immOpNum;
  // Bits 21, 22, 23 are used by LOOP instruction
  let TSFlags{29-24} = opc;     // Slot operation code
  let TSFlags{33-30} = OType;

  // Composite immediate support
  bits<2> ImmOpCount    = 0;
  bits<4> SecondImmOp   = 0;
  bits<4> ThirdImmOp    = 0;
  bits<4> FirstImmBits  = 0;
  bits<4> SecondImmBits = 0;
  bits<4> ThirdImmBits  = 0;

  let TSFlags{34}    = HasCompositeImm;
  let TSFlags{36-35} = ImmOpCount;
  let TSFlags{40-37} = SecondImmOp;
  let TSFlags{44-41} = ThirdImmOp;
  let TSFlags{48-45} = FirstImmBits;
  let TSFlags{52-49} = SecondImmBits;
  let TSFlags{56-53} = ThirdImmBits;
  
  bits<3> Lanes = 0;
  let TSFlags{59-57} = Lanes;
}

//------ Scalar basic classes --------------------------------------------------

class ScalarInstGeneric<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, SpuSlot, ?> {
  let AsmVariantName = "scalar";
  let DecoderNamespace = "ScalarSlot";

  bits<7> Switches = 0;

  let Inst{5-0}   = Opcode;
  let Inst{42-36} = Switches;
}

// Generic class for instructions with predicates.
// All instructions except NOP and HALT use predicates.
//
class ScalarInstPred<bits<6> opc, string asmstr>
  : ScalarInstGeneric<opc, asmstr> {
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;

  let Inst{31}    = PredPolarity;
  let Inst{35-32} = PredAddress;
}

class ScalarInstSrcA<bits<6> opc, string asmstr>
  : ScalarInstPred<opc, asmstr> {
  bits<7> SrcA = 0;

  let Inst{12-6}  = SrcA;
}

class ScalarInstDest<bits<6> opc, string asmstr>
  : ScalarInstSrcA<opc, asmstr> {
  bits<7> Dest = 0;

  let Inst{26-20} = Dest;
}

class ScalarInstSrcB<bits<6> opc, string asmstr>
  : ScalarInstDest<opc, asmstr> {
  bits<7> SrcB = 0;

  let Inst{19-13} = SrcB;
}

class ScalarInstBase<bits<6> opc, string asmstr>
  : ScalarInstSrcB<opc, asmstr> {
  bits<4> OperandType = OpType.FP32;

  let Inst{30-27} = OperandType;
}

class TPCInstScalarNoTypeGen1<bits<6> opc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  bits<2> RoundMode = 0;
  bits<1> ToFlag = 0;

  let OperandType = { 0, ToFlag, RoundMode };
}

class TPCInstScalarNoTypeGen2<bits<6> opc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  bits<3> RoundMode = 0;
  bits<1> ToFlag = 0;

  let OperandType = { ToFlag, RoundMode };
}

class TPCInstScalar<bits<6> opc, string asmstr, bits<4> otype>
  : ScalarInstBase<opc, asmstr> {
  let OperandType = otype;
  let OType = otype; // TODO: remove it
}

//------ Vector basic classes --------------------------------------------------

class VectorInstGeneric<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, VpuSlot, ?> {
  let AsmVariantName = "vector";
  let DecoderNamespace = "VectorSlot";

  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;

  let Inst{5-0}   = Opcode;             // 47-42 = 5
  let Inst{54}    = PredPolarity;       // 97-42 = 54
  let Inst{58-55} = PredAddress;        // 101-42 = 58
  let Inst{59}    = VectorPred;         // 102-42 = 59

  bits<1> srcCIsStoreSrcC = 0;
  let TSFlags{61} = srcCIsStoreSrcC;

  bits<1> hasSrcC = 0;
  let TSFlags{62} = hasSrcC;

  bits<1> hasSrcD = 0;
  let TSFlags{63} = hasSrcD;
}

class VectorInstShort<bits<6> opc, string asmstr>
  : VectorInstGeneric<opc, asmstr> {
  bits<8> SrcA = 0;
  bits<8> SrcB = 0;
  bits<8> Dest = 0;
  bits<7> Switches = 0;
  bits<4> OperandType = OpType.FP32;

  let Inst{13-6}  = SrcA;               // 55-42 = 13
  let Inst{21-14} = SrcB;               // 63-42 = 21
  let Inst{46-39} = Dest;               // 88-42 = 46
  let Inst{49-47} = Switches{2-0};      // 91-42 = 49
  let Inst{53-50} = OperandType;        // 96-42 = 53
  let Inst{63-60} = Switches{6-3};
}

class VectorInstBase<bits<6> opc, string asmstr>
  : VectorInstGeneric<opc, asmstr> {
  bits<8> SrcA = 0;
  bits<8> SrcB = 0;
  bits<8> SrcC = 0;
  bits<9> SrcD = 0;
  bits<8> Dest = 0;
  bits<7> Switches = 0;
  bits<4> OperandType = OpType.FP32;

  let Inst{13-6}  = SrcA;               // 55-42 = 13
  let Inst{21-14} = SrcB;               // 63-42 = 21
  let Inst{29-22} = SrcC;               // 71-42 = 29
  let Inst{38-30} = SrcD;               // 80-42 = 38
  let Inst{46-39} = Dest;               // 88-42 = 46
  let Inst{49-47} = Switches{2-0};      // 91-42 = 49
  let Inst{53-50} = OperandType;        // 96-42 = 53
  let Inst{63-60} = Switches{6-3};
}

class TPCInstVectorNoTypeGen1<bits<6> opc, string asmstr>
  : VectorInstBase<opc, asmstr> {
  bits<2> RoundMode = 0;
  bits<1> ToFlag = 0;

  let OperandType = { 0, ToFlag, RoundMode };
}

class TPCInstVectorNoTypeGen2<bits<6> opc, string asmstr>
  : VectorInstBase<opc, asmstr> {
  bits<3> RoundMode = 0;
  bits<1> ToFlag = 0;

  let OperandType = { ToFlag, RoundMode };
}

class TPCInstVector<bits<6> opc, string asmstr, bits<4> otype>
  : VectorInstBase<opc, asmstr> {
  let OperandType = otype;
  let OType = otype; // TODO: remove it
}

//------ Store basic classes ---------------------------------------------------

// Generic store class. Only 'opcode' field is used.
//
class StoreInstGeneric<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, StoreSlot, ?> {
  let AsmVariantName = "store";
  let DecoderNamespace = "StoreSlot";

  // Always define Switch field. In architectures where it is absent, the field
  // is always zero.
  bits<7> Switches = 0;

  let Inst{20-16} = Opcode{4-0};    // 144-124 = 20
  let Inst{41-35} = Switches;       // Out of slot.
}

// Generic class for instructions with predicates.
// All instructions except NOP use predicates.
//
class StoreInstGenericPred<bits<6> opc, string asmstr>
  : StoreInstGeneric<opc, asmstr> {

  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;

  let Inst{21}    = PredPolarity;   // 145-124 = 21;
  let Inst{25-22} = PredAddress;    // 149-124 = 25;
  let Inst{26}    = VectorPred;     // 150-124 = 26;
}

// Generic class for instructions that use SrcA.
// Almost all store instructions use SrcA. Exceptions are:
// - NOP,
// - CACHE_*
//
class StoreInstGenericSrcA<bits<6> opc, string asmstr>
  : StoreInstGenericPred<opc, asmstr> {
  bits<8> SrcA = 0;

  let Inst{7-0}   = SrcA;           // 131-124 = 8
}

// Generic class for instructions that use SrcC.
// All such instructions also use SrcA.
//
class StoreInstGenericSrcC<bits<6> opc, string asmstr>
  : StoreInstGenericSrcA<opc, asmstr> {
  bits<8> SrcExtra = 0;

  let Inst{34-27} = SrcExtra;
}

// Generic class for instructions that use SrcB.
// All such instructions also use SrcC (and SrcA).
//
class StoreInstGenericSrcB<bits<6> opc, string asmstr>
  : StoreInstGenericSrcC<opc, asmstr> {
  bits<8> SrcB = 0;

  let Inst{15-8}  = SrcB;           // 139-124 = 15
}

//------ Load basic classes ----------------------------------------------------

// Generic load class. Only 'opcode' field is used.
//
class LoadInstGeneric<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, LoadSlot, ?> {

  let AsmVariantName = "load";
  let DecoderNamespace = "LoadSlot";
  bits<7> Switches = 0;

  let Inst{20-16} = Opcode{4-0};
  let Inst{63-57} = Switches{6-0};
}

// Generic class for instructions with predicates.
// All instructions except NOP use predicates.
//
class LoadInstGenericPred<bits<6> opc, string asmstr>
  : LoadInstGeneric<opc, asmstr> {
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;

  let Inst{42}    = PredPolarity;
  let Inst{46-43} = PredAddress;
  let Inst{47}    = VectorPred;
}

// Generic class for instructions that use SrcA.
//
class LoadInstGenericSrcA<bits<6> opc, string asmstr>
  : LoadInstGenericPred<opc, asmstr> {
  bits<8> SrcA = 0;

  let Inst{7-0}   = SrcA;           // 131-124 = 8
}


// Generic class for instructions that use Dest and SrcA.
// Almost all instructions in load slot use Dest and SrcA. Exceptions are:
// - PREFETCH: uses SrcA, but does not use Dest,
// - MOVs from HW reg: use Dest but do not use SrcA.
//
class LoadInstGenericWithDest<bits<6> opc, string asmstr>
  : LoadInstGenericSrcA<opc, asmstr> {
  bits<8> Dest = 0;

  let Inst{15-8}  = Dest;           // 118-103 = 15
}

class LoadInstGenericSrcB<bits<6> opc, string asmstr>
  : LoadInstGenericWithDest<opc, asmstr> {
  bits<9> SrcExtra = 0;

  let Inst{56-48} = SrcExtra;
}

class TPCInstLoad<bits<6> opc, string asmstr>
  : LoadInstGeneric<opc, asmstr> {

  bits<8> SrcA = 0;
  bits<8> Dest = 0;
  bits<9> SrcExtra = 0;
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;
  bits<7> Switches = 0;
  // The size of Load Switches differs between Gen2 (size = 4) and Gen3 (size = 5).
  // In order to not duplicate formats for Gen2 and Gen3, let's use the biggest size
  // here. These bits are out-of-slot, anyway, so they are encoded specially

  let Inst{7-0}   = SrcA;           // 110-103 = 7
  let Inst{15-8}  = Dest;           // 118-103 = 15
                                    // The next fields are out-of-slot, how should we encode them?
  let Inst{42}    = PredPolarity;
  let Inst{46-43} = PredAddress;
  let Inst{47}    = VectorPred;
  let Inst{56-48} = SrcExtra;
  let Inst{63-57} = Switches{6-0};
}

//===----------------------------------------------------------------------===//
//                         VPU Formats
//===----------------------------------------------------------------------===//

// Dummy format for predicate issue workaround (see instruction in TPCInstrInfo.td)
class VPUInstDummy<bits<6> opc, string asmstr, list<dag> pattern = []>
  : TPCInstVector<opc, asmstr, OpType.UINT32> {
  let OutOperandList = (outs);
  let InOperandList = (ins VRF:$op1, VRF:$op2, VRF:$op3, VRF:$op4, VRF:$op5,
                            VRF:$op6, VRF:$op7, VRF:$op8, VRF:$op9, VRF:$op10,
                            VRF:$op11, VRF:$op12, VRF:$op13, VRF:$op14, VRF:$op15,
                            VRF:$op16, VRF:$op17, VRF:$op18, VRF:$op19, VRF:$op20);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

}

// Template for vector slot instructions, which execute unconditionally,
// predicates are not pertinent for them. These are NOP and HALT.
//
class VpuInstUncond<bits<6> opc, string asmstr, list<dag> pattern = []>
  : VectorInstGeneric<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = 0;
}


class VpuInst_MovFromHWRegG<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins i8imm:$reg, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<6> reg;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = ?;
  let SrcB = { 0b0, 0, reg };     // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_MovToHWRegG<bits<6> opc, RegisterClass Rsrc, string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$reg, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<6> reg;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, 1, reg };     // TO_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
}


class VpuInst_MovFromHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rdst), "VRF"), 0, 1);

  bits<8> dest;
  bits<6> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = ?;
  let SrcB = { 0b0, 0, src };     // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), "VRF"), 0, 1);

  bits<6> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, 1, dest };    // TO_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_Mov<bits<6> opc,
                  RegisterClass Rdst, RegisterClass Rsrc,
                  string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = sw;
  let OperandType = OpType.FP32;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


// This is exactly same as VpuInst_Mov but operand type is INT32.
//
class VpuInst_MovBit<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc,
                     string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = sw;
  let OperandType = OpType.INT32;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_MovType<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;
  let isAsmParserOnly = 1;

  bits<8> dest;
  bits<8> src;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = optype;
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints  = "$dest = $income";
}


class VpuInst_MovIType<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<8> dest;
  bits<8> src;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = optype;
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let Constraints  = "$dest = $income";
}


class VpuInst_MovFlavor<bits<6> opc,
                        RegisterClass Rdst, DAGOperand Src,
                        string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src:$src, i8imm:$flavor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> src;
  bits<8> flavor;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let Dest = dest;
  let SrcA = src;
  let SrcB = flavor;
  let OperandType  = OpType.UINT32;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = !isa<Operand>(Src);
  let immOpNum = 1;
  let Constraints = "$dest = $income";
}

class VpuInst_MovFlavorT<bits<6> opc,
                         RegisterClass Rdst, DAGOperand Src,
                         string asmstr, Operand Pred>
  : VpuInst_MovFlavor<opc, Rdst, Src, asmstr, Pred> {
  let InOperandList = (ins Src:$src, i8imm:$flavor, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let isAsmParserOnly = 1;
}

class VpuInst_MovFlavorBroadcast<bits<6> opc,
                                 RegisterClass Rdst, DAGOperand Src,
                                 string asmstr, Operand Pred>
  : VpuInst_MovFlavor<opc, Rdst, Src, asmstr, Pred> {
  let isReMaterializable = !isa<Operand>(Src);
  let isAsCheapAsAMove = !isa<Operand>(Src);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let SrcB = 0b00001000;
}


class VpuInstBinSwT<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype{3-0};
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInstBinSwTI<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                     string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype{3-0};
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInstBinSwTA<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                     string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> dt;
  bits<7> sw;
  bits<1> acc;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInstBinSwTAI<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc2,
                      string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc2>:$op2, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> dt;
  bits<7> sw;
  bits<1> acc;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInstBinSwReserved<bits<6> opc,
                   RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                   string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<3> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{2-0} = sw;
  let Switches{0} = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_MAC_X2<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2, bit accI16,
                     string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
   : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = !if(accI16, (ins Rsrc1:$op1, Rsrc2:$op2, VRF:$op3, VRF:$op4, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred),
                                  (ins Rsrc1:$op1, Rsrc2:$op2, VRF:$op3, VRF:$op4, DataType:$dt, SwitchSet:$sw,              Rdst:$income, Pred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<8> op4;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let SrcD{7-0} = op4;
  let OperandType = optype{3-0};
  let Switches = sw;
  let hasSrcC = 1;
  let hasSrcD = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInstBinSwIReserved<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<3> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{2-0} = sw;
  let Switches{0} = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInstBinSwD<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                     string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, RhazRsOp:$rhaz, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<3> sw;
  bits<1> rhaz;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{2-0} = sw;
  let Switches{0} = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInstBinSwDI<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc2,
                     string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc2>:$op2, DataType:$optype, SwitchSet:$sw, RhazRsOp:$rhaz, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<3> sw;
  bits<1> rhaz;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{2-0} = sw;
  let Switches{0} = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let Constraints = "$dest = $income";
}

class VpuInst_UnarySw<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), "VRF"), 0, 1);

  bits<8> dest;
  bits<8> op;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let Switches{3-0} = sw{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_FCLASS_LIMIT<bits<6> opc,
                           RegisterClass Rdst, RegisterClass Rsrc1,
                           RegisterClass Rsrc2, RegisterClass Rsrc3,
                           string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, Rsrc3:$op3, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = 1;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcD{7-0} = op3;
  let OperandType = optype;
  let Switches{0} = sw{0};
  let hasSrcD = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_UnarySwI<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$op, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let Switches{2-0} = sw{2-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInst_NEARBYINT<bits<6> opc,
                        RegisterClass Rdst, RegisterClass Rsrc,
                        string asmstr, Operand Pred, list<dag> pattern = []>
  : VpuInst_UnarySw<opc, Rdst, Rsrc, asmstr, Pred, pattern> {
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rdst), "VRF"), 0, 1);
  let DecoderMethod = "decodeNearbyint";
  let Switches = { sw{6-3}, sw{18-16} };   // round_mode + switches
  let SrcB = sw{15-8};
}

class VpuInst_PACK<bits<6> opc,
                   RegisterClass Rdst, RegisterClass Rsrc,
                   string asmstr, Operand Pred, list<dag> pattern = []>
  : VpuInst_UnarySw<opc, Rdst, Rsrc, asmstr, Pred, pattern> {
  let SrcB{4-0} = sw{12-8};   // Dual group, stride and unpack_lane
}

class VpuInstBinSw<bits<6> opc,
                   RegisterClass Rdst, RegisterClass Rsrc1, DAGOperand Rsrc2,
                   string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = !if(!and(!eq(!cast<string>(Rsrc1), "VRF"),
                                   !eq(!cast<string>(Rsrc2), "VRF")), 0, 1);
  let HasImm   = !isa<Operand>(Rsrc2);
  let immOpNum = 2;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<4> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{3-0} = sw;

  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInstBinSwI<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<4> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches{3-0} = sw;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class VpuInst_Sel<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc1,
                  DAGOperand Src2, RegisterClass Rsrc3, DAGOperand Src4,
                  Operand Pred, string asmstr, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Src2:$op2, Rsrc3:$op3, Src4:$op4,
                       DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<9> op4;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let SrcD = op4;
  let OperandType = optype;
  let Switches = sw;
  let hasSrcC = 1;
  let hasSrcD = 1;

  bit Src2IsVRF = !eq(!cast<string>(Src2), "VRF");
  bit Src4IsVRF = !eq(!cast<string>(Src4), "VRF");
  let isNotUsedInDisasm = !if(!and(Src2IsVRF, Src4IsVRF), 0, 1);

  bit Src2IsImm = !isa<Operand>(Src2);
  bit Src4IsImm = !isa<Operand>(Src4);
  let HasImm = !or(Src2IsImm, Src4IsImm);
  let immOpNum  = !if(Src2IsImm, 2, 4);
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_Msac<bits<6> opc, RegisterClass Rsrc1, DAGOperand Src2,
                                RegisterClass Rsrc3, DAGOperand Src4,
                                Operand Pred, string asmstr, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = (ins Rsrc1:$op1, Src2:$op2, Rsrc3:$op3, Src4:$op4,
                       DataType:$optype, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<9> op4;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let SrcD = op4;
  let OperandType = optype;
  let Switches{2-1} = sw{2-1};
  let hasSrcC = 1;
  let hasSrcD = 1;

  bit Src2IsVRF = !eq(!cast<string>(Src2), "VRF");
  bit Src4IsVRF = !eq(!cast<string>(Src4), "VRF");
  let isNotUsedInDisasm = !if(!and(Src2IsVRF, Src4IsVRF), 0, 1);

  bit Src2IsImm = !isa<Operand>(Src2);
  bit Src4IsImm = !isa<Operand>(Src4);
  let HasImm = !or(Src2IsImm, Src4IsImm);
  let immOpNum  = !if(Src2IsImm, 2, 4);
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_MOV_DUAL_GROUP<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstShort<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dst;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let Dest = dst;
  let SrcA = src;
  let SrcB = sw{15-8};
  let Switches = 0;
  let VectorPred    = !eq(!cast<string>(Pred), "VPred");
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};

  let HasImm = 1;
  let immOpNum  = 2;
  let Constraints = "$dst = $income";
}


class VpuInst_MOV_DUAL_GROUP_ALL<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstBase<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, MovDGAllOp:$movdgall, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8>  dst;
  bits<8>  src;
  bits<32> sw;
  bits<6>  pred;

  let Dest = dst;
  let SrcA = src;
  let SrcB = sw{15-8};
  let SrcC = sw{23-16};
  let Switches{0}  = 1;   // ALL
  let hasSrcC = 1;
  let VectorPred   = !eq(!cast<string>(Pred), "VPred");
  let PredAddress  = pred{3-0};
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum  = 2;
  let Constraints = "$dst = $income";
}


class VpuInst_MOV_DUAL_GROUP_PACK<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstShort<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src, SwitchSet:$sw, MovDGPackOp:$movdgpack, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dst;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let Dest = dst;
  let SrcA = src;
  let Switches = { sw{6-2}, 0b10 };   // ALL=0, PACK=1
  let VectorPred    = !eq(!cast<string>(Pred), "VPred");
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};

  let Constraints = "$dst = $income";
}

class VpuInst_MOV_DUAL_GROUP_UNPACK<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstBase<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, MovDGUnpackOp:$movdgunpack, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8>  dst;
  bits<8>  src;
  bits<32> sw;
  bits<6>  pred;

  let Dest = dst;
  let SrcA = src;
  let SrcB = sw{15-8};
  let SrcC = sw{23-16};
  let Switches = { sw{6-2}, 0b11 };   // UNPACK=3
  let hasSrcC = 1;
  let VectorPred   = !eq(!cast<string>(Pred), "VPred");
  let PredAddress  = pred{3-0};
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum  = 2;
  let Constraints = "$dst = $income";
}

class VpuInst_GET_LUT<bits<6> opc, string asmstr, list<dag> pattern = [], Operand Pred>
  :VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs DRF:$dest);
  let InOperandList = (ins VRF:$src, i8imm:$shift, DataType:$optype, SwitchSet:$sw, DRF:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> src;
  bits<5> shift;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcB = { sw{15-13}, shift };
  let Switches{1-0} = sw{1-0};
  let OperandType = optype;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let Constraints = "$dest = $income";
}


class VpuInstConvert<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred, list<dag> pattern = []>
: VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList  = (ins  Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let isNotUsedInDisasm = !if(!and(!eq(!cast<string>(Rsrc), "VRF"),
                                   !eq(!cast<string>(Rdst), "VRF")), 0, 1);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8>  dst;
  bits<8>  src;
  bits<32> sw;
  bits<6>  pred;
  bits<4>  optype;

  let SrcA          = src;
  let SrcB{7}       = sw{7};    // X2
  let SrcB{6}       = sw{6};    // single_lane/num_lanes
  let SrcB{5-4}     = sw{1-0}; // lane_sel
  let SrcB{3-0}     = sw{11-8};  // target_type
  let Dest          = dst;
  let Switches{4-0} = sw{20-16};   // round_mode and x4
  let HasImm        = 0;
  let OperandType   = optype;
  let VectorPred    = !eq(!cast<string>(Pred), "VPred");
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let Constraints   = "$dst = $income";
}

class VpuInstConvertIntGen1<bits<6> opc, RegisterClass Rsrc2, Operand Pred,
                            string asmstr, bit hasImm>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = !if(hasImm,
                          (ins VRF:$op1, TPCImm<i32imm>:$op2, SwitchSet:$sw, VRF:$income, Pred:$pred),
                          (ins VRF:$op1,          Rsrc2:$op2, SwitchSet:$sw, VRF:$income, Pred:$pred));
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType{1-0} = sw{17-16}; // round wode
  let OperandType{2} = sw{19};      // destination type
  let Switches{1-0} = sw{1-0};      // LANE_SEL

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let HasImm = hasImm;
  let immOpNum = !if(hasImm, 2, 0);
  let Constraints = "$dest = $income";
}


class VpuInstConvertIntGen2<bits<6> opc, RegisterClass Rsrc1, RegisterClass Rsrc2,
                            Operand Pred, string asmstr, bit hasImm>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = !if(hasImm,
                          (ins Rsrc1:$op1, TPCImm<i32imm>:$op2, SwitchSet:$sw, VRF:$income, Pred:$pred),
                          (ins Rsrc1:$op1,          Rsrc2:$op2, SwitchSet:$sw, VRF:$income, Pred:$pred));
  let Itinerary = IIC_VectorOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType{2-0} = sw{18-16}; // round wode
  let OperandType{3} = sw{19};      // destination type
  let Switches = sw{6-0};

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let HasImm = hasImm;
  let immOpNum = !if(hasImm, 2, 0);
  let Constraints = "$dest = $income";
}


class VpuInst_FORM_FP_NUM<bits<6> opc, DAGOperand Src1, string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = (ins Src1:$op1, VRF:$op2, VRF:$op3, DataType:$optype, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Src1), "VRF"), 0, 1);

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let SrcD = sw{16-8};
  let hasSrcC = 1;
  let hasSrcD = 1;

  bit HasImm = !isa<Operand>(Src1);
  let immOpNum  = 1;

  let OperandType = optype;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_CALC_FP_SPECIAL<bits<6> opc, string asmstr, Operand Pred, list<dag> pattern = []>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = (ins VRF:$op1, VRF:$op2, DataType:$optype, SwitchSet:$funcId, VRF:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<6> pred;
  bits<7> funcId;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = funcId;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class VpuInst_CONVERT_FP_FLEX<bits<6> opc, DAGOperand Src2, string asmstr, Operand Pred>
: VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList  = (ins VRF:$op1, Src2:$op2, DataType:$optype, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Src2), "VRF"), 0, 1);

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest          = dest;
  let SrcA          = op1;
  let SrcB          = op2;
  let Switches{2-0} = sw{18-16}; // round
  let Switches{3}   = sw{20};    // clip_fp
  let Switches{4}   = sw{0};     // lane_sel
  let HasImm        = !isa<Operand>(Src2);
  let immOpNum      = 2;
  let OperandType   = optype;

  let VectorPred    = !eq(!cast<string>(Pred), "VPred");
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let Constraints   = "$dest = $income";
}


class VPUInstReadSpec<bits<6> opc, RegisterClass Rdst, string asmstr,
                      list<dag> pattern, bits<4> optype, bits<3> switches = 0>
  : TPCInstVector<opc, asmstr, optype> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dst;
  bits<8> src = ?;  // Must be set in instantiations
  let Dest = dst;
  let SrcA = src;
  let Switches{2-0} = switches;
}


class VPUInstWriteSpec<bits<6> opc, RegisterClass Rsrc, string asmstr,
                       list<dag> pattern, bits<4> optype, bits<3> switches = 0>
  : TPCInstVector<opc, asmstr, optype> {
  let OutOperandList = (outs);
  let InOperandList = (ins Rsrc:$src);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> dst = ?;  // Must be set in instantiations
  bits<8> src;
  let Dest = dst;
  let SrcA = src;
  let Switches{2-0} = switches;
}


class VpuInst_MOV_GROUP<bits<6> opc, string asmstr, Operand Pred, list<dag> pattern = []>
: VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorOp;

  bits<8> src;
  bits<8> dest;
  bits<6> pred;
  bits<6> sw;
  
  let SrcA         = src;
  let Dest         = dest;
  let HasImm       = 1;
  let immOpNum     = 2;
  let SrcB{5-2}    = sw{5-2}; // dual_group_en 
  let SrcB{1-0}    = sw{1-0}; // group_en
  let isNotUsedInDisasm = 0;
  let PredAddress  = pred{3-0};
  let VectorPred   = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let Constraints  = "$dest = $income";
  let DecoderMethod = "decodeMovGroup";
}

//===----------------------------------------------------------------------===//
//                         SPU Formats
//===----------------------------------------------------------------------===//

// Format for scalar slot instructions, which execute unconditionally,
// predicates are not pertinent for them. These are NOP and HALT.
//
class SpuInstUncond<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstGeneric<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;
}


class SpuInst_Mov<bits<6> opc,
                  RegisterClass Rdst, RegisterClass Rsrc,
                  string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = optype;
  let Switches = sw;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovI<bits<6> opc,
                   RegisterClass Rdst, Operand Isrc,
                   string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<7> dest;
  bits<7> src;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = optype;
  let Switches = sw;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_MovType<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr, bits<4> OpType>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = sw;
  let OperandType = OpType;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovTypeI<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr, bits<4> OpType>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<7> dest;
  bits<7> src;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = OpType;
  let Switches = sw;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


// Format for register-to-IRF moves, when mask is represented with compile time
// constant. It does not have datatype argument, as it is always INT32.
class SpuInst_MovMask<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DimMask:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<5> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = { mask, 0b00 };
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovMaskI<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DimMask:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<7> dest;
  bits<7> src;
  bits<5> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = { mask, 0b00 };
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_MovMaskT<bits<6> opc,
                       RegisterClass Rdst, RegisterClass Rsrc,
                       string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<4> optype;
  bits<7> sw;
  bits<5> mask;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = optype;
  let Switches = { mask, 0b00 };
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovMaskIT<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<4> optype;
  bits<5> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = { mask, 0b00 };
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_MovFromHWRegG<bits<6> opc, RegisterClass Rdst, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins i8imm:$reg, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isAsmParserOnly = 1;

  bits<7> dest;
  bits<6> reg;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcB = { 0, reg };      // FROM_HW_REG
  let Switches = 0b0000001;   // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovToHWRegG<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$reg, SwitchSet:$sw, Rsrc:$src, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isAsmParserOnly = 1;

  bits<6> reg;
  bits<7> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b1, reg };    // TO_HW_REG
  let Switches = 0b0000001;   // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
}


class SpuInst_MovFromHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rdst), "SRF"), 0, 1);

  bits<7> dest;
  bits<6> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = ?;
  let SrcB = { 0, src };          // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), "SRF"), 0, 1);

  bits<6> dest;
  bits<7> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b1, dest };       // TO_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_Binary<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                     string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!and(!eq(!cast<string>(Rsrc1), !cast<string>(Rsrc2)),
                                   !ne(!cast<string>(Rsrc2), "SPRF"),
                                   !ne(!cast<string>(Rsrc1), "SPRF")),
                              0, 1);

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = sw{6-0};

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInst_BinaryI<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc, Operand Isrc,
                      string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op1, TPCImm<Isrc>:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = sw{6-0};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class SpuInst_BinaryMask<bits<6> opc,
                         RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                         string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<5> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let Switches = { mask, 0b00 };

  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInst_BinaryMaskI<bits<6> opc,
                          RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc2,
                          string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc2>:$op2, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<5> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = { mask, 0b00 };
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_BinaryMaskIR<bits<6> opc,
                           RegisterClass Rdst, Operand Isrc1, RegisterClass Rsrc2,
                           string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc1>:$op1, Rsrc2:$op2, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<5> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = { mask, 0, sw{0} }; // Explicitly clear DIM_MASK_REG
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_Unary<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc,
                    string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!and(!eq(!cast<string>(Rsrc), !cast<string>(Rdst)),
                                   !ne(!cast<string>(Rsrc), "SPRF")), 0, 1);

  bits<7> dest;
  bits<7> op;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { sw{6-2}, 0, sw{0} }; // Explicitly clear DIM_MASK_REG
  let Constraints = "$dest = $income";
}


class SpuInst_UnaryMask<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc,
                    string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op;
  bits<5> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { mask, 0, sw{0} }; // Explicitly clear DIM_MASK_REG
  let Constraints = "$dest = $income";
}


class SpuInst_UnarySw<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), !cast<string>(Rdst)), 0, 1);

  bits<7> dest;
  bits<7> op;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = sw{6-0};
  let Constraints = "$dest = $income";
}


class SpuInst_UnarySwI<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$op, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = sw;
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class SpuInst_NEARBYINT<bits<6> opc,
                        RegisterClass Rdst, RegisterClass Rsrc,
                        string asmstr, list<dag> pattern = []>
  : SpuInst_UnarySw<opc, Rdst, Rsrc, asmstr, pattern> {
  let Switches{2-0} = sw{18-16};  // round_mode
}


class SpuInstBinarySw<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                      string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = sw;

  bit Src1IsSRF = !eq(!cast<string>(Rsrc1), "SRF");
  bit Src2IsSRF = !eq(!cast<string>(Rsrc2), "SRF");
  let isNotUsedInDisasm = !if(!and(Src1IsSRF, Src2IsSRF), 0, 1);

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInstBinarySwI<bits<6> opc,
                       RegisterClass Rdst, RegisterClass Rsrc, Operand Isrc,
                       string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op1, TPCImm<Isrc>:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = sw;
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


// Formats for ASH
class SpuInstBinarySwReserved<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                      string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = sw;
  let Switches{0} = 1;

  bit Src1IsSRF = !eq(!cast<string>(Rsrc1), "SRF");
  bit Src2IsSRF = !eq(!cast<string>(Rsrc2), "SRF");
  let isNotUsedInDisasm = !if(!and(Src1IsSRF, Src2IsSRF), 0, 1);

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInstBinarySwIReserved<bits<6> opc,
                       RegisterClass Rdst, RegisterClass Rsrc, Operand Isrc,
                       string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op1, TPCImm<Isrc>:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = sw;
  let Switches{0} = 1;
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class SpuInst_MovDim<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc,
                     string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, i8imm:$dim, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<7> dest;
  bits<7> src;
  bits<3> dim;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { sw{6-3},  dim };
  let Switches{3} = 0;
  let Constraints = "$dest = $income";
}

class SpuInst_MovIrfDivBoth<bits<6> opc, RegisterClass Rdst,
                            RegisterClass Rsrc, string asmstr>
: SpuInst_MovDim<opc, Rdst, Rsrc, asmstr> {
  let Switches{3} = sw{3};
}


class SpuInst_ConvertINT64<bits<6> opc, string asmstr, list<dag> pattern = []>
: ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dst);
  let InOperandList =  (ins SRF:$op1, SRF:$op2, SwitchSet:$sw, SRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<7> dst;
  bits<7> op1;
  bits<7> op2;
  bits<6> pred;

  let SrcA          = op1;
  let SrcB          = op2;
  let Switches      = 0;
  let Dest          = dst;
  let HasImm        = 0;
  let OperandType   = OpType.INT64;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let Constraints   = "$dst = $income";
  let DecoderMethod = "decodeConvertScalar";
}


class SpuInst_Convert<bits<6> opc, string asmstr, list<dag> pattern = []>
: ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dst);
  let InOperandList  = (ins  SRF:$src, DataType:$optype, SwitchSet:$sw, SRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<7> dst;
  bits<7> src;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let SrcA          = src;
  let SrcB{3-0}     = sw{11-8};   // target type
  let Switches{4-0} = sw{20-16};  // round_mode, X4, ClipFp
  let Dest          = dst;
  let HasImm        = 0;
  let OperandType   = optype;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let Constraints   = "$dst = $income";
  let DecoderMethod = "decodeConvertScalar";
}



class SpuInstConvertIntGen1<bits<6> opc, string asmstr, bit hasImm>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dest);
  let InOperandList = !if(hasImm,
                        (ins SRF:$op1, TPCImm<i32imm>:$op2, SwitchSet:$sw, SRF:$income, SPred:$pred),
                        (ins SRF:$op1, SRF:$op2, SwitchSet:$sw, SRF:$income, SPred:$pred));
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType{1-0} = sw{17-16}; // round wode
  let OperandType{2} = sw{19};      // destination type
  let Switches{1-0} = sw{1-0};      // LANE_SEL

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = hasImm;
  let immOpNum = !if(hasImm, 2, 0);
  let Constraints = "$dest = $income";
}

class SpuInstConvertIntGen2<bits<6> opc, string asmstr, bit hasImm>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dest);
  let InOperandList = !if(hasImm,
                        (ins SRF:$op1, TPCImm<i32imm>:$op2, SwitchSet:$sw, SRF:$income, SPred:$pred),
                        (ins SRF:$op1, SRF:$op2,            SwitchSet:$sw, SRF:$income, SPred:$pred));
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType{2-0} = sw{18-16}; // round wode
  let OperandType{3} = sw{19};      // destination type
  let Switches{1-0} = sw{1-0};      // LANE_SEL
  let Switches{2} = sw{2};          // NUM_LANES

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = hasImm;
  let immOpNum = 2;
  let Constraints = "$dest = $income";
}

class SpuInstConvertIntGen3<bits<6> opc, string asmstr, bit hasImm>
  : SpuInstConvertIntGen2<opc, asmstr, hasImm> {
  let isNotUsedInDisasm = !if(hasImm, 1, 0);
}

class SpuInst_CONVERT_FP_FLEX<bits<6> opc, string asmstr, bit hasImm>
: ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dest);
  let InOperandList  = !if(hasImm,
                          (ins SRF:$op1, TPCImm<i32imm>:$op2, DataType:$optype, SwitchSet:$sw, SRF:$income, SPred:$pred),
                          (ins SRF:$op1, SRF:$op2, DataType:$optype, SwitchSet:$sw, SRF:$income, SPred:$pred));
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(hasImm, 1, 0);

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest          = dest;
  let SrcA          = op1;
  let SrcB          = op2;
  let Switches{2-0} = sw{18-16}; // round
  let Switches{3}   = sw{20};    // clip_fp
  let HasImm        = hasImm;
  let immOpNum      = 2;
  let OperandType   = optype;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let Constraints   = "$dest = $income";
}

class SpuInst_UDIV_STEP<bits<6> opc, string asmstr, bit stepReg, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs ZRF:$dest);
  let InOperandList = (ins SRF:$src, i32imm:$step, DataType:$optype, SwitchSet:$sw, ZRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src;
  bits<5> step;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches{4-0} = !if(stepReg, {0b00000}, {step});
  let Switches{5} = stepReg;
  let Switches{6} = sw{6}; //x2
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInst_UDIV<bits<6> opc, string asmstr, bit isBoth>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = !if(isBoth,
                           (outs ZRF:$dest),
                           (outs SRF:$dest));
  let InOperandList = !if(isBoth,
                          (ins SRF:$src1, SRF:$src2, DataType:$optype, SwitchSet:$sw, BothDivModOp:$bothdiv, ZRF:$income, SPred:$pred),
                          (ins SRF:$src1, SRF:$src2, DataType:$optype, SwitchSet:$sw, SRF:$income, SPred:$pred));
  let Itinerary = IIC_ScalarOp;

  bits<7> dest;
  bits<7> src1;
  bits<7> src2;
  bits<4> optype;
  bits<7> sw;
  bits<1> bothdiv;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src1;
  let SrcB = src2;
  let Switches = sw;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

//------ Classes for JMP* instructions
//
// We have to have specific instruction for unconditional jump because such
// jump must be marked with 'isBarrier' attribute.

class SpuInstJmpU<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$target);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let DecoderMethod = "decodeJmpTargetImm";

  let SrcA = 0b1111111;
  let HasImm = 1;
  let immOpNum = 0;
  let PredAddress = 0b0000;
  let OutOfSlotData = 1;
}

class SpuInstJmp<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$target, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let DecoderMethod = "decodeJmpTargetImm";

  bits<6> pred;

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let SrcA = 0b1111111;
//  let IsPredicated = 1;
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
}

class SpuInstJmpR<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SRF:$target, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> target;
  bits<6> pred;

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let SrcA = target;
//  let IsPredicated = 1;
  let OutOfSlotData = 1;
}


class SpuInst_SET_INDX<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<7> dst;
  bits<7> src;
  bits<5> mask;
  bits<6> sw;
  bits<6> pred;

  let Dest = dst;
  let SrcA = src;
  let Switches =  { mask, MaskInReg, sw{0} };   // DIM_MASK_REG
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dst = $income";
}


class SpuInst_SET_INDXI<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<7> dst;
  bits<7> src;
  bits<5> mask;
  bits<6> sw;
  bits<6> pred;

  let Dest = dst;
  let SrcA = src;
  let Switches = { mask, MaskInReg, sw{0} };    // DIM_MASK_REG
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 2;
  let Constraints = "$dst = $income";
}


class SpuInst_CALC_FP_SPECIAL<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs SRF:$dest);
  let InOperandList = (ins SRF:$op1, SRF:$op2, DataType:$optype, SwitchSet:$funcId, SRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<7> dest;
  bits<7> op1;
  bits<7> op2;
  bits<4> optype;
  bits<6> pred;
  bits<7> funcId;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let OperandType = optype;
  let Switches = funcId;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SPUInstRI<bits<6> opc, RegisterClass Rdst, Operand Isrc, string asmstr,
                list<dag> pattern = [], bits<4> optype = OpType.UINT32, bits<7> switches = 0>
  : TPCInstScalar<opc, asmstr, optype> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins TPCImm<Isrc>:$src);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<7> src;

  let SrcA = src;
  let HasImm = 1;
  let immOpNum = 1;
  bits<7> dst;
  let Dest = dst;
  let Switches = switches;
}


//===----------------------------------------------------------------------===//
//                         Load Formats
//===----------------------------------------------------------------------===//


// Format for load slot instructions, which execute unconditionally,
// predicates are not pertinent for them. These are NOP and HALT.
//
class LoadInstUncond<bits<6> opc, string asmstr, list<dag> pattern = []>
  : LoadInstGeneric<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_LoadOp;
  let isNotUsedInDisasm = 0;
}


class LoadInst_Mov<bits<6> opc,
                   RegisterClass Rdst, RegisterClass Rsrc,
                   string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let Dest = dest;
  let Switches = { sw{6-1}, 0 };   // HW_REG
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let Constraints = "$dest = $income";
}


class LoadInst_MOVnotype<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let Constraints = "$dest = $income";
}


class LoadInst_MovType<bits<6> opc,
                       RegisterClass Rdst, RegisterClass Rsrc,
                       string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<8> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b0, optype{3-0}, 0b0000 };
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovTypeI<bits<6> opc, RegisterClass Rdst, Operand Isrc, string asmstr, Operand Pred, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<8> dest;
  bits<8> src;
  bits<8> optype;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b0, optype{3-0}, 0b0000 };
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovI<bits<6> opc, RegisterClass Rdst, Operand Isrc, string asmstr>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let Dest = dest;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovFlavor<bits<6> opc,
                         RegisterClass Rdst, DAGOperand Src,
                         string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src:$src, i32imm:$flavor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<8> flavor;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00000, flavor{3-0} };
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let HasImm = !isa<Operand>(Src);
  let immOpNum = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_MovFlavorT<bits<6> opc, RegisterClass Rdst, DAGOperand Src, string asmstr, Operand Pred>
  : LoadInst_MovFlavor<opc, Rdst, Src, asmstr, Pred> {
  let InOperandList = (ins Src:$src, i32imm:$flavor, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let isAsmParserOnly = 1;
}

class LoadInst_MovFlavorBroadcast<bits<6> opc,
                                  RegisterClass Rdst, DAGOperand Src,
                                  string asmstr, Operand Pred>
  : LoadInst_MovFlavor<opc, Rdst, Src, asmstr, Pred> {
  let isReMaterializable = !isa<Operand>(Src);
  let isAsCheapAsAMove = !isa<Operand>(Src);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let SrcExtra = 0b000001000;
 }


class LoadInst_MovMask<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DimMask:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<8> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, mask{4-0}, 0b00 };
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovMaskI<bits<6> opc, RegisterClass Rdst, Operand Isrc, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, DimMask:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<8> dest;
  bits<8> src;
  bits<8> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, mask{4-0}, 0b00 };
  let Switches = sw;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovIRF<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, 0b11111, 0b00 };
  let Switches = sw;

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovIRFT<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins DataType:$optype, Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let Dest = dest;
  let SrcExtra = { 0b00, 0b11111, 0b00 };
  let Switches = sw;

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovFromHWRegG<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins i8imm:$reg, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;
  let isAsmParserOnly = 1;

  bits<8> dest;
  bits<6> reg;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcExtra = { 0b00, 0, reg };  // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };    // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovToHWRegG<bits<6> opc, RegisterClass Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$reg, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;
  let isAsmParserOnly = 1;

  bits<6> reg;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, 1, reg };  // TO_HW_REG
  let Switches = { sw{6-1}, 1 };    // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
}


class LoadInst_MovFromHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rdst), "VRF"), 0, 1);

  bits<8> dest;
  bits<6> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = ?;
  let SrcExtra = { 0b00, 0, src };  // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };    // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), "VRF"), 0, 1);

  bits<6> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, 1, dest }; // TO_HW_REG
  let Switches = { sw{6-1}, 1 };    // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInstReadSpec<bits<6> opc, RegisterClass Rdst, string asmstr,
                       list<dag> pattern>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_LoadOp;

  bits<8> dst;
  bits<8> src; // Must be set in instantiations

  let Dest = dst;
  let SrcA = src;
}

class LoadInstWriteSpec<bits<6> opc, RegisterClass Rsrc, string asmstr,
                        list<dag> pattern>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins Rsrc:$src);
  let Pattern = pattern;
  let Itinerary = IIC_LoadOp;

  bits<8> dst;  // Must be set in instantiations
  bits<8> src;

  let Dest = dst;
  let SrcA = src;
}

class LoadInst_GEN_ADDR<bits<6> opc, string asmstr, bit TnsrReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs ADRF:$dest);
  let InOperandList = (ins       i8imm:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<9> tensor;
  bits<8> coords;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = tensor;
  let Dest = dest;
  let Switches = sw;
  let Switches{5} = TnsrReg;  // TNSR_ID_REG

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInstrPRMT_INDX<bits<6> opc, string asmstr, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs IRF:$dest);
  let InOperandList  = (ins IRF:$src1, SRF:$src2, SwitchSet:$sw, IRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadScalLocal;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> src1;
  bits<9> src2;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src1;
  let SrcExtra = src2;
  let Dest = dest;
  let Switches = sw;

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_EVENT<bits<6> opc, DAGOperand Src, string asmstr, list<dag> pattern = []>
  : LoadInstGenericSrcA<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList  = (ins Src:$src, SwitchSet:$sw, SPred:$pred);
  let HasImm   = !isa<Operand>(Src);
  let immOpNum = 0;
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;
  let hasSideEffects = 1;


  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let Switches{0}  = sw{0};
  let PredAddress  = pred{3-0};
  let VectorPred   = 0;
  let PredPolarity = pred{5};
}

class LoadInst_SET_INDX<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dst;
  bits<8> src;
  bits<5> mask;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, mask, 0b00 };
  let Dest = dst;
  let Switches = sw;
  let Switches{1} = MaskInReg;    // DIM_MASK_REG
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dst = $income";
}


class LoadInst_SET_INDXI<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, mask, 0b00 };
  let Dest = dst;
  let Switches = sw;
  let Switches{1} = MaskInReg;    // DIM_MASK_REG
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dst = $income";
}


class LoadInst_LD_G<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins ADRF:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadScalGlobal;
  let mayLoad = 1;

  bits<8> dest;
  bits<8> addr;
  bits<32> sw;
  bits<6> pred;

  let SrcA = addr;
  let Dest = dest;
  let SrcExtra = sw{16-8};
  let Switches{0} = sw{0}; // AUTO_INC
  let Switches{1} = 0;     // DIM_MASK_REG
  let Switches{6-2} = sw{6-2};
  let PredAddress = pred{3-0};
  let VectorPred  = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let OutOfSlotData = 0;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_Gg4<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G<opc, Rdst, asmstr, Pred> {
  let Switches{2-0} = sw{2-0};  // AUTO_INC
  let Switches{3} = 0;          // DIM_MASK_REG
  let Switches{6-4} = sw{6-4};
}

class LoadInst_LD_G_INC<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G<opc, Rdst, asmstr, Pred> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
  let Switches{0} = 1; // AUTO_INC
}

class LoadInst_LD_Gg4_INC<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_Gg4<opc, Rdst, asmstr, Pred> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
}

class LoadInst_LD_GI<bits<6> opc, RegisterClass Rdst, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins ADRF:$addr, DimMask:$dimmask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_LoadScalGlobalMask;
  let mayLoad = 1;

  bits<8> dest;
  bits<8> addr;
  bits<5> dimmask;
  bits<7> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcExtra = { 0b00, dimmask, 0b00 };
  let Dest = dest;
  let Switches = sw;
  let Switches{1} = MaskInReg;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class LoadInstr_PREFETCH<bits<6> opc, string asmstr>
  : LoadInstGenericSrcA<opc, asmstr> {

  let OutOperandList = (outs);
  let InOperandList = (ins ADRF:$addr, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_LoadScalGlobal;
  let isNotUsedInDisasm = 0;

  bits<8> addr;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let SrcA = addr{7-0};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
}


class LoadInst_LD_Vrr<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins MEMrr:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadVectLocal;
  let mayLoad = 1;

  bits<8> dest;
  bits<16> addr;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let Dest = dest;
  let SrcA = addr{7-0};
  let SrcExtra = { 0, addr{15-8} };
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_Vri<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins MEMri:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadVectLocal;
  let mayLoad = 1;

  bits<8> dest;
  bits<16> addr;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let Dest = dest;
  let SrcA = 0b01111111;
  let SrcExtra = { 0, addr{7-0} };
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_Vr<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInstGenericWithDest<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins SRF:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadShortVectLocal;
  let mayLoad = 1;

  bits<8> dest;
  bits<8> addr;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let Dest = dest;
  let SrcA = addr;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class LoadInst_LD_Vi<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInstGenericWithDest<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins i32imm:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadShortVectLocal;
  let mayLoad = 1;

  bits<8> dest;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let Dest = dest;
  let SrcA = 0b01111111;
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let Constraints = "$dest = $income";
}


class LoadInst_LD_L<bits<6> opc, RegisterClass Rdst, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins SRF:$addr, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadScalLocal;
  let mayLoad = 1;

  bits<8> addr;
  bits<8> dest;
  bits<9> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcExtra = sw;
  let Dest = dest;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_LI<bits<6> opc, RegisterClass Rdst, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins i32imm:$addr, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadScalLocal;
  let mayLoad = 1;

  bits<8> addr;
  bits<8> dest;
  bits<9> sw;
  bits<6> pred;

  let SrcA = 0b01111111;
  let SrcExtra = sw;
  let Dest = dest;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let HasImm = 1;
  let immOpNum = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_TNSR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadTensor;

  bits<9> tensor;
  bits<8> coords;
  bits<8> dst;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = tensor;
  let Dest = dst;

  let Switches    = sw;
  let Switches{0} = 0; // partial
  let Switches{3} = 0; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class LoadInst_LD_TNSR_D<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_TNSR<opc, Rdst, asmstr, Pred> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, SwitchSet:$sw, Rdst:$income, Pred:$pred);

  bits<8> coords1;
  bits<9> coords2;

  let SrcA     = coords1;
  let SrcExtra = coords2;
}

class LoadInst_LD_TNSR_R<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, bit UseHWR>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords,   TnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadTensor;

  bits<8> coords;
  bits<8> dst;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let Dest = dst;

  let Switches    = sw;
  let Switches{0} = 0; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}


class LoadInst_LD_TNSR_P<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, bit UseHWR>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadTensor;

  bits<9> tensor;
  bits<8> coords;
  bits<8> dst;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = tensor;
  let Dest = dst;

  let Switches    = sw;
  let Switches{0} = 1; // partial
  let Switches{3} = 0; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class LoadInst_LD_TNSR_PD<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, bit UseHWR>
  : LoadInst_LD_TNSR_P<opc, Rdst, asmstr, Pred, UseHWR> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  bits<8> coords1;
  bits<9> coords2;

  let SrcA     = coords1;
  let SrcExtra = coords2;
}

class LoadInst_LD_TNSR_PR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, bit UseHWR>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords,   TnsrRegLd:$tensor,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadTensor;

  bits<8> coords;
  bits<8> dst;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let Dest = dst;

  let Switches    = sw;
  let Switches{0} = 1; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}


class LoadInst_LOOKUP_GEN1<bits<6> opc, RegisterClass Rdst, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins VRF:$src, i32imm:$fid, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let isNotUsedInDisasm = 0;
  let Itinerary = IIC_LoadComplexOp;

  bits<8> dest;
  bits<8> src;
  bits<6> pred;
  bits<6> fid;
  bits<32> sw;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { fid, sw{2-0} };
  let HasImm = 1;
  let immOpNum = 2;

  let PredAddress = pred{3-0};
  let VectorPred  = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;

  let Constraints = "$dest = $income";
}

class LoadInst_LOOKUP_GEN2<bits<6> opc, RegisterClass Rdst, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins VRF:$src, i32imm:$fid, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> src;
  bits<6> pred;
  bits<9> fid;
  bits<32> sw;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = fid;
  let Switches = sw{6-0};
  let HasImm = 1;
  let immOpNum = 2;

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  
  let Constraints = "$dest = $income";
}

class LoadInst_LOOKUP_GEN3<bits<6> opc, RegisterClass Rdst, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins VRF:$src, SRF:$fid, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;
  let DecoderMethod = "decodeLookupLutPtr";

  bits<8> dest;
  bits<8> src;
  bits<6> pred;
  bits<9> fid;
  bits<32> sw;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = fid;
  let Switches = {sw{6-4}, 1, sw{2-0}}; // LUT_PTR is set to 1 by default.

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  
  let Constraints = "$dest = $income";
}


//===----------------------------------------------------------------------===//
//                         Store Formats
//===----------------------------------------------------------------------===//

// Format for store slot instructions, which execute unconditionally,
// predicates are not pertinent for them. These are NOP.
//
class StoreInstUncond<bits<6> opc, string asmstr, list<dag> pattern = []>
  : StoreInstGeneric<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = pattern;
  let Itinerary = IIC_StoreOp;
  let isNotUsedInDisasm = 0;
}


// Format for instructions in store that do not produce values, do not have
// inputs, but they may be executed conditionally. These are CACHE_*.
//
class StoreInstPred<bits<6> opc, string asmstr, list<dag> pattern = []>
  : StoreInstGenericPred<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreOp;
  let isNotUsedInDisasm = 0;

  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let PredPolarity = pred{5};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
}


// Format for instructions in store that do not produce values, do not have
// inputs, but have flags in SrcA. Now only ASO satisfies these requirements.
//
class StoreInstA<bits<6> opc, string asmstr, list<dag> pattern = []>
  : StoreInstGenericSrcA<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreOp;
  let isBarrier = 1;
  let hasSideEffects = 1;
  let isNotUsedInDisasm = 0;

  bits<8> sw;
  bits<6> pred;

  let SrcA = sw;
  let PredPolarity = pred{5};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
}


class StoreInst_SET_INDX<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;
  let isNotUsedInDisasm = 0;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, mask, 0b00 };
  let SrcExtra = dst;
  let Switches = sw;
  let Switches{1} =  MaskInReg;   // DIM_MASK_REG
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let OutOfSlotData = 1;
  let Constraints = "$dst = $income";
}


class StoreInst_SET_INDXI<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, mask, 0b00 };
  let SrcExtra = dst;
  let Switches = sw;
  let Switches{1} =  MaskInReg;   // DIM_MASK_REG
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dst = $income";
}


class StoreInst_ST_L<bits<6> opc, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SRF:$addr, Rsrc:$value, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreScalLocal;
  let mayStore = 1;

  bits<8> addr;
  bits<8> value;
  bits<8> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcB = sw;
  let SrcExtra = value;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
}

class StoreInst_ST_LI<bits<6> opc, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins TPCImm<i32imm>:$addr, Rsrc:$value, SwitchSet:$sw, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreScalLocal;
  let mayStore = 1;

  bits<8> addr;
  bits<8> value;
  bits<8> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcB = sw;
  let SrcExtra = value;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let HasImm = 1;
  let immOpNum = 0;
}


class StoreInst_ST_Vrrr<bits<6> opc,
                        RegisterClass Rsrc,
                        string asmstr,
                        Operand Pred>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins MEMrr:$addr, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_StoreVectLocal;
  let mayStore = 1;

  bits<16> addr;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let SrcA = addr{7-0};
  let SrcB = addr{15-8};
  let SrcExtra = src;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
}


class StoreInst_ST_Vrir<bits<6> opc,
                        RegisterClass Rsrc,
                        string asmstr,
                        Operand Pred>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins MEMri:$addr, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_StoreVectLocal;
  let mayStore= 1;

  bits<16> addr;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let SrcA = 0b01111111;
  let SrcB = addr{7-0};
  let SrcExtra = src;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
}


class StoreInst_ST_Vrr<bits<6> opc,
                       RegisterClass Rsrc,
                       string asmstr,
                       Operand Pred>
  : StoreInstGenericSrcC<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SRF:$addr, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_StoreVectLocal;
  let mayStore = 1;

  bits<8> addr;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let SrcA = addr;
  let SrcExtra = src;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
}


class StoreInst_ST_Vir<bits<6> opc,
                       RegisterClass Rsrc,
                       string asmstr,
                       Operand Pred>
  : StoreInstGenericSrcC<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i32imm:$addr, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_StoreVectLocal;
  let mayStore = 1;

  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let Switches = sw;
  let SrcA = 0b01111111;
  let SrcExtra = src;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 0;
  let OutOfSlotData = 1;
}


class StoreInst_ST_G<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInstGenericSrcB<opc, asmstr> {

  let OutOperandList  = (outs);
  let InOperandList = (ins ADRF:$addr, Rsrc:$value, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreScalGlobal;
  let mayStore = 1;

  bits<8> addr;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcB = sw{15-8};
  let SrcExtra = value;
  let Switches{6-0} = sw{6-0};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
}

class StoreInst_ST_G_INC<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInst_ST_G<opc, Rsrc, asmstr> {
  let OutOperandList  = (outs ADRF:$new_addr);
  let isCodeGenOnly = 1;
  let Switches{0} = 1; // AUTO_INC
}

class StoreInst_GEN_ADDR<bits<6> opc, string asmstr, bit TnsrReg, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs ADRF:$dest);
  let InOperandList = (ins       i8imm:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> tensor;
  bits<8> coords;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = dest;
  let Switches = sw;
  let Switches{5} = TnsrReg;  // TNSR_ID_REG

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class StoreInst_EVENT<bits<6> opc, DAGOperand Src, string asmstr, list<dag> pattern = []>
  : StoreInstGenericSrcA<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList  = (ins Src:$src, SwitchSet:$sw, SPred:$pred);
  let HasImm   = !isa<Operand>(Src);
  let immOpNum = 0;
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;
  let hasSideEffects = 1;


  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let Switches{0}  = sw{0};
  let PredAddress  = pred{3-0};
  let VectorPred   = 0;
  let PredPolarity = pred{5};
}

class StoreInstrPRMT_INDX<bits<6> opc, string asmstr, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs IRF:$dest);
  let InOperandList  = (ins IRF:$src1, SRF:$src2, SwitchSet:$sw, IRF:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> src1;
  bits<8> src2;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src1;
  let SrcB = src2;
  let SrcExtra = dest;
  let Switches = sw;

  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class StoreInst_ST_TNSR<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, Rsrc:$value, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_D<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInst_ST_TNSR<opc, Rsrc, asmstr> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, Rsrc:$value, SwitchSet:$sw, SPred:$pred);

  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_RMW<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{1} = 1; // rmw_sel

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_RMW_D<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_RMW<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred);
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_RMW_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{1} = 1; // rmw_sel
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{0} = 1; // partial

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P_D<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_P<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_P_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{0} = 1; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P_RMW<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{0} = 1; // partial
  let Switches{1} = 1; // rmw_sel

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P_D_RMW<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_P_RMW<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_P_RMW_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = value;

  let Switches    = sw;
  let Switches{0} = 1; // partial
  let Switches{1} = 1; // rmw_sel
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_LD_TNSR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreTensor;

  bits<8> tensor;
  bits<8> coords;
  bits<8> dst;
  bits<7> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = dst;
  let SrcB = tensor;

  let Switches    = sw;
  let Switches{0} = 0; // partial
  let Switches{3} = 0; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class StoreInst_LD_TNSR_D<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}


//===----------------------------------------------------------------------===//
//                         Loop Formats
//===----------------------------------------------------------------------===//

class TPCInstLOOP<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, LoopSlot, IIC_Loop> {

  let Pattern = [];
  let OutOperandList = (outs);
  let AsmVariantName = "special";

  bit StartSel = 0;
  bit BoundarySel = 0;
  bit StepSel = 0;

  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;

  let Inst{5-0} = Opcode;
  let Inst{29} = 0;

  let Inst{35-32} = PredAddress;
  let Inst{31}    = PredPolarity;

  let TSFlags{21} = StartSel;
  let TSFlags{22} = BoundarySel;
  let TSFlags{23} = StepSel;
}

class TPCInstLOOPEND<bits<6> opc, string asmstr>
  : TPCInstGeneric<opc, asmstr, LoopSlot, NoItinerary> {

  let Pattern = [];
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$out, brtarget:$back);
}

class LOOPInstRRR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 0;
  let Inst{28} = 0;
  let Inst{38-36} = cmp;
}

class LOOPInstRRI<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 0;
  let Inst{28} = 1;
  let Inst{38-36} = cmp;
  
  let StepSel = 1;
}

class LOOPInstRIR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 1;
  let Inst{28} = 0;
  let Inst{38-36} = cmp;
  
  let BoundarySel = 1;
}

class LOOPInstIRR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 0;
  let Inst{28} = 0;
  let Inst{38-36} = cmp;

  let StartSel = 1;
}

class LOOPInstRII<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 1;
  let Inst{28} = 1;
  let Inst{38-36} = cmp;

  let BoundarySel = 1;
  let StepSel = 1;
}

class LOOPInstIRI<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 0;
  let Inst{28} = 1;
  let Inst{38-36} = cmp;

  let StartSel = 1;
  let StepSel = 1;
}

class LOOPInstIIR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 1;
  let Inst{28} = 0;
  let Inst{38-36} = cmp;

  let StartSel = 1;
  let BoundarySel = 1;
}

class LOOPInstIII<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<7> start; bits<7> boundary; bits<6> step; bits<3> cmp;
  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 1;
  let Inst{28} = 1;
  let Inst{38-36} = cmp;

  let StartSel = 1;
  let BoundarySel = 1;
  let StepSel = 1;
}

class LoopInstRRRp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins SRF:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins SRF:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 0;
  let Inst{28} = 0;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
}

class LoopInstRRIp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 0;
  let Inst{28} = 1;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let StepSel = 1;
}

class LoopInstRIRp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins SRF:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins SRF:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 1;
  let Inst{28} = 0;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let BoundarySel = 1;
}

class LoopInstIRRp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins LoopStartImm:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins LoopStartImm:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 0;
  let Inst{28} = 0;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let StartSel = 1;
}

class LoopInstRIIp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins SRF:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins SRF:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 0;
  let Inst{27} = 1;
  let Inst{28} = 1;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let BoundarySel = 1;
  let StepSel = 1;
}

class LoopInstIRIp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins LoopStartImm:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins LoopStartImm:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 0;
  let Inst{28} = 1;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let StartSel = 1;
  let StepSel = 1;
}

class LoopInstIIRp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 1;
  let Inst{28} = 0;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let StartSel = 1;
  let BoundarySel = 1;
}

class LoopInstIIIp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<7> start;
  bits<7> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{12-6} = start;
  let Inst{19-13} = boundary;
  let Inst{25-20} = step;
  let Inst{26} = 1;
  let Inst{27} = 1;
  let Inst{28} = 1;
  let Inst{29} = !if(repeate_once, rep1, 0);
  let Inst{38-36} = cmp;

  let IsPredicated = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let StartSel = 1;
  let BoundarySel = 1;
  let StepSel = 1;
}

class TPCInstPseudo<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : TPCInstGeneric<0, asmstr, PseudoSlot, NoItinerary> {
  let Pattern = pattern;
  let OutOperandList = outs;
  let InOperandList = ins;
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

class TPCInstKeepAlive<string asmstr, RegisterClass Rsrc>
  : TPCInstPseudo<(outs), (ins Rsrc:$src), asmstr> {
}
