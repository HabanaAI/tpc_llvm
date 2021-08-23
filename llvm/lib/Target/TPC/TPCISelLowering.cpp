//===-- TPCISelLowering.cpp - TPC DAG Lowering
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

using namespace llvm;

#define DEBUG_TYPE "tpc-isel"

const unsigned VRF_REGISTER_LENGTH_IN_BYTES = 256;
const unsigned VRF_REGISTER_LENGTH_IN_BITS = 8 * VRF_REGISTER_LENGTH_IN_BYTES;
const unsigned SRF_REGISTER_LENGTH_IN_BYTES = 4;
const unsigned SRF_REGISTER_LENGTH_IN_BITS = 8 * SRF_REGISTER_LENGTH_IN_BYTES;

//
// Option to disable Scalar to IRF pass.
//
static cl::opt<bool> DisableIRFCopy("no-irf-copy", cl::Hidden, cl::ZeroOrMore,
                                    cl::init(false));

static cl::opt<std::string> TPCSchedPref("tpc-sched-preference", cl::Hidden,
                                         cl::ZeroOrMore, cl::init("source"));

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
  addRegisterClass(MVT::v2i32, &TPC::ZRFRegClass);
  addRegisterClass(MVT::v2i16, &TPC::ZRFRegClass);
  addRegisterClass(MVT::v2i8, &TPC::ZRFRegClass);

  setOperationAction(ISD::FADD, MVT::v256f32, Custom);
  setOperationAction(ISD::FADD, MVT::v128f32, Custom);
  setOperationAction(ISD::FSUB, MVT::v256f32, Custom);
  setOperationAction(ISD::FSUB, MVT::v128f32, Custom);
  setOperationAction(ISD::SUB, MVT::v256i32, Custom);
  setOperationAction(ISD::SUB, MVT::v128i32, Custom);

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

  setOperationAction(ISD::SELECT_CC, MVT::f8_143, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f8_152, Custom);
  setOperationAction(ISD::SELECT, MVT::f32, Custom);
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

  for (auto VT :
       {MVT::v256i32, MVT::v128i32, MVT::v256i16, MVT::v128i16, MVT::v256i8})
    setOperationAction(ISD::SIGN_EXTEND, VT, Custom);

  setOperationAction(ISD::ZERO_EXTEND, MVT::v128i32, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v256i32, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v256i16, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::v256i8, Custom);

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

  for (auto VT : {MVT::v128i16, MVT::v256i8, MVT::v256i16}) {
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

  setOperationAction(ISD::TRUNCATE, MVT::i32, Custom);

  for (auto VT : {MVT::i32, MVT::i16, MVT::i8}) {
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

  for (auto VT : {MVT::v64f32, MVT::v128f16, MVT::v128bf16}) {
    setOperationAction(ISD::FDIV, VT, Custom);
    setOperationAction(ISD::FREM, VT, Custom);
  }

  //  setOperationAction(ISD::SUB, MVT::v256i32, Expand);
  // we got v64i32 = BUILD_VECTOR t1305, t1308,... which can not be supported

  setTruncStoreAction(MVT::i64, MVT::i32, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::bf16, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::i64, MVT::i32, Custom);

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

  setMinFunctionAlignment(Align(2));
  computeRegisterProperties(Subtarget->getRegisterInfo());

  // we don't need to sink (and duplicate) compares
  // aggressively in CodeGenPrep.
  setHasMultipleConditionRegisters();
}

unsigned TPCTargetLowering::getZeroReg() const { return TPC::S31; }

bool TPCTargetLowering::isProfitableToHoist(Instruction *I) const {
  if (I->getOpcode() == Instruction::Call)
    return false;
  return TargetLowering::isProfitableToHoist(I);
}

unsigned getTargetConvertType(const TPCSubtarget &Subtarget) {
  if (Subtarget.hasGaudiISA()) {
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

#include "TPCGenCallingConv.inc"

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
    if (ArgLoc.isRegLoc()) {
      unsigned v_reg = mf.addLiveIn(ArgLoc.getLocReg(), &TPC::SRFRegClass);
      SDValue copy = DAG.getCopyFromReg(chain, dl, v_reg, ArgLoc.getLocVT());
      if (ArgLoc.getValVT() != ArgLoc.getLocVT()) {
        assert(ArgLoc.isExtInLoc());
        copy = DAG.getZExtOrTrunc(copy, dl, ArgLoc.getValVT());
      }
      InVals.push_back(copy);
    } else {
      llvm_unreachable("Not implemented");
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
    llvm_unreachable("unimplemented operand");
    return SDValue();
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
  case ISD::ATOMIC_CMP_SWAP:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
    return lowerAtomicCmpXchg(Op, DAG);
  case ISD::UDIV:
    return lowerUDIV(Op, DAG);
  case ISD::SDIV:
    return lowerSDIV(Op, DAG);
  case ISD::UREM:
    return lowerUREM(Op, DAG);
  case ISD::SREM:
    return lowerSREM(Op, DAG);
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

SDValue TPCTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
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
  SDValue cond = DAG.getNode(ISD::SETCC, SDLoc(Op), MVT::i1, Op0, Op1, Op4);

  SDValue SecondMov = DAG.getNode(TPCISD::COND_MOV_INVERT, SDLoc(Op),
                                  Op3->getVTList(), Op3, cond, Op2);
  return SecondMov;
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

  bool is_double_vect = false;
  if (VT.isVector()) {
    EVT EltT = VT.getVectorElementType();
    unsigned EltSize = EltT.getStoreSizeInBits();
    EVT OpType = Op1.getValueType();
    unsigned OpVectSize = OpType.getVectorNumElements();
    is_double_vect = EltSize == 32 && OpVectSize == 128;
  }
  if (is_double_vect) {
    EVT EltT = VT.getVectorElementType();
    EVT SubregType = EVT::getVectorVT(*DAG.getContext(), EltT, 64);
    SDValue Subreg1Start = DAG.getConstant(0, DL, MVT::i32);
    SDValue Subreg2Start = DAG.getConstant(64, DL, MVT::i32);
    SDValue Op1_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg1Start);
    SDValue Op1_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op1, Subreg2Start);
    SDValue Op2_1 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg1Start);
    SDValue Op2_2 =
        DAG.getNode(ISD::EXTRACT_SUBVECTOR, DL, SubregType, Op2, Subreg2Start);
    EVT SubregCondType = EVT::getVectorVT(*DAG.getContext(), EltT, 64);

    EVT cvt = Cond.getValueType();
    SDValue cond1, cond2;
    if (cvt.isVector()) {
      llvm_unreachable("vector cond is not implemented yet");
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
  } else {
    SDValue SecondMov = DAG.getNode(TPCISD::COND_MOV_INVERT, SDLoc(Op),
                                    Op1->getVTList(), Op2, Cond, Op1);
    return SecondMov;
  }
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
  assert(SubRegTy.getVectorNumElements() == SubVectorSize);
  assert(WideRegTy.getVectorNumElements() == NumElements);
  assert(SubRegTy.getVectorElementType() == WideRegTy.getVectorElementType());

  auto Cst = cast<ConstantSDNode>(Op.getOperand(1));
  unsigned Offset = Cst->getZExtValue();
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

static SDValue helperConvertIntNodeCreate(const SDValue &SDNodeArg, SDLoc &DL,
                                          SelectionDAG &DAG,
                                          unsigned switchType, EVT ResultVT,
                                          unsigned ConvertType,
                                          SDValue *SDNodeArg2 = nullptr) {
  SmallVector<SDValue, 8> Ops(6);
  Ops[0] = SDNodeArg; // Source.
  Ops[1] = DAG.getTargetConstant(0, DL, MVT::i8);
  Ops[2] = DAG.getTargetConstant(switchType, DL, MVT::i32); // Switch.
  Ops[3] =
      (SDNodeArg2 == nullptr) ? DAG.getUNDEF(ResultVT) : *SDNodeArg2; // Income.
  Ops[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(ConvertType, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}

SDValue helperConvertvvpNodeCreate(const SDValue &SDNodeArg, SDLoc &DL,
                                   SelectionDAG &DAG, unsigned opType,
                                   unsigned switchType, EVT ResultVT) {
  SmallVector<SDValue, 8> Ops(6);
  Ops[0] = SDNodeArg;                                       // Source.
  Ops[1] = DAG.getTargetConstant(opType, DL, MVT::i8);      // DataType.
  Ops[2] = DAG.getTargetConstant(switchType, DL, MVT::i32); // Switch.
  Ops[3] = DAG.getUNDEF(ResultVT);                          // Income.
  Ops[4] = DAG.getRegister(TPC::SP0, MVT::i1);              // Predicate.
  Ops[5] = DAG.getTargetConstant(0, DL, MVT::i1);           // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}

SDValue helperANDvipNodeCreate(const SDValue &SDNodeArg, SDLoc &DL,
                               SelectionDAG &DAG, unsigned opType,
                               unsigned mask, EVT ResultVT) {
  SmallVector<SDValue, 8> Ops(7);
  Ops[0] = SDNodeArg;                                  // Source.
  Ops[1] = DAG.getTargetConstant(mask, DL, MVT::i32);  // Switch.
  Ops[2] = DAG.getTargetConstant(opType, DL, MVT::i8); // DataType.
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);     // Switch.
  Ops[4] = DAG.getUNDEF(ResultVT);                     // Income.
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);         // Predicate.
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);      // Polarity.
  MachineSDNode *Node = DAG.getMachineNode(TPC::ANDvip, DL, ResultVT, Ops);
  return SDValue(Node, 0);
}

SDValue helperExtractSubRegSDValue(unsigned subReg, SDLoc &DL,
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
  llvm_unreachable((Op.dump(), "Unhandled shufflevector"));
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

SDValue TPCTargetLowering::lowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  LoadSDNode *LoadNode = cast<LoadSDNode>(Op);
  EVT MemVT = LoadNode->getMemoryVT();

  if (!MemVT.isVector())
    return SDValue();

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
    ResTy = EVT::getVectorVT(*DAG.getContext(), RegTy, ElementCount(2, false));
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

SDValue TPCTargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
                                                   SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "TPC intrinsic custom lowering:\n"; Op.dump(&DAG););

  unsigned IntrinNum = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();

  switch (IntrinNum) {
  default:
    return SDValue();
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
  
  case Intrinsic::tpc_st_tnsr_sqz:
  case Intrinsic::tpc_st_tnsr_sqz_rmw:
    Info.opc = ISD::INTRINSIC_W_CHAIN;
    Info.memVT = MVT::getVT(I.getArgOperand(3)->getType());
    Info.offset = 0;
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

SmallVector<SDValue, 4>
TPCTargetLowering::extend_8_to_32(SDValue Op, SelectionDAG &DAG,
                                  const SDLoc &DL, uint64_t DataType) const {
  EVT ResultVT(MVT::v64i32);
  const SDValue &Src = Op.getOperand(0);
  auto SrcType = Src.getValueType();
  SmallVector<SDValue, 4> ExtArg;

  if (Subtarget->hasGaudiISA()) {
    // unpack-1
    SmallVector<SDValue, 8> Ops1(6);
    Ops1[0] = Src;                                           // Source.
    Ops1[1] = DAG.getTargetConstant(DataType, DL, MVT::i32); // DataType.
    // y.v1 = (int64)v_i8_unpack_v(x, 0/*x00*/, e_group_0,
    // e_every_forth_element, e_lower_half_group);
    Ops1[2] =
        DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
    Ops1[3] = DAG.getConstant(0, DL, ResultVT);                  // Income.
    Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);                // Predicate.
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
    Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops3[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops5[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops6[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops7[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops8[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops9[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops10[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops11[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops12[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);                // Predicate.
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
    Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops3[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops5[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops6[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops7[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops8[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops9[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops10[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops11[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops12[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops13[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops14[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops15[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops16[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops17[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops18[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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

SmallVector<SDValue, 2>
TPCTargetLowering::extend_8_to_16(SDValue Op, SelectionDAG &DAG,
                                  const SDLoc &DL, uint64_t DataType,
                                  unsigned DstDataType) const {
  const SDValue &Src = Op.getOperand(0);
  EVT SrcVT = Src->getValueType(0);
  EVT ResultVT(MVT::v128i16);
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
  Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops3[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node4 =
      DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultVT, Ops4);
  SDValue convert2 = SDValue(Node4, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops5(6);
  Ops5[0] = convert2;                                      // Source.
  Ops5[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 0, 1, 1, 1);
  Ops5[2] = DAG.getTargetConstant((0 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   1 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops5[3] = convert1;                              // Income.
  Ops5[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops5[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node5 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops5);
  SDValue mdg1 = SDValue(Node5, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops6(6);
  Ops6[0] = convert1;                                      // Source.
  Ops6[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v1, 1, 2, 1, 1);
  Ops6[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops6[3] = mdg1;                                  // Income.
  Ops6[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops6[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node6 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops6);
  SDValue mdg2 = SDValue(Node6, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops7(6);
  Ops7[0] = convert2;                                      // Source.
  Ops7[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v1 = v_i16_mov_dual_group_v(y.v2, 0xFFFFFFFF, y.v1, 1, 3, 1, 1);
  Ops7[2] = DAG.getTargetConstant((1 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   3 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops7[3] = mdg2;                                  // Income.
  Ops7[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops7[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node7 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops7);
  SDValue mdg3 = SDValue(Node7, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops8(6);
  Ops8[0] = convert1;                                      // Source.
  Ops8[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 2, 0, 1, 1);
  Ops8[2] = DAG.getTargetConstant((2 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                   0 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                   TPCII::SW_WR_LOWER_GROUP |
                                   TPCII::SW_WR_UPPER_GROUP),
                                  DL, MVT::i32);   // Switch.
  Ops8[3] = convert2;                              // Income.
  Ops8[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops9[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops9[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node9 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops9);
  SDValue mdg5 = SDValue(Node9, 0);

  // mov-dual-group
  SmallVector<SDValue, 8> Ops10(6);
  Ops10[0] = convert1;                                      // Source.
  Ops10[1] = DAG.getTargetConstant(-1, DL, MVT::i32);       // DataType.
  // y.v2 = v_i16_mov_dual_group_v(vec0, 0xFFFFFFFF, y.v2, 3, 2, 1, 1);
  Ops10[2] = DAG.getTargetConstant((3 << TPCII::SW_SRC_DUAL_GROUP_SHIFT |
                                    2 << TPCII::SW_DST_DUAL_GROUP_SHIFT |
                                    TPCII::SW_WR_LOWER_GROUP |
                                    TPCII::SW_WR_UPPER_GROUP),
                                   DL, MVT::i32);   // Switch.
  Ops10[3] = mdg5;                                  // Income.
  Ops10[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    llvm_unreachable("Unsupported source type");
  }
  if (Subtarget->hasGaudiISA()) {
    // mov-group
    SmallVector<SDValue, 8> Ops(6);
    Ops[0] = Temp;                                    // Source.
    Ops[1] = DAG.getTargetConstant(-1, DL, MVT::i32); // DataType.
    Ops[2] =
        DAG.getTargetConstant((TPCII::SW_GROUP_EN | TPCII::SW_DUAL_GROUP_EN),
                              DL, MVT::i32);        // Switch.
    Ops[3] = DAG.getUNDEF(MVT::v128i16);            // Income.
    Ops[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops1[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops2[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops3[4] = DAG.getRegister(TPC::SP0, MVT::i1);            // Predicate.
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
    Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 =
        DAG.getMachineNode(TPC::UNPACKp, DL, MVT::v128i16, Ops4);
    SDValue mdg4 = SDValue(Node4, 0);

    if (SrcType == MVT::v128bf16) {
      mdg3 = DAG.getNode(ISD::BITCAST, DL, MVT::v128bf16, mdg3);
      mdg4 = DAG.getNode(ISD::BITCAST, DL, MVT::v128bf16, mdg4);
    } else if (SrcType == MVT::v128f16) {
      llvm_unreachable("Unsupported source type");
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
    Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);            // Predicate.
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
    Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops5[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops6[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops7[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops8[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops9[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
      llvm_unreachable("Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i16) {
    if (Src.getValueType() == MVT::v256i8) {
      auto ExtArg = extend_8_to_16(Op, DAG, DL, TPCII::OpType::INT8,
                                   TPCII::SW_TO_INT16);
      return createTuple(ExtArg, DAG);
    } else {
      llvm_unreachable("Unsupported source type");
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
      llvm_unreachable("Unsupported source type");
    }
  } else {
    llvm_unreachable("Unsupported target type");
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
      llvm_unreachable("Unsupported source type");
    }
  } else if (Op.getValueType() == MVT::v256i16) {
    if (Src.getValueType() == MVT::v256i8) {
      // TODO : adapt the helper once generalized for not using AND/CONVERT.
      auto ExtArg = extend_8_to_16(Op, DAG, DL, TPCII::OpType::UINT8,
                                   TPCII::SW_TO_INT16);
      return createTuple(ExtArg, DAG);
    } else {
      llvm_unreachable("Unsupported source type");
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
      llvm_unreachable("Unsupported source type");
    }
  } else {
    llvm_unreachable("Unsupported source type");
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
  if ((Op.getValueType() == MVT::v128bf16) &&
      (Subtarget->hasGaudiISA())) {
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

      if (Subtarget->hasGaudiISA())
        return truncate_32_to_16(DAG, Src0, Src1, DL, MVT::v128bf16,
                                 TPCII::OpType::INT16);
      else
        return truncate_32_to_16_goya(DAG, Src0, Src1, DL, MVT::v128bf16,
                                      TPCII::OpType::INT16);
    } else {
      llvm_unreachable("Unsupported target type");
    }
  } else if (Op.getValueType() == MVT::v256bf16) {
    if (Src.getValueType() == MVT::v256f32) {
      if (Subtarget->hasGaudiISA()) {
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
        if (Subtarget->hasGaudiISA()) {
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
        llvm_unreachable("Unsupported vector type");
      }
    } else {
      llvm_unreachable("Unsupported source type");
    }
  } else {
    llvm_unreachable("Unsupported target type");
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
  Ops[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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

  if (Op.getValueType() == MVT::v128f32) {
    if (Src.getValueType() == MVT::v128bf16)
      return lowerVectorFP_Extend(Op, DAG, DL, TPCII::OpType::BF16,
                                  MVT::v64f32, InputSwitch,
                                  IsCastIntrinsicWithRoundMode);
    else
      llvm_unreachable("Unsupported source type");
  } else if (Op.getValueType() == MVT::v256f32) {
    if (Src.getValueType() == MVT::v256bf16) {
      if (Subtarget->hasGaudiISA()) {
        SDValue S0Val =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v128bf16, Src);
        SDValue S1Val =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v128bf16, Src);

        SDValue Ext0 = DAG.getNode(ISD::FP_EXTEND, DL, MVT::v128f32, S0Val);
        SDValue Ext1 = DAG.getNode(ISD::FP_EXTEND, DL, MVT::v128f32, S1Val);

        SDValue Ext00 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Ext0);
        SDValue Ext01 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Ext0);
        SDValue Ext10 =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Ext1);
        SDValue Ext11 =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Ext1);

        return createTuple({Ext00, Ext01, Ext10, Ext11}, DAG);
      } else {
        llvm_unreachable("Unsupported target type");
      }
    } else {
      llvm_unreachable("Unsupported source type");
    }
  }
  return SDValue();
}

// Aliases for type constants, to have more convenient names.
constexpr unsigned T_FP32    = TPCII::OpType::FP32;
constexpr unsigned T_BF16    = TPCII::OpType::BF16;
constexpr unsigned T_FP16    = TPCII::OpType::FP16;
constexpr unsigned T_FP8_143 = TPCII::OpType::FP8_143;
constexpr unsigned T_FP8_152 = TPCII::OpType::FP8_152;
constexpr unsigned T_INT32   = TPCII::OpType::INT32;
constexpr unsigned T_UINT32  = TPCII::OpType::UINT32;
constexpr unsigned T_INT16   = TPCII::OpType::INT16;
constexpr unsigned T_UINT16  = TPCII::OpType::UINT16;
constexpr unsigned T_INT8    = TPCII::OpType::INT8;
constexpr unsigned T_UINT8   = TPCII::OpType::UINT8;
constexpr unsigned T_INT4    = TPCII::OpType::INT4;
constexpr unsigned T_UINT4   = TPCII::OpType::UINT4;
constexpr unsigned T_UNDEF   = ~0U;

// Given switch value, as specified for CONVERT intrinsics, determines target
// type of the conversion.
static unsigned findDestTypeInSwitches(unsigned Switches) {
  unsigned Masked = Switches & TPCII::SW_TO_TYPE;
  switch (Masked) {
  default:
    llvm_unreachable("Unknown target type");
  case TPCII::SW_TO_FP32: return T_FP32;
  case TPCII::SW_TO_BF16: return T_BF16;
  case TPCII::SW_TO_FP16: return T_FP16;
  case TPCII::SW_TO_FP8_143: return T_FP8_143;
  case TPCII::SW_TO_FP8_152: return T_FP8_152;
  case TPCII::SW_TO_INT32: return T_INT32;
  case TPCII::SW_TO_UINT32: return T_UINT32;
  case TPCII::SW_TO_INT16: return T_INT16;
  case TPCII::SW_TO_UINT16: return T_UINT16;
  case TPCII::SW_TO_INT8: return T_INT8;
  case TPCII::SW_TO_UINT8: return T_UINT8;
  }
}

// Target feature constants.
constexpr unsigned F_ALL = 0;
constexpr unsigned F_GEN1 = 1;
constexpr unsigned F_GEN2 = 2;
constexpr unsigned F_GEN2PLUS = F_GEN2;

// Check if the target feature is supported by the subtarget.
static bool hasFeature(const TPCSubtarget &Subtarget, unsigned F) {
  if (F == F_ALL)
    return true;
  if (F & F_GEN1)
    return Subtarget.hasGoyaISA();
  if (F & F_GEN2)
    return Subtarget.hasGaudiISA();
  return false;
}

/// Keeps source and destination types of a conversion.
///
/// Types are represented by the enum TPCII::OpType.
///
struct CvtDirection {
  unsigned SourceType = T_UNDEF;
  unsigned DestinationType = T_UNDEF;
};

template<> struct llvm::DenseMapInfo<CvtDirection> {
  static inline CvtDirection getEmptyKey() { return CvtDirection(); }
  static inline CvtDirection getTombstoneKey() { return { T_UNDEF, 0 }; }
  static unsigned getHashValue(const CvtDirection &Val) {
    return (Val.SourceType ^ Val.DestinationType) * 37U;
  }
  static bool isEqual(const CvtDirection &LHS, const CvtDirection &RHS) {
    return LHS.SourceType == RHS.SourceType &&
           LHS.DestinationType == RHS.DestinationType;
  }
};

/// Describes single conversion step.
///
/// Corresponds to single instruction CONVERT.
///
struct CvtItem {
  unsigned DataType = T_UNDEF;
  unsigned Switches = 0;
  unsigned SrcNumElements = 0;
  unsigned DestNumElements = 0;
};

/// Describes sequence of conversion steps implementing required conversion.
///
struct CvtActions {
  unsigned Requires = F_ALL;  // Required feature
  bool IsSequence = false;
  SmallVector<CvtItem, 4> Steps;
};

struct Conversion {
  SmallVector<CvtActions, 4> Actions;
};

static const DenseMap<CvtDirection, Conversion> ConversionDB{
  { {T_INT8, T_FP32}, {} }
};

static unsigned getMultiplicity(EVT VT) {
  unsigned TypeSize = VT.getSizeInBits();
  switch (TypeSize) {
  case 256*8: return 1;
  case 512*8: return 2;
  case 1024*8: return 4;
  }
  llvm_unreachable("Invalid type multiplicity");
}

static unsigned getConvertOpCode(const CvtActions &Cvt, EVT DestVT, EVT SrcVT,
                                 bool VectorPredicate) {
  unsigned SrcMultiplicity = getMultiplicity(SrcVT);
  unsigned DestMultiplicity = getMultiplicity(DestVT);
  unsigned CommonMultiplicity = std::min(SrcMultiplicity, DestMultiplicity);
  unsigned SrcDataWidth = SrcMultiplicity / CommonMultiplicity;
  unsigned DestDataWidth = DestMultiplicity / CommonMultiplicity;
  assert(SrcMultiplicity % CommonMultiplicity == 0);
  assert(DestMultiplicity % CommonMultiplicity == 0);

  unsigned OpCode = 0;
  switch (SrcDataWidth) {
  default:
    llvm_unreachable("Unsupported multiplicity");
  case 1: //------ Source is in VRF
    switch (DestDataWidth) {
    default:
      llvm_unreachable("Unsupported multiplicity");
    case 1:
      OpCode = VectorPredicate ? TPC::CONVERTvvm : TPC::CONVERTvvp;
      break;
    case 2:
      OpCode = VectorPredicate ? TPC::CONVERTdvm : TPC::CONVERTdvp;
      break;
    }
    break;
  case 2: //------  Source is in DRF
    switch (DestDataWidth) {
    default:
      llvm_unreachable("Unsupported multiplicity");
    case 1:
      OpCode = VectorPredicate ? TPC::CONVERTvdm : TPC::CONVERTvdp;
      break;
    }
    break;
  }
  return OpCode;
}

SDValue TPCTargetLowering::lowerTPC_CONVERT(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);

  // Get and check arguments.

  SDValue Source = Op.getOperand(1);
  SDValue DataType = Op.getOperand(2);
  SDValue Switches = Op.getOperand(3);
  SDValue Income = Op.getOperand(4);
  SDValue Predicate = Op.getOperand(5);
  SDValue Polarity = Op.getOperand(6);
  EVT SrcVT = Source.getValueType();
  EVT DestVT = Op.getValueType();
  bool IsVectorPredicate = (Predicate.getValueType() == MVT::v256i1);

  assert(cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() == Intrinsic::tpc_convert);
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
  bool AllLanes = (SrcNumElements == DestNumElements);
  bool SameSize = (SrcVT.getSizeInBits() == DestVT.getSizeInBits());

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
    assert(DataTypeVal == T_INT32 || DataTypeVal == T_UINT32);
  } else if (SrcEltVT == MVT::i16) {
    assert(DataTypeVal == T_INT16 || DataTypeVal == T_UINT16);
  } else if (SrcEltVT == MVT::i8) {
    assert(DataTypeVal == T_INT8 || DataTypeVal == T_UINT8 ||
           DataTypeVal == T_INT4 || DataTypeVal == T_UINT4);
  }

  unsigned DestTypeSwitch = SwitchVal & TPCII::SW_TO_TYPE;
  assert(DestEltVT != MVT::f32 || DestTypeSwitch == TPCII::SW_TO_FP32);
  assert(DestEltVT != MVT::bf16 || DestTypeSwitch == TPCII::SW_TO_BF16);
  assert(DestEltVT != MVT::f16 || DestTypeSwitch == TPCII::SW_TO_FP16);
  assert(DestEltVT != MVT::f8_143 || DestTypeSwitch == TPCII::SW_TO_FP8_143);
  assert(DestEltVT != MVT::f8_152 || DestTypeSwitch == TPCII::SW_TO_FP8_152);
  assert(DestEltVT != MVT::i32 ||
         (DestTypeSwitch == TPCII::SW_TO_INT32 || DestTypeSwitch == TPCII::SW_TO_UINT32));
  assert(DestEltVT != MVT::i16 ||
         (DestTypeSwitch == TPCII::SW_TO_INT16 || DestTypeSwitch == TPCII::SW_TO_UINT16));
  assert(DestEltVT != MVT::i8 ||
         (DestTypeSwitch == TPCII::SW_TO_INT8 || DestTypeSwitch == TPCII::SW_TO_UINT8));

  unsigned T_Src = DataTypeVal;
  unsigned T_Dest = findDestTypeInSwitches(DestTypeSwitch);

  // Get conversion parts from database.
  auto ItemPtr = ConversionDB.find({T_Src, T_Dest});
  if (ItemPtr == ConversionDB.end())
    return SDValue();
  const Conversion &Cvt = ItemPtr->getSecond();
  const CvtActions *Choice = nullptr;
  for (const auto &Action : Cvt.Actions) {
    if (hasFeature(*Subtarget, Action.Requires)) {
      Choice = &Action;
      break;
    }
  }
  if (!Choice)
    return SDValue();

  unsigned OpCode = getConvertOpCode(*Choice, DestVT, SrcVT, IsVectorPredicate);
  if (OpCode == 0)
    return SDValue();

  SmallVector<MachineSDNode *, 4> CvtNodes;
  for (const auto &Part : Choice->Steps) {
    unsigned DT = Part.DataType;
    if (DT == T_UNDEF)
      DT = DataTypeVal;

    unsigned SW = Part.Switches;
    if (SW == 0)
      SW = DestTypeSwitch;
    if (AllLanes) {
      SW &= ~TPCII::SW_NUM_LANES;
      SW |= TPCII::SW_ALL_LANES;
    }

    SmallVector<SDValue, 8> Operands;
    Operands.push_back(Source);
    Operands.push_back(DAG.getTargetConstant(DataTypeVal, DL, MVT::i8));
    Operands.push_back(DAG.getTargetConstant(SwitchVal, DL, MVT::i32));
    Operands.push_back(Income);
    Operands.push_back(Predicate);
    Operands.push_back(DAG.getTargetConstant(PolarityVal, DL, MVT::i1));
    MachineSDNode *Node = DAG.getMachineNode(OpCode, DL, DestVT, Operands);
    CvtNodes.push_back(Node);
  }

  if (CvtNodes.empty())
    return SDValue();
  if (CvtNodes.size() == 1)
    return SDValue(CvtNodes.front(), 0);
  return SDValue();
}

SDValue TPCTargetLowering::truncate_32_to_16_goya(SelectionDAG &DAG,
                                                  SDValue Src0, SDValue Src1,
                                                  SDLoc &DL, EVT ResultVT,
                                                  uint64_t dataType) const {
  // y = v_i16_pack_v(x0, y, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops11[4] = DAG.getRegister(TPC::SP0, MVT::i1);           // Predicate.
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
  Ops12[4] = DAG.getRegister(TPC::SP0, MVT::i1);                   // Predicate.
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
  Ops13[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops14[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops15[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops21[4] = DAG.getRegister(TPC::SP0, MVT::i1);           // Predicate.
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
  Ops22[4] = DAG.getRegister(TPC::SP0, MVT::i1);                   // Predicate.
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
  Ops23[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops24[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops25[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops26[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops26[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node26 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUPp, DL, ResultVT, Ops26);
  SDValue val26 = SDValue(Node26, 0);

  return val26;
}

SDValue TPCTargetLowering::truncate_32_to_16(SelectionDAG &DAG, SDValue Src0,
                                             SDValue Src1, SDLoc &DL,
                                             EVT ResultVT,
                                             uint64_t dataType) const {
  // y = v_i16_pack_v(x0, y, e_group_0, e_every_second_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(0, DL, MVT::i32);       // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                       // Income.
  Ops11[4] = DAG.getRegister(TPC::SP0, MVT::i1);           // Predicate.
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
  Ops12[4] = DAG.getRegister(TPC::SP0, MVT::i1);                   // Predicate.
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
  Ops13[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops14[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops21[4] = DAG.getRegister(TPC::SP0, MVT::i1);           // Predicate.
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
  Ops22[4] = DAG.getRegister(TPC::SP0, MVT::i1);                   // Predicate.
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
  Ops23[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops24[5] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
  Ops24[6] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
  MachineSDNode *Node24 =
      DAG.getMachineNode(TPC::MOV_DUAL_GROUP_ALLp, DL, ResultVT, Ops24);
  SDValue val24 = SDValue(Node24, 0);

  return val24;
}

SDValue TPCTargetLowering::truncate_16_to_8(SelectionDAG &DAG, SDValue Src0,
                                            SDValue Src1, SDLoc &DL,
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
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
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
    Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
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
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  MachineSDNode *MulLSNode = DAG.getMachineNode(TPC::MULsip, DL, MVT::i32, Ops);
  MulLS = SDValue(MulLSNode, 0);

  // Shift of the most signficant part.
  Ops[0] = MulMS;
  Ops[1] = DAG.getTargetConstant(ShiftVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  SDValue OpShiftMS(DAG.getMachineNode(TPC::SHLsip, DL, MVT::i32, Ops), 0);

  // Shift of the lest signficant part.
  Ops[0] = MulLS;
  Ops[1] = DAG.getTargetConstant(ShiftVal, DL, MVT::i32);
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  SDValue OpShiftLS(DAG.getMachineNode(TPC::SHRsip, DL, MVT::i32, Ops), 0);

  // Union of two shifted words.
  Ops[0] = OpShiftMS;
  Ops[1] = OpShiftLS;
  Ops[2] = DAG.getTargetConstant(TPCII::OpType::UINT32, DL, MVT::i8);
  Ops[3] = DAG.getTargetConstant(0, DL, MVT::i32);
  Ops[4] = DAG.getUNDEF(MVT::i32);
  Ops[5] = DAG.getRegister(TPC::SP0, MVT::i1);
  Ops[6] = DAG.getTargetConstant(0, DL, MVT::i1);
  return SDValue(DAG.getMachineNode(TPC::ORssp, DL, MVT::i32, Ops), 0);
}

SDValue TPCTargetLowering::truncate_32_to_8(SelectionDAG &DAG, SDValue Src0,
                                            SDValue Src1, SDValue Src2,
                                            SDValue Src3, SDLoc &DL,
                                            EVT ResultVT,
                                            uint64_t dataType) const {
  // y = v_i8_pack_v(x0, y, e_group_0, e_every_forth_element);
  SmallVector<SDValue, 8> Ops11(6);
  Ops11[0] = Src0;                                         // Source.
  Ops11[1] = DAG.getTargetConstant(dataType, DL, MVT::i8); // DataType.
  Ops11[2] = DAG.getTargetConstant(TPCII::SW_STRIDE_4, DL, MVT::i32); // Switch.
  Ops11[3] = DAG.getUNDEF(ResultVT);                                  // Income.
  Ops11[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops12[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops13[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops14[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops15[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops21[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops22[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops23[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops24[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops25[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops26[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops31[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops32[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops33[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops34[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops35[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops36[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops41[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops42[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops43[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops44[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops45[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  Ops46[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
  bool cast32to16 = false, cast16to8 = false, cast32to8 = false;
  enum TPCII::OpType targetType;
  enum TPCII::OpType sourceType;
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
    Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
    Ops2[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.

    MachineSDNode *Node2 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops2);
    SDValue Src1 = SDValue(Node2, 0);
    if (cast32to16) {
      return (Subtarget->hasGaudiISA())
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
    Ops1[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops2[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops3[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
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
    Ops4[4] = DAG.getRegister(TPC::SP0, MVT::i1);    // Predicate.
    Ops4[5] = DAG.getTargetConstant(0, DL, MVT::i1); // Polarity.
    MachineSDNode *Node4 =
        DAG.getMachineNode(TPC::CONVERTvvp, DL, ResultType, Ops4);
    SDValue Src3 = SDValue(Node4, 0);

    return truncate_32_to_8(DAG, Src0, Src1, Src2, Src3, DL, ResultVT,
                            targetType);
  } else {
    llvm_unreachable("Unsupported source and target type");
  }
  return SDValue();
}

SDValue TPCTargetLowering::helperCONVERTSIGNED(EVT OpType, 
                                  SDValue Src, unsigned InputSwitch, 
                                  bool IsCastIntrinsicWithRoundMode,
                                  SelectionDAG& DAG) const {
  SDLoc DL(Src);
  EVT SrcType = Src.getValueType();

  if (Src.getValueType() == MVT::v128i16) {
    if (OpType == MVT::v128f32) {

      SDValue Ext = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v128i32, Src);

      // cast to INT32 to FP32
      SDValue N0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext);
      SDValue N1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext);

      SDValue ConvertOpNode0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG, TPCII::OpType::INT32, InputSwitch | TPCII::SW_TO_FP32,
          MVT::v64f32);
      SDValue ConvertOpNode1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG, TPCII::OpType::INT32, InputSwitch | TPCII::SW_TO_FP32,
          MVT::v64f32);

      return createTuple({ConvertOpNode0, ConvertOpNode1}, DAG);

    } else if (OpType == MVT::v128bf16) {

      return helperConvertvvpNodeCreate(Src, DL, DAG, TPCII::OpType::INT16,
                                        InputSwitch | TPCII::SW_TO_BF16,
                                        MVT::v128bf16);
    } else {
      llvm_unreachable("Unsupported source and target type");
    }
  } else if (Src.getValueType() == MVT::v256i8) {
    if (OpType == MVT::v256f16) {
      SDValue Ext = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v256i16, Src);
      return (DAG.getNode(ISD::SINT_TO_FP, DL, MVT::v256f16, Ext));

    } else if (OpType == MVT::v256bf16) {
      SDValue Ext = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v256i16, Src);
      return (DAG.getNode(ISD::SINT_TO_FP, DL, MVT::v256bf16, Ext));
    } else if (OpType == MVT::v256f32) {
      SDValue Ext = DAG.getNode(ISD::SIGN_EXTEND, DL, MVT::v256i32, Src);

      auto SubReg00 =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext);
      auto BitCast00 = DAG.getNode(ISD::BITCAST, DL, MVT::v256i8, SubReg00);
      auto SubReg01 =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext);
      auto BitCast01 = DAG.getNode(ISD::BITCAST, DL, MVT::v256i8, SubReg01);
      auto SubReg10 =
          helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, MVT::v64i32, Ext);
      auto BitCast10 = DAG.getNode(ISD::BITCAST, DL, MVT::v256i8, SubReg10);
      auto SubReg11 =
          helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, MVT::v64i32, Ext);
      auto BitCast11 = DAG.getNode(ISD::BITCAST, DL, MVT::v256i8, SubReg11);

      auto opType = TPCII::OpType::INT8;
      auto ResultVT = MVT::v64f32;
      auto ConvertNode00 = helperConvertvvpNodeCreate(
          BitCast00, DL, DAG, opType, TPCII::SW_RHNE | TPCII::SW_TO_FP32,
          ResultVT);
      auto ConvertNode01 = helperConvertvvpNodeCreate(
          BitCast01, DL, DAG, opType, TPCII::SW_RHNE | TPCII::SW_TO_FP32,
          ResultVT);
      auto ConvertNode10 = helperConvertvvpNodeCreate(
          BitCast10, DL, DAG, opType, TPCII::SW_RHNE | TPCII::SW_TO_FP32,
          ResultVT);
      auto ConvertNode11 = helperConvertvvpNodeCreate(
          BitCast11, DL, DAG, opType, TPCII::SW_RHNE | TPCII::SW_TO_FP32,
          ResultVT);
      return createTuple(
          {ConvertNode00, ConvertNode01, ConvertNode10, ConvertNode11}, DAG);
    }
  }
  if (OpType == MVT::v128i16) {
    if (SrcType == MVT::v128f32) {
      if (IsCastIntrinsicWithRoundMode) {
        enum TPCII::OpType TargetType = TPCII::OpType::INT16;
        SDValue N0Val =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
        SDValue N1Val =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);

        SDValue Src0 = helperConvertvvpNodeCreate(
            N0Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_INT32, MVT::v64i32);
        SDValue Src1 = helperConvertvvpNodeCreate(
            N1Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_INT32, MVT::v64i32);

        // CONVERT to Int: 64xi32 To 128xi16
        SDValue Convert0 = helperConvertIntNodeCreate(
            Src0, DL, DAG, InputSwitch | TPCII::SW_TO_INT16, MVT::v128i16,
            TPC::CONVERT_INT16vvp);
        SDValue Convert1 = helperConvertIntNodeCreate(
            Src1, DL, DAG, InputSwitch | TPCII::SW_TO_INT16, MVT::v128i16,
            TPC::CONVERT_INT16vvp);

        return truncate_32_to_16(DAG, Convert0, Convert1, DL, OpType,
                                 TargetType);
      } else {
        SDValue CastFP32ToSINT32 =
            DAG.getNode(ISD::FP_TO_SINT, DL, MVT::v128i32, Src);
        return IsCastIntrinsicWithRoundMode
                   ? lowerTRUNCATE(DAG.getNode(ISD::TRUNCATE, DL, MVT::v128i16,
                                               CastFP32ToSINT32),
                                   DAG, InputSwitch,
                                   IsCastIntrinsicWithRoundMode)
                   : (DAG.getNode(ISD::TRUNCATE, DL, MVT::v128i16,
                                  CastFP32ToSINT32));
      }
    } else if (SrcType == MVT::v128bf16) {
      return helperConvertvvpNodeCreate(Src, DL, DAG, TPCII::OpType::BF16,
                                        InputSwitch | TPCII::SW_TO_INT16,
                                        MVT::v128i16);
    } else {
      llvm_unreachable("Unsupported source and target type");
    }
  }

  if (((OpType == MVT::v256i8) && (SrcType == MVT::v256f16)) ||
      ((OpType == MVT::v256i8) && (SrcType == MVT::v256bf16))) {
    if (IsCastIntrinsicWithRoundMode) {
      LLVM_DEBUG(
          dbgs() << "Truncate SIGNED intrinsic: Get switch from argument: "
                 << InputSwitch << "\n");

      enum TPCII::OpType TargetType = TPCII::OpType::INT8;
      // SUBREG0: 256xBF16|FP16 To 128xBF16|FP16
      SDValue N0Val = helperExtractSubRegSDValue(
          TPC::sub_0, DL, DAG,
          SrcType == MVT::v256bf16 ? MVT::v128bf16 : MVT::v128f16, Src);
      // SUBREG1: 256xBF16|FP16 To 128xBF16|FP16
      SDValue N1Val = helperExtractSubRegSDValue(
          TPC::sub_1, DL, DAG,
          SrcType == MVT::v256bf16 ? MVT::v128bf16 : MVT::v128f16, Src);

      // CONVERT1: 128xBF16|FP16 To 128xi16
      SDValue Src0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG,
          SrcType == MVT::v256bf16 ? TPCII::OpType::BF16 : TPCII::OpType::FP16,
          InputSwitch | TPCII::SW_TO_INT16, MVT::v128i16);
      // CONVERT2: 128xBF16|FP16 To 128xi16
      SDValue Src1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG,
          SrcType == MVT::v256bf16 ? TPCII::OpType::BF16 : TPCII::OpType::FP16,
          InputSwitch | TPCII::SW_TO_INT16, MVT::v128i16);

      // CONVERT11: 128xi16 To 256xi8
      SDValue Src00 = helperConvertIntNodeCreate(
          Src0, DL, DAG, InputSwitch | TPCII::SW_TO_INT8, MVT::v256i8,
          getTargetConvertType(*Subtarget));
      // CONVERT22: 128xi16 To 256xi8
      SDValue Src11 = helperConvertIntNodeCreate(
          Src1, DL, DAG, InputSwitch | TPCII::SW_TO_INT8, MVT::v256i8,
          getTargetConvertType(*Subtarget));

      // This name is somewhat misleading however it linearizes using sequence
      // of PACK and MOV_DUAL_GROUP instructions.
      return truncate_32_to_16(DAG, Src00, Src11, DL, OpType, TargetType);
    } else {
      SDValue CastF16ToSINT16 =
          DAG.getNode(ISD::FP_TO_SINT, DL, MVT::v256i16,
                      Src); // Eliminates the Gen3 additional check

      return (DAG.getNode(ISD::TRUNCATE, DL, MVT::v256i8, CastF16ToSINT16));
    }
  } else if ((OpType == MVT::v256i8) && (SrcType == MVT::v256f32)) {
#if 0 // Commented to resolve a bug temporarily
    // Convert from FP32 --> INT32
    SDValue CastFP32ToSINT32 =
        DAG.getNode(ISD::FP_TO_SINT, DL, MVT::v256i32, Src);

    return IsCastIntrinsicWithRoundMode
               ? lowerTRUNCATE(DAG.getNode(ISD::TRUNCATE, DL, MVT::v256i8,
                                           CastFP32ToSINT32),
                               DAG, InputSwitch, IsCastIntrinsicWithRoundMode)
               : (DAG.getNode(ISD::TRUNCATE, DL, MVT::v256i8,
                              CastFP32ToSINT32));
#else
    // TODO: This instruction sequence directly converts v256f32 to v256i8
    // without having to convert to an intermediate v256i32. This should be
    // enabled once the hardware workaround for converts is disabled.

    SDValue N0Val =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
    SDValue N1Val =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);
    SDValue N2Val =
        helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, MVT::v64f32, Src);
    SDValue N3Val =
        helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, MVT::v64f32, Src);

    SDValue Src0 = helperConvertvvpNodeCreate(
        N0Val, DL, DAG, TPCII::OpType::FP32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_INT8,
        MVT::v256i8);
    SDValue Src1 = helperConvertvvpNodeCreate(
        N1Val, DL, DAG, TPCII::OpType::FP32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_INT8,
        MVT::v256i8);
    SDValue Src2 = helperConvertvvpNodeCreate(
        N2Val, DL, DAG, TPCII::OpType::FP32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_INT8,
        MVT::v256i8);
    SDValue Src3 = helperConvertvvpNodeCreate(
        N3Val, DL, DAG, TPCII::OpType::FP32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_INT8,
        MVT::v256i8);

    return truncate_32_to_8(DAG, Src0, Src1, Src2, Src3, DL, MVT::v256i8,
                            TPCII::OpType::INT8);
#endif
  } else if ((OpType == MVT::v64i32) && (SrcType == MVT::v64f32)) {
    return helperConvertvvpNodeCreate(
        Src, DL, DAG, TPCII::OpType::FP32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_INT32,
        MVT::v64i32);
  } else if ((SrcType == MVT::v64i32) && (OpType == MVT::v64f32)) {
    return helperConvertvvpNodeCreate(
        Src, DL, DAG, TPCII::OpType::INT32,
        (IsCastIntrinsicWithRoundMode ? InputSwitch : TPCII::SW_RHNE) |
            TPCII::SW_TO_FP32,
        MVT::v64f32);
  } else if ((SrcType == MVT::v256i16) && (OpType == MVT::v256bf16)) {
    EVT EltTy = SrcType.getVectorElementType();
    unsigned SubNumElts = SrcType.getVectorNumElements();
    EVT SubSrcTy = EVT::getVectorVT(*DAG.getContext(), EltTy, SubNumElts / 2);
    SDValue h0 = helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, SubSrcTy, Src);
    SDValue h1 = helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, SubSrcTy, Src);
    EVT SubOpTy = EVT::getVectorVT(*DAG.getContext(), OpType.getVectorElementType(), SubNumElts / 2);

    h0 = helperCONVERTSIGNED(SubOpTy, h0, InputSwitch,
                             IsCastIntrinsicWithRoundMode,DAG);
    h1 = helperCONVERTSIGNED(SubOpTy, h1, InputSwitch,
                             IsCastIntrinsicWithRoundMode,DAG);
    SmallVector<SDValue, 4> SubVects;
    SubVects.push_back(h0);
    SubVects.push_back(h1);
    SDValue thu = createTuple(SubVects, DAG);
    return thu;
  } else {
    llvm_unreachable("Unsupported source and target type");
  }
  return SDValue();
}


SDValue TPCTargetLowering::lowerCONVERTSIGNED(SDValue Op,
                                              SelectionDAG &DAG) const {

  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_CSR;
  // If Op is intrinsic, get the switch from it.
  auto IsCastIntrinsicWithRoundMode = NumOp == 3;
  if (IsCastIntrinsicWithRoundMode) {
    InputSwitch = Op.getConstantOperandVal(NumOp - 1);
    LLVM_DEBUG(dbgs() << "lowerCONVERTSIGNED: Get switch from argument: "
                      << InputSwitch << "\n");
  }
  EVT OpType = (IsCastIntrinsicWithRoundMode) ? Op.getValueType()
                                                         : Op->getValueType(0);
  const SDValue &Src = (IsCastIntrinsicWithRoundMode)? Op.getOperand(1)
                           : Op.getOperand(0);

  EVT SrcType = Src.getValueType();
  ///////////////////////////////////////////////////////
  return helperCONVERTSIGNED(OpType, Src, InputSwitch,
                      IsCastIntrinsicWithRoundMode, DAG);
  //////////////////////////////////////////////////////////
}

SDValue TPCTargetLowering::lowerCONVERTUNSIGNED(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned int NumOp = Op.getNumOperands();
  unsigned int InputSwitch = TPCII::SW_CSR;

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

  if (Src.getValueType() == MVT::v128i16) {
    if (Op.getValueType() == MVT::v128f32) {

      SDValue Ext = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::v128i32, Src);

      SDValue N0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64i32, Ext);
      SDValue N1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64i32, Ext);

      SDValue ConvertOpNode0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG, TPCII::OpType::UINT32,
          InputSwitch | TPCII::SW_TO_FP32, MVT::v64f32);
      SDValue ConvertOpNode1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG, TPCII::OpType::UINT32,
          InputSwitch | TPCII::SW_TO_FP32, MVT::v64f32);

      return createTuple({ConvertOpNode0, ConvertOpNode1}, DAG);

    } else if (Op.getValueType() == MVT::v128bf16) {

      return helperConvertvvpNodeCreate(
          Src, DL, DAG, TPCII::OpType::UINT16,
          InputSwitch | TPCII::SW_TO_BF16, MVT::v128bf16);
    } else {
      llvm_unreachable("Unsupported source and target type");
    }
  }
  if (OpType == MVT::v128i16) {
    if (SrcType == MVT::v128f32) {
      if (IsCastIntrinsicWithRoundMode) {
        enum TPCII::OpType TargetType = TPCII::OpType::UINT16;
        SDValue N0Val =
            helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
        SDValue N1Val =
            helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);

        SDValue Src0 = helperConvertvvpNodeCreate(
            N0Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_INT32, MVT::v64i32);
        SDValue Src1 = helperConvertvvpNodeCreate(
            N1Val, DL, DAG, TPCII::OpType::FP32,
            InputSwitch | TPCII::SW_TO_INT32, MVT::v64i32);

        // CONVERT to Int: 64xi32 To 128xi16
        SDValue Convert0 = helperConvertIntNodeCreate(
            Src0, DL, DAG, InputSwitch | TPCII::SW_TO_UINT16, MVT::v128i16,
            TPC::CONVERT_INT16vvp);
        SDValue Convert1 = helperConvertIntNodeCreate(
            Src1, DL, DAG, InputSwitch | TPCII::SW_TO_UINT16, MVT::v128i16,
            TPC::CONVERT_INT16vvp);

        return truncate_32_to_16(DAG, Convert0, Convert1, DL, OpType,
                                 TargetType);
      } else {
        SDValue CastFP32ToUINT32 =
            DAG.getNode(ISD::FP_TO_UINT, DL, MVT::v128i32, Src);

        return (DAG.getNode(ISD::TRUNCATE, DL, MVT::v128i16, CastFP32ToUINT32));
      }
    } else if (SrcType == MVT::v128bf16) {
      SDValue ConvertNode = helperConvertvvpNodeCreate(
          Src, DL, DAG, TPCII::OpType::BF16,
          InputSwitch | TPCII::SW_TO_INT16, MVT::v128i16);
      return helperANDvipNodeCreate(ConvertNode, DL, DAG,
                                    TPCII::OpType::INT16, 0x0000FFFF,
                                    MVT::v128i16);

    } else {
      llvm_unreachable("Unsupported source and target type");
    }
  }

  if (((OpType == MVT::v256i8) && (SrcType == MVT::v256f16)) ||
      ((OpType == MVT::v256i8) && (SrcType == MVT::v256bf16))) {
    if (IsCastIntrinsicWithRoundMode) {
      LLVM_DEBUG(
          dbgs() << "Truncate UNSIGNED intrinsic: Get switch from argument: "
                 << InputSwitch << "\n");

      enum TPCII::OpType TargetType = TPCII::OpType::UINT8;
      // SUBREG0: 256xBF16|FP16 To 128xBF16|FP16
      SDValue N0Val = helperExtractSubRegSDValue(
          TPC::sub_0, DL, DAG,
          SrcType == MVT::v256bf16 ? MVT::v128bf16 : MVT::v128f16, Src);
      // SUBREG1: 256xBF16|FP16 To 128xBF16|FP16
      SDValue N1Val = helperExtractSubRegSDValue(
          TPC::sub_1, DL, DAG,
          SrcType == MVT::v256bf16 ? MVT::v128bf16 : MVT::v128f16, Src);

      // CONVERT1: 128xBF16|FP16 To 128xi16
      SDValue Src0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG,
          SrcType == MVT::v256bf16 ? TPCII::OpType::BF16 : TPCII::OpType::FP16,
          InputSwitch | TPCII::SW_TO_UINT16, MVT::v128i16);
      // CONVERT2: 128xBF16|FP16 To 128xi16
      SDValue Src1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG,
          SrcType == MVT::v256bf16 ? TPCII::OpType::BF16 : TPCII::OpType::FP16,
          InputSwitch | TPCII::SW_TO_UINT16, MVT::v128i16);

      // CONVERT11: 128xi16 To 256xi8
      SDValue Src00 = helperConvertIntNodeCreate(
          Src0, DL, DAG, InputSwitch | TPCII::SW_TO_UINT8, MVT::v256i8,
          TPC::CONVERT_INT16vvp);
      // CONVERT22: 128xi16 To 256xi8
      SDValue Src11 = helperConvertIntNodeCreate(
          Src1, DL, DAG, InputSwitch | TPCII::SW_TO_UINT8, MVT::v256i8,
          TPC::CONVERT_INT16vvp);

      // This name is somewhat misleading however it linearizes using sequence
      // of PACK and MOV_DUAL_GROUP instructions.
      return truncate_32_to_16(DAG, Src00, Src11, DL, OpType, TargetType);
    } else {
      SDValue CastF16ToUINT16 =
          DAG.getNode(ISD::FP_TO_UINT, DL, MVT::v256i16, Src);
      return (DAG.getNode(ISD::TRUNCATE, DL, MVT::v256i8, CastF16ToUINT16));
    }
  }

  if ((OpType == MVT::v256i8) && (SrcType == MVT::v256f32)) {
    // Convert from FP32 --> INT32
    if (IsCastIntrinsicWithRoundMode) {
      SDValue N0Val =
          helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, MVT::v64f32, Src);
      SDValue N1Val =
          helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, MVT::v64f32, Src);
      SDValue N2Val =
          helperExtractSubRegSDValue(TPC::sub_2, DL, DAG, MVT::v64f32, Src);
      SDValue N3Val =
          helperExtractSubRegSDValue(TPC::sub_3, DL, DAG, MVT::v64f32, Src);

      SDValue Src0 = helperConvertvvpNodeCreate(
          N0Val, DL, DAG, TPCII::OpType::FP32, InputSwitch | TPCII::SW_TO_UINT8,
          MVT::v256i8);
      SDValue Src1 = helperConvertvvpNodeCreate(
          N1Val, DL, DAG, TPCII::OpType::FP32, InputSwitch | TPCII::SW_TO_UINT8,
          MVT::v256i8);
      SDValue Src2 = helperConvertvvpNodeCreate(
          N2Val, DL, DAG, TPCII::OpType::FP32, InputSwitch | TPCII::SW_TO_UINT8,
          MVT::v256i8);
      SDValue Src3 = helperConvertvvpNodeCreate(
          N3Val, DL, DAG, TPCII::OpType::FP32, InputSwitch | TPCII::SW_TO_UINT8,
          MVT::v256i8);

      return truncate_32_to_8(DAG, Src0, Src1, Src2, Src3, DL, MVT::v256i8,
                              TPCII::OpType::UINT8);
    } else {
      SDValue CastFP32ToUINT32 =
          DAG.getNode(ISD::FP_TO_UINT, DL, MVT::v256i32, Src);
      return DAG.getNode(ISD::TRUNCATE, DL, MVT::v256i8, CastFP32ToUINT32);
    }
  } else if ((OpType == MVT::v256f32) && (SrcType == MVT::v256i8)) {
    SDValue Ext = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::v256i32, Src);
    return (DAG.getNode(ISD::BITCAST, DL, Op.getValueType(), Ext));
  } else {
    llvm_unreachable("Unsupported source and target type");
  }
  return SDValue();
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
                            DAG.getRegister(TPC::SP0, MVT::i1));
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
    llvm_unreachable("Unexpected vector size");
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
  EVT ResultVT = MulResultTable[ElementBitSize];

  MachineSDNode *Node = DAG.getMachineNode(
      InstrOpCode, DL, ResultVT,
      {Op1, Op2, DataType, DAG.getTargetConstant(0, DL, MVT::i32),
       DAG.getUNDEF(ResultVT), DAG.getRegister(TPC::SP0, MVT::i1),
       DAG.getTargetConstant(0, DL, MVT::i1)});
  EVT SubRegType;
  unsigned SwitchTo;
  unsigned ConvertType;
  // generate converts to extract o/p of the appropriate size.
  if ((VT == MVT::v128i16) && (InstrOpCode == TPC::MULi16vvp)) {
    SDValue Mul(Node, 0);
    SubRegType = MVT::v64i32;
    SwitchTo = TPCII::SW_TO_16;
    ConvertType = Subtarget->hasGoyaISA() ? TPC::CONVERT_INT32vip
                                          : TPC::CONVERT_INT32g2vip;
    SDValue N0Val =
        helperExtractSubRegSDValue(TPC::sub_0, DL, DAG, SubRegType, Mul);
    SDValue N1Val =
        helperExtractSubRegSDValue(TPC::sub_1, DL, DAG, SubRegType, Mul);

    SDValue Src1 = helperConvertIntNodeCreate(
        N0Val, DL, DAG, TPCII::SW_LANE_0 | TPCII::SW_RHNE | SwitchTo, VT,
        ConvertType);
    SDValue Src2 = helperConvertIntNodeCreate(
        N1Val, DL, DAG, TPCII::SW_LANE_1 | TPCII::SW_RHNE | SwitchTo, VT,
        ConvertType, &Src1);
    return Src2;
  }
  SDValue MulResult(Node, 0);

  Node = DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, Op.getValueType(),
                            MulResult,
                            DAG.getTargetConstant(TPC::sub_0, DL, MVT::i32));
  return SDValue(Node, 0);
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

SDValue TPCTargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default:
    LLVM_DEBUG(dbgs() << "Custom combining: skipping\n");
    break;
  case ISD::BITCAST:
    return performBitCastCombine(N, DAG, DCI, Subtarget);
  case ISD::VECTOR_SHUFFLE:
    return performShuffleCombine(N, DAG, DCI, Subtarget);
  }

  return SDValue();
}

SDValue TPCTargetLowering::intDivRemHelper(SDValue Op, SelectionDAG &DAG,
                                           SDLoc &DL, bool Unsigned) const {
  EVT ResTy = Op->getValueType(0);

  // TODO: For vector div, leave it unsupported for now. It could be
  // converted to floating point vector and after division, converted
  // back to integer vector.
  if (ResTy.isVector())
    report_fatal_error("Vector integer division not supported.");

  assert(Op.getNumOperands() == 2 && "DIV/REM should have 2 operands.");

  bool IsGen2 = Subtarget->hasGaudiISA(),
       IsGen1 = Subtarget->hasGoyaISA();

  unsigned UdivOpcode = 0;
  if (IsGen1)
    UdivOpcode = TPC::UDIV_STEP;
  else if (IsGen2)
    UdivOpcode = TPC::UDIV_4STEP;

  unsigned StepWidth = 0;
  if (IsGen1)
    StepWidth = 1;
  else if (IsGen2)
    StepWidth = 4;

  unsigned StepX2 = 0;
  if (IsGen1 || IsGen2)
    StepX2 = TPCII::SW_STEP_REG;

  unsigned Count = 0, Step = 0, MoveType = 0;
  EVT UdivType;

  if (ResTy == MVT::i32) {
    UdivType = MVT::v2i32;
    Step = 31;
    MoveType = TPCII::OpType::UINT32;

    if (IsGen1)
      Count = 32;
    else if (IsGen2)
      Count = 8;
  } else if (ResTy == MVT::i16) {
    UdivType = MVT::v2i16;
    Step = 15;
    MoveType = TPCII::OpType::UINT16;

    if (IsGen1)
      Count = 16;
    else if (IsGen2)
      Count = 4;
  } else {
    UdivType = MVT::v2i8;
    Step = 7;
    MoveType = TPCII::OpType::UINT8;

    if (IsGen1)
      Count = 8;
    else if (IsGen2)
      Count = 2;
  }

  // Handle numerator and denominator for signed case.
  SDValue Numerator = Op.getOperand(0), Denominator = Op.getOperand(1);

  uint64_t SignMask;
  if (ResTy == MVT::i32)
    SignMask = 1 << 31;
  else if (ResTy == MVT::i16)
    SignMask = 1 << 15;
  else
    SignMask = 1 << 7;

  if (!Unsigned) {
    Numerator = DAG.getNode(
        ISD::AND, DL, ResTy,
        {Op.getOperand(0), DAG.getTargetConstant(SignMask, DL, ResTy)});
    Denominator = DAG.getNode(
        ISD::AND, DL, ResTy,
        {Op.getOperand(1), DAG.getTargetConstant(SignMask, DL, ResTy)});
  }

  MachineSDNode *Quotient = DAG.getMachineNode(
      TPC::MOVssp, DL, ResTy,
      {Numerator, DAG.getTargetConstant(MoveType, DL, MVT::i8),
       DAG.getTargetConstant(0, DL, MVT::i32), DAG.getUNDEF(ResTy) /*income*/,
       DAG.getRegister(TPC::SP0, MVT::i1) /*Predicate*/,
       DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/});

  MachineSDNode *Remainder = DAG.getMachineNode(
      TPC::MOVsip, DL, ResTy,
      {DAG.getTargetConstant(0, DL, ResTy),
       DAG.getTargetConstant(MoveType, DL, MVT::i8),
       DAG.getTargetConstant(0, DL, MVT::i32), DAG.getUNDEF(ResTy) /*income*/,
       DAG.getRegister(TPC::SP0, MVT::i1) /*Predicate*/,
       DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/});

  SDValue ZReg =
      createTuple({SDValue(Quotient, 0), SDValue(Remainder, 0)}, DAG);

  // For Gen4 or higher Count is 0
  for (std::size_t i = 0; i < Count; ++i) {
    MachineSDNode *Node = DAG.getMachineNode(
        UdivOpcode, DL, UdivType,
        {Denominator /*Divisor*/,
         DAG.getTargetConstant(Step, DL, ResTy) /*Step*/,
         DAG.getTargetConstant(MoveType, DL, MVT::i8) /*UdivType*/,
         DAG.getTargetConstant(StepX2, DL, MVT::i32) /*Switch*/,
         ZReg /*Quotient/Remainder double reg*/,
         DAG.getRegister(TPC::SP0, MVT::i1) /*Predicate*/,
         DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/});
    SDValue NextZReg = SDValue(Node, 0);
    ZReg = NextZReg;
    Step -= StepWidth;
  }

  return ZReg;
}

SDValue TPCTargetLowering::lowerUDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT ResTy = Op->getValueType(0);
  SDValue ZReg = intDivRemHelper(Op, DAG, DL);
  SDValue SubReg = DAG.getTargetConstant(TPC::sub_s1, DL, MVT::i32);
  MachineSDNode *Node =
      DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, ResTy, ZReg, SubReg);
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::lowerUREM(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT ResTy = Op->getValueType(0);
  SDValue ZReg = intDivRemHelper(Op, DAG, DL);
  SDValue SubReg = DAG.getTargetConstant(TPC::sub_s0, DL, MVT::i32);
  MachineSDNode *Node =
      DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, ResTy, ZReg, SubReg);
  return SDValue(Node, 0);
}

SDValue TPCTargetLowering::signedIntDivRemHelper(SDValue Op, SelectionDAG &DAG,
                                                 unsigned SubregVal) const {
  SDLoc DL(Op);
  EVT ResTy = Op->getValueType(0);

  // TODO: For vector div, leave it unsupported for now. It could be
  // coverted to floating point vector and after division, converted
  // back to integer vector.
  if (ResTy.isVector())
    report_fatal_error("Vector integer division not supported.");

  assert(Op.getNumOperands() == 2 && "DIV/REM should have 2 operands.");

  // Extract the sign bit for the computation. We will restore it back
  // after it goes through udiv_step.
  uint64_t SignMask;
  if (ResTy == MVT::i32)
    SignMask = 1 << 31;
  else if (ResTy == MVT::i16)
    SignMask = 1 << 15;
  else
    SignMask = 1 << 7;

  SDValue XorValue =
      DAG.getNode(ISD::XOR, DL, ResTy, {Op.getOperand(0), Op.getOperand(1)});
  SDValue AndValue =
      DAG.getNode(ISD::AND, DL, ResTy,
                  {XorValue, DAG.getTargetConstant(SignMask, DL, ResTy)});

  SDValue ZReg = intDivRemHelper(Op, DAG, DL, false);
  SDValue SubReg = DAG.getTargetConstant(SubregVal, DL, MVT::i32);
  MachineSDNode *Node =
      DAG.getMachineNode(TargetOpcode::EXTRACT_SUBREG, DL, ResTy, ZReg, SubReg);
  SDValue OutValue = SDValue(Node, 0);
  SDValue SignedOutput = DAG.getNode(ISD::OR, DL, ResTy, {OutValue, AndValue});
  return SignedOutput;
}

SDValue TPCTargetLowering::lowerSDIV(SDValue Op, SelectionDAG &DAG) const {
  return signedIntDivRemHelper(Op, DAG, TPC::sub_s1);
}

SDValue TPCTargetLowering::lowerSREM(SDValue Op, SelectionDAG &DAG) const {
  return signedIntDivRemHelper(Op, DAG, TPC::sub_s0);
}

SDValue TPCTargetLowering::lowerFDIV(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT VT = Op->getValueType(0);
  // TODO: Support scalar FDIV in future.
  if (!VT.isVector())
    report_fatal_error("FDIV not supported on scalar pipe.");

  SDValue X = Op.getOperand(0);
  SDValue Y = Op.getOperand(1);
  unsigned Type;

  if (VT == MVT::v64f32)
    Type = TPCII::OpType::FP32;
  else if (VT == MVT::v128f16)
    Type = TPCII::OpType::FP16;
  else
    Type = TPCII::OpType::BF16;

  // Calculate reciprocal of Y using special function.
  MachineSDNode *RecipNode = DAG.getMachineNode(
      TPC::CALC_FP_SPECIALvvm, DL, VT,
      {Y, DAG.getUNDEF(VT) /*src 2*/,
       DAG.getTargetConstant(Type, DL, MVT::i8) /*OpType*/,
       DAG.getTargetConstant(TPCII::SW_RECIP, DL, MVT::i32) /*Func*/,
       DAG.getUNDEF(VT) /*income*/,
       DAG.getRegister(TPC::VP0, MVT::v256i1) /*Predicate*/,
       DAG.getTargetConstant(0, DL, MVT::i1) /*Polarity*/});

  return DAG.getNode(ISD::FMUL, DL, VT, X, SDValue(RecipNode, 0));
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
  unsigned Multiplicity;
  if (ResTy == MVT::v128i32) {
    Multiplicity = 2;
    SubregTy = MVT::v64i32;
  } else if (ResTy == MVT::v256i32) {
    Multiplicity = 4;
    SubregTy = MVT::v64i32;
  } else if(ResTy == MVT::v128f32) {
    Multiplicity = 2;
    SubregTy = MVT::v64f32;
  } else if (ResTy == MVT::v256f32) {
    Multiplicity = 4;
    SubregTy = MVT::v64f32;
  } else {
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
