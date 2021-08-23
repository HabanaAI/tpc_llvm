// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

void main(int x2) {
    aso_b(1, 1, x2, 0);
    aso_b(0, 1, x2, 1);
}
// CHECK: aso dec vpu %SP{{[0-9]+}}
// CHECK: aso vpu !%SP{{[0-9]+}}
