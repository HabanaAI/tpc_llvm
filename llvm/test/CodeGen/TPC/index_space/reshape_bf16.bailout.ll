; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './kernels/gaudi/misc/reshape_bf16.c'
source_filename = "./kernels/gaudi/misc/reshape_bf16.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 !unroll_info !3 {
  %1 = tail call i32 @llvm.tpc.ld.l.i32(i32 1040, i32 1, i32 undef, i1 true, i1 false) #4
  %2 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #4
  %3 = tail call i32 @llvm.tpc.ld.l.i32(i32 1056, i32 1, i32 undef, i1 true, i1 false) #4
  %4 = tail call i32 @llvm.tpc.ld.l.i32(i32 1064, i32 1, i32 undef, i1 true, i1 false) #4
  %5 = tail call i32 @llvm.tpc.ld.l.i32(i32 1072, i32 1, i32 undef, i1 true, i1 false) #4
  %6 = tail call i32 @llvm.tpc.ld.l.i32(i32 1052, i32 1, i32 undef, i1 true, i1 false) #4
  %7 = tail call i32 @llvm.tpc.ld.l.i32(i32 1060, i32 1, i32 undef, i1 true, i1 false) #4
  %8 = tail call i32 @llvm.tpc.ld.l.i32(i32 1068, i32 1, i32 undef, i1 true, i1 false) #4
  %9 = tail call i32 @llvm.tpc.ld.l.i32(i32 1076, i32 1, i32 undef, i1 true, i1 false) #4
  %10 = tail call i32 @llvm.tpc.ld.l.i32(i32 1096, i32 1, i32 undef, i1 true, i1 false) #4
  %11 = tail call i32 @llvm.tpc.ld.l.i32(i32 1104, i32 1, i32 undef, i1 true, i1 false) #4
  %12 = tail call i32 @llvm.tpc.ld.l.i32(i32 1112, i32 1, i32 undef, i1 true, i1 false) #4
  %13 = tail call i32 @llvm.tpc.ld.l.i32(i32 1120, i32 1, i32 undef, i1 true, i1 false) #4
  %14 = tail call i32 @llvm.tpc.ld.l.i32(i32 1128, i32 1, i32 undef, i1 true, i1 false) #4
  %15 = tail call i32 @llvm.tpc.ld.l.i32(i32 1108, i32 1, i32 undef, i1 true, i1 false) #4
  %16 = tail call i32 @llvm.tpc.ld.l.i32(i32 1116, i32 1, i32 undef, i1 true, i1 false) #4
  %17 = tail call i32 @llvm.tpc.ld.l.i32(i32 1124, i32 1, i32 undef, i1 true, i1 false) #4
  %18 = tail call i32 @llvm.tpc.ld.l.i32(i32 1132, i32 1, i32 undef, i1 true, i1 false) #4
  %19 = icmp eq i32 %1, %6
  br i1 %19, label %20, label %52

20:                                               ; preds = %0
  %21 = icmp eq i32 %7, 0
  %22 = mul nsw i32 %2, %1
  %23 = select i1 %21, i32 %22, i32 %7
  %24 = mul nsw i32 %5, %9
  %25 = mul nsw i32 %1, %2
  %26 = icmp eq i32 %25, %23
  br i1 %26, label %27, label %44

27:                                               ; preds = %20
  %28 = icmp eq i32 %8, 0
  %29 = mul nsw i32 %3, %23
  %30 = select i1 %28, i32 %29, i32 %8
  %31 = mul nsw i32 %23, %3
  %32 = icmp eq i32 %31, %30
  br i1 %32, label %33, label %44

33:                                               ; preds = %27
  %34 = icmp eq i32 %9, 0
  %35 = mul nsw i32 %4, %30
  %36 = select i1 %34, i32 %35, i32 %9
  %37 = mul nsw i32 %30, %4
  %38 = icmp eq i32 %37, %36
  br i1 %38, label %39, label %44

39:                                               ; preds = %33
  %40 = icmp eq i32 %24, 0
  %41 = mul nsw i32 %5, %36
  %42 = select i1 %40, i32 %41, i32 %24
  %43 = mul nsw i32 %36, %5
  br label %44

44:                                               ; preds = %39, %33, %27, %20
  %45 = phi i32 [ %3, %20 ], [ %4, %27 ], [ %5, %33 ], [ 1, %39 ]
  %46 = phi i32 [ %4, %20 ], [ %5, %27 ], [ 1, %33 ], [ 1, %39 ]
  %47 = phi i32 [ %5, %20 ], [ 1, %27 ], [ 1, %33 ], [ 1, %39 ]
  %48 = phi i32 [ %9, %20 ], [ %24, %27 ], [ %24, %33 ], [ %24, %39 ]
  %49 = phi i32 [ %8, %20 ], [ %9, %27 ], [ %24, %33 ], [ %24, %39 ]
  %50 = phi i32 [ %23, %20 ], [ %30, %27 ], [ %36, %33 ], [ %42, %39 ]
  %51 = phi i32 [ %25, %20 ], [ %31, %27 ], [ %37, %33 ], [ %43, %39 ]
  tail call void @llvm.tpc.st.l.i32(i32 1040, i32 %51, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1048, i32 %45, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1056, i32 %46, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1064, i32 %47, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1072, i32 1, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1052, i32 %50, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1060, i32 %49, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1068, i32 %48, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1076, i32 %24, i32 1, i1 true, i1 false) #4
  br label %52

52:                                               ; preds = %0, %44
  %53 = phi i32 [ %51, %44 ], [ %1, %0 ]
  %54 = phi i32 [ %45, %44 ], [ %2, %0 ]
  %55 = phi i32 [ %46, %44 ], [ %3, %0 ]
  %56 = phi i32 [ %47, %44 ], [ %4, %0 ]
  %57 = phi i32 [ 1, %44 ], [ %5, %0 ]
  %58 = icmp eq i32 %10, %15
  br i1 %58, label %59, label %91

59:                                               ; preds = %52
  %60 = icmp eq i32 %16, 0
  %61 = mul nsw i32 %10, %11
  %62 = select i1 %60, i32 %61, i32 %16
  %63 = mul nsw i32 %18, %14
  %64 = mul nsw i32 %11, %10
  %65 = icmp eq i32 %64, %62
  br i1 %65, label %66, label %83

66:                                               ; preds = %59
  %67 = icmp eq i32 %17, 0
  %68 = mul nsw i32 %62, %12
  %69 = select i1 %67, i32 %68, i32 %17
  %70 = mul nsw i32 %12, %62
  %71 = icmp eq i32 %70, %69
  br i1 %71, label %72, label %83

72:                                               ; preds = %66
  %73 = icmp eq i32 %18, 0
  %74 = mul nsw i32 %69, %13
  %75 = select i1 %73, i32 %74, i32 %18
  %76 = mul nsw i32 %13, %69
  %77 = icmp eq i32 %76, %75
  br i1 %77, label %78, label %83

78:                                               ; preds = %72
  %79 = icmp eq i32 %63, 0
  %80 = mul nsw i32 %75, %14
  %81 = select i1 %79, i32 %80, i32 %63
  %82 = mul nsw i32 %14, %75
  br label %83

83:                                               ; preds = %78, %72, %66, %59
  %84 = phi i32 [ %18, %59 ], [ %63, %66 ], [ %63, %72 ], [ %63, %78 ]
  %85 = phi i32 [ %17, %59 ], [ %18, %66 ], [ %63, %72 ], [ %63, %78 ]
  %86 = phi i32 [ %14, %59 ], [ 1, %66 ], [ 1, %72 ], [ 1, %78 ]
  %87 = phi i32 [ %13, %59 ], [ %14, %66 ], [ 1, %72 ], [ 1, %78 ]
  %88 = phi i32 [ %12, %59 ], [ %13, %66 ], [ %14, %72 ], [ 1, %78 ]
  %89 = phi i32 [ %62, %59 ], [ %69, %66 ], [ %75, %72 ], [ %81, %78 ]
  %90 = phi i32 [ %64, %59 ], [ %70, %66 ], [ %76, %72 ], [ %82, %78 ]
  tail call void @llvm.tpc.st.l.i32(i32 1096, i32 %90, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1104, i32 %88, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1112, i32 %87, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1120, i32 %86, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1128, i32 1, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1108, i32 %89, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1116, i32 %85, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1124, i32 %84, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1132, i32 %84, i32 1, i1 true, i1 false) #4
  br label %91

91:                                               ; preds = %52, %83
  %92 = phi i32 [ 1, %83 ], [ %14, %52 ]
  %93 = phi i32 [ %86, %83 ], [ %13, %52 ]
  %94 = phi i32 [ %87, %83 ], [ %12, %52 ]
  %95 = phi i32 [ %88, %83 ], [ %11, %52 ]
  %96 = phi i32 [ %90, %83 ], [ %10, %52 ]
  %97 = icmp eq i32 %53, 1
  %98 = zext i1 %97 to i32
  %99 = icmp eq i32 %54, 1
  %100 = select i1 %99, i32 2, i32 0
  %101 = icmp eq i32 %55, 1
  %102 = select i1 %101, i32 4, i32 0
  %103 = icmp eq i32 %56, 1
  %104 = select i1 %103, i32 8, i32 0
  %105 = icmp eq i32 %57, 1
  %106 = select i1 %105, i32 16, i32 0
  %107 = or i32 %104, %106
  %108 = or i32 %107, %102
  %109 = or i32 %108, %100
  %110 = or i32 %109, %98
  %111 = icmp eq i32 %96, 1
  %112 = zext i1 %111 to i32
  %113 = icmp eq i32 %95, 1
  %114 = select i1 %113, i32 2, i32 0
  %115 = or i32 %114, %112
  %116 = icmp eq i32 %94, 1
  %117 = select i1 %116, i32 4, i32 0
  %118 = or i32 %115, %117
  %119 = icmp eq i32 %93, 1
  %120 = select i1 %119, i32 8, i32 0
  %121 = or i32 %118, %120
  %122 = icmp eq i32 %92, 1
  %123 = select i1 %122, i32 16, i32 0
  %124 = or i32 %121, %123
  %125 = mul nsw i32 %53, %54
  %126 = mul nsw i32 %125, %55
  %127 = mul nsw i32 %126, %56
  %128 = mul nsw i32 %95, %96
  %129 = mul nsw i32 %128, %94
  %130 = mul nsw i32 %129, %93
  %131 = mul nsw i32 %127, %57
  %132 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %133 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %134 = add <5 x i32> %133, %132
  %135 = ashr i32 %131, 3
  %136 = extractelement <5 x i32> %132, i32 0
  %137 = mul nsw i32 %136, %135
  %138 = extractelement <5 x i32> %134, i32 0
  %139 = mul nsw i32 %138, %135
  %140 = icmp eq i32 %138, 8
  %141 = select i1 %140, i32 %131, i32 %139
  %142 = icmp slt i32 %57, 2
  %143 = icmp slt i32 %137, %127
  %144 = or i1 %142, %143
  br i1 %144, label %155, label %145

145:                                              ; preds = %91
  br label %146

146:                                              ; preds = %145, %146
  %147 = phi i32 [ %150, %146 ], [ %137, %145 ]
  %148 = phi <5 x i32> [ %149, %146 ], [ zeroinitializer, %145 ]
  %149 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %148, i32 1, i32 16, i8 2, i32 0, <5 x i32> %148, i1 true, i1 false)
  %150 = sub nsw i32 %147, %127
  %151 = icmp slt i32 %150, %127
  br i1 %151, label %152, label %146

152:                                              ; preds = %146
  %153 = phi <5 x i32> [ %149, %146 ]
  %154 = phi i32 [ %150, %146 ]
  br label %155

155:                                              ; preds = %152, %91
  %156 = phi <5 x i32> [ zeroinitializer, %91 ], [ %153, %152 ]
  %157 = phi i32 [ %137, %91 ], [ %154, %152 ]
  %158 = icmp slt i32 %56, 2
  %159 = icmp slt i32 %157, %126
  %160 = or i1 %158, %159
  br i1 %160, label %171, label %161

161:                                              ; preds = %155
  br label %162

162:                                              ; preds = %161, %162
  %163 = phi i32 [ %166, %162 ], [ %157, %161 ]
  %164 = phi <5 x i32> [ %165, %162 ], [ %156, %161 ]
  %165 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %164, i32 1, i32 8, i8 2, i32 0, <5 x i32> %164, i1 true, i1 false)
  %166 = sub nsw i32 %163, %126
  %167 = icmp slt i32 %166, %126
  br i1 %167, label %168, label %162

168:                                              ; preds = %162
  %169 = phi <5 x i32> [ %165, %162 ]
  %170 = phi i32 [ %166, %162 ]
  br label %171

171:                                              ; preds = %168, %155
  %172 = phi <5 x i32> [ %156, %155 ], [ %169, %168 ]
  %173 = phi i32 [ %157, %155 ], [ %170, %168 ]
  %174 = icmp slt i32 %55, 2
  %175 = icmp slt i32 %173, %125
  %176 = or i1 %174, %175
  br i1 %176, label %187, label %177

177:                                              ; preds = %171
  br label %178

178:                                              ; preds = %177, %178
  %179 = phi i32 [ %182, %178 ], [ %173, %177 ]
  %180 = phi <5 x i32> [ %181, %178 ], [ %172, %177 ]
  %181 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %180, i32 1, i32 4, i8 2, i32 0, <5 x i32> %180, i1 true, i1 false)
  %182 = sub nsw i32 %179, %125
  %183 = icmp slt i32 %182, %125
  br i1 %183, label %184, label %178

184:                                              ; preds = %178
  %185 = phi <5 x i32> [ %181, %178 ]
  %186 = phi i32 [ %182, %178 ]
  br label %187

187:                                              ; preds = %184, %171
  %188 = phi <5 x i32> [ %172, %171 ], [ %185, %184 ]
  %189 = phi i32 [ %173, %171 ], [ %186, %184 ]
  %190 = icmp slt i32 %54, 2
  %191 = icmp slt i32 %189, %53
  %192 = or i1 %190, %191
  br i1 %192, label %203, label %193

193:                                              ; preds = %187
  br label %194

194:                                              ; preds = %193, %194
  %195 = phi i32 [ %198, %194 ], [ %189, %193 ]
  %196 = phi <5 x i32> [ %197, %194 ], [ %188, %193 ]
  %197 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %196, i32 1, i32 2, i8 2, i32 0, <5 x i32> %196, i1 true, i1 false)
  %198 = sub nsw i32 %195, %53
  %199 = icmp slt i32 %198, %53
  br i1 %199, label %200, label %194

200:                                              ; preds = %194
  %201 = phi <5 x i32> [ %197, %194 ]
  %202 = phi i32 [ %198, %194 ]
  br label %203

203:                                              ; preds = %200, %187
  %204 = phi <5 x i32> [ %188, %187 ], [ %201, %200 ]
  %205 = phi i32 [ %189, %187 ], [ %202, %200 ]
  %206 = insertelement <5 x i32> %204, i32 %205, i32 0
  %207 = extractelement <5 x i32> %204, i32 1
  %208 = extractelement <5 x i32> %204, i32 2
  %209 = extractelement <5 x i32> %204, i32 3
  %210 = extractelement <5 x i32> %204, i32 4
  %211 = icmp slt i32 %92, 2
  %212 = icmp slt i32 %137, %130
  %213 = or i1 %211, %212
  br i1 %213, label %224, label %214

214:                                              ; preds = %203
  br label %215

215:                                              ; preds = %214, %215
  %216 = phi i32 [ %219, %215 ], [ %137, %214 ]
  %217 = phi <5 x i32> [ %218, %215 ], [ zeroinitializer, %214 ]
  %218 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %217, i32 1, i32 16, i8 2, i32 0, <5 x i32> %217, i1 true, i1 false)
  %219 = sub nsw i32 %216, %130
  %220 = icmp slt i32 %219, %130
  br i1 %220, label %221, label %215

221:                                              ; preds = %215
  %222 = phi <5 x i32> [ %218, %215 ]
  %223 = phi i32 [ %219, %215 ]
  br label %224

224:                                              ; preds = %221, %203
  %225 = phi <5 x i32> [ zeroinitializer, %203 ], [ %222, %221 ]
  %226 = phi i32 [ %137, %203 ], [ %223, %221 ]
  %227 = icmp slt i32 %93, 2
  %228 = icmp slt i32 %226, %129
  %229 = or i1 %227, %228
  br i1 %229, label %240, label %230

230:                                              ; preds = %224
  br label %231

231:                                              ; preds = %230, %231
  %232 = phi i32 [ %235, %231 ], [ %226, %230 ]
  %233 = phi <5 x i32> [ %234, %231 ], [ %225, %230 ]
  %234 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %233, i32 1, i32 8, i8 2, i32 0, <5 x i32> %233, i1 true, i1 false)
  %235 = sub nsw i32 %232, %129
  %236 = icmp slt i32 %235, %129
  br i1 %236, label %237, label %231

237:                                              ; preds = %231
  %238 = phi <5 x i32> [ %234, %231 ]
  %239 = phi i32 [ %235, %231 ]
  br label %240

240:                                              ; preds = %237, %224
  %241 = phi <5 x i32> [ %225, %224 ], [ %238, %237 ]
  %242 = phi i32 [ %226, %224 ], [ %239, %237 ]
  %243 = icmp slt i32 %94, 2
  %244 = icmp slt i32 %242, %128
  %245 = or i1 %243, %244
  br i1 %245, label %256, label %246

246:                                              ; preds = %240
  br label %247

247:                                              ; preds = %246, %247
  %248 = phi i32 [ %251, %247 ], [ %242, %246 ]
  %249 = phi <5 x i32> [ %250, %247 ], [ %241, %246 ]
  %250 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %249, i32 1, i32 4, i8 2, i32 0, <5 x i32> %249, i1 true, i1 false)
  %251 = sub nsw i32 %248, %128
  %252 = icmp slt i32 %251, %128
  br i1 %252, label %253, label %247

253:                                              ; preds = %247
  %254 = phi <5 x i32> [ %250, %247 ]
  %255 = phi i32 [ %251, %247 ]
  br label %256

256:                                              ; preds = %253, %240
  %257 = phi <5 x i32> [ %241, %240 ], [ %254, %253 ]
  %258 = phi i32 [ %242, %240 ], [ %255, %253 ]
  %259 = icmp slt i32 %95, 2
  %260 = icmp slt i32 %258, %96
  %261 = or i1 %259, %260
  br i1 %261, label %272, label %262

262:                                              ; preds = %256
  br label %263

263:                                              ; preds = %262, %263
  %264 = phi i32 [ %267, %263 ], [ %258, %262 ]
  %265 = phi <5 x i32> [ %266, %263 ], [ %257, %262 ]
  %266 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %265, i32 1, i32 2, i8 2, i32 0, <5 x i32> %265, i1 true, i1 false)
  %267 = sub nsw i32 %264, %96
  %268 = icmp slt i32 %267, %96
  br i1 %268, label %269, label %263

269:                                              ; preds = %263
  %270 = phi <5 x i32> [ %266, %263 ]
  %271 = phi i32 [ %267, %263 ]
  br label %272

272:                                              ; preds = %269, %256
  %273 = phi <5 x i32> [ %257, %256 ], [ %270, %269 ]
  %274 = phi i32 [ %258, %256 ], [ %271, %269 ]
  %275 = insertelement <5 x i32> %273, i32 %274, i32 0
  %276 = extractelement <5 x i32> %273, i32 1
  %277 = extractelement <5 x i32> %273, i32 2
  %278 = extractelement <5 x i32> %273, i32 3
  %279 = extractelement <5 x i32> %273, i32 4
  %280 = icmp eq i32 %110, 28
  %281 = icmp eq i32 %110, 26
  %282 = or i1 %280, %281
  %283 = icmp eq i32 %124, 30
  %284 = and i1 %282, %283
  %285 = icmp slt i32 %53, 256
  %286 = and i1 %285, %284
  br i1 %286, label %287, label %330

287:                                              ; preds = %272
  %288 = zext i1 %280 to i32
  %289 = insertelement <5 x i32> <i32 0, i32 undef, i32 undef, i32 0, i32 0>, i32 %288, i32 1
  %290 = zext i1 %281 to i32
  %291 = insertelement <5 x i32> %289, i32 %290, i32 2
  %292 = insertelement <5 x i32> %291, i32 -128, i32 0
  %293 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %53, i32 0
  %294 = add nsw i32 %53, -128
  %295 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %294, i32 0
  %296 = icmp eq i32 %205, 0
  br i1 %296, label %381, label %297

297:                                              ; preds = %287
  %298 = sub nsw i32 %53, %205
  %299 = icmp slt i32 %298, 127
  %300 = select i1 %299, i32 %298, i32 127
  %301 = icmp sgt i32 %298, 0
  br i1 %301, label %302, label %324

302:                                              ; preds = %297
  br label %303

303:                                              ; preds = %302, %303
  %304 = phi i32 [ %318, %303 ], [ %300, %302 ]
  %305 = phi i32 [ %314, %303 ], [ %205, %302 ]
  %306 = phi <5 x i32> [ %313, %303 ], [ %275, %302 ]
  %307 = phi <5 x i32> [ %312, %303 ], [ %206, %302 ]
  %308 = phi i32 [ %315, %303 ], [ %137, %302 ]
  %309 = add nuw nsw i32 %304, 255
  %310 = and i32 %309, 255
  %311 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.partial.v128bf16.i1(<5 x i32> %307, i8 0, i32 %310, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1 false)
  %312 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %307, i32 %304, i32 1, i8 2, i32 0, <5 x i32> %307, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32> %306, i8 1, <128 x bfloat> %311, i32 %310, i32 1, i1 true, i1 false)
  %313 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %306, i32 %304, i32 1, i8 2, i32 0, <5 x i32> %306, i1 true, i1 false)
  %314 = add nsw i32 %304, %305
  %315 = add nsw i32 %304, %308
  %316 = sub nsw i32 %53, %314
  %317 = icmp slt i32 %316, 127
  %318 = select i1 %317, i32 %316, i32 127
  %319 = icmp sgt i32 %316, 0
  br i1 %319, label %303, label %320

320:                                              ; preds = %303
  %321 = phi <5 x i32> [ %312, %303 ]
  %322 = phi <5 x i32> [ %313, %303 ]
  %323 = phi i32 [ %315, %303 ]
  br label %324

324:                                              ; preds = %320, %297
  %325 = phi i32 [ %137, %297 ], [ %323, %320 ]
  %326 = phi <5 x i32> [ %206, %297 ], [ %321, %320 ]
  %327 = phi <5 x i32> [ %275, %297 ], [ %322, %320 ]
  %328 = insertelement <5 x i32> %326, i32 0, i32 0
  %329 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %328, <5 x i32> %291, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  br label %381

330:                                              ; preds = %272
  %331 = icmp eq i32 %110, 30
  br i1 %331, label %332, label %455

332:                                              ; preds = %330
  %333 = icmp eq i32 %124, 28
  %334 = icmp eq i32 %124, 26
  %335 = or i1 %333, %334
  %336 = icmp slt i32 %96, 256
  %337 = and i1 %336, %335
  br i1 %337, label %338, label %381

338:                                              ; preds = %332
  %339 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %96, i32 0
  %340 = add nsw i32 %96, -128
  %341 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %340, i32 0
  %342 = zext i1 %333 to i32
  %343 = insertelement <5 x i32> <i32 0, i32 undef, i32 undef, i32 0, i32 0>, i32 %342, i32 1
  %344 = zext i1 %334 to i32
  %345 = insertelement <5 x i32> %343, i32 %344, i32 2
  %346 = insertelement <5 x i32> %345, i32 -128, i32 0
  %347 = icmp eq i32 %274, 0
  br i1 %347, label %381, label %348

348:                                              ; preds = %338
  %349 = sub nsw i32 %96, %274
  %350 = icmp slt i32 %349, 127
  %351 = select i1 %350, i32 %349, i32 127
  %352 = icmp sgt i32 %349, 0
  br i1 %352, label %353, label %375

353:                                              ; preds = %348
  br label %354

354:                                              ; preds = %353, %354
  %355 = phi i32 [ %369, %354 ], [ %351, %353 ]
  %356 = phi i32 [ %365, %354 ], [ %274, %353 ]
  %357 = phi <5 x i32> [ %364, %354 ], [ %275, %353 ]
  %358 = phi <5 x i32> [ %363, %354 ], [ %206, %353 ]
  %359 = phi i32 [ %366, %354 ], [ %137, %353 ]
  %360 = add nuw nsw i32 %355, 255
  %361 = and i32 %360, 255
  %362 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.partial.v128bf16.i1(<5 x i32> %358, i8 0, i32 %361, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1 false)
  %363 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %358, i32 %355, i32 1, i8 2, i32 0, <5 x i32> %358, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32> %357, i8 1, <128 x bfloat> %362, i32 %361, i32 1, i1 true, i1 false)
  %364 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %357, i32 %355, i32 1, i8 2, i32 0, <5 x i32> %357, i1 true, i1 false)
  %365 = add nsw i32 %355, %356
  %366 = add nsw i32 %355, %359
  %367 = sub nsw i32 %96, %365
  %368 = icmp slt i32 %367, 127
  %369 = select i1 %368, i32 %367, i32 127
  %370 = icmp sgt i32 %367, 0
  br i1 %370, label %354, label %371

371:                                              ; preds = %354
  %372 = phi <5 x i32> [ %363, %354 ]
  %373 = phi <5 x i32> [ %364, %354 ]
  %374 = phi i32 [ %366, %354 ]
  br label %375

375:                                              ; preds = %371, %348
  %376 = phi i32 [ %137, %348 ], [ %374, %371 ]
  %377 = phi <5 x i32> [ %206, %348 ], [ %372, %371 ]
  %378 = phi <5 x i32> [ %275, %348 ], [ %373, %371 ]
  %379 = insertelement <5 x i32> %378, i32 0, i32 0
  %380 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %379, <5 x i32> %345, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  br label %381

381:                                              ; preds = %287, %338, %332, %375, %324
  %382 = phi i32 [ %325, %324 ], [ %137, %287 ], [ %376, %375 ], [ %137, %338 ], [ %137, %332 ]
  %383 = phi <5 x i32> [ %329, %324 ], [ %206, %287 ], [ %377, %375 ], [ %206, %338 ], [ %206, %332 ]
  %384 = phi <5 x i32> [ %327, %324 ], [ %275, %287 ], [ %380, %375 ], [ %275, %338 ], [ %275, %332 ]
  %385 = phi i32 [ 0, %324 ], [ 0, %287 ], [ %205, %375 ], [ %205, %338 ], [ %205, %332 ]
  %386 = phi i32 [ %274, %324 ], [ %274, %287 ], [ 0, %375 ], [ 0, %338 ], [ %274, %332 ]
  %387 = phi i32 [ 2, %324 ], [ 2, %287 ], [ 2, %375 ], [ 2, %338 ], [ 0, %332 ]
  %388 = phi i32 [ %53, %324 ], [ %53, %287 ], [ %96, %375 ], [ %96, %338 ], [ 0, %332 ]
  %389 = phi <5 x i32> [ %291, %324 ], [ %291, %287 ], [ %339, %375 ], [ %339, %338 ], [ zeroinitializer, %332 ]
  %390 = phi <5 x i32> [ %292, %324 ], [ %292, %287 ], [ %341, %375 ], [ %341, %338 ], [ zeroinitializer, %332 ]
  %391 = phi <5 x i32> [ %293, %324 ], [ %293, %287 ], [ %345, %375 ], [ %345, %338 ], [ zeroinitializer, %332 ]
  %392 = phi <5 x i32> [ %295, %324 ], [ %295, %287 ], [ %346, %375 ], [ %346, %338 ], [ zeroinitializer, %332 ]
  %393 = icmp eq i32 %110, 30
  %394 = and i1 %393, %283
  br i1 %394, label %395, label %407

395:                                              ; preds = %381
  %396 = icmp slt i32 %382, %141
  br i1 %396, label %397, label %599

397:                                              ; preds = %395
  br label %398

398:                                              ; preds = %397, %398
  %399 = phi i32 [ %405, %398 ], [ %382, %397 ]
  %400 = phi <5 x i32> [ %404, %398 ], [ %384, %397 ]
  %401 = phi <5 x i32> [ %403, %398 ], [ %383, %397 ]
  %402 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %401, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %403 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %401, i32 128, i32 1, i8 2, i32 0, <5 x i32> %401, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %400, i8 1, <128 x bfloat> %402, i32 0, i1 true, i1 false)
  %404 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %400, i32 128, i32 1, i8 2, i32 0, <5 x i32> %400, i1 true, i1 false)
  %405 = add nsw i32 %399, 128
  %406 = icmp slt i32 %405, %141
  br i1 %406, label %398, label %594, !llvm.loop !4

407:                                              ; preds = %381
  %408 = icmp eq i32 %387, 0
  br i1 %408, label %455, label %409

409:                                              ; preds = %407
  %410 = icmp slt i32 %388, 128
  br i1 %410, label %411, label %425

411:                                              ; preds = %409
  %412 = icmp slt i32 %382, %141
  br i1 %412, label %413, label %599

413:                                              ; preds = %411
  %414 = add nsw i32 %388, 255
  %415 = and i32 %414, 255
  br label %416

416:                                              ; preds = %413, %416
  %417 = phi i32 [ %382, %413 ], [ %423, %416 ]
  %418 = phi <5 x i32> [ %384, %413 ], [ %422, %416 ]
  %419 = phi <5 x i32> [ %383, %413 ], [ %421, %416 ]
  %420 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.partial.v128bf16.i1(<5 x i32> %419, i8 0, i32 %415, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1 false)
  %421 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %419, <5 x i32> %389, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32> %418, i8 1, <128 x bfloat> %420, i32 %415, i32 1, i1 true, i1 false)
  %422 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %418, <5 x i32> %391, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  %423 = add nsw i32 %417, %388
  %424 = icmp slt i32 %423, %141
  br i1 %424, label %416, label %596, !llvm.loop !7

425:                                              ; preds = %409
  %426 = icmp eq i32 %388, 128
  %427 = icmp slt i32 %382, %141
  br i1 %426, label %432, label %428

428:                                              ; preds = %425
  br i1 %427, label %429, label %599

429:                                              ; preds = %428
  %430 = add nuw i32 %388, 127
  %431 = and i32 %430, 255
  br label %443

432:                                              ; preds = %425
  br i1 %427, label %433, label %599

433:                                              ; preds = %432
  br label %434

434:                                              ; preds = %433, %434
  %435 = phi i32 [ %441, %434 ], [ %382, %433 ]
  %436 = phi <5 x i32> [ %440, %434 ], [ %384, %433 ]
  %437 = phi <5 x i32> [ %439, %434 ], [ %383, %433 ]
  %438 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %437, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %439 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %437, <5 x i32> %389, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %436, i8 1, <128 x bfloat> %438, i32 0, i1 true, i1 false)
  %440 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %436, <5 x i32> %391, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  %441 = add nsw i32 %435, 128
  %442 = icmp slt i32 %441, %141
  br i1 %442, label %434, label %597, !llvm.loop !8

443:                                              ; preds = %429, %443
  %444 = phi i32 [ %382, %429 ], [ %453, %443 ]
  %445 = phi <5 x i32> [ %384, %429 ], [ %452, %443 ]
  %446 = phi <5 x i32> [ %383, %429 ], [ %451, %443 ]
  %447 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %446, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %448 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %446, i32 128, i32 1, i8 2, i32 0, <5 x i32> %446, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %445, i8 1, <128 x bfloat> %447, i32 0, i1 true, i1 false)
  %449 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %445, i32 128, i32 1, i8 2, i32 0, <5 x i32> %445, i1 true, i1 false)
  %450 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.partial.v128bf16.i1(<5 x i32> %448, i8 0, i32 %431, i32 1, <128 x bfloat> zeroinitializer, i1 true, i1 false)
  %451 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %448, <5 x i32> %390, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32> %449, i8 1, <128 x bfloat> %450, i32 %431, i32 1, i1 true, i1 false)
  %452 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> %449, <5 x i32> %392, i32 7, i8 2, i32 0, <5 x i32> zeroinitializer, i1 true, i1 false)
  %453 = add nsw i32 %444, %388
  %454 = icmp slt i32 %453, %141
  br i1 %454, label %443, label %598, !llvm.loop !9

455:                                              ; preds = %330, %407
  %456 = phi i32 [ %382, %407 ], [ %137, %330 ]
  %457 = phi <5 x i32> [ %383, %407 ], [ %206, %330 ]
  %458 = phi <5 x i32> [ %384, %407 ], [ %275, %330 ]
  %459 = phi i32 [ %385, %407 ], [ %205, %330 ]
  %460 = phi i32 [ %386, %407 ], [ %274, %330 ]
  %461 = extractelement <5 x i32> %457, i32 0
  %462 = add nsw i32 %456, %53
  %463 = sub i32 %462, %461
  %464 = icmp slt i32 %456, %141
  br i1 %464, label %465, label %599

465:                                              ; preds = %455
  br label %466

466:                                              ; preds = %465, %492
  %467 = phi i32 [ %518, %492 ], [ %463, %465 ]
  %468 = phi i32 [ %498, %492 ], [ %279, %465 ]
  %469 = phi i32 [ %497, %492 ], [ %278, %465 ]
  %470 = phi i32 [ %496, %492 ], [ %277, %465 ]
  %471 = phi i32 [ %495, %492 ], [ %276, %465 ]
  %472 = phi i32 [ %494, %492 ], [ %460, %465 ]
  %473 = phi i32 [ %516, %492 ], [ %210, %465 ]
  %474 = phi i32 [ %514, %492 ], [ %209, %465 ]
  %475 = phi i32 [ %510, %492 ], [ %208, %465 ]
  %476 = phi i32 [ %506, %492 ], [ %207, %465 ]
  %477 = phi i32 [ %502, %492 ], [ %459, %465 ]
  %478 = phi <5 x i32> [ %493, %492 ], [ %458, %465 ]
  %479 = phi <5 x i32> [ %517, %492 ], [ %457, %465 ]
  %480 = phi i32 [ %467, %492 ], [ %456, %465 ]
  %481 = icmp slt i32 %467, %141
  %482 = select i1 %481, i32 %467, i32 %141
  %483 = icmp sgt i32 %482, %480
  br i1 %483, label %484, label %492

484:                                              ; preds = %466
  br label %519

485:                                              ; preds = %585
  %486 = phi <5 x i32> [ %586, %585 ]
  %487 = phi i32 [ %587, %585 ]
  %488 = phi i32 [ %588, %585 ]
  %489 = phi i32 [ %589, %585 ]
  %490 = phi i32 [ %590, %585 ]
  %491 = phi i32 [ %591, %585 ]
  br label %492

492:                                              ; preds = %485, %466
  %493 = phi <5 x i32> [ %478, %466 ], [ %486, %485 ]
  %494 = phi i32 [ %472, %466 ], [ %487, %485 ]
  %495 = phi i32 [ %471, %466 ], [ %488, %485 ]
  %496 = phi i32 [ %470, %466 ], [ %489, %485 ]
  %497 = phi i32 [ %469, %466 ], [ %490, %485 ]
  %498 = phi i32 [ %468, %466 ], [ %491, %485 ]
  %499 = sub nsw i32 %467, %480
  %500 = add nsw i32 %499, %477
  %501 = icmp sge i32 %500, %53
  %502 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %500, i1 %501, i1 false)
  %503 = insertelement <5 x i32> undef, i32 %502, i32 0
  %504 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %476, i32 1, i8 2, i32 0, i32 %476, i1 %501, i1 false)
  %505 = icmp sge i32 %504, %54
  %506 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %504, i1 %505, i1 false)
  %507 = insertelement <5 x i32> %503, i32 %506, i32 1
  %508 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %475, i32 1, i8 2, i32 0, i32 %475, i1 %505, i1 false)
  %509 = icmp sge i32 %508, %55
  %510 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %508, i1 %509, i1 false)
  %511 = insertelement <5 x i32> %507, i32 %510, i32 2
  %512 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %474, i32 1, i8 2, i32 0, i32 %474, i1 %509, i1 false)
  %513 = icmp sge i32 %512, %56
  %514 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %512, i1 %513, i1 false)
  %515 = insertelement <5 x i32> %511, i32 %514, i32 3
  %516 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %473, i32 1, i8 2, i32 0, i32 %473, i1 %513, i1 false)
  %517 = insertelement <5 x i32> %515, i32 %516, i32 4
  %518 = add nsw i32 %467, %53
  br i1 %481, label %466, label %595

519:                                              ; preds = %484, %585
  %520 = phi i32 [ %592, %585 ], [ %480, %484 ]
  %521 = phi i32 [ %591, %585 ], [ %468, %484 ]
  %522 = phi i32 [ %590, %585 ], [ %469, %484 ]
  %523 = phi i32 [ %589, %585 ], [ %470, %484 ]
  %524 = phi i32 [ %588, %585 ], [ %471, %484 ]
  %525 = phi i32 [ %587, %585 ], [ %472, %484 ]
  %526 = phi <5 x i32> [ %586, %585 ], [ %478, %484 ]
  %527 = phi <5 x i32> [ %532, %585 ], [ %479, %484 ]
  %528 = sub nsw i32 %482, %520
  %529 = icmp slt i32 %528, 128
  %530 = select i1 %529, i32 %528, i32 128
  %531 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %527, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %532 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %527, i32 128, i32 1, i8 2, i32 0, <5 x i32> %527, i1 true, i1 false)
  %533 = sub nsw i32 %96, %525
  %534 = icmp slt i32 %530, %533
  %535 = select i1 %534, i32 %530, i32 %533
  %536 = icmp sgt i32 %528, 0
  br i1 %536, label %537, label %585

537:                                              ; preds = %519
  br label %538

538:                                              ; preds = %537, %538
  %539 = phi i32 [ %576, %538 ], [ %535, %537 ]
  %540 = phi i32 [ %573, %538 ], [ %530, %537 ]
  %541 = phi i32 [ %572, %538 ], [ 0, %537 ]
  %542 = phi i32 [ %570, %538 ], [ %521, %537 ]
  %543 = phi i32 [ %568, %538 ], [ %522, %537 ]
  %544 = phi i32 [ %564, %538 ], [ %523, %537 ]
  %545 = phi i32 [ %560, %538 ], [ %524, %537 ]
  %546 = phi i32 [ %556, %538 ], [ %525, %537 ]
  %547 = phi <5 x i32> [ %571, %538 ], [ %526, %537 ]
  %548 = icmp ne i32 %539, 128
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %547, i8 1, <128 x bfloat> %531, i32 0, i1 %548, i1 true)
  %549 = add i32 %539, 255
  %550 = and i32 %549, 255
  %551 = shl i32 %541, 8
  %552 = and i32 %551, 65280
  %553 = or i32 %550, %552
  tail call void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32> %547, i8 1, <128 x bfloat> %531, i32 %553, i32 1, i1 %548, i1 false)
  %554 = add nsw i32 %539, %546
  %555 = icmp sge i32 %554, %96
  %556 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %554, i1 %555, i1 false)
  %557 = insertelement <5 x i32> undef, i32 %556, i32 0
  %558 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %545, i32 1, i8 2, i32 0, i32 %545, i1 %555, i1 false)
  %559 = icmp sge i32 %558, %95
  %560 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %558, i1 %559, i1 false)
  %561 = insertelement <5 x i32> %557, i32 %560, i32 1
  %562 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %544, i32 1, i8 2, i32 0, i32 %544, i1 %559, i1 false)
  %563 = icmp sge i32 %562, %94
  %564 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %562, i1 %563, i1 false)
  %565 = insertelement <5 x i32> %561, i32 %564, i32 2
  %566 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %543, i32 1, i8 2, i32 0, i32 %543, i1 %563, i1 false)
  %567 = icmp sge i32 %566, %93
  %568 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 0, i8 2, i32 0, i32 %566, i1 %567, i1 false)
  %569 = insertelement <5 x i32> %565, i32 %568, i32 3
  %570 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 %542, i32 1, i8 2, i32 0, i32 %542, i1 %567, i1 false)
  %571 = insertelement <5 x i32> %569, i32 %570, i32 4
  %572 = add nsw i32 %539, %541
  %573 = sub nsw i32 %540, %539
  %574 = sub nsw i32 %96, %556
  %575 = icmp slt i32 %573, %574
  %576 = select i1 %575, i32 %573, i32 %574
  %577 = icmp sgt i32 %573, 0
  br i1 %577, label %538, label %578, !llvm.loop !10

578:                                              ; preds = %538
  %579 = phi i32 [ %556, %538 ]
  %580 = phi i32 [ %560, %538 ]
  %581 = phi i32 [ %564, %538 ]
  %582 = phi i32 [ %568, %538 ]
  %583 = phi i32 [ %570, %538 ]
  %584 = phi <5 x i32> [ %571, %538 ]
  br label %585

585:                                              ; preds = %578, %519
  %586 = phi <5 x i32> [ %526, %519 ], [ %584, %578 ]
  %587 = phi i32 [ %525, %519 ], [ %579, %578 ]
  %588 = phi i32 [ %524, %519 ], [ %580, %578 ]
  %589 = phi i32 [ %523, %519 ], [ %581, %578 ]
  %590 = phi i32 [ %522, %519 ], [ %582, %578 ]
  %591 = phi i32 [ %521, %519 ], [ %583, %578 ]
  %592 = add nsw i32 %520, 128
  %593 = icmp sgt i32 %482, %592
  br i1 %593, label %519, label %485

594:                                              ; preds = %398
  br label %599

595:                                              ; preds = %492
  br label %599

596:                                              ; preds = %416
  br label %599

597:                                              ; preds = %434
  br label %599

598:                                              ; preds = %443
  br label %599

599:                                              ; preds = %598, %597, %596, %595, %594, %428, %432, %411, %455, %395
  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.partial.v128bf16.i1(<5 x i32>, i8, i32, i32, <128 x bfloat>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.partial.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32>, <5 x i32>, i32, i8, i32, <5 x i32>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare i32 @llvm.tpc.mov.i32.i32.i1(i32, i8, i32, i32, i1, i1) #1

; Function Attrs: nounwind readnone
declare i32 @llvm.tpc.add.i32.i32.i32.i1(i32, i32, i8, i32, i32, i1, i1) #1

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #3

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind readonly }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = !{!"{0, 1}", !"{0, 1}"}
!4 = distinct !{!4, !5, !6}
!5 = !{!"llvm.loop.machine.unroll.count", i32 4}
!6 = !{!"llvm.loop.taken", i1 true}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !6}
!10 = distinct !{!10, !6}
