; RUN: opt -S -scalar2irf < %s | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %ptr = inttoptr i32 %src to <5 x i32> addrspace(1)*
  %ndx1 = load <5 x i32>, <5 x i32> addrspace(1)* %ptr, align 4
  %inc.ptr = getelementptr inbounds <5 x i32>, <5 x i32> addrspace(1)* %ptr, i32 1
  %ndx2 = load <5 x i32>, <5 x i32> addrspace(1)* %inc.ptr, align 4
  %vecext1 = extractelement <5 x i32> %ndx1, i32 0
  %vecext2 = extractelement <5 x i32> %ndx2, i32 0
  %cmp = icmp eq i32 %vecext1, %vecext2
  %conv = zext i1 %cmp to i32
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  store i32 %conv, i32 addrspace(1)* %dptr, align 4
  ret void
}

; CHECK:  %0 = call i1 @llvm.tpc.cmp.eq.mask.v5i32.v5i32(<5 x i32> %ndx1, <5 x i32> %ndx2, i32 1, i8 2, i32 0, i1 undef, i1 true, i1 false)
; CHECK:  %conv = zext i1 %0 to i32
