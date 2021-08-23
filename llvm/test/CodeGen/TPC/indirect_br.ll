; RUN: llc -O0 %s -o - | FileCheck %s
; CHECK-LABEL: tmp0:
; COM: tmp0:{{.*}}Block address taken
; CHECK: %if.then
; CHECK: mov.i32  %S{{[0-9]+}},{{.*}}tmp0{{.*}}, %SP{{[0-9]+}}
; CHECK: jmpr %S{{[0-9]+}}, %SP{{[0-9]+}}

target triple = "tpc"

@ifmCoords = external dso_local addrspace(1) global <5 x i32>, section ".sldata"
@depthEnd = external dso_local addrspace(1) global i32, section ".sldata"
@ain1 = external dso_local addrspace(2) global <64 x i32>, section ".vldata"
@d = external dso_local addrspace(1) global i32, section ".sldata"
@inputVal = external dso_local addrspace(1) global i32, section ".sldata"

; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
entry:
  %ld1 = load <5 x i32>, <5 x i32> addrspace(1)* null, align 4
  %ld2 = load i32, i32 addrspace(1)* inttoptr (i32 24 to i32 addrspace(1)*), align 4
  %vecins = insertelement <5 x i32> %ld1, i32 %ld2, i32 2
  %ga = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %vecins, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %lds = call i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)* %ga, i32 0, i32 undef, i1 true, i1 false)
  %ld3 = load i32, i32 addrspace(1)* inttoptr (i32 28 to i32 addrspace(1)*), align 4
  %cmp3 = icmp slt i32 %lds, %ld3
  br i1 %cmp3, label %if.then, label %if.end
if.then:
  %ld4 = load <5 x i32>, <5 x i32> addrspace(1)* null, align 4
  %ld5 = load <64 x i32>, <64 x i32> addrspace(2)* inttoptr (i32 256 to <64 x i32> addrspace(2)*), align 256
  call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %ld4, i8 2, <64 x i32> %ld5, i32 0, i1 true, i1 false)
  ;calculate block address
  ;%bla = add i8* blockaddress(@main, %if.end) 
  ;br label %if.end
  indirectbr i8* blockaddress(@main, %if.then), [ label %if.end , label %if.then ]
if.end:
  ret void

}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)*, i32, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #3

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="dali" "target-features"="+dali" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://pchandrasekaran@gerrit.habana-labs.com:29418/tpc_clang2 465f805ddfdef932c67d41f432c00e374d4c5cc7) (ssh://pchandrasekaran@gerrit.habana-labs.com:29418/tpc_llvm2 4f9c05bd2322160c42a0b0b079905fbf86e3cf81)"}
!2 = !{i32 32}
!3 = !{i32 512}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
