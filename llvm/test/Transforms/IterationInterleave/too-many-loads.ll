; RUN: opt -S -iter-ilv < %s | FileCheck %s

target triple = "tpc"

define void @main() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %coord.011 = phi <5 x i32> [ zeroinitializer, %entry ], [ %50, %for.body ]
  %i.010 = phi i32 [ 0, %entry ], [ %inc.16, %for.body ]
  ; Ensure that every container's IF is 0
  ; CHECK: call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1
  ; CHECK-NEXT: call <64 x float> @llvm.fabs.v64f32
  ; CHECK-NEXT: call void @llvm.tpc.st.tnsr.v64f32
  ; CHECK-NEXT: call <5 x i32> @llvm.tpc.add.mask.v5i32.i32
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coord.011, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %0)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %coord.011, i8 1, <64 x float> %1, i32 0, i1 true, i1 false)
  %2 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coord.011, i32 %i.010, i32 1, i8 2, i32 0, <5 x i32> %coord.011, i1 true, i1 false)
  %inc = add nuw nsw i32 %i.010, 1
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %2, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %3)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %2, i8 1, <64 x float> %4, i32 0, i1 true, i1 false)
  %5 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %2, i32 %inc, i32 1, i8 2, i32 0, <5 x i32> %2, i1 true, i1 false)
  %inc.1 = add nuw nsw i32 %i.010, 2
  %6 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %5, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %7 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %6)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %5, i8 1, <64 x float> %7, i32 0, i1 true, i1 false)
  %8 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %5, i32 %inc.1, i32 1, i8 2, i32 0, <5 x i32> %5, i1 true, i1 false)
  %inc.2 = add nuw nsw i32 %i.010, 3
  %9 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %8, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %10 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %9)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %8, i8 1, <64 x float> %10, i32 0, i1 true, i1 false)
  %11 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %8, i32 %inc.2, i32 1, i8 2, i32 0, <5 x i32> %8, i1 true, i1 false)
  %inc.3 = add nuw nsw i32 %i.010, 4
  %12 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %11, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %13 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %12)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %11, i8 1, <64 x float> %13, i32 0, i1 true, i1 false)
  %14 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %11, i32 %inc.3, i32 1, i8 2, i32 0, <5 x i32> %11, i1 true, i1 false)
  %inc.4 = add nuw nsw i32 %i.010, 5
  %15 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %14, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %16 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %15)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %14, i8 1, <64 x float> %16, i32 0, i1 true, i1 false)
  %17 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %14, i32 %inc.4, i32 1, i8 2, i32 0, <5 x i32> %14, i1 true, i1 false)
  %inc.5 = add nuw nsw i32 %i.010, 6
  %18 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %17, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %19 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %18)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %17, i8 1, <64 x float> %19, i32 0, i1 true, i1 false)
  %20 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %17, i32 %inc.5, i32 1, i8 2, i32 0, <5 x i32> %17, i1 true, i1 false)
  %inc.6 = add nuw nsw i32 %i.010, 7
  %21 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %20, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %22 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %21)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %20, i8 1, <64 x float> %22, i32 0, i1 true, i1 false)
  %23 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %20, i32 %inc.6, i32 1, i8 2, i32 0, <5 x i32> %20, i1 true, i1 false)
  %inc.7 = add nuw nsw i32 %i.010, 8
  %24 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %25 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %24)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %23, i8 1, <64 x float> %25, i32 0, i1 true, i1 false)
  %26 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %23, i32 %inc.7, i32 1, i8 2, i32 0, <5 x i32> %23, i1 true, i1 false)
  %inc.8 = add nuw nsw i32 %i.010, 9
  %27 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %26, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %28 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %27)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %26, i8 1, <64 x float> %28, i32 0, i1 true, i1 false)
  %29 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %26, i32 %inc.8, i32 1, i8 2, i32 0, <5 x i32> %26, i1 true, i1 false)
  %inc.9 = add nuw nsw i32 %i.010, 10
  %30 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %29, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %31 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %30)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %29, i8 1, <64 x float> %31, i32 0, i1 true, i1 false)
  %32 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %29, i32 %inc.9, i32 1, i8 2, i32 0, <5 x i32> %29, i1 true, i1 false)
  %inc.10 = add nuw nsw i32 %i.010, 11
  %33 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %32, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %34 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %33)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %32, i8 1, <64 x float> %34, i32 0, i1 true, i1 false)
  %35 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %32, i32 %inc.10, i32 1, i8 2, i32 0, <5 x i32> %32, i1 true, i1 false)
  %inc.11 = add nuw nsw i32 %i.010, 12
  %36 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %35, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %37 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %36)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %35, i8 1, <64 x float> %37, i32 0, i1 true, i1 false)
  %38 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %35, i32 %inc.11, i32 1, i8 2, i32 0, <5 x i32> %35, i1 true, i1 false)
  %inc.12 = add nuw nsw i32 %i.010, 13
  %39 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %38, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %40 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %39)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %38, i8 1, <64 x float> %40, i32 0, i1 true, i1 false)
  %41 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %38, i32 %inc.12, i32 1, i8 2, i32 0, <5 x i32> %38, i1 true, i1 false)
  %inc.13 = add nuw nsw i32 %i.010, 14
  %42 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %41, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %43 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %42)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %41, i8 1, <64 x float> %43, i32 0, i1 true, i1 false)
  %44 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %41, i32 %inc.13, i32 1, i8 2, i32 0, <5 x i32> %41, i1 true, i1 false)
  %inc.14 = add nuw nsw i32 %i.010, 15
  %45 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %44, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %46 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %45)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %44, i8 1, <64 x float> %46, i32 0, i1 true, i1 false)
  %47 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %44, i32 %inc.14, i32 1, i8 2, i32 0, <5 x i32> %44, i1 true, i1 false)
  %inc.15 = add nuw nsw i32 %i.010, 16
  %48 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %47, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %49 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %48)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %47, i8 1, <64 x float> %49, i32 0, i1 true, i1 false)
  %50 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %47, i32 %inc.15, i32 1, i8 2, i32 0, <5 x i32> %47, i1 true, i1 false)
  %inc.16 = add nuw nsw i32 %i.010, 17
  %exitcond.16 = icmp eq i32 %inc.16, 68
  br i1 %exitcond.16, label %for.cond.cleanup, label %for.body
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

declare <64 x float> @llvm.fabs.v64f32(<64 x float>)

declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1)
