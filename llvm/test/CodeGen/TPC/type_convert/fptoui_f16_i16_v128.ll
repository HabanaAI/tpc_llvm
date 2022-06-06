; RUN: llc -march=tpc -mcpu=goya2 -O0 %s -o - | FileCheck %s

; CHECK: convert.f16 all_lanes target_type=uint16 rhne [[V1:%V[0-9]+]], [[V0:%V[0-9]+]]

target triple = "tpc"

; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
entry:
  %0 = call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <128 x half> undef, i1 true, i1 false)
  %.cast = fptoui <128 x half> %0 to <128 x i16>
  call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> zeroinitializer, i8 1, <128 x i16> %.cast, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #2
