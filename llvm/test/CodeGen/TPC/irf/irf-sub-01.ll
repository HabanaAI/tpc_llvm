; RUN: opt -S -scalar2irf < %s | FileCheck %s

target triple = "tpc"

define dso_local void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %dest to <5 x i32> addrspace(1)*
  %1 = load <5 x i32>, <5 x i32> addrspace(1)* %0, align 4
  %vecext = extractelement <5 x i32> %1, i32 0
  %add = sub nsw i32 %vecext, %src
  %vecins = insertelement <5 x i32> %1, i32 %add, i32 0
  store <5 x i32> %vecins, <5 x i32> addrspace(1)* %0, align 4
  ret void
}

; CHECK: define dso_local void @main(i32 %dest, i32 %src) {
; CHECK: entry:
; CHECK:   %0 = inttoptr i32 %dest to <5 x i32> addrspace(1)*
; CHECK:   %1 = load <5 x i32>, <5 x i32> addrspace(1)* %0, align 4
; CHECK:   %2 = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 %src, <5 x i32> %1, i32 1, i8 2, i32 0, <5 x i32> %1, i1 true, i1 false)
; CHECK:   %3 = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 0, <5 x i32> %2, i32 1, i8 2, i32 0, <5 x i32> %2, i1 true, i1 false)
; CHECK:   store <5 x i32> %3, <5 x i32> addrspace(1)* %0, align 4
; CHECK:   ret void
; CHECK: }

