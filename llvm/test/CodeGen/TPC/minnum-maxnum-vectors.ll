; RUN: llc -mtriple=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

; fminnum/fmaxnum (fast-math a < b ? a : b) on bf16 and f16 vectors are MIN
; and MAX; register pairs and quads are split. bf16 and f16 had no pattern.

define void @min_bf16(i32 %pa, i32 %pb, i32 %pc) #0 {
; CHECK-LABEL: min_bf16:
; CHECK: min.bf16
  %a.p = inttoptr i32 %pa to ptr addrspace(2)
  %b.p = inttoptr i32 %pb to ptr addrspace(2)
  %c.p = inttoptr i32 %pc to ptr addrspace(2)
  %a = load <128 x bfloat>, ptr addrspace(2) %a.p
  %b = load <128 x bfloat>, ptr addrspace(2) %b.p
  %m = call <128 x bfloat> @llvm.minnum.v128bf16(<128 x bfloat> %a, <128 x bfloat> %b)
  store <128 x bfloat> %m, ptr addrspace(2) %c.p
  ret void
}

define void @max_f16(i32 %pa, i32 %pb, i32 %pc) #0 {
; CHECK-LABEL: max_f16:
; CHECK: max.f16
  %a.p = inttoptr i32 %pa to ptr addrspace(2)
  %b.p = inttoptr i32 %pb to ptr addrspace(2)
  %c.p = inttoptr i32 %pc to ptr addrspace(2)
  %a = load <128 x half>, ptr addrspace(2) %a.p
  %b = load <128 x half>, ptr addrspace(2) %b.p
  %m = call <128 x half> @llvm.maxnum.v128f16(<128 x half> %a, <128 x half> %b)
  store <128 x half> %m, ptr addrspace(2) %c.p
  ret void
}

define void @min_bf16_pair(i32 %pa, i32 %pb, i32 %pc) #0 {
; CHECK-LABEL: min_bf16_pair:
; CHECK-COUNT-2: min.bf16
  %a.p = inttoptr i32 %pa to ptr addrspace(2)
  %b.p = inttoptr i32 %pb to ptr addrspace(2)
  %c.p = inttoptr i32 %pc to ptr addrspace(2)
  %a = load <256 x bfloat>, ptr addrspace(2) %a.p
  %b = load <256 x bfloat>, ptr addrspace(2) %b.p
  %m = call <256 x bfloat> @llvm.minnum.v256bf16(<256 x bfloat> %a, <256 x bfloat> %b)
  store <256 x bfloat> %m, ptr addrspace(2) %c.p
  ret void
}

define void @max_f32_quad(i32 %pa, i32 %pb, i32 %pc) #0 {
; CHECK-LABEL: max_f32_quad:
; CHECK-COUNT-4: max.f32
  %a.p = inttoptr i32 %pa to ptr addrspace(2)
  %b.p = inttoptr i32 %pb to ptr addrspace(2)
  %c.p = inttoptr i32 %pc to ptr addrspace(2)
  %a = load <256 x float>, ptr addrspace(2) %a.p
  %b = load <256 x float>, ptr addrspace(2) %b.p
  %m = call <256 x float> @llvm.maxnum.v256f32(<256 x float> %a, <256 x float> %b)
  store <256 x float> %m, ptr addrspace(2) %c.p
  ret void
}

attributes #0 = { nounwind "target-cpu"="gaudi2" "tpc-kernel"="true" }
