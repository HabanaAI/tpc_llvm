// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -ast-dump %s -o - | FileCheck %s

void main() {
  float res = 11.4; 
  // CHECK: FloatingLiteral {{.*}} 'float' 1.140000e+01
}

