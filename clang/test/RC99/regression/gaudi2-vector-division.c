// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s --check-prefix=GAUDI2
// RUN: %tpc_clang -march=gaudi2 -O0 -S %s -o - | FileCheck %s --check-prefix=GAUDI2
// RUN: %tpc_clang -march=gaudi -O2 -S %s -o - | FileCheck %s --check-prefix=GAUDI

// A vector f32 division is w * (1/v). On gaudi2 the reciprocal sets the
// result's exponent with form_fp_num's special_func mode SW_POST_RECIP, as
// Habana's compiler does; gaudi has no such mode and uses integer and select
// instructions.

void main(tensor a, tensor b, tensor out) {
  int5 c = 0;
  float64 v = v_f32_ld_tnsr_b(c, a);
  float64 w = v_f32_ld_tnsr_b(c, b);
  v_f32_st_tnsr(c, out, w / v);
}

// GAUDI2:     form_fp_num.f32 exp_add_bias force_sign0 exp_is_num
// GAUDI2-NOT: sel_
// GAUDI2:     form_fp_num.f32 exp_add_bias special_func=4
// GAUDI2-NOT: sel_
// GAUDI2:     calc_fp_special.f32 func=recip
// GAUDI2:     mul.f32

// GAUDI-NOT:  special_func
// GAUDI:      sel_leq.i32
// GAUDI:      calc_fp_special.f32 func=recip
