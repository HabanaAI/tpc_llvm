; RUN: llc -march=tpc -mcpu=gaudib %s -o - | FileCheck %s

; CHECK: ld_tnsr  [[V0:%V[0-9]+]], 0x0, [[I2:%I[0-9]+]]
; CHECK: convert.f16 all_lanes target_type=fp32 rhne [[V1:%V[0-9]+]], [[V0]]
; CHECK: st_tnsr  0x1, [[I2]], [[V1]]
; CHECK: halt


; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <128 x half> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.convert.v64f32.v128f16.i1(<128 x half> %0, i8 11, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %1, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.convert.v64f32.v128f16.i1(<128 x half>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2
