//===-- TPCDump.cpp --- TPC-specific functionality in llvm-objdump --------===//
//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "TPCDump.h"
#include "llvm-objdump.h"
#include "tpc-encoding-info.h"
#include "tpc-encoding-cover.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Target/TPCMetadataSection.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

using namespace llvm;
using namespace object;
using namespace objdump;

cl::opt<bool> TPCCompressStatus(
    "tpc-compress-info",
    cl::desc("Displays compression status of TPC instructions"));

cl::opt<std::string> TPCCompressStatusFile(
    "tpc-compress-info-file",
    cl::desc("Name of the output file for TPC compression status information (default is 'stdout')"),
    cl::ValueOptional, cl::init("-"));

cl::opt<bool> TPCLeadingAddr(
    "tpc-leading-addr",
    cl::desc("Print leading address"));

cl::opt<bool> TPCEncodingInfo(
    "tpc-encoding-info",
    cl::desc("Displays the encoding info for instructions"));

cl::opt<bool> IgnoreMovDT(
    "tpc-ignore-mov-dt",
    cl::desc("Do not print fields reflecting data type for MOV instructions"),
    cl::Hidden);

cl::opt<bool> TPCEncodingCover(
    "tpc-encoding-cover",
    cl::desc("Gathering the coverage info for instructions"));

cl::opt<std::string> TPCEncodingCoverFile(
    "tpc-encoding-cover-file",
    cl::desc("Name of file containing the binaries to cover"),
    cl::ValueRequired);

cl::opt<bool> ForAssembler(
    "tpc-for-assembler",
    cl::desc("Output disassembled text in form suitable for assembling"));

// Alias for ForAssembler, to facilitate transition.
static cl::opt<bool> NoCommonHeader(
    "no-common-header",
    cl::desc("Alias for --tpc-for-assembler"),
    cl::Hidden);


static TPCArch TpcArch = Goya;

static TPCArch MCPUToTPCArch() {
  if (MCPU.compare("dali") == 0 || MCPU.compare("goya") == 0)
    return Goya;
  else if (MCPU.compare("gaudi") == 0)
    return Gaudi;
  else if (MCPU.compare("gaudib") == 0)
    return Gaudib;
  else if (MCPU.compare("goya2") == 0 || MCPU.compare("greco") == 0)
    return Greco;
  else if (MCPU.compare("gaudi2") == 0)
    return Gaudi2;
  else if (MCPU.compare("doron1") == 0)
    return Doron1;
  else
    reportError("", "'" + MCPU + "' is not a recognized processor for this target");
}

bool isDali() { return TpcArch == Goya; }

bool isGaudi() { return TpcArch == Gaudi; }

bool isGaudib() { return TpcArch == Gaudib; }

bool isGreco() { return TpcArch == Greco; }

bool isGaudi2() { return TpcArch == Gaudi2; }

bool isDoron1() { return TpcArch == Doron1; }

bool skipTpcOperandsType(const std::vector<EncodingField> *enc,
                            std::string bins, size_t instr_size) {
  int8_t src = (isGreco() || isGaudi2()) ? SRCA_IND_G3 : SRCA_IND;
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

bool IsMovFlavorSet(const std::vector<EncodingField> *enc, std::string bins, size_t instr_size) {
  int8_t src = (isGreco() || isGaudi2()) ? SRCA_IND_G3 : SRCA_IND;
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

void llvm::printTpcEncodingInfo(const object::ObjectFile *O,
                                ArrayRef<uint8_t> instruction,
                                formatted_raw_ostream& FOS) {
  if (!isTPCTarget(O) || !TPCEncodingInfo)
    return;

  bool compressed = false;
  uint8_t part = 0xFF;
  assert(EncodingDict.find(TpcArch) != EncodingDict.end());
  const Encoding &E = EncodingDict.at(TpcArch);
  const EncodingLayout *enc = E.generic;
  if (E.comp1 != nullptr) {
    uint8_t v = instruction[0] & 0x3;
    compressed = hexDigitValue(hexdigit(v)) & 1;
    if (compressed) {
      part = hexDigitValue(hexdigit(v)) & 2;
      enc = (part == 0) ? E.comp1 : E.comp2;
    }
  }

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
  if (isGreco() || isGaudi2() || isDoron1()) {
    if (compressed) {
      if (part == 0) {
        spu_opcode_ind = SPU_OPCODE_IND_G3;
        vpu_opcode_ind = VPU_OPCODE_IND_G3;
      } else { //// part == 1
        ld_opcode_ind = LD_OPCODE_IND_G3_C;
      }
    } else {
      ld_opcode_ind  = LD_OPCODE_IND_G3;
      spu_opcode_ind = SPU_OPCODE_IND_G3;
      vpu_opcode_ind = VPU_OPCODE_IND_G3;
    }
  } else if (isDali() || isGaudi() || isGaudib()) {
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
      enc = E.loop;
  }

  bool ignoreMovDT = IgnoreMovDT && (ld_opcode == MOV_OPCODE_LD_SLOT || vpu_opcode == MOV_OPCODE_VPU_SLOT) && skipTpcOperandsType(enc, binaries, instr_size);
  FOS << "\t// ";
  uint8_t num = (*enc).size();
  for (const EncodingField rule : *enc) {
    StringRef fn = rule.field_name;
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

    FOS << fn << '=' << value << (--num == 0 ? '\n' : ',');
  }
}

static void printSectionEncodingInfo(const ObjectFile *Obj,
                                     const StringRef &Contents,
                                     formatted_raw_ostream& FOS) {
  ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(Contents.data()),
                          Contents.size());
  for (std::size_t addr = 0, end = Bytes.size() - 31; addr < end; addr += 32) {
    if (isGreco() || isGaudi2()) {
      bool compressed = hexDigitValue(hexdigit(Bytes[addr] & 0xF)) & 1;
      if (compressed) {
        printTpcEncodingInfo(Obj, Bytes.slice(addr, 16), FOS);      // part #1
        printTpcEncodingInfo(Obj, Bytes.slice(addr + 16, 16), FOS); // part #2
        continue;
      }
    }

    printTpcEncodingInfo(Obj, Bytes.slice(addr, 32), FOS);
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

void llvm::collectSymbolsTpc(
    const ObjectFile *Obj, const MCDisassembler &DisAsm, MCContext &Context,
    SectionSymbolsTy &Symbols) {

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
                [Address](const SymbolInfoTy& e) {
                    return e.Addr == Address;
                });
        if (it == Symbols.end()) {
          Symbols.emplace_back(Address, sbuf, ELF::STT_NOTYPE);
        }
      }
    }
  }
}

enum class Slots : uint8_t { Load, Scalar, Vector, Store };

void InstrCover(vector<InstrCoverage> *cover, uint8_t encoding, uint16_t opType, uint16_t switches) {
  int ind = 0;
  for (InstrCoverage item : *cover) {
    if (item.encoding == encoding) {
      if (opType != 0xFF) {
        uint16_t mask = ~(1 << opType);
        item.typesMask &= mask;
        //// TODO: switches mask processing needs to add.
      }

      InstrCoverage newItem(item.instrName, item.encoding, item.typesMask, item.switchesMask);
      newItem.cover = true;
      cover->at(ind) = newItem;
    }

    ind++;
  }
}

void InstrCover(TPCArch arch, uint8_t encoding, Slots slot, uint16_t opType, uint16_t switches) {
  std::vector<InstrCoverage> *cover = nullptr;
  if (arch == TPCArch::Goya) {
    if (slot == Slots::Load) {
      cover = &goyaLdSlot;
    } else if (slot == Slots::Scalar) {
      cover = &goyaSpuSlot;
    } else if (slot == Slots::Vector) {
      cover = &goyaVpuSlot;
    } else if (slot == Slots::Store) {
      cover = &goyaStSlot;
    }
  } else if (arch == TPCArch::Gaudi || arch == TPCArch::Gaudib) {
    if (slot == Slots::Load) {
      cover = &gaudiLdSlot;
    } else if (slot == Slots::Scalar) {
      cover = &gaudiSpuSlot;
    } else if (slot == Slots::Vector) {
      cover = &gaudiVpuSlot;
    } else if (slot == Slots::Store) {
      cover = &gaudiStSlot;
    }
  } else if (arch == TPCArch::Greco || arch == TPCArch::Gaudi2) {
    if (slot == Slots::Load) {
      cover = &goya2LdSlot;
    } else if (slot == Slots::Scalar) {
      cover = &goya2SpuSlot;
    } else if (slot == Slots::Vector) {
      cover = &goya2VpuSlot;
    } else if (slot == Slots::Store) {
      cover = &goya2StSlot;
    }
  } else {
    assert(false);
  }

  InstrCover(cover, encoding, opType, switches);
}

void TpcInstructionCover(ArrayRef<uint8_t> instruction, TPCArch arch) {
  bool compressed = false;
  uint8_t part = 0xFF;
  const EncodingLayout *enc = &dali_encoding;
  if (arch == TPCArch::Goya)
    enc = &dali_encoding;
  else if (arch == TPCArch::Gaudi || arch == TPCArch::Gaudib)
    enc = &gaudi_encoding;
  else if (arch == TPCArch::Greco || arch == TPCArch::Gaudi2 || arch == TPCArch::Doron1) {
    compressed = hexDigitValue(hexdigit(instruction[0] & 0xF)) & 1;
    if (!compressed)
      switch (arch) {
      case TPCArch::Greco:
        enc = &uncompress_goya2_encoding;
        break;
      case TPCArch::Gaudi2:
        enc = &uncompress_gaudi2_encoding;
        break;
      case TPCArch::Doron1:
        enc = &uncompress_doron1_encoding;
        break;
      default:
        break;
      }
    else {
      uint8_t type = hexDigitValue(hexdigit(instruction[1] & 0xF)) & 1;
      switch (arch) {
      case TPCArch::Greco:
        type == 0 ? enc = &compress0_goya2_encoding  : enc = &compress1_goya2_encoding;
        break;
      case TPCArch::Gaudi2:
        type == 0 ? enc = &compress0_gaudi2_encoding : enc = &compress1_gaudi2_encoding;
        break;
      case TPCArch::Doron1:
        type == 0 ? enc = &compress0_doron1_encoding : enc = &compress1_doron1_encoding;
        break;
      default:
        break;
      }
    }
  }

  unsigned long long ld_opcode  = 0xFF;
  unsigned long long spu_opcode = 0xFF;
  unsigned long long vpu_opcode = 0xFF;
  unsigned long long st_opcode  = 0xFF;
  unsigned long long spu_operand_type = 0xFF;
  unsigned long long vpu_operand_type = 0xFF;
  uint8_t ld_switches  = 0;
  uint8_t spu_switches = 0;
  uint8_t vpu_switches = 0;
  uint8_t st_switches  = 0;

  uint8_t start_bit;
  size_t instr_size = instruction.size() << 3;

  int8_t spu_opcode_ind = -1;

  if (arch == TPCArch::Greco) {
    if (compressed && part == 0) {
      spu_opcode_ind = SPU_OPCODE_IND_G3;
    }
  } else if (isDali() || isGaudi()) {
    spu_opcode_ind = SPU_OPCODE_IND;
  } else
    assert(false);

  std::string binaries;
  for (size_t i = 0; i < instruction.size(); i++) {
    std::bitset<8> bs(instruction[i]);
    binaries.insert(0, bs.to_string());
  }

  if (spu_opcode_ind >= 0) {
    start_bit = instr_size - (*enc)[spu_opcode_ind].start_bit - (*enc)[spu_opcode_ind].field_size;
    spu_opcode = std::stoull(binaries.substr(start_bit, (*enc)[spu_opcode_ind].field_size), 0, 2);
  }

  if (spu_opcode == LOOP_OPCODE) //// for LOOP instruction a special encoding to be used.
    enc = (arch == TPCArch::Greco) ? &goya2_loop_encoding : &loop_encoding;

  for (const EncodingField rule : *enc) {
    std::string fn = rule.field_name;
    start_bit = instr_size - rule.start_bit - rule.field_size;
    unsigned long long value = std::stoull(binaries.substr(start_bit, rule.field_size), 0, 2);
    if (fn == "LOAD_OPCODE")
      ld_opcode = value;
    else if (fn == "SPU_OPCODE")
      spu_opcode = value;
    else if (fn == "VPU_OPCODE")
      vpu_opcode = value;
    else if (fn == "STORE_OPCODE")
      st_opcode = value;
    else if (fn == "SPU_OPERANDS_TYPE")
      spu_operand_type = value;
    else if (fn == "VPU_OPERANDS_TYPE")
      vpu_operand_type = value;
    else if (fn == "LOAD_SWITCHES")
      ld_switches = value;
    else if (fn == "SPU_SWITCHES")
      spu_switches = value;
    else if (fn == "VPU_SWITCHES")
      vpu_switches = value;
    else if (fn == "STORE_SWITCHES")
      st_switches = value;
    else
      continue;
  }

  InstrCover(arch, ld_opcode, Slots::Load, 0xFF, ld_switches);
  InstrCover(arch, spu_opcode, Slots::Scalar, spu_operand_type, spu_switches);
  InstrCover(arch, vpu_opcode, Slots::Vector, vpu_operand_type, vpu_switches);
  InstrCover(arch, st_opcode, Slots::Store, 0xFF, st_switches);
}

void TpcObjectCover(const SectionRef &Section, TPCArch arch) {
  Expected<StringRef> Res = Section.getContents();
  if (Res.takeError()) reportError("", "Could not read the ELF section");
  StringRef Contents = StringRef(reinterpret_cast<const char *>(Res->data()), Res->size());

  ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(Contents.data()), Contents.size());
  for (std::size_t addr = 0, end = Bytes.size() - 31; addr < end; addr += 32) {
    if (arch == TPCArch::Greco) {
      bool compressed = hexDigitValue(hexdigit(Bytes[addr] & 0xF)) & 1;
      if (compressed) {
        TpcInstructionCover(Bytes.slice(addr, 16), arch);      // part #1
        TpcInstructionCover(Bytes.slice(addr + 16, 16), arch); // part #2
        continue;
      }
    }

    TpcInstructionCover(Bytes.slice(addr, 32), arch);
  }
}

void TpcObjectCover(const object::ObjectFile *objFile, TPCArch arch) {
  assert(objFile != nullptr);
  for (SectionRef Section : objFile->sections())
    if (!Section.isText() || !Section.getSize()) continue;
    else TpcObjectCover(Section, arch);
}

void DisplaySlotCoverageInfo(TPCArch arch, std::string message, const std::vector<InstrCoverage> *cover) {
  outs() << "----------------------------\n";
  outs() << message << " " << " slot opCodes:\n";
  outs() << "----------------------------\n";
  for (InstrCoverage item : *cover) {
    if (!item.cover) {
      outs() << item.instrName << ", ";
    }
  }
  outs() << "\n\n";
  outs() << "============================\n";
  outs() << message << " " << " slot opTypes:\n";
  outs() << "============================\n";
  for (InstrCoverage item : *cover) {
    uint16_t mask = item.typesMask;
    if (item.cover && mask != 0) {
      string types = "";
      if (arch == TPCArch::Goya) {
        if ((mask & (uint16_t)GoyaTypes::FP32)) {
          types += "FP32, ";
        }
        if ((mask & (uint16_t)GoyaTypes::FP16)) {
          types += "FP16, ";
        }
        if ((mask & (uint16_t)GoyaTypes::INT32)) {
          types += "INT32, ";
        }
        if ((mask & (uint16_t)GoyaTypes::UINT32)) {
          types += "UINT32, ";
        }
        if ((mask & (uint16_t)GoyaTypes::INT16)) {
          types += "INT16, ";
        }
        if ((mask & (uint16_t)GoyaTypes::UINT16)) {
          types += "UINT16, ";
        }
        if ((mask & (uint16_t)GoyaTypes::INT8)) {
          types += "INT8, ";
        }
        if ((mask & (uint16_t)GoyaTypes::UINT8)) {
          types += "UINT8, ";
        }
        if ((mask & (uint16_t)GoyaTypes::BOOL)) {
          types += "BOOL, ";
        }
      } else if (arch == TPCArch::Gaudi || arch == TPCArch::Gaudib) { //FIXME FP16 case
        if ((mask & (uint16_t)GaudiTypes::FP32)) {
          types += "FP32, ";
        }
        if ((mask & (uint16_t)GaudiTypes::BF16)) {
          types += "BF16, ";
        }
        if ((mask & (uint16_t)GaudiTypes::INT32)) {
          types += "INT32, ";
        }
        if ((mask & (uint16_t)GaudiTypes::UINT32)) {
          types += "UINT32, ";
        }
        if ((mask & (uint16_t)GaudiTypes::INT16)) {
          types += "INT16, ";
        }
        if ((mask & (uint16_t)GaudiTypes::UINT16)) {
          types += "UINT16, ";
        }
        if ((mask & (uint16_t)GaudiTypes::INT8)) {
          types += "INT8, ";
        }
        if ((mask & (uint16_t)GaudiTypes::UINT8)) {
          types += "UINT8, ";
        }
        if ((mask & (uint16_t)GaudiTypes::BOOL)) {
          types += "BOOL, ";
        }
      } else if (arch == TPCArch::Greco) {
        if ((mask & (uint16_t)Goya2Types::FP32)) {
          types += "FP32, ";
        }
        if ((mask & (uint16_t)Goya2Types::BF16)) {
          types += "BF16, ";
        }
        if ((mask & (uint16_t)Goya2Types::FP16)) {
          types += "FP16, ";
        }
        if ((mask & (uint16_t)Goya2Types::INT32)) {
          types += "INT32, ";
        }
        if ((mask & (uint16_t)Goya2Types::UINT32)) {
          types += "UINT32, ";
        }
        if ((mask & (uint16_t)Goya2Types::INT16)) {
          types += "INT16, ";
        }
        if ((mask & (uint16_t)Goya2Types::UINT16)) {
          types += "UINT16, ";
        }
        if ((mask & (uint16_t)Goya2Types::INT8)) {
          types += "INT8, ";
        }
        if ((mask & (uint16_t)Goya2Types::UINT8)) {
          types += "UINT8, ";
        }
        if ((mask & (uint16_t)Goya2Types::INT4)) {
          types += "INT4, ";
        }
        if ((mask & (uint16_t)Goya2Types::UINT4)) {
          types += "UINT4, ";
        }
        if ((mask & (uint16_t)Goya2Types::BOOL)) {
          types += "BOOL, ";
        }
      }

      if (types != "") {
        string name = item.instrName;
        if (name.size() > 3)
          outs() << item.instrName << ": \t\t" << types << "\n";
        else
          outs() << item.instrName << ": \t\t\t" << types << "\n";
      }
    }
  }

  outs() << "\n\n";
}

void DisplayCoverageInfo(TPCArch arch) {
  const std::vector<InstrCoverage> *loadSlotInfo   = nullptr;
  const std::vector<InstrCoverage> *scalarSlotInfo = nullptr;
  const std::vector<InstrCoverage> *vectorSlotInfo = nullptr;
  const std::vector<InstrCoverage> *storeSlotInfo  = nullptr;
  if (arch == TPCArch::Goya) {
    loadSlotInfo   = &goyaLdSlot;
    scalarSlotInfo = &goyaSpuSlot;
    vectorSlotInfo = &goyaVpuSlot;
    storeSlotInfo  = &goyaStSlot;
  } else if (arch == TPCArch::Gaudi || arch == TPCArch::Gaudib) {
    loadSlotInfo   = &gaudiLdSlot;
    scalarSlotInfo = &gaudiSpuSlot;
    vectorSlotInfo = &gaudiVpuSlot;
    storeSlotInfo  = &gaudiStSlot;
  } else if (arch == TPCArch::Greco) {
    loadSlotInfo   = &goya2LdSlot;
    scalarSlotInfo = &goya2SpuSlot;
    vectorSlotInfo = &goya2VpuSlot;
    storeSlotInfo  = &goya2StSlot;
  } else if (arch == TPCArch::Gaudi2) {
  } else if (arch == TPCArch::Doron1) {
  }

  if (loadSlotInfo != nullptr) {
    DisplaySlotCoverageInfo(arch, "Unused Load  ", loadSlotInfo);
  }

  if (scalarSlotInfo != nullptr) {
    DisplaySlotCoverageInfo(arch, "Unused Scalar", scalarSlotInfo);
  }

  if (vectorSlotInfo != nullptr) {
    DisplaySlotCoverageInfo(arch, "Unused Vector", vectorSlotInfo);
  }

  if (storeSlotInfo != nullptr) {
    DisplaySlotCoverageInfo(arch, "Unused Store ", storeSlotInfo);
  }
}

bool llvm::TpcEncodingCover(StringRef Triple) {
  if (TripleName != "tpc" || !TPCEncodingCover || TPCEncodingCoverFile.empty())
    return false;

  TPCArch arch;
  if (isDali()) {
    arch = TPCArch::Goya;
  } else if (isGaudi()) {
    arch = TPCArch::Gaudi;
  } else if (isGreco()) {
    arch = TPCArch::Greco;
  } else if (isGaudi2()) {
    arch = TPCArch::Gaudi2;
  } else if (isDoron1()) {
    arch = TPCArch::Doron1;
  } else {
    outs() << "Could not distinguish the architecture.";
    return true;
  }

  ifstream stream(TPCEncodingCoverFile);
  if (!stream) {
    outs() << "Could not open file '" + TPCEncodingCoverFile + "'" << "\n";
    return true;
  }

  outs() << "Please wait a little until the tool finishes to work ...";
  std::ifstream objsList(TPCEncodingCoverFile);
  std::string file;
  int cnt = 0;
  bool completed = false;
  while (std::getline(objsList, file)) {
    Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(file);
    if (!BinaryOrErr) {
      completed = false;
      stream.close();
      reportError(file, "Invalid binary file");
    }

    Binary &Binary = *BinaryOrErr.get().getBinary();
    if (ObjectFile *obj = dyn_cast<ObjectFile>(&Binary)) {
      TpcObjectCover(obj, arch);
    } else {
      completed = false;
      stream.close();
      reportError(file, "Invalid file type");
    }

    if (cnt == 1) {
      cnt = 0;
      outs() << "\b/";
    } else {
      cnt = 1;
      outs() << "\b\\";
    }

    completed = true;
  }

  if (completed) {
    std::cout << "\b Done!\n";
    DisplayCoverageInfo(arch);
  } else {
    std::cout << "Could not read whole objects list.\n";
  }

  stream.close();
  return true;
}

void PrintTPCMetadata(const TPCMetadataSection &Header) {
  for (const TPCMetadataFieldInfo &FI : TPCMetadataFieldsInfo) {
    if (FI.IsPlaceholder || FI.startWithVersion > Header.version)
      continue;

    const std::uint64_t Value = FI.GetValueFun(Header);
    const char *const TD = TPCMetadataTypeDirectives.at(FI.elementSize);

    if (FI.isArray()) {
      assert(FI.elementSize == 1);
      for (unsigned I = 0; I < FI.length; ++I) {
        if (Value & (1U << I))
          outs() << '\t' << FI.fieldName << '[' << I << "]: " << TD << " 1\n";
      }
      continue;
    }

    outs() << '\t' << FI.fieldName << ": " << TD << ' ' << Value << '\n';
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
          else
            return false;
        }
  );

  return Iter;
}



void llvm::calculateMCPU(const ObjectFile *ObjFile) {
  if (!isTPCTarget(ObjFile))
    return;

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
  else
    llvm_unreachable("Unknown TPC metadata section");

  if (!MCPU.empty()) {
    if (TpcArch != Metadata.march)
      reportWarning("Specified '-mcpu' value contradicts architecture encoded in tpc_metadata section.",
                    MetadataSection->getObject()->getFileName());

  } else {
    switch ((uint16_t)Metadata.march) {
    case 1:
      TpcArch = Goya;
      break;
    case 2:
      TpcArch = Gaudi;
      break;
    case 3:
      TpcArch = Greco;
      break;
    case 4:
      TpcArch = Gaudi2;
      break;
    case 5:
      TpcArch = Gaudib;
      break;
    case 6:
      TpcArch = Doron1;
      break;
    default:
      llvm_unreachable("Unhandled tpc arch");
    }
  }
}

std::string llvm::getTPCCPUName(const object::ObjectFile *ObjFile, std::string MCPU) {
  if (!isTPCTarget(ObjFile))
    return MCPU;
  switch ((int)TpcArch) {
  case Goya:
    return "goya";
  case Gaudi:
    return "gaudi";
  case Greco:
    return "greco";
  case Gaudi2:
    return "gaudi2";
  case Doron1:
    return "doron1";
  case Gaudib:
    return "gaudib";
  default:
    llvm_unreachable("Unhandled CPU name");
  }
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

void llvm::printTPCMetadataFromBinary(const SectionRef& Section) {
  Expected<StringRef> SectionNameOrError = Section.getName();
  assert(SectionNameOrError &&
         SectionNameOrError->equals(BinaryTPCMetadataSectionName));

  TPCMetadataSection Header = getMetadataFromBinarySection(Section);

  outs() << '\n';
  outs() << formatv(".section {0}\n", *SectionNameOrError);
  PrintTPCMetadata(Header);
}

static void printSectionASCII(const StringRef &Name, const StringRef &Content) {
  outs() << '\n';
  outs() << formatv(".section {0}\n", Name);
  outs() << Content;
  outs() << '\n';
}

bool llvm::getTPCData(const SectionRef &Section,
                      bool &IsIndexMap, bool &IsBinaryTPCMetadata) {
  IsIndexMap = IsBinaryTPCMetadata = false;
  const ObjectFile *Obj = Section.getObject();
  if (!isTPCTarget(Obj) || !Obj->isELF())
    return true;

  Expected<StringRef> SectionNameOrError = Section.getName();
  if (!SectionNameOrError) {
    consumeError(SectionNameOrError.takeError());
    return false;
  }

  StringRef SectionName = *SectionNameOrError;
  IsIndexMap = SectionName == ".IndexMap";
  IsBinaryTPCMetadata = SectionName == BinaryTPCMetadataSectionName;
  return true;
}


static void printTpcEncodingInfoFile(const ObjectFile *Obj) {
  assert(Obj != nullptr);
  formatted_raw_ostream FOS(outs());
  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    if (!Section.isText())
      continue;
    if (!Section.getSize())
      continue;
    printSectionEncodingInfo(Obj, unwrapOrError(Section.getContents(), Obj->getFileName()), FOS);
  }
}

void llvm::printTPCTables(const ObjectFile *Obj, bool Disassemble) {
  if (!isTPCTarget(Obj))
    return;
  if (TPCCompressStatus)
    printTpcCompressStatus(Obj);
  if (TPCEncodingInfo && !Disassemble) {
    calculateMCPU(Obj);
    printTpcEncodingInfoFile(Obj);
  }
}

bool llvm::printTPCArch(const object::ObjectFile *Obj) {
  if (!isTPCTarget(Obj))
    return false;
  bool ToPrint = !ForAssembler && !NoCommonHeader;
  if (ToPrint)
    outs() << "\nTPC architecture: " << getTPCCPUName(Obj, MCPU) << "\n";
  return ToPrint;
}

bool llvm::tryDisassembleTPCSection(const object::SectionRef &Section) {
  const ObjectFile *Obj = Section.getObject();
  if (Section.getSize() == 0 || !Obj->isELF() || !isTPCTarget(Obj))
    return false;

  Expected<StringRef> SectionNameOrError = Section.getName();
  if (!SectionNameOrError) {
    consumeError(SectionNameOrError.takeError());
    return false;
  }
  if (*SectionNameOrError == BinaryTPCMetadataSectionName) {
    printTPCMetadataFromBinary(Section);
    return true;
  }
  if (*SectionNameOrError == ".IndexMap" || 
      *SectionNameOrError == ".UnrollInfo" ||
      *SectionNameOrError == ".BailoutGCCUSTOM" ||
      *SectionNameOrError == ".KernelInfo" ||
      *SectionNameOrError == ".tpc_compiler") {
        if (ForAssembler || NoCommonHeader) {
          // TODO support in assembler
          return true;
        }
    Expected<StringRef> SectionContentOrError = Section.getContents();
    if (!SectionContentOrError) {
      consumeError(SectionContentOrError.takeError());
      return false;
    }
    //outs() << "\nDisassembly of section " << *SectionNameOrError << ":\n";

    printSectionASCII(*SectionNameOrError, *SectionContentOrError);
    return true;
  }

  return false;
}

bool llvm::printAsTPCSymbol(const SectionRef &Section, SymbolInfoTy Symbol) {
  const ObjectFile *Obj = Section.getObject();
  if (!isTPCTarget(Obj))
    return false;

  StringRef SymbolName = Symbol.Name;
  StringRef SectionName = unwrapOrError(Section.getName(), Obj->getFileName());
  uint64_t SectionAddr = Section.getAddress();

  return !(SymbolName == SectionName && Symbol.Addr == SectionAddr && (ForAssembler || NoCommonHeader));
}

bool llvm::hasTPCActionOptions() {
  return TPCCompressStatus || TPCEncodingInfo || TPCEncodingCover;
}

bool llvm::printCommonHeader(const object::ObjectFile *Obj) {
  if (!isTPCTarget(Obj))
    return true;
  return !ForAssembler && !NoCommonHeader;
}
