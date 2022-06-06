// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  uint64 *sptr = (uint64 *)src;
  int64 *dptr = (int64 *)dest;
  uint64 src_val = *sptr;
  *dptr = convert_uint64_to_int64(src_val, 0);
}

// CHECK-IR: call <64 x i32> @llvm.tpc.convert.v64i32.v64i32.i1(<64 x i32> {{.*}}, i8 3, i32 512, <64 x i32> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v      [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.u32 all_lanes target_type=int32 rhne %V{{[0-9]+}}, [[SRC]]
