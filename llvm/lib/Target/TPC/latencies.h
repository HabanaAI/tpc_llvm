//===- latencies.h - TPC latencies database ---------------- ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file is the main database of the TPC hardware
//
//===----------------------------------------------------------------------===//
#ifndef TPC_LATENCIES_H
#define TPC_LATENCIES_H

#include <cstdint>
#include <map>
#include <utility>
#include <string>
#include <iostream>

//
// Some stuff copied from tpcsim's VPE_ISA_GEN.h in order to compile this file
//
#define ISSUE_SLOT_ID_LOAD   0
#define ISSUE_SLOT_ID_SPU    1
#define ISSUE_SLOT_ID_VPU    2
#define ISSUE_SLOT_ID_STORE  3

#define OP_MAC						0
#define OP_MUL						1
#define OP_ADD						2
#define OP_SUB						3
#define OP_SHUFFLE							44
#define OP_PACK								45
#define OP_UNPACK							46
#define OP_MOV_DUAL_GROUP					49
#define OP_MOV_GROUP						50
#define OP_MSAC								51
#define OP_MADD								55
#define OP_UDIV_4STEP	40
#define OP_UDIV         40
#define OP_MOV_IRF_DIM  38

#define LOAD_OP_LD_G				12
#define LOAD_OP_LD_L_V				14 
#define LOAD_OP_LD_L_V_LOW			15
#define LOAD_OP_LD_L_V_HIGH			16
#define STORE_OP_EVENT                          24
#define STORE_OP_ST_G 				6
#define STORE_OP_ST_L_V 			7
#define STORE_OP_ST_L_V_LOW                     8
#define STORE_OP_ST_L_V_HIGH                    9

#define VPE_INSTRUCTION_LATENCY_TILL_COMMIT_CLOCKS	7
#define UDIV_LATENCY_TILL_COMMIT_CLOCKS	12
//
// END OF Some stuff copied from tpcsim's VPE_ISA_GEN.h
//

using namespace std;

/*

IssueSlot the_slotID;
e_issue_slot_load  = 0
e_issue_slot_spu   = 1
e_issue_slot_vpu   = 2
e_issue_slot_store = 3

uint32_t the_opCode;
opcode of the instruction (mac/mul/add/sub/ld_tnsr/st_l_v... etc.)

OperandID the_operandID;
e_src_a = 0
e_src_b = 1
e_src_c = 2
e_src_d = 3
e_src_e = 4
e_src_p = 50
e_src_lfsr_reseed = 55
e_dst   = 60

bool the_isOpTypeFloat;
true for FP32, BF16, FP16, FP8_152, FP8_143

bool the_isIRFDest;
true if this instruction has an IRF destination

bool the_isVectorPipe;
true if this instruction has a VRF/VPRF destination
set to true only for e_src_* types, but not for e_dst

bool the_isLFSRImplicitDst;
true if this instruction updates LFSR implicitly (CONVERT with SR)

bool the_isAccFp32;
true if it is an FMA operation (MAC/MUL/MADD for any FP type, ADD/SUB only for FP16/FP8*) and accumulator is of type FP32

uint8_t the_idxDst0;
relevant only for BF16/FP16/FP8 FMA operations (MAC/MUL/ADD/SUB/MADD)
(destination register index) % (modulo)
modulo is 4 for FP8* operations, and 2 for FP16/BF16 operations
we are using it to determine whether it is same-lane-bypass (i.e. from the same FMA to the same FMA) or cross-lane-bypass (from one FMA to another)

bool the_is2SrfDst;
true if this instruction has 2 SRF destinations (UDIV/MOV_IRF_DIM with BOTH/MOV ADRF-->2XSRF).
In this case this producer instrution will not have bypasses to other instructions (i.e. longer latency)

bool the_is2xLookupAddSub;
true if instruction is LOOKUP* with X2, or ADD/SUB FP32 with X2

bool the_isFp16;
true if source operands are FP16

bool the_isFp8;
true if source operands are FP8

RegisterFile the_registerFile;
e_rf_none = 0
e_rf_v    = 1
e_rf_s    = 2
e_rf_i    = 3
e_rf_a    = 4
e_rf_sp   = 5
e_rf_vp   = 6
e_rf_imm  = 7
*/

namespace TPCLatencyEvaluation {

typedef enum _IssueSlot
{
       e_issue_slot_load  = 0,
       e_issue_slot_spu   = 1,
       e_issue_slot_vpu   = 2,
       e_issue_slot_store = 3
} IssueSlot;


typedef enum _RegisterFile
{
       e_rf_none = 0,
       e_rf_v    = 1,
       e_rf_s    = 2,
       e_rf_i    = 3,
       e_rf_a    = 4,
       e_rf_sp   = 5,
       e_rf_vp   = 6,
       e_rf_imm  = 7,
} RegisterFile;


typedef enum _Sop
{
	e_sop_load_a =  0,
	e_sop_load_ia_bypass =  1,
	e_sop_load_b =  2,
	e_sop_load_ib_bypass =  3,
	e_sop_spu_a =  4,
	e_sop_spu_b =  5,
	e_sop_spu_c =  6,
	e_sop_spu_e =  7,
	e_sop_spu_ia =  8,
	e_sop_spu_ia_bypass =  9,
	e_sop_spu_ib =  10,
	e_sop_spu_ib_bypass =  11,
	e_sop_spu_p =  12,
	e_sop_store_a =  13,
	e_sop_store_ia_bypass =  14,
	e_sop_store_b =  15,
	e_sop_store_ib_bypass =  16,
	e_sop_store_c_g =  17,
	e_sop_store_c_g_p =  18,
	e_sop_store_c_l =  19,
	e_sop_store_c_l_v =  20,
	e_sop_store_c_l_p =  21,
	e_sop_vpu_a =  22,
	e_sop_vpu_b =  23,
	e_sop_vpu_c =  24,
	e_sop_vpu_d =  25,
	e_sop_vpu_e =  26,
	e_sop_vpu_p =  27,
	e_sop_vpu_spu =  28,
	e_sop_none =  29,
} Sop;


typedef enum _Stage
{
       e_stage_d2 = 0,
       e_stage_e1 = 1,
       e_stage_e2 = 2,
       e_stage_e3 = 3,
       e_stage_e4 = 4,
       e_stage_e5 = 5,
       e_stage_e6 = 6,
       e_stage_e7 = 7,
	   e_stage_e8 = 8,
	   e_stage_e9 = 9,
	   e_stage_e10 = 10,
	   e_stage_e11 = 11
} Stage;

typedef enum _OperandID
{
    e_src_a = 0,
    e_src_b = 1,
    e_src_c = 2,
    e_src_d = 3,
    e_src_e = 4,
    e_src_p = 50,
    e_src_lfsr_reseed = 55,
    e_dst   = 60
} OperandID;


typedef struct _InstructionForLatencyDetermination
{
       _InstructionForLatencyDetermination() : the_slotID(e_issue_slot_store), the_opCode(0)
	   			, the_operandID(e_src_a), the_isOpTypeFloat(false), the_isIRFDest(false)
				, the_isVectorPipe(false), the_isLFSRImplicitDst(false), the_registerFile(e_rf_imm)
       {

       }
/*
       _InstructionForLatencyDetermination(
              IssueSlot slotID,
              uint32_t opCode,
              OperandID operandID,
              bool isOpTypeFloat,
              bool isIRFDest,
              bool isVectorPipe,
              bool isLFSRImplicitDst,
              RegisterFile registerFile
       ) : the_slotID(slotID), the_opCode(opCode), the_operandID(operandID)
	     , the_isOpTypeFloat(isOpTypeFloat), the_isIRFDest(isIRFDest)
		 , the_isVectorPipe(isVectorPipe), the_isLFSRImplicitDst(isLFSRImplicitDst)
		 , the_registerFile(registerFile)
       {

       }

       _InstructionForLatencyDetermination(
              IssueSlot slotID,
              uint32_t opCode,
              OperandID operandID,
              bool isOpTypeFloat,
              bool isIRFDest,
              bool isVectorPipe,
              bool isLFSRImplicitDst,
              bool isAccFp32,
              bool idxDst0,
              RegisterFile registerFile
       ) : the_slotID(slotID), the_opCode(opCode), the_operandID(operandID)
	   , the_isOpTypeFloat(isOpTypeFloat), the_isIRFDest(isIRFDest)
	   , the_isVectorPipe(isVectorPipe), the_isLFSRImplicitDst(isLFSRImplicitDst)
	   , the_isAccFp32(isAccFp32), the_idxDst0(idxDst0), the_registerFile(registerFile)
       {

       }	   
*/	     
       _InstructionForLatencyDetermination(
              IssueSlot slotID,
              uint32_t opCode,
              OperandID operandID,
              bool isOpTypeFloat,
              bool isIRFDest,
              bool isVectorPipe,
              bool isLFSRImplicitDst,
              bool isAccFp32,
              uint8_t idxDst0,
              bool is2SrfDst,  
              bool is2xLookupAddSub,
              bool isFp16,
		      bool isFp8,
              RegisterFile registerFile
       ) : the_slotID(slotID), the_opCode(opCode), the_operandID(operandID), 
		   the_isOpTypeFloat(isOpTypeFloat), the_isIRFDest(isIRFDest), 
		   the_isVectorPipe(isVectorPipe), the_isLFSRImplicitDst(isLFSRImplicitDst), 
		   the_isAccFp32(isAccFp32), the_idxDst0(idxDst0),
		   the_is2SrfDst(is2SrfDst), the_is2xLookupAddSub(is2xLookupAddSub), the_isFp16(isFp16), the_isFp8(isFp8),
		   the_registerFile(registerFile)
       {

       }

       IssueSlot the_slotID;
       uint32_t the_opCode;
       OperandID the_operandID;
       bool the_isOpTypeFloat;
       bool the_isIRFDest;
       bool the_isVectorPipe;
       bool the_isLFSRImplicitDst;
       bool the_isAccFp32;
       uint8_t the_idxDst0;
       bool the_is2SrfDst;
       bool the_is2xLookupAddSub;
       bool the_isFp16;
       bool the_isFp8;
       RegisterFile the_registerFile;


       friend bool operator<(const struct _InstructionForLatencyDetermination& l, const struct _InstructionForLatencyDetermination& r)
       {
              return   std::tie(l.the_slotID, l.the_opCode, l.the_operandID, l.the_isOpTypeFloat, l.the_isIRFDest, l.the_isVectorPipe, l.the_isLFSRImplicitDst, l.the_isAccFp32, l.the_idxDst0, l.the_is2SrfDst, l.the_is2xLookupAddSub, l.the_isFp16, l.the_isFp8, l.the_registerFile)
                     < std::tie(r.the_slotID, r.the_opCode, r.the_operandID, r.the_isOpTypeFloat, r.the_isIRFDest, r.the_isVectorPipe, r.the_isLFSRImplicitDst, r.the_isAccFp32, r.the_idxDst0, r.the_is2SrfDst, r.the_is2xLookupAddSub, r.the_isFp16, r.the_isFp8, r.the_registerFile); // keep the same order
       }

       friend std::ostream& operator<<(std::ostream& os,  const struct _InstructionForLatencyDetermination& r)
       {
        return os << std::dec << "slotID." << std::dec << r.the_slotID << ", opCode." << r.the_opCode << ", operandID." << r.the_operandID << ", isOpTypeFloat." << r.the_isOpTypeFloat << ", isIRFDest." << r.the_isIRFDest << ", isVectorPipe." << r.the_isVectorPipe <<  ", isLFSRImplicitDst." << r.the_isLFSRImplicitDst << ", isAccFp32." << r.the_isAccFp32 << ", isEvenIdxDst0." << +r.the_idxDst0 << ", is2SrfDst." << r.the_is2SrfDst << ", is2xLookupAddSub." << r.the_is2xLookupAddSub << ", isFp16." << r.the_isFp16 << ", isFp8." << r.the_isFp8 << ", RF." << r.the_registerFile << '\n';
       }

       _InstructionForLatencyDetermination
       operator=(const _InstructionForLatencyDetermination &other) {
         if (this != &other) {
           this->the_slotID = other.the_slotID;
           this->the_operandID = other.the_operandID;
           this->the_opCode = other.the_opCode;
           this->the_isOpTypeFloat = other.the_isOpTypeFloat;
           this->the_isIRFDest = other.the_isIRFDest;
           this->the_isVectorPipe = other.the_isVectorPipe;
           this->the_isLFSRImplicitDst = other.the_isLFSRImplicitDst;
           this->the_isAccFp32 = other.the_isAccFp32;
           this->the_idxDst0 = other.the_idxDst0;
           this->the_is2SrfDst = other.the_is2SrfDst;
           this->the_is2xLookupAddSub = other.the_is2xLookupAddSub;
           this->the_isFp16 = other.the_isFp16;
           this->the_registerFile = other.the_registerFile;
         }
         return *this;
       }

       std::string str() const;
} InstructionForLatencyDetermination;

typedef std::pair<Stage, Sop> StageSopPair;

extern std::map<InstructionForLatencyDetermination, StageSopPair> latenciesDB;
extern std::map<Sop, Stage> sopDB;

//void buildInstructionLatenciesDB(TPCGenerations tpc_generation=TPCGenerations::DALI);
void dali_buildInstructionLatenciesDB();
void gaudi_buildInstructionLatenciesDB();
bool dali_overrideMulIRFToD2(InstructionForLatencyDetermination &producer, InstructionForLatencyDetermination &consumer);
bool gaudi_overrideMulIRFToD2(InstructionForLatencyDetermination &producer, InstructionForLatencyDetermination &consumer);
uint32_t calculateLatency( InstructionForLatencyDetermination &producer
                         , InstructionForLatencyDetermination &consumer
                         , uint8_t tpc_generation=1); //1=TPCGenerations::DALI

void setDefaultLatency(uint32_t val);
} // namespace TPCLatencyEvaluation

#endif // TPC_LATENCIES_H
