; RUN: llc -march=tpc -mcpu=goya -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

target triple = "tpc"

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #1

; Function Attrs: nounwind writeonly
define void @main(i32 %input) local_unnamed_addr #1 {

bb:
  br label %bb30

bb30:                                             ; preds = %bb30, %bb
  %i2818 = phi i32 [ 0, %bb ], [ %ind, %bb30 ]
  %addphi = phi i32 [ 0, %bb ], [ %add, %bb30 ]
  %ifm = phi <5 x i32> [ zeroinitializer, %bb ], [ %0, %bb30 ]
  %0 = insertelement <5 x i32> %ifm, i32 %addphi, i32 0
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %0, i8 0, i32 0, <64 x float> zeroinitializer, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %ifm, i8 1, <64 x float> %1, i32 0, i1 true, i1 false)
  ; runtime argument used to update coords :
  %add = add nuw nsw i32 %input, %i2818
  %ind = add nuw nsw i32 %i2818, 1
  %exitcond = icmp eq i32 %ind, 128
  br i1 %exitcond, label %bb75, label %bb30

bb75:                                             ; preds = %bb30
  ret void
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind writeonly }
