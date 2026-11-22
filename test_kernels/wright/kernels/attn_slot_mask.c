// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, mask: the additive bf16 mask of each slot's C keys and Tq query
// tokens. Key j (position key0 + j) is visible to token t (position qpos0 + t) when
// key0 + j <= qpos0 + t and key0 + j < klen; visible keys get 0, the rest -30000. Padding slots
// (group -1) are written too.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, ...] (see attn_slot_p)
// tensors: 0 slots (8, S) i32 | 1 mask (C, Tq, 1, S) bf16
// index space: [ceil(Tq / 64), S] (64 query rows of one slot per member)
#include "common.h"

void main(tensor slots, tensor mask)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int C = get_dim_size(mask, 0), Tq = get_dim_size(mask, 1);
    // pack_sigma(a, b) stores a[i] at 2i and b[i] at 2i + 1: lane i covers keys 2i and 2i + 1
    const float64 even = 2.0f * v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
        int5 fc = {2, s, 0, 0, 0};
        const int qpos0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(fc, slots));
        for (int tb = off[0]; tb < end[0]; tb++) {
            const int t1 = tb * 64 + 64 < Tq ? tb * 64 + 64 : Tq;
            for (int t = tb * 64; t < t1; t++) {
                int lim = qpos0 + t + 1 < klen ? qpos0 + t + 1 : klen;
                lim -= key0;
                lim = lim < 0 ? 0 : (lim > C ? C : lim);
                int5 mc = {0, t, 0, s, 0};
                #pragma unroll 4
                for (int j = 0; j < C; j += 128) {
                    const float64 k = even + (float)j;
                    const float64 a = v_f32_sel_less_f32_b(k, (float)lim, 0.0f, -30000.0f);
                    const float64 b = v_f32_sel_less_f32_b(k + 1.0f, (float)lim, 0.0f, -30000.0f);
                    mc[0] = j;
                    v_bf16_st_tnsr(mc, mask, pack_sigma(a, b));
                }
            }
        }
    }
}
