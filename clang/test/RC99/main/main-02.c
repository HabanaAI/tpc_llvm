// RUN: not %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none %s 2>&1 | FileCheck %s

void main1() 
{
  int a = 7;
  int x = 0;
}
// CHECK: error: entry function must be declared in translation unit
