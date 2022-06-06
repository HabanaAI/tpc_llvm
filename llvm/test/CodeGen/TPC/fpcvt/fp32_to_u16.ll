; RUN: llc -mcpu goya2 %s -o - | FileCheck %s --check-prefix=GEN3
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, float %x) {
entry:
  %0 = inttoptr i32 %dest to i16 addrspace(1)*
  %1 = fptoui float %x to i16
  store i16 %1, i16 addrspace(1)* %0, align 4
  ret void
}

; GEN3: convert.f32 target_type=int32 [[VAL1:%S[0-9]+]], %S1
; GEN3: convert.i32 target_type=int16 rhne [[VAL2:%S[0-9]+]], [[VAL1]]
; GEN3: convert.i16 target_type=uint16 rhne [[VAL3:%S[0-9]+]], [[VAL2]]
; GEN3: st_l        %S0, [[VAL3]]


; GEN4: convert.f32 target_type=uint16 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l        %S0, [[VAL]]
