; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i1 zeroext %pred) {
entry:
  %0 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 10, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = tail call f8_143 @llvm.tpc.ld.g.f8_143(i8 addrspace(3)* %0, i32 0, f8_143 0xG00, i1 %pred, i1 false)
  %2 = inttoptr i32 %dest to f8_143 addrspace(1)*
  store f8_143 %1, f8_143 addrspace(1)* %2, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)
declare f8_143 @llvm.tpc.ld.g.f8_143(i8 addrspace(3)*, i32, f8_143, i1, i1)

; CHECK-LABEL: main:
; CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK: gen_addr  dt=fp8_143 [[ADDR:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK: ld_g      %S{{[0-9]+}}, [[ADDR]]
