// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya             %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-ASM %s



void main(int x0, int x1, int x2, int dest0) {
  {
      float64 __local *ptr_x0 = (float64 __local *)x0;
      uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
      
      float64 __local *res0 = (float64  __local *)dest0;
      float64 temp_res0 = 0;
      temp_res0 = v_f32_shuffle_v_v(*ptr_x0, *ptr_x1);
      *res0 = temp_res0;
  //CHECK-ASM-DAG: shuffle.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    float64 __local *ptr_x0 = (float64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    float64 __local *res0 = (float64 __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    float64 __local *ptr_x0 = (float64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    float64 __local *res0 = (float64 __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
      short128 __local *ptr_x0 = (short128 __local *)x0;
      uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
      
      short128 __local *res0 = (short128  __local *)dest0;
      short128 temp_res0 = 0;
      temp_res0 = v_i16_shuffle_v_v(*ptr_x0, *ptr_x1);
      *res0 = temp_res0;
      //CHECK-ASM-DAG: shuffle.i16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    short128 __local *ptr_x0 = (short128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    short128 __local *res0 = (short128 __local *)dest0;
    short128 temp_res0 = 0;
    temp_res0 = v_i16_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    short128 __local *ptr_x0 = (short128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    short128 __local *res0 = (short128 __local *)dest0;
    short128 temp_res0 = 0;
    temp_res0 = v_i16_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
    int64 __local *ptr_x0 = (int64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    int64 __local *res0 = (int64 __local *)dest0;
    int64 temp_res0 = 0;
    temp_res0 = v_i32_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    int64 __local *ptr_x0 = (int64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    int64 __local *res0 = (int64 __local *)dest0;
    int64 temp_res0 = 0;
    temp_res0 = v_i32_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    int64 __local *ptr_x0 = (int64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    int64 __local *res0 = (int64 __local *)dest0;
    int64 temp_res0 = 0;
    temp_res0 = v_i32_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
    char256 __local *ptr_x0 = (char256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    char256 __local *res0 = (char256 __local *)dest0;
    char256 temp_res0 = 0;
    temp_res0 = v_i8_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    char256 __local *ptr_x0 = (char256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    char256 __local *res0 = (char256 __local *)dest0;
    char256 temp_res0 = 0;
    temp_res0 = v_i8_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    char256 __local *ptr_x0 = (char256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    char256 __local *res0 = (char256 __local *)dest0;
    char256 temp_res0 = 0;
    temp_res0 = v_i8_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.i8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
    ushort128 __local *ptr_x0 = (ushort128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    ushort128 __local *res0 = (ushort128 __local *)dest0;
    ushort128 temp_res0 = 0;
    temp_res0 = v_u16_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    ushort128 __local *ptr_x0 = (ushort128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    ushort128 __local *res0 = (ushort128 __local *)dest0;
    ushort128 temp_res0 = 0;
    temp_res0 = v_u16_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    ushort128 __local *ptr_x0 = (ushort128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    ushort128 __local *res0 = (ushort128 __local *)dest0;
    ushort128 temp_res0 = 0;
    temp_res0 = v_u16_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    uint64 __local *res0 = (uint64 __local *)dest0;
    uint64 temp_res0 = 0;
    temp_res0 = v_u32_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    uint64 __local *res0 = (uint64 __local *)dest0;
    uint64 temp_res0 = 0;
    temp_res0 = v_u32_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    uint64 __local *res0 = (uint64 __local *)dest0;
    uint64 temp_res0 = 0;
    temp_res0 = v_u32_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
////////
  {
    uchar256 __local *ptr_x0 = (uchar256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    uchar256 __local *res0 = (uchar256 __local *)dest0;
    uchar256 temp_res0 = 0;
    temp_res0 = v_u8_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
  {
    uchar256 __local *ptr_x0 = (uchar256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    uchar256 __local *res0 = (uchar256 __local *)dest0;
    uchar256 temp_res0 = 0;
    temp_res0 = v_u8_shuffle_v_v_b(*ptr_x0, *ptr_x1, temp_res0, x2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    unsigned a = 1;
    uchar256 __local *ptr_x0 = (uchar256 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;
    bool256 pred2;
    pred2 = bv_mov_b(1);

    uchar256 __local *res0 = (uchar256 __local *)dest0;
    uchar256 temp_res0 = 0;
    temp_res0 = v_u8_shuffle_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.u8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
}