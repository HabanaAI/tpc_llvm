; RUN: opt -S -passes=scalar2irf < %s | FileCheck %s

; Lane updates become one masked IRF operation only when each one updates the
; vector that the previous one produced. Before, two updates of the same
; vector into different results were merged: in
;   right = c; right[0] = c[0] + 1;  down = c; down[1] = c[1] + 1;
; down got add.mask b00011, which also incremented lane 0.
;
; The element may also be operand 1 of the operation. Before, the pass took
; operand 1 as the scalar, so c[0] + c[1] into lane 1 became c[1] + c[1], and
; k - c[0] became c[0] - k (with assertions, it aborted).

target triple = "tpc"

define void @independent_updates(<5 x i32> %c, ptr addrspace(1) %p, ptr addrspace(1) %q) {
; CHECK-LABEL: @independent_updates(
; CHECK-NEXT:    [[RIGHT:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %c, i32 1, i32 1, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    [[DOWN:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %c, i32 1, i32 2, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    store <5 x i32> [[RIGHT]], ptr addrspace(1) %p
; CHECK-NEXT:    store <5 x i32> [[DOWN]], ptr addrspace(1) %q
; CHECK-NEXT:    ret void
  %e0 = extractelement <5 x i32> %c, i32 0
  %a0 = add nsw i32 %e0, 1
  %right = insertelement <5 x i32> %c, i32 %a0, i32 0
  %e1 = extractelement <5 x i32> %c, i32 1
  %a1 = add nsw i32 %e1, 1
  %down = insertelement <5 x i32> %c, i32 %a1, i32 1
  store <5 x i32> %right, ptr addrspace(1) %p
  store <5 x i32> %down, ptr addrspace(1) %q
  ret void
}

define <5 x i32> @chained_updates(<5 x i32> %c, i32 %k) {
; CHECK-LABEL: @chained_updates(
; CHECK-NEXT:    [[R:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %c, i32 %k, i32 3, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    ret <5 x i32> [[R]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %a0 = add nsw i32 %e0, %k
  %v0 = insertelement <5 x i32> %c, i32 %a0, i32 0
  %e1 = extractelement <5 x i32> %v0, i32 1
  %a1 = add nsw i32 %e1, %k
  %v1 = insertelement <5 x i32> %v0, i32 %a1, i32 1
  ret <5 x i32> %v1
}

; Lane 0 is updated twice, so the third update cannot join the mask.
define <5 x i32> @same_lane_twice(<5 x i32> %c, i32 %k) {
; CHECK-LABEL: @same_lane_twice(
; CHECK-NEXT:    [[R0:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %c, i32 %k, i32 3, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    [[R1:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> [[R0]], i32 %k, i32 1, i8 2, i32 0, <5 x i32> [[R0]], i1 true, i1 false)
; CHECK-NEXT:    ret <5 x i32> [[R1]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %a0 = add nsw i32 %e0, %k
  %v0 = insertelement <5 x i32> %c, i32 %a0, i32 0
  %e1 = extractelement <5 x i32> %v0, i32 1
  %a1 = add nsw i32 %e1, %k
  %v1 = insertelement <5 x i32> %v0, i32 %a1, i32 1
  %e2 = extractelement <5 x i32> %v1, i32 0
  %a2 = add nsw i32 %e2, %k
  %v2 = insertelement <5 x i32> %v1, i32 %a2, i32 0
  ret <5 x i32> %v2
}

; d[1] = c[0] + c[1]
define <5 x i32> @add_element_second(<5 x i32> %c) {
; CHECK-LABEL: @add_element_second(
; CHECK-NEXT:    [[E0:%.*]] = extractelement <5 x i32> %c, i32 0
; CHECK-NEXT:    [[R:%.*]] = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %c, i32 [[E0]], i32 2, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    ret <5 x i32> [[R]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %e1 = extractelement <5 x i32> %c, i32 1
  %a = add nsw i32 %e0, %e1
  %d = insertelement <5 x i32> %c, i32 %a, i32 1
  ret <5 x i32> %d
}

; d[0] = k - c[0] is sub.mask(k, c).
define <5 x i32> @sub_scalar_first(<5 x i32> %c, i32 %k) {
; CHECK-LABEL: @sub_scalar_first(
; CHECK-NEXT:    [[R:%.*]] = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 %k, <5 x i32> %c, i32 1, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    ret <5 x i32> [[R]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %s = sub nsw i32 %k, %e0
  %d = insertelement <5 x i32> %c, i32 %s, i32 0
  ret <5 x i32> %d
}

; c[0] - k and k - c[1] are different operations and are not merged.
define <5 x i32> @sub_mixed_order(<5 x i32> %c, i32 %k) {
; CHECK-LABEL: @sub_mixed_order(
; CHECK-NEXT:    [[S0:%.*]] = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 %k, <5 x i32> %c, i32 1, i8 2, i32 0, <5 x i32> %c, i1 true, i1 false)
; CHECK-NEXT:    [[R0:%.*]] = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 0, <5 x i32> [[S0]], i32 1, i8 2, i32 0, <5 x i32> [[S0]], i1 true, i1 false)
; CHECK-NEXT:    [[R1:%.*]] = call <5 x i32> @llvm.tpc.sub.mask.i32.v5i32(i32 %k, <5 x i32> [[R0]], i32 2, i8 2, i32 0, <5 x i32> [[R0]], i1 true, i1 false)
; CHECK-NEXT:    ret <5 x i32> [[R1]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %s0 = sub nsw i32 %e0, %k
  %v0 = insertelement <5 x i32> %c, i32 %s0, i32 0
  %e1 = extractelement <5 x i32> %v0, i32 1
  %s1 = sub nsw i32 %k, %e1
  %v1 = insertelement <5 x i32> %v0, i32 %s1, i32 1
  ret <5 x i32> %v1
}

; k << c[0] has no IRF form with the element second; it is left alone.
define <5 x i32> @shl_element_second(<5 x i32> %c, i32 %k) {
; CHECK-LABEL: @shl_element_second(
; CHECK-NEXT:    [[E0:%.*]] = extractelement <5 x i32> %c, i32 0
; CHECK-NEXT:    [[S:%.*]] = shl i32 %k, [[E0]]
; CHECK-NEXT:    [[D:%.*]] = insertelement <5 x i32> %c, i32 [[S]], i32 0
; CHECK-NEXT:    ret <5 x i32> [[D]]
  %e0 = extractelement <5 x i32> %c, i32 0
  %s = shl i32 %k, %e0
  %d = insertelement <5 x i32> %c, i32 %s, i32 0
  ret <5 x i32> %d
}
