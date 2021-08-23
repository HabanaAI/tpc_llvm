// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -ast-dump %s | FileCheck %s
int func_01(int x) {
  return x + 2;
}
// CHECK-LABEL: FunctionDecl {{.*}} func_01 'int (int)'
// CHECK: AlwaysInlineAttr {{.*}} Implicit always_inline

void main() {
  int y = func_01(12);
}
// CHECK-LABEL: FunctionDecl {{.*}} main 'void ()'
// CHECK-NOT: AlwaysInlineAttr
