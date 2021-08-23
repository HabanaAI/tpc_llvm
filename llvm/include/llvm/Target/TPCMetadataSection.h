//===- llvm/Target/TPCProgramHeader.h - TPC Specific header -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a TPC specific data. This info usages for write
// .TPC_METADATA section.
//
//===----------------------------------------------------------------------===//

#ifndef TPC_METADATA_SECTION_H
#define TPC_METADATA_SECTION_H

#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Support/CommandLine.h"
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {

extern cl::opt<bool> TPCElfMemcpy;

static const char *StringTPCMetadataSectionName = ".TPC_METADATA";
static const char *BinaryTPCMetadataSectionName = ".tpc_metadata";

static const unsigned TPCMetadataSectionSize = 262;
static const unsigned TPCNumTensors = 16;
static const unsigned TPCReservedSize = 232;
static const unsigned TPCReservedSize1 = 12;
static const unsigned TPCMetadataVersion = 9;
static const unsigned TPCLastMarch = 5;

static const char *TPCVersionName = "version";
static const char *TPCSpecialFunctionUsedName = "specialFunctionUsed";
static const char *TPCPrintfUsedName = "printfUsed";
static const char *TPCLockUnLockName = "lockUnLock";
static const char *TPCScalarLdName = "scalarLd";
static const char *TPCRMWStoreName = "rmwStore";
static const char *TPCMarchName = "march";
static const char *TPCMMIOName = "mmioUsed";
static const char *TPCParamsNumName = "paramsNum";
static const char *TPCPrintfTensorIDName = "printfTensorID";
static const char *TPCBubbleName = "TPCBubble";

static const char *TPCUnhandledMetadataField = "Unhandled tpc metadata field";

struct TPCMetadataSection {
  uint32_t    version = TPCMetadataVersion;
  bool        specialFunctionUsed = false;
  bool        printfUsed = false;
  bool        lockUnLock = false;
  bool        mmioUsed = false;
  uint16_t    march = 1; // Dali
  uint8_t     paramsNum = 0;
  uint8_t     printfTensorID = 0;
  uint8_t     reservedTemp[TPCReservedSize1];
  bool        scalarLd[TPCNumTensors] = {false};
  bool        rmwStore[TPCNumTensors] = {false};
  // Do not use these fields:
  uint8_t     reserved[TPCReservedSize];
};

std::vector<uint8_t>
bianrySerializeTPCProgramHeader(const TPCMetadataSection &Header);
TPCMetadataSection binaryDeserializeTPCProgramHeader(
    const std::vector<uint8_t> &Data);

TPCMetadataSection stringDeserializeTPCProgramHeader(const StringRef& String);

static const std::unordered_map<unsigned, const char*>
TPCMetadataTypeDirectives = {
  {1, "DB"},
  {2, "DW"},
  {4, "DD"},
  {8, "DQ"},
  {10, "DT"},
  {16, "DH"}
};

struct TPCMetadataFieldInfo {
  const char  *fieldName;
  unsigned    elementSize;
  unsigned    length;
  unsigned    minValue;
  unsigned    maxValie;
  uint8_t     startWithVersion;
  unsigned    offset;

  bool isArray() const {return length > 1;}

  static TPCMetadataFieldInfo CreateBoolFieldInfo(const char *FieldName,
                                                  uint8_t StartWithVersion, unsigned Offset) {
    return TPCMetadataFieldInfo{FieldName, 1, 1, 0, 1, StartWithVersion, Offset};
  }
};

static const TPCMetadataFieldInfo TPCMetadataFieldsInfo[] = {
  TPCMetadataFieldInfo{TPCVersionName, 4, 1, 0, TPCMetadataVersion, 3, 0},
  TPCMetadataFieldInfo::CreateBoolFieldInfo(TPCSpecialFunctionUsedName, 3, 4),
  TPCMetadataFieldInfo::CreateBoolFieldInfo(TPCPrintfUsedName, 3, 5),
  TPCMetadataFieldInfo::CreateBoolFieldInfo(TPCLockUnLockName, 3,6),
  TPCMetadataFieldInfo{TPCScalarLdName, 1, TPCNumTensors, 0, 1, 3, 26},
  TPCMetadataFieldInfo{TPCRMWStoreName, 1, TPCNumTensors, 0, 1, 4, 42},
  TPCMetadataFieldInfo{TPCMarchName, 2, 1, 1, TPCLastMarch, 5, 8},
  TPCMetadataFieldInfo{TPCBubbleName, 2, 1, 1, TPCLastMarch, 8, 10},           //Pading the the TPCMarch with more 2 bytes to align to elf_api
  TPCMetadataFieldInfo::CreateBoolFieldInfo(TPCMMIOName, 6, 7),
  TPCMetadataFieldInfo{TPCParamsNumName, 1, 1, 0,
                       std::numeric_limits<uint8_t>::max(), 8, 12},
  TPCMetadataFieldInfo{TPCPrintfTensorIDName, 1, 1, 0, TPCNumTensors, 9, 13}
};

bool setTpcMetadataValue(int64_t Value,
                         const TPCMetadataFieldInfo &FieldInfo,
                         TPCMetadataSection &Result,
                         std::string &ErrorMessage);

bool setTpcMetadataArrayValue(int64_t Value,
                              unsigned Index,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage);

bool setTpcMetadataArrayValue(const StringRef &Value,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage);
}

#endif // TPC_METADATA_SECTION_H
