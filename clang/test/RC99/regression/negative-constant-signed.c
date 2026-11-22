// RUN: %tpc_clang -march=gaudi3 -O2 -S %s -o - | FileCheck --check-prefix=DIV %s
// RUN: %tpc_clang -march=goya -O2 -S -DNEARBYINT %s -o - | FileCheck --check-prefix=NEAR %s

// Two expansions passed a negative constant to ConstantInt::get, which takes
// an unsigned 64-bit value and asserted for a narrower type. On gaudi3 a
// vector division becomes a reciprocal, which EvalSpecialFunctionPass expands
// with a compare against 0xc0800000; on goya TPCSelectorPreshaper guards
// nearbyint with a rounding mode by a compare of the exponent against -32.

void main(tensor in, tensor out) {
  int5 c = 0;
  float64 v = v_f32_ld_tnsr_b(c, in);
#if defined(NEARBYINT)
  v_f32_st_tnsr(c, out, v_f32_nearbyint_b(v, SW_RD, 0, 1, 0));
#else
  v_f32_st_tnsr(c, out, v / (v + 1.0f));
#endif
}

// DIV:  sel_leq.i32 {{.*}}, -0x3f800000,
// NEAR: extract_exp.f32 [[E:%V[0-9]+]],
// NEAR: cmp_neq.i32 %VP{{[0-9]+}}, [[E]], -0x20
