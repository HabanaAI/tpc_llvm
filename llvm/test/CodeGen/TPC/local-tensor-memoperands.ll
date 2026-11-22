; RUN: llc -march=tpc -mcpu=gaudi2 -stop-after=finalize-isel %s -o - | FileCheck %s

; SLM and VLM accesses and tensor stores carry a memory operand in their
; address space, so that the schedulers order them only against accesses of
; the same memory (TPCInstrInfo::areMemAccessesTriviallyDisjoint). An MMIO
; access has none and stays a barrier to every other access.

target triple = "tpc"

; CHECK-LABEL: name: accesses
; CHECK: LD_L_Vvsp {{.*}} :: (load (s2048), align 4, addrspace 2)
; CHECK: ST_TNSRvp {{.*}} :: (store (s2048), align 4, addrspace 3)
; CHECK: ST_L_Vsvp {{.*}} :: (store (s2048), align 4, addrspace 2)
; CHECK: LD_Lsip 16, {{.*}} :: (load (s32), addrspace 1)
; CHECK: ST_Lisp 32, {{.*}} :: (store (s32), addrspace 1)
; CHECK: ST_Lisp 1336, {{[^:]*$}}
define void @accesses(i32 %a) {
  %c = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %v = call <64 x float> @llvm.tpc.ld.l.v.v64f32.i1(i32 %a, i32 0, <64 x float> undef, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %v, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.l.v.v64f32.i1(i32 %a, <64 x float> %v, i32 0, i1 true, i1 false)
  %s = call i32 @llvm.tpc.ld.l.i32(i32 16, i32 0, i32 undef, i1 true, i1 false)
  call void @llvm.tpc.st.l.i32(i32 32, i32 %s, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.l.i32(i32 1336, i32 %s, i32 1, i1 true, i1 false)
  ret void
}

declare <5 x i32> @llvm.tpc.get.index.space.offset()
declare <64 x float> @llvm.tpc.ld.l.v.v64f32.i1(i32, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.l.v.v64f32.i1(i32, <64 x float>, i32, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1)
declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1)
