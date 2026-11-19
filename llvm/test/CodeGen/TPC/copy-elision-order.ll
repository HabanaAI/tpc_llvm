; RUN: opt -S -passes=copyelide < %s | FileCheck %s

; TpcCopyElision removes the temporaries the frontend makes for struct values
; returned by and passed to intrinsics. Removing one moves an access, so it is
; only done when nothing between depends on the order. Before, in
;   float128 q = f(); use(p.v1); p = q;
; the store of f() went straight to p, so use(p.v1) read the new value: a
; float128 carried from one loop iteration to the next was the current one.

target triple = "tpc"

%pair = type { <64 x float>, <64 x float> }

declare %pair @f()
declare void @use(<64 x float>)
declare void @use128(<128 x float>)
declare void @llvm.memcpy.p0.p0.i32(ptr, ptr, i32, i1)

; The destination is read between the store and the copy: keep the temporary.
define void @return_temp_dest_read(ptr %out) {
; CHECK-LABEL: define void @return_temp_dest_read(
; CHECK:         store %pair %v, ptr %q, align 256
; CHECK-NEXT:    %old = load <64 x float>, ptr %p, align 256
; CHECK-NEXT:    call void @use(<64 x float> %old)
; CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i32(ptr align 256 %p, ptr align 256 %q, i32 512, i1 false)
  %p = alloca %pair, align 256
  %q = alloca %pair, align 256
  %v = call %pair @f()
  store %pair %v, ptr %q, align 256
  %old = load <64 x float>, ptr %p, align 256
  call void @use(<64 x float> %old)
  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %p, ptr align 256 %q, i32 512, i1 false)
  %new = load <64 x float>, ptr %p, align 256
  call void @use(<64 x float> %new)
  ret void
}

; The same with the read after the copy: the store goes straight to p.
define void @return_temp_dest_read_after(ptr %out) {
; CHECK-LABEL: define void @return_temp_dest_read_after(
; CHECK-NOT:     %q
; CHECK:         store %pair %v, ptr %p, align 256
; CHECK-NOT:     memcpy
; CHECK:         ret void
  %p = alloca %pair, align 256
  %q = alloca %pair, align 256
  %v = call %pair @f()
  store %pair %v, ptr %q, align 256
  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %p, ptr align 256 %q, i32 512, i1 false)
  %new = load <64 x float>, ptr %p, align 256
  call void @use(<64 x float> %new)
  ret void
}

; The store is in another block than the copy: keep the temporary.
define void @return_temp_other_block(i1 %c) {
; CHECK-LABEL: define void @return_temp_other_block(
; CHECK:         store %pair %v, ptr %q, align 256
; CHECK:       next:
; CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i32(ptr align 256 %p, ptr align 256 %q, i32 512, i1 false)
  %p = alloca %pair, align 256
  %q = alloca %pair, align 256
  %v = call %pair @f()
  store %pair %v, ptr %q, align 256
  br label %next
next:
  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %p, ptr align 256 %q, i32 512, i1 false)
  %new = load <64 x float>, ptr %p, align 256
  call void @use(<64 x float> %new)
  ret void
}

; The source of an argument copy is written before the temporary is read: keep
; the temporary.
define void @argument_temp_src_written(<128 x float> %x) {
; CHECK-LABEL: define void @argument_temp_src_written(
; CHECK:         call void @llvm.memcpy.p0.p0.i32(ptr align 256 %tmp, ptr align 256 %s, i32 512, i1 false)
; CHECK-NEXT:    store <128 x float> %x, ptr %s, align 256
; CHECK-NEXT:    %a = load <128 x float>, ptr %tmp, align 256
  %s = alloca %pair, align 256
  %tmp = alloca %pair, align 256
  %v = call %pair @f()
  store %pair %v, ptr %s, align 256
  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %tmp, ptr align 256 %s, i32 512, i1 false)
  store <128 x float> %x, ptr %s, align 256
  %a = load <128 x float>, ptr %tmp, align 256
  call void @use128(<128 x float> %a)
  ret void
}

; Nothing writes the source in between: the load reads it directly.
define void @argument_temp(<128 x float> %x) {
; CHECK-LABEL: define void @argument_temp(
; CHECK-NOT:     %tmp
; CHECK:         %a = load <128 x float>, ptr %s, align 256
; CHECK-NEXT:    call void @use128(<128 x float> %a)
; CHECK-NEXT:    store <128 x float> %x, ptr %s, align 256
  %s = alloca %pair, align 256
  %tmp = alloca %pair, align 256
  %v = call %pair @f()
  store %pair %v, ptr %s, align 256
  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %tmp, ptr align 256 %s, i32 512, i1 false)
  %a = load <128 x float>, ptr %tmp, align 256
  call void @use128(<128 x float> %a)
  store <128 x float> %x, ptr %s, align 256
  ret void
}
