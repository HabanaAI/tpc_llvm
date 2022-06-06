// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 -tpc-dnorm %s -o - | FileCheck %s

void main(unsigned a, unsigned b, unsigned dest) {
  // MUL
  void __local *ptr = (void __local *)dest;
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    bfloat __local *dest_ptr = (bfloat __local *)ptr;
    bfloat __local *a_ptr = (bfloat __local *)a;
    bfloat __local *b_ptr = (bfloat __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.bf16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    half __local *dest_ptr = (half __local *)ptr;
    half __local *a_ptr = (half __local *)a;
    half __local *b_ptr = (half __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minifloat __local *dest_ptr = (minifloat __local *)ptr;
    minifloat __local *a_ptr = (minifloat __local *)a;
    minifloat __local *b_ptr = (minifloat __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f8_143 acc_fp32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)ptr;
    minihalf __local *a_ptr = (minihalf __local *)a;
    minihalf __local *b_ptr = (minihalf __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f8_152 acc_fp32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  
  // ADD
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    bfloat __local *dest_ptr = (bfloat __local *)ptr;
    bfloat __local *a_ptr = (bfloat __local *)a;
    bfloat __local *b_ptr = (bfloat __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.bf16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    half __local *dest_ptr = (half __local *)ptr;
    half __local *a_ptr = (half __local *)a;
    half __local *b_ptr = (half __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minifloat __local *dest_ptr = (minifloat __local *)ptr;
    minifloat __local *a_ptr = (minifloat __local *)a;
    minifloat __local *b_ptr = (minifloat __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f8_143 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)ptr;
    minihalf __local *a_ptr = (minihalf __local *)a;
    minihalf __local *b_ptr = (minihalf __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f8_152 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  
  // SUB
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    bfloat __local *dest_ptr = (bfloat __local *)ptr;
    bfloat __local *a_ptr = (bfloat __local *)a;
    bfloat __local *b_ptr = (bfloat __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.bf16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    half __local *dest_ptr = (half __local *)ptr;
    half __local *a_ptr = (half __local *)a;
    half __local *b_ptr = (half __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f16 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minifloat __local *dest_ptr = (minifloat __local *)ptr;
    minifloat __local *a_ptr = (minifloat __local *)a;
    minifloat __local *b_ptr = (minifloat __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f8_143 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)ptr;
    minihalf __local *a_ptr = (minihalf __local *)a;
    minihalf __local *b_ptr = (minihalf __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f8_152 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  
  // Intrinsics
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = s_f32_mul(*a_ptr, *b_ptr);
    // CHECK: mul.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = s_f32_add(*a_ptr, *b_ptr);
    // CHECK: add.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = s_f32_sub(*a_ptr, *b_ptr);
    // CHECK: sub.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
  {
    float __local *dest_ptr = (float __local *)ptr;
    float __local *a_ptr = (float __local *)a;
    float __local *b_ptr = (float __local *)b;
    *dest_ptr++ = s_f32_mac(*a_ptr, *b_ptr, *dest_ptr);
    // CHECK: mac.f32 dnorm %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
  }
}
