// RUN: not %tpc_clang -S -O1 %s -o - 2>&1 | FileCheck %s

float64 gval[65];

void main(int x, int y) {
  *(float64 __local *)x = gval[0];
  gval[0] = v_f32_lookup((uint64)x, 1, 1, 0, 1, 0);
  *(float64 __local *)y = x;
}

// CHECK: fatal error: error in backend: too much vector memory is used for statically allocated data: 16640 is allocated, but only 16384 is available