; RUN: %tpc_clang -S -march=gaudi2 -O2 %s -o -
; GAUDI-2365
; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"

declare <5 x i32> @llvm.tpc.get.index.space.offset()

declare <5 x i32> @llvm.tpc.get.index.space.size()

define void @main() #0 {
bb:
  %i = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %i5 = call <5 x i32> @llvm.tpc.get.index.space.size()
  %i6 = add <5 x i32> %i, %i5
  %i7 = extractelement <5 x i32> %i, i32 0
  %i8 = shl i32 %i7, 7
  %i9 = extractelement <5 x i32> %i6, i32 0
  %i10 = shl i32 %i9, 7
  %i11 = extractelement <5 x i32> %i, i32 1
  %i12 = extractelement <5 x i32> %i6, i32 1
  %i13 = extractelement <5 x i32> %i, i32 2
  %i14 = extractelement <5 x i32> %i6, i32 2
  %i15 = extractelement <5 x i32> %i, i32 3
  %i16 = extractelement <5 x i32> %i6, i32 3
  br label %bb17

bb17:                                             ; preds = %bb40, %bb
  %.sroa.03.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.03.1, %bb40 ]
  %.sroa.0.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.0.1, %bb40 ]
  %i18 = phi i32 [ %i8, %bb ], [ %i41, %bb40 ]
  %i19 = icmp slt i32 %i18, %i10
  br i1 %i19, label %bb20, label %bb42

bb20:                                             ; preds = %bb17
  %.sroa.03.0.vec.insert = insertelement <5 x i32> %.sroa.03.0, i32 %i18, i32 0
  %.sroa.0.0.vec.insert = insertelement <5 x i32> %.sroa.0.0, i32 %i18, i32 0
  br label %bb21

bb21:                                             ; preds = %bb38, %bb20
  %.sroa.03.1 = phi <5 x i32> [ %.sroa.03.0.vec.insert, %bb20 ], [ %.sroa.03.2, %bb38 ]
  %.sroa.0.1 = phi <5 x i32> [ %.sroa.0.0.vec.insert, %bb20 ], [ %.sroa.0.2, %bb38 ]
  %i22 = phi i32 [ %i15, %bb20 ], [ %i39, %bb38 ]
  %i23 = icmp slt i32 %i22, %i16
  br i1 %i23, label %bb24, label %bb40

bb24:                                             ; preds = %bb21
  %.sroa.03.12.vec.insert = insertelement <5 x i32> %.sroa.03.1, i32 %i22, i32 3
  %.sroa.0.12.vec.insert = insertelement <5 x i32> %.sroa.0.1, i32 %i22, i32 3
  br label %bb25

bb25:                                             ; preds = %bb36, %bb24
  %.sroa.03.2 = phi <5 x i32> [ %.sroa.03.12.vec.insert, %bb24 ], [ %.sroa.03.3, %bb36 ]
  %.sroa.0.2 = phi <5 x i32> [ %.sroa.0.12.vec.insert, %bb24 ], [ %.sroa.0.3, %bb36 ]
  %i26 = phi i32 [ %i13, %bb24 ], [ %i37, %bb36 ]
  %i27 = icmp slt i32 %i26, %i14
  br i1 %i27, label %bb28, label %bb38

bb28:                                             ; preds = %bb25
  %.sroa.03.8.vec.insert = insertelement <5 x i32> %.sroa.03.2, i32 %i26, i32 2
  %.sroa.0.8.vec.insert = insertelement <5 x i32> %.sroa.0.2, i32 %i26, i32 2
  br label %bb29

bb29:                                             ; preds = %bb32, %bb28
  %.sroa.03.3 = phi <5 x i32> [ %.sroa.03.8.vec.insert, %bb28 ], [ %.sroa.03.4.vec.insert, %bb32 ]
  %.sroa.0.3 = phi <5 x i32> [ %.sroa.0.8.vec.insert, %bb28 ], [ %.sroa.0.4.vec.insert, %bb32 ]
  %i30 = phi i32 [ %i11, %bb28 ], [ %i35, %bb32 ]
  %i31 = icmp slt i32 %i30, %i12
  br i1 %i31, label %bb32, label %bb36

bb32:                                             ; preds = %bb29
  %.sroa.03.4.vec.insert = insertelement <5 x i32> %.sroa.03.3, i32 %i30, i32 1
  %.sroa.0.4.vec.insert = insertelement <5 x i32> %.sroa.0.3, i32 %i30, i32 1
  %i33 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %.sroa.0.4.vec.insert, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i34 = call <128 x half> @llvm.tpc.sitofp.swch.v128f16.v128i16(<128 x i16> %i33, i32 327680)
  call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> %.sroa.03.4.vec.insert, i8 1, <128 x half> %i34, i32 0, i1 true, i1 false)
  %i35 = add i32 %i30, 1
  br label %bb29

bb36:                                             ; preds = %bb29
  %i37 = add i32 %i26, 1
  br label %bb25

bb38:                                             ; preds = %bb25
  %i39 = add i32 %i22, 1
  br label %bb21

bb40:                                             ; preds = %bb21
  %i41 = add i32 %i18, 128
  br label %bb17

bb42:                                             ; preds = %bb17
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.sitofp.swch.v128f16.v128i16(<128 x i16>, i32) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128f16(<5 x i32>, i8, <128 x half>, i32, i1, i1) #2

attributes #0 = { "target-cpu"="gaudi2" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "mlir", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "fused_kernel", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
