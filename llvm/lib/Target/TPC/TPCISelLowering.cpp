//===-- TPCISelLowering.cpp - TPC DAG Lowering
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that TPC uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "TPCISelLowering.h"
#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPC.h"
#include "TPCTargetMachine.h"
#include "TPCTargetObjectFile.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"

#ifdef LLVM_TPC_COMPILER
#include "llvm/Target/TPCClangInfo.h"
#endif //LLVM_TPC_COMPILER

using namespace llvm;

#define DEBUG_TYPE "tpc-isel"

const unsigned VRF_REGISTER_LENGTH_IN_BYTES = 256;
const unsigned VRF_REGISTER_LENGTH_IN_BITS = 8 * VRF_REGISTER_LENGTH_IN_BYTES;
const unsigned SRF_REGISTER_LENGTH_IN_BYTES = 4;
// const unsigned SRF_REGISTER_LENGTH_IN_BITS = 8 * SRF_REGISTER_LENGTH_IN_BYTES;

//
// Option to disable Scalar to IRF pass.
//
static cl::opt<bool> DisableIRFCopy("no-irf-copy", cl::Hidden, cl::ZeroOrMore,
                                    cl::init(false));

static cl::opt<std::string> TPCSchedPref("tpc-sched-preference", cl::Hidden,
                                         cl::ZeroOrMore, cl::init("source"));

// Aliases for type constants, to have more convenient names.
constexpr unsigned T_FP32 = TPCII::OpType::FP32;
constexpr unsigned T_BF16 = TPCII::OpType::BF16;
constexpr unsigned T_FP16 = TPCII::OpType::FP16;
constexpr unsigned T_FP8_143 = TPCII::OpType::FP8_143;
constexpr unsigned T_FP8_152 = TPCII::OpType::FP8_152;
constexpr unsigned T_INT32 = TPCII::OpType::INT32;
constexpr unsigned T_UINT32 = TPCII::OpType::UINT32;
constexpr unsigned T_INT16 = TPCII::OpType::INT16;
constexpr unsigned T_UINT16 = TPCII::OpType::UINT16;
constexpr unsigned T_INT8 = TPCII::OpType::INT8;
constexpr unsigned T_UINT8 = TPCII::OpType::UINT8;
constexpr unsigned T_INT4 = TPCII::OpType::INT4;
constexpr unsigned T_UINT4 = TPCII::OpType::UINT4;
constexpr unsigned T_INT1 = TPCII::OpType::BOOL;

static SDValue MoveWithPred(SDValue Source, EVT DestVT, SDValue Income,
                            SDValue Predicate, unsigned PolarityVal,
                            SelectionDAG &DAG, const SDLoc &DL);
static bool isSplatBuildVector(SDValue Op, SDValue &El);
static SDValue isSplatValue(SDValue V);
static unsigned getMultiplicity(EVT VT);
static EVT getQuarterEVT(EVT evt);
static EVT getHalfEVT(EVT evt);
static SDValue helperExtractSubRegSDValue(unsigned subReg, const SDLoc &DL,
                                          SelectionDAG &DAG, EVT elemType,
                                          SDValue reg);


    TPCTargetLowering::TPCTargetLowering(const TargetMachine &TM,
                                     const TPCSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {

  Sched::Preference spref = Sched::Source;
  if (StringRef(TPCSchedPref.getValue()).equals("none"))
    spref = Sched::None;
  else if (StringRef(TPCSchedPref.getValue()).equals("source"))
    spref = Sched::Source;
  else if (StringRef(TPCSchedPref.getValue()).equals("regpressure"))
    spref = Sched::RegPressure;
  else if (StringRef(TPCSchedPref.getValue()).equals("hybrid"))
    spref = Sched::Hybrid;
  else if (StringRef(TPCSchedPref.getValue()).equals("ilp"))
    spref = Sched::ILP;
  else if (StringRef(TPCSchedPref.getValue()).equals("vliw"))
    spref = Sched::VLIW;
  else
    report_fatal_error(
        "Invalid value for -tpc-sched-preference option.\nPossible values are: "
        "none, source, regpressure, hybrid, ilp, vliw.\n");

  setSchedulingPreference(spref);

  // Common value for scalar and vector memories.
  MaxStoresPerMemsetOptSize = MaxStoresPerMemset = 0;

  addRegisterClass(MVT::i1, &TPC::SPRFRegClass);
  addRegisterClass(MVT::f32, &TPC::SRFRegClass);
  addRegisterClass(MVT::bf16, &TPC::SRFRegClass);
  addRegisterClass(MVT::f16, &TPC::SRFRegClass);
  addRegisterClass(MVT::f8_143, &TPC::SRFRegClass);
  addRegisterClass(MVT::f8_152, &TPC::SRFRegClass);
  addRegisterClass(MVT::i32, &TPC::SRFRegClass);
  addRegisterClass(MVT::i16, &TPC::SRFRegClass);
  addRegisterClass(MVT::i64, &TPC::ADRFRegClass);
  addRegisterClass(MVT::i8, &TPC::SRFRegClass);
  addRegisterClass(MVT::v5i32, &TPC::IRFRegClass);
  addRegisterClass(MVT::v64f32, &TPC::VRFRegClass);
  addRegisterClass(MVT::v128f16, &TPC::VRFRegClass);
  addRegisterClass(MVT::v128bf16, &TPC::VRFRegClass);
  addRegisterClass(MVT::v256f8_143, &TPC::VRFRegClass);
  addRegisterClass(MVT::v256f8_152, &TPC::VRFRegClass);
  addRegisterClass(MVT::v64i32, &TPC::VRFRegClass);
  addRegisterClass(MVT::v128i16, &TPC::VRFRegClass);
  addRegisterClass(MVT::v256i8, &TPC::VRFRegClass);
  addRegisterClass(MVT::v128f32, &TPC::DRFRegClass);
  addRegisterClass(MVT::v256f16, &TPC::DRFRegClass);
  addRegisterClass(MVT::v256bf16, &TPC::DRFRegClass);
  addRegisterClass(MVT::v512f8_143, &TPC::DRFRegClass);
  addRegisterClass(MVT::v512f8_152, &TPC::DRFRegClass);
  addRegisterClass(MVT::v128i32, &TPC::DRFRegClass);
  addRegisterClass(MVT::v256i16, &TPC::DRFRegClass);
  addRegisterClass(MVT::v512i8, &TPC::DRFRegClass);
  addRegisterClass(MVT::v256i32, &TPC::ARFRegClass);
  addRegisterClass(MVT::v256f32, &TPC::ARFRegClass);
  addRegisterClass(MVT::v256i1, &TPC::VPRFRegClass);
  addRegisterClass(MVT::v128i1, &TPC::VPRFRegClass);
  addRegisterClass(MVT::v64i1, &TPC::VPRFRegClass);
  addRegisterClass(MVT::v2i32, &TPC::ZRFRegClass);
  addRegisterClass(MVT::v2i16, &TPC::ZRFRegClass);
  addRegisterClass(MVT::v2i8, &TPC::ZRFRegClass);

  setOperationAction(ISD::FADD, MVT::f8_143, Promote);
  AddPromotedToType(ISD::FADD, MVT::f8_143, MVT::f32);
  setOperationAction(ISD::FADD, MVT::f8_152, Promote);
  AddPromotedToType(ISD::FADD, MVT::f8_152, MVT::f32);

  setOperationAction(ISD::FSUB, MVT::f8_143, Promote);
  AddPromotedToType(ISD::FSUB, MVT::f8_143, MVT::f32);
  setOperationAction(ISD::FSUB, MVT::f8_152, Promote);
  AddPromotedToType(ISD::FSUB, MVT::f8_152, MVT::f32);

  setOperationAction(ISD::FADD, MVT::v256f32, Custom);
  setOperationAction(ISD::FADD, MVT::v128f32, Custom);
  setOperationAction(ISD::FADD, MVT::v256bf16, Custom);
  setOperationAction(ISD::FADD, MVT::v256f16, Custom);
  setOperationAction(ISD::FSUB, MVT::v256f32, Custom);
  setOperationAction(ISD::FSUB, MVT::v128f32, Custom);
  setOperationAction(ISD::FSUB, MVT::v256bf16, Custom);
  setOperationAction(ISD::FSUB, MVT::v256f16, Custom);
      
  setOperationAction(ISD::FMUL, MVT::f8_143, Promote);
  AddPromotedToType(ISD::FMUL, MVT::f8_143, MVT::f32);
  setOperationAction(ISD::FMUL, MVT::f8_152, Promote);
  AddPromotedToType(ISD::FMUL, MVT::f8_152, MVT::f32);

  setOperationAction(ISD::ADD, MVT::v256i32, Custom);
  setOperationAction(ISD::ADD, MVT::v128i32, Custom);
  setOperationAction(ISD::ADD, MVT::v256i16, Custom);
  setOperationAction(ISD::SUB, MVT::v256i32, Custom);
  setOperationAction(ISD::SUB, MVT::v128i32, Custom);
  setOperationAction(ISD::SUB, MVT::v256i16, Custom);

  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2i32, Custom);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2i16, Custom);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2i8, Custom);

  setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::v2i32, Custom);
  setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::v2i16, Custom);
  setOperationAction(ISD::INSERT_VECTOR_ELT, MVT::v2i8, Custom);

  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v5i32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v64f32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v128f32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256f32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v128f16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256f16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v128bf16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256bf16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256f8_143, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256f8_152, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v64i32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v128i32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256i32, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v128i16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v256i16, Custom);
  setOperationAction(ISD::VECTOR_SHUFFLE, MVT::v512i8, Custom);

  setOperationAction(ISD::BUILD_VECTOR, MVT::v256i32, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v128i32, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v128f32, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v256f32, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v256i16, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v256f16, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v256bf16, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v512i8, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v2i8, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v2i16, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v2i32, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v512f8_143, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v512f8_152, Custom);
  setOperationAction(ISD::BUILD_VECTOR, MVT::v256i8, Custom);

  setOperationAction(ISD::CONCAT_VECTORS, MVT::v256i32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v128i32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v128f32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v256f32, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v256f16, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v256bf16, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v256i16, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v512i8, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v512f8_143, Custom);
  setOperationAction(ISD::CONCAT_VECTORS, MVT::v512f8_152, Custom);

  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v64i32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v64f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v128i16, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v128f16, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v128bf16, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v256i8, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v256f8_143, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v256f8_152, Custom);

  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v128f32, Custom);
  setOperationAction(ISD::EXTRACT_SUBVECTOR, MVT::v128i32, Custom);


  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v128i32, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v128f32, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v256f32, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v256i32, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v256i16, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v256f16, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v256bf16, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v512i8, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v512f8_143, Custom);
  setOperationAction(ISD::INSERT_SUBVECTOR, MVT::v512f8_152, Custom);

  setOperationAction(ISD::BR_CC, MVT::i1, Expand);
  setOperationAction(ISD::BR_CC, MVT::i8, Expand);
  setOperationAction(ISD::BR_CC, MVT::i16, Expand);
  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::BR_CC, MVT::i64, Expand);
  setOperationAction(ISD::BR_CC, MVT::f32, Expand);
  setOperationAction(ISD::BR_CC, MVT::f64, Expand);
  setOperationAction(ISD::BR_CC, MVT::f16, Expand);
  setOperationAction(ISD::BR_CC, MVT::bf16, Expand);
  setOperationAction(ISD::BR_CC, MVT::f8_143, Expand);
  setOperationAction(ISD::BR_CC, MVT::f8_152, Expand);

  for (auto VT : {MVT::i8, MVT::i16, MVT::i32, MVT::i64}) {
    setOperationAction(ISD::ATOMIC_CMP_SWAP, VT, Custom);
    setOperationAction(ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS, VT, Custom);
    setOperationAction(ISD::ATOMIC_SWAP, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_ADD, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_SUB, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_AND, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_OR, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_XOR, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_NAND, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_MIN, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_MAX, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_UMIN, VT, Custom);
    setOperationAction(ISD::ATOMIC_LOAD_UMAX, VT, Custom);
  }

  setOperationAction(ISD::ATOMIC_FENCE, MVT::Other, Custom);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);

  setOperationAction(ISD::SELECT_CC, MVT::f32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::f16, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::bf16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f8_143, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f8_152, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i1, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i8, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i16, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::v5i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::v64i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v64f32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v128i16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v128bf16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v128f16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256i8, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256f8_143, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256f8_152, Custom);
  // double vectors
  setOperationAction(ISD::SELECT_CC, MVT::v128i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v128f32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256i16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256bf16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256f16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v512i8, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v512f8_143, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v512f8_152, Custom);
  //  quarter vectors
  setOperationAction(ISD::SELECT_CC, MVT::v256i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v256f32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::v512i16, Custom);

  setOperationAction(ISD::SELECT, MVT::f8_143, Custom);
  setOperationAction(ISD::SELECT, MVT::f8_152, Custom);
  setOperationAction(ISD::SELECT, MVT::f32, Custom);
  setOperationAction(ISD::SELECT, MVT::bf16, Custom);
  setOperationAction(ISD::SELECT, MVT::f16, Custom);
  setOperationAction(ISD::SELECT, MVT::i1, Custom);
  setOperationAction(ISD::SELECT, MVT::i8, Custom);
  setOperationAction(ISD::SELECT, MVT::i16, Custom);
  setOperationAction(ISD::SELECT, MVT::i32, Custom);
  setOperationAction(ISD::SELECT, MVT::v5i32, Custom);
  setOperationAction(ISD::SELECT, MVT::v64i32, Custom);
  setOperationAction(ISD::SELECT, MVT::v64f32, Custom);
  setOperationAction(ISD::SELECT, MVT::v128i16, Custom);
  setOperationAction(ISD::SELECT, MVT::v128bf16, Custom);
  setOperationAction(ISD::SELECT, MVT::v128f16, Custom);
  setOperationAction(ISD::SELECT, MVT::v256i8, Custom);
  setOperationAction(ISD::SELECT, MVT::v128f32, Custom);
  setOperationAction(ISD::SELECT, MVT::v256f8_143, Custom);
  setOperationAction(ISD::SELECT, MVT::v256f8_152, Custom);

  // double vectors
  setOperationAction(ISD::SELECT, MVT::v128i32, Custom);
  setOperationAction(ISD::SELECT, MVT::v128f32, Custom);
  setOperationAction(ISD::SELECT, MVT::v256i16, Custom);
  setOperationAction(ISD::SELECT, MVT::v256bf16, Custom);
  setOperationAction(ISD::SELECT, MVT::v256f16, Custom);
  setOperationAction(ISD::SELECT, MVT::v512i8, Custom);
  setOperationAction(ISD::SELECT, MVT::v512f8_143, Custom);
  setOperationAction(ISD::SELECT, MVT::v512f8_152, Custom);

    //  quarter vectors
  setOperationAction(ISD::SELECT, MVT::v256i32, Custom);
  setOperationAction(ISD::SELECT, MVT::v256f32, Custom);
  setOperationAction(ISD::SELECT, MVT::v512i16, Custom);

#if 0
  // may be will be needed somewhere 
  setOperationAction(ISD::VSELECT, MVT::v64i32, Custom);
  setOperationAction(ISD::VSELECT, MVT::v64f32, Custom);
  setOperationAction(ISD::VSELECT, MVT::v128i16, Custom);
  setOperationAction(ISD::VSELECT, MVT::v128bf16, Custom);
  setOperationAction(ISD::VSELECT, MVT::v128f16, Custom);
  setOperationAction(ISD::VSELECT, MVT::v256i8, Custom);
  setOperationAction(ISD::VSELECT, MVT::v128f32, Custom);
  setOperationAction(ISD::VSELECT, MVT::v256f8_143, Custom);
  setOperationAction(ISD::VSELECT, MVT::v256f8_152, Custom);
#endif

  setOperationAction(ISD::SETCC, MVT::v64i1, Custom);
  setOperationAction(ISD::SETCC, MVT::v128i1, Custom);
  setOperationAction(ISD::SETCC, MVT::v256i1, Custom);

  setOperationAction(ISD::ConstantFP, MVT::f16, Legal);
  setOperationAction(ISD::ConstantFP, MVT::bf16, Legal);
  setOperationAction(ISD::ConstantFP, MVT::f32, Legal);

  setOperationAction(ISD::LOAD, MVT::v5i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v256i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v128i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v128f32, Custom);
  setOperationAction(ISD::LOAD, MVT::v256f32, Custom);
  setOperationAction(ISD::LOAD, MVT::v256i16, Custom);
  setOperationAction(ISD::LOAD, MVT::v256f16, Custom);
  setOperationAction(ISD::LOAD, MVT::v256bf16, Custom);
  setOperationAction(ISD::LOAD, MVT::v512i8, Custom);
  setOperationAction(ISD::LOAD, MVT::v2i8, Custom);
  setOperationAction(ISD::LOAD, MVT::v2i16, Custom);
  setOperationAction(ISD::LOAD, MVT::v2i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v512f8_143, Custom);
  setOperationAction(ISD::LOAD, MVT::v512f8_152, Custom);

  // These types are custom lowered when loaded from global memory.
  setOperationAction(ISD::LOAD, MVT::i8, Custom);
  setOperationAction(ISD::LOAD, MVT::i16, Custom);
  setOperationAction(ISD::LOAD, MVT::i32, Custom);
  setOperationAction(ISD::LOAD, MVT::f32, Custom);
  setOperationAction(ISD::LOAD, MVT::f16, Custom);
  setOperationAction(ISD::LOAD, MVT::bf16, Custom);
  setOperationAction(ISD::LOAD, MVT::f8_143, Custom);
  setOperationAction(ISD::LOAD, MVT::f8_152, Custom);

  setOperationAction(ISD::STORE, MVT::v5i32, Custom);
  setOperationAction(ISD::STORE, MVT::v256i32, Custom);
  setOperationAction(ISD::STORE, MVT::v128i32, Custom);
  setOperationAction(ISD::STORE, MVT::v128f32, Custom);
  setOperationAction(ISD::STORE, MVT::v256f32, Custom);
  setOperationAction(ISD::STORE, MVT::v256i16, Custom);
  setOperationAction(ISD::STORE, MVT::v256f16, Custom);
  setOperationAction(ISD::STORE, MVT::v256bf16, Custom);
  setOperationAction(ISD::STORE, MVT::v512i8, Custom);
  setOperationAction(ISD::STORE, MVT::v2i8, Custom);
  setOperationAction(ISD::STORE, MVT::v2i16, Custom);
  setOperationAction(ISD::STORE, MVT::v2i32, Custom);

  if (Subtarget->getCPU() == "gaudi2")
    for (auto VT : {MVT::f8_143, MVT::f8_152, MVT::bf16, MVT::f16, MVT::f32,
                    MVT::i8, MVT::i16, MVT::i32}) {
      setIndexedLoadAction(ISD::POST_INC, VT, Custom);
      setIndexedStoreAction(ISD::POST_INC, VT, Custom);
    }

  for (auto VT :
       {MVT::v256i32, MVT::v128i32, MVT::v256i16, MVT::v128i16, MVT::v256i8})
    setOperationAction(ISD::SIGN_EXTEND, VT, Custom);

  setOperationAction(ISD::ZERO_EXTEND, MVT::v128i32, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v256i32, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v256i16, Custom);

  for (auto VT : {MVT::v128f32, MVT::v128f16, MVT::v128bf16, MVT::v256f16})
    setOperationAction(ISD::FP_ROUND, VT, Custom);

  for (auto VT : {MVT::v256bf16, MVT::v256f16, MVT::v128f32, MVT::v256f32})
    setOperationAction(ISD::FP_EXTEND, VT, Custom);

  setOperationAction(ISD::TRUNCATE, MVT::v256i16, Custom);
  setOperationAction(ISD::TRUNCATE, MVT::v128i16, Custom);
  setOperationAction(ISD::TRUNCATE, MVT::v256i8, Custom);

  for (auto VT : {MVT::v256i8, MVT::v128i16}) {
    setOperationAction(ISD::FP_TO_SINT, VT, Custom);
    setOperationAction(ISD::FP_TO_UINT, VT, Custom);
  }

  setOperationAction(ISD::FP_TO_SINT, MVT::v64i32, Custom);

  for (auto VT : {MVT::v256i32, MVT::v128i32, MVT::v64i32, MVT::v128i16,
                  MVT::v256i8, MVT::v256i16, MVT::v64i1}) {
    setOperationAction(ISD::SINT_TO_FP, VT, Custom);
    setOperationAction(ISD::UINT_TO_FP, VT, Custom);
  }

  // TPC doesn't have sext_inreg, replace them with shl/sra
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  setOperationAction(ISD::FMINNUM, MVT::v64f32, Legal);
  setOperationAction(ISD::FMAXNUM, MVT::v64f32, Legal);

  setOperationAction(ISD::MUL, MVT::v64i32, Custom);
  setOperationAction(ISD::MUL, MVT::v128i16, Custom);
  setOperationAction(ISD::MUL, MVT::v256i8, Custom);
  setOperationAction(ISD::MULHU, MVT::i32, Custom);
  setOperationAction(ISD::MULHU, MVT::i16, Custom);
  setOperationAction(ISD::MULHU, MVT::i8, Custom);
  setOperationAction(ISD::MULHS, MVT::i32, Custom);
  setOperationAction(ISD::MULHS, MVT::i16, Custom);
  setOperationAction(ISD::MULHS, MVT::i8, Custom);

  setOperationAction(ISD::TRUNCATE, MVT::i32, Custom);

  for (auto VT : {MVT::i32, MVT::i16, MVT::i8, MVT::v64i32}) {
    setOperationAction(ISD::UDIV, VT, Custom);
    setOperationAction(ISD::UREM, VT, Custom);
    setOperationAction(ISD::SDIV, VT, Custom);
    setOperationAction(ISD::SREM, VT, Custom);
  }

  for (auto VT :{ MVT::i32, MVT::i16, MVT::i8,
                  MVT::v64i32, MVT::v128i16, MVT::v256i8 }) {
    setOperationAction(ISD::SADDSAT, VT, Legal);
    setOperationAction(ISD::UADDSAT, VT, Legal);
    setOperationAction(ISD::SSUBSAT, VT, Legal);
    setOperationAction(ISD::USUBSAT, VT, Legal);
  }

  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);

  for (auto VT : { MVT::f32, MVT::v64f32 }) {
    setOperationAction(ISD::FDIV, VT, Custom);
    setOperationAction(ISD::FREM, VT, Custom);
  }

  //  setOperationAction(ISD::SUB, MVT::v256i32, Expand);
  // we got v64i32 = BUILD_VECTOR t1305, t1308,... which can not be supported

  setTruncStoreAction(MVT::i64, MVT::i32, Custom);

  for (auto ValTy : {MVT::f32, MVT::f16, MVT::bf16}) {
    setLoadExtAction(ISD::EXTLOAD, ValTy, MVT::f8_143, Expand);
    setLoadExtAction(ISD::EXTLOAD, ValTy, MVT::f8_152, Expand);
  }
  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::bf16, Expand);

  // Mark extending vector loads (like v128bf16(mem)->v128f32(reg)) as custom
  // lowered. They are unavailable on TPC and considered legal by default.
  for (auto DTy : { MVT::v256f32, MVT::v128f32, MVT::v256f16, MVT::v256bf16 })
    for (auto STy : { MVT::v64f32, MVT::v128f32,
                      MVT::v128f16, MVT::v256f16,
                      MVT::v128bf16, MVT::v256bf16,
                      MVT::v256f8_143, MVT::v256f8_152 })
      setLoadExtAction(ISD::EXTLOAD, DTy, STy, Custom);

  setOperationAction(ISD::FP_EXTEND, MVT::f32, Legal);

  // Unordered comparisons.
  //
  // These unordered comparisons appear as a result of canonicalization in
  // InstCombiner::visitBranchInst.
  setCondCodeAction(ISD::SETUGT, MVT::f32, Expand);
  setCondCodeAction(ISD::SETUGT, MVT::f16, Expand);
  setCondCodeAction(ISD::SETUGT, MVT::bf16, Expand);
  setCondCodeAction(ISD::SETULT, MVT::f32, Expand);
  setCondCodeAction(ISD::SETULT, MVT::f16, Expand);
  setCondCodeAction(ISD::SETULT, MVT::bf16, Expand);
  setCondCodeAction(ISD::SETUEQ, MVT::f32, Expand);
  setCondCodeAction(ISD::SETUEQ, MVT::f16, Expand);
  setCondCodeAction(ISD::SETUEQ, MVT::bf16, Expand);
  setCondCodeAction(ISD::SETUEQ, MVT::f8_143, Expand);
  setCondCodeAction(ISD::SETUEQ, MVT::f8_152, Expand);
  // These are created in DAGCombiner.
  setCondCodeAction(ISD::SETUNE, MVT::f32, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f16, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::bf16, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f8_143, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f8_152, Expand);

  if (!DisableIRFCopy) {
    setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);
  }
  setOperationAction(ISD::INTRINSIC_VOID, MVT::Other, Custom);

  for (auto Ty :{ MVT::v64f32, MVT::v128f32, MVT::v256f32,
                  MVT::v128f16, MVT::v256f16,
                  MVT::v128bf16, MVT::v256bf16,
                  MVT::v256f8_143, MVT::v256f8_152,
                  MVT::v64i32, MVT::v128i32, MVT::v256i32,
                  MVT::v128i16, MVT::v256i16,
                  MVT::v256i8, MVT::v512i8 })
    setOperationAction(ISD::INTRINSIC_WO_CHAIN, Ty, Custom);


  setOperationAction(ISD::INTRINSIC_W_CHAIN, MVT::i64, Custom);
  setOperationAction(ISD::INTRINSIC_W_CHAIN, MVT::Other, Custom);

  setTargetDAGCombine(ISD::BITCAST);
  setTargetDAGCombine(ISD::ADD);
  setTargetDAGCombine(ISD::VECTOR_SHUFFLE);
  setTargetDAGCombine(ISD::VSELECT);

  setMinFunctionAlignment(Align(2));
  computeRegisterProperties(Subtarget->getRegisterInfo());

  // we don't need to sink (and duplicate) compares
  // aggressively in CodeGenPrep.
  setHasMultipleConditionRegisters();
  InitConversionMatrix();
}

unsigned TPCTargetLowering::getZeroReg() const { return TPC::S31; }

bool TPCTargetLowering::isProfitableToHoist(Instruction *I) const {
  if (I->getOpcode() == Instruction::Call)
    return false;
  return TargetLowering::isProfitableToHoist(I);
}

unsigned getTargetConvertType(const TPCSubtarget &Subtarget) {
  if (Subtarget.hasGrecoISA() || Subtarget.hasGaudiISA()) {
    return TPC::CONVERT_INT16g2vip;
  } else {
    return TPC::CONVERT_INT16vvp;
  }
}

const char *TPCTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
    case (TPCISD::FPOR):  return "TPC::FPOR";
    case (TPCISD::FPAND): return "TPC::FPAND";
    case (TPCISD::FPXOR): return "TPC::FPXOR";
    case (TPCISD::FPNOT): return "TPC::FPNOT";
    case (TPCISD::MAC):   return "TPC::MAC";
    case (TPCISD::FMAC):  return "TPC::FMAC";
    case (TPCISD::MAX):   return "TPC::MAX";
    case (TPCISD::MIN):   return "TPC::MIN";
    case (TPCISD::FMAX):  return "TPC::FMAX";
    case (TPCISD::FMIN):  return "TPC::FMIN";
    case (TPCISD::INOT):  return "TPC::INOT";
    case (TPCISD::FSRL):  return "TPC::FSRL";
    case (TPCISD::FSHL):  return "TPC::FSHL";
    case (TPCISD::FSRA):  return "TPC::FSRA";
    case (TPCISD::HALT):  return "halt";
    case (TPCISD::COND_MOV): return "TPC::COND_MOV";
    case (TPCISD::COND_MOV_INVERT): return "TPC::COND_MOV_INVERT";
    case (TPCISD::LD_G):  return "TPC::LD_G";
    case (TPCISD::LD_G_INC):  return "TPC::LD_G_INC";
    case (TPCISD::ST_G):  return "TPC::ST_G";
    case (TPCISD::ST_G_INC):  return "TPC::ST_G_INC";
    default:              return nullptr;
  }
}

Register TPCTargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                              const MachineFunction &MF) const {
  unsigned Reg = StringSwitch<unsigned>(RegName)
                     .Case("sp0", TPC::SP0)
                     .Case("vp0", TPC::VP0)
                     .Case("lfsr", TPC::LFSR)
                     .Case("lfsr_no_change", TPC::LFSR_NO_CHANGE)
                     .Case("s_lfsr", TPC::S_LFSR)
                     .Case("s_lfsr_no_change", TPC::S_LFSR_NO_CHANGE)
                     .Case("v_lane_id_32", TPC::V_LANE_ID_32)
                     .Case("v_lane_id_16", TPC::V_LANE_ID_16)
                     .Case("v_lane_id_8", TPC::V_LANE_ID_8)
                     .Case("i2", TPC::I2)
                     .Default(0);

  if (Reg)
    return Reg;

  report_fatal_error("Invalid register name global variable");
}

static bool CC_TPC_EXTRA_ARGS(unsigned &ValNo, MVT &ValVT,
                             MVT &LocVT, CCValAssign::LocInfo &LocInfo,
                             ISD::ArgFlagsTy &ArgFlags, CCState &State) {
  MachineFunction &MF = State.getMachineFunction();
  
  const unsigned MaxTensor = MF.getTarget().Options.MaxTensor;
  if (((NumArgumentTensors == MaxTensor - 2) && UsePrintf) ||
      (NumArgumentTensors == MaxTensor - 1))
    report_fatal_error("Number of used tensors exceeds architecture limit. "
                       "Note that passing so many arguments to main function "
                       "requires extra (hidden) tensor.");
  
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  
  assert(!MF.empty());
  MachineBasicBlock &MBB = MF.front();
  
  Register IRFIncome = MRI.createVirtualRegister(&TPC::IRFRegClass);
  MachineInstr *IrfInit = BuildMI(MBB, MBB.begin(), DebugLoc(),
                                  TII->get(TargetOpcode::IMPLICIT_DEF),
                                  IRFIncome)
    .getInstr();
  
  Register IRF = MRI.createVirtualRegister(&TPC::IRFRegClass);
  static unsigned NumExtraArg = 0;
  MachineInstr *Mov = BuildMI(MBB, std::next(IrfInit->getIterator()),
                              DebugLoc(), TII->get(TPC::MOV_ld_Iip), IRF)
    .addImm(NumExtraArg++)
    .addImm(1)
    .addImm(0)
    .addReg(IRFIncome)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0)
    .getInstr();
  
  
  Register AddrIncome = MRI.createVirtualRegister(&TPC::ADRFRegClass);
  MachineInstr *AddrInit = BuildMI(MBB, std::next(Mov->getIterator()),
                                   DebugLoc(),
                                   TII->get(TargetOpcode::IMPLICIT_DEF),
                                   AddrIncome)
    .getInstr();
  
  Register Addr = MRI.createVirtualRegister(&TPC::ADRFRegClass);
  MachineInstr *GenAddr = BuildMI(MBB, std::next(AddrInit->getIterator()),
                                  DebugLoc(), TII->get(TPC::GEN_ADDR_ld), Addr)
    .addImm(NumArgumentTensors)
    .addReg(IRF, getKillRegState(true))
    .addImm(0)
    .addReg(AddrIncome)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0)
    .getInstr();
  
  Register ArgIncome = MRI.createVirtualRegister(&TPC::SRFRegClass);
  MachineInstr *ArgInit = BuildMI(MBB, std::next(GenAddr->getIterator()),
                                 DebugLoc(),
                                 TII->get(TargetOpcode::IMPLICIT_DEF),
                                 ArgIncome);
  
  Register Arg = MRI.createVirtualRegister(&TPC::SRFRegClass);
  BuildMI(MBB, std::next(ArgInit->getIterator()), DebugLoc(),
          TII->get(TPC::LD_Gsap), Arg)
    .addReg(Addr, getKillRegState(true))
    .addImm(0)
    .addReg(ArgIncome)
    .addReg(TPC::SPRF_TRUE)
    .addImm(0);
  
  State.addLoc(CCValAssign::getCustomReg(ValNo, ValVT, Arg, LocVT, LocInfo));
  return true;
}

#include "TPCGenCallingConv.inc"

SDValue TPCTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  Function *F = CLI.CB->getCalledFunction();
  StringRef FuncName = "<>";
  if (F)
    FuncName = F->getName();
  report_fatal_error("Function '" + FuncName + "' is used but not defined", false);
  return SDValue();
}

SDValue TPCTargetLowering::LowerFormalArguments(
    SDValue chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &mf = DAG.getMachineFunction();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 32> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
      
  CCInfo.AnalyzeFormalArguments(Ins, CC_TPC);

  for (unsigned i = 0; i != ArgLocs.size(); ++i) {
    CCValAssign &ArgLoc = ArgLocs[i];
    unsigned Register = MCRegister::NoRegister;
    
    if (ArgLoc.isRegLoc()) {
      if (!ArgLoc.needsCustom())
        Register = mf.addLiveIn(ArgLoc.getLocReg(), &TPC::SRFRegClass);
      else
        Register = ArgLoc.getLocReg();
    } else {
      llvm_unreachable("Only register available");
    }
    
    SDValue Copy = DAG.getCopyFromReg(chain, dl, Register, ArgLoc.getLocVT());
    if (ArgLoc.getValVT() != ArgLoc.getLocVT()) {
      assert(ArgLoc.isExtInLoc());
      Copy = DAG.getZExtOrTrunc(Copy, dl, ArgLoc.getValVT());
    }
    InVals.push_back(Copy);
  }

  if (mf.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
    for (MachineBasicBlock &MBB : mf) {
      if (MBB.getNumber() != 0)
        MBB.addLiveIn(getZeroReg());
    }
  }

  return chain;
}

SDValue
TPCTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CC, bool Flag,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &Vec,
                               const SDLoc &Loc, SelectionDAG &DAG) const {
  return DAG.getNode(TPCISD::HALT, Loc, MVT::Other, Chain);
}

bool TPCTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT,
                                     bool ForCodeSize) const {
  return VT == MVT::f32 || VT == MVT::f16 || VT == MVT::bf16;
}

TargetLowering::ConstraintType
TPCTargetLowering::getConstraintType(StringRef Constraint) const {
  unsigned S = Constraint.size();

  if (S == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 's': // Scalar register
    case 'v': // Vector register
    case 'S': // Scalar predicate register
    case 'V': // Vector predicare register
      return C_RegisterClass;
    case 'g': // Global memory
    case 'l': // Local scalar memory
    case 'L': // Local vector memory
      return C_Memory;
    }
  }

  if (S > 1 && Constraint[0] == '{' && Constraint[S - 1] == '}') {
    if (S == 8 && Constraint.substr(1, 6) == "memory") // "{memory}"
      return C_Memory;
    return C_Register;
  }
  return C_Unknown;
}

SDValue TPCTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "TPC custom lowering:\n    "; Op.dump(&DAG););
  switch (Op.getOpcode()) {
  default:
    assert(false && "unimplemented operand");
    return SDValue();
  case ISD::SETCC:
    return LowerSETCC(Op, DAG);
#if 0
  case ISD::VSELECT:
    return LowerVSELECT(Op, DAG);
#endif
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::SELECT:
    return LowerSELECT(Op, DAG);
  case ISD::VECTOR_SHUFFLE:
    return lowerVECTOR_SHUFFLE(Op, DAG);
  case ISD::BUILD_VECTOR:
    return lowerBUILD_VECTOR(Op, DAG);
  case ISD::LOAD:
    return lowerLOAD(Op, DAG);
  case ISD::STORE:
    return lowerSTORE(Op, DAG);
  case ISD::CONCAT_VECTORS:
    return lowerCONCAT_VECTORS(Op, DAG);
  case ISD::EXTRACT_SUBVECTOR:
    return lowerEXTRACT_SUBVECTOR(Op, DAG);
  case ISD::INSERT_SUBVECTOR:
    return lowerINSERT_SUBVECTOR(Op, DAG);
  case ISD::TRUNCATE:
    return lowerTRUNCATE(Op, DAG);
  case ISD::INTRINSIC_WO_CHAIN:
    return LowerINTRINSIC_WO_CHAIN(Op, DAG);
  case ISD::INTRINSIC_W_CHAIN:
    return LowerINTRINSIC_W_CHAIN(Op, DAG);
  case ISD::INTRINSIC_VOID:
    return LowerINTRINSIC_VOID(Op, DAG);
  case ISD::SIGN_EXTEND:
    return lowerSIGN_EXTEND(Op, DAG);
  case ISD::ZERO_EXTEND:
    return lowerZERO_EXTEND(Op, DAG);
  case ISD::FP_ROUND:
    return lowerFP_ROUND(Op, DAG);
  case ISD::FP_EXTEND:
    return lowerFP_EXTEND(Op, DAG);
  case ISD::FP_TO_SINT:
  case ISD::SINT_TO_FP:
    return lowerCONVERTSIGNED(Op, DAG);
  case ISD::FP_TO_UINT:
  case ISD::UINT_TO_FP:
    return lowerCONVERTUNSIGNED(Op, DAG);
  case ISD::EXTRACT_VECTOR_ELT:
    return lowerEXTRACT_VECTOR_ELT(Op, DAG);
  case ISD::INSERT_VECTOR_ELT:
    return lowerINSERT_VECTOR_ELT(Op, DAG);
  case ISD::MUL:
    return lowerMUL(Op, DAG);
  case ISD::MULHU:
    return lowerMULHU(Op, DAG);
  case ISD::MULHS:
    return lowerMULHS(Op, DAG);
  case ISD::ATOMIC_CMP_SWAP:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
    return lowerAtomicCmpXchg(Op, DAG);
  case ISD::UDIV:
    return lowerUDIV(Op, DAG);
  case ISD::SDIV:
    return lowerSDIV(Op, DAG);
  case ISD::UREM:
    return lowerUREM(Op, DAG);
  case ISD::FADD:
    return lowerFADD(Op, DAG);
  case ISD::FSUB:
    return lowerFSUB(Op, DAG);
  case ISD::ADD:
    return lowerADD(Op, DAG);
  case ISD::SUB:
    return lowerSUB(Op, DAG);
  case ISD::FDIV:
    return lowerFDIV(Op, DAG);
  case ISD::FREM:
    return lowerFREM(Op, DAG);
  case ISD::ATOMIC_FENCE:
    return lowerAtomicFence(Op, DAG);
  case ISD::ATOMIC_SWAP:
  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_LOAD_SUB:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_LOAD_NAND:
  case ISD::ATOMIC_LOAD_MIN:
  case ISD::ATOMIC_LOAD_MAX:
  case ISD::ATOMIC_LOAD_UMIN:
  case ISD::ATOMIC_LOAD_UMAX:
    return lowerAtomicRMW(Op, DAG);
  case ISD::BlockAddress:
    return lowerBLOCK_ADDRESS(Op, DAG);
  }
}

EVT TPCTargetLowering::getSetCCResultType(const DataLayout &DL,
                                          LLVMContext &Context, EVT VT) const {
  if (VT.isVector()) {
    return EVT::getVectorVT(Context, MVT::i1, VT.getVectorNumElements());
  }
  return MVT::SimpleValueType::i1;
}

/// Return true if the addressing mode represented by AM is legal for this
/// target, for a load/store of the specified type.
///
/// The type may be VoidTy, in which case only return true if the addressing
/// mode is legal for a load/store of any legal type.  TODO: Handle
/// pre/postinc as well.
///
bool TPCTargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM, Type *Ty,
                                              unsigned AddrSpace,
                                              Instruction *I) const {
  /// An addressing mode is represented as:
  ///    BaseGV + BaseOffs + BaseReg + Scale*ScaleReg
  /// If BaseGV is null,  there is no BaseGV.
  /// If BaseOffs is zero, there is no base offset.
  /// If HasBaseReg is false, there is no base register.
  /// If Scale is zero, there is no ScaleReg.  Scale of 1 indicates a reg with
  /// no scale.

  // No global is ever allowed as a base.
  if (AM.BaseGV)
    return false;

  switch (AM.Scale) {
  case 0: // "r+i" or just "i", depending on HasBaseReg.
    // Although "r+i" generally is not supported on TPC, disallowing it results
    // in malfunction of many passes, in particular, LICM and StrengthReduction.
    //    if (AM.HasBaseReg &&
    //        !(Subtarget->getFeatureBits()[TPC::FeatureAddr2] && AddrSpace ==
    //        LOCAL_VECTOR) && AM.BaseOffs)
    //      return false;
    break;
  case 1:
    if (AM.HasBaseReg && AM.BaseOffs) // "r+r+i" is not allowed.
      return false;
    // Otherwise we have r+r or r+i.
    if (AM.BaseOffs && !(Subtarget->getFeatureBits()[TPC::FeatureAddr2] &&
                         AddrSpace == LOCAL_VECTOR))
      return false;
    break;
  case 2:
    if (AM.HasBaseReg || AM.BaseOffs) // 2*r+r  or  2*r+i is not allowed.
      return false;
    // Allow 2*r as r+r on target with 2-component address.
    if (!Subtarget->getFeatureBits()[TPC::FeatureAddr2] ||
        AddrSpace != LOCAL_VECTOR)
      return false;
    break;
  default: // Don't allow other modes
    return false;
  }
  return true;
}

bool TPCTargetLowering::getIndexedAddressParts(SDNode *Op, SDValue &Base,
                                               SDValue &Offset,
                                               SelectionDAG &DAG) const {
  if (Op->getOpcode() != ISD::ADD)
    return false;

  Base = Op->getOperand(0);
  // All of the indexed addressing mode instructions take an immediate offset.
  if (ConstantSDNode *IncVal = dyn_cast<ConstantSDNode>(Op->getOperand(1))) {
    int64_t Inc = IncVal->getSExtValue();
    if (Inc != 1 && Inc != 2 && Inc != 4 && Inc != 8)
      return false;
    Offset = Op->getOperand(1);
    return true;
  }
  return false;
}

/// Returns true by value, base pointer and offset pointer and addressing mode
/// by reference if this node can be combined with a load / store to form a
/// post-indexed load / store.
bool TPCTargetLowering::getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                                   SDValue &Base,
                                                   SDValue &Offset,
                                                   ISD::MemIndexedMode &AM,
                                                   SelectionDAG &DAG) const {
  EVT VT;
  SDValue Ptr;
  unsigned AS;
  if (LoadSDNode *LD = dyn_cast<LoadSDNode>(N)) {
    VT = LD->getMemoryVT();
    Ptr = LD->getBasePtr();
    AS = LD->getAddressSpace();
  } else if (StoreSDNode *ST = dyn_cast<StoreSDNode>(N)) {
    VT = ST->getMemoryVT();
    Ptr = ST->getBasePtr();
    AS = ST->getAddressSpace();
  } else
    return false;

  if (AS != 3)
    return false;
  if (!getIndexedAddressParts(Op, Base, Offset, DAG))
    return false;
  // Post-indexing updates the base, so it's not a valid transform
  // if that's not the same as the load's pointer.
  if (Ptr != Base)
    return false;
  AM = ISD::POST_INC;
  return true;
}

bool TPCTargetLowering::canMergeStoresTo(unsigned AS, EVT MemVT,
                                         const SelectionDAG &DAG) const {
  // On TPC only stores into global address space can be merged.
  return AS == AddressSpace::GLOBAL && MemVT.getSizeInBits() <= 4;
}


static SDValue SelectCCHeart(SDValue Op0, SDValue Op1, SDValue Op2, SDValue Op3,
                             SDValue Op4, SelectionDAG &DAG, const SDLoc &DL) {
  EVT ccvt;
  EVT VT0 = Op0.getValueType();
  if (VT0.isVector()) {
    assert(0 && "Vector compare is unsupported (yet)");
  } else {
    ccvt = MVT::i1;
  }

  SDValue cond = DAG.getNode(ISD::SETCC, DL, MVT::i1, Op0, Op1, Op4);

  SDValue SecondMov = DAG.getNode(TPCISD::COND_MOV_INVERT, DL,
                                  Op3->getVTList(), Op3, cond, Op2);
  return SecondMov;
}

SDValue TPCTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Op0 = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  SDValue Op2 = Op.getOperand(2);
  SDValue Op3 = Op.getOperand(3);
  SDValue Op4 = Op.getOperand(4);
  EVT ccvt;
  EVT VT0 = Op0.getValueType();
  if (VT0.isVector()) {
    assert(0 && "Vector compare is unsupported (yet)");
  } else {
    ccvt = MVT::i1;
  }

  EVT OpndType = Op3.getValueType();
  unsigned multiplicity = OpndType.isVector() ? getMultiplicity(OpndType) : 0;
  if (multiplicity == 1 || multiplicity == 0 /*scalar*/) {
    return SelectCCHeart(Op0, Op1, Op2, Op3, Op4, DAG, DL);
  }
  if (multiplicity == 2) {
    EVT halfType = getHalfEVT(OpndType);

   assert(!Op0.getValueType().isVector() && !Op1.getValueType().isVector() &&
           "not for vector select");

    SDValue then0 =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfType, Op2);
    SDValue then1 =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfType, Op2);

    SDValue else0 =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfType, Op3);
    SDValue else1 =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfType, Op3);

    SDValue select0 = SelectCCHeart(Op0, Op1, then0, else0, Op4, DAG, DL);
    SDValue select1 = SelectCCHeart(Op0, Op1, then1, else1, Op4, DAG, DL);
    return createTuple({select0, select1}, DAG);
  }
  if (multiplicity == 4) {
    EVT quaType = getQuarterEVT(OpndType);

    assert(!Op0.getValueType().isVector() && !Op1.getValueType().isVector() &&
           "not for vector select");

    SDValue then0 =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, quaType, Op2);
    SDValue then1 =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, quaType, Op2);
    SDValue then2 =
        helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, quaType, Op2);
    SDValue then3 =
        helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, quaType, Op2);

    SDValue else0 =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, quaType, Op3);
    SDValue else1 =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, quaType, Op3);
    SDValue else2 =
        helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, quaType, Op3);
    SDValue else3 =
        helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, quaType, Op3);

    SDValue select0 = SelectCCHeart(Op0, Op1, then0, else0, Op4, DAG, DL);
    SDValue select1 = SelectCCHeart(Op0, Op1, then1, else1, Op4, DAG, DL);
    SDValue select2 = SelectCCHeart(Op0, Op1, then2, else2, Op4, DAG, DL);
    SDValue select3 = SelectCCHeart(Op0, Op1, then3, else3, Op4, DAG, DL);
    return createTuple({select0, select1, select2, select3}, DAG);
  }
  assert(false && "Never was before");
  return SDValue();
}

#if 0  //not needed for awhile
static unsigned getSElOpcode(SDValue Op) { 
  SDValue Cond = Op.getOperand(0);
  SDValue trd = Cond.getOperand(2);
  if (trd.getOpcode() == ISD::CONDCODE) {
    auto concod = cast<CondCodeSDNode>(trd)->get();
    switch (concod) {
    case ISD::SETEQ:
    case ISD::SETOEQ:
    case ISD::SETUEQ:
      return TPC::SEL_EQvvvvp;
    case ISD::SETNE:
    case ISD::SETONE:
    case ISD::SETUNE:
      return TPC::SEL_NEQvvvvp;

    case ISD::SETGT:
    case ISD::SETOGT:
    case ISD::SETUGT:
      return TPC::SEL_GRTvvvvp;
    case ISD::SETLT:
    case ISD::SETOLT:
    case ISD::SETULT:
      return TPC::SEL_LESSvvvvp;

    case ISD::SETGE:
    case ISD::SETOGE:
    case ISD::SETUGE:
      return TPC::SEL_GEQvvvvp;
    case ISD::SETLE:
    case ISD::SETOLE:
    case ISD::SETULE:
      return TPC::SEL_LEQvvvvp;
    default:
      return 0;
    }
  }
  return 0;
}
#endif 

static unsigned getCMPOpcode(SDValue Op, bool isImm, bool *isUnsigned) {
  SDValue trd = Op.getOperand(2);
  if (trd.getOpcode() == ISD::CONDCODE) {
    auto concod = cast<CondCodeSDNode>(trd)->get();
    switch (concod) { 
    case ISD::SETUEQ:
    case ISD::SETUNE:
    case ISD::SETUGT:
    case ISD::SETULT:
    case ISD::SETUGE:
    case ISD::SETULE:
      *isUnsigned = true;
      break;
    default:
      *isUnsigned = false;
    }
    switch (concod) {
    case ISD::SETEQ:
    case ISD::SETOEQ:
    case ISD::SETUEQ:
      return (isImm) ? TPC::CMP_EQvip : TPC::CMP_EQvvp;
    case ISD::SETNE:
    case ISD::SETONE:
    case ISD::SETUNE:
      return (isImm) ? TPC::CMP_NEQvip : TPC::CMP_NEQvvp;

    case ISD::SETGT:
    case ISD::SETOGT:
    case ISD::SETUGT:
      return (isImm) ? TPC::CMP_GRTvip : TPC::CMP_GRTvvp;
    case ISD::SETLT:
    case ISD::SETOLT:
    case ISD::SETULT:
      return (isImm) ? TPC::CMP_LESSvip : TPC::CMP_LESSvvp;

    case ISD::SETGE:
    case ISD::SETOGE:
    case ISD::SETUGE:
      return (isImm) ? TPC::CMP_GEQvip : TPC::CMP_GEQvvp;
    case ISD::SETLE:
    case ISD::SETOLE:
    case ISD::SETULE:
      return (isImm) ? TPC::CMP_LEQvip : TPC::CMP_LEQvvp;
    default:
      return 0;
    }
  }
  return 0;
}

static unsigned int getInstrDataType(EVT SrcEltVT, bool isUnsigned) {
  if (SrcEltVT == MVT::f32) {
    return T_FP32;
  } else if (SrcEltVT == MVT::bf16) {
    return T_BF16;
  } else if (SrcEltVT == MVT::f16) {
    return T_FP16;
  } else if (SrcEltVT == MVT::f8_143) {
    return T_FP8_143;
  } else if (SrcEltVT == MVT::f8_152) {
    return T_FP8_152;
  } else if (SrcEltVT == MVT::i32) {
    return (isUnsigned)? T_UINT32 : T_INT32;
  } else if (SrcEltVT == MVT::i16) {
    return (isUnsigned) ? T_UINT16 : T_INT16;
  } else if (SrcEltVT == MVT::i8) {
      return (isUnsigned) ? T_UINT8 : T_INT8;
  } else if (SrcEltVT == MVT::i1) {
      return T_INT1;
  }
  assert(false && "Unsupported source type");
  return (unsigned)-1;
}

static unsigned int getSwitchByDestDataType(EVT DestEltVT, bool isUnsigned) {
  if (DestEltVT == MVT::f32) {
    return TPCII::SW_TO_FP32;
  } else if (DestEltVT == MVT::bf16) {
    return TPCII::SW_TO_BF16;
  } else if (DestEltVT == MVT::f16) {
    return TPCII::SW_TO_FP16;
  } else if (DestEltVT == MVT::f8_143) {
    return TPCII::SW_TO_FP8_143;
  } else if (DestEltVT == MVT::f8_152) {
    return TPCII::SW_TO_FP8_152;
  } else if (DestEltVT == MVT::i32) {
    return (isUnsigned) ? TPCII::SW_TO_UINT32 : TPCII::SW_TO_INT32;
  } else if (DestEltVT == MVT::i16) {
    return (isUnsigned) ? TPCII::SW_TO_UINT16 : TPCII::SW_TO_INT16;
  } else if (DestEltVT == MVT::i8) {
    return (isUnsigned) ? TPCII::SW_TO_UINT8 : TPCII::SW_TO_INT8;
  }

  assert(false && "Unsupported destination type");
  return (unsigned)-1;
}


SDValue TPCTargetLowering::LowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  SDNode* Node = Op.getNode();
  for (SDNode::use_iterator U = Node->use_begin(),
                            UE=Node->use_end();
                             U != UE; ++U) {
    SDValue V = SDValue(*U, 0);
    if (V.getOpcode() == ISD::VSELECT) {
      // do not destroy pattern setcc + vselect
      return Op;
    }
  }
  EVT VT = Op.getOperand(0).getValueType();
  assert(VT.isVector());
  EVT EltVT = VT.getVectorElementType();
  SDLoc DL(Op);
  SDValue Element;
  unsigned ArgVal;
  bool isImm = false;
  bool isUnsigned = false;
  SDValue Op0 = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  if (isSplatBuildVector(Op1, Element)) {
    if (Element.getOpcode() == ISD::Constant) {
      ArgVal = cast<ConstantSDNode>(Element)->getZExtValue();
      isImm = true;
    }
  }
  unsigned opco = getCMPOpcode(Op,isImm, &isUnsigned);
  if (opco > 0) {
    unsigned DataTypeVal = getInstrDataType(EltVT, isUnsigned);
    unsigned swich = 0;
    SDValue m0 = Op0;
    SDValue m1 = Op1;
    if (isImm) {
      if (ArgVal == 0) {
        if (Op0.getOpcode() == ISD::AND) {
          swich |= TPCII::SW_MASK_EQ_ZERO;
          isImm = false;
          m1 = Op0.getOperand(1);
          m0 = Op0.getOperand(0);
          opco = getCMPOpcode(Op, isImm, &isUnsigned);
        }
      }
    }
    MachineSDNode *ExtraPred = DAG.getMachineNode(
        opco, DL, Op.getValueType(),
        {m0, /*Src1*/
         (isImm)?  DAG.getTargetConstant(ArgVal, DL, MVT::i32) : m1,
         DAG.getTargetConstant(DataTypeVal, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(swich, DL, MVT::i32),          /*Switch*/
         DAG.getUNDEF(MVT::v256i1),                       /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),              /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)});         /*Polarity*/
    SDValue evp = SDValue(ExtraPred, 0);
    return evp;
  }
  return Op;
}


SDValue TPCTargetLowering::LowerVSELECT(SDValue Op, SelectionDAG &DAG) const {
  SDValue Cond = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  SDValue Op2 = Op.getOperand(2);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  return MoveWithPred(Op1, VT, Op2, Cond, 0, DAG, DL);
}


SDValue TPCTargetLowering::LowerSELECT(SDValue Op, SelectionDAG &DAG) const {
  SDValue Cond = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  SDValue Op2 = Op.getOperand(2);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);

  // to recognize MIN/MAX idioms
  if (!VT.isVector() && (VT == MVT::i32 || VT == MVT::i16 || VT == MVT::i8)) {
    if (Cond.getOpcode() == ISD::SETCC) {
      if (Cond.getOperand(0) == Op1 && Cond.getOperand(1) == Op2) {
        SDValue trd = Cond.getOperand(2);
        if (trd.getOpcode() == ISD::CONDCODE) {
          auto concod = cast<CondCodeSDNode>(trd)->get();
          if (concod == ISD::SETGT || concod == ISD::SETUGT) {
            return Op;
          } else if (concod == ISD::SETLT || concod == ISD::SETULT) {
            return Op;
          }
        }
      }
    }
  }
 
  bool is_double_vect = false, is_quarter_vector = false;
  unsigned EltSize = 0;
  unsigned OpVectSize = 0;
  unsigned base_vector_size = 0;
  if (VT.isVector()) {
    EVT EltT = VT.getVectorElementType();
    EltSize = EltT.getStoreSizeInBits();
    EVT OpType = Op1.getValueType();
    OpVectSize = OpType.getVectorNumElements();
    is_double_vect = (EltSize == 32 && OpVectSize == 128) ||
                     (EltSize == 16 && OpVectSize == 256) ||
                     (EltSize == 8 && OpVectSize == 512);
    is_quarter_vector = (EltSize == 32 && OpVectSize == 256) ||
                        (EltSize == 16 && OpVectSize == 512);
    base_vector_size = 2048 / EltSize;
  }
  if (is_double_vect) {
    EVT EltT = VT.getVectorElementType();
    EVT SubregType =
        EVT::getVectorVT(*DAG.getContext(), EltT, base_vector_size);
    SDValue Subreg1Start = DAG.getConstant(0, DL, MVT::i32);
    SDValue Subreg2Start = DAG.getConstant(base_vector_size, DL, MVT::i32);
    SDValue Op1_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg1Start);
    SDValue Op1_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg2Start);
    SDValue Op2_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg1Start);
    SDValue Op2_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg2Start);

    EVT cvt = Cond.getValueType();
    SDValue cond1, cond2;
    if (cvt.isVector()) {
      assert(false && "vector cond is not implemented yet");
    } else {
      cond1 = cond2 = Cond;
    }

    SDValue half1 =
        DAG.getNode(ISD::SELECT, DL, SubregType, cond1, Op1_1, Op1_2);
    SDValue half2 =
        DAG.getNode(ISD::SELECT, DL, SubregType, cond2, Op2_1, Op2_2);

    SmallVector<SDValue, 2> SubVects;
    SubVects.push_back(LowerSELECT(half1, DAG));
    SubVects.push_back(LowerSELECT(half2, DAG));
    return createTuple(SubVects, DAG);
  } else if (is_quarter_vector) {
    EVT EltT = VT.getVectorElementType();
    EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, base_vector_size);
    SDValue Subreg1Start = DAG.getConstant(0, DL, MVT::i32);
    SDValue Subreg2Start = DAG.getConstant(base_vector_size, DL, MVT::i32);
    SDValue Subreg3Start =
        DAG.getConstant(base_vector_size + base_vector_size, DL, MVT::i32);
    SDValue Subreg4Start = DAG.getConstant(
        base_vector_size + base_vector_size + base_vector_size, DL, MVT::i32);

    SDValue Op1_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg1Start);
    SDValue Op1_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg2Start);
    SDValue Op1_3 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg3Start);
    SDValue Op1_4 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg4Start);

    SDValue Op2_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg1Start);
    SDValue Op2_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg2Start);
    SDValue Op2_3 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg3Start);
    SDValue Op2_4 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg4Start);

    EVT cvt = Cond.getValueType();
    SDValue cond1, cond2,cond3,cond4;
    if (cvt.isVector()) {
      assert(false && "vector cond is not implemented yet");
    } else {
      cond1 = cond2 = cond3 = cond4 = Cond;
    }

    SDValue q1 =
        DAG.getNode(ISD::SELECT, DL, SubregType, cond1, Op1_1, Op2_1);
    SDValue q2 =
        DAG.getNode(ISD::SELECT, DL, SubregType, cond2, Op1_2, Op2_2);

    SDValue q3 = DAG.getNode(ISD::SELECT, DL, SubregType, cond2, Op1_3, Op2_3);
    SDValue q4 = DAG.getNode(ISD::SELECT, DL, SubregType, cond3, Op1_4, Op2_4);

    SmallVector<SDValue, 4> SubVects;
    SubVects.push_back(LowerSELECT(q1, DAG));
    SubVects.push_back(LowerSELECT(q2, DAG));
    SubVects.push_back(LowerSELECT(q3, DAG));
    SubVects.push_back(LowerSELECT(q4, DAG));
    return createTuple(SubVects, DAG);
  }
  // in remain usual vectors and scalars
  SDValue SecondMov = DAG.getNode(TPCISD::COND_MOV_INVERT, SDLoc(Op),
                                    Op1->getVTList(), Op2, Cond, Op1);
  return SecondMov;  
}

SDValue TPCTargetLowering::lowerEXTRACT_SUBVECTOR(SDValue Op,
                                                  SelectionDAG &DAG) const {
  EVT SubRegTy = Op.getValueType();
  assert(SubRegTy.isVector());
  EVT EltTy = SubRegTy.getVectorElementType();
  SDValue WideReg = Op.getOperand(0);
  EVT WideRegTy = WideReg.getValueType();
  SDLoc DL(Op);
  unsigned EltSz = EltTy.getScalarSizeInBits();
  assert(EltSz > 0);
  unsigned NumElements = WideRegTy.getVectorNumElements();
  unsigned SubVectorSize = VRF_REGISTER_LENGTH_IN_BITS / EltSz;
  assert(VRF_REGISTER_LENGTH_IN_BITS % EltSz == 0);
  unsigned NumSubVectors = NumElements / SubVectorSize;
  assert(NumElements % SubVectorSize == 0);
  auto Cst = cast<ConstantSDNode>(Op.getOperand(1));
  unsigned Offset = Cst->getZExtValue();
  unsigned vne = SubRegTy.getVectorNumElements(); 
  if (vne != SubVectorSize) {
    // other way, need extract and unite
    unsigned SubRegIndex0, SubRegIndex1;
    if (vne == 128) {
      if (Offset == 0) {
        SubRegIndex0 = TPC::sub_0;        
        SubRegIndex1 = TPC::sub_1;
      } else {
        assert(Offset == 128);
        SubRegIndex0 = TPC::sub_2;
        SubRegIndex1 = TPC::sub_3;
      }
      EVT SubType = getHalfEVT(SubRegTy);      
      SDValue SubReg0 = DAG.getTargetConstant(SubRegIndex0, DL, MVT::i32);
      MachineSDNode *Node0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                               SubType, WideReg, SubReg0);
      SDValue V0 = SDValue(Node0, 0);
      SDValue SubReg1 = DAG.getTargetConstant(SubRegIndex1, DL, MVT::i32);
      MachineSDNode *Node1 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                               SubType, WideReg, SubReg1);
      SDValue V1 = SDValue(Node1, 0);
      return createTuple({V0,V1}, DAG);
    }
  }
  assert(vne == SubVectorSize);
  assert(WideRegTy.getVectorNumElements() == NumElements);
  assert(SubRegTy.getVectorElementType() == WideRegTy.getVectorElementType());

  assert(Offset < NumElements);
  assert(Offset % SubVectorSize == 0);

  unsigned SubRegIndex;
  if (NumSubVectors == 4) {
    assert(SubVectorSize == 64);
    switch (Offset) {
    case 0:
      SubRegIndex = TPC::sub_0;
      break;
    case 64:
      SubRegIndex = TPC::sub_1;
      break;
    case 128:
      SubRegIndex = TPC::sub_2;
      break;
    case 192:
      SubRegIndex = TPC::sub_3;
      break;
    default:
      llvm_unreachable("Invalid subregister of ARF");
    }
  } else if (NumSubVectors == 2) {
    if (Offset == 0)
      SubRegIndex = TPC::sub_0;
    else if (Offset == SubVectorSize)
      SubRegIndex = TPC::sub_1;
    else
      llvm_unreachable("Invalid subregister of DRF");
  } else {
    llvm_unreachable("Invalid register multiplicity");
  }

  SDValue SubReg = DAG.getTargetConstant(SubRegIndex, DL, MVT::i32);
  MachineSDNode *Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                           SubRegTy, WideReg, SubReg);
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::lowerINSERT_SUBVECTOR(SDValue Op,
                                                 SelectionDAG &DAG) const {
  EVT ResTy = Op.getValueType();
  assert(ResTy.isVector());
  EVT EltTy = ResTy.getVectorElementType();
  SDLoc DL(Op);

  SDValue WideReg = Op.getOperand(0);
  SDValue ShortReg = Op.getOperand(1);
  EVT SubTy = ShortReg.getValueType();
  (void)SubTy;
  assert(SubTy.isVector());

  unsigned EltSz = EltTy.getScalarSizeInBits();
  assert(EltSz > 0);
  unsigned NumElements = ResTy.getVectorNumElements();
  unsigned SubVectorSize = VRF_REGISTER_LENGTH_IN_BITS / EltSz;
  assert(VRF_REGISTER_LENGTH_IN_BITS % EltSz == 0);
  unsigned NumSubVectors = NumElements / SubVectorSize;
  assert(NumElements % SubVectorSize == 0);
  assert(SubTy.getVectorNumElements() == SubVectorSize);

  ConstantSDNode *Start = cast<ConstantSDNode>(Op.getOperand(2));
  unsigned Offset = Start->getZExtValue();
  assert(Offset < NumElements);
  assert(Offset % SubVectorSize == 0);
  unsigned RegNo = Offset / SubVectorSize;

  int SubRegIndex;
  if (NumSubVectors == 4) {
    switch (RegNo) {
    case 0:
      SubRegIndex = TPC::sub_0;
      break;
    case 1:
      SubRegIndex = TPC::sub_1;
      break;
    case 2:
      SubRegIndex = TPC::sub_2;
      break;
    case 3:
      SubRegIndex = TPC::sub_3;
      break;
    default:
      llvm_unreachable("Wrong subindex");
    }
  } else if (NumSubVectors == 2) {
    switch (RegNo) {
    case 0:
      SubRegIndex = TPC::sub_0;
      break;
    case 1:
      SubRegIndex = TPC::sub_1;
      break;
    default:
      llvm_unreachable("Wrong subindex");
    }
  } else {
    llvm_unreachable("Wrong subvector size");
  }

  SDValue SubReg = DAG.getTargetConstant(SubRegIndex, DL, MVT::i32);
  MachineSDNode *Node =
      DAG.getMachineNode(TargetOpcode::INSERT_SUBREG, DL, ResTy.getSimpleVT(),
                         WideReg, ShortReg, SubReg);
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::lowerBUILD_VECTOR(SDValue Op,
                                             SelectionDAG &DAG) const {
  BuildVectorSDNode *Node = cast<BuildVectorSDNode>(Op);
  SDLoc DL(Op);
  EVT ResTy = Op->getValueType(0);
  assert(ResTy.isVector());
  EVT EltTy = ResTy.getVectorElementType();
  unsigned NumElements = ResTy.getVectorNumElements();
  unsigned EltSz = EltTy.getScalarSizeInBits();
  assert(EltSz > 0);

  if (NumElements == 2) {
    assert(EltTy.isScalarInteger());
    assert(EltSz == 8 || EltSz == 16 || EltSz == 32);
    SDValue V1 = Node->getOperand(0);
    SDValue V2 = Node->getOperand(1);
    return createTuple({V1, V2}, DAG);
  }

  unsigned SubVectorSize = VRF_REGISTER_LENGTH_IN_BITS / EltSz;
  assert(VRF_REGISTER_LENGTH_IN_BITS % EltSz == 0);
  unsigned NumSubVectors = NumElements / SubVectorSize;
  assert(NumElements % SubVectorSize == 0);
  EVT SubVectorTy = EVT::getVectorVT(*DAG.getContext(), EltTy, SubVectorSize);
  assert(NumElements == Node->getNumOperands());

  // We provide specific lowering only for vectors of 64, 128, 256 or 512
  // elements.
  switch (NumElements) {
  case 64:
  case 128:
  case 256:
  case 512:
    break;
  default:
    llvm_unreachable("Unsupported vector size");
  }
  if (NumSubVectors == 1) { // Need to consider special constant vector
    if (EltSz == 8 && NumElements == 256) {
      ConstantSDNode *e0, *e1, *e2, *e3;
      e0 = dyn_cast<ConstantSDNode>(Node->getOperand(0));
      e1 = dyn_cast<ConstantSDNode>(Node->getOperand(1));
      e2 = dyn_cast<ConstantSDNode>(Node->getOperand(2));
      e3 = dyn_cast<ConstantSDNode>(Node->getOperand(3));
      if (e0 && e1 && e2 && e3 &&
          !(e0==e1 && e1==e2 && e2==e3 && e3==e0)) {
        //check if vector is periodic
        for (unsigned int i = 4; i < NumElements; i+=4) {
          if (Node->getOperand(i+0) == Node->getOperand(0) &&
              Node->getOperand(i+1) == Node->getOperand(1) &&
              Node->getOperand(i+2) == Node->getOperand(2) &&
              Node->getOperand(i+3) == Node->getOperand(3)) {
            continue;
          } else {
            return Op; // not our case
          }
        }
        unsigned u0 = e0->getZExtValue();
        unsigned u1 = e1->getZExtValue();
        unsigned u2 = e2->getZExtValue();
        unsigned u3 = e3->getZExtValue();
        u0 <<= 0;
        u1 <<= 8;
        u2 <<= 16;
        u3 <<= 24;
        unsigned comval = u0 | u1 | u2 | u3;
        SDValue sdv = DAG.getTargetConstant(comval, DL, MVT::i32);
        SmallVector<SDValue, 64> Ops;
        for (int i = 0; i < 64; i++) {
          Ops.push_back(sdv);
        }
        EVT VectorTy =
            EVT::getVectorVT(*DAG.getContext(), MVT::i32, 64);
        SDValue const64vector = DAG.getBuildVector(VectorTy, DL, Ops);
        return DAG.getNode(ISD::BITCAST, DL, ResTy, const64vector);
      }
    }
    return Op;
  }
  // Replace BUILD_VECTOR with concatenation of v64i32/v64f32 elements.
  SmallVector<SDValue, 4> Sections;
  for (unsigned I = 0; I < NumSubVectors; ++I) {
    SmallVector<SDValue, 256> Args;
    bool HasUndef = false;
    bool HasDefined = false;
    for (unsigned SubN = 0; SubN < SubVectorSize; ++SubN) {
      SDValue Item = Node->getOperand(SubVectorSize * I + SubN);
      if (Item.isUndef())
        HasUndef = true;
      else
        HasDefined = true;
      Args.push_back(Item);
    }
    if (HasUndef && !HasDefined) {
      Sections.push_back(DAG.getUNDEF(SubVectorTy.getSimpleVT()));
    } else {
      SDValue Item = DAG.getBuildVector(SubVectorTy.getSimpleVT(), DL, Args);
      Sections.push_back(Item);
    }
  }
  return createTuple(Sections, DAG);
}

static bool isSequentialMask(const ArrayRef<int> &Mask) {
  for (unsigned I = 1; I < Mask.size(); ++I)
    if (Mask[I] != Mask[I - 1] + 1)
      return false;
  return true;
}

static bool isUndefinedMask(const ArrayRef<int> &Mask) {
  for (unsigned I = 1; I < Mask.size(); ++I)
    if (Mask[I] != -1)
      return false;
  return true;
}

// Returns true if all elements of the mask are zeroes.
static bool isZeroMask(const ArrayRef<int> &Mask) {
  for (unsigned I = 1; I < Mask.size(); ++I)
    if (Mask[I] != 0)
      return false;
  return true;
}

static unsigned getFullVectorSize(EVT EltTy) {
  if (EltTy.getSizeInBits() == 8 || EltTy.getSizeInBits() == 1)
    return 256;
  if (EltTy.getSizeInBits() == 16)
    return 128;
  if (EltTy.getSizeInBits() == 32)
    return 64;
  return 0;
}

static ArrayRef<int> getHalfMask(const ArrayRef<int> &Mask, size_t HalfNo) {
  assert(Mask.size() % 2 == 0);
  assert(HalfNo <= 1);
  int HalfSize = Mask.size() / 2;
  return HalfNo == 0 ? Mask.drop_back(HalfSize) : Mask.drop_front(HalfSize);
}

// Checks if the given a shuffle_vector mask implements a "set subregister"
// operation. For instance, shuffle_vector operation with mask (0, 5, 6, 7)
// applied to two 4-element vectors produces vector that can be considered as
// a result of operations:
//
//      subreg = get_subreg(op0, 0)
//      set_subreg(op1, 0, subreg)
//
// If it is so, the function returns true, 'ArgNo' in this case contains the
// argument that provides subregister (0 or 1) and 'RegNo' is a number of
// subregister inside the other argument.
//
static bool isSetSubregisterMask(const ArrayRef<int> &Mask, unsigned &ArgNo,
                                 unsigned &RegNo) {
  // Process only masks with 128 or 256 elements. Vectors of 64 elements do
  // not contain subregisters.
  if (Mask.size() % 64 || Mask.size() < 128)
    return false;

  // Number of subregisters of 64 elements contained in the result formed by
  // this mask.
  unsigned NumSubregs = Mask.size() / 64;

  // We assume that we have no more than 4 subregisters (in the case of int256).
  // It determineds size of 'SubRegisterSource' (see below).
  assert(NumSubregs <= 4);

  // For each subregister in the result this array keeps the subregister number
  // in the source operand where it comes from. Subregister numbers of the
  // second operand are shifted by NumSubregs.
  SmallVector<unsigned, 8> SubRegisterSource(8, ~0U);

  for (unsigned R = 0; R < Mask.size(); R += 64) {
    const ArrayRef<int> SubMask = Mask.slice(R, 64);
    if (!isSequentialMask(SubMask)) {
      if (isUndefinedMask(SubMask))
        continue;
      return false;
    }
    if (SubMask.front() % 64)
      return false;
    SubRegisterSource[R / 64] = SubMask.front() / 64;
  }

  // Subregister numbers must form continuous sequence starting from 0 provided
  // that subregister numbers of the second argument are corrected.
  unsigned FromFirst = 0, // Number of subregisters from the first argument
      FromSecond = 0;     // Number of subregisters from the second argument
  for (unsigned I = 0; I < NumSubregs; ++I) {
    unsigned SubregNo = SubRegisterSource[I];
    // All subregisters in the result must be defined.
    if (SubregNo == ~0U)
      continue;
    if (SubregNo < NumSubregs) {
      ++FromFirst;
      if (SubregNo != I)
        return false;
    } else {
      ++FromSecond;
      if (SubregNo - NumSubregs != I)
        return false;
    }
  }

  // Both arguments must be used.
  if (FromFirst == 0 || FromSecond == 0)
    return false;

  if (FromFirst <= FromSecond) {
    // Subregister is extracted from the first argument and is inserted to the
    // second.
    ArgNo = 0;
    for (unsigned I = 0; I < NumSubregs; ++I)
      if (SubRegisterSource[I] < NumSubregs) {
        RegNo = I;
        break;
      }
  } else {
    // Subregister is extracted from the second argument and is inserted to the
    // first.
    ArgNo = 1;
    for (unsigned I = 0; I < NumSubregs; ++I)
      if (SubRegisterSource[I] >= NumSubregs) {
        RegNo = I;
        break;
      }
  }
  return true;
}

// Returns true if the argument is BUILD_VECTOR, in which all operands but the
// first are undefs.
static bool isBuildVectorWithUndefs(SDValue V) {
  if (V.getOpcode() != ISD::BUILD_VECTOR)
    return false;
  for (unsigned I = 1, E = V.getNumOperands(); I != E; ++I)
    if (V.getOperand(I).getOpcode() != ISD::UNDEF)
      return false;
  return true;
}

// Returns true if the argument is BUILD_VECTOR, in which all operands are
// identical.
static bool isSplat(SDValue V) {
  if (V.getOpcode() != ISD::BUILD_VECTOR)
    return false;
  SDValue First = V.getOperand(0);
  for (unsigned I = 1, E = V.getNumOperands(); I != E; ++I)
    if (V.getOperand(I) != First)
      return false;
  return true;
}

static SDValue getSubRegisterValue(SDValue SubRegSource, EVT ResTy,
                                   unsigned Start, SDLoc DL,
                                   SelectionDAG &DAG) {
  if (ResTy.getSizeInBits() != 64 * 32)
    return SDValue();

  EVT EltT = ResTy.getVectorElementType();
  unsigned EltSize = EltT.getStoreSizeInBits();
  assert(EltSize >= 8);
  unsigned Factor = 32 / EltSize;
  unsigned SubRegSize = 64 * Factor;
  assert(Start % SubRegSize == 0);

  unsigned SubRegNo = Start / SubRegSize;

  if (SubRegSource.getNode()->getOpcode() == ISD::CONCAT_VECTORS)
    return SubRegSource.getOperand(SubRegNo);

  if (auto BVN = dyn_cast<BuildVectorSDNode>(SubRegSource)) {
    SmallVector<SDValue, 256> Args;
    for (unsigned I = SubRegNo * 64, E = 64 * (SubRegNo + 1); I != E; ++I)
      Args.push_back(BVN->getOperand(I));
    return DAG.getNode(ISD::BUILD_VECTOR, DL, ResTy, Args);
  }

  if (auto *SVN = dyn_cast<ShuffleVectorSDNode>(SubRegSource)) {
    ArrayRef<int> ShuffleMask = SVN->getMask();
    if ((ShuffleMask[Start] % 64 == 0) &&
        isSequentialMask(ShuffleMask.slice(Start, 64))) {
      unsigned NewStart = ShuffleMask[Start];
      unsigned ArgNo = NewStart >= SVN->getValueType(0).getVectorNumElements();
      if (ArgNo)
        NewStart -= SVN->getValueType(0).getVectorNumElements();
      return getSubRegisterValue(SVN->getOperand(ArgNo), ResTy, NewStart, DL,
                                 DAG);
    }
  }

  if (SubRegSource.getOpcode() == ISD::CopyFromReg) {
    SDValue SubVectorStart = DAG.getConstant(APInt(32, Start), DL, MVT::i32);
    return DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, ResTy, SubRegSource,
                       SubVectorStart);
  }

  return SDValue();
}

//
// If the shuffle mask is taking exactly one element from the first vector
// operand and passing through all other elements from the second vector
// operand, return the index of the mask element that is choosing an element
// from the first operand. Otherwise, return -1.
//
static int getShuffleMaskIndexOfOneElementFromOp0IntoOp1(ArrayRef<int> Mask) {
  int MaskSize = Mask.size();
  int EltFromOp0 = -1;
  for (int i = 0; i != MaskSize; ++i) {
    if (Mask[i] >= 0 && Mask[i] < MaskSize) {
      // We're looking for a shuffle of exactly one element from operand 0.
      if (EltFromOp0 != -1)
        return -1;
      EltFromOp0 = i;
    } else if (Mask[i] != i + MaskSize) {
      // Nothing from operand 1 can change lanes.
      return -1;
    }
  }
  return EltFromOp0;
}

//
// If a shuffle inserts exactly one element from a source vector operand into
// another vector operand, then we can eliminate the shuffle by replacing it
// with extract/insert pair.
//
static SDValue replaceShuffleWithInsert(ShuffleVectorSDNode *Shuf,
                                        SelectionDAG &DAG) {
  // First, check if we are taking one element of a vector and shuffling that
  // element into another vector.
  ArrayRef<int> Mask = Shuf->getMask();
  SmallVector<int, 16> CommutedMask(Mask.begin(), Mask.end());
  SDValue Op0 = Shuf->getOperand(0);
  SDValue Op1 = Shuf->getOperand(1);
  int ShufOp0Index = getShuffleMaskIndexOfOneElementFromOp0IntoOp1(Mask);
  if (ShufOp0Index == -1) {
    // Commute mask and check again.
    ShuffleVectorSDNode::commuteMask(CommutedMask);
    ShufOp0Index = getShuffleMaskIndexOfOneElementFromOp0IntoOp1(CommutedMask);
    if (ShufOp0Index == -1)
      return SDValue();
    // Commute operands to match the commuted shuffle mask.
    std::swap(Op0, Op1);
    Mask = CommutedMask;
  }

  EVT VT = Op0.getValueType();
  EVT EltT = VT.getVectorElementType();

  SDValue ExtrElt =
      DAG.getNode(ISD::EXTRACT_VECTOR_ELT, SDLoc(Shuf), EltT, Op0,
                  DAG.getConstant(Mask[ShufOp0Index], SDLoc(Shuf), MVT::i32));

  SDValue Result =
      DAG.getNode(ISD::INSERT_VECTOR_ELT, SDLoc(Shuf), VT, Op1, ExtrElt,
                  DAG.getConstant(ShufOp0Index, SDLoc(Shuf), MVT::i32));

  LLVM_DEBUG(dbgs() << "Replace:\n"; dbgs() << "    "; Shuf->dump(&DAG);
             dbgs() << "With:\n"; dbgs() << "    "; ExtrElt.dump(&DAG);
             dbgs() << "    "; Result.dump(&DAG););

  return Result;
}

static bool is_all_eq(const int *ar, int n, int el) {
  for (int i = 0; i < n; i++) {
    if (ar[i] != 0) {
      return false;
    }
  }
  return true;
}



static bool is_lin(const int *ar, int n) {
  for (int i = 1; i < n; i++) {
    if ((ar[i] - ar[i - 1]) != 1) {
      return false;
    }
  }
  return true;
}

static bool is_lin64(const int *ar) { return is_lin(ar, 64); }

static bool is_lin128(const int *ar) { return is_lin(ar, 128); }

static bool is_lin256(const int *ar) { return is_lin(ar, 256); }
#if 0
static SDValue helperConvertIntNodeCreate(const SDValue &SDNodeArg, const SDLoc &DL,
                                          SelectionDAG &DAG,
                                          unsigned switchType, EVT ResultVT,
                                          unsigned ConvertType,
                                          SDValue *SDNodeArg2 = nullptr,
                                          SDValue *Shift = nullptr
) {
  SmallVector<SDValue, 8> Ops(6);
  Ops[0] = SDNodeArg; // Source.
  Ops[1] = (Shift)? *Shift : DAG.getTargetConstant(0, DL, MVT::i8);
  Ops[2] = DAG.getTargetConstant(switchType, DL, MVT::i32); // Switch.
  Ops[3] =
      (SDNodeArg2 == nullptr) ? DAG.getUNDEF(ResultVT) : *SDNodeArg2; // Income.
  Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(ConvertType, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}
#endif
static SDValue helperConvertvvpNodeCreate(const SDValue &SDNodeArg, const SDLoc &DL,
                                   SelectionDAG &DAG, unsigned opType,
                                   unsigned switchType, EVT ResultVT) {
  SmallVector<SDValue, 8> Ops(6);
  Ops[0] = SDNodeArg;                                       // Source.
  Ops[1] = DAG.getTargetConstant(opType, DL, MVT::i8);      // DataType.
  Ops[2] = DAG.getTargetConstant(switchType, DL, MVT::i32); // Switch.
  Ops[3] = DAG.getUNDEF(ResultVT);                          // Income.
  Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);              // Predicate.
  Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1);           // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}

static SDValue helperANDvipNodeCreate(const SDValue &SDNodeArg, const SDLoc &DL,
                               SelectionDAG &DAG, unsigned opType,
                               unsigned mask, EVT ResultVT) {
  SmallVector<SDValue, 8> Ops(7);
  Ops[0] = SDNodeArg;                                  // Source.
  Ops[1] = DAG.getTargetConstant(mask, DL, MVT::i32);  // Switch.
  Ops[2] = DAG.getTargetConstant(opType, DL, MVT::i8); // DataType.
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);     // Switch.
  Ops[4] = DAG.getUNDEF(ResultVT);                     // Income.
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);         // Predicate.
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);      // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(TPC::ANDvip, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}

static SDValue helperExtractSubRegSDValue(unsigned subReg, const SDLoc &DL,
                                   SelectionDAG &DAG, EVT elemType,
                                   SDValue reg) {
  SDValue NIndex = DAG.getTargetConstant(subReg, DL, MVT::i32);
  SDNode *N = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, elemType,
                                 reg, NIndex);
  return SDValue(N, 0);
}

// Returns true if the specified node is a binary operation, which can be
// handles by pseudo-scalar folding of shuffle_vector.
static bool isBinaryVectorOperation(SDValue V) {
  switch (V.getOpcode()) {
  case ISD::ADD:
  case ISD::SUB:
  case ISD::FADD:
  case ISD::FSUB:
  case ISD::FMUL:
  case ISD::FDIV:
  case ISD::FREM:
    return true;
  default:
    return false;
  }
}

// Returns true if the specified node roots an expression where all operands
// recursively descent to either splat vectors of build_vector nodes where all
// operands but the first are undefs.
static bool isPseudoScalar(SDValue V) {
  if (isBuildVectorWithUndefs(V) || isSplat(V))
    return true;
  if (isBinaryVectorOperation(V)) {
    SDValue Op1 = V.getOperand(0);
    SDValue Op2 = V.getOperand(1);
    return isPseudoScalar(Op1) && isPseudoScalar(Op2);
  }
  return false;
}

// Service class that implements recursive transformation of pseudo-scalar
// expression to the equivalent form supported by the instruction selector.
class Expander {
  SelectionDAG &DAG;
public:
  Expander(SelectionDAG &DAG) : DAG(DAG) {}

  SDValue expand(SDValue V) {
    SDLoc DL(V);
    EVT VT = V.getValueType();
    unsigned VectWidth = VT.getVectorNumElements();

    if (V.getOpcode() == ISD::BUILD_VECTOR) {
      if (isSplat(V))
        return V;
      if (!isBuildVectorWithUndefs(V))
        return SDValue();
      SmallVector<SDValue, 64> Args;
      Args.append(VectWidth, V.getOperand(0));
      return DAG.getNode(ISD::BUILD_VECTOR, DL, VT, Args);
    }

    if (isBinaryVectorOperation(V)) {
      SDValue Op1 = V.getOperand(0);
      SDValue Op2 = V.getOperand(1);
      SDValue NewOp1 = expand(Op1);
      if (!NewOp1)
        return SDValue();
      SDValue NewOp2 = expand(Op2);
      if (!NewOp2)
        return SDValue();
      return DAG.getNode(V.getOpcode(), DL, VT, NewOp1, NewOp2);
    }

    return SDValue();
  }
};


static SDValue reachBuildVector(SDValue oprnd,
                                SmallVector<SDValue, 4> & ChainUn) {
  if (oprnd.getOpcode() == ISD::BUILD_VECTOR) {
    return oprnd;
  }
  if (oprnd.getNumOperands() == 1) {
    ChainUn.push_back(oprnd);
    return reachBuildVector(oprnd.getOperand(0), ChainUn);
  }
  return SDValue();
}

SDValue reachScalar(SDValue bv, int iqel, SmallVector<SDValue, 4> &ChainUn,
                    SelectionDAG &DAG) {
  SDLoc DL(bv);
  SDValue s0 = bv.getOperand(iqel);
  while (!ChainUn.empty()) {
    SDValue tran = ChainUn.back();
    EVT tranType = tran.getValueType();
    EVT tranEType = tranType.getVectorElementType();
    // SDValue TranArg = tran.getOperand(0);
    // EVT tranArgType = TranArg.getValueType();
    // EVT tranArgEType = tranArgType.getVectorElementType();
    s0 = DAG.getNode(tran.getOpcode(), DL, tranEType, s0);
    ChainUn.pop_back();
  }
  return s0;
}



SDValue TPCTargetLowering::lowerVECTOR_SHUFFLE(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op.getValueType();
  EVT EltT = VT.getVectorElementType();
  unsigned EltSize = EltT.getStoreSizeInBits();
  unsigned VectWidth = VT.getVectorNumElements();

  ShuffleVectorSDNode *SVN = cast<ShuffleVectorSDNode>(Op.getNode());
  ArrayRef<int> ShuffleMask = SVN->getMask();

  SDValue V1 = Op.getOperand(0);
  SDValue V2 = Op.getOperand(1);
  EVT OpType = V1.getValueType();
  unsigned OpVectSize = OpType.getVectorNumElements();
  // Recognize trivial operations.
  if (isSequentialMask(ShuffleMask)) {
    if (static_cast<unsigned>(ShuffleMask.back()) <= OpVectSize) {
      // Select subvector of the first operand.
      if (ShuffleMask.size() == OpVectSize)
        // Entire vector is selected.
        return V1;
    }
  }

  // For IRF vectors, try to eliminate the shuffle by replacing it with an
  // extract/insert pair
  if (OpVectSize == 5) { // v5i32 - IRF
    if (SDValue InsElt = replaceShuffleWithInsert(SVN, DAG)) {
      return InsElt;
    }
    return SDValue();
  }

  // Recognize set_subreg operation.
  unsigned ArgNo;
  unsigned RegNo;
  if (ShuffleMask.size() < 256 &&
      isSetSubregisterMask(ShuffleMask, ArgNo, RegNo)) {
    SDValue SubRegSource = (ArgNo == 0) ? V1 : V2;
    SDValue OtherRegs = (ArgNo == 0) ? V2 : V1;
    unsigned Factor = 32 / EltSize;
    unsigned SubRegSize = 64 * Factor;
    EVT SubRegType = EVT::getVectorVT(*DAG.getContext(), EltT, SubRegSize);
    SDValue SubRegValue = getSubRegisterValue(SubRegSource, SubRegType,
                                              64 * Factor * RegNo, DL, DAG);
    if (SubRegValue) {
      SDValue SubVectorStart =
          DAG.getConstant(APInt(32, RegNo * 64), DL, MVT::i32);
      return DAG.getNode(ISD::INSERT_SUBVECTOR, DL, VT, OtherRegs, SubRegValue,
                         SubVectorStart);
    }
  }

  // Recognize subvector concatenation.
  //
  // If concatenation is made of subregisters of long vectors, such as v128i32,
  // the result can be represented as a vector shuffle.
  //
  if (ShuffleMask.size() == 128 && EltSize == 32 && OpVectSize == 128) {
    if (isSequentialMask(ShuffleMask.take_front(64)) &&
        isSequentialMask(ShuffleMask.take_back(64))) {
      EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, 64);
      unsigned SubregNdx1 = ShuffleMask[0];
      unsigned SubregNdx2 = ShuffleMask[64];
      bool FirsfFromFirst = SubregNdx1 < 128;
      bool SecondFromSecond = SubregNdx2 >= 128;

      if (FirsfFromFirst == SecondFromSecond) {
        if (FirsfFromFirst) {
          assert(SubregNdx1 == 0 || SubregNdx1 == 64);
          assert(SubregNdx2 == 128 || SubregNdx2 == 192);
        } else {
          assert(SubregNdx1 == 128 || SubregNdx1 == 192);
          assert(SubregNdx2 == 0 || SubregNdx2 == 64);
        }
        SDValue LongReg1 = FirsfFromFirst ? V1 : V2;
        SDValue LongReg2 = SecondFromSecond ? V2 : V1;
        if (FirsfFromFirst)
          SubregNdx2 -= 128;
        else
          SubregNdx1 -= 128;
        SDValue Subreg1Start = DAG.getConstant(SubregNdx1, DL, MVT::i32);
        SDValue Subreg2Start = DAG.getConstant(SubregNdx2, DL, MVT::i32);
        SDValue Op1 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType,
                                  LongReg1, Subreg1Start);
        SDValue Op2 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType,
                                  LongReg2, Subreg2Start);
        SmallVector<SDValue, 2> SubVects;
        SubVects.push_back(Op1);
        SubVects.push_back(Op2);
        return createTuple(SubVects, DAG);
      }
    }
  }

  // Recognize vector operations when all arguments are splats. InstCombiner
  // generates such constructs:
  //
  //   vector_shuffle<0,0, ...>
  //   +- vector_operation
  //   |  +- BUILD_VECTOR a, undef, ...
  //   |  +- BUILD_VECTOR b, undef, ...
  //   +- undef
  //
  // We must generate DAG like this:
  //
  //   vector_operation
  //   +- BUILD_VECTOR a, a, ...
  //   +- BUILD_VECTOR b, b, ...
  //
  // Note that there can be several binary operations, like:
  //
  //   vector_shuffle<0,0, ...>
  //   +- fsub
  //   |  + fadd
  //   |  | +- BUILD_VECTOR a, undef, ...
  //   |  | +- BUILD_VECTOR b, undef, ...
  //   |  +- BUILD_VECTOR c, undef, ...
  //   +- undef

  if (isZeroMask(ShuffleMask) && ShuffleMask.size() == VectWidth &&
      V2.getOpcode() == ISD::UNDEF && isPseudoScalar(V1))
    return Expander(DAG).expand(V1);

  if (ShuffleMask.size() == 512 && EltSize == 8 && OpVectSize == 512) {
    int SubregNdx1 = ShuffleMask[0];
    int SubregNdx2 = ShuffleMask[256];
    if ((is_lin256(&ShuffleMask[0]) || SubregNdx1 < 0) &&
        (is_lin256(&ShuffleMask[256]) || SubregNdx2 < 0)) {
      int v1, v2;
      SDValue o1, o2;
      SDValue Op1, Op2;

      EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, 256);
      if (SubregNdx1 >= 0) {
        v1 = SubregNdx1 / 256;
        if (v1 < 2) {
          o1 = V1;
        } else {
          o1 = V2;
          v1 -= 4;
          SubregNdx1 -= 512;
        }
        SDValue Subreg1Start = DAG.getConstant(SubregNdx1, DL, MVT::i32);
        Op1 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o1,
                          Subreg1Start);
      } else {
        Op1 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      if (SubregNdx2 >= 0) {
        v2 = SubregNdx2 / 256;
        if (v2 < 2) {
          o2 = V1;
        } else {
          o2 = V2;
          v2 -= 4;
          SubregNdx2 -= 512;
        }
        SDValue Subreg2Start = DAG.getConstant(SubregNdx2, DL, MVT::i32);
        Op2 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o2,
                          Subreg2Start);
      } else {
        Op2 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      SmallVector<SDValue, 2> SubVects;
      SubVects.push_back(Op1);
      SubVects.push_back(Op2);
      return createTuple(SubVects, DAG);
    }
  }
  if (ShuffleMask.size() == 256 && EltSize == 32 && OpVectSize == 256) {

    int SubregNdx1 = ShuffleMask[0];
    int SubregNdx2 = ShuffleMask[64];
    int SubregNdx3 = ShuffleMask[128];
    int SubregNdx4 = ShuffleMask[192];

    if ((is_lin64(&ShuffleMask[0]) || SubregNdx1 < 0) &&
        (is_lin64(&ShuffleMask[64]) || SubregNdx2 < 0) &&
        (is_lin64(&ShuffleMask[128]) || SubregNdx3 < 0) &&
        (is_lin64(&ShuffleMask[192]) || SubregNdx4 < 0)) {

      int v1, v2, v3, v4;
      SDValue o1, o2, o3, o4;
      SDValue Op1, Op2, Op3, Op4;

      EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, 64);
      if (SubregNdx1 >= 0) {
        v1 = SubregNdx1 / 64;
        if (v1 < 4) {
          o1 = V1;
        } else {
          o1 = V2;
          v1 -= 4;
          SubregNdx1 -= 256;
        }
        SDValue Subreg1Start = DAG.getConstant(SubregNdx1, DL, MVT::i32);
        Op1 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o1,
                          Subreg1Start);
      } else {
        Op1 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      if (SubregNdx2 >= 0) {
        v2 = SubregNdx2 / 64;
        if (v2 < 4) {
          o2 = V1;
        } else {
          o2 = V2;
          v2 -= 4;
          SubregNdx2 -= 256;
        }
        SDValue Subreg2Start = DAG.getConstant(SubregNdx2, DL, MVT::i32);
        Op2 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o2,
                          Subreg2Start);
      } else {
        Op2 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      if (SubregNdx3 >= 0) {
        v3 = SubregNdx3 / 64;
        if (v3 < 4) {
          o3 = V1;
        } else {
          o3 = V2;
          v3 -= 4;
          SubregNdx3 -= 256;
        }
        SDValue Subreg3Start = DAG.getConstant(SubregNdx3, DL, MVT::i32);
        Op3 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o3,
                          Subreg3Start);
      } else {
        Op3 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      if (SubregNdx4 >= 0) {
        v4 = SubregNdx4 / 64;
        if (v4 < 4) {
          o4 = V1;
        } else {
          o4 = V2;
          v4 -= 4;
          SubregNdx4 -= 256;
        }
        SDValue Subreg4Start = DAG.getConstant(SubregNdx4, DL, MVT::i32);
        Op4 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o4,
                          Subreg4Start);
      } else {
        Op4 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      SmallVector<SDValue, 4> SubVects;
      SubVects.push_back(Op1);
      SubVects.push_back(Op2);
      SubVects.push_back(Op3);
      SubVects.push_back(Op4);
      return createTuple(SubVects, DAG);
    }
  }
  if (ShuffleMask.size() == 256 && EltSize == 16 && OpVectSize == 256) {
    int SubregNdx1 = ShuffleMask[0];
    int SubregNdx2 = ShuffleMask[128];
    if ((is_lin128(&ShuffleMask[0]) || SubregNdx1 < 0) &&
        (is_lin128(&ShuffleMask[128]) || SubregNdx2 < 0)) {
      int v1, v2;
      SDValue o1, o2;
      SDValue Op1, Op2;

      EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, 128);
      if (SubregNdx1 >= 0) {
        v1 = SubregNdx1 / 128;
        if (v1 < 2) {
          o1 = V1;
        } else {
          o1 = V2;
          v1 -= 4;
          SubregNdx1 -= 256;
        }
        SDValue Subreg1Start = DAG.getConstant(SubregNdx1, DL, MVT::i32);
        Op1 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o1,
                          Subreg1Start);
      } else {
        Op1 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      if (SubregNdx2 >= 0) {
        v2 = SubregNdx2 / 128;
        if (v2 < 2) {
          o2 = V1;
        } else {
          o2 = V2;
          v2 -= 4;
          SubregNdx2 -= 256;
        }
        SDValue Subreg2Start = DAG.getConstant(SubregNdx2, DL, MVT::i32);
        Op2 = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, o2,
                          Subreg2Start);
      } else {
        Op2 = DAG.getNode(ISD::UNDEF, DL, SubregType);
      }
      SmallVector<SDValue, 2> SubVects;
      SubVects.push_back(Op1);
      SubVects.push_back(Op2);
      return createTuple(SubVects, DAG);
    }
  }
  int iqel = 0;
  if (is_all_eq(&ShuffleMask[0], ShuffleMask.size(), iqel)) {
    SmallVector<SDValue, 4> sbv0, sbv1;
    auto opco = V1.getOpcode();
    if (V1.getNumOperands() == 2) {
      SDValue bv0 = reachBuildVector(V1.getOperand(0), sbv0);
      SDValue bv1 = reachBuildVector(V1.getOperand(1), sbv1);
      if (bv0 != SDValue() && bv1 != SDValue()) {
        SDValue s0, s1;
        s0 = reachScalar(bv0, 0, sbv0, DAG);
        s1 = reachScalar(bv1, 0, sbv1, DAG);
        auto ScalarOprnd = DAG.getNode(opco, DL, EltT, s0, s1);
        unsigned DataTypeVal = 0;
        if (EltT == MVT::f32) {
          DataTypeVal = TPCII::OpType::FP32;
        } else if (EltT == MVT::bf16) {
          DataTypeVal = TPCII::OpType::BF16;
        } else if (EltT == MVT::f16) {
          DataTypeVal = TPCII::OpType::FP16;
        } else if (EltT == MVT::f8_143) {
          DataTypeVal = TPCII::OpType::FP8_143;
        } else if (EltT == MVT::f8_152) {
          DataTypeVal = TPCII::OpType::FP8_152;
        } else if (EltT == MVT::i32) {
          DataTypeVal = TPCII::OpType::INT32;
        } else if (EltT == MVT::i16) {
          DataTypeVal = TPCII::OpType::INT16;
        } else if (EltT == MVT::i8) {
          DataTypeVal = TPCII::OpType::INT8;
        }
        SDValue Income = DAG.getUNDEF(VT);
        SmallVector<SDValue, 8> Operands;
        Operands.push_back(ScalarOprnd);
        Operands.push_back(DAG.getTargetConstant(DataTypeVal, DL, MVT::i8));
        Operands.push_back(DAG.getTargetConstant(0, DL, MVT::i32));
        Operands.push_back(Income);
        Operands.push_back(DAG.getRegister(TPC::SPRF_TRUE, MVT::i1));
        Operands.push_back(DAG.getTargetConstant(0, DL, MVT::i1));
        MachineSDNode *Node = DAG.getMachineNode(TPC::MOVvsp, DL, VT, Operands);
        return SDValue(Node,0);
#if 0
        // Vector Now
        SmallVector<SDValue, 256> Args;
        for (int i = 0; i < ShuffleMask.size(); i++)
          Args.push_back(ScalarOp);
        return DAG.getNode(ISD::BUILD_VECTOR, DL, VT, Args);
#endif
      }
    }
  }

  assert(false && (Op.dump(), "Unhandled shufflevector"));
  return SDValue();
}



static SDValue lowerLoadOfLongScalar(LoadSDNode *LoadNode,
                                     const TPCTargetLowering &TL,
                                     SelectionDAG &DAG) {
  EVT MemTy = LoadNode->getMemoryVT();
  EVT EltTy = MemTy.getVectorElementType();
  unsigned EltSz = EltTy.getScalarSizeInBits();
  (void)EltSz;

  assert(MemTy.getVectorNumElements() == 2);
  assert(EltTy.isScalarInteger());
  assert(EltSz == 8 || EltSz == 16 || EltSz == 32);

  SDValue Chain = LoadNode->getChain();
  SDValue Ptr = LoadNode->getBasePtr();
  SDLoc DL(LoadNode);

  MachinePointerInfo PtrInfo = LoadNode->getPointerInfo();
  unsigned Alignment = LoadNode->getAlignment();
  MachineMemOperand *MemOp = LoadNode->getMemOperand();
  AAMDNodes AAInfo = LoadNode->getAAInfo();

  // Load first register.
  SDValue V0 = DAG.getLoad(EltTy, DL, Chain, Ptr, PtrInfo.getWithOffset(0),
                           Alignment, MemOp->getFlags(), AAInfo);

  // Get pointer to the next register location in memory.
  SDValue NewPtr = DAG.getNode(
      ISD::ADD, DL, Ptr.getValueType(), Ptr,
      DAG.getConstant(SRF_REGISTER_LENGTH_IN_BYTES, DL, Ptr.getValueType()));

  // Store second register.
  SDValue V1 = DAG.getLoad(EltTy, DL, Chain, NewPtr,
                           PtrInfo.getWithOffset(SRF_REGISTER_LENGTH_IN_BYTES),
                           Alignment, MemOp->getFlags(), AAInfo);

  // Make wide register.
  SDValue WideValue = TL.createTuple({V0, V1}, DAG);
  SDValue Ops[2] = {WideValue, Chain};
  return DAG.getMergeValues(Ops, DL);
}

static SDValue lowerLoadOInt5(LoadSDNode *LoadNode, const TPCTargetLowering &TL,
                              SelectionDAG &DAG) {
  EVT MemTy = LoadNode->getMemoryVT();
  EVT EltTy = MemTy.getVectorElementType();
  unsigned EltSz = EltTy.getScalarSizeInBits();
  (void)EltSz;

  assert(MemTy.getVectorNumElements() == 5);
  assert(EltTy.isScalarInteger());
  assert(EltSz == 32);

  SDValue Chain = LoadNode->getChain();
  SDValue Ptr = LoadNode->getBasePtr();
  SDLoc DL(LoadNode);

  MachinePointerInfo PtrInfo = LoadNode->getPointerInfo();
  unsigned Alignment = LoadNode->getAlignment();
  MachineMemOperand *MemOp = LoadNode->getMemOperand();
  AAMDNodes AAInfo = LoadNode->getAAInfo();

  SDValue Result = DAG.getUNDEF(MemTy);
  for (unsigned I = 0; I < 5; ++I) {
    if (I) {
      // Get pointer to the next register location in memory.
      Ptr = DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                        DAG.getConstant(SRF_REGISTER_LENGTH_IN_BYTES, DL,
                                        Ptr.getValueType()));
    }
    SDValue V0 =
        DAG.getLoad(EltTy, DL, Chain, Ptr,
                    PtrInfo.getWithOffset(SRF_REGISTER_LENGTH_IN_BYTES * I),
                    Alignment, MemOp->getFlags(), AAInfo);
    Result = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MemTy, Result, V0,
                         DAG.getConstant(I, DL, MVT::i32));
  }

  SDValue Ops[2] = {Result, Chain};
  return DAG.getMergeValues(Ops, DL);
}

static unsigned getOpcodeLD_G_INC(const SDNode *Node, bool Gen3) {
  EVT type = Node->getValueType(0);
  if (Gen3) {
    return (type == MVT::i1) ? TPC::LD_G_INCpap
                             : TPC::LD_G_INCsap;
  } else {
    if (type == MVT::v2i32)
      return TPC::LD_Gg5_INCzap;
    else
      return (type == MVT::i1) ? TPC::LD_Gg4_INCpap : TPC::LD_Gg4_INCsap;
  }
}

static void applyGlobalIncSwitch(unsigned &Sw, unsigned Ofs, bool Gen3) {
  if (!Gen3) {
    Sw &= ~TPCII::SW_INC_VAL;
    switch (Ofs) {
    case 1:
      Sw |= TPCII::SW_INC_1;
      break;
    case 2:
      Sw |= TPCII::SW_INC_2;
      break;
    case 4:
      Sw |= TPCII::SW_INC_4;
      break;
    case 8:
      Sw |= TPCII::SW_INC_8;
      break;
    default:
      llvm_unreachable("Unexpected offset");
    }
  } else {
    Sw |= TPCII::SW_AUTO_INC_G3;
    Sw &= ~TPCII::SW_INC_VAL_G3;
    switch (Ofs) {
    case 1:
      Sw |= TPCII::SW_INC_1_G3;
      break;
    case 2:
      Sw |= TPCII::SW_INC_2_G3;
      break;
    case 4:
      Sw |= TPCII::SW_INC_4_G3;
      break;
    case 8:
      Sw |= TPCII::SW_INC_8_G3;
      break;
    default:
      llvm_unreachable("Unexpected offset");
    }
  }
}

static SDValue lowerPostIncLoad(LoadSDNode *LoadNode,
                                const TPCTargetLowering &TL,
                                const TPCSubtarget &Subtarget,
                                SelectionDAG &DAG) {
  if (LoadNode->getAddressingMode() != ISD::POST_INC ||
      LoadNode->getAddressSpace() != GLOBAL)
    return SDValue();
  if (!Subtarget.getFeatureBits()[TPC::FeatureGen3Plus])
    return SDValue();

  bool Gen3 = Subtarget.hasGrecoISA();
  unsigned InstrCode = getOpcodeLD_G_INC(LoadNode, Gen3);
  SDLoc DL = SDLoc(LoadNode);
  SDVTList VTs = DAG.getVTList(LoadNode->getValueType(0), MVT::i64, MVT::Other);
  unsigned Switches = 0;
  if (auto V = dyn_cast<ConstantSDNode>(LoadNode->getOffset())) {
    unsigned Offset = V->getZExtValue();
    applyGlobalIncSwitch(Switches, Offset, Gen3);
  } else
    return SDValue();

  SDValue Ops[] = {
      LoadNode->getBasePtr(),                        // Ptr
      DAG.getTargetConstant(Switches, DL, MVT::i32), // Sw
      DAG.getUNDEF(LoadNode->getValueType(0)),       // Income
      DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),            // Pred
      DAG.getTargetConstant(0, DL, MVT::i1),         // Polarity
      LoadNode->getChain(),                          // Chain
  };
  MachineSDNode *Node = DAG.getMachineNode(InstrCode, DL, VTs, Ops);
  DAG.ReplaceAllUsesOfValueWith(SDValue(LoadNode, 1), SDValue(Node, 1));
  DAG.ReplaceAllUsesOfValueWith(SDValue(LoadNode, 2), SDValue(Node, 2));
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::lowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  LoadSDNode *LoadNode = cast<LoadSDNode>(Op);
  EVT MemVT = LoadNode->getMemoryVT();

  if (!MemVT.isVector())
    return lowerPostIncLoad(LoadNode, *this, *Subtarget, DAG);

  unsigned NumElements = MemVT.getVectorNumElements();
  if (NumElements == 2)
    return lowerLoadOfLongScalar(LoadNode, *this, DAG);
  if (NumElements == 5)
    return lowerLoadOInt5(LoadNode, *this, DAG);

  SDValue Chain = LoadNode->getChain();
  SDValue Ptr = LoadNode->getBasePtr();
  SDLoc DL(Op);

  if (LoadNode->getExtensionType() != ISD::LoadExtType::NON_EXTLOAD)
    return SDValue();

  EVT VT = Op.getValueType();
  EVT EltTy = VT.getVectorElementType();

  unsigned EltSz = MemVT.getScalarSizeInBits();
  assert(EltSz > 0);
  unsigned SubVectorSize = 8 * 256 / EltSz;
  unsigned NumSubVectors = NumElements / SubVectorSize;
  EVT SubVectorTy = EVT::getVectorVT(*DAG.getContext(), EltTy, SubVectorSize);
  unsigned SubRegSize = SubVectorTy.getStoreSizeInBits() / 8;

  // We provide specific lowering only for vectors of 128, 256 or 512 elements.
  switch (NumElements) {
  case 128:
  case 256:
  case 512:
    break;
  default:
    return SDValue();
  }
  assert(NumSubVectors == 2 || NumSubVectors == 4);

  SmallVector<SDValue, 4> SubRegValues;
  if (auto *CP = dyn_cast<ConstantPoolSDNode>(Ptr)) {
    const Constant *C = CP->getConstVal();
    if (isa<ConstantAggregateZero>(C)) {
      SDValue ZeroSubVector = DAG.getConstant(APInt(32, 0), DL, SubVectorTy);
      SubRegValues.append(NumSubVectors, ZeroSubVector);
    } else {
      return SDValue();
    }
  } else {
    SDValue V0 = DAG.getLoad(
        SubVectorTy, DL, Chain, Ptr,
        LoadNode->getPointerInfo().getWithOffset(0), LoadNode->getAlignment(),
        LoadNode->getMemOperand()->getFlags(), LoadNode->getAAInfo());
    SubRegValues.push_back(V0);
    SDValue NewPtr =
        DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                    DAG.getConstant(SubRegSize, DL, Ptr.getValueType()));
    SDValue V1 = DAG.getLoad(
        SubVectorTy, DL, Chain, NewPtr,
        LoadNode->getPointerInfo().getWithOffset(SubRegSize),
        LoadNode->getAlignment(), LoadNode->getMemOperand()->getFlags(),
        LoadNode->getAAInfo());
    SubRegValues.push_back(V1);
    if (NumSubVectors > 2) {
      NewPtr =
          DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                      DAG.getConstant(2 * SubRegSize, DL, Ptr.getValueType()));
      SDValue V2 = DAG.getLoad(
          SubVectorTy, DL, Chain, NewPtr,
          LoadNode->getPointerInfo().getWithOffset(2 * SubRegSize),
          LoadNode->getAlignment(), LoadNode->getMemOperand()->getFlags(),
          LoadNode->getAAInfo());
      SubRegValues.push_back(V2);
      NewPtr =
          DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                      DAG.getConstant(3 * SubRegSize, DL, Ptr.getValueType()));
      SDValue V3 = DAG.getLoad(
          SubVectorTy, DL, Chain, NewPtr,
          LoadNode->getPointerInfo().getWithOffset(3 * SubRegSize),
          LoadNode->getAlignment(), LoadNode->getMemOperand()->getFlags(),
          LoadNode->getAAInfo());
      SubRegValues.push_back(V3);
    }
  }

  SDValue WideValue = createTuple(SubRegValues, DAG);
  SDValue Ops[2] = {WideValue, Chain};
  return DAG.getMergeValues(Ops, DL);
}

static SDValue lowerStoreOfLongScalar(StoreSDNode *StoreNode,
                                      SelectionDAG &DAG) {
  EVT MemTy = StoreNode->getMemoryVT();
  EVT EltTy = MemTy.getVectorElementType();
  unsigned EltSz = EltTy.getScalarSizeInBits();
  (void)EltSz;

  assert(MemTy.getVectorNumElements() == 2);
  assert(EltTy.isScalarInteger());
  assert(EltSz == 8 || EltSz == 16 || EltSz == 32);

  SDValue Chain = StoreNode->getChain();
  SDValue Ptr = StoreNode->getBasePtr();
  SDValue Value = StoreNode->getValue();
  SDLoc DL(StoreNode);

  SmallVector<SDValue, 2> SubRegStores;
  MachinePointerInfo PtrInfo = StoreNode->getPointerInfo();
  unsigned Alignment = StoreNode->getAlignment();

  MachineSDNode *Node;

  // Store first register.
  SDValue SubReg = DAG.getTargetConstant(TPC::sub_s0, DL, MVT::i32);
  Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, EltTy, Value,
                            SubReg);
  SDValue V1(Node, 0);
  SDValue NewChain =
      DAG.getStore(StoreNode->getChain(), DL, V1, Ptr, PtrInfo.getWithOffset(0),
                   Alignment, StoreNode->getMemOperand()->getFlags()
                   /* TODO: use correct AAInfo */);
  SubRegStores.push_back(NewChain);

  // Get pointer to the next register location in memory.
  SDValue NewPtr = DAG.getNode(
      ISD::ADD, DL, Ptr.getValueType(), Ptr,
      DAG.getConstant(SRF_REGISTER_LENGTH_IN_BYTES, DL, Ptr.getValueType()));

  // Store second register.
  SubReg = DAG.getTargetConstant(TPC::sub_s1, DL, MVT::i32);
  Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, EltTy, Value,
                            SubReg);
  SDValue V2(Node, 0);
  NewChain = DAG.getStore(Chain, DL, V2, NewPtr,
                          PtrInfo.getWithOffset(SRF_REGISTER_LENGTH_IN_BYTES),
                          Alignment, StoreNode->getMemOperand()->getFlags()
                          /* TODO: use correct AAInfo */);
  SubRegStores.push_back(NewChain);

  // Group sores.
  Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, SubRegStores);
  return Chain;
}

static SDValue lowerStoreOInt5(StoreSDNode *StoreNode,
                               const TPCTargetLowering &TL, SelectionDAG &DAG) {
  EVT MemTy = StoreNode->getMemoryVT();
  (void)MemTy;
  assert(MemTy.getVectorNumElements() == 5);
  assert(MemTy.getVectorElementType().isScalarInteger());

  SDValue Chain = StoreNode->getChain();
  SDValue Ptr = StoreNode->getBasePtr();
  SDLoc DL(StoreNode);

  MachinePointerInfo PtrInfo = StoreNode->getPointerInfo();
  unsigned Alignment = StoreNode->getAlignment();
  MachineMemOperand *MemOp = StoreNode->getMemOperand();
  AAMDNodes AAInfo = StoreNode->getAAInfo();

  for (unsigned I = 0; I < 5; ++I) {
    if (I) {
      // Get pointer to the next register location in memory.
      Ptr = DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                        DAG.getConstant(SRF_REGISTER_LENGTH_IN_BYTES, DL,
                                        Ptr.getValueType()));
    }
    SDValue SubReg =
        DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
                    StoreNode->getValue(), DAG.getConstant(I, DL, MVT::i32));
    Chain =
        DAG.getStore(Chain, DL, SubReg, Ptr,
                     PtrInfo.getWithOffset(SRF_REGISTER_LENGTH_IN_BYTES * I),
                     Alignment, MemOp->getFlags(), AAInfo);
  }

  return Chain;
}

SDValue TPCTargetLowering::lowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  StoreSDNode *StoreNode = cast<StoreSDNode>(Op);

  EVT MemVT = StoreNode->getMemoryVT();
  if (!MemVT.isVector())
    return SDValue();

  unsigned NumElements = MemVT.getVectorNumElements();

  if (NumElements == 2)
    return lowerStoreOfLongScalar(StoreNode, DAG);

  if (NumElements == 5)
    return lowerStoreOInt5(StoreNode, *this, DAG);

  EVT EltTy = MemVT.getVectorElementType();
  unsigned EltSz = MemVT.getScalarSizeInBits();
  assert(EltSz > 0);

  SDValue Chain = StoreNode->getChain();
  SDValue Ptr = StoreNode->getBasePtr();
  SDValue Value = StoreNode->getValue();
  SDLoc DL(Op);

  unsigned SubVectorSize = 8 * 256 / EltSz;
  unsigned NumSubVectors = NumElements / SubVectorSize;
  EVT SubVectorTy = EVT::getVectorVT(*DAG.getContext(), EltTy, SubVectorSize);
  unsigned SubRegSize = SubVectorTy.getStoreSizeInBits() / 8;

  // We provide specific lowering only for vectors of 128, 256 or 512 elements.
  switch (NumElements) {
  case 128:
  case 256:
  case 512:
    break;
  default:
    return SDValue();
  }
  assert(NumSubVectors == 2 || NumSubVectors == 4);

  SmallVector<SDValue, 4> SubRegStores;
  MachinePointerInfo PtrInfo = StoreNode->getPointerInfo();
  unsigned Alignment = StoreNode->getAlignment();

  SDValue SubReg = DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32);
  MachineSDNode *Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                           SubVectorTy, Value, SubReg);
  SDValue V1(Node, 0);
  SDValue NewChain =
      DAG.getStore(Chain, DL, V1, Ptr, PtrInfo.getWithOffset(0), Alignment,
                   StoreNode->getMemOperand()->getFlags()
                   /* TODO: use correct AAInfo */);
  SubRegStores.push_back(NewChain);

  SDValue NewPtr =
      DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                  DAG.getConstant(SubRegSize, DL, Ptr.getValueType()));
  SubReg = DAG.getTargetConstant(TPC::sub_1, DL, MVT::i32);
  Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubVectorTy,
                            Value, SubReg);
  SDValue V2(Node, 0);
  NewChain =
      DAG.getStore(Chain, DL, V2, NewPtr, PtrInfo.getWithOffset(SubRegSize),
                   Alignment, StoreNode->getMemOperand()->getFlags()
                   /* TODO: use correct AAInfo */);
  SubRegStores.push_back(NewChain);

  if (NumSubVectors == 4) {
    NewPtr =
        DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                    DAG.getConstant(2 * SubRegSize, DL, Ptr.getValueType()));
    SubReg = DAG.getTargetConstant(TPC::sub_2, DL, MVT::i32);
    Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubVectorTy,
                              Value, SubReg);
    SDValue V3(Node, 0);
    NewChain = DAG.getStore(Chain, DL, V3, NewPtr,
                            PtrInfo.getWithOffset(2 * SubRegSize), Alignment,
                            StoreNode->getMemOperand()->getFlags()
                            /* TODO: use correct AAInfo */);
    SubRegStores.push_back(NewChain);

    NewPtr =
        DAG.getNode(ISD::ADD, DL, Ptr.getValueType(), Ptr,
                    DAG.getConstant(3 * SubRegSize, DL, Ptr.getValueType()));
    SubReg = DAG.getTargetConstant(TPC::sub_3, DL, MVT::i32);
    Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubVectorTy,
                              Value, SubReg);
    SDValue V4(Node, 0);
    NewChain = DAG.getStore(Chain, DL, V4, NewPtr,
                            PtrInfo.getWithOffset(3 * SubRegSize), Alignment,
                            StoreNode->getMemOperand()->getFlags()
                            /* TODO: use correct AAInfo */);
    SubRegStores.push_back(NewChain);
  }

  Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, SubRegStores);
  return Chain;
}

SDValue TPCTargetLowering::lowerCONCAT_VECTORS(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op.getValueType();
  assert(VT.isVector());
  unsigned NumElements = VT.getVectorNumElements();

  // We provide specific lowering only for vectors of 128, 256 or 512 elements.
  switch (NumElements) {
  case 128:
  case 256:
  case 512:
    break;
  default:
    return SDValue();
  }

  unsigned NumOperands = Op.getNode()->getNumOperands();
  assert(NumOperands == 2 || NumOperands == 4);
  SmallVector<SDValue, 4> SubVects;

  bool IsSubRegExtractRequired = false;
  EVT RegTy = Op.getOperand(0).getValueType();
  EVT EltTy = RegTy.getVectorElementType();
  if (NumOperands == 2) {
    // if both operands are DRF vector registers, we need to extract
    // VRF vector sub-registers and add them to the SubVects
    unsigned NumElts = RegTy.getVectorNumElements();
    unsigned RegBits = NumElts * EltTy.getSizeInBits();

    // TODO: is there a better way to get number of bits in ARF/DRF registers?
    assert((RegBits == 2048 || RegBits == 4096) &&
           "Invalid number of vector bits");
    IsSubRegExtractRequired = (RegBits == 4096) ? true : false;
  }

  for (unsigned i = 0; i < NumOperands; ++i) {
    SDValue OpSubReg = Op.getOperand(i);
    if (IsSubRegExtractRequired) {
      EVT SubSubVectorTy;
      SubSubVectorTy = OpSubReg.getValueType();
      assert(SubSubVectorTy.isVector());
      unsigned SubNumElts = SubSubVectorTy.getVectorNumElements();
      EVT SubTy = EVT::getVectorVT(*DAG.getContext(), EltTy, SubNumElts / 2);
      if (OpSubReg.isUndef()) {
        SDValue uv = DAG.getUNDEF(SubTy);
        SubVects.push_back(uv);
        SubVects.push_back(uv);

      } else {
        SDValue h0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, SubTy, OpSubReg);
        SDValue h1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, SubTy, OpSubReg);

        SubVects.push_back(h0);
        SubVects.push_back(h1);
      }
    } else {
      SubVects.push_back(OpSubReg);
    }
  }
  SDValue thu = createTuple(SubVects, DAG);
  return thu;
}

SDValue TPCTargetLowering::createTuple(ArrayRef<SDValue> Regs,
                                       SelectionDAG &DAG) const {
  assert(Regs.size() == 2 || Regs.size() == 4);
  SDLoc DL(Regs[0]);
  EVT RegTy = Regs[0].getValueType();
  EVT ResTy;
  unsigned RegId;
  if (RegTy.isVector()) {
    EVT EltTy = RegTy.getVectorElementType();
    unsigned NumElts = RegTy.getVectorNumElements();
    ResTy = EVT::getVectorVT(*DAG.getContext(), EltTy, NumElts * Regs.size());
    if (Regs.size() == 4)
      RegId = TPC::ARFRegClassID;
    else
      RegId = TPC::DRFRegClassID;
  } else {
    ResTy = EVT::getVectorVT(*DAG.getContext(), RegTy, ElementCount::getFixed(2));
    RegId = TPC::ZRFRegClassID;
  }

  static const unsigned SubRegs[] = {TPC::sub_0, TPC::sub_1, TPC::sub_2,
                                     TPC::sub_3};
  static const unsigned SubRegsScalar[] = {TPC::sub_s0, TPC::sub_s1};
  SmallVector<SDValue, 4> Ops;

  // First operand of REG_SEQUENCE is the desired RegClass.
  Ops.push_back(DAG.getTargetConstant(RegId, DL, MVT::i32));

  // Then we get pairs of source & subregister-position for the components.
  for (unsigned I = 0, E = Regs.size(); I < E; ++I) {
    Ops.push_back(Regs[I]);
    if (RegId == TPC::ZRFRegClassID)
      Ops.push_back(DAG.getTargetConstant(SubRegsScalar[I], DL, MVT::i32));
    else
      Ops.push_back(DAG.getTargetConstant(SubRegs[I], DL, MVT::i32));
  }

  SDNode *N = DAG.getMachineNode(TargetOpcode::REG_SEQUENCE, DL, ResTy, Ops);
  return SDValue(N, 0);
}

// TODO: Remove it.
#if 0
static unsigned getIRFOpcodeForIntrin(unsigned func, bool immOperand) {
  unsigned opcode = 0;
  switch (func) {
  default:
    return 0;
    // TODO: Scal2IRF
    //  case Intrinsic::tpc_i_i32_add_s_i:
//    opcode = immOperand ? TPC::ADDi32IiIe : TPC::ADDi32IsIe;
//    break;
//  case Intrinsic::tpc_i_i32_sub_s_i:
//    opcode = immOperand ? TPC::SUBi32IiIe : TPC::SUBi32IsIe;
//    break;
//  case Intrinsic::tpc_i_i32_mul_s_i:
//    opcode = immOperand ? TPC::MULi32IiIe : TPC::MULi32IsIe;
//    break;
//  case Intrinsic::tpc_i_i32_or_s_i:
//    opcode = immOperand ? TPC::ORi32IiIe : TPC::ORi32IsIe;
//    break;
//  case Intrinsic::tpc_i_i32_xor_s_i:
//    opcode = immOperand ? TPC::XORi32IiIe : TPC::XORi32IsIe;
//    break;
//  case Intrinsic::tpc_i_i32_and_s_i:
//    opcode = immOperand ? TPC::ANDi32IiIe : TPC::ANDi32IsIe;
//    break;
  }
  return opcode;
}
#endif

SDValue TPCTargetLowering::LowerINTRINSIC_VOID(SDValue Op,
                                                   SelectionDAG &DAG) const {
  unsigned IntrinNum = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();

  switch (IntrinNum) {
    default:
      return SDValue();
    case Intrinsic::tpc_reset_sqz_cntr: {
      auto OpIt = Op->op_begin();
      SDValue Chain = *OpIt++;
      ++OpIt;

      SDValue SqzIdx = *OpIt++;
      SDValue SqzVal = *OpIt++;
      SDValue Predicate = *OpIt++;
      SDValue Polarity = *OpIt++;

      SDLoc DL(SqzVal);
      const auto ImmToSrf = [&DAG, &DL](uint32_t Value) -> SDValue {
        return SDValue(
        DAG.getMachineNode(
          TPC::MOVsip, DL, MVT::i32, {
            DAG.getTargetConstant(Value, DL, MVT::i32),
            DAG.getTargetConstant(0, DL, MVT::i8),
            DAG.getTargetConstant(0, DL, MVT::i32),
            DAG.getUNDEF(MVT::i32),
            DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
            DAG.getTargetConstant(0, DL, MVT::i1)}),
        0);
      };

      const FeatureBitset &Features = Subtarget->getFeatureBits();
      if (Features[TPC::FeatureDoron1]) {
        // In gaudi 3 addresses for sqz_cntr registers are not contiguous but grouped for threads
        // Every quarter in its own memory region
        // 0th thread - 0x1174, 1st thread - 0x1374, 2nd thread - 0x1574, 3rd thread - 0x1774
        int RegIndex = cast<ConstantSDNode>(SqzIdx)->getLimitedValue();
        int Quartet = RegIndex / 4;
        int SmallIndex = RegIndex % 4;

        SDValue SqzLo = DAG.getTargetConstant(0x1174 + SmallIndex * 8 + Quartet * 0x200, DL, MVT::i32);
        SDValue SqzHi = DAG.getTargetConstant(0x1174 + SmallIndex * 8 + 4 + Quartet * 0x200, DL, MVT::i32);

        unsigned PolVal = cast<ConstantSDNode>(Polarity)->getZExtValue();

        auto LoSub = DAG.getMachineNode(TPC::ST_Lisp, DL, MVT::Other, {
                                                     SqzLo,
                                                     ImmToSrf(cast<ConstantSDNode>(SqzVal)->getLimitedValue()),
                                                     DAG.getTargetConstant(TPCII::SW_MMIO | TPCII::SW_VPU_ONLY, DL, MVT::i32),
                                                     Predicate,
                                                     DAG.getTargetConstant(PolVal, DL, Polarity.getValueType()),
                                                     Chain
                                                   });
        SDValue NewChain = SDValue(LoSub, 0);
        auto HiSub = DAG.getMachineNode(TPC::ST_Lisp, DL, MVT::Other, {
                                                     SqzHi,
                                                     ImmToSrf(0),
                                                     DAG.getTargetConstant(TPCII::SW_MMIO | TPCII::SW_VPU_ONLY, DL, MVT::i32),
                                                     Predicate,
                                                     DAG.getTargetConstant(PolVal, DL, Polarity.getValueType()),
                                                     NewChain
                                                   });
        DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 0), SDValue(HiSub, 0));
        return SDValue(HiSub, 0);
      } else {
        int RegVal = cast<ConstantSDNode>(SqzIdx)->getLimitedValue();
        SDValue SqzLo = DAG.getTargetConstant(0x10 + RegVal * 2, DL, MVT::i32);
        SDValue SqzHi = DAG.getTargetConstant(0x10 + RegVal * 2 + 1, DL, MVT::i32);

        unsigned PolVal = cast<ConstantSDNode>(Polarity)->getZExtValue();

        auto LoSub = DAG.getMachineNode(TPC::MOVsqz, DL, MVT::Other, {
                                                     SqzLo,
                                                     DAG.getTargetConstant(0, DL, MVT::i32),
                                                     ImmToSrf(cast<ConstantSDNode>(SqzVal)->getLimitedValue()),
                                                     Predicate,
                                                     DAG.getTargetConstant(PolVal, DL, Polarity.getValueType()),
                                                     Chain
                                                   });
        SDValue NewChain = SDValue(LoSub, 0);
        auto HiSub = DAG.getMachineNode(TPC::MOVsqz, DL, MVT::Other, {
                                                     SqzHi,
                                                     DAG.getTargetConstant(0, DL, MVT::i32),
                                                     ImmToSrf(0),
                                                     Predicate,
                                                     DAG.getTargetConstant(PolVal, DL, Polarity.getValueType()),
                                                     NewChain
                                                   });
        DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 0), SDValue(HiSub, 0));
        return SDValue(HiSub, 0);
        }
    }
  }
}

SDValue TPCTargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
                                                   SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "TPC intrinsic custom lowering:\n"; Op.dump(&DAG););

  unsigned IntrinNum = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();

  switch (IntrinNum) {
  default:
    return SDValue();
  case Intrinsic::tpc_convert_linear:
    return lowerTPC_CONVERT_LINEAR(Op, DAG);
  case Intrinsic::tpc_convert:
    return lowerTPC_CONVERT(Op, DAG);
  case Intrinsic::tpc_fptrunc_swch:
    return lowerFP_ROUND(Op, DAG);
  case Intrinsic::tpc_fpext_swch:
    return lowerFP_EXTEND(Op, DAG);
  case Intrinsic::tpc_sitofp_swch:
  case Intrinsic::tpc_fptosi_swch:
    return lowerCONVERTSIGNED(Op, DAG);
  case Intrinsic::tpc_uitofp_swch:
  case Intrinsic::tpc_fptoui_swch:
    return lowerCONVERTUNSIGNED(Op, DAG);
  case Intrinsic::tpc_convert_int:
    return lowerTPC_CONVERT_INT(Op, DAG);
  case Intrinsic::tpc_convert_uint:
    return lowerTPC_CONVERT_UINT(Op, DAG);
  case Intrinsic::tpc_convert_int_linear:
    return lowerTPC_CONVERT_INT_LINEAR(Op, DAG);
  case Intrinsic::tpc_convert_uint_linear:
    return lowerTPC_CONVERT_UINT_LINEAR(Op, DAG);
  case Intrinsic::tpc_mov_dual_group:
    return lowerTPC_MovDualGroup(Op, false, DAG);
  case Intrinsic::tpc_mov_dual_group_all:
    return lowerTPC_MovDualGroup(Op, true, DAG);
  }

// TODO: Remove it.
#if 0
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDLoc dl(Op);
  MachineSDNode *copyNode;
  SDValue copy;

  switch (IntNo) {
  default: return Op;    // Don't custom lower most intrinsics.
// TODO: Scal2IRF
//  case Intrinsic::tpc_i_i32_add_s_i:
//  case Intrinsic::tpc_i_i32_sub_s_i:
//  case Intrinsic::tpc_i_i32_mul_s_i:
//  case Intrinsic::tpc_i_i32_or_s_i:
  case Intrinsic::tpc_i_i32_xor_s_i:
//  case Intrinsic::tpc_i_i32_and_s_i:
    SDValue Src1 = Op.getOperand(1);
    SDValue Src2 = Op.getOperand(2);
    SDValue Src3 = Op.getOperand(3);
    SDValue Src4 = Op.getOperand(4);

    // Generate COPY for 'plink' parameter. Do it only if Undef value passed as 'plink'.
    // This is to distinguish between user-defined anf compiler-generated intrinsics -
    // for the latter we do not have insert COPY.
    if (Src3->isUndef()) {
      copyNode = DAG.getMachineNode(TargetOpcode::COPY, dl, MVT::v5i32, Src2);
      copy = SDValue(copyNode, 0);
    }
    else {
      copy = Src3;
    }
    
    unsigned dim = cast<ConstantSDNode>(Src4)->getZExtValue();
    SDValue DimVal = DAG.getTargetConstant(dim, dl, MVT::i8);

    SDValue Val;
    unsigned IRFOpcode;
    bool isImmOperand;
    if (auto Scl = dyn_cast<ConstantSDNode>(Src1)) {
      unsigned v = Scl->getZExtValue();
      Val = DAG.getTargetConstant(v, dl, MVT::i32);
      isImmOperand = true;
    }
    else {
      Val = Src1;
      isImmOperand = false;
    }
    IRFOpcode = getIRFOpcodeForIntrin(IntNo, isImmOperand);

    SmallVector<SDValue, 8> Ops(4);
    Ops[0] = Val;
    Ops[1] = Src2;
    Ops[2] = copy;
    Ops[3] = DimVal;

    MachineSDNode *Node = DAG.getMachineNode(IRFOpcode, dl, MVT::v5i32, Ops);

    return SDValue(Node, 0);
  }
#endif
}

static SDValue InitSQZFromImm(SDValue SQZValue, SelectionDAG &DAG) {
  assert(isa<ConstantSDNode>(SQZValue) && "Must be a constant");
  uint32_t Value = cast<ConstantSDNode>(SQZValue)->getLimitedValue();
  
  SDLoc DL(SQZValue);
  const auto ImmToSrf = [&DAG, &DL](uint32_t Value) -> SDValue {
    return SDValue(
    DAG.getMachineNode(
      TPC::MOVsip, DL, MVT::i32, {
        DAG.getTargetConstant(Value, DL, MVT::i32),
        DAG.getTargetConstant(0, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(MVT::i32),
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
        DAG.getTargetConstant(0, DL, MVT::i1)}),
    0);
  };
  
  SDValue RegClass = DAG.getTargetConstant(TPC::HWSqzCntrRegClassID, DL, MVT::i32);
  SDValue SubRegClass1 = DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32);
  SDValue SubRegClass2 = DAG.getTargetConstant(TPC::sub_1, DL, MVT::i32);
  
  SDValue LoSub = SDValue(DAG.getMachineNode(TPC::COPY, DL, MVT::i32, ImmToSrf(Value)), 0);
  SDValue HiSub = SDValue(DAG.getMachineNode(TPC::COPY, DL, MVT::i32, ImmToSrf(0)), 0);
  return SDValue(
    DAG.getMachineNode(TargetOpcode::REG_SEQUENCE, DL, MVT::i32,
                       {RegClass, LoSub, SubRegClass1, HiSub, SubRegClass2}),
    0);
}

static SDValue SRFToStTnsrIdReg(SDValue Value, SelectionDAG &DAG) {
  SDLoc DL(Value);
  return SDValue(DAG.getMachineNode(TPC::COPY, DL, MVT::i32, Value), 0);
}

SDValue TPCTargetLowering::LowerINTRINSIC_W_CHAIN(SDValue Op,
                                                  SelectionDAG &DAG) const {
  unsigned IntrID = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();
  SDLoc DL(Op);

  switch (IntrID) {
  case Intrinsic::tpc_ld_g: {
    MemSDNode *M = cast<MemSDNode>(Op);
    SDValue Ops[] = {
        Op.getOperand(0), // Chain
        Op.getOperand(2), // Ptr
        Op.getOperand(3), // Sw
        Op.getOperand(4), // Income
        Op.getOperand(5), // Pred
        Op.getOperand(6), // Polarity
    };
    return DAG.getMemIntrinsicNode(TPCISD::LD_G, DL,
                                   Op->getVTList(), Ops, M->getMemoryVT(),
                                   M->getMemOperand());
  }
  case Intrinsic::tpc_ld_g_int5: {
    MemSDNode *M = cast<MemSDNode>(Op);
    SDValue Ops[] = {
        Op.getOperand(0), // Chain
        Op.getOperand(2), // Ptr
        Op.getOperand(3), // Mask
        Op.getOperand(4), // Sw
        Op.getOperand(5), // Income
        Op.getOperand(6), // Pred
        Op.getOperand(7), // Polarity
    };
    return DAG.getMemIntrinsicNode(TPCISD::LD_GI, DL,
                                   Op->getVTList(), Ops, M->getMemoryVT(),
                                   M->getMemOperand());
  }
  case Intrinsic::tpc_ld_g_inc: {
    SDValue Chain = Op.getOperand(0);
    SDValue Ptr = Op.getOperand(2);
    SDValue Offset = Op.getOperand(3);
    SDValue Switches = Op.getOperand(4);
    SDValue Income = Op.getOperand(5);
    SDValue Pred = Op.getOperand(6);
    SDValue Polarity = Op.getOperand(7);

    unsigned Ofs = cast<ConstantSDNode>(Offset)->getZExtValue();
    unsigned Sw = cast<ConstantSDNode>(Switches)->getZExtValue();
    unsigned PolVal = cast<ConstantSDNode>(Polarity)->getZExtValue();

    assert(Subtarget->getFeatureBits()[TPC::FeatureGen3Plus]);
    bool Gen3 = Subtarget->hasGrecoISA();
    applyGlobalIncSwitch(Sw, Ofs, Gen3);
    unsigned Opcode = getOpcodeLD_G_INC(Op.getNode(), Gen3);
    SDValue PolNode =
        DAG.getTargetConstant(PolVal, DL, Polarity.getValueType());
    SDValue SwNode = DAG.getTargetConstant(Sw, DL, Switches.getValueType());
    MachineSDNode *LdNode =
        DAG.getMachineNode(Opcode, DL, Op.getNode()->getVTList(),
                           {Ptr, SwNode, Income, Pred, PolNode});
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 1), SDValue(LdNode, 1));
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 2), Chain);
    return SDValue(LdNode, 0);
  }
  case Intrinsic::tpc_ld_g_partial_inc: {
    SDValue Chain = Op.getOperand(0);
    SDValue Ptr = Op.getOperand(2);
    SDValue Offset = Op.getOperand(3);
    SDValue Partial = Op.getOperand(4);
    SDValue Switches = Op.getOperand(5);
    SDValue Income = Op.getOperand(6);
    SDValue Pred = Op.getOperand(7);
    SDValue Polarity = Op.getOperand(8);

    unsigned Ofs = cast<ConstantSDNode>(Offset)->getZExtValue();
    unsigned Sw = cast<ConstantSDNode>(Switches)->getZExtValue();
    unsigned PolVal = cast<ConstantSDNode>(Polarity)->getZExtValue();

    assert(Subtarget->getFeatureBits()[TPC::FeatureGen3Plus]);
    bool Gen3 = Subtarget->hasGrecoISA();
    applyGlobalIncSwitch(Sw, Ofs, Gen3);
    unsigned Opcode;
    if (!Gen3) {
      Opcode = Pred.getValueType() == MVT::i1 ? TPC::LD_G_P_INCg4vap
                                              : TPC::LD_G_P_INCg4vam;
    } else {
      Opcode = Pred.getValueType() == MVT::i1 ? TPC::LD_G_P_INCvap
                                              : TPC::LD_G_P_INCvam;
    }
    SDValue SwNode = DAG.getTargetConstant(Sw, DL, Switches.getValueType());
    SDValue PolNode =
        DAG.getTargetConstant(PolVal, DL, Polarity.getValueType());
    MachineSDNode *LdNode =
        DAG.getMachineNode(Opcode, DL, Op.getNode()->getVTList(),
                           {Ptr, Partial, SwNode, Income, Pred, PolNode});
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 1), SDValue(LdNode, 1));
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 2), Chain);
    return SDValue(LdNode, 0);
  }
  case Intrinsic::tpc_ld_g_int5_inc: {
    SDValue Chain = Op.getOperand(0);
    SDValue Ptr = Op.getOperand(2);
    SDValue Offset = Op.getOperand(3);
    SDValue DimMask = Op.getOperand(4);
    SDValue Switches = Op.getOperand(5);
    SDValue Income = Op.getOperand(6);
    SDValue Pred = Op.getOperand(7);
    SDValue Polarity = Op.getOperand(8);

    unsigned Ofs = cast<ConstantSDNode>(Offset)->getZExtValue();
    unsigned Sw = cast<ConstantSDNode>(Switches)->getZExtValue();
    unsigned PolVal = cast<ConstantSDNode>(Polarity)->getZExtValue();

    assert(Subtarget->getFeatureBits()[TPC::FeatureGen3Plus]);
    bool Gen3 = Subtarget->hasGrecoISA();
    applyGlobalIncSwitch(Sw, Ofs, Gen3);

    bool DitMaskIsImm = false;
    SDValue DimMaskNode = DimMask;
    if (auto DimMaskImm = dyn_cast<ConstantSDNode>(DimMask)) {
      DitMaskIsImm = true;
      DimMaskNode = DAG.getTargetConstant(DimMaskImm->getZExtValue(), DL,
                                          DimMask.getValueType());
    }

    unsigned Opcode;
    if (!Gen3) {
      Opcode = DitMaskIsImm ? TPC::LD_G_INCg4Iap : TPC::LD_G_INCg4Iwap;
    } else {
      Opcode = DitMaskIsImm ? TPC::LD_G_INCIap : TPC::LD_G_INCIwap;
    }
    SDValue PolNode =
        DAG.getTargetConstant(PolVal, DL, Polarity.getValueType());
    SDValue SwNode = DAG.getTargetConstant(Sw, DL, Switches.getValueType());
    MachineSDNode *LdNode =
        DAG.getMachineNode(Opcode, DL, Op.getNode()->getVTList(),
                           {Ptr, DimMaskNode, SwNode, Income, Pred, PolNode});
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 1), SDValue(LdNode, 1));
    DAG.ReplaceAllUsesOfValueWith(SDValue(Op.getNode(), 2), Chain);
    return SDValue(LdNode, 0);
  }
  case Intrinsic::tpc_st_g: {
    MemSDNode *M = cast<MemSDNode>(Op);
    SDValue Ops[] = {
        Op.getOperand(0), // Chain
        Op.getOperand(2), // Ptr
        Op.getOperand(3), // Value
        Op.getOperand(4), // Sw
        Op.getOperand(5), // Pred
        Op.getOperand(6), // Polarity
    };
    return DAG.getMemIntrinsicNode(TPCISD::ST_G, DL, Op->getVTList(), Ops,
                                   M->getMemoryVT(), M->getMemOperand());
  }
  case Intrinsic::tpc_st_g_inc: {
    auto *Node = cast<MemSDNode>(Op.getNode());
    SDLoc DL(Op);
    // SDValue StoredValue = Node->getOperand(3);
    SDValue Offset = Node->getOperand(4);
    SDValue Switches = Node->getOperand(5);
    unsigned Sw = cast<ConstantSDNode>(Switches)->getZExtValue();
    uint64_t Ofs = cast<ConstantSDNode>(Offset)->getZExtValue();
    assert(Subtarget->getFeatureBits()[TPC::FeatureGen4Plus]);
    applyGlobalIncSwitch(Sw, Ofs, false);
    Switches = DAG.getConstant(Sw, DL, MVT::i32);
    SDValue Ops[] = {
        Op.getOperand(0), // Chain
        Op.getOperand(2), // Address
        Op.getOperand(3), // Stored Value
        Switches,
        Op.getOperand(6), // Predicate
        Op.getOperand(7)  // Polarity
    };

    return DAG.getMemIntrinsicNode(TPCISD::ST_G_INC, DL, Op->getVTList(), Ops,
                                   Node->getMemoryVT(), Node->getMemOperand());
  }
  case Intrinsic::tpc_st_tnsr_s_hwr:
  case Intrinsic::tpc_st_tnsr_s_hwr_rmw: {
    auto OpIt = Op->op_begin();

    // Read
    SDValue Chain = *OpIt++;
    ++OpIt;
    SDValue Index = *OpIt++;
    SDValue Tensor = *OpIt++;
    SDValue SQZ = *OpIt++;
    SDValue RMW;
    if (IntrID == Intrinsic::tpc_st_tnsr_s_hwr_rmw)
      RMW = *OpIt++;
    SDValue Switch = *OpIt++;
    SDValue Predicate = *OpIt++;
    SDValue Polarity = *OpIt++;

    // Modification
    if (isa<ConstantSDNode>(SQZ))
      SQZ = InitSQZFromImm(SQZ, DAG);

    bool TensorIsImm = isa<ConstantSDNode>(Tensor);
    if (TensorIsImm)
      Tensor = DAG.getTargetConstant(
        cast<ConstantSDNode>(Tensor)->getZExtValue(), SDLoc(Tensor), MVT::i8);
    else
      Tensor = SRFToStTnsrIdReg(Tensor, DAG);

    Switch = DAG.getTargetConstant(cast<ConstantSDNode>(Switch)->getZExtValue(),
                                   SDLoc(Switch), MVT::i32);
    Polarity = DAG.getTargetConstant(cast<ConstantSDNode>(Polarity)->getZExtValue(),
                                     SDLoc(Polarity), MVT::i1);

    SmallVector<SDValue, 9> Ops;
    if (IntrID == Intrinsic::tpc_st_tnsr_s_hwr)
      Ops = {Index, Tensor, SQZ, Switch, Predicate, Polarity, Chain};
    else
      Ops = {Index, Tensor, SQZ, RMW, Switch, Predicate, Polarity, Chain};

    unsigned Opcode = IntrID == Intrinsic::tpc_st_tnsr_s_hwr ?
      (TensorIsImm ? TPC::ST_TNSR_S_HWRi : TPC::ST_TNSR_S_HWRr) :
      (TensorIsImm ? TPC::ST_TNSR_S_HWR_Ri : TPC::ST_TNSR_S_HWR_Rr);

    MachineSDNode *NewNode = DAG.getMachineNode(Opcode, DL,
                                                Op->getVTList(), Ops);

    return SDValue(NewNode, 0);
  }
  }
  return SDValue();
}

bool TPCTargetLowering::getTgtMemIntrinsic(IntrinsicInfo &Info,
                                           const CallInst &I,
                                           MachineFunction &MF,
                                           unsigned Intrinsic) const {
  switch (Intrinsic) {
  default:
    return false;
  case Intrinsic::tpc_ld_g:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(I.getType());
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = 0;
    Info.flags = MachineMemOperand::MOLoad;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_ld_g_int5:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::i32;
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = 0;
    Info.flags = MachineMemOperand::MOLoad;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_ld_g_inc: {
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(cast<StructType>(I.getType())->getElementType(0));
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = cast<ConstantInt>(I.getArgOperand(1))->getZExtValue();
    assert(Info.offset == 1 || Info.offset == 2 || Info.offset == 4 || Info.offset == 8);
    Info.flags = MachineMemOperand::MOLoad;
    Info.align = Align(4);
    return true;
  }
  case Intrinsic::tpc_ld_g_partial_inc:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(cast<StructType>(I.getType())->getElementType(0));
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = cast<ConstantInt>(I.getArgOperand(1))->getZExtValue();
    assert(Info.offset == 1 || Info.offset == 2 || Info.offset == 4 || Info.offset == 8);
    Info.flags = MachineMemOperand::MOLoad;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_ld_g_int5_inc:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(cast<StructType>(I.getType())->getElementType(0));
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = cast<ConstantInt>(I.getArgOperand(1))->getZExtValue();
    assert(Info.offset == 1 || Info.offset == 2 || Info.offset == 4 || Info.offset == 8);
    Info.flags = MachineMemOperand::MOLoad;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_st_g:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(I.getArgOperand(1)->getType());
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = 0;
    Info.flags = MachineMemOperand::MOStore;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_st_g_inc:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(I.getArgOperand(1)->getType());
    Info.ptrVal = I.getArgOperand(0);
    Info.offset = cast<ConstantInt>(I.getArgOperand(2))->getZExtValue();
    assert(Info.offset == 1 || Info.offset == 2 || Info.offset == 4 || Info.offset == 8);
    Info.flags = MachineMemOperand::MOStore;
    Info.align = Align(4);
    return true;
  case Intrinsic::tpc_st_tnsr_s_hwr:
  case Intrinsic::tpc_st_tnsr_s_hwr_rmw:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(I.getArgOperand(2)->getType());
    Info.offset = 0;
    Info.flags = MachineMemOperand::MOStore;
    Info.align = Align(4);
    return true;
  }

  return false;
}

static EVT getHalfEVT(EVT evt) {
  if (evt == MVT::v128f32) return MVT::v64f32;
  if (evt == MVT::v128i32) return MVT::v64i32;
  if (evt == MVT::v256i16) return MVT::v128i16;
  if (evt == MVT::v256bf16) return MVT::v128bf16;
  if (evt == MVT::v256f16) return MVT::v128f16;
  if (evt == MVT::v256f32) return MVT::v128f32;
  if (evt == MVT::v256i32) return MVT::v128i32;
  if (evt == MVT::v512i8) return MVT::v256i8;
  assert(false && "Not supported half");
  return MVT::Other;
}

static EVT getQuarterEVT(EVT evt) {
  if (evt == MVT::v256f32) return MVT::v64f32;
  if (evt == MVT::v256i32) return MVT::v64i32;
  assert(false && "Not supported quarter");
  return MVT::Other;
}

SmallVector<SDValue, 4>
TPCTargetLowering::extend_8_to_32(SDValue Op, SelectionDAG &DAG,
                                  const SDLoc &DL, uint64_t DataType) const {
  EVT ResultVT(MVT::v64i32);
  const SDValue &Src = Op.getOperand(0);
  auto SrcType = Src.getValueType();
  SmallVector<SDValue, 4> ExtArg;
  const FeatureBitset &Features = Subtarget->getFeatureBits();

  if (Features[TPC::FeatureGen2Plus] || Features[TPC::FeatureGen2BPlus]) {
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = (int64)v_i8_unpack_v(x, 0/*x00*/, e_group_0,
    // e_every_forth_element, e_lower_half_group);
    Ops1[2] =
        DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
    Ops1[3] = DAG.getConstant(0, DL, ResultVT);                  // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1);             // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops1);
    SDValue unpack1 = SDValue(Node1, 0);

    auto BitCastUnpack1 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack1);

    // unpack-2
    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = Src;                                           // Source.
    Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t1 = (int64)v_i8_unpack_v(x, 0/*x01*/, e_group_0, e_every_forth_element,
    // e_higher_half_group);
    Ops2[2] = DAG.getTargetConstant((TPCII::SW_STRIDE_4 | // Switch
                                     TPCII::SW_GROUP_HALF_1),
                                    DL, MVT::i32);
    Ops2[3] = DAG.getConstant(0, DL, ResultVT);      // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops2);
    SDValue unpack2 = SDValue(Node2, 0);

    auto BitCastUnpack2 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack2);

    // unpack-3
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = Src;                                           // Source.
    Ops3[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t2 = (int64)v_i8_unpack_v(x, 0/*x02*/, e_group_1, e_every_forth_element,
    // e_lower_half_group);
    Ops3[2] = DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                                     TPCII::SW_STRIDE_4),
                                    DL, MVT::i32);
    Ops3[3] = DAG.getConstant(0, DL, ResultVT);      // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node3 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops3);
    SDValue unpack3 = SDValue(Node3, 0);

    auto BitCastUnpack3 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack3);

    // unpack-4
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = Src;                                           // Source.
    Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = (int64)v_i8_unpack_v(x, 0/*x03*/, e_group_1,
    // e_every_forth_element, e_higher_half_group);
    Ops4[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                               TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_HALF_1),
                              DL, MVT::i32);
    Ops4[3] = DAG.getConstant(0, DL, ResultVT);      // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops4);
    SDValue unpack4 = SDValue(Node4, 0);

    auto BitCastUnpack4 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack4);

    SDValue Temp0 = BitCastUnpack1;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops5(7);
    Ops5[0] = BitCastUnpack2;                          // Source.
    Ops5[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_all_v(y.v2, 0xFFFFFFFF, y.v1, 1, 0, 3, 2,
    // 0b00, 0b11, 0b00, 0b11);
    Ops5[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops5[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops5[4] = BitCastUnpack1;                        // Income.
    Ops5[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops5[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node5 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops5);
    SDValue mdg1 = SDValue(Node5, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops6(7);
    Ops6[0] = Temp0;                                   // Source.
    Ops6[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_all_v(t0, 0xFFFFFFFF, y.v2, 1, 0, 3, 2, 0b11,
    // 0b00, 0b11, 0b00);
    Ops6[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops6[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops6[4] = BitCastUnpack2;                        // Income.
    Ops6[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops6[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node6 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops6);
    SDValue mdg2 = SDValue(Node6, 0);

    SDValue Temp1 = BitCastUnpack3;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops7(7);
    Ops7[0] = BitCastUnpack4;                          // Source.
    Ops7[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_all_v(y.v4, 0xFFFFFFFF, y.v3, 1, 0, 3, 2,
    // 0b00, 0b11, 0b00, 0b11);
    Ops7[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops7[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops7[4] = BitCastUnpack3;                        // Income.
    Ops7[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops7[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node7 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops7);
    SDValue mdg3 = SDValue(Node7, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops8(7);
    Ops8[0] = Temp1;                                   // Source.
    Ops8[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_all_v(t1, 0xFFFFFFFF, y.v4, 1, 0, 3, 2, 0b11,
    // 0b00, 0b11, 0b00);
    Ops8[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops8[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops8[4] = BitCastUnpack4;                        // Income.
    Ops8[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops8[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node8 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops8);
    SDValue mdg4 = SDValue(Node8, 0);

    Temp0 = mdg1;
    Temp1 = mdg2;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops9(7);
    Ops9[0] = mdg3;                                    // Source.
    Ops9[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_all_v(y.v3, 0xFFFFFFFF, y.v1, 2, 3, 0, 1,
    // 0b00, 0b00, 0b11, 0b11);
    Ops9[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops9[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops9[4] = mdg1;                                  // Income.
    Ops9[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops9[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node9 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops9);
    SDValue mdg5 = SDValue(Node9, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops10(7);
    Ops10[0] = mdg4;                                    // Source.
    Ops10[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_all_v(y.v4, 0xFFFFFFFF, y.v2, 2, 3, 0, 1,
    // 0b00, 0b00, 0b11, 0b11);
    Ops10[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops10[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops10[4] = mdg2;                                  // Income.
    Ops10[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops10[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node10 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops10);
    SDValue mdg6 = SDValue(Node10, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops11(7);
    Ops11[0] = Temp0;                                   // Source.
    Ops11[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_all_v(t0, 0xFFFFFFFF, y.v3, 2, 3, 0, 1, 0b11,
    // 0b11, 0b00, 0b00);
    Ops11[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops11[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops11[4] = mdg3;                                  // Income.
    Ops11[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops11[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node11 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops11);
    SDValue mdg7 = SDValue(Node11, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops12(7);
    Ops12[0] = Temp1;                                   // Source.
    Ops12[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_all_v(t1, 0xFFFFFFFF, y.v4, 2, 3, 0, 1, 0b11,
    // 0b11, 0b00, 0b00);
    Ops12[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops12[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops12[4] = mdg4;                                  // Income.
    Ops12[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops12[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node12 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops12);
    SDValue mdg8 = SDValue(Node12, 0);
    ExtArg.push_back(mdg5);
    ExtArg.push_back(mdg6);
    ExtArg.push_back(mdg7);
    ExtArg.push_back(mdg8);
  } else {
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = (int64)v_i8_unpack_v(x, 0/*x00*/, e_group_0,
    // e_every_forth_element, e_lower_half_group);
    Ops1[2] =
        DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
    Ops1[3] = DAG.getUNDEF(ResultVT);                            // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1);             // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops1);
    SDValue unpack1 = SDValue(Node1, 0);

    // unpack-2
    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = Src;                                           // Source.
    Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t1 = (int64)v_i8_unpack_v(x, 0/*x01*/, e_group_0, e_every_forth_element,
    // e_higher_half_group);
    Ops2[2] = DAG.getTargetConstant((TPCII::SW_STRIDE_4 | // Switch
                                     TPCII::SW_GROUP_HALF_1),
                                    DL, MVT::i32);
    Ops2[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops2);
    SDValue unpack2 = SDValue(Node2, 0);

    // unpack-3
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = Src;                                           // Source.
    Ops3[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t2 = (int64)v_i8_unpack_v(x, 0/*x02*/, e_group_1, e_every_forth_element,
    // e_lower_half_group);
    Ops3[2] = DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                                     TPCII::SW_STRIDE_4),
                                    DL, MVT::i32);
    Ops3[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node3 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops3);
    SDValue unpack3 = SDValue(Node3, 0);

    // unpack-4
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = Src;                                           // Source.
    Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = (int64)v_i8_unpack_v(x, 0/*x03*/, e_group_1,
    // e_every_forth_element, e_higher_half_group);
    Ops4[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                               TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_HALF_1),
                              DL, MVT::i32);
    Ops4[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops4);
    SDValue unpack4 = SDValue(Node4, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops5(6);
    Ops5[0] = unpack2;                                       // Source.
    Ops5[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v1, 0, 1, 1, 1);
    Ops5[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops5[3] = unpack1;                               // Income.
    Ops5[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node5 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
    SDValue mdg1 = SDValue(Node5, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops6(6);
    Ops6[0] = unpack3;                                       // Source.
    Ops6[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v1, 0, 2, 1, 1);
    Ops6[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops6[3] = mdg1;                                  // Income.
    Ops6[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node6 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
    SDValue mdg2 = SDValue(Node6, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops7(6);
    Ops7[0] = unpack4;                                       // Source.
    Ops7[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v1, 0, 3, 1, 1);
    Ops7[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops7[3] = mdg2;                                  // Income.
    Ops7[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node7 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
    SDValue mdg3 = SDValue(Node7, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops8(6);
    Ops8[0] = unpack1;                                       // Source.
    Ops8[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v2, 1, 0, 1, 1);
    Ops8[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops8[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops8[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops8[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node8 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops8);
    SDValue mdg4 = SDValue(Node8, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops9(6);
    Ops9[0] = unpack2;                                       // Source.
    Ops9[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v2, 1, 1, 1, 1);
    Ops9[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops9[3] = mdg4;                                  // Income.
    Ops9[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node9 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
    SDValue mdg5 = SDValue(Node9, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops10(6);
    Ops10[0] = unpack3;                                       // Source.
    Ops10[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v2, 1, 2, 1, 1);
    Ops10[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops10[3] = mdg5;                                  // Income.
    Ops10[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops10[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node10 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops10);
    SDValue mdg6 = SDValue(Node10, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops11(6);
    Ops11[0] = unpack4;                                       // Source.
    Ops11[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v2, 1, 3, 1, 1);
    Ops11[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops11[3] = mdg6;                                  // Income.
    Ops11[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops11[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node11 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops11);
    SDValue mdg7 = SDValue(Node11, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops12(6);
    Ops12[0] = unpack1;                                       // Source.
    Ops12[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v3, 2, 0, 1, 1);
    Ops12[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops12[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops12[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops12[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node12 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops12);
    SDValue mdg8 = SDValue(Node12, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops13(6);
    Ops13[0] = unpack2;                                       // Source.
    Ops13[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v3, 2, 1, 1, 1);
    Ops13[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops13[3] = mdg8;                                  // Income.
    Ops13[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops13[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node13 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops13);
    SDValue mdg9 = SDValue(Node13, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops14(6);
    Ops14[0] = unpack3;                                       // Source.
    Ops14[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v3, 2, 2, 1, 1);
    Ops14[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops14[3] = mdg9;                                  // Income.
    Ops14[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops14[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node14 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops14);
    SDValue mdg10 = SDValue(Node14, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops15(6);
    Ops15[0] = unpack4;                                       // Source.
    Ops15[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v3, 2, 3, 1, 1);
    Ops15[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops15[3] = mdg10;                                 // Income.
    Ops15[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops15[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node15 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops15);
    SDValue mdg11 = SDValue(Node15, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops16(6);
    Ops16[0] = unpack1;                                       // Source.
    Ops16[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v4, 3, 0, 1, 1);
    Ops16[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops16[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops16[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops16[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node16 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops16);
    SDValue mdg12 = SDValue(Node16, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops17(6);
    Ops17[0] = unpack2;                                       // Source.
    Ops17[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v4, 3, 1, 1, 1);
    Ops17[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops17[3] = mdg12;                                 // Income.
    Ops17[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops17[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node17 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops17);
    SDValue mdg13 = SDValue(Node17, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops18(6);
    Ops18[0] = unpack3;                                       // Source.
    Ops18[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v4, 3, 2, 1, 1);
    Ops18[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops18[3] = mdg13;                                 // Income.
    Ops18[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops18[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node18 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops18);
    SDValue mdg14 = SDValue(Node18, 0);

    ExtArg.push_back(mdg3);
    ExtArg.push_back(mdg7);
    ExtArg.push_back(mdg11);
    ExtArg.push_back(mdg14);
  }

  return ExtArg;
}


SmallVector<SDValue, 4>
TPCTargetLowering::Extend_8_to_32(EVT ResultVT, SDValue Src, SelectionDAG &DAG,
                                  const SDLoc &DL, unsigned DataType) const {
  auto SrcType = Src.getValueType();
  SmallVector<SDValue, 4> ExtArg;
  const FeatureBitset &Features = Subtarget->getFeatureBits();

 if (Features[TPC::FeatureGen2Plus] || Features[TPC::FeatureGen2BPlus]) {
    EVT IncomeType = MVT::v256i8;
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = (int64)v_i8_unpack_v(x, 0/*x00*/, e_group_0,
    // e_every_forth_element, e_lower_half_group);
    Ops1[2] =
        DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
    Ops1[3] = DAG.getConstant(0, DL, IncomeType);                // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1);             // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops1);
    SDValue unpack1 = SDValue(Node1, 0);

    auto BitCastUnpack1 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack1);

    // unpack-2
    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = Src;                                           // Source.
    Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t1 = (int64)v_i8_unpack_v(x, 0/*x01*/, e_group_0, e_every_forth_element,
    // e_higher_half_group);
    Ops2[2] = DAG.getTargetConstant((TPCII::SW_STRIDE_4 | // Switch
                                     TPCII::SW_GROUP_HALF_1),
                                    DL, MVT::i32);
    Ops2[3] = DAG.getConstant(0, DL, IncomeType);    // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops2);
    SDValue unpack2 = SDValue(Node2, 0);

    auto BitCastUnpack2 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack2);

    // unpack-3
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = Src;                                           // Source.
    Ops3[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t2 = (int64)v_i8_unpack_v(x, 0/*x02*/, e_group_1, e_every_forth_element,
    // e_lower_half_group);
    Ops3[2] = DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                                     TPCII::SW_STRIDE_4),
                                    DL, MVT::i32);
    if (ResultVT.isFloatingPoint()) 
      Ops3[3] = DAG.getConstantFP(0.0, DL, ResultVT); // Income.
    else
      Ops3[3] = DAG.getConstant(0, DL, ResultVT);      // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node3 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops3);
    SDValue unpack3 = SDValue(Node3, 0);

    auto BitCastUnpack3 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack3);

    // unpack-4
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = Src;                                           // Source.
    Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = (int64)v_i8_unpack_v(x, 0/*x03*/, e_group_1,
    // e_every_forth_element, e_higher_half_group);
    Ops4[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                               TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_HALF_1),
                              DL, MVT::i32);
    Ops4[3] = DAG.getConstant(0, DL, IncomeType);    // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops4);
    SDValue unpack4 = SDValue(Node4, 0);

    auto BitCastUnpack4 = DAG.getNode(ISD::BITCAST, DL, ResultVT, unpack4);

    SDValue Temp0 = BitCastUnpack1;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops5(7);
    Ops5[0] = BitCastUnpack2;                          // Source.
    Ops5[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_all_v(y.v2, 0xFFFFFFFF, y.v1, 1, 0, 3, 2,
    // 0b00, 0b11, 0b00, 0b11);
    Ops5[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops5[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops5[4] = BitCastUnpack1;                        // Income.
    Ops5[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops5[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node5 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops5);
    SDValue mdg1 = SDValue(Node5, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops6(7);
    Ops6[0] = Temp0;                                   // Source.
    Ops6[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_all_v(t0, 0xFFFFFFFF, y.v2, 1, 0, 3, 2, 0b11,
    // 0b00, 0b11, 0b00);
    Ops6[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops6[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops6[4] = BitCastUnpack2;                        // Income.
    Ops6[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops6[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node6 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops6);
    SDValue mdg2 = SDValue(Node6, 0);

    SDValue Temp1 = BitCastUnpack3;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops7(7);
    Ops7[0] = BitCastUnpack4;                          // Source.
    Ops7[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_all_v(y.v4, 0xFFFFFFFF, y.v3, 1, 0, 3, 2,
    // 0b00, 0b11, 0b00, 0b11);
    Ops7[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops7[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops7[4] = BitCastUnpack3;                        // Income.
    Ops7[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops7[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node7 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops7);
    SDValue mdg3 = SDValue(Node7, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops8(7);
    Ops8[0] = Temp1;                                   // Source.
    Ops8[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_all_v(t1, 0xFFFFFFFF, y.v4, 1, 0, 3, 2, 0b11,
    // 0b00, 0b11, 0b00);
    Ops8[2] = DAG.getTargetConstant(
        (((1 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops8[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops8[4] = BitCastUnpack4;                        // Income.
    Ops8[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops8[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node8 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops8);
    SDValue mdg4 = SDValue(Node8, 0);

    Temp0 = mdg1;
    Temp1 = mdg2;
    // mov-dual-group
    SmallVector<SDValue, 8> Ops9(7);
    Ops9[0] = mdg3;                                    // Source.
    Ops9[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_all_v(y.v3, 0xFFFFFFFF, y.v1, 2, 3, 0, 1,
    // 0b00, 0b00, 0b11, 0b11);
    Ops9[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                               // Switch.
    Ops9[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops9[4] = mdg1;                                  // Income.
    Ops9[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops9[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node9 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops9);
    SDValue mdg5 = SDValue(Node9, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops10(7);
    Ops10[0] = mdg4;                                    // Source.
    Ops10[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_all_v(y.v4, 0xFFFFFFFF, y.v2, 2, 3, 0, 1,
    // 0b00, 0b00, 0b11, 0b11);
    Ops10[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
         (3 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops10[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops10[4] = mdg2;                                  // Income.
    Ops10[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops10[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node10 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops10);
    SDValue mdg6 = SDValue(Node10, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops11(7);
    Ops11[0] = Temp0;                                   // Source.
    Ops11[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_all_v(t0, 0xFFFFFFFF, y.v3, 2, 3, 0, 1, 0b11,
    // 0b11, 0b00, 0b00);
    Ops11[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops11[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops11[4] = mdg3;                                  // Income.
    Ops11[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops11[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node11 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops11);
    SDValue mdg7 = SDValue(Node11, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops12(7);
    Ops12[0] = Temp1;                                   // Source.
    Ops12[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_all_v(t1, 0xFFFFFFFF, y.v4, 2, 3, 0, 1, 0b11,
    // 0b11, 0b00, 0b00);
    Ops12[2] = DAG.getTargetConstant(
        (((2 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
          (0 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT)) |
         (3 << TPCII::SW_WEG0_SHIFT) | (3 << TPCII::SW_WEG1_SHIFT) |
         (0 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
         (TPCII::SW_MDG_TYPE_ALL)),
        DL, MVT::i32);                                // Switch.
    Ops12[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops12[4] = mdg4;                                  // Income.
    Ops12[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops12[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node12 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops12);
    SDValue mdg8 = SDValue(Node12, 0);
    ExtArg.push_back(mdg5);
    ExtArg.push_back(mdg6);
    ExtArg.push_back(mdg7);
    ExtArg.push_back(mdg8);
  } else {
   
   assert(false && "transposition is not suppported for this platform");
#if 0
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = (int64)v_i8_unpack_v(x, 0/*x00*/, e_group_0,
    // e_every_forth_element, e_lower_half_group);
    Ops1[2] =
        DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
    Ops1[3] = DAG.getUNDEF(ResultVT);                            // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1);             // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops1);
    SDValue unpack1 = SDValue(Node1, 0);

    // unpack-2
    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = Src;                                           // Source.
    Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t1 = (int64)v_i8_unpack_v(x, 0/*x01*/, e_group_0, e_every_forth_element,
    // e_higher_half_group);
    Ops2[2] = DAG.getTargetConstant((TPCII::SW_STRIDE_4 | // Switch
                                     TPCII::SW_GROUP_HALF_1),
                                    DL, MVT::i32);
    Ops2[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops2);
    SDValue unpack2 = SDValue(Node2, 0);

    // unpack-3
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = Src;                                           // Source.
    Ops3[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // t2 = (int64)v_i8_unpack_v(x, 0/*x02*/, e_group_1, e_every_forth_element,
    // e_lower_half_group);
    Ops3[2] = DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                                     TPCII::SW_STRIDE_4),
                                    DL, MVT::i32);
    Ops3[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node3 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops3);
    SDValue unpack3 = SDValue(Node3, 0);

    // unpack-4
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = Src;                                           // Source.
    Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = (int64)v_i8_unpack_v(x, 0/*x03*/, e_group_1,
    // e_every_forth_element, e_higher_half_group);
    Ops4[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_1 | // Switch
                               TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_HALF_1),
                              DL, MVT::i32);
    Ops4[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops4);
    SDValue unpack4 = SDValue(Node4, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops5(6);
    Ops5[0] = unpack2;                                       // Source.
    Ops5[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v1, 0, 1, 1, 1);
    Ops5[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops5[3] = unpack1;                               // Income.
    Ops5[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node5 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
    SDValue mdg1 = SDValue(Node5, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops6(6);
    Ops6[0] = unpack3;                                       // Source.
    Ops6[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v1, 0, 2, 1, 1);
    Ops6[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops6[3] = mdg1;                                  // Income.
    Ops6[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node6 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
    SDValue mdg2 = SDValue(Node6, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops7(6);
    Ops7[0] = unpack4;                                       // Source.
    Ops7[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v1, 0, 3, 1, 1);
    Ops7[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops7[3] = mdg2;                                  // Income.
    Ops7[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node7 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
    SDValue mdg3 = SDValue(Node7, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops8(6);
    Ops8[0] = unpack1;                                       // Source.
    Ops8[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v2, 1, 0, 1, 1);
    Ops8[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops8[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops8[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops8[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node8 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops8);
    SDValue mdg4 = SDValue(Node8, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops9(6);
    Ops9[0] = unpack2;                                       // Source.
    Ops9[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v2, 1, 1, 1, 1);
    Ops9[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops9[3] = mdg4;                                  // Income.
    Ops9[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node9 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
    SDValue mdg5 = SDValue(Node9, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops10(6);
    Ops10[0] = unpack3;                                       // Source.
    Ops10[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v2, 1, 2, 1, 1);
    Ops10[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops10[3] = mdg5;                                  // Income.
    Ops10[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops10[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node10 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops10);
    SDValue mdg6 = SDValue(Node10, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops11(6);
    Ops11[0] = unpack4;                                       // Source.
    Ops11[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v2 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v2, 1, 3, 1, 1);
    Ops11[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops11[3] = mdg6;                                  // Income.
    Ops11[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops11[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node11 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops11);
    SDValue mdg7 = SDValue(Node11, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops12(6);
    Ops12[0] = unpack1;                                       // Source.
    Ops12[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v3, 2, 0, 1, 1);
    Ops12[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops12[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops12[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops12[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node12 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops12);
    SDValue mdg8 = SDValue(Node12, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops13(6);
    Ops13[0] = unpack2;                                       // Source.
    Ops13[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v3, 2, 1, 1, 1);
    Ops13[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops13[3] = mdg8;                                  // Income.
    Ops13[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops13[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node13 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops13);
    SDValue mdg9 = SDValue(Node13, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops14(6);
    Ops14[0] = unpack3;                                       // Source.
    Ops14[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v3, 2, 2, 1, 1);
    Ops14[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops14[3] = mdg9;                                  // Income.
    Ops14[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops14[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node14 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops14);
    SDValue mdg10 = SDValue(Node14, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops15(6);
    Ops15[0] = unpack4;                                       // Source.
    Ops15[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v3 = v_i32_mov_dual_group_v(y.v4, 0xFFFFFFFF, y.v3, 2, 3, 1, 1);
    Ops15[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops15[3] = mdg10;                                 // Income.
    Ops15[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops15[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node15 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops15);
    SDValue mdg11 = SDValue(Node15, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops16(6);
    Ops16[0] = unpack1;                                       // Source.
    Ops16[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t0, 0xFFFFFFFF, y.v4, 3, 0, 1, 1);
    Ops16[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops16[3] = DAG.getUNDEF(ResultVT);                // Income.
    Ops16[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops16[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node16 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops16);
    SDValue mdg12 = SDValue(Node16, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops17(6);
    Ops17[0] = unpack2;                                       // Source.
    Ops17[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t1, 0xFFFFFFFF, y.v4, 3, 1, 1, 1);
    Ops17[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops17[3] = mdg12;                                 // Income.
    Ops17[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops17[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node17 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops17);
    SDValue mdg13 = SDValue(Node17, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops18(6);
    Ops18[0] = unpack3;                                       // Source.
    Ops18[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v4 = v_i32_mov_dual_group_v(t2, 0xFFFFFFFF, y.v4, 3, 2, 1, 1);
    Ops18[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                      2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                      TPCII::SW_WR_LOWER_GROUP |
                                      TPCII::SW_WR_UPPER_GROUP),
                                     DL, MVT::i32);   // Switch.
    Ops18[3] = mdg13;                                 // Income.
    Ops18[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops18[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node18 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops18);
    SDValue mdg14 = SDValue(Node18, 0);

    ExtArg.push_back(mdg3);
    ExtArg.push_back(mdg7);
    ExtArg.push_back(mdg11);
    ExtArg.push_back(mdg14);
#endif
  }

  return ExtArg;
}

SmallVector<SDValue, 2>
TPCTargetLowering::Extend_8_to_16(EVT ResultVT, SDValue Src, SelectionDAG &DAG,
                                  const SDLoc &DL, unsigned DataType) const {
 EVT SrcVT = Src.getValueType();
 SmallVector<SDValue, 2> ExtArg;
  // unpack-1
  SmallVector<SDValue, 8> Ops1(6);
  Ops1[0] = Src;                                           // Source.
  Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
  // group0 = v_i8_unpack_v(x, 0, e_group_0, e_every_second_element,
  // e_lower_half_group);
  Ops1[2] = DAG.getTargetConstant(TPCII::SW_GROUP_0 | TPCII::SW_STRIDE_2 |
                                      TPCII::SW_GROUP_HALF_0,
                                  DL, MVT::i32);   // Switch.
  Ops1[3] = DAG.getUNDEF(SrcVT);                   // Income.
  Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcVT, Ops1);
  SDValue unpack1 = SDValue(Node1, 0);

  // unpack-2
  SmallVector<SDValue, 8> Ops2(6);
  Ops2[0] = Src;                                           // Source.
  Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
  // group1 = v_i8_unpack_v(x, 0, e_group_1, e_every_second_element,
  // e_lower_half_group);
  Ops2[2] = DAG.getTargetConstant(
      (TPCII::SW_GROUP_1 | TPCII::SW_STRIDE_2 | TPCII::SW_GROUP_HALF_0), DL,
      MVT::i32);                                   // Switch
  Ops2[3] = DAG.getUNDEF(SrcVT);                   // Income.
  Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcVT, Ops2);
  SDValue unpack2 = SDValue(Node2, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops5(6);
  Ops5[0] = unpack2;                                      // Source.
  Ops5[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 0, 1, 1, 1);
  Ops5[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops5[3] = unpack1;                              // Income.
  Ops5[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node5 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
  SDValue mdg1 = SDValue(Node5, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops6(6);
  Ops6[0] = unpack1;                                      // Source.
  Ops6[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v1, 1, 2, 1, 1);
  Ops6[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops6[3] = mdg1;                                  // Income.
  Ops6[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node6 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
  SDValue mdg2 = SDValue(Node6, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops7(6);
  Ops7[0] = unpack2;                                      // Source.
  Ops7[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 1, 3, 1, 1);
  Ops7[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops7[3] = mdg2;                                  // Income.
  Ops7[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node7 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
  SDValue mdg3 = SDValue(Node7, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops8(6);
  Ops8[0] = unpack1;                                      // Source.
  Ops8[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 2, 0, 1, 1);
  Ops8[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops8[3] = unpack2;                              // Income.
  Ops8[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops8[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node8 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops8);
  SDValue mdg4 = SDValue(Node8, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops9(6);
  Ops9[0] = mdg4;                                          // Source.
  Ops9[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v2 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v2, 2, 1, 1, 1);
  Ops9[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops9[3] = mdg4;                                  // Income.
  Ops9[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node9 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
  SDValue mdg5 = SDValue(Node9, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops10(6);
  Ops10[0] = unpack1;                                      // Source.
  Ops10[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 3, 2, 1, 1);
  Ops10[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops10[3] = mdg5;                                  // Income.
  Ops10[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops10[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node10 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops10);
  SDValue mdg6 = SDValue(Node10, 0);

  ExtArg.push_back(mdg3);
  ExtArg.push_back(mdg6);
  return ExtArg;
}

SmallVector<SDValue, 2>
TPCTargetLowering::extend_8_to_16(SDValue Op, SelectionDAG &DAG,
                                  const SDLoc &DL, uint64_t DataType,
                                  unsigned DstDataType) const {
  const SDValue &Src = Op.getOperand(0);
  EVT SrcVT = Src->getValueType(0);
  // EVT ResultVT(MVT::v128i16);
  EVT ResultVT = getHalfEVT(Op.getValueType());
  SmallVector<SDValue, 2> ExtArg;
  // unpack-1
  SmallVector<SDValue, 8> Ops1(6);
  Ops1[0] = Src;                                           // Source.
  Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
  // group0 = v_i8_unpack_v(x, 0, e_group_0, e_every_second_element,
  // e_lower_half_group);
  Ops1[2] = DAG.getTargetConstant(TPCII::SW_GROUP_0 | TPCII::SW_STRIDE_2 |
                                      TPCII::SW_GROUP_HALF_0,
                                  DL, MVT::i32);   // Switch.
  Ops1[3] = DAG.getUNDEF(SrcVT);                   // Income.
  Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcVT, Ops1);
  SDValue unpack1 = SDValue(Node1, 0);

  // unpack-2
  SmallVector<SDValue, 8> Ops2(6);
  Ops2[0] = Src;                                           // Source.
  Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
  // group1 = v_i8_unpack_v(x, 0, e_group_1, e_every_second_element,
  // e_lower_half_group);
  Ops2[2] = DAG.getTargetConstant(
      (TPCII::SW_GROUP_1 | TPCII::SW_STRIDE_2 | TPCII::SW_GROUP_HALF_0), DL,
      MVT::i32);                                   // Switch
  Ops2[3] = DAG.getUNDEF(SrcVT);                   // Income.
  Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcVT, Ops2);
  SDValue unpack2 = SDValue(Node2, 0);

  // convert-1
  // y.v1 = v_convert_i8_to_i16_v(group0);
  SmallVector<SDValue, 8> Ops3(6);
  Ops3[0] = unpack1;                                      // Source.
  Ops3[1] = DAG.getTargetConstant(DataType, DL, MVT::i8); // DataType.
  Ops3[2] = DAG.getTargetConstant(TPCII::SW_CSR | DstDataType, DL,
                                  MVT::i32);       // Switch.
  Ops3[3] = DAG.getUNDEF(ResultVT);                // Income.
  Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node3 =
      DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultVT, Ops3);
  SDValue convert1 = SDValue(Node3, 0);

  // convert-2
  // y.v2 = v_convert_i8_to_i16_v(group1);
  SmallVector<SDValue, 8> Ops4(6);
  Ops4[0] = unpack2;                                      // Source.
  Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i8); // DataType.
  Ops4[2] = DAG.getTargetConstant(DstDataType, DL,
                                  MVT::i32);       // Switch.
  Ops4[3] = DAG.getUNDEF(ResultVT);                // Income.
  Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node4 =
      DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultVT, Ops4);
  SDValue convert2 = SDValue(Node4, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops5(6);
  Ops5[0] = convert2;                                // Source.
  Ops5[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 0, 1, 1, 1);
  Ops5[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops5[3] = convert1;                              // Income.
  Ops5[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node5 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
  SDValue mdg1 = SDValue(Node5, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops6(6);
  Ops6[0] = convert1;                                // Source.
  Ops6[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v1 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v1, 1, 2, 1, 1);
  Ops6[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops6[3] = mdg1;                                  // Income.
  Ops6[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node6 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
  SDValue mdg2 = SDValue(Node6, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops7(6);
  Ops7[0] = convert2;                                // Source.
  Ops7[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 1, 3, 1, 1);
  Ops7[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops7[3] = mdg2;                                  // Income.
  Ops7[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node7 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
  SDValue mdg3 = SDValue(Node7, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops8(6);
  Ops8[0] = convert1;                                // Source.
  Ops8[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 2, 0, 1, 1);
  Ops8[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops8[3] = convert2;                              // Income.
  Ops8[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops8[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node8 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops8);
  SDValue mdg4 = SDValue(Node8, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops9(6);
  Ops9[0] = mdg4;                                    // Source.
  Ops9[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v2 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v2, 2, 1, 1, 1);
  Ops9[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops9[3] = mdg4;                                  // Income.
  Ops9[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node9 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
  SDValue mdg5 = SDValue(Node9, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops10(6);
  Ops10[0] = convert1;                                // Source.
  Ops10[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 3, 2, 1, 1);
  Ops10[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops10[3] = mdg5;                                  // Income.
  Ops10[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops10[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node10 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops10);
  SDValue mdg6 = SDValue(Node10, 0);

  ExtArg.push_back(mdg3);
  ExtArg.push_back(mdg6);

  return ExtArg;
}

SmallVector<SDValue, 2>
TPCTargetLowering::extend_16_to_32(SDValue Op, SelectionDAG &DAG,
                                   const SDLoc &DL, uint64_t DataType) const {
  EVT ResultVT = Op->getValueType(0);
  auto IsIntrinsicNode = Op.getOpcode() == ISD::INTRINSIC_WO_CHAIN;
  const SDValue &Src = IsIntrinsicNode ? Op.getOperand(1) : Op.getOperand(0);
  auto SrcType = Src.getValueType();
  SmallVector<SDValue, 2> ExtArg;
  SDValue Temp = Src;
  if (SrcType == MVT::v128bf16) {
    Temp = DAG.getNode(ISD::BITCAST, DL, MVT::v128i16, Src);
  } else if (SrcType == MVT::v128f16) {
    if (Subtarget->hasGrecoISA() || Subtarget->hasGaudi2ISA()) {
      Temp = DAG.getNode(ISD::BITCAST, DL, MVT::v128i16, Src);
    } else {
      assert(false && "Unsupported source type");
    }
  }
  if (Subtarget->hasGen2Plus()) {
    // mov-group
    SmallVector<SDValue, 8> Ops(6);
    Ops[0] = Temp;                                    // Source.
    Ops[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_EN | TPCII::SW_DUAL_GROUP_EN),
                              DL, MVT::i32);        // Switch.
    Ops[3] = DAG.getUNDEF(MVT::v128i16);            // Income.
    Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node =
        DAG.getMachineNode(TPC::MOV_GROUP_vpu_vp, DL, MVT::v128i16, Ops);
    SDValue mdg = SDValue(Node, 0);

    // mov-dual-group
    // mov_dg.all sdg0=0 sdg1=2 sdg2=1 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=3 %V8,
    // %V1, -0x1, %SP0;
    SmallVector<SDValue, 8> Ops1(7);
    Ops1[0] = Temp;                                    // Source.
    Ops1[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops1[2] = DAG.getTargetConstant(
        (((0 << TPCII::SW_SDG0_SHIFT) | (2 << TPCII::SW_SDG1_SHIFT) |
          (1 << TPCII::SW_SDG2_SHIFT) | (3 << TPCII::SW_SDG3_SHIFT) |
          (3 << TPCII::SW_WEG0_SHIFT) | (2 << TPCII::SW_WEG1_SHIFT) |
          (1 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT)) |
         TPCII::SW_MDG_TYPE_ALL),
        DL, MVT::i32);                               // Switch.
    Ops1[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops1[4] = DAG.getUNDEF(MVT::v128i16);            // Income.
    Ops1[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops1[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node1 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, MVT::v128i16, Ops1);
    SDValue mdg1 = SDValue(Node1, 0);

    // mov-dual-group
    // mov_dg.all sdg0=2 sdg1=0 sdg2=3 sdg3=1 weg0=2 weg1=1 weg2=2 weg3=1 %V5,
    // %V4, -0x1, %SP0;
    SmallVector<SDValue, 8> Ops2(7);
    Ops2[0] = mdg;                                     // Source.
    Ops2[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops2[2] = DAG.getTargetConstant(                   // 0,
        (((2 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT) |
          (2 << TPCII::SW_WEG0_SHIFT) | (1 << TPCII::SW_WEG1_SHIFT) |
          (2 << TPCII::SW_WEG2_SHIFT) | (1 << TPCII::SW_WEG3_SHIFT)) |
         TPCII::SW_MDG_TYPE_ALL),
        DL, MVT::i32);                               // Switch.
    Ops2[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops2[4] = mdg1;                                  // Income.
    Ops2[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, MVT::v128i16, Ops2);
    SDValue mdg2 = SDValue(Node2, 0);

    // unpack-1
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = mdg2;                                          // Source.
    Ops3[1] =
        DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops3[2] =
        DAG.getTargetConstant(0, DL, MVT::i32); // Switch.
    Ops3[3] = DAG.getConstant(0, DL, MVT::v128i16);          // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);            // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1);         // Polarity.
    MachineSDNode *Node3 =
        DAG.getMachineNode(TPC::UNPACKp, DL, MVT::v128i16, Ops3);
    SDValue mdg3 = SDValue(Node3, 0);

    // unpack-2
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = mdg2;                                          // Source.
    Ops4[1] =
        DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops4[2] = DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32);              // Switch.
    Ops4[3] = DAG.getConstant(0, DL, MVT::v128i16);                  // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 =
        DAG.getMachineNode(TPC::UNPACKp, DL, MVT::v128i16, Ops4);
    SDValue mdg4 = SDValue(Node4, 0);

    if (SrcType == MVT::v128bf16) {
      mdg3 = DAG.getNode(ISD::BITCAST, DL, MVT::v128bf16, mdg3);
      mdg4 = DAG.getNode(ISD::BITCAST, DL, MVT::v128bf16, mdg4);
    } else if (SrcType == MVT::v128f16) {
      if (Subtarget->hasGrecoISA()||Subtarget->hasGaudi2ISA()) {
        mdg3 = DAG.getNode(ISD::BITCAST, DL, MVT::v128f16, mdg3);
        mdg4 = DAG.getNode(ISD::BITCAST, DL, MVT::v128f16, mdg4);
      } else {
        assert(false && "Unsupported source type");
      }
    }
    ExtArg.push_back(mdg3);
    ExtArg.push_back(mdg4);
  } else {
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops1[2] = DAG.getTargetConstant(0, DL, MVT::i32);        // Switch.
    Ops1[3] = DAG.getUNDEF(ResultVT);                        // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);            // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1);         // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops1);
    SDValue unpack1 = SDValue(Node1, 0);

    // unpack-2
    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = Src;                                           // Source.
    Ops2[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops2[2] =
        DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
    Ops2[3] = DAG.getUNDEF(ResultVT);                                // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 = DAG.getMachineNode(TPC::UNPACKp, DL, ResultVT, Ops2);
    SDValue unpack2 = SDValue(Node2, 0);

    // t_h = v_i16_mov_v(y.v1);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = unpack2;                                       // Source.
    Ops4[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops4[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops4[3] = unpack1;                               // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops4);
    SDValue mdg1 = SDValue(Node4, 0);

    SmallVector<SDValue, 8> Ops5(6);
    Ops5[0] = unpack1;                                       // Source.
    Ops5[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops5[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops5[3] = mdg1;                                  // Income.
    Ops5[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node5 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
    SDValue mdg2 = SDValue(Node5, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops6(6);
    Ops6[0] = unpack2;                                       // Source.
    Ops6[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops6[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops6[3] = mdg2;                                  // Income.
    Ops6[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node6 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
    SDValue mdg3 = SDValue(Node6, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops7(6);
    Ops7[0] = unpack1;                                       // Source.
    Ops7[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops7[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops7[3] = unpack2;                               // Income.
    Ops7[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node7 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
    SDValue mdg4 = SDValue(Node7, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops8(6);
    Ops8[0] = mdg4;                                          // Source.
    Ops8[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops8[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops8[3] = mdg4;                                  // Income.
    Ops8[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops8[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node8 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops8);
    SDValue mdg5 = SDValue(Node8, 0);

    // mov-dual-group
    SmallVector<SDValue, 8> Ops9(6);
    Ops9[0] = unpack1;                                       // Source.
    Ops9[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    Ops9[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                     2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                     TPCII::SW_WR_LOWER_GROUP |
                                     TPCII::SW_WR_UPPER_GROUP),
                                    DL, MVT::i32);   // Switch.
    Ops9[3] = mdg5;                                  // Income.
    Ops9[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node9 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
    SDValue mdg6 = SDValue(Node9, 0);

    ExtArg.push_back(mdg3);
    ExtArg.push_back(mdg6);
  }

  return ExtArg;
}

SDValue TPCTargetLowering::lowerSIGN_EXTEND(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const SDValue &Src = Op.getOperand(0);

  if (Op.getValueType() == MVT::v128i32) {
    if (Src.getValueType() == MVT::v128i16) {
      auto ExtArg = extend_16_to_32(Op, DAG, DL, TPCII::OpType::INT16);
      SDValue Src0 = helperConvertvvpNodeCreate(
          ExtArg[0], DL, DAG, TPCII::OpType::INT16,
          TPCII::SW_CSR | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_INT32,
          MVT::v64i32);
      SDValue Src1 = helperConvertvvpNodeCreate(
          ExtArg[1], DL, DAG, TPCII::OpType::INT16,
          TPCII::SW_CSR | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_INT32,
          MVT::v64i32);
      return createTuple({Src0, Src1}, DAG);

    } else {
      assert(false && "Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i16) {
    if (Src.getValueType() == MVT::v256i8) {
      assert(getHalfEVT(Op.getValueType()) == MVT::v128i16);
      auto ExtArg = extend_8_to_16(Op, DAG, DL, TPCII::OpType::INT8,
                                   TPCII::SW_TO_INT16);
      return createTuple(ExtArg, DAG);
    } else {
      assert(false && "Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i32) {
    if (Src.getValueType() == MVT::v256i8) {
      // SmallVector<SDValue, 4> BitCastVec;
      auto ExtArg = extend_8_to_32(Op, DAG, DL, TPCII::OpType::INT8);
      return createTuple(ExtArg, DAG);
    } else if (Src.getValueType() == MVT::v256i16) {
      SDValue S0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v128i16, Src);
      SDValue S1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v128i16, Src);

      SDValue Ext0 = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v128i32, S0Val);
      SDValue Ext1 = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v128i32, S1Val);

      SDValue Ext00 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext0);
      SDValue Ext01 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext0);
      SDValue Ext10 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext1);
      SDValue Ext11 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext1);

      return createTuple({Ext00, Ext01, Ext10, Ext11}, DAG);
    } 
    else {
      assert(false && "Unsupported source type");
    }
  } else {
    assert(false && "Unsupported target type");
  }
  return SDValue();
}

SDValue TPCTargetLowering::lowerZERO_EXTEND(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const SDValue &Src = Op.getOperand(0);

  if (Op.getValueType() == MVT::v128i32) {
    if (Src.getValueType() == MVT::v128i16) {
      auto ExtArg = extend_16_to_32(Op, DAG, DL, TPCII::OpType::UINT16);

      SDValue Conv0 = helperConvertvvpNodeCreate(
          ExtArg[0], DL, DAG, TPCII::OpType::UINT16,
          TPCII::SW_CSR | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_UINT32,
          MVT::v64i32);
      SDValue Conv1 = helperConvertvvpNodeCreate(
          ExtArg[1], DL, DAG, TPCII::OpType::UINT16,
          TPCII::SW_CSR | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_UINT32,
          MVT::v64i32);

      SDValue Src0 = helperANDvipNodeCreate(
          Conv0, DL, DAG, TPCII::OpType::UINT32, 0x0000FFFF, MVT::v64i32);
      SDValue Src1 = helperANDvipNodeCreate(
          Conv1, DL, DAG, TPCII::OpType::UINT32, 0x0000FFFF, MVT::v64i32);

      return createTuple({Src0, Src1}, DAG);
    } else {
      assert(false && "Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i16) {
    if (Src.getValueType() == MVT::v256i8) {
      // TODO : adapt the helper once generalized for not using AND/CONVERT.
      assert(getHalfEVT(Op.getValueType()) == MVT::v128i16);
      auto ExtArg = extend_8_to_16(Op, DAG, DL, TPCII::OpType::UINT8,
                                   TPCII::SW_TO_INT16);
      return createTuple(ExtArg, DAG);
    } else {
      assert(false && "Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i32) {
    if (Src.getValueType() == MVT::v256i8) {
      auto ExtArg = extend_8_to_32(Op, DAG, DL, TPCII::OpType::UINT8);
      return createTuple(ExtArg, DAG);
    } else if (Src.getValueType() == MVT::v256i16) {
      SDValue S0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v128i16, Src);
      SDValue S1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v128i16, Src);

      SDValue Ext0 = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::v128i32, S0Val);
      SDValue Ext1 = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::v128i32, S1Val);

      SDValue Ext00 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext0);
      SDValue Ext01 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext0);
      SDValue Ext10 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext1);
      SDValue Ext11 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext1);

      return createTuple({Ext00, Ext01, Ext10, Ext11}, DAG);
    } else {
      assert(false && "Unsupported source type");
    }
  } else {
    assert(false && "Unsupported source type");
  }
  return SDValue();
}

SDValue TPCTargetLowering::lowerFP_ROUND(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_CSR;

  // If Op is intrinsic, get the switch from it.
  auto IsCastIntrinsicWithRoundMode = NumOp == 3;
  if (IsCastIntrinsicWithRoundMode) {
    // Get rounding mode from 2nd argument of intrinsic
    InputSwitch = Op.getConstantOperandVal(NumOp - 1);
    LLVM_DEBUG(dbgs() << "Get switch from argument: " << InputSwitch << "\n");
  }
  // Get Src from 1st argument of intrinsic
  const SDValue &Src =
      IsCastIntrinsicWithRoundMode ? Op.getOperand(1) : Op.getOperand(0);

  // vector ops
  if ((Op.getValueType() == MVT::v128bf16) && (Subtarget->hasGen2Plus() || Subtarget->hasGaudiBISA())) {
    if (Src.getValueType() == MVT::v128f32) {

      SDValue N0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
      SDValue N1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);

      SDValue Src0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG, TPCII::OpType::FP32,
          TPCII::SW_RHNE | TPCII::SW_SINGLE_LANE_SRCB | TPCII::SW_TO_BF16,
          MVT::v128bf16);
      SDValue Src1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG, TPCII::OpType::FP32,
          TPCII::SW_RHNE | TPCII::SW_SINGLE_LANE_SRCB | TPCII::SW_TO_BF16,
          MVT::v128bf16);

      if (Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA() || Subtarget->hasGaudi2ISA())
        return truncate_32_to_16(DAG, Src0, Src1, DL, MVT::v128bf16,
                                 TPCII::OpType::INT16);
      else
        return truncate_32_to_16_goya(DAG, Src0, Src1, DL, MVT::v128bf16,
                                      TPCII::OpType::INT16);
    } else {
      assert(false && "Unsupported target type");
    }
  } else if (Op.getValueType() == MVT::v128f16 && Subtarget->hasGen3Plus()) {
    if (Src.getValueType() == MVT::v128f32) {

      SDValue N0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
      SDValue N1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);

      SDValue Src0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG, TPCII::OpType::FP32,
          InputSwitch | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_FP16,
          MVT::v128f16);
      SDValue Src1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG, TPCII::OpType::FP32,
          InputSwitch | TPCII::SW_SINGLE_LANE | TPCII::SW_TO_FP16,
          MVT::v128f16);

        return truncate_32_to_16_goya(DAG, Src0, Src1, DL, MVT::v128f16,
                                      TPCII::OpType::FP16);
    } else {
      assert(false && "Unsupported target type");
    }
  } else if (Op.getValueType() == MVT::v256bf16) {
    if (Src.getValueType() == MVT::v256f32) {
      if (Subtarget->hasGen2Plus() || Subtarget->hasGaudiBISA()) {
        SDValue S0Val =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
        SDValue S1Val =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);
        SDValue S2Val =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, MVT::v64f32, Src);
        SDValue S3Val =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, MVT::v64f32, Src);

        SDValue Src0 = helperConvertvvpNodeCreate(
            S0Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_BF16, MVT::v128bf16);
        SDValue Src1 = helperConvertvvpNodeCreate(
            S1Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_BF16, MVT::v128bf16);
        SDValue Src2 = helperConvertvvpNodeCreate(
            S2Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_BF16, MVT::v128bf16);
        SDValue Src3 = helperConvertvvpNodeCreate(
            S3Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_BF16, MVT::v128bf16);

        SDValue mdg0, mdg1;
        if (Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA() || Subtarget->hasGaudi2ISA()) {
          mdg0 = truncate_32_to_16(DAG, Src0, Src1, DL, MVT::v128bf16,
                                   TPCII::OpType::BF16);
          mdg1 = truncate_32_to_16(DAG, Src2, Src3, DL, MVT::v128bf16,
                                   TPCII::OpType::BF16);
        } else {
          mdg0 = truncate_32_to_16_goya(DAG, Src0, Src1, DL, MVT::v128bf16,
                                        TPCII::OpType::BF16);
          mdg1 = truncate_32_to_16_goya(DAG, Src2, Src3, DL, MVT::v128bf16,
                                        TPCII::OpType::BF16);
        }
        return createTuple({mdg0, mdg1}, DAG);
      } else {
        assert(false && "Unsupported vector type");
      }
    } else {
      assert(false && "Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256f16) {
    if (Subtarget->hasGrecoISA()) {
      if (Src.getValueType() == MVT::v256f32) {
        SDValue S0Val =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
        SDValue S1Val =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);
        SDValue S2Val =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, MVT::v64f32, Src);
        SDValue S3Val =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, MVT::v64f32, Src);

        SDValue Src0 = helperConvertvvpNodeCreate(
            S0Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_FP16, MVT::v128f16);
        SDValue Src1 = helperConvertvvpNodeCreate(
            S1Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_FP16, MVT::v128f16);
        SDValue Src2 = helperConvertvvpNodeCreate(
            S2Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_FP16, MVT::v128f16);
        SDValue Src3 = helperConvertvvpNodeCreate(
            S3Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_FP16, MVT::v128f16);

        SDValue mdg0, mdg1;
        mdg0 = truncate_32_to_16_goya(DAG, Src0, Src1, DL, MVT::v128f16,
                                      TPCII::OpType::FP16);
        mdg1 = truncate_32_to_16_goya(DAG, Src2, Src3, DL, MVT::v128f16,
                                      TPCII::OpType::FP16);
        return createTuple({mdg0, mdg1}, DAG);
      } else {
        assert(false && "Unsupported source type");
      }
    } else {
      assert(false && "Unsupported target type");
    }
  } else if (Src.getValueType() == MVT::v256bf16) {
    if (Subtarget->hasGaudi2ISA()) {
      SmallVector<SDValue, 8> Ops(6);
      if (Op.getValueType() == MVT::v256f8_143 ||
          Op.getValueType() == MVT::v256f8_152) {
        unsigned DstTySwVal = Op.getValueType() == MVT::v256f8_143
                                  ? TPCII::SW_TO_FP8_143
                                  : TPCII::SW_TO_FP8_152;
        Ops[0] = Src; // Source.
        Ops[1] = DAG.getTargetConstant(TPCII::OpType::BF16, DL, MVT::i8); // DataType.
        Ops[2] = DAG.getTargetConstant(
            DstTySwVal |
                (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE),
            DL, MVT::i32);                               // Switch.
        Ops[3] = DAG.getUNDEF(Op.getValueType());       // Income.
        Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
        Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
        return ConversionEntity(Op.getValueType(), Ops[0], Ops[1], Ops[2],
                                Ops[3], Ops[4], Ops[5], DAG, DL,
                                IsCastIntrinsicWithRoundMode);
      }
    } else {
      assert(false && "Unsupported target");
    }
  } else {
    assert(false && "Unsupported target type");
  }
  return SDValue();
}

SDValue TPCTargetLowering::lowerVectorFP_Extend(
    SDValue Op, SelectionDAG &DAG, SDLoc &DL, uint64_t DataType, EVT SubRegType,
    unsigned int InputSwitch, bool IsCastIntrinsicWithRoundMode) const {

  if (IsCastIntrinsicWithRoundMode)
    LLVM_DEBUG(dbgs() << "lowerVectorFP_Extend: Incoming switch = "
                      << InputSwitch << "\n");
  auto ExtArg = extend_16_to_32(Op, DAG, DL, DataType);
  SmallVector<SDValue, 8> Ops(6);
  Ops[0] = ExtArg[0];                                    // Source.
  Ops[1] = DAG.getTargetConstant(DataType, DL, MVT::i8); // DataType.
  Ops[2] = DAG.getTargetConstant(
      TPCII::SW_TO_FP32 |
          (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE),
      DL, MVT::i32);                              // Switch.
  Ops[3] = DAG.getUNDEF(Op.getValueType());       // Income.
  Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node =
      DAG.getMachineNode(TPC::CONVERTdvp, DL, Op.getValueType(), Ops);
  SDValue S0Val = SDValue(Node, 0);

  SmallVector<SDValue, 8> Ops1(6);
  Ops1[0] = ExtArg[1];                                    // Source.
  Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i8); // DataType.
  Ops1[2] = DAG.getTargetConstant(
      TPCII::SW_TO_FP32 |
          (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE),
      DL, MVT::i32);                               // Switch.
  Ops1[3] = DAG.getUNDEF(Op.getValueType());       // Income.
  Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node1 =
      DAG.getMachineNode(TPC::CONVERTdvp, DL, Op.getValueType(), Ops1);
  SDValue S1Val = SDValue(Node1, 0);

  SDValue S0ValFirst =
      helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, SubRegType, S0Val);
  SDValue S1ValFirst =
      helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, SubRegType, S1Val);

  SDValue Tuple = createTuple({S0ValFirst, S1ValFirst}, DAG);
  return Tuple;
}

SDValue TPCTargetLowering::lowerFP_EXTEND(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_CSR;

  // If Op is intrinsic, get the switch from it.
  auto IsCastIntrinsicWithRoundMode = Op.getOpcode() == ISD::INTRINSIC_WO_CHAIN;
  if (IsCastIntrinsicWithRoundMode) {
    InputSwitch = Op.getConstantOperandVal(NumOp - 1);
    LLVM_DEBUG(dbgs() << "Get switch from argument: " << InputSwitch << "\n");
  }
  const SDValue &Src =
      IsCastIntrinsicWithRoundMode ? Op.getOperand(1) : Op.getOperand(0);
  EVT SrcType = Src.getValueType();
  EVT OpType =
      (IsCastIntrinsicWithRoundMode) ? Op.getValueType() : Op->getValueType(0);

  // Let's try via any-any
  EVT SrcEltVT = SrcType.getVectorElementType();
  EVT DestEltVT = OpType.getVectorElementType();

  unsigned DDT = getInstrDataType(SrcEltVT, false);
  unsigned SWS = getSwitchByDestDataType(DestEltVT, false);

  return ConversionEntity(
      OpType, Src, DAG.getTargetConstant(DDT, DL, MVT::i8),
      DAG.getTargetConstant(SWS | InputSwitch, DL, MVT::i32),
      DAG.getUNDEF(OpType), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
      DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, false, nullptr);
}

#define GetConvIndex(mvtype) (mvtype - 1)

TPCTargetLowering::CVT
TPCTargetLowering::getSoSigned(TPCTargetLowering::CVT t) const {
  switch(t) {
  case v256i4  :  return v256u4  ;
  case v512i4  :  return v512u4  ;
  //case v1024i4 :  return v1024u4 ;
                    
  case v256i8  :  return v256u8  ;
  case v512i8  :  return v512u8  ;
  //case v1024i8 :  return v1024u8 ;

  case v128i16 :  return v128u16 ; 
  case v256i16 :  return v256u16 ;
  case v512i16 :  return v512u16 ;
  //case v1024i16:  return v1024u16;
                    
  case v64i32  :  return v64u32  ;
  case v128i32 :  return v128u32 ;
  case v256i32 :  return v256u32 ;
  case v512i32 :  return v512u32 ;


  case v256u4 :  return v256i4   ;
  case v512u4 :  return v512i4   ;
  //case v1024u4:  return v1024i4  ;
                                
  case v256u8 :  return v256i8   ;
  case v512u8 :  return v512i8   ;
  //case v1024u8:  return v1024i8  ;
  case v128u16:  return v128i16  ; 
                                
  case v256u16:  return v256i16  ;
  case v512u16:  return v512i16  ;
  //case v1024u16: return v1024i16;
                                
  case v64u32 :  return v64i32   ;
  case v128u32:  return v128i32  ;
  case v256u32:  return v256i32  ;
  case v512u32:  return v512i32  ;
  default:
    return FIRST;
  }
}


TPCTargetLowering::CVT
TPCTargetLowering::toCVT(EVT evt, bool isUnsigned,
                             unsigned bitlen /* for int4 */) const { 
  if (evt.isVector()) {
    EVT tel = evt.getVectorElementType();
    unsigned vnel = evt.getVectorNumElements();
    if (tel == MVT::f32) {
      if (vnel == 64)   return CVT::v64f32;
      if (vnel == 128)  return CVT::v128f32;
      if (vnel == 256)  return CVT::v256f32;
      if (vnel == 512)  return CVT::v512f32;
    } else if (tel == MVT::bf16) {
      if (vnel == 128)  return CVT::v128bf16;
      if (vnel == 256)  return CVT::v256bf16;
    } else if (tel == MVT::f16) {
      if (vnel == 128)  return CVT::v128f16;
      if (vnel == 256)  return CVT::v256f16;
    } else if (tel == MVT::f8_143) {
      if (vnel == 256)  return CVT::v256f8_143;
      if (vnel == 512)  return CVT::v512f8_143;
      //if (vnel == 1024) return CVT::v1024f8_143;
    } else if (tel == MVT::f8_152) {
      if (vnel == 256)  return CVT::v256f8_152;
      if (vnel == 512)  return CVT::v512f8_152;
      //if (vnel == 1024) return CVT::v1024f8_152;
    } else if (tel == MVT::i32) {
       if (vnel == 64) return (isUnsigned)? CVT::v64u32  : CVT::v64i32;
      if (vnel == 128) return (isUnsigned)? CVT::v128u32 : CVT::v128i32;
      if (vnel == 256) return (isUnsigned)? CVT::v256u32 : CVT::v256i32;
      if (vnel == 512) return (isUnsigned)? CVT::v512u32 : CVT::v512i32;
   } else if (tel == MVT::i16) {                 
      if (vnel == 128) return (isUnsigned)? CVT::v128u16 : CVT::v128i16;
      if (vnel == 256) return (isUnsigned)? CVT::v256u16 : CVT::v256i16;
      if (vnel == 512) return (isUnsigned)? CVT::v512u16 : CVT::v512i16;
    } else if (tel == MVT::i8) {
      if (bitlen==0) {
         if (vnel == 256) return (isUnsigned)? CVT::v256u8  : CVT::v256i8;
      } else {
        if (vnel == 256)
          return (isUnsigned) ? CVT::v512u4 : CVT::v512i4;
      }
      if (vnel == 512) return (isUnsigned)? CVT::v512u8  : CVT::v512i8;
      //if (vnel == 1024)return (isUnsigned)? CVT::v1024u8 : CVT::v1024i8;
    } else if (tel == MVT::i1) {
       if (vnel == 64) return CVT::v64i1;
       if (vnel == 128) return CVT::v128i1;
       if (vnel == 256) return CVT::v256i1;
    }
  } 
  assert(false && "not yet type transformation");
  return CVT::FIRST;
}

unsigned TPCTargetLowering::getCVTMultiplicity(TPCTargetLowering::CVT t) const {
  switch (t) {
    case v256i8: return 1;
    case v256u8: return 1;
    case v512i8: return 2;
    case v512u8: return 2;
    case v1024i8: case v1024u8: return 4;
    case v128i16: return 1;
    case v128u16: return 1;

    case v256i16: return 2;
    case v256u16: return 2;
    case v512i16: case v512u16: return 4;

    case v64i32: return 1;
    case v64u32: return 1;
    case v128i32: return 2;
    case v128u32: return 2;
    case v256i32: return 4;
    case v256u32: return 4;
    case v512i32: return 8;
    case v512u32: return 8;

    case v256f8_143: return 1;
    case v512f8_143: return 2;
    case v1024f8_143: return 4;

    case v256f8_152: return 1;
    case v512f8_152: return 2;
    case v1024f8_152: return 4;

    case v128f16: return 1;
    case v256f16: return 2;
    case v512f16: return 4;

    case v128bf16: return 1;
    case v256bf16: return 2;
    case v512bf16: return 4;

    case v64f32: return 1;
    case v128f32: return 2;
    case v256f32: return 4;
    case v512f32: return 8;
    default:  return FIRST;
  }
 }

 


TPCTargetLowering::TConvKind
 TPCTargetLowering::FindConversionElement(CVT fromCVT,
                                                    CVT toCVT) const {
  unsigned i = GetConvIndex(fromCVT);
  unsigned j = GetConvIndex(toCVT);
  assert(i >= 0 && j >= 0 && i < ConvTypeNumber && j < ConvTypeNumber);
  return MatrixConv[i][j];
}

void TPCTargetLowering::InitConversionMatrix(void) {

  const FeatureBitset &Features = Subtarget->getFeatureBits();
  unsigned iv128bf16 = GetConvIndex(v128bf16);
  unsigned iv128f16 = GetConvIndex(v128f16);
  unsigned iv128f32 = GetConvIndex(v128f32);
  unsigned iv128i16 = GetConvIndex(v128i16);
  unsigned iv128i32 = GetConvIndex(v128i32);
  unsigned iv128u16 = GetConvIndex(v128u16);
  unsigned iv128u32 = GetConvIndex(v128u32);
  unsigned iv256bf16 = GetConvIndex(v256bf16);
  unsigned iv256f16 = GetConvIndex(v256f16);
  unsigned iv256f32 = GetConvIndex(v256f32);
  unsigned iv256f8_143 = GetConvIndex(v256f8_143);
  unsigned iv256f8_152 = GetConvIndex(v256f8_152);
  unsigned iv256i16 = GetConvIndex(v256i16);
  unsigned iv256i32 = GetConvIndex(v256i32);
  unsigned iv256i8 = GetConvIndex(v256i8);
  unsigned iv256u16 = GetConvIndex(v256u16);
  unsigned iv256u32 = GetConvIndex(v256u32);
  unsigned iv256u8 = GetConvIndex(v256u8);
  unsigned iv512i4 = GetConvIndex(v512i4);
  unsigned iv512i8 = GetConvIndex(v512i8);
  unsigned iv512u4 = GetConvIndex(v512u4);
  unsigned iv512u8 = GetConvIndex(v512u8);
  unsigned iv64f32 = GetConvIndex(v64f32);
  unsigned iv64i32 = GetConvIndex(v64i32);
  unsigned iv64u32 = GetConvIndex(v64u32);

  unsigned iv64i1 = GetConvIndex(v64i1);
  unsigned iv128i1 = GetConvIndex(v128i1);
  unsigned iv256i1 = GetConvIndex(v256i1);

  unsigned i, j;
  for (i = 0; i < ConvTypeNumber; i++)
    for (j = 0; i < ConvTypeNumber; i++)
      MatrixConv[i][j] = Empty;

  MatrixConv[iv64u32][iv128u16] = Empty; // especially for sanitizer



  // conversion with double types, which is implementing by split
  for (auto FromTy :
       {v128i32,    v128u32,     v128f32,     v256i32,  v256u32,
        v256f32,    v256i16,     v256u16,     v256bf16, v256f16,
        v512i32,    v512u32,     v512f32,     v512i16,  v512u16,
        v512bf16,   v512f16,     v512i8,      v512u8,   v512f8_152,
        v512f8_143, v1024f8_143, v1024f8_152, v1024i8,  v1024u8}) {
    for (auto ToTy :
         {v128i32, v128u32, v128f32, v256i32, v256u32, v256f32, v256i16,
          v256u16, v256bf16, v256f16, v512f8_143, v512f8_152, v512i8, v512u8}) {
      i = GetConvIndex(FromTy);
      j = GetConvIndex(ToTy);
      MatrixConv[i][j] = Split;
    }
  }

  // Soft at the begining, may be rewriten by HARD

  unsigned mf, mt;
  for (auto FromTy :
       {v128i32, v128i16, v128u32, v128u16, v256i16, v256u16, v256i32, v256u32,
        v128f32, v128bf16, v128f16, v256f32, v256bf16, v256f16, v256i8, v256u8,
        v256f8_143, v256f8_152}) {
    for (auto ToTy : {v128i16, v128i32, v128u16, v128u32, v256i8, v256u8,
                      v256i16, v256u16, v256i32, v256u32, v128f32, v128bf16,
                      v128f16, v256f32, v256f8_143, v256f8_152}) {
      if (FromTy == ToTy)
        continue;
      if ((FromTy == v256i16 || FromTy == v256u16) &&
          (ToTy == v256i16 || ToTy == v256u16))
        continue;
      mf = getCVTMultiplicity(FromTy);
      mt = getCVTMultiplicity(ToTy);
      unsigned oro = mf | mt;
      if (!(oro & 1))
        continue;

      i = GetConvIndex(FromTy);
      j = GetConvIndex(ToTy);
      MatrixConv[i][j] = Soft;
    }
  }

  // no instruction on gaudi for unsigned
  MatrixConv[iv64u32][iv128bf16] = Soft;
  MatrixConv[iv64u32][iv128f16] = Soft;
  MatrixConv[iv64i32][iv128f16] = Soft;
  MatrixConv[iv128bf16][iv128i32] = Soft;
  MatrixConv[iv128f16][iv128i32] = Soft;
  MatrixConv[iv256u8][iv256i32] = Soft;
  MatrixConv[iv256i32][iv256u8] = Soft;
  MatrixConv[iv256u8][iv256u16] = Soft;
  MatrixConv[iv256i8][iv128i32] = Soft;
  MatrixConv[iv128f32][iv128f16] = Soft;

  // there are patterns i8->i4, so it may be skipped here
  MatrixConv[iv512u8][iv256u8] = Skip;
  MatrixConv[iv512i8][iv256i8] = Skip;

  MatrixConv[iv128bf16][iv128i32] = Chain;
  MatrixConv[iv128bf16][iv128u32] = Chain;
  MatrixConv[iv128f16][iv128i32] = Chain;
  MatrixConv[iv128f16][iv128u32] = Chain;
  MatrixConv[iv128f32][iv128u16] = Chain;
  MatrixConv[iv128i32][iv128u16] = Chain;
  MatrixConv[iv128u16][iv128f32] = Chain;
  MatrixConv[iv128u16][iv128i16] = Chain;
  MatrixConv[iv128u16][iv128i32] = Chain;
  MatrixConv[iv128u16][iv128u32] = Chain;
  MatrixConv[iv128u32][iv128bf16] = Chain;
  MatrixConv[iv128u16][iv128f16] = Chain;

  MatrixConv[iv128u32][iv128i16] = Chain;
  MatrixConv[iv256f32][iv256u8] = Chain;
  MatrixConv[iv256u16][iv256i8] = Chain;
  MatrixConv[iv256u32][iv256i8] = Chain;
  MatrixConv[iv256u8][iv128i16] = Chain;
  MatrixConv[iv256u8][iv128i32] = Chain;
  MatrixConv[iv256u8][iv256f32] = Chain;
  MatrixConv[iv256u8][iv256i16] = Chain;
  MatrixConv[iv256u8][iv256i8] = Chain;
  MatrixConv[iv256u8][iv256u16] = Chain;
  MatrixConv[iv256u8][iv256u16] = Chain;
  MatrixConv[iv256u8][iv256u16] = Chain;
  MatrixConv[iv256u8][iv256u32] = Chain;
  MatrixConv[iv256u8][iv256u32] = Chain;
  MatrixConv[iv64f32][iv64u32] = Chain;
  MatrixConv[iv64u32][iv64f32] = Chain;
  MatrixConv[iv64u32][iv128i16] = Chain;
  MatrixConv[iv64u32][iv256i8] = Chain;
  MatrixConv[iv64f32][iv256u8] = Chain;
  MatrixConv[iv64f32][iv128u16] = Chain;

  MatrixConv[iv256u8][iv64u32] = Chain;
  MatrixConv[iv256u8][iv128u16] = Chain;
  MatrixConv[iv256u8][iv128i16] = Chain;
  MatrixConv[iv256u8][iv64f32] = Chain;

  MatrixConv[iv256u8][iv128bf16] = Chain;
  MatrixConv[iv256u8][iv128f16] = Chain;

  MatrixConv[iv128u16][iv64i32] = Chain;
  MatrixConv[iv128u16][iv64u32] = Chain;
  MatrixConv[iv128u16][iv64f32] = Chain;

  MatrixConv[iv256i8][iv128bf16] = Chain;
  MatrixConv[iv256i8][iv128f16] = Chain;
  MatrixConv[iv256u8][iv128f16] = Chain;


  MatrixConv[iv64i1][iv64i32] = BoolTo;
  MatrixConv[iv128i1][iv128i16] = BoolTo;
  MatrixConv[iv256i1][iv256i8] = BoolTo;

  MatrixConv[iv64i1][iv64f32] = BoolTo;
  MatrixConv[iv128i1][iv128bf16] = BoolTo;
  MatrixConv[iv128i1][iv128f16] = BoolTo;
  MatrixConv[iv256i1][iv256f8_143] = BoolTo;
  MatrixConv[iv256i1][iv256f8_152] = BoolTo;


  MatrixConv[iv128i16][iv128u16] = Hard;
  MatrixConv[iv128i16][iv256i8] = Hard;
  MatrixConv[iv128i16][iv256u8] = Hard;
  MatrixConv[iv128u16][iv128u16] = Hard;
  MatrixConv[iv128u16][iv256u8] = Hard;

  MatrixConv[iv256i8][iv256u8] = Hard;
  MatrixConv[iv256i8][iv64f32] = Hard;
  MatrixConv[iv256i8][iv64i32] = Hard;
  MatrixConv[iv256i8][iv64u32] = Hard;
  MatrixConv[iv256u8][iv128u32] = Hard;
  MatrixConv[iv256u8][iv64i32] = Hard;
  MatrixConv[iv64f32][iv128i16] = Hard;
  MatrixConv[iv64f32][iv128i16] = Hard;
  MatrixConv[iv64f32][iv256i8] = Hard;
  MatrixConv[iv64f32][iv64i32] = Hard;
  MatrixConv[iv64i32][iv128u16] = Hard;
  MatrixConv[iv64i32][iv256i8] = Hard;
  MatrixConv[iv64i32][iv256u8] = Hard;
  MatrixConv[iv64i32][iv64f32] = Hard;
  MatrixConv[iv64i32][iv64u32] = Hard;
  MatrixConv[iv64u32][iv64i32] = Hard;

  MatrixConv[iv256i8][iv128i16] = HardLane;
  MatrixConv[iv128i16][iv64i32] = HardLane;
  MatrixConv[iv256i8][iv128u16] = HardLane;
  MatrixConv[iv128i16][iv64u32] = HardLane;
  MatrixConv[iv128i16][iv64f32] = HardLane;


   MatrixConv[iv128i16][iv256i8] = IntHard;
   MatrixConv[iv128u16][iv256u8] = IntHard;
   MatrixConv[iv64i32][iv256i8] = IntHard;
   MatrixConv[iv64u32][iv256u8] = IntHard;
   MatrixConv[iv64i32][iv128i16] = IntHard;
   MatrixConv[iv64u32][iv128u16] = IntHard;

   MatrixConv[iv256i16][iv256i8] = IntSoft;
   MatrixConv[iv256u16][iv256u8] = IntSoft;
   MatrixConv[iv256i32][iv256i8] = IntSoft;
   MatrixConv[iv256u32][iv256u8] = IntSoft;
   MatrixConv[iv128i32][iv128i16] = IntSoft;
   MatrixConv[iv128u32][iv128u16] = IntSoft;



  if (Features[TPC::FeatureGen2Plus]) {
    MatrixConv[iv256u8][iv256bf16] = Chain;
    MatrixConv[iv256bf16][iv256i8] = Chain;
    MatrixConv[iv256bf16][iv256u8] = Chain;
    MatrixConv[iv256f16][iv256i8] = Chain;
    MatrixConv[iv256f16][iv256u8] = Chain;
    MatrixConv[iv256i8][iv256bf16] = Chain;
    MatrixConv[iv256u8][iv256bf16] = Chain;
    MatrixConv[iv256i8][iv256f16] = Chain;
    MatrixConv[iv256u8][iv256f16] = Chain;
    MatrixConv[iv128bf16][iv64i32] = Chain;
    MatrixConv[iv128bf16][iv64u32] = Chain;
    MatrixConv[iv128f16][iv64i32] = Chain;
    MatrixConv[iv128f16][iv64u32] = Chain;
    MatrixConv[iv128u32][iv128f16] = Chain;

    MatrixConv[iv128bf16][iv128f32] = VPemu;
    MatrixConv[iv128bf16][iv128i16] = Hard;
    MatrixConv[iv128bf16][iv256bf16] = Hard;
    MatrixConv[iv128bf16][iv256f16] = Hard;
    MatrixConv[iv128bf16][iv256i16] = Hard;
    MatrixConv[iv128bf16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv128f32] = Hard;
    MatrixConv[iv128f16][iv256bf16] = Hard;
    MatrixConv[iv128f16][iv256f16] = Hard;
    MatrixConv[iv128f16][iv256i16] = Hard;
    MatrixConv[iv128f32][iv128bf16] = Hard;
    MatrixConv[iv128i16][iv128bf16] = Hard;
    MatrixConv[iv128i16][iv256bf16] = Hard;
    MatrixConv[iv128i16][iv256f16] = Hard;
    MatrixConv[iv128i16][iv256i16] = Hard;
    MatrixConv[iv128u16][iv128bf16] = Hard;
    MatrixConv[iv256i8][iv128f32] = Hard;
    MatrixConv[iv64f32][iv128bf16] = Hard;
    MatrixConv[iv64f32][iv128f32] = Hard;
    MatrixConv[iv64f32][iv256bf16] = Hard;
    MatrixConv[iv64f32][iv256f16] = Hard;
    MatrixConv[iv64f32][iv256i16] = Hard;
    MatrixConv[iv64i32][iv128bf16] = Hard;
    MatrixConv[iv64i32][iv128f32] = Hard;
    MatrixConv[iv64i32][iv256bf16] = Hard;
    MatrixConv[iv64i32][iv256f16] = Hard;
    MatrixConv[iv64i32][iv256i16] = Hard;
  }

  if (Features[TPC::FeatureGen2BPlus]) {
    MatrixConv[iv128bf16][iv128i32] = Chain;
    MatrixConv[iv128bf16][iv128u32] = Chain;
    MatrixConv[iv128f16][iv128i16] = Chain;
    MatrixConv[iv128f16][iv128i32] = Chain;
    MatrixConv[iv128f16][iv128u16] = Chain;
    MatrixConv[iv128f16][iv128u32] = Chain;
    MatrixConv[iv128i16][iv128f16] = Chain;
    MatrixConv[iv128i32][iv128f16] = Chain;

    MatrixConv[iv128bf16][iv128f32] = VPemu;
    MatrixConv[iv128bf16][iv64f32] = Hard;
    MatrixConv[iv128bf16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv128f32] = VPemu;
    MatrixConv[iv128f16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv64f32] = Hard;
    MatrixConv[iv128i16][iv128u16] = Hard;
    MatrixConv[iv256i8][iv256u8] = Hard;
    MatrixConv[iv64f32][iv128bf16] = Hard;
    MatrixConv[iv64f32][iv128f16] = Hard;
  }

  if (Features[TPC::FeatureGen3Plus]) {

    for (auto ToTy : {v128f32, v256bf16, v256f16, v128i32, v256i16}) {
      j = GetConvIndex(ToTy);
      MatrixConv[iv128f16][j] = Hard;
    }

    for (auto FromTy :
         {v64f32, v128bf16, v64i32, v128i16, v256i8, v64u32, v128u16, v256u8}) {
      j = GetConvIndex(FromTy);
      MatrixConv[j][iv256f16] = Hard;
      MatrixConv[j][iv256bf16] = Hard;
    }

    for (auto ToTy : {v128bf16, v128i16, v256i8, v128u16, v256u8}) {
      j = GetConvIndex(ToTy);
      MatrixConv[iv128f16][j] = Hard;
    }
    for (auto FromTy : {v128bf16, v128i16, v256i8, v128u16, v256u8}) {
      j = GetConvIndex(FromTy);
      MatrixConv[j][iv128f16] = Hard;
      MatrixConv[j][iv128bf16] = Hard;
    }

    for (auto FromTy :
         {v256f32, v256bf16, v256f16, v256i32, v256i16, v256u32, v256u16}) {
      for (auto ToTy : {v256f8_143, v256f8_152}) {
        i = GetConvIndex(FromTy);
        j = GetConvIndex(ToTy);
        MatrixConv[i][j] = Hard;
      }
    }

    for (auto FromTy : {v64f32, v128bf16, v128f16, v256f32, v64i32, v128i16,
                        v256i8, v256i32, v64u32, v128u16, v256u8}) {
      i = GetConvIndex(FromTy);
      MatrixConv[i][iv256f32] = Hard;
    }

    for (auto FromTy : {v64f32, v128bf16, v128f16, v64i32, v128i16, v256i8,
                        v256i32, v64u32, v128u16, v256u8, v256u32}) {
      i = GetConvIndex(FromTy);
      MatrixConv[i][iv256i32] = Hard;
    }
    for (auto FromTy : {v64f32, v128bf16, v128f16, v64i32, v128i16, v256i8,
                        v256i32, v64u32, v128u16, v256u8, v256u32}) {
      i = GetConvIndex(FromTy);
      MatrixConv[i][iv256u32] = Hard;
    }

    MatrixConv[iv256i8][iv128i16] = Hard;
    MatrixConv[iv256i8][iv128u16] = Hard;
    MatrixConv[iv128u16][iv64f32] = Hard;
    MatrixConv[iv256u8][iv128i16] = Hard;
    MatrixConv[iv256u8][iv128u16] = Hard;
    MatrixConv[iv256u8][iv64u32] = Hard;
    MatrixConv[iv128bf16][iv128f32] = Hard;
    MatrixConv[iv128bf16][iv128i16] = Hard;
    MatrixConv[iv128bf16][iv128u16] = Hard;
    MatrixConv[iv128bf16][iv256i8] = Hard;
    MatrixConv[iv128bf16][iv64f32] = Hard;
    MatrixConv[iv128bf16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv128f32] = Hard;
    MatrixConv[iv128f16][iv128f32] = Hard;
    MatrixConv[iv128f16][iv128i16] = Hard;
    MatrixConv[iv128f16][iv128i16] = Hard;
    MatrixConv[iv128f16][iv128i32] = Hard;
    MatrixConv[iv128f16][iv128u16] = Hard;
    MatrixConv[iv128f16][iv256i8] = Hard;
    MatrixConv[iv128f16][iv256u8] = Hard;
    MatrixConv[iv128f16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv64f32] = Hard;
    MatrixConv[iv128f16][iv64i32] = Hard;
    MatrixConv[iv128f32][iv128f16] = Hard;
    MatrixConv[iv128f32][iv128i16] = Hard;
    MatrixConv[iv128f32][iv128u16] = Hard;
    MatrixConv[iv64f32][iv128i16] = Hard;
    MatrixConv[iv64f32][iv128u16] = Hard;
    MatrixConv[iv128i16][iv128f32] = Hard;
    MatrixConv[iv128i16][iv128i32] = Hard;
    MatrixConv[iv128i16][iv128u16] = Hard;
    MatrixConv[iv128i16][iv128u32] = Hard;
    MatrixConv[iv128i32][iv128bf16] = Hard;
    MatrixConv[iv128i32][iv128f16] = Hard;
    MatrixConv[iv128i32][iv128u32] = Hard;
    MatrixConv[iv128u16][iv128bf16] = Hard;
    MatrixConv[iv128u16][iv128f16] = Hard;

    MatrixConv[iv128u16][iv128f32] = Hard;
    MatrixConv[iv128u32][iv128i32] = Hard;
    MatrixConv[iv256bf16][iv256i8] = Hard;
    MatrixConv[iv256bf16][iv256u8] = Hard;
    MatrixConv[iv256f16][iv256i8] = Hard;
    MatrixConv[iv128bf16][iv256u8] = Hard;
    MatrixConv[iv128f16][iv256i8] = Hard;
    MatrixConv[iv256f16][iv256u8] = Hard;
    MatrixConv[iv256f16][iv256u8] = Hard;
    MatrixConv[iv256f32][iv256i8] = Hard;
    MatrixConv[iv256f32][iv256u8] = Hard;
    MatrixConv[iv256i16][iv256u8] = Hard;
    MatrixConv[iv256i32][iv256u8] = Hard;
    MatrixConv[iv256i8][iv128bf16] = Hard;
    MatrixConv[iv256i8][iv128f16] = Hard;
    MatrixConv[iv256i8][iv256f32] = Hard;
    MatrixConv[iv256i8][iv256i16] = Hard;
    MatrixConv[iv256i8][iv256i8] = Hard;
    MatrixConv[iv256i8][iv256u16] = Hard;
    MatrixConv[iv256i8][iv256u8] = Hard;
    MatrixConv[iv256u8][iv128bf16] = Hard;
    MatrixConv[iv256u8][iv128f16] = Hard;
    MatrixConv[iv256u8][iv128i16] = Hard;
    MatrixConv[iv256u8][iv256f16] = Hard;
    MatrixConv[iv256u8][iv256i16] = Hard;
    MatrixConv[iv256u8][iv256u16] = Hard;
    MatrixConv[iv256u8][iv64f32] = Hard;
    MatrixConv[iv512i4][iv256i8] = Hard;
    MatrixConv[iv512i4][iv512i8] = Hard;
    MatrixConv[iv512u4][iv256u8] = Hard;
    MatrixConv[iv512u4][iv512u8] = Hard;
    MatrixConv[iv64f32][iv128bf16] = Hard;
    MatrixConv[iv64f32][iv128f16] = Hard;
    MatrixConv[iv64f32][iv128f16] = Hard;
    MatrixConv[iv64f32][iv64u32] = Hard;
    MatrixConv[iv64i32][iv128bf16] = Hard;
    MatrixConv[iv64i32][iv128f16] = Hard;
    MatrixConv[iv64u32][iv64f32] = Hard;
    MatrixConv[iv256i8][iv128i32] = Hard;


    {
      // INT conversions
      MatrixConv[iv128i16][iv256i8] = HardOrIntHard;
      MatrixConv[iv128u16][iv256u8] = HardOrIntHard;
      MatrixConv[iv128i32][iv128i16] = HardOrIntHard;
      MatrixConv[iv128u32][iv128u16] = HardOrIntHard;
      MatrixConv[iv256i16][iv256i8] = HardOrIntHard;
      MatrixConv[iv256i32][iv256i8] = HardOrIntHard;
      MatrixConv[iv256u16][iv256u8] = HardOrIntHard;
      MatrixConv[iv256u32][iv256u8] = HardOrIntHard;
      MatrixConv[iv64i32][iv128i16] = HardOrIntHard;
      MatrixConv[iv64i32][iv256i8] = HardOrIntHard;
      MatrixConv[iv64u32][iv128u16] = HardOrIntHard;
      MatrixConv[iv64u32][iv256u8] = HardOrIntHard;
    } 

  }



  if (Features[TPC::FeatureGen4Plus]) {
    for (auto FromTy : {v64f32, v128bf16, v128f16, v64i32, v128i16, v256i8,
                        v64u32, v128u16, v256u8}) {
      for (auto ToTy : {v256f8_143, v256f8_152}) {
        i = GetConvIndex(FromTy);
        j = GetConvIndex(ToTy);
        MatrixConv[i][j] = Hard;
      }
    }
    for (auto FromTy : {v256f8_143, v256f8_152}) {
      for (auto ToTy :
           {v256f32, v256i32, v256u32, v256bf16, v256f16, v256i16, v256u16}) {
        i = GetConvIndex(FromTy);
        j = GetConvIndex(ToTy);
        MatrixConv[i][j] = Hard;
      }
    }
    for (auto FromTy : {v256f8_143, v256f8_152}) {
      for (auto ToTy : {v64f32, v128bf16, v128f16, v64i32, v128i16, v256i8,
                        v64u32, v128u16, v256u8}) {
        i = GetConvIndex(FromTy);
        j = GetConvIndex(ToTy);
        MatrixConv[i][j] = Hard;
      }
    }

    MatrixConv[iv128bf16][iv64i32] = Hard;
    MatrixConv[iv128bf16][iv64u32] = Hard;
    MatrixConv[iv128bf16][iv128i32] = Hard;
    MatrixConv[iv128bf16][iv128u32] = Hard;
    MatrixConv[iv128bf16][iv256u8] = Hard;
    MatrixConv[iv128bf16][iv64i32] = Hard;
    MatrixConv[iv128bf16][iv64u32] = Hard;
    MatrixConv[iv128f16][iv128i32] = Hard;
    MatrixConv[iv128f16][iv128u32] = Hard;
    MatrixConv[iv128f16][iv64i32] = Hard;
    MatrixConv[iv128f16][iv64u32] = Hard;
    MatrixConv[iv128i16][iv256i8] = Hard;
    MatrixConv[iv128i32][iv128bf16] = Hard;
    MatrixConv[iv128i32][iv128u16] = Hard;
    MatrixConv[iv128i32][iv256i8] = Hard;
    MatrixConv[iv128u32][iv256i8] = Hard;
    MatrixConv[iv128u32][iv256u8] = Hard;

    MatrixConv[iv128u16][iv128f32] = Hard;
    MatrixConv[iv128u16][iv128i16] = Hard;
    MatrixConv[iv128u16][iv128i32] = Hard;
    MatrixConv[iv128u16][iv128u32] = Hard;
    MatrixConv[iv128u16][iv64f32] = Hard;
    MatrixConv[iv128u16][iv64f32] = Hard;
    MatrixConv[iv128u16][iv64i32] = Hard;
    MatrixConv[iv128u16][iv64u32] = Hard;
    MatrixConv[iv128u32][iv128bf16] = Hard;
    MatrixConv[iv128u32][iv128f16] = Hard;
    MatrixConv[iv128u32][iv128i16] = Hard;
    MatrixConv[iv128u32][iv128u16] = HardOrIntHard;
    MatrixConv[iv128u32][iv64i32] = Hard;
    MatrixConv[iv256f8_143][iv256f8_152] = Hard;
    MatrixConv[iv256f8_152][iv256f8_143] = Hard;
    MatrixConv[iv256i8][iv256bf16] = Hard;
    MatrixConv[iv256i8][iv256f16] = Hard;
    MatrixConv[iv256i16][iv256i8] = HardOrIntHard;
    MatrixConv[iv256u16][iv256u8] = HardOrIntHard;
    MatrixConv[iv256u32][iv128u32] = Hard;
    MatrixConv[iv256u32][iv256f32] = Hard;
    MatrixConv[iv256u32][iv256i8] = Hard;
    MatrixConv[iv256u32][iv256u8] = HardOrIntHard;
    MatrixConv[iv256u8][iv256bf16] = Hard;
    MatrixConv[iv256u8][iv256f16] = Hard;
    MatrixConv[iv256u8][iv256i8] = Hard;
    MatrixConv[iv256u8][iv256u16] = Hard;
    MatrixConv[iv256u8][iv64f32] = Hard;
    MatrixConv[iv256u16][iv256i8] = Hard;

    MatrixConv[iv64f32][iv128u16] = Hard;
    MatrixConv[iv64f32][iv256u8] = Hard;
    MatrixConv[iv64i32][iv128bf16] = Hard;
    MatrixConv[iv64i32][iv128f16] = Hard;
    MatrixConv[iv64u32][iv128bf16] = Hard;
    MatrixConv[iv64u32][iv128f16] = Hard;
    MatrixConv[iv64u32][iv128u16] = HardOrIntHard;
    MatrixConv[iv64u32][iv64i32] = Hard;
    MatrixConv[iv64u32][iv128i16] = Hard;

    MatrixConv[iv64u32][iv256i8] = Hard;
    MatrixConv[iv128u16][iv256i8] = Hard;

    MatrixConv[iv64i32][iv128i16] = HardOrIntHard;
    MatrixConv[iv64i32][iv256i8] = HardOrIntHard;
    MatrixConv[iv64u32][iv256u8] = HardOrIntHard;
    MatrixConv[iv128i32][iv256i8] = HardOrIntHard;
    MatrixConv[iv128u32][iv256u8] = HardOrIntHard;
    MatrixConv[iv128i16][iv256i8] = HardOrIntHard;
    MatrixConv[iv128u16][iv256u8] = HardOrIntHard;

    MatrixConv[iv64i32][iv128i16] = HardOrIntHard;
    MatrixConv[iv64u32][iv128u16] = HardOrIntHard;
    MatrixConv[iv64i32][iv256i8] = HardOrIntHard;
    MatrixConv[iv64u32][iv256u8] = HardOrIntHard;

  }
 
  if (Features[TPC::FeatureGen3Plus]) {
    MatrixConv[iv128i32][iv128f32] = XX;
    MatrixConv[iv256i32][iv256f32] = XX;
    if (Features[TPC::FeatureGen4Plus]) {
      MatrixConv[iv256u32][iv256f32] = XX;
      MatrixConv[iv128u32][iv128f32] = XX;
    }
  }

  if (Features[TPC::FeatureDoron1]) {
    // cant be all_lanes

    MatrixConv[iv256i16][iv256f8_143] = Soft;
    MatrixConv[iv256u16][iv256f8_143] = Soft;
    MatrixConv[iv256i16][iv256f8_152] = Soft;
    MatrixConv[iv256u16][iv256f8_152] = Soft;

    // Some conversions via LOWER/UPPER lanes
    MatrixConv[iv256f8_143][iv256i32] = LoUpLanes;
    MatrixConv[iv256f8_143][iv256u32] = LoUpLanes;

    MatrixConv[iv256f8_152][iv256i32] = LoUpLanes;
    MatrixConv[iv256f8_152][iv256u32] = LoUpLanes;
    MatrixConv[iv256f8_152][iv128i32] = Hard;
    MatrixConv[iv256f8_152][iv128u32] = Hard;

    MatrixConv[iv256f8_143][iv256i8] = LoUpLanes;
    MatrixConv[iv256f8_143][iv256u8] = LoUpLanes;
    MatrixConv[iv256f8_152][iv256i8] = LoUpLanes;
    MatrixConv[iv256f8_152][iv256u8] = LoUpLanes;
  

     MatrixConv[iv256u32][iv256i8] = LoUpLanes;

    MatrixConv[iv256i32][iv256i8] = LoUpLanes;
    MatrixConv[iv256u32][iv256u8] = LoUpLanes;
    MatrixConv[iv256i32][iv256u8] = LoUpLanes;
    MatrixConv[iv256u32][iv256i8] = LoUpLanes;

    MatrixConv[iv128i32][iv256i8] = Hard;
    MatrixConv[iv128u32][iv256u8] = Hard;
    MatrixConv[iv128i32][iv256u8] = Hard;
    MatrixConv[iv128u32][iv256i8] = Hard;

    MatrixConv[iv256f32][iv256i8] = LoUpLanes;
    MatrixConv[iv128f32][iv256i8] = Hard;
    MatrixConv[iv256f32][iv256u8] = LoUpLanes;
    MatrixConv[iv128f32][iv256u8] = Hard;

    MatrixConv[iv256f8_152][iv256u16] = LoUpLanes;
    MatrixConv[iv256f8_152][iv128u16] = Hard;
    MatrixConv[iv256f8_152][iv256i16] = LoUpLanes;
    MatrixConv[iv256f8_152][iv128i16] = Hard;
    MatrixConv[iv256f8_143][iv256u16] = LoUpLanes;
    MatrixConv[iv256f8_143][iv128u16] = Hard;
    MatrixConv[iv256f8_143][iv256i16] = LoUpLanes;
    MatrixConv[iv256f8_143][iv128i16] = Hard;


    MatrixConv[iv256f8_152][iv256u32] = LoUpLanes;
    MatrixConv[iv256f8_152][iv128u32] = Hard;
    MatrixConv[iv256f8_152][iv256i32] = LoUpLanes;
    MatrixConv[iv256f8_152][iv128i32] = Hard;
    MatrixConv[iv256f8_143][iv256u32] = LoUpLanes;
    MatrixConv[iv256f8_143][iv128u32] = Hard;
    MatrixConv[iv256f8_143][iv256i32] = LoUpLanes;
    MatrixConv[iv256f8_143][iv128i32] = Hard;

    MatrixConv[iv256i16][iv256i8] = IntSoft;
    MatrixConv[iv256u16][iv256u8] = IntSoft;
    MatrixConv[iv128i32][iv128i16] = HardOrIntHard;//IntSoft;
    MatrixConv[iv128u32][iv128u16] = HardOrIntHard;//IntSoft;

    MatrixConv[iv256i32][iv256i8] = IntLoUpLanes;
    MatrixConv[iv256u32][iv256u8] = IntLoUpLanes;


  }

  if (Features[TPC::FeatureFP8] == 0) {
    for (unsigned i = 0; i < ConvTypeNumber; i++) {
      for (unsigned j = 0; j < ConvTypeNumber; j++) {
        if (i == iv256f8_143 || i == iv256f8_152 || j == iv256f8_143 ||
            j == iv256f8_152) {
          MatrixConv[i][j] = Ban;
        }
      }
    }
  }
  if (Features[TPC::FeatureFP16] == 0) {
    for (unsigned i = 0; i < ConvTypeNumber; i++) {
      for (unsigned j = 0; j < ConvTypeNumber; j++) {
        if (i == iv256f16 || i == iv128f16 || j == iv128f16 || j == iv256f16) {
          MatrixConv[i][j] = Ban;
        }
      }
    }
  }
  if (Features[TPC::FeatureBF16] == 0) {
    for (unsigned i = 0; i < ConvTypeNumber; i++) {
      for (unsigned j = 0; j < ConvTypeNumber; j++) {
        if (i == iv256bf16 || i == iv128bf16 || j == iv128bf16 ||
            j == iv256bf16) {
          MatrixConv[i][j] = Ban;
        }
      }
    }
  }
}
  

static unsigned getMultiplicity(EVT VT) {
  unsigned TypeSize = VT.getSizeInBits();
  switch (TypeSize) {
  case 64 * 1:
  case 128 * 1:
  case 256 * 1:
  case 256*8: return 1;
  case 512*8: return 2;
  case 1024*8: return 4;
  }
  assert(false && "Invalid type multiplicity");
  return 0;
}

static unsigned getConvertOpCode(EVT DestVT, EVT SrcVT, bool VectorPredicate,
                                 const TPCSubtarget &Subtarget) {
  unsigned SrcMultiplicity = getMultiplicity(SrcVT);
  unsigned DestMultiplicity = getMultiplicity(DestVT);
  
  if (Subtarget.hasGen3Plus()) {
    if (DestMultiplicity == 4 && SrcMultiplicity == 4) {
      return (VectorPredicate) ? TPC::CONVERTg3aam : TPC::CONVERTg3aap;
    }
    if (DestMultiplicity == 2 && SrcMultiplicity == 2) {
      return (VectorPredicate) ? TPC::CONVERTg3ddm : TPC::CONVERTg3ddp;
    }
    if (DestMultiplicity == 4 && SrcMultiplicity == 1) {
      return (VectorPredicate) ? TPC::CONVERTg3avm : TPC::CONVERTg3avp;
    }
    if (DestMultiplicity == 1 && SrcMultiplicity == 4) {
      return (VectorPredicate) ? TPC::CONVERTg3vam : TPC::CONVERTg3vap;
    }
    if (DestMultiplicity == 1 && SrcMultiplicity == 2) {
      return (VectorPredicate) ? TPC::CONVERTg3vdm : TPC::CONVERTg3vdp;
    }
  }
  if (Subtarget.hasGaudiBISA()) { //Gen2B
    if (DestMultiplicity == 1 && SrcMultiplicity == 2) {
      return (VectorPredicate) ? TPC::CONVERTg2bvdm : TPC::CONVERTg2bvdp;
    }
    if (DestMultiplicity == 1 && SrcMultiplicity == 4) {
      return (VectorPredicate) ? TPC::CONVERTg3vam : TPC::CONVERTg3vap;
    }
  }
  if (Subtarget.hasGen2Plus()) {
    if (DestMultiplicity == 2 && SrcMultiplicity == 1) {
      return (VectorPredicate) ? TPC::CONVERTdvm : TPC::CONVERTdvp;
    }
    if (
        (DestMultiplicity == 1 && SrcMultiplicity == 1) ||
        (DestMultiplicity == 2 && SrcMultiplicity == 2)) {
      return (VectorPredicate) ? TPC::CONVERTvvm : TPC::CONVERTvvp;
    }
  }
  if (Subtarget.hasGaudiISA()) { //Gen2
    if (DestMultiplicity == 1 && SrcMultiplicity == 2) {
      return (VectorPredicate) ? TPC::CONVERTvdm : TPC::CONVERTvdp;
    }
  }

  if (DestMultiplicity == 1 && SrcMultiplicity == 1) {
    return (VectorPredicate) ? TPC::CONVERTvvm : TPC::CONVERTvvp;
  }

  if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA()) {
    if (DestMultiplicity == 2 && SrcMultiplicity == 1) {
      return (VectorPredicate) ? TPC::CONVERTdvm : TPC::CONVERTdvp;
    }
    if (DestMultiplicity == 2 && SrcMultiplicity == 2) {
      return (VectorPredicate) ? TPC::CONVERTg3ddm : TPC::CONVERTg3ddp;
    }
    if (DestMultiplicity == 4 && SrcMultiplicity == 1) {
      return (VectorPredicate) ? TPC::CONVERTg3avm : TPC::CONVERTg3avp;
    }
  }
   //for a while
  return 0;
}


static unsigned getIntConvertOpCode(EVT DestVT, EVT SrcVT, bool VectorPredicate,
                                    const TPCSubtarget *Subtarget,
                                    SDValue *Shift, bool IsUnsigned) {
  unsigned SrcMultiplicity = getMultiplicity(SrcVT);
  unsigned DestMultiplicity = getMultiplicity(DestVT);
  const FeatureBitset &Features = Subtarget->getFeatureBits();
  bool isgo  = Features[TPC::FeatureGoya];
  bool isgre = Features[TPC::FeatureGreco] || Features[TPC::FeatureGaudi2];
  bool ShiftisImm = Shift == nullptr;
  bool ShiftisScalar = false;
  if (Shift) {
    SDValue Element;
    EVT sht = Shift->getValueType();
    if (isSplatBuildVector(*Shift, Element)) {
      if (Element.getOpcode() == ISD::Constant) {
        ShiftisImm = true;
      } else {
        ShiftisScalar = true;
      }
    } else if (!sht.isVector()) {
      if (Shift->getOpcode() == ISD::TargetConstant ||
          Shift->getOpcode() == ISD::Constant) {
          ShiftisImm = true;
      } else {
        ShiftisScalar = true;
      }          
    }
  }
  /*
  if (ShiftisImm) {
    ShiftisImm = false;
    ShiftisScalar = true;
  }
  */
  if (SrcMultiplicity == 2 && DestMultiplicity == 1) {
    if (SrcVT == MVT::v256i16 && DestVT == MVT::v256i8) {
      if (isgre) {
        if (VectorPredicate) {
          if (ShiftisImm) {
            return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avim
                                : TPC::CONVERT_INT16g3Avim;
          }
          if (ShiftisScalar) {
            return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avsm
                                : TPC::CONVERT_INT16g3Avsm;
          }
          return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avvm
                              : TPC::CONVERT_INT16g3Avvm;
        }
        if (ShiftisImm) {
          return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avip
                              : TPC::CONVERT_INT16g3Avip;
        }
        if (ShiftisScalar) {
          return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avsp
                              : TPC::CONVERT_INT16g3Avsp;
        }
        return (IsUnsigned) ? TPC::CONVERT_UINT16g3Avvp
                            : TPC::CONVERT_INT16g3Avvp;

      }
      return getIntConvertOpCode(DestVT, MVT::v128i16, VectorPredicate,
                                 Subtarget, Shift, IsUnsigned);

    } else if (SrcVT == MVT::v128i32 && DestVT == MVT::v128i16) {
      if (isgre || Features[TPC::FeatureDoron1]) {
        if (VectorPredicate) {
          if (ShiftisImm) {
            return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vim
                                : TPC::CONVERT_INT32g3i16vim;
          }
          if (ShiftisScalar) {
            return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vsm
                                : TPC::CONVERT_INT32g3i16vsm;
          }
          return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vvm
                              : TPC::CONVERT_INT32g3i16vvm;
        }
        if (ShiftisImm) {
          return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vip
                              : TPC::CONVERT_INT32g3i16vip;
        }
        if (ShiftisScalar) {
          return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vsp
                              : TPC::CONVERT_INT32g3i16vsp;
        }
        return (IsUnsigned) ? TPC::CONVERT_UINT32g3i16vvp
                            : TPC::CONVERT_INT32g3i16vvp;
      }
      return getIntConvertOpCode(DestVT, MVT::v64i32, VectorPredicate,
                                   Subtarget, Shift, IsUnsigned);
    } else if (SrcVT == MVT::v512i8 && DestVT == MVT::v256i8) {
      if (isgre) {
        if (VectorPredicate) {
          if (ShiftisImm) {
            return (IsUnsigned) ? TPC::CONVERT_UINT8vim
                                : TPC::CONVERT_INT8vim;
          }
          if (ShiftisScalar) {
            return (IsUnsigned) ? TPC::CONVERT_UINT8vsm
                                : TPC::CONVERT_INT8vsm;
          }
          return (IsUnsigned) ? TPC::CONVERT_UINT8vvm
                              : TPC::CONVERT_INT8vvm;
        }
        if (ShiftisImm) {
          return (IsUnsigned) ? TPC::CONVERT_UINT8vip
                              : TPC::CONVERT_INT8vip;
        }
        if (ShiftisScalar) {
          return (IsUnsigned) ? TPC::CONVERT_UINT8vsp
                              : TPC::CONVERT_INT8vsp;
        }
        return (IsUnsigned) ? TPC::CONVERT_UINT8vvp
                            : TPC::CONVERT_INT8vvp;
      }
    } else {
      return getIntConvertOpCode(DestVT, MVT::v64i32, VectorPredicate,
                                 Subtarget, Shift, IsUnsigned);
      assert(false && "Impossible conversion");    
      }
    
  } else if (SrcMultiplicity == 4 && DestMultiplicity == 1) { // i32->i8
    if (isgre) {
      if (VectorPredicate) {
        if (ShiftisImm) {
          return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vim
                              : TPC::CONVERT_INT32g3i8vim;
        }
        if (ShiftisScalar) {
          return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vsm
                              : TPC::CONVERT_INT32g3i8vsm;
        }
        return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vvm
                            : TPC::CONVERT_INT32g3i8vvm;
      }

      if (ShiftisImm) {
        return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vip
                            : TPC::CONVERT_INT32g3i8vip;
      }
      if (ShiftisScalar) {
        return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vsp
                            : TPC::CONVERT_INT32g3i8vsp;
      }
      return (IsUnsigned) ? TPC::CONVERT_UINT32g3i8vvp
                          : TPC::CONVERT_INT32g3i8vvp;

    }
    if (SrcVT == MVT::v256i32 && DestVT == MVT::v256i8) {
      return getIntConvertOpCode(DestVT, MVT::v64i32, VectorPredicate, Subtarget,
                                 Shift, IsUnsigned);
    } else {
      assert(false && "Impossible conversion");
    }

    } else if (SrcMultiplicity == 1 && DestMultiplicity == 1) {
    if (SrcVT == MVT::v64i32 &&
        (DestVT == MVT::v256i8 || DestVT == MVT::v128i16)) {
      if (VectorPredicate) {
        if (ShiftisImm) {
          return (IsUnsigned)
                     ? (isgo ? TPC::CONVERT_UINT32vim
                             : TPC::CONVERT_UINT32g2vim)
                     : (isgo ? TPC::CONVERT_INT32vim : TPC::CONVERT_INT32g2vim);
        }
        if (ShiftisScalar) {
          return (IsUnsigned)
                     ? (isgo ? TPC::CONVERT_UINT32vsm
                             : TPC::CONVERT_UINT32g2vsm)
                     : (isgo ? TPC::CONVERT_INT32vsm : TPC::CONVERT_INT32g2vsm);
        }
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT32vvm : TPC::CONVERT_UINT32g2vvm)
                   : (isgo ? TPC::CONVERT_INT32vvm : TPC::CONVERT_INT32g2vvm);
      }

      if (ShiftisImm) {
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT32vip : TPC::CONVERT_UINT32g2vip)
                   : (isgo ? TPC::CONVERT_INT32vip : TPC::CONVERT_INT32g2vip);
      }
      if (ShiftisScalar) {
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT32vsp : TPC::CONVERT_UINT32g2vsp)
                   : (isgo ? TPC::CONVERT_INT32vsp : TPC::CONVERT_INT32g2vsp);
      }
      return (IsUnsigned)
                 ? (isgo ? TPC::CONVERT_UINT32vvp : TPC::CONVERT_UINT32g2vvp)
                 : (isgo ? TPC::CONVERT_INT32vvp : TPC::CONVERT_INT32g2vvp);

    } else if (SrcVT == MVT::v128i16 && DestVT == MVT::v256i8) {
      if (VectorPredicate) {
        if (ShiftisImm) {
          return (IsUnsigned)
                     ? (isgo ? TPC::CONVERT_UINT16vim
                             : TPC::CONVERT_UINT16g2vim)
                     : (isgo ? TPC::CONVERT_INT16vim : TPC::CONVERT_INT16g2vim);
        }
        if (ShiftisScalar) {
          return (IsUnsigned)
                     ? (isgo ? TPC::CONVERT_UINT16vsm
                             : TPC::CONVERT_UINT16g2vsm)
                     : (isgo ? TPC::CONVERT_INT16vsm : TPC::CONVERT_INT16g2vsm);
        }
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT16vvm : TPC::CONVERT_UINT16g2vvm)
                   : (isgo ? TPC::CONVERT_INT16vvm : TPC::CONVERT_INT16g2vvm);
      }

      if (ShiftisImm) {
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT16vip : TPC::CONVERT_UINT16g2vip)
                   : (isgo ? TPC::CONVERT_INT16vip : TPC::CONVERT_INT16g2vip);
      }
      if (ShiftisScalar) {
        return (IsUnsigned)
                   ? (isgo ? TPC::CONVERT_UINT16vsp : TPC::CONVERT_UINT16g2vsp)
                   : (isgo ? TPC::CONVERT_INT16vsp : TPC::CONVERT_INT16g2vsp);
      }
      return (IsUnsigned)
                 ? (isgo ? TPC::CONVERT_UINT16vvp : TPC::CONVERT_UINT16g2vvp)
                 : (isgo ? TPC::CONVERT_INT16vvp : TPC::CONVERT_INT16g2vvp);
    }
  }
      return 0;
}

// extract scalar or imm if possible
static SDValue prepareShift(SDValue* Shift, SelectionDAG &DAG, const SDLoc &DL) {
  EVT ShiftType = MVT::i8; // by default now , will i32 may be
  if (Shift == nullptr)
    return DAG.getTargetConstant(0, DL, ShiftType);
  SDValue Element;
  if (isSplatBuildVector(*Shift, Element)) {
    if (Element.getOpcode() == ISD::Constant) {     
      unsigned ArgVal = cast<ConstantSDNode>(Element)->getZExtValue();
      ShiftType = Shift->getValueType();
      if (ShiftType.isVector()) {
        ShiftType = ShiftType.getVectorElementType();
      }
      return DAG.getTargetConstant(ArgVal, DL, ShiftType);
    } else {
      return Element;
    }
  } else if (Shift->getOpcode() == ISD::Constant) {
    unsigned ArgVal = cast<ConstantSDNode>(*Shift)->getZExtValue();
    return DAG.getTargetConstant(ArgVal, DL, ShiftType);
  }
  return *Shift;
}

static SmallVector<SDValue, 2> 
                      helperExtend_16_32(SDValue Src, SDValue DataType, EVT ResultVT,
                   SelectionDAG &DAG, const SDLoc &DL,
                   const TPCSubtarget *Subtarget) {
  unsigned DataTypeVal = cast<ConstantSDNode>(DataType)->getZExtValue();

  auto SrcType = Src.getValueType();
  SmallVector<SDValue, 2> ExtArg;
  const FeatureBitset &Features = Subtarget->getFeatureBits();

  if (Features[TPC::FeatureGen2Plus] || Features[TPC::FeatureGen2BPlus]) {
    // mov-group
    SmallVector<SDValue, 8> Ops(6);
    Ops[0] = Src; // Source.
    Ops[1] = DAG.getTargetConstant(-1, DL, MVT::i32);
    Ops[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_EN | TPCII::SW_DUAL_GROUP_EN),
                              DL, MVT::i32);        // Switch.
    Ops[3] = DAG.getUNDEF(SrcType);                 // Income.
    Ops[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node =
        DAG.getMachineNode(TPC::MOV_GROUP_vpu_vp, DL, SrcType, Ops);
    SDValue mdg = SDValue(Node, 0);

    // mov-dual-group
    // mov_dg.all sdg0=0 sdg1=2 sdg2=1 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=3 %V8,
    // %V1, -0x1, %SP0;
    SmallVector<SDValue, 8> Ops1(7);
    Ops1[0] = Src;                                     // Source.
    Ops1[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops1[2] = DAG.getTargetConstant(
        (((0 << TPCII::SW_SDG0_SHIFT) | (2 << TPCII::SW_SDG1_SHIFT) |
          (1 << TPCII::SW_SDG2_SHIFT) | (3 << TPCII::SW_SDG3_SHIFT) |
          (3 << TPCII::SW_WEG0_SHIFT) | (2 << TPCII::SW_WEG1_SHIFT) |
          (1 << TPCII::SW_WEG2_SHIFT) | (3 << TPCII::SW_WEG3_SHIFT)) |
         TPCII::SW_MDG_TYPE_ALL),
        DL, MVT::i32);                               // Switch.
    Ops1[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops1[4] = DAG.getUNDEF(SrcType);                 // Income.
    Ops1[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops1[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node1 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, SrcType, Ops1);
    SDValue mdg1 = SDValue(Node1, 0);

    // mov-dual-group
    // mov_dg.all sdg0=2 sdg1=0 sdg2=3 sdg3=1 weg0=2 weg1=1 weg2=2 weg3=1 %V5,
    // %V4, -0x1, %SP0;
    SmallVector<SDValue, 8> Ops2(7);
    Ops2[0] = mdg;                                     // Source.
    Ops2[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops2[2] = DAG.getTargetConstant(                   // 0,
        (((2 << TPCII::SW_SDG0_SHIFT) | (0 << TPCII::SW_SDG1_SHIFT) |
          (3 << TPCII::SW_SDG2_SHIFT) | (1 << TPCII::SW_SDG3_SHIFT) |
          (2 << TPCII::SW_WEG0_SHIFT) | (1 << TPCII::SW_WEG1_SHIFT) |
          (2 << TPCII::SW_WEG2_SHIFT) | (1 << TPCII::SW_WEG3_SHIFT)) |
         TPCII::SW_MDG_TYPE_ALL),
        DL, MVT::i32);                               // Switch.
    Ops2[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
    Ops2[4] = mdg1;                                  // Income.
    Ops2[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 =
        DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, SrcType, Ops2);
    SDValue mdg2 = SDValue(Node2, 0);

    // unpack-1
    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = mdg2;                                             // Source.
    Ops3[1] = DAG.getTargetConstant(DataTypeVal, DL, MVT::i32); // DataType.
    Ops3[2] = DAG.getTargetConstant(0, DL, MVT::i32);           // Switch.
    Ops3[3] = DAG.getConstant(0, DL, MVT::v128i16);             // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);               // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1);            // Polarity.
    MachineSDNode *Node3 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops3);
    SDValue mdg3 = SDValue(Node3, 0);

    // unpack-2
    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = mdg2;                                             // Source.
    Ops4[1] = DAG.getTargetConstant(DataTypeVal, DL, MVT::i32); // DataType.
    Ops4[2] =
        DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
    Ops4[3] = DAG.getConstant(0, DL, MVT::v128i16);                  // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 = DAG.getMachineNode(TPC::UNPACKp, DL, SrcType, Ops4);
    SDValue mdg4 = SDValue(Node4, 0);

    ExtArg.push_back(mdg3);
    ExtArg.push_back(mdg4);
  } else {
    assert(false && "transposition is not suppported for this platform");
    // cod below proved incorrect
  }

  return ExtArg;
}

static SDValue MoveWithPred(SDValue Source, EVT DestVT, SDValue Income,
                            SDValue Predicate, unsigned PolarityVal,
                            SelectionDAG &DAG, const SDLoc &DL) {
  SmallVector<SDValue, 5> Ops1;
  Ops1.push_back(Source); // Source.
  Ops1.push_back(DAG.getTargetConstant(0, DL, MVT::i32));

  Ops1.push_back(Income);                                          // Income.
  Ops1.push_back(Predicate);                                       // Predicate.
  Ops1.push_back(DAG.getTargetConstant(PolarityVal, DL, MVT::i1)); // Polarity.

  MachineSDNode *Node1 = DAG.getMachineNode(TPC::MOVvvm, DL, DestVT, Ops1);
  return SDValue(Node1, 0);
}

SDValue TPCTargetLowering::QVTranspose0213(SDValue Source, SelectionDAG &DAG,
                                           const SDLoc &DL) const {
  EVT SrcVT = Source.getValueType();
  EVT qSrcVT = getQuarterEVT(SrcVT);
  SDValue Source0 =
      helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qSrcVT, Source);
  SDValue Source1 =
      helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qSrcVT, Source);
  SDValue Source2 =
      helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qSrcVT, Source);
  SDValue Source3 =
      helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qSrcVT, Source);

  return createTuple({Source0, Source2, Source1, Source3}, DAG);
}

bool TPCTargetLowering::PreludeForVP(EVT DestVT, SDValue &Source,
                                     SDValue DataType, SDValue Switches,
                                     SDValue &Income, SDValue &Predicate,
                                     SDValue &Polarity, SelectionDAG &DAG,
                                     const SDLoc &DL, bool DestIsUnsigned,
                                     bool SrcIsUnsigned) const {
  bool IsVectorPredicate = Predicate.getValueType() == MVT::v256i1;
  if (IsVectorPredicate) {
    EVT SrcVT = Source.getValueType();
    unsigned SrcMultiplicity = getMultiplicity(SrcVT);
    unsigned DestMultiplicity = getMultiplicity(DestVT);
    if (SrcMultiplicity == 1 && DestMultiplicity > 1) {
      EVT SrcEltVT = SrcVT.getVectorElementType();
      EVT DestEltVT = DestVT.getVectorElementType();
      bool PolarityVal = cast<ConstantSDNode>(Polarity)->getSExtValue() != 0;
      // need income for Src Move with Pred
      unsigned DDT = getInstrDataType(DestEltVT, DestIsUnsigned);
      unsigned SWS = getSwitchByDestDataType(SrcEltVT, SrcIsUnsigned);
      SDValue SrcIncome = ConversionEntity(
          SrcVT, Income, DAG.getTargetConstant(DDT, DL, MVT::i8),
          DAG.getTargetConstant(SWS, DL, MVT::i32), DAG.getUNDEF(SrcVT),
          DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
          DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL);
      Source = MoveWithPred(Source, SrcVT, SrcIncome, Predicate, PolarityVal,
                            DAG, DL);
    }
    Predicate = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
    Polarity = DAG.getTargetConstant(0, DL, MVT::i1);
    Income = DAG.getUNDEF(DestVT);
  }
  return Predicate.getValueType() == MVT::v256i1;
}

SDValue TPCTargetLowering::ConversionEntity(EVT DestVT, SDValue Source,
                                            SDValue DataType, SDValue Switches,
                                            SDValue Income, SDValue Predicate,
                                            SDValue Polarity, SelectionDAG &DAG,
                                            const SDLoc &DL, bool IsSwizzle,
                                            SDValue *ShiftForInt,
                                            bool Onlysub_0) const {
  if (Source.isUndef()) {
    return DAG.getUNDEF(DestVT);
  }

  EVT SrcVT = Source.getValueType();
  bool IsVectorPredicate = (Predicate.getValueType() == MVT::v256i1);
  assert(SrcVT.isVector() == DestVT.isVector());
  assert(Polarity.getValueType() == MVT::i1);
  assert(isa<ConstantSDNode>(DataType));
  assert(isa<ConstantSDNode>(Switches));
  assert(Income.getValueType() == DestVT);
  assert(IsVectorPredicate || Predicate.getValueType() == MVT::i1);
  assert(isa<ConstantSDNode>(Polarity));

  // Scalar data is not handled here.
  if (!SrcVT.isVector())
    return SDValue();

  unsigned SwitchVal = cast<ConstantSDNode>(Switches)->getZExtValue();
  // check and clean linearity
  if (SwitchVal & TPCII::SW_LINEAR) {
    IsSwizzle = false;
    SwitchVal &= ~TPCII::SW_LINEAR;
  }

  unsigned DataTypeVal = cast<ConstantSDNode>(DataType)->getZExtValue();
  bool PolarityVal = cast<ConstantSDNode>(Polarity)->getSExtValue() != 0;

  // Derermine unsigned types.

  bool SrcIsUnsigned;
  switch (DataTypeVal) {
  case TPCII::UINT32:
  case TPCII::UINT16:
  case TPCII::UINT8:
  case TPCII::UINT4:
    SrcIsUnsigned = true;
    break;
  default:
    SrcIsUnsigned = false;
    break;
  }

  bool DestIsUnsigned;
  switch (SwitchVal & TPCII::SW_TO_TYPE) {
  case TPCII::SW_TO_UINT32:
  case TPCII::SW_TO_UINT16:
  case TPCII::SW_TO_UINT8:
    DestIsUnsigned = true;
    if (ShiftForInt)
      SrcIsUnsigned = true;
    break;
  default:
    DestIsUnsigned = false;
    break;
  }

  // Get and check source and destination type properties.

  EVT SrcEltVT = SrcVT.getVectorElementType();
  EVT DestEltVT = DestVT.getVectorElementType();
  unsigned SrcNumElements = SrcVT.getVectorNumElements();
  unsigned DestNumElements = DestVT.getVectorNumElements();
 // bool AllLanes = (SrcNumElements == DestNumElements);
  unsigned SrcElBitsize = SrcEltVT.getSizeInBits();
  unsigned DestElBitsize = DestEltVT.getSizeInBits();

  unsigned SrcMultiplicity = getMultiplicity(SrcVT);
  unsigned DestMultiplicity = getMultiplicity(DestVT);
  unsigned CommonMultiplicity = std::min(SrcMultiplicity, DestMultiplicity);
  assert(SrcMultiplicity % CommonMultiplicity == 0);
  assert(DestMultiplicity % CommonMultiplicity == 0);

  // Check consistency of DataType and Switches.

  if (SrcEltVT == MVT::f32) {
    assert(DataTypeVal == T_FP32);
  } else if (SrcEltVT == MVT::bf16) {
    assert(DataTypeVal == T_BF16);
  } else if (SrcEltVT == MVT::f16) {
    assert(DataTypeVal == T_FP16);
  } else if (SrcEltVT == MVT::f8_143) {
    assert(DataTypeVal == T_FP8_143);
  } else if (SrcEltVT == MVT::f8_152) {
    assert(DataTypeVal == T_FP8_152);
  } else if (SrcEltVT == MVT::i32) {
    assert((DataTypeVal == T_INT32 || DataTypeVal == T_UINT32));
  } else if (SrcEltVT == MVT::i16) {
    assert((DataTypeVal == T_INT16 || DataTypeVal == T_UINT16));
  } else if (SrcEltVT == MVT::i8) {
    assert(DataTypeVal == T_INT8 || DataTypeVal == T_UINT8 ||
           DataTypeVal == T_INT4 || DataTypeVal == T_UINT4);
  } else if (SrcEltVT == MVT::i1) {
    assert(DataTypeVal == T_INT1);
  }
  unsigned DestTypeSwitch = SwitchVal & TPCII::SW_TO_TYPE;
  assert(DestEltVT != MVT::f32 || DestTypeSwitch == TPCII::SW_TO_FP32);
  assert(DestEltVT != MVT::bf16 || DestTypeSwitch == TPCII::SW_TO_BF16);
  assert(DestEltVT != MVT::f16 || DestTypeSwitch == TPCII::SW_TO_FP16);
  assert(DestEltVT != MVT::f8_143 || DestTypeSwitch == TPCII::SW_TO_FP8_143);
  assert(DestEltVT != MVT::f8_152 || DestTypeSwitch == TPCII::SW_TO_FP8_152);
  if (ShiftForInt == nullptr) {
    assert(DestEltVT != MVT::i32 || (DestTypeSwitch == TPCII::SW_TO_INT32 ||
                                     DestTypeSwitch == TPCII::SW_TO_UINT32));
    assert(DestEltVT != MVT::i16 || (DestTypeSwitch == TPCII::SW_TO_INT16 ||
                                     DestTypeSwitch == TPCII::SW_TO_UINT16));
    assert(DestEltVT != MVT::i8 || (DestTypeSwitch == TPCII::SW_TO_INT8 ||
                                    DestTypeSwitch == TPCII::SW_TO_UINT8));
  }
  unsigned bitlen = 0;
  const FeatureBitset &Features = Subtarget->getFeatureBits();

  CVT DestCVT = toCVT(DestVT, DestIsUnsigned, bitlen);

  if (DataTypeVal == T_INT4 || DataTypeVal == T_UINT4) {
    bitlen = 4;
  }
  
  CVT SrcCVT = toCVT(SrcVT, SrcIsUnsigned, bitlen);
  if (SrcCVT == DestCVT) {
    return Source;
  }
  TConvKind ConvElCoKind = FindConversionElement(SrcCVT, DestCVT);
  if (ShiftForInt) {
    assert(ConvElCoKind != Hard);
  }
  if (ConvElCoKind == Skip) {
    return SDValue();
  }
  if (ConvElCoKind == HardLane) {
    ConvElCoKind = (SwitchVal & TPCII::SW_LANE_SEL) ? Soft : Hard;
  }
  if (ConvElCoKind == HardOrIntHard) {
    ConvElCoKind = (ShiftForInt || Features[TPC::FeatureGaudi] ||
                    Features[TPC::FeatureGaudiB] || Features[TPC::FeatureGreco])
                       ? IntHard
                       : Hard;
  } else if (ConvElCoKind == IntLoUpLanes) {
    ConvElCoKind = (ShiftForInt) ? IntSoft : LoUpLanes;
  } else if (ConvElCoKind == IntSoft) {
    ConvElCoKind = (ShiftForInt || Features[TPC::FeatureGaudi] ||
                    Features[TPC::FeatureGaudiB] || Features[TPC::FeatureGreco])
                       ? IntSoft
                       : Soft;
  }
   // Conversion formed as couple lower + upper
  // Now processed 1 part lower or upper, It is needed to process
  // as hard or int otherwise will be endles recursion
  if ((ConvElCoKind == LoUpLanes ||
      ConvElCoKind == IntLoUpLanes) &&
      (SwitchVal & (TPCII::SW_LOWER_LANES_SRCB | TPCII::SW_UPPER_LANES_SRCB |
                    TPCII::SW_LOWER_LANES | TPCII::SW_UPPER_LANES))) {
    ConvElCoKind = (ShiftForInt)? IntHard : Hard;
  }
  if (ConvElCoKind == Empty) {
    assert(false && "Conversion is not set");
    return SDValue();
  }
  if (ConvElCoKind == Ban) {
    report_fatal_error("Conversion uses type banned for cpu");
    return SDValue();
  }
  if (ConvElCoKind == XX) {
    assert(DestMultiplicity == SrcMultiplicity);
    // Optimization: insert switch to provide instruction with double vectors
    if (DestMultiplicity == 2) {
      SwitchVal |= TPCII::SW_X2_CONVERT;
    } else if (DestMultiplicity == 4) {
      SwitchVal |= TPCII::SW_X4_CONVERT;
    }

    if ((SwitchVal & (TPCII::SW_X4_CONVERT | TPCII::SW_X2_CONVERT)) ==
        0) { // No so switch
      ConvElCoKind = Split;
    } else {
      ConvElCoKind = Hard;
    }
  }
  bool is_vector_truncate = (SrcElBitsize > DestElBitsize);
  bool is_vector_extend =  (SrcElBitsize < DestElBitsize);

  SDValue SavePredicate = Predicate, SaveIncome = Income;
  unsigned SavePolarityVal = PolarityVal;
  SDValue FinalConv = SDValue();

  if (ConvElCoKind == BoolTo) {
    unsigned one;
    if (DestEltVT == MVT::f32) {
      one = 0x3f800000;
    } else if (DestEltVT == MVT::bf16) {
      one = 0x3f80;
    } else if (DestEltVT == MVT::f8_143) {
      one = 0x68;
    } else if (DestEltVT == MVT::f8_152) {
      one = 0x54;
    } else {
      one = 1;
    }
    // For a while no attention to VP and Polarity
    // transform bool vector to int or fp vector
    Income =
        SDValue(DAG.getMachineNode(
                    TPC::MOV_ld_vip, DL, DestVT,
                    {DAG.getTargetConstant(0, DL, MVT::i32), /*Src*/
                     DAG.getTargetConstant(getInstrDataType(DestEltVT, false), DL, MVT::i8),                             /*Optype*/
                     DAG.getTargetConstant(0, DL, MVT::i32),        /*Switch*/
                     DAG.getUNDEF(DestVT),                          /*Income*/
                     DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
                     DAG.getTargetConstant(0, DL, MVT::i1)}),  /*Polarity*/
                0);
    SmallVector<SDValue, 8> Ops1;
    Ops1.push_back(DAG.getTargetConstant(one, DL, MVT::i32)); // Source.
    Ops1.push_back(DAG.getTargetConstant(DataTypeVal, DL, MVT::i8));
    Ops1.push_back(DAG.getTargetConstant(0, DL, MVT::i32)),        /*Switch*/
    Ops1.push_back(Income); // Income.
    Ops1.push_back(Source);                               // Predicate.
    Ops1.push_back(DAG.getTargetConstant(0, DL, MVT::i1)); // Polarity.
    MachineSDNode *Node1 = DAG.getMachineNode(TPC::MOVvim, DL, DestVT, Ops1);
    return SDValue(Node1, 0);
  }

  if (!IsSwizzle &&
      ConvElCoKind != Split) { // it is not clear what to do with VP
    if (is_vector_extend) {
      unsigned work_swi = SwitchVal | TPCII::SW_LANE_0 | TPCII::SW_NUM_LANES;
      if (ConvElCoKind == IntSoft || ConvElCoKind == IntHard) {
        work_swi |= TPCII::SW_SINGLE_LANE;
      } else {
        work_swi |= TPCII::SW_NUM_LANES_SRCB;
      }
      EVT halfDestVT = getHalfEVT(DestVT);
      PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                   Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
      SmallVector<SDValue, 2> ExtArg;
      if (DestMultiplicity == 2 && SrcMultiplicity == 1) {
        if (DestElBitsize == 32 && SrcElBitsize == 16) {
          ExtArg =
              helperExtend_16_32(Source, DataType, SrcVT, DAG, DL, Subtarget);
        } else if (DestElBitsize == 16 && SrcElBitsize == 8) {
          ExtArg = Extend_8_to_16(SrcVT, Source, DAG, DL, DataTypeVal);
        } else {
          assert(false && "impossible extend conversion");
          return SDValue();
        }
        SDValue conv0, conv1;
        if (SrcIsUnsigned && DestIsUnsigned) {
          conv0 = DAG.getNode(ISD::BITCAST, DL, halfDestVT, ExtArg[0]);
          conv1 = DAG.getNode(ISD::BITCAST, DL, halfDestVT, ExtArg[1]);
          FinalConv = createTuple({conv0, conv1}, DAG);
          goto RTExit;
        }
        conv0 = ConversionEntity(halfDestVT, ExtArg[0], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(halfDestVT), Predicate, Polarity,
                                 DAG, DL, true, ShiftForInt, true);

        conv1 = ConversionEntity(halfDestVT, ExtArg[1], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(halfDestVT), Predicate, Polarity,
                                 DAG, DL, true, ShiftForInt, true);

        FinalConv = createTuple({conv0, conv1}, DAG);
        goto RTExit;
      } else if (DestMultiplicity == 4 && SrcMultiplicity == 1) {
        assert(DestElBitsize == 32 && SrcElBitsize == 8);
        EVT qVT = getQuarterEVT(DestVT);
        SmallVector<SDValue, 4> ExtArg;
        ExtArg = Extend_8_to_32(SrcVT, Source, DAG, DL, DataTypeVal);
        SDValue conv0, conv1, conv2, conv3;
        if (SrcIsUnsigned && DestIsUnsigned) {
          conv0 = DAG.getNode(ISD::BITCAST, DL, qVT, ExtArg[0]);
          conv1 = DAG.getNode(ISD::BITCAST, DL, qVT, ExtArg[1]);
          conv2 = DAG.getNode(ISD::BITCAST, DL, qVT, ExtArg[2]);
          conv3 = DAG.getNode(ISD::BITCAST, DL, qVT, ExtArg[3]);
          FinalConv = createTuple({conv0, conv1, conv2, conv3}, DAG);
          goto RTExit;
        }

        conv0 = ConversionEntity(qVT, ExtArg[0], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(qVT), Predicate, Polarity, DAG,
                                 DL);

        conv1 = ConversionEntity(qVT, ExtArg[1], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(qVT), Predicate, Polarity, DAG,
                                 DL);
        conv2 = ConversionEntity(qVT, ExtArg[2], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(qVT), Predicate, Polarity, DAG,
                                 DL);

        conv3 = ConversionEntity(qVT, ExtArg[3], DataType,
                                 DAG.getTargetConstant(work_swi, DL, MVT::i32),
                                 DAG.getUNDEF(qVT), Predicate, Polarity, DAG,
                                 DL);

        FinalConv = createTuple({conv0, conv1, conv2, conv3}, DAG);
        goto RTExit;
      } else {
        assert(false && "extension is not implemented yet");
        return SDValue();
      }
    } else if (is_vector_truncate) {

      PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                   Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
      unsigned TargetTypeVal = (SwitchVal & TPCII::SW_TO_TYPE) >> 8;

      unsigned work_swi = SwitchVal | TPCII::SW_LANE_0 | TPCII::SW_NUM_LANES;
      if (ConvElCoKind == IntSoft || ConvElCoKind == IntHard) {
        work_swi |= TPCII::SW_SINGLE_LANE;
      } else {
        work_swi |= TPCII::SW_NUM_LANES_SRCB;
      }
      SDValue qSwitch = DAG.getTargetConstant(work_swi, DL, MVT::i32);

      if (DestMultiplicity == 1 && SrcMultiplicity == 2) {
        EVT halfSrcVT = getHalfEVT(SrcVT);
        SDValue pref0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfSrcVT, Source);
        SDValue pref1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfSrcVT, Source);
 
        SDValue Converted0 = ConversionEntity(DestVT, pref0, DataType, qSwitch,
                                              Income, Predicate, Polarity,
                                              DAG, DL, true, ShiftForInt, true);

        SDValue Converted1 = ConversionEntity(DestVT, pref1, DataType, qSwitch,
                                              Income, Predicate, Polarity,
                                              DAG, DL, true, ShiftForInt, true);
        if (DestElBitsize == 16 && SrcElBitsize == 32) {
          if (Features[TPC::FeatureGen2Plus] ||
              Features[TPC::FeatureGen2BPlus]) {
            FinalConv = truncate_32_to_16(DAG, Converted0, Converted1, DL,
                                          DestVT, TargetTypeVal);
            goto RTExit;

          } else {
            assert(false && 
                " truncate transposition is not suppported for this platform");
            return SDValue();
          }
        }
        if (DestElBitsize == 8 && SrcElBitsize == 16) {
          FinalConv = truncate_32_to_16(DAG, Converted0, Converted1, DL, DestVT,
                                        TargetTypeVal);
          goto RTExit;
        }
      } else if (DestMultiplicity == 1 && SrcMultiplicity == 4 &&
                 DestElBitsize == 8 && SrcElBitsize == 32) {
        EVT qSrcVT = getQuarterEVT(SrcVT);
        SDValue pref0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qSrcVT, Source);
        SDValue pref1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qSrcVT, Source);
        SDValue pref2 =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qSrcVT, Source);
        SDValue pref3 =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qSrcVT, Source);


        SDValue Converted0 = ConversionEntity(DestVT, pref0, DataType, qSwitch,
                                              Income, Predicate, Polarity,
            DAG, DL, true, ShiftForInt, true);
        SDValue Converted1 = ConversionEntity(DestVT, pref1, DataType, qSwitch,
                                              Income, Predicate, Polarity,
            DAG, DL, true, ShiftForInt, true);
        SDValue Converted2 = ConversionEntity(DestVT, pref2, DataType, qSwitch,
                                              Income, Predicate, Polarity,
            DAG, DL, true, ShiftForInt, true);
        SDValue Converted3 = ConversionEntity(DestVT, pref3, DataType, qSwitch,
                                              Income, Predicate, Polarity,
            DAG, DL, true, ShiftForInt, true);

        FinalConv = truncate_32_to_8(DAG, Converted0, Converted1, Converted2,
                                     Converted3, DL, DestVT, TargetTypeVal);

        goto RTExit;
      } else {
        assert(false && "Unsupported Truncate Conversion");
        return SDValue();
      }

    } else {
      // pass further: no ext? no trunc
    }
  }
 
  if (ConvElCoKind == Hard || ConvElCoKind == VPemu) {
    if (IsVectorPredicate && ConvElCoKind == VPemu) {
      IsVectorPredicate = PreludeForVP(DestVT, Source, DataType, Switches, Income,
                                    Predicate,
                   Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
    }
    unsigned OpCode =
        getConvertOpCode(DestVT, SrcVT, IsVectorPredicate, *Subtarget);
    if (OpCode == 0) {
      assert(false && "OpCode is not set");
      return SDValue();
    }
    SmallVector<SDValue, 8> Operands;
    Operands.push_back(Source);
    Operands.push_back(DAG.getTargetConstant(DataTypeVal, DL, MVT::i8));
    Operands.push_back(DAG.getTargetConstant(SwitchVal, DL, MVT::i32));
    Operands.push_back(Income);
    Operands.push_back(Predicate);
    Operands.push_back(DAG.getTargetConstant(PolarityVal, DL, MVT::i1));
    MachineSDNode *Node = DAG.getMachineNode(OpCode, DL, DestVT, Operands);
    return SDValue(Node, 0);
  }

  if (ConvElCoKind == IntHard) {
    assert(DestVT.isInteger() && SrcVT.isInteger());
    unsigned int ConvertInstr;
    unsigned szel = DestElBitsize;
    unsigned swd = (szel == 8) ? TPCII::SW_TO_8 : TPCII::SW_TO_16;
    SDValue ShifForInstr = prepareShift(ShiftForInt, DAG, DL);
    ConvertInstr = getIntConvertOpCode(DestVT, SrcVT, IsVectorPredicate,
                                       Subtarget,
                                       ShiftForInt, SrcIsUnsigned);
    SmallVector<SDValue, 8> Ops(6);
    Ops[0] = Source; // Source.
    Ops[1] = ShifForInstr;
    Ops[2] = DAG.getTargetConstant(SwitchVal | swd, DL, MVT::i32); // Switch.
    Ops[3] = SaveIncome;
    Ops[4] = SavePredicate;
    Ops[5] = DAG.getTargetConstant(SavePolarityVal, DL, MVT::i1);
    MachineSDNode *Node = DAG.getMachineNode(ConvertInstr, DL, DestVT, Ops);
    return SDValue(Node, 0); // Hard instruction  
  }


   if (ConvElCoKind == LoUpLanes || ConvElCoKind == IntLoUpLanes) {
    if (DestCVT == SrcCVT) {
      return Source;
    }
    PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                 Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
    // Need to differ here common and int conversions
    SDValue sw0, sw1;
    if (ShiftForInt) {
      sw0 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LOWER_LANES, DL, MVT::i32);
      sw1 = DAG.getTargetConstant(SwitchVal | TPCII::SW_UPPER_LANES, DL, MVT::i32);
    } else {  
      sw0 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LOWER_LANES_SRCB, DL, MVT::i32);
      sw1 = DAG.getTargetConstant(SwitchVal | TPCII::SW_UPPER_LANES_SRCB, DL, MVT::i32);
    }
    if (DestMultiplicity == 1 && SrcMultiplicity == 4) {
      EVT qSrcVT = getQuarterEVT(SrcVT);
      SDValue Source0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qSrcVT, Source);
      SDValue Source1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qSrcVT, Source);
      SDValue Source2 = helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qSrcVT, Source);
      SDValue Source3 = helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qSrcVT, Source);

      SDValue h0 = createTuple({Source0, Source1}, DAG);
      SDValue h1 = createTuple({Source2, Source3}, DAG);
      SDValue conv0 =
          ConversionEntity(DestVT, h0, DataType, sw0, Income, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      SDValue conv1 =
          ConversionEntity(DestVT, h1, DataType, sw1, conv0, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      FinalConv = conv1;
      goto RTExit;


    } else if (DestMultiplicity == 4 && SrcMultiplicity == 1) {
      EVT halfDestVT = getHalfEVT(DestVT);
      EVT qDestVT = getQuarterEVT(DestVT);
      SDValue Income0, Income1, Income2, Income3,i0,i1;
      if (Income.isUndef()) {
        i0 = i1 = DAG.getUNDEF(halfDestVT);
      } 
      else {
        Income0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qDestVT, Income);
        Income1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qDestVT, Income);
        Income2 =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qDestVT, Income);
        Income3 =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qDestVT, Income);

        i0 = createTuple({Income0, Income1}, DAG);
        i1 = createTuple({Income2, Income3}, DAG);
      }
      SDValue conv0 =
          ConversionEntity(halfDestVT, Source, DataType, sw0, i0, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      SDValue conv1 =
          ConversionEntity(halfDestVT, Source, DataType, sw1, i1, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      SDValue c0 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qDestVT, conv0);
      SDValue c1 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qDestVT, conv0);
      SDValue c2 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qDestVT, conv1);
      SDValue c3 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qDestVT, conv1);

      FinalConv = createTuple({c0, c1, c2, c3}, DAG);
      goto RTExit;
    } else if (DestMultiplicity == 1 && SrcMultiplicity == 1) {  
      SDValue conv0 =
          ConversionEntity(DestVT, Source, DataType, sw0, Income, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      SDValue conv1 =
          ConversionEntity(DestVT, Source, DataType, sw1, conv0, Predicate,
                           Polarity, DAG, DL, IsSwizzle, ShiftForInt);
      FinalConv = conv1;
      goto RTExit;
    } else if (DestMultiplicity == 2 && SrcMultiplicity == 1) {  
      if (SrcEltVT.isFloatingPoint() && SrcElBitsize == 8 && DestEltVT.isInteger() && DestElBitsize==16) {
        // special way
        EVT halfDestVT = getHalfEVT(DestVT);
        SDValue conv0 = ConversionEntity(
            halfDestVT, Source, DataType, sw0, DAG.getUNDEF(halfDestVT),
            Predicate, Polarity, DAG, DL, IsSwizzle, ShiftForInt);
        SDValue conv1 = ConversionEntity(
            halfDestVT, Source, DataType, sw1, DAG.getUNDEF(halfDestVT),
            Predicate, Polarity, DAG, DL, IsSwizzle, ShiftForInt);
        FinalConv =  createTuple({conv0, conv1}, DAG);
        goto RTExit;
      } else {
        SDValue conv0 =
            ConversionEntity(DestVT, Source, DataType, sw0, Income, Predicate,
                             Polarity, DAG, DL, IsSwizzle, ShiftForInt);
        SDValue conv1 =
            ConversionEntity(DestVT, Source, DataType, sw1, conv0, Predicate,
                             Polarity, DAG, DL, IsSwizzle, ShiftForInt);
        FinalConv = conv1;
        goto RTExit;
      }
    } else {
      assert(false && "do not expect this");
    }
  }

  if (ConvElCoKind == Soft) {
    if (DestCVT == SrcCVT) {
      return Source;
    }    
    PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                 Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
    if (is_vector_truncate) {
      SDValue Source0, Source1, Source2, Source3;
      if (SrcMultiplicity == 2 && DestMultiplicity == 1) {
        EVT halfSrcVT = getHalfEVT(SrcVT);
        Source0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfSrcVT, Source);
        Source1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfSrcVT, Source);
        SDValue sw0 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_0 | TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);
        SDValue sw1 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_1 |
                                                TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);

        assert(!Features[TPC::FeatureGen3Plus] || Features[TPC::FeatureDoron1]); // it is dangerous to use 1
                                                 // vector when in fact used
                                                 // all lines
        SDValue conv0 = ConversionEntity(DestVT, Source0, DataType, sw0, Income,
                                         Predicate, Polarity, DAG, DL);
        FinalConv = conv0;
        if (!Onlysub_0) {
          SDValue conv1 = ConversionEntity(DestVT, Source1, DataType, sw1,
                                           conv0, Predicate, Polarity, DAG, DL);
          FinalConv = conv1;
        }
        goto RTExit;
      } else if (SrcMultiplicity == 4 && DestMultiplicity == 1) {
        EVT qSrcVT = getQuarterEVT(SrcVT);
        Source0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qSrcVT, Source);
        Source1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qSrcVT, Source);
        Source2 =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qSrcVT, Source);
        Source3 =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qSrcVT, Source);

        assert(!Features[TPC::FeatureGreco]);

        SDValue sw0 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_0 |
                                                TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);
        SDValue sw1 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_1 |
                                                TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);
        SDValue sw2 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_2 |
                                                TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);
        SDValue sw3 = DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_3 |
                                                TPCII::SW_NUM_LANES_SRCB,
                                            DL, MVT::i32);

        SDValue conv0 = ConversionEntity(DestVT, Source0, DataType, sw0, Income,
                                         Predicate, Polarity, DAG, DL);
        FinalConv = conv0;
        if (!Onlysub_0) {
          SDValue conv1 = ConversionEntity(DestVT, Source1, DataType, sw1,
                                           conv0, Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(DestVT, Source2, DataType, sw2,
                                           conv1, Predicate, Polarity, DAG, DL);
          SDValue conv3 = ConversionEntity(DestVT, Source3, DataType, sw3,
                                           conv2, Predicate, Polarity, DAG, DL);
          FinalConv = conv3;
        }
        goto RTExit;
      } else if (SrcMultiplicity == 1 && DestMultiplicity == 1) { 
        if (SrcCVT == CVT::v128bf16 && (DestCVT == CVT::v256i8 || DestCVT == CVT::v256u8)) {
          unsigned sw0_val =
              (SwitchVal & ~TPCII::SW_TO_TYPE) | TPCII::SW_TO_INT16;
          SDValue sw0 = DAG.getTargetConstant(sw0_val, DL, MVT::i32);
          SDValue conv1 = ConversionEntity(MVT::v128i16, Source, DataType, sw0,
                                           DAG.getUNDEF(MVT::v128i16),
                                           Predicate,
                                           Polarity, DAG, DL);
          // now int -> int conversion and rm switch can be erased
          unsigned sw1_val = (SwitchVal & ~TPCII::SW_SR_RNE);
          SDValue sw1 = DAG.getTargetConstant(sw1_val, DL, MVT::i32);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_INT16, DL, MVT::i8),
              sw1, Income, Predicate, Polarity, DAG, DL);
          FinalConv = conv2;
          goto RTExit;
        }
        if (SrcCVT == CVT::v128f16 &&
            (DestCVT == CVT::v256i8 || DestCVT == CVT::v256u8)) {
          unsigned sw0_val =
              (SwitchVal & ~(TPCII::SW_TO_TYPE | TPCII::SW_NUM_LANES_SRCB |
                             TPCII::SW_NUM_LANES)) | TPCII::SW_TO_BF16;
          SDValue sw0 = DAG.getTargetConstant(sw0_val, DL, MVT::i32);
          SDValue conv1 = ConversionEntity(MVT::v128bf16, Source, DataType, sw0,
                                           DAG.getUNDEF(MVT::v128bf16),
                                           Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_BF16, DL, MVT::i8),
              Switches, Income, Predicate, Polarity, DAG, DL);
          FinalConv = conv2;
          goto RTExit;
        }
        if (SrcCVT == CVT::v64u32 && 
             (DestCVT == CVT::v128bf16 || DestCVT == CVT::v128f16)) {
          SDValue sw0 = DAG.getTargetConstant((SwitchVal & ~TPCII::SW_TO_TYPE) |
                                                  TPCII::SW_TO_UINT16, DL, MVT::i32);

          SDValue conv1 = ConversionEntity(MVT::v128i16, Source, DataType, sw0,
                                           DAG.getUNDEF(MVT::v128i16),
                                           Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_UINT16, DL, MVT::i8),
              Switches, Income, Predicate, Polarity, DAG, DL);
          FinalConv = conv2;
          goto RTExit;
        }
        if (SrcCVT == CVT::v64i32 &&
            (DestCVT == CVT::v128bf16 || DestCVT == CVT::v128f16)) {
          SDValue sw0 = DAG.getTargetConstant((SwitchVal & ~TPCII::SW_TO_TYPE) |
                                                  TPCII::SW_TO_FP32,
                                              DL, MVT::i32);

          SDValue conv1 = ConversionEntity(MVT::v64f32, Source, DataType, sw0,
                                           DAG.getUNDEF(MVT::v64f32),
                                           Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              Switches, Income, Predicate, Polarity, DAG, DL);
          FinalConv = conv2;
          goto RTExit;
        }
        if (SrcCVT == CVT::v128u16 && DestCVT == CVT::v256i8) {
          SDValue sw0 = DAG.getTargetConstant((SwitchVal & ~TPCII::SW_TO_TYPE) |
                                                  TPCII::SW_TO_UINT8,
                                              DL, MVT::i32);

          SDValue conv1 = ConversionEntity(MVT::v256i8, Source, DataType, sw0,
                                           DAG.getUNDEF(MVT::v256i8),
                                           Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_UINT8, DL, MVT::i8),
              Switches, Income, Predicate, Polarity, DAG, DL);
          FinalConv = conv2;
          goto RTExit;
        }
        assert(false && "partial vector");
      }
      else {
        assert(false && "Support Me!");
        return SDValue();
      }
    }
    if (is_vector_extend) {
      if (SrcMultiplicity == 1 && DestMultiplicity == 2) {
        EVT halfDestVT = getHalfEVT(DestVT);
        SDValue Income0, Income1;
        if (Income.isUndef()) {
          Income0 = Income1 = DAG.getUNDEF(halfDestVT);
        } else {
          Income0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfDestVT,
                                               Income);
          Income1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfDestVT,
                                               Income);
        }
        unsigned swl = ((SwitchVal & (~TPCII::SW_LANE_SEL)) |
                        TPCII::SW_NUM_LANES_SRCB | TPCII::SW_NUM_LANES);
        SDValue conv0 =
            ConversionEntity(halfDestVT, Source, DataType,
                             DAG.getTargetConstant(swl | TPCII::SW_LANE_0, DL, MVT::i32),
                             Income0, Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            halfDestVT, Source, DataType,
            DAG.getTargetConstant(swl | TPCII::SW_LANE_1, DL, MVT::i32),
            Income1, Predicate, Polarity, DAG, DL);

        FinalConv = createTuple({conv0, conv1}, DAG);
        goto RTExit;
      }
      if (SrcMultiplicity == 1 && DestMultiplicity == 4) {
        EVT qDestVT = getQuarterEVT(DestVT);
        SDValue Income0, Income1, Income2, Income3;
        if (Income.isUndef()) {
          Income0 = Income1 = Income2 = Income3 = DAG.getUNDEF(qDestVT);
        } else {
          Income0 =
              helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qDestVT, Income);
          Income1 =
              helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qDestVT, Income);
          Income2 =
              helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qDestVT, Income);
          Income3 =
              helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qDestVT, Income);
        }
        SDValue sw0 =
            DAG.getTargetConstant(SwitchVal | TPCII::SW_LANE_0, DL, MVT::i32);
        assert(!Features[TPC::FeatureGreco]);
        SDValue conv0 = ConversionEntity(qDestVT, Source, DataType, sw0,
                                         Income0, Predicate, Polarity, DAG, DL);

        SDValue SourceI64 = DAG.getNode(ISD::BITCAST, DL, MVT::v64i32, Source);

        SmallVector<SDValue, 7> Operands = {
            SourceI64,
            DAG.getTargetConstant(8, DL, MVT::i32),
            DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                  MVT::i8),         /*Optype*/
            DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
            DAG.getUNDEF(MVT::v64i32),              /*Income*/
            DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),     /*Predicate*/
            DAG.getTargetConstant(0, DL, MVT::i1)   /*Polarity*/
        };
        MachineSDNode *NodeSourceI64_1 =
            DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands);
        SDValue SourceI64_1 = SDValue(NodeSourceI64_1, 0);
        SDValue Source1 = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceI64_1);
        SDValue conv1 = ConversionEntity(qDestVT, Source1, DataType, sw0,
                                         Income1, Predicate, Polarity, DAG, DL);

        SmallVector<SDValue, 7> Operands2 = {
            SourceI64_1,
            DAG.getTargetConstant(8, DL, MVT::i32),
            DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                  MVT::i8),         /*Optype*/
            DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
            DAG.getUNDEF(MVT::v64i32),              /*Income*/
            DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),     /*Predicate*/
            DAG.getTargetConstant(0, DL, MVT::i1)   /*Polarity*/
        };
        MachineSDNode *NodeSourceI64_2 =
            DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands2);
        SDValue SourceI64_2 = SDValue(NodeSourceI64_2, 0);

        Source = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceI64_2);
        SDValue conv2 = ConversionEntity(qDestVT, Source, DataType, sw0,
                                         Income2, Predicate, Polarity, DAG, DL);

        SmallVector<SDValue, 7> Operands3 = {
            SourceI64_2,
            DAG.getTargetConstant(8, DL, MVT::i32),
            DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                  MVT::i8),         /*Optype*/
            DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
            DAG.getUNDEF(MVT::v64i32),              /*Income*/
            DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),     /*Predicate*/
            DAG.getTargetConstant(0, DL, MVT::i1)   /*Polarity*/
        };
        MachineSDNode *NodeSourceI64_3 =
            DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands3);
        SDValue SourceI64_3 = SDValue(NodeSourceI64_3, 0);

        Source = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceI64_3);
        SDValue conv3 = ConversionEntity(qDestVT, Source, DataType, sw0,
                                         Income3, Predicate, Polarity, DAG, DL);

        FinalConv = createTuple({conv0, conv1, conv2, conv3}, DAG);
        goto RTExit;
      } else if (SrcMultiplicity == 1 && DestMultiplicity == 1) {
        unsigned which_lane = SwitchVal & TPCII::SW_LANE_SEL;
        SDValue SourceForConv = Source;
        unsigned shift = SrcElBitsize * which_lane;
        if (shift > 0) {
          SDValue SourceI64 =
              DAG.getNode(ISD::BITCAST, DL, MVT::v64i32, Source);          
          SmallVector<SDValue, 7> Operands = {
              SourceI64,
              DAG.getTargetConstant(shift, DL, MVT::i32),
              DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8),           /*Optype*/
              DAG.getTargetConstant(0, DL, MVT::i32),   /*Switch*/
              DAG.getUNDEF(MVT::v64i32),                /*Income*/
              DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
              DAG.getTargetConstant(0, DL, MVT::i1)     /*Polarity*/
          };
          MachineSDNode *NodeSourceI64_1 =
              DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands);
          SDValue SourceI64_1 = SDValue(NodeSourceI64_1, 0);
          SourceForConv = SourceI64_1;          
        }
        SourceForConv = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceForConv);
        FinalConv = ConversionEntity(
            DestVT, SourceForConv,
            DataType,
            DAG.getTargetConstant(SwitchVal & (~TPCII::SW_LANE_SEL), DL, MVT::i32),
                                         Income, Predicate, Polarity, DAG, DL);
        goto RTExit;

      }
      assert(false && "Support Me!");
      return SDValue();
    }

    //eq elements hw unsupported conversion
    // f16->f32->bf16
    if (SrcVT.isFloatingPoint() && DestVT.isFloatingPoint()) {
      unsigned newswi = (SwitchVal & ~TPCII::SW_TO_TYPE) | TPCII::SW_TO_FP32; 
      SDValue Income0;
      EVT DestVT0;
      if (SrcNumElements == 128) {
        Income0 = DAG.getUNDEF(MVT::v128f32);
        DestVT0 = MVT::v128f32;
      } else if (SrcNumElements == 256) {
        Income0 = DAG.getUNDEF(MVT::v256f32);
        DestVT0 = MVT::v256f32;
      } else {
        assert(false && "Bad Vector Length for it");
        return SDValue();
      }
      SDValue conv0 = ConversionEntity(
                                       DestVT0, Source, DataType,
                                       DAG.getTargetConstant(newswi, DL, MVT::i32),
                                       Income0, Predicate, Polarity, DAG, DL);
      FinalConv = ConversionEntity(DestVT, conv0,
                              DAG.getTargetConstant(T_FP32, DL, MVT::i8),
                              Switches, Income, Predicate, Polarity, DAG, DL);
      goto RTExit;
    }
    if (SrcVT.isFloatingPoint() && DestVT.isInteger()) {
      unsigned newswi = (SwitchVal & ~TPCII::SW_TO_TYPE);
      SDValue Income0;
      EVT DestVT0;
      SDValue Dest0DataType;

      if (SrcNumElements == 128) {
        if (DestIsUnsigned) { // try via up conversion
                              // f16->f32->i32->u32->u16 ?
          DestVT0 = MVT::v128f32;
          Dest0DataType = DAG.getTargetConstant(T_FP32, DL, MVT::i8);
          Income0 = DAG.getUNDEF(MVT::v128f32);
          unsigned newswi0 = newswi | TPCII::SW_TO_FP32;
          SDValue conv0 =
              ConversionEntity(DestVT0, Source, DataType,
                               DAG.getTargetConstant(newswi0, DL, MVT::i32),
                               Income0, Predicate, Polarity, DAG, DL);

          EVT DestVT1 = MVT::v128i32;
          unsigned newswi1 = newswi | TPCII::SW_TO_INT32;
          SDValue Income1 = DAG.getUNDEF(DestVT1);
          SDValue conv1 =
              ConversionEntity(DestVT1, conv0, Dest0DataType,
                               DAG.getTargetConstant(newswi1, DL, MVT::i32),
                               Income1, Predicate, Polarity, DAG, DL);

          SDValue Dest1DataType = DAG.getTargetConstant(T_INT32, DL, MVT::i8);
          EVT DestVT2 = MVT::v128i32;
          unsigned newswi2 = newswi | TPCII::SW_TO_UINT32;
          SDValue Income2 = DAG.getUNDEF(DestVT2);
          SDValue conv2 =
              ConversionEntity(DestVT2, conv1, Dest1DataType,
                               DAG.getTargetConstant(newswi2, DL, MVT::i32),
                               Income2, Predicate, Polarity, DAG, DL);

          SDValue Dest2DataType = DAG.getTargetConstant(T_UINT32, DL, MVT::i8);
          EVT DestVT3 = DestVT;
          unsigned newswi3 = newswi | ((DestElBitsize < 16)? TPCII::SW_TO_UINT8 : TPCII::SW_TO_UINT16);
          SDValue Income3 = DAG.getUNDEF(DestVT3);
          SDValue conv3 =
              ConversionEntity(DestVT3, conv2, Dest2DataType,
                               DAG.getTargetConstant(newswi3, DL, MVT::i32),
                               Income3, Predicate, Polarity, DAG, DL);

          FinalConv = conv3;
          goto RTExit;
        } // if (DestIsUnsigned)
        else {
          if (DestNumElements == 128) {
            DestVT0 = MVT::v128f32;
            Dest0DataType = DAG.getTargetConstant(T_FP32, DL, MVT::i8);
            Income0 = DAG.getUNDEF(DestVT0);
            unsigned newswi0 = newswi | TPCII::SW_TO_FP32;
            SDValue conv0 =
                ConversionEntity(DestVT0, Source, DataType,
                                 DAG.getTargetConstant(newswi0, DL, MVT::i32),
                                 Income0, Predicate, Polarity, DAG, DL);

            EVT DestVT1 = MVT::v128i32;
            unsigned newswi1 = newswi | TPCII::SW_TO_INT32;
            SDValue Income1 = DAG.getUNDEF(DestVT1);
            SDValue conv1 =
                ConversionEntity(DestVT1, conv0, Dest0DataType,
                                 DAG.getTargetConstant(newswi1, DL, MVT::i32),
                                 Income1, Predicate, Polarity, DAG, DL);

            SDValue Dest2DataType = DAG.getTargetConstant(T_INT32, DL, MVT::i8);
            EVT DestVT3 = MVT::v128i16;
            unsigned newswi3 = newswi | TPCII::SW_TO_INT16;
            SDValue Income3 = DAG.getUNDEF(DestVT3);
            SDValue conv3 =
                ConversionEntity(DestVT3, conv1, Dest2DataType,
                                 DAG.getTargetConstant(newswi3, DL, MVT::i32),
                                 Income3, Predicate, Polarity, DAG, DL);

            FinalConv = conv3;
            goto RTExit;
          } else if (DestNumElements == 64) {
            assert(false && "Must avoid it");
            return SDValue();
          }
        }
      } else {
        assert(false && "Bad Vector Length for it");
        return SDValue();
      }
     
      SDValue conv0 =
          ConversionEntity(DestVT0, Source, DataType,
                           DAG.getTargetConstant(newswi, DL, MVT::i32), Income0,
                           Predicate, Polarity, DAG, DL);
      FinalConv =ConversionEntity(DestVT, conv0, Dest0DataType, Switches, Income,
                              Predicate, Polarity, DAG, DL);
      goto RTExit;

    }
    //i16->i32->f32->f16
    if (SrcVT.isInteger() && DestVT.isFloatingPoint()) {
      EVT newDestType = MVT::v64i32;
      unsigned newswi = TPCII::SW_TO_INT32;
      if (SrcIsUnsigned) {
        if (SrcNumElements == 128) {
          newDestType = MVT::v128i32;
        }
        else if (SrcNumElements == 256) {
          newDestType = MVT::v256i32;
        }
        SDValue conv0 = ConversionEntity(
            newDestType, Source, DataType,
            DAG.getTargetConstant(newswi, DL, MVT::i32),
            DAG.getUNDEF(newDestType), Predicate, Polarity, DAG, DL);

        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            Switches, Income, Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      } else {
        if (SrcVT == MVT::v64i32) {
          newDestType = MVT::v64f32;
          newswi = TPCII::SW_TO_FP32;
          SDValue conv0 = ConversionEntity(
              newDestType, Source,
              DataType, 
              DAG.getTargetConstant(newswi, DL, MVT::i32),
              DAG.getUNDEF(newDestType), Predicate, Polarity, DAG, DL);

          FinalConv = ConversionEntity(
              DestVT, conv0, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              Switches, Income, Predicate, Polarity, DAG, DL);
          goto RTExit;
        }
      }
      assert(false && "Implement Me!");
      return SDValue();
    }
  } 
  else if (ConvElCoKind == Split) {
    PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                 Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);

    EVT halfDestVT = getHalfEVT(DestVT);
    EVT halfSrcVT = getHalfEVT(SrcVT);
    SDValue Source0, Source1,Income0,Income1, Shift0, Shift1;
    
    if (SrcMultiplicity == 4) {
      EVT quartSrcVT = getHalfEVT(halfSrcVT);
      SDValue h0 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, quartSrcVT, Source);
      SDValue h1 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, quartSrcVT, Source);
      SDValue h2 =
          helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, quartSrcVT, Source);
      SDValue h3 =
          helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, quartSrcVT, Source);
      if (ShiftForInt) {
        EVT sht = ShiftForInt->getValueType();
        if (sht.isVector()) {
          EVT qsht = getQuarterEVT(sht);
          SDValue sh0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, qsht,
                                                   *ShiftForInt);
          SDValue sh1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, qsht,
                                                   *ShiftForInt);
          SDValue sh2 = helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, qsht,
                                                   *ShiftForInt);
          SDValue sh3 = helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, qsht,
                                                   *ShiftForInt);
          Shift0 = createTuple({sh0, sh1}, DAG);
          Shift1 = createTuple({sh2, sh3}, DAG);
        } else {
          Shift1 = Shift0 = *ShiftForInt;
        }
      }
      Source0 = createTuple({h0, h1}, DAG);
      Source1 = createTuple({h2, h3}, DAG);
    } else if (SrcMultiplicity == 2) {
      Source0 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfSrcVT, Source);
      Source1 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfSrcVT, Source);
      if (ShiftForInt) {
        EVT sht = ShiftForInt->getValueType();
        if (sht.isVector()) {
        EVT hsht = getHalfEVT(sht);
        Shift0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, hsht,
                                            *ShiftForInt);
        Shift1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, hsht,
                                            *ShiftForInt);
        } else {
          Shift1 = Shift0 = *ShiftForInt;
        }
      }

    } else {
      assert(false && "Bad Multiplicity");
      return SDValue();
    }
    EVT quartDestVT = EVT();
    if (DestMultiplicity == 4) {
      quartDestVT = getHalfEVT(halfDestVT);
      if (Income.isUndef()) {
        Income0 = DAG.getUNDEF(halfDestVT);
        Income1 = DAG.getUNDEF(halfDestVT);
      } else {
        SDValue h0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG,
                                                quartDestVT, Income);
        SDValue h1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG,
                                                quartDestVT, Income);
        SDValue h2 = helperExtractSubRegSDValue(TPC::sub_2, DL, DAG,
                                                quartDestVT, Income);
        SDValue h3 = helperExtractSubRegSDValue(TPC::sub_3, DL, DAG,
                                                quartDestVT, Income);
        Income0 = createTuple({h0, h1}, DAG);
        Income1 = createTuple({h2, h3}, DAG);
      }
    } else if (DestMultiplicity == 2) {
      if (Income.isUndef()) {
        Income0 = DAG.getUNDEF(halfDestVT);
        Income1 = DAG.getUNDEF(halfDestVT);
      } else {
        Income0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfDestVT, Source);
        Income1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfDestVT, Source);
      }
    } else {
      assert(false && "Bad Multiplicity");
      return SDValue();
    }
    SDValue predicate0, predicate1;
    predicate0 = predicate1 = Predicate;
    // SDValue polarity = Polarity;
    if (IsVectorPredicate) {
      // need to split predicate
      // may be can done with mov group?
      // or mov bool256 to uint64?
      assert(false && "Vpred is impossible in conversion with double vectors");
    }

   
    SDValue conv0 = ConversionEntity(halfDestVT, Source0, DataType, Switches, Income0,
                         predicate0, Polarity, DAG, DL, IsSwizzle, (ShiftForInt)? &Shift0: nullptr);
    SDValue conv1 = ConversionEntity(halfDestVT, Source1, DataType, Switches, Income1,
                         predicate1, Polarity, DAG, DL, IsSwizzle, (ShiftForInt)?&Shift1 : nullptr);
    SDValue thu;
    if (DestMultiplicity == 2) {
      thu = createTuple({conv0, conv1}, DAG);
    } else if (DestMultiplicity == 4) {
      SDValue h0 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, quartDestVT, conv0);
      SDValue h1 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, quartDestVT, conv0);
      SDValue h2 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, quartDestVT, conv1);
      SDValue h3 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, quartDestVT, conv1);
      thu = createTuple({h0, h1, h2, h3}, DAG);
    }
    FinalConv = thu;
    goto RTExit;
  } 
  else if (ConvElCoKind == IntSoft) {
    PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                 Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
    SDValue ShifForInstr = prepareShift(ShiftForInt, DAG, DL);
    if (SrcMultiplicity == 2 && DestMultiplicity == 1) { // i32->i16 or i16->8
      EVT halfSrcVT = getHalfEVT(SrcVT);
      SDValue Source0 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfSrcVT, Source);
      SDValue Source1 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfSrcVT, Source);
      SDValue sw0 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_0 | TPCII::SW_SINGLE_LANE,
                                          DL, MVT::i32);
      SDValue sw1 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_1 | TPCII::SW_SINGLE_LANE,
                                          DL, MVT::i32);
      SDValue conv0 = ConversionEntity(DestVT, Source0, DataType, sw0, Income,
                                       Predicate, Polarity, DAG, DL, IsSwizzle,
                                       &ShifForInstr, Onlysub_0);
      FinalConv = conv0;
      if (!Onlysub_0) {
        SDValue conv1 = ConversionEntity(DestVT, Source1, DataType, sw1, conv0,
                                         Predicate, Polarity, DAG, DL,
                                         IsSwizzle, &ShifForInstr, Onlysub_0);
        FinalConv = conv1;
      }
      goto RTExit;
    }
    if (SrcMultiplicity == 4 && DestMultiplicity == 1) {
      EVT halfhalfSrcVT = getQuarterEVT(SrcVT);
      SDValue Source0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG,
                                                   halfhalfSrcVT, Source);
      SDValue Source1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG,
                                                   halfhalfSrcVT, Source);
      SDValue Source2 = helperExtractSubRegSDValue(TPC::sub_2, DL, DAG,
                                                   halfhalfSrcVT, Source);
      SDValue Source3 = helperExtractSubRegSDValue(TPC::sub_3, DL, DAG,
                                                   halfhalfSrcVT, Source);
      SDValue sw0 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_0 | TPCII::SW_SINGLE_LANE, DL, MVT::i32);
      SDValue sw1 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_1 | TPCII::SW_SINGLE_LANE, DL, MVT::i32);
      SDValue sw2 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_2 | TPCII::SW_SINGLE_LANE, DL, MVT::i32);
      SDValue sw3 = DAG.getTargetConstant(
          SwitchVal | TPCII::SW_LANE_3 | TPCII::SW_SINGLE_LANE, DL, MVT::i32);

      SDValue conv0 = ConversionEntity(DestVT, Source0, DataType, sw0, Income,
                                       Predicate, Polarity, DAG, DL, IsSwizzle,
                                       &ShifForInstr, Onlysub_0);
      FinalConv = conv0;
      if (!Onlysub_0) {
        SDValue conv1 = ConversionEntity(DestVT, Source1, DataType, sw1, conv0,
                                         Predicate, Polarity, DAG, DL,
                                         IsSwizzle, &ShifForInstr, Onlysub_0);
        SDValue conv2 = ConversionEntity(DestVT, Source2, DataType, sw2, conv1,
                                         Predicate, Polarity, DAG, DL,
                                         IsSwizzle, &ShifForInstr, Onlysub_0);
        SDValue conv3 = ConversionEntity(DestVT, Source3, DataType, sw3, conv2,
                                         Predicate, Polarity, DAG, DL,
                                         IsSwizzle, &ShifForInstr, Onlysub_0);

        FinalConv = conv3;
      }
      goto RTExit;
    }
 } 
  else if (ConvElCoKind == Chain) {
    PreludeForVP(DestVT, Source, DataType, Switches, Income, Predicate,
                 Polarity, DAG, DL, DestIsUnsigned, SrcIsUnsigned);
    unsigned newswi = (SwitchVal & ~TPCII::SW_TO_TYPE);
    if (SrcVT.isFloatingPoint() && DestVT.isInteger()) {
      if (SrcNumElements == 64 && DestNumElements == 64) {
        if (DestIsUnsigned) {
          EVT DestVT1 = MVT::v64i32;
          unsigned swiu = TPCII::SW_TO_INT32;
          unsigned newswi1 = newswi | swiu;
          SDValue Income1 = DAG.getUNDEF(MVT::v64i32);
          SDValue conv1 =
              ConversionEntity(DestVT1, Source, DataType,
                               DAG.getTargetConstant(newswi1, DL, MVT::i32),
                               Income1, Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32), Income, Predicate,
              Polarity, DAG, DL);
          // Now lost big values
          unsigned FMAxInt  = 0x4f000000;
          //unsigned FMAxInt = __builtin_bit_cast(unsigned, 2147483647.0f);
          unsigned FMAxIntP = 0x4f000000;                                  
          //unsigned FMAxIntP =__builtin_bit_cast(unsigned, 2147483648.0f);
          MachineSDNode *ExtraPred = DAG.getMachineNode(
              TPC::CMP_GRTvip, DL, MVT::v256i1,
              {Source, /*Src1*/
               DAG.getTargetConstant(FMAxInt, DL, MVT::i32),
               DAG.getTargetConstant(DataTypeVal, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32),          /*Switch*/
               DAG.getUNDEF(MVT::v256i1),                       /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),              /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)});         /*Polarity*/
          SDValue evp = SDValue(ExtraPred, 0);
          SmallVector<SDValue, 7> SubOperands = {
              Source,
              DAG.getTargetConstant(FMAxIntP, DL, MVT::i32),
              DAG.getTargetConstant(DataTypeVal, DL, MVT::i8), /*Optype*/
              DAG.getTargetConstant(0, DL, MVT::i32),          /*Switch*/
              DAG.getUNDEF(SrcVT),                             /*Income*/
              DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),              /*Predicate*/
              DAG.getTargetConstant(0, DL, MVT::i1)            /*Polarity*/
          };
          MachineSDNode *NodeSub =
              DAG.getMachineNode(TPC::SUBvip, DL, SrcVT, SubOperands);
          SDValue VSub = SDValue(NodeSub, 0);

          SDValue Uconv1 =
              ConversionEntity(DestVT1, VSub, DataType,
                               DAG.getTargetConstant(newswi1, DL, MVT::i32),
                               Income1, Predicate, Polarity, DAG, DL);
          SDValue Uconv2 = ConversionEntity(
              DestVT, Uconv1, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32), Income, Predicate,
              Polarity, DAG, DL);

          SmallVector<SDValue, 7> AddOperands = {
              Uconv2,
              DAG.getTargetConstant(-0x80000000, DL, MVT::i32),
              DAG.getTargetConstant(T_UINT32, DL, MVT::i8),       /*Optype*/
              DAG.getTargetConstant(TPCII::SW_SAT, DL, MVT::i32), /*Switch*/
              conv2,                                              /*Income*/
              evp,                                                /*Predicate*/
              DAG.getTargetConstant(0, DL, MVT::i1)               /*Polarity*/
          };
          MachineSDNode *NodeAdd =
              DAG.getMachineNode(TPC::ADDvim, DL, MVT::v64i32, AddOperands);
          SDValue ASub = SDValue(NodeAdd, 0);
          FinalConv = ASub;
          goto RTExit;
        }
      }
      if ((SrcNumElements == 128 && DestNumElements == 128) ||
          (SrcNumElements == 256 && DestNumElements == 256)) {
        assert(!(DestElBitsize == 32 &&
                 SrcElBitsize == 32) &&
               "must be in SPLIT");
        unsigned newswi0 = newswi | TPCII::SW_TO_FP32;
        SDValue conv0 = Source;
        EVT dvt = (SrcNumElements == 128) ? MVT::v128f32 : MVT::v256f32;
        bool will_need_transpose = false;
        if (SrcElBitsize < 32) {
          conv0 =
              ConversionEntity(dvt, Source, DataType,
                               DAG.getTargetConstant(newswi0, DL, MVT::i32),
                               DAG.getUNDEF(dvt), Predicate, Polarity, DAG, DL);
          will_need_transpose =
              SrcElBitsize == 16 && DestElBitsize == 8;
          
        }
        unsigned sw32, dt32;
        if (DestIsUnsigned && DestElBitsize > 16) {
          sw32 = newswi | TPCII::SW_TO_UINT32;
          dt32 = T_UINT32;
        } else {
          sw32 = newswi | TPCII::SW_TO_INT32;
          dt32 = T_INT32;
        }
        //->128f32->128u32
        dvt = (SrcNumElements == 128) ? MVT::v128i32 : MVT::v256i32;
        SDValue conv2 = ConversionEntity(
            dvt, conv0, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
            DAG.getTargetConstant(sw32, DL, MVT::i32), DAG.getUNDEF(dvt),
            Predicate, Polarity, DAG, DL);
        FinalConv = conv2;
        if (DestElBitsize <= 16) {
          // trunc 128u32 ->128u16 or less
          if (will_need_transpose) {
            conv2 = QVTranspose0213(conv2, DAG, DL);
          }
          SDValue conv16 = ConversionEntity(
              DestVT, conv2, DAG.getTargetConstant(dt32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32),
              DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
          FinalConv = conv16;
        }
        goto RTExit;
      }
      if (SrcMultiplicity == 1) { //partial vectors
        if (SrcCVT == CVT::v64f32 && DestCVT == CVT::v256u8) {
          SDValue conv0 = ConversionEntity(
              MVT::v128i16, Source, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL, MVT::i32),
              DAG.getUNDEF(MVT::v128i16), Predicate, Polarity, DAG, DL);
          SDValue conv1 = ConversionEntity(
              DestVT, conv0, DAG.getTargetConstant(T_INT16, DL, MVT::i8),
              DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT8, DL, MVT::i32),
              DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
          FinalConv = conv1;
          goto RTExit;
        }
        if (SrcCVT == CVT::v64f32 && DestCVT == CVT::v128u16) {
          SDValue conv0 = ConversionEntity(
              MVT::v64i32, Source, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT32, DL, MVT::i32),
              DAG.getUNDEF(MVT::v64i32), Predicate, Polarity, DAG, DL);
          SDValue conv1 = ConversionEntity(
              DestVT, conv0, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
              DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT16, DL, MVT::i32),
              DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
          FinalConv = conv1;
          goto RTExit;
        }
      }
      if (SrcNumElements == 128 && DestNumElements == 64) {
        if (SrcElBitsize == 16 && DestElBitsize == 32) {
          SDValue conv0 = ConversionEntity(
              MVT::v64f32, Source, DataType,
              DAG.getTargetConstant(newswi | TPCII::SW_TO_FP32, DL, MVT::i32),
              DAG.getUNDEF(MVT::v64f32), Predicate, Polarity, DAG, DL);
          SDValue conv1 = ConversionEntity(
              DestVT, conv0, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              Switches,
              DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
          FinalConv = conv1;
          goto RTExit;
          return SDValue();
        }
      }
    }
    if (SrcVT.isInteger() && DestVT.isFloatingPoint()) {
      if (SrcNumElements == 256 &&
          (DestNumElements == 64 || DestNumElements == 128)) {
        SDValue conv0 = ConversionEntity(
            MVT::v128i16, Source, DataType,
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL, MVT::i32),
            DAG.getUNDEF(MVT::MVT::v128i16), Predicate, Polarity, DAG, DL);

        unsigned sw0_val =
            (SwitchVal & ~(TPCII::SW_NUM_LANES_SRCB | TPCII::SW_NUM_LANES));
        SDValue sw =
            DAG.getTargetConstant(sw0_val | TPCII::SW_RHNE, DL, MVT::i32);

        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT16, DL, MVT::i8), sw,
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
       }

      if (SrcNumElements == 64 && DestNumElements == 64) {
        if (SrcIsUnsigned) { // to signed first
          EVT DestVT1 = MVT::v64i32;
          unsigned swiu = TPCII::SW_TO_INT32;
          unsigned newswi1 = newswi | swiu;
          SDValue Income1 = DAG.getUNDEF(MVT::v64i32);
          SDValue conv1 =
              ConversionEntity(DestVT1, Source, DataType,
                               DAG.getTargetConstant(newswi1, DL, MVT::i32),
                               Income1, Predicate, Polarity, DAG, DL);
          SDValue conv2 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32), Income, Predicate,
              Polarity, DAG, DL);

          // Now lost "negative" values
          MachineSDNode *ExtraPred = DAG.getMachineNode(
              TPC::CMP_GRTvip, DL, MVT::v256i1,
              {Source, /*Src1*/
               DAG.getTargetConstant(0x7fffffff, DL, MVT::i32),
               DAG.getTargetConstant(DataTypeVal, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32),          /*Switch*/
               DAG.getUNDEF(MVT::v256i1),                       /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),              /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)});         /*Polarity*/
          SDValue evp = SDValue(ExtraPred, 0);
          SmallVector<SDValue, 7> Operands = {
              Source,
              DAG.getTargetConstant(1, DL, MVT::i32),
              DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                    MVT::i8),         /*Optype*/
              DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
              DAG.getUNDEF(MVT::v64i32),              /*Income*/
              DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),     /*Predicate*/
              DAG.getTargetConstant(0, DL, MVT::i1)   /*Polarity*/
          };
          MachineSDNode *NodeShr_1 =
              DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands);

          SDValue Negconv1 =
              ConversionEntity(DestVT1, SDValue(NodeShr_1, 0), DataType,
                               DAG.getTargetConstant(newswi1, DL, MVT::i32),
                               Income1, evp, Polarity, DAG, DL);
          SDValue Negconv2 = ConversionEntity(
              DestVT, Negconv1, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32), Income, evp,
              Polarity, DAG, DL);
          unsigned R2 = 0x40000000; // __builtin_bit_cast(unsigned, 2.0f);
                                    // stend compiler do not understand
                                    // _builtin_bit_cast
          MachineSDNode *NMul2 = DAG.getMachineNode(
              TPC::MULf32vim, DL, MVT::v64f32,
              {Negconv2, /*Src1*/
               DAG.getTargetConstant(R2, DL, MVT::i32),
               DAG.getTargetConstant(T_FP32, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(SwitchVal, DL, MVT::i32), /*Switch*/
               conv2,                                      /*Income*/
               evp,                                        /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)});    /*Polarity*/
          FinalConv = SDValue(NMul2, 0);
          goto RTExit;
        }
      }
      if (SrcNumElements == 256 && DestNumElements == 256 &&
          SrcElBitsize == 8 && DestElBitsize==16 && DestEltVT == MVT::bf16) {
         //i8->i16 -> fp16
        SDValue conv0 = ConversionEntity(
            MVT::v256i16, Source, DataType,
             DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL,
                                   MVT::i32),
             DAG.getUNDEF(MVT::MVT::v256i16), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT16, DL, MVT::i8),
            DAG.getTargetConstant(SwitchVal | TPCII::SW_RHNE, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if ((SrcNumElements == 128 && DestNumElements == 128) ||
          (SrcNumElements == 256 && DestNumElements == 256)) {
        unsigned newswi0, nextdt;
        SDValue conv0 = Source;
        if (SrcIsUnsigned) {
          newswi0 = newswi | TPCII::SW_TO_UINT32;
          nextdt = T_UINT32;
        } else {
          newswi0 = newswi | TPCII::SW_TO_INT32;
          nextdt = T_INT32;
        }
        EVT dvt = (SrcNumElements == 128) ? MVT::v128i32 : MVT::v256i32;
        if (SrcElBitsize < 32) {
          conv0 = ConversionEntity(
              dvt, Source, DataType,
              DAG.getTargetConstant(newswi0, DL, MVT::i32),
              DAG.getUNDEF(dvt), Predicate, Polarity, DAG, DL);
          nextdt = T_INT32;
        }
        // now we have full size int/uint type
        dvt = (SrcNumElements == 128) ? MVT::v128f32 : MVT::v256f32;
        SDValue conv1 = ConversionEntity(
            dvt, conv0, DAG.getTargetConstant(nextdt, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_FP32, DL, MVT::i32),
            DAG.getUNDEF(dvt), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        // now trunc if needed
        if (DestElBitsize <= 16) {
          // trunc 128f32 ->128(b)f16 or less
          if (SrcEltVT.getSizeInBits() == 8 &&
              DestElBitsize == 16 && DestNumElements == 256) {
            // need 32bit el vector  transpose
            conv1 = QVTranspose0213(conv1, DAG, DL);
          }
          SDValue conv16 = ConversionEntity(
              DestVT, conv1, DAG.getTargetConstant(T_FP32, DL, MVT::i8),
              DAG.getTargetConstant(SwitchVal, DL, MVT::i32),
              DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
          FinalConv = conv16;
          goto RTExit;
        }
      }
      if (SrcNumElements == 128 && DestNumElements == 64) {
        SDValue sw =
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32);
        SDValue conv0 = ConversionEntity(
            MVT::v64i32, Source, DataType,
            sw,
            DAG.getUNDEF(MVT::MVT::v64i32), Predicate, Polarity, DAG, DL);
        unsigned sw1_val =
            (SwitchVal & ~(TPCII::SW_NUM_LANES_SRCB | TPCII::SW_NUM_LANES));
        SDValue sw1 =
            DAG.getTargetConstant(sw1_val, DL, MVT::i32);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT32, DL, MVT::i8), sw1,
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
    }

    if (SrcVT.isInteger() && DestVT.isInteger()) {
      if (DestElBitsize == SrcElBitsize) {
        // one must be unsigned , one signed
        assert(DestElBitsize < 32);
        assert(DestIsUnsigned ^ SrcIsUnsigned);
        unsigned cmpLimit = 0, TDest;

        SDValue Income, evp;
        MachineSDNode *ExtraPred;
        if (DestElBitsize == 16) {
          cmpLimit = 0x7fff;
          TDest = T_INT16;
        } else { // == 8
          cmpLimit = 0x7f;
          TDest = T_INT8;
        }
        ExtraPred = DAG.getMachineNode(
            (SrcIsUnsigned) ? TPC::CMP_GEQvip : TPC::CMP_LESSvip, DL,
            MVT::v256i1,
            {Source, /*Src1*/
             DAG.getTargetConstant(0, DL, MVT::i32),
             DAG.getTargetConstant(TDest, DL, MVT::i8), /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),    /*Switch*/
             DAG.getUNDEF(MVT::v256i1),                 /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),        /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)});   /*Polarity*/
        evp = SDValue(ExtraPred, 0);
        Income =
            SDValue(DAG.getMachineNode(
                        TPC::MOV_ld_vip, DL, DestVT,
                        {DAG.getTargetConstant(cmpLimit, DL, MVT::i32), /*Src*/
                         DAG.getTargetConstant(TDest, DL, MVT::i8), /*Optype*/
                         DAG.getTargetConstant(0, DL, MVT::i32),    /*Switch*/
                         DAG.getUNDEF(DestVT),                      /*Income*/
                         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
                         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
                    0);
        FinalConv = MoveWithPred(Source, DestVT, Income, evp, 0, DAG, DL);
        goto RTExit;
      }
      if (SrcCVT == v256u16 && DestVT == MVT::v256i8) {
        EVT halfVT = MVT::v128i16;
        SDValue V0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfVT, Source);
        SDValue V1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfVT, Source);

        MachineSDNode *ExtraPred0 = DAG.getMachineNode(
            TPC::CMP_LESSvip, DL, MVT::v256i1,
            {V0, DAG.getTargetConstant(0x7f, DL, MVT::i32),
             DAG.getTargetConstant(T_UINT16, DL, MVT::i8), /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),       /*Switch*/
             DAG.getUNDEF(MVT::v256i1),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),           /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)});      /*Polarity*/
        SDValue evp0 = SDValue(ExtraPred0, 0);
        MachineSDNode *ExtraPred1 = DAG.getMachineNode(
            TPC::CMP_LESSvip, DL, MVT::v256i1,
            {V1, DAG.getTargetConstant(0x7f, DL, MVT::i32),
             DAG.getTargetConstant(T_UINT16, DL, MVT::i8), /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),       /*Switch*/
             DAG.getUNDEF(MVT::v256i1),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),           /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)});      /*Polarity*/
        SDValue evp1 = SDValue(ExtraPred1, 0);
        SDValue Income0 =
            SDValue(DAG.getMachineNode(
                        TPC::MOV_ld_vip, DL, DestVT,
                        {DAG.getTargetConstant(0x7f, DL, MVT::i32),  /*Src*/
                         DAG.getTargetConstant(T_INT8, DL, MVT::i8), /*Optype*/
                         DAG.getTargetConstant(0, DL, MVT::i32),     /*Switch*/
                         DAG.getUNDEF(halfVT),                       /*Income*/
                         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
                         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
                    0);
        SDValue Mdf0 = MoveWithPred(V0, halfVT, Income0, evp0, 0, DAG, DL);
        SDValue Mdf1 = MoveWithPred(V1, halfVT, Income0, evp1, 0, DAG, DL);
        SDValue Tupel = createTuple({Mdf0, Mdf1}, DAG);
        SDValue conv0 = ConversionEntity(
            DestVT, Tupel, DAG.getTargetConstant(T_INT16, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT8, DL, MVT::i32),
            Income, Predicate, Polarity, DAG, DL);
        FinalConv = conv0;
        goto RTExit;
      }
      if (SrcCVT == v256u32 && DestCVT == v256i8) {
        SDValue conv0 = ConversionEntity(
            MVT::v256i32, Source, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v256i32), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT8, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v256u8 && DestCVT == v64u32) {
        SDValue conv0 = ConversionEntity(
            MVT::v64i32, Source, DAG.getTargetConstant(T_UINT8, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v64i32), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            Switches,
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v256u8 && (DestCVT == v128u16 || DestCVT == v128i16)) {
        if (newswi & (TPCII::SW_NUM_LANES | TPCII::SW_NUM_LANES_SRCB)) {
          if ((newswi & TPCII::SW_LANE_SEL) == 0) {
            SDValue Source1 = DAG.getNode(ISD::BITCAST, DL, DestVT, Source);
            FinalConv = Source1;
          }
          else if ((newswi & TPCII::SW_LANE_SEL) == 1) {
            SDValue SourceI64 =
                DAG.getNode(ISD::BITCAST, DL, MVT::v64i32, Source);
            SmallVector<SDValue, 7> Operands = {
                SourceI64,
                DAG.getTargetConstant(SrcElBitsize, DL, MVT::i32),
                DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                      MVT::i8),           /*Optype*/
                DAG.getTargetConstant(0, DL, MVT::i32),   /*Switch*/
                DAG.getUNDEF(MVT::v64i32),                /*Income*/
                DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
                DAG.getTargetConstant(0, DL, MVT::i1)     /*Polarity*/
            };
            MachineSDNode *NodeSourceI64_1 =
                DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands);
            SDValue SourceI64_1 = SDValue(NodeSourceI64_1, 0);
            SDValue Source1 = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceI64_1);
            FinalConv = Source1;
          }
          else {
            assert(false && "Not proper lane for 256u8=>128u16");
            return SDValue();
          }
          unsigned axe8 = 0xff;
          MachineSDNode *Cut = DAG.getMachineNode(
              TPC::ANDvip, DL, DestVT,
              {FinalConv, /*Src1*/
               DAG.getTargetConstant(axe8, DL, MVT::i32),
               DAG.getTargetConstant(T_UINT16, DL, MVT::i8),   /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32),   /*Switch*/
               DAG.getUNDEF(DestVT),                     /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)});  /*Polarity*/
              FinalConv = SDValue(Cut, 0);
              goto RTExit;
        }
      }

      if (SrcCVT == v64u32 && DestCVT == v256i8) {
        SDValue conv0 = ConversionEntity(
            MVT::v64i32, Source, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v64i32), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT8, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }

      if (SrcCVT == v256i32 && DestCVT == v256u8) {
        SDValue conv0 = ConversionEntity(
            MVT::v256i32, Source, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v256i32), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT8, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v128u32 && DestCVT == v128i16) {
        SDValue conv0 = ConversionEntity(
            MVT::v128i16, Source, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT16, DL, MVT::i32),
            DAG.getUNDEF(MVT::v128i16), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_UINT16, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v64u32 && DestCVT == v128i16) {
        SDValue conv0 = ConversionEntity(
            MVT::v128i16, Source, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT16, DL, MVT::i32),
            DAG.getUNDEF(MVT::v128i16), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_UINT16, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v128i32 && DestCVT == v128u16) {
        unsigned sw0_val =
            (newswi & ~(TPCII::SW_NUM_LANES_SRCB | TPCII::SW_NUM_LANES));
        SDValue conv0 = ConversionEntity(
            MVT::v128i32, Source, DAG.getTargetConstant(T_INT32, DL, MVT::i8),
            DAG.getTargetConstant(sw0_val | TPCII::SW_TO_UINT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v128i32), Predicate, Polarity, DAG, DL);
        SDValue conv1 = ConversionEntity(
            DestVT, conv0, DAG.getTargetConstant(T_UINT32, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_UINT16, DL, MVT::i32),
            DAG.getUNDEF(DestVT), Predicate, Polarity, DAG, DL);
        FinalConv = conv1;
        goto RTExit;
      }
      if (SrcCVT == v128u16 && (DestCVT == v64u32 || DestCVT == v64i32)) {
        if (newswi & (TPCII::SW_NUM_LANES | TPCII::SW_NUM_LANES_SRCB)) {
          if ((newswi & TPCII::SW_LANE_SEL) == 0) {
            SDValue Source1 = DAG.getNode(ISD::BITCAST, DL, DestVT, Source);
            FinalConv = Source1;
          } else if ((newswi & TPCII::SW_LANE_SEL) == 1) {
            SDValue SourceI64 =
                DAG.getNode(ISD::BITCAST, DL, MVT::v64i32, Source);
            SmallVector<SDValue, 7> Operands = {
                SourceI64,
                DAG.getTargetConstant(SrcElBitsize, DL, MVT::i32),
                DAG.getTargetConstant(TPCII::OpType::UINT32, DL,
                                      MVT::i8),           /*Optype*/
                DAG.getTargetConstant(0, DL, MVT::i32),   /*Switch*/
                DAG.getTargetConstant(0, DL, MVT::i32),   /*Switch*/
                DAG.getUNDEF(MVT::v64i32),                /*Income*/
                DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
                DAG.getTargetConstant(0, DL, MVT::i1)     /*Polarity*/
            };
            MachineSDNode *NodeSourceI64_1 =
                DAG.getMachineNode(TPC::SHRvip, DL, MVT::v64i32, Operands);
            SDValue SourceI64_1 = SDValue(NodeSourceI64_1, 0);
            SDValue Source1 = DAG.getNode(ISD::BITCAST, DL, SrcVT, SourceI64_1);
            FinalConv = Source1;
          } else {
            assert(false && "Not proper lane for 128u16=>64(u)i32");
            return SDValue();
          }
          unsigned axe16 = 0xffff;
          MachineSDNode *Cut = DAG.getMachineNode(
              TPC::ANDvip, DL, DestVT,
              {FinalConv, /*Src1*/
               DAG.getTargetConstant(axe16, DL, MVT::i32),
               DAG.getTargetConstant(T_UINT32, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32),       /*Switch*/
               DAG.getUNDEF(DestVT),                         /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),     /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)});      /*Polarity*/
          FinalConv = SDValue(Cut, 0);
          goto RTExit;
        }
      } // if (SrcCVT == v128u16 && DestCVT == v64u32) {
      if (SrcCVT == v128u16 && (DestCVT == v128u32 || DestCVT == v128i32)) {
        // 128i16->128i32
        SDValue conv0 = ConversionEntity(
            MVT::v128i32, Source, DAG.getTargetConstant(T_INT16, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v128i32), Predicate, Polarity, DAG, DL);
        // cut now
        EVT halfVT = MVT::v64i32;
        SDValue V0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfVT, conv0);
        SDValue V1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfVT, conv0);
        unsigned axe16 = 0xffff;
        unsigned tt = DestCVT == v128u32 ? T_UINT32 : T_INT32;
        MachineSDNode *Cut0 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfVT,
            {V0, /*Src1*/
             DAG.getTargetConstant(axe16, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        MachineSDNode *Cut1 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfVT,
            {V1, /*Src1*/
             DAG.getTargetConstant(axe16, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/

        FinalConv = createTuple({SDValue(Cut0, 0), SDValue(Cut1, 0)}, DAG);
        goto RTExit;
      }

      if (SrcCVT == v256u8 && (DestCVT == v256u16 || DestCVT == v256i16)) {
        SDValue conv0 = ConversionEntity(
            MVT::v256i16, Source, DAG.getTargetConstant(T_INT8, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT16, DL, MVT::i32),
            DAG.getUNDEF(MVT::v256i16), Predicate, Polarity, DAG, DL);
        // cut now
        EVT halfVT = MVT::v128i16;
        SDValue V0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfVT, conv0);
        SDValue V1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfVT, conv0);
        unsigned axe8 = 0xff;
        unsigned tt = DestCVT == v256u16 ? T_UINT16 : T_INT16;
        MachineSDNode *Cut0 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfVT,
            {V0, /*Src1*/
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        MachineSDNode *Cut1 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfVT,
            {V1, /*Src1*/
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        FinalConv = createTuple({SDValue(Cut0, 0), SDValue(Cut1, 0)}, DAG);
        goto RTExit;
      }
      if (SrcCVT == v256u8 && (DestCVT == v256u32 || DestCVT == v256i32)) {
        SDValue conv0 = ConversionEntity(
            MVT::v256i32, Source, DAG.getTargetConstant(T_INT8, DL, MVT::i8),
            DAG.getTargetConstant(newswi | TPCII::SW_TO_INT32, DL, MVT::i32),
            DAG.getUNDEF(MVT::v256i32), Predicate, Polarity, DAG, DL);
        // cut now
        EVT halfhalfVT = MVT::v64i32;
        SDValue V0 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, halfhalfVT, conv0);
        SDValue V1 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, halfhalfVT, conv0);
        SDValue V2 =
            helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, halfhalfVT, conv0);
        SDValue V3 =
            helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, halfhalfVT, conv0);
        unsigned axe8 = 0xff;
        unsigned tt = DestCVT == v256u32 ? T_UINT32 : T_INT32;
        MachineSDNode *Cut0 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfhalfVT,
            {V0,
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfhalfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        MachineSDNode *Cut1 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfhalfVT,
            {V1,
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfhalfVT),                    /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        MachineSDNode *Cut2 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfhalfVT,
            {V2,
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfhalfVT),                /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        MachineSDNode *Cut3 = DAG.getMachineNode(
            TPC::ANDvip, DL, halfhalfVT,
            {V3, 
             DAG.getTargetConstant(axe8, DL, MVT::i32),
             DAG.getTargetConstant(tt, DL, MVT::i8),  /*Optype*/
             DAG.getTargetConstant(0, DL, MVT::i32),  /*Switch*/
             DAG.getUNDEF(halfhalfVT),                /*Income*/
             DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),      /*Predicate*/
             DAG.getTargetConstant(0, DL, MVT::i1)}); /*Polarity*/
        FinalConv = createTuple({SDValue(Cut0, 0), SDValue(Cut1, 0),
                                 SDValue(Cut2, 0), SDValue(Cut3, 0)},
                                DAG);
        goto RTExit;
      }
    }
  }
  
  RTExit:
     if (FinalConv != SDValue()) {
      if (IsVectorPredicate) {
         if (ConvElCoKind == Split) {
           // No rule how apply 256bit predicate,
           // So in that case Vpred will be ignored
         } else if (DestMultiplicity > 1) {
           // MoveWithPred done for Source at start
         } else {
           FinalConv = MoveWithPred(FinalConv, DestVT, SaveIncome,
                                    SavePredicate, SavePolarityVal, DAG, DL);
         }
      }
      return FinalConv;
    }
  assert(false && "Conversion is not considered");
  return SDValue();
}

SDValue TPCTargetLowering::lowerTPC_CONVERT(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT        = Op.getValueType();
  SDValue Source = Op.getOperand(1);
  SDValue DataType  = Op.getOperand(2);
  SDValue Switches  = Op.getOperand(3);
  SDValue Income    = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity  = Op.getOperand(6);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL);
}
SDValue TPCTargetLowering::lowerTPC_CONVERT_LINEAR(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT = Op.getValueType();
  SDValue Source = Op.getOperand(1);
  SDValue DataType = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL, false);
}

SDValue TPCTargetLowering::lowerTPC_CONVERT_INT(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT = Op.getValueType();
  if (!DestVT.isVector())
    return SDValue();
  SDValue Source = Op.getOperand(1);
  SDValue ShiftForInt = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  EVT SrcVT = Source.getValueType();
  EVT SrcEltVT = SrcVT.getVectorElementType();

  SDValue DataType =
      DAG.getTargetConstant(getInstrDataType(SrcEltVT, 0), DL, MVT::i8);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL, true, &ShiftForInt);
}

SDValue TPCTargetLowering::lowerTPC_CONVERT_INT_LINEAR(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT = Op.getValueType();
  if (!DestVT.isVector())
    return SDValue();
  SDValue Source = Op.getOperand(1);
  SDValue ShiftForInt = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  EVT SrcVT = Source.getValueType();
  EVT SrcEltVT = SrcVT.getVectorElementType();

  SDValue DataType =
      DAG.getTargetConstant(getInstrDataType(SrcEltVT, 0), DL, MVT::i8);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL, false, &ShiftForInt);
}


SDValue TPCTargetLowering::lowerTPC_CONVERT_UINT(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT = Op.getValueType();
  if (!DestVT.isVector())
    return SDValue();
  SDValue Source = Op.getOperand(1);
  SDValue ShiftForInt = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  EVT SrcVT = Source.getValueType();
  EVT SrcEltVT = SrcVT.getVectorElementType();
  EVT DestEltVT = DestVT.getVectorElementType();
 
  SDValue DataType =
      DAG.getTargetConstant(getInstrDataType(SrcEltVT, true), DL, MVT::i8);
  unsigned SwitchVal = cast<ConstantSDNode>(Switches)->getZExtValue();
  SwitchVal |= getSwitchByDestDataType(DestEltVT, true);
  Switches = DAG.getTargetConstant(SwitchVal, DL, MVT::i32);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL, true, &ShiftForInt);
}

SDValue TPCTargetLowering::lowerTPC_CONVERT_UINT_LINEAR(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  EVT DestVT = Op.getValueType();
  if (!DestVT.isVector())
    return SDValue();
  SDValue Source = Op.getOperand(1);
  SDValue ShiftForInt = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  EVT SrcVT = Source.getValueType();
  EVT SrcEltVT = SrcVT.getVectorElementType();
  EVT DestEltVT = DestVT.getVectorElementType();

  SDValue DataType =
      DAG.getTargetConstant(getInstrDataType(SrcEltVT, true), DL, MVT::i8);
  unsigned SwitchVal = cast<ConstantSDNode>(Switches)->getZExtValue();
  SwitchVal |= getSwitchByDestDataType(DestEltVT, true);
  Switches = DAG.getTargetConstant(SwitchVal, DL, MVT::i32);
  return ConversionEntity(DestVT, Source, DataType, Switches, Income, Predicate,
                          Polarity, DAG, DL, false, &ShiftForInt);
}

SDValue TPCTargetLowering::lowerTPC_MovDualGroup(SDValue Op, bool ModeAll, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  // Get and check arguments.
  SDValue Source = Op.getOperand(1);
  SDValue ByteEn = Op.getOperand(2);
  SDValue Ctrl = Op.getOperand(3);
  bool isCtrlReg = !isa<ConstantSDNode>(Ctrl);
  if (isCtrlReg && !Subtarget->hasDoron1()) {
    report_fatal_error("Control value of MOV_DUAL_GROUP has to be immediate for architectures prior to Doron1");
  }
  SDValue Switches = Op.getOperand(4);
  SDValue Income = Op.getOperand(5);
  SDValue Predicate = Op.getOperand(6);
  SDValue Polarity = Op.getOperand(7);

  static const unsigned OpMap[2/*isCtrlReg*/][2/*ModeAll*/][2/*isVp*/] = {
    {
      {TPC::MOV_DUAL_GROUPm,TPC::MOV_DUAL_GROUPp},
      {TPC::MOV_DUAL_GROUP_ALLm,TPC::MOV_DUAL_GROUP_ALLp}
    }, 
    {
      {TPC::MOV_DUAL_GROUP_CTRL_REGm,TPC::MOV_DUAL_GROUP_CTRL_REGp},
      {TPC::MOV_DUAL_GROUP_CTRL_REG_ALLm,TPC::MOV_DUAL_GROUP_CTRL_REG_ALLp}
     }
  };
  unsigned OpCode = OpMap[isCtrlReg][ModeAll][Predicate.getValueType() == MVT::i1];
  unsigned SwitchVal = cast<ConstantSDNode>(Switches)->getZExtValue();
  if (ModeAll) SwitchVal |= TPCII::SW_MDG_TYPE_ALL;
  if (isCtrlReg) SwitchVal |= TPCII::SW_MDG_CTRL_REG;
  if (!isCtrlReg) SwitchVal |= cast<ConstantSDNode>(Ctrl)->getZExtValue() << 8;
  bool PolarityVal = cast<ConstantSDNode>(Polarity)->getSExtValue() != 0;

  SmallVector<SDValue, 8> Operands;
  Operands.push_back(Source);
  if (isCtrlReg) Operands.push_back(Ctrl);
  Operands.push_back(DAG.getTargetConstant(cast<ConstantSDNode>(ByteEn)->getZExtValue(), DL, MVT::i32));
  Operands.push_back(DAG.getTargetConstant(SwitchVal, DL, MVT::i32));
  if (ModeAll) Operands.push_back(DAG.getTargetConstant(0, DL, MVT::i8));
  Operands.push_back(Income);
  Operands.push_back(Predicate);
  Operands.push_back(DAG.getTargetConstant(PolarityVal, DL, MVT::i1));
  MachineSDNode *Node = DAG.getMachineNode(OpCode, DL, Op.getValueType(), Operands);
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::truncate_32_to_16_goya(SelectionDAG &DAG,
                                                  SDValue Src0, SDValue Src1,
                                                  const SDLoc &DL, EVT ResultVT,
                                                  uint64_t dataType) const {
  // y = v_i16_pack_v(x0, y, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops11[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);           // Predicate.
  Ops11[5] = DAG.getTargetConstant(0, DL, MVT::i1);        // Polarity.
  MachineSDNode *Node11 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops11);
  SDValue val11 = SDValue(Node11, 0);

  // y = v_i16_pack_v(x0, y, e_group_1, e_every_second_element);
  SmallVector<SDValue, 8> Ops12(6);
  Ops12[0] = Src0;                                         // Source.
  Ops12[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops12[2] =
      DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
  Ops12[3] = val11;                                                // Income.
  Ops12[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                   // Predicate.
  Ops12[5] = DAG.getTargetConstant(0, DL, MVT::i1);                // Polarity.
  MachineSDNode *Node12 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops12);
  SDValue val12 = SDValue(Node12, 0);

  // y = v_i16_mov_dual_group_v(y, 0xFFFFFFFF, y, 1, 0, 0, 1);
  SmallVector<SDValue, 8> Ops13(6);
  Ops13[0] = val12;                                         // Source.
  Ops13[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops13[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops13[3] = val12;                                 // Income.
  Ops13[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops13[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.

  MachineSDNode *Node13 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops13);
  SDValue val13 = SDValue(Node13, 0);

  // y = v_i16_mov_dual_group_v(y, 0xFFFFFFFF, y, 2, 1, 1, 0);
  SmallVector<SDValue, 8> Ops14(6);
  Ops14[0] = val13;                                         // Source.
  Ops14[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops14[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops14[3] = val13;                                 // Income.
  Ops14[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops14[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node14 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops14);
  SDValue val14 = SDValue(Node14, 0);

  // y = v_i16_mov_dual_group_v(y, 0xFFFFFFFF, y, 3, 1, 0, 1);
  SmallVector<SDValue, 8> Ops15(6);
  Ops15[0] = val14;                                         // Source.
  Ops15[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops15[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops15[3] = val14;                                 // Income.
  Ops15[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops15[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node15 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops15);
  SDValue val15 = SDValue(Node15, 0);

  // t = v_i16_pack_v(x1, t, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops21(6);
  Ops21[0] = Src1;                                         // Source.
  Ops21[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops21[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops21[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops21[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);           // Predicate.
  Ops21[5] = DAG.getTargetConstant(0, DL, MVT::i1);        // Polarity.
  MachineSDNode *Node21 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops21);
  SDValue val21 = SDValue(Node21, 0);

  // t = v_i16_pack_v(x1, t, e_group_1, e_every_second_element);
  SmallVector<SDValue, 8> Ops22(6);
  Ops22[0] = Src1;                                         // Source.
  Ops22[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops22[2] =
      DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
  Ops22[3] = val21;                                                // Income.
  Ops22[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                   // Predicate.
  Ops22[5] = DAG.getTargetConstant(0, DL, MVT::i1);                // Polarity.
  MachineSDNode *Node22 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops22);
  SDValue val22 = SDValue(Node22, 0);

  // // 0..15, 16..31, 32..47, 48..63, 64..79
  // y = v_i16_mov_dual_group_v(t, 0xFFFFFFFF, y, 0, 2, 1, 0);
  SmallVector<SDValue, 8> Ops23(6);
  Ops23[0] = val22;                                         // Source.
  Ops23[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops23[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops23[3] = val15;                                 // Income.
  Ops23[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops23[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node23 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops23);
  SDValue val23 = SDValue(Node23, 0);

  // // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95
  // y = v_i16_mov_dual_group_v(t, 0xFFFFFFFF, y, 1, 2, 0, 1);
  SmallVector<SDValue, 8> Ops24(6);
  Ops24[0] = val22;                                         // Source.
  Ops24[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops24[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops24[3] = val23;                                 // Income.
  Ops24[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops24[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node24 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops24);
  SDValue val24 = SDValue(Node24, 0);

  // // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95, 96..111
  // y = v_i16_mov_dual_group_v(t, 0xFFFFFFFF, y, 2, 3, 1, 0);
  SmallVector<SDValue, 8> Ops25(6);
  Ops25[0] = val22;                                         // Source.
  Ops25[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops25[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops25[3] = val24;                                 // Income.
  Ops25[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops25[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node25 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops25);
  SDValue val25 = SDValue(Node25, 0);

  // // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95, 96..111, 112..127
  // y = v_i16_mov_dual_group_v(t, 0xFFFFFFFF, y, 3, 3, 0, 1);
  SmallVector<SDValue, 8> Ops26(6);
  Ops26[0] = val22;                                         // Source.
  Ops26[1] = DAG.getTargetConstant(dataType, DL, MVT::i32); // DataType.
  Ops26[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops26[3] = val25;                                 // Income.
  Ops26[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops26[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node26 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops26);
  SDValue val26 = SDValue(Node26, 0);

  return val26;
}

SDValue TPCTargetLowering::truncate_32_to_16(SelectionDAG &DAG, SDValue Src0,
                                             SDValue Src1, const SDLoc &DL,
                                             EVT ResultVT,
                                             uint64_t dataType) const {
  // y = v_i16_pack_v(x0, y, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops11[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);           // Predicate.
  Ops11[5] = DAG.getTargetConstant(0, DL, MVT::i1);        // Polarity.
  MachineSDNode *Node11 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops11);
  SDValue val11 = SDValue(Node11, 0);

  // y = v_i16_pack_v(x0, y, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops12(6);
  Ops12[0] = Src0;                                         // Source.
  Ops12[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops12[2] =
      DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
  Ops12[3] = val11;                                                // Income.
  Ops12[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                   // Predicate.
  Ops12[5] = DAG.getTargetConstant(0, DL, MVT::i1);                // Polarity.
  MachineSDNode *Node12 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops12);
  SDValue val12 = SDValue(Node12, 0);

  // y = v_i16_mov_dual_group_all_v(y, 0xFFFFFFFF, y, 1, 2, 3, 0, 0b10, 0b01,
  // 0b00, 0b00);
  SmallVector<SDValue, 8> Ops13(7);
  Ops13[0] = val12;                                   // Source.
  Ops13[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  Ops13[2] = DAG.getTargetConstant(
      (((1 << TPCII::SW_SDG0_SHIFT) | (2 << TPCII::SW_SDG1_SHIFT) |
        (3 << TPCII::SW_SDG2_SHIFT) | (0 << TPCII::SW_SDG3_SHIFT)) |
       (2 << TPCII::SW_WEG0_SHIFT) | (1 << TPCII::SW_WEG1_SHIFT) |
       (0 << TPCII::SW_WEG2_SHIFT) | (0 << TPCII::SW_WEG3_SHIFT) |
       (TPCII::SW_MDG_TYPE_ALL)),
      DL, MVT::i32);                                // Switch.
  Ops13[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
  Ops13[4] = val12;                                 // Income.
  Ops13[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops13[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node13 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops13);
  SDValue val13 = SDValue(Node13, 0);

  // y = v_i16_mov_dual_group_v(y, 0xFFFFFFFF, y, 3, 1, 0, 1);
  SmallVector<SDValue, 8> Ops14(6);
  Ops14[0] = val13;                                         // Source.
  Ops14[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  Ops14[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops14[3] = val13;                                 // Income.
  Ops14[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops14[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node14 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops14);
  SDValue val14 = SDValue(Node14, 0);

  // t = v_i16_pack_v(x1, t, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops21(6);
  Ops21[0] = Src1;                                         // Source.
  Ops21[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops21[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops21[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops21[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);           // Predicate.
  Ops21[5] = DAG.getTargetConstant(0, DL, MVT::i1);        // Polarity.
  MachineSDNode *Node21 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops21);
  SDValue val21 = SDValue(Node21, 0);

  // t = v_i16_pack_v(x1, t, e_group_1, e_every_second_element);
  SmallVector<SDValue, 8> Ops22(6);
  Ops22[0] = Src1;                                         // Source.
  Ops22[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops22[2] =
      DAG.getTargetConstant(TPCII::SW_GROUP_SOURCE, DL, MVT::i32); // Switch.
  Ops22[3] = val21;                                                // Income.
  Ops22[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);                   // Predicate.
  Ops22[5] = DAG.getTargetConstant(0, DL, MVT::i1);                // Polarity.
  MachineSDNode *Node22 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops22);
  SDValue val22 = SDValue(Node22, 0);

  // y = v_i16_mov_dual_group_all_v(t, 0xFFFFFFFF, y, 1, 3, 0, 2, 0b00, 0b00,
  // 0b01, 0b01);
  SmallVector<SDValue, 8> Ops23(7);
  Ops23[0] = val22;                                   // Source.
  Ops23[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  Ops23[2] = DAG.getTargetConstant(
      (((1 << TPCII::SW_SDG0_SHIFT) | (3 << TPCII::SW_SDG1_SHIFT) |
        (0 << TPCII::SW_SDG2_SHIFT) | (2 << TPCII::SW_SDG3_SHIFT)) |
       (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
       (1 << TPCII::SW_WEG2_SHIFT) | (1 << TPCII::SW_WEG3_SHIFT) |
       (TPCII::SW_MDG_TYPE_ALL)),
      DL, MVT::i32);                                // Switch.
  Ops23[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
  Ops23[4] = val14;                                 // Income.
  Ops23[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops23[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node23 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops23);
  SDValue val23 = SDValue(Node23, 0);

  // y = v_i16_mov_dual_group_all_v(t, 0xFFFFFFFF, y, 0, 2, 1, 3, 0b00, 0b00,
  // 0b10, 0b10);
  SmallVector<SDValue, 8> Ops24(7);
  Ops24[0] = val22;                                   // Source.
  Ops24[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
  Ops24[2] = DAG.getTargetConstant(
      (((0 << TPCII::SW_SDG0_SHIFT) | (2 << TPCII::SW_SDG1_SHIFT) |
        (1 << TPCII::SW_SDG2_SHIFT) | (3 << TPCII::SW_SDG3_SHIFT)) |
       (0 << TPCII::SW_WEG0_SHIFT) | (0 << TPCII::SW_WEG1_SHIFT) |
       (2 << TPCII::SW_WEG2_SHIFT) | (2 << TPCII::SW_WEG3_SHIFT) |
       (TPCII::SW_MDG_TYPE_ALL)),
      DL, MVT::i32);                                // Switch.
  Ops24[3] = DAG.getTargetConstant(0, DL, MVT::i8); // MovDGAllOp
  Ops24[4] = val23;                                 // Income.
  Ops24[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops24[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node24 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops24);
  SDValue val24 = SDValue(Node24, 0);

  return val24;
}

SDValue TPCTargetLowering::truncate_16_to_8(SelectionDAG &DAG, SDValue Src0,
                                            SDValue Src1, const SDLoc &DL,
                                            EVT ResultVT,
                                            uint64_t dataType) const {
  return truncate_32_to_16_goya(DAG, Src0, Src1, DL, ResultVT, dataType);
}

SDValue TPCTargetLowering::lowerTRUNCATE_i32(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDLoc DL(Op);

  // Make special handling of code like:
  //
  //         t16: i64 = zero_extend t4
  //       t18: i64 = mul t16, Constant : i64<3435973837>
  //     t23 : i64 = srl t18, Constant : i32<34>
  //   t24 : i32 = truncate t23
  //
  // which appears as a result of unsigned integer division by a constant.

  EVT ResTy = Op->getValueType(0);
  if (ResTy != MVT::i32)
    return SDValue();
  EVT ArgTy = Op.getOperand(0).getValueType();
  if (ArgTy != MVT::i64)
    return SDValue();

  SDValue ShiftOp = Op.getOperand(0);
  if (ShiftOp.getOpcode() != ISD::SRL)
    return SDValue();
  SDValue ShiftValOp = ShiftOp.getOperand(1);
  if (ShiftValOp.getOpcode() != ISD::Constant &&
      ShiftValOp.getOpcode() != ISD::TargetConstant)
    return SDValue();
  unsigned ShiftVal = cast<ConstantSDNode>(ShiftValOp)->getZExtValue();
  assert(ShiftVal < 64);

  SDValue MulOp = ShiftOp.getOperand(0);
  if (MulOp.getOpcode() != ISD::MUL)
    return SDValue();
  SDValue MulValOp = MulOp.getOperand(1);
  if (MulValOp.getOpcode() != ISD::Constant &&
      MulValOp.getOpcode() != ISD::TargetConstant)
    return SDValue();
  unsigned MulVal = cast<ConstantSDNode>(MulValOp)->getZExtValue();
  assert(MulVal < 0xFFFFFFFFULL);

  SDValue ExtOp = MulOp.getOperand(0);
  SDValue DividendOp;
  if (ExtOp.getOpcode() == ISD::ZERO_EXTEND) {
    DividendOp = ExtOp.getOperand(0);
    EVT DividendTy = DividendOp.getValueType();
    if (DividendTy != MVT::i32)
      return SDValue();
  } else if (ExtOp.getOpcode() == ISD::LOAD) {
    if (ExtOp.getValueType() == MVT::i64) {
      DividendOp = DAG.getNode(ISD::TRUNCATE, DL, MVT::i32, ExtOp);
    }
  } else
    return SDValue();

  // Instruction creation.
  SmallVector<SDValue, 8> Ops(7);

  // Most significant part of the product.
  Ops[0] = DividendOp;
  Ops[1] = DAG.getTargetConstant(MulVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(TPCII::SW_UPPER32, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  MachineSDNode *MulMSNode = DAG.getMachineNode(TPC::MULsip, DL, MVT::i32, Ops);
  SDValue MulMS(MulMSNode, 0);

  if (ShiftVal == 32)
    return MulMS;

  if (ShiftVal > 32) {
    Ops[0] = MulMS;
    Ops[1] = DAG.getTargetConstant(ShiftVal - 32, DL, MVT::i32);
    Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
    Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
    Ops[4] = DAG.getUNDEF(MVT::i32);
    Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
    Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
    MachineSDNode *Node = DAG.getMachineNode(TPC::SHRsip, DL, MVT::i32, Ops);
    return SDValue(Node, 0);
  }

  // Least significant part of the product.
  SDValue MulLS;
  Ops[0] = DividendOp;
  Ops[1] = DAG.getTargetConstant(MulVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  MachineSDNode *MulLSNode = DAG.getMachineNode(TPC::MULsip, DL, MVT::i32, Ops);
  MulLS = SDValue(MulLSNode, 0);

  // Shift of the most signficant part.
  Ops[0] = MulMS;
  Ops[1] = DAG.getTargetConstant(ShiftVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  SDValue OpShiftMS(DAG.getMachineNode(TPC::SHLsip, DL, MVT::i32, Ops), 0);

  // Shift of the lest signficant part.
  Ops[0] = MulLS;
  Ops[1] = DAG.getTargetConstant(ShiftVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  SDValue OpShiftLS(DAG.getMachineNode(TPC::SHRsip, DL, MVT::i32, Ops), 0);

  // Union of two shifted words.
  Ops[0] = OpShiftMS;
  Ops[1] = OpShiftLS;
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  return SDValue(DAG.getMachineNode(TPC::ORssp, DL, MVT::i32, Ops), 0);
}

SDValue TPCTargetLowering::truncate_32_to_8(SelectionDAG &DAG, SDValue Src0,
                                            SDValue Src1, SDValue Src2,
                                            SDValue Src3, const SDLoc &DL,
                                            EVT ResultVT,
                                            uint64_t dataType) const {
  // y = v_i8_pack_v(x0, y, e_group_0, e_every_forth_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                                  // Income.
  Ops11[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops11[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node11 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops11);
  SDValue val11 = SDValue(Node11, 0);

  // y = v_i8_pack_v(x0, y, e_group_1, e_every_forth_element);
  SmallVector<SDValue, 8> Ops12(6);
  Ops12[0] = Src0;                                         // Source.
  Ops12[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops12[2] = DAG.getTargetConstant(
      (TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_SOURCE), DL, MVT::i32); // Switch.
  Ops12[3] = val11;                                                 // Income.
  Ops12[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops12[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node12 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops12);
  SDValue val12 = SDValue(Node12, 0);

  // y = v_i8_mov_dual_group_v(y, 0xFFFF0000, y, 1, 0, 1, 0);
  SmallVector<SDValue, 8> Ops13(6);
  Ops13[0] = val12;                                           // Source.
  Ops13[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops13[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops13[3] = val12;                                 // Income.
  Ops13[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops13[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node13 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops13);
  SDValue val13 = SDValue(Node13, 0);

  // y = v_i8_mov_dual_group_v(y, 0x0000FFFF, y, 2, 0, 0, 1);
  SmallVector<SDValue, 8> Ops14(6);
  Ops14[0] = val13;                                           // Source.
  Ops14[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops14[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops14[3] = val13;                                 // Income.
  Ops14[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops14[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node14 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops14);
  SDValue val14 = SDValue(Node14, 0);

  // y = v_i8_mov_dual_group_v(y, 0xFFFF0000, y, 3, 0, 0, 1);
  SmallVector<SDValue, 8> Ops15(6);
  Ops15[0] = val14;                                           // Source.
  Ops15[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops15[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops15[3] = val14;                                 // Income.
  Ops15[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops15[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node15 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops15);
  SDValue val15 = SDValue(Node15, 0);

  // t = v_i8_pack_v(x1, t, e_group_0, e_every_forth_element);
  SmallVector<SDValue, 8> Ops21(6);
  Ops21[0] = Src1;                                         // Source.
  Ops21[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops21[2] = DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
  Ops21[3] = DAG.getUNDEF(ResultVT);                                  // Income.
  Ops21[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops21[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node21 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops21);
  SDValue val21 = SDValue(Node21, 0);

  // t = v_i8_pack_v(x1, t, e_group_1, e_every_forth_element);
  SmallVector<SDValue, 8> Ops22(6);
  Ops22[0] = Src1;                                         // Source.
  Ops22[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops22[2] = DAG.getTargetConstant(
      (TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_SOURCE), DL, MVT::i32); // Switch.
  Ops22[3] = val21;                                                 // Income.
  Ops22[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops22[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node22 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops22);
  SDValue val22 = SDValue(Node22, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 0, 1, 1, 0);
  SmallVector<SDValue, 8> Ops23(6);
  Ops23[0] = val22;                                           // Source.
  Ops23[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops23[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops23[3] = val15;                                 // Income.
  Ops23[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops23[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node23 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops23);
  SDValue val23 = SDValue(Node23, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 1, 1, 1, 0);
  SmallVector<SDValue, 8> Ops24(6);
  Ops24[0] = val22;                                           // Source.
  Ops24[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops24[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops24[3] = val23;                                 // Income.
  Ops24[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops24[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node24 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops24);
  SDValue val24 = SDValue(Node24, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 2, 1, 0, 1);
  SmallVector<SDValue, 8> Ops25(6);
  Ops25[0] = val22;                                           // Source.
  Ops25[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops25[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops25[3] = val24;                                 // Income.
  Ops25[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops25[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node25 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops25);
  SDValue val25 = SDValue(Node25, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 3, 1, 0, 1);
  SmallVector<SDValue, 8> Ops26(6);
  Ops26[0] = val22;                                           // Source.
  Ops26[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops26[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops26[3] = val25;                                 // Income.
  Ops26[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops26[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node26 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops26);
  SDValue val26 = SDValue(Node26, 0);

  // t = v_i8_pack_v(x2, t, e_group_0, e_every_forth_element);
  SmallVector<SDValue, 8> Ops31(6);
  Ops31[0] = Src2;                                         // Source.
  Ops31[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops31[2] = DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
  Ops31[3] = val22;                                                   // Income.
  Ops31[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops31[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node31 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops31);
  SDValue val31 = SDValue(Node31, 0);

  // t = v_i8_pack_v(x2, t, e_group_1, e_every_forth_element);
  SmallVector<SDValue, 8> Ops32(6);
  Ops32[0] = Src2;                                         // Source.
  Ops32[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops32[2] = DAG.getTargetConstant(
      (TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_SOURCE), DL, MVT::i32); // Switch.
  Ops32[3] = val31;                                                 // Income.
  Ops32[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops32[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node32 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops32);
  SDValue val32 = SDValue(Node32, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 0, 2, 1, 0);
  SmallVector<SDValue, 8> Ops33(6);
  Ops33[0] = val32;                                           // Source.
  Ops33[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops33[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops33[3] = val26;                                 // Income.
  Ops33[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops33[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node33 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops33);
  SDValue val33 = SDValue(Node33, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 1, 2, 1, 0);
  SmallVector<SDValue, 8> Ops34(6);
  Ops34[0] = val32;                                           // Source.
  Ops34[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops34[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops34[3] = val33;                                 // Income.
  Ops34[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops34[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node34 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops34);
  SDValue val34 = SDValue(Node34, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 2, 2, 0, 1);
  SmallVector<SDValue, 8> Ops35(6);
  Ops35[0] = val32;                                           // Source.
  Ops35[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops35[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops35[3] = val34;                                 // Income.
  Ops35[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops35[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node35 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops35);
  SDValue val35 = SDValue(Node35, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 3, 2, 0, 1);
  SmallVector<SDValue, 8> Ops36(6);
  Ops36[0] = val32;                                           // Source.
  Ops36[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops36[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops36[3] = val35;                                 // Income.
  Ops36[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops36[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node36 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops36);
  SDValue val36 = SDValue(Node36, 0);

  // t = v_i8_pack_v(x3, t, e_group_0, e_every_forth_element);
  SmallVector<SDValue, 8> Ops41(6);
  Ops41[0] = Src3;                                         // Source.
  Ops41[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops41[2] = DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
  Ops41[3] = val32;                                                   // Income.
  Ops41[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops41[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node41 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops41);
  SDValue val41 = SDValue(Node41, 0);

  // t = v_i8_pack_v(x3, t, e_group_1, e_every_forth_element);
  SmallVector<SDValue, 8> Ops42(6);
  Ops42[0] = Src3;                                         // Source.
  Ops42[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops42[2] = DAG.getTargetConstant(
      (TPCII::SW_STRIDE_4 | TPCII::SW_GROUP_SOURCE), DL, MVT::i32); // Switch.
  Ops42[3] = val41;                                                 // Income.
  Ops42[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops42[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node42 = DAG.getMachineNode(TPC::PACKp, DL, ResultVT, Ops42);
  SDValue val42 = SDValue(Node42, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 0, 3, 1, 0);
  SmallVector<SDValue, 8> Ops43(6);
  Ops43[0] = val42;                                           // Source.
  Ops43[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops43[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops43[3] = val36;                                 // Income.
  Ops43[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops43[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node43 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops43);
  SDValue val43 = SDValue(Node43, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 1, 3, 1, 0);
  SmallVector<SDValue, 8> Ops44(6);
  Ops44[0] = val42;                                           // Source.
  Ops44[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops44[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops44[3] = val43;                                 // Income.
  Ops44[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops44[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node44 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops44);
  SDValue val44 = SDValue(Node44, 0);

  // y = v_i8_mov_dual_group_v(t, 0x0000FFFF, y, 2, 3, 0, 1);
  SmallVector<SDValue, 8> Ops45(6);
  Ops45[0] = val42;                                           // Source.
  Ops45[1] = DAG.getTargetConstant(0x0000FFFF, DL, MVT::i32); // DataType.
  Ops45[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops45[3] = val44;                                 // Income.
  Ops45[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops45[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node45 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops45);
  SDValue val45 = SDValue(Node45, 0);

  // y = v_i8_mov_dual_group_v(t, 0xFFFF0000, y, 3, 3, 0, 1);
  SmallVector<SDValue, 8> Ops46(6);
  Ops46[0] = val42;                                           // Source.
  Ops46[1] = DAG.getTargetConstant(0xFFFF0000, DL, MVT::i32); // DataType.
  Ops46[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops46[3] = val45;                                 // Income.
  Ops46[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
  Ops46[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node46 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops46);
  SDValue val46 = SDValue(Node46, 0);

  return val46;
}



SDValue TPCTargetLowering::lowerTRUNCATE(SDValue Op, SelectionDAG &DAG,
                                         unsigned int InputSwitch,
                                         bool IsCastIntrinsicWithRoundMode) const {
  SDLoc DL(Op);
  EVT OpType = Op->getValueType(0);
  EVT ResultType = Op->getValueType(0);
  EVT ResultVT = Op->getValueType(0);
  const SDValue &Src = Op.getOperand(0);
  EVT SrcType = Src.getValueType();
  if (ResultType.isVector() && SrcType.isVector()) {
    EVT SrcEltVT = SrcType.getVectorElementType();
    EVT DestEltVT = ResultVT.getVectorElementType();
    unsigned SrcMultiplicity = getMultiplicity(SrcType);
    unsigned DestMultiplicity = getMultiplicity(ResultVT);
        // need split
    if ((SrcMultiplicity | DestMultiplicity) == 6) {
      unsigned sourceType=0, targetType=0;
      if (SrcEltVT == MVT::f32) {
        sourceType = T_FP32;
      } else if (SrcEltVT == MVT::bf16) {
        sourceType = T_BF16;
      } else if (SrcEltVT == MVT::f16) {
        sourceType = T_FP16;
      } else if (SrcEltVT == MVT::f8_143) {
        sourceType = T_FP8_143;
      } else if (SrcEltVT == MVT::f8_152) {
        sourceType = T_FP8_152;
      } else if (SrcEltVT == MVT::i32) {
        sourceType = (Src.getOpcode() == ISD::FP_TO_UINT) ? T_UINT32 : T_INT32;
      } else if (SrcEltVT == MVT::i16) {
        sourceType = (Src.getOpcode() == ISD::FP_TO_UINT) ? T_UINT16 : T_INT16;
      } else if (SrcEltVT == MVT::i8) {
        sourceType = (Src.getOpcode() == ISD::FP_TO_UINT) ? T_UINT8 : T_INT8;
      } else
        assert(false && "Unsupported source trunc type");

      if (DestEltVT == MVT::f32)
        targetType = TPCII::SW_TO_FP32;
      else if (DestEltVT == MVT::bf16)
        targetType = TPCII::SW_TO_BF16;
      else if (DestEltVT == MVT::f16)
        targetType = TPCII::SW_TO_FP16;
      else if (DestEltVT == MVT::f8_143)
        targetType = TPCII::SW_TO_FP8_143;
      else if (DestEltVT == MVT::f8_152)
        targetType = TPCII::SW_TO_FP8_152;
      else if (DestEltVT == MVT::i32)
        targetType = (Src.getOpcode() == ISD::FP_TO_UINT) ? TPCII::SW_TO_UINT32
                                                          : TPCII::SW_TO_INT32;
      else if (DestEltVT == MVT::i16)
        targetType = (Src.getOpcode() == ISD::FP_TO_UINT) ? TPCII::SW_TO_UINT16
                                                          : TPCII::SW_TO_INT16;
      else if (DestEltVT == MVT::i8)
        targetType = (Src.getOpcode() == ISD::FP_TO_UINT) ? TPCII::SW_TO_UINT8
                                                          : TPCII::SW_TO_INT8;
      else
        assert(false && "Unsupported target trunc type");

      return ConversionEntity(
          ResultVT, Src, DAG.getTargetConstant(sourceType, DL, MVT::i8),
          DAG.getTargetConstant(targetType, DL, MVT::i32),
          DAG.getUNDEF(ResultVT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
          DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, false);
    }
  }
 
  enum TPCII::OpType sourceType;
  enum TPCII::OpType targetType;
  bool cast32to16 = false, cast16to8 = false, cast32to8 = false;
  if ((OpType == MVT::v128i16) && (SrcType == MVT::v128i32)) {

    ResultType = MVT::v64i32;
    ResultVT = MVT::v128i16;
    cast32to16 = true;
    targetType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT16
                     : TPCII::OpType::INT16;
    sourceType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT32
                     : TPCII::OpType::INT32;
  } else if ((OpType == MVT::v256i8) && (SrcType == MVT::v256i16)) {
    ResultType = MVT::v128i16;
    ResultVT = MVT::v256i8;
    cast16to8 = true;
    targetType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT8
                     : TPCII::OpType::INT8;
    sourceType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT16
                     : TPCII::OpType::INT16;
  } else if ((OpType == MVT::v256i8) && (SrcType == MVT::v256i32)) {
    ResultType = MVT::v64i32;
    ResultVT = MVT::v256i8;
    cast32to8 = true;
    targetType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT8
                     : TPCII::OpType::INT8;
    sourceType = (Src.getOpcode() == ISD::FP_TO_UINT)
                     ? TPCII::OpType::UINT32
                     : TPCII::OpType::INT32;
  } else if (OpType == MVT::i32) {
    return lowerTRUNCATE_i32(Op, DAG);
  }

  if (cast32to16 || cast16to8) {
    SDValue NIndex0 = DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32);
    SDNode *N0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex0);
    SDValue N0Val = SDValue(N0, 0);

    SDValue NIndex1 = DAG.getTargetConstant(TPC::sub_1, DL, MVT::i32);
    SDNode *N1 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex1);
    SDValue N1Val = SDValue(N1, 0);

    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = N0Val;                                          // Source.
    Ops1[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops1[2] = (cast32to16)
                  ? (Src.getOpcode() == ISD::FP_TO_UINT
                         ? DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_UINT16,
                               DL, MVT::i32)
                         : DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_INT16,
                               DL, MVT::i32))
                  : (Src.getOpcode() == ISD::FP_TO_UINT
                         ? DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_UINT8,
                               DL, MVT::i32)
                         : DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_INT8,
                               DL, MVT::i32));
    Ops1[3] = DAG.getUNDEF(ResultType);              // Income.
    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.

    MachineSDNode *Node1 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops1);
    SDValue Src0 = SDValue(Node1, 0);

    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = N1Val;                                          // Source.
    Ops2[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops2[2] = (cast32to16)
                  ? (Src.getOpcode() == ISD::FP_TO_UINT
                         ? DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_UINT16,
                               DL, MVT::i32)
                         : DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_INT16,
                               DL, MVT::i32))
                  : (Src.getOpcode() == ISD::FP_TO_UINT
                         ? DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_UINT8,
                               DL, MVT::i32)
                         : DAG.getTargetConstant(
                               (IsCastIntrinsicWithRoundMode ? InputSwitch : 0) |
                                   TPCII::SW_TO_INT8,
                               DL, MVT::i32));
    Ops2[3] = DAG.getUNDEF(ResultType);              // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.

    MachineSDNode *Node2 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops2);
    SDValue Src1 = SDValue(Node2, 0);
    if (cast32to16) {
      return (Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA())
                 ? truncate_32_to_16(DAG, Src0, Src1, DL, ResultVT, targetType)
                 : truncate_32_to_16_goya(DAG, Src0, Src1, DL, ResultVT,
                                          targetType);
    } else {
      return truncate_16_to_8(DAG, Src0, Src1, DL, ResultVT, targetType);
    }
  } else if (cast32to8) { // SrcType --> v256i32 v64i32
    SDValue NIndex0 = DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32);
    SDNode *N0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex0);
    SDValue N0Val = SDValue(N0, 0);

    SDValue NIndex1 = DAG.getTargetConstant(TPC::sub_1, DL, MVT::i32);
    SDNode *N1 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex1);
    SDValue N1Val = SDValue(N1, 0);

    SDValue NIndex2 = DAG.getTargetConstant(TPC::sub_2, DL, MVT::i32);
    SDNode *N2 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex2);
    SDValue N2Val = SDValue(N2, 0);

    SDValue NIndex3 = DAG.getTargetConstant(TPC::sub_3, DL, MVT::i32);
    SDNode *N3 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                    ResultType, Src, NIndex3);
    SDValue N3Val = SDValue(N3, 0);

    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = N0Val;                                          // Source.
    Ops1[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops1[2] =
        (Src.getOpcode() == ISD::FP_TO_UINT
             ? // Switch.
             DAG.getTargetConstant(
                 (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                     TPCII::SW_LANE_0 | TPCII::SW_TO_UINT8,
                 DL, MVT::i32)
             : DAG.getTargetConstant(
                   (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                       TPCII::SW_LANE_0 | TPCII::SW_TO_INT8,
                   DL, MVT::i32));
    Ops1[3] = DAG.getUNDEF(ResultType);              // Income.


    Ops1[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops1[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node1 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops1);
    SDValue Src0 = SDValue(Node1, 0);

    SmallVector<SDValue, 8> Ops2(6);
    Ops2[0] = N1Val;                                          // Source.
    Ops2[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops2[2] =
        (Src.getOpcode() == ISD::FP_TO_UINT
             ? // Switch.
             DAG.getTargetConstant(
                 (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                     TPCII::SW_LANE_0 | TPCII::SW_TO_UINT8,
                 DL, MVT::i32)
             : DAG.getTargetConstant(
                   (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                       TPCII::SW_LANE_0 | TPCII::SW_TO_INT8,
                   DL, MVT::i32));
    Ops2[3] = DAG.getUNDEF(ResultType);              // Income.
    Ops2[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node2 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops2);
    SDValue Src1 = SDValue(Node2, 0);

    SmallVector<SDValue, 8> Ops3(6);
    Ops3[0] = N2Val;                                          // Source.
    Ops3[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops3[2] =
        (Src.getOpcode() == ISD::FP_TO_UINT
             ? // Switch.
             DAG.getTargetConstant(
                 (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                     TPCII::SW_LANE_0 | TPCII::SW_TO_UINT8,
                 DL, MVT::i32)
             : DAG.getTargetConstant(
                   (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                       TPCII::SW_LANE_0 | TPCII::SW_TO_INT8,
                   DL, MVT::i32));
    Ops3[3] = DAG.getUNDEF(ResultType);              // Income.
    Ops3[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops3[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node3 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops3);
    SDValue Src2 = SDValue(Node3, 0);

    SmallVector<SDValue, 8> Ops4(6);
    Ops4[0] = N3Val;                                          // Source.
    Ops4[1] = DAG.getTargetConstant(sourceType, DL, MVT::i8); // DataType.
    Ops4[2] =
        (Src.getOpcode() == ISD::FP_TO_UINT
             ? // Switch.
             DAG.getTargetConstant(
                 (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                     TPCII::SW_LANE_0 | TPCII::SW_TO_UINT8,
                 DL, MVT::i32)
             : DAG.getTargetConstant(
                   (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
                       TPCII::SW_LANE_0 | TPCII::SW_TO_INT8,
                   DL, MVT::i32));
    Ops4[3] = DAG.getUNDEF(ResultType);              // Income.
    Ops4[4] = DAG.getRegister(TPC::SPRF_TRUE, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops4);
    SDValue Src3 = SDValue(Node4, 0);

    return truncate_32_to_8(DAG, Src0, Src1, Src2, Src3, DL, ResultVT,
                            targetType);
  } else {
    assert(false && "Unsupported source and target type");
  }

  return SDValue();
}
SDValue TPCTargetLowering::lowerCONVERTSIGNED(SDValue Op,
                                              SelectionDAG &DAG) const {

  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_RHNE;
  // If Op is intrinsic, get the switch from it.
  auto IsCastIntrinsicWithRoundMode = NumOp == 3;
  if (IsCastIntrinsicWithRoundMode) {
    InputSwitch = Op.getConstantOperandVal(NumOp - 1);
    LLVM_DEBUG(dbgs() << "lowerCONVERTSIGNED: Get switch from argument: "
                      << InputSwitch << "\n");
  }
  EVT OpType =
      (IsCastIntrinsicWithRoundMode) ? Op.getValueType() : Op->getValueType(0);
  const SDValue &Src =
      (IsCastIntrinsicWithRoundMode) ? Op.getOperand(1) : Op.getOperand(0);

  EVT SrcType = Src.getValueType();
  // Let's try via any-any
  EVT SrcEltVT = SrcType.getVectorElementType();
  EVT DestEltVT = OpType.getVectorElementType();

  unsigned DDT = getInstrDataType(SrcEltVT, false);
  unsigned SWS = getSwitchByDestDataType(DestEltVT, false);

  return ConversionEntity(
      OpType, Src, DAG.getTargetConstant(DDT, DL, MVT::i8),
      DAG.getTargetConstant(SWS | InputSwitch, DL, MVT::i32),
      DAG.getUNDEF(OpType), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
      DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, false, nullptr);
}

SDValue TPCTargetLowering::lowerCONVERTUNSIGNED(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_RHNE;

  // If Op is intrinsic, get the switch from it.
  auto IsCastIntrinsicWithRoundMode = NumOp == 3;
  if (IsCastIntrinsicWithRoundMode) {
    InputSwitch = Op.getConstantOperandVal(NumOp - 1);
    LLVM_DEBUG(dbgs() << "lowerCONVERTUNSIGNED: Get switch from argument: "
                      << InputSwitch << "\n");
  }
  EVT OpType = IsCastIntrinsicWithRoundMode ? Op.getValueType() : Op->getValueType(0);
  const SDValue &Src =
      IsCastIntrinsicWithRoundMode ? Op.getOperand(1) : Op.getOperand(0);

  EVT SrcType = Src.getValueType();
  // Let's try via any-any
  EVT SrcEltVT = SrcType.getVectorElementType();
  EVT DestEltVT = OpType.getVectorElementType();

  unsigned DDT = getInstrDataType(SrcEltVT, true);
  unsigned SWS = getSwitchByDestDataType(DestEltVT, true);

  return ConversionEntity(
      OpType, Src, DAG.getTargetConstant(DDT, DL, MVT::i8),
      DAG.getTargetConstant(SWS | InputSwitch, DL, MVT::i32),
      DAG.getUNDEF(OpType), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
      DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, false, nullptr);
}

SDValue TPCTargetLowering::lowerEXTRACT_VECTOR_ELT(SDValue Op,
                                                   SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const SDValue &Vector = Op.getOperand(0);
  const SDValue &Index = Op.getOperand(1);

  EVT VT = Vector.getValueType();
  assert(VT.isVector());
  if (VT.getVectorNumElements() != 2)
    return SDValue();

  EVT EltTy = Op.getValueType();
  if (!EltTy.isScalarInteger())
    return SDValue();

  auto IndexVal = dyn_cast<ConstantSDNode>(Index);
  if (!IndexVal)
    return SDValue();
  assert(IndexVal->getZExtValue() < 2);
  const unsigned IndexMap[2] = {TPC::sub_s0, TPC::sub_s1};
  unsigned SubNdx = IndexMap[IndexVal->getZExtValue()];
  SDValue NIndex = DAG.getTargetConstant(SubNdx, DL, MVT::i32);
  SDNode *N = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, EltTy,
                                 Vector, NIndex);
  return SDValue(N, 0);
}

SDValue TPCTargetLowering::lowerINSERT_VECTOR_ELT(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const SDValue &Vector = Op.getOperand(0);
  const SDValue &Item = Op.getOperand(1);
  const SDValue &Index = Op.getOperand(2);

  EVT VT = Vector.getValueType();
  assert(VT.isVector());
  if (VT.getVectorNumElements() != 2)
    return SDValue();
  assert(VT.getVectorElementType().isScalarInteger());

  auto IndexVal = dyn_cast<ConstantSDNode>(Index);
  if (!IndexVal)
    return SDValue();
  assert(IndexVal->getZExtValue() < 2);

  const unsigned IndexMap[2] = {TPC::sub_s0, TPC::sub_s1};
  unsigned SubNdx = IndexMap[IndexVal->getZExtValue()];
  SDValue NIndex = DAG.getTargetConstant(SubNdx, DL, MVT::i32);
  SDNode *N = DAG.getMachineNode(TargetOpcode::INSERT_SUBREG, DL, VT, Vector,
                                 Item, NIndex);
  return SDValue(N, 0);
}

SDValue TPCTargetLowering::lowerAtomicCmpXchg(SDValue Op,
                                              SelectionDAG &DAG) const {
  report_fatal_error("atomic compare exchange is not supported in TPC");
}

static bool isSplatBuildVector(SDValue Op, SDValue &El) {
  if (Op.getOpcode() != ISD::BUILD_VECTOR)
    return false;
  unsigned NumElements = Op.getNumOperands();
  SDValue Element = Op.getOperand(0);
  for (unsigned i = 1; i < NumElements; ++i)
    if (Op.getOperand(i) != Element)
      return false;
  El = Element;
  return true;
}

SDValue TPCTargetLowering::lowerAtomicFence(SDValue Op,
                                            SelectionDAG &DAG) const {
  report_fatal_error("atomic fence is not supported in TPC");
}

SDValue TPCTargetLowering::lowerAtomicRMW(SDValue Op, SelectionDAG &DAG) const {
  report_fatal_error("atomic rmw is not supported in TPC");
}

SDValue TPCTargetLowering::lowerBLOCK_ADDRESS(SDValue Op,
                                              SelectionDAG &DAG) const {
  BlockAddressSDNode *BASDN = cast<BlockAddressSDNode>(Op);
  const BlockAddress *BA = BASDN->getBlockAddress();
  EVT Ty = getPointerTy(DAG.getDataLayout());
  SDValue tba = DAG.getTargetBlockAddress(BA, Ty);
  SDValue Mov = DAG.getNode(TPCISD::COND_MOV, SDLoc(Op), Ty, tba,
                            DAG.getRegister(TPC::SPRF_TRUE, MVT::i1));
  return Mov;
}

SDValue TPCTargetLowering::lowerMUL(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Op1 = Op.getOperand(0);
  SDValue Op2 = Op.getOperand(1);
  EVT VT = Op.getValueType();
  if (!VT.isVector())
    return SDValue();
  EVT EltTy = VT.getVectorElementType();
  if (!EltTy.isScalarInteger())
    return SDValue();

  enum BitSize { Bits32, Bits16, Bits8 } ElementBitSize;
  SDValue DataType;
  EVT ElemTy;
  switch (VT.getVectorNumElements()) {
  case 64:
    assert(EltTy.getSizeInBits() == 32);
    DataType = DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8);
    ElementBitSize = Bits32;
    ElemTy = MVT::i32;
    break;
  case 128:
    assert(EltTy.getSizeInBits() == 16);
    DataType = DAG.getTargetConstant(TPCII::OpType::INT16, DL, MVT::i8);
    ElementBitSize = Bits16;
    ElemTy = MVT::i16;
    break;
  case 256:
    assert(EltTy.getSizeInBits() == 8);
    DataType = DAG.getTargetConstant(TPCII::OpType::INT8, DL, MVT::i8);
    ElementBitSize = Bits8;
    ElemTy = MVT::i8;
    break;
  default:
    assert(false && "Unexpected vector size");
    ElementBitSize = Bits32;
    ElemTy = MVT::i32;
    break;
  }

  enum InstrVariant { VV, VS, VI } Variant = VV;
  if (Op1.getOpcode() == ISD::BUILD_VECTOR)
    std::swap(Op1, Op2);
  SDValue Element;
  if (isSplatBuildVector(Op2, Element)) {
    if (Element.getOpcode() == ISD::Constant) {
      unsigned ArgVal = cast<ConstantSDNode>(Element)->getZExtValue();
      Element = DAG.getTargetConstant(ArgVal, DL, ElemTy);
      Variant = VI;
    } else if (Element.getOpcode() == ISD::TargetConstant) {
      Variant = VI;
    } else {
      Variant = VS;
    }
    Op2 = Element;
  }

  static const unsigned InstrOpCodeTable[/*DT*/ 3][/*VX*/ 3] = {
      {TPC::MULi32vvp, TPC::MULi32vsp, TPC::MULi32vip},
      {TPC::MULi16vvp, TPC::MULi16vsp, TPC::MULi16vip},
      {TPC::MULi8vvp, TPC::MULi8vsp, TPC::MULi8vip}};
  const unsigned InstrOpCode = InstrOpCodeTable[ElementBitSize][Variant];

  static const EVT MulResultTable[3] = {MVT::v128i32, MVT::v256i16,
                                        MVT::v512i8};
  // generate converts to extract o/p of the appropriate size.
  if (VT == MVT::v128i16 && InstrOpCode == TPC::MULi16vvp) {
    EVT ResultVT = MVT::v128i32;
    MachineSDNode *Node = DAG.getMachineNode(
        InstrOpCode, DL, ResultVT,
        {Op1, Op2, DataType, DAG.getTargetConstant(0, DL, MVT::i32),
         DAG.getUNDEF(ResultVT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
         DAG.getTargetConstant(0, DL, MVT::i1)});
    SDValue Mul(Node, 0);
    return ConversionEntity(
        VT , Mul,
        DAG.getTargetConstant(TPCII::INT32, DL, MVT::i8),
        DAG.getTargetConstant(TPCII::SW_RHNE | TPCII::SW_TO_INT16, DL, MVT::i32),
        DAG.getUNDEF(VT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
        DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, true, nullptr);   
  }
  else if (VT == MVT::v256i8 && InstrOpCode == TPC::MULi8vvp) {
    if (Subtarget->hasMulI8()) {
      EVT ResultVT = MVT::v256i32;
      MachineSDNode *Node = DAG.getMachineNode(
          InstrOpCode, DL, ResultVT,
          {Op1, Op2, DataType, DAG.getTargetConstant(0, DL, MVT::i32),
           DAG.getUNDEF(ResultVT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
           DAG.getTargetConstant(0, DL, MVT::i1)});
      SDValue Mul(Node, 0);
      return ConversionEntity(
          VT, Mul, DAG.getTargetConstant(TPCII::INT32, DL, MVT::i8),
          DAG.getTargetConstant(TPCII::SW_RHNE | TPCII::SW_TO_INT8, DL,
                                MVT::i32),
          DAG.getUNDEF(VT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
          DAG.getTargetConstant(0, DL, MVT::i1), DAG, DL, true, nullptr);
    } else {
      assert(false && "Need emulate such MulI8 for this platform");
    }
  } else {
    EVT ResultVT = MulResultTable[ElementBitSize];
    MachineSDNode *Node = DAG.getMachineNode(
        InstrOpCode, DL, ResultVT,
        {Op1, Op2, DataType, DAG.getTargetConstant(0, DL, MVT::i32),
         DAG.getUNDEF(ResultVT), DAG.getRegister(TPC::SPRF_TRUE, MVT::i1),
         DAG.getTargetConstant(0, DL, MVT::i1)});
    EVT SubRegType;

    SDValue MulResult(Node, 0);

    Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                              Op.getValueType(), MulResult,
                              DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32));
    return SDValue(Node, 0);
  }
  return SDValue();
}

SDValue TPCTargetLowering::lowerMULHU(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Op1 = Op.getOperand(0);
  SDValue Op2 = Op.getOperand(1);
  EVT ResTy = Op.getValueType();
  if (ResTy.isVector())
    return SDValue();

  unsigned OpCode;
  if (const auto *Const = dyn_cast<ConstantSDNode>(Op2)) {
    OpCode = TPC::MULsip;
    uint64_t Val = Const->getZExtValue();
    Op2 = DAG.getTargetConstant(Val, DL, ResTy);
  } else {
    OpCode = TPC::MULssp;
  }
  if (ResTy == MVT::i32) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::UINT32, DL, MVT::i8),
        DAG.getTargetConstant(TPCII::SW_UPPER32, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return SDValue(Quotinent, 0);
  }
  if (ResTy == MVT::i16) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::UINT16, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    MachineSDNode *Shifted = DAG.getMachineNode(
      TPC::SHRsip, DL, MVT::i32,
      { SDValue(Quotinent, 0),
        DAG.getTargetConstant(16, DL, MVT::i32),
        DAG.getTargetConstant(TPCII::UINT32, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return DAG.getZExtOrTrunc(SDValue(Shifted, 0), DL, MVT::i16);
  }
  if (ResTy == MVT::i8) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::UINT8, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    MachineSDNode *Shifted = DAG.getMachineNode(
      TPC::SHRsip, DL, MVT::i32,
      { SDValue(Quotinent, 0),
        DAG.getTargetConstant(8, DL, MVT::i32),
        DAG.getTargetConstant(TPCII::UINT32, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return DAG.getZExtOrTrunc(SDValue(Shifted, 0), DL, MVT::i8);
  }

  return SDValue();
}


SDValue TPCTargetLowering::lowerMULHS(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Op1 = Op.getOperand(0);
  SDValue Op2 = Op.getOperand(1);
  EVT ResTy = Op.getValueType();
  if (ResTy.isVector())
    return SDValue();

  unsigned OpCode;
  if (const auto *Const = dyn_cast<ConstantSDNode>(Op2)) {
    OpCode = TPC::MULsip;
    uint64_t Val = Const->getZExtValue();
    Op2 = DAG.getTargetConstant(Val, DL, ResTy);
  } else {
    OpCode = TPC::MULssp;
  }

  if (ResTy == MVT::i32) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::INT32, DL, MVT::i8),
        DAG.getTargetConstant(TPCII::SW_UPPER32, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return SDValue(Quotinent, 0);
  }

  if (ResTy == MVT::i16) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::INT16, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    MachineSDNode *Shifted = DAG.getMachineNode(
      TPC::ASHsip, DL, MVT::i32,
      { SDValue(Quotinent, 0),
        DAG.getTargetConstant(-16, DL, MVT::i32),
        DAG.getTargetConstant(TPCII::UINT32, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return DAG.getSExtOrTrunc(SDValue(Shifted, 0), DL, MVT::i16);
  }

  if (ResTy == MVT::i8) {
    MachineSDNode *Quotinent = DAG.getMachineNode(
      OpCode, DL, MVT::i32,
      { Op1,
        Op2,
        DAG.getTargetConstant(TPCII::INT8, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    MachineSDNode *Shifted = DAG.getMachineNode(
      TPC::ASHsip, DL, MVT::i32,
      { SDValue(Quotinent, 0),
        DAG.getTargetConstant(-8, DL, MVT::i32),
        DAG.getTargetConstant(TPCII::UINT32, DL, MVT::i8),
        DAG.getTargetConstant(0, DL, MVT::i32),
        DAG.getUNDEF(ResTy) /*Income*/,
        DAG.getRegister(TPC::SPRF_TRUE, MVT::i1) /*Predicate*/,
        DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/
      });
    return DAG.getSExtOrTrunc(SDValue(Shifted, 0), DL, MVT::i8);
  }

  return SDValue();
}

static SDValue performBitCastCombine(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const TPCSubtarget *Subtarget) {
  SDLoc DL(N);
  EVT ResultVT = N->getValueType(0);
  SDValue CastedArg = N->getOperand(0);
  EVT CastedVT = CastedArg.getValueType();

  if (CastedArg.getOpcode() == ISD::EXTRACT_VECTOR_ELT &&
      CastedVT.isInteger() && CastedVT.getSizeInBits() == 2048) {
    SDValue VectorArg = CastedArg.getOperand(0);
    uint64_t EltNo = CastedArg.getNode()->getConstantOperandVal(1);

    // Convert pattern like:
    //
    //   t20: v128i32 = ...
    //   t23: v2i2048 = bitcast t20
    //   t24: i2048 = extract_vector_elt t23, Constant : i32<1>
    //   t36: v64f32 = bitcast t24
    //
    // into:
    //
    //   t20: v128i32 = ...
    //   t24: v64i32 = extract_subvector t23, Constant : i32<64>
    //   t36: v64f32 = bitcast t24
    //
    if (VectorArg.getOpcode() == ISD::BITCAST) {
      SDValue OrigArg = VectorArg.getOperand(0);
      EVT OrigType = OrigArg.getValueType();
      assert(OrigType.getSizeInBits() == 4096);
      EVT ResultType =
          EVT::getVectorVT(*DAG.getContext(), OrigType.getVectorElementType(),
                           OrigType.getVectorNumElements() / 2);
      SDValue SubRegNo = DAG.getConstant(
          EltNo * ResultType.getVectorNumElements(), DL, MVT::i32);
      SDValue ExtrSubV = DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, ResultType,
                                     OrigArg, SubRegNo);
      if (ResultType == ResultVT)
        return ExtrSubV;
      SDValue Result = DAG.getNode(ISD::BITCAST, DL, ResultVT, ExtrSubV);
      return Result;
    }
  }

  return SDValue();
}

static SDValue performIncGPtrCombine(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const TPCSubtarget *Subtarget) {
  assert(N->getOpcode() == ISD::ADD);
  assert(N->getValueType(0) == MVT::i64);
  if (!Subtarget->getFeatureBits()[TPC::FeatureGen3Plus])
    return SDValue();

  SDLoc DL(N);
  SDNode *Result = nullptr;

  LLVM_DEBUG(dbgs() << "Try to remove: "; N->dump(&DAG); dbgs() << '\n');

  // InstCombine convers sequence of pointer increments into increments of a
  // single pointer. For example, the C code:
  //
  // void main(tensor in, int src1, int src2, int src3) {
  //   int5 ndx = 0;
  //   int __global *ptr = (int __global *) gen_addr(ndx, in, 0, 0, 1, 0);
  //   *ptr++ = src1;
  //   *ptr++ = src2;
  //   *ptr = src3;
  // }
  //
  // is converted to DAG like:
  //
  //  t10: i64 = llvm.tpc.gen.addr ...
  //  t14: ch = store<(store 4 into %ir.1, !tbaa !3, addrspace 3)> t0, t1, t10, undef : i64
  //  t12 : i64 = add nuw t10, Constant : i64<4>
  //  t17 : ch = store<(store 4 into %ir.2, !tbaa !3, addrspace 3)> t0, t2, t12, undef : i64
  //  t16 : i64 = add nuw t10, Constant : i64<8>
  //  t21 : ch = store<(store 4 into %ir.3, !tbaa !3, addrspace 3)> t0, t3, t16, undef : i64
  //
  // Here all values of global pointer are calculated using single value pointer
  // (t10) but different values of offset. To convert such DAG into the form
  // where increments are used, repeatedly replace this 'base' pointer in
  // increments with post-incremented value, thus reducing offsets until the
  // gived 'add' node gets zero offset.
  SDNode *CurrentNode = N;
  do {
    assert(CurrentNode->getOpcode() == ISD::ADD);
    assert(CurrentNode->getValueType(0) == MVT::i64);
    SDValue BasePointer = CurrentNode->getOperand(0);

    // Scan "base" pointer users. All of them must be either memory operations
    // or increments. The aime is to find the last memory operation that uses
    // this pointer.
    MemSDNode *Last = nullptr;
    {
      SmallVector<MemSDNode *, 8> LoadsAndStores;
      for (auto U : BasePointer.getNode()->uses()) {
        if (U->getOpcode() == ISD::TokenFactor)
          continue;
        if (auto Ld = dyn_cast<LoadSDNode>(U)) {
          if (Ld->getAddressSpace() == GLOBAL &&
              Ld->getBasePtr() == BasePointer)
            LoadsAndStores.push_back(Ld);
        } else if (auto St = dyn_cast<StoreSDNode>(U)) {
          if (St->getAddressSpace() == GLOBAL &&
              St->getBasePtr() == BasePointer) {
            if (!Subtarget->getFeatureBits()[TPC::FeatureGen4Plus])
              return SDValue();
            else
              LoadsAndStores.push_back(St);
          }
        } else if (U->getOpcode() != ISD::ADD) {
          if (isa<StoreSDNode>(BasePointer.getNode()))
            return SDValue();
        }
      }

      // Get the last user of the "base" pointer that is a memory operation.
      // There must be such as pointer increments may appear only from *ptr++.
      if (LoadsAndStores.empty())
        return SDValue();
      Last = LoadsAndStores.back();
      {
        ArrayRef<MemSDNode *> Preceding(LoadsAndStores);
        Preceding = Preceding.slice(0, Preceding.size() - 1);
        for (auto I : Preceding) {
          if (I != Last && !I->isPredecessorOf(Last))
            Last = I;
        }
      }
    }

    // Look for minimal increment among ADDs. The node with the minimal
    // increment will be merged into preceeding memory operation. Others will be
    // be replaced with ADDs with reduced increment values.
    struct IncInstr {
      SDNode *Instr;
      uint64_t Inc;
      SDValue Replacement;
    };
    SmallVector<IncInstr, 8> Incrs;
    uint64_t MinInc;

    if (auto IncVal = dyn_cast<ConstantSDNode>(N->getOperand(1)))
      MinInc = IncVal->getZExtValue();
    else
      return SDValue();

    // Collect all increment nodes.
    for (auto U : BasePointer.getNode()->uses())
      if (U->getOpcode() == ISD::ADD) {
        uint64_t CInc;
        if (auto CIncVal = dyn_cast<ConstantSDNode>(U->getOperand(1)))
          CInc = CIncVal->getZExtValue();
        else
          return SDValue();
        Incrs.push_back(IncInstr{U, CInc, SDValue()});
        if (CInc < MinInc)
          MinInc = CInc;
      }
    assert(MinInc > 0);
    assert(MinInc == 1 || MinInc == 2 || MinInc == 4 || MinInc == 8);
    std::sort(Incrs.begin(), Incrs.end(),
              [](const IncInstr &N1, const IncInstr &N2) -> bool {
                return N1.Inc <= N2.Inc;
              });

    // Create replacement for the last memory operation.
    SDValue NewPointer;
    SDValue IncValue = DAG.getConstant(MinInc, SDLoc(Last), MVT::i64);
    if (auto Ld = dyn_cast<LoadSDNode>(Last)) {
      NewPointer =
          DAG.getIndexedLoad(SDValue(Ld, 0), SDLoc(Ld), Ld->getBasePtr(),
                             IncValue, ISD::MemIndexedMode::POST_INC);

    } else if (auto St = dyn_cast<StoreSDNode>(Last)) {
      NewPointer =
          DAG.getIndexedStore(SDValue(St, 0), SDLoc(Last), St->getBasePtr(),
                              IncValue, ISD::MemIndexedMode::POST_INC);
    } else {
      llvm_unreachable("Unexpected node");
    }

    // Create replacements for pointer increments with reduced value.
    for (auto &I : Incrs) {
      assert(I.Inc >= MinInc);
      if (I.Inc == MinInc) {
        // Replace increment with the base pointer value.
        LLVM_DEBUG(dbgs() << "  Node going to be removed: ";
                   I.Instr->dump(&DAG); dbgs() << "\n";);
      } else {
        // Replace increment with updated ADD node.
        SDValue NewInc;
        if (isa<StoreSDNode>(NewPointer))
          NewInc = DAG.getNode(
              ISD::ADD, SDLoc(I.Instr), I.Instr->getValueType(0), NewPointer,
              DAG.getConstant(I.Inc - MinInc, SDLoc(I.Instr), MVT::i64));
        else
          NewInc = DAG.getNode(
              ISD::ADD, SDLoc(I.Instr), I.Instr->getValueType(0),
              SDValue(NewPointer.getNode(), 1),
              DAG.getConstant(I.Inc - MinInc, SDLoc(I.Instr), MVT::i64));
        NewInc->setFlags(I.Instr->getFlags());
        LLVM_DEBUG(dbgs() << "  Node: "; I.Instr->dump(&DAG);
                   dbgs() << "  Will be replaced with: ";
                   NewInc.getNode()->dump(&DAG); dbgs() << "\n";);
        I.Replacement = NewInc;
      }
    }

    // Replace the last memory operation with post-incremented operation. It
    // becomes new pointer source.
    LLVM_DEBUG(dbgs() << "  Replacing: "; Last->dump(&DAG);
               dbgs() << "  With: "; NewPointer.getNode()->dump(&DAG);
               dbgs() << "\n";);
    if (isa<LoadSDNode>(Last)) {
      DAG.ReplaceAllUsesOfValueWith(SDValue(Last, 0), NewPointer.getValue(0));
      DAG.ReplaceAllUsesOfValueWith(SDValue(Last, 1), NewPointer.getValue(2));
    } else {
      DAG.ReplaceAllUsesOfValueWith(SDValue(Last, 0), NewPointer.getValue(1));
    }
    DCI.recursivelyDeleteUnusedNodes(Last);
    DCI.AddToWorklist(NewPointer.getNode());
    Last = nullptr;

    // Replace increment operations.
    for (auto I : Incrs) {
      if (I.Instr == CurrentNode && !I.Replacement) {
        Result = NewPointer.getNode();
        LLVM_DEBUG(dbgs() << "Found replacement\n";);
      }
      LLVM_DEBUG(dbgs() << "  Replacing: "; I.Instr->dump(&DAG);
                 dbgs() << "  With: ";
                 if (I.Replacement) I.Replacement.getNode()->dump(&DAG);
                 else NewPointer->dump(&DAG); dbgs() << "\n";);
      if (I.Replacement)
        DAG.ReplaceAllUsesOfValueWith(SDValue(I.Instr, 0), I.Replacement);
      else {
        if (isa<LoadSDNode>(NewPointer))
          DAG.ReplaceAllUsesOfValueWith(SDValue(I.Instr, 0),
                                        SDValue(NewPointer.getNode(), 1));
        else
          DAG.ReplaceAllUsesOfValueWith(SDValue(I.Instr, 0), NewPointer);
      }
      if (I.Instr == CurrentNode) {
        CurrentNode = I.Replacement.getNode();
      }
      if (I.Replacement) {
        //        DCI.recursivelyDeleteUnusedNodes(I.Instr);
        DCI.AddToWorklist(I.Replacement.getNode());
      }
    }

    LLVM_DEBUG(dbgs() << "DAG after this round:\n"; DAG.dump(););
  } while (!Result);

  LLVM_DEBUG(dbgs() << "Ultimate replacement: "; Result->dump(&DAG););

  if (Result->getOpcode() == ISD::LOAD)
    return SDValue(Result, 1);
  return SDValue(Result, 0);
}

static SDValue performShuffleCombine(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const TPCSubtarget *Subtarget) {
  assert(N->getOpcode() == ISD::VECTOR_SHUFFLE);
  LLVM_DEBUG(
    dbgs() << "Try to combine: ";
    N->dump(&DAG);
    dbgs() << '\n'
  );

  SDLoc DL(N);
  auto *ShuffleVector = cast<ShuffleVectorSDNode>(N);
  SDValue Op1 = N->getOperand(0);
  SDValue Op2 = N->getOperand(1);
  EVT SrcTy = Op1->getValueType(0);
  EVT EltTy = SrcTy.getVectorElementType();
  EVT DestTy = ShuffleVector->getValueType(0);
  ArrayRef<int> Mask = ShuffleVector->getMask();
  unsigned VectorSize = getFullVectorSize(EltTy);
  EVT VectorTy = EVT::getVectorVT(*DAG.getContext(), EltTy, VectorSize);

  // Handle constructs where vector_shuffle ignores some subvector of the
  // concatenated value. Try to replace it with shorter vector_shuffle or with
  // another shorter construct to obtain finally concat_vectors where the
  // ignored part is undef.
  //
  // As an example:
  //
  //   t18: v128f32 = concat_vectors t17, undef:v64f32
  //   t20: v128f32 = vector_shuffle<u, ... 63 more 'u', 0, ... 63 more '0'> t18, undef:v128f32
  //
  // is replaced by
  //
  //   t20a = vector_shuffle<0, ... 63 more '0'> t17, undef:v64f32
  //   t21a = concat_vectors undef:v64f32, t20a

  if (VectorSize * 2 == Mask.size()) {
    ArrayRef<int> HalfMask = getHalfMask(Mask, 0);
    if (isUndefinedMask(HalfMask)) {
      if (Op1.getOpcode() == ISD::CONCAT_VECTORS &&
          Op1.getNumOperands() == 2 &&
          Op2.getOpcode() == ISD::UNDEF) {
        HalfMask = getHalfMask(Mask, 1);
        SDValue NewShuffOp1 = Op1.getOperand(0);
        SDValue NewShuffle = DAG.getVectorShuffle(VectorTy, DL, NewShuffOp1, DAG.getUNDEF(VectorTy), HalfMask);
        return DAG.getNode(ISD::CONCAT_VECTORS, DL, DestTy, DAG.getUNDEF(VectorTy), NewShuffle);
      }
    }
  }
  return SDValue();
}


static SDValue isSplatValue(SDValue V) {
  if (V.getOpcode() == ISD::BUILD_VECTOR && isSplat(V))
    return V.getOperand(0);
  if (V.getOpcode() == ISD::VECTOR_SHUFFLE) {
    auto *VS = cast<ShuffleVectorSDNode>(V);
    if (VS->isSplat()) {
      SDValue Op0 = V->getOperand(0);
      if (Op0.getOpcode() == ISD::INSERT_VECTOR_ELT)
        return Op0.getOperand(1);
    }
  }
  return SDValue();
}
#if 0
static SDValue makeTargetConst(SDValue C, const SDLoc &DL, SelectionDAG &DAG) {
  if (C.getOpcode() == ISD::TargetConstant || C.getOpcode() == ISD::TargetConstantFP)
    return C;
  if (auto FC = dyn_cast<ConstantFPSDNode>(C.getNode()))
    return DAG.getConstantFP(FC->getValueAPF(), DL, C.getValueType(), true);
  if (auto IC = dyn_cast<ConstantSDNode>(C.getNode()))
    return DAG.getConstant(IC->getAPIntValue(), DL, C.getValueType(), true);
  llvm_unreachable("Not a constant");
}
#endif


enum SEL_kind {
  SEL_vvvv,
  SEL_vivv,
  SEL_vsvv,
  SEL_vvvi,
  SEL_vvvs,
  SEL_total
};

enum CMP_kind {
  CMP_eq,
  CMP_ne,
  CMP_lt,
  CMP_le,
  CMP_gt,
  CMP_ge,
  CMP_total
};

unsigned SEL_map[CMP_total][SEL_total] = {
  { TPC::SEL_EQvvvvp,   TPC::SEL_EQvivvp,   TPC::SEL_EQvsvvp,   TPC::SEL_EQvvvip,   TPC::SEL_EQvvvsp },
  { TPC::SEL_NEQvvvvp,  TPC::SEL_NEQvivvp,  TPC::SEL_NEQvsvvp,  TPC::SEL_NEQvvvip,  TPC::SEL_NEQvvvsp },
  { TPC::SEL_LESSvvvvp, TPC::SEL_LESSvivvp, TPC::SEL_LESSvsvvp, TPC::SEL_LESSvvvip, TPC::SEL_LESSvvvsp },
  { TPC::SEL_LEQvvvvp,  TPC::SEL_LEQvivvp,  TPC::SEL_LEQvsvvp,  TPC::SEL_LEQvvvip,  TPC::SEL_LEQvvvsp },
  { TPC::SEL_GRTvvvvp,  TPC::SEL_GRTvivvp,  TPC::SEL_GRTvsvvp,  TPC::SEL_GRTvvvip,  TPC::SEL_GRTvvvsp },
  { TPC::SEL_GEQvvvvp,  TPC::SEL_GEQvivvp,  TPC::SEL_GEQvsvvp,  TPC::SEL_GEQvvvip,  TPC::SEL_GEQvvvsp }
};

static SDValue performVSelectCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo &DCI,
                                    const TPCSubtarget *Subtarget) {
  assert(N->getOpcode() == ISD::VSELECT);
  SDLoc DL(N);
  SDValue SelectCond = N->getOperand(0);
  SDValue OpTrue = N->getOperand(1);
  SDValue OpFalse = N->getOperand(2);
  EVT ResultTy = N->getValueType(0);

  if (SelectCond.getOpcode() == ISD::SETCC) {
    SDValue Op1 = SelectCond.getOperand(0);
    SDValue Op2 = SelectCond.getOperand(1);
    CondCodeSDNode *CondNode = cast<CondCodeSDNode>(SelectCond.getOperand(2));
    ISD::CondCode Cond = CondNode->get();
    EVT CmpTy = Op1.getValueType();

    // If compared values are floating point, they cannot be swapped due to NaNs.
    if (CmpTy.isFloatingPoint())
      return SDValue();

    bool SwappedCond = false;

    if (isSplatValue(OpTrue)) {
      std::swap(OpTrue, OpFalse);
      SwappedCond = true;
    }
    if (isSplatValue(Op1)) {
      std::swap(Op1, Op2);
      SwappedCond = !SwappedCond;
    }

    ISD::CondCode NewCond = Cond;
    if (SwappedCond)
      switch (Cond) {
      case ISD::CondCode::SETEQ:
      case ISD::CondCode::SETNE:
        break;
      case ISD::CondCode::SETLT: NewCond = ISD::CondCode::SETGT; break;
      case ISD::CondCode::SETGT: NewCond = ISD::CondCode::SETLT; break;
      case ISD::CondCode::SETLE: NewCond = ISD::CondCode::SETGE; break;
      case ISD::CondCode::SETGE: NewCond = ISD::CondCode::SETLE; break;
      default:
        llvm_unreachable("Invalid condition code");
      }

    if (NewCond != Cond) {
      SDValue NewCmp = DAG.getSetCC(DL, SelectCond.getValueType(), Op1, Op2, NewCond);
      SDValue NewSel = DAG.getSelect(DL, ResultTy, NewCmp, OpTrue, OpFalse);
      return NewSel;
    }
  }

  return SDValue();
}

SDValue TPCTargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default:
    LLVM_DEBUG(dbgs() << "Custom combining: skipping\n");
    break;
  case ISD::BITCAST:
    return performBitCastCombine(N, DAG, DCI, Subtarget);
  case ISD::ADD:
    if (N->getValueType(0) == MVT::i64)
      return performIncGPtrCombine(N, DAG, DCI, Subtarget);
    break;
  case ISD::VECTOR_SHUFFLE:
    return performShuffleCombine(N, DAG, DCI, Subtarget);
  case ISD::VSELECT:
    return performVSelectCombine(N, DAG, DCI, Subtarget);
  }

  return SDValue();
}

SDValue TPCTargetLowering::FormFpNumScalar(SDValue Op1, SDValue Op2,
                                           SDValue Op3, SelectionDAG &DAG,
                                           SDLoc &DL) const {
  assert(!Op1->getValueType(0).isVector() &&
         !Op2->getValueType(0).isVector() &&
         !Op3->getValueType(0).isVector() &&
         "Scalars are expected");
  
  Op2 = SDValue(
    DAG.getMachineNode(
      TPC::ANDsip, DL, MVT::i32,
      {Op2, /*Src1*/
       DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(MVT::i32), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Op3 = SDValue(
    DAG.getMachineNode(
      TPC::ANDsip, DL, MVT::i32,
      {Op3, /*Src1*/
       DAG.getTargetConstant(0x007fffff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(MVT::i32), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Op1 = SDValue(
    DAG.getMachineNode(
      TPC::SHLsip, DL, MVT::i32,
      {Op1, /*Src1*/
       DAG.getTargetConstant(0x17, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(MVT::i32), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue BinaryOr1 = SDValue(
    DAG.getMachineNode(
      TPC::ORssp, DL, MVT::i32,
      {Op2, /*Src1*/
       Op1, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(MVT::i32), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue BinaryOr2 = SDValue(
    DAG.getMachineNode(
      TPC::ORssp, DL, MVT::i32,
      {BinaryOr1, /*Src1*/
       Op3, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(MVT::i32), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  return BinaryOr2;
}

SDValue TPCTargetLowering::ReciprocalCephesFastF32(SDValue Op,
                                                   SelectionDAG &DAG,
                                                   SDLoc &DL) const {
  EVT Type = Op->getValueType(0);
  bool IsVector = Type.isVector();
  
  const float A = 2.58586f;
  const float B = -5.81818f;
  const float C = 4.24242f;
  
  EVT FloatType = Type;
  EVT IntType = IsVector ? MVT::v64i32 : MVT::i32;

  SDValue Significand = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::ANDvip : TPC::ANDsip, DL, IntType,
      {Op, /*Src1*/
       DAG.getTargetConstant(0x007fffff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Significand = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::ORvip : TPC::ORsip, DL, IntType,
      {Significand, /*Src1*/
       DAG.getTargetConstant(0x3f000000, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue Result = Significand;
  
  SDValue Exponent = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::SHRvip : TPC::SHRsip, DL, IntType,
      {Op, /*Src1*/
       DAG.getTargetConstant(0x17, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Exponent = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::ANDvip : TPC::ANDsip, DL, IntType,
      {Exponent, /*Src1*/
       DAG.getTargetConstant(0x000000ff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Exponent = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::SUBvip : TPC::SUBsip, DL, IntType,
      {Exponent, /*Src1*/
       DAG.getTargetConstant(0x7e, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Exponent, /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue Temp0Income = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MOV_ld_vip : TPC::MOV_ld_sip, DL, FloatType,
      {DAG.getTargetConstant(APInt::floatToBits(B), DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Temp0 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MACf32vip : TPC::MACsip, DL, FloatType,
      {Result, /*Src1*/
       DAG.getTargetConstant(APInt::floatToBits(A), DL, MVT::i32),
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp0Income, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue Temp1Income = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MOV_ld_vip : TPC::MOV_ld_sip, DL, FloatType,
      {DAG.getTargetConstant(APInt::floatToBits(C), DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(FloatType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Temp1 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MACf32vvp : TPC::MACssp, DL, FloatType,
      {Result, /*Src1*/
       Temp0, /*Src*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp1Income, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue NegResult = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::XORvip : TPC::XORsip, DL, FloatType,
      {Result, /*Src1*/
       DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(FloatType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Temp2Temp0Income = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MOV_ld_vip : TPC::MOV_ld_sip, DL, FloatType,
      {DAG.getTargetConstant(APInt::floatToBits(2.0), DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(FloatType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Temp2 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MACf32vvp : TPC::MACssp, DL, FloatType,
      {NegResult, /*Src1*/
       Temp1, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp2Temp0Income, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Temp2 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MULf32vvp : TPC::MULssp, DL, FloatType,
      {Temp2, /*Src1*/
       Temp1, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp2, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Temp0 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MACf32vvp : TPC::MACssp, DL, FloatType,
      {NegResult, /*Src1*/
       Temp2, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp2Temp0Income, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  Temp0 = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MULf32vvp : TPC::MULssp, DL, FloatType,
      {Temp0, /*Src1*/
       Temp2, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Temp0, /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  SDValue Exp = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::SHRvip : TPC::SHRsip, DL, IntType,
      {Temp0, /*Src1*/
       DAG.getTargetConstant(0x17, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Exp = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::ANDvip : TPC::ANDsip, DL, IntType,
      {Exp, /*Src1*/
       DAG.getTargetConstant(0x000000ff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Exp = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::SUBvvp : TPC::SUBssp, DL, IntType,
      {Exp, /*Src1*/
       Exponent, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Exp = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::ANDvip : TPC::ANDsip, DL, IntType,
      {Exp, /*Src1*/
       DAG.getTargetConstant(0x000000ff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(IntType), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  
  if (IsVector)
    return SDValue(
      DAG.getMachineNode(
        TPC::FORM_FP_NUMvvvp, DL, FloatType,
        {Exp, /*Src1*/
         Op, /*Src2*/
         Temp0, /*Src3*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_EXP_IS_NUM, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(IntType), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  else
    return FormFpNumScalar(Exp, Op, Temp0, DAG, DL);
}

const unsigned FltMin = 0x800000;
const unsigned FltMax = 0x7f7fffff;
const unsigned NanInf = 0x7fffffff;
const unsigned PlusInf = 0x7f800000;

SDValue TPCTargetLowering::ReciprocalCephesF32(SDValue Op, SelectionDAG &DAG,
                                               SDLoc &DL) const {
  EVT Type = Op->getValueType(0);
  bool IsVector = Type.isVector();
  
  SDValue Result = ReciprocalCephesFastF32(Op, DAG, DL);

  SDValue AbsX = SDValue(IsVector ?
    DAG.getMachineNode(
      TPC::FORM_FP_NUMvvvp, DL, Type,
      {Op, /*Src1*/
       Op, /*Src2*/
       Op, /*Src3*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(TPCII::SW_FORCE_SIGN0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}) /*Polarity*/
    :
    DAG.getMachineNode(
      TPC::ANDsip, DL, Type,
      {Op, /*Src1*/
       DAG.getTargetConstant(0x7fffffff, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  if (Subtarget->hasGoyaISA()) {
    SDValue AbsResult = SDValue(IsVector ?
      DAG.getMachineNode(
        TPC::FORM_FP_NUMvvvp, DL, Type,
        {Result, /*Src1*/
         Result, /*Src2*/
         Result, /*Src3*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_FORCE_SIGN0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}) /*Polarity*/
      :
      DAG.getMachineNode(
        TPC::ANDsip, DL, Type,
        {Result, /*Src1*/
         DAG.getTargetConstant(0x7fffffff, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    if (IsVector) {
      SDValue FltMinNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(FltMin, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      SDValue PlusInfNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::SEL_LESSvvvvp, DL, Type,
          {AbsX, /*Src1*/
           FltMinNode, /*Src2*/
           PlusInfNode, /*Src3*/
           AbsResult, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      SDValue FltMaxNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(FltMax, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      SDValue ZeroNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(APInt::floatToBits(0.0), DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::SEL_GEQvvvvp, DL, Type,
          {AbsX, /*Src1*/
           FltMaxNode, /*Src2*/
           ZeroNode, /*Src3*/
           AbsResult, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      SDValue NanInfNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::SEL_GRTvvvvp, DL, Type,
          {AbsX, /*Src1*/
           PlusInfNode, /*Src2*/
           NanInfNode, /*Src3*/
           AbsResult, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Result = SDValue (
        DAG.getMachineNode(
          TPC::FORM_FP_NUMvvvp, DL, Type,
          {AbsResult, /*Src1*/
           Op, /*Src2*/
           AbsResult, /*Src3*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    } else {
      SDValue Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_LESSsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(FltMin, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           AbsResult, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_GEQsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(FltMax, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(APInt::floatToBits(0.0), DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           AbsResult, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_GRTsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      AbsResult = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           AbsResult, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      SDValue SignRes = SDValue(
        DAG.getMachineNode(
          TPC::ANDsip, DL, Type,
              {Op, /*Src1*/
               DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
               DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
               DAG.getUNDEF(Type), /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::ORssp, DL, Type,
              {AbsResult, /*Src1*/
               SignRes, /*Src2*/
               DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
               DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
               DAG.getUNDEF(Type), /*Income*/
               DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
               DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    }
  } else {
    SDValue FClass = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::FCLASSvvp : TPC::FCLASSsp, DL, Type,
        {Op, /*Src1*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    Result = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::CALC_FP_SPECIALvvp : TPC::CALC_FP_SPECIALssp, DL, Type,
        {FClass, /*Src1*/
         FClass, /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
         Result, /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    if (IsVector) {
      SDValue FltMaxNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(FltMax, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      SDValue ZeroNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(APInt::floatToBits(0.0), DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::SEL_GEQvvvvp, DL, Type,
          {AbsX, /*Src1*/
           FltMaxNode, /*Src2*/
           ZeroNode, /*Src3*/
           Result, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    } else {
      SDValue Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_GEQsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(FltMax, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(APInt::floatToBits(0.0), DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    }
  }
  
  return Result;
}

// For details look at s_div_cephes_f32 and div_cephes_f32 in
// test-rt/operator/div_helper.h
SDValue TPCTargetLowering::DivCephesF32(SDValue LHO, SDValue RHO,
                                        SelectionDAG &DAG, SDLoc &DL) const {
  EVT Type = LHO->getValueType(0);
  assert((Type == MVT::f32 ||
          Type == MVT::v64f32) &&
         "Expected f32 scalar or vector type");
  bool IsVector = Type.isVector();
  
  RHO = ReciprocalCephesF32(RHO, DAG, DL);
  SDValue Result = SDValue(
    DAG.getMachineNode(
      IsVector ? TPC::MULf32vvp : TPC::MULssp, DL, Type,
      {LHO,
       RHO,
       DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Accumulator*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  if (Subtarget->hasGoyaISA()) {
    SDValue AbsX = SDValue(IsVector ?
      DAG.getMachineNode(
        TPC::FORM_FP_NUMvvvp, DL, Type,
        {LHO, /*Src1*/
         LHO, /*Src2*/
         LHO, /*Src3*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_FORCE_SIGN0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}) /*Polarity*/
      :
      DAG.getMachineNode(
        TPC::ANDsip, DL, Type,
        {LHO, /*Src1*/
         DAG.getTargetConstant(0x7fffffff, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue AbsY = SDValue(IsVector ?
      DAG.getMachineNode(
        TPC::FORM_FP_NUMvvvp, DL, Type,
        {RHO, /*Src1*/
         RHO, /*Src2*/
         RHO, /*Src3*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_FORCE_SIGN0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}) /*Polarity*/
      :
      DAG.getMachineNode(
        TPC::ANDsip, DL, Type,
        {RHO, /*Src1*/
         DAG.getTargetConstant(0x7fffffff, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue SignRes = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::XORvvp : TPC::XORssp, DL, Type,
          {LHO, /*Src1*/
           RHO, /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    SignRes = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::ANDvip : TPC::ANDsip, DL, Type,
        {SignRes, /*Src1*/
         DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue InfXY = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::ORvip : TPC::ORsip, DL, Type,
        {SignRes, /*Src1*/
         DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    if (IsVector) {
      SDValue PlusInfNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      SDValue NanInfNode = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_vip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::SEL_GRTvvvvp, DL, Type,
          {AbsX, /*Src1*/
           PlusInfNode, /*Src2*/
           NanInfNode, /*Src3*/
           Result, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i8), /*Func*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      SDValue InfX = SDValue(
        DAG.getMachineNode(
          TPC::CMP_EQvvp, DL, Type,
          {AbsX, /*Src1*/
           PlusInfNode,
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::SEL_EQvvvvm, DL, Type,
          {AbsY, /*Src1*/
           PlusInfNode, /*Src2*/
           NanInfNode, /*Src3*/
           InfXY, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i8), /*Func*/
           Result, /*Income*/
           InfX, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      SDValue ZeroX = SDValue(
        DAG.getMachineNode(
          TPC::CMP_LESSvip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(FltMin, DL, MVT::i32),
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::SEL_LESSvivvm, DL, Type,
          {AbsY, /*Src1*/
           DAG.getTargetConstant(FltMin, DL, MVT::i32), /*Src2*/
           NanInfNode, /*Src3*/
           SignRes, /*Src4*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i8), /*Func*/
           Result, /*Income*/
           ZeroX, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    } else {
      SDValue Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_GRTsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_EQsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_EQsip, DL, Type,
          {AbsY, /*Src1*/
           DAG.getTargetConstant(PlusInf, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Predicate, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_LESSsip, DL, Type,
          {AbsX, /*Src1*/
           DAG.getTargetConstant(FltMin, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           DAG.getUNDEF(Type), /*Income*/
           DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_ssp, DL, Type,
          {SignRes, /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);

      Predicate = SDValue(
        DAG.getMachineNode(
          TPC::CMP_LESSsip, DL, Type,
          {AbsY, /*Src1*/
           DAG.getTargetConstant(FltMin, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Predicate, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      Result = SDValue(
        DAG.getMachineNode(
          TPC::MOV_ld_sip, DL, Type,
          {DAG.getTargetConstant(NanInf, DL, MVT::i32), /*Src*/
           DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           Predicate, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    }
  } else {
    SDValue FClassLHO = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::FCLASSvvp : TPC::FCLASSsp, DL, Type,
        {LHO, /*Src1*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue FClassRHO = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::FCLASSvvp : TPC::FCLASSsp, DL, Type,
        {RHO, /*Src1*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    Result = SDValue(
      DAG.getMachineNode(
        IsVector ? TPC::CALC_FP_SPECIALvvp : TPC::CALC_FP_SPECIALssp, DL, Type,
        {FClassLHO, /*Src1*/
         FClassRHO, /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::FP32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_DIV, DL, MVT::i8), /*Func*/
         Result, /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  }
  
  return Result;
}

static void MakeVectorDivOneIterarton(unsigned IterationNum,
                                      SDValue &Z, SDValue &B, SDValue &BR,
                                      SDValue &AbsLHO, SDValue AbsRHO,
                                      SDValue IsEquals1,
                                      SelectionDAG &DAG, const SDLoc &DL) {
  EVT Type = MVT::v64i32;
  Z = SDValue(
    DAG.getMachineNode(
      TPC::SHLvim, DL, Type,
      {Z, /*Src1*/
       DAG.getTargetConstant(1, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Z, /*Income*/
       IsEquals1, /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  BR = SDValue(
    DAG.getMachineNode(
      TPC::SHRvip, DL, Type,
      {AbsRHO, /*Src1*/
       DAG.getTargetConstant(31 - IterationNum, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  B = SDValue(
    DAG.getMachineNode(
      TPC::SHLvip, DL, Type,
      {AbsRHO, /*Src1*/
       DAG.getTargetConstant(IterationNum, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  SDValue Equal21 = SDValue(
    DAG.getMachineNode(
      TPC::CMP_EQvip, DL, Type,
      {BR, /*Src1*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Zeros = SDValue(
    DAG.getMachineNode(
      TPC::MOV_ld_vip, DL, Type,
      {DAG.getTargetConstant(0, DL, MVT::i1), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::BOOL, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Equal2 = SDValue(
    DAG.getMachineNode(
      TPC::CMP_GEQvvm, DL, Type,
      {AbsLHO, /*Src1*/
       B, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Zeros, /*Income*/
       Equal21, /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  Z = SDValue(
    DAG.getMachineNode(
      TPC::ORvim, DL, Type,
      {Z, /*Src1*/
       DAG.getTargetConstant(1, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       Z, /*Income*/
       Equal2, /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  AbsLHO = SDValue(
    DAG.getMachineNode(
      TPC::SUBvvm, DL, Type,
      {AbsLHO, /*Src1*/
       B, /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       AbsLHO, /*Income*/
       Equal2, /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
}

static std::pair<SDValue, SDValue> MakeVectorDivI32(SDValue LHO, SDValue RHO,
                                                    bool IsSign,
                                                    SelectionDAG &DAG,
                                                    const SDLoc &DL) {
  EVT Type = MVT::v64i32;
  SDValue B = SDValue(
    DAG.getMachineNode(
      TPC::MOV_ld_vip, DL, Type,
      {DAG.getTargetConstant(0, DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue BR = SDValue(
    DAG.getMachineNode(
      TPC::MOV_ld_vip, DL, Type,
      {DAG.getTargetConstant(0, DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue Z = SDValue(
      DAG.getMachineNode(
        TPC::MOV_ld_vip, DL, Type,
        {DAG.getTargetConstant(0, DL, MVT::i32), /*Src*/
         DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

  SDValue AndResult;
  if (IsSign) {
    SDValue Negativ = SDValue(
      DAG.getMachineNode(
        TPC::XORvvp, DL, Type,
        {LHO, /*Src1*/
         RHO, /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    AndResult = SDValue(
      DAG.getMachineNode(
        TPC::ANDvip, DL, Type,
        {Negativ, /*Src1*/
         DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  }

  SDValue AbsLHO = SDValue(
    DAG.getMachineNode(
      TPC::ABSvvp, DL, Type,
      {LHO, /*Src*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue AbsRHO = SDValue(
    DAG.getMachineNode(
      TPC::ABSvvp, DL, Type,
      {RHO, /*Src*/
       DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  SDValue IsEqual1 = SDValue(
    DAG.getMachineNode(
      TPC::CMP_GEQvip, DL, Type,
      {AbsRHO, /*Src1*/
       DAG.getTargetConstant(1, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);
  SDValue IsEqual0 = SDValue(
    DAG.getMachineNode(
      TPC::CMP_EQvip, DL, Type,
      {AbsRHO, /*Src1*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  Z = SDValue(
      DAG.getMachineNode(
        TPC::MOV_ld_vim, DL, Type,
        {DAG.getTargetConstant(2147483647, DL, MVT::i32), /*Src*/
         DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         Z, /*Income*/
         IsEqual0, /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

  for (int Step = 31; Step >= 0; --Step) {
    MakeVectorDivOneIterarton(Step, Z, B, BR, AbsLHO, AbsRHO, IsEqual1, DAG,
                              DL);
  }

  SDValue DivideResult;
  SDValue RemainResult;
  if (IsSign) {
    SDValue DividePred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_NEQvip, DL, Type,
        {AndResult, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    DivideResult = SDValue(
      DAG.getMachineNode(
        TPC::SUBvim, DL, Type,
        {Z, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
         Z, /*Income*/
         DividePred, /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue RemainPred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_LESSvip, DL, Type,
        {LHO, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    RemainResult = SDValue(
      DAG.getMachineNode(
        TPC::SUBvim, DL, Type,
        {AbsLHO, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
         AbsLHO, /*Income*/
         RemainPred, /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  } else {
    DivideResult = Z;
    RemainResult = AbsLHO;
  }

  return std::make_pair(DivideResult, RemainResult);
}

std::pair<SDValue, SDValue> TPCTargetLowering::MakeScalarDividePriorGen4(
    SDValue LHO, SDValue RHO, bool IsSigned,
    SelectionDAG &DAG, const SDLoc &DL) const {
  EVT Type = LHO->getValueType(0);

  assert((Type == MVT::i32 || Type == MVT::i16 || Type == MVT::i8) &&
         "Unexpectable type");
  assert(!Subtarget->hasGen4Plus() &&
         "For Gen 4 and upper use MakeScalarDivideUpperGen4(...)");

  unsigned UdivOpcode = Subtarget->hasGoyaISA() ?
        TPC::UDIV_STEP : TPC::UDIV_4STEP;

  unsigned StepWidth = 0;
  if (Subtarget->hasGoyaISA())
    StepWidth = 1;
  else if (Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA())
    StepWidth = 4;
  else if (Subtarget->hasGrecoISA())
    StepWidth = 8;

  unsigned Switch = 0;
  if (Subtarget->hasGoyaISA() ||
      Subtarget->hasGaudiISA() || Subtarget->hasGaudiBISA())
    Switch = TPCII::SW_STEP_REG;
  else if (Subtarget->hasGrecoISA())
    Switch = TPCII::SW_X2_UDIV_4STEP;

  int Step = Type.getSizeInBits() - (Subtarget->hasGrecoISA() ? 2 : 1);

  unsigned TypeValue = TPCII::OpType::Invalid;
  EVT UdivType = MVT::INVALID_SIMPLE_VALUE_TYPE;
  if (Type == MVT::i32) {
    TypeValue = TPCII::OpType::UINT32;
    UdivType = MVT::v2i32;
  } else if (Type == MVT::i16) {
    TypeValue = TPCII::OpType::UINT16;
    UdivType = MVT::v2i16;
  } else if (Type == MVT::i8) {
    TypeValue = TPCII::OpType::UINT8;
    UdivType = MVT::v2i8;
  }

  SDValue Zero = SDValue(
    DAG.getMachineNode(
      TPC::MOV_ld_sip, DL, Type,
      {DAG.getTargetConstant(0, DL, MVT::i32), /*Src*/
       DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
       DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(Type), /*Income*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  SDValue Income;
  SDValue Denominator;
  if (IsSigned) {
    SDValue AbsLHO = SDValue(
      DAG.getMachineNode(
        TPC::ABSssp, DL, Type,
        {LHO, /*Src*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    Income = createTuple({AbsLHO, Zero}, DAG);

    Denominator = SDValue(
      DAG.getMachineNode(
        TPC::ABSssp, DL, Type,
        {RHO, /*Src*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  } else {
    Income = createTuple({LHO, Zero}, DAG);
    Denominator = RHO;
  }

  while (Step >= 0) {
    Income = SDValue(
      DAG.getMachineNode(
        UdivOpcode, DL, UdivType,
        {Denominator, /*Denominator*/
         DAG.getTargetConstant(Step, DL, Type), /*Step*/
         DAG.getTargetConstant(TypeValue, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(Switch, DL, MVT::i32), /*Switch*/
         Income, /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
     0);
    Step -= StepWidth;
  }

  SDValue DivideResult = DAG.getTargetExtractSubreg(TPC::sub_s1, DL, MVT::i32,
                                                    Income);
  SDValue RemainResult = DAG.getTargetExtractSubreg(TPC::sub_s0, DL, MVT::i32,
                                                    Income);
  if (IsSigned) {
    SDValue XorResult = SDValue(
      DAG.getMachineNode(
        TPC::XORssp, DL, Type,
        {LHO, /*Src1*/
         RHO, /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    SDValue AndResult = SDValue(
      DAG.getMachineNode(
        TPC::ANDsip, DL, Type,
        {XorResult, /*Src1*/
         DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue DividePred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_NEQsip, DL, Type,
        {AndResult, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    DivideResult = SDValue(
      DAG.getMachineNode(
        TPC::SUBsip, DL, Type,
        {DivideResult, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
         DivideResult, /*Income*/
         DividePred, /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue RemainPred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_LESSsip, DL, Type,
        {LHO, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    RemainResult = SDValue(
      DAG.getMachineNode(
        TPC::SUBsip, DL, Type,
        {RemainResult, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
         RemainResult, /*Income*/
         RemainPred, /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  }

  return std::make_pair(DivideResult, RemainResult);
}

std::pair<SDValue, SDValue> TPCTargetLowering::MakeScalarDivideUpperGen4(
    SDValue LHO, SDValue RHO, unsigned Switch, bool IsSigned,
    SelectionDAG &DAG, const SDLoc &DL) const {
  EVT Type = LHO->getValueType(0);

  assert((Type == MVT::i32 || Type == MVT::i16 || Type == MVT::i8) &&
         "Wrong type for division");
  assert(Subtarget->hasGen4Plus() &&
         "For Gen 3 and lower use MakeScalarDividePriorGen4(...)");
  assert(Switch <= 2 && "Wrong switch value");

  EVT ResultType = Type;
  unsigned UdivOpcode = TPC::UDIV;
  if (Switch == TPCII::SW_DIV_MODE_BOTH) {
    UdivOpcode = TPC::UDIV_BOTH;

    if (Type == MVT::i32)
      ResultType = MVT::v2i32;
    else if (Type == MVT::i16)
      ResultType = MVT::v2i16;
    else if (Type == MVT::i8)
      ResultType = MVT::v2i8;
  }

  unsigned TypeValue = TPCII::OpType::Invalid;
  if (Type == MVT::i32)
    TypeValue = TPCII::OpType::UINT32;
  else if (Type == MVT::i16)
    TypeValue = TPCII::OpType::UINT16;
  else if (Type == MVT::i8)
    TypeValue = TPCII::OpType::UINT8;

  SDValue Numerator;
  SDValue Denominator;
  if (IsSigned) {
    Numerator = SDValue(
      DAG.getMachineNode(
        TPC::ABSssp, DL, Type,
        {LHO, /*Src*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    Denominator = SDValue(
      DAG.getMachineNode(
        TPC::ABSssp, DL, Type,
        {RHO, /*Src*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
  } else {
    Numerator = LHO;
    Denominator = RHO;
  }

  SDValue Result = SDValue(
    DAG.getMachineNode(
      UdivOpcode, DL, ResultType,
      {Numerator, /*Src1*/
       Denominator, /*Src2*/
       DAG.getTargetConstant(TypeValue, DL, MVT::i8), /*UdivType*/
       DAG.getTargetConstant(Switch, DL, MVT::i32), /*Switch*/
       DAG.getUNDEF(ResultType), /*Quotient/Remainder double reg*/
       DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
       DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
    0);

  SDValue DivideResult;
  SDValue RemainResult;
  if (IsSigned) {
    SDValue XorResult = SDValue(
      DAG.getMachineNode(
        TPC::XORssp, DL, Type,
        {LHO, /*Src1*/
         RHO, /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    SDValue AndResult = SDValue(
      DAG.getMachineNode(
        TPC::ANDsip, DL, Type,
        {XorResult, /*Src1*/
         DAG.getTargetConstant(0x80000000, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    SDValue DividePred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_NEQsip, DL, Type,
        {AndResult, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);
    SDValue RemainPred = SDValue(
      DAG.getMachineNode(
        TPC::CMP_LESSsip, DL, Type,
        {LHO, /*Src1*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
         DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
         DAG.getTargetConstant(0, DL, MVT::i32), /*Switch*/
         DAG.getUNDEF(Type), /*Income*/
         DAG.getRegister(TPC::SPRF_TRUE, MVT::i1), /*Predicate*/
         DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
      0);

    if (Switch == TPCII::SW_DIV_MODE_DIV) {
      DivideResult = SDValue(
        DAG.getMachineNode(
          TPC::SUBsip, DL, Type,
          {Result, /*Src1*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           DividePred, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    } else if (Switch == TPCII::SW_DIV_MODE_MOD) {
      RemainResult = SDValue(
        DAG.getMachineNode(
          TPC::SUBsip, DL, Type,
          {Result, /*Src1*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
           Result, /*Income*/
           RemainPred, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    } else if (Switch == TPCII::SW_DIV_MODE_BOTH) {
      DivideResult = DAG.getTargetExtractSubreg(
            TPC::sub_s0, DL, MVT::i32, Result);
      RemainResult = DAG.getTargetExtractSubreg(
            TPC::sub_s1, DL, MVT::i32, Result);

      DivideResult = SDValue(
        DAG.getMachineNode(
          TPC::SUBsip, DL, Type,
          {DivideResult, /*Src1*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
           DivideResult, /*Income*/
           DividePred, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
      RemainResult = SDValue(
        DAG.getMachineNode(
          TPC::SUBsip, DL, Type,
          {RemainResult, /*Src1*/
           DAG.getTargetConstant(0, DL, MVT::i32), /*Src2*/
           DAG.getTargetConstant(TPCII::OpType::INT32, DL, MVT::i8), /*Optype*/
           DAG.getTargetConstant(TPCII::SW_NEG, DL, MVT::i32), /*Switch*/
           RemainResult, /*Income*/
           RemainPred, /*Predicate*/
           DAG.getTargetConstant(0, DL, MVT::i1)}), /*Polarity*/
        0);
    }
  } else {
    if (Switch == TPCII::SW_DIV_MODE_DIV) {
      DivideResult = Result;
    } else if (Switch == TPCII::SW_DIV_MODE_MOD) {
      RemainResult = Result;
    } else if (Switch == TPCII::SW_DIV_MODE_BOTH) {
      DivideResult = DAG.getTargetExtractSubreg(
            TPC::sub_s0, DL, MVT::i32, Result);
      RemainResult = DAG.getTargetExtractSubreg(
            TPC::sub_s1, DL, MVT::i32, Result);
    }
  }

  return std::make_pair(DivideResult, RemainResult);
}

SDValue TPCTargetLowering::lowerUDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  if (X->getValueType(0).isVector())
    return MakeVectorDivI32(X, Y, false, DAG, DL).first;
  else {
    if (Subtarget->hasGen4Plus())
      return MakeScalarDivideUpperGen4(X, Y, TPCII::SW_DIV_MODE_DIV,
                                       false, DAG, DL).first;
    else
      return MakeScalarDividePriorGen4(X, Y, false, DAG, DL).first;
  }
}

SDValue TPCTargetLowering::lowerSDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  if (X->getValueType(0).isVector())
    return MakeVectorDivI32(X, Y, true, DAG, DL).first;
  else {
    if (Subtarget->hasGen4Plus())
      return MakeScalarDivideUpperGen4(X, Y, TPCII::SW_DIV_MODE_DIV,
                                       true, DAG, DL).first;
    else
      return MakeScalarDividePriorGen4(X, Y, true, DAG, DL).first;
  }
}

SDValue TPCTargetLowering::lowerUREM(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  if (X->getValueType(0).isVector())
    return MakeVectorDivI32(X, Y, false, DAG, DL).second;
  else {
    if (Subtarget->hasGen4Plus())
      return MakeScalarDivideUpperGen4(X, Y, TPCII::SW_DIV_MODE_MOD,
                                       false, DAG, DL).second;
    else
      return MakeScalarDividePriorGen4(X, Y, false, DAG, DL).second;
  }
}

SDValue TPCTargetLowering::lowerSREM(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  if (X->getValueType(0).isVector())
    return MakeVectorDivI32(X, Y, true, DAG, DL).second;
  else {
    if (Subtarget->hasGen4Plus())
      return MakeScalarDivideUpperGen4(X, Y, TPCII::SW_DIV_MODE_MOD,
                                       true, DAG, DL).second;
    else
      return MakeScalarDividePriorGen4(X, Y, true, DAG, DL).second;
  }
}

SDValue TPCTargetLowering::lowerFDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  return DivCephesF32(X, Y, DAG, DL);
}

SDValue TPCTargetLowering::lowerFREM(SDValue Op, SelectionDAG &DAG) const {
  // TODO: Can't be tested until all the underlying ops are supported.
  SDLoc DL(Op);
  EVT VT = Op->getValueType(0);
  // TODO: Support scalar FREM in future.
  if (!VT.isVector())
    report_fatal_error("FREM not supported on scalar pipe.");

  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);

  SDValue Div = DAG.getNode(ISD::FDIV, DL, VT, X, Y);
  SDValue Floor = DAG.getNode(ISD::FTRUNC, DL, VT, Div);
  SDValue Mul = DAG.getNode(ISD::FMUL, DL, VT, Floor, Y);
  return DAG.getNode(ISD::FSUB, DL, VT, X, Mul);
}

static SDValue expandBinaryOperation(SDValue Op, SelectionDAG &DAG) {
  SDLoc DL(Op);
  EVT ResTy = Op->getValueType(0);
  unsigned Operation = Op.getOpcode();
  EVT SubregTy;
  unsigned Multiplicity = ResTy.getSizeInBits().getFixedSize() /
                          EVT(MVT::v64i32).getSizeInBits().getFixedSize();
  switch (ResTy.getSimpleVT().SimpleTy) {
  case MVT::v256f32:
  case MVT::v128f32:
    SubregTy = MVT::v64f32;
    break;
  case MVT::v256bf16:
    SubregTy = MVT::v128bf16;
    break;
  case MVT::v256f16:
    SubregTy = MVT::v128f16;
    break;
  case MVT::v256i32:
  case MVT::v128i32:
    SubregTy = MVT::v64i32;
    break;
  case MVT::v256i16:
    SubregTy = MVT::v128i16;
    break;
  default:
    return SDValue();
  }

  SDValue A = Op.getOperand(0);
  SDValue B = Op.getOperand(1);

  SDValue SubReg = DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32);
  MachineSDNode *A0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                         SubregTy, A, SubReg);
  MachineSDNode *B0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL,
                                         SubregTy, B, SubReg);
  SDValue Sum0 = DAG.getNode(Operation, DL, A0->getValueType(0),
                             SDValue(A0, 0), SDValue(B0, 0));

  SubReg = DAG.getTargetConstant(TPC::sub_1, DL, MVT::i32);
  A0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, A, SubReg);
  B0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, B, SubReg);
  SDValue Sum1 = DAG.getNode(Operation, DL, A0->getValueType(0),
                             SDValue(A0, 0), SDValue(B0, 0));

  if (Multiplicity == 2)
    return DAG.getNode(ISD::CONCAT_VECTORS, DL, ResTy, Sum0, Sum1);

  SubReg = DAG.getTargetConstant(TPC::sub_2, DL, MVT::i32);
  A0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, A, SubReg);
  B0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, B, SubReg);
  SDValue Sum2 = DAG.getNode(Operation, DL, A0->getValueType(0),
                             SDValue(A0, 0), SDValue(B0, 0));

  SubReg = DAG.getTargetConstant(TPC::sub_3, DL, MVT::i32);
  A0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, A, SubReg);
  B0 = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, SubregTy, B, SubReg);
  SDValue Sum3 = DAG.getNode(Operation, DL, A0->getValueType(0),
                             SDValue(A0, 0), SDValue(B0, 0));

  return DAG.getNode(ISD::CONCAT_VECTORS, DL, ResTy, Sum0, Sum1, Sum2, Sum3);
}

SDValue TPCTargetLowering::lowerFADD(SDValue Op, SelectionDAG &DAG) const {
  // TODO: Use ADD.X2 on Gen4
  return expandBinaryOperation(Op, DAG);
}

SDValue TPCTargetLowering::lowerFSUB(SDValue Op, SelectionDAG &DAG) const {
  return expandBinaryOperation(Op, DAG);
}

SDValue TPCTargetLowering::lowerADD(SDValue Op, SelectionDAG &DAG) const {
  // TODO: Use ADD.X2 on Gen4
  return expandBinaryOperation(Op, DAG);
}

SDValue TPCTargetLowering::lowerSUB(SDValue Op, SelectionDAG &DAG) const {
  return expandBinaryOperation(Op, DAG);
}
