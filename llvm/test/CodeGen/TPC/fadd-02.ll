; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src) {
entry:
  %sptr = inttoptr i32 %src to <256 x float> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x float> addrspace(2)*
  %vala = load <256 x float>, <256 x float> addrspace(2)* %sptr, align 256
  %sptr2 = getelementptr inbounds <256 x float>, <256 x float> addrspace(2)* %sptr, i32 1
  %valb = load <256 x float>, <256 x float> addrspace(2)* %sptr2, align 256
  %add = fadd <256 x float> %vala, %valb
  store <256 x float> %add, <256 x float> addrspace(2)* %dptr
  ret void
}

; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: ld_l_v
; CHECK: add.f32
; CHECK: add.f32
; CHECK: add.f32
; CHECK: add.f32
; CHECK: st_l_v
; CHECK: st_l_v
; CHECK: st_l_v
; CHECK: st_l_v
