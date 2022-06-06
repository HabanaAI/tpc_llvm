// signless operation (same as u32 to u8)
const llvm::StringRef GaudiTruncI32I8LL = R"(
; Function Attrs: alwaysinline nounwind
define  dso_local <256 x i8> @trunc_i32_to_i8(<64 x i32> %x0, <64 x i32> %x1, <64 x i32> %x2, <64 x i32> %x3) local_unnamed_addr #2 {
entry:
  %0 = bitcast <64 x i32> %x0 to <256 x i8>
  %1 = bitcast <64 x i32> %x1 to <256 x i8>
  %2 = bitcast <64 x i32> %x2 to <256 x i8>
  %3 = bitcast <64 x i32> %x3 to <256 x i8>
  %4 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %0, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #4
  %5 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %0, i8 4, i32 768, <256 x i8> %4, i1 true, i1 false) #4
  %6 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %5, i32 -65536, i32 0, i32 4352, <256 x i8> %5, i1 true, i1 false) #4
  %7 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %6, i32 65535, i32 0, i32 8704, <256 x i8> %6, i1 true, i1 false) #4
  %8 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %7, i32 -65536, i32 0, i32 8960, <256 x i8> %7, i1 true, i1 false) #4
  %9 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %1, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #4
  %10 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %1, i8 4, i32 768, <256 x i8> %9, i1 true, i1 false) #4
  %11 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %10, i32 65535, i32 0, i32 5120, <256 x i8> %8, i1 true, i1 false) #4
  %12 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %10, i32 -65536, i32 0, i32 5376, <256 x i8> %11, i1 true, i1 false) #4
  %13 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %10, i32 65535, i32 0, i32 9728, <256 x i8> %12, i1 true, i1 false) #4
  %14 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %10, i32 -65536, i32 0, i32 9984, <256 x i8> %13, i1 true, i1 false) #4
  %15 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %2, i8 4, i32 512, <256 x i8> %10, i1 true, i1 false) #4
  %16 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %2, i8 4, i32 768, <256 x i8> %15, i1 true, i1 false) #4
  %17 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %16, i32 65535, i32 0, i32 6144, <256 x i8> %14, i1 true, i1 false) #4
  %18 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %16, i32 -65536, i32 0, i32 6400, <256 x i8> %17, i1 true, i1 false) #4
  %19 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %16, i32 65535, i32 0, i32 10752, <256 x i8> %18, i1 true, i1 false) #4
  %20 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %16, i32 -65536, i32 0, i32 11008, <256 x i8> %19, i1 true, i1 false) #4
  %21 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %3, i8 4, i32 512, <256 x i8> %16, i1 true, i1 false) #4
  %22 = call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %3, i8 4, i32 768, <256 x i8> %21, i1 true, i1 false) #4
  %23 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %22, i32 65535, i32 0, i32 7168, <256 x i8> %20, i1 true, i1 false) #4
  %24 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %22, i32 -65536, i32 0, i32 7424, <256 x i8> %23, i1 true, i1 false) #4
  %25 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %22, i32 65535, i32 0, i32 11776, <256 x i8> %24, i1 true, i1 false) #4
  %26 = call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %22, i32 -65536, i32 0, i32 12032, <256 x i8> %25, i1 true, i1 false) #4
  ret <256 x i8> %26
}
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

declare <5 x i32> @llvm.tpc.get.index.space.size() #1

declare <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32>, <5 x i32>, i32, i8, i32, <5 x i32>, i1, i1) #1

declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #1

declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

declare <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8>, i8, i32, <256 x i8>, i1, i1) #1

declare <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8>, i32, i32, i32, <256 x i8>, i1, i1) #1

declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind writeonly }
)";
