; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

define void @main(i8 %src) {
entry:
  %0 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  store i8 %src, i8 addrspace(3)* %0, align 4
  %incptr = getelementptr inbounds i8, i8 addrspace(3)* %0, i64 1
  store i8 2, i8 addrspace(3)* %incptr, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

; CHECK: gen_addr {{.*}}[[ADRF:%AD[0-9]+]], 0x0, %I{{.*}}
; CHECK: st_g     inc_1 [[ADRF]], %S0
; CHECK: st_g     [[ADRF]], %S{{.*}}
