; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_143 %fp) {
entry:

  %0 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 10, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = bitcast i8 addrspace(3)* %0 to f8_143 addrspace(3)*
  store f8_143 %fp, f8_143 addrspace(3)* %1, align 4

  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; CHECK-LABEL: main:
; CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK: gen_addr  dt=fp8_143 [[ADDR:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK: st_g      [[ADDR]], %S0
