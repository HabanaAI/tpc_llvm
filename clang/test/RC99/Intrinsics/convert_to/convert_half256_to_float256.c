// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  half256 *sptr = (half256 *)src;
  float256 *dptr = (float256 *)dest;
  half256 src_val = *sptr;
  *dptr = convert_half256_to_float256(src_val, 0);
}

// CHECK-IR: call <256 x float> @llvm.tpc.convert.v256f32.v256f16.i1(<256 x half> {{.*}}, i8 11, i32 0, <256 x float> undef, i1 true, i1 false)
