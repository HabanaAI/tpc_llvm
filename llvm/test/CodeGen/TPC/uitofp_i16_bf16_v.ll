; RUN: llc -march=tpc -mcpu=goya2 -O0 %s -o - | FileCheck %s

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"


; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
entry:
  %0 = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %0, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %.cast = uitofp <128 x i16> %1 to <128 x bfloat16>
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %0, i8 1, <128 x bfloat16> %.cast, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat16>, i32, i1, i1) #2

; CHECK: convert.u16 all_lanes target_type=bf16 rhne  %V{{[0-9]+}}, %V{{[0-9]+}}
; CHECK: st_tnsr  0x1, %I{{[0-9]+}}, %V{{[0-9]+}}
; CHECK: halt
