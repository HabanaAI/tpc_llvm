// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, finish (one slot per query column, no accumulators): the normalized output of each
// column from its slot's unnormalized P.V result O and softmax sum l:  out[:, c, g] = O[:, m, s] / l[m, s]
// for the slot s of group g covering column c = qcol0 + m (slots of group -1 are padding); columns no slot
// covers are written as zeros. A member owns a 64-column block of one group.
// tensors: 0 O (256, M, S) f32  1 l (Mp, S) f32  2 slots (8, S) i32 | 3 out (256, W, G) f32
// index space: [ceil(W / 64), G]
#include "common.h"

// lane k of v broadcast to all lanes
#define GET_LANE(v, k, out)                                                                  \
    { const float lo_ = (float)(k) - 0.5f, hi_ = (float)(k) + 0.5f;                          \
      float64 t_ = v_f32_sel_less_f32_b(lanef, lo_, 0.0f, v_f32_sel_less_f32_b(lanef, hi_, v, 0.0f)); \
      NL_REDUCE_ADD(t_, IDX, out) }

void main(tensor O, tensor l, tensor slots, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(O, 1), S = get_dim_size(O, 2), W = get_dim_size(out, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const float64 zero = 0.0f;
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int g = off[1]; g < end[1]; g++) {
        for (int cb = off[0]; cb < end[0]; cb++) {
            const int c0 = cb * 64, c1 = c0 + 64 < W ? c0 + 64 : W;
            for (int c = c0; c < c1; c++) {
                int5 oc = {0, c, g, 0, 0};
                #pragma unroll
                for (int d = 0; d < 256; d += 64) {
                    oc[0] = d;
                    v_f32_st_tnsr(oc, out, zero);
                }
            }
            for (int s = 0; s < S; s++) {
                int5 fc = {0, s, 0, 0, 0};
                if (s_i32_ld_g(gen_addr(fc, slots)) != g)
                    continue;
                fc[0] = 1;
                const int q0 = s_i32_ld_g(gen_addr(fc, slots));
                const int lo = c0 > q0 ? c0 : q0, hi = c1 < q0 + M ? c1 : q0 + M;
                for (int c = lo; c < hi; c++) {
                    const int m = c - q0;
                    int5 sc = {m & ~63, s, 0, 0, 0};
                    float64 ls;
                    GET_LANE(v_f32_ld_tnsr_b(sc, l), m & 63, ls)
                    const float64 inv = nl_recip(v_f32_max_b(ls, 1.0e-30f));
                    int5 oc = {0, c, g, 0, 0}, pc = {0, m, s, 0, 0};
                    #pragma unroll
                    for (int d = 0; d < 256; d += 64) {
                        oc[0] = d;
                        pc[0] = d;
                        v_f32_st_tnsr(oc, out, v_f32_ld_tnsr_b(pc, O) * inv);
                    }
                }
            }
        }
    }
}
