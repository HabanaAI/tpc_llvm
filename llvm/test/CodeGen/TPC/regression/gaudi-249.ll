; RUN: llc --enable-misched=0 -post-RA-scheduler=0 -disable-tpc-packetizer %s -o - | FileCheck %s

target triple = "tpc-none-none"

; Function Attrs: nounwind
define void @main(i32 %x, i32 %y) local_unnamed_addr {
  %ndx1 = insertelement <5 x i32> <i32 0, i32 undef, i32 0, i32 0, i32 0>, i32 %x, i32 1
  %ndx2 = insertelement <5 x i32> <i32 0, i32 undef, i32 0, i32 0, i32 0>, i32 %y, i32 1
  %value = call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %ndx1, i8 0, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %ndx2, i8 1, <256 x i8> %value, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1)

; Function Attrs: nounwind
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1)


; CHECK:     set_indx [[NDX0:%I[0-9]+]], b11101, 0x0
; CHECK-DAG: mov      b11111 {{%I[0-9]+}}, [[NDX0]]
; CHECK-DAG: set_indx [[NDX1:%I[0-9]+]], b00010, %S0
; CHECK-DAG: set_indx [[NDX2:%I[0-9]+]], b00010, %S1
; CHECK:     ld_tnsr  [[VREG:%V[0-9]+]], 0x0, [[NDX1]]
; CHECK:     st_tnsr  0x1, [[NDX2]], [[VREG]]
