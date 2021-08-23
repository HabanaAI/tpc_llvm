// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -ast-dump %s -o - | FileCheck %s


int bbbb;
// CHECK: VarDecl {{.*}} bbbb '__attribute__((address_space(1))) int'

__local int cccc;
// CHECK: VarDecl {{.*}} cccc '__attribute__((address_space(1))) int'

__local float64 dddd;
// CHECK: VarDecl {{.*}} dddd '__attribute__((address_space(2))) float64'

void main() {
}