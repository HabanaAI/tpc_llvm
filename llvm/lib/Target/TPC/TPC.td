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
def FeatureCarry  : SubtargetFeature<"carry", "HasCarry", "true",
                                     "Supports add with carry">;
def FeatureMulI8   : SubtargetFeature<"muli8", "HasMulI8", "true",
                                      "Support of mul/mac for vector i8/u8">;
def FeatureAddr1   : SubtargetFeature<"addr1", "HasAddr1", "true",
                                      "Support of one-component address in LD/ST_L_V">;
def FeatureAddr2   : SubtargetFeature<"addr2", "HasAddr2", "true",
                                      "Support of two-component address in LD/ST_L_V">;
def FeatureLdVectMask : SubtargetFeature<"ld_vprf_mask", "HasLdVectMask", "true",
                                        "LD_TNSR may use vector mask">;
def FeatureRMW        : SubtargetFeature<"rmw", "HasRMW", "true",
                                        "ST_TNSR may apply read-modify-write action">;
def FeatureTnsrPack   : SubtargetFeature<"tnsr_pack", "HasTnsrPack", "true",
                                        "ST_TNSR may have PACK modifier">;
def FeatureGetHSRF    : SubtargetFeature<"get_hsrf", "HasGetHSRF", "true",
                                        "Move hardware SRF to SRF">;

def FeatureGoya  : SubtargetFeature<"goya", "HasGoyaISA", "true",
                                    "Enable TPC version 1 ISA (goya)">;
def FeatureGaudi : SubtargetFeature<"gaudi", "HasGaudiISA", "true",
                                    "Enable TPC version 2 ISA (gaudi)">;

// These are umbrella features, covering other minor changes
// accumulated in the course of architecture evolution.
// Ideally, they also should have been splitted into set of particular features,
// but still exist to ease support.
def FeatureGen2Plus   : SubtargetFeature<"gen2plus", "HasGen2Plus", "true",
                                         "Features introduced in Gen2 and valid in all higher">;

//----------------------------------------------------------------------------//
// TPC predicate definitions.
//----------------------------------------------------------------------------//

def BF16             : Predicate<"Subtarget->hasBFloat16()">,
                       AssemblerPredicate<"FeatureBF16", "BFloat16 type">;
def MULI8            : Predicate<"Subtarget->hasMulI8()">,
                       AssemblerPredicate<"FeatureMulI8", "MUL.I8">;
def Addr1            : Predicate<"Subtarget->hasAddr1()">,
                       AssemblerPredicate<"FeatureAddr1", "Direct addressing">;
def Addr2            : Predicate<"Subtarget->hasAddr2()">,
                       AssemblerPredicate<"FeatureAddr2", "Based addressing">;
def LdVectMask       : Predicate<"Subtarget->hasLdVectMask()">,
                       AssemblerPredicate<"FeatureLdVectMask", "LD_TNSR may use vector mask">;
def RMW              : Predicate<"Subtarget->hasRMW()">,
                       AssemblerPredicate<"FeatureRMW", "ST_TNSR may apply read-modify-write action">;
def TnsrPack         : Predicate<"Subtarget->hasTnsrPack()">,
                       AssemblerPredicate<"FeatureTnsrPack", "ST_TNSR may have PACK modifier">;
def GetHSRF          : Predicate<"Subtarget->hasGetHSRF()">,
                       AssemblerPredicate<"FeatureGetHSRF", "Move hardware SRF to SRF">;

def Gen1             : Predicate<"Subtarget->hasGoyaISA()">, AssemblerPredicate<"FeatureGoya", "Gen1 (Goya) mode">;
def Gen2             : Predicate<"Subtarget->hasGaudiISA()">, 
                       AssemblerPredicate<"!FeatureGoya", "Gen2 (Gaudi) mode">;

def Gen2Plus         : Predicate<"Subtarget->hasGen2Plus()">,
                       AssemblerPredicate<"FeatureGen2Plus", "Gen2 or higher architecture">;


//===----------------------------------------------------------------------===//
// Relation Map Definitions.
//===----------------------------------------------------------------------===//

class MultiSlot <string baseOp> {
  string BaseOp = baseOp;
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

