// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor in, int dimmask, int desta, _Bool pred) {
  int5 indx = { 0, 0, 0, 0, 0 };
  void __global *addr = gen_addr(indx, in, 0, 0, 1, 0);

  int5 __local *dest_ptr = (int5 __local *)desta;

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, dimmask, 0, res, pred, 0);
    // CHECK: ld_g inc_4 %M{{[0-9]+}} %I{{[0-9]+}},  %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, 0b00011, 0, res, pred, 0);
    // CHECK: ld_g inc_4 b00011 %I{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, 0b00011, SW_L0CS, res, pred, 0);
    // CHECK: ld_g inc_4 l0cs b00011 %I{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, 0b00011, SW_EV_HINT, res, pred, 0);
    // CHECK: ld_g inc_4 ev_hint b00011 %I{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, 0b00011, SW_BV64, res, pred, 0);
    // CHECK: ld_g inc_4 bv64 b00011 %I{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }

  {
    int5 res = {0};
    res = i_i32_ld_g_inc(&addr, 0b00011, SW_L0CS | SW_EV_HINT | SW_BV64, res, pred, 0);
    // CHECK: ld_g inc_4 bv64 l0cs ev_hint b00011 %I{{[0-9]+}}, %AD{{[0-9]+}}, %SP{{[0-9]+}}
    *dest_ptr++ = res;
  }
}
