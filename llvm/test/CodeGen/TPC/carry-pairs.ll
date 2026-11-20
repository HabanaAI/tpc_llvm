; RUN: opt < %s -passes=tpc-carry-pairs -S | FileCheck %s

; TPCCarryPairs joins each carry-reading tpc_add (CARRY, switch 2) or tpc_sub
; (BORROW, switch 8) with the integer add or sub before it on the same unit
; into one tpc_carry_pair call, so that later passes cannot separate them.
; The IR is as the front end emits it: each result stored to its variable.

target triple = "tpc"

; The store of the low result moves after the pair; the low add goes away.
; CHECK-LABEL: @vector(
; CHECK-NOT:   @llvm.tpc.add.
; CHECK:       [[P:%.*]] = call { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %a, <64 x i32> %b, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false, <64 x i32> %c, <64 x i32> %d, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
; CHECK-NEXT:  [[LO:%.*]] = extractvalue { <64 x i32>, <64 x i32> } [[P]], 0
; CHECK-NEXT:  [[HI:%.*]] = extractvalue { <64 x i32>, <64 x i32> } [[P]], 1
; CHECK-NEXT:  store <64 x i32> [[LO]], ptr %lo.addr
; CHECK-NEXT:  store <64 x i32> [[HI]], ptr %hi.addr
; CHECK-NOT:   @llvm.tpc.add.
define void @vector(<64 x i32> %a, <64 x i32> %b, ptr %pc, ptr %pd) {
  %c.addr = alloca <64 x i32>
  %d.addr = alloca <64 x i32>
  %lo.addr = alloca <64 x i32>
  %hi.addr = alloca <64 x i32>
  %c0 = load <64 x i32>, ptr %pc
  store <64 x i32> %c0, ptr %c.addr
  %d0 = load <64 x i32>, ptr %pd
  store <64 x i32> %d0, ptr %d.addr
  %lo = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %a, <64 x i32> %b, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false)
  store <64 x i32> %lo, ptr %lo.addr
  call void @llvm.lifetime.start.p0(ptr %hi.addr)
  %c = load <64 x i32>, ptr %c.addr
  %d = load <64 x i32>, ptr %d.addr
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %c, <64 x i32> %d, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  store <64 x i32> %hi, ptr %hi.addr
  call void @use(ptr %lo.addr, ptr %hi.addr)
  ret void
}

; A low add written with '+', with the operands of ADD (INT32, no switch);
; a scalar add between does not count, a vector one would.
; CHECK-LABEL: @plain_add(
; CHECK:       %s = add i32 %x, %y
; CHECK:       call { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %a, <64 x i32> %b, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false, <64 x i32> %c, i32 0, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
define void @plain_add(<64 x i32> %a, <64 x i32> %b, <64 x i32> %c, i32 %x, i32 %y, ptr %p) {
  %lo = add <64 x i32> %a, %b
  %s = add i32 %x, %y
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.i32.i1(<64 x i32> %c, i32 0, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use3(<64 x i32> %lo, <64 x i32> %hi, i32 %s)
  ret void
}

; A constant first operand of '+' goes second, as the instructions take it.
; CHECK-LABEL: @constant_first(
; CHECK:       @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %a, <64 x i32> splat (i32 5), i8 2,
define void @constant_first(<64 x i32> %a, <64 x i32> %c, ptr %p) {
  %lo = add <64 x i32> splat (i32 5), %a
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.i32.i1(<64 x i32> %c, i32 0, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use2(<64 x i32> %lo, <64 x i32> %hi)
  ret void
}

; Scalar halves, and a 16-bit pair; a vector add between does not count.
; CHECK-LABEL: @scalar(
; CHECK:       call { i32, i32 } @llvm.tpc.carry.pair.{{.*}}(i32 0, i32 %a, i32 %b, i8 2, i32 0, i32 undef, i1 true, i1 false, i32 %c, i32 %d, i8 2, i32 2, i32 undef, i1 true, i1 false)
; CHECK:       call { <128 x i16>, <128 x i16> } @llvm.tpc.carry.pair.{{.*}}(i32 0, <128 x i16> %v, <128 x i16> %w, i8 7, i32 0,
define void @scalar(i32 %a, i32 %b, i32 %c, i32 %d, <64 x i32> %x, <128 x i16> %v, <128 x i16> %w) {
  %lo = call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %a, i32 %b, i8 2, i32 0, i32 undef, i1 true, i1 false)
  %y = add <64 x i32> %x, %x
  %hi = call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %c, i32 %d, i8 2, i32 2, i32 undef, i1 true, i1 false)
  %vlo = add <128 x i16> %v, %w
  %vhi = call <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16> %w, <128 x i16> %w, i8 7, i32 2, <128 x i16> undef, i1 true, i1 false)
  call void @uses(i32 %lo, i32 %hi, <64 x i32> %y, <128 x i16> %vlo, <128 x i16> %vhi)
  ret void
}

; SUB then SUB with BORROW: kind 3. ADD then SUB with BORROW: kind 2.
; CHECK-LABEL: @sub(
; CHECK:       @llvm.tpc.carry.pair.{{.*}}(i32 3, <64 x i32> %a, <64 x i32> %b, i8 2, i32 0,
; CHECK-SAME:  <64 x i32> %c, <64 x i32> %d, i8 2, i32 8,
; CHECK:       @llvm.tpc.carry.pair.{{.*}}(i32 2, <64 x i32> %a, <64 x i32> %c, i8 2, i32 0,
define void @sub(<64 x i32> %a, <64 x i32> %b, <64 x i32> %c, <64 x i32> %d) {
  %lo = call <64 x i32> @llvm.tpc.sub.v64i32.v64i32.v64i32.i1(<64 x i32> %a, <64 x i32> %b, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false)
  %hi = call <64 x i32> @llvm.tpc.sub.v64i32.v64i32.v64i32.i1(<64 x i32> %c, <64 x i32> %d, i8 2, i32 8, <64 x i32> undef, i1 true, i1 false)
  %lo2 = add <64 x i32> %a, %c
  %hi2 = call <64 x i32> @llvm.tpc.sub.v64i32.v64i32.v64i32.i1(<64 x i32> %b, <64 x i32> %d, i8 2, i32 8, <64 x i32> undef, i1 true, i1 false)
  call void @use4(<64 x i32> %lo, <64 x i32> %hi, <64 x i32> %lo2, <64 x i32> %hi2)
  ret void
}

; The carry read takes the carry of the vector add just before it, %x.
; CHECK-LABEL: @nearest(
; CHECK:       %lo = add <64 x i32> %a, %b
; CHECK:       @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %c, <64 x i32> %d, i8 2,
define void @nearest(<64 x i32> %a, <64 x i32> %b, <64 x i32> %c, <64 x i32> %d) {
  %lo = add <64 x i32> %a, %b
  %x = add <64 x i32> %c, %d
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.i32.i1(<64 x i32> %a, i32 0, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use3v(<64 x i32> %lo, <64 x i32> %x, <64 x i32> %hi)
  ret void
}

; Not joined: a call between; NO_CARRY_GEN on the low add; a carry read in
; another block than the add; a read of the carry of a carry read (the third
; add of a chain: only the first two are joined).
; CHECK-LABEL: @not_joined(
; CHECK:       %h1 = call <64 x i32> @llvm.tpc.add.{{.*}} i32 2,
; CHECK:       %h2 = call <64 x i32> @llvm.tpc.add.{{.*}} i32 2,
; CHECK:       %h3 = call <64 x i32> @llvm.tpc.add.{{.*}} i32 2,
; CHECK:       @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %a, <64 x i32> %b, i8 2, i32 0,
; CHECK-SAME:  <64 x i32> %b, <64 x i32> %c, i8 2, i32 2,
; CHECK:       %w2 = call <64 x i32> @llvm.tpc.add.{{.*}} i32 2,
define void @not_joined(<64 x i32> %a, <64 x i32> %b, <64 x i32> %c) {
  %l1 = add <64 x i32> %a, %b
  call void @f()
  %h1 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %b, <64 x i32> %c, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %l2 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %a, <64 x i32> %b, i8 2, i32 4, <64 x i32> undef, i1 true, i1 false)
  %h2 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %b, <64 x i32> %c, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %l3 = add <64 x i32> %a, %b
  br label %next
next:
  %h3 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %b, <64 x i32> %c, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %w0 = add <64 x i32> %a, %b
  %w1 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %b, <64 x i32> %c, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  %w2 = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %c, <64 x i32> %c, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use4(<64 x i32> %l1, <64 x i32> %h1, <64 x i32> %l2, <64 x i32> %h2)
  call void @use4(<64 x i32> %l3, <64 x i32> %h3, <64 x i32> %w0, <64 x i32> %w1)
  call void @use2(<64 x i32> %w2, <64 x i32> %w2)
  ret void
}

; The low result is used before the carry read (here by its operand, and by
; a store to a variable read before it): the pair is made, and the low add
; stays for those uses.
; CHECK-LABEL: @low_used_before(
; CHECK:       %lo = add <64 x i32> %a, %b
; CHECK-NEXT:  store <64 x i32> %lo, ptr %lo.addr
; CHECK:       [[P:%.*]] = call { <64 x i32>, <64 x i32> } @llvm.tpc.carry.pair.{{.*}}(i32 0, <64 x i32> %a, <64 x i32> %b, {{.*}}, <64 x i32> %again, <64 x i32> %lo, i8 2, i32 2,
; CHECK:       call void @use2(<64 x i32> %{{[0-9]+}},
define void @low_used_before(<64 x i32> %a, <64 x i32> %b) {
  %lo.addr = alloca <64 x i32>
  %lo = add <64 x i32> %a, %b
  store <64 x i32> %lo, ptr %lo.addr
  %again = load <64 x i32>, ptr %lo.addr
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %again, <64 x i32> %lo, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use2(<64 x i32> %lo, <64 x i32> %hi)
  ret void
}

; A floating-point add between does not write the carry.
; CHECK-LABEL: @float_between(
; CHECK:       @llvm.tpc.carry.pair
define void @float_between(<64 x i32> %a, <64 x i32> %b, <64 x float> %f) {
  %lo = add <64 x i32> %a, %b
  %g = call <64 x float> @llvm.tpc.add.v64f32.v64f32.v64f32.i1(<64 x float> %f, <64 x float> %f, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %hi = call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32> %a, <64 x i32> %b, i8 2, i32 2, <64 x i32> undef, i1 true, i1 false)
  call void @use2f(<64 x i32> %lo, <64 x i32> %hi, <64 x float> %g)
  ret void
}

declare <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.i1(<64 x i32>, <64 x i32>, i8, i32, <64 x i32>, i1, i1)
declare <64 x i32> @llvm.tpc.add.v64i32.v64i32.i32.i1(<64 x i32>, i32, i8, i32, <64 x i32>, i1, i1)
declare <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, i8, i32, <128 x i16>, i1, i1)
declare i32 @llvm.tpc.add.i32.i32.i32.i1(i32, i32, i8, i32, i32, i1, i1)
declare <64 x i32> @llvm.tpc.sub.v64i32.v64i32.v64i32.i1(<64 x i32>, <64 x i32>, i8, i32, <64 x i32>, i1, i1)
declare <64 x float> @llvm.tpc.add.v64f32.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.lifetime.start.p0(ptr)
declare void @f()
declare void @use(ptr, ptr)
declare void @use2(<64 x i32>, <64 x i32>)
declare void @use2f(<64 x i32>, <64 x i32>, <64 x float>)
declare void @use3(<64 x i32>, <64 x i32>, i32)
declare void @use3v(<64 x i32>, <64 x i32>, <64 x i32>)
declare void @use4(<64 x i32>, <64 x i32>, <64 x i32>, <64 x i32>)
declare void @uses(i32, i32, <64 x i32>, <128 x i16>, <128 x i16>)
