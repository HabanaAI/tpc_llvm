; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

; CHECK: convert.i32
; CHECK: pack.i16
; CHECK: mov_dg

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

 

define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 7
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul2 = shl nsw i32 %vecext1, 7
  %vecinit6 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %mul, i32 0
  %cmp44 = icmp slt i32 %mul, %mul2
  br i1 %cmp44, label %for.body.preheader, label %for.cond.cleanup

 

for.body.preheader:                               ; preds = %entry
  br label %for.body

 

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

 

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

 

for.body:                                         ; preds = %for.body.preheader, %for.body
  %d.046 = phi i32 [ %add16, %for.body ], [ %mul, %for.body.preheader ]
  %ifmCoords.045 = phi <5 x i32> [ %vecins, %for.body ], [ %vecinit6, %for.body.preheader ]
  %vecins = insertelement <5 x i32> %ifmCoords.045, i32 %d.046, i32 0
  %2 = tail call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %vecins, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %3 = tail call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %vecins, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %4 = shufflevector <64 x i32> %2, <64 x i32> %3, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %5 = trunc <128 x i32> %4 to <128 x i16>
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins, i8 1, <128 x i16> %5, i32 0, i1 true, i1 false)
  %add16 = add nsw i32 %d.046, 128
  %cmp = icmp slt i32 %add16, %mul2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}
 

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

 

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

 

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #1

 

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #2

 

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.pack.v128i16.i1(<128 x i16>, i8, i32, <128 x i16>, i1, i1) #1

 

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.mov.dual.group.v128i16.i1(<128 x i16>, i32, i32, <128 x i16>, i1, i1) #1

 

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="goya2" "target-features"="+goya2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind }

