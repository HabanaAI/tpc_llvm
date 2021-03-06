; NOTE: Assertions have been autogenerated by utils/update_analyze_test_checks.py
 ; RUN: opt < %s -cost-model -cost-kind=latency -analyze -mtriple=tpc -mcpu=dali | FileCheck %s --check-prefix=LATENCY
;XFAIL:*
target triple = "tpc"

%struct._float64_quad_t = type { <64 x float>, <64 x float>, <64 x float>, <64 x float> }

; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
; LATENCY-LABEL: 'main'
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 4 for instruction: %vecins = call <5 x i32> @llvm.tpc.get.index.space.offset()
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 0 for instruction: %fp_source = bitcast %struct._float64_quad_t addrspace(2)* null to <256 x float> addrspace(2)*
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 4 for instruction: %0 = load <256 x float>, <256 x float> addrspace(2)* %fp_source, align 256
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 128 for instruction: %1 = fptosi <256 x float> %0 to <256 x i8>
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 2 for instruction: tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %vecins, i8 1, <256 x i8> %1, i32 0, i1 true, i1 false)
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret void
;
entry:
  %vecins = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %fp_source = bitcast %struct._float64_quad_t addrspace(2)* inttoptr (i32 0 to %struct._float64_quad_t addrspace(2)*) to <256 x float> addrspace(2)*
  %0 = load <256 x float>, <256 x float> addrspace(2)* %fp_source, align 256

  %1 = fptosi <256 x float> %0 to <256 x i8>

  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %vecins, i8 1, <256 x i8> %1, i32 0, i1 true, i1 false)

  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

