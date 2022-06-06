// RUN: not %clang_cc1 -triple tpc -std=rc99 -fsyntax-only %s 2>&1 | FileCheck %s

// GAUDI-506/SW-1949

void main(int dest0, int x0, int x1) {
  bfloat128 __local *sptr1 = (bfloat128 __local *)x0;
  ushort128 __local *sptr2 = (ushort128 __local *)x1;
  ushort128 __local *dest  = (ushort128 __local *)dest;
  ushort128_bfloat128_pair_t res = {0,0};
  res = v_u16_sel2_grt_bf16_b(sptr1[0], sptr1[1], sptr2[0], sptr2[1], 0, (ushort128_bfloat128_pair_t){0}, 1, 0);
// CHECK: error: intrinsic function 'v_u16_sel2_grt_bf16_b' is not available for current processor
  *dest = temp_res0.v1;
}
