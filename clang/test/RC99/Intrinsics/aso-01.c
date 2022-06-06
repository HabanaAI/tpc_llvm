// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(_Bool x) {
  aso(0, x, 0);
  aso(SW_VPU, x, 0);
  aso(0, 1, 0);
  aso(SW_VPU, x, 1);
}

// CHECK: aso %SP{{[0-9]+}}
// CHECK: aso vpu %SP{{[0-9]+}}
// CHECK: aso
// CHECK: aso vpu !%SP{{[0-9]+}}
