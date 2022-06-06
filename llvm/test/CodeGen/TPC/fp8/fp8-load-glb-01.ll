; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i1 zeroext %pred) {
entry:
  %0 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 10, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = bitcast i8 addrspace(3)* %0 to f8_143 addrspace(3)*
  %2 = load f8_143, f8_143 addrspace(3)* %1, align 4
  %3 = inttoptr i32 %dest to f8_143 addrspace(1)*
  store f8_143 %2, f8_143 addrspace(1)* %3, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

; CHECK-LABEL: main:
; CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK: gen_addr  dt=fp8_143 [[ADDR:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK: ld_g      %S{{[0-9]+}}, [[ADDR]]
