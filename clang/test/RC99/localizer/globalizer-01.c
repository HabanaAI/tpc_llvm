// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O0 %s -o %t
// RUN: opt -S < %t -o %t.out 

struct ABC {
  int ggg[12];
};

void main(int x) {
  struct ABC vvv;
  vvv.ggg[1] = 11;
  vvv.ggg[0] = 777;
  *(struct ABC __local *)x = vvv;
}

// CHECK: @0 = private global
// CHECK: define void @main(i32 %x)
// CHECK-NEXT: entry:
// CHECK-NEXT: = bitcast {{.*}}* @0