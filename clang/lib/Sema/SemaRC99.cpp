//===--- SemaRC99.cpp - Semantic Analysis for Declarations ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for declarations.
//
//===----------------------------------------------------------------------===//
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclLookups.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaInternal.h"

#include "../lib/Target/TPC/MCTargetDesc/InstructionDB.h"
#include "llvm/ADT/APInt.h"

#include <map>
#include <sstream>

using namespace clang;
using namespace sema;
using llvm::TPCII::OpType;


/*
static int getOptypeValue(QualType Ty) {
  BuiltinType::Kind kind;
  if (auto BT = Ty->getAs<BuiltinType>()) {
    kind = BT->getKind();
  }
  else if (auto VT = Ty->getAs<clang::VectorType>()) {
    //bool256 processing
    if (32 == VT->getNumElements())
      kind = BuiltinType::Bool;
    else
      kind = VT->getElementType()->getAs<BuiltinType>()->getKind();
  }
  else
    llvm_unreachable("Unexpected type");

  //V32c
  switch (kind) {
  case BuiltinType::Float:    return OpType::FP32;
  case BuiltinType::BFloat16: return OpType::BF16;
  case BuiltinType::Float8_152: return OpType::FP8_152;
  case BuiltinType::Float8_143: return OpType::FP8_143;
  case BuiltinType::Half:     return OpType::FP16;
  case BuiltinType::Char_S:   return OpType::INT8;
  case BuiltinType::Short:    return OpType::INT16;
  case BuiltinType::Int:      return OpType::INT32;
  case BuiltinType::UChar:    return OpType::UINT8;
  case BuiltinType::UShort:   return OpType::UINT16;
  case BuiltinType::UInt:     return OpType::UINT32;
  case BuiltinType::Bool:     return OpType::BOOL;
  default:
    llvm_unreachable("Unexpected type");
  }
}
*/

static bool isPrintFWithValue(unsigned BI) {
  switch (BI) {
  case TPC::BIprintf_i:
  case TPC::BIprintf_ui:
  case TPC::BIprintf_s:
  case TPC::BIprintf_us:
  case TPC::BIprintf_c:
  case TPC::BIprintf_uc:
  case TPC::BIprintf_f:
  case TPC::BIprintf_bf:
  case TPC::BIprintf_h:
  case TPC::BIprintf_h8:
  case TPC::BIprintf_f8:
    return true;
  default:
    return false;
  }
}


/// Checks if the given type is valid TPC vector type.
///
/// \param T          Type to investigate.
/// \param InVectMem  If set, the function checks if the gived type is valid
///                   for objects allocated in vector memory/Vx registers.
///
bool isTpcVectorType(QualType T, bool InVectMem) {
  T = T.getCanonicalType();
  if (auto VT = dyn_cast<VectorType>(T.getTypePtr()))
    if (const BuiltinType *BT = VT->getElementType()->getAs<BuiltinType>()) {
      if (VT->getNumElements() == 256 &&
          (BT->getKind() == BuiltinType::Kind::SChar ||
           BT->getKind() == BuiltinType::Kind::UChar ||
           BT->getKind() == BuiltinType::Kind::Char_S ||
           BT->getKind() == BuiltinType::Kind::Char_U ||
           BT->getKind() == BuiltinType::Kind::Short ||
           BT->getKind() == BuiltinType::Kind::UShort ||
           BT->getKind() == BuiltinType::Kind::BFloat16 ||
           BT->getKind() == BuiltinType::Kind::Float8_152 ||
           BT->getKind() == BuiltinType::Kind::Float8_143 ||
           BT->getKind() == BuiltinType::Kind::Int ||
           BT->getKind() == BuiltinType::Kind::UInt))
        return true;
      if (VT->getNumElements() == 128 &&
          (BT->getKind() == BuiltinType::Kind::Short ||
           BT->getKind() == BuiltinType::Kind::UShort ||
           BT->getKind() == BuiltinType::Kind::Int ||
           BT->getKind() == BuiltinType::Kind::UInt ||
           BT->getKind() == BuiltinType::Kind::BFloat16 ||
           BT->getKind() == BuiltinType::Kind::Half))
        return true;
      if (VT->getNumElements() == 64 &&
          (BT->getKind() == BuiltinType::Kind::Int ||
           BT->getKind() == BuiltinType::Kind::UInt ||
           BT->getKind() == BuiltinType::Kind::Float))
        return true;
      if (VT->getNumElements() == 32 &&
          (BT->getKind() == BuiltinType::Kind::UChar ||
           BT->getKind() == BuiltinType::Kind::Char_U))
        return true;
      if (VT->getNumElements() == 16 &&
          (BT->getKind() == BuiltinType::Kind::UChar ||
           BT->getKind() == BuiltinType::Kind::Char_U))
        return true;
      if (VT->getNumElements() == 8 &&
          (BT->getKind() == BuiltinType::Kind::UChar ||
           BT->getKind() == BuiltinType::Kind::Char_U))
        return true;
      if (VT->getNumElements() == 5 && !InVectMem &&
          BT->getKind() == BuiltinType::Kind::Int)
        return true;
    }

  return false;
}


bool clang::isInTPCVectorAddressSpace(QualType T) {
  // Only kosher vector types may be allocated in vector storage.
  if (isTpcVectorType(T, true))
    return true;

  // All other vector types (int5) are allocated in scalar storage.
  if (T->isVectorType())
    return false;

  if (T->isArrayType())
    return isInTPCVectorAddressSpace(T->getAsArrayTypeUnsafe()->getElementType());

  if (T->isRecordType()) {
    auto D = cast<RecordDecl>(T->getAsTagDecl());
    for (auto *Field : D->fields()) {
      bool IsVector = false;
      bool IsScalar = false;
      QualType FT = Field->getType();
      bool FieldIsInVectorMem = isInTPCVectorAddressSpace(FT);
      if (FieldIsInVectorMem) {
        assert(!IsScalar);
        IsVector = true;
      } else {
        assert(!IsVector);
        IsScalar = true;
      }
      return IsVector;
    }
  }

  return false;
}

static bool isBuiltinShortType(const BuiltinType *BT) {
  switch (BT->getKind()) {
  case BuiltinType::Char_U:
  case BuiltinType::UChar:
  case BuiltinType::SChar:
  case BuiltinType::WChar_U:
  case BuiltinType::WChar_S:
  case BuiltinType::Char8:
  case BuiltinType::Char16:
  case BuiltinType::Short:
  case BuiltinType::UShort:
  case BuiltinType::Half:
  case BuiltinType::BFloat16:
  case BuiltinType::Float8_152:
  case BuiltinType::Float8_143:
    return true;
  default:
    return false;
  }
}

enum AlignmentStatus {
  NoShort,
  OneShort,
  AlignmentConflict
};

static AlignmentStatus containsShortValue(QualType Ty) {
  Ty = Ty->getCanonicalTypeUnqualified();
  if (const auto *Record = dyn_cast<RecordType>(Ty)) {
    if (const auto *RDecl = dyn_cast<RecordDecl>(Record->getDecl())) {
      AlignmentStatus Num = NoShort;
      unsigned NumFields = 0;
      for (const auto &Field : RDecl->fields()) {
        if (++NumFields > 1) {
          if (Num != NoShort && RDecl->getTagKind() != TTK_Union)
            return AlignmentConflict;
        }
        QualType FType = Field->getType();
        switch (containsShortValue(FType)) {
        case NoShort:
          break;
        case OneShort:
          if (Num == NoShort)
            Num = OneShort;
          else
            Num = AlignmentConflict;
          break;
        case AlignmentConflict:
          Num = AlignmentConflict;
          break;
        }
        if (Num == AlignmentConflict)
          break;
      }
      if (Num == AlignmentConflict && RDecl->getTagKind() == TTK_Union)
        return OneShort;
      return Num;
    }
  } else if (const auto *AT = dyn_cast<ConstantArrayType>(Ty)) {
    QualType ET = AT->getElementType();
    switch (containsShortValue(ET)) {
    case NoShort: return NoShort;
    case AlignmentConflict: return AlignmentConflict;
    case OneShort:
      return AT->getSize().getZExtValue() > 1 ? AlignmentConflict : OneShort;
    }
  } else if (const auto *ET = dyn_cast<EnumType>(Ty)) {
    const EnumDecl *ED = ET->getDecl();
    return containsShortValue(ED->getIntegerType());
  } else if (const auto *BT = dyn_cast<BuiltinType>(Ty)) {
    return isBuiltinShortType(BT) ? OneShort : NoShort;
  }
  return NoShort;
}

static bool isIndexType(QualType Ty) {
  Ty = Ty.getCanonicalType();
  if (Ty->isVectorType()) {
    auto VTy = Ty->getAs<VectorType>();
    if (VTy->getNumElements() != 5)
      return false;
    auto ElTy = VTy->getElementType()->getAs<BuiltinType>();
    return ElTy->getKind() == BuiltinType::Kind::Int;
  }
  return false;
}

namespace {

class RC99Scanner : public RecursiveASTVisitor<RC99Scanner> {
  using BaseType = RecursiveASTVisitor<RC99Scanner>;
  using TpcType = ASTContext::TpcType;

  Sema &SemaRef;
  TargetOptions &TOptions;
  std::map <Expr *, Expr *> MacCall;
  std::set<ArraySubscriptExpr *> SubscrInPrintF;
  unsigned Int5Dim;
  bool AllowInt5Access = false;
  bool NeedCheckInt5;

  class Int5CheckResetter {
    RC99Scanner &Scanner;
    bool OldAllowInt5Access;
    unsigned OldInt5Dim;
  public:
    Int5CheckResetter(RC99Scanner &Scanner)
      : Scanner(Scanner),
        OldAllowInt5Access(Scanner.AllowInt5Access),
        OldInt5Dim(Scanner.Int5Dim) {
      Scanner.AllowInt5Access = false;
    }
    ~Int5CheckResetter() { Scanner.AllowInt5Access = OldAllowInt5Access; }
    bool getOldAllowInt5Access() const { return OldAllowInt5Access; }
    unsigned getOldInt5Dim() const { return OldInt5Dim; }
  };

  ASTContext &getContext() const { return SemaRef.getASTContext(); }

  bool isBoolX(QualType Ty) const {
    QualType CTy = Ty.getCanonicalType();
    return CTy == SemaRef.getASTContext().getTpcType(TpcType::bool256) ||
           CTy == SemaRef.getASTContext().getTpcType(TpcType::bool128) ||
           CTy == SemaRef.getASTContext().getTpcType(TpcType::bool64);
  }

  bool checkNonZeroElement(QualType TargetType,Expr *E, bool MultiElts) {
    TargetType = TargetType.getCanonicalType();
    if (auto ILE = dyn_cast<InitListExpr>(E)) {
      TargetType = ILE->getType();
      if (ILE->getNumInits() == 1) {
        return checkNonZeroElement(TargetType, ILE->getInit(0), false);
      } else {
        for (Expr *El : ILE->inits())
          if (checkNonZeroElement(TargetType, El, true))
            return true;
      }
      return false;
    }

    // Check only vector types located in vector memory. Skip int5, as
    // there is no problem in initializing it by elements.
    if (!isTpcVectorType(TargetType, true))
      return false;

    // Skip implicit casts. They may be found if integer constant is used to
    // initialize value of real type.
    while (auto ICE = dyn_cast<ImplicitCastExpr>(E))
      E = ICE->getSubExpr();

    if (!E->isIntegerConstantExpr(SemaRef.getASTContext())) {
      SemaRef.Diag(E->getBeginLoc(), diag::err_partial_initlist);
      if (!MultiElts)
        SemaRef.Diag(E->getBeginLoc(), diag::note_maybe_broadcast);
      return true;
    }

    llvm::APSInt Val = E->EvaluateKnownConstInt(SemaRef.getASTContext());
    if (Val.getBoolValue()) {
        SemaRef.Diag(E->getBeginLoc(), diag::err_partial_initlist);
        if (!MultiElts)
          SemaRef.Diag(E->getBeginLoc(), diag::note_maybe_broadcast);
        return true;
    }

    return false;
  }

  // If argument has type of global pointer, issue error message.
  //
  bool checkNotGlobalPtr(const Expr *Op, SourceLocation Loc) const {
    QualType Ty = Op->getType();
    if (Ty->isPointerType()) {
      QualType PointeeTy = Ty->getAs<PointerType>()->getPointeeType();
      if (PointeeTy.getAddressSpace() == LangAS::rc99_global) {
        SemaRef.Diag(Loc, diag::err_unsupported_op_on_gptr);
        return false;
      }
    }
    return true;
  }

  bool isGloabLPtr(QualType Ty) {
    if (!Ty->isPointerType())
      return false;
    QualType PointeeTy = Ty->getAs<PointerType>()->getPointeeType();
    return PointeeTy.getAddressSpace() == LangAS::rc99_global;
  }

  bool checkGlobalPtrType(QualType Ty, SourceLocation Loc) {
    if (Ty->isPointerType()) {
      unsigned Indirection = 0;
      while (true) {
        QualType PointeeTy = Ty->getAs<PointerType>()->getPointeeType();
        if (PointeeTy.getAddressSpace() == LangAS::rc99_global) {
          if (Indirection) {
            SemaRef.Diag(Loc, diag::err_pointer_to_gptr);
            return false;
          }
          QualType ValueTy = PointeeTy.getUnqualifiedType();
          if (!ValueTy->isIntegerType() && !ValueTy->isFloatingType() &&
              !(ValueTy->isVectorType() && isInTPCVectorAddressSpace(ValueTy)) &&
              !ValueTy->isVoidType()) {
              SemaRef.Diag(Loc, diag::err_unsupported_gptr);
              return false;
          }
        }
        if (!PointeeTy->isPointerType())
          return checkGlobalPtrType(PointeeTy, Loc);
        Ty = PointeeTy;
        Indirection++;
      }
    }
    else if (Ty->isArrayType()) {
      const Type *ElementType = Ty->getArrayElementTypeNoTypeQual();
      if (ElementType->isPointerType()) {
        QualType PointeeTy = ElementType->getAs<PointerType>()->getPointeeType();
        if (PointeeTy.getAddressSpace() == LangAS::rc99_global) {
          SemaRef.Diag(Loc, diag::err_array_of_gptr);
          return false;
        }
      }
      return checkGlobalPtrType(QualType(ElementType, 0), Loc);
    }
    else if (Ty->isStructureOrClassType() || Ty->isUnionType()) {
      auto RecTy = Ty->getAs<RecordType>();
      for (auto Field : RecTy->getDecl()->fields()) {
        QualType FieldTy = Field->getType();
        if (FieldTy->isPointerType()) {
          QualType PointeeTy = FieldTy->getAs<PointerType>()->getPointeeType();
          if (PointeeTy.getAddressSpace() == LangAS::rc99_global) {
            SemaRef.Diag(Field->getLocation(), diag::err_field_is_gptr);
            return false;
          }
        }
        if (!checkGlobalPtrType(FieldTy, Loc))
          return false;
      }
    }
    return true;
  }

  bool checkMulValidity(const BinaryOperator *E) {
    QualType Ty = E->getType();
    assert(E->getOpcode() == BinaryOperatorKind::BO_Mul ||
           E->getOpcode() == BinaryOperatorKind::BO_MulAssign);
    if (auto *VTy = Ty.getCanonicalType()->getAs<VectorType>()) {
      QualType EltTy = VTy->getElementType();
      if (EltTy->isIntegerType()) {
        switch (EltTy.getCanonicalType()->getAs<BuiltinType>()->getKind()) {
        case BuiltinType::Int:
        case BuiltinType::UInt:
          break;
        default:
          SemaRef.Diag(E->getExprLoc(), diag::err_unsupported_mul)
            << E->getSourceRange();
          return false;
        }
      }
    }
    return true;
  }

  bool getMaddSwitchValue(unsigned BuiltinId, CallExpr *E, unsigned &Val) {
    unsigned SwArgNo;
    switch (BuiltinId) {
      case TPC::BIv_f32_madd_b:
      case TPC::BIv_f32_madd_vb:
      case TPC::BIv_bf16_madd_b:
      case TPC::BIv_bf16_madd_vb:
      case TPC::BIv_f16_madd_b:
      case TPC::BIv_f16_madd_vb:
      case TPC::BIv_i8_madd_b:
      case TPC::BIv_i8_madd_vb:
      case TPC::BIv_u8_madd_b:
      case TPC::BIv_u8_madd_vb:
      case TPC::BIv_i16_madd_b:
      case TPC::BIv_i16_madd_vb:
      case TPC::BIv_u16_madd_b:
      case TPC::BIv_u16_madd_vb:
      case TPC::BIv_bf16_madd_acc32_b:
      case TPC::BIv_bf16_madd_acc32_vb:
      case TPC::BIv_f16_madd_acc32_b:
      case TPC::BIv_f16_madd_acc32_vb:
      case TPC::BIv_i8_madd_acc16_b:
      case TPC::BIv_i8_madd_acc16_vb:
      case TPC::BIv_u8_madd_acc16_b:
      case TPC::BIv_u8_madd_acc16_vb:
      case TPC::BIv_u8_madd_acc32_b:
      case TPC::BIv_u8_madd_acc32_vb:
      case TPC::BIv_u16_madd_acc32_b:
      case TPC::BIv_u16_madd_acc32_vb:
      {
        SwArgNo = 3;
        break;
      }
      case TPC::BIv_i8_madd_zp_vb:
      case TPC::BIv_i8_madd_zp_b:
      case TPC::BIv_u8_madd_zp_vb:
      case TPC::BIv_u8_madd_zp_b:
      case TPC::BIv_i8_madd_zp_acc16_vb:
      case TPC::BIv_i8_madd_zp_acc16_b:
      case TPC::BIv_u8_madd_zp_acc16_vb:
      case TPC::BIv_u8_madd_zp_acc16_b:
      case TPC::BIv_u8_madd_zp_acc32_vb:
      case TPC::BIv_u8_madd_zp_acc32_b:
      {
        SwArgNo = 4;
        break;
      }
      default:
        return false;
    }
    ASTContext &Ctx = SemaRef.getASTContext();
    auto *D = cast<FunctionDecl>(E->getCallee()->getReferencedDeclOfCallee());
    const Expr *SwArg = E->getArg(SwArgNo);
    Expr::EvalResult ResVal;
    if (!SwArg->EvaluateAsRValue(ResVal, Ctx)) {
      SemaRef.Diag(SwArg->getBeginLoc(), diag::err_constant_integer_arg_type)
          << D->getDeclName() << SwArg->getSourceRange();
      return false;
    }
    Val = ResVal.Val.getInt().getLimitedValue();
    return true;
  }

  bool getMacSwitchValue(unsigned BuiltinId, CallExpr *E, unsigned &Val) {
    unsigned SwArgNo;
    switch (BuiltinId) {
    case TPC::BIs_f32_mac:
    case TPC::BIs_i8_mac:
    case TPC::BIs_u8_mac:
    case TPC::BIs_i16_mac:
    case TPC::BIs_u16_mac:
    case TPC::BIs_bf16_mac:
    case TPC::BIs_f16_mac:
    case TPC::BIs_i8_mac_acc16:
    case TPC::BIs_u8_mac_acc16:
    case TPC::BIs_bf16_mac_acc32:
    case TPC::BIs_f16_mac_acc32:
    case TPC::BIs_u8_mac_acc32:
    case TPC::BIs_u16_mac_acc32:
    case TPC::BIv_f32_mac_b:
    case TPC::BIv_f32_mac_vb:
    case TPC::BIv_bf16_mac_b:
    case TPC::BIv_bf16_mac_vb:
    case TPC::BIv_f16_mac_b:
    case TPC::BIv_f16_mac_vb:
    case TPC::BIv_i8_mac_b:
    case TPC::BIv_i8_mac_vb:
    case TPC::BIv_u8_mac_b:
    case TPC::BIv_u8_mac_vb:
    case TPC::BIv_i16_mac_b:
    case TPC::BIv_i16_mac_vb:
    case TPC::BIv_u16_mac_b:
    case TPC::BIv_u16_mac_vb:
    case TPC::BIv_bf16_mac_acc32_b:
    case TPC::BIv_bf16_mac_acc32_vb:
    case TPC::BIv_f16_mac_acc32_b:
    case TPC::BIv_f16_mac_acc32_vb:
    case TPC::BIv_i8_mac_acc16_b:
    case TPC::BIv_i8_mac_acc16_vb:
    case TPC::BIv_u8_mac_acc16_b:
    case TPC::BIv_u8_mac_acc16_vb:
    case TPC::BIv_u8_mac_acc32_b:
    case TPC::BIv_u8_mac_acc32_vb:
    case TPC::BIv_u16_mac_acc32_b:
    case TPC::BIv_u16_mac_acc32_vb:
    {
      SwArgNo = 3;
      break;
    }
    case TPC::BIv_i8_mac_zp_vb:
    case TPC::BIv_i8_mac_zp_b:
    case TPC::BIv_u8_mac_zp_vb:
    case TPC::BIv_u8_mac_zp_b:
    case TPC::BIv_i8_mac_zp_acc16_vb:
    case TPC::BIv_i8_mac_zp_acc16_b:
    case TPC::BIv_u8_mac_zp_acc16_vb:
    case TPC::BIv_u8_mac_zp_acc16_b:
    case TPC::BIv_u8_mac_zp_acc32_vb:
    case TPC::BIv_u8_mac_zp_acc32_b:
    {
      SwArgNo = 4;
      break;
    }
    case TPC::BIv_i8_mac_x2_vb:
    case TPC::BIv_i8_mac_x2_b:
    case TPC::BIv_u8_mac_x2_vb:
    case TPC::BIv_u8_mac_x2_b:
    case TPC::BIv_i8_mac_x2_acc16_vb:
    case TPC::BIv_i8_mac_x2_acc16_b:
    case TPC::BIv_u8_mac_x2_acc16_vb:
    case TPC::BIv_u8_mac_x2_acc16_b:
    case TPC::BIv_u8_mac_x2_acc32_vb:
    case TPC::BIv_u8_mac_x2_acc32_b:
    {
      SwArgNo = 5;
      break;
    }
    case TPC::BIv_i8_mac_x2_zp_vb:
    case TPC::BIv_i8_mac_x2_zp_b:
    case TPC::BIv_u8_mac_x2_zp_vb:
    case TPC::BIv_u8_mac_x2_zp_b:
    case TPC::BIv_i8_mac_x2_zp_acc16_vb:
    case TPC::BIv_i8_mac_x2_zp_acc16_b:
    case TPC::BIv_u8_mac_x2_zp_acc16_vb:
    case TPC::BIv_u8_mac_x2_zp_acc16_b:
    case TPC::BIv_u8_mac_x2_zp_acc32_vb:
    case TPC::BIv_u8_mac_x2_zp_acc32_b:
    {
      SwArgNo = 6;
      break;
    }

    default:
      return false;
    }
    ASTContext &Ctx = SemaRef.getASTContext();
    auto *D = cast<FunctionDecl>(E->getCallee()->getReferencedDeclOfCallee());
    const Expr *SwArg = E->getArg(SwArgNo);
    Expr::EvalResult ResVal;
    if (!SwArg->EvaluateAsRValue(ResVal, Ctx)) {
      SemaRef.Diag(SwArg->getBeginLoc(), diag::err_constant_integer_arg_type)
              << D->getDeclName() << SwArg->getSourceRange();
      return false;
    }
    Val = ResVal.Val.getInt().getLimitedValue();
    return true;
  }

  void processMacMaddSwitches(unsigned SwValue, unsigned BuiltinId, CallExpr *E, unsigned SwitchArgNo) {
    const unsigned ValidSwitches = llvm::TPCII::SW_SAT |
                                   llvm::TPCII::SW_NEG |
                                   llvm::TPCII::SW_ACC_FP32 /*also SW_ACC_I16*/ |
                                   llvm::TPCII::SW_ACC_I32 |
                                   llvm::TPCII::SW_X2_ARITHMETIC |
                                   llvm::TPCII::SW_ZP |
                                   llvm::TPCII::SW_NEG_ZP;
    unsigned NewSwValue = SwValue;
    const Expr *SwArg = E->getArg(SwitchArgNo);
    QualType ArgType = E->getArg(0)->getType();

    // Check switch validity.
    if (SwValue & llvm::TPCII::SW_SAT) {
      // Must be absent in the case of float types.
      if (ArgType->isFloatingType()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
          << ArgType << "SW_SAT";
        return;
      }
    }
    if ((SwValue & llvm::TPCII::SW_ACC_FP32) && ArgType->isFloatingType()) {
      // ACC_FP32 is same as ACC_I16.
      if (ArgType->getAs<BuiltinType>()->getKind() != BuiltinType::BFloat16) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
          << ArgType << "SW_ACC_I16";
        return;
      }
    }
    if (SwValue & llvm::TPCII::SW_ACC_I16 && !ArgType->isFloatingType()) {
      // ACC_I16 is same as ACC_FP32
      // Allowed only for 8-bit types.
      if (!ArgType->isCharType() && !ArgType->isChar8Type()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
          << ArgType << "SW_ACC_I16";
        return;
      }
    }
    if (SwValue & llvm::TPCII::SW_ACC_I32) {
      // Allowed only for unsigned 8 and 16-bit types.
      if (!ArgType->isIntegerType() || !ArgType->isUnsignedIntegerType()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
          << ArgType << "SW_ACC_I32";
        return;
      }
    }

    if (SwValue & llvm::TPCII::SW_X2_ARITHMETIC) {
      // Allowed only for Vector 8-bit types.
      if (!ArgType->isCharType() && !ArgType->isChar8Type() && !ArgType->isVectorType()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
                << ArgType << "SW_X2_MAC";
        return;
      }
    }
    if (SwValue & llvm::TPCII::SW_ZP) {
      // Allowed only for Vector 8-bit types.
      if (!ArgType->isCharType() && !ArgType->isChar8Type() && !ArgType->isVectorType()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
                << ArgType << "SW_ZP";
        return;
      }
    }

    if (SwValue & llvm::TPCII::SW_NEG_ZP) {
      // Allowed only for Vector 8-bit types.
      if (!ArgType->isCharType() && !ArgType->isChar8Type() && !ArgType->isVectorType()) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_type_incompatible_with_switch)
                << ArgType << "SW_NEG_ZP";
        return;
      }
    }

    // Check switch compatibility.
    if ((NewSwValue & llvm::TPCII::SW_ACC_I32) && (NewSwValue & llvm::TPCII::SW_ACC_I16)) {
      SemaRef.Diag(SwArg->getBeginLoc(), diag::err_conflicting_switches)
        << "SW_ACC_I32" << "SW_ACC_I16";
      return;
    }
    if (NewSwValue & ~ValidSwitches) {
      std::stringstream ss;
      ss << std::hex << "0x" << (SwValue & ~ValidSwitches);
      SemaRef.Diag(SwArg->getBeginLoc(), diag::err_invalid_switches) << ss.str();
      return;
    }

    // Update switches if necessary.
    if (SwValue != NewSwValue) {
      llvm::APInt Val(32, NewSwValue);
      E->setArg(SwitchArgNo,
                IntegerLiteral::Create(SemaRef.getASTContext(), Val, SwArg->getType(), SwArg->getBeginLoc()));
    }
  }

  void processMacSwitches(unsigned SwValue, unsigned BuiltinId, CallExpr *E) {
    processMacMaddSwitches(SwValue, BuiltinId, E, E->getNumArgs() - 3);
  }

  void processMaddSwitches(unsigned SwValue, unsigned BuiltinId, CallExpr *E) {
    processMacMaddSwitches(SwValue, BuiltinId, E, E->getNumArgs() - 4);
  }

  bool getMulSwitchValue(unsigned BuiltinId, CallExpr *E, unsigned &Val) {
    switch (BuiltinId) {
    case TPC::BIs_f32_mul:
    case TPC::BIs_i32_mul:
    case TPC::BIs_u32_mul:
    case TPC::BIs_i16_mul:
    case TPC::BIs_u16_mul:
    case TPC::BIs_i8_mul:
    case TPC::BIs_u8_mul:
    case TPC::BIs_bf16_mul:
    case TPC::BIs_bf16_mul_acc32:
    case TPC::BIs_f16_mul:
    case TPC::BIs_f16_mul_acc32:
    case TPC::BIi_i32_mul:
    {
      ASTContext &Ctx = SemaRef.getASTContext();
      auto *D = cast<FunctionDecl>(E->getCallee()->getReferencedDeclOfCallee());
      const Expr *SwArg = E->getArg(2);
      Expr::EvalResult ResVal;
      if (!SwArg->EvaluateAsRValue(ResVal, Ctx)) {
        SemaRef.Diag(SwArg->getBeginLoc(), diag::err_constant_integer_arg_type)
          << D->getDeclName() << SwArg->getSourceRange();
        return false;
      }
      Val = ResVal.Val.getInt().getLimitedValue();
      return true;
    }
    default:
      return false;
    }
  }

  bool isInstrWithMask(unsigned BuiltinId, CallExpr *E, unsigned &OpNum) {
    switch (BuiltinId) {
    case TPC::BIi_i32_mov:
      OpNum = 1;
      return true;
    default:
      return false;
    }
  }

  void checkIntrinsicArguments(unsigned BuiltinId, CallExpr *E) {
    unsigned SwValue;
    if (getMacSwitchValue(BuiltinId, E, SwValue))
      processMacSwitches(SwValue, BuiltinId, E);
    if (getMaddSwitchValue(BuiltinId, E, SwValue))
      processMaddSwitches(SwValue, BuiltinId, E);

    unsigned MaskOp;
    if (isInstrWithMask(BuiltinId, E, MaskOp)) {
      assert(MaskOp < E->getNumArgs());
      assert(E->getArg(MaskOp)->getType()->isIntegerType());
/* Temporarily, until constant arguments will be repared.
      if (TOptions.CPU != "greco" && TOptions.CPU != "gaudi2") {
        // Mask must be constant on old architectures.
        Expr *Mask = E->getArg(MaskOp);
        SourceLocation Loc;
        llvm::APSInt MaskValue;
        if (!Mask->isIntegerConstantExpr(MaskValue, getContext(), &Loc)) {
          auto *FD = cast<FunctionDecl>(E->getCallee()->getReferencedDeclOfCallee());
          SemaRef.Diag(Loc, diag::err_constant_integer_arg_type)
              << FD->getDeclName() << Mask->getSourceRange();
        }
      }
*/
    }
  }

  bool isTypeSupportedInDivRem(QualType Ty) {
    assert(Ty->isBuiltinType() || Ty->isVectorType());
    bool IsVector = Ty->isVectorType();
    QualType ScalarTy;
    if (IsVector)
      ScalarTy = Ty->getAs<VectorType>()->getElementType().getCanonicalType();
    else
      ScalarTy = Ty.getCanonicalType();
    if (ScalarTy == getContext().FloatTy ||
        ScalarTy == getContext().IntTy ||
        ScalarTy == getContext().UnsignedIntTy)
      return true;
    if (IsVector)
      return ScalarTy == getContext().ShortTy ||
             ScalarTy == getContext().UnsignedShortTy ||
             ScalarTy == getContext().CharTy ||
             ScalarTy == getContext().UnsignedCharTy;
    return false;
  }

public:
  RC99Scanner(Sema &S)
    : SemaRef(S),
      TOptions(SemaRef.getASTContext().getTargetInfo().getTargetOpts()) {
  }

  bool VisitVarDecl(VarDecl *VD) {
    QualType Ty = VD->getType();

    // Prohibit global variables that points to a data in global
    // addressspace - we cannot lower them in -O0 mode.
    if (VD->getDeclContext()->isFileContext() ||
        VD->isStaticDataMember() ||
        VD->getStorageClass() == SC_Static) {
      if (Ty.getAddressSpace() == LangAS::rc99_global) {
        SemaRef.Diag(VD->getLocation(), diag::err_var_in_glob_addrspace);
        return false;
      }
      if (Ty->isPointerType()) {
        const PointerType *PT = Ty->getAs<PointerType>();
        QualType PointeeT = PT->getPointeeType();
        if (PointeeT.getAddressSpace() == LangAS::rc99_global) {
          SemaRef.Diag(VD->getLocation(), diag::err_global_ptr_to_glob_addrspace);
          return false;
        }
      }
    }

    checkGlobalPtrType(Ty, VD->getLocation());

    // Prohibit aggregates containing short types.
    if (Ty->isArrayType() && containsShortValue(Ty) == AlignmentConflict) {
      SemaRef.Diag(VD->getLocation(), diag::err_aggregate_of_short_values);
      return true;
    }

    if (VD->hasInit()) {
      Expr *Init = VD->getInit();
      if (isa<InitListExpr>(Init))
        checkNonZeroElement(VD->getType(), Init, false);
    }
    return true;
  }

  bool VisitCastExpr(CastExpr *E) {
    Expr *Src = E->getSubExpr();
    QualType SrcTy = Src->getType();
    QualType DstTy = E->getType();
    SourceLocation DiagLoc;

    if (auto ECast = dyn_cast<ExplicitCastExpr>(E)) {
      DiagLoc = ECast->getTypeInfoAsWritten()->getTypeLoc().getEndLoc();
    } else {
      DiagLoc = E->getBeginLoc();
    }

    if (isGloabLPtr(SrcTy)) {
      // For global pointers only bitcasts to other global pointers are allowed.
      if (E->getCastKind() != CastKind::CK_NoOp &&
          E->getCastKind() != CastKind::CK_BitCast &&
          E->getCastKind() != CastKind::CK_LValueToRValue) {
        SemaRef.Diag(DiagLoc, diag::err_unsupported_op_on_gptr);
        return true;
      }
      if (!isGloabLPtr(DstTy)) {
        SemaRef.Diag(DiagLoc, diag::err_unsupported_op_on_gptr);
        return true;
      }
      checkGlobalPtrType(SrcTy, DiagLoc);
    }
    else if (isGloabLPtr(DstTy)) {
      if (E->getCastKind() != CastKind::CK_NullToPointer)
        SemaRef.Diag(DiagLoc, diag::err_unsupported_op_on_gptr);
    }

    return true;
  }

  // Checks if the provided operator acts on operand of boolX type, and if it is
  // so, emit error.
  bool checkBool256Operands(const BinaryOperator *BO) {
    const Expr *LHS = BO->getLHS();
    if (isBoolX(LHS->getType())) {
      SemaRef.Diag(BO->getExprLoc(), diag::err_invalid_operation_on_bool256)
          << BO->getSourceRange();
      return false;
    }
    return true;
  }

  bool checkBool256Operands(const UnaryOperator *UO) {
    Expr *Op = UO->getSubExpr();
    if (isBoolX(Op->getType())) {
      SemaRef.Diag(UO->getExprLoc(), diag::err_invalid_operation_on_bool256)
        << UO->getSourceRange();
      return false;
    }
    return true;
  }

  bool checkInt5Access(const ArraySubscriptExpr *ASE, int &IndexVal) {
    // Subscript must be an integer constant.
    const Expr *Index = ASE->getIdx();
    Expr::EvalResult EvalResult;
    bool Result = Index->EvaluateAsRValue(EvalResult, SemaRef.getASTContext());
    if (!Result) {
      SemaRef.Diag(Index->getBeginLoc(), diag::err_non_constant_index)
        << Index->getSourceRange();
      return false;
    }
    if (!EvalResult.Val.isInt()) {
      SemaRef.Diag(Index->getBeginLoc(), diag::err_non_constant_index)
        << Index->getSourceRange();
      return false;
    }
    llvm::APSInt &V = EvalResult.Val.getInt();
    if (V.isNegative() || V >= 5) {
      SemaRef.Diag(Index->getBeginLoc(), diag::err_index_out_of_range)
        << Index->getSourceRange();
      return false;
    }
    IndexVal = V.getLimitedValue();
    return true;
  }

  const ArraySubscriptExpr *getLHSAccessToInt5(const Expr *E) {
    const Expr *LHS = E->IgnoreParenLValueCasts();
    if (auto ASE = dyn_cast<ArraySubscriptExpr>(LHS))
      if (isIndexType(ASE->getBase()->getType()))
        return ASE;
    return nullptr;
  }

  const ArraySubscriptExpr *getRHSAccessToInt5(const Expr *E) {
    const Expr *LHS = E->IgnoreParenCasts();
    if (auto ASE = dyn_cast<ArraySubscriptExpr>(LHS))
      if (isIndexType(ASE->getBase()->getType()))
        return ASE;
    return nullptr;
  }

  bool TraverseBinOperatorDescendants(BinaryOperator *E) {
    assert(SemaRef.getLangOpts().LongIRF);

    Int5CheckResetter R(*this);

    if (auto *ASE = getRHSAccessToInt5(E->getLHS())) {
      int IndexVal;
      if (!checkInt5Access(ASE, IndexVal))
        return true;
      if (R.getOldAllowInt5Access()) {
        if (R.getOldInt5Dim() != (unsigned)IndexVal) {
          SemaRef.Diag(ASE->getBeginLoc(), diag::err_different_int5_dims)
            << ASE->getSourceRange();
          return true;
        }
      }
      Int5Dim = IndexVal;
      AllowInt5Access = true;
      NeedCheckInt5 = true;
    }

    return TraverseStmt(E->getLHS()) && TraverseStmt(E->getRHS());
  }

  bool TraverseBinCompareOperatorDescendants(BinaryOperator *E) {
    assert(SemaRef.getLangOpts().LongIRF);

    Int5CheckResetter R(*this);

    if (auto *ASE = getRHSAccessToInt5(E->getLHS())) {
      int IndexVal;
      if (!checkInt5Access(ASE, IndexVal))
        return true;
      Int5Dim = IndexVal;
      AllowInt5Access = true;
      NeedCheckInt5 = true;
    }

    return TraverseStmt(E->getLHS()) && TraverseStmt(E->getRHS());
  }

  bool TraverseCompoundAssignOperator(CompoundAssignOperator *CAO) {
    if (SemaRef.getLangOpts().LongIRF) {
      if (!WalkUpFromBinaryOperator(CAO))
        return false;

      Int5CheckResetter R(*this);

      if (auto *ASE = getLHSAccessToInt5(CAO->getLHS())) {
        int IndexVal;
        if (!checkInt5Access(ASE, IndexVal))
          return true;
        Int5Dim = IndexVal;
        AllowInt5Access = true;
        NeedCheckInt5 = true;
      }
      return TraverseStmt(CAO->getLHS()) && TraverseStmt(CAO->getRHS());
    }
    return BaseType::TraverseCompoundAssignOperator(CAO);
  }

  bool TraverseBinaryOperator(BinaryOperator *BO) {
    switch (BO->getOpcode()) {
    default:
      break;
    case BinaryOperator::Opcode::BO_Assign:
      if (SemaRef.getLangOpts().LongIRF) {
        Int5CheckResetter R(*this);

        if (!WalkUpFromBinaryOperator(BO))
          return false;

        if (auto *ASE = getLHSAccessToInt5(BO->getLHS())) {
          int IndexVal;
          if (!checkInt5Access(ASE, IndexVal))
            return true;
          Int5Dim = IndexVal;
          AllowInt5Access = true;
          NeedCheckInt5 = true;
        }

        return TraverseStmt(BO->getLHS()) && TraverseStmt(BO->getRHS());
      }
      break;
    case BinaryOperator::Opcode::BO_Add:
    case BinaryOperator::Opcode::BO_Mul:
      if (!SemaRef.getLangOpts().LongIRF)
        return BaseType::TraverseBinaryOperator(BO);
      if (!WalkUpFromBinaryOperator(BO))
        return false;
      return TraverseBinCompareOperatorDescendants(BO);
    case BinaryOperator::Opcode::BO_EQ:
    case BinaryOperator::Opcode::BO_NE:
    case BinaryOperator::Opcode::BO_LT:
    case BinaryOperator::Opcode::BO_LE:
    case BinaryOperator::Opcode::BO_GT:
    case BinaryOperator::Opcode::BO_GE:
    case BinaryOperator::Opcode::BO_Cmp:
      if (SemaRef.getLangOpts().LongIRF) {
        if (!WalkUpFromBinaryOperator(BO))
          return false;
        return TraverseBinOperatorDescendants(BO);
      }
      break;
    }
    return BaseType::TraverseBinaryOperator(BO);
  }

  bool VisitBinaryOperator(BinaryOperator *BO) {
    switch (BO->getOpcode()) {
    default:
      break;
    case BinaryOperator::Opcode::BO_Assign: {
      Expr *LHS = BO->getLHS()->IgnoreParenCasts();
      Expr *RHS = BO->getRHS();

      // Check for global ptr assign.
      Expr *RHSPeeled = RHS->IgnoreParenCasts();
      bool CheckForGPtr = true;

      // Do not report error for code like:
      //
      //   ptr = (__global int *) a_gen_addr_i(c0, out);
      //
      if (auto Call = dyn_cast<CallExpr>(RHSPeeled))
        if (auto RefExpr = dyn_cast<DeclRefExpr>(Call->getCallee()->IgnoreImpCasts()))
          if (auto Func = dyn_cast<FunctionDecl>(RefExpr->getDecl()))
            if (Func->getBuiltinID())
              CheckForGPtr = false;

      // Do not report error for code like:
      //
      //   ptr = (__global int *)12;
      //
      // Error for this code will be issued when processing cast. However report
      // error for constructs like:
      //
      //   ptr = (__global int *)0;
      //
      // now, because the cast is valid in this case.
      //
      if (auto Cast = dyn_cast<CastExpr>(RHS->IgnoreParens())) {
        if (Cast->getCastKind() != CastKind::CK_NullToPointer &&
            Cast->getCastKind() != CastKind::CK_LValueToRValue)
          CheckForGPtr = false;
      }
      if (CheckForGPtr)
        checkNotGlobalPtr(LHS, BO->getExprLoc()) &&
        checkNotGlobalPtr(RHS, BO->getExprLoc());
      break;
    }
    case BinaryOperator::Opcode::BO_Add:
    case BinaryOperator::Opcode::BO_Sub:
      // Catch operations on global pointer, like 'ptr + 3'. TPC does not have
      // instructions to carry out such computations.
      checkNotGlobalPtr(BO->getLHS(), BO->getExprLoc()) &&
        checkNotGlobalPtr(BO->getRHS(), BO->getExprLoc());
      // Catch operation on boolX operands.
      checkBool256Operands(BO);
      break;
    case BinaryOperator::Opcode::BO_Mul:
    case BinaryOperator::Opcode::BO_MulAssign:
      if (checkBool256Operands(BO))
        checkMulValidity(BO);
      break;
    case BinaryOperator::Opcode::BO_AddAssign:
    case BinaryOperator::Opcode::BO_SubAssign:
      checkNotGlobalPtr(BO->getLHS(), BO->getExprLoc());
      checkBool256Operands(BO);
      break;
    case BinaryOperator::Opcode::BO_LT:
    case BinaryOperator::Opcode::BO_LE:
    case BinaryOperator::Opcode::BO_GT:
    case BinaryOperator::Opcode::BO_GE:
    case BinaryOperator::Opcode::BO_Cmp:
      checkBool256Operands(BO);
      LLVM_FALLTHROUGH;
    case BinaryOperator::Opcode::BO_EQ:
    case BinaryOperator::Opcode::BO_NE:
      checkNotGlobalPtr(BO->getLHS(), BO->getExprLoc()) &&
        checkNotGlobalPtr(BO->getRHS(), BO->getExprLoc());
      break;
    case BinaryOperator::Opcode::BO_Shl:
    case BinaryOperator::Opcode::BO_Shr:
    case BinaryOperator::Opcode::BO_ShlAssign:
    case BinaryOperator::Opcode::BO_ShrAssign:
      if (SemaRef.CheckNotIndexType(BO->getLHS(), BO->getExprLoc()))
        SemaRef.CheckNotIndexType(BO->getRHS(), BO->getExprLoc());
      checkBool256Operands(BO);
      break;
    case BinaryOperator::Opcode::BO_Div:
    case BinaryOperator::Opcode::BO_Rem: {
      if (checkBool256Operands(BO)) {
        const Expr *LHS = BO->getLHS();
        const Expr *RHS = BO->getRHS();
        ASTContext &Ctx = SemaRef.getASTContext();

        if (LHS->isCXX98IntegralConstantExpr(Ctx) &&
            RHS->isCXX98IntegralConstantExpr(Ctx))
          break;

        Expr::EvalResult ResVal;
        if (BO->EvaluateAsRValue(ResVal, Ctx))
          break;

        Expr::EvalResult RHSVal;
        if (RHS->EvaluateAsRValue(RHSVal, Ctx))
          break;

        if (!isTypeSupportedInDivRem(BO->getType()))
          SemaRef.Diag(BO->getExprLoc(), diag::err_divrem);
      }
      break;
    }
    case BinaryOperator::Opcode::BO_DivAssign:
    case BinaryOperator::Opcode::BO_RemAssign:
      if (checkBool256Operands(BO)) {
        if (!isTypeSupportedInDivRem(BO->getType()))
          SemaRef.Diag(BO->getExprLoc(), diag::err_divrem);
      }
      break;
    case BinaryOperator::Opcode::BO_LAnd:
    case BinaryOperator::Opcode::BO_LOr:
      if (SemaRef.CheckNotIndexType(BO->getLHS(), BO->getExprLoc()) &&
          SemaRef.CheckNotIndexType(BO->getRHS(), BO->getExprLoc())) {
        checkBool256Operands(BO);
      }
      break;
    }
    return BaseType::VisitBinaryOperator(BO);
  }

  bool VisitCallExpr(CallExpr *E) {
    if (Decl *D = E->getCallee()->getReferencedDeclOfCallee())
      if (const auto *FD = dyn_cast<FunctionDecl>(D)) {
        unsigned BuiltinId = FD->getBuiltinID();
        if (BuiltinId != 0) {
          checkIntrinsicArguments(BuiltinId, E);

          // Check printf. We need to bypass index checking.
          if (isPrintFWithValue(BuiltinId)) {
            Expr *Val = E->getArg(1)->IgnoreParenCasts();
            if (auto ASE = dyn_cast<ArraySubscriptExpr>(Val)) {
              SubscrInPrintF.insert(ASE);
            }
          }
        }
      }
    return true;
  }

  bool VisitGenericSelectionExpr(GenericSelectionExpr *GS) {
    if (auto ASE = dyn_cast<ArraySubscriptExpr>(GS->getControllingExpr()->IgnoreParenCasts())) {
      SubscrInPrintF.insert(ASE);
    }
    return true;
  }

  bool VisitArraySubscriptExpr(ArraySubscriptExpr *ASE) {
    // Prohibit indexed access to global data.
    checkNotGlobalPtr(ASE->getBase(), ASE->getExprLoc()) &&
      checkNotGlobalPtr(ASE->getIdx(), ASE->getExprLoc());

    // Indexed access in printf is implemented in a special way.
    if (SubscrInPrintF.find(ASE) != SubscrInPrintF.end())
      return true;

    // Make checks depending on base type.
    QualType LHSTy = ASE->getBase()->getType();
    if (LHSTy->isVectorType()) {
      if (isInTPCVectorAddressSpace(LHSTy)) {
        SemaRef.Diag(ASE->getExprLoc(), diag::err_cant_access_vector_element)
          << ASE->getBase()->getSourceRange() << LHSTy.getLocalUnqualifiedType();
      } else {
        // The vector type is not in vector address space, it must be int5.
        const auto *VType = LHSTy->getAs<VectorType>();
        (void)VType;
        assert(isIndexType(LHSTy));

        int IndexVal;
        checkInt5Access(ASE, IndexVal);

        // If support for irf44 is in effect, indexed access is allowed only in
        // certain contexts.
        if (SemaRef.getLangOpts().LongIRF) {
          if (AllowInt5Access) {
            if (NeedCheckInt5 && Int5Dim != (unsigned)IndexVal) {
              SemaRef.Diag(ASE->getBeginLoc(), diag::err_different_int5_dims)
                << ASE->getSourceRange();
              return true;
            }
          } else {
            SemaRef.Diag(ASE->getBeginLoc(), diag::err_unsupported_int5_access)
              << ASE->getSourceRange();
            return true;
          }
        }
      }
    }
    return true;
  }

  bool TraverseUnaryOperatorDescendants(UnaryOperator *E) {
    assert(SemaRef.getLangOpts().LongIRF);

    Int5CheckResetter R(*this);

    if (auto *ASE = getLHSAccessToInt5(E->getSubExpr())) {
      int IndexVal;
      if (!checkInt5Access(ASE, IndexVal))
        return true;
      AllowInt5Access = true;
      NeedCheckInt5 = false;
    }

    if (!TraverseStmt(E->getSubExpr()))
      return false;
    return true;
  }

  bool TraverseUnaryOperator(UnaryOperator *UO) {
    if (SemaRef.getLangOpts().LongIRF) {
      if (!WalkUpFromUnaryOperator(UO))
        return false;
      return TraverseUnaryOperatorDescendants(UO);
    }
    return BaseType::TraverseUnaryOperator(UO);
  }

  bool VisitUnaryOperator(UnaryOperator *UO) {
    switch (UO->getOpcode()) {
    default:
      break;
    case UnaryOperator::Opcode::UO_PostInc:
      if (TOptions.CPU != "goya2" && TOptions.CPU != "greco" &&
          TOptions.CPU != "gaudi2" && TOptions.CPU != "doron1")
        checkNotGlobalPtr(UO->getSubExpr(), UO->getExprLoc());
      checkBool256Operands(UO);
      break;
    case UnaryOperator::Opcode::UO_PreInc:
    case UnaryOperator::Opcode::UO_PreDec:
    case UnaryOperator::Opcode::UO_PostDec:
    case UnaryOperator::Opcode::UO_Minus:
    case UnaryOperator::Opcode::UO_Plus:
    case UnaryOperator::Opcode::UO_LNot:
      checkNotGlobalPtr(UO->getSubExpr(), UO->getExprLoc());
      checkBool256Operands(UO);
      break;
    }
    return BaseType::VisitUnaryOperator(UO);
  }
};


class RC99TranslationUnitScanner : public RecursiveASTVisitor<RC99TranslationUnitScanner> {
  Sema &SemaRef;
public:
  RC99TranslationUnitScanner(Sema &S) : SemaRef(S) {
    assert(S.getLangOpts().RC99);
  }

  bool VisitRecordDecl(RecordDecl *RD) {
    // Check presence of short values.
    QualType RecType(RD->getTypeForDecl(), 0);
    if (containsShortValue(RecType) == AlignmentConflict) {
      SemaRef.Diag(RD->getLocation(), diag::err_aggregate_of_short_values);
    }
    bool HasScalar = false;
    bool HasVector = false;

    // Check that fields are from the same address space.
    for (const auto *F : RD->fields()) {
      if (isInTPCVectorAddressSpace(F->getType()))
        HasVector = true;
      else
        HasScalar = true;
      if (HasVector && HasScalar) {
        SemaRef.Diag(F->getLocation(), diag::err_incompatible_addr_space);
      }
    }

    return true;
  }
};

}

bool Sema::CheckNotIndexType(Expr *E, SourceLocation Loc) {
  if (isIndexType(E->getType())) {
    Diag(Loc, diag::err_unallowed_operation_for_int5);
      return false;
  }
  return true;
}

bool Sema::checkBeforeCodegen(Decl *D) {
  if (!getLangOpts().RC99)
    return true;

  // For now skip anything other than function declarations, function template
  // declarations and variable declarations. Type declarations (like 'bool256')
  // do not need special treatment.
  if (!isa<FunctionDecl>(D) && !isa<FunctionTemplateDecl>(D) && !isa<VarDecl>(D))
    return true;

  // If source file does not contain intrinsic calls, TPC types may be absent,
  // so load them now.
  Context.loadTpcTypes();

  if (!getLangOpts().CompileForLibrary) {
    if (auto *FD = dyn_cast<FunctionDecl>(D)) {
      if (FD->getDeclContext()->isTranslationUnit() &&
          FD->getName() == LangOpts.MainFunction) {
        FD->addAttr(OpenCLKernelAttr::CreateImplicit(Context));
      } else {
        if ((FD->getStorageClass() != SC_Extern) &&
            FD->isThisDeclarationADefinition() &&
            !FD->getName().startswith("__")) {
          FD->addAttr(AlwaysInlineAttr::CreateImplicit(Context));
          FD->setInlineSpecified(true);
        }
      }
    } else if (auto FTD = dyn_cast<FunctionTemplateDecl>(D)) {
      FunctionDecl *FD = FTD->getTemplatedDecl();
      FD->addAttr(AlwaysInlineAttr::CreateImplicit(Context));
      FD->setInlineSpecified(true);
    }
  }

  RC99Scanner Scanner(*this);
  Scanner.TraverseDecl(D);

  return true;
}

bool Sema::isPrintfEnabled(FunctionDecl *FDecl) {
  DeclarationName MemberName = FDecl->getDeclName();
  IdentifierInfo *Member = MemberName.getAsIdentifierInfo();
  if (Member->getName().startswith("printf_")) {
    return TpcPrintfPragmaIsOn || LangOpts.UsePrintf;
  }
  return true;
}

void Sema::checkTranslationUnin() {
  // Check if main function is present.
  FunctionDecl *MainFunc = nullptr;
  if (!getLangOpts().CompileForLibrary) {
    IdentifierInfo *II = PP.getIdentifierInfo(getLangOpts().MainFunction);
    DeclarationName DN(II);
    DeclarationNameInfo DNI(DN, SourceLocation());
    LookupResult R(*this, DNI, LookupOrdinaryName);
    LookupName(R, TUScope);
    if (R.getResultKind() != LookupResult::Found ||
        !isa<FunctionDecl>(R.getFoundDecl())) {
      Diag(SourceLocation(), diag::err_main_not_found);
      return;
    }
    MainFunc = cast<FunctionDecl>(R.getFoundDecl());

    // Check if main function has correct prototype.
    QualType T = MainFunc->getType();
    assert(T->isFunctionType() && "function decl is not of function type");
    auto *FT = T->castAs<FunctionType>();
    if (!Context.hasSameType(FT->getReturnType(), Context.VoidTy))
      Diag(MainFunc->getTypeSpecStartLoc(), diag::warn_tpc_not_void_function);
    if (MainFunc->isInlineSpecified())
      Diag(MainFunc->getBeginLoc(), diag::err_inline_entry_function)
        << FixItHint::CreateRemoval(MainFunc->getBeginLoc());

    if (!isa<FunctionNoProtoType>(FT)) {
      auto *FTP = cast<const FunctionProtoType>(FT);
      unsigned nparams = FTP->getNumParams();
      assert(MainFunc->getNumParams() == nparams);
      for (unsigned i = 0; i < nparams; ++i) {
        QualType AT = FTP->getParamType(i);
        if (!AT->isBuiltinType()) {
          Diag(MainFunc->getParamDecl(i)->getLocation(),
               diag::err_main_type_of_arg_wrong)
            << MainFunc->getParamDecl(i)->getNameAsString();
          MainFunc->setInvalidDecl(true);
        }
      }
    }
  }

  // If 'printf' is used, check if there is available tensor for printf.
  if (TpcPrintfIsUsed) {
    unsigned NumAvailableTensors = getLangOpts().NumTensors;
    if (!ExtraParams.empty())
      --NumAvailableTensors;
    if (TpcPrintfIsUsed)
      --NumAvailableTensors;
    if (Tensors.size() > NumAvailableTensors) {
      Diag(Tensors[NumAvailableTensors]->getLocation(),
           diag::err_tensors_num_exceed_limit);
    }
  }

  RC99TranslationUnitScanner Scanner(*this);
  if (!Scanner.TraverseDecl(Context.getTranslationUnitDecl()))
    return;

  // create __initialize function definition.
  if (MainFunc) {
    SmallVector<Stmt*, 32> InitStmts;

    auto FnBody = CompoundStmt::Create(Context, InitStmts, MainFunc->getBeginLoc(), MainFunc->getBeginLoc());
    IdentifierInfo *III = PP.getIdentifierInfo("__initialize");
    DeclarationName DN1(III);
    DeclarationNameInfo DNII(DN1, SourceLocation());
    LookupResult RI(*this, DNII, LookupOrdinaryName);
    LookupName(RI, TUScope);
    assert(RI.getResultKind() == LookupResult::Found);
    FunctionDecl *FDI = cast<FunctionDecl>(RI.getFoundDecl());
    FDI->setBody(FnBody);
    Consumer.HandleTopLevelDecl(DeclGroupRef(FDI));
  }
}

VarDecl *Sema::ActOnTensorDeclarator(VarDecl *TensorDecl, unsigned TensorNumber,
                                     bool ExtraArgsPresent) {
  // Number of tensors available for user. Not not count tensor for 'printf', as
  // during processing of declaration of 'main' it is not known if 'printf' is
  // used.
  unsigned NumAvailableTensors = getLangOpts().NumTensors;
  if (ExtraArgsPresent)
    --NumAvailableTensors;

  if (TensorNumber == NumAvailableTensors) {
    Diag(TensorDecl->getLocation(), diag::err_tensors_num_exceed_limit);
  }

  Expr *Init = ActOnIntegerConstant(TensorDecl->getLocation(), TensorNumber).get();
  SourceLocation Loc = TensorDecl->getLocation();
  SourceLocation LocStart = TensorDecl->getBeginLoc();
  DeclarationName Name = TensorDecl->getDeclName();
  IdentifierInfo *II = Name.getAsIdentifierInfo();
  QualType Type = Init->getType();
  Type.addConst();
  TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
  VarDecl *NewVD = VarDecl::Create(getASTContext(), TU, LocStart, Loc, II, Type, nullptr, SC_None);
  AddInitializerToDecl(NewVD, Init, false);
  DeclContext *SaveCtx = CurContext;
  CurContext = TU;
  NewVD->setDeclContext(CurContext);
  PushOnScopeChains(NewVD, TUScope, true);
  CurContext = SaveCtx;

  assert(Tensors.size() == TensorNumber &&
         "Invalid sequence of tensor declarations");
  Tensors.push_back(NewVD);

  return NewVD;
}

/// Create declaration statement for shadow variable of extra parameter.
///
/// arg20 = *pointer;
///
DeclStmt *Sema::ProcessExtraArgs(ParmVarDecl *VD, VarDecl *Ptr) {
  assert(cast<FunctionDecl>(CurContext)->getName().equals(getLangOpts().MainFunction));

  SourceLocation Loc = VD->getLocation();
  SourceLocation LocStart = VD->getBeginLoc();
  CXXScopeSpec SS;
  DeclarationName Name = VD->getDeclName();
  IdentifierInfo *II = Name.getAsIdentifierInfo();
  VarDecl *NewVD = VarDecl::Create(getASTContext(), CurContext, LocStart, Loc, II, VD->getType(), nullptr, SC_None);
  QualType Ty = Ptr->getType();
  DeclRefExpr *Idx = DeclRefExpr::Create(getASTContext(), NestedNameSpecifierLoc(), SourceLocation(),
    Ptr, false, SourceLocation(), Ty, VK_LValue);
  ExprResult Res = ImplicitCastExpr::Create(getASTContext(), Ty, CK_LValueToRValue, Idx, nullptr, VK_RValue, FPOptionsOverride());
  QualType Type = VD->getType();
  if (!(Type == Context.UnsignedIntTy ||
    Type == Context.IntTy)) {
    Type = Context.getAddrSpaceQualType(Type, LangAS::rc99_global);
    Res = ImplicitCastExpr::Create(getASTContext(), Context.getPointerType(Type), CK_BitCast, Res.get(), nullptr, VK_RValue, FPOptionsOverride());
  }
  Res = CreateBuiltinUnaryOp(SourceLocation(), UO_Deref, Res.get());
  AddInitializerToDecl(NewVD, Res.get(), false);
  PushOnScopeChains(NewVD, getCurScope(), true);
  return new (Context) DeclStmt(DeclGroupRef(NewVD), LocStart, Loc);
}

/// Create tensor indices for access to extra main arguments.
///
/// int5 hiddenTensorOffset (0,0,0,0,0);
///
VarDecl *Sema::CreateTensorOffsetForExtraArgs() {
  // Find declaration of "int5".
  IdentifierInfo *II = PP.getIdentifierInfo("int5");
  DeclarationName DN(II);
  SourceLocation Loc = SourceLocation(); // TODO: use main location
  DeclarationNameInfo DNI(DN, Loc);
  LookupResult R(*this, DNI, LookupOrdinaryName);
  LookupName(R, TUScope);
  assert(R.getResultKind() == LookupResult::Found);
  Decl *FD = cast<Decl>(R.getFoundDecl());
  QualType Type;
  if (auto TDD = dyn_cast<TypedefDecl>(FD))
    Type = TDD->getTypeSourceInfo()->getType() ;
  assert(!Type.isNull());

  // int5 hiddenTensorOffset (0,0,0,0,0);
  SmallVector<Expr*, 5> InitExprs;
  II = PP.getIdentifierInfo("HiddenTensorOffset");

  VarDecl *NewVD = VarDecl::Create(getASTContext(), CurContext, Loc, Loc, II, Type, nullptr, SC_None);
  Expr *Init = ActOnIntegerConstant(Loc, 0).get();
  for (unsigned int i = 0; i<5; i++)
    InitExprs.push_back(Init);
  InitListExpr *E = new (Context) InitListExpr(Context, Loc, InitExprs, Loc);
  E->setType(Type); 
  AddInitializerToDecl(NewVD, E, false);

//  NewVD->setDeclContext(CurContext);                  
//  PushOnScopeChains(NewVD, getCurScope(), true);
  return NewVD;
}

/// Get global address of the extra argument.
///
/// __global void* pointer = gen_addr(15, hiddenTensorOffset, 0, 0, 1, 0);
///
Stmt *Sema::getArgumentAddress(VarDecl *Ptr, VarDecl *TensorOffset) {
  SourceLocation Loc = SourceLocation(); // TODO: use main location

  // Fins intrinsic function.
  IdentifierInfo *II = PP.getIdentifierInfo("gen_addr");
  DeclarationName DN(II);
  DeclarationNameInfo DNI(DN, Loc);
  LookupResult R(*this, DNI, LookupOrdinaryName);
  LookupName(R, CurScope, true /*AllowBuiltinCreation*/);
  assert(R.getResultKind() == LookupResult::Found);
  CXXScopeSpec SS;
  ExprResult IntrinsicFuncPtr = BuildDeclarationNameExpr(SS, R, 0);
  assert(IntrinsicFuncPtr.isUsable());
  QualType ResultTy = getASTContext().VoidTy;
  Qualifiers Q = ResultTy.getQualifiers();
  Q.setAddressSpace(LangAS::rc99_global);
  ResultTy = getASTContext().getQualifiedType(ResultTy, Q);
  ResultTy = getASTContext().getPointerType(ResultTy);

  // Get reference to index set (hiddenTensorOffset).
  QualType Ty = TensorOffset->getType();
  DeclRefExpr *Idx = DeclRefExpr::Create(getASTContext(), NestedNameSpecifierLoc(), Loc, 
                                         TensorOffset, false, Loc, Ty, VK_LValue);

  // Call the intrinsic: gen_address(15, hiddenTensorOffset, 0, out_ptr, true, true).
  SmallVector<Expr*, 5> Args;
  Expr *Init = ActOnIntegerConstant(Loc, getLangOpts().NumTensors - 1).get();
  Args.push_back(Idx);
  Args.push_back(Init);
  Args.push_back(IntegerLiteral::Create(getASTContext(), llvm::APInt::getNullValue(32), getASTContext().IntTy, Loc));
  Args.push_back(new (getASTContext()) CXXNullPtrLiteralExpr(ResultTy, Ptr->getLocation()));
  Args.push_back(new (getASTContext()) CXXBoolLiteralExpr(true, getASTContext().BoolTy, Ptr->getLocation()));
  Args.push_back(new (getASTContext()) CXXBoolLiteralExpr(false, getASTContext().BoolTy, Ptr->getLocation()));
  ExprResult RHS = ActOnCallExpr(getCurScope(), IntrinsicFuncPtr.get(), Loc, Args, Loc, 0);
  DeclRefExpr *LHS = DeclRefExpr::Create(getASTContext(), NestedNameSpecifierLoc(), Loc,
                                         Ptr, false, Loc, Ptr->getType(), VK_LValue);
  ExprResult SubL = BuildBinOp(CurScope, Loc, BO_Assign, LHS, RHS.get());
  return ActOnExprStmt(SubL).get();
}

/// Create extra argument pointer.
///
/// __global__ void *HiddenGlobPtr;
///
VarDecl *Sema::CreateGlobPointer() {
  SourceLocation Loc = SourceLocation(); // TODO: use main location
  IdentifierInfo *II = PP.getIdentifierInfo("HiddenGlobPtr");
  QualType Type;
  Type = Context.IntTy;
  Type = Context.getAddrSpaceQualType(Type, LangAS::rc99_global);
  Type = Context.getPointerType(Type);
  VarDecl *NewVD = VarDecl::Create(getASTContext(), CurContext, Loc, Loc, II, Type, nullptr, SC_None);
  return NewVD;
}

Stmt *Sema::ShiftIndex(VarDecl *TensorOffset) {
  SourceLocation Loc = SourceLocation(); // TODO: use main location
  IdentifierInfo *II = PP.getIdentifierInfo("x");
  DeclarationName MemberName(II);
  IdentifierInfo *Member = MemberName.getAsIdentifierInfo();
  QualType Ty = TensorOffset->getType();
  DeclRefExpr *Idx = DeclRefExpr::Create(getASTContext(), NestedNameSpecifierLoc(), Loc,
                                         TensorOffset, false, Loc, Ty, VK_LValue);
  QualType TyEl = Ty->getAs<ExtVectorType>()->getElementType();

  ExprResult SubL = new (Context) ExtVectorElementExpr(TyEl, VK_LValue, Idx, *Member, Loc);
  DeclRefExpr *IdxR = DeclRefExpr::Create(getASTContext(), NestedNameSpecifierLoc(), Loc,
                                          TensorOffset, false, Loc, Ty, VK_RValue);
  ExprResult SubR = new (Context) ExtVectorElementExpr(TyEl, VK_LValue, IdxR, *Member, Loc);
  Expr *Init = ActOnIntegerConstant(Loc, 1).get();
  ExprResult RHS = BuildBinOp(CurScope, Loc, BO_Add, SubR.get(), Init);
  SubL = BuildBinOp(CurScope, Loc, BO_Assign, SubL.get(), RHS.get());
  return ActOnExprStmt(SubL).get();
}

/// For each argument beyond 32-th creates shadow declaration with the same name
/// and initializes it.
/// \param Stmts Statement list that keeps generated statements. it will become
/// a beginning of main body.
/// \param ExtraArgs Parameters of main beyond 32th. Prepared by parser.
///
void Sema::DeclareExtraArgs(SmallVector<Stmt*, 32> &Stmts,
                            const SmallVector<ParmVarDecl*, 4> &ExtraArgs) {
  //  int5 hiddenTensorOffset (0,0,0,0,0);

  VarDecl *TensorOffset = CreateTensorOffsetForExtraArgs();
  SourceLocation Loc = SourceLocation(); // TODO: use location on main
  DeclStmt *TensorOffsetDecl = new (Context)
      DeclStmt(DeclGroupRef(TensorOffset), Loc, Loc);
  Stmts.push_back(TensorOffsetDecl);

  // __global__ void *pointer;

  VarDecl *GlobPointer = CreateGlobPointer();
  DeclStmt *GlobPointerDecl = new (Context) DeclStmt(DeclGroupRef(GlobPointer), Loc, Loc);
  Stmts.push_back(GlobPointerDecl);

  // pointer = gen_addr(15, hiddenTensorOffset);

  Stmts.push_back(getArgumentAddress(GlobPointer, TensorOffset));

  // arg32 = *pointer;
  Stmts.push_back(ProcessExtraArgs(ExtraArgs[0], GlobPointer));

  // for (int i = 33; i < num_args; ++i) {
  //    hiddenTensorOffset.x = hiddenTensorOffset.x+1;
  //     pointer = gen_address(11, hiddenTensorOffset);

  for (unsigned i = 1; i < ExtraArgs.size(); i++) {
    Stmts.push_back(ShiftIndex(TensorOffset));
    Stmts.push_back(getArgumentAddress(GlobPointer, TensorOffset));
    Stmts.push_back(ProcessExtraArgs(ExtraArgs[i], GlobPointer));
    ExtraParams.push_back(ExtraArgs[i]);
  }
}


void Sema::ActOnPragmaTpcPrintf(PragmaTpcPrintfAction Action) {
  if (Action == PragmaTpcPrintfAction::Enable) {
    TpcPrintfPragmaIsOn = true;
    TpcPrintfIsUsed = true;
  } else {
    assert(Action == PragmaTpcPrintfAction::Disable);
    TpcPrintfPragmaIsOn = false;
  }
}

void Sema::ActOnPragmaIndexSpace(
    Scope *CurScope, SourceLocation PragmaLoc,
    llvm::SmallVector<unsigned, 16> &TensorIDs,
    llvm::SmallVector<llvm::SmallString<128>, 5> &IndexFactors) {

  // Get the closest function scope from where the pragma
  // is declared.
  Scope *FnScope = CurScope->getFnParent();
  if (!FnScope)
    return;

  DeclContext *FnDeclCtx = FnScope->getEntity();
  if (!FnDeclCtx)
    return;

  // From the decl context get the lookup context.
  // It has all the defs in the module.
  DeclContext *FnDeclLookupCtx = FnDeclCtx->getLookupParent();
  if (!FnDeclLookupCtx)
    return;

  std::vector<StringRef> IndexFactorsRef;
  for (unsigned i = 0; i < IndexFactors.size(); ++i) {
    IndexFactorsRef.push_back(IndexFactors[i].str());
  }

  for (DeclContextLookupResult DL : FnDeclLookupCtx->lookups()) {
    for (NamedDecl *ND : DL) {
      if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(ND)) {
        // Add the pragma as attribute to the main function.
        if (FD->isMain()) {
          FD->addAttr(IndexSpaceAttr::CreateImplicit(
              Context, TensorIDs.data(), TensorIDs.size(),
              IndexFactorsRef.data(), IndexFactorsRef.size()));
        }
      }
    }
  }
  TensorIDs.clear();
  IndexFactors.clear();
}

void Sema::ActOnPragmaIndexSpaceb(
    Scope *CurScope, SourceLocation PragmaLoc,
    llvm::SmallVector<unsigned, 16> &TensorIDs,
    llvm::SmallVector<llvm::SmallString<128>, 10> &IndexFactors) {

  // Get the closest function scope from where the pragma
  // is declared.
  Scope *FnScope = CurScope->getFnParent();
  if (!FnScope)
    return;

  DeclContext *FnDeclCtx = FnScope->getEntity();
  if (!FnDeclCtx)
    return;

  // From the decl context get the lookup context.
  // It has all the defs in the module.
  DeclContext *FnDeclLookupCtx = FnDeclCtx->getLookupParent();
  if (!FnDeclLookupCtx)
    return;

  std::vector<StringRef> IndexFactorsRef;
  for (unsigned i = 0; i < IndexFactors.size(); ++i) {
    IndexFactorsRef.push_back(IndexFactors[i].str());
  }

  for (DeclContextLookupResult DL : FnDeclLookupCtx->lookups()) {
    for (NamedDecl *ND : DL) {
      if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(ND)) {
        // Add the pragma as attribute to the main function.
        if (FD->isMain()) {
          FD->addAttr(IndexSpacebAttr::CreateImplicit(
              Context, TensorIDs.data(), TensorIDs.size(),
              IndexFactorsRef.data(), IndexFactorsRef.size()));
        }
      }
    }
  }
  TensorIDs.clear();
  IndexFactors.clear();
}

void Sema::ActOnPragmaReductionOrNormAxes(
    Scope *CurScope, SourceLocation PragmaLoc,
    llvm::SmallVector<unsigned, 16> &TensorIDs,
    llvm::SmallVector<llvm::SmallString<128>, 5> &Axes) {

  // Get the closest function scope from where the pragma
  // is declared.
  Scope *FnScope = CurScope->getFnParent();
  if (!FnScope)
    return;

  DeclContext *FnDeclCtx = FnScope->getEntity();
  if (!FnDeclCtx)
    return;

  // From the decl context get the lookup context.
  // It has all the defs in the module.
  DeclContext *FnDeclLookupCtx = FnDeclCtx->getLookupParent();
  if (!FnDeclLookupCtx)
    return;

  std::vector<StringRef> AxesRef;
  for (unsigned i = 0; i < Axes.size(); ++i) {
    AxesRef.push_back(Axes[i].str());
  }

  for (DeclContextLookupResult DL : FnDeclLookupCtx->lookups()) {
    for (NamedDecl *ND : DL) {
      if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(ND)) {
        // Add the pragma as attribute to the main function.
        if (FD->isMain()) {
          FD->addAttr(ReductionOrNormAxesAttr::CreateImplicit(
              Context, TensorIDs.data(), TensorIDs.size(), AxesRef.data(),
              AxesRef.size()));
        }
      }
    }
  }
}

static const llvm::fltSemantics &getSemanticsFromType(ASTContext &Context, QualType Ty) {
  Ty = Ty.getCanonicalType();
  const llvm::fltSemantics *FSema;
  if (Ty == Context.FloatTy)
    FSema = &llvm::APFloat::IEEEsingle();
  else if (Ty == Context.HalfTy)
    FSema = &llvm::APFloat::IEEEhalf();
  else if (Ty == Context.BFloat16Ty)
    FSema = &llvm::APFloat::BFloat();
  else if (Ty == Context.Float8_143Ty)
    FSema = &llvm::APFloat::Float8_143();
  else if (Ty == Context.Float8_152Ty)
    FSema = &llvm::APFloat::Float8_152();
  else
    llvm_unreachable("cannot determine floating semantics");
  return *FSema;
}

static unsigned getIntWidthFromType(ASTContext &Context, QualType Ty) {
  Ty = Ty.getCanonicalType();
  if (Ty == Context.IntTy || Ty == Context.UnsignedIntTy)
    return 32;
  if (Ty == Context.ShortTy || Ty == Context.UnsignedShortTy)
    return 16;
  if (Ty == Context.CharTy || Ty == Context.UnsignedCharTy)
    return 8;
  llvm_unreachable("unsuported integer type");
}

Expr *Sema::buildDefaultArgument(const FunctionProtoType *FT,
                                 unsigned ID, unsigned ParamNo) {
  int FirstParamWithDefault = Context.BuiltinInfo.getParamWithDefault(ID);
  if (FirstParamWithDefault < 0 || ParamNo < (unsigned)FirstParamWithDefault)
    return nullptr;

  int NParams = FT->getNumParams();
  QualType ParamTy = FT->getParamType(ParamNo);
  if (ParamNo == (unsigned)(NParams - 1) && ParamTy->isBooleanType()) {
    // The last argument, polarity.
    return new(Context) CXXBoolLiteralExpr(false, Context.BoolTy, SourceLocation());
  } else if (ParamNo == (unsigned)(NParams - 2) && ParamTy->isBooleanType()) {
    // The last but one argument, predicate.
    return new(Context) CXXBoolLiteralExpr(true, Context.BoolTy, SourceLocation());
  } else if (ParamNo == (unsigned)(NParams - 3) && ParamTy->isBooleanType()) {
    // Income.
    return new(Context) CXXBoolLiteralExpr(false, Context.BoolTy, SourceLocation());
  } else if (ParamNo == (unsigned)(NParams - 3) && ParamTy == FT->getReturnType()) {
    QualType ArgTy = FT->getReturnType();
    // The last but two argument, income.
    if (ArgTy->isStructureType()) {
      TypeSourceInfo *TInfo = Context.getTrivialTypeSourceInfo(ParamTy);
      ExprResult Init = BuildInitList(SourceLocation(), None, SourceLocation());
      assert(Init.isUsable());
      ExprResult R = BuildCompoundLiteralExpr(SourceLocation(), TInfo, SourceLocation(), Init.get());
      assert(R.isUsable());
      return R.get();
    } else if (ArgTy->isVectorType()) {
      QualType EltTy = ArgTy->getAs<VectorType>()->getElementType();
      Expr *Zero;
      if (EltTy->isFloatingType()) {
        llvm::APFloat Val = llvm::APFloat::getZero(getSemanticsFromType(Context, EltTy));
        Zero = FloatingLiteral::Create(Context, Val, true, EltTy, SourceLocation());
      } else {
        assert(EltTy->isIntegerType());
        llvm::APInt Val(getIntWidthFromType(Context, EltTy), 0);
        Zero = IntegerLiteral::Create(Context, Val, EltTy, SourceLocation());
      }
      return ImplicitCastExpr::Create(Context, ArgTy, CastKind::CK_VectorSplat, Zero, nullptr, VK_RValue, FPOptionsOverride());
    } else if (ArgTy->isFloatingType()) {
      llvm::APFloat Val(getSemanticsFromType(Context, ArgTy));
      return FloatingLiteral::Create(Context, Val, true, ArgTy, SourceLocation());
    } else if (ArgTy->isIntegerType()) {
      llvm::APInt Val(getIntWidthFromType(Context, ArgTy), 0);
      return IntegerLiteral::Create(Context, Val, ArgTy, SourceLocation());
    } else if (ArgTy->isPointerType()) {
      return new(Context) CXXNullPtrLiteralExpr(ArgTy, SourceLocation());
    }
  } else if (ParamTy == Context.IntTy) {
    // Integer parameter, probably switches.
    llvm::APInt Val(32, 0);
    return IntegerLiteral::Create(Context, Val, Context.IntTy, SourceLocation());
  }
  llvm_unreachable("Cannot create default argument");
}
