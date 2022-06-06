// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -mllvm -tpc-nearbyint-workaround=0  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -mllvm -tpc-nearbyint-workaround=0   %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2  -mllvm -tpc-nearbyint-workaround=0  %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -mllvm -tpc-nearbyint-workaround=0   %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3,GEN4 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  -mllvm -tpc-nearbyint-workaround=0  %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3,GEN4 %s

void main(int dest, int x, int vpredp, _Bool pred) {
  volatile int __local *dptr = (int __local *)dest;

  // s_f32_nearbyint   /Intrinsics/s_xx_nearbyint.c
  {
    volatile float __local *fptr = (float __local *)dptr;
    float res = *fptr++;
    float xf = as_float(x);
    // CHECK: ld_l [[DEST:%S[0-9]+]], %S0

    res = s_f32_nearbyint(xf, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // CHECK: nearbyint.f32 rhne [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f32_nearbyint(xf, SW_RD, res, pred, 0);
    *fptr++ = res;
    // CHECK: nearbyint.f32 rd [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f32_nearbyint(xf, SW_RU, res, pred, 0);
    *fptr++ = res;
    // CHECK: nearbyint.f32 ru [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f32_nearbyint(xf, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // CHECK: nearbyint.f32 rz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }

#if defined(__gaudi2__) || defined(__doron1__)
  {
    volatile float __local *fptr = (float __local *)dptr;
    float res = *fptr++;
    float xf = as_float(x);
    // GEN4: ld_l [[DEST:%S[0-9]+]], %S{{[0-9]+}}

    res = s_f32_nearbyint(xf, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f32 rhaz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }
#endif

#if defined (__gaudi__) || defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // s_bf16_nearbyint
  {
    volatile _BFloat16 __local *fptr = (_BFloat16 __local *)dptr;
    _BFloat16 res = *fptr++;
    short sx = x;
    _BFloat16 xf = as_bfloat(sx);
    // GEN2: ld_l [[DEST:%S[0-9]+]], %S{{[0-9]+}}

    res = s_bf16_nearbyint(xf, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rhne [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_bf16_nearbyint(xf, SW_RD, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rd [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_bf16_nearbyint(xf, SW_RU, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 ru [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_bf16_nearbyint(xf, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // GEN2: nearbyint.bf16 rz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }
#endif  

#if defined(__gaudi2__) || defined(__doron1__)
  {
    volatile _BFloat16 __local *fptr = (_BFloat16 __local *)dptr;
    _BFloat16 res = *fptr++;
    short sx = x;
    _BFloat16 xf = as_bfloat(sx);
    // GEN4: ld_l [[DEST:%S[0-9]+]], %S{{[0-9]+}}

    res = s_bf16_nearbyint(xf, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.bf16 rhaz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }
#endif

#if defined(__goya2__) || defined(__gaudi2__) || defined(__doron1__)
  // s_f16_nearbyint
  {
    volatile half __local *fptr = (half __local *)dptr;
    half res = *fptr++;
    short sx = x;
    half xf = as_half(sx);
    // GEN3: ld_l [[DEST:%S[0-9]+]], %S{{[0-9]+}}

    res = s_f16_nearbyint(xf, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 rhne [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f16_nearbyint(xf, SW_RD, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 rd [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f16_nearbyint(xf, SW_RU, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 ru [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    res = s_f16_nearbyint(xf, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // GEN3: nearbyint.f16 rz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }
#endif

#if defined(__gaudi2__) || defined(__doron1__)
  {
    volatile half __local *fptr = (half __local *)dptr;
    half res = *fptr++;
    short sx = x;
    half xf = as_half(sx);
    // GEN4: ld_l [[DEST:%S[0-9]+]], %S{{[0-9]+}}

    res = s_f16_nearbyint(xf, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // GEN4: nearbyint.f16 rhaz [[DEST]], %S1, [[PRED:%SP[0-9]+]]

    dptr = (int __local *)fptr;
  }
#endif
}
