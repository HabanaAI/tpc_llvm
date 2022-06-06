; RUN: llc -march=tpc -mcpu=goya -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[2].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 }[2].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

target triple = "tpc"

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)*, i32, i32, i1, i1)

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #1

; Function Attrs: nounwind writeonly
define void @main() local_unnamed_addr #1 {

bb:
  br label %bb30

bb30:                                             ; preds = %bb30, %bb
  %i2818 = phi i32 [ 0, %bb ], [ %ind, %bb30 ]
  %ifm = phi <5 x i32> [ zeroinitializer, %bb ], [ %0, %bb30 ]
  %genaddr = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %ifm, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %ldg = tail call i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)* %genaddr, i32 0, i32 undef, i1 1, i1 false)
  ; coord update has runtime dependence on 'ld.g'
  %mul = mul nsw i32 %ldg, 4
  %0 = insertelement <5 x i32> %ifm, i32 %mul, i32 0
  %1 = insertelement <5 x i32> %ifm, i32 %i2818, i32 0
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %0, i8 0, i32 0, <64 x float> zeroinitializer, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %1, i8 0, i32 0, <64 x float> zeroinitializer, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %ifm, i8 1, <64 x float> %2, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %ifm, i8 2, <64 x float> %3, i32 0, i1 true, i1 false)
  %ind = add nuw nsw i32 %i2818, 1
  %exitcond = icmp eq i32 %ind, 20
  br i1 %exitcond, label %bb75, label %bb30

bb75:                                             ; preds = %bb30
  ret void
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind writeonly }
