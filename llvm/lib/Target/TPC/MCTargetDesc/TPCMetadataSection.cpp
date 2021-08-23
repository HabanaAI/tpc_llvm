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


const int VersionLength = 4;

using namespace llvm;

cl::opt<bool> llvm::TPCElfMemcpy(
    "tpc-elf-spec-bin", cl::init(false), cl::Hidden,
    cl::desc("Serialize TPC Metadata in binary format"));

template<typename T>
static inline void FromUint8Array(const uint8_t *Array, size_t Length, T &Data) {
  std::size_t SizeType = sizeof(T);
  Data &= 0 ;
  for (std::size_t i = 0; i < (SizeType < Length ? SizeType : Length); ++i) {
    Data |= (*Array << 8 * (i));
    ++Array;
  }
}

static inline void ToUint8Array(long Number, int length, uint8_t *&ptr) {
  for (std::size_t i = 0; i < length; ++i) {
    uint8_t val = (Number >> 8 * i & 0xFF);
    *ptr = val;
    ++ptr;
  }
}

std::vector<uint8_t>
llvm::bianrySerializeTPCProgramHeader(const llvm::TPCMetadataSection &Header) {
  assert(Header.version <= TPCMetadataVersion);
  int val = 0;
  std::vector<uint8_t> Data(TPCMetadataSectionSize, 0);
  uint8_t *bufTemp = Data.data();
  ToUint8Array(Header.version, TPCMetadataFieldsInfo[0].elementSize, bufTemp);
  if (Header.version >= 3) {
    ToUint8Array(Header.specialFunctionUsed,
                TPCMetadataFieldsInfo[1].elementSize, bufTemp);
    ToUint8Array(Header.printfUsed, TPCMetadataFieldsInfo[2].elementSize,
                bufTemp);
    ToUint8Array(Header.lockUnLock, TPCMetadataFieldsInfo[3].elementSize,
                bufTemp);

    if (Header.version >= 6)
      ToUint8Array(Header.mmioUsed, TPCMetadataFieldsInfo[8].elementSize,
                  bufTemp);
    else
      ToUint8Array(0, TPCMetadataFieldsInfo[8].elementSize, bufTemp);

    if (Header.version >= 5)
      ToUint8Array(Header.march, TPCMetadataFieldsInfo[6].elementSize, bufTemp);
    else
      ToUint8Array(0, TPCMetadataFieldsInfo[6].elementSize, bufTemp);

    ToUint8Array(0, 2, bufTemp);
    if (Header.version >= 8) {
      ToUint8Array(Header.paramsNum, TPCMetadataFieldsInfo[9].elementSize,
                   bufTemp);
    } else
      ToUint8Array(0, TPCMetadataFieldsInfo[9].elementSize,
                   bufTemp);

    if (Header.version >= 9) {
      ToUint8Array(Header.printfTensorID, TPCMetadataFieldsInfo[10].elementSize,
                   bufTemp);
    } else
      ToUint8Array(0, TPCMetadataFieldsInfo[10].elementSize,
                   bufTemp);

    ToUint8Array(0, sizeof(Header.reservedTemp), bufTemp);

    for (unsigned i = 0; i < TPCNumTensors; ++i) {
      val += ((int)Header.scalarLd[i] << i);
    }
    ToUint8Array(val, 2, bufTemp);
  }
  if (Header.version >= 4) {
    val = 0;
    for (unsigned i = 0; i < TPCNumTensors; ++i) {
      val += Header.rmwStore[i] << i;
    }
    ToUint8Array(val, 2, bufTemp);
  }
  // Write reserved
  ToUint8Array(0, sizeof(Header.reserved), bufTemp);
  return Data;
}

TPCMetadataSection llvm::stringDeserializeTPCProgramHeader(
    const llvm::StringRef& String) {

  TPCMetadataSection Header;
  int CurrentPos = 0;

  // Parse version
  String.substr(CurrentPos, VersionLength).getAsInteger(10, Header.version);
  CurrentPos += VersionLength;

  for(const TPCMetadataFieldInfo *Cur = &TPCMetadataFieldsInfo[1];
      Cur != std::end(TPCMetadataFieldsInfo); ++Cur) {
    if (Cur->startWithVersion > Header.version)
      continue;

    if(!Cur->isArray()) {
       StringRef StringValue = String.substr(Cur->offset, Cur->elementSize);
      if (StringRef(Cur->fieldName).compare_lower(TPCSpecialFunctionUsedName) == 0)
        StringValue.getAsInteger(10,  Header.specialFunctionUsed);
      else if (StringRef(Cur->fieldName).compare_lower(TPCPrintfUsedName) == 0)
        StringValue.getAsInteger(10,  Header.printfUsed);
      else if (StringRef(Cur->fieldName).compare_lower(TPCLockUnLockName) == 0)
        StringValue.getAsInteger(10,  Header.lockUnLock);
      else if (StringRef(Cur->fieldName).compare_lower(TPCMarchName) == 0)
        StringValue.getAsInteger(10,  Header.march);
      else if (StringRef(Cur->fieldName).compare_lower(TPCMMIOName) == 0)
        StringValue.getAsInteger(10, Header.mmioUsed);
      else
        llvm_unreachable(TPCUnhandledMetadataField);

    } else {
      if (StringRef(Cur->fieldName).compare_lower(TPCScalarLdName) == 0) {
        for (unsigned i = 0; i < Cur->length; ++i) {
          StringRef StringValue = String.substr(Cur->offset+i, Cur->elementSize);
          StringValue.getAsInteger(10, Header.scalarLd[i]);
        }
      } else if (StringRef(Cur->fieldName).compare_lower(TPCRMWStoreName) == 0) {
        for (unsigned i = 0; i < Cur->length; ++i) {
          StringRef StringValue = String.substr(Cur->offset+i, Cur->elementSize);
          StringValue.getAsInteger(10, Header.rmwStore[i]);
          CurrentPos += Cur->elementSize;
        }
      } else
        llvm_unreachable(TPCUnhandledMetadataField);
    }
  }

  return Header;
}


TPCMetadataSection llvm::binaryDeserializeTPCProgramHeader(
    const std::vector<uint8_t> &Data) {
  assert(Data.size() == TPCMetadataSectionSize);

  const uint8_t *CurPos = Data.data();
  unsigned RestLength = TPCMetadataSectionSize;

  TPCMetadataSection Header;

  if (Header.version >= 5) {
    FromUint8Array(CurPos, RestLength, Header.version);
    CurPos += sizeof(Header.version);
    RestLength -= sizeof(Header.version);

    FromUint8Array(CurPos, RestLength, Header.specialFunctionUsed);
    CurPos += sizeof(Header.specialFunctionUsed);
    RestLength -= sizeof(Header.specialFunctionUsed);

    FromUint8Array(CurPos, RestLength, Header.printfUsed);
    CurPos += sizeof(Header.printfUsed);
    RestLength -= sizeof(Header.printfUsed);

    FromUint8Array(CurPos, RestLength, Header.lockUnLock);
    CurPos += sizeof(Header.lockUnLock);
    RestLength -= sizeof(Header.lockUnLock);

    FromUint8Array(CurPos, RestLength, Header.mmioUsed);
    CurPos += sizeof(Header.mmioUsed);
    RestLength -= sizeof(Header.mmioUsed);

    FromUint8Array(CurPos, RestLength, Header.march);
    CurPos += sizeof(Header.march);
    RestLength -= sizeof(Header.march);

    CurPos += 2;
    RestLength -= 2;

    FromUint8Array(CurPos, RestLength, Header.paramsNum);
    CurPos += sizeof(Header.paramsNum);
    RestLength -= sizeof(Header.paramsNum);

    FromUint8Array(CurPos, RestLength, Header.printfTensorID);
    CurPos += sizeof(Header.printfTensorID);
    RestLength -= sizeof(Header.printfTensorID);

    CurPos += sizeof(Header.reservedTemp);
    RestLength -= sizeof(Header.reservedTemp);

    unsigned short temp = 0;
    FromUint8Array(CurPos, RestLength, temp);
    CurPos += 2; // The size of the scalarLd in the tpc_metadata(elfapi) struct is 2
    RestLength -= 2;
    for (unsigned i = 0; i < TPCNumTensors; ++i) {
      Header.scalarLd[i] = (temp >> i) & 0x1;
    }
    temp = 0;
    FromUint8Array(CurPos, RestLength, temp);
    CurPos += 2; // The size of the scalarLd in the tpc_metadata(elfapi) struct is 2
    RestLength -= 2;
    for (unsigned i = 0; i < TPCNumTensors; ++i) {
      Header.rmwStore[i] = temp >> i & 0x1;
    }
  }

  return Header;
}

bool llvm::setTpcMetadataValue(int64_t  Value,
                         const TPCMetadataFieldInfo &FieldInfo,
                         TPCMetadataSection &Result,
                         std::string &ErrorMessage) {

  if (FieldInfo.isArray()) {
    ErrorMessage = "Do not use this method for work with array field.";
    return false;
  } else if (FieldInfo.minValue > Value || FieldInfo.maxValie < Value) {
    ErrorMessage = formatv("Incorrect value."
                           " Expected value in range [{0}, {1}].",
                           FieldInfo.minValue, FieldInfo.maxValie);
    return false;
  }

  StringRef FieldName(FieldInfo.fieldName);
  if (FieldName.equals(TPCVersionName))
    Result.version = Value;
  else if (FieldName.equals(TPCSpecialFunctionUsedName))
    Result.specialFunctionUsed = Value;
  else if (FieldName.equals(TPCPrintfUsedName))
    Result.printfUsed = Value;
  else if (FieldName.equals(TPCLockUnLockName))
    Result.lockUnLock = Value;
  else if (FieldName.equals(TPCMarchName))
    Result.march = Value;
  else if (FieldName.equals(TPCMMIOName))
    Result.mmioUsed = Value;
  else if (FieldName.equals(TPCParamsNumName))
    Result.paramsNum = Value;
  else if (FieldName.equals(TPCPrintfTensorIDName))
    Result.printfTensorID = Value;
  else
    llvm_unreachable("An unhandled case occurred");

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
  } else if (Value < FieldInfo.minValue || Value > FieldInfo.maxValie) {
    ErrorMessage = formatv("Incorrect value."
                           " Expected value in range [{0}, {1}].",
                           FieldInfo.minValue, FieldInfo.maxValie);
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

  auto ToDigitsVector = [](const StringRef &Value) {
    std::vector<unsigned> Digits;
    for (char C : Value) {
      assert(std::isdigit(C));
      Digits.push_back(C - '0');
    }

    return Digits;
  };

  StringRef FieldName(FieldInfo.fieldName);

  std::vector<unsigned> Digits = ToDigitsVector(Value);
  if (Digits.size() != FieldInfo.length) {
    ErrorMessage = formatv("Invalid vector length."
                           " Expected vector with length equals {0}.",
                           FieldInfo.length);
    return false;
  }

  if (FieldName.equals(TPCScalarLdName)) {
    for(std::size_t i = 0; i < Digits.size(); ++i) {
      if (FieldInfo.minValue > Digits[i] || FieldInfo.maxValie < Digits[i]) {
        ErrorMessage = formatv("Incorrect value."
                               " Expected value in range [{0}, {1}].",
                               FieldInfo.minValue, FieldInfo.maxValie);
        return false;
      }

      Result.scalarLd[i] = Digits[i];
    }
  } else if (FieldName.equals(TPCRMWStoreName)) {
    for(std::size_t i = 0; i < Digits.size(); ++i) {
      if (FieldInfo.minValue > Digits[i] || FieldInfo.maxValie < Digits[i]) {
        ErrorMessage = formatv("Incorrect value."
                               " Expected value in range [{0}, {1}].",
                               FieldInfo.minValue, FieldInfo.maxValie);
        return false;
      }

      Result.rmwStore[i] = Digits[i];
    }
  }

  return true;
}
