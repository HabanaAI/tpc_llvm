; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %x0, i32 %x1, i32 %income, i32 %pred) {
entry:
  %0 = inttoptr i32 %x0 to <256 x float> addrspace(2)*
  %1 = load <256 x float>, <256 x float> addrspace(2)* %0, align 256
  %2 = inttoptr i32 %x1 to <256 x f8_143> addrspace(2)*
  %3 = load <256 x f8_143>, <256 x f8_143> addrspace(2)* %2, align 256
  %4 = inttoptr i32 %income to <256 x float> addrspace(2)*
  %5 = load <256 x float>, <256 x float> addrspace(2)* %4, align 256
  %vpptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %vpptr, align 256
  %6 = inttoptr i32 %dest to <256 x float> addrspace(2)*

  %7 = call <256 x float> @llvm.tpc.add.v256f32.v256f8_143.v256f32.v256i1(<256 x float> %1, <256 x f8_143> %3, i8 13, i32 0, <256 x float> %5, <256 x i1> %vpred, i1 false)

  store <256 x float> %7, <256 x float> addrspace(2)* %6, align 256
  ret void
}

declare <256 x float> @llvm.tpc.add.v256f32.v256f8_143.v256f32.v256i1(<256 x float>, <256 x f8_143>, i8, i32, <256 x float>, <256 x i1>, i1)

; CHECK-DAG: ld_l_v     %V[[X0:[0-9]+]], %S1
; CHECK-DAG: ld_l_v     %V[[X1:[0-9]+]], %S2
; CHECK-DAG: ld_l_v     %V[[RES:[0-9]+]], %S3
; CHECK-DAG: ld_l_v     [[PRED:%VP[0-9]+]], %S4
; CHECK:     add.f8_143 %A[[RES]], %A[[X0]], %V[[X1]], [[PRED]]
; CHECK:     st_l_v     %S0, %V[[RES]]
