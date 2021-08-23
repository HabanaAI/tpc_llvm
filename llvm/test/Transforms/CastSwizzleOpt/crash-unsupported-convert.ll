;RUN: opt < %s -cast-swizzle-opt -mcpu=gaudi -S | FileCheck %s


; CHECK-NOT:    call <256 x float> @llvm.tpc.convert.v256f32.v256i8.i1

; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

declare <256 x i8> @llvm.tpc.ld.g.v256i8(i8 addrspace(3)*, i32, <256 x i8>, i1, i1)

declare <256 x float> @llvm.tpc.sitofp.swch.v256f32.v256i8(<256 x i8>, i32)

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

declare <5 x i32> @llvm.tpc.get.index.space.offset()

declare <5 x i32> @llvm.tpc.get.index.space.size()

define void @main() {
bb:
	%i = call <5 x i32> @llvm.tpc.get.index.space.offset()
	%i26 = call <5 x i32> @llvm.tpc.get.index.space.size()
	%i27 = add <5 x i32> %i, %i26
	%i28 = extractelement <5 x i32> %i, i32 0
	%i29 = shl i32 %i28, 8
	%i30 = extractelement <5 x i32> %i27, i32 0
	%i31 = shl i32 %i30, 8
	%i32 = extractelement <5 x i32> %i, i32 1
	%i33 = extractelement <5 x i32> %i27, i32 1
	%i34 = extractelement <5 x i32> %i, i32 2
	%i35 = extractelement <5 x i32> %i27, i32 2
	%.sroa.0.0.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 0
	br label %bb36

	bb36:                                             ; preds = %bb69, %bb
	%.sroa.024.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.024.1, %bb69 ]
	%.sroa.022.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.022.1, %bb69 ]
	%.sroa.020.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.020.1, %bb69 ]
	%.sroa.018.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.018.1, %bb69 ]
	%.sroa.016.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.016.1, %bb69 ]
	%.sroa.014.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.014.1, %bb69 ]
	%.sroa.012.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.012.1, %bb69 ]
	%.sroa.010.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.010.1, %bb69 ]
	%.sroa.0.0 = phi <5 x i32> [ %.sroa.0.0.vec.insert, %bb ], [ %.sroa.0.1, %bb69 ]
	%i37 = phi i32 [ %i70, %bb69 ], [ %i32, %bb ]
	%i38 = icmp slt i32 %i37, %i33
	br i1 %i38, label %bb39, label %bb71

	bb39:                                             ; preds = %bb36
	%.sroa.024.4.vec.insert = insertelement <5 x i32> %.sroa.024.0, i32 %i37, i32 1
	%.sroa.022.4.vec.insert = insertelement <5 x i32> %.sroa.022.0, i32 %i37, i32 1
	%.sroa.020.4.vec.insert = insertelement <5 x i32> %.sroa.020.0, i32 %i37, i32 1
	%.sroa.018.4.vec.insert = insertelement <5 x i32> %.sroa.018.0, i32 %i37, i32 1
	%.sroa.016.4.vec.insert = insertelement <5 x i32> %.sroa.016.0, i32 %i37, i32 1
	%.sroa.014.4.vec.insert = insertelement <5 x i32> %.sroa.014.0, i32 %i37, i32 1
	%.sroa.012.4.vec.insert = insertelement <5 x i32> %.sroa.012.0, i32 %i37, i32 1
	%.sroa.010.4.vec.insert = insertelement <5 x i32> %.sroa.010.0, i32 %i37, i32 1
	%.sroa.0.4.vec.insert = insertelement <5 x i32> %.sroa.0.0, i32 %i37, i32 1
	br label %bb40

	bb40:                                             ; preds = %bb67, %bb39
	%.sroa.024.1 = phi <5 x i32> [ %.sroa.024.4.vec.insert, %bb39 ], [ %.sroa.024.2, %bb67 ]
	%.sroa.022.1 = phi <5 x i32> [ %.sroa.022.4.vec.insert, %bb39 ], [ %.sroa.022.2, %bb67 ]
	%.sroa.020.1 = phi <5 x i32> [ %.sroa.020.4.vec.insert, %bb39 ], [ %.sroa.020.2, %bb67 ]
	%.sroa.018.1 = phi <5 x i32> [ %.sroa.018.4.vec.insert, %bb39 ], [ %.sroa.018.2, %bb67 ]
	%.sroa.016.1 = phi <5 x i32> [ %.sroa.016.4.vec.insert, %bb39 ], [ %.sroa.016.2, %bb67 ]
	%.sroa.014.1 = phi <5 x i32> [ %.sroa.014.4.vec.insert, %bb39 ], [ %.sroa.014.2, %bb67 ]
	%.sroa.012.1 = phi <5 x i32> [ %.sroa.012.4.vec.insert, %bb39 ], [ %.sroa.012.2, %bb67 ]
	%.sroa.010.1 = phi <5 x i32> [ %.sroa.010.4.vec.insert, %bb39 ], [ %.sroa.010.2, %bb67 ]
	%.sroa.0.1 = phi <5 x i32> [ %.sroa.0.4.vec.insert, %bb39 ], [ %.sroa.0.2, %bb67 ]
	%i41 = phi i32 [ %i68, %bb67 ], [ %i29, %bb39 ]
	%i42 = icmp slt i32 %i41, %i31
	br i1 %i42, label %bb43, label %bb69

	bb43:                                             ; preds = %bb40
	%i44 = or i32 %i41, 64
	%i45 = or i32 %i41, 128
	%i46 = or i32 %i41, 192
	br label %bb47

	bb47:                                             ; preds = %bb50, %bb43
	%.sroa.024.2 = phi <5 x i32> [ %.sroa.024.1, %bb43 ], [ %.sroa.024.0.vec.insert, %bb50 ]
	%.sroa.022.2 = phi <5 x i32> [ %.sroa.022.1, %bb43 ], [ %.sroa.022.0.vec.insert, %bb50 ]
	%.sroa.020.2 = phi <5 x i32> [ %.sroa.020.1, %bb43 ], [ %.sroa.020.0.vec.insert, %bb50 ]
	%.sroa.018.2 = phi <5 x i32> [ %.sroa.018.1, %bb43 ], [ %.sroa.018.0.vec.insert, %bb50 ]
	%.sroa.016.2 = phi <5 x i32> [ %.sroa.016.1, %bb43 ], [ %.sroa.016.0.vec.insert, %bb50 ]
	%.sroa.014.2 = phi <5 x i32> [ %.sroa.014.1, %bb43 ], [ %.sroa.014.0.vec.insert, %bb50 ]
	%.sroa.012.2 = phi <5 x i32> [ %.sroa.012.1, %bb43 ], [ %.sroa.012.0.vec.insert, %bb50 ]
	%.sroa.010.2 = phi <5 x i32> [ %.sroa.010.1, %bb43 ], [ %.sroa.010.0.vec.insert, %bb50 ]
	%.sroa.0.2 = phi <5 x i32> [ %.sroa.0.1, %bb43 ], [ %.sroa.0.8.vec.insert, %bb50 ]
	%i48 = phi i32 [ %i66, %bb50 ], [ %i34, %bb43 ]
	%i49 = icmp slt i32 %i48, %i35
	br i1 %i49, label %bb50, label %bb67

	bb50:                                             ; preds = %bb47
	%.sroa.024.8.vec.insert = insertelement <5 x i32> %.sroa.024.2, i32 %i48, i32 2
	%.sroa.022.8.vec.insert = insertelement <5 x i32> %.sroa.022.2, i32 %i48, i32 2
	%.sroa.020.8.vec.insert = insertelement <5 x i32> %.sroa.020.2, i32 %i48, i32 2
	%.sroa.018.8.vec.insert = insertelement <5 x i32> %.sroa.018.2, i32 %i48, i32 2
	%.sroa.016.8.vec.insert = insertelement <5 x i32> %.sroa.016.2, i32 %i48, i32 2
	%.sroa.014.8.vec.insert = insertelement <5 x i32> %.sroa.014.2, i32 %i48, i32 2
	%.sroa.012.8.vec.insert = insertelement <5 x i32> %.sroa.012.2, i32 %i48, i32 2
	%.sroa.010.8.vec.insert = insertelement <5 x i32> %.sroa.010.2, i32 %i48, i32 2
	%.sroa.0.8.vec.insert = insertelement <5 x i32> %.sroa.0.2, i32 %i48, i32 2
	%i51 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.0.8.vec.insert, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
	%i52 = call <256 x i8> @llvm.tpc.ld.g.v256i8(i8 addrspace(3)* %i51, i32 0, <256 x i8> undef, i1 true, i1 false)
	%i53 = call <256 x float> @llvm.tpc.sitofp.swch.v256f32.v256i8(<256 x i8> %i52, i32 0)
	%i54 = shufflevector <256 x float> %i53, <256 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
	%i55 = shufflevector <256 x float> %i53, <256 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
	%i56 = shufflevector <256 x float> %i53, <256 x float> undef, <64 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191>
	%i57 = shufflevector <256 x float> %i53, <256 x float> undef, <64 x i32> <i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
	%.sroa.010.0.vec.insert = insertelement <5 x i32> %.sroa.010.8.vec.insert, i32 %i41, i32 0
	%i58 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.010.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.012.0.vec.insert = insertelement <5 x i32> %.sroa.012.8.vec.insert, i32 %i44, i32 0
	%i59 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.012.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.014.0.vec.insert = insertelement <5 x i32> %.sroa.014.8.vec.insert, i32 %i45, i32 0
	%i60 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.014.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.016.0.vec.insert = insertelement <5 x i32> %.sroa.016.8.vec.insert, i32 %i46, i32 0
	%i61 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.016.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%i62 = fadd <64 x float> %i58, %i54
	%i63 = fadd <64 x float> %i59, %i55
	%i64 = fadd <64 x float> %i60, %i56
	%i65 = fadd <64 x float> %i61, %i57
	%.sroa.018.0.vec.insert = insertelement <5 x i32> %.sroa.018.8.vec.insert, i32 %i41, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.018.0.vec.insert, i8 2, <64 x float> %i62, i32 0, i1 true, i1 false)
	%.sroa.020.0.vec.insert = insertelement <5 x i32> %.sroa.020.8.vec.insert, i32 %i44, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.020.0.vec.insert, i8 2, <64 x float> %i63, i32 0, i1 true, i1 false)
	%.sroa.022.0.vec.insert = insertelement <5 x i32> %.sroa.022.8.vec.insert, i32 %i45, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.022.0.vec.insert, i8 2, <64 x float> %i64, i32 0, i1 true, i1 false)
	%.sroa.024.0.vec.insert = insertelement <5 x i32> %.sroa.024.8.vec.insert, i32 %i46, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.024.0.vec.insert, i8 2, <64 x float> %i65, i32 0, i1 true, i1 false)
	%i66 = add i32 %i48, 1
	br label %bb47

	bb67:                                             ; preds = %bb47
	%i68 = add i32 %i41, 256
	br label %bb40

	bb69:                                             ; preds = %bb40
	%i70 = add i32 %i37, 1
	br label %bb36

	bb71:                                             ; preds = %bb36
	ret void
}

