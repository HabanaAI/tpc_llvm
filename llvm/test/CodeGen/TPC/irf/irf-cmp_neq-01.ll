; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %pval) {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext1 = extractelement <5 x i32> %0, i32 0
  %vecext2 = extractelement <5 x i32> %add, i32 0
  %cmp = icmp ne i32 %vecext1, %vecext2
  %2 = inttoptr i32 %pval to i32 addrspace(1)*
  %3 = zext i1 %cmp to i32
  store i32 %3, i32 addrspace(1)* %2, align 4
  ret void
}

declare <5 x i32> @llvm.tpc.get.index.space.offset()
declare <5 x i32> @llvm.tpc.get.index.space.size()

; CHECK: cmp_neq.i32  b00001 %SP{{[0-9]+}}, %I0, %I{{[0-9]+}}
