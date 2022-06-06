// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s  -target-cpu gaudi2 -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s  -target-cpu doron1 -o - | FileCheck %s

void main(int x0, int x1, int x2, int dest0, int vpredp)
{

  // v_bf16_fclass_limit_b
  {
    minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;
    minifloat256 __local *ptr_x1 = (minifloat256 __local *)x1;
    minifloat256 __local *ptr_x2 = (minifloat256 __local *)x2;

    minifloat256 __local *ptr_dest = (minifloat256  __local *)dest0;
    minifloat256 res = 0;

    res = v_f8_fclass_limit_b(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK:fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
    
    res = v_f8_fclass_limit_b(1.0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f8_fclass_limit_b(*ptr_x0, 1.0, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}

    res = v_f8_fclass_limit_b(*ptr_x0, *ptr_x1, 1.0, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
  }

  // v_f8_fclass_limit_vb
  {
    minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;
    minifloat256 __local *ptr_x1 = (minifloat256 __local *)x1;
    minifloat256 __local *ptr_x2 = (minifloat256 __local *)x2;

    bool256 __local *ptr_vpredp = (bool256 __local *)vpredp;

    minifloat256 __local *ptr_dest = (minifloat256  __local *)dest0;
    minifloat256 res = 0;

    res = v_f8_fclass_limit_vb(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_fclass_limit_vb(1.0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_fclass_limit_vb(*ptr_x0, 1.0, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_fclass_limit_vb(*ptr_x0, *ptr_x1, 1.0, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_143 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }

  // v_h8_fclass_limit_b
  {
    minihalf256 __local *ptr_x0 = (minihalf256 __local *)x0;
    minihalf256 __local *ptr_x1 = (minihalf256 __local *)x1;
    minihalf256 __local *ptr_x2 = (minihalf256 __local *)x2;

    minihalf256 __local *ptr_dest = (minihalf256  __local *)dest0;
    minihalf256 res = 0;

    res = v_h8_fclass_limit_b(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK:fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_h8_fclass_limit_b(1.0, *ptr_x1, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

    res = v_h8_fclass_limit_b(*ptr_x0, 1.0, *ptr_x2, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}

    res = v_h8_fclass_limit_b(*ptr_x0, *ptr_x1, 1.0, 1, res, 1, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
  }

  // v_h8_fclass_limit_vb
  {
    minihalf256 __local *ptr_x0 = (minihalf256 __local *)x0;
    minihalf256 __local *ptr_x1 = (minihalf256 __local *)x1;
    minihalf256 __local *ptr_x2 = (minihalf256 __local *)x2;

    bool256 __local *ptr_vpredp = (bool256 __local *)vpredp;

    minihalf256 __local *ptr_dest = (minihalf256  __local *)dest0;
    minihalf256 res = 0;

    res = v_h8_fclass_limit_vb(*ptr_x0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_fclass_limit_vb(1.0, *ptr_x1, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_fclass_limit_vb(*ptr_x0, 1.0, *ptr_x2, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_fclass_limit_vb(*ptr_x0, *ptr_x1, 1.0, 1, res, *ptr_vpredp, 0);
    *ptr_dest++ = res;
    // CHECK: fclass.f8_152 limit %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }

}
