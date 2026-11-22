// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM DSA indexer, paged: pool keys and the raw [k | gate] tail into the arena. Pool P =
// tokens 4P .. 4P + 3; its key is sum_slot softmax_slot(gate + ape[slot]) k per channel (tools/glm_np.py indexer).
// Column n of the launch, by rows (8, T) i32:
//   [pool unit (256-byte unit of the index page; -1: this token completes no pool),
//    src0, src1, src2 (the pool's first three tokens: column >= 0, or -(arena row + 1) of a stored tail),
//    tail row (first of 2 rows; -1: not stored), 0, 0, 0]
// tensors: 0 arena (512, PR, NB) u8  1 rows (8, T) i32  2 kg (256, T) f32 [k | gate]  3 ape (128, 4) f32
//        | 4 sink (64, 1) i32
// index space: [T]; member 0 writes the sink.
#include "common.h"

#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor rows, tensor kg, tensor ape, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    scalar_cache_reset();
    float64 a[4][2];
    #pragma unroll
    for (int s = 0; s < 4; s++) {
        int5 c = {0, s, 0, 0, 0};
        a[s][0] = v_f32_ld_tnsr_b(c, ape);
        c[0] = 64;
        a[s][1] = v_f32_ld_tnsr_b(c, ape);
    }
    for (int n = off[0]; n < end[0]; n++) {
        int5 rc = {0, n, 0, 0, 0};
        const int unit = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 4;
        const int tail = s_i32_ld_g(gen_addr(rc, rows));
        float64 k[4][2], g[4][2];
        {
            int5 c = {0, n, 0, 0, 0};
            k[3][0] = v_f32_ld_tnsr_b(c, kg); c[0] = 64;
            k[3][1] = v_f32_ld_tnsr_b(c, kg); c[0] = 128;
            g[3][0] = v_f32_ld_tnsr_b(c, kg); c[0] = 192;
            g[3][1] = v_f32_ld_tnsr_b(c, kg);
        }
        if (tail >= 0) {
            int5 dc = {0, ROW(tail), 0, 0};
            v_f8_st_tnsr(dc, arena, __builtin_bit_cast(minifloat256, k[3][0]));
            dc[0] = 256;
            v_f8_st_tnsr(dc, arena, __builtin_bit_cast(minifloat256, k[3][1]));
            int5 dd = {0, ROW(tail + 1), 0, 0};
            v_f8_st_tnsr(dd, arena, __builtin_bit_cast(minifloat256, g[3][0]));
            dd[0] = 256;
            v_f8_st_tnsr(dd, arena, __builtin_bit_cast(minifloat256, g[3][1]));
        }
        if (unit >= 0) {
            for (int s = 0; s < 3; s++) {
                rc[0] = 1 + s;
                const int src = s_i32_ld_g(gen_addr(rc, rows));
                if (src >= 0) {
                    int5 c = {0, src, 0, 0, 0};
                    k[s][0] = v_f32_ld_tnsr_b(c, kg); c[0] = 64;
                    k[s][1] = v_f32_ld_tnsr_b(c, kg); c[0] = 128;
                    g[s][0] = v_f32_ld_tnsr_b(c, kg); c[0] = 192;
                    g[s][1] = v_f32_ld_tnsr_b(c, kg);
                } else {
                    const int r = -src - 1;
                    int5 c = {0, ROW(r), 0, 0};
                    k[s][0] = __builtin_bit_cast(float64, v_f8_ld_tnsr_b(c, arena)); c[0] = 256;
                    k[s][1] = __builtin_bit_cast(float64, v_f8_ld_tnsr_b(c, arena));
                    int5 d = {0, ROW(r + 1), 0, 0};
                    g[s][0] = __builtin_bit_cast(float64, v_f8_ld_tnsr_b(d, arena)); d[0] = 256;
                    g[s][1] = __builtin_bit_cast(float64, v_f8_ld_tnsr_b(d, arena));
                }
            }
            float64 o[2];
            #pragma unroll 1
            for (int hh = 0; hh < 2; hh++) {
                float64 l[4];
                #pragma unroll
                for (int s = 0; s < 4; s++) l[s] = g[s][hh] + a[s][hh];
                const float64 mx = v_f32_max_b(v_f32_max_b(l[0], l[1]), v_f32_max_b(l[2], l[3]));
                float64 sum = 0.0f, acc = 0.0f;
                #pragma unroll
                for (int s = 0; s < 4; s++) {
                    const float64 e = nl_exp(l[s] - mx);
                    sum += e;
                    acc = v_f32_mac_b(e, k[s][hh], acc);
                }
                o[hh] = acc * nl_recip(sum);
            }
            const bfloat128 b = cvt_bf16_lin(o[0], o[1]);
            int5 dc = {(unit & 1) * 256, ROW(unit >> 1), 0, 0};
            v_f8_st_tnsr(dc, arena, __builtin_bit_cast(minifloat256, b));
        }
    }
    if (off[0] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
