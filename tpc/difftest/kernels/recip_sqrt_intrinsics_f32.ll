; llvm.sqrt, llvm.tpc.rsqrt and llvm.tpc.reciprocal on <64 x float>, which C
; kernels cannot call: the compilers link their own bodies for them (the fork's
; LinkTPCHeaders). Rows as in div_range_f32.c: row r < 256 gives the input the
; exponent field r with its random sign and mantissa, then rows with a zero
; mantissa, an all-ones mantissa, +-0 and tiny denormals. The IR parses with
; Habana's LLVM 14 too. LinkTPCHeaders takes the arch from target-cpu.
;
; DIFFTEST: arch gaudi2
; DIFFTEST: in  f32 1024 260
; DIFFTEST: out f32 1024 260
; DIFFTEST: out f32 1024 260
; DIFFTEST: out f32 1024 260
; DIFFTEST: index-space 16 260

target triple = "tpc"

define void @main() #0 {
entry:
  %off = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %size = call <5 x i32> @llvm.tpc.get.index.space.size()
  %x.off = extractelement <5 x i32> %off, i32 0
  %x.size = extractelement <5 x i32> %size, i32 0
  %y.start = extractelement <5 x i32> %off, i32 1
  %y.size = extractelement <5 x i32> %size, i32 1
  %x.start = shl i32 %x.off, 6
  %x.last = add i32 %x.off, %x.size
  %x.end = shl i32 %x.last, 6
  %y.end = add i32 %y.start, %y.size
  %any.y = icmp slt i32 %y.start, %y.end
  br i1 %any.y, label %row, label %exit

row:
  %y = phi i32 [ %y.start, %entry ], [ %y.next, %row.end ]
  %exp = shl i32 %y, 23
  %r256 = icmp eq i32 %y, 256
  %r257 = icmp eq i32 %y, 257
  %r258 = icmp eq i32 %y, 258
  %r259 = icmp eq i32 %y, 259
  %keep.a = select i1 %r256, i32 -8388608, i32 -2139095041
  %keep.b = select i1 %r257, i32 -1, i32 %keep.a
  %keep.c = select i1 %r258, i32 -2147483648, i32 %keep.b
  %keep.s = select i1 %r259, i32 -2147482625, i32 %keep.c
  %high = icmp sgt i32 %y, 255
  %set.a = select i1 %high, i32 0, i32 %exp
  %set.s = select i1 %r257, i32 8388607, i32 %set.a
  %keep.0 = insertelement <64 x i32> undef, i32 %keep.s, i32 0
  %keep = shufflevector <64 x i32> %keep.0, <64 x i32> undef, <64 x i32> zeroinitializer
  %set.0 = insertelement <64 x i32> undef, i32 %set.s, i32 0
  %set = shufflevector <64 x i32> %set.0, <64 x i32> undef, <64 x i32> zeroinitializer
  %any.x = icmp slt i32 %x.start, %x.end
  br i1 %any.x, label %col, label %row.end

col:
  %x = phi i32 [ %x.start, %row ], [ %x.next, %col ]
  %c.0 = insertelement <5 x i32> zeroinitializer, i32 %x, i32 0
  %c = insertelement <5 x i32> %c.0, i32 %y, i32 1
  %in = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %c, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %bits = bitcast <64 x float> %in to <64 x i32>
  %kept = and <64 x i32> %bits, %keep
  %vbits = or <64 x i32> %kept, %set
  %v = bitcast <64 x i32> %vbits to <64 x float>
  %sqrt = call <64 x float> @llvm.sqrt.v64f32(<64 x float> %v)
  %rsqrt = call <64 x float> @llvm.tpc.rsqrt.v64f32(<64 x float> %v)
  %recip = call <64 x float> @llvm.tpc.reciprocal.v64f32(<64 x float> %v)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 1, <64 x float> %sqrt, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 2, <64 x float> %rsqrt, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %c, i8 3, <64 x float> %recip, i32 0, i1 true, i1 false)
  %x.next = add i32 %x, 64
  %more.x = icmp slt i32 %x.next, %x.end
  br i1 %more.x, label %col, label %row.end

row.end:
  %y.next = add i32 %y, 1
  %more.y = icmp slt i32 %y.next, %y.end
  br i1 %more.y, label %row, label %exit

exit:
  ret void
}

declare <5 x i32> @llvm.tpc.get.index.space.offset()
declare <5 x i32> @llvm.tpc.get.index.space.size()
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)
declare <64 x float> @llvm.sqrt.v64f32(<64 x float>)
declare <64 x float> @llvm.tpc.rsqrt.v64f32(<64 x float>)
declare <64 x float> @llvm.tpc.reciprocal.v64f32(<64 x float>)

attributes #0 = { nounwind "target-cpu"="gaudi2" "tpc-kernel"="true" }
