// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  uint256 *sptr = (uint256 *)src;
  int256 *dptr = (int256 *)dest;
  uint256 src_val = *sptr;
  *dptr = convert_uint256_to_int256(src_val, 0);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256i32.i1(<256 x i32> {{.*}}, i8 3, i32 512, <256 x i32> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v      [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.u32 all_lanes target_type=int32 rhne %V{{[0-9]+}}, [[SRC]]
