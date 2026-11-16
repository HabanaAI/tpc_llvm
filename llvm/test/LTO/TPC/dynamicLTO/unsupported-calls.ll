; RUN: split-file %s %t
; RUN: opt -passes=link-tpc-headers -ILTO=%S/libTpcDynamicLTO.so -S %t/kept.ll | FileCheck %s
; RUN: not opt -passes=link-tpc-headers -ILTO=%S/libTpcDynamicLTO.so -S %t/mismatch.ll 2>&1 | FileCheck %s --check-prefix=MISMATCH

; Calls that -ILTO cannot replace. Each of these used to crash: the first two
; have no called function, and the library has no GAUDI_tpc_twice_Func. They
; are left as they would be without -ILTO.

;--- kept.ll
target triple = "tpc"

; CHECK-LABEL: define void @main(
; CHECK:         call void asm sideeffect "nop", ""()
; CHECK-NEXT:    call void %fp()
; CHECK-NEXT:    %r = call <64 x float> @tpc_twice(<64 x float> %v)
define void @main(ptr %fp) #0 {
entry:
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  call void asm sideeffect "nop", ""()
  call void %fp()
  %r = call <64 x float> @tpc_twice(<64 x float> %v)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %r, i32 0, i1 true, i1 false)
  ret void
}

define <64 x float> @tpc_twice(<64 x float> %x) {
  %r = fadd <64 x float> %x, %x
  ret <64 x float> %r
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

attributes #0 = { "target-cpu"="gaudi" }

;--- mismatch.ll
; The kernel declares tpc_cos_f32 with a different type from the library's.
target triple = "tpc"

; MISMATCH: LLVM ERROR: the declaration of tpc_cos_f32 does not match the -ILTO library's definition
define void @main(i32 %k) #0 {
entry:
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %r = call <64 x float> @tpc_cos_f32(<64 x float> %v, i32 %k)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %r, i32 0, i1 true, i1 false)
  ret void
}

declare <64 x float> @tpc_cos_f32(<64 x float>, i32)
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

attributes #0 = { "target-cpu"="gaudi" }
