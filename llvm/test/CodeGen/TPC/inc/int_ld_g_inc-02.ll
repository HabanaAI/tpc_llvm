; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s
; RUN: llc -march=tpc -mcpu=goya2 -O2 %s -o - | FileCheck %s

define void @main(i32 %dest, i1 zeroext %pred) {
entry:
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  %0 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 2, i8 addrspace(3)* undef, i1 true, i1 false)
  %gptr = bitcast i8 addrspace(3)* %0 to i32 addrspace(3)*
  %1 = call {i32, i32 addrspace(3)*} @llvm.tpc.ld.g.inc.i32.p3i32(i32 addrspace(3)* %gptr, i32 8, i32 0, i32 0, i1 true, i1 false)
  %2 = extractvalue {i32, i32 addrspace(3)*} %1, 0
  %3 = extractvalue {i32, i32 addrspace(3)*} %1, 1
  store i32 %2, i32 addrspace(1)* %dptr
  %4 = bitcast i32 addrspace(3)* %3 to i8 addrspace(3)*
  %5 = call i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)* %4, i32 0, i32 0, i1 true, i1 false)
  %inc.dptr = getelementptr inbounds i32, i32 addrspace(1)* %dptr, i32 1
  store i32 %5, i32 addrspace(1)* %inc.dptr

  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)
declare { i32, i32 addrspace(3)* } @llvm.tpc.ld.g.inc.i32.p3i32(i32 addrspace(3)*, i32, i32, i32, i1, i1)
declare i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)*, i32, i32, i1, i1)

; CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
; CHECK: gen_addr  {{.*}}[[ADRF:%AD[0-9]+]], 0x0, [[NDX]]
; CHECK: ld_g      inc_8 %S{{[0-9]+}}, [[ADRF]]
; CHECK: ld_g      %S{{[0-9]+}}, [[ADRF]]
