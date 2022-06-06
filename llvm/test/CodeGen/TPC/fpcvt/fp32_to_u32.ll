; RUN: llc -mcpu goya2 %s -o - | FileCheck %s --check-prefix=GEN3
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, float %x) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %1 = fptoui float %x to i32
  store i32 %1, i32 addrspace(1)* %0, align 4
  ret void
}

; GEN3: convert.f32 target_type=int32 [[VAL1:%S[0-9]+]], %S1
; GEN3: convert.i32 target_type=uint32 {{.*}}[[VAL2:%S[0-9]+]], [[VAL1]]
; GEN3: st_l        %S0, [[VAL2]]

; GEN4: convert.f32 target_type=uint32 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l        %S0, [[VAL]]
