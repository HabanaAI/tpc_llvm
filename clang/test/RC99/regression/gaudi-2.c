// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o -

int Global_Var = 777;

void main() {
  Global_Var = 1;
}