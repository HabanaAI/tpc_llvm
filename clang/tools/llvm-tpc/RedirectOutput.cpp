//===- RedirectOutput.cpp - Override standard output streams --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "API.h"
#include "MemoryManager.h"
#include "clang/Basic/Version.h"
#include "llvm/Support/raw_ostream.h"
#include <atomic>

namespace llvm {

raw_ostream &outs();
raw_ostream &errs();
raw_ostream &dbgs();

raw_ostream &outs_std();
raw_ostream &errs_std();
raw_ostream &dbgs_std();

} // end namespace llvm

//
// Redirect the standard output stream functions to an internal name, so we may
// override them later (below).
//

#define outs outs_std
#define errs errs_std
#include "lib/Support/raw_ostream.cpp"
#undef outs
#undef errs

#define dbgs dbgs_std
#include "lib/Support/Debug.cpp"
#undef dbgs

namespace {

/// A redirect output stream.
class RedirectStream : public llvm::raw_ostream {
  /// See raw_ostream::write_impl.
  void write_impl(const char *ptr, size_t size) override {
    (*write)(ptr, size);
  }

  /// See raw_ostream::current_pos.
  uint64_t current_pos() const override { return 0; }

public:
  RedirectStream(WritePfn pfn) : write(pfn) { SetUnbuffered(); }

  std::atomic<WritePfn> write;
};

} // end anonymous namespace

static void writeStdOut(const char *ptr, unsigned size) {
  outs_std().write(ptr, size);
}

static void writeStdErr(const char *ptr, unsigned size) {
  errs_std().write(ptr, size);
}

static void writeStdDbg(const char *ptr, unsigned size) {
  dbgs_std().write(ptr, size);
}

static RedirectStream redirectOuts(writeStdOut);
static RedirectStream redirectErrs(writeStdErr);
static RedirectStream redirectDbgs(writeStdDbg);

/// Override the standard raw output streams using the redirects.
raw_ostream &llvm::outs() { return redirectOuts; }
raw_ostream &llvm::errs() { return redirectErrs; }
raw_ostream &llvm::dbgs() { return redirectDbgs; }

/// Set the write functions for redirecting the `out`, `err` and `dbg` streams.
void llvm_tpc_redirectOutput(WritePfn out, WritePfn err, WritePfn dbg) {
  redirectOuts.write = out ? out : writeStdOut;
  redirectErrs.write = err ? err : writeStdErr;
  redirectDbgs.write = dbg ? dbg : writeStdDbg;
}
