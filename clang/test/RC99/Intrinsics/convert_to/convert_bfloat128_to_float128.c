// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat128 *sptr = (bfloat128 *)src;
  float128 *dptr = (float128 *)dest;
  bfloat128 src_val = *sptr;
  *dptr = convert_bfloat128_to_float128(src_val, 0);
}

// CHECK-IR: call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1({{.*}}, i8 1, i32 0, <128 x float> undef, i1 true, i1 false)
