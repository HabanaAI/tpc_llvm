// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -main-function qqq -O1 %s -o - | FileCheck %s

void qqq(int I){
}
// CHECK: define dso_local void @qqq(i32 %I) {{.*}} {
// CHECK: ret void
