const llvm::StringRef GaudiReduceAddBF16LL = R"(
; Function Attrs: alwaysinline nounwind
define  dso_local <128 x bfloat16> @v_bf16_reduce_add(<128 x bfloat16> %x) local_unnamed_addr #5 {
entry:
  %0 = call <64 x i32> @llvm.read_register.v64i32(metadata !0)
  %1 = call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %0, i32 140, i32 0, <128 x float> undef, i1 true, i1 false)
  %2 = shufflevector <128 x float> %1, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %3 = shufflevector <128 x float> %1, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %add = add <64 x i32> %0, <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>
  %4 = call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %add, i32 140, i32 0, <128 x float> undef, i1 true, i1 false)
  %5 = shufflevector <128 x float> %4, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %6 = shufflevector <128 x float> %4, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %7 = bitcast <64 x float> %2 to <256 x i8>
  %8 = bitcast <64 x float> %3 to <256 x i8>
  %9 = bitcast <64 x float> %5 to <256 x i8>
  %10 = bitcast <64 x float> %6 to <256 x i8>
  %11 = call <128 x bfloat16> @llvm.tpc.mov.dual.group.all.v128bf16.i1(<128 x bfloat16> %x, i32 -1, i32 0, i32 16756992, <128 x bfloat16> zeroinitializer, i1 true, i1 false)
  %12 = fadd <128 x bfloat16> %11, %x
  %13 = call <128 x bfloat16> @llvm.tpc.mov.dual.group.all.v128bf16.i1(<128 x bfloat16> %12, i32 -1, i32 0, i32 16731648, <128 x bfloat16> %11, i1 true, i1 false)
  %14 = fadd <128 x bfloat16> %12, %13
  %15 = call <128 x bfloat16> @llvm.tpc.mov.group.v128bf16.v128bf16.i1(<128 x bfloat16> %14, i32 -1, i32 63, <128 x bfloat16> %13, i1 true, i1 false)
  %16 = fadd <128 x bfloat16> %14, %15
  %17 = call <128 x bfloat16> @llvm.tpc.shuffle.v128bf16.i1(<128 x bfloat16> %16, <256 x i8> %7, i8 1, i32 0, <128 x bfloat16> %15, i1 true, i1 false)
  %18 = fadd <128 x bfloat16> %16, %17
  %19 = call <128 x bfloat16> @llvm.tpc.shuffle.v128bf16.i1(<128 x bfloat16> %18, <256 x i8> %8, i8 1, i32 0, <128 x bfloat16> %17, i1 true, i1 false)
  %20 = fadd <128 x bfloat16> %18, %19
  %21 = call <128 x bfloat16> @llvm.tpc.shuffle.v128bf16.i1(<128 x bfloat16> %20, <256 x i8> %9, i8 1, i32 0, <128 x bfloat16> %19, i1 true, i1 false)
  %22 = fadd <128 x bfloat16> %20, %21
  %23 = call <128 x bfloat16> @llvm.tpc.shuffle.v128bf16.i1(<128 x bfloat16> %22, <256 x i8> %10, i8 1, i32 0, <128 x bfloat16> %21, i1 true, i1 false)
  %24 = fadd <128 x bfloat16> %22, %23
  ret <128 x bfloat16> %24
}

declare bfloat16 @llvm.tpc.convert.bf16.f32.i1(float, i8, i32, bfloat16, i1, i1) #1

declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

declare <5 x i32> @llvm.tpc.get.index.space.size() #1

declare <5 x i32> @llvm.tpc.set.indx(<5 x i32>, i32, i32, i32, i1, i1) #1

declare <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32>, <5 x i32>, i32, i8, i32, <5 x i32>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.mac.v128bf16.v128bf16.i1(<128 x bfloat16>, <128 x bfloat16>, i8, i32, <128 x bfloat16>, i1, i1) #1

declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat16>, i32, i1, i1) #2

declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #3

declare <64 x i32> @llvm.read_register.v64i32(metadata) #3

declare <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32>, i32, i32, <128 x float>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.mov.dual.group.all.v128bf16.i1(<128 x bfloat16>, i32, i32, i32, <128 x bfloat16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.mov.group.v128bf16.v128bf16.i1(<128 x bfloat16>, i32, i32, <128 x bfloat16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.shuffle.v128bf16.i1(<128 x bfloat16>, <256 x i8>, i8, i32, <128 x bfloat16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x bfloat16>, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16>, i16, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.shr.v128i16.i16.i1(<128 x i16>, i16, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.form.fp.num.v128bf16.v128bf16.i1(<128 x bfloat16>, <128 x bfloat16>, <128 x bfloat16>, i8, i32, <128 x bfloat16>, i1, i1) #1

declare <256 x i16> @llvm.tpc.get.lut.entry.v256i16.v128bf16.i1(<128 x bfloat16>, i8, i8, i32, <256 x i16>, i1, i1) #1

declare <256 x bfloat16> @llvm.tpc.lookup.2c.v256bf16.v128i16(<128 x i16>, i32, i32, <256 x bfloat16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.fclass.v128bf16.i1(<128 x bfloat16>, i8, i32, <128 x bfloat16>, i1, i1) #1

declare <128 x bfloat16> @llvm.tpc.calc.fp.special.v128bf16.i1(<128 x bfloat16>, <128 x bfloat16>, i8, i32, <128 x bfloat16>, i1, i1) #1

!llvm.named.register.v_lane_id_32 = !{!0}

!0 = !{!"v_lane_id_32"}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind writeonly }
attributes #5 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind }
attributes #7 = { nounwind readonly }
attributes #8 = { nounwind   }
)";
