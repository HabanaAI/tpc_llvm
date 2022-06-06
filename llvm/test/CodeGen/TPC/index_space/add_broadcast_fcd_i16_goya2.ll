; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*128, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*128, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ not(%2)*128, not(%3)*4, not(%4)*1, not(%5)*1, not(%6)*1 }[1].[Input].{ not(%7)*128, not(%8)*4, not(%9)*1, not(%10)*1, not(%11)*1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/goya2/elementwise/binary/add_broadcast_fcd_i16_goya2.c'
source_filename = "./kernels/goya2/elementwise/binary/add_broadcast_fcd_i16_goya2.c"
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
  %36 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %37 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %38 = icmp ne i8 %12, 0
  %39 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %36, i32 0, <128 x i16> zeroinitializer, i1 %38, i1 false)
  %40 = icmp ne i8 %13, 0
  %41 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %37, i32 0, <128 x i16> zeroinitializer, i1 %40, i1 false)
  %42 = icmp slt i32 %21, %23
  br i1 %35, label %49, label %43

43:                                               ; preds = %16
  br i1 %42, label %44, label %276

44:                                               ; preds = %43
  %45 = icmp slt i32 %32, %33
  %46 = icmp slt i32 %30, %31
  %47 = icmp slt i32 %28, %29
  %48 = icmp slt i32 %25, %27
  br label %71

49:                                               ; preds = %16
  br i1 %42, label %50, label %276

50:                                               ; preds = %49
  %51 = icmp ne i8 %2, 0
  %52 = icmp ne i8 %7, 0
  %53 = icmp ne i8 %14, 0
  %54 = icmp ne i8 %15, 0
  %55 = icmp slt i32 %32, %33
  %56 = icmp ne i8 %6, 0
  %57 = icmp ne i8 %11, 0
  %58 = icmp slt i32 %30, %31
  %59 = icmp ne i8 %5, 0
  %60 = icmp ne i8 %10, 0
  %61 = icmp slt i32 %28, %29
  %62 = icmp ne i8 %4, 0
  %63 = icmp ne i8 %9, 0
  %64 = icmp slt i32 %25, %27
  %65 = icmp ne i8 %3, 0
  %66 = icmp ne i8 %8, 0
  %67 = or i8 %14, %2
  %68 = icmp ne i8 %67, 0
  %69 = or i8 %15, %7
  %70 = icmp ne i8 %69, 0
  br label %151

71:                                               ; preds = %44, %82
  %72 = phi <5 x i32> [ zeroinitializer, %44 ], [ %85, %82 ]
  %73 = phi i32 [ %21, %44 ], [ %86, %82 ]
  %74 = phi <128 x i16> [ %41, %44 ], [ %84, %82 ]
  %75 = phi <128 x i16> [ %39, %44 ], [ %83, %82 ]
  %76 = insertelement <5 x i32> %72, i32 %73, i32 0
  br i1 %45, label %77, label %82

77:                                               ; preds = %71
  br label %88

78:                                               ; preds = %99
  %79 = phi <128 x i16> [ %100, %99 ]
  %80 = phi <128 x i16> [ %101, %99 ]
  %81 = phi <5 x i32> [ %102, %99 ]
  br label %82

82:                                               ; preds = %78, %71
  %83 = phi <128 x i16> [ %75, %71 ], [ %79, %78 ]
  %84 = phi <128 x i16> [ %74, %71 ], [ %80, %78 ]
  %85 = phi <5 x i32> [ %76, %71 ], [ %81, %78 ]
  %86 = add nsw i32 %73, 128
  %87 = icmp slt i32 %86, %23
  br i1 %87, label %71, label %275, !llvm.loop !3

88:                                               ; preds = %77, %99
  %89 = phi <5 x i32> [ %102, %99 ], [ %76, %77 ]
  %90 = phi i32 [ %103, %99 ], [ %32, %77 ]
  %91 = phi <128 x i16> [ %101, %99 ], [ %74, %77 ]
  %92 = phi <128 x i16> [ %100, %99 ], [ %75, %77 ]
  %93 = insertelement <5 x i32> %89, i32 %90, i32 4
  br i1 %46, label %94, label %99

94:                                               ; preds = %88
  br label %105

95:                                               ; preds = %116
  %96 = phi <128 x i16> [ %117, %116 ]
  %97 = phi <128 x i16> [ %118, %116 ]
  %98 = phi <5 x i32> [ %119, %116 ]
  br label %99

99:                                               ; preds = %95, %88
  %100 = phi <128 x i16> [ %92, %88 ], [ %96, %95 ]
  %101 = phi <128 x i16> [ %91, %88 ], [ %97, %95 ]
  %102 = phi <5 x i32> [ %93, %88 ], [ %98, %95 ]
  %103 = add i32 %90, 1
  %104 = icmp eq i32 %103, %33
  br i1 %104, label %78, label %88, !llvm.loop !5

105:                                              ; preds = %94, %116
  %106 = phi <5 x i32> [ %119, %116 ], [ %93, %94 ]
  %107 = phi i32 [ %120, %116 ], [ %30, %94 ]
  %108 = phi <128 x i16> [ %118, %116 ], [ %91, %94 ]
  %109 = phi <128 x i16> [ %117, %116 ], [ %92, %94 ]
  %110 = insertelement <5 x i32> %106, i32 %107, i32 3
  br i1 %47, label %111, label %116

111:                                              ; preds = %105
  br label %122

112:                                              ; preds = %134
  %113 = phi <128 x i16> [ %135, %134 ]
  %114 = phi <128 x i16> [ %136, %134 ]
  %115 = phi <5 x i32> [ %137, %134 ]
  br label %116

116:                                              ; preds = %112, %105
  %117 = phi <128 x i16> [ %109, %105 ], [ %113, %112 ]
  %118 = phi <128 x i16> [ %108, %105 ], [ %114, %112 ]
  %119 = phi <5 x i32> [ %110, %105 ], [ %115, %112 ]
  %120 = add i32 %107, 1
  %121 = icmp eq i32 %120, %31
  br i1 %121, label %95, label %105, !llvm.loop !6

122:                                              ; preds = %111, %134
  %123 = phi <5 x i32> [ %137, %134 ], [ %110, %111 ]
  %124 = phi i32 [ %138, %134 ], [ %28, %111 ]
  %125 = phi <128 x i16> [ %136, %134 ], [ %108, %111 ]
  %126 = phi <128 x i16> [ %135, %134 ], [ %109, %111 ]
  %127 = insertelement <5 x i32> %123, i32 %124, i32 2
  %128 = insertelement <5 x i32> %127, i32 %25, i32 1
  br i1 %48, label %129, label %134

129:                                              ; preds = %122
  br label %140

130:                                              ; preds = %140
  %131 = phi <128 x i16> [ %145, %140 ]
  %132 = phi <128 x i16> [ %146, %140 ]
  %133 = phi <5 x i32> [ %148, %140 ]
  br label %134

134:                                              ; preds = %130, %122
  %135 = phi <128 x i16> [ %126, %122 ], [ %131, %130 ]
  %136 = phi <128 x i16> [ %125, %122 ], [ %132, %130 ]
  %137 = phi <5 x i32> [ %128, %122 ], [ %133, %130 ]
  %138 = add i32 %124, 1
  %139 = icmp eq i32 %138, %29
  br i1 %139, label %112, label %122, !llvm.loop !7

140:                                              ; preds = %129, %140
  %141 = phi <5 x i32> [ %148, %140 ], [ %128, %129 ]
  %142 = phi i32 [ %149, %140 ], [ %25, %129 ]
  %143 = phi <128 x i16> [ %146, %140 ], [ %125, %129 ]
  %144 = phi <128 x i16> [ %145, %140 ], [ %126, %129 ]
  %145 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %141, i8 0, i32 0, <128 x i16> %144, i1 %40, i1 false)
  %146 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %141, i8 1, i32 0, <128 x i16> %143, i1 %38, i1 false)
  %147 = tail call <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16> %145, <128 x i16> %146, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %141, i8 2, <128 x i16> %147, i32 0, i1 true, i1 false)
  %148 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %141, i32 1, i32 2, i8 2, i32 0, <5 x i32> %141, i1 true, i1 false)
  %149 = add i32 %142, 1
  %150 = icmp eq i32 %149, %27
  br i1 %150, label %130, label %140, !llvm.loop !8

151:                                              ; preds = %50, %172
  %152 = phi <5 x i32> [ zeroinitializer, %50 ], [ %177, %172 ]
  %153 = phi <5 x i32> [ zeroinitializer, %50 ], [ %176, %172 ]
  %154 = phi <5 x i32> [ zeroinitializer, %50 ], [ %175, %172 ]
  %155 = phi <128 x i16> [ %39, %50 ], [ %174, %172 ]
  %156 = phi i32 [ %21, %50 ], [ %178, %172 ]
  %157 = phi <128 x i16> [ %41, %50 ], [ %173, %172 ]
  %158 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %152, i32 1, i32 %156, i32 0, i1 %51, i1 true)
  %159 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %153, i32 1, i32 %156, i32 0, i1 %52, i1 true)
  %160 = insertelement <5 x i32> %154, i32 %156, i32 0
  %161 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %158, i32 2, i32 0, i32 0, i1 %53, i1 false)
  %162 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %159, i32 2, i32 0, i32 0, i1 %54, i1 false)
  %163 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %161, i8 0, i32 0, <128 x i16> %155, i1 %53, i1 false)
  %164 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %162, i8 1, i32 0, <128 x i16> %157, i1 %54, i1 false)
  br i1 %55, label %165, label %172

165:                                              ; preds = %151
  br label %180

166:                                              ; preds = %197
  %167 = phi <128 x i16> [ %198, %197 ]
  %168 = phi <128 x i16> [ %199, %197 ]
  %169 = phi <5 x i32> [ %200, %197 ]
  %170 = phi <5 x i32> [ %201, %197 ]
  %171 = phi <5 x i32> [ %202, %197 ]
  br label %172

172:                                              ; preds = %166, %151
  %173 = phi <128 x i16> [ %164, %151 ], [ %167, %166 ]
  %174 = phi <128 x i16> [ %163, %151 ], [ %168, %166 ]
  %175 = phi <5 x i32> [ %160, %151 ], [ %169, %166 ]
  %176 = phi <5 x i32> [ %162, %151 ], [ %170, %166 ]
  %177 = phi <5 x i32> [ %161, %151 ], [ %171, %166 ]
  %178 = add nsw i32 %156, 128
  %179 = icmp slt i32 %178, %23
  br i1 %179, label %151, label %274, !llvm.loop !10

180:                                              ; preds = %165, %197
  %181 = phi <5 x i32> [ %202, %197 ], [ %161, %165 ]
  %182 = phi <5 x i32> [ %201, %197 ], [ %162, %165 ]
  %183 = phi <5 x i32> [ %200, %197 ], [ %160, %165 ]
  %184 = phi <128 x i16> [ %199, %197 ], [ %163, %165 ]
  %185 = phi i32 [ %203, %197 ], [ %32, %165 ]
  %186 = phi <128 x i16> [ %198, %197 ], [ %164, %165 ]
  %187 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %181, i32 16, i32 %185, i32 0, i1 %56, i1 true)
  %188 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %182, i32 16, i32 %185, i32 0, i1 %57, i1 true)
  %189 = insertelement <5 x i32> %183, i32 %185, i32 4
  br i1 %58, label %190, label %197

190:                                              ; preds = %180
  br label %205

191:                                              ; preds = %222
  %192 = phi <128 x i16> [ %223, %222 ]
  %193 = phi <128 x i16> [ %224, %222 ]
  %194 = phi <5 x i32> [ %225, %222 ]
  %195 = phi <5 x i32> [ %226, %222 ]
  %196 = phi <5 x i32> [ %227, %222 ]
  br label %197

197:                                              ; preds = %191, %180
  %198 = phi <128 x i16> [ %186, %180 ], [ %192, %191 ]
  %199 = phi <128 x i16> [ %184, %180 ], [ %193, %191 ]
  %200 = phi <5 x i32> [ %189, %180 ], [ %194, %191 ]
  %201 = phi <5 x i32> [ %188, %180 ], [ %195, %191 ]
  %202 = phi <5 x i32> [ %187, %180 ], [ %196, %191 ]
  %203 = add i32 %185, 1
  %204 = icmp eq i32 %203, %33
  br i1 %204, label %166, label %180, !llvm.loop !11

205:                                              ; preds = %190, %222
  %206 = phi <5 x i32> [ %227, %222 ], [ %187, %190 ]
  %207 = phi <5 x i32> [ %226, %222 ], [ %188, %190 ]
  %208 = phi <5 x i32> [ %225, %222 ], [ %189, %190 ]
  %209 = phi <128 x i16> [ %224, %222 ], [ %184, %190 ]
  %210 = phi i32 [ %228, %222 ], [ %30, %190 ]
  %211 = phi <128 x i16> [ %223, %222 ], [ %186, %190 ]
  %212 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %206, i32 8, i32 %210, i32 0, i1 %59, i1 true)
  %213 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %207, i32 8, i32 %210, i32 0, i1 %60, i1 true)
  %214 = insertelement <5 x i32> %208, i32 %210, i32 3
  br i1 %61, label %215, label %222

215:                                              ; preds = %205
  br label %230

216:                                              ; preds = %247
  %217 = phi <128 x i16> [ %248, %247 ]
  %218 = phi <128 x i16> [ %249, %247 ]
  %219 = phi <5 x i32> [ %250, %247 ]
  %220 = phi <5 x i32> [ %251, %247 ]
  %221 = phi <5 x i32> [ %252, %247 ]
  br label %222

222:                                              ; preds = %216, %205
  %223 = phi <128 x i16> [ %211, %205 ], [ %217, %216 ]
  %224 = phi <128 x i16> [ %209, %205 ], [ %218, %216 ]
  %225 = phi <5 x i32> [ %214, %205 ], [ %219, %216 ]
  %226 = phi <5 x i32> [ %213, %205 ], [ %220, %216 ]
  %227 = phi <5 x i32> [ %212, %205 ], [ %221, %216 ]
  %228 = add i32 %210, 1
  %229 = icmp eq i32 %228, %31
  br i1 %229, label %191, label %205, !llvm.loop !12

230:                                              ; preds = %215, %247
  %231 = phi <5 x i32> [ %252, %247 ], [ %212, %215 ]
  %232 = phi <5 x i32> [ %251, %247 ], [ %213, %215 ]
  %233 = phi <5 x i32> [ %250, %247 ], [ %214, %215 ]
  %234 = phi <128 x i16> [ %249, %247 ], [ %209, %215 ]
  %235 = phi i32 [ %253, %247 ], [ %28, %215 ]
  %236 = phi <128 x i16> [ %248, %247 ], [ %211, %215 ]
  %237 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %231, i32 4, i32 %235, i32 0, i1 %62, i1 true)
  %238 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %232, i32 4, i32 %235, i32 0, i1 %63, i1 true)
  %239 = insertelement <5 x i32> %233, i32 %235, i32 2
  br i1 %64, label %240, label %247

240:                                              ; preds = %230
  br label %255

241:                                              ; preds = %255
  %242 = phi <5 x i32> [ %262, %255 ]
  %243 = phi <5 x i32> [ %263, %255 ]
  %244 = phi <5 x i32> [ %264, %255 ]
  %245 = phi <128 x i16> [ %269, %255 ]
  %246 = phi <128 x i16> [ %270, %255 ]
  br label %247

247:                                              ; preds = %241, %230
  %248 = phi <128 x i16> [ %236, %230 ], [ %246, %241 ]
  %249 = phi <128 x i16> [ %234, %230 ], [ %245, %241 ]
  %250 = phi <5 x i32> [ %239, %230 ], [ %244, %241 ]
  %251 = phi <5 x i32> [ %238, %230 ], [ %243, %241 ]
  %252 = phi <5 x i32> [ %237, %230 ], [ %242, %241 ]
  %253 = add i32 %235, 1
  %254 = icmp eq i32 %253, %29
  br i1 %254, label %216, label %230, !llvm.loop !13

255:                                              ; preds = %240, %255
  %256 = phi <5 x i32> [ %262, %255 ], [ %237, %240 ]
  %257 = phi <5 x i32> [ %263, %255 ], [ %238, %240 ]
  %258 = phi <5 x i32> [ %264, %255 ], [ %239, %240 ]
  %259 = phi <128 x i16> [ %269, %255 ], [ %234, %240 ]
  %260 = phi i32 [ %272, %255 ], [ %25, %240 ]
  %261 = phi <128 x i16> [ %270, %255 ], [ %236, %240 ]
  %262 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %256, i32 2, i32 %260, i32 0, i1 %65, i1 true)
  %263 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %257, i32 2, i32 %260, i32 0, i1 %66, i1 true)
  %264 = insertelement <5 x i32> %258, i32 %260, i32 1
  %265 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %262, i8 0, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %266 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %263, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %267 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %265, i32 0, <128 x i16> %259, i1 %51, i1 false)
  %268 = tail call <128 x i16> @llvm.tpc.ld.g.v128i16.i1(i8 addrspace(3)* %266, i32 0, <128 x i16> %261, i1 %52, i1 false)
  %269 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %262, i8 0, i32 0, <128 x i16> %267, i1 %68, i1 true)
  %270 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %263, i8 1, i32 0, <128 x i16> %268, i1 %70, i1 true)
  %271 = tail call <128 x i16> @llvm.tpc.add.v128i16.v128i16.v128i16.i1(<128 x i16> %269, <128 x i16> %270, i8 7, i32 1, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %264, i8 2, <128 x i16> %271, i32 0, i1 true, i1 false)
  %272 = add i32 %260, 1
  %273 = icmp eq i32 %272, %27
  br i1 %273, label %241, label %255, !llvm.loop !14

274:                                              ; preds = %172
  br label %276

275:                                              ; preds = %82
  br label %276

276:                                              ; preds = %275, %274, %43, %49
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
