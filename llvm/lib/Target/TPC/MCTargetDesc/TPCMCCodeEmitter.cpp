//===-- TPCMCCodeEmitter.cpp - Convert TPC Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the TPCMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#include <math.h>

#include "TPCRegisterInfo.h"
#include "TPCSubtarget.h"
#include "TPCAsmBackend.h"
#include "TPCMCCodeEmitter.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/Support/raw_ostream.h"


#define DEBUG_TYPE "mccodeemitter"

//#define GET_INSTRMAP_INFO
#include "TPCGenInstrInfo.inc"
#include "TPCInstrComposer.h"

//#undef GET_INSTRMAP_INFO

static cl::opt<bool> TPCCompressNops("tpc-compress-nops", cl::Hidden,
  cl::ZeroOrMore, cl::init(true),
  cl::desc("Enable compression of NOP instructions"));

static cl::opt<bool> TPCAssertOnImmConflict("tpc-imm-conflict-assert", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("assert on imm conflict"));

////  Load/Store/Imm (75 bits + 1 reserved)
// [159] [127-158] [123-126] [122] [117-121] [110-116] [103-109] [98-102] [91-97] [84-90]
// rsrv    imm      ldst_pr   pp     st_opc    st_srcB  st_srcA   ld_opc  ld_dst  ld_srcA
//
////  VPU (51 bit)
// [80-83] [79] [76-78] [74-75] [67-73] [60-66] [53-59] [46-52] [39-45] [33-38]
//   pr     pp   type     sw     dst    ldsrcD  stsrcC    srcB    srcA    opc
//
////  SPU (33 bits)
// [31-32] [28-30] [27] [24-26] [18-23] [12-17] [6-11] [0-5]
//   sw      pr     pp   type     dst     srcB   srcA   opc


/****

// VPU      	    Gen1        Gen2       Gen3

{"VPU_Opcode",    {{ 0,  5},  { 0,  5},  { 0,  5}}},
{"VPU_SrcA",      {{ 6, 13},  { 6, 13},  { 6, 13}}},
{"VPU_SrcB",      {{14, 21},  {14, 21},  {14, 21}}},
{"VPU_Dest",      {{39, 46},  {39, 46},  {22, 29}}},
{"VPU_OpType",    {{50, 53},  {50, 53},  {30, 33}}},
{"VPU_Polarity",  {{54, 54},  {54, 54},  {34, 34}}},
{"VPU_Pred",      {{55, 59},  {55, 59},  {35, 39}}},
{"VPU_Switches",  {{47, 49},  {40, 43},  {40, 43}}},
{"VPU_SrcC",      {{22, 29},  {22, 29},  {44, 51}}},
{"VPU_SrcD",      {{30, 38},  {30, 38},  {52, 60}}},

// SPU      	    Gen1        Gen2       Gen3

{"SPU_Opcode",    {{ 0,  5},  { 0,  5},  { 0,  5}}},
{"SPU_SrcA",      {{ 6, 12},  { 6, 12},  { 6, 12}}},
{"SPU_SrcB",      {{13, 19},  {13, 19},  {13, 19}}},
{"SPU_Dest",      {{20, 26},  {20, 26},  {20, 26}}},
{"SPU_OpType",    {{27, 30},  {27, 30},  {27, 30}}},
{"SPU_Polarity",  {{31, 31},  {31, 31},  {31, 31}}},
{"SPU_Pred",      {{32, 35},  {32, 35},  {32, 35}}},
{"SPU_Switches",  {{36, 42},  {36, 42},  {36, 42}}},

// LOAD      	    Gen1        Gen2       Gen3

{"LD_SrcA",       {{ 0,  7},  { 0,  7},  { 0,  7}}},
{"LD_Dest",       {{ 8, 15},  { 8, 15},  { 8, 15}}},
{"LD_Opcode",     {{16, 20},  {16, 20},  {16, 20}}},
{"LD_SrcBD",      {{48, 56},  {48, 56},  {21, 29}}},
{"LDST_Polarity", {{42, 42},  {42, 42},  {30, 30}}},
{"LDST_Pred",     {{43, 47},  {43, 47},  {31, 35}}},
{"LD_Switches",   {{57, 62},  {57, 62},  {36, 41}}},

// STORE      	    Gen1        Gen2       Gen3

{"ST_SrcA",       {{ 0,  7},  { 0,  7},  { 0,  7}}},
{"ST_SrcB",       {{ 8, 15},  { 8, 15},  { 8, 15}}},
{"ST_Opcode",     {{16, 20},  {16, 20},  {16, 20}}},
{"ST_SrcC",       {{27, 34},  {27, 34},  {21, 28}}},
{"ST_Switches",   {{35, 40},  {35, 40},  {29, 34}}},

****/

namespace llvm {
MCCodeEmitter *createTPCMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx) {
  return new TPCMCCodeEmitter(MCII, Ctx);
}

void TPCMCCodeEmitter::EmitByte(unsigned char C, llvm::raw_ostream& OS) const {
  OS << (char)C;
}

static unsigned curPos = 0;

void TPCMCCodeEmitter::EmitInstruction(APInt &Instruction, unsigned Size, raw_ostream& OS) const {
  for (unsigned Start = 0; Start < Size; ) {
    unsigned RemainedBits = Size - Start;
    if (RemainedBits >= 64) {
      APInt Chunk = Instruction.lshr(Start).trunc(64);
      EmitConstant(Chunk.getZExtValue(), 8, OS);
      Start += 64;
    } else {
      unsigned ChunkSize = RemainedBits / 8;
      assert(ChunkSize * 8 == RemainedBits);
      APInt Chunk = Instruction.lshr(Start).trunc(RemainedBits);
      EmitConstant(Chunk.getZExtValue(), ChunkSize, OS);
      break;
    }
  }
  curPos += (Size/8);
}

bool TPCMCCodeEmitter::mayCompress(const MCSubtargetInfo &STI) const {
  return STI.getFeatureBits()[TPC::FeatureCompress];
}

bool TPCMCCodeEmitter::isGoya2(const MCSubtargetInfo &STI) const {
  return STI.getFeatureBits()[TPC::FeatureGreco];
}

bool TPCMCCodeEmitter::isGaudi2(const MCSubtargetInfo &STI) const {
  return STI.getFeatureBits()[TPC::FeatureGaudi2];
}

#ifndef NDEBUG

// Helper dump functions for debug purpose //

static void dumpInstrNames(const MCInst& MI, const MCInstrInfo &MCII) {
  fprintf(stderr, "At offset %d bytes:\n", curPos);
  if (MI.getOpcode() == TPC::BUNDLE) {
     for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
         MCInst &Bundle_MI = const_cast<MCInst &>(*I.getInst());
         fprintf(stderr, "--- (%*s)\n", (int)TPCMCInstrInfo::getName(MCII, Bundle_MI).size(), TPCMCInstrInfo::getName(MCII, Bundle_MI).begin());
     }
  }
  else {
    fprintf(stderr, "--- (%*s)\n", (int)TPCMCInstrInfo::getName(MCII, MI).size(), TPCMCInstrInfo::getName(MCII, MI).begin());
  }
}

static void dumpTPCBundleGen4(const APInt &Instr, bool lowHalfOnly = true) {
  unsigned isCompressed1 = (unsigned)(Instr.zextOrTrunc(1).getZExtValue() & 0x01);
  unsigned isCompressed2;
  unsigned compressionType1 = 0;
  unsigned compressionType2 = 0;
  
  if (isCompressed1) {
    compressionType1 = (unsigned)(Instr.lshr(1).zextOrTrunc(1).getZExtValue() & 0x01);
    if (!lowHalfOnly) {
      isCompressed2 = (unsigned)(Instr.lshr(TPCII::cmpr2Start).zextOrTrunc(1).getZExtValue() & 0x01);
      assert (isCompressed2);
      compressionType2 = (unsigned)(Instr.lshr(TPCII::cmpr2Start+1).zextOrTrunc(1).getZExtValue() & 0x01);
    }
  }

#define MASK(Len) ((1<<Len)-1)
  uint64_t slot1;
  uint64_t spu_opc;
  uint64_t spu_srcA;
  uint64_t spu_srcB;
  uint64_t spu_dst;
  uint64_t spu_op_tp;
  uint64_t spu_pp;
  uint64_t spu_pr;
  uint64_t spu_sw;

  uint64_t slot2;
  uint64_t vpu_opc;
  uint64_t vpu_srcA;
  uint64_t vpu_srcB;
  uint64_t vpu_dest;
  uint64_t vpu_sw;
  uint64_t vpu_op_tp;
  uint64_t vpu_pp;
  uint64_t vpu_pr;

  uint64_t ld_srcA;
  uint64_t ld_dst;
  uint64_t ld_opc;
  uint64_t ld_srcB;
  uint64_t ld_pp;
  uint64_t ld_pr;
  uint64_t ld_sw;

  uint64_t st_srcA;
  uint64_t st_srcB;
  uint64_t st_opc;
  uint64_t st_srcC;
  uint64_t st_pp;
  uint64_t st_pr;
  uint64_t st_sw;

  uint64_t vpu_srcC;
  uint64_t vpu_srcD;
  uint64_t vpu_srcE;

  if (isCompressed1) {
    uint64_t imm1 = Instr.lshr(TPCII::imm1Start).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();
    fprintf(stderr, "Compressed = %d\n", isCompressed1);
    fprintf(stderr, "Compression type = %d\n", compressionType1);
    if (compressionType1 == 0) {
      slot1 = Instr.lshr(TPCII::slot1Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
      spu_opc    = slot1         & MASK(6);
      spu_srcA   = (slot1 >>  6) & MASK(7);
      spu_srcB   = (slot1 >> 13) & MASK(7);
      spu_dst    = (slot1 >> 20) & MASK(7);
      spu_op_tp  = (slot1 >> 27) & MASK(4);
      spu_pp     = (slot1 >> 31) & 0x01;
      spu_pr     = (slot1 >> 32) & MASK(4);
      spu_sw     = (slot1 >> 36) & MASK(7);

      slot2 = Instr.lshr(TPCII::slot2Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
      vpu_opc    = (slot2 >> 0)  & MASK(6);
      vpu_srcA   = (slot2 >> 6)  & MASK(8);
      vpu_srcB   = (slot2 >> 14) & MASK(8);
      vpu_dest   = (slot2 >> 22) & MASK(8);
      vpu_op_tp  = (slot2 >> 30) & MASK(4);
      vpu_pp     = (slot2 >> 34) & MASK(1);
      vpu_pr     = (slot2 >> 35) & MASK(5);
      vpu_sw     = (slot2 >> 40) & MASK(7);

      fprintf(stderr, "SPU slot:\n");
      fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
      fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
      fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
      fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
      fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
      fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
      fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
      fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
      fprintf(stderr, "VPU slot:\n");
      fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
      fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
      fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
      fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
      fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
      fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
      fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
      fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
    }
    else if (compressionType1 == 1) {
      slot1 = Instr.lshr(TPCII::slot1Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
      ld_srcA    = (slot1 >> 0)  & MASK(8);
      ld_dst     = (slot1 >> 8)  & MASK(8);
      ld_opc     = (slot1 >> 16) & MASK(5);
      ld_srcB    = (slot1 >> 21) & MASK(9);
      ld_pp      = (slot1 >> 30) & 0x01;
      ld_pr      = (slot1 >> 31) & MASK(5);
      ld_sw      = (slot1 >> 36) & MASK(7);

      slot2 = Instr.lshr(TPCII::slot2Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
      st_srcA    = (slot2 >> 0)  & MASK(8);
      st_srcB    = (slot2 >> 8)  & MASK(8);
      st_opc     = (slot2 >> 16) & MASK(5);
      st_srcC    = (slot2 >> 21) & MASK(8);
      st_pp      = (slot2 >> 29) & 0x01;
      st_pr      = (slot2 >> 30) & MASK(5);
      st_sw      = (slot2 >> 35) & MASK(7);

      fprintf(stderr, "Load slot:\n");
      fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
      fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
      fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
      fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
      fprintf(stderr, "  ld_pp     = %" PRIu64 "     \n", ld_pp);
      fprintf(stderr, "  ld_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_pr, ld_pr);
      fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
      fprintf(stderr, "Store slot:\n");
      fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
      fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
      fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
      fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
      fprintf(stderr, "  st_pp     = %" PRIu64 "     \n", st_pp);
      fprintf(stderr, "  st_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", st_pr, st_pr);
      fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);
    }
    fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm1, imm1);

    if (!lowHalfOnly) {
      //
      // Second part of compressed instruction
      //
      imm1 = Instr.lshr(TPCII::imm2Start).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();
      fprintf(stderr, "Compressed = %d\n", isCompressed2);
      fprintf(stderr, "Compression type = %d\n", compressionType2);
      if (compressionType2 == 0) {
        slot1 = Instr.lshr(TPCII::slot3Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
        spu_opc    = slot1         & MASK(6);
        spu_srcA   = (slot1 >>  6) & MASK(7);
        spu_srcB   = (slot1 >> 13) & MASK(7);
        spu_dst    = (slot1 >> 20) & MASK(7);
        spu_op_tp  = (slot1 >> 27) & MASK(4);
        spu_pp     = (slot1 >> 31) & 0x01;
        spu_pr     = (slot1 >> 32) & MASK(4);
        spu_sw     = (slot1 >> 36) & MASK(7);

        slot2 = Instr.lshr(TPCII::slot4Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
        vpu_opc    = (slot2 >> 0)  & MASK(6);
        vpu_srcA   = (slot2 >> 6)  & MASK(8);
        vpu_srcB   = (slot2 >> 14) & MASK(8);
        vpu_dest   = (slot2 >> 22) & MASK(8);
        vpu_op_tp  = (slot2 >> 30) & MASK(4);
        vpu_pp     = (slot2 >> 34) & MASK(1);
        vpu_pr     = (slot2 >> 35) & MASK(5);
        vpu_sw     = (slot2 >> 40) & MASK(7);

        fprintf(stderr, "SPU slot:\n");
        fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
        fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
        fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
        fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
        fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
        fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
        fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
        fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
        fprintf(stderr, "VPU slot:\n");
        fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
        fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
        fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
        fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
        fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
        fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
        fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
        fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
      }
      else if (compressionType2 == 1) {
        slot1 = Instr.lshr(TPCII::slot3Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
        ld_srcA    = (slot1 >> 0)  & MASK(8);
        ld_dst     = (slot1 >> 8)  & MASK(8);
        ld_opc     = (slot1 >> 16) & MASK(5);
        ld_srcB    = (slot1 >> 21) & MASK(9);
        ld_pp      = (slot1 >> 30) & 0x01;
        ld_pr      = (slot1 >> 31) & MASK(5);
        ld_sw      = (slot1 >> 36) & MASK(7);

        slot2 = Instr.lshr(TPCII::slot4Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
        st_srcA    = (slot2 >> 0)  & MASK(8);
        st_srcB    = (slot2 >> 8)  & MASK(8);
        st_opc     = (slot2 >> 16) & MASK(5);
        st_srcC    = (slot2 >> 21) & MASK(8);
        st_pp      = (slot2 >> 29) & 0x01;
        st_pr      = (slot2 >> 30) & MASK(5);
        st_sw      = (slot2 >> 35) & MASK(7);

        fprintf(stderr, "Load slot:\n");
        fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
        fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
        fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
        fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
        fprintf(stderr, "  ld_pp     = %" PRIu64 "     \n", ld_pp);
        fprintf(stderr, "  ld_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_pr, ld_pr);
        fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
        fprintf(stderr, "Store slot:\n");
        fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
        fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
        fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
        fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
        fprintf(stderr, "  st_pp     = %" PRIu64 "     \n", st_pp);
        fprintf(stderr, "  st_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", st_pr, st_pr);
        fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);
      }
      fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm1, imm1);
    }
  }
  else {
    slot1 = Instr.lshr(TPCII::Gen3SPUStart).zextOrTrunc(TPCII::Gen3SPUSize).getZExtValue();
    spu_opc    = slot1         & MASK(6);
    spu_srcA   = (slot1 >>  6) & MASK(7);
    spu_srcB   = (slot1 >> 13) & MASK(7);
    spu_dst    = (slot1 >> 20) & MASK(7);
    spu_op_tp  = (slot1 >> 27) & MASK(4);
    spu_pp     = (slot1 >> 31) & 0x01;
    spu_pr     = (slot1 >> 32) & MASK(4);
    spu_sw     = (slot1 >> 36) & MASK(7);

    slot2 = Instr.lshr(TPCII::Gen3VPUStart).zextOrTrunc(TPCII::Gen3VPUSize).getZExtValue();
    vpu_opc    = (slot2 >> 0)  & MASK(6);
    vpu_srcA   = (slot2 >> 6)  & MASK(8);
    vpu_srcB   = (slot2 >> 14) & MASK(8);
    vpu_dest   = (slot2 >> 22) & MASK(8);
    vpu_op_tp  = (slot2 >> 30) & MASK(4);
    vpu_pp     = (slot2 >> 34) & MASK(1);
    vpu_pr     = (slot2 >> 35) & MASK(5);
    vpu_sw     = (slot2 >> 40) & MASK(7);

    uint64_t slot3 = Instr.lshr(TPCII::Gen3LDStart).zextOrTrunc(TPCII::Gen3LDSize).getZExtValue();
    ld_srcA    = (slot3 >> 0)  & MASK(8);
    ld_dst     = (slot3 >> 8)  & MASK(8);
    ld_opc     = (slot3 >> 16) & MASK(5);
    ld_srcB    = (slot3 >> 21) & MASK(9);
    ld_pp      = (slot3 >> 30) & 0x01;
    ld_pr      = (slot3 >> 31) & MASK(5);
    ld_sw      = (slot3 >> 36) & MASK(7);

    uint64_t slot4 = Instr.lshr(TPCII::Gen3STStart).zextOrTrunc(TPCII::Gen3STSize).getZExtValue();
    st_srcA    = (slot4 >> 0)  & MASK(8);
    st_srcB    = (slot4 >> 8)  & MASK(8);
    st_opc     = (slot4 >> 16) & MASK(5);
    st_srcC    = (slot4 >> 21) & MASK(8);
    st_pp      = (slot4 >> 29) & 0x01;
    st_pr      = (slot4 >> 30) & MASK(5);
    st_sw      = (slot4 >> 35) & MASK(7);

    uint64_t imm = Instr.lshr(TPCII::Gen3ImmStart).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();

    vpu_srcC = Instr.lshr(TPCII::Gen3VPUSrcCStart).zextOrTrunc(TPCII::Gen3VPUSrcCSize).getZExtValue();
    vpu_srcD = Instr.lshr(TPCII::Gen3VPUSrcDStart).zextOrTrunc(TPCII::Gen3VPUSrcDSize).getZExtValue();
    vpu_srcE = Instr.lshr(TPCII::Gen3VPUSrcEStart).zextOrTrunc(TPCII::Gen3VPUSrcESize).getZExtValue();

    fprintf(stderr, "Compressed= %d\n", isCompressed1);
    fprintf(stderr, "SPU slot:\n");
    fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
    fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
    fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
    fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
    fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
    fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
    fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
    fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
    fprintf(stderr, "VPU slot:\n");
    fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
    fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
    fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
    fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
    fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
    fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
    fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
    fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
    fprintf(stderr, "Load slot:\n");
    fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
    fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
    fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
    fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
    fprintf(stderr, "  ld_pp     = %" PRIu64 "     \n", ld_pp);
    fprintf(stderr, "  ld_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_pr, ld_pr);
    fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
    fprintf(stderr, "Store slot:\n");
    fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
    fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
    fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
    fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
    fprintf(stderr, "  st_pp     = %" PRIu64 "     \n", st_pp);
    fprintf(stderr, "  st_pr     = %" PRIu64 " (0x%" PRIX64 ")\n", st_pr, st_pr);
    fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);

    // Out-of-slot fields
    fprintf(stderr, "Vpu_srcC  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcC, vpu_srcC);
    fprintf(stderr, "Vpu_srcD  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcD, vpu_srcD);
    fprintf(stderr, "Vpu_srcE  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcE, vpu_srcE);
    fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm, imm);
  }
#undef MASK
}


static void dumpTPCBundleGen3(const APInt &Instr, bool lowHalfOnly = true) {
  unsigned isCompressed1 = (unsigned)(Instr.zextOrTrunc(1).getZExtValue() & 0x01);
  unsigned isCompressed2;
  unsigned compressionType1 = 0;
  unsigned compressionType2 = 0;
  
  if (isCompressed1) {
    compressionType1 = (unsigned)(Instr.lshr(1).zextOrTrunc(1).getZExtValue() & 0x01);
    if (!lowHalfOnly) {
      isCompressed2 = (unsigned)(Instr.lshr(TPCII::cmpr2Start).zextOrTrunc(1).getZExtValue() & 0x01);
      assert (isCompressed2);
      compressionType2 = (unsigned)(Instr.lshr(TPCII::cmpr2Start+1).zextOrTrunc(1).getZExtValue() & 0x01);
    }
  }

#define MASK(Len) ((1<<Len)-1)
  uint64_t slot1;
  uint64_t spu_opc;
  uint64_t spu_srcA;
  uint64_t spu_srcB;
  uint64_t spu_dst;
  uint64_t spu_op_tp;
  uint64_t spu_pp;
  uint64_t spu_pr;
  uint64_t spu_sw;

  uint64_t slot2;
  uint64_t vpu_opc;
  uint64_t vpu_srcA;
  uint64_t vpu_srcB;
  uint64_t vpu_dest;
  uint64_t vpu_sw;
  uint64_t vpu_op_tp;
  uint64_t vpu_pp;
  uint64_t vpu_pr;

  uint64_t ld_srcA;
  uint64_t ld_dst;
  uint64_t ld_opc;
  uint64_t ld_srcB;
  uint64_t ldst_pp;
  uint64_t ldst_pr;
  uint64_t ld_sw;

  uint64_t st_srcA;
  uint64_t st_srcB;
  uint64_t st_opc;
  uint64_t st_srcC;
  uint64_t st_sw;

  uint64_t vpu_srcC;
  uint64_t vpu_srcD;
  uint64_t vpu_srcE;

  if (isCompressed1) {
    uint64_t imm1 = Instr.lshr(TPCII::imm1Start).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();
    fprintf(stderr, "Compressed = %d\n", isCompressed1);
    fprintf(stderr, "Compression type = %d\n", compressionType1);
    if (compressionType1 == 0) {
      slot1 = Instr.lshr(TPCII::slot1Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
      spu_opc    = slot1         & MASK(6);
      spu_srcA   = (slot1 >>  6) & MASK(7);
      spu_srcB   = (slot1 >> 13) & MASK(7);
      spu_dst    = (slot1 >> 20) & MASK(7);
      spu_op_tp  = (slot1 >> 27) & MASK(4);
      spu_pp     = (slot1 >> 31) & 0x01;
      spu_pr     = (slot1 >> 32) & MASK(4);
      spu_sw     = (slot1 >> 36) & MASK(7);

      slot2 = Instr.lshr(TPCII::slot2Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
      vpu_opc    = (slot2 >> 0)  & MASK(6);
      vpu_srcA   = (slot2 >> 6)  & MASK(8);
      vpu_srcB   = (slot2 >> 14) & MASK(8);
      vpu_dest   = (slot2 >> 22) & MASK(8);
      vpu_op_tp  = (slot2 >> 30) & MASK(4);
      vpu_pp     = (slot2 >> 34) & MASK(1);
      vpu_pr     = (slot2 >> 35) & MASK(5);
      vpu_sw     = (slot2 >> 40) & MASK(7);

      fprintf(stderr, "SPU slot:\n");
      fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
      fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
      fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
      fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
      fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
      fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
      fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
      fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
      fprintf(stderr, "VPU slot:\n");
      fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
      fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
      fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
      fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
      fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
      fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
      fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
      fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
    }
    else if (compressionType1 == 1) {
      slot1 = Instr.lshr(TPCII::slot1Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
      ld_srcA    = (slot1 >> 0)  & MASK(8);
      ld_dst     = (slot1 >> 8)  & MASK(8);
      ld_opc     = (slot1 >> 16) & MASK(5);
      ld_srcB    = (slot1 >> 21) & MASK(9);
      ldst_pp    = (slot1 >> 30) & 0x01;
      ldst_pr    = (slot1 >> 31) & MASK(5);
      ld_sw      = (slot1 >> 36) & MASK(6);

      slot2 = Instr.lshr(TPCII::slot2Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
      st_srcA    = (slot2 >> 0)  & MASK(8);
      st_srcB    = (slot2 >> 8)  & MASK(8);
      st_opc     = (slot2 >> 16) & MASK(5);
      st_srcC    = (slot2 >> 21) & MASK(8);
      st_sw      = (slot2 >> 29) & MASK(6);

      fprintf(stderr, "Load slot:\n");
      fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
      fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
      fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
      fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
      fprintf(stderr, "  ldst_pp   = %" PRIu64 "     \n", ldst_pp);
      fprintf(stderr, "  ldst_pr   = %" PRIu64 " (0x%" PRIX64 ")\n", ldst_pr, ldst_pr);
      fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
      fprintf(stderr, "Store slot:\n");
      fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
      fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
      fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
      fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
      fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);
    }
    fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm1, imm1);

    if (!lowHalfOnly) {
      //
      // Second part of compressed instruction
      //
      imm1 = Instr.lshr(TPCII::imm2Start).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();
      fprintf(stderr, "Compressed = %d\n", isCompressed2);
      fprintf(stderr, "Compression type = %d\n", compressionType2);
      if (compressionType2 == 0) {
        slot1 = Instr.lshr(TPCII::slot3Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
        spu_opc    = slot1         & MASK(6);
        spu_srcA   = (slot1 >>  6) & MASK(7);
        spu_srcB   = (slot1 >> 13) & MASK(7);
        spu_dst    = (slot1 >> 20) & MASK(7);
        spu_op_tp  = (slot1 >> 27) & MASK(4);
        spu_pp     = (slot1 >> 31) & 0x01;
        spu_pr     = (slot1 >> 32) & MASK(4);
        spu_sw     = (slot1 >> 36) & MASK(7);

        slot2 = Instr.lshr(TPCII::slot4Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
        vpu_opc    = (slot2 >> 0)  & MASK(6);
        vpu_srcA   = (slot2 >> 6)  & MASK(8);
        vpu_srcB   = (slot2 >> 14) & MASK(8);
        vpu_dest   = (slot2 >> 22) & MASK(8);
        vpu_op_tp  = (slot2 >> 30) & MASK(4);
        vpu_pp     = (slot2 >> 34) & MASK(1);
        vpu_pr     = (slot2 >> 35) & MASK(5);
        vpu_sw     = (slot2 >> 40) & MASK(7);

        fprintf(stderr, "SPU slot:\n");
        fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
        fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
        fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
        fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
        fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
        fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
        fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
        fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
        fprintf(stderr, "VPU slot:\n");
        fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
        fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
        fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
        fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
        fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
        fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
        fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
        fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
      }
      else if (compressionType2 == 1) {
        slot1 = Instr.lshr(TPCII::slot3Start).zextOrTrunc(TPCII::slot1Size).getZExtValue();
        ld_srcA    = (slot1 >> 0)  & MASK(8);
        ld_dst     = (slot1 >> 8)  & MASK(8);
        ld_opc     = (slot1 >> 16) & MASK(5);
        ld_srcB    = (slot1 >> 21) & MASK(9);
        ldst_pp    = (slot1 >> 30) & 0x01;
        ldst_pr    = (slot1 >> 31) & MASK(5);
        ld_sw      = (slot1 >> 36) & MASK(6);

        slot2 = Instr.lshr(TPCII::slot4Start).zextOrTrunc(TPCII::slot2Size).getZExtValue();
        st_srcA    = (slot2 >> 0)  & MASK(8);
        st_srcB    = (slot2 >> 8)  & MASK(8);
        st_opc     = (slot2 >> 16) & MASK(5);
        st_srcC   = (slot2 >> 21) & MASK(8);
        st_sw      = (slot2 >> 29) & MASK(6);

        fprintf(stderr, "Load slot:\n");
        fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
        fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
        fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
        fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
        fprintf(stderr, "  ldst_pp   = %" PRIu64 "     \n", ldst_pp);
        fprintf(stderr, "  ldst_pr   = %" PRIu64 " (0x%" PRIX64 ")\n", ldst_pr, ldst_pr);
        fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
        fprintf(stderr, "Store slot:\n");
        fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
        fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
        fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
        fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
        fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);
      }
      fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm1, imm1);
    }
  }
  else {
    slot1 = Instr.lshr(TPCII::Gen3SPUStart).zextOrTrunc(TPCII::Gen3SPUSize).getZExtValue();
    spu_opc    = slot1         & MASK(6);
    spu_srcA   = (slot1 >>  6) & MASK(7);
    spu_srcB   = (slot1 >> 13) & MASK(7);
    spu_dst    = (slot1 >> 20) & MASK(7);
    spu_op_tp  = (slot1 >> 27) & MASK(4);
    spu_pp     = (slot1 >> 31) & 0x01;
    spu_pr     = (slot1 >> 32) & MASK(4);
    spu_sw     = (slot1 >> 36) & MASK(7);

    slot2 = Instr.lshr(TPCII::Gen3VPUStart).zextOrTrunc(TPCII::Gen3VPUSize).getZExtValue();
    vpu_opc    = (slot2 >> 0)  & MASK(6);
    vpu_srcA   = (slot2 >> 6)  & MASK(8);
    vpu_srcB   = (slot2 >> 14) & MASK(8);
    vpu_dest   = (slot2 >> 22) & MASK(8);
    vpu_op_tp  = (slot2 >> 30) & MASK(4);
    vpu_pp     = (slot2 >> 34) & MASK(1);
    vpu_pr     = (slot2 >> 35) & MASK(5);
    vpu_sw     = (slot2 >> 40) & MASK(7);

    uint64_t slot3 = Instr.lshr(TPCII::Gen3LDStart).zextOrTrunc(TPCII::Gen3LDSize).getZExtValue();
    ld_srcA    = (slot3 >> 0)  & MASK(8);
    ld_dst     = (slot3 >> 8)  & MASK(8);
    ld_opc     = (slot3 >> 16) & MASK(5);
    ld_srcB    = (slot3 >> 21) & MASK(9);
    ldst_pp    = (slot3 >> 30) & 0x01;
    ldst_pr    = (slot3 >> 31) & MASK(5);
    ld_sw      = (slot3 >> 36) & MASK(6);

    uint64_t slot4 = Instr.lshr(TPCII::Gen3STStart).zextOrTrunc(TPCII::Gen3STSize).getZExtValue();
    st_srcA    = (slot4 >> 0)  & MASK(8);
    st_srcB    = (slot4 >> 8)  & MASK(8);
    st_opc     = (slot4 >> 16) & MASK(5);
    st_srcC    = (slot4 >> 21) & MASK(8);
    st_sw      = (slot4 >> 29) & MASK(6);

    uint64_t imm = Instr.lshr(TPCII::Gen3ImmStart).zextOrTrunc(TPCII::Gen3ImmSize).getZExtValue();

    vpu_srcC = Instr.lshr(TPCII::Gen3VPUSrcCStart).zextOrTrunc(TPCII::Gen3VPUSrcCSize).getZExtValue();
    vpu_srcD = Instr.lshr(TPCII::Gen3VPUSrcDStart).zextOrTrunc(TPCII::Gen3VPUSrcDSize).getZExtValue();
    vpu_srcE = Instr.lshr(TPCII::Gen3VPUSrcEStart).zextOrTrunc(TPCII::Gen3VPUSrcESize).getZExtValue();

    fprintf(stderr, "Compressed= %d\n", isCompressed1);
    fprintf(stderr, "SPU slot:\n");
    fprintf(stderr, "  spu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_opc, spu_opc);
    fprintf(stderr, "  spu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcA, spu_srcA);
    fprintf(stderr, "  spu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", spu_srcB, spu_srcB);
    fprintf(stderr, "  spu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", spu_dst, spu_dst);
    fprintf(stderr, "  spu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", spu_op_tp, spu_op_tp);
    fprintf(stderr, "  spu_pp    = %" PRIu64 "      \n", spu_pp);
    fprintf(stderr, "  spu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_pr, spu_pr);
    fprintf(stderr, "  spu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", spu_sw, spu_sw);
    fprintf(stderr, "VPU slot:\n");
    fprintf(stderr, "  vpu_opc   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_opc, vpu_opc);
    fprintf(stderr, "  vpu_srcA  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcA, vpu_srcA);
    fprintf(stderr, "  vpu_srcB  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcB, vpu_srcB);
    fprintf(stderr, "  vpu_dst   = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_dest, vpu_dest);
    fprintf(stderr, "  vpu_op_tp = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_op_tp, vpu_op_tp);
    fprintf(stderr, "  vpu_pp    = %" PRIu64 "      \n", vpu_pp);
    fprintf(stderr, "  vpu_pr    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_pr, vpu_pr);
    fprintf(stderr, "  vpu_sw    = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_sw, vpu_sw);
    fprintf(stderr, "Load slot:\n");
    fprintf(stderr, "  ld_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_opc, ld_opc);
    fprintf(stderr, "  ld_dst    = %" PRIu64 " (0x%" PRIX64 ")\n", ld_dst, ld_dst);
    fprintf(stderr, "  ld_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcA, ld_srcA);
    fprintf(stderr, "  ld_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", ld_srcB, ld_srcB);
    fprintf(stderr, "  ldst_pp   = %" PRIu64 "     \n", ldst_pp);
    fprintf(stderr, "  ldst_pr   = %" PRIu64 " (0x%" PRIX64 ")\n", ldst_pr, ldst_pr);
    fprintf(stderr, "  ld_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", ld_sw, ld_sw);
    fprintf(stderr, "Store slot:\n");
    fprintf(stderr, "  st_opc    = %" PRIu64 " (0x%" PRIX64 ")\n", st_opc, st_opc);
    fprintf(stderr, "  st_srcA   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcA, st_srcA);
    fprintf(stderr, "  st_srcB   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcB, st_srcB);
    fprintf(stderr, "  st_srcC   = %" PRIu64 " (0x%" PRIX64 ")\n", st_srcC, st_srcC);
    fprintf(stderr, "  st_sw     = %" PRIu64 " (0x%" PRIX64 ")\n", st_sw, st_sw);

    // Out-of-slot fields
    fprintf(stderr, "Vpu_srcC  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcC, vpu_srcC);
    fprintf(stderr, "Vpu_srcD  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcD, vpu_srcD);
    fprintf(stderr, "Vpu_srcE  = %" PRIu64 " (0x%" PRIX64 ")\n", vpu_srcE, vpu_srcE);
    fprintf(stderr, "Imm       = %" PRIu64 " (0x%" PRIX64 ")\n", imm, imm);
  }
#undef MASK
}

static void dumpTPCBundleGen2(const APInt &Instr) {

#define MASK(Len) ((1<<Len)-1)

  uint64_t spu_slot = Instr.lshr(TPCII::SPUStart).zextOrTrunc(TPCII::SPUSize).getZExtValue();
  uint64_t spu_opc    = spu_slot         & MASK(6);
  uint64_t spu_srcA   = (spu_slot >>  6) & MASK(7);
  uint64_t spu_srcB   = (spu_slot >> 13) & MASK(7);
  uint64_t spu_dst    = (spu_slot >> 20) & MASK(7);
  uint64_t spu_op_tp  = (spu_slot >> 27) & MASK(4);
  uint64_t spu_pp     = (spu_slot >> 31) & 0x01;
  uint64_t spu_pr     = (spu_slot >> 32) & MASK(4);
  uint64_t spu_sw     = (spu_slot >> 36) & MASK(7);

  uint64_t vpu_slot = Instr.lshr(TPCII::VPUStart).zextOrTrunc(TPCII::VPUSize).getZExtValue();
#define SHIFT(Start) (Start - TPCII::VPUStart)
  uint64_t vpu_opc    = (vpu_slot >> SHIFT(43)) & MASK(6);
  uint64_t vpu_srcA   = (vpu_slot >> SHIFT(49)) & MASK(8);
  uint64_t vpu_srcB   = (vpu_slot >> SHIFT(57)) & MASK(8);
  uint64_t vpu_srcC   = (vpu_slot >> SHIFT(65)) & MASK(8);
  uint64_t vpu_srcD   = (vpu_slot >> SHIFT(73)) & MASK(9);
  uint64_t vpu_dest   = (vpu_slot >> SHIFT(82)) & MASK(8);
  uint64_t vpu_sw     = (vpu_slot >> SHIFT(90)) & MASK(2);
  uint64_t vpu_op_tp  = (vpu_slot >> SHIFT(93)) & MASK(4);
  uint64_t vpu_pp     = (vpu_slot >> SHIFT(97)) & MASK(1);
  uint64_t vpu_pr     = (vpu_slot >> SHIFT(98)) & MASK(5);
#undef SHIFT

  uint64_t ld_slot = Instr.lshr(TPCII::LDStart).zextOrTrunc(TPCII::LDSize).getZExtValue();
#define SHIFT(Start) (Start - TPCII::LDStart)
  uint64_t ld_srcA    = (ld_slot >> SHIFT(103)) & MASK(8);
  uint64_t ld_dst     = (ld_slot >> SHIFT(111)) & MASK(8);
  uint64_t ld_opc     = (ld_slot >> SHIFT(119)) & MASK(5);
#undef SHIFT

  uint64_t st_slot = Instr.lshr(TPCII::STStart).zextOrTrunc(TPCII::STSize).getZExtValue();
#define SHIFT(Start) (Start - TPCII::STStart)
  uint64_t st_srcA    = (st_slot >> SHIFT(124)) & MASK(8);
  uint64_t st_srcB    = (st_slot >> SHIFT(132)) & MASK(8);
  uint64_t st_opc     = (st_slot >> SHIFT(140)) & MASK(5);
#undef SHIFT

  uint64_t ldst_pp = Instr[TPCII::LDSTPolarity];
  uint64_t ldst_pr = Instr.lshr(TPCII::LDSTPredicateStart).zextOrTrunc(TPCII::LDSTPredicateSize).getZExtValue();
  uint64_t imm = Instr.lshr(TPCII::ImmStart).zextOrTrunc(TPCII::ImmSize).getZExtValue();

  fprintf(stderr, "spu_opc   = %" PRIu64 "\n", spu_opc);
  fprintf(stderr, "spu_srcA  = %" PRIu64 "\n", spu_srcA);
  fprintf(stderr, "spu_srcB  = %" PRIu64 "\n", spu_srcB);
  fprintf(stderr, "spu_dst   = %" PRIu64 "\n", spu_dst);
  fprintf(stderr, "spu_op_tp = %" PRIu64 "\n", spu_op_tp);
  fprintf(stderr, "spu_pp    = %" PRIu64 "\n", spu_pp);
  fprintf(stderr, "spu_pr    = %" PRIu64 "\n", spu_pr);
  fprintf(stderr, "spu_sw    = %" PRIu64 "\n", spu_sw);
  fprintf(stderr, "vpu_opc   = %" PRIu64 "\n", vpu_opc);
  fprintf(stderr, "vpu_srcA  = %" PRIu64 "\n", vpu_srcA);
  fprintf(stderr, "vpu_srcB  = %" PRIu64 "\n", vpu_srcB);
  fprintf(stderr, "vpu_stsrcC= %" PRIu64 "\n", vpu_srcC);
  fprintf(stderr, "vpu_ldsrcD= %" PRIu64 "\n", vpu_srcD);
  fprintf(stderr, "vpu_dst   = %" PRIu64 "\n", vpu_dest);
  fprintf(stderr, "vpu_op_tp = %" PRIu64 "\n", vpu_op_tp);
  fprintf(stderr, "vpu_pp    = %" PRIu64 "\n", vpu_pp);
  fprintf(stderr, "vpu_pr    = %" PRIu64 "\n", vpu_pr);
  fprintf(stderr, "vpu_sw    = %" PRIu64 "\n", vpu_sw);
  fprintf(stderr, "ld_srcA   = %" PRIu64 "\n", ld_srcA);
  fprintf(stderr, "ld_dst    = %" PRIu64 "\n", ld_dst);
  fprintf(stderr, "ld_opc    = %" PRIu64 "\n", ld_opc);
  fprintf(stderr, "st_srcA   = %" PRIu64 "\n", st_srcA);
  fprintf(stderr, "st_srcB   = %" PRIu64 "\n", st_srcB);
  fprintf(stderr, "st_opc    = %" PRIu64 "\n", st_opc);
  fprintf(stderr, "ldst_pp   = %" PRIu64 "\n", ldst_pp);
  fprintf(stderr, "ldst_pr   = %" PRIu64 "\n", ldst_pr);
  fprintf(stderr, "imm       = %" PRIu64 "\n", imm);
}

static void dumpTPCBundle(const APInt &Instr, const FeatureBitset &Features) {
  if (Features[TPC::FeatureGoya]) {
    dumpTPCBundleGen2(Instr);
  }
  else if (Features[TPC::FeatureGaudi]) {
    dumpTPCBundleGen2(Instr);
  } else if (Features[TPC::FeatureGaudiB]) {
    dumpTPCBundleGen2(Instr);
  } else if (Features[TPC::FeatureGreco]) {
    dumpTPCBundleGen3(Instr);
  } else if (Features[TPC::FeatureGaudi2]) {
    dumpTPCBundleGen4(Instr);
  } else if (Features[TPC::FeatureDoron1]) {
    //dumpTPCBundleGen4(Instr);
    fprintf(stderr, "TODO dumpTPCBundleDoron1\n");
  }
}
#endif


// Helper function which emits Noop instructions.
//
#if defined(TPC_NOPS_AFTER_ALL) || defined(TPC_DISABLE_ALL_SCHED)
static void insertNoops(int NopCnt, const TPCMCCodeEmitter* CE, raw_ostream& OS) {
  if (NopCnt == 0) return;
  APInt DummyInstruction(TPCII::InstructionSize, 0);
  APInt DSPUSlot(TPCII::SPUSize, TPCII::spuNOP);
  APInt DVPUSlot(TPCII::VPUSize, TPCII::vpuNOP);
  APInt DLDSlot(TPCII::LDSize, TPCII::ldNOP << 16);
  APInt DSTSlot(TPCII::STSize, TPCII::stNOP << 16);
  APInt DImmValue(TPCII::ImmSize, 0);

  DummyInstruction |= DSPUSlot.zext(TPCII::InstructionSize).shl(TPCII::SPUStart);
  DummyInstruction |= DVPUSlot.zext(TPCII::InstructionSize).shl(TPCII::VPUStart);
  DummyInstruction |= DLDSlot.zext(TPCII::InstructionSize).shl(TPCII::LDStart);
  DummyInstruction |= DSTSlot.zext(TPCII::InstructionSize).shl(TPCII::STStart);
  DummyInstruction |= DImmValue.zext(TPCII::InstructionSize).shl(TPCII::ImmStart);

  for (int i = 0; i < NopCnt; ++i) {
    EmitInstruction(DummyInstruction, TPCII::InstructionSize, OS);
  }
}
#endif

//
// getInstrBits(const MCInst& MI, const MCSubtargetInfo& STI)
//
// Returns binary code for instruction MI. 
// MI must be a single instruction, not a bundle.
// Converts Gen2 bits to Gen3.
//
uint64_t TPCMCCodeEmitter::getInstrBits(const MCInst& MI, SmallVectorImpl<MCFixup>& Fixups, const MCSubtargetInfo& STI) const {
  uint64_t inst = getBinaryCodeForInstr(MI, Fixups, STI);
  return inst;
}

void TPCMCCodeEmitter::fillInstrBits(const MCInst& MI,
                                     SmallVectorImpl<MCFixup>& Fixups,
                                     TPCInstrBits& Bits,
                                     const MCSubtargetInfo& STI,
                                     bool ignoreNops,
                                     bool isSingle,
                                     const Optional<uint64_t> &StoreSrcC) const
{
  bool isDoron1 = STI.getFeatureBits()[TPC::FeatureDoron1];
  const unsigned opcode = MI.getOpcode();
  if (isSingle) {
    Bits.MI = &MI;
  }
  if (opcode == TPC::BUNDLE) {
    // Lookup instruction with SrcCIsStoreSrcC flag
    Optional<uint64_t> StoreSrcCLocal;
    for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
      MCInst &BundleMI = const_cast<MCInst &>(*I.getInst());
      if (TPCII::getSrcCIsStoreSrcC(MCII.get(BundleMI.getOpcode()))) {

        assert(isDoron1 || STI.getFeatureBits()[TPC::FeatureGaudi2]);
        const Field VPUSrcCField =
            TPCInstrLayout::getVPUInstrLayout(false, STI.getFeatureBits())
                .at(VPU_SRC_C);

        APInt Inst(64, getInstrBits(BundleMI, Fixups, STI));
        StoreSrcCLocal = Inst.lshr(VPUSrcCField.startLLVM)
            .zextOrTrunc(VPUSrcCField.sizeBin).getZExtValue();
      }
    }

    unsigned SlotCount = 0;
    for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
      MCInst &BundleMI = const_cast<MCInst &>(*I.getInst());
      if (BundleMI.getOpcode() == TPC::LOOPEND)
        continue;

      if(isStoreInst(BundleMI)) {
        fillInstrBits(BundleMI, Fixups, Bits, STI,
                      ignoreNops, false, StoreSrcCLocal);
      } else {
        fillInstrBits(BundleMI, Fixups, Bits, STI,
                      ignoreNops, false, StoreSrcCLocal);
      }
      SlotCount++;

      if (isDoron1 &&
          TPCMCInstrInfo::HasPredicate(BundleMI, MCII.get(MI.getOpcode()),
                                       isDoron1)) {
        if (isSPUInst(BundleMI)) {
          assert(Bits.SPUPredicated);
        } else if (isVPUInst(BundleMI)) {
          assert(Bits.VPUPredicated);
        } else if (isStoreInst(BundleMI)) {
          assert(Bits.StorePredicated);
        } else if (isLoadInst(BundleMI)) {
          assert(Bits.LoadPredicated);
        }
      }
    }

    if (!STI.getFeatureBits()[TPC::FeatureCompress]) {
      Bits.compress = 0;
    }
    else if (!Bits.hasVPU && !Bits.hasSPU && !Bits.hasLD && !Bits.hasST && ignoreNops) {
      // Instruction is a full NOP - do not compress it
      if (TPCCompressNops) {
        Bits.compress = 1;
      }
      else {
        Bits.compress = 0;
      }
    }
    else if (Bits.StSrcExtra || Bits.LdSrcExtra) {
      Bits.compress = 0;
    }
    else if ((Bits.hasVPU || Bits.hasSPU) && (Bits.hasLD || Bits.hasST)) {
      // Cannot compress
      Bits.compress = 0;
    }
    else if (Bits.hasVPU || Bits.hasSPU) {
      Bits.compress = 1;
    }
    else {
      Bits.compress = 3;
    }
    assert ((Bits.compress != 0) || (SlotCount == 4));
    assert ((Bits.compress == 0) || (SlotCount == 2));
  }
  else {
    if (isSingle) {
      if (MI.getOpcode() == TPC::LOOPEND) return; // Don't encode pseudos
      Bits.VPUInst = TPCII::vpuNOP;
      Bits.SPUInst = TPCII::spuNOP;
      Bits.LDInst = TPCII::ldNOP << (TPCInstrLayout::getLDInstrLayoutUnsafe()
                                         .at(Fields::LOAD_OPCODE)
                                         .startLLVM);
      Bits.STInst = TPCII::stNOP << (TPCInstrLayout::getSTInstrLayoutUnsafe()
                                         .at(Fields::STORE_OPCODE)
                                         .startLLVM);
      Bits.compress = 0;
    }

    uint64_t Inst = getInstrBits(MI, Fixups, STI);
    const MCInstrDesc &Desc = MCII.get(MI.getOpcode());

    if (isSingle) {
      if (isVPUInst(MI) && MI.getOpcode() == TPC::HALTv) {
        Bits.SPUInst = TPCII::spuHALT;
      }
      if (isSPUInst(MI)) {
        if (MI.getOpcode() == TPC::HALTs) {
           Bits.VPUInst = TPCII::vpuHALT;
        }
      }
    }

    if (isVPUInst(MI)) {
      Bits.VPUInst = Inst;
      if (MI.getOpcode() != TPC::NOPv || !ignoreNops) {
        Bits.hasVPU = true;
        if (isDoron1 && TPCMCInstrInfo::HasPredicate(MI, Desc, isDoron1)) {
          Bits.VPUPredicated = 1;
        }
      }
    }
    else if (isSPUInst(MI)) {
      Bits.SPUInst = Inst;
      if (MI.getOpcode() != TPC::NOPs || !ignoreNops) {
        Bits.hasSPU = true;
        if (isDoron1 && TPCMCInstrInfo::HasPredicate(MI, Desc, isDoron1)) {
          Bits.SPUPredicated = 1;
        }
      }
    }
    else if (isLoadInst(MI)) {
      Bits.LDInst = Inst;
      if (MI.getOpcode() != TPC::NOPld || !ignoreNops) {
        Bits.hasLD = true;
        if (isDoron1 && TPCMCInstrInfo::HasPredicate(MI, Desc, isDoron1)) {
            Bits.LoadPredicated = 1;
        }
      }
    }
    else if (isStoreInst(MI)) {
      if (StoreSrcC) {
        assert(isDoron1 || STI.getFeatureBits()[TPC::FeatureGaudi2]);
        const Field StoreSrcCField =
            TPCInstrLayout::getSTInstrLayout(false, STI.getFeatureBits())
                .at(STORE_SRC_C);

        APInt StoreSrcCMask = APInt(64, 0);
         StoreSrcCMask.setBits(
               StoreSrcCField.startLLVM,
               StoreSrcCField.startLLVM + StoreSrcCField.sizeBin + 1);
        StoreSrcCMask.flipAllBits();

        APInt StoreSrcCValue = APInt(64, StoreSrcC.getValue())
            .shl(StoreSrcCField.startLLVM);

        Inst &= StoreSrcCMask.getZExtValue();
        Inst |= StoreSrcCValue.getZExtValue();
      }

      Bits.STInst = Inst;
      if (MI.getOpcode() != TPC::NOPst || !ignoreNops) {
        Bits.hasST = true;
        if (isDoron1 && TPCMCInstrInfo::HasPredicate(MI, Desc, isDoron1)) {
          Bits.StorePredicated = 1;
        }
      }
    }
    else {
      llvm_unreachable("Unknown MCInst");
    }
  }
  
  // Extract Imm field
  if (TPCMCInstrInfo::hasImm(MCII, MI)) {
    if (MI.getOperand(0).isExpr()) {
       Fixups.push_back(MCFixup::create(0, MI.getOperand(0).getExpr(), FK_PCRel_4));
       Bits.Imm = 0;
       Bits.immSlotBusy = true;
    }
    else {
      int64_t opImm = 0;
      unsigned opNum = TPCMCInstrInfo::getImmFieldOpNum(MCII, MI);
      assert(MI.getOperand(opNum).isImm() || MI.getOperand(opNum).isFPImm());

      if (MI.getOperand(opNum).isImm()) {
         opImm = MI.getOperand(opNum).getImm();
      }
      else if (MI.getOperand(opNum).isFPImm()) {
         float fimm = MI.getOperand(opNum).getFPImm();
         void *Storage = static_cast<void *>(&fimm);
         opImm= *static_cast<uint64_t *>(Storage);
      }
      const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
      const MCOperandInfo &Info = Desc.OpInfo[opNum];
      if (!STI.getFeatureBits()[TPC::FeatureShortImm]
        || TPCMCInstrInfo::useImmSlotForImm(Info, opImm, TPCMCInstrInfo::isInstTypeSigned(Desc, MI))) {
        // encode immediate in the IMM slot (otherwise, it is encoded in place of the operand)
        if (Bits.immSlotBusy) {
          if ((Bits.Imm != (uint64_t)opImm) && TPCAssertOnImmConflict) {
            dbgs() << "Imm conflict: imm1(" << Bits.Imm << "), imm2(" << (uint64_t)opImm << ")\n";
            assert (0 && "Too many immediates in one bundle");
	      }
        }
        Bits.Imm = (uint64_t)opImm;
        Bits.immSlotBusy = true;
      }
    }
  }
  Bits.inited = true;
}

void TPCMCCodeEmitter::emitBits(raw_ostream& OS, TPCInstrBits *Bits, const MCSubtargetInfo &STI) const {

  CompressionType CT = Bits->compress == 1 ? CompressionType::SPU_VPU : CompressionType::LD_ST;

  TPCInstrComposer composer(Bits->SPUInst, Bits->VPUInst, Bits->LDInst, Bits->STInst, Bits->Imm,
          STI.getFeatureBits(), (Bits->compress != 0), CT,
          Bits->SPUPredicated, Bits->VPUPredicated, Bits->LoadPredicated, Bits->StorePredicated);

  APInt Instruction = composer.createBundle();

  LLVM_DEBUG( dumpTPCBundle(Instruction, STI.getFeatureBits()); );

  EmitInstruction(Instruction, (Bits->compress == 0) ? TPCII::InstructionSize : (TPCII::InstructionSize / 2), OS);
}

void TPCMCCodeEmitter::encodeInstruction(const MCInst& MI, raw_ostream& OS,
                                         SmallVectorImpl<MCFixup>& Fixups,
                                         const MCSubtargetInfo& STI) const
{
  if (isLoopInst(MI)) {
    LLVM_DEBUG( fprintf(stderr, "\n"); dumpInstrNames(MI, MCII); );
    encodeLoop(MI, OS, getInstrBits(MI, Fixups, STI), Fixups, STI);
    return;
  }

  LLVM_DEBUG( fprintf(stderr, "\n"); dumpInstrNames(MI, MCII); );

  TPCInstrBits Bits;
  fillInstrBits(MI, Fixups, Bits, STI, false, true);
  emitBits(OS, &Bits, STI);
}

#define MASK(Len) ((1<<Len)-1)

void TPCMCCodeEmitter::encodeLoop(const MCInst &Inst, raw_ostream& OS, uint64_t Bin, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo& STI) const {
  bool isDoron1 = STI.getFeatureBits()[TPC::FeatureDoron1];

  unsigned loop_start_imm_start = isDoron1 ? TPCII::Gen4LoopStartImmStart : (mayCompress(STI) ? TPCII::Gen3LoopStartImmStart    : TPCII::LoopStartImmStart);
  unsigned loop_bound_imm_start = isDoron1 ? TPCII::Gen4LoopBoundaryImmStart : (mayCompress(STI) ? TPCII::Gen3LoopBoundaryImmStart : TPCII::LoopBoundaryImmStart);
  unsigned loop_step_imm_start  = isDoron1 ? TPCII::Gen4LoopStepImmStart : (mayCompress(STI) ? TPCII::Gen3LoopStepImmStart     : TPCII::LoopStepImmStart);
  unsigned loop_offset_start    = isDoron1 ? TPCII::Gen4LoopOffsetStart : (mayCompress(STI) ? TPCII::Gen3LoopOffsetStart      : TPCII::LoopOffsetStart);
  const std::map<Fields, Field> &Layout = TPCInstrLayout::getLoopInstrLayout(
      mayCompress(STI), STI.getFeatureBits());

  APInt Instruction(TPCII::InstructionSize, 0);

  uint64_t Bits = 0;
  APInt Field;
  for (auto FieldLayout : Layout) {
    Bits = Bin >> FieldLayout.second.startLLVM;
    Bits &= MASK(FieldLayout.second.sizeBin);
    Field = APInt(TPCII::InstructionSize, Bits);
    Instruction |= Field.shl(FieldLayout.second.startBin);
  }

  if (isDoron1 &&
      TPCMCInstrInfo::HasPredicate(Inst, MCII.get(Inst.getOpcode()), isDoron1)) {
    APInt ImmValue(TPCII::InstructionSize, 1);
    Instruction |= ImmValue.shl(Layout.at(LOOP_IS_PREDICATED).startBin);
  }

  // Start immediate
  if (Inst.getOperand(0).isImm()) {
    APInt ImmValue(TPCII::ImmSize, Inst.getOperand(0).getImm());
    Instruction |= ImmValue.zext(TPCII::InstructionSize).shl(loop_start_imm_start);
  }

  // Boundary immediate
  if (Inst.getOperand(1).isImm()) {
    APInt ImmValue(TPCII::ImmSize, Inst.getOperand(1).getImm());
    Instruction |= ImmValue.zext(TPCII::InstructionSize).shl(loop_bound_imm_start);
  }

  // Step immediate
  if (Inst.getOperand(2).isImm()) {
    APInt ImmValue(TPCII::ImmSize, Inst.getOperand(2).getImm());
    Instruction |= ImmValue.zext(TPCII::InstructionSize).shl(loop_step_imm_start);
  }

  // END_PC offset
  assert (Inst.getOperand(4).isExpr() && "END_PC is not a BasicBlock");
  TPC::Fixups FixupKind = TPC::Fixups::FK_LOOP;
  Fixups.push_back(MCFixup::create(0, Inst.getOperand(4).getExpr(),
                                   MCFixupKind(FixupKind), Inst.getLoc()));
  int32_t Imm = 0;
  APInt ImmValue(TPCII::ImmSize, Imm);
  Instruction |= ImmValue.zext(TPCII::InstructionSize).shl(loop_offset_start);

  EmitInstruction(Instruction, TPCII::InstructionSize, OS);
#if defined(TPC_NOPS_AFTER_ALL) || defined(TPC_DISABLE_ALL_SCHED)
  insertNoops(1, this, OS);
#endif
}

unsigned TPCMCCodeEmitter::getRrMemoryOpValue(
    const MCInst &Inst, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &SubtargetInfo) const {
  const MCRegisterInfo * TRI = Ctx.getRegisterInfo();
  unsigned Encoding;
  const MCOperand Op1 = Inst.getOperand(OpNo + 0);
  const MCOperand Op2 = Inst.getOperand(OpNo + 1);

  assert(Op1.isReg() && "First operand is not register.");
  assert(TRI->getRegClass(TPC::SRFRegClassID).contains(Op1.getReg()) && "Register for Base must be SRF");
  Encoding = (TRI->getEncodingValue(Op1.getReg()) + 64);
  assert(Op2.isReg() && "Second operand is not register.");
  assert(TRI->getRegClass(TPC::SRFRegClassID).contains(Op2.getReg()) && "Register for Offset must be SRF");
  Encoding |= (TRI->getEncodingValue(Op2.getReg()) + 64) << 8;

  return Encoding;
}

unsigned TPCMCCodeEmitter::getRiMemoryOpValue(
    const MCInst &Inst, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &SubtargetInfo) const {
  const MCRegisterInfo * TRI = Ctx.getRegisterInfo();
  unsigned Encoding;
  const MCOperand Op1 = Inst.getOperand(OpNo + 0);
  const MCOperand Op2 = Inst.getOperand(OpNo + 1);

  assert(Op1.isReg() && "First operand is not register.");
  assert(TRI->getRegClass(TPC::SRFRegClassID).contains(Op1.getReg()) && "Register for Base must be SRF");
  Encoding = (TRI->getEncodingValue(Op1.getReg()) + 64);
  assert(Op2.isImm() && "Second operand is not immediate.");
  unsigned ImmBits;
  if (SubtargetInfo.getFeatureBits()[TPC::FeatureDoron1])
    ImmBits = 0xff;
  else
    ImmBits = 0x7f;
  Encoding |= ImmBits << 8;

  return Encoding;
}

unsigned TPCMCCodeEmitter::encodePredicate(
    const MCInst &Inst, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &SubtargetInfo) const {
  const MCRegisterInfo *TRI = Ctx.getRegisterInfo();
  const MCOperand PredReg = Inst.getOperand(OpNo + 0);
  const MCOperand Polarity = Inst.getOperand(OpNo + 1);

  assert(PredReg.isReg() && "The first operand must be a register");
  assert(Polarity.isImm() && "The second operand is not a register");
  assert((Polarity.getImm() <= 1) && "Polarity value is invalid");

  bool IsVector = TRI->getRegClass(TPC::VPRFRegClassID).contains(PredReg.getReg());
  bool IsScalar = TRI->getRegClass(TPC::SPRFRegClassID).contains(PredReg.getReg());
  (void)IsScalar;
  assert((IsVector || IsScalar) && "Predicate register must be of SPRF or VPRF class");

  // For unpredicate instruction emit SP0/VP0 register
  unsigned Encoding = 0;
  if (PredReg.getReg() == TPC::SPRF_TRUE)
    Encoding = TRI->getEncodingValue(TPC::SP0);
  else if (PredReg.getReg() == TPC::VPRF_TRUE)
    Encoding = TRI->getEncodingValue(TPC::VP0);
  else
    Encoding = TRI->getEncodingValue(PredReg.getReg());
  
  if (IsVector)
    Encoding += 16;
  Encoding |= ((Polarity.getImm() & 0x1) << 5);

  return Encoding;
}

unsigned TPCMCCodeEmitter::encodeTPCImm(
    const MCInst &Inst, unsigned OpNo,
    SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {

  const MCOperand ImmOp = Inst.getOperand(OpNo);
  assert(ImmOp.isImm() && "The operand must be an immediate");
  int64_t Imm = ImmOp.getImm();

  const MCInstrDesc &Desc = MCII.get(Inst.getOpcode());
  const MCOperandInfo &Info = Desc.OpInfo[OpNo];
  if (!STI.getFeatureBits()[TPC::FeatureShortImm]
    || TPCMCInstrInfo::useImmSlotForImm(Info, Imm, TPCMCInstrInfo::isInstTypeSigned(Desc, Inst)))
  {
    // encoded in a separate imm slot
    if (STI.getFeatureBits()[TPC::FeatureDoron1])
      return 0xff;
    else
      return 0x7f;
  }

  unsigned Encoding = ((uint64_t)Imm & 0xF);
  unsigned Prefix = 0;
  if (STI.getFeatureBits()[TPC::FeatureDoron1]) {
    Prefix = 0xf0; // 0b11110000;
    if (Encoding == 0xFu)
      Prefix = 0xE0; // 0b11100000;
  } else {
    Prefix = 0x70; // 0b01110000;
    if (Encoding == 0xFu)
      Prefix = 0x60; // 0b01100000;
    }

  return Encoding | Prefix;
}

unsigned TPCMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    const auto &IsHWMovFromToSRF = [](unsigned Opcode) {
      return Opcode == TPC::MOVhsp || Opcode == TPC::MOV_ld_hsp ||
        Opcode == TPC::MOVhsip || Opcode == TPC::MOV_ld_hsip ||
        Opcode == TPC::MOVshp || Opcode == TPC::MOV_ld_shp ||
        // Assembler Opcode
        Opcode == TPC::MOVghspDoron1 || Opcode == TPC::MOV_ld_ghspDoron1 ||
        Opcode == TPC::MOVgshpDoron1 || Opcode == TPC::MOV_ld_gshpDoron1 ||
        Opcode == TPC::MOVghspPriorDoron1 || Opcode == TPC::MOV_ld_ghspPriorDoron1 ||
        Opcode == TPC::MOVgshpPriorDoron1 || Opcode == TPC::MOV_ld_gshpPriorDoron1;
    };
    unsigned Reg = MO.getReg();
    const MCRegisterInfo * TRI = Ctx.getRegisterInfo();
    unsigned RegNo = TRI->getEncodingValue(Reg);
    bool UnifiedOpEncoding = STI.getFeatureBits()[TPC::FeatureDoron1];
    if (UnifiedOpEncoding) {
      if (TRI->getRegClass(TPC::SRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::ZRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::VRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::DRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::VPRFRegClassID).contains(Reg)) {
        return RegNo + 48;
      }
      else if (TRI->getRegClass(TPC::SPRFRegClassID).contains(Reg)) {
        return RegNo + 192;
      }
      else if (TRI->getRegClass(TPC::ARFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::IRFRegClassID).contains(Reg)) {
        return RegNo + 128;
      }
      else if (TRI->getRegClass(TPC::ADRFRegClassID).contains(Reg)) {
        return RegNo + 208;
      }
      else if (TRI->getRegClass(TPC::MRFRegClassID).contains(Reg)){
        // Process MRF separately from other HW registers. It is encoded
        // differently if it represents a mask or is a destination in MOV.
        if (IsHWMovFromToSRF(MI.getOpcode()))
          return RegNo;
        return RegNo - 4;
      }
      else if (TRI->getRegClass(TPC::HVRFDoron1RegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HSRFDoron1RegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HVPRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HSPRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      // Some fake register for Doron1
      else if (TRI->getRegClass(TPC::HWSqzCntrRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HWSqzCntrSubRegClassID).contains(Reg)) {
        return RegNo;
      }
      else {
        llvm_unreachable("Wrong register for Doron1");
      }
    } else if (isVPUInst(MI)) {
      if (TRI->getRegClass(TPC::SRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::ZRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::VRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::DRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::VPRFRegClassID).contains(Reg)) {
        return RegNo + 240;
      }
      else if (TRI->getRegClass(TPC::SPRFRegClassID).contains(Reg)) {
        return RegNo + 224;
      }
      else if (TRI->getRegClass(TPC::ARFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::IRFRegClassID).contains(Reg)) {
        return RegNo + 128;
      }
      else if (TRI->getRegClass(TPC::ADRFRegClassID).contains(Reg)) {
        return RegNo + 160;
      } else if (TRI->getRegClass(TPC::HSRFRegClassID).contains(Reg)  ||
                 TRI->getRegClass(TPC::HVRFRegClassID).contains(Reg)  ||
                 TRI->getRegClass(TPC::HVPRFRegClassID).contains(Reg) ||
                 TRI->getRegClass(TPC::HSPRFRegClassID).contains(Reg)) {
        return RegNo;
      } else {
        llvm_unreachable("Wrong register class for a vpu instruction");
      }
    }
    else if (isSPUInst(MI)) {
      if (TRI->getRegClass(TPC::SPRFRegClassID).contains(Reg)) {
        return RegNo + 48;
      }
      else if (TRI->getRegClass(TPC::SRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::ZRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::IRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::ADRFRegClassID).contains(Reg)) {
              return RegNo + 96;
      }
      else if (TRI->getRegClass(TPC::MRFRegClassID).contains(Reg)) {
        // Process MRF separately from other HW registers. It is encoded
        // differently if it represents a mask or is a destination in MOV.
        if (IsHWMovFromToSRF(MI.getOpcode()))
          return RegNo;
        return RegNo - 4;
      } else if (TRI->getRegClass(TPC::HSRFRegClassID).contains(Reg)  ||
                 TRI->getRegClass(TPC::HVRFRegClassID).contains(Reg)  ||
                 TRI->getRegClass(TPC::HVPRFRegClassID).contains(Reg) ||
                 TRI->getRegClass(TPC::HSPRFRegClassID).contains(Reg) ||
                 TRI->getRegClass(TPC::HWSqzCntrRegClassID).contains(Reg)) {
        return RegNo;
      } else {
        llvm_unreachable("Wrong register class for a spu instruction");
      }
    }
    else if (isStoreInst(MI)) {
      if (TRI->getRegClass(TPC::SRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::ZRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::VRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::DRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::VPRFRegClassID).contains(Reg)) {
        return RegNo + 240;
      }
      else if (TRI->getRegClass(TPC::SPRFRegClassID).contains(Reg)) {
        return RegNo + 224;
      }
      else if (TRI->getRegClass(TPC::IRFRegClassID).contains(Reg)) {
        return RegNo + 128;
      }
      else if (TRI->getRegClass(TPC::ADRFRegClassID).contains(Reg)) {
        return RegNo + 160;
      }
      else if (TRI->getRegClass(TPC::MRFRegClassID).contains(Reg)) {
        return RegNo - 4;
      }
      else if (TRI->getRegClass(TPC::HSRFRegClassID).contains(Reg)  ||
               TRI->getRegClass(TPC::HVRFRegClassID).contains(Reg)  ||
               TRI->getRegClass(TPC::HVPRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HSPRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HWSqzCntrRegClassID).contains(Reg)) {
        return RegNo;
      } else {
        llvm_unreachable("Wrong register class for a store instruction");
      }
    }
    else if (isLoadInst(MI)) {
      if (TRI->getRegClass(TPC::SRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::ZRFRegClassID).contains(Reg)) {
        return RegNo + 64;
      }
      else if (TRI->getRegClass(TPC::VRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::DRFRegClassID).contains(Reg)) {
        return RegNo;
      }
      else if (TRI->getRegClass(TPC::VPRFRegClassID).contains(Reg)) {
        return RegNo + 240;
      }
      else if (TRI->getRegClass(TPC::SPRFRegClassID).contains(Reg)) {
        return RegNo + 224;
      }
      else if (TRI->getRegClass(TPC::IRFRegClassID).contains(Reg)) {
        return RegNo + 128;
      }
      else if (TRI->getRegClass(TPC::ADRFRegClassID).contains(Reg)) {
        return RegNo + 160;
      } else if (TRI->getRegClass(TPC::MRFRegClassID).contains(Reg)) {
        // Process MRF separately from other HW registers. It is encoded
        // differently if it represents a mask or is a destination in MOV.
        if (IsHWMovFromToSRF(MI.getOpcode()))
          return RegNo;
        return RegNo - 4;
      }
      else if (TRI->getRegClass(TPC::HSRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HVRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HVPRFRegClassID).contains(Reg) ||
               TRI->getRegClass(TPC::HSPRFRegClassID).contains(Reg)) {
        return RegNo;
      } else {
        llvm_unreachable("Wrong register class for a load instruction");
      }
    }
    return RegNo;
  }

  if (MO.isImm())
    return MO.getImm();

  if (MO.isFPImm()) {
    float fimm = MO.getFPImm();
    // Using this two-step static_cast via void * instead of cast
    // silences a -Wstrict-aliasing false positive from GCC6 and earlier.
    // return *((unsigned*)&fimm);
    void *Storage = static_cast<void *>(&fimm);
    return *static_cast<unsigned *>(Storage);
  }

  if (MO.isExpr()) {
    Fixups.push_back(MCFixup::create(0, MO.getExpr(), FK_PCRel_4));
    return 0;
  }

  llvm_unreachable("Unable to encode MCOperand!");
}

int TPCMCCodeEmitter:: isVPUInst(const MCInst &MI) const {
  unsigned ikind = TPCMCInstrInfo::getType(MCII, MI);
  return (ikind == TPCII::TypeVPU);
}

int TPCMCCodeEmitter:: isSPUInst(const MCInst &MI) const {
  unsigned ikind = TPCMCInstrInfo::getType(MCII, MI);
  return (ikind == TPCII::TypeSPU);
}

int TPCMCCodeEmitter:: isLoadInst(const MCInst &MI) const {
  unsigned ikind = TPCMCInstrInfo::getType(MCII, MI);
  return (ikind == TPCII::TypeLOAD);
}

int TPCMCCodeEmitter:: isStoreInst(const MCInst &MI) const {
  unsigned ikind = TPCMCInstrInfo::getType(MCII, MI);
  return (ikind == TPCII::TypeSTORE);
}

int TPCMCCodeEmitter:: isLoopInst(const MCInst &MI) const {
  unsigned ikind = TPCMCInstrInfo::getType(MCII, MI);
  return (ikind == TPCII::TypeLOOP);
}

} // End of namespace llvm.

#include "TPCGenMCCodeEmitter.inc"
