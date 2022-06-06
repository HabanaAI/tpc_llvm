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

static constexpr unsigned TPCMetadataSectionSize = 262;
static constexpr unsigned TPCLastMarch = 6; // Doron1

template <typename FieldT>
static TPCMetadataFieldInfo
makeScalarFI(const char *FieldName, const unsigned ElementSize,
             const unsigned MinValue, const unsigned MaxValue,
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
      false};
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
      true};
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

namespace llvm {

const std::array<TPCMetadataFieldInfo, 16> TPCMetadataFieldsInfo = {
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
    makeScalarFI("dnorm", 1, 0, 1, 12, 16, &TPCMetadataSection::dnorm),
    makePlaceholderFI("reserveMidArea", 17, TPCReservedSize1, 3),
    TPCMetadataFieldInfo{
        TPCScalarLdName, 1, TPCNumTensors, 0, 1, 3, 26,
        [](const TPCMetadataSection &MD) {
          return convertTensorMaskToU64(MD.scalarLd, TPCNumTensors);
        },
        [](TPCMetadataSection &MD, std::uint64_t Value) {
          convertU64ToTensorMask(Value, MD.scalarLd, TPCNumTensors);
        },
        2, false},
    TPCMetadataFieldInfo{
        TPCRMWStoreName, 1, TPCNumTensors, 0, 1, 4, 28,
        [](const TPCMetadataSection &MD) {
          return convertTensorMaskToU64(MD.rmwStore, TPCNumTensors);
        },
        [](TPCMetadataSection &MD, std::uint64_t Value) {
          convertU64ToTensorMask(Value, MD.rmwStore, TPCNumTensors);
        },
        2, false},
    makePlaceholderFI("reserveEndArea", 30, TPCReservedSize, 3),
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
    Result |= (*(Array++)) << (8 * I);
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
    if (FieldName.compare_lower(I.fieldName) == 0)
      return &I;
  return nullptr;
}

bool llvm::setTpcMetadataValue(int64_t  Value,
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

bool llvm::setTpcMetadataArrayValue(int64_t Value,
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
                           " Expected index in range [0, {1}].",
                           FieldInfo.length);
    return false;
  }

  StringRef FieldName(FieldInfo.fieldName);
  if (FieldName.equals(TPCScalarLdName))
    Result.scalarLd[Index] = Value;
  else if (FieldName.equals(TPCRMWStoreName))
    Result.rmwStore[Index] = Value;
  else
    llvm_unreachable("An unhandled case occurred");

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

  const StringRef FieldName(FieldInfo.fieldName);

  bool *TensorMask = nullptr;
  if (FieldName.equals(TPCScalarLdName))
    TensorMask = Result.scalarLd;
  else if (FieldName.equals(TPCRMWStoreName))
    TensorMask = Result.rmwStore;
  else {
    ErrorMessage = formatv("Unexpected array field {0}.", FieldName);
    return false;
  }

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
