; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 1, 1, 1, 1, 1 }[1].[Input].{ 1, 1, 1, 1, 1 }[2].[Output].{ 1, 1, 1, 1, 1 }[3].[Output].{ 1, 1, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 1, 1, 1, 1, 1 }[1].[Input].{ 1, 1, 1, 1, 1 }[2].[Output].{ 1, 1, 1, 1, 1 }[3].[Output].{ 1, 1, 1, 1, 1 } #SCEVEND"

; ModuleID = './goya/gnmt/top_k_st2_dim0_i16.c'
source_filename = "./goya/gnmt/top_k_st2_dim0_i16.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

%struct._ushort128_short128_pair_t = type { <128 x i16>, <128 x i16> }

@localArray = external dso_local addrspace(2) global [37 x %struct._ushort128_short128_pair_t], section ".vldata"

; Function Attrs: nounwind
define dso_local void @main(i32 %bsw) local_unnamed_addr #0 {
entry:
  %0 = bitcast [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null to i8 addrspace(2)*
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %2 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %2, %1
  %3 = tail call i32 @llvm.tpc.ld.l.i32(i32 1040, i32 1, i32 undef, i1 true, i1 false) #5
  %vecext = extractelement <5 x i32> %1, i32 1
  %vecext1 = extractelement <5 x i32> %add, i32 1
  %vecext2 = extractelement <5 x i32> %1, i32 2
  %vecext3 = extractelement <5 x i32> %add, i32 2
  %vecext4 = extractelement <5 x i32> %1, i32 3
  %vecext5 = extractelement <5 x i32> %add, i32 3
  %vecext6 = extractelement <5 x i32> %1, i32 4
  %vecext7 = extractelement <5 x i32> %add, i32 4
  %cmp247 = icmp slt i32 %vecext6, %vecext7
  br i1 %cmp247, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp10241 = icmp slt i32 %vecext4, %vecext5
  %cmp16235 = icmp slt i32 %vecext2, %vecext3
  %cmp22229 = icmp slt i32 %vecext, %vecext1
  %cmp29216 = icmp sgt i32 %bsw, 0
  %cmp37221 = icmp sgt i32 %3, %bsw
  %sub = add nsw i32 %bsw, -1
  %v141266 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %sub, i32 0
  %v244270 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %sub, i32 1
  %sub50 = add nsw i32 %bsw, -2
  %cmp52219 = icmp sgt i32 %bsw, 1
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup11
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup11, %for.body.lr.ph
  %f.0250 = phi i32 [ %vecext6, %for.body.lr.ph ], [ %add97, %for.cond.cleanup11 ]
  %loadCoords.0249 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %loadCoords.1.lcssa, %for.cond.cleanup11 ]
  %storeCoords.0248 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %storeCoords.1.lcssa, %for.cond.cleanup11 ]
  %vecins = insertelement <5 x i32> %loadCoords.0249, i32 %f.0250, i32 4
  %vecins8 = insertelement <5 x i32> %storeCoords.0248, i32 %f.0250, i32 4
  br i1 %cmp10241, label %for.body12.preheader, label %for.cond.cleanup11

for.body12.preheader:                             ; preds = %for.body
  br label %for.body12

for.cond.cleanup11.loopexit:                      ; preds = %for.cond.cleanup17
  %storeCoords.2.lcssa.lcssa = phi <5 x i32> [ %storeCoords.2.lcssa, %for.cond.cleanup17 ]
  %loadCoords.2.lcssa.lcssa = phi <5 x i32> [ %loadCoords.2.lcssa, %for.cond.cleanup17 ]
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.body
  %storeCoords.1.lcssa = phi <5 x i32> [ %vecins8, %for.body ], [ %storeCoords.2.lcssa.lcssa, %for.cond.cleanup11.loopexit ]
  %loadCoords.1.lcssa = phi <5 x i32> [ %vecins, %for.body ], [ %loadCoords.2.lcssa.lcssa, %for.cond.cleanup11.loopexit ]
  %add97 = add i32 %f.0250, 1
  %exitcond257 = icmp eq i32 %add97, %vecext7
  br i1 %exitcond257, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !4

for.body12:                                       ; preds = %for.body12.preheader, %for.cond.cleanup17
  %b.0244 = phi i32 [ %add94, %for.cond.cleanup17 ], [ %vecext4, %for.body12.preheader ]
  %loadCoords.1243 = phi <5 x i32> [ %loadCoords.2.lcssa, %for.cond.cleanup17 ], [ %vecins, %for.body12.preheader ]
  %storeCoords.1242 = phi <5 x i32> [ %storeCoords.2.lcssa, %for.cond.cleanup17 ], [ %vecins8, %for.body12.preheader ]
  %vecins13 = insertelement <5 x i32> %loadCoords.1243, i32 %b.0244, i32 3
  %vecins14 = insertelement <5 x i32> %storeCoords.1242, i32 %b.0244, i32 3
  br i1 %cmp16235, label %for.body18.preheader, label %for.cond.cleanup17

for.body18.preheader:                             ; preds = %for.body12
  br label %for.body18

for.cond.cleanup17.loopexit:                      ; preds = %for.cond.cleanup23
  %storeCoords.3.lcssa.lcssa = phi <5 x i32> [ %storeCoords.3.lcssa, %for.cond.cleanup23 ]
  %loadCoords.3.lcssa.lcssa = phi <5 x i32> [ %loadCoords.3.lcssa, %for.cond.cleanup23 ]
  br label %for.cond.cleanup17

for.cond.cleanup17:                               ; preds = %for.cond.cleanup17.loopexit, %for.body12
  %storeCoords.2.lcssa = phi <5 x i32> [ %vecins14, %for.body12 ], [ %storeCoords.3.lcssa.lcssa, %for.cond.cleanup17.loopexit ]
  %loadCoords.2.lcssa = phi <5 x i32> [ %vecins13, %for.body12 ], [ %loadCoords.3.lcssa.lcssa, %for.cond.cleanup17.loopexit ]
  %add94 = add i32 %b.0244, 1
  %exitcond256 = icmp eq i32 %add94, %vecext5
  br i1 %exitcond256, label %for.cond.cleanup11.loopexit, label %for.body12, !llvm.loop !6

for.body18:                                       ; preds = %for.body18.preheader, %for.cond.cleanup23
  %h.0238 = phi i32 [ %add91, %for.cond.cleanup23 ], [ %vecext2, %for.body18.preheader ]
  %loadCoords.2237 = phi <5 x i32> [ %loadCoords.3.lcssa, %for.cond.cleanup23 ], [ %vecins13, %for.body18.preheader ]
  %storeCoords.2236 = phi <5 x i32> [ %storeCoords.3.lcssa, %for.cond.cleanup23 ], [ %vecins14, %for.body18.preheader ]
  %vecins19 = insertelement <5 x i32> %loadCoords.2237, i32 %h.0238, i32 2
  %vecins20 = insertelement <5 x i32> %storeCoords.2236, i32 %h.0238, i32 2
  br i1 %cmp22229, label %for.body24.preheader, label %for.cond.cleanup23

for.body24.preheader:                             ; preds = %for.body18
  br label %for.body24

for.cond.cleanup23.loopexit:                      ; preds = %for.cond.cleanup77
  %storeCoords.4.lcssa.lcssa = phi <5 x i32> [ %storeCoords.4.lcssa, %for.cond.cleanup77 ]
  %loadCoords.5.lcssa.lcssa = phi <5 x i32> [ %loadCoords.5.lcssa, %for.cond.cleanup77 ]
  br label %for.cond.cleanup23

for.cond.cleanup23:                               ; preds = %for.cond.cleanup23.loopexit, %for.body18
  %storeCoords.3.lcssa = phi <5 x i32> [ %vecins20, %for.body18 ], [ %storeCoords.4.lcssa.lcssa, %for.cond.cleanup23.loopexit ]
  %loadCoords.3.lcssa = phi <5 x i32> [ %vecins19, %for.body18 ], [ %loadCoords.5.lcssa.lcssa, %for.cond.cleanup23.loopexit ]
  %add91 = add i32 %h.0238, 1
  %exitcond255 = icmp eq i32 %add91, %vecext3
  br i1 %exitcond255, label %for.cond.cleanup17.loopexit, label %for.body18, !llvm.loop !7

for.body24:                                       ; preds = %for.body24.preheader, %for.cond.cleanup77
  %w.0232 = phi i32 [ %add88, %for.cond.cleanup77 ], [ %vecext, %for.body24.preheader ]
  %loadCoords.3231 = phi <5 x i32> [ %loadCoords.5.lcssa, %for.cond.cleanup77 ], [ %vecins19, %for.body24.preheader ]
  %storeCoords.3230 = phi <5 x i32> [ %storeCoords.4.lcssa, %for.cond.cleanup77 ], [ %vecins20, %for.body24.preheader ]
  %vecins26 = insertelement <5 x i32> %storeCoords.3230, i32 %w.0232, i32 1
  %4 = insertelement <5 x i32> %loadCoords.3231, i32 0, i32 0
  %vecins27 = insertelement <5 x i32> %4, i32 %w.0232, i32 1
  br i1 %cmp29216, label %for.body31.preheader, label %for.cond36.preheader

for.body31.preheader:                             ; preds = %for.body24
  br label %for.body31

for.cond36.preheader.loopexit:                    ; preds = %for.body31
  %.lcssa = phi <5 x i32> [ %7, %for.body31 ]
  br label %for.cond36.preheader

for.cond36.preheader:                             ; preds = %for.cond36.preheader.loopexit, %for.body24
  %loadCoords.4.lcssa = phi <5 x i32> [ %vecins27, %for.body24 ], [ %.lcssa, %for.cond36.preheader.loopexit ]
  br i1 %cmp37221, label %for.body39.preheader, label %for.cond75.preheader

for.body39.preheader:                             ; preds = %for.cond36.preheader
  br label %for.body39

for.body31:                                       ; preds = %for.body31.preheader, %for.body31
  %j.0218 = phi i32 [ %inc, %for.body31 ], [ 0, %for.body31.preheader ]
  %loadCoords.4217 = phi <5 x i32> [ %7, %for.body31 ], [ %vecins27, %for.body31.preheader ]
  %5 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %loadCoords.4217, i8 1, i32 0, <128 x i16> undef, i1 true, i1 false)
  %v1263 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %j.0218, i32 0
  store <128 x i16> %5, <128 x i16> addrspace(2)* %v1263, align 256, !tbaa !8
  %6 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %loadCoords.4217, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %v2267 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %j.0218, i32 1
  store <128 x i16> %6, <128 x i16> addrspace(2)* %v2267, align 256, !tbaa !8
  %7 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %loadCoords.4217, i32 1, i32 1, i8 2, i32 0, <5 x i32> %loadCoords.4217, i1 true, i1 false)
  %inc = add nuw nsw i32 %j.0218, 1
  %exitcond = icmp eq i32 %inc, %bsw
  br i1 %exitcond, label %for.cond36.preheader.loopexit, label %for.body31

for.cond75.preheader.loopexit:                    ; preds = %for.cond.cleanup53
  %.lcssa262 = phi <5 x i32> [ %12, %for.cond.cleanup53 ]
  br label %for.cond75.preheader

for.cond75.preheader:                             ; preds = %for.cond75.preheader.loopexit, %for.cond36.preheader
  %loadCoords.5.lcssa = phi <5 x i32> [ %loadCoords.4.lcssa, %for.cond36.preheader ], [ %.lcssa262, %for.cond75.preheader.loopexit ]
  br i1 %cmp29216, label %for.body78.preheader, label %for.cond.cleanup77

for.body78.preheader:                             ; preds = %for.cond75.preheader
  br label %for.body78

for.body39:                                       ; preds = %for.body39.preheader, %for.cond.cleanup53
  %col.0223 = phi i32 [ %add73, %for.cond.cleanup53 ], [ %bsw, %for.body39.preheader ]
  %loadCoords.5222 = phi <5 x i32> [ %12, %for.cond.cleanup53 ], [ %loadCoords.4.lcssa, %for.body39.preheader ]
  %8 = load <128 x i16>, <128 x i16> addrspace(2)* %v141266, align 256, !tbaa !8
  %9 = load <128 x i16>, <128 x i16> addrspace(2)* %v244270, align 256, !tbaa !8
  %10 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %loadCoords.5222, i8 1, i32 0, <128 x i16> undef, i1 true, i1 false)
  %11 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %loadCoords.5222, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  %12 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %loadCoords.5222, i32 1, i32 1, i8 2, i32 0, <5 x i32> %loadCoords.5222, i1 true, i1 false)
  %13 = tail call <256 x i16> @llvm.tpc.sel2.grt.v256i16.v128i16.v128i16.v128i16.v128i16.i1(<128 x i16> %11, <128 x i16> %9, <128 x i16> %10, <128 x i16> %8, i8 7, i32 0, <256 x i16> undef, i1 true, i1 false)
  %14 = shufflevector <256 x i16> %13, <256 x i16> undef, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %15 = shufflevector <256 x i16> %13, <256 x i16> undef, <128 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  store <128 x i16> %14, <128 x i16> addrspace(2)* %v141266, align 256, !tbaa.struct !11
  store <128 x i16> %15, <128 x i16> addrspace(2)* %v244270, align 256, !tbaa.struct !11
  br i1 %cmp52219, label %for.body54.preheader, label %for.cond.cleanup53

for.body54.preheader:                             ; preds = %for.body39
  br label %for.body54

for.cond.cleanup53.loopexit:                      ; preds = %for.body54
  br label %for.cond.cleanup53

for.cond.cleanup53:                               ; preds = %for.cond.cleanup53.loopexit, %for.body39
  %add73 = add nsw i32 %col.0223, 1
  %exitcond252 = icmp eq i32 %add73, %3
  br i1 %exitcond252, label %for.cond75.preheader.loopexit, label %for.body39

for.body54:                                       ; preds = %for.body54.preheader, %for.body54
  %16 = phi <128 x i16> [ %22, %for.body54 ], [ %15, %for.body54.preheader ]
  %17 = phi <128 x i16> [ %21, %for.body54 ], [ %14, %for.body54.preheader ]
  %i.0220 = phi i32 [ %dec, %for.body54 ], [ %sub50, %for.body54.preheader ]
  %v156265 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %i.0220, i32 0
  %18 = load <128 x i16>, <128 x i16> addrspace(2)* %v156265, align 256, !tbaa !8
  %v258269 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %i.0220, i32 1
  %19 = load <128 x i16>, <128 x i16> addrspace(2)* %v258269, align 256, !tbaa !8
  %add59 = add nuw nsw i32 %i.0220, 1
  %v161264 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %add59, i32 0
  %v264268 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %add59, i32 1
  %20 = tail call <256 x i16> @llvm.tpc.sel2.grt.v256i16.v128i16.v128i16.v128i16.v128i16.i1(<128 x i16> %16, <128 x i16> %19, <128 x i16> %17, <128 x i16> %18, i8 7, i32 0, <256 x i16> undef, i1 true, i1 false)
  %21 = shufflevector <256 x i16> %20, <256 x i16> undef, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %22 = shufflevector <256 x i16> %20, <256 x i16> undef, <128 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  %23 = tail call <256 x i16> @llvm.tpc.sel2.leq.v256i16.v128i16.v128i16.v128i16.v128i16.i1(<128 x i16> %16, <128 x i16> %19, <128 x i16> %17, <128 x i16> %18, i8 7, i32 0, <256 x i16> undef, i1 true, i1 false)
  %24 = shufflevector <256 x i16> %23, <256 x i16> undef, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %25 = shufflevector <256 x i16> %23, <256 x i16> undef, <128 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  store <128 x i16> %21, <128 x i16> addrspace(2)* %v156265, align 256, !tbaa.struct !11
  store <128 x i16> %22, <128 x i16> addrspace(2)* %v258269, align 256, !tbaa.struct !11
  store <128 x i16> %24, <128 x i16> addrspace(2)* %v161264, align 256, !tbaa.struct !11
  store <128 x i16> %25, <128 x i16> addrspace(2)* %v264268, align 256, !tbaa.struct !11
  %dec = add nsw i32 %i.0220, -1
  %cmp52 = icmp sgt i32 %i.0220, 0
  br i1 %cmp52, label %for.body54, label %for.cond.cleanup53.loopexit

for.cond.cleanup77.loopexit:                      ; preds = %for.body78
  %vecins79.lcssa = phi <5 x i32> [ %vecins79, %for.body78 ]
  br label %for.cond.cleanup77

for.cond.cleanup77:                               ; preds = %for.cond.cleanup77.loopexit, %for.cond75.preheader
  %storeCoords.4.lcssa = phi <5 x i32> [ %vecins26, %for.cond75.preheader ], [ %vecins79.lcssa, %for.cond.cleanup77.loopexit ]
  %add88 = add i32 %w.0232, 1
  %exitcond254 = icmp eq i32 %add88, %vecext1
  br i1 %exitcond254, label %for.cond.cleanup23.loopexit, label %for.body24, !llvm.loop !12

for.body78:                                       ; preds = %for.body78.preheader, %for.body78
  %k.0227 = phi i32 [ %inc85, %for.body78 ], [ 0, %for.body78.preheader ]
  %storeCoords.4226 = phi <5 x i32> [ %vecins79, %for.body78 ], [ %vecins26, %for.body78.preheader ]
  %vecins79 = insertelement <5 x i32> %storeCoords.4226, i32 %k.0227, i32 0
  %v281272 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %k.0227, i32 1
  %26 = load <128 x i16>, <128 x i16> addrspace(2)* %v281272, align 256, !tbaa !8
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins79, i8 2, <128 x i16> %26, i32 0, i1 true, i1 false)
  %v183271 = getelementptr inbounds [37 x %struct._ushort128_short128_pair_t], [37 x %struct._ushort128_short128_pair_t] addrspace(2)* null, i32 0, i32 %k.0227, i32 0
  %27 = load <128 x i16>, <128 x i16> addrspace(2)* %v183271, align 256, !tbaa !8
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %vecins79, i8 3, <128 x i16> %27, i32 0, i1 true, i1 false)
  %inc85 = add nuw nsw i32 %k.0227, 1
  %exitcond253 = icmp eq i32 %inc85, %bsw
  br i1 %exitcond253, label %for.cond.cleanup77.loopexit, label %for.body78
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #2

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <256 x i16> @llvm.tpc.sel2.grt.v256i16.v128i16.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, <128 x i16>, <128 x i16>, i8, i32, <256 x i16>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <256 x i16> @llvm.tpc.sel2.leq.v256i16.v128i16.v128i16.v128i16.v128i16.i1(<128 x i16>, <128 x i16>, <128 x i16>, <128 x i16>, i8, i32, <256 x i16>, i1, i1) #2

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #3

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #4

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind readonly }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 75e9e281e80e2cdbb12ac136263d6ab9d052ab60)"}
!2 = !{i32 0}
!3 = !{i32 18944}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{i64 0, i64 256, !8, i64 256, i64 256, !8}
!12 = distinct !{!12, !5}
