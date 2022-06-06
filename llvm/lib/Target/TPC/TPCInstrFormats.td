//===----------------------------------------------------------------------===//
//                         TPC Instruction Flags +
//
//                    *** Must match TPCMCInstrInfo.h (TPCBaseInfo.h) ***
//===----------------------------------------------------------------------===//

def AddrRR : ComplexPattern<i32, 2, "SelectAddrRR", [], []>;
def AddrRI : ComplexPattern<i32, 2, "SelectAddrRI", [], []>;

let OperandNamespace = "TPC" in {
//Designates operands which should be encoded in the IMM field (possibly as SMALL_IMM on goya2)
  class TPCImm<Operand ImmOp> : Operand<ImmOp.Type> {
    let EncoderMethod = "encodeTPCImm";
    let OperandType = "OPERAND_TPC_IMM";
  }
}

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
  let MIOperandInfo = (ops SRF, TPCImm<i32imm>);
  let EncoderMethod = "getRiMemoryOpValue";
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
  let OperandNamespace = "TPC";
  let OperandType = "OPERAND_SWITCH";
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
  bits<32> HIGH32       = 0x02;
}

//===----------------------------------------------------------------------===//
//                         Instruction Classes Definitions
//===----------------------------------------------------------------------===//

class PredicateFormarter<string asmstr> {
  string Value = !subst(" $pred", "$pred", !subst(", $pred", "$pred", asmstr));
}

class TPCInstGeneric<bits<6> opc, string asmstr, SlotType slot, InstrItinClass itin>
  : Instruction {
  let Namespace = "TPC";
  let Itinerary = itin;
  let AsmString = PredicateFormarter<asmstr>.Value;
  let AsmVariantName = ?;
  let hasSideEffects = 0;
  let isNotUsedInDisasm = 1;    // Turn off disassembler by default

  // Assume maximal instruction size (actually slot size), otherwise the
  // instruction will not be treated by disassembler.
  let Size = 8;

  bits<6> Opcode  = opc;
  bits<4> OType = OpType.INT32; // Bogus value for data type
  bit HasImm = 0;
  bit HasPredicate = 0;
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
  let TSFlags{4} = HasPredicate;
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
  bits<1> IsPredicated = 0;

  let Inst{5-0}   = Opcode;
  let Inst{39} = IsPredicated;
  let Inst{46-40} = Switches;
}

// Generic class for instructions with predicates.
// All instructions except NOP and HALT use predicates.
//
class ScalarInstPred<bits<6> opc, string asmstr>
  : ScalarInstGeneric<opc, asmstr> {
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  let HasPredicate = 1;

  let Inst{34}    = PredPolarity;
  let Inst{38-35} = PredAddress;
}

class ScalarInstSrcA<bits<6> opc, string asmstr>
  : ScalarInstPred<opc, asmstr> {
  bits<8> SrcA = 0;

  let Inst{13-6}  = SrcA;
}

class ScalarInstDest<bits<6> opc, string asmstr>
  : ScalarInstSrcA<opc, asmstr> {
  bits<8> Dest = 0;

  let Inst{29-22} = Dest;
}

class ScalarInstSrcB<bits<6> opc, string asmstr>
  : ScalarInstDest<opc, asmstr> {
  bits<8> SrcB = 0;

  let Inst{21-14} = SrcB;
}

class ScalarInstBase<bits<6> opc, string asmstr>
  : ScalarInstSrcB<opc, asmstr> {
  bits<4> OperandType = OpType.FP32;

  let Inst{33-30} = OperandType;
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
  let Inst{42-36} = Switches;       // Out of slot.
}

// Generic class for instructions with predicates.
// All instructions except NOP use predicates.
//
class StoreInstGenericPred<bits<6> opc, string asmstr>
  : StoreInstGeneric<opc, asmstr> {

  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;
  bits<1> IsPredicated = 0;

  let Inst{21}    = PredPolarity;   // 145-124 = 21;
  let Inst{25-22} = PredAddress;    // 149-124 = 25;
  let Inst{26}    = VectorPred;     // 150-124 = 26;
  let Inst{27}    = IsPredicated;
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

  let Inst{35-28} = SrcExtra;
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
  let Inst{43-37} = Switches{6-0};
}

// Generic class for instructions with predicates.
// All instructions except NOP use predicates.
//
class LoadInstGenericPred<bits<6> opc, string asmstr>
  : LoadInstGeneric<opc, asmstr> {
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;
  bits<1> IsPredicated = 0;

  let Inst{21}    = PredPolarity;
  let Inst{25-22} = PredAddress;
  let Inst{26}    = VectorPred;
  let Inst{27}    = IsPredicated;
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

  let Inst{36-28} = SrcExtra;
}

class TPCInstLoad<bits<6> opc, string asmstr>
  : LoadInstGeneric<opc, asmstr> {

  bits<8> SrcA = 0;
  bits<8> Dest = 0;
  bits<9> SrcExtra = 0;
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;
  bits<1> VectorPred = 0;
  bits<1> IsPredicated = 0;
  bits<7> Switches = 0;
  // The size of Load Switches differs between Gen2 (size = 4) and Gen3 (size = 5).
  // In order to not duplicate formats for Gen2 and Gen3, let's use the biggest size
  // here. These bits are out-of-slot, anyway, so they are encoded specially

  let Inst{7-0}   = SrcA;           // 110-103 = 7
  let Inst{15-8}  = Dest;           // 118-103 = 15
                                    // The next fields are out-of-slot, how should we encode them?
  let Inst{21}    = PredPolarity;
  let Inst{25-22} = PredAddress;
  let Inst{26}    = VectorPred;
  let Inst{27}    = IsPredicated;
  let Inst{36-28} = SrcExtra;
  let Inst{43-37} = Switches{6-0};
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


class VpuInst_MovToHWRegG<bits<6> opc, DAGOperand Rsrc, string asmstr, Operand Pred>
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


class VpuInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, DAGOperand Src, string asmstr, Operand Pred>
  : VectorInstShort<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

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
  let HasImm = !isa<Operand>(Src);
  let immOpNum = !if(HasImm, 1, 0);
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

class VpuInst_MAC_X2_I<bits<6> opc,
                       RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,  bit accI16,
                       string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = !if(accI16, (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, VRF:$op3, VRF:$op4, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred),
                                  (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, VRF:$op3, VRF:$op4, DataType:$dt, SwitchSet:$sw,              Rdst:$income, Pred:$pred));
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
  let dt = optype;
  let Switches = sw;
  let hasSrcC = 1;
  let hasSrcD = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInst_MAC_X2_ZP<bits<6> opc,
                        RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2, bit accI16,
                        string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInst_MAC_X2<opc, Rdst, Rsrc1, Rsrc2, accI16, asmstr, optype, Pred, pattern> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = !if(accI16, (ins Rsrc1:$op1, Rsrc2:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred),
                                  (ins Rsrc1:$op1, Rsrc2:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw,              Rdst:$income, Pred:$pred));
  let Switches{5} = 1; //ZP
}

class VpuInst_MAC_X2_ZP_I<bits<6> opc,
                          RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,  bit accI16,
                          string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  :  VpuInst_MAC_X2_I<opc, Rdst, Rsrc1, Isrc, accI16, asmstr, optype, Pred, pattern> {

  let InOperandList = !if(accI16, (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred),
                                  (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw,              Rdst:$income, Pred:$pred));
  let Switches{5} = 1; //ZP
}

class VpuInst_MAC_ZP<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInstBinSwT<opc, Rdst, Rsrc1, Rsrc2, asmstr, optype, Pred, pattern> {

  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

class VpuInst_MAC_ZP_I<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInstBinSwTI<opc, Rdst, Rsrc1, Isrc, asmstr, optype, Pred, pattern> {
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

// Only for Gen4 - Gaudi2
class VpuInst_MAC_MUL_X2<bits<6> opc, RegisterClass Rdst,
                         DAGOperand Src1, DAGOperand Src2, DAGOperand Src3,
                         string asmstr, bits<4> optype, Operand Pred>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src1:$op1, Src2:$op2, Src3:$op3, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  bit src1IsImm = !isa<Operand>(Src1);
  bit src2IsImm = !isa<Operand>(Src2);
  bit src3IsImm = !isa<Operand>(Src3);
  let immOpNum = !if(src1IsImm,
                     1,
                     !if(src2IsImm, 2, !if(src3IsImm, 3, 0)));
  let SecondImmOp = !if(src1IsImm,
                        !if(src2IsImm, 2, !if(src3IsImm, 3, 0)),
                        !if(src2IsImm, !if(src3IsImm, 3, 0), 0));
  let ThirdImmOp = !if(src1IsImm,
                       !if(src2IsImm, !if(src3IsImm, 3, 0), 0),
                       0);
  let HasImm = !or(src1IsImm, src2IsImm, src3IsImm);
  let ImmOpCount = !if(ThirdImmOp, 3, !if(SecondImmOp, 2, !if(immOpNum, 1, 0)));

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcD{7-0} = op3;
  let OperandType = optype{3-0};
  let Switches = sw;
  let Switches{4} = 1; //X2
  let hasSrcD = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_MACA_ZP<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInstBinSwTA<opc, Rdst, Rsrc1, Rsrc2, asmstr, optype, Pred, pattern> {

  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

class VpuInst_MACA_ZP_I<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInstBinSwTAI<opc, Rdst, Rsrc1, Isrc, asmstr, optype, Pred, pattern> {
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);

  let Switches{5} = 1; //ZP
}

class VpuInst_MADD<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, Rdst:$op3, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let OperandType = optype{3-0};
  let Switches = sw;
  let hasSrcC = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_MADDI<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                     string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, Rdst:$op3, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let OperandType = optype{3-0};
  let Switches = sw;
  let hasSrcC = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInst_MADDA<bits<6> opc,
                     RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                     string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, Rdst:$op3, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> dt;
  bits<3> sw;
  bits<1> acc;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let OperandType = optype;
  let Switches{2-0} = sw;
  let hasSrcC = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class VpuInst_MADDAI<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc2,
                      string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc2>:$op2, Rdst:$op3, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> dt;
  bits<7> sw;
  bits<1> acc;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let OperandType = optype;
  let Switches = sw;
  let hasSrcC = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class VpuInst_MADD_ZP<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInst_MADD<opc, Rdst, Rsrc1, Rsrc2, asmstr, optype, Pred, pattern> {

  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, Rdst:$op3, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

class VpuInst_MADD_ZP_I<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInst_MADDI<opc, Rdst, Rsrc1, Isrc, asmstr, optype, Pred, pattern> {
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, Rdst:$op3, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

class VpuInst_MADDA_ZP<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInst_MADDA<opc, Rdst, Rsrc1, Rsrc2, asmstr, optype, Pred, pattern> {

  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, Rdst:$op3, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

class VpuInst_MADDA_ZP_I<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc,
                    string asmstr, bits<4> optype, Operand Pred, list<dag> pattern = []>
  : VpuInst_MADDAI<opc, Rdst, Rsrc1, Isrc, asmstr, optype, Pred, pattern> {
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc>:$op2, Rdst:$op3, HWZPReg:$zp, DataType:$dt, SwitchSet:$sw, MacAcc:$acc, Rdst:$income, Pred:$pred);
  let Switches{5} = 1; //ZP
}

// Only for Gen4
class VpuInst_MADD_X2<bits<6> opc, RegisterClass Rdst,
                      DAGOperand Src1, DAGOperand Src2,
                      DAGOperand Src3, DAGOperand Src4,
                      string asmstr, bits<4> optype, Operand Pred>
  : VectorInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src1:$op1, Src2:$op2, Src3:$op3, Src4:$op4, DataType:$dt, SwitchSet:$sw, X2Op:$x2, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<8> op4;
  bits<4> dt;
  bits<7> sw;
  bits<6> pred;

  bit src1IsImm = !isa<Operand>(Src1);
  bit src2IsImm = !isa<Operand>(Src2);
  bit src3IsImm = !isa<Operand>(Src3);
  bit src4IsImm = !isa<Operand>(Src4);
  let immOpNum = !if(src1IsImm, 1, !if(src2IsImm, 2,
                   !if(src3IsImm, 3, !if(src4IsImm, 4, 0))));
  let SecondImmOp = !if(src2IsImm, !if(src4IsImm, 4, 0), 0);
  let HasImm = !if(immOpNum, 1, 0);
  let ImmOpCount = !if(SecondImmOp, 2, !if(immOpNum, 1, 0));

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcC = op3;
  let SrcD{7-0} = op4;
  let OperandType = dt;
  let Switches = sw;
  let Switches{4} = 1; // X2
  let hasSrcC = 1;
  let hasSrcD = 1;
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

// Only for Gen4
class VpuInst_ADD_SUB_X2<bits<6> opc, RegisterClass Rdst,
                        RegisterClass Rsrc1, DAGOperand Src2, DAGOperand Src3,
                        string asmstr, Operand Pred>
  : VectorInstBase<opc, asmstr> {

  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Src2:$op2, Src3:$op3, DataType:$optype, SwitchSet:$sw, X2Op:$x2, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> op3;
  bits<4> optype;
  bits<7> sw;
  bits<6> pred;

  bit src2IsImm = !isa<Operand>(Src2);
  bit src3IsImm = !isa<Operand>(Src3);
  let immOpNum = !if(src2IsImm, 2, !if(src3IsImm, 3, 0));
  let SecondImmOp = !if(src2IsImm, !if(src3IsImm, 3, 0), 0);
  let HasImm = !or(src2IsImm, src3IsImm);
  let ImmOpCount = !if(src2IsImm, !if(src3IsImm, 2, 1), !if(src3IsImm, 1, 0));

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let SrcD{7-0} = op3;
  let OperandType = optype;
  let Switches = sw;
  let Switches{4} = 1;
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

  let HasImm      = 1;
  let immOpNum    = 2;
  let Constraints = "$dst = $income";
}

class VpuInst_MOV_DUAL_GROUP_CTRL_REG<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstShort<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src1, SRF:$src2, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorOp;

  bits<8> dst;
  bits<8> src1;
  bits<8> src2;
  bits<32> sw;
  bits<6> pred;

  let Dest = dst;
  let SrcA = src1;
  let SrcB = src2;
  let Switches      = 0;
  let Switches{4}   = 1;
  let VectorPred    = !eq(!cast<string>(Pred), "VPred");
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};

  let HasImm      = 1;
  let immOpNum    = 3;
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
  let Switches  = { 0b000, sw{3-2}, 0b01 }; // SWAP_TYPE (for Doron1) | ALL;
  let hasSrcC = 1;
  let VectorPred   = !eq(!cast<string>(Pred), "VPred");
  let PredAddress  = pred{3-0};
  let PredPolarity = pred{5};

  let HasImm      = 1;
  let immOpNum    = 2;
  let Constraints = "$dst = $income";
}

class VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL<bits<6> Opc, string Asmstr, Operand Pred>
  :VectorInstBase<Opc, Asmstr> {
  let OutOperandList = (outs VRF:$dst);
  let InOperandList = (ins VRF:$src1, SRF:$src2, i32imm:$imm, SwitchSet:$sw, MovDGAllOp:$movdgall, VRF:$income, Pred:$pred);
  let Itinerary = IIC_VectorComplexOp;

  bits<8>  dst;
  bits<8>  src1;
  bits<8>  src2;
  bits<32> sw;
  bits<6>  pred;

  let Dest = dst;
  let SrcA = src1;
  let SrcB = src2;
  let SrcC = sw{23-16};
  let Switches  = 0b0010001; // CTRL_REG (for Doron1) | ALL;
  let hasSrcC = 1;
  let VectorPred   = !eq(!cast<string>(Pred), "VPred");
  let PredAddress  = pred{3-0};
  let PredPolarity = pred{5};

  let HasImm      = 1;
  let immOpNum    = 3;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src;
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


// Format for register-to-IRF moves, when mask is represented with an
// expression. It does not have datatype argument, as it is always INT32.
class SpuInst_MovMReg<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc,
                      string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, MRF:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> src;
  bits<4> optype;
  bits<2> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { 0b000, mask, 0b10 };  // DIM_MASK_REG is set
  let Constraints = "$dest = $income";
}


// Format for immediate-to-IRF moves, when mask is represented with an
// expression. It does not have datatype argument, as it is always INT32.
class SpuInst_MovMRegI<bits<6> opc,
                       RegisterClass Rdst, Operand Isrc,
                       string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc>:$src, MRF:$mask, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isReMaterializable = 1;
  let isAsCheapAsAMove = 1;

  bits<8> dest;
  bits<8> src;
  bits<4> optype;
  bits<2> mask;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let OperandType = OpType.INT32;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { 0b000, mask, 0b10 };  // DIM_MASK_REG is set
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

  bits<8> dest;
  bits<6> reg;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcB = { 0b00, reg };      // FROM_HW_REG
  let Switches = 0b0000001;   // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovToHWRegG<bits<6> opc, DAGOperand Rsrc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$reg, SwitchSet:$sw, Rsrc:$src, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isAsmParserOnly = 1;

  bits<6> reg;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b01, reg };    // TO_HW_REG
  let Switches = 0b0000001;   // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
}


class SpuInst_MovFromHWReg<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<6> src;
  bits<7> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = ?;
  let SrcB = { 0b00, src };          // FROM_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, DAGOperand Src, string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<6> dest;
  bits<8> src;
  bits<7> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b01, dest };       // TO_HW_REG
  let Switches = { sw{6-1}, 1 };  // HW_REG

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = !isa<Operand>(Src);
  let immOpNum = !if(HasImm, 1, 0);
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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


class SpuInst_BinaryMReg<bits<6> opc,
                         RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                         string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, MRF:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!and(!eq(!cast<string>(Rsrc1), !cast<string>(Rdst)),
                                   !eq(!cast<string>(Rsrc2), !cast<string>(Rdst)),
                                   !eq(!cast<string>(Rsrc1), "IRF")), 0, 1);

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<2> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let Switches = { sw{6-4}, mask, 1, sw{0} }; // DIM_MASK_REG

  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}


class SpuInst_BinaryMRegI<bits<6> opc,
                          RegisterClass Rdst, RegisterClass Rsrc1, Operand Isrc2,
                          string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, TPCImm<Isrc2>:$op2, MRF:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<2> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let Switches = { sw{6-4}, mask, 1, sw{0} }; // DIM_MASK_REG

  let OperandType = optype;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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


class SpuInst_BinaryMRegIR<bits<6> opc,
                           RegisterClass Rdst, Operand Isrc1, RegisterClass Rsrc2,
                           string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins TPCImm<Isrc1>:$op1, Rsrc2:$op2, MRF:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<2> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op1;
  let SrcB = op2;
  let Switches = { sw{6-4}, mask, 1, sw{0} }; // DIM_MASK_REG

  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let HasImm = 1;
  let immOpNum = 1;
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}


class CMPInstHigh32<bits<6> opc,
                    RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                    string asmstr>
  : SpuInst_Binary<opc, Rdst, Rsrc1, Rsrc2, asmstr> {
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);

  let Switches{1} = 1; // HIGH32
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

  bits<8> dest;
  bits<8> op;
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

  bits<8> dest;
  bits<8> op;
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


class SpuInst_UnaryMReg<bits<6> opc,
                        RegisterClass Rdst, RegisterClass Rsrc,
                        string asmstr>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$op, MRF:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), !cast<string>(Rdst)), 0, 1);

  bits<8> dest;
  bits<8> op;
  bits<2> mask;
  bits<4> optype;
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = { sw{6-4}, mask, 1, sw{0} };
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

  bits<8> dest;
  bits<8> op;
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

  bits<8> dest;
  bits<8> op;
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


class SpuInst_UnarySwMask<bits<6> opc,
                          RegisterClass Rdst, RegisterClass Rsrc,
                          string asmstr, bit Mask = DMask.Imm, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = !if(!eq(Mask, DMask.Imm),
                         (ins Rsrc:$op, DimMask:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred),
                         (ins Rsrc:$op,     MRF:$mask, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> op;
  bits<4> optype;
  bits<7> sw;
  bits<5> mask;
  bits<6> pred;

  let Dest = dest;
  let SrcA = op;
  let OperandType = optype;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Switches = !if(!eq(Mask, DMask.Reg), { 0b000, mask{1-0}, 0b10 }, { mask, 0b00 });
  let Constraints = "$dest = $income";
}


class SpuInstBinarySw<bits<6> opc,
                      RegisterClass Rdst, RegisterClass Rsrc1, RegisterClass Rsrc2,
                      string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc1:$op1, Rsrc2:$op2, DataType:$optype, SwitchSet:$sw, Rdst:$income, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dest;
  bits<8> src;
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

  bits<8> dst;
  bits<8> op1;
  bits<8> op2;
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

  bits<8> dst;
  bits<8> src;
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
  let InOperandList = !if(stepReg,
                        (ins SRF:$src, HWDivStep:$step, DataType:$optype, SwitchSet:$sw, ZRF:$income, SPred:$pred),
                        (ins SRF:$src, i32imm:$step, DataType:$optype, SwitchSet:$sw, ZRF:$income, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<8> dest;
  bits<8> src;
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

  bits<8> dest;
  bits<8> src1;
  bits<8> src2;
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
let isNotUsedInDisasm = 1 in {
class SpuInstJmpU<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$target);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let DecoderMethod = "decodeJmpTargetImm";

  let SrcA = 0b11111111;
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

  let SrcA = 0b11111111;
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
}
}

class SpuInstJmpR<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins SRF:$target, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<8> target;
  bits<6> pred;

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let SrcA = target;
  let OutOfSlotData = 1;
}


class SpuInst_SET_INDX<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, SRF:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;

  bits<8> dst;
  bits<8> src;
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
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, TPCImm<i32imm>:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;

  bits<8> dst;
  bits<8> src;
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
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let Constraints = "$dest = $income";
}

class SpuInst_THREAD_SYNC<bits<6> opc, string asmstr>
  : ScalarInstGeneric<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList  = (ins);
  let HasPredicate = 0;
  let Itinerary = IIC_ScalarOp;
  let isNotUsedInDisasm = 0;
  let hasSideEffects = 1;
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

  bits<8> src;

  let SrcA = src;
  let HasImm = 1;
  let immOpNum = 1;
  bits<8> dst;
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
  bits<32> sw;
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b0, optype{3-0}, 0b0000 };
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b0, optype{3-0}, 0b0000 };
  let Switches = sw{6-0};
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
  bits<32> sw;
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00000, flavor{3-0} };
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, mask{4-0}, 0b00 };
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, mask{4-0}, 0b00 };
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Dest = dest;
  let SrcA = src;
  let SrcExtra = { 0b00, 0b11111, 0b00 };
  let Switches = sw{6-0};

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
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let Dest = dest;
  let SrcExtra = { 0b00, 0b11111, 0b00 };
  let Switches = sw{6-0};

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
  bits<32> sw;
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


class LoadInst_MovToHWRegG<bits<6> opc, DAGOperand Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$reg, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;
  let isAsmParserOnly = 1;

  bits<6> reg;
  bits<8> src;
  bits<32> sw;
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

  bits<8> dest;
  bits<6> src;
  bits<32> sw;
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


class LoadInst_MovToHWReg<bits<6> opc, RegisterClass Rdst, DAGOperand Src, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadComplexOp;

  bits<6> dest;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, 1, dest }; // TO_HW_REG
  let Switches = { sw{6-1}, 1 };    // HW_REG

  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let HasImm = !isa<Operand>(Src);
  let immOpNum = !if(HasImm, 1, 0);
  let Constraints = "$dest = $income";
}

class ReadLFSR<string asmstr, Operand Pred>
  : VpuInst_Mov<0b001000, VRF, VRF, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = 40; // LFSR
  let hasSideEffects = 1;
  let Uses = [LFSR];
  let Defs = [LFSR];
}

class ReadLFSRNC<string asmstr, Operand Pred>
  : VpuInst_Mov<0b001000, VRF, VRF, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = 41; // LFSR_NO_CHANGE
  let mayLoad = 1;
  let Uses = [LFSR_NO_CHANGE, LFSR];
}

class ReadSLFSR<string asmstr>
  : LoadInst_Mov<0b000101, SRF, SRF, asmstr, SPred> {
  let InOperandList = (ins SwitchSet:$sw, SRF:$income, SPred:$pred);
  let src = 104; // LFSR
  let hasSideEffects = 1;
  let Uses = [S_LFSR];
  let Defs = [S_LFSR];
}

class ReadSLFSRNC<string asmstr>
  : LoadInst_Mov<0b000101, SRF, SRF, asmstr, SPred> {
  let InOperandList = (ins SwitchSet:$sw, SRF:$income, SPred:$pred);
  let src = 105; // LFSR_NO_CHANGE
  let mayLoad = 1;
  let Uses = [S_LFSR_NO_CHANGE, S_LFSR];
}

class WriteLFSR<string asmstr, Operand Pred>
  : VpuInst_Mov <0b001000, VRF, VRF, asmstr, Pred> {
  let OutOperandList = (outs);
  let InOperandList = (ins VRF:$src, SwitchSet:$sw, Pred:$pred);
  let dest = 40; // LFSR
  let mayStore = 1;
  let Defs = [S_LFSR];
  let Constraints  = "";
}

class WriteSLFSR<string asmstr>
  : LoadInst_Mov<0b000101, SRF, SRF, asmstr, SPred> {
  let OutOperandList = (outs);
  let InOperandList = (ins SRF:$src, SwitchSet:$sw, SPred:$pred);
  let dest = 104; // S_LFSR
  let mayStore = 1;
  let Defs = [S_LFSR];
  let Constraints  = "";
}

class ReadVPU_LFSR<string asmstr, Operand Pred>
  : LoadInst_MovFromHWReg<0b000101, VRF, HWVpuLFSR, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = 16; // VPU_LFSR
  let hasSideEffects = 1;
  let Uses = [VPU_LFSR];
  let Defs = [VPU_LFSR];
}

class ReadVPU_LFSRNC<string asmstr, Operand Pred>
  : LoadInst_MovFromHWReg<0b000101, VRF, HWVpuLFSRRo, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = 18; // VPU_LFSR_RO
  let mayLoad = 1;
  let Uses = [VPU_LFSR_RO, VPU_LFSR];
}

class ReadSPU_LFSR<string asmstr>
  : LoadInst_MovFromHWReg<0b000101, SRF, HWSpuLFSR, asmstr, SPred> {
  let InOperandList = (ins SwitchSet:$sw, SRF:$income, SPred:$pred);
  let src = 17; // SPU_LFSR
  let hasSideEffects = 1;
  let Uses = [SPU_LFSR];
  let Defs = [SPU_LFSR];
}

class ReadSPU_LFSRNC<string asmstr>
  : LoadInst_MovFromHWReg<0b000101, SRF, HWSpuLFSRRo, asmstr, SPred> {
  let InOperandList = (ins SwitchSet:$sw, SRF:$income, SPred:$pred);
  let src = 19; // SPU_LFSR_RO
  let mayLoad = 1;
  let Uses = [SPU_LFSR_RO, SPU_LFSR];
}

class WriteVPU_LFSR<DAGOperand Src, string asmstr, Operand Pred>
  : LoadInst_MovToHWReg<0b000101, HWVpuLFSR, Src, asmstr, Pred> {
  let OutOperandList = (outs);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, Pred:$pred);
  let dest = 16; // VPU_LFSR
  let mayStore = 1;
  let Defs = [VPU_LFSR];
  let Constraints  = "";
}

class WriteSPU_LFSR<DAGOperand Src, string asmstr>
  : LoadInst_MovToHWReg<0b000101, HWSpuLFSR, Src, asmstr, SPred> {
  let OutOperandList = (outs);
  let InOperandList = (ins Src:$src, SwitchSet:$sw, SPred:$pred);
  let dest = 17; // SPU_LFSR
  let mayStore = 1;
  let Defs = [SPU_LFSR];
  let Constraints  = "";
}

class ReadLaneId<string asmstr, Operand Pred>
  : LoadInst_MovFromHWReg<0b000101, VRF, HWLaneId, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = ?;
  let Uses = ?;
}

class ReadVLaneId<string asmstr, Operand Pred>
  : VpuInst_Mov<0b001000, VRF, VRF, asmstr, Pred> {
  let InOperandList = (ins SwitchSet:$sw, VRF:$income, Pred:$pred);
  let src = ?;
  let Uses = ?;
}


class LoadInst_GEN_ADDR<bits<6> opc, string asmstr, bit TnsrReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs ADRF:$dest);
  let InOperandList = !if(TnsrReg,
                          (ins HWTnsrRegLd:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred),
                          (ins       i8imm:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<9> tensor;
  bits<8> coords;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = tensor;
  let Dest = dest;
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let SrcA = src1;
  let SrcExtra = src2;
  let Dest = dest;
  let Switches = sw{6-0};

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
  bits<32> sw;
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
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, SRF:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dst;
  bits<8> src;
  bits<5> mask;
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, mask, 0b00 };
  let Dest = dst;
  let Switches = sw{6-0};
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
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, TPCImm<i32imm>:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, mask, 0b00 };
  let Dest = dst;
  let Switches = sw{6-0};
  let Switches{1} = MaskInReg;    // DIM_MASK_REG
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};

  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
  let Constraints = "$dst = $income";
}


class LoadInst_MovMRF<bits<6> opc, RegisterClass Rsrc, string asmstr, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs MRF:$dst);
  let InOperandList = (ins Rsrc:$src);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;

  bits<6> dst;
  bits<8> src;
  bits<6> pred;

  let SrcA = src;
  let SrcExtra = { 0b00, 1, dst};

  let Switches{0} = 1; // HW_REG
  let OutOfSlotData = 1;
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

class LoadInst_LD_G64<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G<opc, ZRF, asmstr, Pred> {

  let Switches{4}    = 1;     // SW_BV64
  let VectorPred     = 0;
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

class LoadInst_LD_G64_INC<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  :  LoadInst_LD_Gg4_INC<opc, Rdst, asmstr, Pred> {
  let Switches{4} = 1; // SW_BV64
  let VectorPred  = 0;
}

class LoadInst_LD_GI<bits<6> opc, RegisterClass Rdst, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = !if(MaskInReg,
                          (ins ADRF:$addr,     MRF:$dimmask, SwitchSet:$sw, Rdst:$income, SPred:$pred),
                          (ins ADRF:$addr, DimMask:$dimmask, SwitchSet:$sw, Rdst:$income, SPred:$pred));
  let Itinerary = IIC_LoadScalGlobalMask;
  let mayLoad = 1;

  bits<8> dest;
  bits<8> addr;
  bits<5> dimmask;
  bits<32> sw;
  bits<6> pred;
  bits<1> exc;	

  let exc{0} = sw{9}; // SW_EXC for doron1.
  let SrcA = addr;
  let SrcExtra = { 0b00, dimmask, exc, 0b0 };
  let Dest = dest;
  let Switches = sw{6-0};
  let Switches{1} = MaskInReg;
  let PredAddress = pred{3-0};
  let VectorPred = 0;
  let PredPolarity = pred{5};
  let OutOfSlotData = 1;
  let Constraints = "$dest = $income";
}

class LoadInst_LD_GI_INC<bits<6> opc, RegisterClass Rdst, string asmstr, bit MaskInReg>
  : LoadInst_LD_GI<opc, Rdst, asmstr, MaskInReg> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
  let Switches{0} = 1; // AUTO_INC
}

class LoadInst_LD_GI_INCg4<bits<6> opc, RegisterClass Rdst, string asmstr, bit MaskInReg>
  : LoadInst_LD_GI<opc, Rdst, asmstr, MaskInReg> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
}

class LoadInst_LD_GIg4<bits<6> opc, RegisterClass Rdst, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : LoadInst_LD_GI<opc, Rdst, asmstr, MaskInReg, pattern> {
  let Switches = sw{6-0};
  let Switches{3} = MaskInReg;
}

class LoadInst_LD_G_P<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G<opc, Rdst, asmstr, Pred> {
  let InOperandList = (ins ADRF:$addr, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let SrcExtra{0} = 1;
}

class LoadInst_LD_G_Pg4<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_Gg4<opc, Rdst, asmstr, Pred> {
  let InOperandList = (ins ADRF:$addr, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let SrcExtra{0} = 1;
}

class LoadInst_LD_G_P_INC<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G_P<opc, Rdst, asmstr, Pred> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
  let Switches{0} = 1; // AUTO_INC
}

class LoadInst_LD_G_P_INC_Pg4<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred>
  : LoadInst_LD_G_P<opc, Rdst, asmstr, Pred> {
  let isCodeGenOnly = 1;
  let OutOperandList = (outs Rdst:$dest, ADRF:$iaddr);
}

class LoadInstr_PREFETCH<bits<6> opc, string asmstr>
  : LoadInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs);
  let InOperandList = (ins ADRF:$addr, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_LoadScalGlobal;
  let isNotUsedInDisasm = 0;

  bits<8> addr;
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
  let SrcExtra = sw{15-7};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
  let Dest = dest;
  let SrcA = addr{15-8};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
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
  let InOperandList = (ins TPCImm<i32imm>:$addr, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadShortVectLocal;
  let mayLoad = 1;

  bits<8> dest;
  bits<32> sw;
  bits<6> pred;
  bits<8> addr;

  let Switches = sw{6-0};
  let Dest = dest;
  let SrcA = addr;
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
  let InOperandList = (ins TPCImm<i32imm>:$addr, SwitchSet:$sw, Rdst:$income, SPred:$pred);
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
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra{4-0} = tensor{4-0};
  let SrcExtra{7-5} = sw{10-8};
  let Dest = dst;

  let Switches = sw{6-0};
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
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, HWTnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred),
                          (ins IRF:$coords,   TnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred));
  let Itinerary = IIC_LoadTensor;

  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra{7-5} = sw{10-8};
  let Dest = dst;

  let Switches = sw{6-0};
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
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, i8imm:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred),
                          (ins IRF:$coords, i8imm:$tensor,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred));
  let Itinerary = IIC_LoadTensor;

  bits<9> tensor;
  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra{4-0} = tensor{4-0};
  let SrcExtra{7-5} = sw{10-8};
  let Dest = dst;

  let Switches = sw{6-0};
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
  let InOperandList = !if(UseHWR,
                          (ins SRF:$coords1, SRF:$coords2, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred),
                          (ins SRF:$coords1, SRF:$coords2,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred));
  bits<8> coords1;
  bits<9> coords2;

  let SrcA     = coords1;
  let SrcExtra = coords2;
}

class LoadInst_LD_TNSR_PR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, bit UseHWR>
  : LoadInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = !if(UseHWR,
                         (ins IRF:$coords, HWTnsrRegLd:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred),
                         (ins IRF:$coords,   TnsrRegLd:$tensor,   OffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred));
  let Itinerary = IIC_LoadTensor;

  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra{7-5} = sw{10-8};
  let Dest = dst;

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class LoadInst_FCLASS<bits<6> opc, RegisterClass Rdst, RegisterClass Rsrc, string asmstr, Operand Pred>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dest);
  let InOperandList = (ins Rsrc:$src, DataType:$optype, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Itinerary = IIC_LoadOp;
  let isNotUsedInDisasm = !if(!eq(!cast<string>(Rsrc), "VRF"), 0, 1);
  
  bits<8> dest;
  bits<8> src;
  bits<8> optype;
  bits<32> sw;
  bits<6> pred;
  
  let Dest = dest;
  let SrcA = src;
  
  let Switches = sw{6-0};
  let SrcExtra = { 0b0, optype };
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  
  let Constraints = "$dest = $income";
}

class LoadInst_CALC_FP_SPECIAL<bits<6> opc, string asmstr, Operand Pred>
: TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs VRF:$dest);
  // The second parameter for compatibility with VpuInst_CALC_FP_SPECIAL
  let InOperandList = (ins VRF:$op1, VRF:$op2, DataType:$optype, SwitchSet:$funcId, VRF:$income, Pred:$pred);
  let Itinerary = IIC_LoadOp;
  
  bits<8> dest;
  bits<8> op1;
  bits<8> op2;
  bits<8> optype;
  bits<7> funcId;
  bits<6> pred;
  
  let Dest = dest;
  let SrcA = op1;
  
  let Switches = funcId;
  let SrcExtra = { 0b0, optype };
  let PredAddress = pred{3-0};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  let PredPolarity = pred{5};
  
  let Constraints = "$dest = $income";
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

class LoadInst_LD_TNSR_CNVRT<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : TPCInstLoad<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_LoadComplexOp;
  
  bits<8> coords;
  bits<5> tensor;
  bits<8> dest;
  bits<32> sw;
  bits<6> pred;
  
  let Dest = dest;
  let SrcA = coords;
  let SrcExtra{4-0} = tensor{4-0};
  let SrcExtra{7-5} = sw{10-8};
  let Switches{0} = 0;      // PARTIAL
  let Switches{1} = sw{1};  // CNVRT_DT
  let Switches{2} = sw{20}; // CLIP_FP
  let Switches{3} = 0;      // TNSR_ID_REG
  let Switches{4} = sw{4};  // PART
  let Switches{5} = sw{5};  // L0CS
  let Switches{6} = 0;      // DIRECT
  
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  bit OutOfSlotData = 1;
  
  let Constraints = "$dst = $income";
}

class LoadInst_LD_TNSR_CNVRTR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : LoadInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords, HWTnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 0; // PARTIAL
  let Switches{3} = 1; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class LoadInst_LD_TNSR_CNVRT_P<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : LoadInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class LoadInst_LD_TNSR_CNVRT_PR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : LoadInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords,  HWTnsrRegLd:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 1; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class LoadInst_LD_TNSR_CNVRT_D<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : LoadInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  bits<8> coords1;
  bits<9> coords2;
  
  let SrcA = coords1;
  let SrcExtra = coords2;
  
  let Switches{0} = 0; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 1; // DIRECT
}

class LoadInst_LD_TNSR_CNVRT_PD<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : LoadInst_LD_TNSR_CNVRT_D<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 1; // DIRECT
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

  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
  let PredPolarity = pred{5};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
}

// Format for instructions in store that do not produce values, but have
// inputs, but they may be executed conditionally. These are CACHE_* for Gen4Plus.
//
class StoreInstPredAddr<bits<6> opc, string asmstr, bit fl>
  : StoreInstGenericSrcA<opc, asmstr> {
  let DecoderMethod = "decodeStoreInstPredAddr";
  let OutOperandList = (outs);
  let InOperandList = (ins ADRF:$addr, SwitchSet:$sw, SPred:$pred);
  let Itinerary = IIC_StoreOp;
  let isNotUsedInDisasm = 1;

  bits<32> sw;
  bits<6> pred;
  bits<8> addr;
  bits<7> flush =  { sw{6-2}, 0b1, sw{0} }; // SW_CL
  bits<7> inv   =  { sw{6}, 0b1, sw{4-2}, 0b1, sw{0} }; // SW_INV_CL | SW_D

  let SrcA = addr;
  let Switches = !if(fl, flush, inv) ;
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

  bits<32> sw;
  bits<6> pred;

  let SrcA = sw{15-8};
  let PredPolarity = pred{5};
  let PredAddress = pred{3-0};
  let VectorPred = 0;
}


class StoreInst_SET_INDX<bits<6> opc, string asmstr, bit MaskInReg, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs IRF:$dst);
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, SRF:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, SRF:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;
  let isNotUsedInDisasm = 0;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, mask, 0b00 };
  let SrcExtra = dst;
  let Switches = sw{6-0};
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
  let InOperandList = !if(MaskInReg,
                          (ins IRF:$income, TPCImm<i32imm>:$src,     MRF:$mask, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$income, TPCImm<i32imm>:$src, DimMask:$mask, SwitchSet:$sw, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;

  bits<5> mask;
  bits<8> dst;
  bits<8> src;
  bits<32> sw;
  bits<6> pred;

  let SrcA = src;
  let SrcB = { 0b0, mask, 0b00 };
  let SrcExtra = dst;
  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let SrcA = addr;
  let SrcB = sw{7-0};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
  let SrcA = addr{15-8};
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
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};
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
  let InOperandList = (ins TPCImm<i32imm>:$addr, Rsrc:$src, SwitchSet:$sw, Pred:$pred);
  let Itinerary = IIC_StoreVectLocal;
  let mayStore = 1;

  bits<8> src;
  bits<32> sw;
  bits<6> pred;
  bits<8> addr;

  let Switches = sw{6-0};
  let SrcA = addr;
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

class StoreInst_ST_G64<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInst_ST_G<opc, ZRF, asmstr> {

  let Switches{4} = 1; // SW_BV64
  let VectorPred  = 0;
}

class StoreInst_ST_G_INC<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInst_ST_G<opc, Rsrc, asmstr> {
  let OutOperandList  = (outs ADRF:$new_addr);
  let isCodeGenOnly = 1;
  let Switches{0} = 1; // AUTO_INC
}

class StoreInst_ST_G64_INC<bits<6> opc, RegisterClass Rsrc, string asmstr>
  : StoreInst_ST_G_INC<opc, Rsrc, asmstr> {
  let Switches{4} = 1; // SW_BV64
  let VectorPred  = 0;
}

class StoreInst_GEN_ADDR<bits<6> opc, string asmstr, bit TnsrReg, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {

  let OutOperandList = (outs ADRF:$dest);
  let InOperandList = !if(TnsrReg,
                          (ins HWTnsrRegSt:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred),
                          (ins       i8imm:$tensor, IRF:$coords, SwitchSet:$sw, ADRF:$income, SPred:$pred));
  let Pattern = pattern;
  let Itinerary = IIC_StoreComplexOp;
  let isNotUsedInDisasm = 0;

  bits<8> dest;
  bits<8> tensor;
  bits<8> coords;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB = tensor;
  let SrcExtra = dest;
  let Switches = sw{6-0};
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
  bits<32> sw;
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
  bits<32> sw;
  bits<6> pred;

  let SrcA = src1;
  let SrcB = src2;
  let SrcExtra = dest;
  let Switches = sw{6-0};

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
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};

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
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Rsrc:$value, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_RMW<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
  let Switches{1} = 1; // rmw_sel

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_RMW_D<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_RMW<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = !if(UseHWR,
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred),
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred));
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_RMW_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Rsrc:$value, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   RMWReg:$rmw, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
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
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P_D<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_P<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = !if(UseHWR,
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_P_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Rsrc:$value, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
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
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value, HWRMWReg:$rmw, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords, i8imm:$tensor, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial
  let Switches{1} = 1; // rmw_sel

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_P_D_RMW<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInst_ST_TNSR_P_RMW<opc, Rsrc, asmstr, UseHWR> {
  let InOperandList = !if(UseHWR,
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value, HWRMWReg:$rmw, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins SRF:$coords1, SRF:$coords2, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_ST_TNSR_P_RMW_T<bits<6> opc, RegisterClass Rsrc, string asmstr, bit UseHWR>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(UseHWR,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Rsrc:$value, HWRMWReg:$rmw, HWOffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,   TnsrRegSt:$tensor, Rsrc:$value,   RMWReg:$rmw,   OffsSizeRegSt:$offsize, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<8> tensor;
  bits<8> value;
  bits<8> rmw;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = value;

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial
  let Switches{1} = 1; // rmw_sel
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 0;
}

class StoreInst_ST_TNSR_SQZ<bits<6> opc, string asmstr>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, i8imm:$squeeze, VRF:$value, SwitchSet:$sw, VPred:$pred);
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;
    
  bits<8> coords;
  bits<3> squeeze;
  bits<5> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;
    
  let SrcA = coords;
  let SrcB{4-0} = tensor;
  let SrcB{7-5} = squeeze;
  let SrcExtra = value;
    
  let Switches = sw{6-0};
    
  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 1;
}

class StoreInst_ST_TNSR_SQZ_RMW<bits<6> opc, string asmstr>
  : StoreInst_ST_TNSR_SQZ<opc, asmstr> {
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, i8imm:$squeeze, VRF:$value, HWRMWReg:$rmw, SwitchSet:$sw, VPred:$pred);
  let Switches{1} = 1; // rmw_sel
}

class StoreInst_ST_TNSR_SQZ_T<bits<6> opc, string asmstr>
  : StoreInst_ST_TNSR_SQZ<opc, asmstr> {
  let InOperandList = (ins IRF:$coords, HWTnsrRegSt:$tensor, i8imm:$squeeze, VRF:$value, SwitchSet:$sw, VPred:$pred);

  let Switches{3} = 1; // tnsr_id_reg
}

class StoreInst_ST_TNSR_SQZ_RMW_T<bits<6> opc, string asmstr>
  : StoreInst_ST_TNSR_SQZ<opc, asmstr> {
  let InOperandList = (ins IRF:$coords, HWTnsrRegSt:$tensor, i8imm:$squeeze, VRF:$value, HWRMWReg:$rmw, SwitchSet:$sw, VPred:$pred);

  let Switches{1} = 1; // rmw_sel
  let Switches{3} = 1; // tnsr_id_reg
}

class StoreInst_ST_TNSR_SQZ_COUNT<bits<6> opc, string asmstr>
  : StoreInstGenericSrcB<opc, asmstr> {
  let InOperandList = (ins i8imm:$squeeze, SwitchSet:$sw, VPred:$pred);
  let OutOperandList = (outs);
  let Itinerary = IIC_StoreTensor;

  bits<3> squeeze;
  bits<32> sw;
  bits<6> pred;

  let Switches = sw{6-0};

  let SrcB{7-5} = squeeze;
  let Switches{0} = 1; // cnt_only
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = 1;
}

class StoreInst_ST_TNSR_SQZ_FLUSH<bits<6> opc, string asmstr>
  : StoreInstGenericSrcB<opc, asmstr> {
  let InOperandList = (ins i8imm:$squeeze, SwitchSet:$sw);
  let OutOperandList = (outs);
  let Itinerary = IIC_StoreTensor;

  bits<3> squeeze;
  bits<32> sw;

  let Switches = sw{6-0};

  let SrcB{7-5} = squeeze;
  let Switches{4} = 1; // flush
}

class StoreInst_ST_TNSR_S<bits<6> opc, DAGOperand Src, string asmstr, bit TnsrReg>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(TnsrReg,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Src:$value, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,       i8imm:$tensor, Src:$value, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let HasImm    = !isa<Operand>(Src);
  let immOpNum  = 2;
  let mayStore = 1;

  bits<8> coords;
  bits<5> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA        = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let SrcExtra    = value;
  let Switches = sw{6-0};
  let Switches{3} = TnsrReg;  // TNSR_ID_REG

  let OutOfSlotData = 1;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let VectorPred    = 0;
}

class StoreInst_ST_TNSR_S_RMW<bits<6> opc, DAGOperand Src, string asmstr, bit TnsrReg>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(TnsrReg,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, Src:$value, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords,       i8imm:$tensor, Src:$value, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let HasImm    = !isa<Operand>(Src);
  let immOpNum  = 2;
  let mayStore = 1;

  bits<8> coords;
  bits<5> tensor;
  bits<8> value;
  bits<32> sw;
  bits<6> pred;

  let SrcA        = coords;
  let SrcB{4-0}   = tensor{4-0};
  let SrcB{7-5}   = sw{10-8};
  let SrcExtra    = value;
  let Switches    = sw{6-0};
  let Switches{1} = 1; // RMW_SEL
  let Switches{3} = TnsrReg;  // TNSR_ID_REG

  let OutOfSlotData = 1;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let VectorPred    = 0;
}

class StoreInst_ST_TNSR_S_HWR<bits<6> opc, string asmstr, bit TnsrReg>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = !if(TnsrReg,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, HWSqzCntr:$hw_reg_sel, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords, i8imm:$tensor, HWSqzCntr:$hw_reg_sel, SwitchSet:$sw, SPred:$pred));
  let Itinerary = IIC_StoreTensor;
  let mayStore = 1;

  bits<8> coords;
  bits<5> tensor;
  bits<3> hw_reg_sel;
  bits<32> sw;
  bits<6> pred;

  let SrcA        = coords;
  let SrcB{4-0}   = tensor;
  let SrcB{7-5}   = hw_reg_sel;
  let Switches    = sw{6-0};
  let Switches{0} = 1; // HW_REG
  let Switches{3} = TnsrReg; // TNSR_ID_REG

  let OutOfSlotData = 1;
  let PredAddress   = pred{3-0};
  let PredPolarity  = pred{5};
  let VectorPred    = 0;
}

class StoreInst_ST_TNSR_S_HWR_RMW<bits<6> opc, string asmstr, bit TnsrReg>
  : StoreInst_ST_TNSR_S_HWR<opc, asmstr, TnsrReg> {
  let InOperandList = !if(TnsrReg,
                          (ins IRF:$coords, HWTnsrRegSt:$tensor, HWSqzCntr:$hw_reg_sel, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred),
                          (ins IRF:$coords, i8imm:$tensor, HWSqzCntr:$hw_reg_sel, HWRMWReg:$rmw, SwitchSet:$sw, SPred:$pred));
  let Switches{1} = 1; // RMW_SEL
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
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = dst;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};

  let Switches = sw{6-0};
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

class StoreInst_LD_TNSR_R<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, HWTnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreTensor;

  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcB{7-5} = sw{10-8};
  let SrcExtra = dst;

  let Switches = sw{6-0};
  let Switches{0} = 0; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class StoreInst_LD_TNSR_P<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreTensor;

  bits<8> tensor;
  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = dst;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial
  let Switches{3} = 0; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class StoreInst_LD_TNSR_P_D<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_P<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);

  bits<8> coords1;
  bits<8> coords2;

  let SrcA = coords1;
  let SrcB = coords2;
}

class StoreInst_LD_TNSR_PR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, HWTnsrRegLd:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreTensor;

  bits<8> coords;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;

  let SrcA = coords;
  let SrcExtra = dst;
  let SrcB{7-5} = sw{10-8};

  let Switches = sw{6-0};
  let Switches{0} = 1; // partial
  let Switches{3} = 1; // tnsr_id_reg

  let OutOfSlotData = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");

  let Constraints = "$dst = $income";
}

class StoreInst_LD_TNSR_CNVRT<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInstGenericSrcB<opc, asmstr> {
  let OutOperandList = (outs Rdst:$dst);
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_StoreTensor;
  
  bits<8> coords;
  bits<5> tensor;
  bits<8> dst;
  bits<32> sw;
  bits<6> pred;
  
  let SrcExtra = dst;
  let SrcA = coords;
  let SrcB{4-0} = tensor{4-0};
  let SrcB{7-5} = sw{10-8};
  let Switches{0} = 0;      // PARTIAL
  let Switches{1} = sw{1};  // CNVRT_DT
  let Switches{2} = sw{20}; // CLIP_FP
  let Switches{3} = 0;      // TNSR_ID_REG
  let Switches{4} = sw{4};  // PART
  let Switches{5} = sw{5};  // L0CS
  let Switches{6} = 0;      // DIRECT
  
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
  let VectorPred = !eq(!cast<string>(Pred), "VPred");
  bit OutOfSlotData = 1;
  
  let Constraints = "$dst = $income";
}

class StoreInst_LD_TNSR_CNVRTR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords, HWTnsrRegLd:$tensor, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 0; // PARTIAL
  let Switches{3} = 1; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class StoreInst_LD_TNSR_CNVRT_P<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords, i8imm:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class StoreInst_LD_TNSR_CNVRT_PR<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins IRF:$coords, HWTnsrRegLd:$tensor, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 1; // TNSR_ID_REG
  let Switches{6} = 0; // DIRECT
}

class StoreInst_LD_TNSR_CNVRT_D<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_CNVRT<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  bits<8> coords1;
  bits<8> coords2;
  
  let SrcA = coords1;
  let SrcB = coords2;
  
  let Switches{0} = 0; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 1; // DIRECT
}

class StoreInst_LD_TNSR_CNVRT_PD<bits<6> opc, RegisterClass Rdst, string asmstr, Operand Pred, list<dag> pattern = []>
  : StoreInst_LD_TNSR_CNVRT_D<opc, Rdst, asmstr, Pred, pattern> {
  let InOperandList = (ins SRF:$coords1, SRF:$coords2, HWOffsSizeRegLd:$offsize, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  
  let Switches{0} = 1; // PARTIAL
  let Switches{3} = 0; // TNSR_ID_REG
  let Switches{6} = 1; // DIRECT
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

  bits<1> IsPredicated = 0;
  bits<1> PredPolarity = 0;
  bits<4> PredAddress = 0;

  let Inst{5-0} = Opcode;
  let Inst{31} = 0;

  let Inst{37-34} = PredAddress;
  let Inst{33}    = PredPolarity;
  let Inst{41}    = IsPredicated; // Just put it at the end, no need to reshuffle everything

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

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 0;
  let Inst{30} = 0;
  let Inst{40-38} = cmp;
}

class LOOPInstRRI<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 0;
  let Inst{30} = 1;
  let Inst{40-38} = cmp;
  
  let StepSel = 1;
}

class LOOPInstRIR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 1;
  let Inst{30} = 0;
  let Inst{40-38} = cmp;
  
  let BoundarySel = 1;
}

class LOOPInstIRR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, SRF:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 0;
  let Inst{30} = 0;
  let Inst{40-38} = cmp;

  let StartSel = 1;
}

class LOOPInstRII<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins SRF:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 1;
  let Inst{30} = 1;
  let Inst{40-38} = cmp;

  let BoundarySel = 1;
  let StepSel = 1;
}

class LOOPInstIRI<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 0;
  let Inst{30} = 1;
  let Inst{40-38} = cmp;

  let StartSel = 1;
  let StepSel = 1;
}

class LOOPInstIIR<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, SRF:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 1;
  let Inst{30} = 0;
  let Inst{40-38} = cmp;

  let StartSel = 1;
  let BoundarySel = 1;
}

class LOOPInstIII<bits<6> opc, string asmstr>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = (ins LoopStartImm:$start, LoopBoundaryImm:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target);

  bits<8> start; bits<8> boundary; bits<6> step; bits<3> cmp;
  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 1;
  let Inst{30} = 1;
  let Inst{40-38} = cmp;

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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 0;
  let Inst{30} = 0;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};
}

class LoopInstRRIp<bits<6> opc, string asmstr, bit repeate_once = 0>
  : TPCInstLOOP<opc, asmstr> {
  let InOperandList = !if(repeate_once,
        (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, i8imm:$rep1, SPred:$pred),
        (ins SRF:$start, SRF:$boundary, LoopStepImm:$step, Comparison:$cmp, brtarget:$target, SPred:$pred)
  );

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 0;
  let Inst{30} = 1;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 1;
  let Inst{30} = 0;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 0;
  let Inst{30} = 0;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 0;
  let Inst{29} = 1;
  let Inst{30} = 1;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 0;
  let Inst{30} = 1;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 1;
  let Inst{30} = 0;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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

  bits<8> start;
  bits<8> boundary;
  bits<6> step;
  bits<3> cmp;
  bits<6> pred;
  bits<1> rep1;

  let Inst{13-6} = start;
  let Inst{21-14} = boundary;
  let Inst{27-22} = step;
  let Inst{28} = 1;
  let Inst{29} = 1;
  let Inst{30} = 1;
  let Inst{31} = !if(repeate_once, rep1, 0);
  let Inst{40-38} = cmp;

  let HasPredicate = 1;
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
