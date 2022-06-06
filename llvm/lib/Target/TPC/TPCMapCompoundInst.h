//===----------------------------------------------------------------------===//
/// \file
/// This file contains code that maps compound Instruction to the corresponding
/// set of Intrinsics.
/// This code is the property of Habana.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPCMAPCOMPOUNDINST_H
#define LLVM_TPCMAPCOMPOUNDINST_H
#include "TPCSubtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include <llvm/Support/Debug.h>

using namespace llvm;

#define _PRIMARY_TYPE 0
#define _VECTOR_TYPE 1

struct IntrinsicInfo {
#define TYPE_MATRIX 2
  Intrinsic::ID Id;
  Type::TypeID Ty[TYPE_MATRIX];
};

// Count enables us to capture multiple usages of the same
// intrinsic which is common in One-To-Many instructions
struct IntrinsicBunch {
  Intrinsic::ID Id;
  unsigned count;
};
using IntrinsicVector = SmallVector<IntrinsicBunch, 8>;

// TODO: Make the classes Singleton or use the pimpl idiom to reduce
// invocation cost
class MapCompundInstToIntrinsics {

public:
  static void printIntrinsicVector(const IntrinsicVector &I, raw_ostream &OS) {
    for (auto Elem : I) {
      OS << "ID is " << Elem.Id << " ";
      OS << "Count is " << Elem.count << "\n";
      OS << "\n";
    }
  }

  MapCompundInstToIntrinsics() = default;

  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST) {
    return {{}};
  }

  virtual ~MapCompundInstToIntrinsics() = default;
};
using CompoundInstToIntrinsMap =
    DenseMap<int, std::shared_ptr<MapCompundInstToIntrinsics>>;

class MapTruncate : virtual public MapCompundInstToIntrinsics {

  IntrinsicVector Trunc_32_16 = {{Intrinsic::tpc_pack, 4},
                                 {Intrinsic::tpc_mov_dual_group, 4}};
  IntrinsicVector Trunc_32_16_goya = {{Intrinsic::tpc_pack, 4},
                                      {Intrinsic::tpc_mov_dual_group, 7}};
  IntrinsicVector Trunc_32_8 = {{Intrinsic::tpc_pack, 9},
                                {Intrinsic::tpc_mov_dual_group, 14}};

public:
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
  const IntrinsicVector &getTrunc32_16Goya() const { return Trunc_32_16_goya; }
  const IntrinsicVector &getTrunc32_16() const { return Trunc_32_16; }
  const IntrinsicVector &getTrunc32_16(const TPCSubtarget *ST) const {
    return (ST->hasGoyaISA()) ? getTrunc32_16Goya() : getTrunc32_16();
  }
  const IntrinsicVector &getTrunc16_8() const { return getTrunc32_16Goya(); }
  const IntrinsicVector &getTrunc32_8() const { return Trunc_32_8; }

  virtual ~MapTruncate() = default;
  IntrinsicVector downConvertVec(const Instruction *I, const TPCSubtarget *ST);
};

class MapExtend : virtual public MapCompundInstToIntrinsics {
  IntrinsicVector Ext_8_32_Gaudi = {{Intrinsic::tpc_pack, 4},
                                    {Intrinsic::tpc_mov_dual_group, 8}};
  IntrinsicVector Ext_8_32 = {{Intrinsic::tpc_pack, 4},
                              {Intrinsic::tpc_mov_dual_group, 14}};
  IntrinsicVector Ext_8_16 = {{Intrinsic::tpc_pack, 2},
                              {Intrinsic::tpc_mov_dual_group, 6},
                              {Intrinsic::tpc_convert, 2}};
  IntrinsicVector Ext_16_32_Gaudi = {{Intrinsic::tpc_pack, 2},
                                     {Intrinsic::tpc_mov_dual_group, 2},
                                     {Intrinsic::tpc_mov_group, 1}};
  IntrinsicVector Ext_16_32 = {{Intrinsic::tpc_pack, 2},
                               {Intrinsic::tpc_mov_dual_group, 6}};

public:
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
  const IntrinsicVector &getExt8_32Gaudi() const { return Ext_8_32_Gaudi; }
  const IntrinsicVector &getExt8_32() const { return Ext_8_32; }
  const IntrinsicVector &getExt8_16() const { return Ext_8_16; }
  const IntrinsicVector &getExt16_32Gaudi() const { return Ext_16_32_Gaudi; }
  const IntrinsicVector &getExt16_32() const { return Ext_16_32; }
  const IntrinsicVector &getExt8_32(const TPCSubtarget *ST) const {
    return (ST->hasGaudiISA() || ST->hasGaudiBISA()) ? getExt8_32Gaudi()
                                                     : getExt8_32();
  }
  const IntrinsicVector &getExt8_16(const TPCSubtarget *ST) const {
    return getExt8_16();
  }
  const IntrinsicVector &getExt16_32(const TPCSubtarget *ST) const {
    return (ST->hasGaudiISA() || ST->hasGaudiBISA()) ? getExt16_32Gaudi()
                                                     : getExt16_32();
  }
  IntrinsicVector upConvertVec(const Instruction *I, const TPCSubtarget *ST);
  virtual ~MapExtend() = default;
};

class MapExtendTruncate : public MapExtend, public MapTruncate {

public:
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
  virtual ~MapExtendTruncate() = default;
};

class MapSelect : public MapCompundInstToIntrinsics {

  IntrinsicVector Select_Mov = {{Intrinsic::tpc_mov, 2}};
  IntrinsicVector Max = {{Intrinsic::tpc_max, 1}};
  IntrinsicVector Min = {{Intrinsic::tpc_min, 1}};

public:
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
  const IntrinsicVector &getSelect_Mov() const { return Select_Mov; }
  const IntrinsicVector &getMax() const { return Max; }
  const IntrinsicVector &getMin() const { return Min; }
  virtual ~MapSelect() = default;
};

class MapDivide : public MapCompundInstToIntrinsics {
  bool Unsigned;
  IntrinsicVector And = {{Intrinsic::tpc_and, 2}};
  IntrinsicVector Base = {{Intrinsic::tpc_mov, 2}};

public:
  IntrinsicVector &getBase() { return Base; }

  MapDivide(bool Unsigned) : Unsigned(Unsigned) {}

  unsigned getCount(const Instruction *I, const TPCSubtarget *ST) {
    unsigned Count = 0;
    bool Gen3 = ST->hasGrecoISA();
    bool Gen2 = ST->hasGaudiISA() || ST->hasGaudiBISA();
    bool Gen1 = ST->hasGoyaISA();
    // Note: Division lowering code does not consider Gaudi2 in Gen2OrHigher
    bool Gen2OrHigher =
        ST->hasGrecoISA() || ST->hasGaudiISA() || ST->hasGaudiBISA();
    if (I->getType()->isIntegerTy()) {
      switch (I->getType()->getPrimitiveSizeInBits()) {
      case 32:
        if (Gen1)
          Count = 32;
        else if (Gen2)
          Count = 8;
        else if (Gen3)
          Count = 4;
        else
          assert(false && "Gaudi2 do not support udiv_step");
        break;
      case 16:
        if (Gen1)
          Count = 16;
        else if (Gen2)
          Count = 4;
        else if (Gen3)
          Count = 2;
        else
          assert(false && "Gaudi2 do not support udiv_step");
        break;
      default:
        if (Gen1)
          Count = 8;
        else if (Gen2)
          Count = 2;
        else if (Gen3)
          Count = 1;
        else
          assert(false && "Gaudi2 do not support udiv_step");
        break;
      }
    } else {
      Count = Gen3 ? 1 : (Gen2OrHigher ? 2 : 8);
    }
    return Count;
  }

  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
};

class MapUnsignedDivide : public MapDivide {
public:
  MapUnsignedDivide() : MapDivide(true) {}
  virtual ~MapUnsignedDivide(){};
};

class MapSignedDivide : public MapDivide {
  IntrinsicVector Base = {
      {Intrinsic::tpc_xor, 1}, {Intrinsic::tpc_and, 1}, {Intrinsic::tpc_or, 1}};

public:
  IntrinsicVector &getSBase() { return Base; }
  MapSignedDivide() : MapDivide(false) {}
  virtual ~MapSignedDivide() = default;

  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
};

class MapFDivide : public MapCompundInstToIntrinsics {
public:
  virtual ~MapFDivide() = default;
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST) {
    assert(I->getType()->isVectorTy() && "FDIV not supported on scalar pipe.");
    return {{Intrinsic::tpc_calc_fp_special, 1}};
  }
};

class MapFRem : public MapFDivide {

public:
  IntrinsicVector Base = {{Intrinsic::tpc_mul, 1}, {Intrinsic::tpc_sub, 1}};
  const IntrinsicVector &getBase() const { return Base; }

  virtual ~MapFRem() = default;
  virtual IntrinsicVector getMapping(const Instruction *I,
                                     const TPCSubtarget *ST);
};
#endif // LLVM_TPCMAPCOMPOUNDINST_H
