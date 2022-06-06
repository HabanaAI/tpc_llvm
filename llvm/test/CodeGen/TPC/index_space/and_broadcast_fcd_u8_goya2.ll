; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*256, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*256, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*256, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*256, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/and_broadcast_fcd_u8_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/and_broadcast_fcd_u8_goya2.c"
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
  %38 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %39 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %40 = icmp ne i8 %12, 0
  %41 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %38, i32 0, <256 x i8> zeroinitializer, i1 %40, i1 false)
  %42 = icmp ne i8 %13, 0
  %43 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %39, i32 0, <256 x i8> zeroinitializer, i1 %42, i1 false)
  %44 = icmp slt i32 %21, %23
  br i1 %37, label %51, label %45

45:                                               ; preds = %16
  br i1 %44, label %46, label %280

46:                                               ; preds = %45
  %47 = icmp slt i32 %32, %33
  %48 = icmp slt i32 %30, %31
  %49 = icmp slt i32 %28, %29
  %50 = icmp slt i32 %25, %27
  br label %73

51:                                               ; preds = %16
  br i1 %44, label %52, label %280

52:                                               ; preds = %51
  %53 = icmp ne i8 %2, 0
  %54 = icmp ne i8 %7, 0
  %55 = icmp ne i8 %14, 0
  %56 = icmp ne i8 %15, 0
  %57 = icmp slt i32 %32, %33
  %58 = icmp ne i8 %6, 0
  %59 = icmp ne i8 %11, 0
  %60 = icmp slt i32 %30, %31
  %61 = icmp ne i8 %5, 0
  %62 = icmp ne i8 %10, 0
  %63 = icmp slt i32 %28, %29
  %64 = icmp ne i8 %4, 0
  %65 = icmp ne i8 %9, 0
  %66 = icmp slt i32 %25, %27
  %67 = icmp ne i8 %3, 0
  %68 = icmp ne i8 %8, 0
  %69 = or i8 %14, %2
  %70 = icmp ne i8 %69, 0
  %71 = or i8 %15, %7
  %72 = icmp ne i8 %71, 0
  br label %154

73:                                               ; preds = %46, %84
  %74 = phi <5 x i32> [ zeroinitializer, %46 ], [ %87, %84 ]
  %75 = phi i32 [ %21, %46 ], [ %88, %84 ]
  %76 = phi <256 x i8> [ %43, %46 ], [ %86, %84 ]
  %77 = phi <256 x i8> [ %41, %46 ], [ %85, %84 ]
  %78 = insertelement <5 x i32> %74, i32 %75, i32 0
  br i1 %47, label %79, label %84

79:                                               ; preds = %73
  br label %90

80:                                               ; preds = %101
  %81 = phi <256 x i8> [ %102, %101 ]
  %82 = phi <256 x i8> [ %103, %101 ]
  %83 = phi <5 x i32> [ %104, %101 ]
  br label %84

84:                                               ; preds = %80, %73
  %85 = phi <256 x i8> [ %77, %73 ], [ %81, %80 ]
  %86 = phi <256 x i8> [ %76, %73 ], [ %82, %80 ]
  %87 = phi <5 x i32> [ %78, %73 ], [ %83, %80 ]
  %88 = add nsw i32 %75, 256
  %89 = icmp slt i32 %88, %23
  br i1 %89, label %73, label %279, !llvm.loop !3

90:                                               ; preds = %79, %101
  %91 = phi <5 x i32> [ %104, %101 ], [ %78, %79 ]
  %92 = phi i32 [ %105, %101 ], [ %32, %79 ]
  %93 = phi <256 x i8> [ %103, %101 ], [ %76, %79 ]
  %94 = phi <256 x i8> [ %102, %101 ], [ %77, %79 ]
  %95 = insertelement <5 x i32> %91, i32 %92, i32 4
  br i1 %48, label %96, label %101

96:                                               ; preds = %90
  br label %107

97:                                               ; preds = %118
  %98 = phi <256 x i8> [ %119, %118 ]
  %99 = phi <256 x i8> [ %120, %118 ]
  %100 = phi <5 x i32> [ %121, %118 ]
  br label %101

101:                                              ; preds = %97, %90
  %102 = phi <256 x i8> [ %94, %90 ], [ %98, %97 ]
  %103 = phi <256 x i8> [ %93, %90 ], [ %99, %97 ]
  %104 = phi <5 x i32> [ %95, %90 ], [ %100, %97 ]
  %105 = add i32 %92, 1
  %106 = icmp eq i32 %105, %33
  br i1 %106, label %80, label %90, !llvm.loop !5

107:                                              ; preds = %96, %118
  %108 = phi <5 x i32> [ %121, %118 ], [ %95, %96 ]
  %109 = phi i32 [ %122, %118 ], [ %30, %96 ]
  %110 = phi <256 x i8> [ %120, %118 ], [ %93, %96 ]
  %111 = phi <256 x i8> [ %119, %118 ], [ %94, %96 ]
  %112 = insertelement <5 x i32> %108, i32 %109, i32 3
  br i1 %49, label %113, label %118

113:                                              ; preds = %107
  br label %124

114:                                              ; preds = %136
  %115 = phi <256 x i8> [ %137, %136 ]
  %116 = phi <256 x i8> [ %138, %136 ]
  %117 = phi <5 x i32> [ %139, %136 ]
  br label %118

118:                                              ; preds = %114, %107
  %119 = phi <256 x i8> [ %111, %107 ], [ %115, %114 ]
  %120 = phi <256 x i8> [ %110, %107 ], [ %116, %114 ]
  %121 = phi <5 x i32> [ %112, %107 ], [ %117, %114 ]
  %122 = add i32 %109, 1
  %123 = icmp eq i32 %122, %31
  br i1 %123, label %97, label %107, !llvm.loop !6

124:                                              ; preds = %113, %136
  %125 = phi <5 x i32> [ %139, %136 ], [ %112, %113 ]
  %126 = phi i32 [ %140, %136 ], [ %28, %113 ]
  %127 = phi <256 x i8> [ %138, %136 ], [ %110, %113 ]
  %128 = phi <256 x i8> [ %137, %136 ], [ %111, %113 ]
  %129 = insertelement <5 x i32> %125, i32 %126, i32 2
  %130 = insertelement <5 x i32> %129, i32 %25, i32 1
  br i1 %50, label %131, label %136

131:                                              ; preds = %124
  br label %142

132:                                              ; preds = %142
  %133 = phi <256 x i8> [ %147, %142 ]
  %134 = phi <256 x i8> [ %148, %142 ]
  %135 = phi <5 x i32> [ %151, %142 ]
  br label %136

136:                                              ; preds = %132, %124
  %137 = phi <256 x i8> [ %128, %124 ], [ %133, %132 ]
  %138 = phi <256 x i8> [ %127, %124 ], [ %134, %132 ]
  %139 = phi <5 x i32> [ %130, %124 ], [ %135, %132 ]
  %140 = add i32 %126, 1
  %141 = icmp eq i32 %140, %29
  br i1 %141, label %114, label %124, !llvm.loop !7

142:                                              ; preds = %131, %142
  %143 = phi <5 x i32> [ %151, %142 ], [ %130, %131 ]
  %144 = phi i32 [ %152, %142 ], [ %25, %131 ]
  %145 = phi <256 x i8> [ %148, %142 ], [ %127, %131 ]
  %146 = phi <256 x i8> [ %147, %142 ], [ %128, %131 ]
  %147 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %143, i8 0, i32 0, <256 x i8> %146, i1 %42, i1 false)
  %148 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %143, i8 1, i32 0, <256 x i8> %145, i1 %40, i1 false)
  %149 = tail call <256 x i1> @llvm.tpc.cmp.neq.v256i1.v256i8.v256i8.i1(<256 x i8> %148, <256 x i8> %35, i8 5, i32 0, <256 x i1> undef, i1 true, i1 false)
  %150 = tail call <256 x i8> @llvm.tpc.sel.neq.v256i8.v256i8.v256i8.v256i8.v256i8.v256i1(<256 x i8> %147, <256 x i8> %35, <256 x i8> %34, <256 x i8> %35, i8 5, i32 0, <256 x i8> %35, <256 x i1> %149, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %143, i8 2, <256 x i8> %150, i32 0, i1 true, i1 false)
  %151 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %143, i32 1, i32 2, i8 2, i32 0, <5 x i32> %143, i1 true, i1 false)
  %152 = add i32 %144, 1
  %153 = icmp eq i32 %152, %27
  br i1 %153, label %132, label %142, !llvm.loop !8

154:                                              ; preds = %52, %175
  %155 = phi <5 x i32> [ zeroinitializer, %52 ], [ %180, %175 ]
  %156 = phi <5 x i32> [ zeroinitializer, %52 ], [ %179, %175 ]
  %157 = phi <5 x i32> [ zeroinitializer, %52 ], [ %178, %175 ]
  %158 = phi <256 x i8> [ %41, %52 ], [ %177, %175 ]
  %159 = phi i32 [ %21, %52 ], [ %181, %175 ]
  %160 = phi <256 x i8> [ %43, %52 ], [ %176, %175 ]
  %161 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %155, i32 1, i32 %159, i32 0, i1 %53, i1 true)
  %162 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %156, i32 1, i32 %159, i32 0, i1 %54, i1 true)
  %163 = insertelement <5 x i32> %157, i32 %159, i32 0
  %164 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %161, i32 2, i32 0, i32 0, i1 %55, i1 false)
  %165 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %162, i32 2, i32 0, i32 0, i1 %56, i1 false)
  %166 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %164, i8 0, i32 0, <256 x i8> %158, i1 %55, i1 false)
  %167 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %165, i8 1, i32 0, <256 x i8> %160, i1 %56, i1 false)
  br i1 %57, label %168, label %175

168:                                              ; preds = %154
  br label %183

169:                                              ; preds = %200
  %170 = phi <256 x i8> [ %201, %200 ]
  %171 = phi <256 x i8> [ %202, %200 ]
  %172 = phi <5 x i32> [ %203, %200 ]
  %173 = phi <5 x i32> [ %204, %200 ]
  %174 = phi <5 x i32> [ %205, %200 ]
  br label %175

175:                                              ; preds = %169, %154
  %176 = phi <256 x i8> [ %167, %154 ], [ %170, %169 ]
  %177 = phi <256 x i8> [ %166, %154 ], [ %171, %169 ]
  %178 = phi <5 x i32> [ %163, %154 ], [ %172, %169 ]
  %179 = phi <5 x i32> [ %165, %154 ], [ %173, %169 ]
  %180 = phi <5 x i32> [ %164, %154 ], [ %174, %169 ]
  %181 = add nsw i32 %159, 256
  %182 = icmp slt i32 %181, %23
  br i1 %182, label %154, label %278, !llvm.loop !10

183:                                              ; preds = %168, %200
  %184 = phi <5 x i32> [ %205, %200 ], [ %164, %168 ]
  %185 = phi <5 x i32> [ %204, %200 ], [ %165, %168 ]
  %186 = phi <5 x i32> [ %203, %200 ], [ %163, %168 ]
  %187 = phi <256 x i8> [ %202, %200 ], [ %166, %168 ]
  %188 = phi i32 [ %206, %200 ], [ %32, %168 ]
  %189 = phi <256 x i8> [ %201, %200 ], [ %167, %168 ]
  %190 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %184, i32 16, i32 %188, i32 0, i1 %58, i1 true)
  %191 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %185, i32 16, i32 %188, i32 0, i1 %59, i1 true)
  %192 = insertelement <5 x i32> %186, i32 %188, i32 4
  br i1 %60, label %193, label %200

193:                                              ; preds = %183
  br label %208

194:                                              ; preds = %225
  %195 = phi <256 x i8> [ %226, %225 ]
  %196 = phi <256 x i8> [ %227, %225 ]
  %197 = phi <5 x i32> [ %228, %225 ]
  %198 = phi <5 x i32> [ %229, %225 ]
  %199 = phi <5 x i32> [ %230, %225 ]
  br label %200

200:                                              ; preds = %194, %183
  %201 = phi <256 x i8> [ %189, %183 ], [ %195, %194 ]
  %202 = phi <256 x i8> [ %187, %183 ], [ %196, %194 ]
  %203 = phi <5 x i32> [ %192, %183 ], [ %197, %194 ]
  %204 = phi <5 x i32> [ %191, %183 ], [ %198, %194 ]
  %205 = phi <5 x i32> [ %190, %183 ], [ %199, %194 ]
  %206 = add i32 %188, 1
  %207 = icmp eq i32 %206, %33
  br i1 %207, label %169, label %183, !llvm.loop !11

208:                                              ; preds = %193, %225
  %209 = phi <5 x i32> [ %230, %225 ], [ %190, %193 ]
  %210 = phi <5 x i32> [ %229, %225 ], [ %191, %193 ]
  %211 = phi <5 x i32> [ %228, %225 ], [ %192, %193 ]
  %212 = phi <256 x i8> [ %227, %225 ], [ %187, %193 ]
  %213 = phi i32 [ %231, %225 ], [ %30, %193 ]
  %214 = phi <256 x i8> [ %226, %225 ], [ %189, %193 ]
  %215 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %209, i32 8, i32 %213, i32 0, i1 %61, i1 true)
  %216 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %210, i32 8, i32 %213, i32 0, i1 %62, i1 true)
  %217 = insertelement <5 x i32> %211, i32 %213, i32 3
  br i1 %63, label %218, label %225

218:                                              ; preds = %208
  br label %233

219:                                              ; preds = %250
  %220 = phi <256 x i8> [ %251, %250 ]
  %221 = phi <256 x i8> [ %252, %250 ]
  %222 = phi <5 x i32> [ %253, %250 ]
  %223 = phi <5 x i32> [ %254, %250 ]
  %224 = phi <5 x i32> [ %255, %250 ]
  br label %225

225:                                              ; preds = %219, %208
  %226 = phi <256 x i8> [ %214, %208 ], [ %220, %219 ]
  %227 = phi <256 x i8> [ %212, %208 ], [ %221, %219 ]
  %228 = phi <5 x i32> [ %217, %208 ], [ %222, %219 ]
  %229 = phi <5 x i32> [ %216, %208 ], [ %223, %219 ]
  %230 = phi <5 x i32> [ %215, %208 ], [ %224, %219 ]
  %231 = add i32 %213, 1
  %232 = icmp eq i32 %231, %31
  br i1 %232, label %194, label %208, !llvm.loop !12

233:                                              ; preds = %218, %250
  %234 = phi <5 x i32> [ %255, %250 ], [ %215, %218 ]
  %235 = phi <5 x i32> [ %254, %250 ], [ %216, %218 ]
  %236 = phi <5 x i32> [ %253, %250 ], [ %217, %218 ]
  %237 = phi <256 x i8> [ %252, %250 ], [ %212, %218 ]
  %238 = phi i32 [ %256, %250 ], [ %28, %218 ]
  %239 = phi <256 x i8> [ %251, %250 ], [ %214, %218 ]
  %240 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %234, i32 4, i32 %238, i32 0, i1 %64, i1 true)
  %241 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %235, i32 4, i32 %238, i32 0, i1 %65, i1 true)
  %242 = insertelement <5 x i32> %236, i32 %238, i32 2
  br i1 %66, label %243, label %250

243:                                              ; preds = %233
  br label %258

244:                                              ; preds = %258
  %245 = phi <5 x i32> [ %265, %258 ]
  %246 = phi <5 x i32> [ %266, %258 ]
  %247 = phi <5 x i32> [ %267, %258 ]
  %248 = phi <256 x i8> [ %272, %258 ]
  %249 = phi <256 x i8> [ %273, %258 ]
  br label %250

250:                                              ; preds = %244, %233
  %251 = phi <256 x i8> [ %239, %233 ], [ %249, %244 ]
  %252 = phi <256 x i8> [ %237, %233 ], [ %248, %244 ]
  %253 = phi <5 x i32> [ %242, %233 ], [ %247, %244 ]
  %254 = phi <5 x i32> [ %241, %233 ], [ %246, %244 ]
  %255 = phi <5 x i32> [ %240, %233 ], [ %245, %244 ]
  %256 = add i32 %238, 1
  %257 = icmp eq i32 %256, %29
  br i1 %257, label %219, label %233, !llvm.loop !13

258:                                              ; preds = %243, %258
  %259 = phi <5 x i32> [ %265, %258 ], [ %240, %243 ]
  %260 = phi <5 x i32> [ %266, %258 ], [ %241, %243 ]
  %261 = phi <5 x i32> [ %267, %258 ], [ %242, %243 ]
  %262 = phi <256 x i8> [ %272, %258 ], [ %237, %243 ]
  %263 = phi i32 [ %276, %258 ], [ %25, %243 ]
  %264 = phi <256 x i8> [ %273, %258 ], [ %239, %243 ]
  %265 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %259, i32 2, i32 %263, i32 0, i1 %67, i1 true)
  %266 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %260, i32 2, i32 %263, i32 0, i1 %68, i1 true)
  %267 = insertelement <5 x i32> %261, i32 %263, i32 1
  %268 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %265, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %269 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %266, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %270 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %268, i32 0, <256 x i8> %262, i1 %53, i1 false)
  %271 = tail call <256 x i8> @llvm.tpc.ld.g.v256i8.i1(i8 addrspace(3)* %269, i32 0, <256 x i8> %264, i1 %54, i1 false)
  %272 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %265, i8 0, i32 0, <256 x i8> %270, i1 %70, i1 true)
  %273 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %266, i8 1, i32 0, <256 x i8> %271, i1 %72, i1 true)
  %274 = tail call <256 x i1> @llvm.tpc.cmp.neq.v256i1.v256i8.v256i8.i1(<256 x i8> %273, <256 x i8> %35, i8 5, i32 0, <256 x i1> undef, i1 true, i1 false)
  %275 = tail call <256 x i8> @llvm.tpc.sel.neq.v256i8.v256i8.v256i8.v256i8.v256i8.v256i1(<256 x i8> %272, <256 x i8> %35, <256 x i8> %34, <256 x i8> %35, i8 5, i32 0, <256 x i8> %35, <256 x i1> %274, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %267, i8 2, <256 x i8> %275, i32 0, i1 true, i1 false)
  %276 = add i32 %263, 1
  %277 = icmp eq i32 %276, %27
  br i1 %277, label %244, label %258, !llvm.loop !14

278:                                              ; preds = %175
  br label %280

279:                                              ; preds = %84
  br label %280

280:                                              ; preds = %279, %278, %45, %51
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
