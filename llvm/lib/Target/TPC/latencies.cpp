//===- latencies.cpp - TPC latencies database ---------------- ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is the main database of the TPC hardware
//
//===----------------------------------------------------------------------===//

#define TPCSIM 0
#define MISSING_LATENCIES_DB_ENTRY_IS_ERROR 0
#define DEBUG_LATNECIES 0

#if TPCSIM
#include "DbgControl.h"
#define LATENCIES_DB_LOG_INFO   LOG_INFO
#define LATENCIES_DB_LOG_WARN  LOG_WARN
#define LATENCIES_DB_LOG_ERROR LOG_ERROR
#else 
#include <cstdio>
#define LATENCIES_DB_LOG_INFO   std::puts
#define LATENCIES_DB_LOG_WARN  std::puts
#define LATENCIES_DB_LOG_ERROR std::puts
#endif

#include <assert.h>
#include "latencies.h" 
#include <algorithm>
#include <sstream>
using namespace std;

namespace TPCLatencyEvaluation {

map<InstructionForLatencyDetermination, StageSopPair> latenciesDB;
map<Sop, Stage> sopDB;

//void buildInstructionLatenciesDB(uint8_t tpc_generation)
//{
    //switch (tpc_generation) {
    //case 1: // 1=TPCGenerations::DALI
        //dali_buildInstructionLatenciesDB();
        //break;
    //case 2: // 2=TPCGenerations::GAUDI
        //gaudi_buildInstructionLatenciesDB();
        //break;
    //default:
        //assert(false);
    //}
//}

uint32_t tpc_default_latency = 4;

void setDefaultLatency(uint32_t val) {
	tpc_default_latency = val;
}

uint32_t calculateLatency(InstructionForLatencyDetermination &producer, InstructionForLatencyDetermination &consumer, uint8_t tpc_generation)
{
	bool producer_isAccFp32 = producer.the_isAccFp32;
	bool consumer_isAccFp32 = consumer.the_isAccFp32;
	uint8_t producer_idxDst0 = producer.the_idxDst0;
	uint8_t consumer_idxDst0 = consumer.the_idxDst0;
	bool producer_is2xLookupAddSub = producer.the_is2xLookupAddSub;
	if(tpc_generation>1)
	{
		producer.the_isAccFp32 = 0;
		consumer.the_isAccFp32 = 0;
		producer.the_idxDst0 = 0;
		consumer.the_idxDst0 = 0;
		if((producer.the_slotID == e_issue_slot_load) && ((producer.the_opCode == 7) || (producer.the_opCode == 9)))
			producer.the_is2xLookupAddSub = 0;
	}
	if (consumer.the_operandID == e_src_lfsr_reseed) 
	{
		if (DEBUG_LATNECIES) 
			LATENCIES_DB_LOG_INFO("returning latency of 4 due to lfsr_reseed");
		return 4; //LFSR reseed always return 4
	}
	
	if (!producer.the_isLFSRImplicitDst && (latenciesDB.find(producer) == latenciesDB.end()))
	{
#if TPCSIM
		LATENCIES_DB_LOG_WARN("Warning: Producer wasn't found in latencies DB therefore returning defualt latency (4)");
#else // TPC_COMPILER
		char buff[256];
		sprintf(buff, "Warning: Producer wasn't found in latencies DB therefore returning defualt latency (%d)", tpc_default_latency);
		LATENCIES_DB_LOG_WARN(buff);
#endif
		LATENCIES_DB_LOG_WARN("please contact Ron/Hilla with this example:");
        std::stringstream ss;
		ss << "producer = " << producer;
        LATENCIES_DB_LOG_WARN(ss.str().c_str());
		if (MISSING_LATENCIES_DB_ENTRY_IS_ERROR)
		{
			assert(0);
		}

		if (tpc_generation > 1)
		{
			producer.the_isAccFp32 = producer_isAccFp32;
			consumer.the_isAccFp32 = consumer_isAccFp32;
			producer.the_idxDst0 = producer_idxDst0;
			consumer.the_idxDst0 = consumer_idxDst0;
			producer.the_is2xLookupAddSub = producer_is2xLookupAddSub;
		}
		return tpc_default_latency;
	}
	if (latenciesDB.find(consumer) == latenciesDB.end()) 
	{
#if TPCSIM
		LATENCIES_DB_LOG_WARN("Warning: Consumer wasn't found in latencies DB therefore returning defualt latency (4)");
#else // TPC_COMPILER
		char buff[256];
		sprintf(buff, "Warning: Consumer wasn't found in latencies DB therefore returning defualt latency (%d)", tpc_default_latency);
		LATENCIES_DB_LOG_WARN(buff);
#endif
		LATENCIES_DB_LOG_WARN("please contact Ron/Hilla with this example:");
                std::stringstream ss;
		ss << "consumer = " << consumer;
                LATENCIES_DB_LOG_WARN(ss.str().c_str());
		if (MISSING_LATENCIES_DB_ENTRY_IS_ERROR)
		{
			assert(0);
		}

		if (tpc_generation > 1)
		{
			producer.the_isAccFp32 = producer_isAccFp32;
			consumer.the_isAccFp32 = consumer_isAccFp32;
			producer.the_idxDst0 = producer_idxDst0;
			consumer.the_idxDst0 = consumer_idxDst0;
			producer.the_is2xLookupAddSub = producer_is2xLookupAddSub;
		}
		return tpc_default_latency;
	}

	if (producer.the_operandID != e_dst) {
		LATENCIES_DB_LOG_ERROR("Error: producer operand must be dest");
		assert(0);
		return -1;
	}

	if (consumer.the_operandID == e_dst) {
		LATENCIES_DB_LOG_ERROR("Error: consumer operand can NOT be dest");
		assert(0);
		return -1;
	}

	bool shouldOverrideMulIRFToD2 = false;
    switch (tpc_generation) {
    case 1: // 1=TPCGenerations::DALI
        shouldOverrideMulIRFToD2 = dali_overrideMulIRFToD2(producer, consumer);
        break;
    case 2: // 2=TPCGenerations::GAUDI
      shouldOverrideMulIRFToD2 = gaudi_overrideMulIRFToD2(producer, consumer);
      break;
    default:
        assert(false);
    }

	//For LFSR implicit dest, override producer stage to e4 without looking at the DB
	Stage producerStage = producer.the_isLFSRImplicitDst ? e_stage_e3 : latenciesDB[producer].first;
	consumer.the_isLFSRImplicitDst = false; //override LFSR implict dest from consumer POV as it isn't inlcuded in DB
	Sop consumerSop = latenciesDB[consumer].second;
	Stage consumerStage = shouldOverrideMulIRFToD2 ? e_stage_d2 : latenciesDB[consumer].first;

	Stage consumerSopStage = sopDB[consumerSop];
	int latency;
	if (consumerSopStage > producerStage)
		latency =  std::max(1,consumerSopStage - consumerStage + 1);
	else
	{
		latency =  std::max(1,producerStage - consumerStage + 1);
	}
		

	//special case for LOOKUP_* - latency+1 because instruction is taking 2 cycles
	if ((producer.the_slotID == e_issue_slot_load) &&
		((producer.the_opCode == 7) || (producer.the_opCode == 9)) && (producer_is2xLookupAddSub == 0))
		latency = latency + 1;

	if(tpc_generation>1)
	{
		bool consumer_is_fma_accumulator = (((consumer.the_opCode == OP_MAC) || (consumer.the_opCode == OP_MADD)) && consumer.the_operandID == e_src_c && consumer.the_isOpTypeFloat == 1) ||
			((consumer.the_opCode == OP_ADD || consumer.the_opCode == OP_SUB) && (consumer.the_isFp16 == true || consumer.the_isFp8 == true || consumer.the_is2xLookupAddSub == true) && consumer.the_operandID == e_src_a);

		bool producer_is_fma_accumulator = ((((producer.the_opCode == OP_MAC) || (producer.the_opCode == OP_MADD) || (producer.the_opCode == OP_MUL)) && producer.the_isOpTypeFloat == 1) ||
			((producer.the_opCode == OP_ADD || producer.the_opCode == OP_SUB) && (producer.the_isFp16 == true || producer.the_isFp8 == true || producer.the_is2xLookupAddSub == true))) && producer.the_operandID == e_dst;

	if (consumer_is_fma_accumulator) //bypass to MAC/MADD SRC_C - needs special treatment
	{
		//from all instructions except MAC/MUL/MADD - bypass goes to D2 and not E1, 
		//so need to increase latency by 1, 
		//most instructions already do it because they are E3 and SOP min stage input for src_c is E4, 
		//need to fix it only for instructions which are E4/E5 + UDIV_4STEP which is E6
		if (!producer_is_fma_accumulator &&
			(((producer.the_slotID == ISSUE_SLOT_ID_VPU) && (producer.the_opCode == OP_SHUFFLE ||
				producer.the_opCode == OP_PACK ||
				producer.the_opCode == OP_UNPACK || 
				producer.the_opCode == OP_MOV_GROUP ||
				producer.the_opCode == OP_MOV_DUAL_GROUP ||
				producer.the_opCode == OP_MSAC)) ||
					((producer.the_slotID == ISSUE_SLOT_ID_SPU) &&
					(producer.the_opCode == OP_UDIV_4STEP ||
					(producer.the_opCode == OP_MOV_IRF_DIM && producer.the_is2SrfDst == true)) &&
					(consumer.the_slotID == ISSUE_SLOT_ID_SPU))))
			{
				//only for FP8 lanes 2-3 (src_C taken at E1 so we have a real smapled bypass from E6)
				//if (!((producer.the_slotID == ISSUE_SLOT_ID_VPU) && (consumer.the_isFp8 == 1) && ((consumer_idxDst0 & 0x80) != 0))) 
					latency = latency + 1;
			}

		//special ugly treatment for MAC bypass from DST to SRC_C (=DST)
		if (producer_is_fma_accumulator && consumer_is_fma_accumulator && latency == 5)
		{
			// If consumer is a SPU
			bool SPUSlot = (consumer.the_slotID == ISSUE_SLOT_ID_SPU);
			// If the consumer is a VPU and their destination (even/odd) both equal.
			bool VPUSlotSameIdxDst = ((consumer.the_slotID == ISSUE_SLOT_ID_VPU) && (consumer_idxDst0 == producer_idxDst0));
			// Accumulator (for Gaudi) works with BF16 to F32 with double VRF registers begin with VRF even.
			bool atLeastOneAcc = (consumer_isAccFp32==1 || producer_isAccFp32==1);
			/*  4 cycles latency bypass works only for consumer-producer indices that are either even or odd,
				but it can't cross between the two groups.
				E.g.
					MAC.BF16.ACC_FP32 dst=V12-V13  ->  MAC.BF16.ACC_FP32 dst=V12-V13
				We have latency of 4, but for
					MAC.FP32 dst=V13  ->   MAC.BF16.ACC_FP32 dst=V12-V13
				or
					MAC.BF16.ACC_FP32 dst=V13-V14 -> MAC.BF16.ACC_FP32 dst=V14-V15
				We can't enjoy 4 cycles. Since register pairs are always allocated on even indices, we
				only have to enforce 6 cycles in case of Accfp32 with non-AccFp32.
				For convention and follow the latency at tpcsim isSameLaneByPass is true all the time. */
			bool isSameLaneBypass = (SPUSlot || VPUSlotSameIdxDst || !(atLeastOneAcc));
			if ((producer_isAccFp32 == consumer_isAccFp32) && isSameLaneBypass && (producer.the_isFp16 == consumer.the_isFp16))
				latency = 4;
			else
			{
					//if ((consumer.the_isFp8 == 1) &&
					//	((consumer_idxDst0 & 0x80) != 0)) //only for FP8 lanes 2-3 (src_C taken at E1 so we have a real smapled bypass from E6)
					//	latency = 5;
					//else
					latency = 6;
			}
	      }
	}	
    }

	if (tpc_generation > 1)
	{
		producer.the_isAccFp32 = producer_isAccFp32;
		consumer.the_isAccFp32 = consumer_isAccFp32;
		producer.the_idxDst0 = producer_idxDst0;
		consumer.the_idxDst0 = consumer_idxDst0;
		producer.the_is2xLookupAddSub = producer_is2xLookupAddSub;
	}		
	if (DEBUG_LATNECIES) {
        std::stringstream ss;
        ss << "consumer = " << consumer;
        ss << " consumerStage = " << consumerStage;
		ss << " consumerSop = " << consumerSop;
		ss << " producer = " << producer;
		ss << " producerStage = " << producerStage;
		ss << " latency = " << latency;
        LATENCIES_DB_LOG_INFO(ss.str().c_str());
	}

	return latency;
}
std::string InstructionForLatencyDetermination::str() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

} // namespace TPCLatencyEvaluation


#if 1
#else
int main() {
    using namespace TPCLatencyEvaluation;
    dali_buildInstructionLatenciesDB();
    InstructionForLatencyDetermination producer = InstructionForLatencyDetermination(e_issue_slot_spu,0,e_dst,0,0,0,0,e_rf_s);
    InstructionForLatencyDetermination consumer = InstructionForLatencyDetermination(e_issue_slot_spu,0,e_src_a,0,0,0,0,e_rf_s);
    cout << calculateLatency(producer, consumer) <<'\n';
    return 0;
}
#endif
