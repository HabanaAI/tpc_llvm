; RUN: opt < %s -S -passes=loop-swp -loop-software-pipelining=true | FileCheck %s

; The load's coordinates start from a constant (zeroinitializer). LoopSWP
; clones the coordinate PHI into the preheader as a PHI over the
; preheader's predecessors; it used to cast the preheader value to an
; Instruction and crash.

; With a predecessor, the clone takes the constant from it.
; CHECK-LABEL: define void @preheader_with_pred(
; CHECK:       ph:
; CHECK-NEXT:    [[C:%.*]] = phi <5 x i32> [ zeroinitializer, %entry ]
; CHECK-NEXT:    call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[C]],

; The entry block has no predecessor to take it from: not pipelined.
; CHECK-LABEL: define void @entry_preheader(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label %loop

target triple = "tpc"

define void @preheader_with_pred() {
entry:
  br label %ph

ph:
  br label %loop

exit:
  ret void

loop:
  %coords = phi <5 x i32> [ %5, %loop ], [ zeroinitializer, %ph ]
  %i = phi i32 [ %i.next, %loop ], [ 0, %ph ]
  %0 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coords, i32 0, i32 0, i8 0, i32 0, <5 x i32> zeroinitializer, i1 false, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coords, i8 0, i32 0, <64 x float> zeroinitializer, i1 false, i1 false)
  %2 = fadd <64 x float> %1, zeroinitializer
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 0, <64 x float> %2, i32 0, i1 false, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> zeroinitializer, i1 false, i1 false)
  %4 = fadd <64 x float> %3, zeroinitializer
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 0, <64 x float> %4, i32 0, i1 false, i1 false)
  %5 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> zeroinitializer, i32 0, i32 0, i8 0, i32 0, <5 x i32> zeroinitializer, i1 false, i1 false)
  %i.next = add i32 %i, 1
  %done = icmp eq i32 %i.next, 0
  br i1 %done, label %exit, label %loop, !llvm.loop !0
}

define void @entry_preheader() {
entry:
  br label %loop

exit:
  ret void

loop:
  %coords = phi <5 x i32> [ %5, %loop ], [ zeroinitializer, %entry ]
  %i = phi i32 [ %i.next, %loop ], [ 0, %entry ]
  %0 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coords, i32 0, i32 0, i8 0, i32 0, <5 x i32> zeroinitializer, i1 false, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %coords, i8 0, i32 0, <64 x float> zeroinitializer, i1 false, i1 false)
  %2 = fadd <64 x float> %1, zeroinitializer
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 0, <64 x float> %2, i32 0, i1 false, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> zeroinitializer, i1 false, i1 false)
  %4 = fadd <64 x float> %3, zeroinitializer
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 0, <64 x float> %4, i32 0, i1 false, i1 false)
  %5 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> zeroinitializer, i32 0, i32 0, i8 0, i32 0, <5 x i32> zeroinitializer, i1 false, i1 false)
  %i.next = add i32 %i, 1
  %done = icmp eq i32 %i.next, 0
  br i1 %done, label %exit, label %loop, !llvm.loop !0
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #1
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #0

attributes #0 = { nounwind memory(none) }
attributes #1 = { nounwind memory(write) }

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.taken", i1 true}
!2 = !{!"llvm.loop.unroll.disable"}
