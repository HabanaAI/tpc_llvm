// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s --check-prefixes=GAUDI2,FULL
// RUN: %tpc_clang -march=gaudi2 -O0 -S %s -o - | FileCheck %s --check-prefixes=GAUDI2,FULL
// RUN: %tpc_clang -march=gaudi2 -O2 -S -DFAST %s -o - | FileCheck %s --check-prefixes=GAUDI2,FAST
// RUN: %tpc_clang -march=gaudi -O2 -S %s -o - | FileCheck %s --check-prefix=GAUDI

// On gaudi2, the header's v_reciprocal_f32, v_sqrt_f32, v_rsqrt_f32 and their
// fast versions do the exponent work with form_fp_num's special_func modes
// (1 before the table lookup of sqrt and rsqrt, 2, 3 and 4 after the lookups
// of sqrt, rsqrt and the reciprocal). The other architectures have no such
// modes and use extract_exp and integer and select instructions.

void main(tensor a, tensor o1, tensor o2, tensor o3) {
  int5 c = 0;
  float64 v = v_f32_ld_tnsr_b(c, a);
#ifdef FAST
  v_f32_st_tnsr(c, o1, v_reciprocal_fast_f32(v));
  v_f32_st_tnsr(c, o2, v_sqrt_fast_f32(v));
  v_f32_st_tnsr(c, o3, v_rsqrt_fast_f32(v));
#else
  v_f32_st_tnsr(c, o1, v_reciprocal_f32(v));
  v_f32_st_tnsr(c, o2, v_sqrt_f32(v));
  v_f32_st_tnsr(c, o3, v_rsqrt_f32(v));
#endif
}

// GAUDI2-NOT: extract_exp
// GAUDI2-DAG: form_fp_num.f32 special_func=1
// GAUDI2-DAG: form_fp_num.f32 special_func=2
// GAUDI2-DAG: form_fp_num.f32 special_func=3
// GAUDI2-DAG: form_fp_num.f32 exp_add_bias special_func=4
// FULL-DAG:   calc_fp_special.f32 func=recip
// FAST-NOT:   calc_fp_special
// GAUDI2-NOT: extract_exp
// GAUDI2-NOT: sel_leq.i32

// GAUDI-NOT: special_func
// GAUDI:     extract_exp.f32
// GAUDI-NOT: special_func
