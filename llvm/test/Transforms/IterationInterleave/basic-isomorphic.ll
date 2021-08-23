; RUN: opt -S -iter-ilv -iter-ilv-compute-IF=4 -iter-ilv-store-IF=4 < %s | FileCheck %s

target triple = "tpc"

define void @main() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %coord.011 = phi <5 x i32> [ zeroinitializer, %entry ], [ %11, %for.body ]
  %i.010 = phi i32 [ 0, %entry ], [ %inc.3, %for.body ]
  ; CHECK: call <64 x float> @llvm.fabs.v64f32
  ; CHECK-NEXT: call <64 x float> @llvm.fabs.v64f32
  ; CHECK-NEXT: call <64 x float> @llvm.fabs.v64f32
  ; CHECK-NEXT: call <64 x float> @llvm.fabs.v64f32
  ; CHECK: call void @llvm.tpc.st.tnsr.v64f32
  ; CHECK-NEXT: call void @llvm.tpc.st.tnsr.v64f32
  ; CHECK-NEXT: call void @llvm.tpc.st.tnsr.v64f32
  ; CHECK-NEXT: call void @llvm.tpc.st.tnsr.v64f32
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coord.011, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %0)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %coord.011, i8 1, <64 x float> %1, i32 0, i1 true, i1 false)
  %2 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coord.011, i32 %i.010, i32 1, i8 2, i32 0, <5 x i32> %coord.011, i1 true, i1 false)
  %inc = or i32 %i.010, 1
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %2, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %3)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %2, i8 1, <64 x float> %4, i32 0, i1 true, i1 false)
  %5 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %2, i32 %inc, i32 1, i8 2, i32 0, <5 x i32> %2, i1 true, i1 false)
  %inc.1 = or i32 %i.010, 2
  %6 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %5, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %7 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %6)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %5, i8 1, <64 x float> %7, i32 0, i1 true, i1 false)
  %8 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %5, i32 %inc.1, i32 1, i8 2, i32 0, <5 x i32> %5, i1 true, i1 false)
  %inc.2 = or i32 %i.010, 3
  %9 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %8, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %10 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %9)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %8, i8 1, <64 x float> %10, i32 0, i1 true, i1 false)
  %11 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %8, i32 %inc.2, i32 1, i8 2, i32 0, <5 x i32> %8, i1 true, i1 false)
  %inc.3 = add nuw nsw i32 %i.010, 4
  %exitcond.3 = icmp eq i32 %inc.3, 16
  br i1 %exitcond.3, label %for.cond.cleanup, label %for.body
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

declare <64 x float> @llvm.fabs.v64f32(<64 x float>)

declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1)
