; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s

; CHECK: "Loop nest unroll Info":
; CHECK-NEXT: .ascii  "{4, 4}; "

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(float %aboveZeroScale, float %belowZeroScale) local_unnamed_addr #0 !unroll_info !3 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 6
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul2 = shl nsw i32 %vecext1, 6
  %vecext3 = extractelement <5 x i32> %0, i32 1
  %mul4 = shl nsw i32 %vecext3, 2
  %vecext5 = extractelement <5 x i32> %add, i32 1
  %mul6 = shl i32 %vecext5, 2
  %vecext7 = extractelement <5 x i32> %0, i32 2
  %vecext8 = extractelement <5 x i32> %add, i32 2
  %vecext9 = extractelement <5 x i32> %0, i32 3
  %vecext10 = extractelement <5 x i32> %add, i32 3
  %vecext11 = extractelement <5 x i32> %0, i32 4
  %vecext12 = extractelement <5 x i32> %add, i32 4
  %cmp121 = icmp slt i32 %mul, %mul2
  br i1 %cmp121, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp16117 = icmp slt i32 %vecext11, %vecext12
  %cmp21113 = icmp slt i32 %vecext9, %vecext10
  %cmp26109 = icmp slt i32 %vecext7, %vecext8
  %cmp31106 = icmp slt i32 %mul4, %mul6
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup17
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup17, %for.body.lr.ph
  %d.0123 = phi i32 [ %mul, %for.body.lr.ph ], [ %add46, %for.cond.cleanup17 ]
  %ifmCoords.0122 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %ifmCoords.1.lcssa, %for.cond.cleanup17 ]
  %vecins = insertelement <5 x i32> %ifmCoords.0122, i32 %d.0123, i32 0
  br i1 %cmp16117, label %for.body18.preheader, label %for.cond.cleanup17

for.body18.preheader:                             ; preds = %for.body
  br label %for.body18

for.cond.cleanup17.loopexit:                      ; preds = %for.cond.cleanup22
  %ifmCoords.2.lcssa.lcssa = phi <5 x i32> [ %ifmCoords.2.lcssa, %for.cond.cleanup22 ]
  br label %for.cond.cleanup17

for.cond.cleanup17:                               ; preds = %for.cond.cleanup17.loopexit, %for.body
  %ifmCoords.1.lcssa = phi <5 x i32> [ %vecins, %for.body ], [ %ifmCoords.2.lcssa.lcssa, %for.cond.cleanup17.loopexit ]
  %add46 = add nsw i32 %d.0123, 64
  %cmp = icmp slt i32 %add46, %mul2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit, !llvm.loop !4

for.body18:                                       ; preds = %for.cond.cleanup22, %for.body18.preheader
  %f.0119 = phi i32 [ %add43, %for.cond.cleanup22 ], [ %vecext11, %for.body18.preheader ]
  %ifmCoords.1118 = phi <5 x i32> [ %ifmCoords.2.lcssa, %for.cond.cleanup22 ], [ %vecins, %for.body18.preheader ]
  %vecins19 = insertelement <5 x i32> %ifmCoords.1118, i32 %f.0119, i32 4
  br i1 %cmp21113, label %for.body23.preheader, label %for.cond.cleanup22

for.body23.preheader:                             ; preds = %for.body18
  br label %for.body23

for.cond.cleanup22.loopexit:                      ; preds = %for.cond.cleanup27
  %ifmCoords.3.lcssa.lcssa = phi <5 x i32> [ %ifmCoords.3.lcssa, %for.cond.cleanup27 ]
  br label %for.cond.cleanup22

for.cond.cleanup22:                               ; preds = %for.cond.cleanup22.loopexit, %for.body18
  %ifmCoords.2.lcssa = phi <5 x i32> [ %vecins19, %for.body18 ], [ %ifmCoords.3.lcssa.lcssa, %for.cond.cleanup22.loopexit ]
  %add43 = add i32 %f.0119, 1
  %exitcond126 = icmp eq i32 %add43, %vecext12
  br i1 %exitcond126, label %for.cond.cleanup17.loopexit, label %for.body18, !llvm.loop !6

for.body23:                                       ; preds = %for.cond.cleanup27, %for.body23.preheader
  %b.0115 = phi i32 [ %add40, %for.cond.cleanup27 ], [ %vecext9, %for.body23.preheader ]
  %ifmCoords.2114 = phi <5 x i32> [ %ifmCoords.3.lcssa, %for.cond.cleanup27 ], [ %vecins19, %for.body23.preheader ]
  %vecins24 = insertelement <5 x i32> %ifmCoords.2114, i32 %b.0115, i32 3
  br i1 %cmp26109, label %for.body28.preheader, label %for.cond.cleanup27

for.body28.preheader:                             ; preds = %for.body23
  br label %for.body28

for.cond.cleanup27.loopexit:                      ; preds = %for.cond.cleanup32
  %ifmCoords.4.lcssa.lcssa = phi <5 x i32> [ %ifmCoords.4.lcssa, %for.cond.cleanup32 ]
  br label %for.cond.cleanup27

for.cond.cleanup27:                               ; preds = %for.cond.cleanup27.loopexit, %for.body23
  %ifmCoords.3.lcssa = phi <5 x i32> [ %vecins24, %for.body23 ], [ %ifmCoords.4.lcssa.lcssa, %for.cond.cleanup27.loopexit ]
  %add40 = add i32 %b.0115, 1
  %exitcond125 = icmp eq i32 %add40, %vecext10
  br i1 %exitcond125, label %for.cond.cleanup22.loopexit, label %for.body23, !llvm.loop !7

for.body28:                                       ; preds = %for.cond.cleanup32, %for.body28.preheader
  %h.0111 = phi i32 [ %add37, %for.cond.cleanup32 ], [ %vecext7, %for.body28.preheader ]
  %ifmCoords.3110 = phi <5 x i32> [ %ifmCoords.4.lcssa, %for.cond.cleanup32 ], [ %vecins24, %for.body28.preheader ]
  %vecins29 = insertelement <5 x i32> %ifmCoords.3110, i32 %h.0111, i32 2
  br i1 %cmp31106, label %for.body33.preheader, label %for.cond.cleanup32

for.body33.preheader:                             ; preds = %for.body28
  br label %for.body33

for.cond.cleanup32.loopexit:                      ; preds = %for.body33
  %vecins34.lcssa = phi <5 x i32> [ %vecins34.3, %for.body33 ]
  br label %for.cond.cleanup32

for.cond.cleanup32:                               ; preds = %for.cond.cleanup32.loopexit, %for.body28
  %ifmCoords.4.lcssa = phi <5 x i32> [ %vecins29, %for.body28 ], [ %vecins34.lcssa, %for.cond.cleanup32.loopexit ]
  %add37 = add i32 %h.0111, 1
  %exitcond124 = icmp eq i32 %add37, %vecext8
  br i1 %exitcond124, label %for.cond.cleanup27.loopexit, label %for.body28, !llvm.loop !8

for.body33:                                       ; preds = %for.body33, %for.body33.preheader
  %w.0108 = phi i32 [ %mul4, %for.body33.preheader ], [ %add35.3, %for.body33 ]
  %ifmCoords.4107 = phi <5 x i32> [ %vecins29, %for.body33.preheader ], [ %vecins34.3, %for.body33 ]
  %vecins34 = insertelement <5 x i32> %ifmCoords.4107, i32 %w.0108, i32 1
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins34, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins34, i8 1, <64 x float> %2, i32 0, i1 true, i1 false)
  %add35 = add i32 %w.0108, 1
  %vecins34.1 = insertelement <5 x i32> %vecins34, i32 %add35, i32 1
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins34.1, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins34.1, i8 1, <64 x float> %3, i32 0, i1 true, i1 false)
  %add35.1 = add i32 %add35, 1
  %vecins34.2 = insertelement <5 x i32> %vecins34.1, i32 %add35.1, i32 1
  %4 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins34.2, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins34.2, i8 1, <64 x float> %4, i32 0, i1 true, i1 false)
  %add35.2 = add i32 %add35.1, 1
  %vecins34.3 = insertelement <5 x i32> %vecins34.2, i32 %add35.2, i32 1
  %5 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins34.3, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins34.3, i8 1, <64 x float> %5, i32 0, i1 true, i1 false)
  %add35.3 = add i32 %add35.2, 1
  %exitcond.3 = icmp eq i32 %add35.3, %mul6
  br i1 %exitcond.3, label %for.cond.cleanup32.loopexit, label %for.body33, !llvm.loop !9
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "target-cpu"="gaudi" }
attributes #2 = { nounwind writeonly "target-cpu"="gaudi" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 006866d4c86c5ef82a1c4a51d5f7c67cfd49d65d)"}
!2 = !{i32 0}
!3 = !{!"{4, 4}"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
!9 = distinct !{!9, !5, !10}
!10 = !{!"llvm.loop.unroll.disable"}
