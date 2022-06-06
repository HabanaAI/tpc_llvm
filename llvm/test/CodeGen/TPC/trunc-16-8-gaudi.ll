; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

; CHECK: convert.i16
; CHECK: pack.i8
; CHECK: mov_dg

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
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
  %cmp40 = icmp slt i32 %mul, %mul2
  br i1 %cmp40, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %ifmCoords.042 = phi <5 x i32> [ %vecins, %for.body ], [ %vecinit6, %for.body.preheader ]
  %d.041 = phi i32 [ %add8, %for.body ], [ %mul, %for.body.preheader ]
  %vecins = insertelement <5 x i32> %ifmCoords.042, i32 %d.041, i32 0
  %2 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %vecins, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %3 = tail call <256 x i16> @llvm.tpc.convert.v256i16.v256i8.v256i1(<256 x i8> %2, i8 4, i32 1799, <256 x i16> undef, <256 x i1> undef, i1 true)
  %4 = trunc <256 x i16> %3 to <256 x i8>
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %vecins, i8 1, <256 x i8> %4, i32 0, i1 true, i1 false)
  %add8 = add nsw i32 %d.041, 256
  %cmp = icmp slt i32 %add8, %mul2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i16> @llvm.tpc.convert.v256i16.v256i8.v256i1(<256 x i8>, i8, i32, <256 x i16>, <256 x i1>, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #2
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="goya2" "target-features"="+goya2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

