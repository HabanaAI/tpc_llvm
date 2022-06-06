; RUN: llc -march=tpc -mcpu=gaudi -O2 %s -o -

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> <i32 100, i32 100, i32 100, i32 100, i32 100>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %3 = fdiv <64 x float> %1, %2
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %3, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2
