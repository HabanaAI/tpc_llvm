; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -S -loop-swp -loop-software-pipelining=true | FileCheck %s

target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(float %epsilon, i32 %axis) local_unnamed_addr #0 {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
; CHECK-NEXT:    [[TMP1:%.*]] = tail call <5 x i32> @llvm.tpc.get.index.space.size()
; CHECK-NEXT:    [[ADD:%.*]] = add <5 x i32> [[TMP1]], [[TMP0]]
; CHECK-NEXT:    [[VECEXT:%.*]] = extractelement <5 x i32> [[TMP0]], i32 1
; CHECK-NEXT:    [[MUL:%.*]] = shl nsw i32 [[VECEXT]], 2
; CHECK-NEXT:    [[VECEXT1:%.*]] = extractelement <5 x i32> [[ADD]], i32 1
; CHECK-NEXT:    [[MUL2:%.*]] = shl i32 [[VECEXT1]], 2
; CHECK-NEXT:    [[VECEXT3:%.*]] = extractelement <5 x i32> [[TMP0]], i32 2
; CHECK-NEXT:    [[VECEXT5:%.*]] = extractelement <5 x i32> [[ADD]], i32 2
; CHECK-NEXT:    [[CMP61:%.*]] = icmp slt i32 [[VECEXT3]], [[VECEXT5]]
; CHECK-NEXT:    br i1 [[CMP61]], label [[FOR_BODY_LR_PH:%.*]], label [[FOR_COND_CLEANUP:%.*]]
; CHECK:       for.body.lr.ph:
; CHECK-NEXT:    [[CMP858:%.*]] = icmp slt i32 [[MUL]], [[MUL2]]
; CHECK-NEXT:    br label [[FOR_BODY:%.*]]
; CHECK:       for.cond.cleanup.loopexit:
; CHECK-NEXT:    br label [[FOR_COND_CLEANUP]]
; CHECK:       for.cond.cleanup:
; CHECK-NEXT:    ret void
; CHECK:       for.body:
; CHECK-NEXT:    [[H_063:%.*]] = phi i32 [ [[VECEXT3]], [[FOR_BODY_LR_PH]] ], [ [[ADD13:%.*]], [[FOR_COND_CLEANUP9:%.*]] ]
; CHECK-NEXT:    [[COORDS0_062:%.*]] = phi <5 x i32> [ zeroinitializer, [[FOR_BODY_LR_PH]] ], [ [[COORDS0_1_LCSSA:%.*]], [[FOR_COND_CLEANUP9]] ]
; CHECK-NEXT:    [[TMP2:%.*]] = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> [[COORDS0_062]], i32 4, i32 [[H_063]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP3:%.*]] = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> [[TMP2]], i32 2, i32 0, i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP4:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP3]], i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP5:%.*]] = fmul <64 x float> [[TMP4]], [[TMP4]]
; CHECK-NEXT:    [[TMP6:%.*]] = fmul <64 x float> [[TMP4]], [[TMP5]]
; CHECK-NEXT:    br i1 [[CMP858]], label [[FOR_BODY10_PREHEADER:%.*]], label [[FOR_COND_CLEANUP9]]
; CHECK:       for.body10.preheader:
; CHECK-NEXT:    [[TMP7:%.*]] = phi <5 x i32> [ [[TMP3]], [[FOR_BODY]] ]
; CHECK-NEXT:    [[TMP8:%.*]] = phi i32 [ [[MUL]], [[FOR_BODY]] ]
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <5 x i32> [[TMP7]], i32 [[TMP8]], i32 1
; CHECK-NEXT:    [[TMP10:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP9]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP11:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP9]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP12:%.*]] = or i32 [[TMP8]], 1
; CHECK-NEXT:    [[TMP13:%.*]] = insertelement <5 x i32> [[TMP9]], i32 [[TMP12]], i32 1
; CHECK-NEXT:    [[TMP14:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP13]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP15:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP13]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP16:%.*]] = or i32 [[TMP8]], 2
; CHECK-NEXT:    [[TMP17:%.*]] = insertelement <5 x i32> [[TMP13]], i32 [[TMP16]], i32 1
; CHECK-NEXT:    [[TMP18:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP17]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP19:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP17]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP20:%.*]] = or i32 [[TMP8]], 3
; CHECK-NEXT:    [[TMP21:%.*]] = insertelement <5 x i32> [[TMP17]], i32 [[TMP20]], i32 1
; CHECK-NEXT:    [[TMP22:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP21]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP23:%.*]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[TMP21]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP24:%.*]] = add i32 [[TMP8]], 4
; CHECK-NEXT:    [[TMP25:%.*]] = icmp eq i32 [[TMP24]], [[MUL2]]
; CHECK-NEXT:    br i1 [[TMP25]], label [[FOR_COND_CLEANUP9_LOOPEXIT:%.*]], label [[FOR_BODY10:%.*]]
; CHECK:       for.cond.cleanup9.loopexit:
; CHECK-NEXT:    [[TMP26:%.*]] = phi i32 [ [[MUL]], [[FOR_BODY10_PREHEADER]] ], [ [[TMP108:%.*]], [[FOR_BODY10]] ]
; CHECK-NEXT:    [[TMP27:%.*]] = phi <5 x i32> [ [[TMP3]], [[FOR_BODY10_PREHEADER]] ], [ [[TMP100:%.*]], [[FOR_BODY10]] ]
; CHECK-NEXT:    [[VECINS_3_LCSSA:%.*]] = phi <5 x i32> [ [[VECINS_3:%.*]], [[FOR_BODY10]] ], [ [[TMP21]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP28:%.*]] = phi <64 x float> [ [[TMP74:%.*]], [[FOR_BODY10]] ], [ [[TMP10]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP29:%.*]] = phi <64 x float> [ [[TMP75:%.*]], [[FOR_BODY10]] ], [ [[TMP11]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP30:%.*]] = phi <64 x float> [ [[TMP83:%.*]], [[FOR_BODY10]] ], [ [[TMP14]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP31:%.*]] = phi <64 x float> [ [[TMP84:%.*]], [[FOR_BODY10]] ], [ [[TMP15]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP32:%.*]] = phi <64 x float> [ [[TMP92:%.*]], [[FOR_BODY10]] ], [ [[TMP18]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP33:%.*]] = phi <64 x float> [ [[TMP93:%.*]], [[FOR_BODY10]] ], [ [[TMP19]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP34:%.*]] = phi <64 x float> [ [[TMP101:%.*]], [[FOR_BODY10]] ], [ [[TMP22]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP35:%.*]] = phi <64 x float> [ [[TMP102:%.*]], [[FOR_BODY10]] ], [ [[TMP23]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP36:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP28]])
; CHECK-NEXT:    [[TMP37:%.*]] = fmul <64 x float> [[TMP6]], [[TMP28]]
; CHECK-NEXT:    [[TMP38:%.*]] = fmul <64 x float> [[TMP37]], [[TMP36]]
; CHECK-NEXT:    [[TMP39:%.*]] = fsub <64 x float> [[TMP4]], [[TMP38]]
; CHECK-NEXT:    [[TMP40:%.*]] = fmul <64 x float> [[TMP29]], [[TMP39]]
; CHECK-NEXT:    [[TMP41:%.*]] = insertelement <5 x i32> [[TMP27]], i32 [[TMP26]], i32 1
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP41]], i8 3, <64 x float> [[TMP40]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP42:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP30]])
; CHECK-NEXT:    [[TMP43:%.*]] = fmul <64 x float> [[TMP6]], [[TMP30]]
; CHECK-NEXT:    [[TMP44:%.*]] = fmul <64 x float> [[TMP43]], [[TMP42]]
; CHECK-NEXT:    [[TMP45:%.*]] = fsub <64 x float> [[TMP4]], [[TMP44]]
; CHECK-NEXT:    [[TMP46:%.*]] = fmul <64 x float> [[TMP31]], [[TMP45]]
; CHECK-NEXT:    [[TMP47:%.*]] = or i32 [[TMP26]], 1
; CHECK-NEXT:    [[TMP48:%.*]] = insertelement <5 x i32> [[TMP41]], i32 [[TMP47]], i32 1
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP48]], i8 3, <64 x float> [[TMP46]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP49:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP32]])
; CHECK-NEXT:    [[TMP50:%.*]] = fmul <64 x float> [[TMP6]], [[TMP32]]
; CHECK-NEXT:    [[TMP51:%.*]] = fmul <64 x float> [[TMP50]], [[TMP49]]
; CHECK-NEXT:    [[TMP52:%.*]] = fsub <64 x float> [[TMP4]], [[TMP51]]
; CHECK-NEXT:    [[TMP53:%.*]] = fmul <64 x float> [[TMP33]], [[TMP52]]
; CHECK-NEXT:    [[TMP54:%.*]] = or i32 [[TMP26]], 2
; CHECK-NEXT:    [[TMP55:%.*]] = insertelement <5 x i32> [[TMP48]], i32 [[TMP54]], i32 1
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP55]], i8 3, <64 x float> [[TMP53]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP56:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP34]])
; CHECK-NEXT:    [[TMP57:%.*]] = fmul <64 x float> [[TMP6]], [[TMP34]]
; CHECK-NEXT:    [[TMP58:%.*]] = fmul <64 x float> [[TMP57]], [[TMP56]]
; CHECK-NEXT:    [[TMP59:%.*]] = fsub <64 x float> [[TMP4]], [[TMP58]]
; CHECK-NEXT:    [[TMP60:%.*]] = fmul <64 x float> [[TMP35]], [[TMP59]]
; CHECK-NEXT:    [[TMP61:%.*]] = or i32 [[TMP26]], 3
; CHECK-NEXT:    [[TMP62:%.*]] = insertelement <5 x i32> [[TMP55]], i32 [[TMP61]], i32 1
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP62]], i8 3, <64 x float> [[TMP60]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    br label [[FOR_COND_CLEANUP9]]
; CHECK:       for.cond.cleanup9:
; CHECK-NEXT:    [[COORDS0_1_LCSSA]] = phi <5 x i32> [ [[TMP3]], [[FOR_BODY]] ], [ [[VECINS_3_LCSSA]], [[FOR_COND_CLEANUP9_LOOPEXIT]] ]
; CHECK-NEXT:    [[ADD13]] = add i32 [[H_063]], 1
; CHECK-NEXT:    [[EXITCOND64:%.*]] = icmp eq i32 [[ADD13]], [[VECEXT5]]
; CHECK-NEXT:    br i1 [[EXITCOND64]], label [[FOR_COND_CLEANUP_LOOPEXIT:%.*]], label [[FOR_BODY]]
; CHECK:       for.body10:
; CHECK-NEXT:    [[TMP63:%.*]] = phi <64 x float> [ [[TMP74]], [[FOR_BODY10]] ], [ [[TMP10]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP64:%.*]] = phi <64 x float> [ [[TMP75]], [[FOR_BODY10]] ], [ [[TMP11]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP65:%.*]] = phi <64 x float> [ [[TMP83]], [[FOR_BODY10]] ], [ [[TMP14]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP66:%.*]] = phi <64 x float> [ [[TMP84]], [[FOR_BODY10]] ], [ [[TMP15]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP67:%.*]] = phi <64 x float> [ [[TMP92]], [[FOR_BODY10]] ], [ [[TMP18]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP68:%.*]] = phi <64 x float> [ [[TMP93]], [[FOR_BODY10]] ], [ [[TMP19]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP69:%.*]] = phi <64 x float> [ [[TMP101]], [[FOR_BODY10]] ], [ [[TMP22]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP70:%.*]] = phi <64 x float> [ [[TMP102]], [[FOR_BODY10]] ], [ [[TMP23]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP71:%.*]] = phi <5 x i32> [ [[TMP3]], [[FOR_BODY10_PREHEADER]] ], [ [[TMP100]], [[FOR_BODY10]] ]
; CHECK-NEXT:    [[TMP72:%.*]] = phi i32 [ [[MUL]], [[FOR_BODY10_PREHEADER]] ], [ [[TMP108]], [[FOR_BODY10]] ]
; CHECK-NEXT:    [[W_060:%.*]] = phi i32 [ [[ADD11_3:%.*]], [[FOR_BODY10]] ], [ [[TMP24]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[COORDS0_159:%.*]] = phi <5 x i32> [ [[VECINS_3]], [[FOR_BODY10]] ], [ [[TMP21]], [[FOR_BODY10_PREHEADER]] ]
; CHECK-NEXT:    [[TMP73:%.*]] = insertelement <5 x i32> [[TMP71]], i32 [[TMP72]], i32 1
; CHECK-NEXT:    [[VECINS:%.*]] = insertelement <5 x i32> [[COORDS0_159]], i32 [[W_060]], i32 1
; CHECK-NEXT:    [[TMP74]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP75]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP76:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP63]])
; CHECK-NEXT:    [[TMP77:%.*]] = fmul <64 x float> [[TMP6]], [[TMP63]]
; CHECK-NEXT:    [[TMP78:%.*]] = fmul <64 x float> [[TMP77]], [[TMP76]]
; CHECK-NEXT:    [[TMP79:%.*]] = fsub <64 x float> [[TMP4]], [[TMP78]]
; CHECK-NEXT:    [[TMP80:%.*]] = fmul <64 x float> [[TMP64]], [[TMP79]]
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP73]], i8 3, <64 x float> [[TMP80]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP81:%.*]] = or i32 [[TMP72]], 1
; CHECK-NEXT:    [[ADD11:%.*]] = or i32 [[W_060]], 1
; CHECK-NEXT:    [[TMP82:%.*]] = insertelement <5 x i32> [[TMP73]], i32 [[TMP81]], i32 1
; CHECK-NEXT:    [[VECINS_1:%.*]] = insertelement <5 x i32> [[VECINS]], i32 [[ADD11]], i32 1
; CHECK-NEXT:    [[TMP83]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_1]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP84]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_1]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP85:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP65]])
; CHECK-NEXT:    [[TMP86:%.*]] = fmul <64 x float> [[TMP6]], [[TMP65]]
; CHECK-NEXT:    [[TMP87:%.*]] = fmul <64 x float> [[TMP86]], [[TMP85]]
; CHECK-NEXT:    [[TMP88:%.*]] = fsub <64 x float> [[TMP4]], [[TMP87]]
; CHECK-NEXT:    [[TMP89:%.*]] = fmul <64 x float> [[TMP66]], [[TMP88]]
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP82]], i8 3, <64 x float> [[TMP89]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP90:%.*]] = or i32 [[TMP72]], 2
; CHECK-NEXT:    [[ADD11_1:%.*]] = or i32 [[W_060]], 2
; CHECK-NEXT:    [[TMP91:%.*]] = insertelement <5 x i32> [[TMP82]], i32 [[TMP90]], i32 1
; CHECK-NEXT:    [[VECINS_2:%.*]] = insertelement <5 x i32> [[VECINS_1]], i32 [[ADD11_1]], i32 1
; CHECK-NEXT:    [[TMP92]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_2]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP93]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_2]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP94:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP67]])
; CHECK-NEXT:    [[TMP95:%.*]] = fmul <64 x float> [[TMP6]], [[TMP67]]
; CHECK-NEXT:    [[TMP96:%.*]] = fmul <64 x float> [[TMP95]], [[TMP94]]
; CHECK-NEXT:    [[TMP97:%.*]] = fsub <64 x float> [[TMP4]], [[TMP96]]
; CHECK-NEXT:    [[TMP98:%.*]] = fmul <64 x float> [[TMP68]], [[TMP97]]
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP91]], i8 3, <64 x float> [[TMP98]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP99:%.*]] = or i32 [[TMP72]], 3
; CHECK-NEXT:    [[ADD11_2:%.*]] = or i32 [[W_060]], 3
; CHECK-NEXT:    [[TMP100]] = insertelement <5 x i32> [[TMP91]], i32 [[TMP99]], i32 1
; CHECK-NEXT:    [[VECINS_3]] = insertelement <5 x i32> [[VECINS_2]], i32 [[ADD11_2]], i32 1
; CHECK-NEXT:    [[TMP101]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_3]], i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP102]] = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[VECINS_3]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[TMP103:%.*]] = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> [[TMP69]])
; CHECK-NEXT:    [[TMP104:%.*]] = fmul <64 x float> [[TMP6]], [[TMP69]]
; CHECK-NEXT:    [[TMP105:%.*]] = fmul <64 x float> [[TMP104]], [[TMP103]]
; CHECK-NEXT:    [[TMP106:%.*]] = fsub <64 x float> [[TMP4]], [[TMP105]]
; CHECK-NEXT:    [[TMP107:%.*]] = fmul <64 x float> [[TMP70]], [[TMP106]]
; CHECK-NEXT:    tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> [[TMP100]], i8 3, <64 x float> [[TMP107]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[TMP108]] = add i32 [[TMP72]], 4
; CHECK-NEXT:    [[ADD11_3]] = add i32 [[W_060]], 4
; CHECK-NEXT:    [[EXITCOND_3:%.*]] = icmp eq i32 [[ADD11_3]], [[MUL2]]
; CHECK-NEXT:    br i1 [[EXITCOND_3]], label [[FOR_COND_CLEANUP9_LOOPEXIT]], label [[FOR_BODY10]], !llvm.loop !3
;
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 1
  %mul = shl nsw i32 %vecext, 2
  %vecext1 = extractelement <5 x i32> %add, i32 1
  %mul2 = shl i32 %vecext1, 2
  %vecext3 = extractelement <5 x i32> %0, i32 2
  %vecext5 = extractelement <5 x i32> %add, i32 2
  %cmp61 = icmp slt i32 %vecext3, %vecext5
  br i1 %cmp61, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp858 = icmp slt i32 %mul, %mul2
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup9, %for.body.lr.ph
  %h.063 = phi i32 [ %vecext3, %for.body.lr.ph ], [ %add13, %for.cond.cleanup9 ]
  %coords0.062 = phi <5 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %coords0.1.lcssa, %for.cond.cleanup9 ]
  %2 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %coords0.062, i32 4, i32 %h.063, i32 0, i1 true, i1 false)
  %3 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %2, i32 2, i32 0, i32 0, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %3, i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
  %5 = fmul <64 x float> %4, %4
  %6 = fmul <64 x float> %4, %5
  br i1 %cmp858, label %for.body10.preheader, label %for.cond.cleanup9

for.body10.preheader:                             ; preds = %for.body
  br label %for.body10

for.cond.cleanup9.loopexit:                       ; preds = %for.body10
  %vecins.3.lcssa = phi <5 x i32> [ %vecins.3, %for.body10 ]
  br label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond.cleanup9.loopexit, %for.body
  %coords0.1.lcssa = phi <5 x i32> [ %3, %for.body ], [ %vecins.3.lcssa, %for.cond.cleanup9.loopexit ]
  %add13 = add i32 %h.063, 1
  %exitcond64 = icmp eq i32 %add13, %vecext5
  br i1 %exitcond64, label %for.cond.cleanup.loopexit, label %for.body

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %w.060 = phi i32 [ %add11.3, %for.body10 ], [ %mul, %for.body10.preheader ]
  %coords0.159 = phi <5 x i32> [ %vecins.3, %for.body10 ], [ %3, %for.body10.preheader ]
  %vecins = insertelement <5 x i32> %coords0.159, i32 %w.060, i32 1
  %7 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %8 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %9 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %7)
  %10 = fmul <64 x float> %6, %7
  %11 = fmul <64 x float> %10, %9
  %12 = fsub <64 x float> %4, %11
  %13 = fmul <64 x float> %8, %12
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins, i8 3, <64 x float> %13, i32 0, i1 true, i1 false)
  %add11 = or i32 %w.060, 1
  %vecins.1 = insertelement <5 x i32> %vecins, i32 %add11, i32 1
  %14 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.1, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %15 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.1, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %16 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %14)
  %17 = fmul <64 x float> %6, %14
  %18 = fmul <64 x float> %17, %16
  %19 = fsub <64 x float> %4, %18
  %20 = fmul <64 x float> %15, %19
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins.1, i8 3, <64 x float> %20, i32 0, i1 true, i1 false)
  %add11.1 = or i32 %w.060, 2
  %vecins.2 = insertelement <5 x i32> %vecins.1, i32 %add11.1, i32 1
  %21 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.2, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %22 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.2, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %23 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %21)
  %24 = fmul <64 x float> %6, %21
  %25 = fmul <64 x float> %24, %23
  %26 = fsub <64 x float> %4, %25
  %27 = fmul <64 x float> %22, %26
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins.2, i8 3, <64 x float> %27, i32 0, i1 true, i1 false)
  %add11.2 = or i32 %w.060, 3
  %vecins.3 = insertelement <5 x i32> %vecins.2, i32 %add11.2, i32 1
  %28 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.3, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %29 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %vecins.3, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %30 = tail call <64 x float> @llvm.fabs.v64f32(<64 x float> %28)
  %31 = fmul <64 x float> %6, %28
  %32 = fmul <64 x float> %31, %30
  %33 = fsub <64 x float> %4, %32
  %34 = fmul <64 x float> %29, %33
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %vecins.3, i8 3, <64 x float> %34, i32 0, i1 true, i1 false)
  %add11.3 = add i32 %w.060, 4
  %exitcond.3 = icmp eq i32 %add11.3, %mul2
  br i1 %exitcond.3, label %for.cond.cleanup9.loopexit, label %for.body10, !llvm.loop !3
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.set.indx(<5 x i32>, i32, i32, i32, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare <64 x float> @llvm.fabs.v64f32(<64 x float>) #3

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 4111c7f125204e995146f7741cc2c3989cfb74d0)"}
!2 = !{i32 0}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.unroll.disable"}
