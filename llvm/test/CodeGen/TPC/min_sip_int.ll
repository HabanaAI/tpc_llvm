; RUN: llc -march=tpc -mcpu=dali %s -o - | FileCheck %s

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

@slmShort = external dso_local addrspace(1) global [20 x i32], section ".sldata", align 4
@slmByte = external dso_local addrspace(1) global [20 x i32], section ".sldata", align 4
@slm = external dso_local addrspace(1) global [20 x i32], section ".sldata", align 4

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul231 = sub i32 %vecext1, %vecext
  %sub = shl i32 %mul231, 6
  %cmp32 = icmp sgt i32 %sub, 0
  br i1 %cmp32, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %2 = shl i32 %vecext, 6
  %3 = shl i32 %vecext1, 6
  %4 = xor i32 %3, -1
  %5 = add i32 %2, %4
  %6 = icmp sgt i32 %5, -129
  %smax = select i1 %6, i32 %5, i32 -129
  %7 = add i32 %smax, %3
  %8 = add i32 %7, 128
  %9 = sub i32 %8, %2
  %10 = lshr i32 %9, 7
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %remLength.036 = phi i32 [ %sub8, %for.body ], [ %sub, %for.body.preheader ]
  %counterChar.035 = phi i8 [ %inc7, %for.body ], [ 0, %for.body.preheader ]
  %counterShort.034 = phi i16 [ %inc6, %for.body ], [ 64, %for.body.preheader ]
  %slmIdx.033 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %11 = icmp slt i32 %remLength.036, 128
  %12 = select i1 %11, i32 %remLength.036, i32 128
  %13 = icmp slt i16 %counterShort.034, 128
  %14 = select i1 %13, i16 %counterShort.034, i16 128
  %15 = icmp slt i8 %counterChar.035, 64
  %16 = select i1 %15, i8 %counterChar.035, i8 64
  %conv = sext i16 %14 to i32
  %arrayidx = getelementptr inbounds [20 x i32], [20 x i32] addrspace(1)* null, i32 0, i32 %slmIdx.033
  store volatile i32 %conv, i32 addrspace(1)* %arrayidx, align 4, !tbaa !4
  %conv3 = sext i8 %16 to i32
  %arrayidx4 = getelementptr inbounds [20 x i32], [20 x i32] addrspace(1)* inttoptr (i32 80 to [20 x i32] addrspace(1)*), i32 0, i32 %slmIdx.033
  store volatile i32 %conv3, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !4
  %arrayidx5 = getelementptr inbounds [20 x i32], [20 x i32] addrspace(1)* inttoptr (i32 160 to [20 x i32] addrspace(1)*), i32 0, i32 %slmIdx.033
  store volatile i32 %12, i32 addrspace(1)* %arrayidx5, align 4, !tbaa !4
  %inc = add nuw nsw i32 %slmIdx.033, 1
  %inc6 = add i16 %counterShort.034, 1
  %inc7 = add i8 %counterChar.035, 1
  %sub8 = add nsw i32 %remLength.036, -128
  %exitcond = icmp eq i32 %slmIdx.033, %10
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!2 = !{i32 240}
!3 = !{i32 256}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

; CHECK: min.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x80
; CHECK: min.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x40
; CHECK: min.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x80
; CHECK: convert.i16 target_type=int32 rhne %S{{[0-9]+}}, %S{{[0-9]+}}
; CHECK: convert.i8 target_type=int32 rhne %S{{[0-9]+}}, %S{{[0-9]+}}
