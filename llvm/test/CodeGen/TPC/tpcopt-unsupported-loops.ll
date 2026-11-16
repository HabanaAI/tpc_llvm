; RUN: opt < %s -passes='loop(tpcopt)' -S | FileCheck %s

; Loops that TpcLoopOpt cannot transform, each of which used to crash it. The
; pass now leaves them unchanged. The first, second and fourth come from C at
; -O2 (a loop with #pragma unroll); Habana's compiler crashes on all four.

target triple = "tpc"

; No outer loop.
; CHECK-LABEL: @no_parent_loop(
; CHECK:       loop:
; CHECK-NEXT:    %i = phi i32
; CHECK-NEXT:    %c = insertelement <5 x i32> zeroinitializer, i32 %i, i64 1
; CHECK-NEXT:    %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c,
; CHECK-NEXT:    %w = fadd <64 x float> %v, %v
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c,
define void @no_parent_loop(i32 %n) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %c = insertelement <5 x i32> zeroinitializer, i32 %i, i64 1
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %w = fadd <64 x float> %v, %v
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %w, i32 0, i1 true, i1 false)
  %i.next = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i.next, %n
  br i1 %cmp, label %loop, label %exit, !llvm.loop !0

exit:
  ret void
}

; The induction step is not a constant.
; CHECK-LABEL: @step_not_constant(
; CHECK:       inner:
; CHECK-NEXT:    %i = phi i32
; CHECK-NEXT:    %c0 = insertelement <5 x i32> zeroinitializer, i32 %i, i64 1
; CHECK-NEXT:    %c = insertelement <5 x i32> %c0, i32 %j, i64 2
; CHECK-NEXT:    %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c,
; CHECK-NEXT:    %w = fadd <64 x float> %v, %v
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c,
; CHECK-NEXT:    %i.next = add nsw i32 %i, %s
define void @step_not_constant(i32 %n, i32 %m, i32 %s) {
entry:
  br label %outer

outer:
  %j = phi i32 [ 0, %entry ], [ %j.next, %outer.latch ]
  br label %inner

inner:
  %i = phi i32 [ 0, %outer ], [ %i.next, %inner ]
  %c0 = insertelement <5 x i32> zeroinitializer, i32 %i, i64 1
  %c = insertelement <5 x i32> %c0, i32 %j, i64 2
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %w = fadd <64 x float> %v, %v
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %w, i32 0, i1 true, i1 false)
  %i.next = add nsw i32 %i, %s
  %cmp = icmp slt i32 %i.next, %n
  br i1 %cmp, label %inner, label %outer.latch, !llvm.loop !0

outer.latch:
  %j.next = add nsw i32 %j, 1
  %cmp.j = icmp slt i32 %j.next, %m
  br i1 %cmp.j, label %outer, label %exit

exit:
  ret void
}

; The induction goes into a coordinate whose index is not a constant (C rejects
; this, but Habana's front end accepts it).
; CHECK-LABEL: @index_not_constant(
; CHECK:       inner:
; CHECK-NEXT:    %i = phi i32
; CHECK-NEXT:    %c = insertelement <5 x i32> zeroinitializer, i32 %i, i32 %k
; CHECK-NEXT:    %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c,
; CHECK-NEXT:    %w = fadd <64 x float> %v, %v
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c,
define void @index_not_constant(i32 %n, i32 %m, i32 %k) {
entry:
  br label %outer

outer:
  %j = phi i32 [ 0, %entry ], [ %j.next, %outer.latch ]
  br label %inner

inner:
  %i = phi i32 [ 0, %outer ], [ %i.next, %inner ]
  %c = insertelement <5 x i32> zeroinitializer, i32 %i, i32 %k
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %w = fadd <64 x float> %v, %v
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %w, i32 0, i1 true, i1 false)
  %i.next = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i.next, %n
  br i1 %cmp, label %inner, label %outer.latch, !llvm.loop !0

outer.latch:
  %j.next = add nsw i32 %j, 1
  %cmp.j = icmp slt i32 %j.next, %m
  br i1 %cmp.j, label %outer, label %exit

exit:
  ret void
}

; The inner loop has no PHI: its exit test reads local memory.
; CHECK-LABEL: @no_phi(
; CHECK:       inner:
; CHECK-NEXT:    %x = call i32 @llvm.tpc.ld.l.i32(
; CHECK-NEXT:    %c0 = insertelement <5 x i32> zeroinitializer, i32 %x, i64 1
; CHECK-NEXT:    %c = insertelement <5 x i32> %c0, i32 %j, i64 2
; CHECK-NEXT:    %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c,
; CHECK-NEXT:    %w = fadd <64 x float> %v, %v
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c,
define void @no_phi(i32 %m) {
entry:
  br label %outer

outer:
  %j = phi i32 [ 0, %entry ], [ %j.next, %outer.latch ]
  br label %inner

inner:
  %x = call i32 @llvm.tpc.ld.l.i32(i32 0, i32 0, i32 undef, i1 true, i1 false)
  %c0 = insertelement <5 x i32> zeroinitializer, i32 %x, i64 1
  %c = insertelement <5 x i32> %c0, i32 %j, i64 2
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %w = fadd <64 x float> %v, %v
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %w, i32 0, i1 true, i1 false)
  %done = icmp eq i32 %x, 0
  br i1 %done, label %outer.latch, label %inner, !llvm.loop !0

outer.latch:
  %j.next = add nsw i32 %j, 1
  %cmp.j = icmp slt i32 %j.next, %m
  br i1 %cmp.j, label %outer, label %exit

exit:
  ret void
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1)

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
