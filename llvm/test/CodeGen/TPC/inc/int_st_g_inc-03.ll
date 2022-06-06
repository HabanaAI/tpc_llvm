; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

define void @main(i32 %x, i1 zeroext %pred) {
entry:
  %0 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = call i8 addrspace(3)* @llvm.tpc.st.g.inc.i32(i8 addrspace(3)* %0, i32 %x, i32 4, i32 0, i1 %pred, i1 false)
  call void @llvm.tpc.st.g.i32(i8 addrspace(3)* %1, i32 %x, i32 0, i1 %pred, i1 false)
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)
declare i8 addrspace(3)* @llvm.tpc.st.g.inc.i32(i8 addrspace(3)*, i32, i32, i32, i1, i1)
declare void @llvm.tpc.st.g.i32(i8 addrspace(3)*, i32, i32, i1, i1)

; CHECK-DAG: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK-DAG: gen_addr  {{.*}}[[ADRF:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK-DAG: mov       [[PRED:%SP[0-9]+]], %S1
; CHECK:     st_g      inc_4 [[ADRF]], %S0, [[PRED]]
; CHECK:     st_g      [[ADRF]], %S0, [[PRED]]
