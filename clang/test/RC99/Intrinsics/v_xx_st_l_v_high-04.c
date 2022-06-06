// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr, int dest, int vpredp, _Bool pred) {
  volatile int64 __local *vptr = (int64 __local *)dest;

  {
    float64 __local *sptr = (float64 __local *)vptr;
    float64 value = *sptr++;

    bool64 __local *vpred_ptr = (bool64 __local *)vpredp;
    bool64 vpred = *vpred_ptr;

    v_f32_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_f32_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    bfloat128 __local *sptr = (bfloat128 __local *)vptr;
    bfloat128 value = *sptr++;

    bool128 __local *vpred_ptr = (bool128 __local *)vpredp;
    bool128 vpred = *vpred_ptr;

    v_bf16_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_bf16_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    half128 __local *sptr = (half128 __local *)vptr;
    half128 value = *sptr++;

    bool128 __local *vpred_ptr = (bool128 __local *)vpredp;
    bool128 vpred = *vpred_ptr;

    v_f16_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_f16_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    int64 __local *sptr = (int64 __local *)vptr;
    int64 value = *sptr++;

    bool64 __local *vpred_ptr = (bool64 __local *)vpredp;
    bool64 vpred = *vpred_ptr;

    v_i32_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_i32_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    uint64 __local *sptr = (uint64 __local *)vptr;
    uint64 value = *sptr++;

    bool64 __local *vpred_ptr = (bool64 __local *)vpredp;
    bool64 vpred = *vpred_ptr;

    v_u32_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_u32_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    short128 __local *sptr = (short128 __local *)vptr;
    short128 value = *sptr++;

    bool128 __local *vpred_ptr = (bool128 __local *)vpredp;
    bool128 vpred = *vpred_ptr;

    v_i16_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_i16_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    ushort128 __local *sptr = (ushort128 __local *)vptr;
    ushort128 value = *sptr++;

    bool128 __local *vpred_ptr = (bool128 __local *)vpredp;
    bool128 vpred = *vpred_ptr;

    v_u16_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_u16_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    char256 __local *sptr = (char256 __local *)vptr;
    char256 value = *sptr++;

    bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
    bool256 vpred = *vpred_ptr;

    v_i8_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_i8_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    uchar256 __local *sptr = (uchar256 __local *)vptr;
    uchar256 value = *sptr++;

    bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
    bool256 vpred = *vpred_ptr;

    v_u8_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_u8_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  
  {
    bool256 __local *sptr = (bool256 __local *)vptr;
    bool256 value = *sptr++;

    bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
    bool256 vpred = *vpred_ptr;

    v_i1_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %VP{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_i1_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %VP{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    minifloat256 __local *sptr = (minifloat256 __local *)vptr;
    minifloat256 value = *sptr++;

    bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
    bool256 vpred = *vpred_ptr;

    v_f8_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_f8_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    minihalf256 __local *sptr = (minihalf256 __local *)vptr;
    minihalf256 value = *sptr++;

    bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
    bool256 vpred = *vpred_ptr;

    v_h8_st_l_v_high_vb(addr, value, 0, vpred, 0);
    // CHECK1: st_l_v_high  %S0, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    v_h8_st_l_v_high_vb(0x100, value, 0, vpred, 0);
    // CHECK1: st_l_v_high  0x100, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

    {
    minifloat256 __local *sptr = (minifloat256 __local *)vptr;
    minifloat256 value = *sptr++;

    v_f8_st_l_v_high_b(addr, value, 0, pred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    v_f8_st_l_v_high_b(0x100, value, 0, pred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    minihalf256 __local *sptr = (minihalf256 __local *)vptr;
    minihalf256 value = *sptr++;

    v_h8_st_l_v_high_b(addr, value, 0, pred, 0);
    // CHECK: st_l_v_high  %S0, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    v_h8_st_l_v_high_b(0x100, value, 0, pred, 0);
    // CHECK: st_l_v_high  0x100, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
}
