// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// A verify step's acceptance on the device, and the tables of the steps that follow it: for each row b
// (rows[1, b] = 1), the accepted drafts a = the longest prefix of drafts tin[1 ..] equal to the target's
// samples y[0 ..] (y[t] is the sample after column t), then
//   acc[b] = a;
//   commit: counts[l, b] = a + 1, bases[:, l, b] = (a < rows[2, b] ? cb0 : cb1)[:, l, b] (the STATE block of
//           position pos0 + a), hidden[:, b] = (b Tq + a, a < rows[2, b] ? rows[3, b] : rows[4, b]);
//   block step at p = pos0 + a + 1: btok[b w] = banc[b] = y[a, b], bpos[b w + j] = p + j;
//   slot records (patch[:, e] = [record offset in tables, row, kind]): a context chunk (kind 0) gets
//           qpos0 = klen = p and kmin = max(0, p - window + 1); the block (kind 1) qpos0 = key0 = kmin = p and
//           klen = p + w.
// Rows with rows[1, b] = 0 are left as the host wrote them.
// tensors: 0 y (Tq, B) i32  1 tin (Tq, B) i32  2 rows (8, B) i32 [pos0, real, switch, hid0, hid1, -, -, -]
//          3 cb0 (4, L B) i32  4 cb1 (4, L B) i32  5 patch (4, Pmax) i32  6 meta (4) i32 [P, w, window, -]
//        | 7 acc (B) i32  8 bases (4, L B) i32  9 counts (L B) i32  10 hidden (2, B) i32
//          11 btok (N) i32  12 bpos (N) i32  13 banc (B) i32  14 tables (Z) i32
// (P patch entries, w block columns.) params: L. index space: [1]
#include "common.h"
#define BMAX 64
__local__ int NEXT[BMAX];  // each row's next anchor position

void main(tensor y, tensor tin, tensor rows, tensor cb0, tensor cb1, tensor patch, tensor meta, tensor acc,
          tensor bases, tensor counts, tensor hidden, tensor btok, tensor bpos, tensor banc, tensor tables,
          int L)
{
    const int Tq = get_dim_size(y, 0), B = get_dim_size(y, 1);
    scalar_cache_reset();
    int5 mc = {0, 0, 0, 0, 0};
    const int P = s_i32_ld_g(gen_addr(mc, meta));
    mc[0] = 1;
    const int w = s_i32_ld_g(gen_addr(mc, meta));
    mc[0] = 2;
    const int window = s_i32_ld_g(gen_addr(mc, meta));
    for (int b = 0; b < B; b++) {
        int5 rc = {1, b, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(rc, rows)) == 0)
            continue;
        rc[0] = 0;
        const int pos0 = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 2;
        const int sw = s_i32_ld_g(gen_addr(rc, rows));
        int a = 0;
        for (; a < Tq - 1; a++) {
            int5 yc = {a, b, 0, 0, 0}, dc = {a + 1, b, 0, 0, 0};
            if (s_i32_ld_g(gen_addr(yc, y)) != s_i32_ld_g(gen_addr(dc, tin)))
                break;
        }
        int5 yc = {a, b, 0, 0, 0};
        const int last = s_i32_ld_g(gen_addr(yc, y));
        int5 ac = {b, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(ac, acc), a);
        s_i32_st_g(gen_addr(ac, banc), last);
        const int second = a >= sw;
        for (int l = 0; l < L; l++) {
            int5 lc = {l * B + b, 0, 0, 0, 0};
            s_i32_st_g(gen_addr(lc, counts), a + 1);
            for (int f = 0; f < 4; f++) {
                int5 bc = {f, l * B + b, 0, 0, 0};
                const int v = second ? s_i32_ld_g(gen_addr(bc, cb1)) : s_i32_ld_g(gen_addr(bc, cb0));
                s_i32_st_g(gen_addr(bc, bases), v);
            }
        }
        rc[0] = second ? 4 : 3;
        const int hid = s_i32_ld_g(gen_addr(rc, rows));
        int5 hc = {0, b, 0, 0, 0};
        s_i32_st_g(gen_addr(hc, hidden), b * Tq + a);
        hc[0] = 1;
        s_i32_st_g(gen_addr(hc, hidden), hid);
        const int p = pos0 + a + 1;
        NEXT[b] = p;
        int5 tc = {b * w, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(tc, btok), last);
        for (int j = 0; j < w; j++) {
            tc[0] = b * w + j;
            s_i32_st_g(gen_addr(tc, bpos), p + j);
        }
    }
    for (int e = 0; e < P; e++) {
        int5 pc = {0, e, 0, 0, 0};
        const int off = s_i32_ld_g(gen_addr(pc, patch));
        pc[0] = 1;
        const int b = s_i32_ld_g(gen_addr(pc, patch));
        pc[0] = 2;
        const int kind = s_i32_ld_g(gen_addr(pc, patch));
        int5 rc = {1, b, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(rc, rows)) == 0)
            continue;
        const int p = NEXT[b];
        int5 fc = {off + 2, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(fc, tables), p);
        if (kind == 0) {
            fc[0] = off + 4;
            s_i32_st_g(gen_addr(fc, tables), p);
            fc[0] = off + 6;
            s_i32_st_g(gen_addr(fc, tables), p - window + 1 > 0 ? p - window + 1 : 0);
        } else {
            fc[0] = off + 3;
            s_i32_st_g(gen_addr(fc, tables), p);
            fc[0] = off + 4;
            s_i32_st_g(gen_addr(fc, tables), p + w);
            fc[0] = off + 6;
            s_i32_st_g(gen_addr(fc, tables), p);
        }
    }
}
