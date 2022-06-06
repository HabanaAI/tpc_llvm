const llvm::StringRef Gaudi2RSqrtF16LL = R"(
; Function Attrs: alwaysinline nounwind
define  dso_local <128 x half> @rsqrt_f16(<128 x half> %input) local_unnamed_addr #2 {
entry:
  %0 = call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128f16.i1(<128 x half> %input, i8 11, i32 0, <128 x i16> undef, i1 true, i1 false)
  %1 = call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16> %0, i16 1, i8 8, i32 0, <128 x i16> undef, i1 true, i1 false)
  %2 = bitcast <128 x i16> %1 to <256 x i8>
  %3 = call <128 x half> @llvm.tpc.form.fp.num.v128f16.v256i8.i1(<256 x i8> %2, <128 x half> %input, <128 x half> %input, i8 11, i32 2816, <128 x half> undef, i1 true, i1 false)
  %4 = call <256 x i16> @llvm.tpc.get.lut.entry.v256i16.v128f16.i1(<128 x half> %3, i8 7, i8 11, i32 16384, <256 x i16> undef, i1 true, i1 false)
  %5 = shufflevector <256 x i16> %4, <256 x i16> undef, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %6 = shufflevector <256 x i16> %4, <256 x i16> undef, <128 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  %7 = bitcast <128 x i16> %6 to <128 x half>
  %8 = call <128 x half> @llvm.tpc.sub.v128f16.v128f16.v128f16.i1(<128 x half> %3, <128 x half> %7, i8 11, i32 0, <128 x half> undef, i1 true, i1 false)
  %9 = call <128 x half> @llvm.tpc.lookup.1c.v128f16.v128i16(<128 x i16> %5, i32 441, i32 17, <128 x half> zeroinitializer, i1 true, i1 false)
  %10 = call <256 x half> @llvm.tpc.lookup.2c.v256f16.v128i16(<128 x i16> %5, i32 440, i32 17, <256 x half> zeroinitializer, i1 true, i1 false)
  %C1C2.sroa.0.256.vec.extract = shufflevector <256 x half> %10, <256 x half> undef, <128 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  %C1C2.sroa.0.0.vec.extract = shufflevector <256 x half> %10, <256 x half> undef, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %11 = call <128 x half> @llvm.tpc.mac.v128f16.v128f16.i1(<128 x half> %C1C2.sroa.0.256.vec.extract, <128 x half> %8, i8 11, i32 0, <128 x half> %C1C2.sroa.0.0.vec.extract, i1 true, i1 false)
  %12 = call <128 x half> @llvm.tpc.mac.v128f16.v128f16.i1(<128 x half> %11, <128 x half> %8, i8 11, i32 0, <128 x half> %9, i1 true, i1 false)
  %13 = lshr <128 x i16> %0, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %shl = shl <128 x i16> %13, <i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10>
  %14 = bitcast <128 x half> %12 to <128 x i16>
  %sub = sub <128 x i16> %14, %shl
  %15 = bitcast <128 x i16> %sub to <128 x half>
  %16 = call <128 x half> @llvm.tpc.fclass.v128f16.i1(<128 x half> %input, i8 11, i32 0, <128 x half> undef, i1 true, i1 false)
  %17 = call <128 x half> @llvm.tpc.calc.fp.special.v128f16.i1(<128 x half> %16, <128 x half> undef, i8 11, i32 1, <128 x half> %15, i1 true, i1 false)
  ret <128 x half> %17
}

declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

declare <5 x i32> @llvm.tpc.get.index.space.size() #1

declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #1

declare void @llvm.tpc.st.tnsr.v128f16(<5 x i32>, i8, <128 x half>, i32, i1, i1) #2

declare <128 x i16> @llvm.tpc.extract.exp.v128i16.v128f16.i1(<128 x half>, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16>, i16, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x half> @llvm.tpc.form.fp.num.v128f16.v256i8.i1(<256 x i8>, <128 x half>, <128 x half>, i8, i32, <128 x half>, i1, i1) #1

declare <256 x i16> @llvm.tpc.get.lut.entry.v256i16.v128f16.i1(<128 x half>, i8, i8, i32, <256 x i16>, i1, i1) #1

declare <128 x half> @llvm.tpc.sub.v128f16.v128f16.v128f16.i1(<128 x half>, <128 x half>, i8, i32, <128 x half>, i1, i1) #1

declare <128 x half> @llvm.tpc.lookup.1c.v128f16.v128i16(<128 x i16>, i32, i32, <128 x half>, i1, i1) #1

declare <256 x half> @llvm.tpc.lookup.2c.v256f16.v128i16(<128 x i16>, i32, i32, <256 x half>, i1, i1) #1

declare <128 x half> @llvm.tpc.mac.v128f16.v128f16.i1(<128 x half>, <128 x half>, i8, i32, <128 x half>, i1, i1) #1

declare <128 x half> @llvm.tpc.fclass.v128f16.i1(<128 x half>, i8, i32, <128 x half>, i1, i1) #1

declare <128 x half> @llvm.tpc.calc.fp.special.v128f16.i1(<128 x half>, <128 x half>, i8, i32, <128 x half>, i1, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind readnone   willreturn }

)";