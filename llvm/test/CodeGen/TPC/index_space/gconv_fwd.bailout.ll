; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index"
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[2].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index"
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[2].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

target triple = "tpc"

; Function Attrs: norecurse nounwind
define dso_local void @main(i32 %kPerGroup, i32 %numGroups) local_unnamed_addr #0 !unroll_info !3 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 7
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul2 = shl nsw i32 %vecext1, 7
  %vecext3 = extractelement <5 x i32> %0, i32 1
  %vecext5 = extractelement <5 x i32> %add, i32 1
  %vecext7 = extractelement <5 x i32> %0, i32 2
  %vecext9 = extractelement <5 x i32> %add, i32 2
  %vecext11 = extractelement <5 x i32> %0, i32 3
  %vecext13 = extractelement <5 x i32> %add, i32 3
  %2 = tail call i32 @llvm.tpc.ld.l.i32(i32 24, i32 1, i32 undef, i1 true, i1 false)
  %cmp128 = icmp slt i32 %vecext11, %vecext13
  br i1 %cmp128, label %for.cond15.preheader.lr.ph, label %for.cond.cleanup

for.cond15.preheader.lr.ph:                       ; preds = %entry
  %cmp16125 = icmp slt i32 %vecext7, %vecext9
  %cmp20122 = icmp slt i32 %vecext3, %vecext5
  %cmp24120 = icmp slt i32 %mul, %mul2
  %cmp43116 = icmp sgt i32 %numGroups, 0
  %3 = add i32 %numGroups, -1
  %xtraiter = and i32 %numGroups, 7
  %4 = icmp ult i32 %3, 7
  %unroll_iter = sub nuw nsw i32 %numGroups, %xtraiter
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br label %for.cond15.preheader

for.cond15.preheader:                             ; preds = %for.cond.cleanup17, %for.cond15.preheader.lr.ph
  %b.0129 = phi i32 [ %vecext11, %for.cond15.preheader.lr.ph ], [ %add61, %for.cond.cleanup17 ]
  br i1 %cmp16125, label %for.cond19.preheader.preheader, label %for.cond.cleanup17

for.cond19.preheader.preheader:                   ; preds = %for.cond15.preheader
  br label %for.cond19.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup17
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond19.preheader:                             ; preds = %for.cond19.preheader.preheader, %for.cond.cleanup21
  %h.0126 = phi i32 [ %add58, %for.cond.cleanup21 ], [ %vecext7, %for.cond19.preheader.preheader ]
  br i1 %cmp20122, label %for.cond23.preheader.preheader, label %for.cond.cleanup21

for.cond23.preheader.preheader:                   ; preds = %for.cond19.preheader
  br label %for.cond23.preheader

for.cond.cleanup17.loopexit:                      ; preds = %for.cond.cleanup21
  br label %for.cond.cleanup17

for.cond.cleanup17:                               ; preds = %for.cond.cleanup17.loopexit, %for.cond15.preheader
  %add61 = add i32 %b.0129, 1
  %exitcond133 = icmp eq i32 %add61, %vecext13
  br i1 %exitcond133, label %for.cond.cleanup.loopexit, label %for.cond15.preheader, !llvm.loop !4

for.cond23.preheader:                             ; preds = %for.cond23.preheader.preheader, %for.cond.cleanup25
  %w.0123 = phi i32 [ %add55, %for.cond.cleanup25 ], [ %vecext3, %for.cond23.preheader.preheader ]
  br i1 %cmp24120, label %for.body26.preheader, label %for.cond.cleanup25

for.body26.preheader:                             ; preds = %for.cond23.preheader
  br label %for.body26

for.cond.cleanup21.loopexit:                      ; preds = %for.cond.cleanup25
  br label %for.cond.cleanup21

for.cond.cleanup21:                               ; preds = %for.cond.cleanup21.loopexit, %for.cond19.preheader
  %add58 = add i32 %h.0126, 1
  %exitcond132 = icmp eq i32 %add58, %vecext9
  br i1 %exitcond132, label %for.cond.cleanup17.loopexit, label %for.cond19.preheader, !llvm.loop !6

for.cond.cleanup25.loopexit:                      ; preds = %for.cond.cleanup44
  br label %for.cond.cleanup25

for.cond.cleanup25:                               ; preds = %for.cond.cleanup25.loopexit, %for.cond23.preheader
  %add55 = add i32 %w.0123, 1
  %exitcond131 = icmp eq i32 %add55, %vecext5
  br i1 %exitcond131, label %for.cond.cleanup21.loopexit, label %for.cond23.preheader, !llvm.loop !7

for.body26:                                       ; preds = %for.body26.preheader, %for.cond.cleanup44
  %d.0121 = phi i32 [ %add52, %for.cond.cleanup44 ], [ %mul, %for.body26.preheader ]
  %5 = insertelement <5 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 0>, i32 %d.0121, i32 0
  %6 = insertelement <5 x i32> %5, i32 %w.0123, i32 1
  %7 = insertelement <5 x i32> %6, i32 %h.0126, i32 2
  %vecinit30 = insertelement <5 x i32> %7, i32 %b.0129, i32 3
  %8 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %vecinit30, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %mul36 = shl i32 %d.0121, 1
  %vecinit41 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %mul36, i32 0
  br i1 %cmp43116, label %for.body45.preheader, label %for.cond.cleanup44

for.body45.preheader:                             ; preds = %for.body26
  br i1 %4, label %for.cond.cleanup44.loopexit.unr-lcssa, label %for.body45.preheader140

for.body45.preheader140:                          ; preds = %for.body45.preheader
  br label %for.body45

for.cond.cleanup44.loopexit.unr-lcssa.loopexit:   ; preds = %for.body45
  %.lcssa141 = phi <5 x i32> [ %42, %for.body45 ]
  %.lcssa = phi <5 x i32> [ %44, %for.body45 ]
  br label %for.cond.cleanup44.loopexit.unr-lcssa

for.cond.cleanup44.loopexit.unr-lcssa:            ; preds = %for.cond.cleanup44.loopexit.unr-lcssa.loopexit, %for.body45.preheader
  %coordsMask.0118.unr = phi <5 x i32> [ %vecinit41, %for.body45.preheader ], [ %.lcssa141, %for.cond.cleanup44.loopexit.unr-lcssa.loopexit ]
  %coordsOut.0117.unr = phi <5 x i32> [ %vecinit30, %for.body45.preheader ], [ %.lcssa, %for.cond.cleanup44.loopexit.unr-lcssa.loopexit ]
  br i1 %lcmp.mod, label %for.cond.cleanup44, label %for.body45.epil.preheader

for.body45.epil.preheader:                        ; preds = %for.cond.cleanup44.loopexit.unr-lcssa
  br label %for.body45.epil

for.body45.epil:                                  ; preds = %for.body45.epil.preheader, %for.body45.epil
  %coordsMask.0118.epil = phi <5 x i32> [ %10, %for.body45.epil ], [ %coordsMask.0118.unr, %for.body45.epil.preheader ]
  %coordsOut.0117.epil = phi <5 x i32> [ %12, %for.body45.epil ], [ %coordsOut.0117.unr, %for.body45.epil.preheader ]
  %epil.iter = phi i32 [ %epil.iter.sub, %for.body45.epil ], [ %xtraiter, %for.body45.epil.preheader ]
  %9 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %coordsMask.0118.epil, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %10 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coordsMask.0118.epil, i32 1, i32 2, i8 2, i32 0, <5 x i32> %coordsMask.0118.epil, i1 true, i1 false)
  %11 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %9, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %coordsOut.0117.epil, i8 1, <128 x bfloat> %11, i32 0, i1 true, i1 false)
  %12 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coordsOut.0117.epil, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %coordsOut.0117.epil, i1 true, i1 false)
  %epil.iter.sub = add i32 %epil.iter, -1
  %epil.iter.cmp = icmp eq i32 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.cond.cleanup44.loopexit, label %for.body45.epil, !llvm.loop !8

for.cond.cleanup44.loopexit:                      ; preds = %for.body45.epil
  br label %for.cond.cleanup44

for.cond.cleanup44:                               ; preds = %for.cond.cleanup44.loopexit, %for.cond.cleanup44.loopexit.unr-lcssa, %for.body26
  %add52 = add nsw i32 %d.0121, 128
  %cmp24 = icmp slt i32 %add52, %mul2
  br i1 %cmp24, label %for.body26, label %for.cond.cleanup25.loopexit, !llvm.loop !11

for.body45:                                       ; preds = %for.body45.preheader140, %for.body45
  %coordsMask.0118 = phi <5 x i32> [ %42, %for.body45 ], [ %vecinit41, %for.body45.preheader140 ]
  %coordsOut.0117 = phi <5 x i32> [ %44, %for.body45 ], [ %vecinit30, %for.body45.preheader140 ]
  %niter = phi i32 [ %niter.nsub.7, %for.body45 ], [ %unroll_iter, %for.body45.preheader140 ]
  %13 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %coordsMask.0118, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %14 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coordsMask.0118, i32 1, i32 2, i8 2, i32 0, <5 x i32> %coordsMask.0118, i1 true, i1 false)
  %15 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %13, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %coordsOut.0117, i8 1, <128 x bfloat> %15, i32 0, i1 true, i1 false)
  %16 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %coordsOut.0117, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %coordsOut.0117, i1 true, i1 false)
  %17 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %14, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %18 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %14, i32 1, i32 2, i8 2, i32 0, <5 x i32> %14, i1 true, i1 false)
  %19 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %17, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %16, i8 1, <128 x bfloat> %19, i32 0, i1 true, i1 false)
  %20 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %16, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %16, i1 true, i1 false)
  %21 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %18, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %22 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %18, i32 1, i32 2, i8 2, i32 0, <5 x i32> %18, i1 true, i1 false)
  %23 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %21, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %20, i8 1, <128 x bfloat> %23, i32 0, i1 true, i1 false)
  %24 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %20, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %20, i1 true, i1 false)
  %25 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %22, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %26 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %22, i32 1, i32 2, i8 2, i32 0, <5 x i32> %22, i1 true, i1 false)
  %27 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %25, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %24, i8 1, <128 x bfloat> %27, i32 0, i1 true, i1 false)
  %28 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %24, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %24, i1 true, i1 false)
  %29 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %26, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %30 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %26, i32 1, i32 2, i8 2, i32 0, <5 x i32> %26, i1 true, i1 false)
  %31 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %29, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %28, i8 1, <128 x bfloat> %31, i32 0, i1 true, i1 false)
  %32 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %28, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %28, i1 true, i1 false)
  %33 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %30, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %34 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %30, i32 1, i32 2, i8 2, i32 0, <5 x i32> %30, i1 true, i1 false)
  %35 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %33, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %32, i8 1, <128 x bfloat> %35, i32 0, i1 true, i1 false)
  %36 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %32, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %32, i1 true, i1 false)
  %37 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %34, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %38 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %34, i32 1, i32 2, i8 2, i32 0, <5 x i32> %34, i1 true, i1 false)
  %39 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %37, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %36, i8 1, <128 x bfloat> %39, i32 0, i1 true, i1 false)
  %40 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %36, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %36, i1 true, i1 false)
  %41 = tail call <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32> %38, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %42 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %38, i32 1, i32 2, i8 2, i32 0, <5 x i32> %38, i1 true, i1 false)
  %43 = tail call <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat> zeroinitializer, i8 1, i32 0, <128 x bfloat> %8, <256 x i1> %41, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %40, i8 1, <128 x bfloat> %43, i32 0, i1 true, i1 false)
  %44 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %40, i32 %2, i32 2, i8 2, i32 0, <5 x i32> %40, i1 true, i1 false)
  %niter.nsub.7 = add i32 %niter, -8
  %niter.ncmp.7 = icmp eq i32 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.cleanup44.loopexit.unr-lcssa.loopexit, label %for.body45, !llvm.loop !12
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i1> @llvm.tpc.ld.tnsr.v256i1.i1(<5 x i32>, i8, i32, <256 x i1>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.mov.v128bf16.v128bf16.v256i1(<128 x bfloat>, i8, i32, <128 x bfloat>, <256 x i1>, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #2

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #3

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi2" "target-features"="+gaudi2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind readonly }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 0}
!3 = !{!"{4, 8}"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !9, !10}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = !{!"llvm.loop.unroll.remainderloop.marker", i32 0}
!11 = distinct !{!11, !5}
!12 = distinct !{!12, !9}
