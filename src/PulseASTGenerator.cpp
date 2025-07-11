#include "PulseASTGenerator.h"
#include "Globals.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attrs.inc"
#include "clang/AST/Comment.h"
#include "clang/AST/CurrentSourceLocExprScope.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Analysis/Analyses/ExprMutationAnalyzer.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#define DEBUG_TYPE "pulse-ast-gen"

using namespace clang;

void PulseConsumer::setNewModules(
    std::map<std::string, PulseModul *> &PulseModules) {
  Modules = PulseModules;
}

std::map<std::string, PulseModul *> &PulseConsumer::getNewModules() {
  return Modules;
}

PulseConsumer::PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R)
    : Visitor(R, Ctx) {}

void PulseConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  setNewModules(Visitor.getPulseModules());
}

std::map<std::string, PulseModul *> &PulseVisitor::getPulseModules() {
  return Modules;
}

void PulseVisitor::InferDeclType(Decl *Dec, FunctionDecl *FD) {

  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      for (auto *InnerStmt : CS->body()) {
        inferDeclType(Dec, InnerStmt);
      }
    }
  }
}

std::map<Decl *, QualType> PulseVisitor::inferArrayTypes(FunctionDecl *FD) {

  std::map<Decl *, QualType> DeclToPulseSymbol;

  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      for (auto *InnerStmt : CS->body()) {
        inferArrayTypesStmt(InnerStmt);
      }
    }
  }

  return DeclToPulseSymbol;
}

bool checkIsSameStructInstance(Expr *A, Expr *B, ASTContext &Context) {
  A = A->IgnoreImpCasts();
  B = B->IgnoreImpCasts();

  DeclRefExpr *DeclRefA = dyn_cast<DeclRefExpr>(A);
  DeclRefExpr *DeclRefB = dyn_cast<DeclRefExpr>(B);

  if (!DeclRefA)
    return false;

  if (!DeclRefB)
    return false;

  if (DeclRefA && DeclRefB) {
    return DeclRefA->getDecl() == DeclRefB->getDecl();
  }

  MemberExpr *MemA = dyn_cast<MemberExpr>(A);
  MemberExpr *MemB = dyn_cast<MemberExpr>(B);

  if (MemA && MemB) {
    return checkIsSameStructInstance(MemA->getBase(), MemB->getBase(), Context);
  }

  // Also check recursively pointer derefernces and array access.
  UnaryOperator *UOA = dyn_cast<UnaryOperator>(A);
  UnaryOperator *UOB = dyn_cast<UnaryOperator>(B);

  if (UOA && UOB && UOA->getOpcode() == UOB->getOpcode()) {
    return checkIsSameStructInstance(UOA->getSubExpr(), UOB->getSubExpr(),
                                     Context);
  }

  return false;
}

bool checkIfLastStructAccess(MemberExpr *ToCheck, CompoundStmt *CS,
                             ASTContext &Context) {
  Expr *ToCheckBase = ToCheck->getBase()->IgnoreImpCasts();
  SourceManager &SM = Context.getSourceManager();
  SourceLocation ToCheckLoc = ToCheck->getExprLoc();
  SourceLocation LastLoc = ToCheckLoc;

  for (const Stmt *S : CS->body()) {
    // Traverse the compound statement and visit it recursively.
    // We store the last accessed member in LastLoc.
    // When we encounter a member expr, we check if this is the same instance as
    //  the ToBeChecked member expression.
    // We also check if the member expression comes after the to be checked
    // member,
    //  if so we update Last loc.
    for (const Stmt *Child : S->children()) {
      if (auto *ME = dyn_cast<MemberExpr>(Child)) {
        Expr *Base = ME->getBase()->IgnoreImpCasts();
        if (Context.getParents(*Base).size() > 0 &&
            SM.isBeforeInTranslationUnit(ToCheckLoc, ME->getExprLoc()) &&
            checkIsSameStructInstance(Base, ToCheckBase, Context)) {
          LastLoc = ME->getExprLoc();
        }
      }
    }
  }

  // If the location of the member to be checked is the same as the last loc
  // This is indeed the last access to the member.
  return ToCheckLoc == LastLoc;
}

bool PulseVisitor::checkIsRecursiveFunction(FunctionDecl *FD) {

  bool result = false;
  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      for (auto *InnerStmt : CS->body()) {
        result = result || checkIsRecursiveStmt(InnerStmt, FD);
      }
    }
  }

  return result;
}

void PulseVisitor::inferDeclType(Decl *Dec, Stmt *InnerStmt) {

  /// Check the types of the statements here.
  if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    for (auto *D : DS->decls()) {
      if (D != Dec)
        continue;
      if (auto *VD = dyn_cast<VarDecl>(D)) {
        /// If we can tell it is an array type from the declaration we just store
        /// it in a map. Otherwise we use array subscript operations to check.
        if (VD->getType()->isArrayType()) {
          DeclTyMap.insert(std::make_pair(VD, VD->getType()));
        }
      }
    }
  } else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)) {
    inferArrayTypesExpr(CExpr);
  } else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)) {

    auto *Cond = While->getCond();
    auto *Body = While->getBody();

    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Body);

  } else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
    for (auto *InnerStmt : CS->body()) {
      inferArrayTypesStmt(InnerStmt);
    }
  } else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)) {
    auto *SubExpr = AttrStmt->getSubStmt();
    inferArrayTypesStmt(SubExpr);
  }
  else {
    // TODO: Vidush see if we want to handle any other statement.
    return;
  }
}

void PulseVisitor::inferArrayTypesStmt(Stmt *InnerStmt) {

  /// Check the types of the statements here.
  if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {
        /// if we can tell it is an array type from the declaration we just store
        /// it in a map. Otherwise we use array subscript operations to check.
        if (VD->getType()->isArrayType()) {
          DeclTyMap.insert(std::make_pair(VD, VD->getType()));
        }

        if (auto *Init = VD->getInit()) {
          auto *InitNoCasts = Init->IgnoreParenImpCasts()->IgnoreCasts();
          if (!InitNoCasts)
            break;
          if (const CallExpr *Call = dyn_cast<CallExpr>(InitNoCasts)) {
            if (const FunctionDecl *FD = Call->getDirectCallee()) {
              if (FD->getName() == "malloc") {
                IsAllocatedOnHeap.insert(VD);
              }
            }
          }
        }
      }
    }
  } else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)) {
    inferArrayTypesExpr(CExpr);
  } else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)) {

    auto *Cond = While->getCond();
    auto *Body = While->getBody();

    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Body);

  } else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
    for (auto *InnerStmt : CS->body()) {
      inferArrayTypesStmt(InnerStmt);
    }
  } else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)) {
    auto *SubExpr = AttrStmt->getSubStmt();
    inferArrayTypesStmt(SubExpr);
  }
  else{
    // TODO: Vidush see if we want to handle any other statement.
    return;
  }
}

void PulseVisitor::inferArrayTypesExpr(Expr *ExprPtr) {

  if (auto *BinOp = dyn_cast<clang::BinaryOperator>(ExprPtr)) {

    // TODO: Vidush:
    // If this BinOp is of the shape: *Arr + 8 etc, we may conclude it is of an
    // array type.
    auto *Lhs = BinOp->getLHS();
    auto *Rhs = BinOp->getRHS();

    inferArrayTypesExpr(Lhs);
    inferArrayTypesExpr(Rhs);

    // try to figure out if the variable is allocated on the heap.
    if (const CallExpr *Call =
            dyn_cast<CallExpr>(Rhs->IgnoreParenImpCasts()->IgnoreCasts())) {
      if (const FunctionDecl *FD = Call->getDirectCallee()) {
        if (FD->getName() == "malloc") {
          if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(
                  Lhs->IgnoreParenImpCasts()->IgnoreCasts())) {
            ValueDecl *VD = DRE->getDecl();
            IsAllocatedOnHeap.insert(VD);
          }
        }
      }
    }

  } else if (auto *UOp = dyn_cast<clang::UnaryOperator>(ExprPtr)) {
    inferArrayTypesExpr(UOp->getSubExpr());

  } else if (auto *Call = dyn_cast<clang::CallExpr>(ExprPtr)) {
    auto NumArgs = Call->getNumArgs();
    for (size_t Idx = 0; Idx < NumArgs; Idx++) {
      auto *Arg = Call->getArg(Idx);
      inferArrayTypesExpr(Arg);
    }
  } else if (auto *ASub = dyn_cast<clang::ArraySubscriptExpr>(ExprPtr)) {
    if (auto *BaseDecl = ASub->getBase()->getReferencedDeclOfCallee()) {
      // Find if this BaseDecl
      if (VarDecl *VD = dyn_cast<VarDecl>(BaseDecl)) {
        clang::QualType IncompleteArrayTy =
            Ctx.getIncompleteArrayType(VD->getType()->getPointeeType(),
                                       clang::ArraySizeModifier::Normal, 0);

        DeclTyMap.insert(std::make_pair(BaseDecl, IncompleteArrayTy));
      }
    }
  } else {
    return;
  }
}

bool PulseVisitor::checkIsRecursiveStmt(Stmt *InnerStmt,
                                        FunctionDecl *CurrFunction) {

  if (!InnerStmt)
    return false;

  if (auto *N = dyn_cast<NullStmt>(InnerStmt)) {
    return false;
  }
  /// Check the types of the statements here.
  else if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    return false;
  } else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)) {
    return checkIsRecursiveExpr(CExpr, CurrFunction);
  } else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)) {

    auto *Cond = While->getCond();
    auto *Body = While->getBody();

    auto isRecCond = checkIsRecursiveExpr(Cond, CurrFunction);
    auto isRecBody = checkIsRecursiveStmt(Body, CurrFunction);

    return isRecCond || isRecBody;

  } else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
    bool result = false;
    for (auto *InnerStmt : CS->body()) {
      result = result || checkIsRecursiveStmt(InnerStmt, CurrFunction);
    }
    return result;
  } else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)) {
    auto *SubExpr = AttrStmt->getSubStmt();
    return checkIsRecursiveStmt(SubExpr, CurrFunction);
  } else if (auto *RetStmt = dyn_cast<ReturnStmt>(InnerStmt)) {
    auto *RetExpr = RetStmt->getRetValue();

    return checkIsRecursiveExpr(RetExpr, CurrFunction);
  } else if (auto *ClangIfStmt = dyn_cast<IfStmt>(InnerStmt)) {
    auto *Cond = ClangIfStmt->getCond();
    auto *Then = ClangIfStmt->getThen();
    auto *Else = ClangIfStmt->getElse();

    auto CondIsRec = checkIsRecursiveExpr(Cond, CurrFunction);
    auto ThenIsRec = checkIsRecursiveStmt(Then, CurrFunction);
    auto ElseIsRec = checkIsRecursiveStmt(Else, CurrFunction);

    return CondIsRec || ThenIsRec || ElseIsRec;

  } else {
    /// TODO: Vidush see if we want to handle any other statement.
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    InnerStmt->dump();
    });
    emitErrorWithLocation("Encountered an unhandled case!", &SM,
                          InnerStmt->getEndLoc());
  }
}

/// Recurse all expressions.
bool PulseVisitor::checkIsRecursiveExpr(Expr *ExprPtr,
                                        FunctionDecl *CurrFunction) {

  if (!ExprPtr)
        return false;

  if (auto *BinOp = dyn_cast<clang::BinaryOperator>(ExprPtr)) {
    /// TODO: Vidush:
    /// If this BinOp is of the shape: *Arr + 8 etc, we may conclude it is of an
    /// array type.
    auto *Lhs = BinOp->getLHS();
    auto *Rhs = BinOp->getRHS();

    return checkIsRecursiveExpr(Lhs, CurrFunction) ||
           checkIsRecursiveExpr(Rhs, CurrFunction);

  } else if (auto *UOp = dyn_cast<clang::UnaryOperator>(ExprPtr)) {
    return checkIsRecursiveExpr(UOp->getSubExpr(), CurrFunction);
  } else if (auto *Call = dyn_cast<clang::CallExpr>(ExprPtr)) {
    auto NumArgs = Call->getNumArgs();
    bool isRec = false;
    for (size_t Idx = 0; Idx < NumArgs; Idx++) {
      auto *Arg = Call->getArg(Idx);
      isRec = isRec || checkIsRecursiveExpr(Arg, CurrFunction);
    }
    const FunctionDecl *callee = Call->getDirectCallee();
    if (callee && callee == CurrFunction) {
      isRec = isRec || true;
    }
    
    return isRec;
  } else if (auto *ASub = dyn_cast<clang::ArraySubscriptExpr>(ExprPtr)) {
    return false;
  } else {
    return false;
  }
}

bool PulseVisitor::VisitTypedefDecl(TypedefDecl *TypeDefDec) {

  /// TODO: Angelica: This might fail for analyzing programs that use
  /// struct definitions from system libraries or C standard libraries.
  auto SourceLoc = TypeDefDec->getLocation();
  if (SM.isInSystemHeader(SourceLoc))
    return true;

  auto *Def = TypeDefDec->getUnderlyingDecl();

  /// Check if we have a record declaration associated with the typedefDecl.
  ///  For every record type we create a new Module.
  if (const auto *RT = TypeDefDec->getUnderlyingType()->getAs<RecordType>()) {
    const RecordDecl *RD = RT->getDecl();

    auto StructName = Def->getNameAsString();
    auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
    if (!FileEnt) {
      llvm::errs() << "Error: Main file entry not found in source manager.\n";
      exit(1);
    }
    
    /// Parse the file name. 
    /// The filename is same as module.
    auto FilePath = FileEnt->tryGetRealPathName();
    std::filesystem::path FilePathSys = FilePath.str();
    auto Extension = FilePathSys.extension().string();
    auto TempFilePathWithoutExtension = FilePathSys.replace_extension("");
    auto FileName = TempFilePathWithoutExtension.filename();
    auto FileNameStr = FileName.string();
    if (!FileNameStr.empty()) {
      FileNameStr[0] = std::toupper(FileNameStr[0]);
    }

    // change dots to _ since . is reserved for nested modules.
    std::replace(FileNameStr.begin(), FileNameStr.end(), '.', '_');

    auto It = Modules.find(FileNameStr);
    PulseModul *NewModul = nullptr;
    if (It != Modules.end()) {
      NewModul = It->second;
    }
    else {
      NewModul = new PulseModul();
      NewModul->includePulsePrelude = true;
      NewModul->ModuleName = FileNameStr;
      Modules.insert(std::make_pair(NewModul->ModuleName, NewModul));
    }

    auto *AbstractType = new GenericDecl();
    AbstractType->Ident = "noeq\n";
    AbstractType->Ident += "type ";
    AbstractType->Ident += StructName + " = {\n";
    for (const FieldDecl *FD : RD->fields()) {
      auto *PulseTy = getPulseTyFromCTy(FD->getType());
      AbstractType->Ident += FD->getNameAsString() + ": ref ";
      AbstractType->Ident += PulseTy->print() + ";";
      AbstractType->Ident += "\n";

    }
    AbstractType->Ident += "}\n";
    NewModul->Decls.push_back(AbstractType);

    //2. A purely functional specification type for the struct
    // [@@erasable]
    // noeq
    // type u32_pair_struct_spec = {
    //   first: FStar.UInt32.t;
    //   second: FStar.UInt32.t;
    // }

    auto *Tycon = new TyConDecl();
    auto *TyconRec = new TyConRecord();
    auto *ErasableAttr = new Name("[@@erasable]");
    auto *NoEqTerm = new Name("noeq");
    TyconRec->Ident = Def->getNameAsString() + "_spec";
    TyconRec->Attrs.push_back(ErasableAttr);
    TyconRec->Attrs.push_back(NoEqTerm);

    std::vector<RecordElement *> Fields;
    for (const FieldDecl *FD : RD->fields()) {
      auto *Element = new RecordElement();
      Element->ElementTerm = getPulseTyFromCTy(FD->getType());
      Element->Ident = FD->getNameAsString();
      Fields.push_back(Element);
    }
    auto NumRecordFields = Fields.size();
    TyconRec->RecordFields = Fields;
    Tycon->TyCons.push_back(TyconRec);
    NewModul->Decls.push_back(Tycon);

    // Generate predicate
    // 3. A predicate that relates a u32_pair_struct to its specification
    // let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
    //     exists* (y: u32_pair_struct). (x |-> y) **
    //     (y.first |-> s.first) **
    //     (y.second |-> s.second)

    auto *GenericPredicate = new GenericDecl(); 
    GenericPredicate->Ident = "let ";
    GenericPredicate->Ident += StructName + "_pred (x:ref " +  StructName + ") (s:" + StructName + "_spec) : slprop =\n";
    GenericPredicate->Ident += "exists* (y: " + StructName + "). (x |-> y) **\n";
    size_t Counter = 0;
    for (auto *Fld : RD->fields()){
        GenericPredicate->Ident += "(y.";
        GenericPredicate->Ident += Fld->getNameAsString(); 
        GenericPredicate->Ident += " |-> ";
        GenericPredicate->Ident += "s." + Fld->getNameAsString() + ")";
        if (Counter < NumRecordFields - 1){
          GenericPredicate->Ident += " **";
        }
        Counter++;
        GenericPredicate->Ident += "\n";
    }
    NewModul->Decls.push_back(GenericPredicate);

    // Auto Generated functions for Stack Allocated Structs.
    // assume val point_spec_default : point_spec

    auto StructPrefix = StructName + "_var";
    auto StructPrefixSpec = StructPrefix + "_spec";

    auto *StructSpecDefault = new GenericDecl();
    StructSpecDefault->Ident = "assume val " + StructName +
                               "_spec_default : " + StructName + "_spec\n";
    NewModul->Decls.push_back(StructSpecDefault);

    // assume val point_default (ps:point_spec) : point

    auto *StructDefault = new GenericDecl();
    StructDefault->Ident = "assume val " + StructName + "_default ";
    StructDefault->Ident += "(" + StructPrefixSpec + ":" + StructName +
                            "_spec)" + " : " + StructName + "\n";
    NewModul->Decls.push_back(StructDefault);

    // ghost
    // fn point_pack (p:ref point) (#ps:point_spec)
    // requires p |-> point_default ps
    // ensures exists* v. point_pred p v ** pure (v == ps)
    // { admit() }

    auto *GhostPack = new GenericDecl();
    GhostPack->Ident = "ghost\n";
    GhostPack->Ident += "fn " + StructName + "_pack (" + StructPrefix +
                        ":ref " + StructName + ") " + "(#" + StructPrefixSpec +
                        ":" + StructName + "_spec)\n";
    GhostPack->Ident += "requires " + StructPrefix + "|-> " + StructName +
                        "_default " + StructPrefixSpec + "\n";
    GhostPack->Ident += "ensures exists* v. " + StructName + "_pred " +
                        StructPrefix + " v ** pure (v == " + StructPrefixSpec +
                        ")\n";
    GhostPack->Ident += "{ admit() }\n";
    NewModul->Decls.push_back(GhostPack);

    // ghost
    // fn point_unpack (p:ref point)
    // requires exists* v. point_pred p v
    // ensures exists* u. p |-> u
    // { admit() }

    auto *GhostUnpack = new GenericDecl();
    GhostUnpack->Ident = "ghost\n";
    GhostUnpack->Ident += "fn " + StructName + "_unpack " + "(" + StructPrefix +
                          ":ref " + StructName + ")\n";
    GhostUnpack->Ident +=
        "requires exists* v. " + StructName + "_pred " + StructPrefix + " v \n";
    GhostUnpack->Ident += "ensures exists* u. " + StructPrefix + " |-> u\n";
    GhostUnpack->Ident += "{ admit() }\n";
    NewModul->Decls.push_back(GhostUnpack);

    auto *UtilityFunctionHeap = new GenericDecl();
    UtilityFunctionHeap->Ident += "fn " + StructName + "_alloc ()\n";
    UtilityFunctionHeap->Ident += "returns x:ref " + StructName + "\n";
    UtilityFunctionHeap->Ident += "ensures freeable x\n";
    UtilityFunctionHeap->Ident += "ensures exists* v. " + StructName + "_pred x v\n";
    UtilityFunctionHeap->Ident += "{ admit () }\n\n";

    // fn u32_pair_struct_free (x:ref u32_pair_struct)
    // requires freeable x
    // requires exists* v. u32_pair_struct_pred x v
    // { admit () }

    UtilityFunctionHeap->Ident += "fn " + StructName + "_free " + "(x:ref " + StructName + ")\n";
    UtilityFunctionHeap->Ident += "requires freeable x\n";
    UtilityFunctionHeap->Ident += "requires exists* v. " + StructName + "_pred x v\n";
    UtilityFunctionHeap->Ident += "{ admit() }\n\n";

    NewModul->Decls.push_back(UtilityFunctionHeap);

    //5. A ghost function that unfolds the predicate for u32_pair_struct_refs
    // ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
    // requires u32_pair_struct_pred x s
    // ensures exists* (v: u32_pair_struct). (x |-> v)
    //   ** (v.first |-> s.first) ** (v.second |-> s.second)
    // { unfold u32_pair_struct_pred }

    auto *GhostExplode = new GenericDecl(); 
    GhostExplode->Ident = "ghost fn " + StructName + "_explode (x:ref " + StructName + ") " + "(#s:" + StructName + "_spec)\n";
    GhostExplode->Ident += "requires " + StructName + "_pred x s\n";
    GhostExplode->Ident += "ensures exists* (v: " + StructName + "). " + "(x |-> v)";
    GhostExplode->Ident += " ** ";
    Counter = 0;
    for (auto *Fld : RD->fields()){
      GhostExplode->Ident += "(v." + Fld->getNameAsString() + " |-> " + "s." + Fld->getNameAsString() + ")";
      if (Counter < NumRecordFields - 1){
        GhostExplode->Ident += " ** ";
      }
      Counter++;
    }
    GhostExplode->Ident += "\n";
    GhostExplode->Ident += "{unfold " + StructName + "_pred" + "}\n\n";
    NewModul->Decls.push_back(GhostExplode);

    //8. A ghost function that folds the predicate for u32_pair_struct_refs
    // ghost
    // fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 #a1 :erased U32.t)
    // requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
    // ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
    // { fold u32_pair_struct_pred x ({first = a0; second = a1}) }

    std::string FieldPrefix = "a";
    auto *NewGhostFunction = new GenericDecl();
    NewGhostFunction->Ident = "ghost\n";
    NewGhostFunction->Ident += "fn " + StructName + "_recover ";
    NewGhostFunction->Ident += "(x:ref " + StructName + ") ";
    
    Counter = 0;
    for (auto *Fld : RD->fields()) {
      auto Ty = Fld->getType(); 
      auto *PulseTy = getPulseTyFromCTy(Ty);
      NewGhostFunction->Ident += "(";
      NewGhostFunction->Ident += "#" + FieldPrefix + std::to_string(Counter) + " : ";
      NewGhostFunction->Ident += PulseTy->print(); 
      NewGhostFunction->Ident += ") ";
      Counter++;
    }
    NewGhostFunction->Ident += "\n";

    NewGhostFunction->Ident += "requires exists* (y: " + StructName + "). (x |-> y) ** ";
    Counter = 0;
    for (auto *Fld : RD->fields()){
      NewGhostFunction->Ident += "(";
      NewGhostFunction->Ident += "y.";
      NewGhostFunction->Ident += Fld->getNameAsString() + " ";
      NewGhostFunction->Ident += "|-> ";
      NewGhostFunction->Ident += FieldPrefix + std::to_string(Counter);
      NewGhostFunction->Ident += ")";

      if (Counter < NumRecordFields - 1){
        NewGhostFunction->Ident += " ** ";
      }
      Counter++;
    }
    NewGhostFunction->Ident += "\n";

    NewGhostFunction->Ident += "ensures exists* w. " + StructName + "_pred x w ** pure (w == {";
    Counter = 0;
    std::string TempStr = "";
    for (auto *Fld : RD->fields()){
      TempStr += Fld->getNameAsString();
      TempStr += " = ";
      TempStr += FieldPrefix;
      TempStr += std::to_string(Counter);

      if (Counter < NumRecordFields - 1){
        TempStr += "; ";
      }
      Counter++;
    }
    NewGhostFunction->Ident += TempStr;
    NewGhostFunction->Ident += "})\n";

    NewGhostFunction->Ident += "{fold " + StructName + "_pred x ({";
    NewGhostFunction->Ident += TempStr + "}) }\n";

    NewModul->Decls.push_back(NewGhostFunction);
  }

  return true;
}

/// A helper function to generate a pulse sequence from expressions that
/// need to be released.
static PulseSequence *releaseExprs(SmallVector<PulseStmt *> &ExprsBefore) {
  PulseSequence *Start = nullptr;
  if (!ExprsBefore.empty()) {

    for (size_t Idx = 0; Idx < ExprsBefore.size(); Idx++) {
      if (Start == nullptr) {
        auto *Seq = new PulseSequence();
        Seq->assignS1(ExprsBefore[Idx]);
        Start = Seq;
      }

      auto *NextSeq = new PulseSequence();
      NextSeq->assignS1(ExprsBefore[Idx]);
      Start->assignS2(NextSeq);
      Start = NextSeq;
    }

    /// Remove expressions that have been added to the sequence.
    ExprsBefore.clear();
  }
  return Start;
}

bool PulseVisitor::VisitFunctionDecl(FunctionDecl *FD) {

  if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()) ||
      FD->isImplicit() ||
      (FD->getLocation().isMacroID() &&
       !SM.isWrittenInMainFile(SM.getExpansionLoc(FD->getLocation()))))
    return true;

  auto FuncName = FD->getNameAsString();

  auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
  if (!FileEnt) {
    llvm::errs() << "Error: Main file entry not found in source manager.\n";
    exit(1);
  }

  auto FilePath = FileEnt->tryGetRealPathName();
  std::filesystem::path FilePathSys = FilePath.str();
  auto Extension = FilePathSys.extension().string();
  auto TempFilePathWithoutExtension = FilePathSys.replace_extension("");
  auto FileName = TempFilePathWithoutExtension.filename();
  auto FileNameStr = FileName.string();
  if (!FileNameStr.empty()) {
    FileNameStr[0] = std::toupper(FileNameStr[0]);
  }

  // change dots to _ since . is reserved for nested modules.
  std::replace(FileNameStr.begin(), FileNameStr.end(), '.', '_');

  std::string ClangModuleName = FileNameStr;
  inferArrayTypes(FD);

  auto *FDefn = new _PulseFnDefn();
  FDefn->Name = FuncName;
  FDefn->isRecursive = true;

  if (!checkIsRecursiveFunction(FD)) {
    FDefn->isRecursive = false;
  }

  auto It = Modules.find(ClangModuleName);
  PulseModul *Module = nullptr;
  if (It != Modules.end()) {
    Module = It->second;
  }
  // Create new function defintions.
  else {
    Module = new PulseModul();
    Module->includePulsePrelude = true;
    Module->ModuleName = ClangModuleName;
  }
  Modules.insert(std::make_pair(ClangModuleName, Module));

  std::vector<Binder *> PulseArgs;
  std::vector<Binder *> ErasedArgs;
  if (FD->hasAttrs()) {
    auto AnnotationsAttachedToFD = FD->getAttrs();
    for (auto *Attr : AnnotationsAttachedToFD) {

      if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (AnnAttr->getAttrName()->getName() == "pulse") {
          
          // Uncomment to print location information.
          // 
          // auto Loc = AnnAttr->getLocation();
          // auto ExpansionLoc = SM.getExpansionLoc(Loc);
          // ExpansionLoc.dump(SM);
          // auto PLoc = SM.getPresumedLoc(ExpansionLoc);
          // llvm::outs() << "Print Location Info: for attribute: " << "\n";
          // llvm::outs() << "Filename: " <<  PLoc.getFilename() << "\n";
          // llvm::outs() << "Column: " << PLoc.getColumn() << "\n";
          // llvm::outs() << "LineNumber: " << PLoc.getLine() << "\n";
          // llvm::outs() << "End printing location info for attribute\n";
          // auto Args = AnnAttr->args();
          // llvm::outs() << "\n\n";
          // for (auto *Arg : Args){
          //   Arg->dump();
          //   llvm::outs() << "\n";
          //   auto Loc = Arg->getExprLoc();
          //      auto ExpansionLoc = SM.getExpansionLoc(Loc);
          //      ExpansionLoc.dump(SM);
          //       auto PLoc = SM.getPresumedLoc(ExpansionLoc);
          //       llvm::outs() << "Print Location Info for Arg: " << "\n";
          //       llvm::outs() << "Filename: " <<  PLoc.getFilename() << "\n";
          //       llvm::outs() << "Column: " << PLoc.getColumn() << "\n";
          //       llvm::outs() << "LineNumber: " << PLoc.getLine() << "\n";
          //       llvm::outs() << "End printing location info for attribute\n";
          //       llvm::outs() << "\n\n";
          // }
           
          auto Ref = AnnAttr->getAnnotation();
          if (!Ref.empty()) {
            std::string Match = "";
            PulseAnnKind AnnKind = getPulseAnnKindFromString(
                AnnAttr->getAnnotation().data(), Match);
            switch (AnnKind) {
            case PulseAnnKind::ExpectFailure: {
              StringRef MatchRef(Match);
              llvm::SmallVector<StringRef, 4> CommaSeperatedItems;
              MatchRef.split(CommaSeperatedItems, ",");
              auto *NewAttr = new Name();

              if (CommaSeperatedItems.empty()) {
                NewAttr->setName("[@@expect_failure]");
                FDefn->Attr.push_back(NewAttr);
                break;
              }

              if (!CommaSeperatedItems.empty() &&
                  llvm::all_of(CommaSeperatedItems,
                               [](const llvm::StringRef &item) {
                                 return item.empty();
                               })) {
                NewAttr->setName("[@@expect_failure]");
                FDefn->Attr.push_back(NewAttr);
                break;
              }

              std::string AttrStr = "[@@expect_failure [";
              size_t Counter = 0;
              for (auto &Item : CommaSeperatedItems) {
                auto RTrimmed = Item.rtrim();
                auto LTrimmed = RTrimmed.ltrim();
                AttrStr.append(LTrimmed);
                if (Counter < CommaSeperatedItems.size() - 1) {
                  AttrStr.append(";");
                }
                Counter++;
              }
              AttrStr.append("]]");
              NewAttr->setName(AttrStr);
              FDefn->Attr.push_back(NewAttr);
              break;
            }
            case PulseAnnKind::Requires: {
              auto *NewRequires = new Requires();
              NewRequires->Ann = Match;
              FDefn->Annotation.push_back(NewRequires);
              break;
            }
            case PulseAnnKind::Ensures: {
              auto *NewEnsures = new Ensures();
              NewEnsures->Ann = Match;
              FDefn->Annotation.push_back(NewEnsures);
              break;
            }
            case PulseAnnKind::Returns: {
              auto *ReturnSpec = new Returns();
              ReturnSpec->Ann = Match;
              FDefn->Annotation.push_back(ReturnSpec);
              break;
            }
            case PulseAnnKind::ErasedArg: {
              auto *NewErasedArgBinder = new Binder(Match);
              NewErasedArgBinder->useFallBack = true;
              ErasedArgs.push_back(NewErasedArgBinder);
              break;
            }
            case PulseAnnKind::Includes: {
              //Assume modules will be comma seperated.
              //StringRef MatchRef(Match);
              //llvm::SmallVector<StringRef, 4> IncModules;
              //MatchRef.split(IncModules, ",");
              //for (auto &IncModule : IncModules){
              //  auto RTrimmed = IncModule.rtrim();
              //  auto LTrimmed = RTrimmed.ltrim();
              //  Module->insertModule(LTrimmed.str());
              //}
              auto *Inc = new GenericDecl();
              Inc->Ident = Match;
              Module->Decls.push_back(Inc);
              //If function name is the anchor dummy function. 
              //Skip generating code for it entirely.
              if ((FuncName.find("__pulse_include_anchor") != std::string::npos)){
                return true;
              }
              break;
            }
            case PulseAnnKind::IsArray:
              llvm::report_fatal_error(
                  "Unhandled PulseAnnKind::IsArray in switch statement");
              break;
            case PulseAnnKind::Invariants:
              llvm::report_fatal_error(
                  "Unhandled PulseAnnKind::Invariants in switch statement");
              break;
            case PulseAnnKind::LemmaStatement:
              llvm::report_fatal_error(
                  "Unhandled PulseAnnKind::LemmaStatement in switch statement");
              break;
            }
          }
        }
      }
    }
  }

  for (unsigned i = 0; i < FD->getNumParams(); i++) {
    auto *Param = FD->getParamDecl(i);
    auto ParamName = Param->getNameAsString();

    /// See if this array parameter has any annotations arrached to it.
    auto Attrs = Param->attrs();
    for (auto *Attr : Attrs) {
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (AnnotAttr->getAttrName()->getName() == "pulse") {
          auto AnnotationData = AnnotAttr->getAnnotation();
          std::string Match;
          auto PulseAnnotKind =
              getPulseAnnKindFromString(AnnotationData, Match);

          if (PulseAnnotKind == PulseAnnKind::IsArray) {
            // Add type to map.
            // Make a clang Array Type
            // Try to get element type
            if (!Param->getType()->isPointerType() &&
                !Param->getType()->isArrayType()) {
              emitErrorWithLocation(
                  "Expected parameter to be a ref or an array!", &SM,
                  Param->getLocation());
            }

            QualType ElementType = Param->getType()->getPointeeType();

            if (!std::regex_match(Match, std::regex("[-+]?[0-9]+"))) {

              // Step 2: Create a VarDecl for the size variable 'n'
              // We should check here is the length is a constant or of variable
              // array type.
              IdentifierInfo &Id = Ctx.Idents.get(Match);
              VarDecl *SizeVar = VarDecl::Create(
                  Ctx, Ctx.getTranslationUnitDecl(), SourceLocation(),
                  SourceLocation(), &Id, Ctx.IntTy, nullptr, SC_Auto);

              // Step 3: Create a DeclRefExpr to refer to 'n'
              DeclRefExpr *SizeExpr = DeclRefExpr::Create(
                  Ctx, NestedNameSpecifierLoc(), SourceLocation(), SizeVar,
                  false, SourceLocation(), Ctx.IntTy,
                  clang::Expr::getValueKindForType(ElementType));

              // Step 4: Create the VLA type
              QualType VLAType = Ctx.getVariableArrayType(
                  ElementType, SizeExpr, ArraySizeModifier::Normal, 0);
              // llvm::outs() << "Print the element type here!!!\n";
              // llvm::outs() <<
              // QualType(VLAType->getPointeeOrArrayElementType(), 0);
              // llvm::outs() << "End of element type!" << "\n"; exit(1);
              DeclTyMap.insert(std::make_pair(Param, VLAType));
            } else {
              clang::QualType ConstArrayTy = Ctx.getConstantArrayType(
                  ElementType, llvm::APInt(32, std::stoi(Match)), nullptr,
                  ArraySizeModifier::Normal, 0);
              DeclTyMap.insert(std::make_pair(Param, ConstArrayTy));
            }
          } else if (PulseAnnotKind == PulseAnnKind::HeapAllocated) {
            IsAllocatedOnHeap.insert(Param);
          } else {
            emitErrorWithLocation("Pulse annotation kind not implemented yet!",
                                  &SM, FD->getLocation());
          }
        }
      }
    }

    FStarType *ParamTy;
    auto It = DeclTyMap.find(Param);
    if (It != DeclTyMap.end()) {
      // Get the qualification
      auto Ty = It->second;
      if (Ty->isArrayType() || Ty->isConstantArrayType() ||
          Ty->isVariableArrayType()) {
        auto *FArrTy = new FStarArrType();
        FArrTy->ElementType =
            getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        ParamTy = FArrTy;
      } else {
        ParamTy = getPulseTyFromCTy(Param->getType());
      }
    } else {
      InferDeclType(Param, FD);
      auto It = DeclTyMap.find(Param);
      if (It != DeclTyMap.end()) {
        auto Ty = It->second;
        auto *FArrTy = new FStarArrType();
        FArrTy->ElementType =
            getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        ParamTy = FArrTy;

      } else {
        ParamTy = getPulseTyFromCTy(Param->getType());
      }
    }
    
    /// set param to boxed if it is heap allcated.
    if (IsAllocatedOnHeap.count(Param)){
      if (auto *CastPointer = dyn_cast<FStarPointerType>(ParamTy)){
        CastPointer->isBoxed = true;
      }
    }

    auto *Binder = new struct Binder(ParamName, ParamTy);
    PulseArgs.push_back(Binder);
  }
  std::copy(ErasedArgs.begin(), ErasedArgs.end(),
            std::back_inserter(PulseArgs));
  
  FDefn->Args = PulseArgs;

  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      ExprMutationAnalyzer Analyzer(*CS, Ctx);

      // Track struct variables per function.
      //TrackStructExplodeAndRecover.clear();
      auto *PulseBody = pulseFromCompoundStmt(CS, &Analyzer, Module);

      // Release declarations that are function parameters.
      //TODO: Vidush Eventually we should get rid of these.
      //This is just in case there are release expressions left and we need to release them.
      PulseSequence *NewSeq = nullptr;
      PulseSequence *Head = nullptr;
      // for (auto It = TrackStructExplodeAndRecover.begin(); It != TrackStructExplodeAndRecover.end();) {
      //   auto ItElem = *It;
      //   auto &Decl = ItElem.first;
      //   auto &Info = ItElem.second;
      //   // recover not released.
      //   if (!Info.second) {
      //     if (auto *ParamD = dyn_cast<ParmVarDecl>(Decl)) {

      //       auto StructName = ParamD->getType()->getPointeeType().getAsString();

      //       auto *RecoverStatememt = new GenericStmt();
      //       RecoverStatememt->body =
      //           StructName + "_recover " + ParamD->getNameAsString() + ";";
      //       if (Head == nullptr) {
      //         Head = new PulseSequence();
      //         NewSeq = Head;
      //         NewSeq->assignS1(PulseBody);

      //         auto *NextSequence = new PulseSequence();
      //         NextSequence->assignS1(RecoverStatememt);
      //         NewSeq->assignS2(NextSequence);
      //         NewSeq = NextSequence;
      //          TrackStructExplodeAndRecover.erase(It++);
      //         continue;
      //       }

      //       auto *NextSequence = new PulseSequence();
      //       NextSequence->assignS1(RecoverStatememt);
      //       NewSeq->assignS2(NextSequence);
      //       NewSeq = NextSequence;

      //       TrackStructExplodeAndRecover.erase(It++);
      //       continue;
      //     }
      //   }
      //   It++;
      // }

      // assert(TrackStructExplodeAndRecover.empty() && "Failed to recover all structure types in the function!\n");

      if (Head != nullptr) {
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        Head->dumpPretty();
        });
        FDefn->Body = Head;
      } else if (PulseBody != nullptr) {
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        PulseBody->dumpPretty();
        });
        FDefn->Body = PulseBody;
      }
    }
  }

  PulseFnDefn *PulseFn = new PulseFnDefn(FDefn);

  DEBUG_WITH_TYPE(DEBUG_TYPE, {
    llvm::outs() << "=================================================";
    llvm::outs() << "\nPrint the Pulse function Definition:\n\n";
    PulseFn->dumpPretty();
    llvm::outs() << "\nEnd printing the function Definition\n\n";
    llvm::outs() << "=================================================\n";
  });

  PulseFn->Kind = PulseDeclKind::FnDefn;
  Module->Decls.push_back(PulseFn);
  DeclarationsMap.insert(std::make_pair(FD, PulseFn));
  return true;
}

FStarType *PulseVisitor::getPulseTyFromCTy(clang::QualType CType) {
  // TODO: Check if Ctype is a pointer type, if so, use FStarPointerType.

  FStarType *PulseTy;
  if (CType->isPointerType()) {

    if (CType->isArrayType()) {
      emitError("PulseVisitor: Did not implement array type in clang");
    }

    PulseTy = new FStarPointerType();
    auto *PulsePointerTy = static_cast<FStarPointerType *>(PulseTy);
    auto BaseTy = CType->getPointeeType();
    PulsePointerTy->setName("ref " + BaseTy.getAsString());
    auto UnderLyingType = CType->getPointeeType();
    auto *FStartUnderLyingType = getPulseTyFromCTy(UnderLyingType);
    PulsePointerTy->setPointerToTy(FStartUnderLyingType);
    return PulsePointerTy;
  }

  PulseTy = new FStarType();
  auto CTyKey = getSymbolKeyForCType(CType, Ctx);
  std::string CTyKeyStr;
  if (CTyKey != SymbolTable::UNKNOWN)
    CTyKeyStr = lookupSymbol(CTyKey);
  else{
    if (CType->isStructureType()){
      if (auto *TD = dyn_cast<TypedefType>(CType.getTypePtr())) {
        TypedefNameDecl *TyD = TD->getDecl();
        std::string TypedefName = TyD->getNameAsString();
        CTyKeyStr = "ref " + TypedefName;
      }
      else {
        CTyKeyStr = CType.getAsString();
      }
    }
    else{
      CTyKeyStr = CType.getAsString();
    }
  }
  PulseTy->setName(CTyKeyStr);

  return PulseTy;

}

PulseStmt *PulseVisitor::pulseFromCompoundStmt(Stmt *S,
                                               ExprMutationAnalyzer *Analyzer,
                                               PulseModul *Modul) {

  PulseSequence *Stmt = nullptr;
  PulseSequence *Head = nullptr;
  if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    // //This only works on Structs for now.
    // //For any struct declaration,
    // TrackScopeOfStackAllocatedStructs.clear();

    for (auto *InnerStmt : CS->body()) {

      auto *NextPulseStmt =
          pulseFromStmt(InnerStmt, Analyzer, nullptr, Modul, CS);
      if (NextPulseStmt == nullptr)
        continue;

      if (Stmt == nullptr) {
        auto *NewSeq = new PulseSequence();
        NewSeq->assignS1(NextPulseStmt);
        Stmt = NewSeq;
        Head = NewSeq;
        continue;
      }

      auto *NewSeq = new PulseSequence();
      NewSeq->assignS1(NextPulseStmt);
      Stmt->assignS2(NewSeq);
      Stmt = NewSeq;
    }

    /// Check what variables are defined in the Compound statement.
  }

  return Head;
}

PulseStmt *PulseVisitor::pulseFromStmt(Stmt *S, ExprMutationAnalyzer *Analyzer,
                                       Stmt *Parent, PulseModul *Module,
                                       CompoundStmt *CS) {

  if (!S)
    return nullptr;

  if (auto *DS = dyn_cast<DeclStmt>(S)) {
    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {

        if (VD->hasInit()) {
          auto *Init = VD->getInit();
          auto VarName = VD->getNameAsString();
          // Unsure if we really need the type here.
          // Though it may be usefuel checking invalid casting operations.
          // auto VarType = VD->getType();

          // Don't forget to release these exprs.
          SmallVector<PulseStmt *> NewExprs;
          Term *LetInit = getTermFromCExpr(Init, Analyzer, NewExprs, Parent,
                                           VD->getType(), Module);

          LetBinding *PulseLet;
          if (Analyzer->isMutated(D)) {
            PulseLet = new LetBinding(VarName, LetInit, MutOrRef::MUT);
          } else {
            PulseLet = new LetBinding(VarName, LetInit, MutOrRef::NOTMUT);
          }

          //Set the corresponding source location for the C ast node. 
          //PulseLet->RegInfo = new RegionMapping();
          //PulseLet->RegInfo->CInfo = 

          // We need to make a sequence of pulse statements.
          PulseSequence *Start = nullptr;
          if (!NewExprs.empty()) {

            for (size_t Idx = 0; Idx < NewExprs.size(); Idx++) {
              if (Start == nullptr) {
                auto *Seq = new PulseSequence();
                Seq->assignS1(NewExprs[Idx]);
                Start = Seq;
              }

              auto *NextSeq = new PulseSequence();
              NextSeq->assignS1(NewExprs[Idx]);
              Start->assignS2(NextSeq);
              Start = NextSeq;
            }
          }

          NewExprs.clear();
          assert(NewExprs.empty() && "Expected expressions to be released!");

          if (Start != nullptr) {
            Start->assignS2(PulseLet);

            // check for any lemmas to be released.
            auto Attrs = VD->attrs();
            for (auto *Att : Attrs) {
              if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)) {
                if (AnnonAttr->getAttrName()->getName() == "pulse") {
                  std::string Match;
                  auto AnnKind = getPulseAnnKindFromString(
                      AnnonAttr->getAnnotation(), Match);
                  assert((AnnKind == PulseAnnKind::LemmaStatement) &&
                         "Expected a Lemma statement!\n");
                  
                  auto *LS = new LemmaStatement();
                  LS->Lemma = Match;
                  auto *LSE = new PulseExpr();
                  LSE->E = LS;

                  auto *NewS = new PulseSequence();
                  NewS->assignS1(LSE);
                  NewS->assignS2(Start);
                  Start = NewS;
                }
              }
            }
            return Start;
          }

          // check for any lemmas to be released.
          auto Attrs = VD->attrs();
          for (auto *Att : Attrs) {
            if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)) {
              if (AnnonAttr->getAttrName()->getName() == "pulse") {
                std::string Match;
                auto AnnKind = getPulseAnnKindFromString(
                    AnnonAttr->getAnnotation(), Match);

                if (AnnKind == PulseAnnKind::LemmaStatement) {
                  auto *LS = new LemmaStatement();
                  LS->Lemma = Match;
                  DEBUG_WITH_TYPE(DEBUG_TYPE , {
                  llvm::outs() << "Found Lemma: " << "\n";
                  llvm::outs() << Match << "\n";
                  llvm::outs() << "End.\n";
                  });

                  auto *LSE = new PulseExpr();
                  LSE->E = LS;
                  if (Start == nullptr) {
                    auto *NewS = new PulseSequence();
                    NewS->assignS1(LSE);
                    Start = NewS;
                  }
                  Start->assignS2(LSE);
                } else if (AnnKind == PulseAnnKind::HeapAllocated) {
                  IsAllocatedOnHeap.insert(VD);
                } else {
                  emitErrorWithLocation("Did not expect pulse annotation kind!",
                                        &SM, VD->getLocation());
                }
              }
            }
          }

          auto *AppendLet = new PulseSequence();
          AppendLet->assignS1(Start);
          AppendLet->assignS2(PulseLet);

          return AppendLet;
        }

        if (const TypedefType *TT = VD->getType()->getAs<TypedefType>()) {

            auto *TypedefDecl = TT->getDecl();
            auto StructName = TypedefDecl->getDeclName();
            DEBUG_WITH_TYPE(DEBUG_TYPE , {
            llvm::outs() << "Typedef name: " << StructName << "\n";
            });
            if (Analyzer->isMutated(VD)){

              auto *Rhs = new Name(StructName.getAsString() + "_default " +
                                   StructName.getAsString() + "_spec_default");
              auto *NewMutLet =
                  new LetBinding(VD->getNameAsString(), Rhs, MutOrRef::MUT);
              return NewMutLet;
            }
            
            //Implement case when the allocation is not mutated.
            // A normal let bind
            emitErrorWithLocation(
                "Did not implement case when struct allocation is not mutated!",
                &SM, VD->getLocation());
        }

        // Any uninitialized declaration that is not a struct
        auto CType = VD->getType();
        auto PulseTySymbol = getSymbolKeyForCType(CType, Ctx);
        auto *PulseTyStr = lookupSymbol(PulseTySymbol);
        auto ClangVarName = VD->getNameAsString();

        auto *GenericDecl = new GenericStmt();
        GenericDecl->body =
            "let mut " + ClangVarName + ": " + PulseTyStr + " = witness #_ #_;";
        return GenericDecl;
      }
      emitErrorWithLocation(
          "Declarations other than variable declarations not implemented!", &SM,
          D->getLocation());
    }

  } else if (auto *BO = dyn_cast<BinaryOperator>(S)) {

    if (BO->isAssignmentOp()) {
      auto *Lhs = BO->getLHS();
      auto *Rhs = BO->getRHS();

      if (UnaryOperator *UO = dyn_cast<UnaryOperator>(Lhs)) {

        if (UO->getOpcode() == UO_Deref) {

          // TODO: Make sure to release these expressions
          SmallVector<PulseStmt *> ExprsBef;

          auto *PulseLhsTerm = getTermFromCExpr(
              UO->getSubExpr(), Analyzer, ExprsBef, Parent, BO->getType(), Module);
          auto *PulseRhsTerm =
              getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent,BO->getType(), Module);
          PulseAssignment *Assignment =
              new PulseAssignment(PulseLhsTerm, PulseRhsTerm);

          assert(ExprsBef.empty() && "Expected expressions to be released!");

          return Assignment;
        }
      } else if (auto *ArrSub = dyn_cast<ArraySubscriptExpr>(Lhs)) {

        // TODO: Make sure to release these expressions
        SmallVector<PulseStmt *> ExprsBef;

        auto *ArrayAssignExpr = new PulseArrayAssignment();
        ArrayAssignExpr->Arr = getTermFromCExpr(
            ArrSub->getBase(), Analyzer, ExprsBef, Parent, BO->getType(), Module);
        ArrayAssignExpr->Index = getTermFromCExpr(
            ArrSub->getIdx(), Analyzer, ExprsBef, Parent, BO->getType(), Module);
        ArrayAssignExpr->Value =
            getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent, BO->getType(), Module);

        // We need to make a sequence of pulse statements.
        PulseSequence *Start = nullptr;
        if (!ExprsBef.empty()) {

          for (size_t Idx = 0; Idx < ExprsBef.size(); Idx++) {
            if (Start == nullptr) {
              auto *Seq = new PulseSequence();
              Seq->assignS1(ExprsBef[Idx]);
              Start = Seq;
            }

            auto *NextSeq = new PulseSequence();
            NextSeq->assignS1(ExprsBef[Idx]);
            Start->assignS2(NextSeq);
            Start = NextSeq;
          }

          ExprsBef.clear();
        }

        assert(ExprsBef.empty() && "Expected expressions to be released!");

        if (Start != nullptr) {
          Start->assignS2(ArrayAssignExpr);
          return Start;
        }

        return ArrayAssignExpr;
      } else if (auto *ME = dyn_cast<MemberExpr>(Lhs)) {

        auto *LhsDecl = ME->getMemberDecl();
        auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        BaseExpr->dump();
        });

        std::string NameOfDecl;
        QualType TyOfDecl;
        std::string StructName;
        if (const clang::DeclRefExpr *DRE =
                llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
          const clang::ValueDecl *VD = DRE->getDecl();
          // Now you can safely cast VD to a more specific Decl type if needed
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          VD->dump();
          llvm::outs() << VD->getDeclName() << "End\n";
          });
          NameOfDecl = VD->getDeclName().getAsString();
          TyOfDecl = VD->getType();
          
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << TyOfDecl->getPointeeType().getAsString() << "\n";
          });

          StructName = TyOfDecl->getPointeeType().getAsString();

          auto MemberName = LhsDecl->getDeclName();

          // UPDATE: Vidush: Releasing expresssions may be required in certain cases. 
          //Especially when you want to add calls to explode struct.
          SmallVector<PulseStmt *> ExprsBef;
          auto *PulseRhsTerm =
              getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent,BO->getType(), Module);
          PulseAssignment *Assignment;

          auto *DerefAppE = new AppE("!");

          auto *InnerTermCallArg = new VarTerm(NameOfDecl);
          DerefAppE->pushArg(InnerTermCallArg);

          // Wrap this deref in a parenthesis.
          auto *ParenthesisDeref = new Paren(DerefAppE);

          //Hack to check if the base type is a pointer. 
          //Leveraging the C syntax.
          bool BaseIsPointer = ME->isArrow() ? true : false;

          if (BaseIsPointer){
            auto *PulseCall =
                new AppE("Mk" + StructName + "?." + MemberName.getAsString());
            PulseCall->pushArg(ParenthesisDeref);
            Assignment = new PulseAssignment(PulseCall, PulseRhsTerm);
          }
          else {
            auto *NewProjection = new Project();
            NewProjection->BaseTerm = ParenthesisDeref;
            NewProjection->MemberName = MemberName.getAsString();
            Assignment = new PulseAssignment(NewProjection, PulseRhsTerm);
          }

          // auto It = TrackStructExplodeAndRecover.find(VD);
          // if (It == TrackStructExplodeAndRecover.end()){
          //   auto *NewSeq = new PulseSequence();
          //   NewSeq->assignS2(Assignment);
          //   auto *ExplodeStmt = new GenericStmt();
          //   ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString() + ";";
          //   NewSeq->assignS1(ExplodeStmt);
          //   TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
          //   return NewSeq;
          // }

          auto *RetSeq = releaseExprs(ExprsBef);

          assert(ExprsBef.empty() && "Expected ExprsBefore to be empty!\n");

          // if (checkIfLastStructAccess(ME, CS, Ctx)) {

          //   auto It = TrackStructExplodeAndRecover.find(VD);
          //   auto ItElem = *It;
          //   auto &Decl = ItElem.first;
          //   auto &Info = ItElem.second;
          //   // recover not released.
          //   // In fact assert that a recover should not be released before.
          //   assert(!Info.second && "A recover was released for the struct when "
          //                          "there are accesses remaining!\n");
          //   if (auto *ParamD = dyn_cast<ParmVarDecl>(Decl)) {

          //     auto StructName =
          //         ParamD->getType()->getPointeeType().getAsString();

          //     auto *RecoverStatememt = new GenericStmt();
          //     RecoverStatememt->body =
          //         StructName + "_recover " + ParamD->getNameAsString() + ";";
          //     TrackStructExplodeAndRecover.erase(It);

          //     auto *NewSeq = new PulseSequence();
          //     NewSeq->assignS1(Assignment);
          //     NewSeq->assignS2(RecoverStatememt);
          //     if (RetSeq) {
          //       RetSeq->assignS2(NewSeq);
          //       return RetSeq;
          //     }

          //     return NewSeq;
          //   }
          // }

          if (RetSeq) {
            RetSeq->assignS2(Assignment);
            return RetSeq;
          }

          return Assignment;
        }

        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        ME->dump();
        });
        emitErrorWithLocation(
            "Could not cast member base expression to its declaration!", &SM,
            ME->getBeginLoc());

      } else if (auto *ME = dyn_cast<MemberExpr>(Rhs)) {

        auto *RhsDecl = ME->getMemberDecl();
        auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        BaseExpr->dump();
        });

        std::string NameOfDecl;
        QualType TyOfDecl;
        std::string StructName;
        if (const clang::DeclRefExpr *DRE =
                llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
          const clang::ValueDecl *VD = DRE->getDecl();
          NameOfDecl = VD->getDeclName().getAsString();
          TyOfDecl = VD->getType();
          StructName = TyOfDecl->getPointeeType().getAsString();

          auto MemberName = RhsDecl->getDeclName();

          //x->f translates to (!(!x).f)
          auto *GenStmt = new Name("(!(!" + NameOfDecl + ")." +
                                   MemberName.getAsString() + ")");

          SmallVector<PulseStmt *> ExprsBef;
          auto *PulseLhsTerm = getTermFromCExpr(Lhs, Analyzer, ExprsBef,
                                              Parent, BO->getType(), Module, true);

          PulseAssignment *Assignment =
              new PulseAssignment(PulseLhsTerm, GenStmt);

          // auto It = TrackStructExplodeAndRecover.find(VD);
          // if (It == TrackStructExplodeAndRecover.end()) {
          //   auto *NewSeq = new PulseSequence();
          //   NewSeq->assignS2(Assignment);
          //   auto *ExplodeStmt = new GenericStmt();
          //   ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString();
          //   NewSeq->assignS2(ExplodeStmt);
          //   TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
          //   return NewSeq;
          // }

          assert(ExprsBef.empty() && "Expected ExprsBefore to be empty!\n");

          return Assignment;
        }

      }
      // TODO:
      // We should generate Lets otherwise
      else {

        // TODO: Make sure to release these expressions
        SmallVector<PulseStmt *> ExprsBef;
        auto *PulseLhsTerm = getTermFromCExpr(Lhs, Analyzer, ExprsBef,
                                              Parent, BO->getType(), Module, true);
        auto *PulseRhsTerm =
            getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent, BO->getType(), Module);
        PulseAssignment *Assignment =
            new PulseAssignment(PulseLhsTerm, PulseRhsTerm);

        // We need to make a sequence of pulse statements.
        PulseSequence *Start = nullptr;
        if (!ExprsBef.empty()) {

          for (size_t Idx = 0; Idx < ExprsBef.size(); Idx++) {
            if (Start == nullptr) {
              auto *Seq = new PulseSequence();
              Seq->assignS1(ExprsBef[Idx]);
              Start = Seq;
            }

            auto *NextSeq = new PulseSequence();
            NextSeq->assignS1(ExprsBef[Idx]);
            Start->assignS2(NextSeq);
            Start = NextSeq;
          }

          ExprsBef.clear();
        }

        assert(ExprsBef.empty() && "Expected expressions to be released!");

        if (Start != nullptr) {
          Start->assignS2(Assignment);
          return Start;
        }

        return Assignment;
      }
    } else {

      SmallVector<PulseStmt *> ExprsBefore;

      auto *PExpr = new PulseExpr();
      auto ExprTerm =
          getTermFromCExpr(BO, Analyzer, ExprsBefore, Parent, BO->getType(), Module);
      if (!ExprTerm)
        return nullptr;

      PExpr->E = ExprTerm;

      // We need to make a sequence of pulse statements.
      PulseSequence *Start = nullptr;
      if (!ExprsBefore.empty()) {

        for (size_t Idx = 0; Idx < ExprsBefore.size(); Idx++) {
          if (Start == nullptr) {
            auto *Seq = new PulseSequence();
            Seq->assignS1(ExprsBefore[Idx]);
            Start = Seq;
          }

          auto *NextSeq = new PulseSequence();
          NextSeq->assignS1(ExprsBefore[Idx]);
          Start->assignS2(NextSeq);
          Start = NextSeq;
        }

        // remove all released expressions.
        ExprsBefore.clear();
      }

      if (Start != nullptr) {
        Start->assignS2(PExpr);
        return Start;
      }

      assert(ExprsBefore.empty() && "Expected expressions to be released!");
      return PExpr;
    }

  }
  // else if (auto *ParenExpr = dyn_cast<clang::ParenExpr>(S)) {

  //   auto *ClangSubExpr = ParenExpr->getSubExpr();

  //   auto *PulseSubExpr = pulseFromStmt(ClangSubExpr, Analyzer,
  //                                         Parent, Module, CS);

  //   auto *PulseParenExpr = new Paren(PulseSubExpr);
  //   return PulseParenExpr;
  // }
  else if (auto *E = dyn_cast<Expr>(S)) {

    SmallVector<PulseStmt *> ExprsBefore;

    auto *PulseExpression = new PulseExpr();

    auto *PExprTerm =
        getTermFromCExpr(E, Analyzer, ExprsBefore, Parent, E->getType(), Module);

    if (PExprTerm == nullptr)
      return nullptr;

    PulseExpression->E = PExprTerm;

    auto *NewSeq = releaseExprs(ExprsBefore);

    if (NewSeq) {
      NewSeq->assignS2(PulseExpression);
      return NewSeq;
    }

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return PulseExpression;
  } else if (auto *IF = dyn_cast<IfStmt>(S)) {

    auto *PulseIfStmt = new PulseIf();

    auto *Cond = IF->getCond();
    auto *Then = IF->getThen();
    auto *Else = IF->getElse();

    SmallVector<PulseStmt *> ExprsBefore;

    auto *PulseCond = getTermFromCExpr(Cond, Analyzer, ExprsBefore, Parent,
                                       Cond->getType(), Module);

    PulseStmt *PulseThen;
    if (auto *AttrStmt = dyn_cast<AttributedStmt>(Then)) {
      auto *ThenBody = AttrStmt->getSubStmt();

      auto Attributes = AttrStmt->getAttrs();
      // auto *PulseWhile = new PulseWhileStmt();
      for (auto *Attr : Attributes) {

        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
          if (AnnAttr->getAttrName()->getName() == "pulse") {

            auto AnnotationData = AnnAttr->getAnnotation().str();

            std::string Match;
            auto AnnKind = getPulseAnnKindFromString(AnnotationData, Match);
            switch (AnnKind) {
            case PulseAnnKind::Ensures: {
              auto *NewEnsures = new Ensures();
              NewEnsures->Ann = Match;
              PulseIfStmt->IfLemmas.push_back(NewEnsures);
              break;
            };
            default:
              emitErrorWithLocation("Annotation not expected for IfStmt", &SM,
                                    IF->getBeginLoc());
            };
          }
        }
      }
      PulseThen = pulseFromStmt(ThenBody, Analyzer, Parent, Module, CS);
    } else {
      PulseThen = pulseFromStmt(Then, Analyzer, Parent, Module, CS);
    }

    auto *PulseElse = pulseFromStmt(Else, Analyzer, Parent, Module, CS);

    PulseIfStmt->Head = PulseCond;
    PulseIfStmt->Else = PulseElse;
    PulseIfStmt->Then = PulseThen;

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return PulseIfStmt;
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {

    if (auto *RetVal = RS->getRetValue()) {

      // if (auto *CastToStmt = dyn_cast<Stmt>(RS->getRetValue())){
      //  auto *RetStmt = pulseFromStmt(RetVal, Analyzer,
      //                                 RetVal, Module, CS);

      // return RetStmt;
      //}
      // else{

      SmallVector<PulseStmt *> ExprsBefore;
      auto *RetTerm = getTermFromCExpr(RetVal, Analyzer, ExprsBefore, Parent,
                                       RetVal->getType(), Module);

      if (RetTerm == nullptr)
        return nullptr;

      auto *NewPulseExpr = new PulseExpr();
      NewPulseExpr->E = RetTerm;

      assert(ExprsBefore.empty() && "Expected expressions to be released!");
      return NewPulseExpr;
      //}
      //   if (auto *DeclRef =
      //   dyn_cast<DeclRefExpr>(RetVal->IgnoreParenImpCasts()->IgnoreImpCasts())){
      //     auto It = TrackStructExplodeAndRecover.find(DeclRef->getDecl());
      //     if (It != TrackStructExplodeAndRecover.end()){
      //       auto Info = It->second;
      //       if (!Info.second){

      //         //Get struct name from declration.
      //           const auto *VD = DeclRef->getDecl();
      //           auto *PSeq = new PulseSequence();
      //           auto StructName =
      //           VD->getType()->getPointeeType().getAsString(); if (RetTerm){
      //              NewPulseExpr->E = RetTerm;
      //              PSeq->assignS2(NewPulseExpr);
      //              auto *FallBack  = new GenericStmt();
      //              FallBack->body += StructName + "_recover " +
      //              DeclRef->getDecl()->getNameAsString() + ";";
      //              PSeq->assignS1(FallBack);
      //              // update element in map.
      //              TrackStructExplodeAndRecover.erase(It);
      //              return PSeq;
      //          }

      //         auto *FallBack  = new GenericStmt();
      //         FallBack->body += StructName + "_recover " +
      //         DeclRef->getDecl()->getNameAsString() + ";";

      //         // update element in map.
      //         TrackStructExplodeAndRecover.erase(It);
      //         return FallBack;
      //     }
      //   }
      // }
    }

    return nullptr;
  } else if (auto *FS = dyn_cast<ForStmt>(S)) {
    S->dumpPretty(Ctx);
    emitErrorWithLocation("For loops not implemented since pulse does not "
                          "support for expressions",
                          &SM, FS->getBeginLoc());
  } else if (auto *WS = dyn_cast<WhileStmt>(S)) {

    auto *WhileCond = WS->getCond();
    auto *WhileBody = WS->getBody();

    if (auto *AttrStmt = dyn_cast<AttributedStmt>(WhileBody)) {
      auto *CompundBody = AttrStmt->getSubStmt();

      auto Attributes = AttrStmt->getAttrs();
      auto *PulseWhile = new PulseWhileStmt();
      for (auto *Attr : Attributes) {

        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
          if (AnnAttr->getAttrName()->getName() == "pulse") {

            auto AnnotationData = AnnAttr->getAnnotation().str();

            std::string StartDelimiter = "invariants:";
            size_t pos = AnnotationData.find(StartDelimiter);
            std::string EndDelimiter = "|END";

            size_t end = AnnotationData.find(EndDelimiter);
            std::string match;
            if (pos != std::string::npos) {
              std::string firstPart = AnnotationData.substr(0, pos);
              match =
                  AnnotationData.substr(pos + StartDelimiter.length(),
                                        end - (pos + StartDelimiter.length()));
              if (!match.empty() && match.front() == '"' &&
                  match.back() == '"') {
                match = match.substr(1, match.size() - 2);
              }
            }

            std::vector<std::string> tokens;
            std::stringstream ss(match);
            std::string token;
            while (std::getline(ss, token, ',')) {
              tokens.push_back(token.c_str());
            }
            // llvm::outs() << "PRINT OUT the invarinat tokens\n";
            for (auto token : tokens) {
              //llvm::outs() << token << "\n";
              auto *NewLemmaTerm = new LemmaStatement();
              NewLemmaTerm->Lemma.assign(token.c_str());
              PulseWhile->Invariant.push_back(NewLemmaTerm);
            }
            // llvm::outs() << "\n";
          }
        }
      }

      PulseWhile->Guard =
          pulseFromStmt(WhileCond, Analyzer, Parent, Module, CS);
      PulseWhile->Body = pulseFromCompoundStmt(CompundBody, Analyzer, Module);

      return PulseWhile;
    } else {

      auto *PulseWhile = new PulseWhileStmt();
      PulseWhile->Guard =
          pulseFromStmt(WhileCond, Analyzer, Parent, Module, CS);
      PulseWhile->Body = pulseFromCompoundStmt(WhileBody, Analyzer, Module);

      return PulseWhile;
    }
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
    S->dumpPretty(Ctx);
    emitErrorWithLocation(
        "Did not implement translation from C unary expression to PulseStmt!",
        &SM, US->getBeginLoc());
  } else if (auto *NS = dyn_cast<NullStmt>(S)) {
    return nullptr;
  } else if (auto *CS = dyn_cast<CompoundStmt>(S)) {
    return pulseFromCompoundStmt(CS, Analyzer, Module);
  } else if (auto *AttrStmt = dyn_cast<AttributedStmt>(S)) {

    auto *SubStmt = AttrStmt->getSubStmt();
    PulseSequence *NewSequence = nullptr;

    auto Attrs = AttrStmt->getAttrs();
    assert(Attrs.size() == 1 && "Did not handle multiple attrs in an attributed stmt.\n");
    for (auto *Attr : Attrs) {
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (AnnotAttr->getAttrName()->getName() == "pulse") {
          std::string Match;
          auto AttrKind =
              getPulseAnnKindFromString(AnnotAttr->getAnnotation(), Match);
            if (AttrKind == PulseAnnKind::LemmaStatement){
              auto *LS = new LemmaStatement();
              LS->Lemma = Match;
              auto *PE = new PulseExpr();
              PE->E = LS;
              return PE;
            }
            else if (AttrKind == PulseAnnKind::Assert){
              auto *GenStmt = new GenericStmt(); 
              GenStmt->body = "assert(" + Match + ");";
              return GenStmt;
            }
            else {
              emitErrorWithLocation("Unhandled Attr in Attributed Stmt!", &SM,
                                    AttrStmt->getAttrLoc());
            }
        }
      }
    }
    NewSequence->assignS2(pulseFromStmt(SubStmt, Analyzer, Parent, Module, CS));
    return NewSequence;
  } else {
    S->dumpPretty(Ctx);
    emitErrorWithLocation("Not implemented Clang expr!", &SM, S->getBeginLoc());
  }

  return nullptr;
}

const clang::Stmt *getNextStatement(const clang::Expr *expr,
                                    clang::ASTContext &Context) {
  if (!expr)
    return nullptr;

  const clang::Stmt *parent =
      Context.getParents(*expr).begin()->get<clang::Stmt>(); // Get parent node
  if (!parent)
    return nullptr;

  bool foundExpr = false;
  for (const clang::Stmt *child : parent->children()) {
    if (foundExpr)
      return child; // Return next statement after expr
    if (child == expr)
      foundExpr = true;
  }

  return nullptr; // No next statement found
}

static bool checkIfExprIsNullPtr(Expr *E){

  //Make sure we remove any parens and casts around the expr.
  if (CastExpr *CE = dyn_cast<CastExpr>(E)) {
    if (CE->getType()->isPointerType()) {
        auto *IgnoreParenCasts = CE->getSubExpr()->IgnoreParens()->IgnoreCasts();
        auto *IgnoreImpCasts = IgnoreParenCasts->IgnoreImpCasts();
      if (const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(IgnoreImpCasts)) {
        if (IL->getValue() == 0) {
          return true;
        }
      }
    }
  }

  return false;
}

Term *
PulseVisitor::getTermFromCExpr(Expr *E, ExprMutationAnalyzer *MutAnalyzer,
                               llvm::SmallVector<PulseStmt *> &ExprsBefore,
                               Stmt *Parent, 
                               QualType ParentType, PulseModul *Module,
                               bool isWrite) {

  if (auto *IL = dyn_cast<IntegerLiteral>(E)) {

    auto *NewConstTerm = new ConstTerm();
    NewConstTerm->ConstantValue = std::to_string(IL->getValue().getSExtValue());

    DEBUG_WITH_TYPE(DEBUG_TYPE , {
    llvm::outs() << "Found Integer Literal: " << NewConstTerm->ConstantValue
                 << "\n";
    llvm::outs() << ParentType.getAsString() << "\n";
    });

    NewConstTerm->Symbol = getSymbolKeyForCType(ParentType, Ctx);

    return NewConstTerm;
  } else if (auto *FL = dyn_cast<FloatingLiteral>(E)) {
    E->dumpPretty(Ctx);
    emitErrorWithLocation("Floating Literal not implemented!", &SM,
                          E->getExprLoc());
  } else if (auto *SL = dyn_cast<StringLiteral>(E)) {
    E->dumpPretty(Ctx);
    emitErrorWithLocation("String Literal not implemented!", &SM,
                          E->getExprLoc());
  } else if (auto *CL = dyn_cast<CharacterLiteral>(E)) {
    E->dumpPretty(Ctx);
    emitErrorWithLocation("Character Liternal not implemented!", &SM,
                          E->getExprLoc());
  } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {

    auto *Lhs = BO->getLHS();
    auto *Rhs = BO->getRHS();
    auto Op = BO->getOpcode();

    if (Lhs->getType() != Rhs->getType()) {
      E->dumpPretty(Ctx);
      LLVM_DEBUG(llvm::dbgs() << "\n");
      emitErrorWithLocation("Expected types of Lhs and Rhs to be the same, "
                            "unsafe type casting now allowed in pulse!",
                            &SM, E->getExprLoc());
    }

    switch (Op) {
    case clang::BO_EQ: {

      // Check if either Lhs or Rhs is NULL.
      // In case it is NULL we would like to generate is_null checks for pulse.
      if (checkIfExprIsNullPtr(Lhs)) {

        // Vidush: REMARK: In general if we have a bunch of nested calls
        //  or repeated applications of ! operator, we should just use a
        // fall back generic ast node like Name to generate the call.
        // Choosing a pulse Ast call node since its not a complicated call
        // expression.
        auto *IsNullCall = new AppE("is_null");
        auto *RhsTerm = getTermFromCExpr(Rhs, MutAnalyzer, ExprsBefore, Parent,
                                         BO->getType(), Module);
        IsNullCall->pushArg(RhsTerm);
        return IsNullCall;
      }

      if (checkIfExprIsNullPtr(Rhs)) {

        auto *IsNullCall = new AppE("is_null");
        auto *LhsTerm = getTermFromCExpr(Lhs, MutAnalyzer, ExprsBefore, Parent,
                                         BO->getType(), Module);
        IsNullCall->pushArg(LhsTerm);
        return IsNullCall;
      }

      goto default_bin_op_case;
    }
    case clang::BO_NE: {

      // Check if either Lhs or Rhs is NULL.
      // In case it is NULL we would like to generate is_null checks for pulse.
      if (checkIfExprIsNullPtr(Lhs)) {
       
        //Generate null call
        auto *IsNullCall = new AppE("is_null");
        auto *RhsTerm = getTermFromCExpr(Rhs, MutAnalyzer, ExprsBefore, Parent,
                                         BO->getType(), Module);
        IsNullCall->pushArg(RhsTerm);

        //Generate not call
        auto *NotCall = new AppE("not");

        //Wrap null call around parenthesis
        auto *ParenNullCall = new Paren(IsNullCall);

        NotCall->pushArg(ParenNullCall);
        return NotCall;
      }

      if (checkIfExprIsNullPtr(Rhs)) {
        
        //Generate null call
        auto *IsNullCall = new AppE("is_null");
        auto *LhsTerm = getTermFromCExpr(Lhs, MutAnalyzer, ExprsBefore, Parent,
                                         BO->getType(), Module);
        IsNullCall->pushArg(LhsTerm);
        
        //Generate not call
        auto *NotCall = new AppE("not");

        //Wrap null call around parenthesis
        auto *ParenNullCall = new Paren(IsNullCall);

        NotCall->pushArg(ParenNullCall);
        return NotCall;
      }

      goto default_bin_op_case;
    }
    default_bin_op_case:
    default: {

      if (checkIfExprIsNullPtr(Lhs) || checkIfExprIsNullPtr(Rhs)){
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        BO->dump();
        });
        emitErrorWithLocation("Null check not implemented for binary operator "
                              "other that Eq and Neq!",
                              &SM, BO->getExprLoc());
      }

      SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
      auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);

      auto *NewAppENode = new AppE(OpKey);
      auto *LhsTerm = getTermFromCExpr(Lhs, MutAnalyzer, ExprsBefore, Parent,
                                       Lhs->getType(), Module);
      auto *RhsTerm = getTermFromCExpr(Rhs, MutAnalyzer, ExprsBefore, Parent,
                                       Rhs->getType(), Module);
      NewAppENode->pushArg(LhsTerm);
      NewAppENode->pushArg(RhsTerm);

      // Wrap Call Expr into a Paren to be safe.
      auto *NewParen = new Paren(NewAppENode);
      return NewParen;
      break;
    }
    }

  } else if (auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_Deref) {
      auto *DerefAppE = new AppE("!");

      auto *TermForBaseExpr = getTermFromCExpr(UO->getSubExpr(), MutAnalyzer,
                                               ExprsBefore, Parent, ParentType, Module);
      DerefAppE->pushArg(TermForBaseExpr);

      // Wrap this deref in a parenthesis.
      auto *Parenthesis = new Paren(DerefAppE);
      return Parenthesis;

    } 
    else if (UO->getOpcode() == UO_AddrOf){

      auto SubExpr = UO->getSubExpr();
      if (auto *Mem = dyn_cast<MemberExpr>(SubExpr)) {

        auto *BaseExpr = Mem->getBase();
        if (auto *Dec = dyn_cast<DeclRefExpr>(
                BaseExpr->IgnoreParenImpCasts()->IgnoreImpCasts())) {
          auto *VD = Dec->getDecl();
          auto VDTy = VD->getType();
          auto StructName = VDTy->getPointeeType().getAsString();

          auto *GenStmt =
              new Name("(!" + Dec->getDecl()->getNameAsString() + ")." +
                       Mem->getMemberDecl()->getDeclName().getAsString());

          // auto It = TrackStructExplodeAndRecover.find(VD);
          // if (It == TrackStructExplodeAndRecover.end()) {
          //   auto *ExplodeStmt = new GenericStmt();
          //   ExplodeStmt->body =
          //       StructName + "_explode " + VD->getNameAsString() + ";";
          //   ExprsBefore.push_back(ExplodeStmt);
          //   TrackStructExplodeAndRecover.insert(
          //       std::make_pair(VD, std::make_pair(true, false)));
          // }

          return GenStmt;
        }
      }
      else {

        /// Expecting Decl ref to add ! since it will consider addof as yes to
        /// the variable being mutated.
        auto *TermForBaseExpr = getTermFromCExpr(UO->getSubExpr(), MutAnalyzer,
                                               ExprsBefore, Parent, ParentType, Module);

        // Wrap address of in a parenthesis.
        auto *Parenthesis = new Paren(TermForBaseExpr);
        return Parenthesis;
      }
    } else {
      DEBUG_WITH_TYPE(DEBUG_TYPE, {
      E->dumpPretty(Ctx);
      E->dump();
      });
      emitErrorWithLocation(
          "Unhandeled case in UnaryOperator getTermFromCExpr!", &SM,
          E->getExprLoc());
    }
  } else if (auto *CE = dyn_cast<CallExpr>(E)) {

    if (CE->getDirectCallee()->getNameAsString() == pulseProofTermFromC) {
      auto NumArgs = CE->getNumArgs();
      assert(NumArgs == 1 &&
             "Expected number of arguments for Pulse Proof Term to be 1!");
      auto *UserLemma = new Lemma();
      if (auto *ArgToString =
              dyn_cast<StringLiteral>(CE->getArg(0)->IgnoreCasts())) {
        auto ArgString = ArgToString->getString();
        UserLemma->lemmas.push_back(ArgString.str());
      } else {
        emitErrorWithLocation(
            "Expected pulse while to have arguments as string literals", &SM,
            CE->getBeginLoc());
      }

      return UserLemma;
    } else if (CE->getDirectCallee()->getNameAsString() ==
               pulseWhileInvariantFromC) {

      auto NumArgs = CE->getNumArgs();

      std::vector<Slprop *> VectorLemmas;
      for (size_t Idx = 0; Idx < NumArgs; Idx++) {
        auto *UserLemma = new Lemma();
        auto *Arg = CE->getArg(Idx);
        // assert that each argument is actually a string literal
        if (auto *ArgToString = dyn_cast<StringLiteral>(Arg->IgnoreCasts())) {
          auto ArgString = ArgToString->getString();
          UserLemma->lemmas.push_back(ArgString.str());
          Slprop *Prop = UserLemma;
          VectorLemmas.push_back(Prop);
          // assert that next statement is a while loop
        } else {
          emitErrorWithLocation(
              "Expected pulse while to have arguments as string literals", &SM,
              E->getExprLoc());
        }
      }

      auto *Next = getNextStatement(E, Ctx);
      if (auto *While = dyn_cast<WhileStmt>(Next)) {
        // Add corresponding while invariant.
        StmtToLemmas.insert(std::make_pair(While, VectorLemmas));
        return nullptr;
      }
      emitErrorWithLocation(
          "Expected next statement after pulse invariant to be a while!", &SM,
          CE->getBeginLoc());
    }

    auto CallName = CE->getDirectCallee()->getNameAsString();
    auto *CallAppE = new AppE();

    if (CallName != "free") {
      CallAppE->makeCallName(CallName);

      auto *CallE = CE->getDirectCallee(); 
      for (size_t i = 0; i < CE->getNumArgs(); i++) {
        auto *Arg = CE->getArg(i);

        //Vidush: TODO: check: for void argument types we don't want to generated any code. 
        if (Arg->getType()->isVoidType()){
          continue;
        }

        auto *Param = CallE->getParamDecl(i);

        //check if this is an addr of.
        if (auto *UO_Arg = dyn_cast<UnaryOperator>(Arg)){
          if (UO_Arg->getOpcode() == UO_AddrOf){
            assert(Param->getType()->isPointerType() && "Expect to pass a reference since function param expects it!");
            auto *BaseExpr = UO_Arg->getSubExpr();
            if (auto *DeclSub = dyn_cast<DeclRefExpr>(BaseExpr)){
              auto *NewVar = new VarTerm(DeclSub->getDecl()->getNameAsString());
              CallAppE->pushArg(NewVar);
              continue;
            }
          }
        }

        auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                         Arg->getType(), Module);
        CallAppE->pushArg(ArgTerm);
      }
    } else {

      // check argument type.
      assert(CE->getNumArgs() == 1 &&
             "Did not expect free to have more than one argument!");
      auto *Arg = CE->getArg(0);
      if (auto *ArgDeclR =
              dyn_cast<DeclRefExpr>(Arg->IgnoreParens()->IgnoreImpCasts())) {
        auto *ArgDecl = ArgDeclR->getDecl();
        auto Ty = ArgDecl->getType();
        auto PointeeType = Ty->getPointeeType();
        if (Ty->isPointerType() && PointeeType->isStructureType()) {
          auto StructName = PointeeType.getAsString();
          auto NewCallName = StructName + "_free";
          CallAppE->makeCallName(NewCallName);
          auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                           ParentType, Module);
          CallAppE->pushArg(ArgTerm);
        } else {
          CallAppE->makeCallName(CallName + "_ref");
          auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                           ParentType, Module);
          CallAppE->pushArg(ArgTerm);
        }
      }
    }

    // Wrap Call expr in Paren Node
    auto *NewParen = new Paren(CallAppE);
    return NewParen;
  } else if (auto *IC = dyn_cast<ImplicitCastExpr>(E)) {

    // TODO: Check : Vidush
    // Right now we basically ignore implicit cast expressions.
    // However, since pulse is pure this may not be expected.
    auto *SubExpr = IC->getSubExpr();
    return getTermFromCExpr(SubExpr, MutAnalyzer, ExprsBefore, Parent,
                            ParentType, Module);
  } else if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {

    auto *DreDecl = DRE->getDecl();
    // PulseDecl *PDef = nullptr;
    // CallExpr *Call = nullptr;
    // if (Parent){
    //   if (auto *C = dyn_cast<CallExpr>(Parent)){
    //       Call = C;
    //       llvm::outs() << "Print the call Name in DeclRefExpr\n";
    //       llvm::outs() << C->getDirectCallee()->getNameAsString() << "\n";
    //       llvm::outs() << "End call name print in DeclRefExpr.\n";
    //       auto *ClangFunctionDec = C->getDirectCallee();
    //       auto It = DeclarationsMap.find(ClangFunctionDec);          
    //       if (It != DeclarationsMap.end()){
    //         PDef = It->second;
    //       }
    //   }
    // }
    
    // PulseFnDefn *PFDef = nullptr;
    // if (PDef){
    //   if (auto *PFDefTmp = dyn_cast<PulseFnDefn>(PDef)){
    //     PFDef = PFDefTmp;
    //   }
    // }

    //TODO : Vidush check if this declaration has a attribute attached to it that says something 
    // about if this is head or stack allocated?

    //TODO: FIXME: 
    //If the Variable is mutated and If the variable is a boxed. 
    //We cannot just use ! to retrieve its value.
    if (MutAnalyzer->isMutated(DreDecl) && !isWrite) {
      // Create a new variable to be returned.
      // TODO: Vidush create a gensym for to get variable name.

      auto *InitAppE = new AppE("!");

      // The actual variable whose value we want
      VarTerm *VTerm = new VarTerm(DRE->getDecl()->getNameAsString());
      InitAppE->pushArg(VTerm);

      // Wrap this AppE in a Parenthesis.
      auto *PulseParenthesis = new Paren(InitAppE);
      return PulseParenthesis;
    }

    VarTerm *VTerm = new VarTerm(DRE->getDecl()->getNameAsString());
    return VTerm;

  } else if (auto *ArrSubExpr = dyn_cast<ArraySubscriptExpr>(E)) {
    auto *ArrBase = ArrSubExpr->getBase();
    auto *ArrIdx = ArrSubExpr->getIdx();

    auto *PulseCall = new AppE("op_Array_Access");
    PulseCall->pushArg(getTermFromCExpr(ArrBase, MutAnalyzer, ExprsBefore, Parent,
                                        ParentType, Module));
    PulseCall->pushArg(
        getTermFromCExpr(ArrIdx, MutAnalyzer, ExprsBefore, Parent, ParentType, Module));

    // wrap PulseCall in Paren
    auto *NewParen = new Paren(PulseCall);
    return NewParen;

  } else if (auto *ParenExpr = dyn_cast<clang::ParenExpr>(E)) {

    auto *ClangSubExpr = ParenExpr->getSubExpr();

    auto *PulseSubExpr = getTermFromCExpr(ClangSubExpr, MutAnalyzer,
                                          ExprsBefore, Parent, ParentType, Module);

    auto *PulseParenExpr = new Paren(PulseSubExpr);
    return PulseParenExpr;
  } else if (auto *CCastExpr = dyn_cast<CStyleCastExpr>(E)) {
    if (const CallExpr *Call =
            dyn_cast<CallExpr>(E->IgnoreParenImpCasts()->IgnoreCasts())) {
      if (const FunctionDecl *FD = Call->getDirectCallee()) {
        if (FD->getName() == "malloc") {
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << "Found a malloc call inside CStyleCastExpr!\n";
          });
          auto *ElementTy =
              CCastExpr->getType()->getPointeeOrArrayElementType();
          auto *DesugaredElemTy = ElementTy->getUnqualifiedDesugaredType();
          if (const TypedefType *TT = ElementTy->getAs<TypedefType>()) {
            if (auto *RT = dyn_cast<RecordType>(DesugaredElemTy)) {
              // const RecordDecl *RD = RT->getDecl();
              auto RecordName = TT->getDecl()->getDeclName();
              auto *NewCall = new AppE(RecordName.getAsString() + "_alloc");
              return NewCall;
            }
            auto *CastType = CCastExpr->getType()
                                 ->getPointeeOrArrayElementType()
                                 ->getUnqualifiedDesugaredType();
            DEBUG_WITH_TYPE(DEBUG_TYPE, {
            CastType->dump();
            });
            emitErrorWithLocation(
                "Not implemented a non record type in malloc call", &SM,
                FD->getLocation());
          }
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << "Print the type of cast!" << "\n";
          }); 
          auto CastType = CCastExpr->getType();
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          CastType->dump();
          llvm::outs() << "The corresponding pulse type is: ";
          });
          auto *PulseTy = getPulseTyFromCTy(CastType);
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << PulseTy->print() << "\n";
          });
          if (auto *PulsePointerTy = dyn_cast<FStarPointerType>(PulseTy)){
            auto *NewCall =
                new AppE("alloc_ref #" + PulsePointerTy->PointerTo->print());
            return NewCall;
          }

          emitErrorWithLocation(
              "Expected allocated type for malloc to be a reference but found "
              "a pulse type that's not a reference!",
              &SM, FD->getBeginLoc());
        }
      }
    } else {
      CCastExpr->dumpPretty(Ctx);
      emitErrorWithLocation("Unimplemented case in CStyle Cast Expression!",
                            &SM, CCastExpr->getExprLoc());
    }
  } else if (auto *RE = dyn_cast<clang::RecoveryExpr>(E)) {
    if (Expr *SubExpr = RE->getExprStmt()) {
      return getTermFromCExpr(SubExpr, MutAnalyzer, ExprsBefore, Parent, ParentType,
                              Module);
    }
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    RE->dump();
    });
    llvm::errs() << "RecoveryExpr without sub-expression, returning nullptr.\n";
    return nullptr;
  } else if (auto *ME = dyn_cast<MemberExpr>(E)) {

    auto *MemberExprDecl = ME->getMemberDecl();
    auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();

    std::string NameOfDecl;
    QualType TyOfDecl;
    std::string StructName;
    if (const clang::DeclRefExpr *DRE =
            llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
      const ValueDecl *VD = DRE->getDecl();
      // Now you can safely cast VD to a more specific Decl type if needed
      DEBUG_WITH_TYPE(DEBUG_TYPE , {
      VD->dump();
      llvm::outs() << VD->getDeclName() << "End\n";
      });

      NameOfDecl = VD->getDeclName().getAsString();
      TyOfDecl = VD->getType();
      
      DEBUG_WITH_TYPE(DEBUG_TYPE , {
      llvm::outs() << TyOfDecl->getPointeeType().getAsString() << "\n";
      });
      StructName = TyOfDecl->getPointeeType().getAsString();

      auto MemberName = MemberExprDecl->getDeclName();
      assert(!isWrite && "expected isWrite to be false");
      auto *GenStmt =
          new Name("(!(!" + NameOfDecl + ")." + MemberName.getAsString() + ")");

      //TODO: Vidush, ensure heuristic is correct.
      //check if we already added an explode for the struct here. 
      // auto It = TrackStructExplodeAndRecover.find(VD);
      
      // //We did not add a explode expression. 
      // if (It == TrackStructExplodeAndRecover.end()){
      //   auto *ExplodeStmt = new GenericStmt();
      //   ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString() + ";";
      //   ExprsBefore.push_back(ExplodeStmt);
      //   TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
      // }

      return GenStmt;
    }

    return nullptr;
  } else {
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    E->dump();
    });
    emitErrorWithLocation("Expression not implemented in getTermFromCExpr!",
                          &SM, E->getExprLoc());
  }
}


PulseTransformer::PulseTransformer(ASTContext &Ctx) : AstCtx(Ctx) {
  RewriterForPlugin.setSourceMgr(Ctx.getSourceManager(), Ctx.getLangOpts());
}

// TODO: Make this return a bool instead of a string.
// Bool says if the function passed or failed. instead of exiting.
std::string PulseTransformer::writeToFile() {

  clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
  auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
  if (!FileEnt) {
    llvm::errs() << "Error: Main file entry not found in source manager.\n";
    exit(1);
  }

  auto FilePath = FileEnt->tryGetRealPathName();
  std::filesystem::path FilePathSys = FilePath.str();
  auto Extension = FilePathSys.extension().string();
  auto TempFilePathWithoutExtension = FilePathSys.replace_extension("");
  auto FileName = TempFilePathWithoutExtension.filename();
  auto FileNameStr = FileName.string();
  if (!FileNameStr.empty()) {
    FileNameStr[0] = std::toupper(FileNameStr[0]);
  }

  // change dots to _ since . is reserved for nested modules.
  std::replace(FileNameStr.begin(), FileNameStr.end(), '.', '_');

  auto NewPath = TempFilePathWithoutExtension.parent_path();
  NewPath += "/";

  auto &ModulesToBeOutputted = CodeGen.getEmittedModules();
  for (auto &M : ModulesToBeOutputted) {

    // ASSUME: The module name if the file name atm.
    auto ModuleName = M.first;
    auto &OutputString = M.second;

    // Calculate path and then add NewFileName
    auto FilePath = NewPath.string() + ModuleName;
    // Don't remove these since the run.sh script
    // depends on printing the output path of the filename.
    llvm::outs() << "Print the filename!\n";
    llvm::outs() << FilePath << "\n";
    llvm::outs() << "End printing the filename!\n";
    std::ofstream OutFile(FilePath);
    if (!OutFile.is_open()) {
      llvm::errs()
          << "Error: Failed to create temporary file for transformed code.\n";
    }

    OutFile << OutputString->str();
    OutFile.close();
  }

  // TODO what should we return for writeToFile
  return "";
}

void PulseTransformer::transform() {

  PulseConsumer Consumer(AstCtx, RewriterForPlugin);
  Consumer.HandleTranslationUnit(AstCtx);

  auto &Modules = Consumer.getNewModules();
  for (auto &Pair : Modules) {
    DEBUG_WITH_TYPE(DEBUG_TYPE , {
    llvm::outs() << "Generating code for Module: ";
    llvm::outs() << Pair.first;
    llvm::outs() << "End generating code for Module.\n";
    });
    auto ModuleName = Pair.first;
    auto *Module = Pair.second;
    std::string Extension = ".fst";
    if (Module->isHeader)
      Extension = ".fsti";
    CodeGen.generateCodeFromModule(ModuleName + Extension, Module);
  }
}