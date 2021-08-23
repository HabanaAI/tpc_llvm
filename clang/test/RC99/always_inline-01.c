// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -main-function entry -ast-dump %s | FileCheck %s
int main(int x) {
  return x + 2;
}
// CHECK-LABEL: FunctionDecl {{.*}} main 'int (int)'
// CHECK: AlwaysInlineAttr {{.*}} Implicit always_inline

void entry() {
  int y = main(12);
}
// CHECK-LABEL: FunctionDecl {{.*}} entry 'void ()'
// CHECK-NOT: AlwaysInlineAttr
