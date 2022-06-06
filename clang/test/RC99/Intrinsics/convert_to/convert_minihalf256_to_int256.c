// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  minihalf256 *sptr = (minihalf256 *)src;
  int256 *dptr = (int256 *)dest;
  minihalf256 src_val = *sptr;
  *dptr++ = convert_minihalf256_to_int256(src_val, SW_RZ);
  *dptr = convert_minihalf256_to_int256(src_val, SW_RD);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 66048, <256 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 197120, <256 x i32> undef, i1 true, i1 false)
