; RUN: opt -S -passes='loop-mssa(licm)' < %s | FileCheck %s

; Since LLVM 22, LICM moves a loop-invariant write-only call out of its loop,
; which runs it once instead of on every iteration. TPC store intrinsics are
; write-only, but an RMW store adds to memory, so on TPC these calls stay in
; the loop, as with LLVM 12 and Habana's compiler.

target triple = "tpc"

; RMW_DT_INT32 | RMW_OP_ADD | RMW_SET (gaudi2): adds 1 on every iteration.
; CHECK-LABEL: @rmw_add(
; CHECK:       loop:
; CHECK-NEXT:    %i = phi
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.rmw.v64i32(
define void @rmw_add(i32 %n) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  call void @llvm.tpc.st.tnsr.rmw.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> splat (i32 1), i32 258, i32 0, i1 true, i1 false)
  %i.next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %i.next, %n
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

; A plain store would give the same result moved, but Habana keeps it too.
; CHECK-LABEL: @plain_store(
; CHECK:       loop:
; CHECK-NEXT:    %i = phi
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v64i32(
define void @plain_store(i32 %n) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> splat (i32 1), i32 0, i1 true, i1 false)
  %i.next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %i.next, %n
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

declare void @llvm.tpc.st.tnsr.rmw.v64i32(<5 x i32>, i8, <64 x i32>, i32, i32, i1, i1)
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1)
