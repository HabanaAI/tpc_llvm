//===--- TPC.cpp - TPC ToolChain Implementations ----------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//

#include "Arch/TPC.h"
#include "TPC.h"
#include "CommonArgs.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

const bool Default_load_tpc_library = false;

void clang::driver::addCommonTPCTargetArgs(const ArgList &Args, ArgStringList &CmdArgs) {
  // If no options -instr-compress and -no-option-compress specified, turn the
  // compression on if -O2 is specified and target supports compression.
  if (!Args.hasArg(options::OPT_instr_compress, options::OPT_no_instr_compress)) {
    bool NeedCompression = llvm::StringSwitch<bool>(StringRef(tpc::getTPCTargetCPU(Args)))
                                    .Case("goya", false)
                                    .Case("gaudi", false)
                                    .Case("gaudib", false)
                                    .Default(true);
    if (NeedCompression) {
      // -O2 is default, check if overridden
      if (Arg *A = Args.getLastArg(options::OPT_O_Group))
        if (A->getOption().matches(options::OPT_O0) ||
            (A->getNumValues() && StringRef(A->getValue()).equals("1")))
          NeedCompression = false;
    }
    if (NeedCompression)
      CmdArgs.push_back("-instr-compress");
    else
      CmdArgs.push_back("-no-instr-compress");
  } else if (Args.hasFlag(options::OPT_instr_compress, options::OPT_no_instr_compress))
    CmdArgs.push_back("-instr-compress");
  else
    CmdArgs.push_back("-no-instr-compress");

}


TPCToolChain::TPCToolChain(const Driver &D, const llvm::Triple &Triple,
                           const ArgList &Args)
  : Generic_ELF(D, Triple, Args) {
}

Tool *TPCToolChain::buildLinker() const {
  getDriver().Diag(diag::err_drv_link_unavailable);
  return nullptr;
}

void TPCToolChain::addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                                         llvm::opt::ArgStringList &CC1Args,
                                         Action::OffloadKind DeviceOffloadKind) const {
  StringRef CPUName = tpc::getTPCTargetCPU(DriverArgs);

  addCommonTPCTargetArgs(DriverArgs, CC1Args);

  // Enable warnings that are useful for TPC.
  CC1Args.push_back("-Wfloat-conversion");
  CC1Args.push_back("-Wmissing-braces");
  CC1Args.push_back("-Wuninitialized");
  CC1Args.push_back("-Werror=implicit-function-declaration");

  CC1Args.push_back("-fembed-bitcode=bitcode");

  if (CPUName.equals("gaudi2") || CPUName.equals("doron1"))
    CC1Args.push_back("-float8");
  if (CPUName.equals("gaudi") || CPUName.equals("gaudib") ||
      CPUName.equals("greco") || CPUName.equals("gaudi2") ||
      CPUName.equals("doron1"))
    CC1Args.push_back("-bfloat16");

  if (Arg *A = DriverArgs.getLastArg(options::OPT_vector_local_memory)) {
    CC1Args.push_back("-vlm");
    CC1Args.push_back(A->getValue());
  }
  CC1Args.push_back("-slm");
  if (Arg *A = DriverArgs.getLastArg(options::OPT_scalar_local_memory)) {
    CC1Args.push_back(A->getValue());
  } else {
    if (CPUName.equals("gaudi"))
      CC1Args.push_back("1");  //kb, MAX_SLM for gaudi architecture
    else if (CPUName.equals("gaudib"))
      CC1Args.push_back("1"); // kb, MAX_SLM for gaudib architecture
    else if (CPUName.equals("greco"))
      CC1Args.push_back("2");  //kb, MAX_SLM for goya2 architecture
    else if (CPUName.equals("gaudi2") || CPUName.equals("doron1"))
      CC1Args.push_back("16"); //kb, MAX_SLM for gaudi2/doron1 architecture
    else
      CC1Args.push_back("1");  //kb, MAX_SLM for dali (goya) architecture
  }

  if (Arg *A = DriverArgs.getLastArg(options::OPT_spill_vlm)) {
    CC1Args.push_back("-spill-vlm");
    CC1Args.push_back(A->getValue());
  }

  CC1Args.push_back("-mllvm");
  CC1Args.push_back("-simplifycfg-sink-common=false");
  CC1Args.push_back("-mllvm");
  CC1Args.push_back("-enable-load-pre=false");

  if (Arg *A = DriverArgs.getLastArg(options::OPT_main_function)) {
    CC1Args.push_back("-main-function");
    CC1Args.push_back(A->getValue());
  }

  CC1Args.push_back("-max-tensors");
  if (Arg *A = DriverArgs.getLastArg(options::OPT_max_tensors)) {
    CC1Args.push_back(A->getValue());
  } else {
    if (CPUName.equals("gaudi"))
      CC1Args.push_back("16");  // MAX_TENSORS for gaudi architecture
    else if (CPUName.equals("gaudib"))
      CC1Args.push_back("16");  // MAX_TENSORS for gaudib architecture
    else if (CPUName.equals("greco"))
      CC1Args.push_back("16");  // MAX_TENSORS for goya2 architecture
    else if (CPUName.equals("gaudi2"))
      CC1Args.push_back("16");  // MAX_TENSORS for gaudi2 architecture
    else if (CPUName.equals("doron1"))
      CC1Args.push_back("16");  // MAX_TENSORS for gaudi2 architecture
    else
      CC1Args.push_back("8");   // MAX_TENSORS for dali (goya) architecture
  }

  if (DriverArgs.hasArg(options::OPT_no_tpc_defs))
    CC1Args.push_back("-no-tpc-defs");
  CC1Args.push_back("-tpc-special");

  if (DriverArgs.hasArg(options::OPT_use_printf_in_module)) {
    CC1Args.push_back("-use-printf");
  }

  if (DriverArgs.hasArg(options::OPT_all_loops_taken)) {
    CC1Args.push_back("-all-loops-taken");
  }

  if (DriverArgs.hasArg(options::OPT_reg_mem_count)) {
    CC1Args.push_back("-reg-mem-count");
  }

  if (DriverArgs.hasArg(options::OPT_disable_instruction_scheduling)) {
    Arg *A = DriverArgs.getLastArg(options::OPT_O_Group);
    // skip if -O0
    if (!A || !A->getOption().matches(options::OPT_O0)) {
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-enable-misched=0");
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-enable-post-misched=0");
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-post-RA-scheduler=0");
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-disable-tpc-packetizer");
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-tpc-trans-intr=0");
    }
  }

  if (DriverArgs.hasArg(options::OPT_disable_lut_warn)) {
    CC1Args.push_back("-disable-lut-warn");
  }

  if (DriverArgs.hasFlag(options::OPT_long_irf, options::OPT_no_long_irf, false)) {
    CC1Args.push_back("-long-irf");
  }

  if (DriverArgs.hasFlag(options::OPT_tpc_dnorm, options::OPT_no_tpc_dnorm,
                         false)) {
    CC1Args.push_back("-tpc-dnorm");
  }

  if (DriverArgs.hasArg(options::OPT_tpc_library)) {
    CC1Args.push_back("-tpc-library");
  } else {
    if (DriverArgs.hasFlag(options::OPT_load_tpc_library,
                           options::OPT_no_load_tpc_library,
                           Default_load_tpc_library)) {
      // BC library will be searched for in default library path.
      SmallString<256> DefaultLibPath = StringRef(getDriver().ResourceDir);
      llvm::sys::path::append(DefaultLibPath, Twine("lib"));

      // Determine BC library file name.
      std::string BCLibraryName;
      if (CPUName.equals("goya")) {
        BCLibraryName = "goya.bc";
      } else if (CPUName.equals("gaudi")) {
        BCLibraryName = "gaudi.bc";
      } else if (CPUName.equals("gaudib")) {
        BCLibraryName = "gaudib.bc";
      } else if (CPUName.equals("greco")) {
        BCLibraryName = "greco.bc";
      } else if (CPUName.equals("gaudi2")) {
        BCLibraryName = "gaudi2.bc";
      } else if (CPUName.equals("doron1")) {
        BCLibraryName = "doron1.bc";
      }

      SmallString<256> BCLibraryPath;
      BCLibraryPath = DefaultLibPath;
      llvm::sys::path::append(BCLibraryPath, BCLibraryName);
      CC1Args.push_back("-mlink-builtin-bitcode");
      CC1Args.push_back(DriverArgs.MakeArgString(BCLibraryPath));
    }
  }

  CC1Args.push_back("-mllvm");
  CC1Args.push_back("-loop-unswitch-threshold=0");

  // TPC Cost Model.
  CC1Args.push_back("-mllvm");
  CC1Args.push_back("-dontAnalysis=1");
}
