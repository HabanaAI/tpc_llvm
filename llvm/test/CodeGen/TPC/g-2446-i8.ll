; RUN: llc -mcpu gaudi2  %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu doron1  %s -o - | FileCheck --check-prefixes=CHECK,DORON1 %s


; ModuleID = '/tmp/a.ll-2ecf7.ll'
source_filename = "kernel"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"


; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #0

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.set.indx(<5 x i32>, i32, i32, i32, i1, i1) #1

; Function Attrs: nounwind writeonly
define void @main() local_unnamed_addr #0  {
bb:
  %i = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> zeroinitializer, i32 31, i32 0, i32 0, i1 true, i1 false)
  %i1 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %i, i32 1, i32 0, i32 0, i1 true, i1 false)
  %i2 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i1, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i3 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i1, i8 1, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i4 = mul <256 x i8> %i3, %i2
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %i1, i8 2, <256 x i8> %i4, i32 0, i1 true, i1 false)
  ret void
}



; CHECK-DAG: mul.i8   %A[[MUL:[0-9]+]], %V{{[0-9]+}}, %V{{[0-9]+}}

; GAUDI2: convert.i32 all_lanes target_type=int8 rhne %V[[STT:[0-9]+]], %A[[MUL]]

; DORON1: convert.i32 lower_lanes target_type=int8 rhne %V[[STT:[0-9]+]], %D0
; DORON1: convert.i32 upper_lanes target_type=int8 rhne %V[[STT:[0-9]+]], %D2


; CHECK: st_tnsr  0x2, %I{{[0-9]+}}, %V[[STT]]
; CHECK_NOT: convert.i32 


