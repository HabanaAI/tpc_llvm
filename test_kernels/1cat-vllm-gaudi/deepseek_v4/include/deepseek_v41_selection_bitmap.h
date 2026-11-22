// SPDX-License-Identifier: Apache-2.0
#pragma once
static inline uint64 pack_half(bool64 predicate) {
    const uint64 lanes = (uint64)V_LANE_ID_32;
    uint64 bits = v_u32_mov_vb((uint64)1 << (lanes & 31), 0, (uint64)0, predicate, 0);
    bits |= v_u32_mov_dual_group_all_b(
        bits, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0);
    bits |= v_u32_mov_group_b(bits, 0xffffffff, 63, 0);
    #pragma loop_unroll(3)
    for (int shift = 4; shift > 0; shift >>= 1) {
        const uint64 mask = (((lanes & 7) ^ shift) | 0x80) * 0x01010101;
        bits |= v_u32_shuffle_b(bits, *((const uchar256*)&mask), 0, 0);
    }
    return bits;
}

static inline uint64 pack_words(uint64 first, uint64 second) {
    uint64 words = v_u32_mov_dual_group_b(first, 0x000000f0, 2, 0, SW_WR_LOWER_GROUP, first);
    words = v_u32_mov_dual_group_b(second, 0x00000f00, 0, 0, SW_WR_LOWER_GROUP, words);
    return v_u32_mov_dual_group_b(second, 0x0000f000, 2, 0, SW_WR_LOWER_GROUP, words);
}
