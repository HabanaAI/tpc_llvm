//===- Compiler.cpp - TPC LLVM compiler entry point -----------------------===//
//
// Copyright (C) 2020 HabanaLabs, Ltd.
// All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited.
// Proprietary and confidential.
//
//===----------------------------------------------------------------------===//

#include "API.h"
#include "MemoryManager.h"
#include "lib/Target/TPC/latencies.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/CodeGen/BackendUtil.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/AutoUpgrade.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Target/TargetInfoTPC.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <csignal>

#define GET_SUBTARGETINFO_HEADER
#include "lib/Target/TPC/TPCGenSubtargetInfo.inc"

using namespace llvm;
using namespace llvm::tpc;
using namespace clang;

namespace {

template <unsigned Max> struct UnsignedLimitParser : cl::parser<unsigned> {
  using cl::parser<unsigned>::parser;

  bool parse(cl::Option &option, StringRef argName, StringRef arg,
             unsigned &val) {
    if (cl::parser<unsigned>::parse(option, argName, arg, val))
      return true;

    if (val > Max)
      return option.error("'" + arg + "' value larger than " + utostr(Max) +
                          "!");

    return false;
  }
};

/// A diagnostic handler that doesn't exit on the first error.
struct NoExitDiagnosticHandler : public DiagnosticHandler {
  bool &hasError;

  NoExitDiagnosticHandler(bool &hasError) : hasError(hasError) {}

  bool handleDiagnostics(const DiagnosticInfo &di) override;
};

} // end anonymous namespace

// Determine optimization level.
static cl::opt<unsigned, false, UnsignedLimitParser<3u>>
    optLevel("O", cl::desc("Optimization level. [0-3] (default = 2)"),
             cl::AlwaysPrefix, cl::ZeroOrMore, cl::init(2u));

/// Reset the static Automaton.
static void initAutomaton() {
  // The Automaton is a static variable inside `createDFAPacketizer()` function.
  InstrItineraryData iid;
  std::unique_ptr<DFAPacketizer> p(
      static_cast<TPCGenSubtargetInfo *>(nullptr)->createDFAPacketizer(&iid));

  // Reset the Automaton.
  p->clearResources();
}

static void handleLLVMFatalError(void *, const std::string &reason, bool) {
  WithColor::error(errs()) << reason << '\n';
  raise(SIGABRT);
}

bool NoExitDiagnosticHandler::handleDiagnostics(const DiagnosticInfo &di) {
  if (di.getSeverity() == DS_Error)
    hasError = true;

  if (auto *remark = dyn_cast<DiagnosticInfoOptimizationBase>(&di))
    if (!remark->isEnabled())
      return true;

  // Print the message with a prefix based on the severity.
  auto &os = errs();
  DiagnosticPrinterRawOStream printer(os);
  os << LLVMContext::getDiagnosticMessagePrefix(di.getSeverity()) << ": ";
  di.print(printer);
  os << '\n';
  return true;
}

static void InlineAsmDiagHandler(const SMDiagnostic &diag, void *context,
                                 unsigned locCookie) {
  auto *handler = static_cast<NoExitDiagnosticHandler *>(context);
  if (diag.getKind() == SourceMgr::DK_Error)
    handler->hasError = true;

  diag.print(nullptr, errs());
}

unsigned llvm_tpc_compileModule(const char *moduleBuf, unsigned moduleSize,
                                char **elfBin, char **asmIR, const char *cpu,
                                bool verify) {
  LLVMContext context;

  // Set a diagnostic handler that doesn't exit on the first error.
  bool hasError = false;
  context.setDiagnosticHandler(
      std::make_unique<NoExitDiagnosticHandler>(hasError));
  context.setInlineAsmDiagnosticHandler(
      InlineAsmDiagHandler,
      const_cast<DiagnosticHandler *>(context.getDiagHandlerPtr()));

  SMDiagnostic diag;
  auto module =
      parseIR({StringRef(moduleBuf, moduleSize), "tpc_kernel"}, diag, context);
  if (!module) {
    diag.print(nullptr, errs());
    return 0;
  }

  module->setTargetTriple(Triple::normalize("tpc"));

  if (asmIR) {
    unsigned size;
    GlobalBufOStream os(*asmIR, size);
    module->print(os, nullptr);
    os.write("", 1);
  }

  IntrusiveRefCntPtr<DiagnosticOptions> diaOptions(new DiagnosticOptions());
  IntrusiveRefCntPtr<DiagnosticsEngine> diags(
      new DiagnosticsEngine(new DiagnosticIDs(), diaOptions));

  diags->setClient(new TextDiagnosticPrinter(dbgs(), diaOptions.get()));

  unsigned optLevel = ::optLevel;
  assert(optLevel <= 3);

  HeaderSearchOptions headerOptions;
  LangOptions langOptions;
  DataLayout dataLayout(DataLayoutStringTPC);

  clang::TargetOptions targetOptions;
  targetOptions.CodeModel = "default";
  targetOptions.CPU = (!cpu || !*cpu) ? "goya" : cpu;
  targetOptions.Triple = module->getTargetTriple();

  CodeGenOptions cgOptions;
  cgOptions.OptimizationLevel = optLevel;
  cgOptions.UnrollLoops = optLevel > 1;
  cgOptions.CodeModel = targetOptions.CodeModel;
  cgOptions.ThreadModel = "single";

  // At O0 we want to fully disable inlining outside of cases marked with
  // 'alwaysinline' that are required for correctness.
  cgOptions.setInlining((optLevel == 0) ? CodeGenOptions::OnlyAlwaysInlining
                                        : CodeGenOptions::NormalInlining);

  // Set to MAX_SLM of the architecture.
  cgOptions.ScalarLocalMemory = StringSwitch<unsigned>(targetOptions.CPU)
                                    .Case("goya2", 2 * 1024)
                                    .Case("greco", 2 * 1024)
                                    .Case("gaudi2", 16 * 1024)
                                    .Case("doron1", 16 * 1024)
                                    .Default(1024);

  cgOptions.LutWarn = true;
  cgOptions.CompressInstructions =
      optLevel >= 2 && StringSwitch<bool>(targetOptions.CPU)
                                    .Case("goya", false)
                                    .Case("gaudi", false)
                                    .Case("gaudib", false)
                                    .Default(true);
  cgOptions.VerifyModule = verify;
  cgOptions.setEmbedBitcode(CodeGenOptions::Embed_Bitcode);

  unsigned elfSize = 0;
  clang::EmitBackendOutput(
      *diags, headerOptions, cgOptions, targetOptions, langOptions, dataLayout,
      module.get(), Backend_EmitObj,
      std::make_unique<GlobalBufOStream>(*elfBin, elfSize));

  if (hasError || diags->hasErrorOccurred())
    return 0;

  return elfSize;
}

void llvm_tpc_startup(const char *args) {
  // Override the LLVM fatal error handlers.
  install_bad_alloc_error_handler(handleLLVMFatalError);
  install_fatal_error_handler(handleLLVMFatalError);

  // Initialize targets and passes.
  InitializeAllTargets();
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();
  InitializeAllDisassemblers();

  //
  // Function private (inner) static variables are initialized lazily. To make
  // sure they are initialized in the "global allocation heap" context, we
  // force their initialization, by calling their parent functions.
  //

  // Initialize `static std::unordered_set<std::string> PrintFuncNames`.
  isFunctionInPrintList("");

  // Initialize `static IntrusiveRefCntPtr<FileSystem> FS`.
  vfs::getRealFileSystem();

  // Initialize the static Automaton.
  initAutomaton();

  // Tokenize command line arguments.
  SmallVector<const char *, 20> argv(1, "tpc-llvm");
  BumpPtrAllocator allocator;
  StringSaver saver(allocator);
  if (Triple(sys::getProcessTriple()).isOSWindows())
    cl::TokenizeWindowsCommandLine(args, saver, argv);
  else
    cl::TokenizeGNUCommandLine(args, saver, argv);

  // Search the last occurrence of -O.
  auto itOptLevel = find_if(reverse(argv), [](StringRef arg) {
    return arg.size() == 3 && arg.consume_front("-O");
  });

  // Add the common arguments (independent of the optimization level).
  argv.insert(itOptLevel.base(), {
                                     "-simplifycfg-sink-common=false",
                                     "-enable-load-pre=false",
                                     "-loop-unswitch-threshold=0",
                                     "-unroll-max-count=4",
                                     "-dontAnalysis=1",
                                 });

  // If we have found -O0 then expand its command line options.
  if (itOptLevel != argv.rend() && (*itOptLevel)[2] == '0') {
    argv.insert(itOptLevel.base(), {
                                       "-unroll-runtime=false",
                                       "-unroll-pipelined=true",
                                       "-enable-misched=false",
                                       "-enable-post-misched=false",
                                       "-optimize-predicates=false",
                                       "-set_indx-coalescer=false",
                                       "-tpc-single-packets=true",
                                       "-tpc-debug-nops=8",
                                   });
  }
  // Otherwise, add the default command line options, common to all optimization
  // levels above 0.
  else {
    argv.insert(itOptLevel.base(), {
                                       "-unroll-runtime=false",
                                       "-tpc-unroll-count=4",
                                       "-unroll-pipelined=false",
                                       "-enable-misched=true",
                                       "-enable-post-misched=true",
                                       "-optimize-predicates=true",
                                       "-set_indx-coalescer=true",
                                       "-tpc-single-packets=false",
                                       "-tpc-debug-nops=0",
                                       "-enable-cast-swizzle-opt=true",
                                   });
  }

  cl::ParseCommandLineOptions(argv.size(), argv.data(), "TPC LLVM compiler\n",
                              &errs());
}

void llvm_tpc_shutdown() {
  remove_bad_alloc_error_handler();
  remove_fatal_error_handler();
  llvm_shutdown();
}

void llvm_tpc_cleanup() {
  // Reset global and static variables.
  TPCLatencyEvaluation::latenciesDB.clear();
  TPCLatencyEvaluation::sopDB.clear();
  initAutomaton();
}
