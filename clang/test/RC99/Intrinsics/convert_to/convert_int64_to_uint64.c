// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s


void main(int src, int dest) {
  int64 *sptr = (int64 *)src;
  uint64 *dptr = (uint64 *)dest;
  int64 src_val = *sptr;
  *dptr = convert_int64_to_uint64(src_val, 0);
}

// CHECK-IR: call <64 x i32> @llvm.tpc.convert.v64i32.v64i32.i1(<64 x i32> {{.*}}, i8 2, i32 768, <64 x i32> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v      [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.i32 all_lanes target_type=uint32 rhne %V{{[0-9]+}}, [[SRC]]
