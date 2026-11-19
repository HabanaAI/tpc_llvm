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

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {

constexpr const char BinaryTPCMetadataSectionName[] = ".tpc_metadata";

constexpr unsigned TPCNumTensors = 16;
constexpr unsigned TPCReservedSize = 213;
constexpr unsigned TPCReservedSize1 = 4;
constexpr unsigned TPCMetadataVersion = 20;

constexpr const char TPCMarchName[] = "march";

// Size of the binary .tpc_metadata section.
constexpr unsigned TPCMetadataSectionSize = 262;

struct TPCMetadataSection {
  uint32_t    version = TPCMetadataVersion;
  bool        specialFunctionUsed = false;
  bool        printfUsed = false;
  bool        lockUnLock = false;
  bool        mmioUsed = false;
  uint16_t    march = 1; // Dali
  uint8_t     paramsNum = 0;
  uint8_t     printfTensorID = 0;
  uint8_t     numberOfThreads = 1;
  uint8_t     directMMIOAccess = 0;
  bool        targetSpecific = false;
  bool        dnorm = false;
  uint32_t    hashedISA = 0;
  uint8_t     reservedTemp[TPCReservedSize1];
  bool        scalarLd[TPCNumTensors] = {false};
  bool        rmwStore[TPCNumTensors] = {false};
  uint64_t    duplicateTensors = 0;
  bool        genAddrTensorMask[TPCNumTensors] = {false};
  bool        regIDUsed = false;
  bool        irf32Mode = true;
  bool        partialStore[TPCNumTensors] = {false};
  bool        exclusiveSwitch[TPCNumTensors] = {false};
  bool        unsetSmallVLM = false;
  bool        scalarSt[TPCNumTensors] = {false};
  // Do not use these fields:
  uint8_t     reserved[TPCReservedSize];
};

std::vector<uint8_t>
binarySerializeTPCProgramHeader(const TPCMetadataSection &Header);
TPCMetadataSection binaryDeserializeTPCProgramHeader(
    const std::vector<uint8_t> &Data);

extern const std::unordered_map<unsigned, const char *>
    TPCMetadataTypeDirectives;

struct TPCMetadataFieldInfo {
  const char  *fieldName;
  unsigned    elementSize;
  unsigned    length;
  uint64_t    minValue;
  uint64_t    maxValue;
  uint8_t     startWithVersion;
  unsigned    offset;

  /// Convert actual field value into U64.
  std::function<std::uint64_t(const TPCMetadataSection&)> GetValueFun;

  /// Set actual field value from U64.
  std::function<void(TPCMetadataSection&, std::uint64_t)> SetValueFun;

  /// Size of binary field representation in bytes.
  unsigned BinSizeBytes;

  /// Whether field is a reserved area which should be always zero-filled.
  bool IsPlaceholder;

  /// The per-tensor mask behind an array field, null for other fields.
  bool (TPCMetadataSection::*TensorMask)[TPCNumTensors];

  bool isArray() const {return length > 1;}
};

extern const std::array<TPCMetadataFieldInfo, 26> TPCMetadataFieldsInfo;

const TPCMetadataFieldInfo *getTPCMetadataFieldInfo(const StringRef FieldName);

bool setTpcMetadataValue(uint64_t Value,
                         const TPCMetadataFieldInfo &FieldInfo,
                         TPCMetadataSection &Result,
                         std::string &ErrorMessage);

bool setTpcMetadataArrayValue(uint64_t Value,
                              unsigned Index,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage);

bool setTpcMetadataArrayValue(const StringRef &Value,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage);
} // namespace llvm

#endif // TPC_METADATA_SECTION_H
