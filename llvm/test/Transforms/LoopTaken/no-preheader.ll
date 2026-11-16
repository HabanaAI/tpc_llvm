; RUN: opt < %s -passes=looptaken -S | FileCheck %s

; Loop %a is entered from an indirectbr, so simplifyLoop() cannot give it a
; preheader. The pass used to dereference the missing preheader; now it skips
; that loop and still adds the assume for loop %b.

target triple = "tpc"

; CHECK-LABEL: @f(
; CHECK:       a:
; CHECK-NOT:     call void @llvm.assume
; CHECK:       b.ph:
; CHECK-NEXT:    [[TAKEN:%.*]] = icmp sgt i32 %m, 0
; CHECK-NEXT:    call void @llvm.assume(i1 [[TAKEN]])
; CHECK-NEXT:    br label %b
define void @f(ptr %t, i32 %n, i32 %m) {
entry:
  indirectbr ptr %t, [label %a, label %b.ph]

a:
  %i = phi i32 [ 0, %entry ], [ %i.next, %a ]
  %i.next = add nsw i32 %i, 1
  %ca = icmp slt i32 %i.next, %n
  br i1 %ca, label %a, label %b.ph, !llvm.loop !0

b.ph:
  br label %b

b:
  %j = phi i32 [ 0, %b.ph ], [ %j.next, %b ]
  %j.next = add nsw i32 %j, 1
  %cb = icmp slt i32 %j.next, %m
  br i1 %cb, label %b, label %exit, !llvm.loop !2

exit:
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.taken", i1 true}
!2 = distinct !{!2, !1}
