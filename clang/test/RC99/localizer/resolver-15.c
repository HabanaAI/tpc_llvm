// RUN: %tpc_clang -vlm 17 -S -O1 %s -o %t
// expected-no-diagnostics

float64 gval[64];

void main(int x, int y) {
  *(float64 __local *)x = gval[0];
  gval[0] = v_f32_lookup_v((uint64)x, 1, 1);
  *(float64 __local *)y = x;
}