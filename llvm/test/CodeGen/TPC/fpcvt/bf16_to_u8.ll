; RUN: llc -mcpu goya2 %s -o - | FileCheck %s --check-prefix=GEN3
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, bfloat16 %x) {
entry:
  %0 = inttoptr i32 %dest to i8 addrspace(1)*
  %1 = fptoui bfloat16 %x to i8
  store i8 %1, i8 addrspace(1)* %0, align 4
  ret void
}

; GEN3: convert.bf16 target_type=int16 [[VAL1:%S[0-9]+]], %S1
; GEN3: convert.i16  target_type=uint8 rhne [[VAL2:%S[0-9]+]], [[VAL1]]
; GEN3: st_l         %S0, [[VAL2]]

; GEN4: convert.bf16 target_type=uint8 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l         %S0, [[VAL]]
