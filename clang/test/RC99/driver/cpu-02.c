// RUN: %clang -target tpc -std=rc99 -c -march=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -march=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -march=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s
// RUN: %clang -target tpc -std=rc99 -c -march=gaudib %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDIB %s
// RUN: %clang -target tpc -std=rc99 -c -march=goya2 %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -march=greco %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -march=gaudi2 %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI2 %s

// RUN: %clang -target tpc -std=rc99 -c -mtune=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=gaudib %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDIB %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=goya2 %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=greco %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -mtune=gaudi2 %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI2 %s

// RUN: %clang -target tpc -std=rc99 -c -mcpu=dali %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=goya %s -o - -### 2>&1 | FileCheck --check-prefix=GOYA %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=gaudi %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=gaudib %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDIB %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=goya2 %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=greco %s -o - -### 2>&1 | FileCheck --check-prefix=GRECO %s
// RUN: %clang -target tpc -std=rc99 -c -mcpu=gaudi2 %s -o - -### 2>&1 | FileCheck --check-prefix=GAUDI2 %s

void main() {
}

// GOYA: "-target-cpu" "goya"
// GAUDI: "-target-cpu" "gaudi"
// GAUDIB: "-target-cpu" "gaudib"
// GRECO: "-target-cpu" "greco"
// GAUDI2: "-target-cpu" "gaudi2"
