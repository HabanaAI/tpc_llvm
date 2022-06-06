//===- llvm/IR/TPCTools.h --- TPC target support in IR ----------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for functions used in TPC specific
// IR level passes.
//
//===----------------------------------------------------------------------===//

#ifndef TPCTOOLS_H
#define TPCTOOLS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace llvm {

bool isTpcVectorType(const Type *Ty);
bool isVolatile(const Instruction *Inst);
bool isVolatileVariable(const GlobalVariable &V);
bool isAddressTaken(const Value *V);

/// Replace all uses of \c Old with \c New.
///
/// This function make similar transformation as \c replaceAllUsesWith, but it
/// is capable of changing address space of the values and also can replace
/// constant expressions with non-constant and vice versa.
///
/// \param Old     The value to replace.
/// \param New     The replacement value.
/// \param Builder The builder used to create new instructions.
/// \param DeletedInstrs  Instructions made unused as a result of the
///                       transformation go into this container.
///
void rauwConvertingAddressSpace(Value *Old, Value *New,
                                IRBuilder<> *Builder = nullptr,
                                SmallPtrSetImpl<Instruction *> *DeletedInstrs = nullptr);

class ValueReplacer {
public:
  ValueReplacer() {}
  ~ValueReplacer() { clear(); }

  void clear();
  void replace(Value *Old, Value *New);

private:

  struct PHIInfo {
    bool IsDetermined() const;
    void resolve();

    PHINode *NewPHI = nullptr;
    SmallVector<Value *, 8> NewArguments;
  };
  void schedule(Value *Old, Value *New);
  void processItem(Value *Old, Value *New);

  SmallVector<std::pair<Value *, Value *>, 16> WorkList;
  SmallPtrSet<Instruction *, 16> DeletedInstructions;
  DenseMap<PHINode *, PHIInfo> PHIMapping;
};

}
#endif
