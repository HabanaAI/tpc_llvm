// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  minihalf256 *sptr = (minihalf256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  minihalf256 src_val = *sptr;
  *dptr = convert_minihalf256_to_bfloat256(src_val, 0);
}

// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 256, <256 x bfloat> undef, i1 true, i1 false)
