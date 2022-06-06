; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%3)*128, not(%4)*4, not(%5)*1, not(%6)*1, not(%7)*1 }[1].[Input].{ not(%8)*128, not(%9)*4, not(%10)*1, not(%11)*1, not(%12)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%3)*128, not(%4)*4, not(%5)*1, not(%6)*1, not(%7)*1 }[1].[Input].{ not(%8)*128, not(%9)*4, not(%10)*1, not(%11)*1, not(%12)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/sub_diffscale0_broadcast_fcd_i16_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/sub_diffscale0_broadcast_fcd_i16_goya2.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main(i32 %0, i8 signext %1, i8 signext %2, i8 signext %3, i8 signext %4, i8 signext %5, i8 signext %6, i8 signext %7, i8 signext %8, i8 signext %9, i8 signext %10, i8 signext %11, i8 signext %12, i8 signext %13, i8 signext %14, i8 signext %15, i8 signext %16) local_unnamed_addr #0 {
  %18 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %19 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %20 = add <5 x i32> %19, %18
  %21 = extractelement <5 x i32> %18, i32 0
  %22 = shl nsw i32 %21, 7
  %23 = extractelement <5 x i32> %20, i32 0
  %24 = shl nsw i32 %23, 7
  %25 = extractelement <5 x i32> %18, i32 1
  %26 = shl nsw i32 %25, 2
  %27 = extractelement <5 x i32> %20, i32 1
  %28 = shl i32 %27, 2
  %29 = extractelement <5 x i32> %18, i32 2
  %30 = extractelement <5 x i32> %20, i32 2
  %31 = extractelement <5 x i32> %18, i32 3
  %32 = extractelement <5 x i32> %20, i32 3
  %33 = extractelement <5 x i32> %18, i32 4
  %34 = extractelement <5 x i32> %20, i32 4
  %35 = or i8 %14, %13
  %36 = icmp eq i8 %35, 0
  %37 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %38 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %39 = icmp ne i8 %13, 0
  %40 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %37, i32 0, <128 x i16> zeroinitializer, i1 %39, i1 false)
  %41 = icmp ne i8 %14, 0
  %42 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %38, i32 0, <128 x i16> zeroinitializer, i1 %41, i1 false)
  %43 = trunc i32 %0 to i8
  %44 = insertelement <256 x i8> undef, i8 %43, i32 0
  %45 = shufflevector <256 x i8> %44, <256 x i8> undef, <256 x i32> zeroinitializer
  %46 = tail call <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16> %40, <256 x i8> %45, i8 7, i32 2, <128 x i16> %40, i1 %39, i1 false)
  %47 = icmp slt i32 %22, %24
  br i1 %36, label %54, label %48

48:                                               ; preds = %17
  br i1 %47, label %49, label %283

49:                                               ; preds = %48
  %50 = icmp slt i32 %33, %34
  %51 = icmp slt i32 %31, %32
  %52 = icmp slt i32 %29, %30
  %53 = icmp slt i32 %26, %28
  br label %76

54:                                               ; preds = %17
  br i1 %47, label %55, label %283

55:                                               ; preds = %54
  %56 = icmp ne i8 %3, 0
  %57 = icmp ne i8 %8, 0
  %58 = icmp ne i8 %15, 0
  %59 = icmp ne i8 %16, 0
  %60 = icmp slt i32 %33, %34
  %61 = icmp ne i8 %7, 0
  %62 = icmp ne i8 %12, 0
  %63 = icmp slt i32 %31, %32
  %64 = icmp ne i8 %6, 0
  %65 = icmp ne i8 %11, 0
  %66 = icmp slt i32 %29, %30
  %67 = icmp ne i8 %5, 0
  %68 = icmp ne i8 %10, 0
  %69 = icmp slt i32 %26, %28
  %70 = icmp ne i8 %4, 0
  %71 = icmp ne i8 %9, 0
  %72 = or i8 %15, %3
  %73 = icmp ne i8 %72, 0
  %74 = or i8 %16, %8
  %75 = icmp ne i8 %74, 0
  br label %157

76:                                               ; preds = %49, %87
  %77 = phi <5 x i32> [ zeroinitializer, %49 ], [ %90, %87 ]
  %78 = phi i32 [ %22, %49 ], [ %91, %87 ]
  %79 = phi <128 x i16> [ %42, %49 ], [ %89, %87 ]
  %80 = phi <128 x i16> [ %40, %49 ], [ %88, %87 ]
  %81 = insertelement <5 x i32> %77, i32 %78, i32 0
  br i1 %50, label %82, label %87

82:                                               ; preds = %76
  br label %93

83:                                               ; preds = %104
  %84 = phi <128 x i16> [ %105, %104 ]
  %85 = phi <128 x i16> [ %106, %104 ]
  %86 = phi <5 x i32> [ %107, %104 ]
  br label %87

87:                                               ; preds = %83, %76
  %88 = phi <128 x i16> [ %80, %76 ], [ %84, %83 ]
  %89 = phi <128 x i16> [ %79, %76 ], [ %85, %83 ]
  %90 = phi <5 x i32> [ %81, %76 ], [ %86, %83 ]
  %91 = add nsw i32 %78, 128
  %92 = icmp slt i32 %91, %24
  br i1 %92, label %76, label %282, !llvm.loop !3

93:                                               ; preds = %82, %104
  %94 = phi <5 x i32> [ %107, %104 ], [ %81, %82 ]
  %95 = phi i32 [ %108, %104 ], [ %33, %82 ]
  %96 = phi <128 x i16> [ %106, %104 ], [ %79, %82 ]
  %97 = phi <128 x i16> [ %105, %104 ], [ %80, %82 ]
  %98 = insertelement <5 x i32> %94, i32 %95, i32 4
  br i1 %51, label %99, label %104

99:                                               ; preds = %93
  br label %110

100:                                              ; preds = %121
  %101 = phi <128 x i16> [ %122, %121 ]
  %102 = phi <128 x i16> [ %123, %121 ]
  %103 = phi <5 x i32> [ %124, %121 ]
  br label %104

104:                                              ; preds = %100, %93
  %105 = phi <128 x i16> [ %97, %93 ], [ %101, %100 ]
  %106 = phi <128 x i16> [ %96, %93 ], [ %102, %100 ]
  %107 = phi <5 x i32> [ %98, %93 ], [ %103, %100 ]
  %108 = add i32 %95, 1
  %109 = icmp eq i32 %108, %34
  br i1 %109, label %83, label %93, !llvm.loop !5

110:                                              ; preds = %99, %121
  %111 = phi <5 x i32> [ %124, %121 ], [ %98, %99 ]
  %112 = phi i32 [ %125, %121 ], [ %31, %99 ]
  %113 = phi <128 x i16> [ %123, %121 ], [ %96, %99 ]
  %114 = phi <128 x i16> [ %122, %121 ], [ %97, %99 ]
  %115 = insertelement <5 x i32> %111, i32 %112, i32 3
  br i1 %52, label %116, label %121

116:                                              ; preds = %110
  br label %127

117:                                              ; preds = %139
  %118 = phi <128 x i16> [ %140, %139 ]
  %119 = phi <128 x i16> [ %141, %139 ]
  %120 = phi <5 x i32> [ %142, %139 ]
  br label %121

121:                                              ; preds = %117, %110
  %122 = phi <128 x i16> [ %114, %110 ], [ %118, %117 ]
  %123 = phi <128 x i16> [ %113, %110 ], [ %119, %117 ]
  %124 = phi <5 x i32> [ %115, %110 ], [ %120, %117 ]
  %125 = add i32 %112, 1
  %126 = icmp eq i32 %125, %32
  br i1 %126, label %100, label %110, !llvm.loop !6

127:                                              ; preds = %116, %139
  %128 = phi <5 x i32> [ %142, %139 ], [ %115, %116 ]
  %129 = phi i32 [ %143, %139 ], [ %29, %116 ]
  %130 = phi <128 x i16> [ %141, %139 ], [ %113, %116 ]
  %131 = phi <128 x i16> [ %140, %139 ], [ %114, %116 ]
  %132 = insertelement <5 x i32> %128, i32 %129, i32 2
  %133 = insertelement <5 x i32> %132, i32 %26, i32 1
  br i1 %53, label %134, label %139

134:                                              ; preds = %127
  br label %145

135:                                              ; preds = %145
  %136 = phi <128 x i16> [ %150, %145 ]
  %137 = phi <128 x i16> [ %151, %145 ]
  %138 = phi <5 x i32> [ %154, %145 ]
  br label %139

139:                                              ; preds = %135, %127
  %140 = phi <128 x i16> [ %131, %127 ], [ %136, %135 ]
  %141 = phi <128 x i16> [ %130, %127 ], [ %137, %135 ]
  %142 = phi <5 x i32> [ %133, %127 ], [ %138, %135 ]
  %143 = add i32 %129, 1
  %144 = icmp eq i32 %143, %30
  br i1 %144, label %117, label %127, !llvm.loop !7

145:                                              ; preds = %134, %145
  %146 = phi <5 x i32> [ %154, %145 ], [ %133, %134 ]
  %147 = phi i32 [ %155, %145 ], [ %26, %134 ]
  %148 = phi <128 x i16> [ %151, %145 ], [ %130, %134 ]
  %149 = phi <128 x i16> [ %150, %145 ], [ %131, %134 ]
  %150 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %146, i8 0, i32 0, <128 x i16> %149, i1 %41, i1 false)
  %151 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %146, i8 1, i32 0, <128 x i16> %148, i1 %39, i1 false)
  %152 = tail call <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16> %150, <256 x i8> %45, i8 7, i32 2, <128 x i16> %46, i1 %39, i1 true)
  %153 = tail call <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16> %152, <128 x i16> %151, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %146, i8 2, <128 x i16> %153, i32 0, i1 true, i1 false)
  %154 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %146, i32 1, i32 2, i8 2, i32 0, <5 x i32> %146, i1 true, i1 false)
  %155 = add i32 %147, 1
  %156 = icmp eq i32 %155, %28
  br i1 %156, label %135, label %145, !llvm.loop !8

157:                                              ; preds = %55, %178
  %158 = phi <5 x i32> [ zeroinitializer, %55 ], [ %183, %178 ]
  %159 = phi <5 x i32> [ zeroinitializer, %55 ], [ %182, %178 ]
  %160 = phi <5 x i32> [ zeroinitializer, %55 ], [ %181, %178 ]
  %161 = phi <128 x i16> [ %46, %55 ], [ %180, %178 ]
  %162 = phi <128 x i16> [ %42, %55 ], [ %179, %178 ]
  %163 = phi i32 [ %22, %55 ], [ %184, %178 ]
  %164 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %158, i32 1, i32 %163, i32 0, i1 %56, i1 true)
  %165 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %159, i32 1, i32 %163, i32 0, i1 %57, i1 true)
  %166 = insertelement <5 x i32> %160, i32 %163, i32 0
  %167 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %164, i32 2, i32 0, i32 0, i1 %58, i1 false)
  %168 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %165, i32 2, i32 0, i32 0, i1 %59, i1 false)
  %169 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %167, i8 0, i32 0, <128 x i16> %161, i1 %58, i1 false)
  %170 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %168, i8 1, i32 0, <128 x i16> %162, i1 %59, i1 false)
  br i1 %60, label %171, label %178

171:                                              ; preds = %157
  br label %186

172:                                              ; preds = %203
  %173 = phi <128 x i16> [ %204, %203 ]
  %174 = phi <128 x i16> [ %205, %203 ]
  %175 = phi <5 x i32> [ %206, %203 ]
  %176 = phi <5 x i32> [ %207, %203 ]
  %177 = phi <5 x i32> [ %208, %203 ]
  br label %178

178:                                              ; preds = %172, %157
  %179 = phi <128 x i16> [ %170, %157 ], [ %173, %172 ]
  %180 = phi <128 x i16> [ %169, %157 ], [ %174, %172 ]
  %181 = phi <5 x i32> [ %166, %157 ], [ %175, %172 ]
  %182 = phi <5 x i32> [ %168, %157 ], [ %176, %172 ]
  %183 = phi <5 x i32> [ %167, %157 ], [ %177, %172 ]
  %184 = add nsw i32 %163, 128
  %185 = icmp slt i32 %184, %24
  br i1 %185, label %157, label %281, !llvm.loop !10

186:                                              ; preds = %171, %203
  %187 = phi <5 x i32> [ %208, %203 ], [ %167, %171 ]
  %188 = phi <5 x i32> [ %207, %203 ], [ %168, %171 ]
  %189 = phi <5 x i32> [ %206, %203 ], [ %166, %171 ]
  %190 = phi <128 x i16> [ %205, %203 ], [ %169, %171 ]
  %191 = phi <128 x i16> [ %204, %203 ], [ %170, %171 ]
  %192 = phi i32 [ %209, %203 ], [ %33, %171 ]
  %193 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %187, i32 16, i32 %192, i32 0, i1 %61, i1 true)
  %194 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %188, i32 16, i32 %192, i32 0, i1 %62, i1 true)
  %195 = insertelement <5 x i32> %189, i32 %192, i32 4
  br i1 %63, label %196, label %203

196:                                              ; preds = %186
  br label %211

197:                                              ; preds = %228
  %198 = phi <128 x i16> [ %229, %228 ]
  %199 = phi <128 x i16> [ %230, %228 ]
  %200 = phi <5 x i32> [ %231, %228 ]
  %201 = phi <5 x i32> [ %232, %228 ]
  %202 = phi <5 x i32> [ %233, %228 ]
  br label %203

203:                                              ; preds = %197, %186
  %204 = phi <128 x i16> [ %191, %186 ], [ %198, %197 ]
  %205 = phi <128 x i16> [ %190, %186 ], [ %199, %197 ]
  %206 = phi <5 x i32> [ %195, %186 ], [ %200, %197 ]
  %207 = phi <5 x i32> [ %194, %186 ], [ %201, %197 ]
  %208 = phi <5 x i32> [ %193, %186 ], [ %202, %197 ]
  %209 = add i32 %192, 1
  %210 = icmp eq i32 %209, %34
  br i1 %210, label %172, label %186, !llvm.loop !11

211:                                              ; preds = %196, %228
  %212 = phi <5 x i32> [ %233, %228 ], [ %193, %196 ]
  %213 = phi <5 x i32> [ %232, %228 ], [ %194, %196 ]
  %214 = phi <5 x i32> [ %231, %228 ], [ %195, %196 ]
  %215 = phi <128 x i16> [ %230, %228 ], [ %190, %196 ]
  %216 = phi <128 x i16> [ %229, %228 ], [ %191, %196 ]
  %217 = phi i32 [ %234, %228 ], [ %31, %196 ]
  %218 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %212, i32 8, i32 %217, i32 0, i1 %64, i1 true)
  %219 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %213, i32 8, i32 %217, i32 0, i1 %65, i1 true)
  %220 = insertelement <5 x i32> %214, i32 %217, i32 3
  br i1 %66, label %221, label %228

221:                                              ; preds = %211
  br label %236

222:                                              ; preds = %253
  %223 = phi <128 x i16> [ %254, %253 ]
  %224 = phi <128 x i16> [ %255, %253 ]
  %225 = phi <5 x i32> [ %256, %253 ]
  %226 = phi <5 x i32> [ %257, %253 ]
  %227 = phi <5 x i32> [ %258, %253 ]
  br label %228

228:                                              ; preds = %222, %211
  %229 = phi <128 x i16> [ %216, %211 ], [ %223, %222 ]
  %230 = phi <128 x i16> [ %215, %211 ], [ %224, %222 ]
  %231 = phi <5 x i32> [ %220, %211 ], [ %225, %222 ]
  %232 = phi <5 x i32> [ %219, %211 ], [ %226, %222 ]
  %233 = phi <5 x i32> [ %218, %211 ], [ %227, %222 ]
  %234 = add i32 %217, 1
  %235 = icmp eq i32 %234, %32
  br i1 %235, label %197, label %211, !llvm.loop !12

236:                                              ; preds = %221, %253
  %237 = phi <5 x i32> [ %258, %253 ], [ %218, %221 ]
  %238 = phi <5 x i32> [ %257, %253 ], [ %219, %221 ]
  %239 = phi <5 x i32> [ %256, %253 ], [ %220, %221 ]
  %240 = phi <128 x i16> [ %255, %253 ], [ %215, %221 ]
  %241 = phi <128 x i16> [ %254, %253 ], [ %216, %221 ]
  %242 = phi i32 [ %259, %253 ], [ %29, %221 ]
  %243 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %237, i32 4, i32 %242, i32 0, i1 %67, i1 true)
  %244 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %238, i32 4, i32 %242, i32 0, i1 %68, i1 true)
  %245 = insertelement <5 x i32> %239, i32 %242, i32 2
  br i1 %69, label %246, label %253

246:                                              ; preds = %236
  br label %261

247:                                              ; preds = %261
  %248 = phi <5 x i32> [ %268, %261 ]
  %249 = phi <5 x i32> [ %269, %261 ]
  %250 = phi <5 x i32> [ %270, %261 ]
  %251 = phi <128 x i16> [ %275, %261 ]
  %252 = phi <128 x i16> [ %276, %261 ]
  br label %253

253:                                              ; preds = %247, %236
  %254 = phi <128 x i16> [ %241, %236 ], [ %252, %247 ]
  %255 = phi <128 x i16> [ %240, %236 ], [ %251, %247 ]
  %256 = phi <5 x i32> [ %245, %236 ], [ %250, %247 ]
  %257 = phi <5 x i32> [ %244, %236 ], [ %249, %247 ]
  %258 = phi <5 x i32> [ %243, %236 ], [ %248, %247 ]
  %259 = add i32 %242, 1
  %260 = icmp eq i32 %259, %30
  br i1 %260, label %222, label %236, !llvm.loop !13

261:                                              ; preds = %246, %261
  %262 = phi <5 x i32> [ %268, %261 ], [ %243, %246 ]
  %263 = phi <5 x i32> [ %269, %261 ], [ %244, %246 ]
  %264 = phi <5 x i32> [ %270, %261 ], [ %245, %246 ]
  %265 = phi <128 x i16> [ %275, %261 ], [ %240, %246 ]
  %266 = phi <128 x i16> [ %276, %261 ], [ %241, %246 ]
  %267 = phi i32 [ %279, %261 ], [ %26, %246 ]
  %268 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %262, i32 2, i32 %267, i32 0, i1 %70, i1 true)
  %269 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %263, i32 2, i32 %267, i32 0, i1 %71, i1 true)
  %270 = insertelement <5 x i32> %264, i32 %267, i32 1
  %271 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %268, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %272 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %269, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %273 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %271, i32 0, <128 x i16> %265, i1 %56, i1 false)
  %274 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %272, i32 0, <128 x i16> %266, i1 %57, i1 false)
  %275 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %268, i8 0, i32 0, <128 x i16> %273, i1 %73, i1 true)
  %276 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %269, i8 1, i32 0, <128 x i16> %274, i1 %75, i1 true)
  %277 = tail call <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16> %275, <256 x i8> %45, i8 7, i32 2, <128 x i16> %275, i1 %39, i1 true)
  %278 = tail call <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16> %277, <128 x i16> %276, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %270, i8 2, <128 x i16> %278, i32 0, i1 true, i1 false)
  %279 = add i32 %267, 1
  %280 = icmp eq i32 %279, %28
  br i1 %280, label %247, label %261, !llvm.loop !14

281:                                              ; preds = %178
  br label %283

282:                                              ; preds = %87
  br label %283

283:                                              ; preds = %282, %281, %48, %54
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
declare <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16>, <256 x i8>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, i8, i32, <128 x i16>, i1, i1) #1

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
