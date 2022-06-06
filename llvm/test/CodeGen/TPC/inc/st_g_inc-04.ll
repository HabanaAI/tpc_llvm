; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

define void @main(i32 %src1, i32 %src2, i32 %src3) {
entry:
  %0 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = bitcast i8 addrspace(3)* %0 to i32 addrspace(3)*
  store i32 %src1, i32 addrspace(3)* %1, align 4
  %inc1 = getelementptr inbounds i8, i8 addrspace(3)* %0, i64 4
  %2 = bitcast i8 addrspace(3)* %inc1 to i32 addrspace(3)*
  store i32 %src2, i32 addrspace(3)* %2, align 4
  %inc2 = getelementptr inbounds i8, i8 addrspace(3)* %0, i64 8
  %3 = bitcast i8 addrspace(3)* %inc2 to i32 addrspace(3)*
  store i32 %src3, i32 addrspace(3)* %3, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

; CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK: gen_addr  {{.*}}[[ADDR:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK: st_g      inc_4 [[ADDR]], %S0
; CHECK: st_g      inc_4 [[ADDR]], %S1
; CHECK: st_g      [[ADDR]], %S2
