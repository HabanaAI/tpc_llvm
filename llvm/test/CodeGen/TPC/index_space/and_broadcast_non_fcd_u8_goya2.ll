; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*256, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*256, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*256, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*256, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/and_broadcast_non_fcd_u8_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/and_broadcast_non_fcd_u8_goya2.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main(i8 signext %0, i8 signext %1, i8 signext %2, i8 signext %3, i8 signext %4, i8 signext %5, i8 signext %6, i8 signext %7, i8 signext %8, i8 signext %9, i8 signext %10, i8 signext %11, i8 signext %12, i8 signext %13, i8 signext %14, i8 signext %15) local_unnamed_addr #0 {
  %17 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %18 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %19 = add <5 x i32> %18, %17
  %20 = extractelement <5 x i32> %17, i32 0
  %21 = shl nsw i32 %20, 8
  %22 = extractelement <5 x i32> %19, i32 0
  %23 = shl nsw i32 %22, 8
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
  %34 = tail call <256 x i8> @llvm.tpc.mov.v256i8.i8.i1(i8 1, i8 5, i32 0, <256 x i8> undef, i1 true, i1 false)
  %35 = tail call <256 x i8> @llvm.tpc.mov.v256i8.i8.i1(i8 0, i8 5, i32 0, <256 x i8> undef, i1 true, i1 false)
  %36 = or i8 %13, %12
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %38, label %61

38:                                               ; preds = %16
  %39 = icmp slt i32 %21, %23
  br i1 %39, label %40, label %277

40:                                               ; preds = %38
  %41 = icmp ne i8 %2, 0
  %42 = icmp ne i8 %7, 0
  %43 = icmp ne i8 %14, 0
  %44 = icmp ne i8 %15, 0
  %45 = icmp slt i32 %32, %33
  %46 = icmp ne i8 %6, 0
  %47 = icmp ne i8 %11, 0
  %48 = icmp slt i32 %30, %31
  %49 = icmp ne i8 %5, 0
  %50 = icmp ne i8 %10, 0
  %51 = icmp slt i32 %28, %29
  %52 = icmp ne i8 %4, 0
  %53 = icmp ne i8 %9, 0
  %54 = icmp slt i32 %25, %27
  %55 = icmp ne i8 %3, 0
  %56 = icmp ne i8 %8, 0
  %57 = or i8 %14, %2
  %58 = icmp ne i8 %57, 0
  %59 = or i8 %15, %7
  %60 = icmp ne i8 %59, 0
  br label %155

61:                                               ; preds = %16
  %62 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %63 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %64 = icmp ne i8 %12, 0
  %65 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %62, i32 0, <256 x i8> zeroinitializer, i1 %64, i1 false)
  %66 = icmp ne i8 %13, 0
  %67 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %63, i32 0, <256 x i8> zeroinitializer, i1 %66, i1 false)
  %68 = icmp slt i32 %21, %23
  br i1 %68, label %69, label %277

69:                                               ; preds = %61
  %70 = icmp slt i32 %32, %33
  %71 = icmp slt i32 %30, %31
  %72 = icmp slt i32 %28, %29
  %73 = icmp slt i32 %25, %27
  br label %74

74:                                               ; preds = %69, %85
  %75 = phi i32 [ %21, %69 ], [ %89, %85 ]
  %76 = phi <256 x i8> [ %67, %69 ], [ %88, %85 ]
  %77 = phi <256 x i8> [ %65, %69 ], [ %87, %85 ]
  %78 = phi <5 x i32> [ zeroinitializer, %69 ], [ %86, %85 ]
  %79 = insertelement <5 x i32> %78, i32 %75, i32 0
  br i1 %70, label %80, label %85

80:                                               ; preds = %74
  br label %91

81:                                               ; preds = %102
  %82 = phi <5 x i32> [ %103, %102 ]
  %83 = phi <256 x i8> [ %104, %102 ]
  %84 = phi <256 x i8> [ %105, %102 ]
  br label %85

85:                                               ; preds = %81, %74
  %86 = phi <5 x i32> [ %79, %74 ], [ %82, %81 ]
  %87 = phi <256 x i8> [ %77, %74 ], [ %83, %81 ]
  %88 = phi <256 x i8> [ %76, %74 ], [ %84, %81 ]
  %89 = add nsw i32 %75, 256
  %90 = icmp slt i32 %89, %23
  br i1 %90, label %74, label %276, !llvm.loop !3

91:                                               ; preds = %80, %102
  %92 = phi i32 [ %106, %102 ], [ %32, %80 ]
  %93 = phi <256 x i8> [ %105, %102 ], [ %76, %80 ]
  %94 = phi <256 x i8> [ %104, %102 ], [ %77, %80 ]
  %95 = phi <5 x i32> [ %103, %102 ], [ %79, %80 ]
  %96 = insertelement <5 x i32> %95, i32 %92, i32 4
  br i1 %71, label %97, label %102

97:                                               ; preds = %91
  br label %108

98:                                               ; preds = %119
  %99 = phi <5 x i32> [ %120, %119 ]
  %100 = phi <256 x i8> [ %121, %119 ]
  %101 = phi <256 x i8> [ %122, %119 ]
  br label %102

102:                                              ; preds = %98, %91
  %103 = phi <5 x i32> [ %96, %91 ], [ %99, %98 ]
  %104 = phi <256 x i8> [ %94, %91 ], [ %100, %98 ]
  %105 = phi <256 x i8> [ %93, %91 ], [ %101, %98 ]
  %106 = add i32 %92, 1
  %107 = icmp eq i32 %106, %33
  br i1 %107, label %81, label %91, !llvm.loop !5

108:                                              ; preds = %97, %119
  %109 = phi i32 [ %123, %119 ], [ %30, %97 ]
  %110 = phi <256 x i8> [ %122, %119 ], [ %93, %97 ]
  %111 = phi <256 x i8> [ %121, %119 ], [ %94, %97 ]
  %112 = phi <5 x i32> [ %120, %119 ], [ %96, %97 ]
  %113 = insertelement <5 x i32> %112, i32 %109, i32 3
  br i1 %72, label %114, label %119

114:                                              ; preds = %108
  br label %125

115:                                              ; preds = %137
  %116 = phi <5 x i32> [ %138, %137 ]
  %117 = phi <256 x i8> [ %139, %137 ]
  %118 = phi <256 x i8> [ %140, %137 ]
  br label %119

119:                                              ; preds = %115, %108
  %120 = phi <5 x i32> [ %113, %108 ], [ %116, %115 ]
  %121 = phi <256 x i8> [ %111, %108 ], [ %117, %115 ]
  %122 = phi <256 x i8> [ %110, %108 ], [ %118, %115 ]
  %123 = add i32 %109, 1
  %124 = icmp eq i32 %123, %31
  br i1 %124, label %98, label %108, !llvm.loop !6

125:                                              ; preds = %114, %137
  %126 = phi i32 [ %141, %137 ], [ %28, %114 ]
  %127 = phi <256 x i8> [ %140, %137 ], [ %110, %114 ]
  %128 = phi <256 x i8> [ %139, %137 ], [ %111, %114 ]
  %129 = phi <5 x i32> [ %138, %137 ], [ %113, %114 ]
  %130 = insertelement <5 x i32> %129, i32 %126, i32 2
  %131 = insertelement <5 x i32> %130, i32 %25, i32 1
  br i1 %73, label %132, label %137

132:                                              ; preds = %125
  br label %143

133:                                              ; preds = %143
  %134 = phi <256 x i8> [ %148, %143 ]
  %135 = phi <256 x i8> [ %149, %143 ]
  %136 = phi <5 x i32> [ %152, %143 ]
  br label %137

137:                                              ; preds = %133, %125
  %138 = phi <5 x i32> [ %131, %125 ], [ %136, %133 ]
  %139 = phi <256 x i8> [ %128, %125 ], [ %134, %133 ]
  %140 = phi <256 x i8> [ %127, %125 ], [ %135, %133 ]
  %141 = add i32 %126, 1
  %142 = icmp eq i32 %141, %29
  br i1 %142, label %115, label %125, !llvm.loop !7

143:                                              ; preds = %132, %143
  %144 = phi i32 [ %153, %143 ], [ %25, %132 ]
  %145 = phi <256 x i8> [ %149, %143 ], [ %127, %132 ]
  %146 = phi <256 x i8> [ %148, %143 ], [ %128, %132 ]
  %147 = phi <5 x i32> [ %152, %143 ], [ %131, %132 ]
  %148 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %147, i8 0, i32 0, <256 x i8> %146, i1 %66, i1 false)
  %149 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %147, i8 1, i32 0, <256 x i8> %145, i1 %64, i1 false)
  %150 = tail call <256 x i1> @llvm.tpc.cmp.neq.v256i1.v256i8.v256i8.i1(<256 x i8> %149, <256 x i8> %35, i8 5, i32 0, <256 x i1> undef, i1 true, i1 false)
  %151 = tail call <256 x i8> @llvm.tpc.sel.neq.v256i8.v256i8.v256i8.v256i8.v256i8.v256i1(<256 x i8> %148, <256 x i8> %35, <256 x i8> %34, <256 x i8> %35, i8 5, i32 0, <256 x i8> %35, <256 x i1> %150, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %147, i8 2, <256 x i8> %151, i32 0, i1 true, i1 false)
  %152 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %147, i32 1, i32 2, i8 2, i32 0, <5 x i32> %147, i1 true, i1 false)
  %153 = add i32 %144, 1
  %154 = icmp eq i32 %153, %27
  br i1 %154, label %133, label %143, !llvm.loop !8

155:                                              ; preds = %40, %176
  %156 = phi <5 x i32> [ zeroinitializer, %40 ], [ %181, %176 ]
  %157 = phi <5 x i32> [ zeroinitializer, %40 ], [ %180, %176 ]
  %158 = phi i32 [ %21, %40 ], [ %182, %176 ]
  %159 = phi <5 x i32> [ zeroinitializer, %40 ], [ %179, %176 ]
  %160 = phi <256 x i8> [ zeroinitializer, %40 ], [ %178, %176 ]
  %161 = phi <256 x i8> [ zeroinitializer, %40 ], [ %177, %176 ]
  %162 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %156, i32 1, i32 %158, i32 0, i1 %41, i1 true)
  %163 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %157, i32 1, i32 %158, i32 0, i1 %42, i1 true)
  %164 = insertelement <5 x i32> %159, i32 %158, i32 0
  %165 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %162, i32 2, i32 0, i32 0, i1 %43, i1 false)
  %166 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %163, i32 2, i32 0, i32 0, i1 %44, i1 false)
  %167 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %165, i8 0, i32 0, <256 x i8> %160, i1 %43, i1 false)
  %168 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %166, i8 1, i32 0, <256 x i8> %161, i1 %44, i1 false)
  br i1 %45, label %169, label %176

169:                                              ; preds = %155
  br label %184

170:                                              ; preds = %201
  %171 = phi <256 x i8> [ %202, %201 ]
  %172 = phi <256 x i8> [ %203, %201 ]
  %173 = phi <5 x i32> [ %204, %201 ]
  %174 = phi <5 x i32> [ %205, %201 ]
  %175 = phi <5 x i32> [ %206, %201 ]
  br label %176

176:                                              ; preds = %170, %155
  %177 = phi <256 x i8> [ %168, %155 ], [ %171, %170 ]
  %178 = phi <256 x i8> [ %167, %155 ], [ %172, %170 ]
  %179 = phi <5 x i32> [ %164, %155 ], [ %173, %170 ]
  %180 = phi <5 x i32> [ %166, %155 ], [ %174, %170 ]
  %181 = phi <5 x i32> [ %165, %155 ], [ %175, %170 ]
  %182 = add nsw i32 %158, 256
  %183 = icmp slt i32 %182, %23
  br i1 %183, label %155, label %275, !llvm.loop !10

184:                                              ; preds = %169, %201
  %185 = phi <5 x i32> [ %206, %201 ], [ %165, %169 ]
  %186 = phi <5 x i32> [ %205, %201 ], [ %166, %169 ]
  %187 = phi i32 [ %207, %201 ], [ %32, %169 ]
  %188 = phi <5 x i32> [ %204, %201 ], [ %164, %169 ]
  %189 = phi <256 x i8> [ %203, %201 ], [ %167, %169 ]
  %190 = phi <256 x i8> [ %202, %201 ], [ %168, %169 ]
  %191 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %185, i32 16, i32 %187, i32 0, i1 %46, i1 true)
  %192 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %186, i32 16, i32 %187, i32 0, i1 %47, i1 true)
  %193 = insertelement <5 x i32> %188, i32 %187, i32 4
  br i1 %48, label %194, label %201

194:                                              ; preds = %184
  br label %209

195:                                              ; preds = %226
  %196 = phi <256 x i8> [ %227, %226 ]
  %197 = phi <256 x i8> [ %228, %226 ]
  %198 = phi <5 x i32> [ %229, %226 ]
  %199 = phi <5 x i32> [ %230, %226 ]
  %200 = phi <5 x i32> [ %231, %226 ]
  br label %201

201:                                              ; preds = %195, %184
  %202 = phi <256 x i8> [ %190, %184 ], [ %196, %195 ]
  %203 = phi <256 x i8> [ %189, %184 ], [ %197, %195 ]
  %204 = phi <5 x i32> [ %193, %184 ], [ %198, %195 ]
  %205 = phi <5 x i32> [ %192, %184 ], [ %199, %195 ]
  %206 = phi <5 x i32> [ %191, %184 ], [ %200, %195 ]
  %207 = add i32 %187, 1
  %208 = icmp eq i32 %207, %33
  br i1 %208, label %170, label %184, !llvm.loop !11

209:                                              ; preds = %194, %226
  %210 = phi <5 x i32> [ %231, %226 ], [ %191, %194 ]
  %211 = phi <5 x i32> [ %230, %226 ], [ %192, %194 ]
  %212 = phi i32 [ %232, %226 ], [ %30, %194 ]
  %213 = phi <5 x i32> [ %229, %226 ], [ %193, %194 ]
  %214 = phi <256 x i8> [ %228, %226 ], [ %189, %194 ]
  %215 = phi <256 x i8> [ %227, %226 ], [ %190, %194 ]
  %216 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %210, i32 8, i32 %212, i32 0, i1 %49, i1 true)
  %217 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %211, i32 8, i32 %212, i32 0, i1 %50, i1 true)
  %218 = insertelement <5 x i32> %213, i32 %212, i32 3
  br i1 %51, label %219, label %226

219:                                              ; preds = %209
  br label %234

220:                                              ; preds = %251
  %221 = phi <256 x i8> [ %252, %251 ]
  %222 = phi <256 x i8> [ %253, %251 ]
  %223 = phi <5 x i32> [ %254, %251 ]
  %224 = phi <5 x i32> [ %255, %251 ]
  %225 = phi <5 x i32> [ %256, %251 ]
  br label %226

226:                                              ; preds = %220, %209
  %227 = phi <256 x i8> [ %215, %209 ], [ %221, %220 ]
  %228 = phi <256 x i8> [ %214, %209 ], [ %222, %220 ]
  %229 = phi <5 x i32> [ %218, %209 ], [ %223, %220 ]
  %230 = phi <5 x i32> [ %217, %209 ], [ %224, %220 ]
  %231 = phi <5 x i32> [ %216, %209 ], [ %225, %220 ]
  %232 = add i32 %212, 1
  %233 = icmp eq i32 %232, %31
  br i1 %233, label %195, label %209, !llvm.loop !12

234:                                              ; preds = %219, %251
  %235 = phi <5 x i32> [ %256, %251 ], [ %216, %219 ]
  %236 = phi <5 x i32> [ %255, %251 ], [ %217, %219 ]
  %237 = phi i32 [ %257, %251 ], [ %28, %219 ]
  %238 = phi <5 x i32> [ %254, %251 ], [ %218, %219 ]
  %239 = phi <256 x i8> [ %253, %251 ], [ %214, %219 ]
  %240 = phi <256 x i8> [ %252, %251 ], [ %215, %219 ]
  %241 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %235, i32 4, i32 %237, i32 0, i1 %52, i1 true)
  %242 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %236, i32 4, i32 %237, i32 0, i1 %53, i1 true)
  %243 = insertelement <5 x i32> %238, i32 %237, i32 2
  br i1 %54, label %244, label %251

244:                                              ; preds = %234
  br label %259

245:                                              ; preds = %259
  %246 = phi <5 x i32> [ %266, %259 ]
  %247 = phi <5 x i32> [ %267, %259 ]
  %248 = phi <5 x i32> [ %268, %259 ]
  %249 = phi <256 x i8> [ %269, %259 ]
  %250 = phi <256 x i8> [ %270, %259 ]
  br label %251

251:                                              ; preds = %245, %234
  %252 = phi <256 x i8> [ %240, %234 ], [ %250, %245 ]
  %253 = phi <256 x i8> [ %239, %234 ], [ %249, %245 ]
  %254 = phi <5 x i32> [ %243, %234 ], [ %248, %245 ]
  %255 = phi <5 x i32> [ %242, %234 ], [ %247, %245 ]
  %256 = phi <5 x i32> [ %241, %234 ], [ %246, %245 ]
  %257 = add i32 %237, 1
  %258 = icmp eq i32 %257, %29
  br i1 %258, label %220, label %234, !llvm.loop !13

259:                                              ; preds = %244, %259
  %260 = phi <5 x i32> [ %266, %259 ], [ %241, %244 ]
  %261 = phi <5 x i32> [ %267, %259 ], [ %242, %244 ]
  %262 = phi i32 [ %273, %259 ], [ %25, %244 ]
  %263 = phi <5 x i32> [ %268, %259 ], [ %243, %244 ]
  %264 = phi <256 x i8> [ %269, %259 ], [ %239, %244 ]
  %265 = phi <256 x i8> [ %270, %259 ], [ %240, %244 ]
  %266 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %260, i32 2, i32 %262, i32 0, i1 %55, i1 true)
  %267 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %261, i32 2, i32 %262, i32 0, i1 %56, i1 true)
  %268 = insertelement <5 x i32> %263, i32 %262, i32 1
  %269 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %266, i8 0, i32 0, <256 x i8> %264, i1 %58, i1 true)
  %270 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %267, i8 1, i32 0, <256 x i8> %265, i1 %60, i1 true)
  %271 = tail call <256 x i1> @llvm.tpc.cmp.neq.v256i1.v256i8.v256i8.i1(<256 x i8> %270, <256 x i8> %35, i8 5, i32 0, <256 x i1> undef, i1 true, i1 false)
  %272 = tail call <256 x i8> @llvm.tpc.sel.neq.v256i8.v256i8.v256i8.v256i8.v256i8.v256i1(<256 x i8> %269, <256 x i8> %35, <256 x i8> %34, <256 x i8> %35, i8 5, i32 0, <256 x i8> %35, <256 x i1> %271, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %268, i8 2, <256 x i8> %272, i32 0, i1 true, i1 false)
  %273 = add i32 %262, 1
  %274 = icmp eq i32 %273, %27
  br i1 %274, label %245, label %259, !llvm.loop !14

275:                                              ; preds = %176
  br label %277

276:                                              ; preds = %85
  br label %277

277:                                              ; preds = %276, %275, %61, %38
  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.mov.v256i8.i8.i1(i8, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #1

; Function Attrs: nounwind readonly
declare <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)*, i32, <256 x i8>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i1> @llvm.tpc.cmp.neq.v256i1.v256i8.v256i8.i1(<256 x i8>, <256 x i8>, i8, i32, <256 x i1>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.sel.neq.v256i8.v256i8.v256i8.v256i8.v256i8.v256i1(<256 x i8>, <256 x i8>, <256 x i8>, <256 x i8>, i8, i32, <256 x i8>, <256 x i1>, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #3

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
