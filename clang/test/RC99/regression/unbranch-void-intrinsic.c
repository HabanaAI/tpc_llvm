// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// The unbranch pass predicates an intrinsic call under an if. The operand
// before the predicate is an income only for an intrinsic with a result:
// cache_invalidate has none, and was left unpredicated (running whatever the
// condition) or had its switches replaced by undef, which cannot be selected.

void main(tensor out, int n) {
  if (n)
    cache_invalidate(SW_D | SW_RST_D_PREF);
  int5 c = 0;
  v_i32_st_tnsr(c, out, n);
}

// CHECK: cmp_eq.i32 [[P:%SP[0-9]+]], %S{{[0-9]+}}, 0x0
// CHECK: cache_invalidate d rst_d_pref ![[P]]
