; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=GAUDIX %s
target triple = "tpc"

; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"

declare <5 x i32> @llvm.tpc.get.index.space.offset()

declare <5 x i32> @llvm.tpc.get.index.space.size()

define void @main() #0 {
bb:
  %i = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %i1 = call <5 x i32> @llvm.tpc.get.index.space.size()
  %i2 = add <5 x i32> %i, %i1
  %i3 = extractelement <5 x i32> %i, i32 0
  %i4 = shl i32 %i3, 7
  %i5 = extractelement <5 x i32> %i2, i32 0
  %i6 = shl i32 %i5, 7
  %i7 = extractelement <5 x i32> %i, i32 1
  %i8 = extractelement <5 x i32> %i2, i32 1
  %i9 = extractelement <5 x i32> %i, i32 3
  %i10 = extractelement <5 x i32> %i2, i32 3
  br label %bb11

bb11:                                             ; preds = %bb46, %bb
  %i12 = phi i32 [ %i4, %bb ], [ %i47, %bb46 ]
  %i13 = icmp slt i32 %i12, %i6
  br i1 %i13, label %bb14, label %bb48

bb14:                                             ; preds = %bb11
  br label %bb15

bb15:                                             ; preds = %bb44, %bb14
  %i16 = phi i32 [ %i9, %bb14 ], [ %i45, %bb44 ]
  %i17 = icmp slt i32 %i16, %i10
  br i1 %i17, label %bb18, label %bb46

bb18:                                             ; preds = %bb15
  br label %bb19

bb19:                                             ; preds = %bb22, %bb18
  %i20 = phi i32 [ %i7, %bb18 ], [ %i43, %bb22 ]
  %i21 = icmp slt i32 %i20, %i8
  br i1 %i21, label %bb22, label %bb44

bb22:                                             ; preds = %bb19
  %i23 = insertelement <5 x i32> <i32 poison, i32 poison, i32 0, i32 poison, i32 0>, i32 %i12, i32 0
  %i24 = insertelement <5 x i32> %i23, i32 %i20, i32 1
  %i25 = insertelement <5 x i32> %i24, i32 %i16, i32 3
  %i26 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %i25, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i27 = call <128 x half> @llvm.tpc.uitofp.swch.v128f16.v128i16(<128 x i16> %i26, i32 327680)
  call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> %i25, i8 1, <128 x half> %i27, i32 0, i1 true, i1 false)
  %i28 = insertelement <5 x i32> <i32 poison, i32 poison, i32 1, i32 poison, i32 0>, i32 %i12, i32 0
  %i29 = insertelement <5 x i32> %i28, i32 %i20, i32 1
  %i30 = insertelement <5 x i32> %i29, i32 %i16, i32 3
  %i31 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %i30, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i32 = call <128 x half> @llvm.tpc.uitofp.swch.v128f16.v128i16(<128 x i16> %i31, i32 327680)
  call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> %i30, i8 1, <128 x half> %i32, i32 0, i1 true, i1 false)
  %i33 = insertelement <5 x i32> <i32 poison, i32 poison, i32 2, i32 poison, i32 0>, i32 %i12, i32 0
  %i34 = insertelement <5 x i32> %i33, i32 %i20, i32 1
  %i35 = insertelement <5 x i32> %i34, i32 %i16, i32 3
  %i36 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %i35, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i37 = call <128 x half> @llvm.tpc.uitofp.swch.v128f16.v128i16(<128 x i16> %i36, i32 327680)
  call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> %i35, i8 1, <128 x half> %i37, i32 0, i1 true, i1 false)
  %i38 = insertelement <5 x i32> <i32 poison, i32 poison, i32 3, i32 poison, i32 0>, i32 %i12, i32 0
  %i39 = insertelement <5 x i32> %i38, i32 %i20, i32 1
  %i40 = insertelement <5 x i32> %i39, i32 %i16, i32 3
  %i41 = call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %i40, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %i42 = call <128 x half> @llvm.tpc.uitofp.swch.v128f16.v128i16(<128 x i16> %i41, i32 327680)
  call void @llvm.tpc.st.tnsr.v128f16(<5 x i32> %i40, i8 1, <128 x half> %i42, i32 0, i1 true, i1 false)
  %i43 = add i32 %i20, 1
  br label %bb19

bb44:                                             ; preds = %bb19
  %i45 = add i32 %i16, 1
  br label %bb15

bb46:                                             ; preds = %bb15
  %i47 = add i32 %i12, 128
  br label %bb11

bb48:                                             ; preds = %bb11
  ret void
}

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.uitofp.swch.v128f16.v128i16(<128 x i16>, i32) #1

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


; GAUDIX:    convert.u16 all_lanes target_type=f16  [[VRF:%V[0-9]+]]
