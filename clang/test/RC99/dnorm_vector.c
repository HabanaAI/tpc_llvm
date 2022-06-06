// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 -tpc-dnorm %s -o - | FileCheck %s

void main(unsigned a, unsigned b, unsigned dest) {
  int64 __local *ptr = (int64 __local *)dest;
  
  // MUL
  // Vector, standart node
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)ptr;
    
    bfloat128 __local *a_ptr = (bfloat128 __local *)a;
    bfloat128 __local *b_ptr = (bfloat128 __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.bf16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    half128 __local *dest_ptr = (half128 __local *)ptr;
    
    half128 __local *a_ptr = (half128 __local *)a;
    half128 __local *b_ptr = (half128 __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK: mul.f16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  // TODO: FIX fmul lowering
  /*{
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest;
    
    minifloat256 __local *a_ptr = (minifloat256 __local *)a;
    minifloat256 __local *b_ptr = (minifloat256 __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK_NOT_WORK: mul.f8_143 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest;
    
    minihalf256 __local *a_ptr = (minihalf256 __local *)a;
    minihalf256 __local *b_ptr = (minihalf256 __local *)b;
    *dest_ptr++ = *a_ptr * *b_ptr;
    // CHECK_NOT_WORK: mul.f8_152 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }*/
  
  
  // ADD
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)ptr;
    
    bfloat128 __local *a_ptr = (bfloat128 __local *)a;
    bfloat128 __local *b_ptr = (bfloat128 __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.bf16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    half128 __local *dest_ptr = (half128 __local *)ptr;
    
    half128 __local *a_ptr = (half128 __local *)a;
    half128 __local *b_ptr = (half128 __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest;
    
    minifloat256 __local *a_ptr = (minifloat256 __local *)a;
    minifloat256 __local *b_ptr = (minifloat256 __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f8_143 dnorm %A{{[0-9]+}}, %A{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest;
    
    minihalf256 __local *a_ptr = (minihalf256 __local *)a;
    minihalf256 __local *b_ptr = (minihalf256 __local *)b;
    *dest_ptr++ = *a_ptr + *b_ptr;
    // CHECK: add.f8_152 dnorm %A{{[0-9]+}}, %A{{[0-9]+}}, %V{{[0-9]+}}
  }
  
  // SUB
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)ptr;
    
    bfloat128 __local *a_ptr = (bfloat128 __local *)a;
    bfloat128 __local *b_ptr = (bfloat128 __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.bf16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    half128 __local *dest_ptr = (half128 __local *)ptr;
    
    half128 __local *a_ptr = (half128 __local *)a;
    half128 __local *b_ptr = (half128 __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK: sub.f16 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest;
    
    minifloat256 __local *a_ptr = (minifloat256 __local *)a;
    minifloat256 __local *b_ptr = (minifloat256 __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK1: sub.f8_143 dnorm %A{{[0-9]+}}, %A{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest;
    
    minihalf256 __local *a_ptr = (minihalf256 __local *)a;
    minihalf256 __local *b_ptr = (minihalf256 __local *)b;
    *dest_ptr++ = *a_ptr - *b_ptr;
    // CHECK1: sub.f8_152 dnorm %A{{[0-9]+}}, %A{{[0-9]+}}, %V{{[0-9]+}}
  }
  
  // Intrinsics
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = v_f32_mul_b(*a_ptr, *b_ptr);
    // CHECK: mul.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = v_f32_add_b(*a_ptr, *b_ptr);
    // CHECK: add.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = v_f32_sub_b(*a_ptr, *b_ptr);
    // CHECK: sub.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = v_f32_mac_b(*a_ptr, *b_ptr, *dest_ptr);
    // CHECK: mac.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    float64 __local *dest_ptr = (float64 __local *)ptr;
    
    float64 __local *a_ptr = (float64 __local *)a;
    float64 __local *b_ptr = (float64 __local *)b;
    *dest_ptr++ = v_f32_madd_b(*a_ptr, *b_ptr, *dest_ptr);
    // CHECK: madd.f32 dnorm %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
}
