// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=GEN1 %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=GEN1,GEN2 %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=GEN1,GEN2,GEN3 %s

void main(int dest, int src) {
  {
    volatile float __local *dptr = (float __local *)dest;
    volatile float __local *sptr = (float __local *)src;
    float value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// CHECK:	xor.f32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000

#if defined(__gaudi__) || defined(__goya2__)
  {
    volatile _BFloat16 __local *dptr = (_BFloat16 __local *)dest;
    volatile _BFloat16 __local *sptr = (_BFloat16 __local *)src;
    _BFloat16 value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// GEN2: 	xor.bf16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x8000
#endif

#if defined(__goya2__)
  {
    volatile half __local *dptr = (half __local *)dest;
    volatile half __local *sptr = (half __local *)src;
    half value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// GEN3: 	xor.f16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x8000
#endif

  {
    volatile float64 __local *dptr = (float64 __local *)dest;
    volatile float64 __local *sptr = (float64 __local *)src;
    float64 value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// GEN1: xor.f32  %V{{[0-9]+}}, %V{{[0-9]+}}, -0x80000000

#if defined(__gaudi__) || defined(__goya2__)
  {
    volatile bfloat128 __local *dptr = (bfloat128 __local *)dest;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src;
    bfloat128 value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// GEN2: 	xor.bf16  %V{{[0-9]+}}, %V{{[0-9]+}}, 0x8000
#endif

#if defined(__goya2__)
  {
    volatile half128 __local *dptr = (half128 __local *)dest;
    volatile half128 __local *sptr = (half128 __local *)src;
    half128 value = *sptr++;
    *dptr++ = -value;
    dest = (int)dptr;
    src = (int)sptr;
  }
// GEN3: 	xor.f16  %V{{[0-9]+}}, %V{{[0-9]+}}, 0x8000
#endif
}
