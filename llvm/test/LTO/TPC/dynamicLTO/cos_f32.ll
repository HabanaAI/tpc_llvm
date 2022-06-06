; RUN: clang -c --target=tpc -march=gaudi -mllvm -ILTO=%S/libTpcDynamicLTO.so -c -O2 %s -S -o - | FileCheck %s 
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float>  @tpc_cos_f32(<64 x float> %0)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %1, i32 0, i1 true, i1 false)
  ret void
  ; CHECK: {{.*}}get_lut_entry_and_interval_start.f32 sin_cos{{.*}}
  ; CHECK: lookup_1c{{.*}}
  ; CHECK: lookup_2c{{.*}}
}

declare <64 x float>  @tpc_cos_f32(<64 x float> )

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 8659a4597e30349b06c5ae5c82e0105b15a338a0)"}
!2 = !{i32 0}
