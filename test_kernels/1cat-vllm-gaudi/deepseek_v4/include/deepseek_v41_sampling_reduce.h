// SPDX-License-Identifier: Apache-2.0
#pragma once
// Exact integer broadcast reduction, shared with the index selector's proven
// group/lane mapping. No scalar read of a vector store or own-output polling.
static inline int64 sampling_sum(int64 value) {
    value += v_i32_mov_dual_group_all_b(value,0xffffffff,1,0,3,2,MkWrA(3,3,3,3),0);
    value += v_i32_mov_dual_group_all_b(value,0xffffffff,2,3,0,1,MkWrA(3,3,3,3),0);
    value += v_i32_mov_group_b(value,0xffffffff,63,0);
    #pragma loop_unroll(3)
    for(int shift=4;shift>0;shift>>=1) {
        const uint64 mask=((((uint64)V_LANE_ID_32&7)^shift)|0x80)*0x01010101;
        value += v_i32_shuffle_b(value,*((uchar256*)&mask),0,0);
    }
    return value;
}
