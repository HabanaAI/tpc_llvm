; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

define void @main(i16 %src) {
entry:
  %0 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %1 = bitcast i8 addrspace(3)* %0 to i16 addrspace(3)*
  store i16 %src, i16 addrspace(3)* %1, align 4
  %incptr = getelementptr inbounds i8, i8 addrspace(3)* %0, i64 2
  %2 = bitcast i8 addrspace(3)* %incptr to i16 addrspace(3)*
  store i16 2, i16 addrspace(3)* %2, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

; CHECK: gen_addr {{.*}}[[ADRF:%AD[0-9]+]], 0x0, %I{{.*}}
; CHECK: st_g     inc_2 [[ADRF]], %S0
; CHECK: st_g     [[ADRF]], %S{{.*}}
