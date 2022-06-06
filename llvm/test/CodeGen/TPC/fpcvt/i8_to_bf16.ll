; RUN: llc -mcpu gaudi %s -o - | FileCheck %s --check-prefix=GEN2
; RUN: llc -mcpu goya2 %s -o - | FileCheck %s --check-prefix=GEN3

target triple = "tpc"

define void @main(i32 %dest, i8 %x) {
entry:
  %0 = inttoptr i32 %dest to bfloat16 addrspace(1)*
  %1 = sitofp i8 %x to bfloat16
  store bfloat16 %1, bfloat16 addrspace(1)* %0, align 4
  ret void
}

; GEN2: convert.i8  target_type=int16 [[VAL1:%S[0-9]+]], %S1
; GEN2: convert.i16 target_type=bf16 [[VAL2:%S[0-9]+]], [[VAL1]]
; GEN2: st_l        %S0, [[VAL2]]

; GEN3: convert.i8 target_type=bf16 [[VAL:%S[0-9]+]], %S1
; GEN3: st_l       %S0, [[VAL]]
