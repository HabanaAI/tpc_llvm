; RUN: opt -S -passes=early-cse < %s | FileCheck %s
; RUN: opt -S -passes=early-cse -mtriple=x86_64-unknown-linux-gnu < %s | FileCheck %s

; Since LLVM 22, EarlyCSE takes two identical write-only calls with nothing
; written between them for one. An RMW store reads the value it updates and a
; squeeze store depends on its squeeze counter, so neither is write-only and
; two identical ones are two stores, on any target.

target triple = "tpc"

; RMW_DT_INT32 | RMW_OP_ADD | RMW_SET (gaudi2): adds x twice.
; CHECK-LABEL: @rmw_twice(
; CHECK:         call void @llvm.tpc.st.tnsr.rmw.v64i32(
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.rmw.v64i32(
define void @rmw_twice(<64 x i32> %x) {
  call void @llvm.tpc.st.tnsr.rmw.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> %x, i32 66, i32 2, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.rmw.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> %x, i32 66, i32 2, i1 true, i1 false)
  ret void
}

; Each squeeze store writes after the data of the one before.
; CHECK-LABEL: @squeeze_twice(
; CHECK:         call void @llvm.tpc.st.tnsr.sqz.v64i32(
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.sqz.v64i32(
define void @squeeze_twice(<64 x i32> %x, <256 x i1> %p) {
  call void @llvm.tpc.st.tnsr.sqz.v64i32(<5 x i32> zeroinitializer, i8 0, i8 2, <64 x i32> %x, i32 0, <256 x i1> %p, i1 false)
  call void @llvm.tpc.st.tnsr.sqz.v64i32(<5 x i32> zeroinitializer, i8 0, i8 2, <64 x i32> %x, i32 0, <256 x i1> %p, i1 false)
  ret void
}

declare void @llvm.tpc.st.tnsr.rmw.v64i32(<5 x i32>, i8, <64 x i32>, i32, i32, i1, i1)
declare void @llvm.tpc.st.tnsr.sqz.v64i32(<5 x i32>, i8, i8, <64 x i32>, i32, <256 x i1>, i1)
