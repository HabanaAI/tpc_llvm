//===- TPCMetadataSection.cpp - TPC Specific header -------------*- C++ -*-===//
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

#include "llvm/Support/FormatVariadic.h"
#include "llvm/Target/TPCMetadataSection.h"

#include <cstring>

using namespace llvm;

static constexpr unsigned TPCLastMarch = 6; // Doron1

template <typename FieldT>
static TPCMetadataFieldInfo
makeScalarFI(const char *FieldName, const unsigned ElementSize,
             const uint64_t MinValue, const uint64_t MaxValue,
             const uint8_t StartWithVersion, const unsigned Offset,
             FieldT TPCMetadataSection::*Field) {
  return TPCMetadataFieldInfo{
      FieldName,
      ElementSize,
      1,
      MinValue,
      MaxValue,
      StartWithVersion,
      Offset,
      [Field](const TPCMetadataSection &MD) -> std::uint64_t {
        return MD.*Field;
      },
      [Field](TPCMetadataSection &MD, std::uint64_t Val) {
        MD.*Field = static_cast<FieldT>(Val);
      },
      ElementSize,
      false,
      nullptr};
}

static TPCMetadataFieldInfo makePlaceholderFI(const char *FieldName,
                                              const unsigned Offset,
                                              const unsigned BinSize,
                                              const uint8_t StartWithVersion) {
  return TPCMetadataFieldInfo{
      FieldName,
      1,
      1,
      0,
      0,
      StartWithVersion,
      Offset,
      [](const TPCMetadataSection &MD) -> std::uint64_t { return 0; },
      [](TPCMetadataSection &MD, std::uint64_t Val) {},
      BinSize,
      true,
      nullptr};
}

static std::uint64_t convertTensorMaskToU64(const bool Mask[],
                                            const unsigned Count) {
  std::uint64_t Val = 0;
  for (unsigned I = 0; I < Count; ++I) {
    if (Mask[I])
      Val |= 1LU << I;
  }
  return Val;
}

static void convertU64ToTensorMask(const std::uint64_t Value, bool *Mask,
                                   const unsigned Count) {
  for (unsigned I = 0; I < Count; ++I)
    Mask[I] = (Value >> I) & 1;
}

static TPCMetadataFieldInfo
makeTensorMaskFI(const char *FieldName, const uint8_t StartWithVersion,
                 const unsigned Offset,
                 bool (TPCMetadataSection::*Mask)[TPCNumTensors]) {
  return TPCMetadataFieldInfo{
      FieldName,
      1,
      TPCNumTensors,
      0,
      1,
      StartWithVersion,
      Offset,
      [Mask](const TPCMetadataSection &MD) -> std::uint64_t {
        return convertTensorMaskToU64(MD.*Mask, TPCNumTensors);
      },
      [Mask](TPCMetadataSection &MD, std::uint64_t Value) {
        convertU64ToTensorMask(Value, MD.*Mask, TPCNumTensors);
      },
      2,
      false,
      Mask};
}

namespace llvm {

// The layout of version 20, as read by the SynapseAI 1.24 tools.
const std::array<TPCMetadataFieldInfo, 26> TPCMetadataFieldsInfo = {
    makeScalarFI("version", 4, 0, TPCMetadataVersion, 3, 0,
                 &TPCMetadataSection::version),
    makeScalarFI("specialFunctionUsed", 1, 0, 1, 3, 4,
                 &TPCMetadataSection::specialFunctionUsed),
    makeScalarFI("printfUsed", 1, 0, 1, 3, 5, &TPCMetadataSection::printfUsed),
    makeScalarFI("lockUnLock", 1, 0, 1, 3, 6, &TPCMetadataSection::lockUnLock),
    makeScalarFI("mmioUsed", 1, 0, 1, 6, 7, &TPCMetadataSection::mmioUsed),
    makeScalarFI(TPCMarchName, 2, 1, TPCLastMarch, 5, 8,
                 &TPCMetadataSection::march),
    // Padding the TPCMarch with more than 2 bytes to align to elf_api
    makePlaceholderFI("TPCBubble", 10, 2, 8),
    makeScalarFI("paramsNum", 1, 0, std::numeric_limits<uint8_t>::max(), 8, 12,
                 &TPCMetadataSection::paramsNum),
    makeScalarFI("printfTensorID", 1, 0, TPCNumTensors, 9, 13,
                 &TPCMetadataSection::printfTensorID),
    makeScalarFI("numberOfThreads", 1, 1, 4, 10, 14,
                 &TPCMetadataSection::numberOfThreads),
    makeScalarFI("directMMIOAccess", 1, 0, 1, 11, 15,
                 &TPCMetadataSection::directMMIOAccess),
    makeScalarFI("targetSpecific", 1, 0, 1, 13, 16,
                 &TPCMetadataSection::targetSpecific),
    makeScalarFI("dnorm", 1, 0, 1, 12, 17, &TPCMetadataSection::dnorm),
    makeScalarFI("hashedISA", 4, 0, std::numeric_limits<uint32_t>::max(), 14,
                 18, &TPCMetadataSection::hashedISA),
    makePlaceholderFI("reserveMidArea", 22, TPCReservedSize1, 3),
    makeTensorMaskFI("scalarLd", 3, 26, &TPCMetadataSection::scalarLd),
    makeTensorMaskFI("rmwStore", 4, 28, &TPCMetadataSection::rmwStore),
    makeScalarFI("duplicateTensors", 8, 0, std::numeric_limits<uint64_t>::max(),
                 15, 30, &TPCMetadataSection::duplicateTensors),
    makeTensorMaskFI("genAddrTensorMask", 15, 38,
                     &TPCMetadataSection::genAddrTensorMask),
    makeScalarFI("regIDUsed", 1, 0, 1, 15, 40, &TPCMetadataSection::regIDUsed),
    makeScalarFI("irf32Mode", 1, 0, 1, 16, 41, &TPCMetadataSection::irf32Mode),
    makeTensorMaskFI("partialStore", 17, 42, &TPCMetadataSection::partialStore),
    makeTensorMaskFI("exclusiveSwitch", 18, 44,
                     &TPCMetadataSection::exclusiveSwitch),
    makeScalarFI("unsetSmallVLM", 1, 0, 1, 19, 46,
                 &TPCMetadataSection::unsetSmallVLM),
    makeTensorMaskFI("scalarSt", 20, 47, &TPCMetadataSection::scalarSt),
    makePlaceholderFI("reserveEndArea", 49, TPCReservedSize, 3),
};
} // namespace llvm

#ifndef NDEBUG
static void sanityCheckFieldsInfoLayout() {
  unsigned TotalHeaderSize = 0;
  for (unsigned I = 0; I < TPCMetadataFieldsInfo.size(); ++I) {
    assert(TotalHeaderSize == TPCMetadataFieldsInfo[I].offset);
    TotalHeaderSize += TPCMetadataFieldsInfo[I].BinSizeBytes;
  }
  assert(TotalHeaderSize == TPCMetadataSectionSize);
}
#endif

static std::uint64_t fromUint8Array(const uint8_t *Array, const unsigned NumBytes) {
  assert(NumBytes <= 8);
  std::uint64_t Result = 0;
  for (unsigned I = 0; I < NumBytes; ++I)
    Result |= static_cast<std::uint64_t>(*(Array++)) << (8 * I);
  return Result;
}

static void toUint8Array(const std::uint64_t Number, const int Length,
                         uint8_t *const Ptr) {
  assert(Length <= 8);
  for (int I = 0; I < Length; ++I)
    *(Ptr + I) = static_cast<uint8_t>((Number >> (8 * I)) & 0xFF);
}

namespace llvm {
const std::unordered_map<unsigned, const char *> TPCMetadataTypeDirectives = {
    {1, "DB"}, {2, "DW"}, {4, "DD"}, {8, "DQ"}, {10, "DT"}, {16, "DH"},
};
} // namespace llvm

std::vector<uint8_t>
llvm::binarySerializeTPCProgramHeader(const llvm::TPCMetadataSection &Header) {
  assert(Header.version <= TPCMetadataVersion);

#ifndef NDEBUG
  sanityCheckFieldsInfoLayout();
#endif

  std::vector<uint8_t> Data(TPCMetadataSectionSize, 0);
  for (const TPCMetadataFieldInfo &FI : TPCMetadataFieldsInfo) {
    if (FI.IsPlaceholder || Header.version < FI.startWithVersion)
      continue;

    toUint8Array(FI.GetValueFun(Header), FI.BinSizeBytes,
                 Data.data() + FI.offset);
  }
  return Data;
}

TPCMetadataSection llvm::binaryDeserializeTPCProgramHeader(
    const std::vector<uint8_t> &Data) {
  assert(Data.size() == TPCMetadataSectionSize);

#ifndef NDEBUG
  sanityCheckFieldsInfoLayout();
#endif

  TPCMetadataSection Header;
  for (const TPCMetadataFieldInfo &FI : TPCMetadataFieldsInfo) {
    if (FI.IsPlaceholder)
      continue;

    FI.SetValueFun(Header,
                   fromUint8Array(Data.data() + FI.offset, FI.BinSizeBytes));
  }
  return Header;
}

const TPCMetadataFieldInfo *
llvm::getTPCMetadataFieldInfo(const StringRef FieldName) {
  for (const TPCMetadataFieldInfo &I : TPCMetadataFieldsInfo)
    if (FieldName.compare_insensitive(I.fieldName) == 0)
      return &I;
  return nullptr;
}

bool llvm::setTpcMetadataValue(uint64_t Value,
                         const TPCMetadataFieldInfo &FieldInfo,
                         TPCMetadataSection &Result,
                         std::string &ErrorMessage) {

  if (FieldInfo.isArray()) {
    ErrorMessage = "Do not use this method for work with array field.";
    return false;
  } else if (FieldInfo.minValue > Value || FieldInfo.maxValue < Value) {
    ErrorMessage = formatv("Incorrect value."
                           " Expected value in range [{0}, {1}].",
                           FieldInfo.minValue, FieldInfo.maxValue);
    return false;
  }

  FieldInfo.SetValueFun(Result, Value);
  return true;
}

bool llvm::setTpcMetadataArrayValue(uint64_t Value,
                              unsigned Index,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage) {
  if (!FieldInfo.isArray()) {
    ErrorMessage = "Do not use this method for work with scalar field.";
    return false;
  } else if (Value < FieldInfo.minValue || Value > FieldInfo.maxValue) {
    ErrorMessage = formatv("Incorrect value."
                           " Expected value in range [{0}, {1}].",
                           FieldInfo.minValue, FieldInfo.maxValue);
    return false;
  }


  if (Index >= FieldInfo.length) {
    ErrorMessage = formatv("Invalid index."
                           " Expected index in range [0, {0}].",
                           FieldInfo.length - 1);
    return false;
  }

  (Result.*FieldInfo.TensorMask)[Index] = Value;
  return true;
}

bool llvm::setTpcMetadataArrayValue(const StringRef &Value,
                              const TPCMetadataFieldInfo &FieldInfo,
                              TPCMetadataSection &Result,
                              std::string &ErrorMessage) {
  if (!FieldInfo.isArray()) {
    ErrorMessage = "Do not use this method for work with scalar field.";
    return false;
  }
  if (Value.size() != FieldInfo.length) {
    ErrorMessage = formatv("Invalid vector length."
                           " Expected vector with length equals {0}.",
                           FieldInfo.length);
    return false;
  }

  bool *TensorMask = Result.*FieldInfo.TensorMask;
  for (unsigned I = 0; I < FieldInfo.length; ++I) {
    if (Value[I] == '0')
      TensorMask[I] = false;
    else if (Value[I] == '1')
      TensorMask[I] = true;
    else {
      ErrorMessage = "Incorrect value. Expected string of 0 and 1 chars.";
      return false;
    }
  }
  return true;
}
