; RUN: llc -march=tpc -mcpu=gaudi2 %s -o - | FileCheck %s
; RUN: llc -march=tpc -mcpu=gaudi2 -O0 %s -o - | FileCheck %s

; tpc_carry_pair is emitted as its two instructions, the carry read right
; after its own add (no other add of the unit between them): SelectionDAG
; must not merge the high halves of two pairs with equal high operands, nor
; drop a low half whose result is unused.

target triple = "tpc"

; CHECK-LABEL: same_high:
; CHECK:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
; CHECK-NOT:   add.i32
; CHECK:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0
; CHECK-NOT:   add.i32
; CHECK:       add.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
; CHECK-NOT:   add.i32
; CHECK:       add.i32 carry %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0
; CHECK-NOT:   add.i32
; CHECK:       halt
define void @same_high() {
  %a = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %b = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> <i32 0, i32 1, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %c = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> <i32 0, i32 2, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %h = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> <i32 0, i32 3, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %p1 = call { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.v64i32.v64i32.v64i32.v64i32.i1.v64i32.v64i32.i1(i32 0, <64 x i32> %a, <64 x i32> %b, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false, <64 x i32> %h, <64 x i32> zeroinitializer, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %p2 = call { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.v64i32.v64i32.v64i32.v64i32.i1.v64i32.v64i32.i1(i32 0, <64 x i32> %a, <64 x i32> %c, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false, <64 x i32> %h, <64 x i32> zeroinitializer, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %lo1 = extractvalue { <64 x i32>, <64 x i32> } %p1, 0
  %hi1 = extractvalue { <64 x i32>, <64 x i32> } %p1, 1
  %lo2 = extractvalue { <64 x i32>, <64 x i32> } %p2, 0
  %hi2 = extractvalue { <64 x i32>, <64 x i32> } %p2, 1
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 1, <64 x i32> %lo1, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> <i32 0, i32 1, i32 0, i32 0, i32 0>, i8 1, <64 x i32> %hi1, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> <i32 0, i32 2, i32 0, i32 0, i32 0>, i8 1, <64 x i32> %lo2, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> <i32 0, i32 3, i32 0, i32 0, i32 0>, i8 1, <64 x i32> %hi2, i32 0, i1 true, i1 false)
  ret void
}

; Only the carry of the low add is used; scalar halves; SUB then SUB BORROW.
; CHECK-LABEL: dead_low:
; CHECK:       add.i32 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
; CHECK-NOT:   {{add|sub}}.i32
; CHECK:       add.i32 carry %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
; CHECK:       sub.i32 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
; CHECK-NOT:   {{add|sub}}.i32
; CHECK:       sub.i32 borrow %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
; CHECK:       halt
define void @dead_low(i32 %a, i32 %b, i32 %c, i32 %d) {
  %p = call { i32, i32 } @llvm.tpc.carry.pair.i32.i32.i32.i32.i1.i32.i32.i1(i32 0, i32 %a, i32 %b, i8 2, i32 0, i32 undef, i1 true, i1 false, i32 %c, i32 %d, i8 2, i32 2, i32 undef, i1 true, i1 false)
  %hi = extractvalue { i32, i32 } %p, 1
  %q = call { i32, i32 } @llvm.tpc.carry.pair.i32.i32.i32.i32.i1.i32.i32.i1(i32 3, i32 %hi, i32 %b, i8 2, i32 0, i32 undef, i1 true, i1 false, i32 %c, i32 1, i8 2, i32 8, i32 undef, i1 true, i1 false)
  %qlo = extractvalue { i32, i32 } %q, 0
  %qhi = extractvalue { i32, i32 } %q, 1
  store i32 %qlo, ptr addrspace(1) null
  store i32 %qhi, ptr addrspace(1) inttoptr (i32 4 to ptr addrspace(1))
  ret void
}

declare { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.v64i32.v64i32.v64i32.v64i32.i1.v64i32.v64i32.i1(i32 immarg, <64 x i32>, <64 x i32>, i8, i32, <64 x i32>, i1, i1, <64 x i32>, <64 x i32>, i8, i32, <64 x i32>, i1, i1)
declare { i32, i32 } @llvm.tpc.carry.pair.i32.i32.i32.i32.i1.i32.i32.i1(i32 immarg, i32, i32, i8, i32, i32, i1, i1, i32, i32, i8, i32, i32, i1, i1)
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1)
