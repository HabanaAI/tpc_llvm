// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  char256 *sptr = (char256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  char256 src_val = *sptr;
  *dptr++ = convert_char256_to_bfloat256(src_val, 0);
  *dptr = convert_char256_to_bfloat256(src_val, SW_RD);
}

// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256i8.i1(<256 x i8> {{.*}}, i8 4, i32 256, <256 x bfloat> undef, i1 true, i1 false)
// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256i8.i1(<256 x i8> {{.*}}, i8 4, i32 196864, <256 x bfloat> undef, i1 true, i1 false)
