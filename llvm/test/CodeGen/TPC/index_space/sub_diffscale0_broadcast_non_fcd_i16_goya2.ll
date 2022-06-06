; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%3)*128, not(%4)*4, not(%5)*1, not(%6)*1, not(%7)*1 }[1].[Input].{ not(%8)*128, not(%9)*4, not(%10)*1, not(%11)*1, not(%12)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%3)*128, not(%4)*4, not(%5)*1, not(%6)*1, not(%7)*1 }[1].[Input].{ not(%8)*128, not(%9)*4, not(%10)*1, not(%11)*1, not(%12)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/sub_diffscale0_broadcast_non_fcd_i16_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/sub_diffscale0_broadcast_non_fcd_i16_goya2.c"
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
  br i1 %36, label %37, label %64

37:                                               ; preds = %17
  %38 = icmp slt i32 %22, %24
  br i1 %38, label %39, label %283

39:                                               ; preds = %37
  %40 = icmp ne i8 %3, 0
  %41 = icmp ne i8 %8, 0
  %42 = icmp ne i8 %15, 0
  %43 = icmp ne i8 %16, 0
  %44 = icmp slt i32 %33, %34
  %45 = icmp ne i8 %7, 0
  %46 = icmp ne i8 %12, 0
  %47 = icmp slt i32 %31, %32
  %48 = icmp ne i8 %6, 0
  %49 = icmp ne i8 %11, 0
  %50 = icmp slt i32 %29, %30
  %51 = icmp ne i8 %5, 0
  %52 = icmp ne i8 %10, 0
  %53 = icmp slt i32 %26, %28
  %54 = icmp ne i8 %4, 0
  %55 = icmp ne i8 %9, 0
  %56 = or i8 %15, %3
  %57 = icmp ne i8 %56, 0
  %58 = or i8 %16, %8
  %59 = icmp ne i8 %58, 0
  %60 = trunc i32 %0 to i8
  %61 = insertelement <256 x i8> undef, i8 %60, i32 0
  %62 = shufflevector <256 x i8> %61, <256 x i8> undef, <256 x i32> zeroinitializer
  %63 = icmp ne i8 %13, 0
  br label %161

64:                                               ; preds = %17
  %65 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %66 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %67 = icmp ne i8 %13, 0
  %68 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %65, i32 0, <128 x i16> zeroinitializer, i1 %67, i1 false)
  %69 = icmp ne i8 %14, 0
  %70 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %66, i32 0, <128 x i16> zeroinitializer, i1 %69, i1 false)
  %71 = icmp slt i32 %22, %24
  br i1 %71, label %72, label %283

72:                                               ; preds = %64
  %73 = icmp slt i32 %33, %34
  %74 = icmp slt i32 %31, %32
  %75 = icmp slt i32 %29, %30
  %76 = icmp slt i32 %26, %28
  %77 = trunc i32 %0 to i8
  %78 = insertelement <256 x i8> undef, i8 %77, i32 0
  %79 = shufflevector <256 x i8> %78, <256 x i8> undef, <256 x i32> zeroinitializer
  br label %80

80:                                               ; preds = %72, %91
  %81 = phi <5 x i32> [ zeroinitializer, %72 ], [ %94, %91 ]
  %82 = phi i32 [ %22, %72 ], [ %95, %91 ]
  %83 = phi <128 x i16> [ %70, %72 ], [ %93, %91 ]
  %84 = phi <128 x i16> [ %68, %72 ], [ %92, %91 ]
  %85 = insertelement <5 x i32> %81, i32 %82, i32 0
  br i1 %73, label %86, label %91

86:                                               ; preds = %80
  br label %97

87:                                               ; preds = %108
  %88 = phi <128 x i16> [ %109, %108 ]
  %89 = phi <128 x i16> [ %110, %108 ]
  %90 = phi <5 x i32> [ %111, %108 ]
  br label %91

91:                                               ; preds = %87, %80
  %92 = phi <128 x i16> [ %84, %80 ], [ %88, %87 ]
  %93 = phi <128 x i16> [ %83, %80 ], [ %89, %87 ]
  %94 = phi <5 x i32> [ %85, %80 ], [ %90, %87 ]
  %95 = add nsw i32 %82, 128
  %96 = icmp slt i32 %95, %24
  br i1 %96, label %80, label %282, !llvm.loop !3

97:                                               ; preds = %86, %108
  %98 = phi <5 x i32> [ %111, %108 ], [ %85, %86 ]
  %99 = phi i32 [ %112, %108 ], [ %33, %86 ]
  %100 = phi <128 x i16> [ %110, %108 ], [ %83, %86 ]
  %101 = phi <128 x i16> [ %109, %108 ], [ %84, %86 ]
  %102 = insertelement <5 x i32> %98, i32 %99, i32 4
  br i1 %74, label %103, label %108

103:                                              ; preds = %97
  br label %114

104:                                              ; preds = %125
  %105 = phi <128 x i16> [ %126, %125 ]
  %106 = phi <128 x i16> [ %127, %125 ]
  %107 = phi <5 x i32> [ %128, %125 ]
  br label %108

108:                                              ; preds = %104, %97
  %109 = phi <128 x i16> [ %101, %97 ], [ %105, %104 ]
  %110 = phi <128 x i16> [ %100, %97 ], [ %106, %104 ]
  %111 = phi <5 x i32> [ %102, %97 ], [ %107, %104 ]
  %112 = add i32 %99, 1
  %113 = icmp eq i32 %112, %34
  br i1 %113, label %87, label %97, !llvm.loop !5

114:                                              ; preds = %103, %125
  %115 = phi <5 x i32> [ %128, %125 ], [ %102, %103 ]
  %116 = phi i32 [ %129, %125 ], [ %31, %103 ]
  %117 = phi <128 x i16> [ %127, %125 ], [ %100, %103 ]
  %118 = phi <128 x i16> [ %126, %125 ], [ %101, %103 ]
  %119 = insertelement <5 x i32> %115, i32 %116, i32 3
  br i1 %75, label %120, label %125

120:                                              ; preds = %114
  br label %131

121:                                              ; preds = %143
  %122 = phi <128 x i16> [ %144, %143 ]
  %123 = phi <128 x i16> [ %145, %143 ]
  %124 = phi <5 x i32> [ %146, %143 ]
  br label %125

125:                                              ; preds = %121, %114
  %126 = phi <128 x i16> [ %118, %114 ], [ %122, %121 ]
  %127 = phi <128 x i16> [ %117, %114 ], [ %123, %121 ]
  %128 = phi <5 x i32> [ %119, %114 ], [ %124, %121 ]
  %129 = add i32 %116, 1
  %130 = icmp eq i32 %129, %32
  br i1 %130, label %104, label %114, !llvm.loop !6

131:                                              ; preds = %120, %143
  %132 = phi <5 x i32> [ %146, %143 ], [ %119, %120 ]
  %133 = phi i32 [ %147, %143 ], [ %29, %120 ]
  %134 = phi <128 x i16> [ %145, %143 ], [ %117, %120 ]
  %135 = phi <128 x i16> [ %144, %143 ], [ %118, %120 ]
  %136 = insertelement <5 x i32> %132, i32 %133, i32 2
  %137 = insertelement <5 x i32> %136, i32 %26, i32 1
  br i1 %76, label %138, label %143

138:                                              ; preds = %131
  br label %149

139:                                              ; preds = %149
  %140 = phi <128 x i16> [ %154, %149 ]
  %141 = phi <128 x i16> [ %155, %149 ]
  %142 = phi <5 x i32> [ %158, %149 ]
  br label %143

143:                                              ; preds = %139, %131
  %144 = phi <128 x i16> [ %135, %131 ], [ %140, %139 ]
  %145 = phi <128 x i16> [ %134, %131 ], [ %141, %139 ]
  %146 = phi <5 x i32> [ %137, %131 ], [ %142, %139 ]
  %147 = add i32 %133, 1
  %148 = icmp eq i32 %147, %30
  br i1 %148, label %121, label %131, !llvm.loop !7

149:                                              ; preds = %138, %149
  %150 = phi <5 x i32> [ %158, %149 ], [ %137, %138 ]
  %151 = phi i32 [ %159, %149 ], [ %26, %138 ]
  %152 = phi <128 x i16> [ %155, %149 ], [ %134, %138 ]
  %153 = phi <128 x i16> [ %154, %149 ], [ %135, %138 ]
  %154 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %150, i8 0, i32 0, <128 x i16> %153, i1 %69, i1 false)
  %155 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %150, i8 1, i32 0, <128 x i16> %152, i1 %67, i1 false)
  %156 = tail call <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16> %154, <256 x i8> %79, i8 7, i32 2, <128 x i16> zeroinitializer, i1 %67, i1 true)
  %157 = tail call <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16> %156, <128 x i16> %155, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %150, i8 2, <128 x i16> %157, i32 0, i1 true, i1 false)
  %158 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %150, i32 1, i32 2, i8 2, i32 0, <5 x i32> %150, i1 true, i1 false)
  %159 = add i32 %151, 1
  %160 = icmp eq i32 %159, %28
  br i1 %160, label %139, label %149, !llvm.loop !8

161:                                              ; preds = %39, %182
  %162 = phi <5 x i32> [ zeroinitializer, %39 ], [ %187, %182 ]
  %163 = phi <5 x i32> [ zeroinitializer, %39 ], [ %186, %182 ]
  %164 = phi <5 x i32> [ zeroinitializer, %39 ], [ %185, %182 ]
  %165 = phi i32 [ %22, %39 ], [ %188, %182 ]
  %166 = phi <128 x i16> [ zeroinitializer, %39 ], [ %184, %182 ]
  %167 = phi <128 x i16> [ zeroinitializer, %39 ], [ %183, %182 ]
  %168 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %162, i32 1, i32 %165, i32 0, i1 %40, i1 true)
  %169 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %163, i32 1, i32 %165, i32 0, i1 %41, i1 true)
  %170 = insertelement <5 x i32> %164, i32 %165, i32 0
  %171 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %168, i32 2, i32 0, i32 0, i1 %42, i1 false)
  %172 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %169, i32 2, i32 0, i32 0, i1 %43, i1 false)
  %173 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %171, i8 0, i32 0, <128 x i16> %166, i1 %42, i1 false)
  %174 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %172, i8 1, i32 0, <128 x i16> %167, i1 %43, i1 false)
  br i1 %44, label %175, label %182

175:                                              ; preds = %161
  br label %190

176:                                              ; preds = %207
  %177 = phi <128 x i16> [ %208, %207 ]
  %178 = phi <128 x i16> [ %209, %207 ]
  %179 = phi <5 x i32> [ %210, %207 ]
  %180 = phi <5 x i32> [ %211, %207 ]
  %181 = phi <5 x i32> [ %212, %207 ]
  br label %182

182:                                              ; preds = %176, %161
  %183 = phi <128 x i16> [ %174, %161 ], [ %177, %176 ]
  %184 = phi <128 x i16> [ %173, %161 ], [ %178, %176 ]
  %185 = phi <5 x i32> [ %170, %161 ], [ %179, %176 ]
  %186 = phi <5 x i32> [ %172, %161 ], [ %180, %176 ]
  %187 = phi <5 x i32> [ %171, %161 ], [ %181, %176 ]
  %188 = add nsw i32 %165, 128
  %189 = icmp slt i32 %188, %24
  br i1 %189, label %161, label %281, !llvm.loop !10

190:                                              ; preds = %175, %207
  %191 = phi <5 x i32> [ %212, %207 ], [ %171, %175 ]
  %192 = phi <5 x i32> [ %211, %207 ], [ %172, %175 ]
  %193 = phi <5 x i32> [ %210, %207 ], [ %170, %175 ]
  %194 = phi i32 [ %213, %207 ], [ %33, %175 ]
  %195 = phi <128 x i16> [ %209, %207 ], [ %173, %175 ]
  %196 = phi <128 x i16> [ %208, %207 ], [ %174, %175 ]
  %197 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %191, i32 16, i32 %194, i32 0, i1 %45, i1 true)
  %198 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %192, i32 16, i32 %194, i32 0, i1 %46, i1 true)
  %199 = insertelement <5 x i32> %193, i32 %194, i32 4
  br i1 %47, label %200, label %207

200:                                              ; preds = %190
  br label %215

201:                                              ; preds = %232
  %202 = phi <128 x i16> [ %233, %232 ]
  %203 = phi <128 x i16> [ %234, %232 ]
  %204 = phi <5 x i32> [ %235, %232 ]
  %205 = phi <5 x i32> [ %236, %232 ]
  %206 = phi <5 x i32> [ %237, %232 ]
  br label %207

207:                                              ; preds = %201, %190
  %208 = phi <128 x i16> [ %196, %190 ], [ %202, %201 ]
  %209 = phi <128 x i16> [ %195, %190 ], [ %203, %201 ]
  %210 = phi <5 x i32> [ %199, %190 ], [ %204, %201 ]
  %211 = phi <5 x i32> [ %198, %190 ], [ %205, %201 ]
  %212 = phi <5 x i32> [ %197, %190 ], [ %206, %201 ]
  %213 = add i32 %194, 1
  %214 = icmp eq i32 %213, %34
  br i1 %214, label %176, label %190, !llvm.loop !11

215:                                              ; preds = %200, %232
  %216 = phi <5 x i32> [ %237, %232 ], [ %197, %200 ]
  %217 = phi <5 x i32> [ %236, %232 ], [ %198, %200 ]
  %218 = phi <5 x i32> [ %235, %232 ], [ %199, %200 ]
  %219 = phi i32 [ %238, %232 ], [ %31, %200 ]
  %220 = phi <128 x i16> [ %234, %232 ], [ %195, %200 ]
  %221 = phi <128 x i16> [ %233, %232 ], [ %196, %200 ]
  %222 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %216, i32 8, i32 %219, i32 0, i1 %48, i1 true)
  %223 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %217, i32 8, i32 %219, i32 0, i1 %49, i1 true)
  %224 = insertelement <5 x i32> %218, i32 %219, i32 3
  br i1 %50, label %225, label %232

225:                                              ; preds = %215
  br label %240

226:                                              ; preds = %257
  %227 = phi <128 x i16> [ %258, %257 ]
  %228 = phi <128 x i16> [ %259, %257 ]
  %229 = phi <5 x i32> [ %260, %257 ]
  %230 = phi <5 x i32> [ %261, %257 ]
  %231 = phi <5 x i32> [ %262, %257 ]
  br label %232

232:                                              ; preds = %226, %215
  %233 = phi <128 x i16> [ %221, %215 ], [ %227, %226 ]
  %234 = phi <128 x i16> [ %220, %215 ], [ %228, %226 ]
  %235 = phi <5 x i32> [ %224, %215 ], [ %229, %226 ]
  %236 = phi <5 x i32> [ %223, %215 ], [ %230, %226 ]
  %237 = phi <5 x i32> [ %222, %215 ], [ %231, %226 ]
  %238 = add i32 %219, 1
  %239 = icmp eq i32 %238, %32
  br i1 %239, label %201, label %215, !llvm.loop !12

240:                                              ; preds = %225, %257
  %241 = phi <5 x i32> [ %262, %257 ], [ %222, %225 ]
  %242 = phi <5 x i32> [ %261, %257 ], [ %223, %225 ]
  %243 = phi <5 x i32> [ %260, %257 ], [ %224, %225 ]
  %244 = phi i32 [ %263, %257 ], [ %29, %225 ]
  %245 = phi <128 x i16> [ %259, %257 ], [ %220, %225 ]
  %246 = phi <128 x i16> [ %258, %257 ], [ %221, %225 ]
  %247 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %241, i32 4, i32 %244, i32 0, i1 %51, i1 true)
  %248 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %242, i32 4, i32 %244, i32 0, i1 %52, i1 true)
  %249 = insertelement <5 x i32> %243, i32 %244, i32 2
  br i1 %53, label %250, label %257

250:                                              ; preds = %240
  br label %265

251:                                              ; preds = %265
  %252 = phi <5 x i32> [ %272, %265 ]
  %253 = phi <5 x i32> [ %273, %265 ]
  %254 = phi <5 x i32> [ %274, %265 ]
  %255 = phi <128 x i16> [ %275, %265 ]
  %256 = phi <128 x i16> [ %276, %265 ]
  br label %257

257:                                              ; preds = %251, %240
  %258 = phi <128 x i16> [ %246, %240 ], [ %256, %251 ]
  %259 = phi <128 x i16> [ %245, %240 ], [ %255, %251 ]
  %260 = phi <5 x i32> [ %249, %240 ], [ %254, %251 ]
  %261 = phi <5 x i32> [ %248, %240 ], [ %253, %251 ]
  %262 = phi <5 x i32> [ %247, %240 ], [ %252, %251 ]
  %263 = add i32 %244, 1
  %264 = icmp eq i32 %263, %30
  br i1 %264, label %226, label %240, !llvm.loop !13

265:                                              ; preds = %250, %265
  %266 = phi <5 x i32> [ %272, %265 ], [ %247, %250 ]
  %267 = phi <5 x i32> [ %273, %265 ], [ %248, %250 ]
  %268 = phi <5 x i32> [ %274, %265 ], [ %249, %250 ]
  %269 = phi i32 [ %279, %265 ], [ %26, %250 ]
  %270 = phi <128 x i16> [ %275, %265 ], [ %245, %250 ]
  %271 = phi <128 x i16> [ %276, %265 ], [ %246, %250 ]
  %272 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %266, i32 2, i32 %269, i32 0, i1 %54, i1 true)
  %273 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %267, i32 2, i32 %269, i32 0, i1 %55, i1 true)
  %274 = insertelement <5 x i32> %268, i32 %269, i32 1
  %275 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %272, i8 0, i32 0, <128 x i16> %270, i1 %57, i1 true)
  %276 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %273, i8 1, i32 0, <128 x i16> %271, i1 %59, i1 true)
  %277 = tail call <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16> %275, <256 x i8> %62, i8 7, i32 2, <128 x i16> %275, i1 %63, i1 true)
  %278 = tail call <128 x i16> @llvm.tpc.sub.v128i16.v128i16.v128i16.i1(<128 x i16> %277, <128 x i16> %276, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %274, i8 2, <128 x i16> %278, i32 0, i1 true, i1 false)
  %279 = add i32 %269, 1
  %280 = icmp eq i32 %279, %28
  br i1 %280, label %251, label %265, !llvm.loop !14

281:                                              ; preds = %182
  br label %283

282:                                              ; preds = %91
  br label %283

283:                                              ; preds = %282, %281, %64, %37
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
declare <128 x i16> @llvm.tpc.ash.v128i16.v128i16.v256i8.i1(<128 x i16>, <256 x i8>, i8, i32, <128 x i16>, i1, i1) #1

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
