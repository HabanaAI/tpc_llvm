; RUN: llc -march=tpc -mcpu=goya2 %s -o - | FileCheck %s
; CHECK: convert_int32 all_lanes rhne to_8
; CHECK: convert_int32 all_lanes rhne to_8

; ModuleID = 'mlir_kernel_llvm7.ll'
source_filename = "LLVMDialectModule"

 

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #0

 

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #1

 

; Function Attrs: nounwind writeonly
define void @mul_i8() local_unnamed_addr #1 {
bb:
  br label %bb30

 

bb30:                                             ; preds = %bb, %bb30
  %i2818 = phi i32 [ 0, %bb ], [ %i64, %bb30 ]
  %i187.sroa.0.017 = phi <5 x i32> [ zeroinitializer, %bb ], [ %i187.sroa.0.0.vec.insert.1, %bb30 ]
  %i96.sroa.0.016 = phi <5 x i32> [ zeroinitializer, %bb ], [ %i96.sroa.0.0.vec.insert.1, %bb30 ]
  %i5.sroa.0.015 = phi <5 x i32> [ zeroinitializer, %bb ], [ %i5.sroa.0.0.vec.insert.1, %bb30 ]
  %0 = insertelement <5 x i32> %i5.sroa.0.015, i32 0, i32 0
  %i5.sroa.0.0.vec.insert = insertelement <5 x i32> %0, i32 %i2818, i32 1
  %1 = insertelement <5 x i32> %i96.sroa.0.016, i32 0, i32 0
  %i96.sroa.0.0.vec.insert = insertelement <5 x i32> %1, i32 %i2818, i32 1
  %2 = insertelement <5 x i32> %i187.sroa.0.017, i32 0, i32 0
  %i187.sroa.0.0.vec.insert = insertelement <5 x i32> %2, i32 %i2818, i32 1
  %i51 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i187.sroa.0.0.vec.insert, i8 1, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i56 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i96.sroa.0.0.vec.insert, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i57 = mul <256 x i8> %i56, %i51
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %i5.sroa.0.0.vec.insert, i8 2, <256 x i8> %i57, i32 0, i1 true, i1 false)
  %i5.sroa.0.0.vec.insert.1 = insertelement <5 x i32> %i5.sroa.0.0.vec.insert, i32 256, i32 0
  %i96.sroa.0.0.vec.insert.1 = insertelement <5 x i32> %i96.sroa.0.0.vec.insert, i32 256, i32 0
  %i187.sroa.0.0.vec.insert.1 = insertelement <5 x i32> %i187.sroa.0.0.vec.insert, i32 256, i32 0
  %i51.1 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i187.sroa.0.0.vec.insert.1, i8 1, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i56.1 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %i96.sroa.0.0.vec.insert.1, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %i57.1 = mul <256 x i8> %i56.1, %i51.1
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %i5.sroa.0.0.vec.insert.1, i8 2, <256 x i8> %i57.1, i32 0, i1 true, i1 false)
  %i64 = add nuw nsw i32 %i2818, 1
  %i29 = icmp ult i32 %i2818, 19
  br i1 %i29, label %bb30, label %bb65

 

bb65:                                             ; preds = %bb30
  ret void
}

 

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind writeonly }

 

!llvm.module.flags = !{!0}

 

!0 = !{i32 2, !"Debug Info Version", i32 3}
