; RUN: opt -S -iter-ilv < %s | FileCheck %s

define void @main() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %coord.043 = phi <5 x i32> [ zeroinitializer, %entry ], [ %25, %for.body ]
  %i.042 = phi i32 [ 0, %entry ], [ %inc.1, %for.body ]
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coord.043, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coord.043, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coord.043, i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %0, <64 x float> %1, i8 0, i32 0, <64 x float> %1, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %0, <64 x float> %1, i8 0, i32 0, <64 x float> %2, i1 true, i1 false)
  ; Ensure that compute container's IF is 0
  ; CHECK: fmul <64 x float>
  ; CHECK-NEXT: call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1
  %5 = fmul <64 x float> %1, %4
  %6 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %5, <64 x float> %0, i8 0, i32 0, <64 x float> %0, i1 true, i1 false)
  %7 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %6, <64 x float> %0, i8 0, i32 0, <64 x float> %1, i1 true, i1 false)
  %8 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %7, <64 x float> %7, i8 0, i32 0, <64 x float> %7, i1 true, i1 false)
  %9 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %8, <64 x float> %0, i8 0, i32 0, <64 x float> %8, i1 true, i1 false)
  %10 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %3, <64 x float> %0, i8 0, i32 0, <64 x float> %1, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %coord.043, i8 3, <64 x float> %10, i32 0, i1 true, i1 false)
  %11 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coord.043, i32 %i.042, i32 1, i8 2, i32 0, <5 x i32> %coord.043, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %11, i8 3, <64 x float> %9, i32 0, i1 true, i1 false)
  %12 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %11, i32 %i.042, i32 1, i8 2, i32 0, <5 x i32> %11, i1 true, i1 false)
  %inc = or i32 %i.042, 1
  %13 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %12, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %14 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %12, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %15 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %12, i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
  %16 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %13, <64 x float> %14, i8 0, i32 0, <64 x float> %14, i1 true, i1 false)
  %17 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %13, <64 x float> %14, i8 0, i32 0, <64 x float> %15, i1 true, i1 false)
  %18 = fmul <64 x float> %14, %17
  %19 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %18, <64 x float> %13, i8 0, i32 0, <64 x float> %13, i1 true, i1 false)
  %20 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %19, <64 x float> %13, i8 0, i32 0, <64 x float> %14, i1 true, i1 false)
  %21 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %20, <64 x float> %20, i8 0, i32 0, <64 x float> %20, i1 true, i1 false)
  %22 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %21, <64 x float> %13, i8 0, i32 0, <64 x float> %21, i1 true, i1 false)
  %23 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %16, <64 x float> %13, i8 0, i32 0, <64 x float> %14, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %12, i8 3, <64 x float> %23, i32 0, i1 true, i1 false)
  %24 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %12, i32 %inc, i32 1, i8 2, i32 0, <5 x i32> %12, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %24, i8 3, <64 x float> %22, i32 0, i1 true, i1 false)
  %25 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %24, i32 %inc, i32 1, i8 2, i32 0, <5 x i32> %24, i1 true, i1 false)
  %inc.1 = add nuw nsw i32 %i.042, 2
  %exitcond.1 = icmp eq i32 %inc.1, 8
  br i1 %exitcond.1, label %for.cond.cleanup, label %for.body
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1)
