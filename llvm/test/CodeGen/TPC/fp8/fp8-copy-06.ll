; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:

  %sptr = inttoptr i32 %src to f8_152 addrspace(1)*
  %dptr = inttoptr i32 %dest to f8_152 addrspace(1)*
  %val = load f8_152, f8_152 addrspace(1)* %sptr
  store f8_152 %val, f8_152 addrspace(1)* %dptr

  ret void
}

; CHECK: ld_l  %S1, %S1
; CHECK: st_l  %S0, %S1
