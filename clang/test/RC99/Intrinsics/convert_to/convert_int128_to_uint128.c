// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  int128 *sptr = (int128 *)src;
  uint128 *dptr = (uint128 *)dest;
  int128 src_val = *sptr;
  *dptr = convert_int128_to_uint128(src_val, 0);
}

// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128i32.i1(<128 x i32> {{.*}}, i8 2, i32 768, <128 x i32> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v      %V[[SRC:[0-9]+]], %S0
// CHECK-ASM: convert.i32 all_lanes target_type=uint32 rhne %D[[SRC]], %D0
