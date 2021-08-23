//===-TPCDump.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifdef LLVM_TPC_COMPILER
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#include "llvm-objdump.h"
#include "tpc-encoding-info.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Target/TPCMetadataSection.h"

using namespace llvm;
using namespace object;

cl::opt<bool> llvm::TPCCompressStatus("tpc-compress-info",
  cl::desc("Displays compression status of TPC instructions"));

cl::opt<std::string> llvm::TPCCompressStatusFile(
    "tpc-compress-info-file",
    cl::desc("Name of the output file for TPC compression status information (default is 'stdout')"),
    cl::ValueOptional, cl::init("-"));

cl::opt<bool> llvm::TPCLeadingAddr("tpc-leading-addr",
                                   cl::desc("Print leading address"));

cl::opt<bool> llvm::TPCEncodingInfo(
    "tpc-encoding-info",
    cl::desc("Displays the encoding info for instructions."));

cl::opt<bool> llvm::IgnoreMovDT(
    "ignore-mov-dt",
    cl::desc("Do not print fields reflecting data type for MOV instructions."),
    cl::Hidden);

cl::opt<bool> llvm::NoCommonHeader(
    "no-common-header",
    cl::desc("Not to print the common header for assembler text."));

enum TPCArch {
  Goya   = 1,
  Gaudi  = 2
};

static TPCArch TpcArch = Goya;

static TPCArch MCPUToTPCArch() {
  if (MCPU.compare("dali") == 0 || MCPU.compare("goya") == 0)
    return Goya;
  else if (MCPU.compare("gaudi") == 0)
    return Gaudi;
  else
    reportError("", "'" + MCPU + "' is not a recognized processor for this target");
}

bool isDali() { return TpcArch == Goya; }

bool isGaudi() { return TpcArch == Gaudi; }

bool skipTpcOperandsType(const std::vector<Encoding> *enc,
                            std::string bins, size_t instr_size) {
  int8_t src = SRCA_IND;
  int8_t dst = DEST_IND;

  uint8_t start_bit = instr_size - (*enc)[src].start_bit - (*enc)[src].field_size;
  unsigned long long value = std::stoull(bins.substr(start_bit, (*enc)[src].field_size), 0, 2);
  if (value <= 44 || (value >= 240 && value <= 255)) {
    // Source is VRF or VPRF. Look at the destination as well.
    start_bit = instr_size - (*enc)[dst].start_bit - (*enc)[dst].field_size;
    value = std::stoull(bins.substr(start_bit, (*enc)[dst].field_size), 0, 2);
    if (value <= 44 || (value >= 240 && value <= 255)) {
      // Destination is VRF or VPRF too.
      return true;
    }
  }

  return false;
}

bool IsMovFlavorSet(const std::vector<Encoding> *enc, std::string bins, size_t instr_size) {
  int8_t src = SRCA_IND;
  int8_t dst = DEST_IND;
  uint8_t start_bit = instr_size - (*enc)[src].start_bit - (*enc)[src].field_size;
  unsigned long long value = std::stoull(bins.substr(start_bit, (*enc)[src].field_size), 0, 2);
  if ((value >= 64 && value <= 99) || (value >= 111 && value <= 127)) {
    // Source is SRF or IMM. Look at the destination as well.
    start_bit = instr_size - (*enc)[dst].start_bit - (*enc)[dst].field_size;
    value = std::stoull(bins.substr(start_bit, (*enc)[dst].field_size), 0, 2);
    if (value >= 240 && value <= 255) {
      // Destination is VPRF.
      return true;
    }
  }

  return false;
}

static void printSectionCompressStatus(const SectionRef &Section, const ObjectFile *Obj,
                                  raw_fd_ostream &OS) {
  StringRef Contents = unwrapOrError(Section.getContents(), Obj->getFileName());

  uint64_t BaseAddr = Section.getAddress();
  int instrNum = 0;
  for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 32) {
    instrNum++;
    if (TPCLeadingAddr) {
      OS << format(" %04" PRIx64 ": ", BaseAddr + addr);
    }
    unsigned firstByte = hexDigitValue(hexdigit(Contents[addr] & 0xF));
    bool compressed = (firstByte & 1);
    OS << (compressed ? '1' : '0');
    OS << "\n";
  }
}

void llvm::printTpcEncodingInfo(ArrayRef<uint8_t> instruction) {
  const std::vector<Encoding> *enc = &dali_encoding;
  if (isDali())
    enc = &dali_encoding;
  else if (isGaudi())
    enc = &gaudi_encoding;

  std::string binaries;
  for (size_t i = 0; i < instruction.size(); i++) {
    std::bitset<8> bs(instruction[i]);
    binaries.insert(0, bs.to_string());
  }

  uint8_t start_bit;
  size_t instr_size = instruction.size() << 3;
  unsigned long long spu_opcode = 0xFF;
  unsigned long long vpu_opcode = 0xFF;
  unsigned long long ld_opcode  = 0xFF;
  int8_t spu_opcode_ind = -1;
  int8_t vpu_opcode_ind = -1;
  int8_t ld_opcode_ind  = -1;
  if (isDali() || isGaudi()) {
    spu_opcode_ind = SPU_OPCODE_IND;
    vpu_opcode_ind = VPU_OPCODE_IND;
    ld_opcode_ind  = LD_OPCODE_IND;
  } else
    assert(false && "Unknown arch for printing encoding info");

  if (spu_opcode_ind != -1) {
  start_bit  = instr_size - (*enc)[spu_opcode_ind].start_bit - (*enc)[spu_opcode_ind].field_size;
  spu_opcode = std::stoull(binaries.substr(start_bit, (*enc)[spu_opcode_ind].field_size), 0, 2);
  }

  if (vpu_opcode_ind != -1) {
  start_bit  = instr_size - (*enc)[vpu_opcode_ind].start_bit - (*enc)[vpu_opcode_ind].field_size;
  vpu_opcode = std::stoull(binaries.substr(start_bit, (*enc)[vpu_opcode_ind].field_size), 0, 2);
  }

  if (ld_opcode_ind != -1) {
  start_bit  = instr_size - (*enc)[ld_opcode_ind].start_bit - (*enc)[ld_opcode_ind].field_size;
  ld_opcode  = std::stoull(binaries.substr(start_bit, (*enc)[ld_opcode_ind].field_size), 0, 2);
  }

  if (spu_opcode == LOOP_OPCODE) { //// for LOOP instruction a special encoding has to be used.
    enc = &loop_encoding;
  }

  bool ignoreMovDT = IgnoreMovDT && (ld_opcode == MOV_OPCODE_LD_SLOT || vpu_opcode == MOV_OPCODE_VPU_SLOT) && skipTpcOperandsType(enc, binaries, instr_size);
  outs() << "\t// ";
  uint8_t num = (*enc).size();
  for (const Encoding rule : *enc) {
    std::string fn = rule.field_name;
    start_bit = instr_size - rule.start_bit - rule.field_size;
    unsigned long long value = std::stoull(binaries.substr(start_bit, rule.field_size), 0, 2);
    if (ignoreMovDT) {
      if (vpu_opcode == MOV_OPCODE_VPU_SLOT && fn == "VPU_OPERANDS_TYPE")
        value = 0;
      if (ld_opcode == MOV_OPCODE_LD_SLOT && (fn == "VPU_SRC_D_LD_SRC_B" || fn == "LOAD_SRC_B"))
        value = ((value >> 8) << 8) || (value & 0xf);
    }

    if (vpu_opcode == MOV_OPCODE_VPU_SLOT && fn == "VPU_SRC_B" && !IsMovFlavorSet(enc, binaries, instr_size))
      value = 0;

    if (ld_opcode == MOV_OPCODE_LD_SLOT && fn == "VPU_SRC_D_LD_SRC_B" && !IsMovFlavorSet(enc, binaries, instr_size))
      value = 0;

    if (--num == 0)
      outs() << fn << "=" << value << "\n";
    else
      outs() << fn << "=" << value << ",";
  }
}

static void printSectionEncodingInfo(const StringRef &Contents) {
  ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(Contents.data()),
                          Contents.size());
  for (std::size_t addr = 0, end = Bytes.size() - 31; addr < end; addr += 32) {
    printTpcEncodingInfo(Bytes.slice(addr, 32));
  }
}

void llvm::printTpcCompressStatus(const ObjectFile *Obj) {
  assert(Obj != nullptr);
  std::error_code EC;

  llvm::raw_fd_ostream OS(TPCCompressStatusFile, EC,
              llvm::sys::fs::F_Text);
  if (EC) {
    reportError(TPCCompressStatusFile, "Can't open output file");
    return;
  }

  for (const SectionRef &Section : ToolSectionFilter(*Obj)){
    if (!Section.getSize())
      continue;
    if (!Section.isText())
      continue;
    printSectionCompressStatus(Section, Obj, OS);
  }
}

void llvm::printTpcEncodingInfo(const ObjectFile *Obj) {
  assert(Obj != nullptr);
  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    if (!Section.isText())
      continue;
    if (!Section.getSize())
      continue;
    printSectionEncodingInfo(unwrapOrError(Section.getContents(), Obj->getFileName()));
  }
}

void llvm::collectSymbolsTpc(
    const ObjectFile *Obj, const MCDisassembler &DisAsm, MCContext &Context,
    std::vector<std::tuple<uint64_t, StringRef, uint8_t>> &Symbols) {

  assert(Obj != nullptr);
  for (SectionRef Section : Obj->sections()) {
    if (!Section.isText())
      continue;
    if (!Section.getSize())
      continue;

    uint64_t SectionSize = Section.getSize();
    uint64_t Start = Section.getAddress();
    uint64_t End = Start + SectionSize;
    uint64_t Size = 32;

    StringRef BytesStr = unwrapOrError(Section.getContents(), Obj->getFileName());
    ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(BytesStr.data()),
                            BytesStr.size());

    int iLoop=0, iJmp=0;
    for (uint64_t Index = Start; Index < End; Index += Size) {
      MCInst  Inst;
      int64_t Offset = 0;
      bool isOK = DisAsm.getInstruction(Inst, Size, Bytes.slice(Index), (int64_t)&Offset, nulls());
      if (isOK && Offset) {
        char *sbuf = static_cast<char *>(Context.allocate(32));
        if (Inst.getOpcode() == 0x22) { //=TPCII::spuLOOP
          sprintf(sbuf,".L_LOOP_%d",++iLoop);
          Offset += Size; // Label for LOOP should be after the body
        } else {
          sprintf(sbuf,".L_JMP_%d",++iJmp);
        }
        uint64_t Address = Index + Offset;
        auto it = std::find_if(Symbols.begin(), Symbols.end(),
                [Address](const std::tuple<uint64_t,StringRef,uint8_t>& e) {
                    return std::get<0>(e) == Address;
                });
        if (it == Symbols.end()) {
          Symbols.emplace_back(Address, sbuf, ELF::STT_NOTYPE);
        }
      }
    }
  }
}

enum class Slots : uint8_t { Load, Scalar, Vector, Store };

void PrintTPCMetadata(const TPCMetadataSection &Header) {
  for (const TPCMetadataFieldInfo *Cur = std::begin(TPCMetadataFieldsInfo);
       Cur != std::end(TPCMetadataFieldsInfo); ++Cur) {
    if (StringRef(Cur->fieldName).equals(TPCBubbleName))
      continue;
    if (Cur->startWithVersion > Header.version)
      continue;

    if (!Cur->isArray()) {
      outs() << '\t';
      outs() << Cur->fieldName;
      outs() << ": ";

      outs() << TPCMetadataTypeDirectives.at(Cur->elementSize);
      outs() << ' ';

      if (StringRef(Cur->fieldName).equals(TPCVersionName))
        outs() << Header.version;
      else if (StringRef(Cur->fieldName).equals(TPCSpecialFunctionUsedName))
        outs() << Header.specialFunctionUsed;
      else if (StringRef(Cur->fieldName).equals(TPCPrintfUsedName))
        outs() << Header.printfUsed;
      else if (StringRef(Cur->fieldName).equals(TPCLockUnLockName))
        outs() << Header.lockUnLock;
      else if (StringRef(Cur->fieldName).equals(TPCMarchName))
        outs() << std::to_string(Header.march);
      else if (StringRef(Cur->fieldName).equals(TPCMMIOName))
        outs() << std::to_string(Header.mmioUsed);
      else if (StringRef(Cur->fieldName).equals(TPCParamsNumName))
        outs() << std::to_string(Header.paramsNum);
      else if (StringRef(Cur->fieldName).equals(TPCPrintfTensorIDName))
        outs() << std::to_string(Header.printfTensorID);
      else
        llvm_unreachable(TPCUnhandledMetadataField);

      outs() << '\n';
    } else {
      for (unsigned i = 0; i < Cur->length; ++i) {
        bool CurrentValue = false;
        if (StringRef(Cur->fieldName).equals(TPCScalarLdName))
          CurrentValue = Header.scalarLd[i];
        else if(StringRef(Cur->fieldName).equals(TPCRMWStoreName))
          CurrentValue = Header.rmwStore[i];
        else
          llvm_unreachable(TPCUnhandledMetadataField);

        if (CurrentValue) {
          outs() << '\t';
          outs() << Cur->fieldName;
          outs() << '[';
          outs() << i;
          outs() << "]: ";

          outs() << TPCMetadataTypeDirectives.at(Cur->elementSize);
          outs() << ' ';

          outs() << CurrentValue;

          outs() << '\n';
        }
      }
    }
  }
}

static section_iterator findTpcMetadataSection(
    const ObjectFile *ObjectFile) {
  auto Iter = std::find_if(
        ObjectFile->section_begin(), ObjectFile->section_end(),
        [](const SectionRef& Section) {
          Expected<StringRef> SectionNameOrError = Section.getName();
          if (!SectionNameOrError)
            return false;
          else if (SectionNameOrError->equals(BinaryTPCMetadataSectionName))
            return true;
          else if (SectionNameOrError->equals(StringTPCMetadataSectionName))
            return true;
          else
            return false;
        }
  );

  return Iter;
}



void llvm::calculateMCPU(const ObjectFile *ObjFile) {
  section_iterator MetadataSection =
      findTpcMetadataSection(ObjFile);


  if (!MCPU.empty())
    TpcArch = MCPUToTPCArch();

  if (MetadataSection == ObjFile->section_end()) {
    reportWarning(formatv("No {0} section. Possibly this is not a TPC elf.",
                          BinaryTPCMetadataSectionName),
                  ObjFile->getFileName());
    return;
  }

  Expected<StringRef> SectionNameOrError = MetadataSection->getName();
  if (!SectionNameOrError)
    consumeError(SectionNameOrError.takeError());

  TPCMetadataSection Metadata;
  if (SectionNameOrError->equals(BinaryTPCMetadataSectionName))
    Metadata = getMetadataFromBinarySection(*MetadataSection);
  else if (SectionNameOrError->equals(StringTPCMetadataSectionName))
    Metadata = getMetadataFromStringSection(*MetadataSection);
  else
    llvm_unreachable("Unknown TPC metadata section");

  if (!MCPU.empty()) {
    if (TpcArch != Metadata.march)
      reportWarning("Specified '-mcpu' value contradicts architecture encoded in tpc_metadata section.",
                    MetadataSection->getObject()->getFileName());

  } else {
    switch (Metadata.march) {
    case 1:
      TpcArch = Goya;
      break;
    case 2:
      TpcArch = Gaudi;
      break;
    default:
      llvm_unreachable("Unhandled tpc arch");
    }
  }
}

std::string llvm::getCPUName() {
  switch (TpcArch) {
  case Goya:
    return "goya";
  case Gaudi:
    return "gaudi";
  default:
    llvm_unreachable("Unhandled CPU name");
  }
}

TPCMetadataSection llvm::getMetadataFromStringSection(
    const SectionRef& Section) {
  Expected<StringRef> SectionContentOrError = Section.getContents();
  if (!SectionContentOrError)
    consumeError(SectionContentOrError.takeError());

  return stringDeserializeTPCProgramHeader(*SectionContentOrError);
}

TPCMetadataSection llvm::getMetadataFromBinarySection(
    const SectionRef& Section) {
  Expected<StringRef> SectionContentOrError = Section.getContents();
  if (!SectionContentOrError)
    consumeError(SectionContentOrError.takeError());

  std::vector<uint8_t> BinaryInfo;
  for(unsigned i = 0; i < SectionContentOrError->size(); ++i)
    BinaryInfo.push_back((uint8_t)(*SectionContentOrError)[i]);
  return binaryDeserializeTPCProgramHeader(BinaryInfo);
}

void llvm::printTPCMetadataFromString(const SectionRef& Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError &&
         SectionNameOrError->equals(StringTPCMetadataSectionName));

  TPCMetadataSection Header = getMetadataFromStringSection(Section);

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  PrintTPCMetadata(Header);
}

void llvm::printTPCMetadataFromBinary(const SectionRef& Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError &&
         SectionNameOrError->equals(BinaryTPCMetadataSectionName));

  TPCMetadataSection Header = getMetadataFromBinarySection(Section);

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  PrintTPCMetadata(Header);
}

void llvm::printIndexMap(const SectionRef &Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError &&
         SectionNameOrError->equals(".IndexMap"));

  Expected<StringRef> SectionContentOrError = Section.getContents();
  if (!SectionContentOrError)
    consumeError(SectionContentOrError.takeError());

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  outs() << *SectionContentOrError;
  outs() << '\n';
}

void llvm::printUnrollInfo(const SectionRef &Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError && SectionNameOrError->equals(".UnrollInfo"));

  Expected<StringRef> SectionContentOrError = Section.getContents();
  if (!SectionContentOrError)
    consumeError(SectionContentOrError.takeError());

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  outs() << *SectionContentOrError;
  outs() << '\n';
}

void llvm::printBailoutInfo(const SectionRef &Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError && SectionNameOrError->equals(".BailoutGCCUSTOM"));

  Expected<StringRef> SectionContentOrError = Section.getContents();
  if (!SectionContentOrError)
    consumeError(SectionContentOrError.takeError());

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  outs() << *SectionContentOrError;
  outs() << '\n';
}

#endif
