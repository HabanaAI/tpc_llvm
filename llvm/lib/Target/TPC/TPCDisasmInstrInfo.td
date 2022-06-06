//===-- Gen1InstrInfo.td ------ TPC Instruction Definition -----------===//
//
//===----------------------------------------------------------------------===//
//
// This file describes the TPC instruction set, for the purpose of disassembling
// binary representation.
//
//===----------------------------------------------------------------------===//


let isNotUsedInDisasm = 0, AsmVariantName = "None" in {

//------ Load slot.

// GEN_ADDR - decoded fairly
// PRMT_INDX - decoded fairly
// SET_INDX - decoded using RR hack
// MOV - decoded using custom decoder
let DecoderMethod = "decodeMovLd", SrcExtra = ?, Switches = ? in {
  def MOV_ld_p_Dis : LoadInst_Mov <0b000101, VRF, VRF, "mov $dest, $src, $pred", SPred>;
  def MOV_ld_m_Dis : LoadInst_Mov <0b000101, VRF, VRF, "mov $dest, $src, $pred", VPred>;
}

// LOOKUP
// LOOKUP_C1C2
// LOOKUP_C0
// LOOKUP_1C
// LOOKUP_2C

// LD_L
def LD_L_Dis : LoadInst_LD_L<0b001011, VRF, "ld_l $sw $dest, $addr, $pred", []>;

// LD_G
let DecoderMethod = "decodeLD_G", SrcExtra = ?, Switches = ? in {
  def LD_G_p_Dis : LoadInst_LD_G <0b001100, SRF, "ld_g $sw $dest, $addr, $pred", SPred>;
  def LD_G_m_Dis : LoadInst_LD_G <0b001100, VRF, "ld_g $sw $dest, $addr, $pred", VPred>;
  //def LD_G_P_p_Dis : LoadInst_LD_G_P<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>;
  //def LD_G_P_m_Dis : LoadInst_LD_G_P<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>;
  //def LD_G_P_pg4_Dis : LoadInst_LD_G_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", SPred>;
  //def LD_G_P_mg4_Dis : LoadInst_LD_G_Pg4<0b001100, VRF,  "ld_g $sw $dest, $addr, $offsize, $pred", VPred>;
}

// LD_L_V
// LD_L_V_LOW
// LD_L_V_HIGH
//
// We have to create separate definitions as codegen defs have predicates.
def LD_L_Vrrp_Dis : LoadInst_LD_Vrr<0b001110, VRF, "ld_l_v $sw $dest, $addr, $pred", SPred>, Requires<[Addr2]>;
def LD_L_Vrrp_LOW_Dis : LoadInst_LD_Vrr<0b001111, VRF, "ld_l_v_low $sw $dest, $addr, $pred", SPred>, Requires<[Addr2]>;
def LD_L_Vrrp_HIGH_Dis : LoadInst_LD_Vrr<0b010000, VRF, "ld_l_v_high $sw $dest, $addr, $pred", SPred>, Requires<[Addr2]>;
def LD_L_Vrrm_Dis : LoadInst_LD_Vrr<0b001110, VRF, "ld_l_v $sw $dest, $addr, $pred", VPred>, Requires<[Addr2]>;
def LD_L_Vrrm_LOW_Dis : LoadInst_LD_Vrr<0b001111, VRF, "ld_l_v_low $sw $dest, $addr, $pred", VPred>, Requires<[Addr2]>;
def LD_L_Vrrm_HIGH_Dis : LoadInst_LD_Vrr<0b010000, VRF, "ld_l_v_high $sw $dest, $addr, $pred", VPred>, Requires<[Addr2]>;

def LD_L_Vrp_Dis : LoadInst_LD_Vr<0b001110, VRF, "ld_l_v $sw $dest, $addr, $pred", SPred>, Requires<[Addr1]>;
def LD_L_Vrp_LOW_Dis : LoadInst_LD_Vr<0b001111, VRF, "ld_l_v_low $sw $dest, $addr, $pred", SPred>, Requires<[Addr1]>;
def LD_L_Vrp_HIGH_Dis : LoadInst_LD_Vr<0b010000, VRF, "ld_l_v_high $sw $dest, $addr, $pred", SPred>, Requires<[Addr1]>;
def LD_L_Vrm_Dis : LoadInst_LD_Vr<0b001110, VRF, "ld_l_v $sw $dest, $addr, $pred", VPred>, Requires<[Addr1]>;
def LD_L_Vrm_LOW_Dis : LoadInst_LD_Vr<0b001111, VRF, "ld_l_v_low $sw $dest, $addr, $pred", VPred>, Requires<[Addr1]>;
def LD_L_Vrm_HIGH_Dis : LoadInst_LD_Vr<0b010000, VRF, "ld_l_v_high $sw $dest, $addr, $pred", VPred>, Requires<[Addr1]>;

// LD_TNSR
// LD_TNSR_LOW
// LD_TNSR_HIGH
let DecoderMethod = "decodeLdTnsr", SrcExtra = ?, Switches = ? in {
def LD_TNSRp_Dis : LoadInst_LD_TNSR<0b010001, VRF, "ld_tnsr $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSRm_Dis : LoadInst_LD_TNSR<0b010001, VRF, "ld_tnsr $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_LOWp_Dis : LoadInst_LD_TNSR<0b010010, VRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_LOWm_Dis : LoadInst_LD_TNSR<0b010010, VRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_HIGHp_Dis : LoadInst_LD_TNSR<0b010011, VRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_HIGHm_Dis : LoadInst_LD_TNSR<0b010011, VRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_CNVRTp_Dis : LoadInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_CNVRTm_Dis : LoadInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>;
}

// CALC_FP_SPECIAL
def LD_CALC_FP_SPECIALOneArgvvp_Dis : LoadInst_CALC_FP_SPECIAL<0b010110, "calc_fp_special$optype $funcId $dest, $op1, $op1, $pred", SPred> {
  let InOperandList = (ins VRF:$op1, DataType:$optype, SwitchSet:$funcId, VRF:$income, SPred:$pred);
}
def LD_CALC_FP_SPECIALOneArgvvm_Dis : LoadInst_CALC_FP_SPECIAL<0b010110, "calc_fp_special$optype $funcId $dest, $op1, $op1, $pred", VPred> {
  let InOperandList = (ins VRF:$op1, DataType:$optype, SwitchSet:$funcId, VRF:$income, VPred:$pred);
}

// NOP - decoded fairly


//------ Scalar slot.

// MAC - decoded using RR hack
// MUL - decoded using RR hack

// ADD - decoded partially using RR hack and partially by custom decoder because
// DIM_MASK_REG and CARRY are the same bit.
def ADD_Dis : SpuInst_Binary<0b000010, SRF, SRF, SRF, "add$optype $sw $dest, $op1, $op2, $pred"> {
  let Switches = ?;
  let DecoderMethod = "decodeAdd";
}

// SUB - decoded partially using RR hack and partially by custom decoder because
// DIM_MASK_REG and NEG are the same bit.
def SUB_Dis : SpuInst_Binary<0b000011, SRF, SRF, SRF, "sub$optype $sw $dest, $op1, $op2, $pred"> {
  let Switches = ?;
  let DecoderMethod = "decodeSub";
}

// CONVERT_INT16
let DecoderMethod = "decodeConvertIntScalar" in
def CONVERT_INT16_Dis : SpuInstConvertIntGen2<0b000100, "convert_int16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;

// MAX - decoded using RR hack
// MIN - decoded using RR hack

// ABS

// MOV
let DecoderMethod = "decodeMovSpu", Switches = ?, SrcB = ? in {
  def MOV_Dis        : SpuInst_Mov         <0b001000, SRF, SRF, "mov$optype $dest, $src, $pred">;
  def MOV_ToHW_Dis   : SpuInst_MovToHWReg  <0b001000, SRF, SRF, "mov $dest, $src, $pred">;
  def MOV_FromHW_Dis : SpuInst_MovFromHWReg<0b001000, SRF, SRF, "mov $dest, $src, $pred">;
}


// CMP_EQ - decoded using RR hack
// CMP_NEQ - decoded using RR hack
// CMP_LESS - decoded using RR hack
// CMP_LEQ - decoded using RR hack
// CMP_GRT - decoded using RR hack
// CMP_GEQ - decoded using RR hack
// OR - decoded using RR hack
// AND - decoded using RR hack
// XOR - decoded using RR hack

// NOT
// SHR
// SHL
// ASH

// CONVERT - decoded by custom decoder decodeConvertScalar

// CONVERT_INT32
// CONVERT_UINT32
let DecoderMethod = "decodeConvertIntScalar" in {
def CONVERT_INT32_Dis : SpuInstConvertIntGen2<0b010111, "convert_int32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT32_Dis : SpuInstConvertIntGen2<0b011000, "convert_uint32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
}

// POPCNT - decoded fairly
// FIND_FIRST - decoded fairly
// NEARBYINT - decoded fairly

// CONVERT_UINT16
let DecoderMethod = "decodeConvertIntScalar" in
def CONVERT_UINT16_Dis : SpuInstConvertIntGen2<0b011100, "convert_uint16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;

// EXTRACT_EXP - decoded using RR hack
// HALT - decoded fairly

// LOOP
// JMPR
// JMPA
class SpuInstJmpShortSrcA<bits<6> opc, string asmstr, list<dag> pattern = []>
  : ScalarInstBase<opc, asmstr> {
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$target, SPred:$pred);
  let Pattern = pattern;
  let Itinerary = IIC_ScalarOp;
  let DecoderMethod = "decodeJmpTargetImm";

  bits<6> pred;

  let PredAddress = pred{3-0};
  let PredPolarity = pred{5};

  let SrcA = 0b01111111;
  let HasImm = 1;
  let immOpNum = 2;
  let OutOfSlotData = 1;
}

class SpuInstJmpLongSrcA<bits<6> opc, string asmstr, list<dag> pattern = []>
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

def JMPRShort   : SpuInstJmpShortSrcA <0b100100, "jmpr $target, $pred">;
def JMPAShort   : SpuInstJmpShortSrcA <0b100101, "jmpa $target, $pred">;
def JMPRLong   : SpuInstJmpLongSrcA <0b100100, "jmpr $target, $pred">;
def JMPALong   : SpuInstJmpLongSrcA <0b100101, "jmpa $target, $pred">;

// MOV_IRF_DIM - decoded fairly
// SET_INDX - decoded using RR hack

// UDIV_STEP (UDIV_4STEP), UDIV
let DecoderMethod = "decodeUdivAll", Switches = ?  in {
def UDIV_STEP_Dis:   SpuInst_UDIV_STEP<0b101000, "udiv_step$optype $sw $step $dest, $src, $pred", 0>;
}

let DecoderMethod = "decodeUdivAll" in {
def UDIV_DIS      : SpuInst_UDIV<0b101000, "udiv$optype $dest, $src1, $src2, $sw, $pred", 0>;
}

// CALC_FP_SPECIAL - decoded fairly
// NOP - decoded fairly


//------ Vector slot.

// ASH

class AshInsr_Dis<bits<6> opc, string asmstr, Operand Pred>
  : VpuInstBinSw<opc, VRF, VRF, VRF, asmstr, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, DataType:$optype, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Switches{2} = 0; // RHAZ_RS
  let Switches{0} = 1; // Reserved
}

class AshRhazInsr_Dis<bits<6> opc, string asmstr, Operand Pred>
  : VpuInstBinSwD<opc, VRF, DRF, VRF, asmstr, Pred> {
  let InOperandList = (ins DRF:$op1, VRF:$op2, DataType:$optype, SwitchSet:$sw, VRF:$income, Pred:$pred);
  let Switches{2} = 1; // RHAZ_RS
  let Switches{0} = 1; // Reserved
}

def ASHvp_Dis : AshInsr_Dis<0b010101, "ash$optype $sw $dest, $op1, $op2, $pred", SPred>;
def ASHvm_Dis : AshInsr_Dis<0b010101, "ash$optype $sw $dest, $op1, $op2, $pred", VPred>;

def ASHvpD_Dis : AshRhazInsr_Dis<0b010101, "ash$optype $sw rhaz_rs $dest, $op1, $op2, $pred", SPred>;
def ASHvmD_Dis : AshRhazInsr_Dis<0b010101, "ash$optype $sw rhaz_rs $dest, $op1, $op2, $pred", VPred>;

// MAC

class MacInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInstBinSwT<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
}
class MacAInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInstBinSwTA<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
}

class MacAccI32_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInstBinSwTA<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
  let Switches{3} = 1; // ACC_I32
}

class MacInstr_x2_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MAC_X2<opc, Rdst, VRF, VRF, 0, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2,  VRF:$op3, VRF:$op4, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
  let Switches{4} = 1; // x2
}
class MacAInstr_x2_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MAC_X2<opc, Rdst, VRF, VRF, 1, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, VRF:$op3, VRF:$op4, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
  let Switches{4} = 1; //x2
}

def MACf32p_Dis  : MacInstr_Dis<0b000000, VRF, OpType.FP32,   "mac.f32 $sw $dest, $op1, $op2, $pred", SPred>;
def MACbf16p_Dis : MacInstr_Dis<0b000000, VRF, OpType.BF16,   "mac.bf16 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[BF16]>;
def MACfp16p_Dis : MacInstr_Dis<0b000000, VRF, OpType.FP16,   "mac.f16 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[FP16]>;


def MACi16p_Dis  : MacInstr_Dis<0b000000, DRF, OpType.INT16,  "mac.i16 $sw $dest, $op1, $op2, $pred", SPred>;
def MACu16p_Dis  : MacInstr_Dis<0b000000, DRF, OpType.UINT16, "mac.u16 $sw $dest, $op1, $op2, $pred", SPred>;
def MACi8p_Dis   : MacInstr_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[MULI8]>;
def MACu8p_Dis   : MacInstr_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[MULI8]>;
def MACf32m_Dis  : MacInstr_Dis<0b000000, VRF, OpType.FP32,   "mac.f32 $sw $dest, $op1, $op2, $pred", VPred>;
def MACbf16m_Dis : MacInstr_Dis<0b000000, VRF, OpType.BF16,   "mac.bf16 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[BF16]>;
def MACfp16m_Dis : MacInstr_Dis<0b000000, VRF, OpType.FP16,   "mac.f16 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[FP16]>;
def MACi16m_Dis  : MacInstr_Dis<0b000000, DRF, OpType.INT16,  "mac.i16 $sw $dest, $op1, $op2, $pred", VPred>;
def MACu16m_Dis  : MacInstr_Dis<0b000000, DRF, OpType.UINT16, "mac.u16 $sw $dest, $op1, $op2, $pred", VPred>;
def MACi8m_Dis   : MacInstr_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[MULI8]>;
def MACu8m_Dis   : MacInstr_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[MULI8]>;

def MACAbf16p_Dis : MacAInstr_Dis<0b000000, DRF, OpType.BF16,  "mac.bf16 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[BF16]>;
def MACAfp16p_Dis : MacAInstr_Dis<0b000000, DRF, OpType.FP16,  "mac.f16 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP16]>;
def MACAf8p_Dis : MacAInstr_Dis<0b000000, ARF, OpType.FP8_143,   "mac.f8_143 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP8]>;
def MACAh8p_Dis : MacAInstr_Dis<0b000000, ARF, OpType.FP8_152,   "mac.f8_152 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP8]>;

def MACAi8p_Dis   : MacAInstr_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 $sw acc_i16 $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAu8p_Dis   : MacAInstr_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 $sw acc_i16 $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAu16p_Dis  : MacAccI32_Dis<0b000000, DRF, OpType.UINT16, "mac.u16 $sw acc_i32 $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;

def MACAbf16m_Dis : MacAInstr_Dis<0b000000, DRF, OpType.BF16,  "mac.bf16 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[BF16]>;
def MACAfp16m_Dis : MacAInstr_Dis<0b000000, DRF, OpType.FP16,  "mac.f16 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP16]>;
def MACAf8m_Dis : MacAInstr_Dis<0b000000, ARF, OpType.FP8_143,   "mac.f8_143 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP8]>;
def MACAh8m_Dis : MacAInstr_Dis<0b000000, ARF, OpType.FP8_152,   "mac.f8_152 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP8]>;


def MACAi8m_Dis   : MacAInstr_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 $sw acc_i16 $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MACAu8m_Dis   : MacAInstr_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 $sw acc_i16 $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MACAu16m_Dis  : MacAccI32_Dis<0b000000, DRF, OpType.UINT16, "mac.u16 $sw acc_i32 $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;

def MACx2i8p_Dis   : MacInstr_x2_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 x2 $sw $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACx2u8p_Dis   : MacInstr_x2_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 x2 $sw $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACx2i8m_Dis   : MacInstr_x2_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 x2 $sw $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;
def MACx2u8m_Dis   : MacInstr_x2_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 x2 $sw $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;

def MACAx2i8p_Dis  : MacAInstr_x2_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 x2 $sw acc_i16 $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAx2u8p_Dis  : MacAInstr_x2_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 x2 $sw acc_i16 $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAx2i8m_Dis  : MacAInstr_x2_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 x2 $sw acc_i16 $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;
def MACAx2u8m_Dis  : MacAInstr_x2_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 x2 $sw acc_i16 $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;

class MacInstr_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MAC_ZP<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
  let DecoderMethod = "decodeMacZp";
}

class MacAInstr_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MACA_ZP<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
  let DecoderMethod = "decodeMacZp";
}

def MACzpi8p_Dis   : MacInstr_zp_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 $sw $zp $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACzpu8p_Dis   : MacInstr_zp_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 $sw $zp $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACzpi8m_Dis   : MacInstr_zp_Dis<0b000000, ARF, OpType.INT8,   "mac.i8 $sw $zp $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MACzpu8m_Dis   : MacInstr_zp_Dis<0b000000, ARF, OpType.UINT8,  "mac.u8 $sw $zp $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;

def MACAzpi8p_Dis   : MacAInstr_zp_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 $sw acc_i16 $zp $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAzpu8p_Dis   : MacAInstr_zp_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 $sw acc_i16 $zp $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAzpi8m_Dis   : MacAInstr_zp_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 $sw acc_i16 $zp $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MACAzpu8m_Dis   : MacAInstr_zp_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 $sw acc_i16 $zp $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;


class MacInstr_x2_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MAC_X2_ZP<opc, Rdst, VRF, VRF, 0, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
  let Switches{4} = 1; // x2
  let DecoderMethod = "decodeMacZp";
}

class MacAInstr_x2_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MAC_X2_ZP<opc, Rdst, VRF, VRF, 1, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, VRF:$op3, VRF:$op4, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
  let Switches{4} = 1; //x2
  let DecoderMethod = "decodeMacZp";
}


def MACx2zpi8p_Dis   : MacInstr_x2_zp_Dis<0b000000, ARF, OpType.INT8,  "mac.i8 x2 $sw $zp $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACx2zpu8p_Dis   : MacInstr_x2_zp_Dis<0b000000, ARF, OpType.UINT8, "mac.u8 x2 $sw $zp $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACx2zpi8m_Dis   : MacInstr_x2_zp_Dis<0b000000, ARF, OpType.INT8,  "mac.i8 x2 $sw $zp $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;
def MACx2zpu8m_Dis   : MacInstr_x2_zp_Dis<0b000000, ARF, OpType.UINT8, "mac.u8 x2 $sw $zp $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;

def MACAx2zpi8p_Dis  : MacAInstr_x2_zp_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 x2 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAx2zpu8p_Dis  : MacAInstr_x2_zp_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 x2 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen3Plus]>;
def MACAx2zpi8m_Dis  : MacAInstr_x2_zp_Dis<0b000000, DRF, OpType.INT8,  "mac.i8 x2 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;
def MACAx2zpu8m_Dis  : MacAInstr_x2_zp_Dis<0b000000, DRF, OpType.UINT8, "mac.u8 x2 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen3Plus]>;

let DecoderMethod = "decodeMacMulX2" in {
  def MACx2f32vvvp_Dis : VpuInst_MAC_MUL_X2<0b000000, DRF, DRF, VRF, VRF, "mac.f32 $dt$sw $dest, $op1, $op2, $op3, $pred", OpType.FP32, SPred>;
  def MACx2f32vvvm_Dis : VpuInst_MAC_MUL_X2<0b000000, DRF, DRF, VRF, VRF, "mac.f32 $dt$sw $dest, $op1, $op2, $op3, $pred", OpType.FP32, VPred>;
}

// MUL

class MulInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInstBinSwT<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{1} = 0; // DOUBLE_AND_ROUND32
  let Switches{2} = 0; // ACC_FP32/ACC_I16
}
class MulAInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInstBinSwTA<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
}
class MulDND_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: MulInstr_Dis<opc, Rdst, optype, asmstr, Pred> {
  let Switches{1} = 1; // DOUBLE_AND_ROUND32
}
class MulDNDG3_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: MulInstr_Dis<opc, Rdst, optype, asmstr, Pred> {
  let Switches{1-0} = 0b01; // DOUBLE_AND_ROUND32
}
class MulKRSG3_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: MulInstr_Dis<opc, Rdst, optype, asmstr, Pred> {
  let Switches{1-0} = 0b11; // KEEP_RS_FOR_ADD
}
class MulRSG3_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: MulInstr_Dis<opc, Rdst, optype, asmstr, Pred> {
  let Switches{1-0} = 0b10; // KEEP_RS
}

def MULi32DNDG3p_Dis : MulDNDG3_Dis<0b000001, VRF, OpType.INT32,  "mul.i32 $sw double_and_round32 $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULu32DNDG3p_Dis : MulDNDG3_Dis<0b000001, VRF, OpType.UINT32, "mul.u32 $sw double_and_round32 $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULi32DNDG3m_Dis : MulDNDG3_Dis<0b000001, VRF, OpType.INT32,  "mul.i32 $sw double_and_round32 $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULu32DNDG3m_Dis : MulDNDG3_Dis<0b000001, VRF, OpType.UINT32, "mul.u32 $sw double_and_round32 $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULi32KRSG3p_Dis : MulKRSG3_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw keep_rs_for_add $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULu32KRSG3p_Dis : MulKRSG3_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw keep_rs_for_add $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULi32KRSG3m_Dis : MulKRSG3_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw keep_rs_for_add $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULu32KRSG3m_Dis : MulKRSG3_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw keep_rs_for_add $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULi32RSG3p_Dis : MulRSG3_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw keep_rs $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULu32RSG3p_Dis : MulRSG3_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw keep_rs $dest, $op1, $op2, $pred", SPred>, Requires<[Gen3Plus]>;
def MULi32RSG3m_Dis : MulRSG3_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw keep_rs $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULu32RSG3m_Dis : MulRSG3_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw keep_rs $dest, $op1, $op2, $pred", VPred>, Requires<[Gen3Plus]>;
def MULi32DNDp_Dis : MulDND_Dis<0b000001, VRF, OpType.INT32,  "mul.i32 $sw double_and_round32 $dest, $op1, $op2, $pred", SPred>;
def MULu32DNDp_Dis : MulDND_Dis<0b000001, VRF, OpType.UINT32, "mul.u32 $sw double_and_round32 $dest, $op1, $op2, $pred", SPred>;
def MULi32DNDm_Dis : MulDND_Dis<0b000001, VRF, OpType.INT32,  "mul.i32 $sw double_and_round32 $dest, $op1, $op2, $pred", VPred>;
def MULu32DNDm_Dis : MulDND_Dis<0b000001, VRF, OpType.UINT32, "mul.u32 $sw double_and_round32 $dest, $op1, $op2, $pred", VPred>;

def MULf32p_Dis  : MulInstr_Dis<0b000001, VRF, OpType.FP32,   "mul.f32 $sw $dest, $op1, $op2, $pred", SPred>;
def MULbf16p_Dis : MulInstr_Dis<0b000001, VRF, OpType.BF16,   "mul.bf16 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[BF16]>;
def MULfp16p_Dis : MulInstr_Dis<0b000001, VRF, OpType.FP16,   "mul.f16  $sw $dest, $op1, $op2, $pred", SPred>, Requires<[FP16]>;
def MULi32p_Dis  : MulInstr_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw $dest, $op1, $op2, $pred", SPred>;
def MULu32p_Dis  : MulInstr_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw $dest, $op1, $op2, $pred", SPred>;
def MULi16p_Dis  : MulInstr_Dis<0b000001, DRF, OpType.INT16,  "mul.i16 $sw $dest, $op1, $op2, $pred", SPred>;
def MULu16p_Dis  : MulInstr_Dis<0b000001, DRF, OpType.UINT16, "mul.u16 $sw $dest, $op1, $op2, $pred", SPred>;
def MULi8p_Dis   : MulInstr_Dis<0b000001, ARF, OpType.INT8,   "mul.i8 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[MULI8]>;
def MULu8p_Dis   : MulInstr_Dis<0b000001, ARF, OpType.UINT8,  "mul.u8 $sw $dest, $op1, $op2, $pred", SPred>, Requires<[MULI8]>;
def MULf32m_Dis  : MulInstr_Dis<0b000001, VRF, OpType.FP32,   "mul.f32 $sw $dest, $op1, $op2, $pred", VPred>;
def MULbf16m_Dis : MulInstr_Dis<0b000001, VRF, OpType.BF16,   "mul.bf16 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[BF16]>;
def MULfp16m_Dis : MulInstr_Dis<0b000001, VRF, OpType.FP16,   "mul.f16 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[FP16]>;
def MULi32m_Dis  : MulInstr_Dis<0b000001, DRF, OpType.INT32,  "mul.i32 $sw $dest, $op1, $op2, $pred", VPred>;
def MULu32m_Dis  : MulInstr_Dis<0b000001, DRF, OpType.UINT32, "mul.u32 $sw $dest, $op1, $op2, $pred", VPred>;
def MULi16m_Dis  : MulInstr_Dis<0b000001, DRF, OpType.INT16,  "mul.i16 $sw $dest, $op1, $op2, $pred", VPred>;
def MULu16m_Dis  : MulInstr_Dis<0b000001, DRF, OpType.UINT16, "mul.u16 $sw $dest, $op1, $op2, $pred", VPred>;
def MULi8m_Dis   : MulInstr_Dis<0b000001, ARF, OpType.INT8,   "mul.i8 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[MULI8]>;
def MULu8m_Dis   : MulInstr_Dis<0b000001, ARF, OpType.UINT8,  "mul.u8 $sw $dest, $op1, $op2, $pred", VPred>, Requires<[MULI8]>;

def MULAbf16p_Dis : MulAInstr_Dis<0b000001, DRF, OpType.BF16, "mul.bf16 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[BF16]>;
def MULAfp16p_Dis : MulAInstr_Dis<0b000001, DRF, OpType.FP16, "mul.f16 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP16]>;
def MULAf8_143p_Dis : MulAInstr_Dis<0b000001, ARF, OpType.FP8_143, "mul.f8_143 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP8]>;
def MULAf8_152p_Dis : MulAInstr_Dis<0b000001, ARF, OpType.FP8_152, "mul.f8_152 $sw acc_fp32 $dest, $op1, $op2, $pred", SPred>, Requires<[FP8]>;
def MULAi8p_Dis   : MulAInstr_Dis<0b000001, DRF, OpType.INT8, "mul.i8 $sw acc_i16 $dest, $op1, $op2, $pred", SPred>;
def MULAu8p_Dis   : MulAInstr_Dis<0b000001, DRF, OpType.UINT8, "mul.u8 $sw acc_i16 $dest, $op1, $op2, $pred", SPred>;
def MULAbf16m_Dis : MulAInstr_Dis<0b000001, DRF, OpType.BF16, "mul.bf16 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[BF16]>;
def MULAfp16m_Dis : MulAInstr_Dis<0b000001, DRF, OpType.FP16, "mul.f16 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP16]>;
def MULAf8_143m_Dis : MulAInstr_Dis<0b000001, ARF, OpType.FP8_143, "mul.f8_143 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP8]>;
def MULAf8_152m_Dis : MulAInstr_Dis<0b000001, ARF, OpType.FP8_152, "mul.f8_152 $sw acc_fp32 $dest, $op1, $op2, $pred", VPred>, Requires<[FP8]>;
def MULAi8m_Dis   : MulAInstr_Dis<0b000001, DRF, OpType.INT8, "mul.i8 $sw acc_i16 $dest, $op1, $op2, $pred", VPred>;
def MULAu8m_Dis   : MulAInstr_Dis<0b000001, DRF, OpType.UINT8, "mul.u8 $sw acc_i16 $dest, $op1, $op2, $pred", VPred>;

let DecoderMethod = "decodeMacMulX2" in {
  def MULx2f32vvvp_Dis : VpuInst_MAC_MUL_X2<0b000001, DRF, DRF, VRF, VRF, "mul.f32 $dt$sw $dest, $op1, $op2, $op3, $pred", OpType.FP32, SPred>;
  def MULx2f32vvvm_Dis : VpuInst_MAC_MUL_X2<0b000001, DRF, DRF, VRF, VRF, "mul.f32 $dt$sw $dest, $op1, $op2, $op3, $pred", OpType.FP32, VPred>;
}

// MADD

class MaddInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MADD<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, Rdst:$op3, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
}
class MaddAInstr_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MADDA<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, Rdst:$op3, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
}

def MADDf32p_Dis  : MaddInstr_Dis<0b110111, VRF, OpType.FP32,   "madd.f32 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDbf16p_Dis : MaddInstr_Dis<0b110111, VRF, OpType.BF16,   "madd.bf16 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDf16p_Dis  : MaddInstr_Dis<0b110111, VRF, OpType.FP16,   "madd.f16 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDi16p_Dis  : MaddInstr_Dis<0b110111, DRF, OpType.INT16,  "madd.i16 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDu16p_Dis  : MaddInstr_Dis<0b110111, DRF, OpType.UINT16, "madd.u16 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDi8p_Dis   : MaddInstr_Dis<0b110111, ARF, OpType.INT8,   "madd.i8 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDu8p_Dis   : MaddInstr_Dis<0b110111, ARF, OpType.UINT8,  "madd.u8 $sw $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDf32m_Dis  : MaddInstr_Dis<0b110111, VRF, OpType.FP32,   "madd.f32 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDbf16m_Dis : MaddInstr_Dis<0b110111, VRF, OpType.BF16,   "madd.bf16 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDf16m_Dis  : MaddInstr_Dis<0b110111, VRF, OpType.FP16,   "madd.f16 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDi16m_Dis  : MaddInstr_Dis<0b110111, DRF, OpType.INT16,  "madd.i16 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDu16m_Dis  : MaddInstr_Dis<0b110111, DRF, OpType.UINT16, "madd.u16 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDi8m_Dis   : MaddInstr_Dis<0b110111, ARF, OpType.INT8,   "madd.i8 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDu8m_Dis   : MaddInstr_Dis<0b110111, ARF, OpType.UINT8,  "madd.u8 $sw $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;

def MADDAbf16p_Dis : MaddAInstr_Dis<0b110111, DRF, OpType.BF16,  "madd.bf16 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAf16p_Dis  : MaddAInstr_Dis<0b110111, DRF, OpType.FP16,  "madd.f16 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAf8p_Dis  : MaddAInstr_Dis<0b110111, ARF, OpType.FP8_143,  "madd.f8_143 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAh8p_Dis  : MaddAInstr_Dis<0b110111, ARF, OpType.FP8_152,  "madd.f8_152 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;


def MADDAi8p_Dis   : MaddAInstr_Dis<0b110111, DRF, OpType.INT8,  "madd.i8 $sw acc_i16 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAu8p_Dis   : MaddAInstr_Dis<0b110111, DRF, OpType.UINT8, "madd.u8 $sw acc_i16 $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAbf16m_Dis : MaddAInstr_Dis<0b110111, DRF, OpType.BF16,  "madd.bf16 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAf16m_Dis  : MaddAInstr_Dis<0b110111, DRF, OpType.FP16,  "madd.f16 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAf8m_Dis  : MaddAInstr_Dis<0b110111, ARF, OpType.FP8_143,  "madd.f8_143 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAh8m_Dis  : MaddAInstr_Dis<0b110111, ARF, OpType.FP8_152,  "madd.f8_152 $sw acc_fp32 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAi8m_Dis   : MaddAInstr_Dis<0b110111, DRF, OpType.INT8,  "madd.i8 $sw acc_i16 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAu8m_Dis   : MaddAInstr_Dis<0b110111, DRF, OpType.UINT8, "madd.u8 $sw acc_i16 $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;

class MaddInstr_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MADD_ZP<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, Rdst:$op3, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 0; // ACC_FP32/ACC_I16
  let DecoderMethod = "decodeMaddZp";
}

class MaddAInstr_zp_Dis<bits<6> opc, RegisterClass Rdst, bits<4> optype, string asmstr, Operand Pred>
: VpuInst_MADDA_ZP<opc, Rdst, VRF, VRF, asmstr, optype, Pred> {
  let InOperandList = (ins VRF:$op1, VRF:$op2, Rdst:$op3, HWZPReg:$zp, SwitchSet:$sw, Rdst:$income, Pred:$pred);
  let Switches{2} = 1; // ACC_FP32/ACC_I16
  let DecoderMethod = "decodeMaddZp";
}

def MADDzpi8p_Dis   : MaddInstr_zp_Dis<0b110111, ARF, OpType.INT8,   "madd.i8 $sw $zp $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDzpu8p_Dis   : MaddInstr_zp_Dis<0b110111, ARF, OpType.UINT8,  "madd.u8 $sw $zp $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDzpi8m_Dis   : MaddInstr_zp_Dis<0b110111, ARF, OpType.INT8,   "madd.i8 $sw $zp $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDzpu8m_Dis   : MaddInstr_zp_Dis<0b110111, ARF, OpType.UINT8,  "madd.u8 $sw $zp $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;

def MADDAzpi8p_Dis   : MaddAInstr_zp_Dis<0b110111, DRF, OpType.INT8,  "madd.i8 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAzpu8p_Dis   : MaddAInstr_zp_Dis<0b110111, DRF, OpType.UINT8, "madd.u8 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $pred", SPred>, Requires<[MADD]>;
def MADDAzpi8m_Dis   : MaddAInstr_zp_Dis<0b110111, DRF, OpType.INT8,  "madd.i8 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;
def MADDAzpu8m_Dis   : MaddAInstr_zp_Dis<0b110111, DRF, OpType.UINT8, "madd.u8 $sw acc_i16 $zp $dest, $op1, $op2, $op3, $pred", VPred>, Requires<[MADD]>;

class MADDx2_Dis<bits<6> opc, bits<4> Optype, string asmstr, Operand Pred>
  : VpuInst_MADD_X2<opc, DRF, DRF, VRF, DRF, VRF, asmstr, Optype, Pred> {
  let InOperandList = (ins DRF:$op1, VRF:$op2, DRF:$op3, VRF:$op4, DataType:$dt, SwitchSet:$sw, DRF:$income, Pred:$pred);
}
let DecoderMethod = "decodeMaddX2" in {
  def MADDx2f32vvvvp_Dis : MADDx2_Dis<0b110111, OpType.FP32, "madd.f32$dt $sw $dest, $op1, $op2, $op3, $op4, $pred", SPred>, Requires<[Gen4Plus]>;
  def MADDx2f32vvvvm_Dis : MADDx2_Dis<0b110111, OpType.FP32, "madd.f32$dt $sw $dest, $op1, $op2, $op3, $op4, $pred", VPred>, Requires<[Gen4Plus]>;
}

// ADD - decoded using RR hack
// SUB - decoded using RR hack
class ADD_SUB_X2_Dis<bits<6> opc, string asmstr, Operand Pred>
 : VpuInst_ADD_SUB_X2<opc, DRF, DRF, VRF, VRF, asmstr, Pred> {
  let InOperandList = (ins DRF:$op1, VRF:$op2, VRF:$op3, DataType:$optype, SwitchSet:$sw, DRF:$income, Pred:$pred);
}

def ADD_X2p_Dis : ADD_SUB_X2_Dis<0b000010, "add$optype x2 $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def ADD_X2m_Dis : ADD_SUB_X2_Dis<0b000010, "add$optype x2 $sw $dest, $op1, $op2, $op3, $pred", VPred>;
def SUB_X2p_Dis : ADD_SUB_X2_Dis<0b000011, "sub$optype x2 $sw $dest, $op1, $op2, $op3, $pred", SPred>;
def SUB_X2m_Dis : ADD_SUB_X2_Dis<0b000011, "sub$optype x2 $sw $dest, $op1, $op2, $op3, $pred", VPred>;

// CONVERT_INT16
let DecoderMethod = "decodeConvertIntVector" in {
def CONVERT_INT16p_Dis : VpuInstConvertIntGen2<0b000100, VRF, VRF, SPred, "convert_int16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_INT16m_Dis : VpuInstConvertIntGen2<0b000100, VRF, VRF, VPred, "convert_int16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
}

// MAX - decoded using RR hack
// MIN - decoded using RR hack

// ABS

// MOV - decoded using custom decoder
let DecoderMethod = "decodeMovVpu", OperandType = ?, SrcB = ?, Switches = ? in {
  def MOV_vp_Dis : VpuInst_Mov<0b001000, VRF, VRF, "mov $dest, $src, $pred", SPred>;
  def MOV_vm_Dis : VpuInst_Mov<0b001000, VRF, VRF, "mov $dest, $src, $pred", VPred>;
}

// CMP_EQ - decoded using RR hack
// CMP_NEQ - decoded using RR hack
// CMP_LESS - decoded using RR hack
// CMP_LEQ - decoded using RR hack
// CMP_GRT - decoded using RR hack
// CMP_GEQ - decoded using RR hack
// OR - decoded using RR hack
// AND - decoded using RR hack
// XOR - decoded using RR hack

// NOT
// SHR
// SHL
// ASH
// CONVERT

// CONVERT_INT32
// CONVERT_UINT32
let DecoderMethod = "decodeConvertIntVector" in {
def CONVERT_INT32p_Dis  : VpuInstConvertIntGen2<0b010111, VRF, VRF, SPred, "convert_int32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_INT32m_Dis  : VpuInstConvertIntGen2<0b010111, VRF, VRF, VPred, "convert_int32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT32p_Dis : VpuInstConvertIntGen2<0b011000, VRF, VRF, SPred, "convert_uint32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT32m_Dis : VpuInstConvertIntGen2<0b011000, VRF, VRF, VPred, "convert_uint32 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
}

// POPCNT - decoded fairly
// FIND_FIRST - decoded fairly
// NEARBYINT - decoded fairly

// CONVERT_UINT16
let DecoderMethod = "decodeConvertIntVector" in {
def CONVERT_UINT16p_Dis : VpuInstConvertIntGen2<0b011100, VRF, VRF, SPred, "convert_uint16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT16m_Dis : VpuInstConvertIntGen2<0b011100, VRF, VRF, VPred, "convert_uint16 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
}

let DecoderMethod = "decodeConvertIntVector" in {
def CONVERT_INT8p_Dis : VpuInstConvertIntGen2<0b110101, DRF, VRF, SPred, "convert_int8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_INT8m_Dis : VpuInstConvertIntGen2<0b110101, DRF, VRF, VPred, "convert_int8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT8p_Dis : VpuInstConvertIntGen2<0b110110, DRF, VRF, SPred, "convert_uint8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
def CONVERT_UINT8m_Dis : VpuInstConvertIntGen2<0b110110, DRF, VRF, VPred, "convert_uint8 $sw $dest, $op1, $op2, $pred", 0 /*hasImm*/>;
}


// EXTRACT_EXP - decoded using RR hack
// HALT - decoded fairly
// SEL_EQ - decoded using RR hack
// SEL_NEQ - decoded using RR hack
// SEL_LESS - decoded using RR hack
// SEL_LEQ - decoded using RR hack
// SEL_GRT - decoded using RR hack
// SEL_GEQ - decoded using RR hack
// SEL2_LESS - decoded using RR hack
// SEL2_LEQ - decoded using RR hack
// SEL2_GRT - decoded using RR hack
// SEL2_GEQ - decoded using RR hack
// SHUFFLE - decoded fairly
// PACK - decoded fairly
// UNPACK - decoded fairly
// GET_LUT_ENTRY_AND_INTERVAL_START - decoded fairly
// FORM_FP_NUM - decoded fairly

// MOV_DUAL_GROUP
class VpuInst_MOV_DUAL_GROUP_ALL_DIS<bits<6> Opc, string Asmstr, Operand Pred>
  : VpuInst_MOV_DUAL_GROUP_ALL<Opc, Asmstr, Pred> {
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
}

class VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL_DIS<bits<6> Opc, string Asmstr, Operand Pred>
  : VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL<Opc, Asmstr, Pred> {
  let InOperandList = (ins VRF:$src1, SRF:$src2, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
}

class VpuInst_MOV_DUAL_GROUP_PACK_DIS<bits<6> Opc, string Asmstr, Operand Pred>
  : VpuInst_MOV_DUAL_GROUP_PACK<Opc, Asmstr, Pred> {
  let InOperandList = (ins VRF:$src, SwitchSet:$sw, VRF:$income, Pred:$pred);
}

class VpuInst_MOV_DUAL_GROUP_UNPACK_DIS<bits<6> Opc, string Asmstr, Operand Pred>
  : VpuInst_MOV_DUAL_GROUP_UNPACK<Opc, Asmstr, Pred> {
  let InOperandList = (ins VRF:$src, i32imm:$imm, SwitchSet:$sw, VRF:$income, Pred:$pred);
}

let DecoderMethod = "decodeMovDualGroup" in {
  def MOV_DUAL_GROUPp_Dis : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", SPred>;
  def MOV_DUAL_GROUPm_Dis : VpuInst_MOV_DUAL_GROUP<0b110001, "mov_dg $sw $dst, $src, $imm, $pred", VPred>;

  def MOV_DUAL_GROUP_ALLp_Dis : VpuInst_MOV_DUAL_GROUP_ALL_DIS<0b110001, "mov_dg.all $sw $dst, $src, $imm, $pred", SPred>;
  def MOV_DUAL_GROUP_ALLm_Dis : VpuInst_MOV_DUAL_GROUP_ALL_DIS<0b110001, "mov_dg.all $sw $dst, $src, $imm, $pred", VPred>;

  def MOV_DUAL_GROUP_PACKp_Dis : VpuInst_MOV_DUAL_GROUP_PACK_DIS<0b110001, "mov_dg.pack $sw $dst, $src, $pred", SPred>;
  def MOV_DUAL_GROUP_PACKm_Dis : VpuInst_MOV_DUAL_GROUP_PACK_DIS<0b110001, "mov_dg.pack $sw $dst, $src, $pred", VPred>;

  def MOV_DUAL_GROUP_UNPACKp_Dis : VpuInst_MOV_DUAL_GROUP_UNPACK_DIS<0b110001, "mov_dg.unpack $sw $dst, $src, $imm, $pred", SPred>;
  def MOV_DUAL_GROUP_UNPACKm_Dis : VpuInst_MOV_DUAL_GROUP_UNPACK_DIS<0b110001, "mov_dg.unpack $sw $dst, $src, $imm, $pred", VPred>;

  def MOV_DUAL_GROUP_CTRL_REGp_Dis : VpuInst_MOV_DUAL_GROUP_CTRL_REG<0b110001, "mov_dg $sw $dst, $src1, $src2, $imm, $pred", SPred>, Requires<[Doron1]>;
  def MOV_DUAL_GROUP_CTRL_REGm_Dis : VpuInst_MOV_DUAL_GROUP_CTRL_REG<0b110001, "mov_dg $sw $dst, $src1, $src2, $imm, $pred", VPred>, Requires<[Doron1]>;
  
  def MOV_DUAL_GROUP_CTRL_REG_ALLp_Dis : VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL_DIS<0b110001, "mov_dg.all $sw $dst, $src1, $src2, $imm, $pred", SPred>, Requires<[Doron1]>;
  def MOV_DUAL_GROUP_CTRL_REG_ALLm_Dis : VpuInst_MOV_DUAL_GROUP_CTRL_REG_ALL_DIS<0b110001, "mov_dg.all $sw $dst, $src1, $src2, $imm, $pred", VPred>, Requires<[Doron1]>;
}

// MOV_GROUP
// MSAC

// CALC_FP_SPECIAL - decoded fairly
// NOP - decoded fairly

// FCLASS
// FCLASS_LIMIT
let DecoderMethod = "decodeFclass" in {
  def FCLASSp_Dis : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", SPred>;
  def FCLASSm_Dis : VpuInst_UnarySw<0b011110, VRF, VRF, "fclass$optype $sw $dest, $op, $pred", VPred>;

  def FCLASS_LIMITp_Dis : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, VRF, "fclass$optype{.}$sw $dest, $op1, $op2, $op3, $pred", SPred>;
  def FCLASS_LIMITm_Dis : VpuInst_FCLASS_LIMIT<0b011110, VRF, VRF, VRF, VRF, "fclass$optype{.}$sw $dest, $op1, $op2, $op3, $pred", VPred>;
}

//------ Store slot.

// GEN_ADDR - decoded fairly
// PRMT_INDX - decoded fairly
// SET_INDX - decoded using RR hack

// ST_L
def ST_L_Dis : StoreInst_ST_L <0b000101, VRF, "st_l $sw $addr, $value, $pred">;

// ST_G
def ST_G_Dis : StoreInst_ST_G<0b000110, SRF, "st_g $sw $addr, $value, $pred">;

// SET_L_V
// SET_L_V_LOW
// SET_L_V_HIGH
//
// We have to create separate definitions as codegen defs have predicates.
def ST_L_V_Dis : StoreInst_ST_Vrrr<0b000111, VRF, "st_l_v $sw $addr, $src, $pred", SPred>;
def ST_L_Vm_Dis : StoreInst_ST_Vrrr<0b000111, VRF, "st_l_v $sw $addr, $src, $pred", VPred>;
def ST_L_V_LOW_Dis : StoreInst_ST_Vrrr<0b001000, VRF, "st_l_v_low $sw $addr, $src, $pred", SPred>;
def ST_L_V_LOWm_Dis : StoreInst_ST_Vrrr<0b001000, VRF, "st_l_v_low $sw $addr, $src, $pred", VPred>;
def ST_L_V_HIGH_Dis : StoreInst_ST_Vrrr<0b001001, VRF, "st_l_v_high $sw $addr, $src, $pred", SPred>;
def ST_L_V_HIGHm_Dis : StoreInst_ST_Vrrr<0b001001, VRF, "st_l_v_high $sw $addr, $src, $pred", VPred>;

// ASO - decoded fairly

// ST_TNSR
// ST_TNSR_LOW
// ST_TNSR_HIGH
let DecoderMethod = "decodeStTnsr", SrcExtra = ?, Switches = ? in {
  def ST_TNSR_Dis : StoreInst_ST_TNSR<0b001011, VRF, "st_tnsr $sw $tensor, $coords, $value, $pred">;
  def ST_TNSR_HIGH_Dis : StoreInst_ST_TNSR<0b001100, VRF, "st_tnsr_high $sw $tensor, $coords, $value, $pred">;
  def ST_TNSR_LOW_Dis : StoreInst_ST_TNSR<0b001101, VRF, "st_tnsr_low $sw $tensor, $coords, $value, $pred">;
}

let DecoderMethod = "decodeStTnsrSqz", SrcExtra = ?, Switches = ? in {
  def ST_TNSR_SQZ_Dis : StoreInst_ST_TNSR_SQZ<0b010110, "st_tnsr_sqz $sw $tensor, $squeeze, $coords, $value, $pred">, Requires<[Gen4Plus]>;
  def ST_TNSR_SQZ_FLUSH_Dis : StoreInst_ST_TNSR_SQZ_FLUSH<0b010110, "st_tnsr_sqz $sw, $squeeze">, Requires<[Doron1]>;
}

// ST_TNSR_S
let DecoderMethod = "decodeStTnsrS" in {
def ST_TNSR_Ssr_Dis      : StoreInst_ST_TNSR_S<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rsr_Dis    : StoreInst_ST_TNSR_S_RMW<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWRr_Dis   : StoreInst_ST_TNSR_S_HWR<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $pred", 1>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWR_Rr_Dis : StoreInst_ST_TNSR_S_HWR_RMW<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $rmw, $pred", 1>, Requires<[Gen4Plus]>;

def ST_TNSR_Ssi_Dis      : StoreInst_ST_TNSR_S<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_Rsi_Dis    : StoreInst_ST_TNSR_S_RMW<0b010111, SRF, "st_tnsr_s $sw $tensor, $coords, $value, $rmw, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWRi_Dis   : StoreInst_ST_TNSR_S_HWR<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $pred", 0>, Requires<[Gen4Plus]>;
def ST_TNSR_S_HWR_Ri_Dis : StoreInst_ST_TNSR_S_HWR_RMW<0b010111, "st_tnsr_s $sw $tensor, $coords, $hw_reg_sel, $rmw, $pred", 0>, Requires<[Gen4Plus]>;
}

// CACHE_INVALIDATE_ADDR
def CACHE_INVALIDATE_ADDR_Dis : StoreInstPredAddr<0b010101, "cache_invalidate $sw $addr, $pred", 0>, Requires<[Doron1]>;

// CACHE_FLUSH_ADDR
def CACHE_FLUSH_ADDR_Dis      : StoreInstPredAddr<0b010100, "cache_flush $sw $addr, $pred", 1>,      Requires<[Doron1]>;

// NOP - decoded fairly

// LD_TNSR
// LD_TNSR_LOW
// LD_TNSR_HIGH
// LD_TNSR_CNVRT
let DecoderMethod = "decodeLdTnsrSt", SrcExtra = ?, Switches = ? in {
def LD_TNSR_STp_Dis : StoreInst_LD_TNSR<0b010001, VRF, "ld_tnsr $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_STm_Dis : StoreInst_LD_TNSR<0b010001, VRF, "ld_tnsr $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_ST_LOWp_Dis : StoreInst_LD_TNSR<0b010010, VRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_ST_LOWm_Dis : StoreInst_LD_TNSR<0b010010, VRF, "ld_tnsr_low $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_ST_HIGHp_Dis : StoreInst_LD_TNSR<0b010011, VRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_ST_HIGHm_Dis : StoreInst_LD_TNSR<0b010011, VRF, "ld_tnsr_high $sw $dst, $tensor, $coords, $pred", VPred>;
def LD_TNSR_CNVRT_STp_Dis : StoreInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", SPred>;
def LD_TNSR_CNVRT_STm_Dis : StoreInst_LD_TNSR_CNVRT<0b011001, VRF, "ld_tnsr_cnvrt $sw $dst, $tensor, $coords, $pred", VPred>;
}

}
