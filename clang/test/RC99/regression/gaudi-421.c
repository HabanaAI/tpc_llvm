// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -

// SW-1166

int abc();

int abc() {
  return 2;
}

void main() {
}
