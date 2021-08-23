; RUN: llc -march=tpc -mcpu=dali -O0 %s -o - | FileCheck %s

; CHECK: convert.f32 target_type=int32 rhne [[V1:%V[0-9]+]], {{%V[0-9]+}}, [[SP0:%SP[0-9]+]]
; CHECK: st_tnsr  0x1, {{%I[0-9]+}}, [[V1]], [[SP0]]
; CHECK: halt

target triple = "tpc"

; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
entry:
  %0 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %.cast = fptosi <64 x float> %0 to <64 x i32>
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 1, <64 x i32> %.cast, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #2
