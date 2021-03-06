; RUN: llc -march=tpc -mcpu=dali -O1 %s -o - | FileCheck %s

; CHECK: convert.i16 target_type=int32  [[V1:%V[0-9]+]], {{%V[0-9]+}}
; CHECK: st_tnsr  0x1, {{%I[0-9]+}}, [[V1]]

target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 8
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul2 = shl nsw i32 %vecext1, 8
  %vecinit6 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %mul, i32 0
  %cmp31 = icmp slt i32 %mul, %mul2
  br i1 %cmp31, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %ifmCoords.033 = phi <5 x i32> [ %vecins, %for.body ], [ %vecinit6, %for.body.preheader ]
  %d.032 = phi i32 [ %add8, %for.body ], [ %mul, %for.body.preheader ]
  %vecins = insertelement <5 x i32> %ifmCoords.033, i32 %d.032, i32 0
  %2 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %vecins, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  ;%3 = tail call <128 x i32> @llvm.tpc.convert.v128i32.v128i16.i1(<128 x i16> %2, i8 7, i32 512, <128 x i32> undef, i1 true, i1 false)
  %3 = sext <128 x i16> %2 to <128 x i32>
  %4 = shufflevector <128 x i32> %3, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %5 = shufflevector <128 x i32> %3, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %vecins, i8 1, <64 x i32> %4, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %vecins, i8 1, <64 x i32> %5, i32 0, i1 true, i1 false)
  %add8 = add nsw i32 %d.032, 256
  %cmp = icmp slt i32 %add8, %mul2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i32> @llvm.tpc.convert.v128i32.v128i16.i1(<128 x i16>, i8, i32, <128 x i32>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="dali" "target-features"="+dali" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://kshaik@gerrit.habana-labs.com:29418/tpc_clang2.git f8db2bf692c43550c8b7d3f6f25aabcdfdb4198c) (ssh://kshaik@gerrit.habana-labs.com:29418/tpc_llvm2.git b4d9d8797fb5853d6c60771365e9b682b78869cb)"}
!2 = !{i32 0}
!3 = !{i32 256}
