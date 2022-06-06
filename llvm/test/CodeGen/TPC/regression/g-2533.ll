; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=GAUDIX %s
target triple = "tpc"

; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"

declare <5 x i32> @llvm.tpc.get.index.space.offset()

declare <5 x i32> @llvm.tpc.get.index.space.size()

define void @main() #0 !dbg !3 {
  %1 = call <5 x i32> @llvm.tpc.get.index.space.offset(), !dbg !7
  %2 = call <5 x i32> @llvm.tpc.get.index.space.size(), !dbg !7
  %3 = add <5 x i32> %1, %2, !dbg !7
  %4 = extractelement <5 x i32> %1, i32 0, !dbg !7
  %5 = shl i32 %4, 7, !dbg !7
  %6 = extractelement <5 x i32> %3, i32 0, !dbg !7
  %7 = shl i32 %6, 7, !dbg !7
  br label %8, !dbg !9

8:                                                ; preds = %11, %0
  %9 = phi i32 [ %25, %11 ], [ %5, %0 ]
  %10 = icmp slt i32 %9, %7, !dbg !9
  br i1 %10, label %11, label %26, !dbg !9

11:                                               ; preds = %8
  %12 = insertelement <5 x i32> <i32 poison, i32 0, i32 0, i32 0, i32 0>, i32 %9, i32 0, !dbg !9
  %13 = call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> %12, i8 0, i32 0, <128 x half> undef, i1 true, i1 false), !dbg !9
  %14 = call <128 x float> @llvm.tpc.fpext.swch.v128f32.v128f16(<128 x half> %13, i32 327680), !dbg !9
  %15 = shufflevector <128 x float> %14, <128 x float> poison, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>, !dbg !9
  %16 = shufflevector <128 x float> %14, <128 x float> poison, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>, !dbg !9
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %12, i8 1, <64 x float> %15, i32 0, i1 true, i1 false), !dbg !9
  %17 = or i32 %9, 64, !dbg !9
  %18 = insertelement <5 x i32> <i32 poison, i32 0, i32 0, i32 0, i32 0>, i32 %17, i32 0, !dbg !9
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %18, i8 1, <64 x float> %16, i32 0, i1 true, i1 false), !dbg !9
  %19 = insertelement <5 x i32> <i32 poison, i32 1, i32 0, i32 0, i32 0>, i32 %9, i32 0, !dbg !9
  %20 = call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> %19, i8 0, i32 0, <128 x half> undef, i1 true, i1 false), !dbg !9
  %21 = call <128 x float> @llvm.tpc.fpext.swch.v128f32.v128f16(<128 x half> %20, i32 327680), !dbg !9
  %22 = shufflevector <128 x float> %21, <128 x float> poison, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>, !dbg !9
  %23 = shufflevector <128 x float> %21, <128 x float> poison, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>, !dbg !9
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %19, i8 1, <64 x float> %22, i32 0, i1 true, i1 false), !dbg !9
  %24 = insertelement <5 x i32> <i32 poison, i32 1, i32 0, i32 0, i32 0>, i32 %17, i32 0, !dbg !9
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %24, i8 1, <64 x float> %23, i32 0, i1 true, i1 false), !dbg !9
  %25 = add i32 %9, 128, !dbg !9
  br label %8, !dbg !9

26:                                               ; preds = %8
  ret void, !dbg !10
}

; Function Attrs: nounwind readnone
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.fpext.swch.v128f32.v128f16(<128 x half>, i32) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { "target-cpu"="gaudi2" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "mlir", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "fused_kernel", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "main", linkageName: "main", scope: null, file: !4, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!4 = !DIFile(filename: "within split at /home/mzuckerman/trees/npu-stack/tpc_mlir/mlir/habana/test/Integration/Synapse/runner/ops_gaudi2.mlir:88 offset ", directory: "/home/mzuckerman/trees/npu-stack/tpc_mlir")
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 1, scope: !8)
!8 = !DILexicalBlockFile(scope: !3, file: !4, discriminator: 0)
!9 = !DILocation(line: 9, column: 10, scope: !8)
!10 = !DILocation(line: 10, column: 3, scope: !8)


; GAUDIX:  convert.f16 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9]+}}  
; GAUDIX-NOT: all_lanes