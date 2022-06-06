; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*128, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*128, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*128, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*128, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/add_broadcast_non_fcd_i16_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/add_broadcast_non_fcd_i16_goya2.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main(i8 signext %0, i8 signext %1, i8 signext %2, i8 signext %3, i8 signext %4, i8 signext %5, i8 signext %6, i8 signext %7, i8 signext %8, i8 signext %9, i8 signext %10, i8 signext %11, i8 signext %12, i8 signext %13, i8 signext %14, i8 signext %15) local_unnamed_addr #0 {
  %17 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %18 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %19 = add <5 x i32> %18, %17
  %20 = extractelement <5 x i32> %17, i32 0
  %21 = shl nsw i32 %20, 7
  %22 = extractelement <5 x i32> %19, i32 0
  %23 = shl nsw i32 %22, 7
  %24 = extractelement <5 x i32> %17, i32 1
  %25 = shl nsw i32 %24, 2
  %26 = extractelement <5 x i32> %19, i32 1
  %27 = shl i32 %26, 2
  %28 = extractelement <5 x i32> %17, i32 2
  %29 = extractelement <5 x i32> %19, i32 2
  %30 = extractelement <5 x i32> %17, i32 3
  %31 = extractelement <5 x i32> %19, i32 3
  %32 = extractelement <5 x i32> %17, i32 4
  %33 = extractelement <5 x i32> %19, i32 4
  %34 = or i8 %13, %12
  %35 = icmp eq i8 %34, 0
  br i1 %35, label %36, label %59

36:                                               ; preds = %16
  %37 = icmp slt i32 %21, %23
  br i1 %37, label %38, label %273

38:                                               ; preds = %36
  %39 = icmp ne i8 %2, 0
  %40 = icmp ne i8 %7, 0
  %41 = icmp ne i8 %14, 0
  %42 = icmp ne i8 %15, 0
  %43 = icmp slt i32 %32, %33
  %44 = icmp ne i8 %6, 0
  %45 = icmp ne i8 %11, 0
  %46 = icmp slt i32 %30, %31
  %47 = icmp ne i8 %5, 0
  %48 = icmp ne i8 %10, 0
  %49 = icmp slt i32 %28, %29
  %50 = icmp ne i8 %4, 0
  %51 = icmp ne i8 %9, 0
  %52 = icmp slt i32 %25, %27
  %53 = icmp ne i8 %3, 0
  %54 = icmp ne i8 %8, 0
  %55 = or i8 %14, %2
  %56 = icmp ne i8 %55, 0
  %57 = or i8 %15, %7
  %58 = icmp ne i8 %57, 0
  br label %152

59:                                               ; preds = %16
  %60 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %61 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %62 = icmp ne i8 %12, 0
  %63 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %60, i32 0, <128 x i16> zeroinitializer, i1 %62, i1 false)
  %64 = icmp ne i8 %13, 0
  %65 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %61, i32 0, <128 x i16> zeroinitializer, i1 %64, i1 false)
  %66 = icmp slt i32 %21, %23
  br i1 %66, label %67, label %273

67:                                               ; preds = %59
  %68 = icmp slt i32 %32, %33
  %69 = icmp slt i32 %30, %31
  %70 = icmp slt i32 %28, %29
  %71 = icmp slt i32 %25, %27
  br label %72

72:                                               ; preds = %67, %83
  %73 = phi i32 [ %21, %67 ], [ %87, %83 ]
  %74 = phi <128 x i16> [ %65, %67 ], [ %86, %83 ]
  %75 = phi <128 x i16> [ %63, %67 ], [ %85, %83 ]
  %76 = phi <5 x i32> [ zeroinitializer, %67 ], [ %84, %83 ]
  %77 = insertelement <5 x i32> %76, i32 %73, i32 0
  br i1 %68, label %78, label %83

78:                                               ; preds = %72
  br label %89

79:                                               ; preds = %100
  %80 = phi <5 x i32> [ %101, %100 ]
  %81 = phi <128 x i16> [ %102, %100 ]
  %82 = phi <128 x i16> [ %103, %100 ]
  br label %83

83:                                               ; preds = %79, %72
  %84 = phi <5 x i32> [ %77, %72 ], [ %80, %79 ]
  %85 = phi <128 x i16> [ %75, %72 ], [ %81, %79 ]
  %86 = phi <128 x i16> [ %74, %72 ], [ %82, %79 ]
  %87 = add nsw i32 %73, 128
  %88 = icmp slt i32 %87, %23
  br i1 %88, label %72, label %272, !llvm.loop !3

89:                                               ; preds = %78, %100
  %90 = phi i32 [ %104, %100 ], [ %32, %78 ]
  %91 = phi <128 x i16> [ %103, %100 ], [ %74, %78 ]
  %92 = phi <128 x i16> [ %102, %100 ], [ %75, %78 ]
  %93 = phi <5 x i32> [ %101, %100 ], [ %77, %78 ]
  %94 = insertelement <5 x i32> %93, i32 %90, i32 4
  br i1 %69, label %95, label %100

95:                                               ; preds = %89
  br label %106

96:                                               ; preds = %117
  %97 = phi <5 x i32> [ %118, %117 ]
  %98 = phi <128 x i16> [ %119, %117 ]
  %99 = phi <128 x i16> [ %120, %117 ]
  br label %100

100:                                              ; preds = %96, %89
  %101 = phi <5 x i32> [ %94, %89 ], [ %97, %96 ]
  %102 = phi <128 x i16> [ %92, %89 ], [ %98, %96 ]
  %103 = phi <128 x i16> [ %91, %89 ], [ %99, %96 ]
  %104 = add i32 %90, 1
  %105 = icmp eq i32 %104, %33
  br i1 %105, label %79, label %89, !llvm.loop !5

106:                                              ; preds = %95, %117
  %107 = phi i32 [ %121, %117 ], [ %30, %95 ]
  %108 = phi <128 x i16> [ %120, %117 ], [ %91, %95 ]
  %109 = phi <128 x i16> [ %119, %117 ], [ %92, %95 ]
  %110 = phi <5 x i32> [ %118, %117 ], [ %94, %95 ]
  %111 = insertelement <5 x i32> %110, i32 %107, i32 3
  br i1 %70, label %112, label %117

112:                                              ; preds = %106
  br label %123

113:                                              ; preds = %135
  %114 = phi <5 x i32> [ %136, %135 ]
  %115 = phi <128 x i16> [ %137, %135 ]
  %116 = phi <128 x i16> [ %138, %135 ]
  br label %117

117:                                              ; preds = %113, %106
  %118 = phi <5 x i32> [ %111, %106 ], [ %114, %113 ]
  %119 = phi <128 x i16> [ %109, %106 ], [ %115, %113 ]
  %120 = phi <128 x i16> [ %108, %106 ], [ %116, %113 ]
  %121 = add i32 %107, 1
  %122 = icmp eq i32 %121, %31
  br i1 %122, label %96, label %106, !llvm.loop !6

123:                                              ; preds = %112, %135
  %124 = phi i32 [ %139, %135 ], [ %28, %112 ]
  %125 = phi <128 x i16> [ %138, %135 ], [ %108, %112 ]
  %126 = phi <128 x i16> [ %137, %135 ], [ %109, %112 ]
  %127 = phi <5 x i32> [ %136, %135 ], [ %111, %112 ]
  %128 = insertelement <5 x i32> %127, i32 %124, i32 2
  %129 = insertelement <5 x i32> %128, i32 %25, i32 1
  br i1 %71, label %130, label %135

130:                                              ; preds = %123
  br label %141

131:                                              ; preds = %141
  %132 = phi <128 x i16> [ %146, %141 ]
  %133 = phi <128 x i16> [ %147, %141 ]
  %134 = phi <5 x i32> [ %149, %141 ]
  br label %135

135:                                              ; preds = %131, %123
  %136 = phi <5 x i32> [ %129, %123 ], [ %134, %131 ]
  %137 = phi <128 x i16> [ %126, %123 ], [ %132, %131 ]
  %138 = phi <128 x i16> [ %125, %123 ], [ %133, %131 ]
  %139 = add i32 %124, 1
  %140 = icmp eq i32 %139, %29
  br i1 %140, label %113, label %123, !llvm.loop !7

141:                                              ; preds = %130, %141
  %142 = phi i32 [ %150, %141 ], [ %25, %130 ]
  %143 = phi <128 x i16> [ %147, %141 ], [ %125, %130 ]
  %144 = phi <128 x i16> [ %146, %141 ], [ %126, %130 ]
  %145 = phi <5 x i32> [ %149, %141 ], [ %129, %130 ]
  %146 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %145, i8 0, i32 0, <128 x i16> %144, i1 %64, i1 false)
  %147 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %145, i8 1, i32 0, <128 x i16> %143, i1 %62, i1 false)
  %148 = tail call <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16> %146, <128 x i16> %147, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %145, i8 2, <128 x i16> %148, i32 0, i1 true, i1 false)
  %149 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %145, i32 1, i32 2, i8 2, i32 0, <5 x i32> %145, i1 true, i1 false)
  %150 = add i32 %142, 1
  %151 = icmp eq i32 %150, %27
  br i1 %151, label %131, label %141, !llvm.loop !8

152:                                              ; preds = %38, %173
  %153 = phi <5 x i32> [ zeroinitializer, %38 ], [ %178, %173 ]
  %154 = phi <5 x i32> [ zeroinitializer, %38 ], [ %177, %173 ]
  %155 = phi i32 [ %21, %38 ], [ %179, %173 ]
  %156 = phi <5 x i32> [ zeroinitializer, %38 ], [ %176, %173 ]
  %157 = phi <128 x i16> [ zeroinitializer, %38 ], [ %175, %173 ]
  %158 = phi <128 x i16> [ zeroinitializer, %38 ], [ %174, %173 ]
  %159 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %153, i32 1, i32 %155, i32 0, i1 %39, i1 true)
  %160 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %154, i32 1, i32 %155, i32 0, i1 %40, i1 true)
  %161 = insertelement <5 x i32> %156, i32 %155, i32 0
  %162 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %159, i32 2, i32 0, i32 0, i1 %41, i1 false)
  %163 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %160, i32 2, i32 0, i32 0, i1 %42, i1 false)
  %164 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %162, i8 0, i32 0, <128 x i16> %157, i1 %41, i1 false)
  %165 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %163, i8 1, i32 0, <128 x i16> %158, i1 %42, i1 false)
  br i1 %43, label %166, label %173

166:                                              ; preds = %152
  br label %181

167:                                              ; preds = %198
  %168 = phi <128 x i16> [ %199, %198 ]
  %169 = phi <128 x i16> [ %200, %198 ]
  %170 = phi <5 x i32> [ %201, %198 ]
  %171 = phi <5 x i32> [ %202, %198 ]
  %172 = phi <5 x i32> [ %203, %198 ]
  br label %173

173:                                              ; preds = %167, %152
  %174 = phi <128 x i16> [ %165, %152 ], [ %168, %167 ]
  %175 = phi <128 x i16> [ %164, %152 ], [ %169, %167 ]
  %176 = phi <5 x i32> [ %161, %152 ], [ %170, %167 ]
  %177 = phi <5 x i32> [ %163, %152 ], [ %171, %167 ]
  %178 = phi <5 x i32> [ %162, %152 ], [ %172, %167 ]
  %179 = add nsw i32 %155, 128
  %180 = icmp slt i32 %179, %23
  br i1 %180, label %152, label %271, !llvm.loop !10

181:                                              ; preds = %166, %198
  %182 = phi <5 x i32> [ %203, %198 ], [ %162, %166 ]
  %183 = phi <5 x i32> [ %202, %198 ], [ %163, %166 ]
  %184 = phi i32 [ %204, %198 ], [ %32, %166 ]
  %185 = phi <5 x i32> [ %201, %198 ], [ %161, %166 ]
  %186 = phi <128 x i16> [ %200, %198 ], [ %164, %166 ]
  %187 = phi <128 x i16> [ %199, %198 ], [ %165, %166 ]
  %188 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %182, i32 16, i32 %184, i32 0, i1 %44, i1 true)
  %189 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %183, i32 16, i32 %184, i32 0, i1 %45, i1 true)
  %190 = insertelement <5 x i32> %185, i32 %184, i32 4
  br i1 %46, label %191, label %198

191:                                              ; preds = %181
  br label %206

192:                                              ; preds = %223
  %193 = phi <128 x i16> [ %224, %223 ]
  %194 = phi <128 x i16> [ %225, %223 ]
  %195 = phi <5 x i32> [ %226, %223 ]
  %196 = phi <5 x i32> [ %227, %223 ]
  %197 = phi <5 x i32> [ %228, %223 ]
  br label %198

198:                                              ; preds = %192, %181
  %199 = phi <128 x i16> [ %187, %181 ], [ %193, %192 ]
  %200 = phi <128 x i16> [ %186, %181 ], [ %194, %192 ]
  %201 = phi <5 x i32> [ %190, %181 ], [ %195, %192 ]
  %202 = phi <5 x i32> [ %189, %181 ], [ %196, %192 ]
  %203 = phi <5 x i32> [ %188, %181 ], [ %197, %192 ]
  %204 = add i32 %184, 1
  %205 = icmp eq i32 %204, %33
  br i1 %205, label %167, label %181, !llvm.loop !11

206:                                              ; preds = %191, %223
  %207 = phi <5 x i32> [ %228, %223 ], [ %188, %191 ]
  %208 = phi <5 x i32> [ %227, %223 ], [ %189, %191 ]
  %209 = phi i32 [ %229, %223 ], [ %30, %191 ]
  %210 = phi <5 x i32> [ %226, %223 ], [ %190, %191 ]
  %211 = phi <128 x i16> [ %225, %223 ], [ %186, %191 ]
  %212 = phi <128 x i16> [ %224, %223 ], [ %187, %191 ]
  %213 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %207, i32 8, i32 %209, i32 0, i1 %47, i1 true)
  %214 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %208, i32 8, i32 %209, i32 0, i1 %48, i1 true)
  %215 = insertelement <5 x i32> %210, i32 %209, i32 3
  br i1 %49, label %216, label %223

216:                                              ; preds = %206
  br label %231

217:                                              ; preds = %248
  %218 = phi <128 x i16> [ %249, %248 ]
  %219 = phi <128 x i16> [ %250, %248 ]
  %220 = phi <5 x i32> [ %251, %248 ]
  %221 = phi <5 x i32> [ %252, %248 ]
  %222 = phi <5 x i32> [ %253, %248 ]
  br label %223

223:                                              ; preds = %217, %206
  %224 = phi <128 x i16> [ %212, %206 ], [ %218, %217 ]
  %225 = phi <128 x i16> [ %211, %206 ], [ %219, %217 ]
  %226 = phi <5 x i32> [ %215, %206 ], [ %220, %217 ]
  %227 = phi <5 x i32> [ %214, %206 ], [ %221, %217 ]
  %228 = phi <5 x i32> [ %213, %206 ], [ %222, %217 ]
  %229 = add i32 %209, 1
  %230 = icmp eq i32 %229, %31
  br i1 %230, label %192, label %206, !llvm.loop !12

231:                                              ; preds = %216, %248
  %232 = phi <5 x i32> [ %253, %248 ], [ %213, %216 ]
  %233 = phi <5 x i32> [ %252, %248 ], [ %214, %216 ]
  %234 = phi i32 [ %254, %248 ], [ %28, %216 ]
  %235 = phi <5 x i32> [ %251, %248 ], [ %215, %216 ]
  %236 = phi <128 x i16> [ %250, %248 ], [ %211, %216 ]
  %237 = phi <128 x i16> [ %249, %248 ], [ %212, %216 ]
  %238 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %232, i32 4, i32 %234, i32 0, i1 %50, i1 true)
  %239 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %233, i32 4, i32 %234, i32 0, i1 %51, i1 true)
  %240 = insertelement <5 x i32> %235, i32 %234, i32 2
  br i1 %52, label %241, label %248

241:                                              ; preds = %231
  br label %256

242:                                              ; preds = %256
  %243 = phi <5 x i32> [ %263, %256 ]
  %244 = phi <5 x i32> [ %264, %256 ]
  %245 = phi <5 x i32> [ %265, %256 ]
  %246 = phi <128 x i16> [ %266, %256 ]
  %247 = phi <128 x i16> [ %267, %256 ]
  br label %248

248:                                              ; preds = %242, %231
  %249 = phi <128 x i16> [ %237, %231 ], [ %247, %242 ]
  %250 = phi <128 x i16> [ %236, %231 ], [ %246, %242 ]
  %251 = phi <5 x i32> [ %240, %231 ], [ %245, %242 ]
  %252 = phi <5 x i32> [ %239, %231 ], [ %244, %242 ]
  %253 = phi <5 x i32> [ %238, %231 ], [ %243, %242 ]
  %254 = add i32 %234, 1
  %255 = icmp eq i32 %254, %29
  br i1 %255, label %217, label %231, !llvm.loop !13

256:                                              ; preds = %241, %256
  %257 = phi <5 x i32> [ %263, %256 ], [ %238, %241 ]
  %258 = phi <5 x i32> [ %264, %256 ], [ %239, %241 ]
  %259 = phi i32 [ %269, %256 ], [ %25, %241 ]
  %260 = phi <5 x i32> [ %265, %256 ], [ %240, %241 ]
  %261 = phi <128 x i16> [ %266, %256 ], [ %236, %241 ]
  %262 = phi <128 x i16> [ %267, %256 ], [ %237, %241 ]
  %263 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %257, i32 2, i32 %259, i32 0, i1 %53, i1 true)
  %264 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %258, i32 2, i32 %259, i32 0, i1 %54, i1 true)
  %265 = insertelement <5 x i32> %260, i32 %259, i32 1
  %266 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %263, i8 0, i32 0, <128 x i16> %261, i1 %56, i1 true)
  %267 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %264, i8 1, i32 0, <128 x i16> %262, i1 %58, i1 true)
  %268 = tail call <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16> %266, <128 x i16> %267, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %265, i8 2, <128 x i16> %268, i32 0, i1 true, i1 false)
  %269 = add i32 %259, 1
  %270 = icmp eq i32 %269, %27
  br i1 %270, label %242, label %256, !llvm.loop !14

271:                                              ; preds = %173
  br label %273

272:                                              ; preds = %83
  br label %273

273:                                              ; preds = %272, %271, %59, %36
  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; Function Attrs: nounwind readonly
declare <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)*, i32, <128 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #3

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.set.indx(<5 x i32>, i32, i32, i32, i1, i1) #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="greco" "target-features"="+greco" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.taken", i1 true}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = distinct !{!7, !4}
!8 = distinct !{!8, !9, !4}
!9 = !{!"llvm.loop.machine.unroll.count", i32 4}
!10 = distinct !{!10, !4}
!11 = distinct !{!11, !4}
!12 = distinct !{!12, !4}
!13 = distinct !{!13, !4}
!14 = distinct !{!14, !9, !4}
