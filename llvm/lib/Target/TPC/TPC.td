//===-- TPC.td ---- Target definition file for the TPC -----*- tablegen -*-===//
//
//===----------------------------------------------------------------------===//
//
// This is a target description file for the TPC architecture.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//----------------------------------------------------------------------------//

def TPCInstrInfo : InstrInfo;

//----------------------------------------------------------------------------//
// TPC processors features..
//----------------------------------------------------------------------------//

def FeatureBF16   : SubtargetFeature<"bf16", "HasBFloat16Type", "true",
                                     "Supports type bloat16">;
def FeatureFP16   : SubtargetFeature<"fp16", "HasHalfFloatType", "true",
                                     "Supports type half-float">;
def FeatureFP8    : SubtargetFeature<"fp8", "HasFP8Types", "true",
                                     "Supports fp8 types">;
def FeatureCarry  : SubtargetFeature<"carry", "HasCarry", "true",
                                     "Supports add with carry">;
def FeaturePartial : SubtargetFeature<"partial", "HasPartial", "true",
                                     "LD_TNSR can load partial data">;
def FeatureMulI8   : SubtargetFeature<"muli8", "HasMulI8", "true",
                                      "Support of mul/mac for vector i8/u8">;
def FeatureAccI16   : SubtargetFeature<"acci16", "HasAccI16", "true",
                                      "Support of switch ACC_I16 in mac">;
def FeatureAccI32  : SubtargetFeature<"acci32", "HasAccI32", "true",
                                      "Support of switch ACC_I32 in mac">;
def FeatureX2Mac   : SubtargetFeature<"x2mac", "HasX2Mac", "true",
                                      "Support of switch X2 in mac">;
def FeatureZP      : SubtargetFeature<"zp", "HasZP", "true",
                                      "Support of ZP reg in mac">;
def FeatureAddr1   : SubtargetFeature<"addr1", "HasAddr1", "true",
                                      "Support of one-component address in LD/ST_L_V">;
def FeatureAddr2   : SubtargetFeature<"addr2", "HasAddr2", "true",
                                      "Support of two-component address in LD/ST_L_V">;
def FeatureADRFMov : SubtargetFeature<"adrfmov", "HasADRFMov", "true",
                                      "Support of ADRF moves">;
def FeatureCompress : SubtargetFeature<"compress", "HasCompress", "true",
                                      "Instruction compression">;
def FeatureDimMaskR : SubtargetFeature<"dimmask", "HasDimMaskR", "true",
                                       "DimMask may be in register">;
def FeatureShortImm : SubtargetFeature<"shortimm", "HasShortImm", "true",
                                       "Support of short immediates">;
def FeatureMADD     : SubtargetFeature<"madd", "HasMADD", "true",
                                       "Instruction MADD">;
def FeatureLdInStore : SubtargetFeature<"ld_in_store", "HasLdInStore", "true",
                                        "Instruction LD_TNSR in Store slot">;
def FeatureTnsrInReg : SubtargetFeature<"tnsr_in_reg", "HasTnsrInReg", "true",
                                        "Tensor number may be in HW register">;
def FeatureLdVectMask : SubtargetFeature<"ld_vprf_mask", "HasLdVectMask", "true",
                                        "LD_TNSR may use vector mask">;
def FeatureRMW        : SubtargetFeature<"rmw", "HasRMW", "true",
                                        "ST_TNSR may apply read-modify-write action">;
def FeatureTnsrPack   : SubtargetFeature<"tnsr_pack", "HasTnsrPack", "true",
                                        "ST_TNSR may have PACK modifier">;
def FeatureTnsrPackDT : SubtargetFeature<"tnsr_pack_dt", "HasTnsrPackDT", "true",
                                        "ST_TNSR may have PACK modifier with data type specification">;
def FeatureTnsrUnpack : SubtargetFeature<"tnsr_unpack", "HasTnsrUnpack", "true",
                                        "LD_TNSR may have UNPACK modifier">;
def FeatureMovDGUnpack : SubtargetFeature<"mov_dg_unpack", "HasMovDGUnpack", "true",
                                        "MOV_DG may have UNPACK modifier">;
def FeatureL0CS       : SubtargetFeature<"l0cs", "HasL0CS", "true",
                                        "Switch L0CS is supported">;
def FeatureDirect     : SubtargetFeature<"direct", "HasDirect", "true",
                                        "DIRECT switch is supported for LdTnsr*/StTnsr* instructions">;
def FeatureI4CVT      : SubtargetFeature<"i4cvt", "HasI4CVT", "true",
                                        "Convertion of i4 types">;
def FeatureVRF2VPRF   : SubtargetFeature<"vrf2vprf", "HasVRF2VPRF", "true",
                                        "Moves VRF <-> VPRF">;
def FeatureGetHSRF    : SubtargetFeature<"get_hsrf", "HasGetHSRF", "true",
                                        "Move hardware SRF to SRF">;
def FeatureHWMov      : SubtargetFeature<"hwmov", "HasHWMov", "true",
                                        "Moves to/from hardware registers">;
def FeatureVectStMask : SubtargetFeature<"vectstmask", "HasVectStMask", "true",
                                         "Store operation can have vector predicate">;

def FeatureGoya  : SubtargetFeature<"goya", "HasGoyaISA", "true",
                                    "Enable TPC version 1 ISA (goya)">;
def FeatureGaudi : SubtargetFeature<"gaudi", "HasGaudiISA", "true",
                                    "Enable TPC version 2 ISA (gaudi)">;
def FeatureGaudiB : SubtargetFeature<"gaudib", "HasGaudiBISA", "true",
                                    "Enable TPC version 2 ISA (gaudib)">;
def FeatureGreco : SubtargetFeature<"greco", "HasGrecoISA", "true",
                                    "Enable TPC version 3 ISA (greco)">;
def FeatureGaudi2 : SubtargetFeature<"gaudi2", "HasGaudi2ISA", "true",
                                     "Enable TPC version 4 ISA (gaudi2)">;
def FeatureDoron1 : SubtargetFeature<"doron1", "HasDoron1ISA", "true",
                                     "Enable TPC version doron1">;

// These are umbrella features, covering other minor changes
// accumulated in the course of architecture evolution.
// Ideally, they also should have been splitted into set of particular features,
// but still exist to ease support.
def FeatureGen2Plus   : SubtargetFeature<"gen2plus", "HasGen2Plus", "true",
                                         "Features introduced in Gen2 and valid in all higher">;

def FeatureGen2BPlus   : SubtargetFeature<"gen2bplus", "HasGen2BPlus", "true",
                                         "Features introduced in Gen2B and valid in all higher">;

// This is an umbrella feature, which should have been splitted into set of
// particular features but exists as a separate one because of ease of
// Gen4 support. It comprise features introduced in Gen3 and supported in
// all subsequent architectures.
def FeatureGen3Plus   : SubtargetFeature<"gen3plus", "HasGen3Plus", "true",
                                         "Features introduced in Gen3 and valid in all higher">;                       
def FeatureGen4Plus   : SubtargetFeature<"gen4plus", "HasGen4Plus", "true",
                                         "Features introduced in Gen4 and valid in all higher">;

def FeatureConvertFpFlex : SubtargetFeature<"convert_fp_flex", "HasConvertFpFlex", "true",
                                            "Support of convert_fp_flex">;

//----------------------------------------------------------------------------//
// TPC predicate definitions.
//----------------------------------------------------------------------------//

def BF16             : Predicate<"Subtarget->hasBFloat16()">,
                       AssemblerPredicate<(all_of FeatureBF16), "BFloat16 type">;
def FP16             : Predicate<"Subtarget->hasHalfFloat()">,
                       AssemblerPredicate<(all_of FeatureFP16), "Float16 type">;
def FP8              : Predicate<"Subtarget->hasFP8Types()">,
                       AssemblerPredicate<(all_of FeatureFP8), "FP8 types">;
def Partial          : Predicate<"Subtarget->hasPartial()">,
                       AssemblerPredicate<(all_of FeaturePartial), "Partial tensor loads">;
def MULI8            : Predicate<"Subtarget->hasMulI8()">,
                       AssemblerPredicate<(all_of FeatureMulI8), "MUL.I8">;
def AccI16           : Predicate<"Subtarget->hasAccI16()">,
                       AssemblerPredicate<(all_of FeatureAccI16), "ACC_I16 in MAC">;
def AccI32           : Predicate<"Subtarget->hasAccI32()">,
                       AssemblerPredicate<(all_of FeatureAccI32), "ACC_I32 in MAC">;
def X2Mac            : Predicate<"Subtarget->hasX2Mac()">,
                       AssemblerPredicate<(all_of FeatureX2Mac), "X2 in MAC">;
def ZP               : Predicate<"Subtarget->hasZP()">,
                       AssemblerPredicate<(all_of FeatureZP), "ZP in MAC">;
def Addr1            : Predicate<"Subtarget->hasAddr1()">,
                       AssemblerPredicate<(all_of FeatureAddr1), "Direct addressing">;
def Addr2            : Predicate<"Subtarget->hasAddr2()">,
                       AssemblerPredicate<(all_of FeatureAddr2), "Based addressing">;
def ADRFMov          : Predicate<"Subtarget->hasADRFMov()">,
                       AssemblerPredicate<(all_of FeatureADRFMov), "ADRF Moves">;
def Compress         : Predicate<"Subtarget->hasCompress()">,
                       AssemblerPredicate<(all_of FeatureCompress), "Instruction compression">;
def DimMaskR         : Predicate<"Subtarget->hasDimMaskR()">,
                       AssemblerPredicate<(all_of FeatureDimMaskR), "DimMask may be in register">;
def ShortImm         : Predicate<"Subtarget->hasShortImm()">,
                       AssemblerPredicate<(all_of FeatureShortImm), "Short immediates">;
def MADD             : Predicate<"Subtarget->hasMADD()">,
                       AssemblerPredicate<(all_of FeatureMADD), "Instruction MADD">;
def LdInStore        : Predicate<"Subtarget->hasLdInStore()">,
                       AssemblerPredicate<(all_of FeatureLdInStore), "Instruction LD_TNSR in Store slot">;
def TnsrInReg        : Predicate<"Subtarget->hasTnsrInReg()">,
                       AssemblerPredicate<(all_of FeatureTnsrInReg), "Tensor in HW register">;
def LdVectMask       : Predicate<"Subtarget->hasLdVectMask()">,
                       AssemblerPredicate<(all_of FeatureLdVectMask), "LD_TNSR may use vector mask">;
def RMW              : Predicate<"Subtarget->hasRMW()">,
                       AssemblerPredicate<(all_of FeatureRMW), "ST_TNSR may apply read-modify-write action">;
def TnsrPack         : Predicate<"Subtarget->hasTnsrPack()">,
                       AssemblerPredicate<(all_of FeatureTnsrPack), "ST_TNSR may have PACK modifier">;
def TnsrPackDT       : Predicate<"Subtarget->hasTnsrPackDT()">,
                       AssemblerPredicate<(all_of FeatureTnsrPackDT), "ST_TNSR may have PACK modifier with data type specification">;
def TnsrUnpack       : Predicate<"Subtarget->hasTnsrUnpack()">,
                       AssemblerPredicate<(all_of FeatureTnsrUnpack), "LD_TNSR may have UNPACK modifier">;
def MovDGUnpack      : Predicate<"Subtarget->hasMovDGUnpack()">,
                       AssemblerPredicate<(all_of FeatureMovDGUnpack), "MOV_DG may have UNPACK modifier">;
def L0CS             : Predicate<"Subtarget->hasL0CS()">,
                       AssemblerPredicate<(all_of FeatureL0CS), "Switch L0CS is supported">;
def Direct           : Predicate<"Subtarget->hasDirect()">,
                       AssemblerPredicate<(all_of FeatureDirect), "DIRECT Switch is supported">;
def I4CVT            : Predicate<"Subtarget->hasI4CVT()">,
                       AssemblerPredicate<(all_of FeatureI4CVT), "Conversion of i4 types">;
def VRF2VPRF         : Predicate<"Subtarget->hasVRF2VPRF()">,
                       AssemblerPredicate<(all_of FeatureVRF2VPRF), "Moves VRF <-> VPRF">;
def GetHSRF          : Predicate<"Subtarget->hasGetHSRF()">,
                       AssemblerPredicate<(all_of FeatureGetHSRF), "Move hardware SRF to SRF">;
def HWMov            : Predicate<"Subtarget->hasHWMov()">,
                       AssemblerPredicate<(all_of FeatureHWMov), "Move to/from hardware registers">;
def VectStMask       : Predicate<"Subtarget->hasVectStMask()">,
                       AssemblerPredicate<(all_of FeatureVectStMask), "Store may have vector predicate">;
def ConvertFpFlex    : Predicate<"Subtarget->hasConvertFpFlex()">,
                       AssemblerPredicate<(all_of FeatureConvertFpFlex), "Has convert_fp_flex instuction">;

def Gen1             : Predicate<"Subtarget->hasGoyaISA()">,
                       AssemblerPredicate<(all_of FeatureGoya), "Gen1 (Goya) mode">;
def Gen2             : Predicate<"Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA()">, 
                       AssemblerPredicate<(any_of FeatureGaudi, FeatureGaudiB), "Gen2 (Gaudi, GaudiB) mode">;
def Gen2B            : Predicate<"Subtarget->hasGaudiBISA()">,
                       AssemblerPredicate<(all_of FeatureGaudiB), "Gen2 (GaudiB) mode">;
def Gen3             : Predicate<"Subtarget->hasGrecoISA()">,
                       AssemblerPredicate<(all_of FeatureGreco), "Gen3 (Goya2) mode">;
def Gen4             : Predicate<"Subtarget->hasGaudi2ISA()">,
                       AssemblerPredicate<(all_of FeatureGaudi2), "Gen4 (Gaudi2) mode">;
def Sim_device       : Predicate<"Subtarget->hasDoron1ISA()">,
                       AssemblerPredicate<(all_of FeatureDoron1), "Sim_device (Doron1) mode">;

def Gen2Plus         : Predicate<"Subtarget->hasGen2Plus()">,
                       AssemblerPredicate<(any_of FeatureGaudi, FeatureGaudiB, FeatureGreco, FeatureGaudi2, FeatureDoron1), "Gen2 or higher architecture">;
def Gen2BPlus        : Predicate<"Subtarget->hasGaudiBISA() || Subtarget->hasGrecoISA() || Subtarget->hasGaudi2ISA()">,
                       AssemblerPredicate<(any_of FeatureGaudiB, FeatureGreco, FeatureGaudi2, FeatureDoron1), "Gen2B or higher mode">;
def Gen3Plus         : Predicate<"Subtarget->hasGen3Plus()">,
                       AssemblerPredicate<(any_of FeatureGreco, FeatureGaudi2, FeatureDoron1), "Gen3 or higher architecture">;
def Gen4Plus         : Predicate<"Subtarget->hasGen4Plus()">,
                       AssemblerPredicate<(any_of FeatureGaudi2, FeatureDoron1), "Gen4 or higher architecture">;
def Doron1         : Predicate<"Subtarget->hasDoron1()">,
                       AssemblerPredicate<(any_of FeatureDoron1), "Sim_device or higher architecture">;
def PriorGen3        : Predicate<"Subtarget->isPriorGen3()">,
                       AssemblerPredicate<(any_of FeatureGoya, FeatureGaudi, FeatureGaudiB), "architecture prior to Greco">;
def PriorGen4        : Predicate<"Subtarget->isPriorGen4()">,
                       AssemblerPredicate<(any_of FeatureGoya, FeatureGaudi, FeatureGaudiB, FeatureGreco), "architecture prior to Greco">;
def PriorDoron1        : Predicate<"Subtarget->isPriorDoron1()">,
                       AssemblerPredicate<(any_of FeatureGoya, FeatureGaudi, FeatureGaudiB, FeatureGreco, FeatureGaudi2), "architecture prior to Gaudi2">;
def Gen3TillDoron1     : Predicate<"Subtarget->isGen3TillDoron1()">,
                       AssemblerPredicate<(any_of FeatureGreco, FeatureGaudi2), "Greco architecture">;

//===----------------------------------------------------------------------===//
// Relation Map Definitions.
//===----------------------------------------------------------------------===//

class MultiSlot <string baseOp> {
  string BaseOp = baseOp;
}

class MultiSlotGen3 <string baseOp> {
  string BaseOp = baseOp;
}

class MultiSlotGen4 <string baseOp> {
  string BaseOp = baseOp;
}

class MultiSlotDoron1 <string baseOp> {
  string BaseOp = baseOp;
}

def getAltOpcodeLoadDoron1 : InstrMapping {
  let FilterClass = "MultiSlotDoron1";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["LoadSlot"];
  let ValueCols = [["StoreSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeStoreDoron1 : InstrMapping {
  let FilterClass = "MultiSlotDoron1";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["StoreSlot"];
  let ValueCols = [["LoadSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeSpuDoron1 : InstrMapping {
  let FilterClass = "MultiSlotDoron1";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["SpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["VpuSlot"]];
}

def getAltOpcodeVpuDoron1 : InstrMapping {
  let FilterClass = "MultiSlotDoron1";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["VpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["SpuSlot"]];
}

def getAltOpcodeLoadGen4 : InstrMapping {
  let FilterClass = "MultiSlotGen4";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["LoadSlot"];
  let ValueCols = [["StoreSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeStoreGen4 : InstrMapping {
  let FilterClass = "MultiSlotGen4";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["StoreSlot"];
  let ValueCols = [["LoadSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeSpuGen4 : InstrMapping {
  let FilterClass = "MultiSlotGen4";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["SpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["VpuSlot"]];
}

def getAltOpcodeVpuGen4 : InstrMapping {
  let FilterClass = "MultiSlotGen4";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["VpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["SpuSlot"]];
}

def getAltOpcodeLoadGen3 : InstrMapping {
  let FilterClass = "MultiSlotGen3";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["LoadSlot"];
  let ValueCols = [["StoreSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeStoreGen3 : InstrMapping {
  let FilterClass = "MultiSlotGen3";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["StoreSlot"];
  let ValueCols = [["LoadSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeSpuGen3 : InstrMapping {
  let FilterClass = "MultiSlotGen3";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["SpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["VpuSlot"]];
}

def getAltOpcodeVpuGen3 : InstrMapping {
  let FilterClass = "MultiSlotGen3";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["VpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["SpuSlot"]];
}

def getAltOpcodeLoad : InstrMapping {
  let FilterClass = "MultiSlot";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["LoadSlot"];
  let ValueCols = [["StoreSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeStore : InstrMapping {
  let FilterClass = "MultiSlot";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["StoreSlot"];
  let ValueCols = [["LoadSlot"], ["SpuSlot"], ["VpuSlot"]];
}

def getAltOpcodeSpu : InstrMapping {
  let FilterClass = "MultiSlot";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["SpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["VpuSlot"]];
}

def getAltOpcodeVpu : InstrMapping {
  let FilterClass = "MultiSlot";
  // Instructions with the same BaseOpcode form a row.
  let RowFields = ["BaseOp"];
  // Instructions with the same slot form a column.
  let ColFields = ["Slot"];
  let KeyCol = ["VpuSlot"];
  let ValueCols = [["LoadSlot"], ["StoreSlot"], ["SpuSlot"]];
}

//----------------------------------------------------------------------------//
// Register File, Calling Conv, Instruction Descriptions
//----------------------------------------------------------------------------//

include "TPCSchedule.td"
include "TPCRegisterInfo.td"
include "TPCInstrInfo.td"
include "TPCCallingConv.td"

//----------------------------------------------------------------------------//
// TPC processors supported.
//----------------------------------------------------------------------------//

class Proc<string Name, list<SubtargetFeature> Features>
 : ProcessorModel<Name, TPCModel, Features>;

def : Proc<"goya",  [FeatureGoya, FeatureMulI8, FeatureAddr2]>;
def : Proc<"gaudi", [FeatureGaudi,
                     FeatureBF16,
                     FeatureCarry,
                     FeatureAddr1,
                     FeatureLdVectMask,
                     FeatureRMW,
                     FeatureTnsrPack,
                     FeatureGetHSRF,
                     FeatureGen2Plus]>;
def : Proc<"gaudib", [FeatureGaudiB,
                     FeatureBF16,
                     FeatureFP16,
                     FeatureCarry,
                     FeatureAddr1,
                     FeatureLdVectMask,
                     FeatureRMW,
                     FeatureTnsrPack,
                     FeatureMovDGUnpack,
                     FeatureGetHSRF,
                     FeatureGen2Plus,
                     FeatureGen2BPlus]>;
def : Proc<"greco", [FeatureGreco,
                     FeatureBF16,
                     FeatureFP16,
                     FeatureCarry,
                     FeaturePartial,
                     FeatureAddr1,
                     FeatureMulI8,
                     FeatureAccI16,
                     FeatureAccI32,
                     FeatureX2Mac,
                     FeatureZP,
                     FeatureADRFMov,
                     FeatureCompress,
                     FeatureDimMaskR,
                     FeatureShortImm,
                     FeatureMADD,
                     /*FeatureLdInStore,*/
                     FeatureTnsrInReg,
                     FeatureLdVectMask,
                     FeatureRMW,
                     FeatureTnsrPack,
                     FeatureTnsrPackDT,
                     FeatureTnsrUnpack,
                     FeatureL0CS,
                     FeatureI4CVT,
                     FeatureVRF2VPRF,
                     FeatureGetHSRF,
                     FeatureHWMov,
                     FeatureGen2Plus,
                     FeatureGen3Plus]>;
def : Proc<"gaudi2", [FeatureGaudi2,
                     FeatureBF16,
                     FeatureFP16,
                     FeatureFP8,
                     FeatureCarry,
                     FeaturePartial,
                     FeatureAddr1,
                     FeatureMulI8,
                     FeatureAccI16,
                     FeatureAccI32,
                     FeatureX2Mac,
                     FeatureZP,
                     FeatureADRFMov,
                     FeatureCompress,
                     FeatureDimMaskR,
                     FeatureShortImm,
                     FeatureMADD,
                     FeatureLdInStore,
                     FeatureTnsrInReg,
                     FeatureLdVectMask,
                     FeatureRMW,
                     FeatureTnsrPack,
                     FeatureTnsrPackDT,
                     FeatureTnsrUnpack,
                     FeatureMovDGUnpack,
                     FeatureL0CS,
                     FeatureI4CVT,
                     FeatureVRF2VPRF,
                     FeatureGetHSRF,
                     FeatureHWMov,
                     FeatureGen2Plus,
                     FeatureGen3Plus,
                     FeatureGen4Plus,
//                   FeatureConvertFpFlex,
                     FeatureDirect,
                     FeatureVectStMask]>;
def : Proc<"doron1", [FeatureDoron1,
                     FeatureBF16,
                     FeatureFP16,
                     FeatureFP8,
                     FeatureCarry,
                     FeaturePartial,
                     FeatureAddr1,
                     FeatureMulI8,
                     FeatureAccI16,
                     FeatureAccI32,
                     FeatureX2Mac,
                     FeatureZP,
                     FeatureADRFMov,
                     FeatureCompress,
                     FeatureDimMaskR,
                     FeatureShortImm,
                     FeatureMADD,
                     FeatureLdInStore,
                     FeatureTnsrInReg,
                     FeatureLdVectMask,
                     FeatureRMW,
                     FeatureTnsrPack,
                     FeatureTnsrPackDT,
                     FeatureTnsrUnpack,
                     FeatureMovDGUnpack,
                     FeatureL0CS,
                     FeatureI4CVT,
                     FeatureVRF2VPRF,
                     FeatureGetHSRF,
                     FeatureHWMov,
                     FeatureGen2Plus,
                     FeatureGen3Plus,
                     FeatureGen4Plus,
                     FeatureDoron1,
                     FeatureDirect,
                     FeatureVectStMask]>;

//===----------------------------------------------------------------------===//
// Assembly Parser
//===----------------------------------------------------------------------===//

// Derfinitions of 'Variant' fields must agree with definitions of 'SlotParser'
// in 'TpcAsmParser.cpp'.

def SpecialSlotVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "special";
  string CommentDelimiter = "//";
  string RegisterPrefix = "";
}

def LoadSlotVariant : AsmParserVariant {
  int Variant = 1;
  string Name = "load";
  string CommentDelimiter = "//";
  string RegisterPrefix = "";
}

def ScalarSlotVariant : AsmParserVariant {
  int Variant = 2;
  string Name = "scalar";
  string CommentDelimiter = "//";
  string RegisterPrefix = "";
}

def VectorSlotVariant : AsmParserVariant {
  int Variant = 3;
  string Name = "vector";
  string CommentDelimiter = "//";
  string RegisterPrefix = "";
}

def StoreSlotVariant : AsmParserVariant {
  int Variant = 4;
  string Name = "store";
  string CommentDelimiter = "//";
  string RegisterPrefix = "";
}

//----------------------------------------------------------------------------//
// Declare the target which we are implementing
//----------------------------------------------------------------------------//

def TPC : Target {
    let InstructionSet = TPCInstrInfo;
    let AssemblyParserVariants = [SpecialSlotVariant, LoadSlotVariant, ScalarSlotVariant, VectorSlotVariant, StoreSlotVariant];
}

