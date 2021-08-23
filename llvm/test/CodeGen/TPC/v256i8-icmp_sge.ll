; RUN: llc -O0 -march=tpc -mcpu=dali  %s -o - | FileCheck %s
; CHECK: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK: add.i8 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
; CHECK: cmp_geq.i8 %VP{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
; CHECK: st_tnsr 0x{{[0-9]+}}, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

target triple = "tpc"


define dso_local void @main() local_unnamed_addr #0 {
entry:
  %cord = load <5 x i32>, <5 x i32> addrspace(1)* null, align 4
  %ld = call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %cord, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %add = call <256 x i8> @llvm.tpc.add.v256i8.v256i8.i8.i1(<256 x i8> %ld, i8 2, i8 4, i32 0, <256 x i8> undef, i1 true, i1 false)
  %cmp = icmp sge <256 x i8> %ld , %add
  %mov = tail call <256 x i8> @llvm.tpc.mov.v256i8.v256i8.v256i1(<256 x i8> %ld, i8 2, i32 0, <256 x i8> %add, <256 x i1> %cmp, i1 true)
  call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %cord, i8 2, <256 x i8> %mov, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

declare <256 x i8> @llvm.tpc.add.v256i8.v256i8.i8.i1(<256 x i8>, i8, i8, i32, <256 x i8>, i1, i1) #1

declare <256 x i8> @llvm.tpc.mov.v256i8.v256i8.v256i1(<256 x i8>, i8, i32, <256 x i8>, <256 x i1>, i1) #1


attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="dali" "target-features"="+dali" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!""}
!2 = !{i32 28}
!3 = !{i32 256}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
