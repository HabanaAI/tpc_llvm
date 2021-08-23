; RUN: llc -O2 %s -o - | FileCheck %s
; CHECK: tpc_metadata:
; CHECK-NEXT:  .asciz	"{{.+}}"
; CHECK-NEXT:  .size	tpc_metadata, 262

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"


; Function Attrs: nounwind
define dso_local void @main(i16 signext %aboveZeroScale, i16 signext %aboveZeroShift, i16 signext %belowZeroScale, i16 signext %belowZeroShift) local_unnamed_addr #0 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %2 = tail call i32 @llvm.tpc.ld.l.i32(i32 1116, i32 1, i32 undef, i1 true, i1 false) #4
  %3 = tail call i32 @llvm.tpc.ld.l.i32(i32 1128, i32 1, i32 undef, i1 true, i1 false) #4
  %4 = tail call i32 @llvm.tpc.ld.l.i32(i32 1140, i32 1, i32 undef, i1 true, i1 false) #4
  %5 = tail call i32 @llvm.tpc.ld.l.i32(i32 1152, i32 1, i32 undef, i1 true, i1 false) #4
  %6 = icmp eq i32 %2, 1
  %7 = icmp eq i32 %3, 1
  %8 = icmp eq i32 %4, 1
  %9 = icmp eq i32 %5, 1
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 7
  %vecext7 = extractelement <5 x i32> %add, i32 0
  %mul8 = shl nsw i32 %vecext7, 7
  %vecext9 = extractelement <5 x i32> %0, i32 1
  %mul10 = shl nsw i32 %vecext9, 2
  %vecext11 = extractelement <5 x i32> %add, i32 1
  %mul12 = shl i32 %vecext11, 2
  %vecext13 = extractelement <5 x i32> %0, i32 2
  %vecext14 = extractelement <5 x i32> %add, i32 2
  %vecext15 = extractelement <5 x i32> %0, i32 3
  %vecext16 = extractelement <5 x i32> %add, i32 3
  %cmp155 = icmp slt i32 %mul, %mul8
  br i1 %cmp155, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp19149 = icmp slt i32 %vecext15, %vecext16
  %cmp26143 = icmp slt i32 %vecext13, %vecext14
  %cmp33138 = icmp slt i32 %mul10, %mul12
  %conv41 = trunc i16 %belowZeroShift to i8
  %splat.splatinsert = insertelement <256 x i8> undef, i8 %conv41, i32 0
  %splat.splat = shufflevector <256 x i8> %splat.splatinsert, <256 x i8> undef, <256 x i32> zeroinitializer
  %conv42 = trunc i16 %aboveZeroShift to i8
  %splat.splatinsert43 = insertelement <256 x i8> undef, i8 %conv42, i32 0
  %splat.splat44 = shufflevector <256 x i8> %splat.splatinsert43, <256 x i8> undef, <256 x i32> zeroinitializer
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup21
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.cond.cleanup21
  %slopeCoords0.0158 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %slopeCoords0.1.lcssa, %for.cond.cleanup21 ]
  %coords0.0157 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %coords0.1.lcssa, %for.cond.cleanup21 ]
  %d.0156 = phi i32 [ %mul, %for.body.lr.ph ], [ %add53, %for.cond.cleanup21 ]
  %vecins = insertelement <5 x i32> %coords0.0157, i32 %d.0156, i32 0
  %10 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins, i32 1, i32 0, <5 x i32> %slopeCoords0.0158, i1 %6, i1 true)
  br i1 %cmp19149, label %for.body22.preheader, label %for.cond.cleanup21

for.body22.preheader:                             ; preds = %for.body
  br label %for.body22

for.cond.cleanup21.loopexit:                      ; preds = %for.cond.cleanup28
  %coords0.2.lcssa.lcssa = phi <5 x i32> [ %coords0.2.lcssa, %for.cond.cleanup28 ]
  %slopeCoords0.2.lcssa.lcssa = phi <5 x i32> [ %slopeCoords0.2.lcssa, %for.cond.cleanup28 ]
  br label %for.cond.cleanup21

for.cond.cleanup21:                               ; preds = %for.cond.cleanup21.loopexit, %for.body
  %coords0.1.lcssa = phi <5 x i32> [ %vecins, %for.body ], [ %coords0.2.lcssa.lcssa, %for.cond.cleanup21.loopexit ]
  %slopeCoords0.1.lcssa = phi <5 x i32> [ %10, %for.body ], [ %slopeCoords0.2.lcssa.lcssa, %for.cond.cleanup21.loopexit ]
  %add53 = add nsw i32 %d.0156, 128
  %cmp = icmp slt i32 %add53, %mul8
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit, !llvm.loop !4

for.body22:                                       ; preds = %for.body22.preheader, %for.cond.cleanup28
  %slopeCoords0.1152 = phi <5 x i32> [ %slopeCoords0.2.lcssa, %for.cond.cleanup28 ], [ %10, %for.body22.preheader ]
  %coords0.1151 = phi <5 x i32> [ %coords0.2.lcssa, %for.cond.cleanup28 ], [ %vecins, %for.body22.preheader ]
  %b.0150 = phi i32 [ %add50, %for.cond.cleanup28 ], [ %vecext15, %for.body22.preheader ]
  %vecins23 = insertelement <5 x i32> %coords0.1151, i32 %b.0150, i32 3
  %11 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins23, i32 8, i32 0, <5 x i32> %slopeCoords0.1152, i1 %9, i1 true)
  br i1 %cmp26143, label %for.body29.preheader, label %for.cond.cleanup28

for.body29.preheader:                             ; preds = %for.body22
  br label %for.body29

for.cond.cleanup28.loopexit:                      ; preds = %for.cond.cleanup35
  %coords0.3.lcssa.lcssa = phi <5 x i32> [ %coords0.3.lcssa, %for.cond.cleanup35 ]
  %slopeCoords0.3.lcssa.lcssa = phi <5 x i32> [ %slopeCoords0.3.lcssa, %for.cond.cleanup35 ]
  br label %for.cond.cleanup28

for.cond.cleanup28:                               ; preds = %for.cond.cleanup28.loopexit, %for.body22
  %coords0.2.lcssa = phi <5 x i32> [ %vecins23, %for.body22 ], [ %coords0.3.lcssa.lcssa, %for.cond.cleanup28.loopexit ]
  %slopeCoords0.2.lcssa = phi <5 x i32> [ %11, %for.body22 ], [ %slopeCoords0.3.lcssa.lcssa, %for.cond.cleanup28.loopexit ]
  %add50 = add nsw i32 %b.0150, 1
  %exitcond159 = icmp eq i32 %add50, %vecext16
  br i1 %exitcond159, label %for.cond.cleanup21.loopexit, label %for.body22, !llvm.loop !6

for.body29:                                       ; preds = %for.body29.preheader, %for.cond.cleanup35
  %slopeCoords0.2146 = phi <5 x i32> [ %slopeCoords0.3.lcssa, %for.cond.cleanup35 ], [ %11, %for.body29.preheader ]
  %coords0.2145 = phi <5 x i32> [ %coords0.3.lcssa, %for.cond.cleanup35 ], [ %vecins23, %for.body29.preheader ]
  %h.0144 = phi i32 [ %add47, %for.cond.cleanup35 ], [ %vecext13, %for.body29.preheader ]
  %vecins30 = insertelement <5 x i32> %coords0.2145, i32 %h.0144, i32 2
  %12 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins30, i32 4, i32 0, <5 x i32> %slopeCoords0.2146, i1 %8, i1 true)
  br i1 %cmp33138, label %for.body36.lr.ph, label %for.cond.cleanup35

for.body36.lr.ph:                                 ; preds = %for.body29
  %13 = tail call <128 x i16> @llvm.tpc.mov.v128i16.i16.i1(i16 0, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  br label %for.body36

for.cond.cleanup35.loopexit:                      ; preds = %for.body36
  %vecins37.3.lcssa = phi <5 x i32> [ %vecins37.3, %for.body36 ]
  %.lcssa = phi <5 x i32> [ %41, %for.body36 ]
  br label %for.cond.cleanup35

for.cond.cleanup35:                               ; preds = %for.cond.cleanup35.loopexit, %for.body29
  %coords0.3.lcssa = phi <5 x i32> [ %vecins30, %for.body29 ], [ %vecins37.3.lcssa, %for.cond.cleanup35.loopexit ]
  %slopeCoords0.3.lcssa = phi <5 x i32> [ %12, %for.body29 ], [ %.lcssa, %for.cond.cleanup35.loopexit ]
  %add47 = add nsw i32 %h.0144, 1
  %exitcond = icmp eq i32 %add47, %vecext14
  br i1 %exitcond, label %for.cond.cleanup28.loopexit, label %for.body29, !llvm.loop !7

for.body36:                                       ; preds = %for.body36, %for.body36.lr.ph
  %slopeCoords0.3141 = phi <5 x i32> [ %12, %for.body36.lr.ph ], [ %41, %for.body36 ]
  %coords0.3140 = phi <5 x i32> [ %vecins30, %for.body36.lr.ph ], [ %vecins37.3, %for.body36 ]
  %w.0139 = phi i32 [ %mul10, %for.body36.lr.ph ], [ %add45.3, %for.body36 ]
  %vecins37 = insertelement <5 x i32> %coords0.3140, i32 %w.0139, i32 1
  %14 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins37, i32 2, i32 0, <5 x i32> %slopeCoords0.3141, i1 %7, i1 true)
  %15 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %14, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %16 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)* %15, i32 0, <128 x i16> zeroinitializer, i1 %6, i1 false)
  %17 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %14, i8 1, i32 0, <128 x i16> %16, i1 %6, i1 true)
  %18 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %vecins37, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %19 = tail call <256 x i1> @llvm.tpc.cmp.less.v256i1.v128i16.i16.i1(<128 x i16> %18, i16 0, i8 7, i32 0, <256 x i1> undef, i1 true, i1 false)
  %20 = tail call <128 x i16> @llvm.tpc.msac.v128i16.v128i16.i8.v256i1(<128 x i16> %18, <128 x i16> %17, <256 x i8> zeroinitializer, i8 0, i8 7, i32 2, <128 x i16> %13, <256 x i1> %19, i1 false)
  %21 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %20, i16 %belowZeroScale, <256 x i8> %splat.splat, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %13, <256 x i1> %19, i1 false)
  %22 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %18, i16 %aboveZeroScale, <256 x i8> %splat.splat44, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %21, <256 x i1> %19, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins37, i8 2, <128 x i16> %22, i32 0, i1 true, i1 false)
  %add45 = or i32 %w.0139, 1
  %vecins37.1 = insertelement <5 x i32> %vecins37, i32 %add45, i32 1
  %23 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins37.1, i32 2, i32 0, <5 x i32> %14, i1 %7, i1 true)
  %24 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %23, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %25 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)* %24, i32 0, <128 x i16> zeroinitializer, i1 %6, i1 false)
  %26 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %23, i8 1, i32 0, <128 x i16> %25, i1 %6, i1 true)
  %27 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %vecins37.1, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %28 = tail call <256 x i1> @llvm.tpc.cmp.less.v256i1.v128i16.i16.i1(<128 x i16> %27, i16 0, i8 7, i32 0, <256 x i1> undef, i1 true, i1 false)
  %29 = tail call <128 x i16> @llvm.tpc.msac.v128i16.v128i16.i8.v256i1(<128 x i16> %27, <128 x i16> %26, <256 x i8> zeroinitializer, i8 0, i8 7, i32 2, <128 x i16> %13, <256 x i1> %28, i1 false)
  %30 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %29, i16 %belowZeroScale, <256 x i8> %splat.splat, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %13, <256 x i1> %28, i1 false)
  %31 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %27, i16 %aboveZeroScale, <256 x i8> %splat.splat44, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %30, <256 x i1> %28, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins37.1, i8 2, <128 x i16> %31, i32 0, i1 true, i1 false)
  %add45.1 = or i32 %w.0139, 2
  %vecins37.2 = insertelement <5 x i32> %vecins37.1, i32 %add45.1, i32 1
  %32 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins37.2, i32 2, i32 0, <5 x i32> %23, i1 %7, i1 true)
  %33 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %32, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %34 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)* %33, i32 0, <128 x i16> zeroinitializer, i1 %6, i1 false)
  %35 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %32, i8 1, i32 0, <128 x i16> %34, i1 %6, i1 true)
  %36 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %vecins37.2, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %37 = tail call <256 x i1> @llvm.tpc.cmp.less.v256i1.v128i16.i16.i1(<128 x i16> %36, i16 0, i8 7, i32 0, <256 x i1> undef, i1 true, i1 false)
  %38 = tail call <128 x i16> @llvm.tpc.msac.v128i16.v128i16.i8.v256i1(<128 x i16> %36, <128 x i16> %35, <256 x i8> zeroinitializer, i8 0, i8 7, i32 2, <128 x i16> %13, <256 x i1> %37, i1 false)
  %39 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %38, i16 %belowZeroScale, <256 x i8> %splat.splat, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %13, <256 x i1> %37, i1 false)
  %40 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %36, i16 %aboveZeroScale, <256 x i8> %splat.splat44, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %39, <256 x i1> %37, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins37.2, i8 2, <128 x i16> %40, i32 0, i1 true, i1 false)
  %add45.2 = or i32 %w.0139, 3
  %vecins37.3 = insertelement <5 x i32> %vecins37.2, i32 %add45.2, i32 1
  %41 = tail call <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32> %vecins37.3, i32 2, i32 0, <5 x i32> %32, i1 %7, i1 true)
  %42 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %41, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %43 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)* %42, i32 0, <128 x i16> zeroinitializer, i1 %6, i1 false)
  %44 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %41, i8 1, i32 0, <128 x i16> %43, i1 %6, i1 true)
  %45 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %vecins37.3, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %46 = tail call <256 x i1> @llvm.tpc.cmp.less.v256i1.v128i16.i16.i1(<128 x i16> %45, i16 0, i8 7, i32 0, <256 x i1> undef, i1 true, i1 false)
  %47 = tail call <128 x i16> @llvm.tpc.msac.v128i16.v128i16.i8.v256i1(<128 x i16> %45, <128 x i16> %44, <256 x i8> zeroinitializer, i8 0, i8 7, i32 2, <128 x i16> %13, <256 x i1> %46, i1 false)
  %48 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %47, i16 %belowZeroScale, <256 x i8> %splat.splat, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %13, <256 x i1> %46, i1 false)
  %49 = tail call <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16> %45, i16 %aboveZeroScale, <256 x i8> %splat.splat44, <256 x i8> zeroinitializer, i8 7, i32 2, <128 x i16> %48, <256 x i1> %46, i1 true)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins37.3, i8 2, <128 x i16> %49, i32 0, i1 true, i1 false)
  %add45.3 = add nsw i32 %w.0139, 4
  %exitcond.3 = icmp eq i32 %add45.3, %mul12
  br i1 %exitcond.3, label %for.cond.cleanup35.loopexit, label %for.body36, !llvm.loop !8
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.mov.mask.v5i32(<5 x i32>, i32, i32, <5 x i32>, i1, i1) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; Function Attrs: nounwind readonly
declare <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)*, i32, <128 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i1> @llvm.tpc.cmp.less.v256i1.v128i16.i16.i1(<128 x i16>, i16, i8, i32, <256 x i1>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.mov.v128i16.i16.i1(i16, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.msac.v128i16.v128i16.i8.v256i1(<128 x i16>, <128 x i16>, <256 x i8>, i8, i8, i32, <128 x i16>, <256 x i1>, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.msac.v128i16.i16.v256i8.v256i1(<128 x i16>, i16, <256 x i8>, <256 x i8>, i8, i32, <128 x i16>, <256 x i1>, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #3

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="dali" "target-features"="+dali" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://akodnani@gerrit.habana-labs.com:29418/tpc_clang2.git d595f2b7ab1c0c55eb0a80474d13243fbaac4fe4) (ssh://akodnani@gerrit.habana-labs.com:29418/tpc_llvm2.git 70a563161d0645a4395c1c0b11cec70e9f741c3f)"}
!2 = !{i32 0}
!3 = !{i32 256}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5, !9}
!9 = !{!"llvm.loop.unroll.disable"}
