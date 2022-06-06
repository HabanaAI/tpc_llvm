; RUN: %clang_cc1 -emit-llvm-bc -std=rc99 -triple tpc -tpc-library -DLIB %S/library-01.c -o %t.bc
; RUN: %clang_cc1 -S -emit-llvm -std=rc99 -triple tpc -mlink-bitcode-file %t.bc %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %call = tail call i32 @qqq(i32 %src)
  store i32 %call, i32 addrspace(1)* %0, align 4
  ret void
}

declare i32 @qqq(i32)

; CHECK: define void @main(i32 %dest, i32 %src) {{.*}}#0 {
; CHECK: entry:
; CHECK:   %0 = inttoptr i32 %dest to i32 addrspace(1)*
; CHECK:   %add.i = add nsw i32 %src, 22
; CHECK:   store i32 %add.i, i32 addrspace(1)* %0, align 4
; CHECK:   ret void
; CHECK: }
; CHECK: attributes #0 = {{{.*}} "tpc-kernel" {{.*}}}
