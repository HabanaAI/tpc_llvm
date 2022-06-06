; RUN: llc -march=tpc -mcpu=gaudi2 -O2 %s -o - | FileCheck %s

define void @main() {
entry:
  %uptr.1 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 2, i8 addrspace(3)* undef, i1 true, i1 false)
  %ptr.1 = bitcast i8 addrspace(3)* %uptr.1 to i32 addrspace(3)*
  %loaded.1 = load i32, i32 addrspace(3)* %ptr.1, align 4
  %fixed.1 = add i32 %loaded.1, 1
  store i32 %fixed.1, i32 addrspace(3)* %ptr.1, align 4

  %uptr.2 = getelementptr inbounds i8, i8 addrspace(3)* %uptr.1, i64 4
  %ptr.2 = bitcast i8 addrspace(3)* %uptr.2 to i32 addrspace(3)*
  %loaded.2 = load i32, i32 addrspace(3)* %ptr.2, align 4
  %fixed.2 = add i32 %loaded.2, 1
  store i32 %fixed.2, i32 addrspace(3)* %ptr.2, align 4
  ret void
}

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; CHECK: gen_addr dt=int32 [[ADRF:%AD[0-9]+]], 0x0, %I{{[0-9]+}}
; CHECK: ld_g     %S{{[0-9]+}}, [[ADRF]]
; CHECK: st_g     inc_4 [[ADRF]], %S{{[0-9]+}}
; CHECK: ld_g     %S{{[0-9]+}}, [[ADRF]]
; CHECK: st_g     [[ADRF]], %S{{[0-9]+}}
