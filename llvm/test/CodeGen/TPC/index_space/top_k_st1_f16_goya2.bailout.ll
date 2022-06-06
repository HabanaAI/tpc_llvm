; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[2].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 }[2].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './kernels/goya2/gnmt/top_k_st1_f16_goya2.c'
source_filename = "./kernels/goya2/gnmt/top_k_st1_f16_goya2.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

%struct._uint64_float64_pair_t = type { <64 x i32>, <64 x float> }

@0 = external dso_local addrspace(2) global [37 x %struct._uint64_float64_pair_t], section ".vldata"
@1 = external dso_local addrspace(2) global [37 x %struct._uint64_float64_pair_t], section ".vldata"
@2 = external dso_local addrspace(2) global [37 x %struct._uint64_float64_pair_t], section ".vldata"
@3 = external dso_local addrspace(2) global [37 x %struct._uint64_float64_pair_t], section ".vldata"

; Function Attrs: nounwind
define dso_local void @main(i32 %0, i32 %1, i32 %2) local_unnamed_addr #0 !unroll_info !4 {
  %4 = bitcast [37 x %struct._uint64_float64_pair_t] addrspace(2)* null to i8 addrspace(2)*
  %5 = bitcast [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %6 = bitcast [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %7 = bitcast [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*) to i8 addrspace(2)*
  %8 = tail call i32 @llvm.tpc.ld.l.i32(i32 24, i32 1, i32 undef, i1 true, i1 false) #5
  %9 = tail call i32 @llvm.tpc.ld.l.i32(i32 28, i32 1, i32 undef, i1 true, i1 false) #5
  %10 = shl i32 %1, 3
  %11 = add i32 %10, 16
  %12 = tail call i32 @llvm.tpc.ld.l.i32(i32 %11, i32 1, i32 undef, i1 true, i1 false) #5
  %13 = add i32 %10, 20
  %14 = tail call i32 @llvm.tpc.ld.l.i32(i32 %13, i32 1, i32 undef, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 24, i32 %12, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 28, i32 %14, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %11, i32 %8, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %13, i32 %9, i32 1, i1 true, i1 false) #5
  %15 = tail call i32 @llvm.tpc.ld.l.i32(i32 80, i32 1, i32 undef, i1 true, i1 false) #5
  %16 = tail call i32 @llvm.tpc.ld.l.i32(i32 84, i32 1, i32 undef, i1 true, i1 false) #5
  %17 = add i32 %10, 72
  %18 = tail call i32 @llvm.tpc.ld.l.i32(i32 %17, i32 1, i32 undef, i1 true, i1 false) #5
  %19 = add i32 %10, 76
  %20 = tail call i32 @llvm.tpc.ld.l.i32(i32 %19, i32 1, i32 undef, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 80, i32 %18, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 84, i32 %20, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %17, i32 %15, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 %19, i32 %16, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 136, i32 %18, i32 1, i1 true, i1 false) #5
  tail call void @llvm.tpc.st.l.i32(i32 140, i32 %20, i32 1, i1 true, i1 false) #5
  %21 = add i32 %10, 128
  tail call void @llvm.tpc.st.l.i32(i32 %21, i32 %15, i32 1, i1 true, i1 false) #5
  %22 = add i32 %10, 132
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
  %32 = tail call i32 @llvm.tpc.ld.l.i32(i32 24, i32 1, i32 undef, i1 true, i1 false) #5
  %33 = extractelement <5 x i32> %23, i32 2
  %34 = extractelement <5 x i32> %25, i32 2
  %35 = extractelement <5 x i32> %23, i32 3
  %36 = extractelement <5 x i32> %25, i32 3
  %37 = extractelement <5 x i32> %23, i32 4
  %38 = extractelement <5 x i32> %25, i32 4
  %39 = icmp slt i32 %37, %38
  br i1 %39, label %40, label %76

40:                                               ; preds = %3
  %41 = icmp slt i32 %35, %36
  %42 = icmp slt i32 %33, %34
  %43 = icmp slt i32 %27, %29
  %44 = icmp slt i32 %30, %31
  %45 = icmp sgt i32 %0, 0
  %46 = add i32 %0, -1
  %47 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %46, i32 0
  %48 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %46, i32 1
  %49 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 0
  %50 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 1
  %51 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 0
  %52 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 1
  %53 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 0
  %54 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %46, i32 1
  %55 = bitcast <64 x float> addrspace(2)* %48 to <64 x i32> addrspace(2)*
  %56 = bitcast <64 x float> addrspace(2)* %50 to <64 x i32> addrspace(2)*
  %57 = bitcast <64 x float> addrspace(2)* %52 to <64 x i32> addrspace(2)*
  %58 = bitcast <64 x float> addrspace(2)* %54 to <64 x i32> addrspace(2)*
  %59 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 0
  %60 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %61 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %62 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 0
  %63 = add nsw i32 %0, -2
  %64 = icmp sgt i32 %0, 1
  %65 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %46, i32 0
  %66 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %46, i32 1
  %67 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %46, i32 0
  %68 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %46, i32 1
  %69 = bitcast <64 x float> addrspace(2)* %66 to <64 x i32> addrspace(2)*
  %70 = bitcast <64 x float> addrspace(2)* %68 to <64 x i32> addrspace(2)*
  %71 = and i32 %0, 3
  %72 = icmp ult i32 %46, 3
  %73 = sub nuw nsw i32 %0, %71
  %74 = icmp eq i32 %71, 0
  br label %77

75:                                               ; preds = %93
  br label %76

76:                                               ; preds = %75, %3
  ret void

77:                                               ; preds = %93, %40
  %78 = phi <5 x i32> [ zeroinitializer, %40 ], [ %97, %93 ]
  %79 = phi <5 x i32> [ zeroinitializer, %40 ], [ %96, %93 ]
  %80 = phi <5 x i32> [ zeroinitializer, %40 ], [ %95, %93 ]
  %81 = phi <5 x i32> [ zeroinitializer, %40 ], [ %94, %93 ]
  %82 = phi i32 [ %37, %40 ], [ %98, %93 ]
  %83 = insertelement <5 x i32> %78, i32 %82, i32 4
  %84 = insertelement <5 x i32> %79, i32 %82, i32 4
  %85 = insertelement <5 x i32> %80, i32 %82, i32 4
  %86 = insertelement <5 x i32> %81, i32 %82, i32 4
  br i1 %41, label %87, label %93

87:                                               ; preds = %77
  br label %100

88:                                               ; preds = %116
  %89 = phi <5 x i32> [ %117, %116 ]
  %90 = phi <5 x i32> [ %118, %116 ]
  %91 = phi <5 x i32> [ %119, %116 ]
  %92 = phi <5 x i32> [ %120, %116 ]
  br label %93

93:                                               ; preds = %88, %77
  %94 = phi <5 x i32> [ %86, %77 ], [ %89, %88 ]
  %95 = phi <5 x i32> [ %85, %77 ], [ %90, %88 ]
  %96 = phi <5 x i32> [ %84, %77 ], [ %91, %88 ]
  %97 = phi <5 x i32> [ %83, %77 ], [ %92, %88 ]
  %98 = add i32 %82, 1
  %99 = icmp eq i32 %98, %38
  br i1 %99, label %75, label %77

100:                                              ; preds = %87, %116
  %101 = phi <5 x i32> [ %120, %116 ], [ %83, %87 ]
  %102 = phi <5 x i32> [ %119, %116 ], [ %84, %87 ]
  %103 = phi <5 x i32> [ %118, %116 ], [ %85, %87 ]
  %104 = phi <5 x i32> [ %117, %116 ], [ %86, %87 ]
  %105 = phi i32 [ %121, %116 ], [ %35, %87 ]
  %106 = insertelement <5 x i32> %101, i32 %105, i32 3
  %107 = insertelement <5 x i32> %102, i32 %105, i32 3
  %108 = insertelement <5 x i32> %103, i32 %105, i32 3
  %109 = insertelement <5 x i32> %104, i32 %105, i32 3
  br i1 %42, label %110, label %116

110:                                              ; preds = %100
  br label %123

111:                                              ; preds = %139
  %112 = phi <5 x i32> [ %140, %139 ]
  %113 = phi <5 x i32> [ %141, %139 ]
  %114 = phi <5 x i32> [ %142, %139 ]
  %115 = phi <5 x i32> [ %143, %139 ]
  br label %116

116:                                              ; preds = %111, %100
  %117 = phi <5 x i32> [ %109, %100 ], [ %112, %111 ]
  %118 = phi <5 x i32> [ %108, %100 ], [ %113, %111 ]
  %119 = phi <5 x i32> [ %107, %100 ], [ %114, %111 ]
  %120 = phi <5 x i32> [ %106, %100 ], [ %115, %111 ]
  %121 = add i32 %105, 1
  %122 = icmp eq i32 %121, %36
  br i1 %122, label %88, label %100

123:                                              ; preds = %110, %139
  %124 = phi <5 x i32> [ %143, %139 ], [ %106, %110 ]
  %125 = phi <5 x i32> [ %142, %139 ], [ %107, %110 ]
  %126 = phi <5 x i32> [ %141, %139 ], [ %108, %110 ]
  %127 = phi <5 x i32> [ %140, %139 ], [ %109, %110 ]
  %128 = phi i32 [ %144, %139 ], [ %33, %110 ]
  %129 = insertelement <5 x i32> %124, i32 %128, i32 2
  %130 = insertelement <5 x i32> %125, i32 %128, i32 2
  %131 = insertelement <5 x i32> %126, i32 %128, i32 2
  %132 = insertelement <5 x i32> %127, i32 %128, i32 2
  br i1 %43, label %133, label %139

133:                                              ; preds = %123
  br label %146

134:                                              ; preds = %170
  %135 = phi <5 x i32> [ %171, %170 ]
  %136 = phi <5 x i32> [ %172, %170 ]
  %137 = phi <5 x i32> [ %173, %170 ]
  %138 = phi <5 x i32> [ %174, %170 ]
  br label %139

139:                                              ; preds = %134, %123
  %140 = phi <5 x i32> [ %132, %123 ], [ %135, %134 ]
  %141 = phi <5 x i32> [ %131, %123 ], [ %136, %134 ]
  %142 = phi <5 x i32> [ %130, %123 ], [ %137, %134 ]
  %143 = phi <5 x i32> [ %129, %123 ], [ %138, %134 ]
  %144 = add i32 %128, 1
  %145 = icmp eq i32 %144, %34
  br i1 %145, label %111, label %123

146:                                              ; preds = %133, %170
  %147 = phi <5 x i32> [ %174, %170 ], [ %129, %133 ]
  %148 = phi <5 x i32> [ %173, %170 ], [ %130, %133 ]
  %149 = phi <5 x i32> [ %172, %170 ], [ %131, %133 ]
  %150 = phi <5 x i32> [ %171, %170 ], [ %132, %133 ]
  %151 = phi i32 [ %175, %170 ], [ %27, %133 ]
  %152 = insertelement <5 x i32> %147, i32 %151, i32 0
  %153 = insertelement <5 x i32> %148, i32 %151, i32 0
  %154 = insertelement <5 x i32> %149, i32 %151, i32 0
  %155 = or i32 %151, 64
  %156 = insertelement <5 x i32> %150, i32 %155, i32 0
  br i1 %44, label %157, label %170

157:                                              ; preds = %146
  br label %158

158:                                              ; preds = %157, %522
  %159 = phi <5 x i32> [ %208, %522 ], [ %152, %157 ]
  %160 = phi <5 x i32> [ %207, %522 ], [ %153, %157 ]
  %161 = phi <5 x i32> [ %524, %522 ], [ %154, %157 ]
  %162 = phi <5 x i32> [ %523, %522 ], [ %156, %157 ]
  %163 = phi i32 [ %525, %522 ], [ %30, %157 ]
  br i1 %45, label %164, label %178

164:                                              ; preds = %158
  br label %191

165:                                              ; preds = %522
  %166 = phi <5 x i32> [ %523, %522 ]
  %167 = phi <5 x i32> [ %524, %522 ]
  %168 = phi <5 x i32> [ %207, %522 ]
  %169 = phi <5 x i32> [ %208, %522 ]
  br label %170

170:                                              ; preds = %165, %146
  %171 = phi <5 x i32> [ %156, %146 ], [ %166, %165 ]
  %172 = phi <5 x i32> [ %154, %146 ], [ %167, %165 ]
  %173 = phi <5 x i32> [ %153, %146 ], [ %168, %165 ]
  %174 = phi <5 x i32> [ %152, %146 ], [ %169, %165 ]
  %175 = add nsw i32 %151, 128
  %176 = icmp slt i32 %175, %29
  br i1 %176, label %146, label %134

177:                                              ; preds = %191
  br label %178

178:                                              ; preds = %177, %158
  %179 = mul nsw i32 %163, %2
  %180 = add nsw i32 %179, %2
  %181 = icmp slt i32 %180, %32
  %182 = select i1 %181, i32 %180, i32 %32
  %183 = sub i32 1, %179
  %184 = add i32 %183, %182
  %185 = ashr i32 %184, 1
  %186 = insertelement <5 x i32> %159, i32 %179, i32 1
  %187 = add nsw i32 %185, %179
  %188 = insertelement <5 x i32> %160, i32 %187, i32 1
  %189 = icmp slt i32 %179, %182
  br i1 %189, label %190, label %206

190:                                              ; preds = %178
  br label %395

191:                                              ; preds = %164, %191
  %192 = phi i32 [ %201, %191 ], [ 0, %164 ]
  %193 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %192, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %193, align 256, !tbaa !5
  %194 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %194, align 256, !tbaa !5
  %195 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %195, align 256, !tbaa !5
  %196 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 0
  store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* %196, align 256, !tbaa !5
  %197 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %192, i32 1
  store <64 x float> <float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04>, <64 x float> addrspace(2)* %197, align 256, !tbaa !5
  %198 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 1
  store <64 x float> <float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04>, <64 x float> addrspace(2)* %198, align 256, !tbaa !5
  %199 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 37888 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 1
  store <64 x float> <float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04>, <64 x float> addrspace(2)* %199, align 256, !tbaa !5
  %200 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 56832 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %192, i32 1
  store <64 x float> <float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04, float -6.550400e+04>, <64 x float> addrspace(2)* %200, align 256, !tbaa !5
  %201 = add nuw nsw i32 %192, 1
  %202 = icmp eq i32 %201, %0
  br i1 %202, label %177, label %191

203:                                              ; preds = %516
  %204 = phi <5 x i32> [ %410, %516 ]
  %205 = phi <5 x i32> [ %422, %516 ]
  br label %206

206:                                              ; preds = %203, %178
  %207 = phi <5 x i32> [ %188, %178 ], [ %205, %203 ]
  %208 = phi <5 x i32> [ %186, %178 ], [ %204, %203 ]
  br i1 %45, label %213, label %209

209:                                              ; preds = %206
  %210 = mul nsw i32 %163, %0
  %211 = insertelement <5 x i32> %161, i32 %210, i32 1
  %212 = insertelement <5 x i32> %162, i32 %210, i32 1
  br label %522

213:                                              ; preds = %206
  br i1 %72, label %312, label %214

214:                                              ; preds = %213
  br label %215

215:                                              ; preds = %214, %725
  %216 = phi i32 [ %726, %725 ], [ 0, %214 ]
  %217 = phi i32 [ %727, %725 ], [ %73, %214 ]
  %218 = load <64 x i32>, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  %219 = load <64 x float>, <64 x float> addrspace(2)* %66, align 256, !tbaa !5
  %220 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %221 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %222 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %216, i32 0
  %223 = load <64 x i32>, <64 x i32> addrspace(2)* %222, align 256, !tbaa !5
  %224 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %216, i32 1
  %225 = load <64 x float>, <64 x float> addrspace(2)* %224, align 256, !tbaa !5
  %226 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %216, i32 0
  %227 = load <64 x i32>, <64 x i32> addrspace(2)* %226, align 256, !tbaa !5
  %228 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %216, i32 1
  %229 = load <64 x float>, <64 x float> addrspace(2)* %228, align 256, !tbaa !5
  %230 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %225, <64 x float> %219, <64 x i32> %223, <64 x i32> %218, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %231 = shufflevector <128 x i32> %230, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %232 = bitcast <128 x i32> %230 to <2 x i2048>
  %233 = extractelement <2 x i2048> %232, i32 1
  %234 = shufflevector <128 x i32> %230, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %235 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %229, <64 x float> %221, <64 x i32> %227, <64 x i32> %220, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %236 = shufflevector <128 x i32> %235, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %237 = bitcast <128 x i32> %235 to <2 x i2048>
  %238 = extractelement <2 x i2048> %237, i32 1
  %239 = shufflevector <128 x i32> %235, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %231, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  store <64 x i32> %234, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %236, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %239, <64 x i32> addrspace(2)* %70, align 256, !tbaa !5
  br i1 %64, label %240, label %242

240:                                              ; preds = %215
  br label %267

241:                                              ; preds = %267
  br label %242

242:                                              ; preds = %241, %215
  %243 = or i32 %216, 1
  %244 = load <64 x i32>, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  %245 = load <64 x float>, <64 x float> addrspace(2)* %66, align 256, !tbaa !5
  %246 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %247 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %248 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %243, i32 0
  %249 = load <64 x i32>, <64 x i32> addrspace(2)* %248, align 256, !tbaa !5
  %250 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %243, i32 1
  %251 = load <64 x float>, <64 x float> addrspace(2)* %250, align 256, !tbaa !5
  %252 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %243, i32 0
  %253 = load <64 x i32>, <64 x i32> addrspace(2)* %252, align 256, !tbaa !5
  %254 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %243, i32 1
  %255 = load <64 x float>, <64 x float> addrspace(2)* %254, align 256, !tbaa !5
  %256 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %251, <64 x float> %245, <64 x i32> %249, <64 x i32> %244, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %257 = shufflevector <128 x i32> %256, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %258 = bitcast <128 x i32> %256 to <2 x i2048>
  %259 = extractelement <2 x i2048> %258, i32 1
  %260 = shufflevector <128 x i32> %256, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %261 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %255, <64 x float> %247, <64 x i32> %253, <64 x i32> %246, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %262 = shufflevector <128 x i32> %261, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %263 = bitcast <128 x i32> %261 to <2 x i2048>
  %264 = extractelement <2 x i2048> %263, i32 1
  %265 = shufflevector <128 x i32> %261, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %257, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  store <64 x i32> %260, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %262, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %265, <64 x i32> addrspace(2)* %70, align 256, !tbaa !5
  br i1 %64, label %266, label %587

266:                                              ; preds = %242
  br label %543

267:                                              ; preds = %240, %267
  %268 = phi i2048 [ %299, %267 ], [ %238, %240 ]
  %269 = phi <64 x i32> [ %297, %267 ], [ %236, %240 ]
  %270 = phi i2048 [ %291, %267 ], [ %233, %240 ]
  %271 = phi <64 x i32> [ %289, %267 ], [ %231, %240 ]
  %272 = phi i32 [ %308, %267 ], [ %63, %240 ]
  %273 = bitcast i2048 %270 to <64 x float>
  %274 = bitcast i2048 %268 to <64 x float>
  %275 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %272, i32 0
  %276 = load <64 x i32>, <64 x i32> addrspace(2)* %275, align 256, !tbaa !5
  %277 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %272, i32 1
  %278 = load <64 x float>, <64 x float> addrspace(2)* %277, align 256, !tbaa !5
  %279 = add nuw nsw i32 %272, 1
  %280 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %279, i32 0
  %281 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %279, i32 1
  %282 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %272, i32 0
  %283 = load <64 x i32>, <64 x i32> addrspace(2)* %282, align 256, !tbaa !5
  %284 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %272, i32 1
  %285 = load <64 x float>, <64 x float> addrspace(2)* %284, align 256, !tbaa !5
  %286 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %279, i32 0
  %287 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %279, i32 1
  %288 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %273, <64 x float> %278, <64 x i32> %271, <64 x i32> %276, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %289 = shufflevector <128 x i32> %288, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %290 = bitcast <128 x i32> %288 to <2 x i2048>
  %291 = extractelement <2 x i2048> %290, i32 1
  %292 = shufflevector <128 x i32> %288, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %293 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %273, <64 x float> %278, <64 x i32> %271, <64 x i32> %276, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %294 = shufflevector <128 x i32> %293, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %295 = shufflevector <128 x i32> %293, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %296 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %274, <64 x float> %285, <64 x i32> %269, <64 x i32> %283, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %297 = shufflevector <128 x i32> %296, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %298 = bitcast <128 x i32> %296 to <2 x i2048>
  %299 = extractelement <2 x i2048> %298, i32 1
  %300 = shufflevector <128 x i32> %296, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %301 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %274, <64 x float> %285, <64 x i32> %269, <64 x i32> %283, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %302 = shufflevector <128 x i32> %301, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %303 = shufflevector <128 x i32> %301, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %289, <64 x i32> addrspace(2)* %275, align 256, !tbaa !5
  %304 = bitcast <64 x float> addrspace(2)* %277 to <64 x i32> addrspace(2)*
  store <64 x i32> %292, <64 x i32> addrspace(2)* %304, align 256, !tbaa !5
  store <64 x i32> %294, <64 x i32> addrspace(2)* %280, align 256, !tbaa !5
  %305 = bitcast <64 x float> addrspace(2)* %281 to <64 x i32> addrspace(2)*
  store <64 x i32> %295, <64 x i32> addrspace(2)* %305, align 256, !tbaa !5
  store <64 x i32> %297, <64 x i32> addrspace(2)* %282, align 256, !tbaa !5
  %306 = bitcast <64 x float> addrspace(2)* %284 to <64 x i32> addrspace(2)*
  store <64 x i32> %300, <64 x i32> addrspace(2)* %306, align 256, !tbaa !5
  store <64 x i32> %302, <64 x i32> addrspace(2)* %286, align 256, !tbaa !5
  %307 = bitcast <64 x float> addrspace(2)* %287 to <64 x i32> addrspace(2)*
  store <64 x i32> %303, <64 x i32> addrspace(2)* %307, align 256, !tbaa !5
  %308 = add nsw i32 %272, -1
  %309 = icmp sgt i32 %272, 0
  br i1 %309, label %267, label %241

310:                                              ; preds = %725
  %311 = phi i32 [ %726, %725 ]
  br label %312

312:                                              ; preds = %310, %213
  %313 = phi i32 [ 0, %213 ], [ %311, %310 ]
  br i1 %74, label %390, label %314

314:                                              ; preds = %312
  br label %315

315:                                              ; preds = %314, %385
  %316 = phi i32 [ %386, %385 ], [ %313, %314 ]
  %317 = phi i32 [ %387, %385 ], [ %71, %314 ]
  %318 = load <64 x i32>, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  %319 = load <64 x float>, <64 x float> addrspace(2)* %66, align 256, !tbaa !5
  %320 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %321 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %322 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %316, i32 0
  %323 = load <64 x i32>, <64 x i32> addrspace(2)* %322, align 256, !tbaa !5
  %324 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %316, i32 1
  %325 = load <64 x float>, <64 x float> addrspace(2)* %324, align 256, !tbaa !5
  %326 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %316, i32 0
  %327 = load <64 x i32>, <64 x i32> addrspace(2)* %326, align 256, !tbaa !5
  %328 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %316, i32 1
  %329 = load <64 x float>, <64 x float> addrspace(2)* %328, align 256, !tbaa !5
  %330 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %325, <64 x float> %319, <64 x i32> %323, <64 x i32> %318, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %331 = shufflevector <128 x i32> %330, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %332 = bitcast <128 x i32> %330 to <2 x i2048>
  %333 = extractelement <2 x i2048> %332, i32 1
  %334 = shufflevector <128 x i32> %330, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %335 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %329, <64 x float> %321, <64 x i32> %327, <64 x i32> %320, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %336 = shufflevector <128 x i32> %335, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %337 = bitcast <128 x i32> %335 to <2 x i2048>
  %338 = extractelement <2 x i2048> %337, i32 1
  %339 = shufflevector <128 x i32> %335, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %331, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  store <64 x i32> %334, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %336, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %339, <64 x i32> addrspace(2)* %70, align 256, !tbaa !5
  br i1 %64, label %340, label %385

340:                                              ; preds = %315
  br label %341

341:                                              ; preds = %340, %341
  %342 = phi i2048 [ %373, %341 ], [ %338, %340 ]
  %343 = phi <64 x i32> [ %371, %341 ], [ %336, %340 ]
  %344 = phi i2048 [ %365, %341 ], [ %333, %340 ]
  %345 = phi <64 x i32> [ %363, %341 ], [ %331, %340 ]
  %346 = phi i32 [ %382, %341 ], [ %63, %340 ]
  %347 = bitcast i2048 %344 to <64 x float>
  %348 = bitcast i2048 %342 to <64 x float>
  %349 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %346, i32 0
  %350 = load <64 x i32>, <64 x i32> addrspace(2)* %349, align 256, !tbaa !5
  %351 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %346, i32 1
  %352 = load <64 x float>, <64 x float> addrspace(2)* %351, align 256, !tbaa !5
  %353 = add nuw nsw i32 %346, 1
  %354 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %353, i32 0
  %355 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %353, i32 1
  %356 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %346, i32 0
  %357 = load <64 x i32>, <64 x i32> addrspace(2)* %356, align 256, !tbaa !5
  %358 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %346, i32 1
  %359 = load <64 x float>, <64 x float> addrspace(2)* %358, align 256, !tbaa !5
  %360 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %353, i32 0
  %361 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %353, i32 1
  %362 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %347, <64 x float> %352, <64 x i32> %345, <64 x i32> %350, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %363 = shufflevector <128 x i32> %362, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %364 = bitcast <128 x i32> %362 to <2 x i2048>
  %365 = extractelement <2 x i2048> %364, i32 1
  %366 = shufflevector <128 x i32> %362, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %367 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %347, <64 x float> %352, <64 x i32> %345, <64 x i32> %350, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %368 = shufflevector <128 x i32> %367, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %369 = shufflevector <128 x i32> %367, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %370 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %348, <64 x float> %359, <64 x i32> %343, <64 x i32> %357, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %371 = shufflevector <128 x i32> %370, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %372 = bitcast <128 x i32> %370 to <2 x i2048>
  %373 = extractelement <2 x i2048> %372, i32 1
  %374 = shufflevector <128 x i32> %370, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %375 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %348, <64 x float> %359, <64 x i32> %343, <64 x i32> %357, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %376 = shufflevector <128 x i32> %375, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %377 = shufflevector <128 x i32> %375, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %363, <64 x i32> addrspace(2)* %349, align 256, !tbaa !5
  %378 = bitcast <64 x float> addrspace(2)* %351 to <64 x i32> addrspace(2)*
  store <64 x i32> %366, <64 x i32> addrspace(2)* %378, align 256, !tbaa !5
  store <64 x i32> %368, <64 x i32> addrspace(2)* %354, align 256, !tbaa !5
  %379 = bitcast <64 x float> addrspace(2)* %355 to <64 x i32> addrspace(2)*
  store <64 x i32> %369, <64 x i32> addrspace(2)* %379, align 256, !tbaa !5
  store <64 x i32> %371, <64 x i32> addrspace(2)* %356, align 256, !tbaa !5
  %380 = bitcast <64 x float> addrspace(2)* %358 to <64 x i32> addrspace(2)*
  store <64 x i32> %374, <64 x i32> addrspace(2)* %380, align 256, !tbaa !5
  store <64 x i32> %376, <64 x i32> addrspace(2)* %360, align 256, !tbaa !5
  %381 = bitcast <64 x float> addrspace(2)* %361 to <64 x i32> addrspace(2)*
  store <64 x i32> %377, <64 x i32> addrspace(2)* %381, align 256, !tbaa !5
  %382 = add nsw i32 %346, -1
  %383 = icmp sgt i32 %346, 0
  br i1 %383, label %341, label %384

384:                                              ; preds = %341
  br label %385

385:                                              ; preds = %384, %315
  %386 = add nuw nsw i32 %316, 1
  %387 = add i32 %317, -1
  %388 = icmp eq i32 %387, 0
  br i1 %388, label %389, label %315, !llvm.loop !8

389:                                              ; preds = %385
  br label %390

390:                                              ; preds = %389, %312
  %391 = mul nsw i32 %163, %0
  %392 = insertelement <5 x i32> %161, i32 %391, i32 1
  %393 = insertelement <5 x i32> %162, i32 %391, i32 1
  br i1 %45, label %394, label %522

394:                                              ; preds = %390
  br label %527

395:                                              ; preds = %190, %516
  %396 = phi <5 x i32> [ %410, %516 ], [ %186, %190 ]
  %397 = phi i32 [ %517, %516 ], [ %179, %190 ]
  %398 = phi <5 x i32> [ %422, %516 ], [ %188, %190 ]
  %399 = load <64 x i32>, <64 x i32> addrspace(2)* %47, align 256, !tbaa !5
  %400 = load <64 x float>, <64 x float> addrspace(2)* %48, align 256, !tbaa !5
  %401 = extractelement <5 x i32> %396, i32 1
  %402 = insertelement <64 x i32> undef, i32 %401, i32 0
  %403 = shufflevector <64 x i32> %402, <64 x i32> undef, <64 x i32> zeroinitializer
  %404 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> %396, i8 0, i32 16, <128 x half> undef, i1 true, i1 false)
  %405 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128f16.i1(<128 x half> %404, i8 11, i32 0, <128 x float> undef, i1 true, i1 false)
  %406 = shufflevector <128 x float> %405, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %407 = shufflevector <128 x float> %405, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %408 = load <64 x i32>, <64 x i32> addrspace(2)* %49, align 256, !tbaa !5
  %409 = load <64 x float>, <64 x float> addrspace(2)* %50, align 256, !tbaa !5
  %410 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %396, i32 1, i32 2, i8 2, i32 0, <5 x i32> %396, i1 true, i1 false)
  %411 = load <64 x i32>, <64 x i32> addrspace(2)* %51, align 256, !tbaa !5
  %412 = load <64 x float>, <64 x float> addrspace(2)* %52, align 256, !tbaa !5
  %413 = extractelement <5 x i32> %398, i32 1
  %414 = insertelement <64 x i32> undef, i32 %413, i32 0
  %415 = shufflevector <64 x i32> %414, <64 x i32> undef, <64 x i32> zeroinitializer
  %416 = tail call <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32> %398, i8 0, i32 16, <128 x half> undef, i1 true, i1 false)
  %417 = tail call <128 x float> @llvm.tpc.convert.v128f32.v128f16.i1(<128 x half> %416, i8 11, i32 0, <128 x float> undef, i1 true, i1 false)
  %418 = shufflevector <128 x float> %417, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %419 = shufflevector <128 x float> %417, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %420 = load <64 x i32>, <64 x i32> addrspace(2)* %53, align 256, !tbaa !5
  %421 = load <64 x float>, <64 x float> addrspace(2)* %54, align 256, !tbaa !5
  %422 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %398, i32 1, i32 2, i8 2, i32 0, <5 x i32> %398, i1 true, i1 false)
  %423 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %406, <64 x float> %400, <64 x i32> %403, <64 x i32> %399, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %424 = shufflevector <128 x i32> %423, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %425 = bitcast <128 x i32> %423 to <2 x i2048>
  %426 = extractelement <2 x i2048> %425, i32 1
  %427 = shufflevector <128 x i32> %423, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %424, <64 x i32> addrspace(2)* %47, align 256
  store <64 x i32> %427, <64 x i32> addrspace(2)* %55, align 256
  %428 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %407, <64 x float> %409, <64 x i32> %403, <64 x i32> %408, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %429 = shufflevector <128 x i32> %428, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %430 = bitcast <128 x i32> %428 to <2 x i2048>
  %431 = extractelement <2 x i2048> %430, i32 1
  %432 = shufflevector <128 x i32> %428, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %429, <64 x i32> addrspace(2)* %49, align 256
  store <64 x i32> %432, <64 x i32> addrspace(2)* %56, align 256
  %433 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %418, <64 x float> %412, <64 x i32> %415, <64 x i32> %411, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %434 = shufflevector <128 x i32> %433, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %435 = shufflevector <128 x i32> %433, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %434, <64 x i32> addrspace(2)* %51, align 256
  store <64 x i32> %435, <64 x i32> addrspace(2)* %57, align 256
  %436 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %419, <64 x float> %421, <64 x i32> %415, <64 x i32> %420, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false)
  %437 = shufflevector <128 x i32> %436, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %438 = shufflevector <128 x i32> %436, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %437, <64 x i32> addrspace(2)* %53, align 256
  store <64 x i32> %438, <64 x i32> addrspace(2)* %58, align 256
  br i1 %64, label %439, label %516

439:                                              ; preds = %395
  br label %440

440:                                              ; preds = %439, %440
  %441 = phi i2048 [ %488, %440 ], [ %431, %439 ]
  %442 = phi <64 x i32> [ %486, %440 ], [ %429, %439 ]
  %443 = phi i2048 [ %480, %440 ], [ %426, %439 ]
  %444 = phi <64 x i32> [ %478, %440 ], [ %424, %439 ]
  %445 = phi i32 [ %513, %440 ], [ %63, %439 ]
  %446 = bitcast i2048 %443 to <64 x float>
  %447 = bitcast i2048 %441 to <64 x float>
  %448 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %445, i32 0
  %449 = load <64 x i32>, <64 x i32> addrspace(2)* %448, align 256, !tbaa !5
  %450 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %445, i32 1
  %451 = load <64 x float>, <64 x float> addrspace(2)* %450, align 256, !tbaa !5
  %452 = add nuw nsw i32 %445, 1
  %453 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %452, i32 0
  %454 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %452, i32 1
  %455 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %445, i32 0
  %456 = load <64 x i32>, <64 x i32> addrspace(2)* %455, align 256, !tbaa !5
  %457 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %445, i32 1
  %458 = load <64 x float>, <64 x float> addrspace(2)* %457, align 256, !tbaa !5
  %459 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %452, i32 0
  %460 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %452, i32 1
  %461 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %445, i32 0
  %462 = load <64 x i32>, <64 x i32> addrspace(2)* %461, align 256, !tbaa !5
  %463 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %445, i32 1
  %464 = load <64 x float>, <64 x float> addrspace(2)* %463, align 256, !tbaa !5
  %465 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %452, i32 0
  %466 = load <64 x i32>, <64 x i32> addrspace(2)* %465, align 256, !tbaa !5
  %467 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %452, i32 1
  %468 = load <64 x float>, <64 x float> addrspace(2)* %467, align 256, !tbaa !5
  %469 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %445, i32 0
  %470 = load <64 x i32>, <64 x i32> addrspace(2)* %469, align 256, !tbaa !5
  %471 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %445, i32 1
  %472 = load <64 x float>, <64 x float> addrspace(2)* %471, align 256, !tbaa !5
  %473 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %452, i32 0
  %474 = load <64 x i32>, <64 x i32> addrspace(2)* %473, align 256, !tbaa !5
  %475 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %452, i32 1
  %476 = load <64 x float>, <64 x float> addrspace(2)* %475, align 256, !tbaa !5
  %477 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %446, <64 x float> %451, <64 x i32> %444, <64 x i32> %449, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %478 = shufflevector <128 x i32> %477, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %479 = bitcast <128 x i32> %477 to <2 x i2048>
  %480 = extractelement <2 x i2048> %479, i32 1
  %481 = shufflevector <128 x i32> %477, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %482 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %446, <64 x float> %451, <64 x i32> %444, <64 x i32> %449, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %483 = shufflevector <128 x i32> %482, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %484 = shufflevector <128 x i32> %482, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %485 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %447, <64 x float> %458, <64 x i32> %442, <64 x i32> %456, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %486 = shufflevector <128 x i32> %485, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %487 = bitcast <128 x i32> %485 to <2 x i2048>
  %488 = extractelement <2 x i2048> %487, i32 1
  %489 = shufflevector <128 x i32> %485, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %490 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %447, <64 x float> %458, <64 x i32> %442, <64 x i32> %456, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %491 = shufflevector <128 x i32> %490, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %492 = shufflevector <128 x i32> %490, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %493 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %468, <64 x float> %464, <64 x i32> %466, <64 x i32> %462, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %494 = shufflevector <128 x i32> %493, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %495 = shufflevector <128 x i32> %493, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %496 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %468, <64 x float> %464, <64 x i32> %466, <64 x i32> %462, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %497 = shufflevector <128 x i32> %496, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %498 = shufflevector <128 x i32> %496, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %499 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %476, <64 x float> %472, <64 x i32> %474, <64 x i32> %470, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %500 = shufflevector <128 x i32> %499, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %501 = shufflevector <128 x i32> %499, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %502 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %476, <64 x float> %472, <64 x i32> %474, <64 x i32> %470, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %503 = shufflevector <128 x i32> %502, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %504 = shufflevector <128 x i32> %502, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %478, <64 x i32> addrspace(2)* %448, align 256, !tbaa !5
  %505 = bitcast <64 x float> addrspace(2)* %450 to <64 x i32> addrspace(2)*
  store <64 x i32> %481, <64 x i32> addrspace(2)* %505, align 256, !tbaa !5
  store <64 x i32> %483, <64 x i32> addrspace(2)* %453, align 256, !tbaa !5
  %506 = bitcast <64 x float> addrspace(2)* %454 to <64 x i32> addrspace(2)*
  store <64 x i32> %484, <64 x i32> addrspace(2)* %506, align 256, !tbaa !5
  store <64 x i32> %486, <64 x i32> addrspace(2)* %455, align 256, !tbaa !5
  %507 = bitcast <64 x float> addrspace(2)* %457 to <64 x i32> addrspace(2)*
  store <64 x i32> %489, <64 x i32> addrspace(2)* %507, align 256, !tbaa !5
  store <64 x i32> %491, <64 x i32> addrspace(2)* %459, align 256, !tbaa !5
  %508 = bitcast <64 x float> addrspace(2)* %460 to <64 x i32> addrspace(2)*
  store <64 x i32> %492, <64 x i32> addrspace(2)* %508, align 256, !tbaa !5
  store <64 x i32> %494, <64 x i32> addrspace(2)* %461, align 256, !tbaa !5
  %509 = bitcast <64 x float> addrspace(2)* %463 to <64 x i32> addrspace(2)*
  store <64 x i32> %495, <64 x i32> addrspace(2)* %509, align 256, !tbaa !5
  store <64 x i32> %497, <64 x i32> addrspace(2)* %465, align 256, !tbaa !5
  %510 = bitcast <64 x float> addrspace(2)* %467 to <64 x i32> addrspace(2)*
  store <64 x i32> %498, <64 x i32> addrspace(2)* %510, align 256, !tbaa !5
  store <64 x i32> %500, <64 x i32> addrspace(2)* %469, align 256, !tbaa !5
  %511 = bitcast <64 x float> addrspace(2)* %471 to <64 x i32> addrspace(2)*
  store <64 x i32> %501, <64 x i32> addrspace(2)* %511, align 256, !tbaa !5
  store <64 x i32> %503, <64 x i32> addrspace(2)* %473, align 256, !tbaa !5
  %512 = bitcast <64 x float> addrspace(2)* %475 to <64 x i32> addrspace(2)*
  store <64 x i32> %504, <64 x i32> addrspace(2)* %512, align 256, !tbaa !5
  %513 = add nsw i32 %445, -1
  %514 = icmp sgt i32 %445, 0
  br i1 %514, label %440, label %515

515:                                              ; preds = %440
  br label %516

516:                                              ; preds = %515, %395
  %517 = add nsw i32 %397, 2
  %518 = icmp slt i32 %517, %182
  br i1 %518, label %395, label %203

519:                                              ; preds = %527
  %520 = phi <5 x i32> [ %535, %527 ]
  %521 = phi <5 x i32> [ %540, %527 ]
  br label %522

522:                                              ; preds = %519, %209, %390
  %523 = phi <5 x i32> [ %393, %390 ], [ %212, %209 ], [ %521, %519 ]
  %524 = phi <5 x i32> [ %392, %390 ], [ %211, %209 ], [ %520, %519 ]
  %525 = add i32 %163, 1
  %526 = icmp eq i32 %525, %31
  br i1 %526, label %165, label %158

527:                                              ; preds = %394, %527
  %528 = phi i32 [ %541, %527 ], [ 0, %394 ]
  %529 = phi <5 x i32> [ %535, %527 ], [ %392, %394 ]
  %530 = phi <5 x i32> [ %540, %527 ], [ %393, %394 ]
  %531 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %528, i32 0
  %532 = load <64 x i32>, <64 x i32> addrspace(2)* %531, align 256, !tbaa !5
  %533 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* null, i32 0, i32 %528, i32 1
  %534 = load <64 x float>, <64 x float> addrspace(2)* %533, align 256, !tbaa !5
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %529, i8 2, <64 x i32> %532, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %529, i8 1, <64 x float> %534, i32 0, i1 true, i1 false)
  %535 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %529, i32 1, i32 2, i8 2, i32 0, <5 x i32> %529, i1 true, i1 false)
  %536 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %528, i32 0
  %537 = load <64 x i32>, <64 x i32> addrspace(2)* %536, align 256, !tbaa !5
  %538 = getelementptr inbounds [37 x %struct._uint64_float64_pair_t], [37 x %struct._uint64_float64_pair_t] addrspace(2)* inttoptr (i32 18944 to [37 x %struct._uint64_float64_pair_t] addrspace(2)*), i32 0, i32 %528, i32 1
  %539 = load <64 x float>, <64 x float> addrspace(2)* %538, align 256, !tbaa !5
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %530, i8 2, <64 x i32> %537, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %530, i8 1, <64 x float> %539, i32 0, i1 true, i1 false)
  %540 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %530, i32 1, i32 2, i8 2, i32 0, <5 x i32> %530, i1 true, i1 false)
  %541 = add nuw nsw i32 %528, 1
  %542 = icmp eq i32 %541, %0
  br i1 %542, label %519, label %527

543:                                              ; preds = %266, %543
  %544 = phi i2048 [ %575, %543 ], [ %264, %266 ]
  %545 = phi <64 x i32> [ %573, %543 ], [ %262, %266 ]
  %546 = phi i2048 [ %567, %543 ], [ %259, %266 ]
  %547 = phi <64 x i32> [ %565, %543 ], [ %257, %266 ]
  %548 = phi i32 [ %584, %543 ], [ %63, %266 ]
  %549 = bitcast i2048 %546 to <64 x float>
  %550 = bitcast i2048 %544 to <64 x float>
  %551 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %548, i32 0
  %552 = load <64 x i32>, <64 x i32> addrspace(2)* %551, align 256, !tbaa !5
  %553 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %548, i32 1
  %554 = load <64 x float>, <64 x float> addrspace(2)* %553, align 256, !tbaa !5
  %555 = add nuw nsw i32 %548, 1
  %556 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %555, i32 0
  %557 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %555, i32 1
  %558 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %548, i32 0
  %559 = load <64 x i32>, <64 x i32> addrspace(2)* %558, align 256, !tbaa !5
  %560 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %548, i32 1
  %561 = load <64 x float>, <64 x float> addrspace(2)* %560, align 256, !tbaa !5
  %562 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %555, i32 0
  %563 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %555, i32 1
  %564 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %549, <64 x float> %554, <64 x i32> %547, <64 x i32> %552, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %565 = shufflevector <128 x i32> %564, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %566 = bitcast <128 x i32> %564 to <2 x i2048>
  %567 = extractelement <2 x i2048> %566, i32 1
  %568 = shufflevector <128 x i32> %564, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %569 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %549, <64 x float> %554, <64 x i32> %547, <64 x i32> %552, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %570 = shufflevector <128 x i32> %569, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %571 = shufflevector <128 x i32> %569, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %572 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %550, <64 x float> %561, <64 x i32> %545, <64 x i32> %559, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %573 = shufflevector <128 x i32> %572, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %574 = bitcast <128 x i32> %572 to <2 x i2048>
  %575 = extractelement <2 x i2048> %574, i32 1
  %576 = shufflevector <128 x i32> %572, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %577 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %550, <64 x float> %561, <64 x i32> %545, <64 x i32> %559, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %578 = shufflevector <128 x i32> %577, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %579 = shufflevector <128 x i32> %577, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %565, <64 x i32> addrspace(2)* %551, align 256, !tbaa !5
  %580 = bitcast <64 x float> addrspace(2)* %553 to <64 x i32> addrspace(2)*
  store <64 x i32> %568, <64 x i32> addrspace(2)* %580, align 256, !tbaa !5
  store <64 x i32> %570, <64 x i32> addrspace(2)* %556, align 256, !tbaa !5
  %581 = bitcast <64 x float> addrspace(2)* %557 to <64 x i32> addrspace(2)*
  store <64 x i32> %571, <64 x i32> addrspace(2)* %581, align 256, !tbaa !5
  store <64 x i32> %573, <64 x i32> addrspace(2)* %558, align 256, !tbaa !5
  %582 = bitcast <64 x float> addrspace(2)* %560 to <64 x i32> addrspace(2)*
  store <64 x i32> %576, <64 x i32> addrspace(2)* %582, align 256, !tbaa !5
  store <64 x i32> %578, <64 x i32> addrspace(2)* %562, align 256, !tbaa !5
  %583 = bitcast <64 x float> addrspace(2)* %563 to <64 x i32> addrspace(2)*
  store <64 x i32> %579, <64 x i32> addrspace(2)* %583, align 256, !tbaa !5
  %584 = add nsw i32 %548, -1
  %585 = icmp sgt i32 %548, 0
  br i1 %585, label %543, label %586

586:                                              ; preds = %543
  br label %587

587:                                              ; preds = %586, %242
  %588 = or i32 %216, 2
  %589 = load <64 x i32>, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  %590 = load <64 x float>, <64 x float> addrspace(2)* %66, align 256, !tbaa !5
  %591 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %592 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %593 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %588, i32 0
  %594 = load <64 x i32>, <64 x i32> addrspace(2)* %593, align 256, !tbaa !5
  %595 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %588, i32 1
  %596 = load <64 x float>, <64 x float> addrspace(2)* %595, align 256, !tbaa !5
  %597 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %588, i32 0
  %598 = load <64 x i32>, <64 x i32> addrspace(2)* %597, align 256, !tbaa !5
  %599 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %588, i32 1
  %600 = load <64 x float>, <64 x float> addrspace(2)* %599, align 256, !tbaa !5
  %601 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %596, <64 x float> %590, <64 x i32> %594, <64 x i32> %589, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %602 = shufflevector <128 x i32> %601, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %603 = bitcast <128 x i32> %601 to <2 x i2048>
  %604 = extractelement <2 x i2048> %603, i32 1
  %605 = shufflevector <128 x i32> %601, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %606 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %600, <64 x float> %592, <64 x i32> %598, <64 x i32> %591, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %607 = shufflevector <128 x i32> %606, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %608 = bitcast <128 x i32> %606 to <2 x i2048>
  %609 = extractelement <2 x i2048> %608, i32 1
  %610 = shufflevector <128 x i32> %606, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %602, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  store <64 x i32> %605, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %607, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %610, <64 x i32> addrspace(2)* %70, align 256, !tbaa !5
  br i1 %64, label %611, label %656

611:                                              ; preds = %587
  br label %612

612:                                              ; preds = %611, %612
  %613 = phi i2048 [ %644, %612 ], [ %609, %611 ]
  %614 = phi <64 x i32> [ %642, %612 ], [ %607, %611 ]
  %615 = phi i2048 [ %636, %612 ], [ %604, %611 ]
  %616 = phi <64 x i32> [ %634, %612 ], [ %602, %611 ]
  %617 = phi i32 [ %653, %612 ], [ %63, %611 ]
  %618 = bitcast i2048 %615 to <64 x float>
  %619 = bitcast i2048 %613 to <64 x float>
  %620 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %617, i32 0
  %621 = load <64 x i32>, <64 x i32> addrspace(2)* %620, align 256, !tbaa !5
  %622 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %617, i32 1
  %623 = load <64 x float>, <64 x float> addrspace(2)* %622, align 256, !tbaa !5
  %624 = add nuw nsw i32 %617, 1
  %625 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %624, i32 0
  %626 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %624, i32 1
  %627 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %617, i32 0
  %628 = load <64 x i32>, <64 x i32> addrspace(2)* %627, align 256, !tbaa !5
  %629 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %617, i32 1
  %630 = load <64 x float>, <64 x float> addrspace(2)* %629, align 256, !tbaa !5
  %631 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %624, i32 0
  %632 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %624, i32 1
  %633 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %618, <64 x float> %623, <64 x i32> %616, <64 x i32> %621, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %634 = shufflevector <128 x i32> %633, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %635 = bitcast <128 x i32> %633 to <2 x i2048>
  %636 = extractelement <2 x i2048> %635, i32 1
  %637 = shufflevector <128 x i32> %633, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %638 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %618, <64 x float> %623, <64 x i32> %616, <64 x i32> %621, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %639 = shufflevector <128 x i32> %638, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %640 = shufflevector <128 x i32> %638, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %641 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %619, <64 x float> %630, <64 x i32> %614, <64 x i32> %628, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %642 = shufflevector <128 x i32> %641, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %643 = bitcast <128 x i32> %641 to <2 x i2048>
  %644 = extractelement <2 x i2048> %643, i32 1
  %645 = shufflevector <128 x i32> %641, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %646 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %619, <64 x float> %630, <64 x i32> %614, <64 x i32> %628, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %647 = shufflevector <128 x i32> %646, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %648 = shufflevector <128 x i32> %646, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %634, <64 x i32> addrspace(2)* %620, align 256, !tbaa !5
  %649 = bitcast <64 x float> addrspace(2)* %622 to <64 x i32> addrspace(2)*
  store <64 x i32> %637, <64 x i32> addrspace(2)* %649, align 256, !tbaa !5
  store <64 x i32> %639, <64 x i32> addrspace(2)* %625, align 256, !tbaa !5
  %650 = bitcast <64 x float> addrspace(2)* %626 to <64 x i32> addrspace(2)*
  store <64 x i32> %640, <64 x i32> addrspace(2)* %650, align 256, !tbaa !5
  store <64 x i32> %642, <64 x i32> addrspace(2)* %627, align 256, !tbaa !5
  %651 = bitcast <64 x float> addrspace(2)* %629 to <64 x i32> addrspace(2)*
  store <64 x i32> %645, <64 x i32> addrspace(2)* %651, align 256, !tbaa !5
  store <64 x i32> %647, <64 x i32> addrspace(2)* %631, align 256, !tbaa !5
  %652 = bitcast <64 x float> addrspace(2)* %632 to <64 x i32> addrspace(2)*
  store <64 x i32> %648, <64 x i32> addrspace(2)* %652, align 256, !tbaa !5
  %653 = add nsw i32 %617, -1
  %654 = icmp sgt i32 %617, 0
  br i1 %654, label %612, label %655

655:                                              ; preds = %612
  br label %656

656:                                              ; preds = %655, %587
  %657 = or i32 %216, 3
  %658 = load <64 x i32>, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  %659 = load <64 x float>, <64 x float> addrspace(2)* %66, align 256, !tbaa !5
  %660 = load <64 x i32>, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  %661 = load <64 x float>, <64 x float> addrspace(2)* %68, align 256, !tbaa !5
  %662 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %657, i32 0
  %663 = load <64 x i32>, <64 x i32> addrspace(2)* %662, align 256, !tbaa !5
  %664 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %61, i32 %657, i32 1
  %665 = load <64 x float>, <64 x float> addrspace(2)* %664, align 256, !tbaa !5
  %666 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %657, i32 0
  %667 = load <64 x i32>, <64 x i32> addrspace(2)* %666, align 256, !tbaa !5
  %668 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %62, i32 %657, i32 1
  %669 = load <64 x float>, <64 x float> addrspace(2)* %668, align 256, !tbaa !5
  %670 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %665, <64 x float> %659, <64 x i32> %663, <64 x i32> %658, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %671 = shufflevector <128 x i32> %670, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %672 = bitcast <128 x i32> %670 to <2 x i2048>
  %673 = extractelement <2 x i2048> %672, i32 1
  %674 = shufflevector <128 x i32> %670, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %675 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %669, <64 x float> %661, <64 x i32> %667, <64 x i32> %660, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %676 = shufflevector <128 x i32> %675, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %677 = bitcast <128 x i32> %675 to <2 x i2048>
  %678 = extractelement <2 x i2048> %677, i32 1
  %679 = shufflevector <128 x i32> %675, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %671, <64 x i32> addrspace(2)* %65, align 256, !tbaa !5
  store <64 x i32> %674, <64 x i32> addrspace(2)* %69, align 256, !tbaa !5
  store <64 x i32> %676, <64 x i32> addrspace(2)* %67, align 256, !tbaa !5
  store <64 x i32> %679, <64 x i32> addrspace(2)* %70, align 256, !tbaa !5
  br i1 %64, label %680, label %725

680:                                              ; preds = %656
  br label %681

681:                                              ; preds = %680, %681
  %682 = phi i2048 [ %713, %681 ], [ %678, %680 ]
  %683 = phi <64 x i32> [ %711, %681 ], [ %676, %680 ]
  %684 = phi i2048 [ %705, %681 ], [ %673, %680 ]
  %685 = phi <64 x i32> [ %703, %681 ], [ %671, %680 ]
  %686 = phi i32 [ %722, %681 ], [ %63, %680 ]
  %687 = bitcast i2048 %684 to <64 x float>
  %688 = bitcast i2048 %682 to <64 x float>
  %689 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %686, i32 0
  %690 = load <64 x i32>, <64 x i32> addrspace(2)* %689, align 256, !tbaa !5
  %691 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %686, i32 1
  %692 = load <64 x float>, <64 x float> addrspace(2)* %691, align 256, !tbaa !5
  %693 = add nuw nsw i32 %686, 1
  %694 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %693, i32 0
  %695 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %59, i32 %693, i32 1
  %696 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %686, i32 0
  %697 = load <64 x i32>, <64 x i32> addrspace(2)* %696, align 256, !tbaa !5
  %698 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %686, i32 1
  %699 = load <64 x float>, <64 x float> addrspace(2)* %698, align 256, !tbaa !5
  %700 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %693, i32 0
  %701 = getelementptr inbounds %struct._uint64_float64_pair_t, %struct._uint64_float64_pair_t addrspace(2)* %60, i32 %693, i32 1
  %702 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %687, <64 x float> %692, <64 x i32> %685, <64 x i32> %690, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %703 = shufflevector <128 x i32> %702, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %704 = bitcast <128 x i32> %702 to <2 x i2048>
  %705 = extractelement <2 x i2048> %704, i32 1
  %706 = shufflevector <128 x i32> %702, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %707 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %687, <64 x float> %692, <64 x i32> %685, <64 x i32> %690, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %708 = shufflevector <128 x i32> %707, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %709 = shufflevector <128 x i32> %707, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %710 = tail call <128 x i32> @llvm.tpc.sel2.grt.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %688, <64 x float> %699, <64 x i32> %683, <64 x i32> %697, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %711 = shufflevector <128 x i32> %710, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %712 = bitcast <128 x i32> %710 to <2 x i2048>
  %713 = extractelement <2 x i2048> %712, i32 1
  %714 = shufflevector <128 x i32> %710, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %715 = tail call <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float> %688, <64 x float> %699, <64 x i32> %683, <64 x i32> %697, i8 0, i32 0, <128 x i32> undef, i1 true, i1 false) #5
  %716 = shufflevector <128 x i32> %715, <128 x i32> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %717 = shufflevector <128 x i32> %715, <128 x i32> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  store <64 x i32> %703, <64 x i32> addrspace(2)* %689, align 256, !tbaa !5
  %718 = bitcast <64 x float> addrspace(2)* %691 to <64 x i32> addrspace(2)*
  store <64 x i32> %706, <64 x i32> addrspace(2)* %718, align 256, !tbaa !5
  store <64 x i32> %708, <64 x i32> addrspace(2)* %694, align 256, !tbaa !5
  %719 = bitcast <64 x float> addrspace(2)* %695 to <64 x i32> addrspace(2)*
  store <64 x i32> %709, <64 x i32> addrspace(2)* %719, align 256, !tbaa !5
  store <64 x i32> %711, <64 x i32> addrspace(2)* %696, align 256, !tbaa !5
  %720 = bitcast <64 x float> addrspace(2)* %698 to <64 x i32> addrspace(2)*
  store <64 x i32> %714, <64 x i32> addrspace(2)* %720, align 256, !tbaa !5
  store <64 x i32> %716, <64 x i32> addrspace(2)* %700, align 256, !tbaa !5
  %721 = bitcast <64 x float> addrspace(2)* %701 to <64 x i32> addrspace(2)*
  store <64 x i32> %717, <64 x i32> addrspace(2)* %721, align 256, !tbaa !5
  %722 = add nsw i32 %686, -1
  %723 = icmp sgt i32 %686, 0
  br i1 %723, label %681, label %724

724:                                              ; preds = %681
  br label %725

725:                                              ; preds = %724, %656
  %726 = add nuw nsw i32 %216, 4
  %727 = add i32 %217, -4
  %728 = icmp eq i32 %727, 0
  br i1 %728, label %310, label %215, !llvm.loop !11
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
declare <128 x half> @llvm.tpc.ld.tnsr.v128f16.i1(<5 x i32>, i8, i32, <128 x half>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.convert.v128f32.v128f16.i1(<128 x half>, i8, i32, <128 x float>, i1, i1) #2

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
declare <128 x i32> @llvm.tpc.sel2.leq.v128i32.v64f32.v64f32.v64i32.v64i32.i1(<64 x float>, <64 x float>, <64 x i32>, <64 x i32>, i8, i32, <128 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="greco" "target-features"="+greco" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
