; RUN: llc -march=tpc -mcpu=gaudi %s -o %t.o --filetype=obj
; RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

; RUN: llc -march=tpc -mcpu=goya %s -o %t.o --filetype=obj
; RUN: %disasm --mcpu=goya %t.o | FileCheck %s

; CHECK: convert_int32 lane_sel=0 rhne to_16
; CHECK: convert_int32 lane_sel=1 rhne to_16

; ModuleID = 'mlir_kernel_llvm7.ll'
source_filename = "LLVMDialectModule"

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #0

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

define void @mult_1xsi16() local_unnamed_addr {
.critedge:
  %i = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> zeroinitializer, i8 1, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i8 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i9 = mul <128 x i16> %i8, %i
  call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> zeroinitializer, i8 2, <128 x i16> %i9, i32 0, i1 true, i1 false)
  ret void
}

attributes #0 = { nounwind writeonly }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
