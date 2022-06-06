; RUN: llc -march=tpc -mcpu=gaudi  -O2 %s  -o -

; ModuleID = 'tpc_kernel'
 source_filename = "fused_kernel"
 target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
 target triple = "tpc"
 
 ; Function Attrs: nounwind writeonly
 declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #0
 
 ; Function Attrs: nounwind readnone
 declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #1
  
 define void @main() {
entry:
  %ld1 =  call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %ld2 =  call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> zeroinitializer, i8 1, i32 0, <64 x i32> undef, i1 true, i1 false)
  %cmp = icmp eq <64 x i32> %ld1, %ld2
  %zext = zext <64 x i1> %cmp to <64 x i32>
  %out = sitofp <64 x i32> %zext to <64 x float>
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 2, <64 x float> %out, i32 0, i1 true, i1 false)
  ret void
 }
 
 attributes #0 = { nounwind writeonly }
 attributes #1 = { nounwind readnone }


; CHECK: sel_eq.i32  %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x0     
