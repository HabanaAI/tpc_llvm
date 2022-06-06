//===--- EmbeddedTpcHeaders.cpp -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
//===----------------------------------------------------------------------===//
//
//  This file contains content of embedded headers.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/FileEntry.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include <memory>
using namespace clang;
using namespace llvm;

#define DEBUG_TYPE "tpclib"

static const char ContentOf_tpcdefs[] = {
#include "tpc-defs.h.inc"
  '\0'
};

static MemoryBuffer *BufferOf_tpc_defs
    = MemoryBuffer::getMemBuffer(ContentOf_tpcdefs, "tpc-defs.h").release();

static const char ContentOf_reduction[] = {
#include "tpc-reduction_functions_core.h.inc"
  '\0'
};

static MemoryBuffer *BufferOf_reduction
    = MemoryBuffer::getMemBuffer(ContentOf_reduction, "tpc-reduction_functions_core.h").release();

static const char ContentOf_tpc_special[] = {
#include "tpc-special.h.inc"
  '\0'
};

static MemoryBuffer *BufferOf_tpc_special
    = MemoryBuffer::getMemBuffer(ContentOf_tpc_special, "tpc-special.h").release();

static const CompilerInvocation::EmbeddedFile EmbeddedFiles[] = {
  CompilerInvocation::EmbeddedFile { "tpc-defs.h", BufferOf_tpc_defs },
  CompilerInvocation::EmbeddedFile { "tpc-reduction_functions_core.h", BufferOf_reduction },
  CompilerInvocation::EmbeddedFile { "tpc-special.h", BufferOf_tpc_special }
};

ArrayRef<CompilerInvocation::EmbeddedFile> CompilerInvocation::getEmbeddedFiles() {
  return ArrayRef<CompilerInvocation::EmbeddedFile>(
      EmbeddedFiles, array_lengthof(EmbeddedFiles));
}


// Embedded IR libraries.
static const char Content_library_goya[] = {
#include "goya.bc.inc"
  '\0'
};
static const char Content_library_gaudi[] = {
#include "gaudi.bc.inc"
  '\0'
};
static const char Content_library_gaudib[] = {
#include "gaudib.bc.inc"
  '\0'
};
static const char Content_library_greco[] = {
#include "greco.bc.inc"
  '\0'
};
static const char Content_library_gaudi2[] = {
#include "gaudi2.bc.inc"
  '\0'
};
static const char Content_library_doron1[] = {
#include "doron1.bc.inc"
  '\0'
};

void clang::registerEmbeddedLibraries(llvm::vfs::InMemoryFileSystem &FS, CompilerInstance &CI) {
  CompilerInvocation &Invocation = CI.getInvocation();
  if (!Invocation.getLangOpts()->CompileForLibrary) {
    if (Triple(Invocation.getTargetOpts().Triple).getArch() == Triple::ArchType::tpc) {
      StringRef StaticContent;
      StringRef BitcodeName;
      if (Invocation.getTargetOpts().CPU == "goya") {
        StaticContent = StringRef(Content_library_goya, sizeof(Content_library_goya) - 1);
        BitcodeName = "goya.bc";
      } else if (Invocation.getTargetOpts().CPU == "gaudi") {
        StaticContent = StringRef(Content_library_gaudi, sizeof(Content_library_gaudi) - 1);
        BitcodeName = "gaudi.bc";
      } else if (Invocation.getTargetOpts().CPU == "gaudib") {
        StaticContent = StringRef(Content_library_gaudib, sizeof(Content_library_gaudib) - 1);
        BitcodeName = "gaudib.bc";
      } else if (Invocation.getTargetOpts().CPU == "greco") {
        StaticContent = StringRef(Content_library_greco, sizeof(Content_library_greco) - 1);
        BitcodeName = "greco.bc";
      } else if (Invocation.getTargetOpts().CPU == "gaudi2") {
        StaticContent = StringRef(Content_library_gaudi2, sizeof(Content_library_gaudi2) - 1);
        BitcodeName = "gaudi2.bc";
      } else if (Invocation.getTargetOpts().CPU == "doron1") {
        StaticContent = StringRef(Content_library_doron1, sizeof(Content_library_doron1) - 1);
        BitcodeName = "doron1.bc";
      }
//      if (StaticContent.empty()) {
//        LLVM_DEBUG(dbgs() << "No IR embedded library files were used\n");
//        return;
//      }
      LLVM_DEBUG(dbgs() << "Bitcode library '" << BitcodeName << "' was used");

      std::unique_ptr<llvm::MemoryBuffer> Buffer;
      Buffer = llvm::MemoryBuffer::getMemBuffer(StaticContent, BitcodeName, false);

      SmallString<256> LibraryPath = StringRef(Invocation.getHeaderSearchOpts().ResourceDir);
      llvm::sys::path::append(LibraryPath, "lib");
      llvm::sys::path::append(LibraryPath, Buffer->getBufferIdentifier());
      FS.addFile(LibraryPath, 0, std::move(Buffer));
    }
  }
}
