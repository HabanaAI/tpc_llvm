; RUN: opt < %s -passes=tpc-licm -S | FileCheck %s

; A lookup whose index is a constant, not an instruction, is not one the pass
; moves. It used to dereference a null pointer on it (the crash reached from
; C at -O1, where nothing has hoisted the lookup yet).

target triple = "tpc"

; CHECK-LABEL: @main(
; CHECK:       loop:
; CHECK:         call <128 x i16> @llvm.tpc.lookup.1c.v128i16.v128i16(<128 x i16> splat (i16 7)
define void @main(i32 %n, ptr addrspace(1) %out) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %r = call <128 x i16> @llvm.tpc.lookup.1c.v128i16.v128i16(<128 x i16> splat (i16 7), i32 258, i32 1, <128 x i16> zeroinitializer, i1 true, i1 false)
  %p = getelementptr <128 x i16>, ptr addrspace(1) %out, i32 %i
  store <128 x i16> %r, ptr addrspace(1) %p
  %i.next = add i32 %i, 1
  %c = icmp slt i32 %i.next, %n
  br i1 %c, label %loop, label %exit

exit:
  ret void
}

declare <128 x i16> @llvm.tpc.lookup.1c.v128i16.v128i16(<128 x i16>, i32, i32, <128 x i16>, i1, i1)
