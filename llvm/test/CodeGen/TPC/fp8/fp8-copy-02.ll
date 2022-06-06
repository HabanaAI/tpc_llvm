; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, f8_152 %fp) {
entry:

  %dptr = inttoptr i32 %dest to f8_152 addrspace(1)*
  store f8_152 %fp, f8_152 addrspace(1)* %dptr

  ret void
}

; CHECK: st_l  %S0, %S1
