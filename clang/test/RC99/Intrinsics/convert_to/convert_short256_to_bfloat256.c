// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short256 *sptr = (short256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  short256 src_val = *sptr;
  *dptr++ = convert_short256_to_bfloat256(src_val, 0);
  *dptr = convert_short256_to_bfloat256(src_val, SW_RD);
}

// CHECK-IR: sitofp <256 x i16> {{.*}} to <256 x bfloat>
// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256i16.i1(<256 x i16> {{.*}}, i8 7, i32 196864, <256 x bfloat> undef, i1 true, i1 false)
