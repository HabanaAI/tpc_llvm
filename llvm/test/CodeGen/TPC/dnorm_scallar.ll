; RUN: llc -march=tpc -mcpu=doron1 -tpc-dnorm %s -o - | FileCheck %s

; Function Attrs: nounwind
define dso_local void @main(i32 %a, i32 %b, i32 %dest) #0 {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = inttoptr i32 %a to float addrspace(1)*
  %2 = inttoptr i32 %b to float addrspace(1)*
  %3 = load float, float addrspace(1)* %1, align 4
  %4 = load float, float addrspace(1)* %2, align 4
  %mul = fmul float %3, %4
  ; CHECK: mul.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}
  %incdec.ptr = getelementptr inbounds float, float addrspace(1)* %0, i32 1
  store float %mul, float addrspace(1)* %0, align 4
  ret void
}