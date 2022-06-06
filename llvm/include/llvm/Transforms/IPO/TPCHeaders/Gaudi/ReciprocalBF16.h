const llvm::StringRef GaudiRecipBF16LL = R"(
; Function Attrs: alwaysinline nounwind
define  dso_local <128 x bfloat> @reciprocal_bf16(<128 x bfloat> %input) local_unnamed_addr #2 {
entry:
  %0 = tail call <128 x bfloat> @llvm.tpc.fclass.v128bf16.i1(<128 x bfloat> %input, i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %1 = bitcast <128 x bfloat> %input to <128 x i16>
  %2 = tail call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16> %1, i16 127, i8 8, i32 0, <128 x i16> undef, i1 true, i1 false)
  %3 = tail call <128 x bfloat> @llvm.tpc.lookup.1c.v128bf16.v128i16(<128 x i16> %2, i32 288, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1 false)
  %4 = tail call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x bfloat> %input, i8 1, i32 0, <128 x i16> undef, i1 true, i1 false)
  %5 = tail call <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x bfloat> %3, i8 1, i32 1, <128 x i16> undef, i1 true, i1 false)
  %6 = tail call <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16> %5, <128 x i16> %4, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  %7 = bitcast <128 x i16> %6 to <256 x i8>
  %8 = tail call <128 x bfloat> @llvm.tpc.form.fp.num.v128bf16.v256i8.i1(<256 x i8> %7, <128 x bfloat> %input, <128 x bfloat> %3, i8 1, i32 2048, <128 x bfloat> undef, i1 true, i1 false)
  %9 = tail call <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16> %1, i16 32767, i8 8, i32 0, <128 x i16> undef, i1 true, i1 false)
  %10 = tail call <128 x bfloat> @llvm.tpc.sel.grt.v128bf16.v128i16.i16.v128bf16.v128bf16.i1(<128 x i16> %9, i16 32384, <128 x bfloat> zeroinitializer, <128 x bfloat> %8, i8 8, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %11 = tail call <128 x bfloat> @llvm.tpc.calc.fp.special.v128bf16.i1(<128 x bfloat> %0, <128 x bfloat> undef, i8 1, i32 0, <128 x bfloat> %10, i1 true, i1 false)
  ret <128 x bfloat> %11
}

declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

declare <5 x i32> @llvm.tpc.get.index.space.size() #1

declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #1

declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #2

declare <128 x bfloat> @llvm.tpc.fclass.v128bf16.i1(<128 x bfloat>, i8, i32, <128 x bfloat>, i1, i1) #1

declare <128 x i16> @llvm.tpc.and.v128i16.v128i16.i16.i1(<128 x i16>, i16, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x bfloat> @llvm.tpc.lookup.1c.v128bf16.v128i16(<128 x i16>, i32, i32, <128 x bfloat>, i1, i1) #1

declare <128 x i16> @llvm.tpc.extract.exp.v128i16.v128bf16.i1(<128 x bfloat>, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, i8, i32, <128 x i16>, i1, i1) #1

declare <128 x bfloat> @llvm.tpc.form.fp.num.v128bf16.v256i8.i1(<256 x i8>, <128 x bfloat>, <128 x bfloat>, i8, i32, <128 x bfloat>, i1, i1) #1

declare <128 x bfloat> @llvm.tpc.sel.grt.v128bf16.v128i16.i16.v128bf16.v128bf16.i1(<128 x i16>, i16, <128 x bfloat>, <128 x bfloat>, i8, i32, <128 x bfloat>, i1, i1) #1

declare <128 x bfloat> @llvm.tpc.calc.fp.special.v128bf16.i1(<128 x bfloat>, <128 x bfloat>, i8, i32, <128 x bfloat>, i1, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind writeonly }
)";
