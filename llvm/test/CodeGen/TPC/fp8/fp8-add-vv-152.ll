; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %x0, i32 %x1, i32 %dest) {
entry:
  %0 = inttoptr i32 %x0 to <256 x f8_152> addrspace(2)*
  %1 = load <256 x f8_152>, <256 x f8_152> addrspace(2)* %0, align 256
  %2 = inttoptr i32 %x1 to <256 x f8_152> addrspace(2)*
  %3 = load <256 x f8_152>, <256 x f8_152> addrspace(2)* %2, align 256
  %4 = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %5 = fadd <256 x f8_152> %1, %3
  store <256 x f8_152> %5, <256 x f8_152> addrspace(2)* %4, align 256
  ret void
}

; CHECK-DAG: ld_l_v      %V[[VAL1:[0-9]+]], %S0
; CHECK-DAG: ld_l_v      %V[[VAL2:[0-9]+]], %S1
; CHECK:     add.f8_152  %A[[RES:[0-9]+]], %A[[VAL1]], %V[[VAL2]]
; CHECK:     st_l_v      %S2, %V[[RES]]
