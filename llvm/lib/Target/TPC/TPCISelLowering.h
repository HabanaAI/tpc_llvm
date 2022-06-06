//===-- TPCISelLowering.h - TPC DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that TPC uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCISELLOWERING_H
#define LLVM_LIB_TARGET_TPC_TPCISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class TPCSubtarget;
  namespace TPCISD {
    // TPC Specific DAG Nodes
    enum NodeType : unsigned {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,
      FPOR, FPAND, FPXOR, FPNOT, // Bitwise operations for floats
      // Dummy nodes
      MAC, FMAC, MAX, MIN, FMAX, FMIN, INOT,
      FSRL, FSHL, FSRA,
      HALT,
      COND_MOV, COND_MOV_INVERT,
      LD_G = ISD::FIRST_TARGET_MEMORY_OPCODE,
      LD_GI,
      LD_G_INC, //TODO: remove this
      ST_G,
      ST_G_INC
    };
  } // namespace TPCISD

  class TPCTargetLowering : public TargetLowering {
    const TPCSubtarget *Subtarget;

    SDValue truncate_32_to_16(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                              const SDLoc &DL, EVT ResultVT, uint64_t dataType) const;
    SDValue truncate_32_to_16_goya(SelectionDAG &DAG, SDValue Src0,
                                   SDValue Src1, const SDLoc &DL, EVT ResultVT,
                                   uint64_t dataType) const;
    SDValue truncate_16_to_8(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                             const SDLoc &DL, EVT ResultVT, uint64_t dataType) const;
    SDValue truncate_32_to_8(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                             SDValue Src2, SDValue Src3, const SDLoc &DL,
                             EVT ResultVT, uint64_t dataType) const;

    SmallVector<SDValue, 4> extend_8_to_32(SDValue Op, SelectionDAG &DAG,
                                           const SDLoc &DL,
                                           uint64_t DataType) const;
    SmallVector<SDValue, 2> extend_8_to_16(SDValue Op, SelectionDAG &DAG,
                                           const SDLoc &DL, uint64_t DataType,
                                           unsigned DstDataType) const;
    SmallVector<SDValue, 2> extend_16_to_32(SDValue Op, SelectionDAG &DAG,
                                            const SDLoc &DL,
                                            uint64_t DataType) const;
    SDValue lowerVectorFP_Extend(SDValue Op, SelectionDAG &DAG, SDLoc &DL,
                                 uint64_t DataType, EVT SubRegType,
                                 unsigned int InputSwitch,
                                 bool IsCastIntrinsicWithRM = false) const;
    SDValue lowerScalarFP_Extend(SelectionDAG &DAG, const SDValue &Src,
                                 const SDLoc &DL, uint64_t DataType) const;
  public:
    TPCTargetLowering(const TargetMachine &TM, const TPCSubtarget &ST);
    SDValue LowerCall(CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const override;
    SDValue LowerFormalArguments(SDValue, CallingConv::ID, bool,
                                 const SmallVectorImpl<ISD::InputArg> &,
                                 const SDLoc &, SelectionDAG &,
                                 SmallVectorImpl<SDValue> &) const override;
    SDValue LowerReturn(SDValue, CallingConv::ID, bool,
                        const SmallVectorImpl<ISD::OutputArg> &,
                        const SmallVectorImpl<SDValue> &, const SDLoc &,
                        SelectionDAG &) const override;
    bool isFPImmLegal(const APFloat & /*Imm*/, EVT /*VT*/,
                      bool ForCodeSize = false) const override;
    bool ShouldShrinkFPConstant(EVT) const override { return false; }
    TargetLowering::ConstraintType getConstraintType(StringRef Constraint) const override;
    const char *getTargetNodeName(unsigned Opcode) const override;
    Register getRegisterByName(const char *RegName, LLT Ty,
                               const MachineFunction &MF) const override;
      SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
    unsigned getZeroReg() const;
    bool isProfitableToHoist(Instruction *I) const override;
    SDValue createTuple(ArrayRef<SDValue> Regs, SelectionDAG &DAG) const;
    EVT getSetCCResultType(const DataLayout &DL, LLVMContext &, EVT VT) const override;
    bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM,
                               Type *Ty, unsigned AddrSpace,
                               Instruction *I = nullptr) const override;
    bool getIndexedAddressParts(SDNode *Op, SDValue &Base, SDValue &Offset,
                                SelectionDAG &DAG) const;
    bool getPostIndexedAddressParts(SDNode *N, SDNode *Op, SDValue &Base,
                                    SDValue &Offset, ISD::MemIndexedMode &AM,
                                    SelectionDAG &DAG) const override;
    bool canMergeStoresTo(unsigned AS, EVT MemVT, const SelectionDAG &DAG) const override;

    SDValue lowerMUL(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerMULHU(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerMULHS(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerVSELECT(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerVECTOR_SHUFFLE(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerLOAD(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSTORE(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerCONCAT_VECTORS(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerEXTRACT_SUBVECTOR(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerINSERT_SUBVECTOR(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerINTRINSIC_W_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerINTRINSIC_VOID(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSIGN_EXTEND(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerZERO_EXTEND(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFP_ROUND(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFP_EXTEND(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTRUNCATE_i32(SDValue Op, SelectionDAG &DAG) const;
    SDValue helperCONVERTSIGNED(EVT OpType, 
                                  SDValue Src,  unsigned  InputSwitch,
                                  bool IsCastIntrinsicWithRoundMode,
                                  SelectionDAG& DAG) const;
    SDValue lowerCONVERTSIGNED(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerCONVERTUNSIGNED(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerINSERT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTRUNCATE(SDValue Op, SelectionDAG &DAG,
                          unsigned int InputSwitch = 327680U,
                          bool IsCastIntrinsicWithRM = false) const;
    SDValue lowerBLOCK_ADDRESS(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerUDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerUREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFADD(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFSUB(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerADD(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSUB(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerFREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerAtomicCmpXchg(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerAtomicFence(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerAtomicRMW(SDValue Op, SelectionDAG &DAG) const;

    SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

    bool mergeStoresAfterLegalization(EVT MemVT) const override { return false; }

    bool getTgtMemIntrinsic(IntrinsicInfo &Info, const CallInst &I,
                            MachineFunction &MF,
                            unsigned Intrinsic) const override;
    SDValue lowerTPC_CONVERT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_CONVERT_LINEAR(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_CONVERT_INT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_CONVERT_UINT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_CONVERT_INT_LINEAR(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_CONVERT_UINT_LINEAR(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerTPC_MovDualGroup(SDValue Op, bool ModeAll, SelectionDAG &DAG) const;

  private:
    typedef enum {
      FIRST,
      v256i4,
      v256u4,
      v512i4,  v512u4,
      //v1024i4, v1024u4,

      v256i8,
      v256u8,
      v512i8,
      v512u8,
      v1024i8,    v1024u8,
      v128i16,
      v128u16,

      v256i16,
      v256u16,
      v512i16,     v512u16,
     // v1024i16,    v1024u16,

      v64i32,
      v64u32,
      v128i32,
      v128u32,
      v256i32,
      v256u32,
      v512i32,
      v512u32,

      v256f8_143,
      v512f8_143,
      v1024f8_143,

      v256f8_152,
      v512f8_152,
      v1024f8_152,

      v128f16,
      v256f16,
      v512f16,
      //v1024f16,

      v128bf16,
      v256bf16,
      v512bf16,
      //v1024bf16,

      v64f32,
      v128f32,
      v256f32,
      v512f32,
      v64i1, v128i1, v256i1,
      LAST
    } CVT;

    typedef enum {
      Empty,
      Hard,
      Soft,
      Split,
      Ban,
      IntHard,
      IntSoft,
      XX,
      Chain,
      VPemu,
      LoUpLanes,
      IntLoUpLanes,
      Skip,
      HardOrIntHard,
      BoolTo,
      HardLane
    } TConvKind;

        
    enum { ConvTypeNumber = CVT::LAST - CVT::FIRST - 1 };
    TConvKind MatrixConv[ConvTypeNumber][ConvTypeNumber];
    void InitConversionMatrix(void);
    CVT getSoSigned(CVT t) const;
    unsigned getCVTMultiplicity(TPCTargetLowering::CVT t) const;
    CVT toCVT(EVT evt, bool isUnsigned, unsigned bitlen /* for int4 */) const;
    TConvKind FindConversionElement(CVT fromCVT, CVT toCVT) const;

    SmallVector<SDValue, 2> Extend_8_to_16(EVT ResultVT,SDValue Src, SelectionDAG &DAG,
                                  const SDLoc &DL, unsigned DataType) const;
    SmallVector<SDValue, 4> Extend_8_to_32(EVT ResultVT, SDValue Src, SelectionDAG &DAG,
                                      const SDLoc &DL, unsigned DataType) const;

    SDValue QVTranspose0213(SDValue Source, SelectionDAG &DAG,
                            const SDLoc &DL) const;
    bool PreludeForVP(EVT DestVT, SDValue &Source, SDValue DataType,
                      SDValue Switches, SDValue &Income, SDValue &Predicate,
                      SDValue &Polarity, SelectionDAG &DAG, const SDLoc &DL,
                      bool DestIsUnsigned, bool SrcIsUnsigned) const;
    SDValue ConversionEntity(
        EVT DestVT, SDValue Source, SDValue DataType, SDValue Switches,
        SDValue Income, SDValue Predicate, SDValue Polarity,
                             SelectionDAG &DAG, const SDLoc &DL,
                             bool IsSwizzle = true,
                             SDValue *ShiftForInt = nullptr,
                             bool Onlysub_0 = false) const;

    SDValue FormFpNumScalar(SDValue Op1, SDValue Op2, SDValue Op3,
                            SelectionDAG &DAG, SDLoc &DL) const;
    SDValue ReciprocalCephesFastF32(SDValue Op, SelectionDAG &DAG,
                                    SDLoc &DL) const;
    SDValue ReciprocalCephesF32(SDValue Op, SelectionDAG &DAG,
                                SDLoc &DL) const;
    SDValue DivCephesF32(SDValue LHO, SDValue RHO, SelectionDAG &DAG,
                         SDLoc &DL) const;
    std::pair<SDValue, SDValue> MakeScalarDividePriorGen4(SDValue LHO, SDValue RHO,
        bool IsSigned, SelectionDAG &DAG, const SDLoc &DL) const;
    std::pair<SDValue, SDValue> MakeScalarDivideUpperGen4(
        SDValue LHO, SDValue RHO, unsigned Switch,
        bool IsSigned, SelectionDAG &DAG, const SDLoc &DL) const;
  };
} // end namespace llvm

#endif    // TPC_ISELLOWERING_H
