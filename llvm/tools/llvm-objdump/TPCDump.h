//===-- TPCDump.h --- TPC-specific functionality in llvm-objdump ----------===//
//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TOOLS_LLVM_OBJDUMP_TPCDUMP_H
#define LLVM_TOOLS_LLVM_OBJDUMP_TPCDUMP_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetRegistry.h"
#include <stdint.h>
#include <string>

namespace llvm {

class formatted_raw_ostream;
struct TPCMetadataSection;
class MCDisassembler;
class MCContext;

namespace object {
class ObjectFile;
class SectionRef;
}

namespace objdump {
extern cl::opt<std::string> MCPU;
}

const Target *getTarget(const object::ObjectFile *Obj);
inline bool isTPCTarget(const object::ObjectFile *Obj) {
  return getTarget(Obj)->getName() == StringRef("tpc");
}

void printTpcCompressStatus(const object::ObjectFile *O);
void printTpcEncodingInfo(const object::ObjectFile *O,
                          ArrayRef<uint8_t> instruction,
                          formatted_raw_ostream &FOS);
bool TpcEncodingCover(StringRef Triple);
void calculateMCPU(const object::ObjectFile *ObjFile);
std::string getTPCCPUName(const object::ObjectFile *ObjFile, std::string MCPU);
TPCMetadataSection getMetadataFromBinarySection(const object::SectionRef &Section);
void printTPCMetadataFromBinary(const object::SectionRef &Section);
void printIndexMap(const object::SectionRef &Section);
void collectSymbolsTpc(const object::ObjectFile *Obj, const MCDisassembler &DisAsm,
                       MCContext &Context, SectionSymbolsTy &Symbols);
bool getTPCData(const object::SectionRef &Section, bool &IsIndexMap, bool &IsBinaryTPCMetadata);
void printTPCTables(const object::ObjectFile *Obj, bool Disassemble);
bool printTPCArch(const object::ObjectFile *Obj);
bool tryDisassembleTPCSection(const object::SectionRef &Section);
bool printAsTPCSymbol(const object::SectionRef &Section, SymbolInfoTy Symbol);
bool hasTPCActionOptions();
bool printCommonHeader(const object::ObjectFile *Obj);
} // namespace llvm

#endif
