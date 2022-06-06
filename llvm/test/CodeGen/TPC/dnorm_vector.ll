; RUN: llc -march=tpc -mcpu=doron1 -tpc-dnorm %s -o - | FileCheck %s

; Function Attrs: nounwind
define dso_local void @main(i32 %a, i32 %b, i32 %dest) #0 {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = bitcast <64 x i32> addrspace(2)* %0 to <64 x float> addrspace(2)*
  %2 = inttoptr i32 %a to <64 x float> addrspace(2)*
  %3 = inttoptr i32 %b to <64 x float> addrspace(2)*
  %4 = load <64 x float>, <64 x float> addrspace(2)* %2, align 256
  %5 = load <64 x float>, <64 x float> addrspace(2)* %3, align 256
  %mul = fmul <64 x float> %4, %5
  ; CHECK: mul.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}
  %incdec.ptr = getelementptr inbounds <64 x float>, <64 x float> addrspace(2)* %1, i32 1
  store <64 x float> %mul, <64 x float> addrspace(2)* %1, align 256
  ret void
}

