; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <256 x f8_143> addrspace(2)*
  %1 = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %2 = load <256 x f8_143>, <256 x f8_143> addrspace(2)* %0, align 256
  %3 = call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143> %2, i8 13, i32 3072, <256 x f8_152> undef, i1 1, i1 0)
  store <256 x f8_152> %3, <256 x f8_152> addrspace(2)* %1, align 256
  ret void
}

declare <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143>, i8, i32, <256 x f8_152>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; CHECK:     convert.f8_143 all_lanes target_type=f8_152 rhne [[VRF]], [[VRF]]
; CHECK-DAG: st_l_v     %S0, [[VRF]]
