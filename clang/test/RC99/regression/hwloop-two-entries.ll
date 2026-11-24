; RUN: %tpc_clang -march=gaudi2 -O2 -S -x ir %s -o - | FileCheck %s

; A hardware loop entered from two blocks (jump threading copied the block
; before it) gets its own pre-header with the LOOP, which both enter. The LOOP
; was put in one of them and the other jumped into the body; the pass then
; asserted, as the loop had no single predecessor.


define void @main() #1 {
entry:
  br label %for.body5

for.body5:                                        ; preds = %for.cond.cleanup62.loopexit, %entry
  %owner.0193 = phi i32 [ 0, %entry ], [ %inc83, %for.cond.cleanup62.loopexit ]
  %cmp56 = icmp eq i32 %owner.0193, 0
  br i1 %cmp56, label %if.end58.thread, label %for.body63

if.end58.thread:                                  ; preds = %for.body5
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 0, <64 x float> zeroinitializer, i32 0, i1 true, i1 false)
  br label %for.body63

for.cond.cleanup62.loopexit:                      ; preds = %for.body63
  %inc83 = add i32 %owner.0193, 1
  br label %for.body5

for.body63:                                       ; preds = %for.body63, %if.end58.thread, %for.body5
  %tile59.0191 = phi i32 [ %add80, %for.body63 ], [ 0, %if.end58.thread ], [ %owner.0193, %for.body5 ]
  tail call void @llvm.tpc.st.tnsr.partial.v256f8_143(<5 x i32> zeroinitializer, i8 0, <256 x f8_143> zeroinitializer, i32 0, i32 0, i1 true, i1 false)
  %add80 = add i32 %tile59.0191, 1
  %cmp61 = icmp slt i32 %add80, 40
  br i1 %cmp61, label %for.body63, label %for.cond.cleanup62.loopexit
}

; Function Attrs: nounwind memory(write)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #0

; Function Attrs: nounwind memory(write)
declare void @llvm.tpc.st.tnsr.partial.v256f8_143(<5 x i32>, i8, <256 x f8_143>, i32, i32, i1, i1) #0

attributes #0 = { nounwind memory(write) }
attributes #1 = { "target-cpu"="gaudi2" "tpc-kernel"="true" }

; CHECK:      jmpr [[PRE:.LBB0_[0-9]+]], %SP{{[0-9]+}}
; CHECK:      st_tnsr
; CHECK:      [[PRE]]:
; CHECK-NOT:  {{^[.$]}}
; CHECK:      loop
