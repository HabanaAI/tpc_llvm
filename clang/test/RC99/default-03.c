// RUN: %clang_cc1 -S -O2  -emit-llvm -triple tpc -std=rc99 -target-cpu gaudi2 -mllvm -tpc-trans-intr=0 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O2  -emit-llvm -triple tpc -std=rc99 -target-cpu doron1 -mllvm -tpc-trans-intr=0 %s -o - | FileCheck %s

void main(int dest, int src, int src2) {
  float128 __local *dptr = (float128 __local *)dest;
  float64 __local *sptr = (float64 __local *)src;

  float128 sum = *dptr++;
  float64 term = *sptr;
  sum = v_f32_add_x2_b(sum, term, term);
  *dptr++ = sum;

  sum = v_f32_add_x2_b(sum, term, term, 0);
  *dptr++ = sum;

  sum = v_f32_add_x2_b(sum, term, term, 0, sum);
  *dptr++ = sum;

  sum = v_f32_add_x2_b(sum, term, term, 0, sum, 0);
  *dptr++ = sum;

  sum = v_f32_add_x2_b(sum, term, term, 0, sum, 0, 1);
  *dptr++ = sum;
}

// CHECK: call <128 x float> @llvm.tpc.add.x2.v128f32.v64f32.v64f32.i1(<128 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, i8 0, i32 16, <128 x float> undef, i1 true, i1 false)
// CHECK: call <128 x float> @llvm.tpc.add.x2.v128f32.v64f32.v64f32.i1(<128 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, i8 0, i32 16, <128 x float> undef, i1 true, i1 false)
// CHECK: call <128 x float> @llvm.tpc.add.x2.v128f32.v64f32.v64f32.i1(<128 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, i8 0, i32 16, <128 x float> undef, i1 true, i1 false)
// CHECK: call <128 x float> @llvm.tpc.add.x2.v128f32.v64f32.v64f32.i1(<128 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, i8 0, i32 16, <128 x float> {{.*}}, i1 false, i1 false)
// CHECK: call <128 x float> @llvm.tpc.add.x2.v128f32.v64f32.v64f32.i1(<128 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, i8 0, i32 16, <128 x float> undef, i1 false, i1 true)

