// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -bfloat16 -target-cpu goya2 -o - | FileCheck %s
void main(int x0, int x1, int x2, int dest0, int vpredp)
{
  // v_f32_fclass_limit_b
  {
    float64 __local *ptr_x0 = (float64 __local *)x0;
    float64 __local *ptr_x1 = (float64 __local *)x1;
    float64 __local *ptr_x2 = (float64 __local *)x2;

    float64 __local *ptr_dest = (float64 __local *)dest0;
    float64 res = 0;

    res = v_f32_fclass_limit_b(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK:fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
    
    res = v_f32_fclass_limit_b(1.0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f32_fclass_limit_b(*ptr_x0, 1.0, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f32_fclass_limit_b(*ptr_x0, *ptr_x1, 1.0, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
  }

  // v_f32_fclass_limit_vb
  {
    float64 __local *ptr_x0 = (float64 __local *)x0;
    float64 __local *ptr_x1 = (float64 __local *)x1;
    float64 __local *ptr_x2 = (float64 __local *)x2;

    bool64 __local *ptr_vpredp = (bool64 __local *)vpredp;

    float64 __local *ptr_dest = (float64 __local *)dest0;
    float64 res = 0;

    res = v_f32_fclass_limit_vb(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f32_fclass_limit_vb(1.0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f32_fclass_limit_vb(*ptr_x0, 1.0, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f32_fclass_limit_vb(*ptr_x0, *ptr_x1, 1.0, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f32 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }

  // v_bf16_fclass_limit_b
  {
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    bfloat128 __local *ptr_x1 = (bfloat128 __local *)x1;
    bfloat128 __local *ptr_x2 = (bfloat128 __local *)x2;

    bfloat128 __local *ptr_dest = (bfloat128  __local *)dest0;
    bfloat128 res = 0;

    res = v_bf16_fclass_limit_b(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK:fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
    
    res = v_bf16_fclass_limit_b(1.0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_bf16_fclass_limit_b(*ptr_x0, 1.0, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}

    res = v_bf16_fclass_limit_b(*ptr_x0, *ptr_x1, 1.0, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
  }

  // v_bf16_fclass_limit_vb
  {
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    bfloat128 __local *ptr_x1 = (bfloat128 __local *)x1;
    bfloat128 __local *ptr_x2 = (bfloat128 __local *)x2;

    bool128 __local *ptr_vpredp = (bool128 __local *)vpredp;

    bfloat128 __local *ptr_dest = (bfloat128  __local *)dest0;
    bfloat128 res = 0;

    res = v_bf16_fclass_limit_vb(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_bf16_fclass_limit_vb(1.0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_bf16_fclass_limit_vb(*ptr_x0, 1.0, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_bf16_fclass_limit_vb(*ptr_x0, *ptr_x1, 1.0, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.bf16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }

  // v_f16_fclass_limit_b
  {
    half128 __local *ptr_x0 = (half128 __local *)x0;
    half128 __local *ptr_x1 = (half128 __local *)x1;
    half128 __local *ptr_x2 = (half128 __local *)x2;

    half128 __local *ptr_dest = (half128  __local *)dest0;
    half128 res = 0;

    res = v_f16_fclass_limit_b(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK:fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f16_fclass_limit_b(1.0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f16_fclass_limit_b(*ptr_x0, 1.0, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f16_fclass_limit_b(*ptr_x0, *ptr_x1, 1.0, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
  }

  // v_f16_fclass_limit_vb
  {
    half128 __local *ptr_x0 = (half128 __local *)x0;
    half128 __local *ptr_x1 = (half128 __local *)x1;
    half128 __local *ptr_x2 = (half128 __local *)x2;

    bool128 __local *ptr_vpredp = (bool128 __local *)vpredp;

    half128 __local *ptr_dest = (half128  __local *)dest0;
    half128 res = 0;

    res = v_f16_fclass_limit_vb(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f16_fclass_limit_vb(1.0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f16_fclass_limit_vb(*ptr_x0, 1.0, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f16_fclass_limit_vb(*ptr_x0, *ptr_x1, 1.0, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f16 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }

}
