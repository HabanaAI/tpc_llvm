// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  %s -o - | FileCheck --check-prefixes=CHECK,GEN1 %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN3 %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GEN3,GEN4 %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK,GEN3,GEN4 %s

void main(int dest, int x, int vpredp, _Bool pred) {
  volatile int64 __local *vptr = (int64 __local *)dest;
  volatile bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred_val = *vpred_ptr;
  *(bool256 __local *)vptr++ = vpred_val;
  // CHECK: ld_l_v  [[VPRED:%VP[0-9]+]]

  // v_f32_nearbyint_b
  {
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64 res = *fptr++;
    float64 x = *fptr++;
    // CHECK-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // CHECK-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f32_nearbyint_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN2: nearbyint.f32 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.f32 target_type=int32 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f32_nearbyint_b(x, SW_RD, res, pred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN2: nearbyint.f32 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.f32 target_type=int32 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f32_nearbyint_b(x, SW_RU, res, pred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN2: nearbyint.f32 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.f32 target_type=int32 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f32_nearbyint_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.f32 target_type=int32 rz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    vptr = (int64 __local *)fptr;
  }

  // v_f32_nearbyint_vb
  {
    bool64  vpred = to_bool64(vpred_val);
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64 res = *fptr++;
    float64 x = *fptr++;
    // CHECK-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // CHECK-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f32_nearbyint_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rhne [[DEST]], [[VAL]], [[VPRED]]
    // GEN2: nearbyint.f32 rhne [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.f32 target_type=int32 rhne [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rd [[DEST]], [[VAL]], [[VPRED]]
    // GEN2: nearbyint.f32 rd [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.f32 target_type=int32 rd [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 ru [[DEST]], [[VAL]], [[VPRED]]
    // GEN2: nearbyint.f32 ru [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.f32 target_type=int32 ru [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res;
    // GEN1: nearbyint.f32 rz [[DEST]], [[VAL]], [[VPRED]]
    // GEN2: nearbyint.f32 rz [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.f32 target_type=int32 rz [[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }
  
#if defined(__gaudi2__) || defined(__doron1__)
  {
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64 res = *fptr++;
    float64 x = *fptr++;

    res = v_f32_nearbyint_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f32 target_type=int32 rhaz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    vptr = (int64 __local *)fptr;
  }

  {
    bool64  vpred = to_bool64(vpred_val);
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64 res = *fptr++;
    float64 x = *fptr++;

    res = v_f32_nearbyint_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f32 target_type=int32 rhaz [[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }
#endif

#if defined(__gaudi__) || defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // v_bf16_nearbyint_b
  {
    volatile bfloat128 __local *fptr = (bfloat128 __local *)vptr;
    bfloat128 res = *fptr++;
    bfloat128 x = *fptr++;
    // GEN2-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // GEN2-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_bf16_nearbyint_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.bf16 target_type=int16 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_bf16_nearbyint_b(x, SW_RD, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.bf16 target_type=int16 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_bf16_nearbyint_b(x, SW_RU, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.bf16 target_type=int16 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_bf16_nearbyint_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    // GEN3: nearbyint.bf16 target_type=int16 rz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    res = v_bf16_nearbyint_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.bf16 target_type=int16 rhaz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    vptr = (int64 __local *)fptr;
  }

  // v_bf16_nearbyint_vb
  {
    bool128  vpred = to_bool128(vpred_val);
    volatile bfloat128 __local *fptr = (bfloat128 __local *)vptr;
    bfloat128 res = *fptr++;
    bfloat128 x = *fptr++;
    // GEN2-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // GEN2-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_bf16_nearbyint_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rhne [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.bf16 target_type=int16 rhne [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rd [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.bf16 target_type=int16 rd [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 ru [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.bf16 target_type=int16 ru [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rz [[DEST]], [[VAL]], [[VPRED]]
    // GEN3: nearbyint.bf16 target_type=int16 rz [[DEST]], [[VAL]], [[VPRED]]

    res = v_bf16_nearbyint_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.bf16 target_type=int16 rhaz [[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }
#endif  
  
#if defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // v_f16_nearbyint_b
  {
    volatile half128 __local *fptr = (half128 __local *)vptr;
    half128 res = *fptr++;
    half128 x = *fptr++;
    // GEN3-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // GEN3-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f16_nearbyint_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rhne [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f16_nearbyint_b(x, SW_RD, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rd [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f16_nearbyint_b(x, SW_RU, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 ru [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]
    
    res = v_f16_nearbyint_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    res = v_f16_nearbyint_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f16 target_type=int16 rhaz [[DEST]], [[VAL]], [[PRED:%SP[0-9]+]]

    vptr = (int64 __local *)fptr;
  }

  // v_f16_nearbyint_vb
  {
    bool128  vpred = to_bool128(vpred_val);
    volatile half128 __local *fptr = (half128 __local *)vptr;
    half128 res = *fptr++;
    half128 x = *fptr++;
    // GEN3-DAG: ld_l_v  [[DEST:%V[0-9]+]]
    // GEN3-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f16_nearbyint_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rhne [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rd [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 ru [[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 target_type=int16 rz [[DEST]], [[VAL]], [[VPRED]]

    res = v_f16_nearbyint_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f16 target_type=int16 rhaz [[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_f32_nearbyint_cnvrt_b
  {
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64_int64_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    float64 x = *fptr++;
    
    res = v_f32_nearbyint_cnvrt_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rhne [[DEST:%D[0-9]+]], [[VAL:%V[0-9]+]], [[PRED:%SP[0-9]+]]

    
    res = v_f32_nearbyint_cnvrt_b(x, SW_RD, res, pred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rd [[DEST]], [[VAL]], [[PRED]]
    
    res = v_f32_nearbyint_cnvrt_b(x, SW_RU, res, pred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 ru [[DEST]], [[VAL]], [[PRED]]
    
    res = v_f32_nearbyint_cnvrt_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rz [[DEST]], [[VAL]], [[PRED]]

    res = v_f32_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.f32 cnvrt target_type=int32 rhaz [[DEST]], [[VAL]], [[PRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_f32_nearbyint_cnvrt_vb
  {
    bool64  vpred = to_bool64(vpred_val);
    volatile float64 __local *fptr = (float64 __local *)vptr;
    float64_int64_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    float64 x = *fptr++;
    // GEN3-DAG: ld_l_v  %V[[DEST:[0-9]+]]
    // GEN3-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f32_nearbyint_cnvrt_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rhne %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_cnvrt_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rd %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_cnvrt_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 ru %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f32_nearbyint_cnvrt_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f32 cnvrt target_type=int32 rz %D[[DEST]], [[VAL]], [[VPRED]]

    res = v_f32_nearbyint_cnvrt_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(int64 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.f32 cnvrt target_type=int32 rhaz %D[[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_bf16_nearbyint_cnvrt_b
  {
    volatile bfloat128 __local *fptr = (bfloat128 __local *)vptr;
    bfloat128_short128_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    bfloat128 x = *fptr++;
    
    res = v_bf16_nearbyint_cnvrt_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rhne [[DEST:%D[0-9]+]], [[VAL:%V[0-9]+]], [[PRED:%SP[0-9]+]]

    
    res = v_bf16_nearbyint_cnvrt_b(x, SW_RD, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rd [[DEST]], [[VAL]], [[PRED]]
    
    res = v_bf16_nearbyint_cnvrt_b(x, SW_RU, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 ru [[DEST]], [[VAL]], [[PRED]]
    
    res = v_bf16_nearbyint_cnvrt_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rz [[DEST]], [[VAL]], [[PRED]]

    res = v_bf16_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.bf16 cnvrt target_type=int16 rhaz [[DEST]], [[VAL]], [[PRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_bf16_nearbyint_cnvrt_vb
  {
    bool128  vpred = to_bool128(vpred_val);
    volatile bfloat128 __local *fptr = (bfloat128 __local *)vptr;
    bfloat128_short128_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    bfloat128 x = *fptr++;
    // GEN3-DAG: ld_l_v  %V[[DEST:[0-9]+]]
    // GEN3-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_bf16_nearbyint_cnvrt_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rhne %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_cnvrt_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rd %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_cnvrt_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 ru %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_bf16_nearbyint_cnvrt_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.bf16 cnvrt target_type=int16 rz %D[[DEST]], [[VAL]], [[VPRED]]

    res = v_bf16_nearbyint_cnvrt_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.bf16 cnvrt target_type=int16 rhaz %D[[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_f16_nearbyint_cnvrt_b
  {
    volatile half128 __local *fptr = (half128 __local *)vptr;
    half128_short128_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    half128 x = *fptr++;
    
    res = v_f16_nearbyint_cnvrt_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rhne [[DEST:%D[0-9]+]], [[VAL:%V[0-9]+]], [[PRED:%SP[0-9]+]]

    
    res = v_f16_nearbyint_cnvrt_b(x, SW_RD, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rd [[DEST]], [[VAL]], [[PRED]]
    
    res = v_f16_nearbyint_cnvrt_b(x, SW_RU, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 ru [[DEST]], [[VAL]], [[PRED]]
    
    res = v_f16_nearbyint_cnvrt_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rz [[DEST]], [[VAL]], [[PRED]]

    res = v_f16_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.f16 cnvrt target_type=int16 rhaz [[DEST]], [[VAL]], [[PRED]]

    vptr = (int64 __local *)fptr;
  }

  // v_f16_nearbyint_cnvrt_vb
  {
    bool128  vpred = to_bool128(vpred_val);
    volatile half128 __local *fptr = (half128 __local *)vptr;
    half128_short128_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    half128 x = *fptr++;
    // GEN3-DAG: ld_l_v  %V[[DEST:[0-9]+]]
    // GEN3-DAG: ld_l_v  [[VAL:%V[0-9]+]]
    
    res = v_f16_nearbyint_cnvrt_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rhne %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_cnvrt_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rd %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_cnvrt_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 ru %D[[DEST]], [[VAL]], [[VPRED]]
    
    res = v_f16_nearbyint_cnvrt_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN3: nearbyint.f16 cnvrt target_type=int16 rz %D[[DEST]], [[VAL]], [[VPRED]]

    res = v_f16_nearbyint_cnvrt_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(short128 __local *)fptr++ = res.v2;
    // GEN4: nearbyint.f16 cnvrt target_type=int16 rhaz %D[[DEST]], [[VAL]], [[VPRED]]

    vptr = (int64 __local *)fptr;
  }

#endif  
}
