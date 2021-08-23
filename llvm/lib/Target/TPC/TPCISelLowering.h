//===-- TPCISelLowering.h - TPC DAG Lowering Interface ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
      LD_G_INC, //TODO: remove this
      ST_G,
      ST_G_INC
    };
  } // namespace TPCISD

  class TPCTargetLowering : public TargetLowering {
    const TPCSubtarget *Subtarget;

    SDValue truncate_32_to_16(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                              SDLoc &DL, EVT ResultVT, uint64_t dataType) const;
    SDValue truncate_32_to_16_goya(SelectionDAG &DAG, SDValue Src0,
                                   SDValue Src1, SDLoc &DL, EVT ResultVT,
                                   uint64_t dataType) const;
    SDValue truncate_16_to_8(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                             SDLoc &DL, EVT ResultVT, uint64_t dataType) const;
    SDValue truncate_32_to_8(SelectionDAG &DAG, SDValue Src0, SDValue Src1,
                             SDValue Src2, SDValue Src3, SDLoc &DL,
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
    SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
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

  private:
    SDValue intDivRemHelper(SDValue Op, SelectionDAG &DAG, SDLoc &DL,
                            bool Unsigned = true) const;
    SDValue signedIntDivRemHelper(SDValue Op, SelectionDAG &DAG,
                                  unsigned SubregVal) const;
  };
} // end namespace llvm

#endif    // TPC_ISELLOWERING_H
