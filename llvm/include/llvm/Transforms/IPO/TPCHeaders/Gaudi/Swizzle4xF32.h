#ifdef _WIN32
const llvm::StringRef GaudiSwizzleX4F32LL = "";
#else
const llvm::StringRef GaudiSwizzleX4F32LL = R"(

; Function Attrs: alwaysinline nounwind
define dso_local <256 x float> @swizzle_4xf32(<256 x float> %in) local_unnamed_addr #5 {
entry:
  %0 = tail call <64 x i32> @llvm.read_register.v64i32(metadata !0)
  %1 = tail call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %0, i32 289, i32 0, <128 x float> undef, i1 true, i1 false)
  %2 = shufflevector <128 x float> %1, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %3 = shufflevector <128 x float> %1, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %4 = bitcast <64 x float> %2 to <256 x i8>
  %5 = bitcast <64 x float> %3 to <256 x i8>



  ; %6 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %6 = shufflevector <256 x float> %in, <256 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>

  %7 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> zeroinitializer, i32 64, i32 1, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)

  ; %8 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %7, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %8 = shufflevector <256 x float> %in, <256 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>

  %9 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %7, i32 64, i32 1, i8 2, i32 0, <5 x i32> %7, i1 true, i1 false)

  ; %10 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %9, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %10 = shufflevector <256 x float> %in, <256 x float> undef, <64 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191>

  %11 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %9, i32 64, i32 1, i8 2, i32 0, <5 x i32> %9, i1 true, i1 false)
  ; %12 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %11, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %12 = shufflevector <256 x float> %in, <256 x float> undef, <64 x i32> <i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>


  %13 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %6, <256 x i8> %4, i8 0, i32 0, <64 x float> %6, i1 true, i1 false) #4
  %14 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %8, <256 x i8> %4, i8 0, i32 0, <64 x float> %8, i1 true, i1 false) #4
  %15 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %13, i32 -1, i32 63, <64 x float> %13, i1 true, i1 false) #4
  %16 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %14, i32 -1, i32 63, <64 x float> %14, i1 true, i1 false) #4
  %17 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %15, <256 x i8> %5, i8 0, i32 0, <64 x float> %13, i1 true, i1 false) #4
  %18 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %16, <256 x i8> %5, i8 0, i32 0, <64 x float> %14, i1 true, i1 false) #4
  %19 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %17, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %20 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %17, i32 -1, i32 21, <64 x float> %19, i1 true, i1 false) #4
  %21 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %17, i32 -1, i32 22, <64 x float> %19, i1 true, i1 false) #4
  %22 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %21, i32 -1, i32 23, <64 x float> %21, i1 true, i1 false) #4
  %23 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %18, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %24 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %18, i32 -1, i32 21, <64 x float> %23, i1 true, i1 false) #4
  %25 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %18, i32 -1, i32 22, <64 x float> %23, i1 true, i1 false) #4
  %26 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %25, i32 -1, i32 23, <64 x float> %25, i1 true, i1 false) #4
  %27 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %20, i32 -1, i32 0, i32 13824, <64 x float> %20, i1 true, i1 false) #4
  %28 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %24, i32 -1, i32 0, i32 15761408, <64 x float> %27, i1 true, i1 false) #4
  %29 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %22, i32 -1, i32 0, i32 13824, <64 x float> %22, i1 true, i1 false) #4
  %30 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %26, i32 -1, i32 0, i32 15761408, <64 x float> %29, i1 true, i1 false) #4
  %31 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %10, <256 x i8> %4, i8 0, i32 0, <64 x float> %10, i1 true, i1 false) #4
  %32 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %12, <256 x i8> %4, i8 0, i32 0, <64 x float> %12, i1 true, i1 false) #4
  %33 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %31, i32 -1, i32 63, <64 x float> %31, i1 true, i1 false) #4
  %34 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %32, i32 -1, i32 63, <64 x float> %32, i1 true, i1 false) #4
  %35 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %33, <256 x i8> %5, i8 0, i32 0, <64 x float> %31, i1 true, i1 false) #4
  %36 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %34, <256 x i8> %5, i8 0, i32 0, <64 x float> %32, i1 true, i1 false) #4
  %37 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %35, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %38 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %35, i32 -1, i32 21, <64 x float> %37, i1 true, i1 false) #4
  %39 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %35, i32 -1, i32 22, <64 x float> %37, i1 true, i1 false) #4
  %40 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %39, i32 -1, i32 23, <64 x float> %39, i1 true, i1 false) #4
  %41 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %36, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %42 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %36, i32 -1, i32 21, <64 x float> %41, i1 true, i1 false) #4
  %43 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %36, i32 -1, i32 22, <64 x float> %41, i1 true, i1 false) #4
  %44 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %43, i32 -1, i32 23, <64 x float> %43, i1 true, i1 false) #4
  %45 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %38, i32 -1, i32 0, i32 13824, <64 x float> %38, i1 true, i1 false) #4
  %46 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %42, i32 -1, i32 0, i32 15761408, <64 x float> %45, i1 true, i1 false) #4
  %47 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %40, i32 -1, i32 0, i32 13824, <64 x float> %40, i1 true, i1 false) #4
  %48 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %44, i32 -1, i32 0, i32 15761408, <64 x float> %47, i1 true, i1 false) #4
  %49 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %30, <256 x i8> %4, i8 0, i32 0, <64 x float> %30, i1 true, i1 false) #4
  %50 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %48, <256 x i8> %4, i8 0, i32 0, <64 x float> %48, i1 true, i1 false) #4
  %51 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %49, i32 -1, i32 63, <64 x float> %49, i1 true, i1 false) #4
  %52 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %50, i32 -1, i32 63, <64 x float> %50, i1 true, i1 false) #4
  %53 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %51, <256 x i8> %5, i8 0, i32 0, <64 x float> %49, i1 true, i1 false) #4
  %54 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %52, <256 x i8> %5, i8 0, i32 0, <64 x float> %50, i1 true, i1 false) #4
  %55 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %53, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %56 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %53, i32 -1, i32 21, <64 x float> %55, i1 true, i1 false) #4
  %57 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %53, i32 -1, i32 22, <64 x float> %55, i1 true, i1 false) #4
  %58 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %57, i32 -1, i32 23, <64 x float> %57, i1 true, i1 false) #4
  %59 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %54, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %60 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %54, i32 -1, i32 21, <64 x float> %59, i1 true, i1 false) #4
  %61 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %54, i32 -1, i32 22, <64 x float> %59, i1 true, i1 false) #4
  %62 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %61, i32 -1, i32 23, <64 x float> %61, i1 true, i1 false) #4
  %63 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %56, i32 -1, i32 0, i32 13824, <64 x float> %56, i1 true, i1 false) #4
  %64 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %60, i32 -1, i32 0, i32 15761408, <64 x float> %63, i1 true, i1 false) #4
  %65 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %58, i32 -1, i32 0, i32 13824, <64 x float> %58, i1 true, i1 false) #4
  %66 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %62, i32 -1, i32 0, i32 15761408, <64 x float> %65, i1 true, i1 false) #4
  %67 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %28, <256 x i8> %4, i8 0, i32 0, <64 x float> %28, i1 true, i1 false) #4
  %68 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %46, <256 x i8> %4, i8 0, i32 0, <64 x float> %46, i1 true, i1 false) #4
  %69 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %67, i32 -1, i32 63, <64 x float> %67, i1 true, i1 false) #4
  %70 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %68, i32 -1, i32 63, <64 x float> %68, i1 true, i1 false) #4
  %71 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %69, <256 x i8> %5, i8 0, i32 0, <64 x float> %67, i1 true, i1 false) #4
  %72 = tail call <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float> %70, <256 x i8> %5, i8 0, i32 0, <64 x float> %68, i1 true, i1 false) #4
  %73 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %71, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %74 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %71, i32 -1, i32 21, <64 x float> %73, i1 true, i1 false) #4
  %75 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %71, i32 -1, i32 22, <64 x float> %73, i1 true, i1 false) #4
  %76 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %75, i32 -1, i32 23, <64 x float> %75, i1 true, i1 false) #4
  %77 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %72, i32 -1, i32 0, i32 3354880, <64 x float> zeroinitializer, i1 true, i1 false) #4
  %78 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %72, i32 -1, i32 21, <64 x float> %77, i1 true, i1 false) #4
  %79 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %72, i32 -1, i32 22, <64 x float> %77, i1 true, i1 false) #4
  %80 = tail call <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float> %79, i32 -1, i32 23, <64 x float> %79, i1 true, i1 false) #4
  %81 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %74, i32 -1, i32 0, i32 13824, <64 x float> %74, i1 true, i1 false) #4
  %82 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %78, i32 -1, i32 0, i32 15761408, <64 x float> %81, i1 true, i1 false) #4
  %83 = tail call <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float> %76, i32 -1, i32 0, i32 13824, <64 x float> %76, i1 true, i1 false) #4
  %84 = tail call <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float> %80, i32 -1, i32 0, i32 15761408, <64 x float> %83, i1 true, i1 false) #4

  ; %6 = shufflevector <256 x float> %in, <256 x float> undef, <64 x i32>
  %85 = shufflevector <64 x float> %66, <64 x float> %84, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %86 = shufflevector <64 x float> %64, <64 x float> %82, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %87 = shufflevector <128 x float> %85, <128 x float> %86, <256 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127, i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>

  ret <256 x float> %87
}

; Function Attrs: nounwind readonly
declare <64 x i32> @llvm.read_register.v64i32(metadata) #1

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32>, i32, i32, <128 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #3

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.shuffle.v64f32.i1(<64 x float>, <256 x i8>, i8, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mov.group.v64f32.v64f32.i1(<64 x float>, i32, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mov.dual.group.all.v64f32.i1(<64 x float>, i32, i32, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mov.dual.group.v64f32.i1(<64 x float>, i32, i32, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #2

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi2" "target-features"="+gaudi2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind }
attributes #5 = { alwaysinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.named.register.v_lane_id_32 = !{!0}

!0 = !{!"v_lane_id_32"}



)";
#endif
