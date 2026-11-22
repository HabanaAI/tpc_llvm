# test_kernels

Gaudi2 TPC kernels from three open inference projects, collected as a test corpus for the compiler: does a
kernel build, does it give the same output as Habana's `tpc-clang`, and how fast is it. The files are copied
as they are in each project. Each project keeps its own directory layout below its name, so relative
`#include`s still resolve.

| Directory | Project | Commit | Files | License |
|---|---|---|---|---|
| `wright/kernels` | [Wright](https://github.com/abotsis/wright) 0.3.0, `models/gaudi/resin_models_gaudi/kernels` | `953a08bd` (2026-11-16) | 256 `.c`, 5 `.h` | AGPL-3.0-only (`wright/LICENSE`); `kv_gather8.c` and `kv_store8.c` are PolyForm-Noncommercial-1.0.0 (see their SPDX headers) |
| `1cat-vllm-gaudi/<program>` | [1cat-vllm-gaudi](https://github.com/1CatAI/1cat-vllm-gaudi), `csrc/<program>` | `e2266e0e` (2026-10-12) | 470 `.c`, 56 `.h` | Apache-2.0 (`1cat-vllm-gaudi/LICENSE`) |
| `GaudiInfer/flashinfer_gaudi/kernels` | [GaudiInfer](https://github.com/JChen-318/GaudiInfer), `tpc/flashinfer_gaudi/kernels` | `cb260a91` (2026-11-21) | 53 `.c`, 1 `.h` | Apache-2.0 (SPDX header in every file; text in `GaudiInfer/LICENSE-Apache-2.0.txt`) |

The 1cat programs are `deepseek_v4` (`kernels`, `include`, `memory_ready`), `deepseek_v41_unique` (`kernels`,
`include`), `flashinfer_gaudi` and `flashqla_hpu`.

## Building

Every file was compiled with Habana's `tpc-clang` 2.7.33 (tpc_llvm10 c6df8aef53a5):

```sh
tpc-clang -x c++ -march=gaudi2 -O2 -c FILE.c -o FILE.o
```

| Project | Builds with only those flags | Notes |
|---|---|---|
| Wright | 254 of 256 | `kernels/lq/syn_qdq.c` and `syn_qmm.c` are fragments that other kernels `#include`. Wright compiles each top-level `kernels/*.c` file as one kernel, with exactly these flags. |
| 1cat-vllm-gaudi | 248 of 470 | The other 222 also fail in 1cat's own tree with these flags. They need the `-D` variants and `-I` paths that 1cat's `tools/build_*.py` scripts pass, or they are fragments included by other kernels. |
| GaudiInfer | 53 of 53 | |

Each copy gives the same result as the file in its own project.
