// RUN: %clang_cc1 -S -emit-llvm -std=rc99 -triple tpc -tpc-library %s -o - | FileCheck %s

int qqq(int x) {
  return x + 22;
}
// CHECK: define dso_local i32 @qqq(i32 %x) {{.*}}#0 {
