// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  minihalf256 *sptr = (minihalf256 *)src;
  float256 *dptr = (float256 *)dest;
  minihalf256 src_val = *sptr;
  *dptr = convert_minihalf256_to_float256(src_val, 0);
}

// CHECK-IR: call <256 x float> @llvm.tpc.convert.v256f32.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 0, <256 x float> undef, i1 true, i1 false)
