// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o %t.s
// RUN: grep -A1 -E '^\s+loop ' %t.s | not grep -E '^\s+jmpr'

// A hardware LOOP's body must follow it: the LOOP repeats the code from its
// next VLIW to its end label. The post-RA branch folder moved a block ending
// in a LOOP after a predecessor that jumped to it and appended a jump to the
// loop body (it cannot analyze the LOOP, but added a branch after it), so
// the jump sat in the LOOP's delay slot and the loop repeated unrelated
// blocks. tpc/difftest/kernels/hwloop_layout_f32.c runs it.

#define F(V) ((V) * 0.5f + 1.0f)

#define HALF(S0, S1, S2, S3, L0, L1, L2, L3, A, E)                            \
  v_f32_st_tnsr(S0, out, F(A##0)); E##0 = v_f32_ld_tnsr_b(L0, a);             \
  v_f32_st_tnsr(S1, out, F(A##1)); E##1 = v_f32_ld_tnsr_b(L1, a);             \
  v_f32_st_tnsr(S2, out, F(A##2)); E##2 = v_f32_ld_tnsr_b(L2, a);             \
  v_f32_st_tnsr(S3, out, F(A##3)); E##3 = v_f32_ld_tnsr_b(L3, a);

// Pipelined over dim D (0 or 1, step S per vector) for the first M vectors of
// every line, moving to the next line along the other dim with delta d.
#define PIPE(D, S, OUTER0, OUTER1, OSTEP, M, START0, START1, DX, DY)          \
  {                                                                           \
    int5 c0 = 0;                                                              \
    c0[0] = START0; c0[1] = START1;                                           \
    int5 c1 = c0, c2 = c0, c3 = c0;                                           \
    c1[D] += S; c2[D] += 2 * S; c3[D] += 3 * S;                               \
    int5 c4 = c0, c5 = c1, c6 = c2, c7 = c3;                                  \
    c4[D] += 4 * S; c5[D] += 4 * S; c6[D] += 4 * S; c7[D] += 4 * S;           \
    int5 d = 0;                                                               \
    d[0] = DX; d[1] = DY;                                                     \
    float64 a0 = v_f32_ld_tnsr_b(c0, a), a1 = v_f32_ld_tnsr_b(c1, a);         \
    float64 a2 = v_f32_ld_tnsr_b(c2, a), a3 = v_f32_ld_tnsr_b(c3, a);         \
    float64 e0, e1, e2, e3;                                                   \
    for (int o = OUTER0; o < OUTER1; o += OSTEP) {                            \
      for (int i = 8; i < M; i += 8) {                                        \
        HALF(c0, c1, c2, c3, c4, c5, c6, c7, a, e)                            \
        c0[D] += 8 * S; c1[D] += 8 * S; c2[D] += 8 * S; c3[D] += 8 * S;       \
        HALF(c4, c5, c6, c7, c0, c1, c2, c3, e, a)                            \
        c4[D] += 8 * S; c5[D] += 8 * S; c6[D] += 8 * S; c7[D] += 8 * S;       \
      }                                                                       \
      HALF(c0, c1, c2, c3, c4, c5, c6, c7, a, e)                              \
      c0 += d; c1 += d; c2 += d; c3 += d;                                     \
      HALF(c4, c5, c6, c7, c0, c1, c2, c3, e, a)                              \
      c4 += d; c5 += d; c6 += d; c7 += d;                                     \
    }                                                                         \
  }

void main(tensor a, tensor out, int n) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int y0 = start[1] * n, y1 = end[1] * n;
  const int x0 = start[0] * 64, x1 = end[0] * 64;
  const int rows = y1 - y0, cols = end[0] - start[0];
  int ym = y0, xm = x0; // the pipelined part is [x0, xm) x [y0, ym)
  if (rows >= 8) {
    const int m = rows & ~7;
    ym = y0 + m;
    xm = x1;
    PIPE(1, 1, x0, x1, 64, m, x0, y0, 64, 8 - m)
  } else if (rows > 0 && cols >= 8) {
    const int m = cols & ~7;
    xm = x0 + m * 64;
    ym = y1;
    PIPE(0, 64, y0, y1, 1, m, x0, y0, 512 - m * 64, 1)
  }
  // Rows below ym in columns past xm, then every column's rows from ym.
  int5 c = 0;
  if (xm < x1 && ym > y0) {
    for (int x = xm; x < x1; x += 64) {
      c[0] = x;
      for (int y = y0; y < ym; y++) {
        c[1] = y;
        v_f32_st_tnsr(c, out, F(v_f32_ld_tnsr_b(c, a)));
      }
    }
  }
  if (ym < y1) {
    for (int x = x0; x < x1; x += 64) {
      c[0] = x;
      for (int y = ym; y < y1; y++) {
        c[1] = y;
        v_f32_st_tnsr(c, out, F(v_f32_ld_tnsr_b(c, a)));
      }
    }
  }
}
