; RUN: opt < %s -passes=cast-swizzle-opt -S | FileCheck %s

; Cast pairs that have no llvm.tpc.convert form are left unchanged. The switch
; table has no i32/f16 entries (this one comes from C: int -> half -> int), so
; createConvertIntrinsic dereferenced a null entry. A *_swch switch that is not
; a constant was ORed in as -1, which set every switch bit.

target triple = "tpc"

; CHECK-LABEL: @int_half_int(
; CHECK-NEXT:    %h = sitofp i32 %a to half
; CHECK-NEXT:    %b = fptosi half %h to i32
; CHECK-NOT:     llvm.tpc.convert
define void @int_half_int(i32 %a) {
  %h = sitofp i32 %a to half
  %b = fptosi half %h to i32
  %v.0 = insertelement <64 x i32> poison, i32 %b, i64 0
  %v = shufflevector <64 x i32> %v.0, <64 x i32> poison, <64 x i32> zeroinitializer
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> %v, i32 0, i1 true, i1 false)
  ret void
}

; CHECK-LABEL: @switch_not_constant(
; CHECK-NEXT:    %f = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %x, i32 %sw)
; CHECK-NEXT:    %y = call <64 x i32> @llvm.tpc.fptosi.swch.v64i32.v64f32(<64 x float> %f, i32 0)
; CHECK-NOT:     llvm.tpc.convert
define void @switch_not_constant(<64 x i32> %x, i32 %sw) {
  %f = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %x, i32 %sw)
  %y = call <64 x i32> @llvm.tpc.fptosi.swch.v64i32.v64f32(<64 x float> %f, i32 0)
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 0, <64 x i32> %y, i32 0, i1 true, i1 false)
  ret void
}

declare <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32>, i32)
declare <64 x i32> @llvm.tpc.fptosi.swch.v64i32.v64f32(<64 x float>, i32)
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1)
