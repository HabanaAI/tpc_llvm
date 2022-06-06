; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[2].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[3].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[2].[Output].{ 0, 0, 0, 0, 0 }[3].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './kernels/gaudi/gnmt/top_k_st1_dynamic_fwd_bf16.c'
source_filename = "./kernels/gaudi/gnmt/top_k_st1_dynamic_fwd_bf16.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

%struct._int64_float64_pair_t = type { <64 x i32>, <64 x float> }

@0 = external dso_local addrspace(2) global [37 x %struct._int64_float64_pair_t], section ".vldata"
@1 = external dso_local addrspace(2) global [37 x %struct._int64_float64_pair_t], section ".vldata"
@2 = external dso_local addrspace(2) global [37 x %struct._int64_float64_pair_t], section ".vldata"
@3 = external dso_local addrspace(2) global [37 x %struct._int64_float64_pair_t], section ".vldata"

; Function Attrs: nounwind
define dso_local void @main(i32 %0, i32 %1, i32 %2) local_unnamed_addr #0 !unroll_info !4 {
  %4 = bitcast [37 x %struct._int64_float64_pair_t] addrspace(2)* null to i8 addrspace(2)*
  %5 = bitcast [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %6 = bitcast [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %7 = bitcast [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %8 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #5
  %9 = tail call i32 @llvm.tpc.ld.l.i32(i32 1052, i32 1, i32 undef, i1 true, i1 false) #5
  %10 = shl i32 %1, 3
  %11 = add i32 %10, 1040
  %12 = tail call i32 @llvm.tpc.ld.l.i32(i32 %11, i32 1, i32 undef, i1 true, i1 false) #5
  %13 = add i32 %10, 1044
  %14 = tail call i32 @llvm.tpc.ld.l.i32(i32 %13, i32 1, i32 undef, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1048, i32 %12, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1052, i32 %14, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %11, i32 %8, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %13, i32 %9, i32 1, i1 true, i1 false) #5
  %15 = tail call i32 @llvm.tpc.ld.l.i32(i32 1160, i32 1, i32 undef, i1 true, i1 false) #5
  %16 = tail call i32 @llvm.tpc.ld.l.i32(i32 1164, i32 1, i32 undef, i1 true, i1 false) #5
  %17 = add i32 %10, 1152
  %18 = tail call i32 @llvm.tpc.ld.l.i32(i32 %17, i32 1, i32 undef, i1 true, i1 false) #5
  %19 = add i32 %10, 1156
  %20 = tail call i32 @llvm.tpc.ld.l.i32(i32 %19, i32 1, i32 undef, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1160, i32 %18, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1164, i32 %20, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %17, i32 %15, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %19, i32 %16, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1216, i32 %18, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 1220, i32 %20, i32 1, i1 true, i1 false) #5
  %21 = add i32 %10, 1208
  tail call void @llvm.tpc.st.l.i32(i32 %21, i32 %15, i32 1, i1 true, i1 false) #5
  %22 = add i32 %10, 1212
  tail call void @llvm.tpc.st.l.i32(i32 %22, i32 %16, i32 1, i1 true, i1 false) #5
  %23 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %24 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %25 = add <5 x i32> %24, %23
  %26 = extractelement <5 x i32> %23, i32 0
  %27 = shl nsw i32 %26, 7
  %28 = extractelement <5 x i32> %25, i32 0
  %29 = shl nsw i32 %28, 7
  %30 = extractelement <5 x i32> %23, i32 1
  %31 = extractelement <5 x i32> %25, i32 1
  %32 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #5
  %33 = extractelement <5 x i32> %23, i32 2
  %34 = extractelement <5 x i32> %25, i32 2
  %35 = extractelement <5 x i32> %23, i32 3
  %36 = extractelement <5 x i32> %25, i32 3
  %37 = extractelement <5 x i32> %23, i32 4
  %38 = extractelement <5 x i32> %25, i32 4
  %39 = tail call i32 @llvm.tpc.ld.l.i32(i32 1096, i32 1, i32 undef, i1 true, i1 false) #5
  %40 = tail call i32 @llvm.tpc.ld.l.i32(i32 1104, i32 1, i32 undef, i1 true, i1 false) #5
  %41 = icmp slt i32 %37, %38
  br i1 %41, label %42, label %78

42:                                               ; preds = %3
  %43 = icmp slt i32 %35, %36
  %44 = icmp slt i32 %33, %34
  %45 = icmp slt i32 %27, %29
  %46 = icmp slt i32 %30, %31
  %47 = icmp sgt i32 %39, 0
  %48 = add i32 %39, -1
  %49 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %48, i32 0
  %50 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %48, i32 1
  %51 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 0
  %52 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 1
  %53 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 0
  %54 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 1
  %55 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 0
  %56 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %48, i32 1
  %57 = bitcast <64 x float> addrspace(2)* %50 to <64 x i32> addrspace(2)*
  %58 = bitcast <64 x float> addrspace(2)* %52 to <64 x i32> addrspace(2)*
  %59 = bitcast <64 x float> addrspace(2)* %54 to <64 x i32> addrspace(2)*
  %60 = bitcast <64 x float> addrspace(2)* %56 to <64 x i32> addrspace(2)*
  %61 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 0
  %62 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %63 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %64 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %65 = add nsw i32 %39, -2
  %66 = icmp sgt i32 %39, 1
  %67 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %48, i32 0
  %68 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %48, i32 1
  %69 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %48, i32 0
  %70 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %48, i32 1
  %71 = bitcast <64 x float> addrspace(2)* %68 to <64 x i32> addrspace(2)*
  %72 = bitcast <64 x float> addrspace(2)* %70 to <64 x i32> addrspace(2)*
  %73 = and i32 %39, 3
  %74 = icmp ult i32 %48, 3
  %75 = sub nuw nsw i32 %39, %73
  %76 = icmp eq i32 %73, 0
  br label %79

77:                                               ; preds = %95
  br label %78

78:                                               ; preds = %77, %3
  ret void

79:                                               ; preds = %95, %42
  %80 = phi <5 x i32> [ zeroinitializer, %42 ], [ %99, %95 ]
  %81 = phi <5 x i32> [ zeroinitializer, %42 ], [ %98, %95 ]
  %82 = phi i32 [ %37, %42 ], [ %100, %95 ]
  %83 = phi <5 x i32> [ zeroinitializer, %42 ], [ %97, %95 ]
  %84 = phi <5 x i32> [ zeroinitializer, %42 ], [ %96, %95 ]
  %85 = insertelement <5 x i32> %80, i32 %82, i32 4
  %86 = insertelement <5 x i32> %81, i32 %82, i32 4
  %87 = insertelement <5 x i32> %83, i32 %82, i32 4
  %88 = insertelement <5 x i32> %84, i32 %82, i32 4
  br i1 %43, label %89, label %95

89:                                               ; preds = %79
  br label %102

90:                                               ; preds = %118
  %91 = phi <5 x i32> [ %119, %118 ]
  %92 = phi <5 x i32> [ %120, %118 ]
  %93 = phi <5 x i32> [ %121, %118 ]
  %94 = phi <5 x i32> [ %122, %118 ]
  br label %95

95:                                               ; preds = %90, %79
  %96 = phi <5 x i32> [ %88, %79 ], [ %91, %90 ]
  %97 = phi <5 x i32> [ %87, %79 ], [ %92, %90 ]
  %98 = phi <5 x i32> [ %86, %79 ], [ %93, %90 ]
  %99 = phi <5 x i32> [ %85, %79 ], [ %94, %90 ]
  %100 = add i32 %82, 1
  %101 = icmp eq i32 %100, %38
  br i1 %101, label %77, label %79

102:                                              ; preds = %89, %118
  %103 = phi <5 x i32> [ %122, %118 ], [ %85, %89 ]
  %104 = phi <5 x i32> [ %121, %118 ], [ %86, %89 ]
  %105 = phi i32 [ %123, %118 ], [ %35, %89 ]
  %106 = phi <5 x i32> [ %120, %118 ], [ %87, %89 ]
  %107 = phi <5 x i32> [ %119, %118 ], [ %88, %89 ]
  %108 = insertelement <5 x i32> %103, i32 %105, i32 3
  %109 = insertelement <5 x i32> %104, i32 %105, i32 3
  %110 = insertelement <5 x i32> %106, i32 %105, i32 3
  %111 = insertelement <5 x i32> %107, i32 %105, i32 3
  br i1 %44, label %112, label %118

112:                                              ; preds = %102
  br label %125

113:                                              ; preds = %141
  %114 = phi <5 x i32> [ %142, %141 ]
  %115 = phi <5 x i32> [ %143, %141 ]
  %116 = phi <5 x i32> [ %144, %141 ]
  %117 = phi <5 x i32> [ %145, %141 ]
  br label %118

118:                                              ; preds = %113, %102
  %119 = phi <5 x i32> [ %111, %102 ], [ %114, %113 ]
  %120 = phi <5 x i32> [ %110, %102 ], [ %115, %113 ]
  %121 = phi <5 x i32> [ %109, %102 ], [ %116, %113 ]
  %122 = phi <5 x i32> [ %108, %102 ], [ %117, %113 ]
  %123 = add i32 %105, 1
  %124 = icmp eq i32 %123, %36
  br i1 %124, label %90, label %102

125:                                              ; preds = %112, %141
  %126 = phi <5 x i32> [ %145, %141 ], [ %108, %112 ]
  %127 = phi <5 x i32> [ %144, %141 ], [ %109, %112 ]
  %128 = phi i32 [ %146, %141 ], [ %33, %112 ]
  %129 = phi <5 x i32> [ %143, %141 ], [ %110, %112 ]
  %130 = phi <5 x i32> [ %142, %141 ], [ %111, %112 ]
  %131 = insertelement <5 x i32> %126, i32 %128, i32 2
  %132 = insertelement <5 x i32> %127, i32 %128, i32 2
  %133 = insertelement <5 x i32> %129, i32 %128, i32 2
  %134 = insertelement <5 x i32> %130, i32 %128, i32 2
  br i1 %45, label %135, label %141

135:                                              ; preds = %125
  br label %148

136:                                              ; preds = %172
  %137 = phi <5 x i32> [ %173, %172 ]
  %138 = phi <5 x i32> [ %174, %172 ]
  %139 = phi <5 x i32> [ %175, %172 ]
  %140 = phi <5 x i32> [ %176, %172 ]
  br label %141

141:                                              ; preds = %136, %125
  %142 = phi <5 x i32> [ %134, %125 ], [ %137, %136 ]
  %143 = phi <5 x i32> [ %133, %125 ], [ %138, %136 ]
  %144 = phi <5 x i32> [ %132, %125 ], [ %139, %136 ]
  %145 = phi <5 x i32> [ %131, %125 ], [ %140, %136 ]
  %146 = add i32 %128, 1
  %147 = icmp eq i32 %146, %34
  br i1 %147, label %113, label %125

148:                                              ; preds = %135, %172
  %149 = phi <5 x i32> [ %176, %172 ], [ %131, %135 ]
  %150 = phi <5 x i32> [ %175, %172 ], [ %132, %135 ]
  %151 = phi i32 [ %177, %172 ], [ %27, %135 ]
  %152 = phi <5 x i32> [ %174, %172 ], [ %133, %135 ]
  %153 = phi <5 x i32> [ %173, %172 ], [ %134, %135 ]
  %154 = insertelement <5 x i32> %149, i32 %151, i32 0
  %155 = insertelement <5 x i32> %150, i32 %151, i32 0
  %156 = insertelement <5 x i32> %152, i32 %151, i32 0
  %157 = or i32 %151, 64
  %158 = insertelement <5 x i32> %153, i32 %157, i32 0
  br i1 %46, label %159, label %172

159:                                              ; preds = %148
  br label %160

160:                                              ; preds = %159, %542
  %161 = phi <5 x i32> [ %210, %542 ], [ %154, %159 ]
  %162 = phi <5 x i32> [ %209, %542 ], [ %155, %159 ]
  %163 = phi i32 [ %545, %542 ], [ %30, %159 ]
  %164 = phi <5 x i32> [ %544, %542 ], [ %156, %159 ]
  %165 = phi <5 x i32> [ %543, %542 ], [ %158, %159 ]
  br i1 %47, label %166, label %180

166:                                              ; preds = %160
  br label %193

167:                                              ; preds = %542
  %168 = phi <5 x i32> [ %543, %542 ]
  %169 = phi <5 x i32> [ %544, %542 ]
  %170 = phi <5 x i32> [ %209, %542 ]
  %171 = phi <5 x i32> [ %210, %542 ]
  br label %172

172:                                              ; preds = %167, %148
  %173 = phi <5 x i32> [ %158, %148 ], [ %168, %167 ]
  %174 = phi <5 x i32> [ %156, %148 ], [ %169, %167 ]
  %175 = phi <5 x i32> [ %155, %148 ], [ %170, %167 ]
  %176 = phi <5 x i32> [ %154, %148 ], [ %171, %167 ]
  %177 = add nsw i32 %151, 128
  %178 = icmp slt i32 %177, %29
  br i1 %178, label %148, label %136

179:                                              ; preds = %193
  br label %180

180:                                              ; preds = %179, %160
  %181 = mul nsw i32 %163, %40
  %182 = add nsw i32 %181, %40
  %183 = icmp slt i32 %182, %32
  %184 = select i1 %183, i32 %182, i32 %32
  %185 = sub i32 1, %181
  %186 = add i32 %185, %184
  %187 = ashr i32 %186, 1
  %188 = insertelement <5 x i32> %161, i32 %181, i32 1
  %189 = add nsw i32 %187, %181
  %190 = insertelement <5 x i32> %162, i32 %189, i32 1
  %191 = icmp slt i32 %181, %184
  br i1 %191, label %192, label %208

192:                                              ; preds = %180
  br label %397

193:                                              ; preds = %166, %193
  %194 = phi i32 [ %203, %193 ], [ 0, %166 ]
  %195 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %194, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %195, align 256, !tbaa !5
  %196 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %196, align 256, !tbaa !5
  %197 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %197, align 256, !tbaa !5
  %198 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %198, align 256, !tbaa !5
  %199 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %194, i32 1
  store <64 x float> <float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000>, <64 x float> addrspace(2)* %199, align 256, !tbaa !5
  %200 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 1
  store <64 x float> <float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000>, <64 x float> addrspace(2)* %200, align 256, !tbaa !5
  %201 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 1
  store <64 x float> <float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000>, <64 x float> addrspace(2)* %201, align 256, !tbaa !5
  %202 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %194, i32 1
  store <64 x float> <float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000, float 0xC7EFFFFFE0000000>, <64 x float> addrspace(2)* %202, align 256, !tbaa !5
  %203 = add nuw nsw i32 %194, 1
  %204 = icmp eq i32 %203, %39
  br i1 %204, label %179, label %193

205:                                              ; preds = %536
  %206 = phi <5 x i32> [ %421, %536 ]
  %207 = phi <5 x i32> [ %442, %536 ]
  br label %208

208:                                              ; preds = %205, %180
  %209 = phi <5 x i32> [ %190, %180 ], [ %207, %205 ]
  %210 = phi <5 x i32> [ %188, %180 ], [ %206, %205 ]
  br i1 %47, label %215, label %211

211:                                              ; preds = %208
  %212 = mul nsw i32 %163, %39
  %213 = insertelement <5 x i32> %164, i32 %212, i32 1
  %214 = insertelement <5 x i32> %165, i32 %212, i32 1
  br label %542

215:                                              ; preds = %208
  br i1 %74, label %314, label %216

216:                                              ; preds = %215
  br label %217

217:                                              ; preds = %216, %745
  %218 = phi i32 [ %746, %745 ], [ 0, %216 ]
  %219 = phi i32 [ %747, %745 ], [ %75, %216 ]
  %220 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %221 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %222 = load <64 x i32>, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  %223 = load <64 x float>, <64 x float> addrspace(2)* %70, align 256, !tbaa !5
  %224 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %218, i32 0
  %225 = load <64 x i32>, <64 x i32> addrspace(2)* %224, align 256, !tbaa !5
  %226 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %218, i32 1
  %227 = load <64 x float>, <64 x float> addrspace(2)* %226, align 256, !tbaa !5
  %228 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %218, i32 0
  %229 = load <64 x i32>, <64 x i32> addrspace(2)* %228, align 256, !tbaa !5
  %230 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %218, i32 1
  %231 = load <64 x float>, <64 x float> addrspace(2)* %230, align 256, !tbaa !5
  %232 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %227, <64 x float> %221, <64 x i32> %225, <64 x i32> %220, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %233 = shufflevector <128 x i32> %232, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %234 = bitcast <128 x i32> %232 to <2 x i2048>
  %235 = extractelement <2 x i2048> %234, i32 1
  %236 = shufflevector <128 x i32> %232, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %237 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %231, <64 x float> %223, <64 x i32> %229, <64 x i32> %222, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %238 = shufflevector <128 x i32> %237, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %239 = bitcast <128 x i32> %237 to <2 x i2048>
  %240 = extractelement <2 x i2048> %239, i32 1
  %241 = shufflevector <128 x i32> %237, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %233, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %236, <64 x i32> addrspace(2)* %71, align 256, !tbaa !5
  store <64 x i32> %238, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %241, <64 x i32> addrspace(2)* %72, align 256, !tbaa !5
  br i1 %66, label %242, label %244

242:                                              ; preds = %217
  br label %269

243:                                              ; preds = %269
  br label %244

244:                                              ; preds = %243, %217
  %245 = or i32 %218, 1
  %246 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %247 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %248 = load <64 x i32>, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  %249 = load <64 x float>, <64 x float> addrspace(2)* %70, align 256, !tbaa !5
  %250 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %245, i32 0
  %251 = load <64 x i32>, <64 x i32> addrspace(2)* %250, align 256, !tbaa !5
  %252 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %245, i32 1
  %253 = load <64 x float>, <64 x float> addrspace(2)* %252, align 256, !tbaa !5
  %254 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %245, i32 0
  %255 = load <64 x i32>, <64 x i32> addrspace(2)* %254, align 256, !tbaa !5
  %256 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %245, i32 1
  %257 = load <64 x float>, <64 x float> addrspace(2)* %256, align 256, !tbaa !5
  %258 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %253, <64 x float> %247, <64 x i32> %251, <64 x i32> %246, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %259 = shufflevector <128 x i32> %258, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %260 = bitcast <128 x i32> %258 to <2 x i2048>
  %261 = extractelement <2 x i2048> %260, i32 1
  %262 = shufflevector <128 x i32> %258, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %263 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %257, <64 x float> %249, <64 x i32> %255, <64 x i32> %248, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %264 = shufflevector <128 x i32> %263, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %265 = bitcast <128 x i32> %263 to <2 x i2048>
  %266 = extractelement <2 x i2048> %265, i32 1
  %267 = shufflevector <128 x i32> %263, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %259, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %262, <64 x i32> addrspace(2)* %71, align 256, !tbaa !5
  store <64 x i32> %264, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %267, <64 x i32> addrspace(2)* %72, align 256, !tbaa !5
  br i1 %66, label %268, label %607

268:                                              ; preds = %244
  br label %563

269:                                              ; preds = %242, %269
  %270 = phi i2048 [ %301, %269 ], [ %240, %242 ]
  %271 = phi <64 x i32> [ %299, %269 ], [ %238, %242 ]
  %272 = phi i2048 [ %293, %269 ], [ %235, %242 ]
  %273 = phi <64 x i32> [ %291, %269 ], [ %233, %242 ]
  %274 = phi i32 [ %310, %269 ], [ %65, %242 ]
  %275 = bitcast i2048 %272 to <64 x float>
  %276 = bitcast i2048 %270 to <64 x float>
  %277 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %274, i32 0
  %278 = load <64 x i32>, <64 x i32> addrspace(2)* %277, align 256, !tbaa !5
  %279 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %274, i32 1
  %280 = load <64 x float>, <64 x float> addrspace(2)* %279, align 256, !tbaa !5
  %281 = add nuw nsw i32 %274, 1
  %282 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %281, i32 0
  %283 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %281, i32 1
  %284 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %274, i32 0
  %285 = load <64 x i32>, <64 x i32> addrspace(2)* %284, align 256, !tbaa !5
  %286 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %274, i32 1
  %287 = load <64 x float>, <64 x float> addrspace(2)* %286, align 256, !tbaa !5
  %288 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %281, i32 0
  %289 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %281, i32 1
  %290 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %275, <64 x float> %280, <64 x i32> %273, <64 x i32> %278, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %291 = shufflevector <128 x i32> %290, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %292 = bitcast <128 x i32> %290 to <2 x i2048>
  %293 = extractelement <2 x i2048> %292, i32 1
  %294 = shufflevector <128 x i32> %290, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %295 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %275, <64 x float> %280, <64 x i32> %273, <64 x i32> %278, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %296 = shufflevector <128 x i32> %295, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %297 = shufflevector <128 x i32> %295, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %298 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %276, <64 x float> %287, <64 x i32> %271, <64 x i32> %285, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %299 = shufflevector <128 x i32> %298, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %300 = bitcast <128 x i32> %298 to <2 x i2048>
  %301 = extractelement <2 x i2048> %300, i32 1
  %302 = shufflevector <128 x i32> %298, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %303 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %276, <64 x float> %287, <64 x i32> %271, <64 x i32> %285, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %304 = shufflevector <128 x i32> %303, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %305 = shufflevector <128 x i32> %303, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %291, <64 x i32> addrspace(2)* %277, align 256, !tbaa !5
  %306 = bitcast <64 x float> addrspace(2)* %279 to <64 x i32> addrspace(2)*
  store <64 x i32> %294, <64 x i32> addrspace(2)* %306, align 256, !tbaa !5
  store <64 x i32> %296, <64 x i32> addrspace(2)* %282, align 256, !tbaa !5
  %307 = bitcast <64 x float> addrspace(2)* %283 to <64 x i32> addrspace(2)*
  store <64 x i32> %297, <64 x i32> addrspace(2)* %307, align 256, !tbaa !5
  store <64 x i32> %299, <64 x i32> addrspace(2)* %284, align 256, !tbaa !5
  %308 = bitcast <64 x float> addrspace(2)* %286 to <64 x i32> addrspace(2)*
  store <64 x i32> %302, <64 x i32> addrspace(2)* %308, align 256, !tbaa !5
  store <64 x i32> %304, <64 x i32> addrspace(2)* %288, align 256, !tbaa !5
  %309 = bitcast <64 x float> addrspace(2)* %289 to <64 x i32> addrspace(2)*
  store <64 x i32> %305, <64 x i32> addrspace(2)* %309, align 256, !tbaa !5
  %310 = add nsw i32 %274, -1
  %311 = icmp sgt i32 %274, 0
  br i1 %311, label %269, label %243

312:                                              ; preds = %745
  %313 = phi i32 [ %746, %745 ]
  br label %314

314:                                              ; preds = %312, %215
  %315 = phi i32 [ 0, %215 ], [ %313, %312 ]
  br i1 %76, label %392, label %316

316:                                              ; preds = %314
  br label %317

317:                                              ; preds = %316, %387
  %318 = phi i32 [ %388, %387 ], [ %315, %316 ]
  %319 = phi i32 [ %389, %387 ], [ %73, %316 ]
  %320 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %321 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %322 = load <64 x i32>, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  %323 = load <64 x float>, <64 x float> addrspace(2)* %70, align 256, !tbaa !5
  %324 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %318, i32 0
  %325 = load <64 x i32>, <64 x i32> addrspace(2)* %324, align 256, !tbaa !5
  %326 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %318, i32 1
  %327 = load <64 x float>, <64 x float> addrspace(2)* %326, align 256, !tbaa !5
  %328 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %318, i32 0
  %329 = load <64 x i32>, <64 x i32> addrspace(2)* %328, align 256, !tbaa !5
  %330 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %318, i32 1
  %331 = load <64 x float>, <64 x float> addrspace(2)* %330, align 256, !tbaa !5
  %332 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %327, <64 x float> %321, <64 x i32> %325, <64 x i32> %320, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %333 = shufflevector <128 x i32> %332, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %334 = bitcast <128 x i32> %332 to <2 x i2048>
  %335 = extractelement <2 x i2048> %334, i32 1
  %336 = shufflevector <128 x i32> %332, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %337 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %331, <64 x float> %323, <64 x i32> %329, <64 x i32> %322, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %338 = shufflevector <128 x i32> %337, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %339 = bitcast <128 x i32> %337 to <2 x i2048>
  %340 = extractelement <2 x i2048> %339, i32 1
  %341 = shufflevector <128 x i32> %337, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %333, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %336, <64 x i32> addrspace(2)* %71, align 256, !tbaa !5
  store <64 x i32> %338, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %341, <64 x i32> addrspace(2)* %72, align 256, !tbaa !5
  br i1 %66, label %342, label %387

342:                                              ; preds = %317
  br label %343

343:                                              ; preds = %342, %343
  %344 = phi i2048 [ %375, %343 ], [ %340, %342 ]
  %345 = phi <64 x i32> [ %373, %343 ], [ %338, %342 ]
  %346 = phi i2048 [ %367, %343 ], [ %335, %342 ]
  %347 = phi <64 x i32> [ %365, %343 ], [ %333, %342 ]
  %348 = phi i32 [ %384, %343 ], [ %65, %342 ]
  %349 = bitcast i2048 %346 to <64 x float>
  %350 = bitcast i2048 %344 to <64 x float>
  %351 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %348, i32 0
  %352 = load <64 x i32>, <64 x i32> addrspace(2)* %351, align 256, !tbaa !5
  %353 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %348, i32 1
  %354 = load <64 x float>, <64 x float> addrspace(2)* %353, align 256, !tbaa !5
  %355 = add nuw nsw i32 %348, 1
  %356 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %355, i32 0
  %357 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %355, i32 1
  %358 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %348, i32 0
  %359 = load <64 x i32>, <64 x i32> addrspace(2)* %358, align 256, !tbaa !5
  %360 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %348, i32 1
  %361 = load <64 x float>, <64 x float> addrspace(2)* %360, align 256, !tbaa !5
  %362 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %355, i32 0
  %363 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %355, i32 1
  %364 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %349, <64 x float> %354, <64 x i32> %347, <64 x i32> %352, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %365 = shufflevector <128 x i32> %364, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %366 = bitcast <128 x i32> %364 to <2 x i2048>
  %367 = extractelement <2 x i2048> %366, i32 1
  %368 = shufflevector <128 x i32> %364, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %369 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %349, <64 x float> %354, <64 x i32> %347, <64 x i32> %352, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %370 = shufflevector <128 x i32> %369, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %371 = shufflevector <128 x i32> %369, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %372 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %350, <64 x float> %361, <64 x i32> %345, <64 x i32> %359, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %373 = shufflevector <128 x i32> %372, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %374 = bitcast <128 x i32> %372 to <2 x i2048>
  %375 = extractelement <2 x i2048> %374, i32 1
  %376 = shufflevector <128 x i32> %372, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %377 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %350, <64 x float> %361, <64 x i32> %345, <64 x i32> %359, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %378 = shufflevector <128 x i32> %377, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %379 = shufflevector <128 x i32> %377, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %365, <64 x i32> addrspace(2)* %351, align 256, !tbaa !5
  %380 = bitcast <64 x float> addrspace(2)* %353 to <64 x i32> addrspace(2)*
  store <64 x i32> %368, <64 x i32> addrspace(2)* %380, align 256, !tbaa !5
  store <64 x i32> %370, <64 x i32> addrspace(2)* %356, align 256, !tbaa !5
  %381 = bitcast <64 x float> addrspace(2)* %357 to <64 x i32> addrspace(2)*
  store <64 x i32> %371, <64 x i32> addrspace(2)* %381, align 256, !tbaa !5
  store <64 x i32> %373, <64 x i32> addrspace(2)* %358, align 256, !tbaa !5
  %382 = bitcast <64 x float> addrspace(2)* %360 to <64 x i32> addrspace(2)*
  store <64 x i32> %376, <64 x i32> addrspace(2)* %382, align 256, !tbaa !5
  store <64 x i32> %378, <64 x i32> addrspace(2)* %362, align 256, !tbaa !5
  %383 = bitcast <64 x float> addrspace(2)* %363 to <64 x i32> addrspace(2)*
  store <64 x i32> %379, <64 x i32> addrspace(2)* %383, align 256, !tbaa !5
  %384 = add nsw i32 %348, -1
  %385 = icmp sgt i32 %348, 0
  br i1 %385, label %343, label %386

386:                                              ; preds = %343
  br label %387

387:                                              ; preds = %386, %317
  %388 = add nuw nsw i32 %318, 1
  %389 = add i32 %319, -1
  %390 = icmp eq i32 %389, 0
  br i1 %390, label %391, label %317, !llvm.loop !8

391:                                              ; preds = %387
  br label %392

392:                                              ; preds = %391, %314
  %393 = mul nsw i32 %163, %39
  %394 = insertelement <5 x i32> %164, i32 %393, i32 1
  %395 = insertelement <5 x i32> %165, i32 %393, i32 1
  br i1 %47, label %396, label %542

396:                                              ; preds = %392
  br label %547

397:                                              ; preds = %192, %536
  %398 = phi i32 [ %537, %536 ], [ %181, %192 ]
  %399 = phi <5 x i32> [ %421, %536 ], [ %188, %192 ]
  %400 = phi <5 x i32> [ %442, %536 ], [ %190, %192 ]
  %401 = load <64 x i32>, <64 x i32> addrspace(2)* %49, align 256, !tbaa !5
  %402 = load <64 x float>, <64 x float> addrspace(2)* %50, align 256, !tbaa !5
  %403 = extractelement <5 x i32> %399, i32 1
  %404 = insertelement <64 x i32> undef, i32 %403, i32 0
  %405 = shufflevector <64 x i32> %404, <64 x i32> undef, <64 x i32> zeroinitializer
  %406 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %399, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %407 = bitcast <128 x bfloat> %406 to <128 x i16>
  %408 = tail call <128 x i16> @llvm.tpc.mov.group.v128i16.v128i16.i1(<128 x i16> %407, i32 -1, i32 63, <128 x i16> undef, i1 true, i1 false) #5
  %409 = tail call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %407, i32 -1, i32 0, i32 14407680, <128 x i16> undef, i1 true, i1 false) #5
  %410 = tail call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %408, i32 -1, i32 0, i32 6713856, <128 x i16> %409, i1 true, i1 false) #5
  %411 = tail call <128 x i16> @llvm.tpc.unpack.v128i16.i1(<128 x i16> %410, i8 7, i32 0, <128 x i16> zeroinitializer, i1 true, i1 false) #5
  %412 = tail call <128 x i16> @llvm.tpc.unpack.v128i16.i1(<128 x i16> %410, i8 7, i32 256, <128 x i16> zeroinitializer, i1 true, i1 false) #5
  %413 = bitcast <128 x i16> %411 to <128 x bfloat>
  %414 = bitcast <128 x i16> %412 to <128 x bfloat>
  %415 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat> %413, i8 1, i32 0, <128 x float> undef, i1 true, i1 false) #5
  %416 = shufflevector <128 x float> %415, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %417 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat> %414, i8 1, i32 0, <128 x float> undef, i1 true, i1 false) #5
  %418 = shufflevector <128 x float> %417, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %419 = load <64 x i32>, <64 x i32> addrspace(2)* %51, align 256, !tbaa !5
  %420 = load <64 x float>, <64 x float> addrspace(2)* %52, align 256, !tbaa !5
  %421 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %399, i32 1, i32 2, i8 2, i32 0, <5 x i32> %399, i1 true, i1 false)
  %422 = load <64 x i32>, <64 x i32> addrspace(2)* %53, align 256, !tbaa !5
  %423 = load <64 x float>, <64 x float> addrspace(2)* %54, align 256, !tbaa !5
  %424 = extractelement <5 x i32> %400, i32 1
  %425 = insertelement <64 x i32> undef, i32 %424, i32 0
  %426 = shufflevector <64 x i32> %425, <64 x i32> undef, <64 x i32> zeroinitializer
  %427 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %400, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %428 = bitcast <128 x bfloat> %427 to <128 x i16>
  %429 = tail call <128 x i16> @llvm.tpc.mov.group.v128i16.v128i16.i1(<128 x i16> %428, i32 -1, i32 63, <128 x i16> undef, i1 true, i1 false) #5
  %430 = tail call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %428, i32 -1, i32 0, i32 14407680, <128 x i16> undef, i1 true, i1 false) #5
  %431 = tail call <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16> %429, i32 -1, i32 0, i32 6713856, <128 x i16> %430, i1 true, i1 false) #5
  %432 = tail call <128 x i16> @llvm.tpc.unpack.v128i16.i1(<128 x i16> %431, i8 7, i32 0, <128 x i16> zeroinitializer, i1 true, i1 false) #5
  %433 = tail call <128 x i16> @llvm.tpc.unpack.v128i16.i1(<128 x i16> %431, i8 7, i32 256, <128 x i16> zeroinitializer, i1 true, i1 false) #5
  %434 = bitcast <128 x i16> %432 to <128 x bfloat>
  %435 = bitcast <128 x i16> %433 to <128 x bfloat>
  %436 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat> %434, i8 1, i32 0, <128 x float> undef, i1 true, i1 false) #5
  %437 = shufflevector <128 x float> %436, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %438 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat> %435, i8 1, i32 0, <128 x float> undef, i1 true, i1 false) #5
  %439 = shufflevector <128 x float> %438, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %440 = load <64 x i32>, <64 x i32> addrspace(2)* %55, align 256, !tbaa !5
  %441 = load <64 x float>, <64 x float> addrspace(2)* %56, align 256, !tbaa !5
  %442 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %400, i32 1, i32 2, i8 2, i32 0, <5 x i32> %400, i1 true, i1 false)
  %443 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %416, <64 x float> %402, <64 x i32> %405, <64 x i32> %401, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %444 = shufflevector <128 x i32> %443, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %445 = bitcast <128 x i32> %443 to <2 x i2048>
  %446 = extractelement <2 x i2048> %445, i32 1
  %447 = shufflevector <128 x i32> %443, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %444, <64 x i32> addrspace(2)* %49, align 256
  store <64 x i32> %447, <64 x i32> addrspace(2)* %57, align 256
  %448 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %418, <64 x float> %420, <64 x i32> %405, <64 x i32> %419, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %449 = shufflevector <128 x i32> %448, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %450 = bitcast <128 x i32> %448 to <2 x i2048>
  %451 = extractelement <2 x i2048> %450, i32 1
  %452 = shufflevector <128 x i32> %448, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %449, <64 x i32> addrspace(2)* %51, align 256
  store <64 x i32> %452, <64 x i32> addrspace(2)* %58, align 256
  %453 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %437, <64 x float> %423, <64 x i32> %426, <64 x i32> %422, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %454 = shufflevector <128 x i32> %453, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %455 = shufflevector <128 x i32> %453, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %454, <64 x i32> addrspace(2)* %53, align 256
  store <64 x i32> %455, <64 x i32> addrspace(2)* %59, align 256
  %456 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %439, <64 x float> %441, <64 x i32> %426, <64 x i32> %440, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %457 = shufflevector <128 x i32> %456, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %458 = shufflevector <128 x i32> %456, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %457, <64 x i32> addrspace(2)* %55, align 256
  store <64 x i32> %458, <64 x i32> addrspace(2)* %60, align 256
  br i1 %66, label %459, label %536

459:                                              ; preds = %397
  br label %460

460:                                              ; preds = %459, %460
  %461 = phi i2048 [ %508, %460 ], [ %451, %459 ]
  %462 = phi <64 x i32> [ %506, %460 ], [ %449, %459 ]
  %463 = phi i2048 [ %500, %460 ], [ %446, %459 ]
  %464 = phi <64 x i32> [ %498, %460 ], [ %444, %459 ]
  %465 = phi i32 [ %533, %460 ], [ %65, %459 ]
  %466 = bitcast i2048 %463 to <64 x float>
  %467 = bitcast i2048 %461 to <64 x float>
  %468 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %465, i32 0
  %469 = load <64 x i32>, <64 x i32> addrspace(2)* %468, align 256, !tbaa !5
  %470 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %465, i32 1
  %471 = load <64 x float>, <64 x float> addrspace(2)* %470, align 256, !tbaa !5
  %472 = add nuw nsw i32 %465, 1
  %473 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %472, i32 0
  %474 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %472, i32 1
  %475 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %465, i32 0
  %476 = load <64 x i32>, <64 x i32> addrspace(2)* %475, align 256, !tbaa !5
  %477 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %465, i32 1
  %478 = load <64 x float>, <64 x float> addrspace(2)* %477, align 256, !tbaa !5
  %479 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %472, i32 0
  %480 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %472, i32 1
  %481 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %465, i32 0
  %482 = load <64 x i32>, <64 x i32> addrspace(2)* %481, align 256, !tbaa !5
  %483 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %465, i32 1
  %484 = load <64 x float>, <64 x float> addrspace(2)* %483, align 256, !tbaa !5
  %485 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %472, i32 0
  %486 = load <64 x i32>, <64 x i32> addrspace(2)* %485, align 256, !tbaa !5
  %487 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %472, i32 1
  %488 = load <64 x float>, <64 x float> addrspace(2)* %487, align 256, !tbaa !5
  %489 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %465, i32 0
  %490 = load <64 x i32>, <64 x i32> addrspace(2)* %489, align 256, !tbaa !5
  %491 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %465, i32 1
  %492 = load <64 x float>, <64 x float> addrspace(2)* %491, align 256, !tbaa !5
  %493 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %472, i32 0
  %494 = load <64 x i32>, <64 x i32> addrspace(2)* %493, align 256, !tbaa !5
  %495 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %472, i32 1
  %496 = load <64 x float>, <64 x float> addrspace(2)* %495, align 256, !tbaa !5
  %497 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %466, <64 x float> %471, <64 x i32> %464, <64 x i32> %469, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %498 = shufflevector <128 x i32> %497, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %499 = bitcast <128 x i32> %497 to <2 x i2048>
  %500 = extractelement <2 x i2048> %499, i32 1
  %501 = shufflevector <128 x i32> %497, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %502 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %466, <64 x float> %471, <64 x i32> %464, <64 x i32> %469, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %503 = shufflevector <128 x i32> %502, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %504 = shufflevector <128 x i32> %502, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %505 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %467, <64 x float> %478, <64 x i32> %462, <64 x i32> %476, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %506 = shufflevector <128 x i32> %505, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %507 = bitcast <128 x i32> %505 to <2 x i2048>
  %508 = extractelement <2 x i2048> %507, i32 1
  %509 = shufflevector <128 x i32> %505, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %510 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %467, <64 x float> %478, <64 x i32> %462, <64 x i32> %476, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %511 = shufflevector <128 x i32> %510, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %512 = shufflevector <128 x i32> %510, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %513 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %488, <64 x float> %484, <64 x i32> %486, <64 x i32> %482, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %514 = shufflevector <128 x i32> %513, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %515 = shufflevector <128 x i32> %513, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %516 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %488, <64 x float> %484, <64 x i32> %486, <64 x i32> %482, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %517 = shufflevector <128 x i32> %516, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %518 = shufflevector <128 x i32> %516, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %519 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %496, <64 x float> %492, <64 x i32> %494, <64 x i32> %490, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %520 = shufflevector <128 x i32> %519, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %521 = shufflevector <128 x i32> %519, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %522 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %496, <64 x float> %492, <64 x i32> %494, <64 x i32> %490, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %523 = shufflevector <128 x i32> %522, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %524 = shufflevector <128 x i32> %522, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %498, <64 x i32> addrspace(2)* %468, align 256, !tbaa !5
  %525 = bitcast <64 x float> addrspace(2)* %470 to <64 x i32> addrspace(2)*
  store <64 x i32> %501, <64 x i32> addrspace(2)* %525, align 256, !tbaa !5
  store <64 x i32> %503, <64 x i32> addrspace(2)* %473, align 256, !tbaa !5
  %526 = bitcast <64 x float> addrspace(2)* %474 to <64 x i32> addrspace(2)*
  store <64 x i32> %504, <64 x i32> addrspace(2)* %526, align 256, !tbaa !5
  store <64 x i32> %506, <64 x i32> addrspace(2)* %475, align 256, !tbaa !5
  %527 = bitcast <64 x float> addrspace(2)* %477 to <64 x i32> addrspace(2)*
  store <64 x i32> %509, <64 x i32> addrspace(2)* %527, align 256, !tbaa !5
  store <64 x i32> %511, <64 x i32> addrspace(2)* %479, align 256, !tbaa !5
  %528 = bitcast <64 x float> addrspace(2)* %480 to <64 x i32> addrspace(2)*
  store <64 x i32> %512, <64 x i32> addrspace(2)* %528, align 256, !tbaa !5
  store <64 x i32> %514, <64 x i32> addrspace(2)* %481, align 256, !tbaa !5
  %529 = bitcast <64 x float> addrspace(2)* %483 to <64 x i32> addrspace(2)*
  store <64 x i32> %515, <64 x i32> addrspace(2)* %529, align 256, !tbaa !5
  store <64 x i32> %517, <64 x i32> addrspace(2)* %485, align 256, !tbaa !5
  %530 = bitcast <64 x float> addrspace(2)* %487 to <64 x i32> addrspace(2)*
  store <64 x i32> %518, <64 x i32> addrspace(2)* %530, align 256, !tbaa !5
  store <64 x i32> %520, <64 x i32> addrspace(2)* %489, align 256, !tbaa !5
  %531 = bitcast <64 x float> addrspace(2)* %491 to <64 x i32> addrspace(2)*
  store <64 x i32> %521, <64 x i32> addrspace(2)* %531, align 256, !tbaa !5
  store <64 x i32> %523, <64 x i32> addrspace(2)* %493, align 256, !tbaa !5
  %532 = bitcast <64 x float> addrspace(2)* %495 to <64 x i32> addrspace(2)*
  store <64 x i32> %524, <64 x i32> addrspace(2)* %532, align 256, !tbaa !5
  %533 = add nsw i32 %465, -1
  %534 = icmp sgt i32 %465, 0
  br i1 %534, label %460, label %535

535:                                              ; preds = %460
  br label %536

536:                                              ; preds = %535, %397
  %537 = add nsw i32 %398, 2
  %538 = icmp slt i32 %537, %184
  br i1 %538, label %397, label %205

539:                                              ; preds = %547
  %540 = phi <5 x i32> [ %555, %547 ]
  %541 = phi <5 x i32> [ %560, %547 ]
  br label %542

542:                                              ; preds = %539, %211, %392
  %543 = phi <5 x i32> [ %395, %392 ], [ %214, %211 ], [ %541, %539 ]
  %544 = phi <5 x i32> [ %394, %392 ], [ %213, %211 ], [ %540, %539 ]
  %545 = add i32 %163, 1
  %546 = icmp eq i32 %545, %31
  br i1 %546, label %167, label %160

547:                                              ; preds = %396, %547
  %548 = phi i32 [ %561, %547 ], [ 0, %396 ]
  %549 = phi <5 x i32> [ %555, %547 ], [ %394, %396 ]
  %550 = phi <5 x i32> [ %560, %547 ], [ %395, %396 ]
  %551 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %548, i32 0
  %552 = load <64 x i32>, <64 x i32> addrspace(2)* %551, align 256, !tbaa !5
  %553 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* null, i32 0, i32 %548, i32 1
  %554 = load <64 x float>, <64 x float> addrspace(2)* %553, align 256, !tbaa !5
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %549, i8 3, <64 x i32> %552, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %549, i8 2, <64 x float> %554, i32 0, i1 true, i1 false)
  %555 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %549, i32 1, i32 2, i8 2, i32 0, <5 x i32> %549, i1 true, i1 false)
  %556 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %548, i32 0
  %557 = load <64 x i32>, <64 x i32> addrspace(2)* %556, align 256, !tbaa !5
  %558 = getelementptr inbounds [37 x %struct._int64_float64_pair_t], [37 x %struct._int64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._int64_float64_pair_t] addrspace(2)*), i32 0, i32 %548, i32 1
  %559 = load <64 x float>, <64 x float> addrspace(2)* %558, align 256, !tbaa !5
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %550, i8 3, <64 x i32> %557, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %550, i8 2, <64 x float> %559, i32 0, i1 true, i1 false)
  %560 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %550, i32 1, i32 2, i8 2, i32 0, <5 x i32> %550, i1 true, i1 false)
  %561 = add nuw nsw i32 %548, 1
  %562 = icmp eq i32 %561, %39
  br i1 %562, label %539, label %547

563:                                              ; preds = %268, %563
  %564 = phi i2048 [ %595, %563 ], [ %266, %268 ]
  %565 = phi <64 x i32> [ %593, %563 ], [ %264, %268 ]
  %566 = phi i2048 [ %587, %563 ], [ %261, %268 ]
  %567 = phi <64 x i32> [ %585, %563 ], [ %259, %268 ]
  %568 = phi i32 [ %604, %563 ], [ %65, %268 ]
  %569 = bitcast i2048 %566 to <64 x float>
  %570 = bitcast i2048 %564 to <64 x float>
  %571 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %568, i32 0
  %572 = load <64 x i32>, <64 x i32> addrspace(2)* %571, align 256, !tbaa !5
  %573 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %568, i32 1
  %574 = load <64 x float>, <64 x float> addrspace(2)* %573, align 256, !tbaa !5
  %575 = add nuw nsw i32 %568, 1
  %576 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %575, i32 0
  %577 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %575, i32 1
  %578 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %568, i32 0
  %579 = load <64 x i32>, <64 x i32> addrspace(2)* %578, align 256, !tbaa !5
  %580 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %568, i32 1
  %581 = load <64 x float>, <64 x float> addrspace(2)* %580, align 256, !tbaa !5
  %582 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %575, i32 0
  %583 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %575, i32 1
  %584 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %569, <64 x float> %574, <64 x i32> %567, <64 x i32> %572, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %585 = shufflevector <128 x i32> %584, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %586 = bitcast <128 x i32> %584 to <2 x i2048>
  %587 = extractelement <2 x i2048> %586, i32 1
  %588 = shufflevector <128 x i32> %584, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %589 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %569, <64 x float> %574, <64 x i32> %567, <64 x i32> %572, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %590 = shufflevector <128 x i32> %589, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %591 = shufflevector <128 x i32> %589, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %592 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %570, <64 x float> %581, <64 x i32> %565, <64 x i32> %579, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %593 = shufflevector <128 x i32> %592, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %594 = bitcast <128 x i32> %592 to <2 x i2048>
  %595 = extractelement <2 x i2048> %594, i32 1
  %596 = shufflevector <128 x i32> %592, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %597 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %570, <64 x float> %581, <64 x i32> %565, <64 x i32> %579, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %598 = shufflevector <128 x i32> %597, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %599 = shufflevector <128 x i32> %597, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %585, <64 x i32> addrspace(2)* %571, align 256, !tbaa !5
  %600 = bitcast <64 x float> addrspace(2)* %573 to <64 x i32> addrspace(2)*
  store <64 x i32> %588, <64 x i32> addrspace(2)* %600, align 256, !tbaa !5
  store <64 x i32> %590, <64 x i32> addrspace(2)* %576, align 256, !tbaa !5
  %601 = bitcast <64 x float> addrspace(2)* %577 to <64 x i32> addrspace(2)*
  store <64 x i32> %591, <64 x i32> addrspace(2)* %601, align 256, !tbaa !5
  store <64 x i32> %593, <64 x i32> addrspace(2)* %578, align 256, !tbaa !5
  %602 = bitcast <64 x float> addrspace(2)* %580 to <64 x i32> addrspace(2)*
  store <64 x i32> %596, <64 x i32> addrspace(2)* %602, align 256, !tbaa !5
  store <64 x i32> %598, <64 x i32> addrspace(2)* %582, align 256, !tbaa !5
  %603 = bitcast <64 x float> addrspace(2)* %583 to <64 x i32> addrspace(2)*
  store <64 x i32> %599, <64 x i32> addrspace(2)* %603, align 256, !tbaa !5
  %604 = add nsw i32 %568, -1
  %605 = icmp sgt i32 %568, 0
  br i1 %605, label %563, label %606

606:                                              ; preds = %563
  br label %607

607:                                              ; preds = %606, %244
  %608 = or i32 %218, 2
  %609 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %610 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %611 = load <64 x i32>, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  %612 = load <64 x float>, <64 x float> addrspace(2)* %70, align 256, !tbaa !5
  %613 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %608, i32 0
  %614 = load <64 x i32>, <64 x i32> addrspace(2)* %613, align 256, !tbaa !5
  %615 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %608, i32 1
  %616 = load <64 x float>, <64 x float> addrspace(2)* %615, align 256, !tbaa !5
  %617 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %608, i32 0
  %618 = load <64 x i32>, <64 x i32> addrspace(2)* %617, align 256, !tbaa !5
  %619 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %608, i32 1
  %620 = load <64 x float>, <64 x float> addrspace(2)* %619, align 256, !tbaa !5
  %621 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %616, <64 x float> %610, <64 x i32> %614, <64 x i32> %609, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %622 = shufflevector <128 x i32> %621, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %623 = bitcast <128 x i32> %621 to <2 x i2048>
  %624 = extractelement <2 x i2048> %623, i32 1
  %625 = shufflevector <128 x i32> %621, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %626 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %620, <64 x float> %612, <64 x i32> %618, <64 x i32> %611, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %627 = shufflevector <128 x i32> %626, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %628 = bitcast <128 x i32> %626 to <2 x i2048>
  %629 = extractelement <2 x i2048> %628, i32 1
  %630 = shufflevector <128 x i32> %626, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %622, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %625, <64 x i32> addrspace(2)* %71, align 256, !tbaa !5
  store <64 x i32> %627, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %630, <64 x i32> addrspace(2)* %72, align 256, !tbaa !5
  br i1 %66, label %631, label %676

631:                                              ; preds = %607
  br label %632

632:                                              ; preds = %631, %632
  %633 = phi i2048 [ %664, %632 ], [ %629, %631 ]
  %634 = phi <64 x i32> [ %662, %632 ], [ %627, %631 ]
  %635 = phi i2048 [ %656, %632 ], [ %624, %631 ]
  %636 = phi <64 x i32> [ %654, %632 ], [ %622, %631 ]
  %637 = phi i32 [ %673, %632 ], [ %65, %631 ]
  %638 = bitcast i2048 %635 to <64 x float>
  %639 = bitcast i2048 %633 to <64 x float>
  %640 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %637, i32 0
  %641 = load <64 x i32>, <64 x i32> addrspace(2)* %640, align 256, !tbaa !5
  %642 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %637, i32 1
  %643 = load <64 x float>, <64 x float> addrspace(2)* %642, align 256, !tbaa !5
  %644 = add nuw nsw i32 %637, 1
  %645 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %644, i32 0
  %646 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %644, i32 1
  %647 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %637, i32 0
  %648 = load <64 x i32>, <64 x i32> addrspace(2)* %647, align 256, !tbaa !5
  %649 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %637, i32 1
  %650 = load <64 x float>, <64 x float> addrspace(2)* %649, align 256, !tbaa !5
  %651 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %644, i32 0
  %652 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %644, i32 1
  %653 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %638, <64 x float> %643, <64 x i32> %636, <64 x i32> %641, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %654 = shufflevector <128 x i32> %653, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %655 = bitcast <128 x i32> %653 to <2 x i2048>
  %656 = extractelement <2 x i2048> %655, i32 1
  %657 = shufflevector <128 x i32> %653, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %658 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %638, <64 x float> %643, <64 x i32> %636, <64 x i32> %641, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %659 = shufflevector <128 x i32> %658, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %660 = shufflevector <128 x i32> %658, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %661 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %639, <64 x float> %650, <64 x i32> %634, <64 x i32> %648, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %662 = shufflevector <128 x i32> %661, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %663 = bitcast <128 x i32> %661 to <2 x i2048>
  %664 = extractelement <2 x i2048> %663, i32 1
  %665 = shufflevector <128 x i32> %661, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %666 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %639, <64 x float> %650, <64 x i32> %634, <64 x i32> %648, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %667 = shufflevector <128 x i32> %666, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %668 = shufflevector <128 x i32> %666, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %654, <64 x i32> addrspace(2)* %640, align 256, !tbaa !5
  %669 = bitcast <64 x float> addrspace(2)* %642 to <64 x i32> addrspace(2)*
  store <64 x i32> %657, <64 x i32> addrspace(2)* %669, align 256, !tbaa !5
  store <64 x i32> %659, <64 x i32> addrspace(2)* %645, align 256, !tbaa !5
  %670 = bitcast <64 x float> addrspace(2)* %646 to <64 x i32> addrspace(2)*
  store <64 x i32> %660, <64 x i32> addrspace(2)* %670, align 256, !tbaa !5
  store <64 x i32> %662, <64 x i32> addrspace(2)* %647, align 256, !tbaa !5
  %671 = bitcast <64 x float> addrspace(2)* %649 to <64 x i32> addrspace(2)*
  store <64 x i32> %665, <64 x i32> addrspace(2)* %671, align 256, !tbaa !5
  store <64 x i32> %667, <64 x i32> addrspace(2)* %651, align 256, !tbaa !5
  %672 = bitcast <64 x float> addrspace(2)* %652 to <64 x i32> addrspace(2)*
  store <64 x i32> %668, <64 x i32> addrspace(2)* %672, align 256, !tbaa !5
  %673 = add nsw i32 %637, -1
  %674 = icmp sgt i32 %637, 0
  br i1 %674, label %632, label %675

675:                                              ; preds = %632
  br label %676

676:                                              ; preds = %675, %607
  %677 = or i32 %218, 3
  %678 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %679 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %680 = load <64 x i32>, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  %681 = load <64 x float>, <64 x float> addrspace(2)* %70, align 256, !tbaa !5
  %682 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %677, i32 0
  %683 = load <64 x i32>, <64 x i32> addrspace(2)* %682, align 256, !tbaa !5
  %684 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %63, i32 %677, i32 1
  %685 = load <64 x float>, <64 x float> addrspace(2)* %684, align 256, !tbaa !5
  %686 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %677, i32 0
  %687 = load <64 x i32>, <64 x i32> addrspace(2)* %686, align 256, !tbaa !5
  %688 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %64, i32 %677, i32 1
  %689 = load <64 x float>, <64 x float> addrspace(2)* %688, align 256, !tbaa !5
  %690 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %685, <64 x float> %679, <64 x i32> %683, <64 x i32> %678, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %691 = shufflevector <128 x i32> %690, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %692 = bitcast <128 x i32> %690 to <2 x i2048>
  %693 = extractelement <2 x i2048> %692, i32 1
  %694 = shufflevector <128 x i32> %690, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %695 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %689, <64 x float> %681, <64 x i32> %687, <64 x i32> %680, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %696 = shufflevector <128 x i32> %695, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %697 = bitcast <128 x i32> %695 to <2 x i2048>
  %698 = extractelement <2 x i2048> %697, i32 1
  %699 = shufflevector <128 x i32> %695, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %691, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %694, <64 x i32> addrspace(2)* %71, align 256, !tbaa !5
  store <64 x i32> %696, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %699, <64 x i32> addrspace(2)* %72, align 256, !tbaa !5
  br i1 %66, label %700, label %745

700:                                              ; preds = %676
  br label %701

701:                                              ; preds = %700, %701
  %702 = phi i2048 [ %733, %701 ], [ %698, %700 ]
  %703 = phi <64 x i32> [ %731, %701 ], [ %696, %700 ]
  %704 = phi i2048 [ %725, %701 ], [ %693, %700 ]
  %705 = phi <64 x i32> [ %723, %701 ], [ %691, %700 ]
  %706 = phi i32 [ %742, %701 ], [ %65, %700 ]
  %707 = bitcast i2048 %704 to <64 x float>
  %708 = bitcast i2048 %702 to <64 x float>
  %709 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %706, i32 0
  %710 = load <64 x i32>, <64 x i32> addrspace(2)* %709, align 256, !tbaa !5
  %711 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %706, i32 1
  %712 = load <64 x float>, <64 x float> addrspace(2)* %711, align 256, !tbaa !5
  %713 = add nuw nsw i32 %706, 1
  %714 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %713, i32 0
  %715 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %61, i32 %713, i32 1
  %716 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %706, i32 0
  %717 = load <64 x i32>, <64 x i32> addrspace(2)* %716, align 256, !tbaa !5
  %718 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %706, i32 1
  %719 = load <64 x float>, <64 x float> addrspace(2)* %718, align 256, !tbaa !5
  %720 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %713, i32 0
  %721 = getelementptr inbounds %struct._int64_float64_pair_t, %struct._int64_float64_pair_t addrspace(2)* %62, i32 %713, i32 1
  %722 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %707, <64 x float> %712, <64 x i32> %705, <64 x i32> %710, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %723 = shufflevector <128 x i32> %722, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %724 = bitcast <128 x i32> %722 to <2 x i2048>
  %725 = extractelement <2 x i2048> %724, i32 1
  %726 = shufflevector <128 x i32> %722, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %727 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %707, <64 x float> %712, <64 x i32> %705, <64 x i32> %710, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %728 = shufflevector <128 x i32> %727, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %729 = shufflevector <128 x i32> %727, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %730 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %708, <64 x float> %719, <64 x i32> %703, <64 x i32> %717, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %731 = shufflevector <128 x i32> %730, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %732 = bitcast <128 x i32> %730 to <2 x i2048>
  %733 = extractelement <2 x i2048> %732, i32 1
  %734 = shufflevector <128 x i32> %730, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %735 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %708, <64 x float> %719, <64 x i32> %703, <64 x i32> %717, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %736 = shufflevector <128 x i32> %735, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %737 = shufflevector <128 x i32> %735, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %723, <64 x i32> addrspace(2)* %709, align 256, !tbaa !5
  %738 = bitcast <64 x float> addrspace(2)* %711 to <64 x i32> addrspace(2)*
  store <64 x i32> %726, <64 x i32> addrspace(2)* %738, align 256, !tbaa !5
  store <64 x i32> %728, <64 x i32> addrspace(2)* %714, align 256, !tbaa !5
  %739 = bitcast <64 x float> addrspace(2)* %715 to <64 x i32> addrspace(2)*
  store <64 x i32> %729, <64 x i32> addrspace(2)* %739, align 256, !tbaa !5
  store <64 x i32> %731, <64 x i32> addrspace(2)* %716, align 256, !tbaa !5
  %740 = bitcast <64 x float> addrspace(2)* %718 to <64 x i32> addrspace(2)*
  store <64 x i32> %734, <64 x i32> addrspace(2)* %740, align 256, !tbaa !5
  store <64 x i32> %736, <64 x i32> addrspace(2)* %720, align 256, !tbaa !5
  %741 = bitcast <64 x float> addrspace(2)* %721 to <64 x i32> addrspace(2)*
  store <64 x i32> %737, <64 x i32> addrspace(2)* %741, align 256, !tbaa !5
  %742 = add nsw i32 %706, -1
  %743 = icmp sgt i32 %706, 0
  br i1 %743, label %701, label %744

744:                                              ; preds = %701
  br label %745

745:                                              ; preds = %744, %676
  %746 = add nuw nsw i32 %218, 4
  %747 = add i32 %219, -4
  %748 = icmp eq i32 %747, 0
  br i1 %748, label %312, label %217, !llvm.loop !11
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float>, <64 x float>, <64 x i32>, <64 x i32>, i8, i32, <128 x i32>, i1, i1) #2

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #3

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #3

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #4

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #3

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat>, i8, i32, <128 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.mov.group.v128i16.v128i16.i1(<128 x i16>, i32, i32, <128 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.mov.dual.group.all.v128i16.i1(<128 x i16>, i32, i32, i32, <128 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.unpack.v128i16.i1(<128 x i16>, i8, i32, <128 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float>, <64 x float>, <64 x i32>, <64 x i32>, i8, i32, <128 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind readonly }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = !{i32 75776}
!4 = !{!"{5, 4}"}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9, !10}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = !{!"llvm.loop.unroll.remainderloop.marker", i32 0}
!11 = distinct !{!11, !9}
