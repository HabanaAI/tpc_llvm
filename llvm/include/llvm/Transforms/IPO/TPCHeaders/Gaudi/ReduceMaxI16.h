const llvm::StringRef GaudiReduceMaxI16LL = R"(
; Function Attrs: alwaysinline nounwind
define  dso_local <128 x i16> @v_i16_reduce_max(<128 x i16> %0) local_unnamed_addr #2 {
  %2 = call <64 x i32> @llvm.read_register.v64i32(metadata !0)
  %3 = call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %2, i32 140, i32 0, <128 x float> undef, i1 true, i1 false)
  %4 = shufflevector <128 x float> %3, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %5 = shufflevector <128 x float> %3, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %6 = add <64 x i32> %2, <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>
  %7 = call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %6, i32 140, i32 0, <128 x float> undef, i1 true, i1 false)
  %8 = shufflevector <128 x float> %7, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %9 = shufflevector <128 x float> %7, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %10 = bitcast <64 x float> %4 to <256 x i8>
  %11 = bitcast <64 x float> %5 to <256 x i8>
  %12 = bitcast <64 x float> %8 to <256 x i8>
  %13 = bitcast <64 x float> %9 to <256 x i8>
  %14 = call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %0, i32 -1, i32 0, i32 16756992, <128 x i16> zeroinitializer, i1 true, i1 false)
  %15 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %0, <128 x i16> %14, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %16 = call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %15, i32 -1, i32 0, i32 16731648, <128 x i16> %14, i1 true, i1 false)
  %17 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %15, <128 x i16> %16, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %18 = call <128 x i16> @llvm.tpc.mov.group.v128i16.v128i16.i1(<128 x i16> %17, i32 -1, i32 63, <128 x i16> %16, i1 true, i1 false)
  %19 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %17, <128 x i16> %18, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %20 = call <128 x i16> @llvm.tpc.shuffle.v128i16.i1(<128 x i16> %19, <256 x i8> %10, i8 7, i32 0, <128 x i16> %18, i1 true, i1 false)
  %21 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %19, <128 x i16> %20, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %22 = call <128 x i16> @llvm.tpc.shuffle.v128i16.i1(<128 x i16> %21, <256 x i8> %11, i8 7, i32 0, <128 x i16> %20, i1 true, i1 false)
  %23 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %21, <128 x i16> %22, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %24 = call <128 x i16> @llvm.tpc.shuffle.v128i16.i1(<128 x i16> %23, <256 x i8> %12, i8 7, i32 0, <128 x i16> %22, i1 true, i1 false)
  %25 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %23, <128 x i16> %24, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  %26 = call <128 x i16> @llvm.tpc.shuffle.v128i16.i1(<128 x i16> %25, <256 x i8> %13, i8 7, i32 0, <128 x i16> %24, i1 true, i1 false)
  %27 = call <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16> %25, <128 x i16> %26, i8 7, i32 0, <128 x i16> undef, i1 true, i1 false)
  ret <128 x i16> %27
}

declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #2

declare <64 x i32> @llvm.read_register.v64i32(metadata) #3

declare <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32>, i32, i32, <128 x float>, i1, i1) #1

declare <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16>, i32, i32, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.max.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.mov.group.v128i16.v128i16.i1(<128 x i16>, i32, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.shuffle.v128i16.i1(<128 x i16>, <256 x i8>, i8, i32, <128 x i16>, i1, i1) #1

!llvm.named.register.v_lane_id_32 = !{!0}

!0 = !{!"v_lane_id_32"}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind readonly }
)";
