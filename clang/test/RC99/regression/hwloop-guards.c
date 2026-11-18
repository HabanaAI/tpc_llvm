// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=1 %s -o - | FileCheck %s --check-prefix=IDX
// RUN: %tpc_clang -S -O2 -march=gaudi -DP=1 %s -o - | FileCheck %s --check-prefix=IDX
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=1 -mllvm -enable-tpc-optimize-hw-loops-guards=false %s -o - | FileCheck %s --check-prefix=OFF
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=2 %s -o - | FileCheck %s --check-prefix=SUM
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=3 %s -o - | FileCheck %s --check-prefix=IF
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=4 %s -o - | FileCheck %s --check-prefix=STORE
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=5 %s -o - | FileCheck %s --check-prefix=LONG
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=6 %s -o - | FileCheck %s --check-prefix=LOAD
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DP=7 %s -o - | FileCheck %s --check-prefix=LFSR

// A LOOP instruction checks its condition before the first iteration, so the
// jump that skips a rotated loop when it runs zero times is not needed.
// TPCOptimizeHWLoopsGuards removes it, as Habana's compiler does, when the
// code it would skip has no side effects. A jump on another condition becomes
// the LOOP's predicate when it skips at most 10 instructions.

void main(tensor in, tensor out, int n, int k, int a, int b) {
  int5 c = {0, 0, 0, 0, 0};
#if P == 1
  // Index-space loops: the setup of the inner loops runs before the outer
  // LOOP, and the guards compare IRF dimensions.
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  for (int z = start[2]; z < end[2]; z++) {
    c[2] = z;
    for (int y = start[1]; y < end[1]; y++) {
      c[1] = y;
      for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
        c[0] = x;
        float64 v = v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0);
        v_f32_st_tnsr(c, out, v + v, 0, 1, 0);
      }
    }
  }
#elif P == 2
  // The sum is used after the loop: when the loop does not run, the exit
  // takes the initial value from the LOOP's block.
  float64 s = 1.0f;
  for (int x = 0; x < n; x += 64) {
    c[0] = x;
    s = s + v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0);
  }
  c[0] = 0;
  v_f32_st_tnsr(c, out, s, 0, 1, 0);
#elif P == 3
  if (k != 0) {
    for (int x = 0; x < n; x += 64) {
      c[0] = x;
      v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0), 0, 1, 0);
    }
  }
#elif P == 4
  // The guard skips a store, which must not happen when n <= 0. Habana's
  // compiler removes this guard and stores anyway.
  if (n > 0) {
    v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0), 0, 1, 0);
    for (int x = 0; x < n; x += 64) {
      c[0] = x;
      c[1] = 1;
      v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0), 0, 1, 0);
    }
  }
#elif P == 5
  // Too much code under 'k != 0' to run it when k is 0.
  if (k != 0) {
    int t = a * b + 3; t = t * a - b; t = (t << 3) ^ a; t = t * t + b;
    t = t - a * 7; t = t * b + 1; t = (t >> 2) + a; t = t * 5 - b;
    t = t ^ (a * b); t = t + 11 * a; t = t * 3;
    for (int x = 0; x < n; x += 64) {
      c[0] = x;
      v_i32_st_tnsr(c, out, (int64)t + x, 0, 1, 0);
    }
  }
#elif P == 6
  // LICM puts the row's load under the inner loop's guard; a tensor load may
  // run although the loop does not.
  int5 z = {0, 0, 0, 0, 0};
  for (int y = 0; y < k; y++) {
    c[1] = y;
    z[1] = y;
    float64 r = v_f32_ld_tnsr_b(z, in, 0, 0, 1, 0);
    for (int x = 0; x < n; x += 64) {
      c[0] = x;
      v_f32_st_tnsr(c, out, r + v_f32_ld_tnsr_b(c, in, 0, 0, 1, 0), 0, 1, 0);
    }
  }
#elif P == 7
  // Reading the LFSR advances it: when n <= 0 the read after the 'if' must
  // get the first value. Habana's compiler reads it anyway.
  if (n > 0) {
    uint64 r = (uint64)read_lfsr_b();
    for (int x = 0; x < n; x += 64) {
      c[0] = x;
      v_u32_st_tnsr(c, out, r, 0, 1, 0);
    }
  }
  c[1] = 1;
  v_u32_st_tnsr(c, out, (uint64)read_lfsr_b(), 0, 1, 0);
#endif
}

// IDX:     main:
// IDX-NOT: {{cmp_|jmpr}}
// IDX:     loop %S{{[0-9]+}}, %S{{[0-9]+}}, 1, <, .LBB0_{{[0-9]+}}{{$}}
// IDX-NOT: {{cmp_|jmpr}}
// IDX:     loop %S{{[0-9]+}}, %S{{[0-9]+}}, 1, <, .LBB0_{{[0-9]+}}{{$}}
// IDX-NOT: {{cmp_|jmpr}}
// IDX:     loop %S{{[0-9]+}}, %S{{[0-9]+}}, 64, <, .LBB0_{{[0-9]+}}{{$}}
// IDX-NOT: {{cmp_|jmpr}}
// IDX:     halt

// OFF:     cmp_geq.i32 b00100 %SP{{[0-9]+}}
// OFF:     jmpr
// OFF:     loop

// SUM:     main:
// SUM-NOT: jmpr
// SUM:     loop 0, %S{{[0-9]+}}, 64, <
// SUM-NOT: jmpr
// SUM:     halt

// IF:      cmp_eq.i32 %SP[[P:[0-9]+]], %S{{[0-9]+}}, 0x0
// IF-NOT:  jmpr
// IF:      loop 0, %S{{[0-9]+}}, 64, <, .LBB0_{{[0-9]+}}, !%SP[[P]]
// IF-NOT:  jmpr
// IF:      halt

// STORE:   cmp_less.i32 %SP[[P:[0-9]+]], %S{{[0-9]+}}, 0x1
// STORE:   jmpr .LBB0_{{[0-9]+}}, %SP[[P]]
// STORE:   st_tnsr
// STORE:   loop 0, %S{{[0-9]+}}, 64, <, .LBB0_{{[0-9]+}}{{$}}

// LONG:     cmp_eq.i32 %SP[[P:[0-9]+]], %S{{[0-9]+}}, 0x0
// LONG:     jmpr .LBB0_{{[0-9]+}}, %SP[[P]]
// LONG-NOT: {{cmp_|jmpr}}
// LONG:     loop 0, %S{{[0-9]+}}, 64, <, .LBB0_{{[0-9]+}}{{$}}

// LOAD:     main:
// LOAD-NOT: jmpr
// LOAD:     loop 0, %S{{[0-9]+}}, 1, <
// LOAD-NOT: jmpr
// LOAD:     ld_tnsr
// LOAD-NOT: jmpr
// LOAD:     loop 0, %S{{[0-9]+}}, 64, <
// LOAD-NOT: jmpr
// LOAD:     halt

// LFSR:     cmp_less.i32 %SP[[P:[0-9]+]], %S{{[0-9]+}}, 0x1
// LFSR:     jmpr .LBB0_{{[0-9]+}}, %SP[[P]]
// LFSR:     READ %V{{[0-9]+}}, LFSR
// LFSR:     loop 0, %S{{[0-9]+}}, 64, <
