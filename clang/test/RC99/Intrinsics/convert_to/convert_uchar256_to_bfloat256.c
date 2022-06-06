// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  uchar256 *sptr = (uchar256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  uchar256 src_val = *sptr;
  *dptr++ = convert_uchar256_to_bfloat256(src_val, 0);
  *dptr = convert_uchar256_to_bfloat256(src_val, SW_RD);
}

// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256i8.i1(<256 x i8> {{.*}}, i8 5, i32 256, <256 x bfloat> undef, i1 true, i1 false)
// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256i8.i1(<256 x i8> {{.*}}, i8 5, i32 196864, <256 x bfloat> undef, i1 true, i1 false)
