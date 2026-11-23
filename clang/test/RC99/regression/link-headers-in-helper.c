// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// LinkTPCHeaders replaces get_dim_size and links the body of a vector
// division in every function, not only in main: at -O2 a helper is not inlined
// yet when it runs, and the intrinsics left in it could not be selected.

__attribute__((noinline)) static int width(tensor t) { return get_dim_size(t, 0); }
__attribute__((noinline)) static float64 ratio(float64 a, float64 b) { return a / b; }

void main(tensor in, tensor out) {
  int5 c = 0;
  c[0] = width(in);
  float64 x = v_f32_ld_tnsr_b(c, in);
  v_f32_st_tnsr(c, out, ratio(x, x + 1.0f));
}

// CHECK: ld_l mmio %S{{[0-9]+}}, 0x10
// CHECK: calc_fp_special.f32 func=recip
