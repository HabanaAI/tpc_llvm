// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

void main(_Bool x) {
  aso(0, x, 0);
  aso(SW_DEC, x, 0);
  aso(SW_VPU, x, 0);
  aso(SW_DEC | SW_VPU, x, 0);
  aso(0, 1, 0);
  aso(SW_DEC, x, 1);
  aso(SW_VPU, x, 1);
  aso(SW_DEC | SW_VPU, x, 1);
}

// CHECK: aso %SP{{[0-9]+}}
// CHECK: aso dec %SP{{[0-9]+}}
// CHECK: aso vpu %SP{{[0-9]+}}
// CHECK: aso dec vpu %SP{{[0-9]+}}
// CHECK: aso %SP0
// CHECK: aso dec !%SP{{[0-9]+}}
// CHECK: aso vpu !%SP{{[0-9]+}}
// CHECK: aso dec vpu !%SP{{[0-9]+}}
