; RUN: llc -march=tpc -mcpu=gaudib %s -o - | FileCheck %s

; CHECK: ld_tnsr  [[V0:%V[0-9]+]], 0x0, [[I2:%I[0-9]+]]
; CHECK: ld_tnsr  [[V1:%V[0-9]+]], 0x1, [[I2]]
; CHECK: mul.f16  [[V2:%V[0-9]+]], [[V0]], [[V1]]
; CHECK: st_tnsr  0x2, [[I2]], [[V2]]
; CHECK: halt

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <128 x half> undef, i1 true, i1 false)
  %1 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> zeroinitializer, i8 1, i32 0, <128 x half> undef, i1 true, i1 false)
  %2 = tail call <128 x half> @llvm.tpc.mul.v128f16.v128f16.v128f16.i1(<128 x half> %0, <128 x half> %1, i8 11, i32 0, <128 x half> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> zeroinitializer, i8 2, <128 x half> %2, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.mul.v128f16.v128f16.v128f16.i1(<128 x half>, <128 x half>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128f16(<5 x i32>, i8, <128 x half>, i32, i1, i1) #2
