; RUN: llc -march=tpc -mcpu=goya2 -O2 %s -o -
; XFAIL: *

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
  %1 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <128 x half> undef, i1 true, i1 false)
  %2 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> <i32 100, i32 100, i32 100, i32 100, i32 100>, i8 0, i32 0, <128 x half> undef, i1 true, i1 false)
  %3 = fdiv <128 x half> %1, %2
  tail call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> zeroinitializer, i8 1, <128 x half> %3, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128f16(<5 x i32>, i8, <128 x half>, i32, i1, i1) #2
