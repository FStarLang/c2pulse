#include "PulseGenerator.h"
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
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/TargetOpcodes.h"
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
#include "Globals.h"

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
    // InnerStmt->dump();
    // assert(false && "Did not handle statement in inferArrayTypesStmt\n");
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

        auto *Init = VD->getInit();
        if (const CallExpr *Call = dyn_cast<CallExpr>(
                Init->IgnoreParenImpCasts()->IgnoreCasts())) {
          if (const FunctionDecl *FD = Call->getDirectCallee()) {
            if (FD->getName() == "malloc") {
              IsAllocatedOnHeap.insert(VD);
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
    // InnerStmt->dump();
    // assert(false && "Did not handle statement in inferArrayTypesStmt\n");
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

  if (!InnerStmt) {
    return false;
  }

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
    InnerStmt->dump();
    assert(false && "Did not handle statement in inferArrayTypesStmt\n");
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
    auto *ErasableAttr = new Name();
    ErasableAttr->NamedValue = "[@@erasable]";
    auto *NoEqTerm = new Name();
    NoEqTerm->NamedValue = "noeq";
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

    //4. A utility function to heap u32_pair_struct_allocate and u32_pair_struct_free a u32_pair_struct
    //assume val u32_pair_struct_allocated (x: ref u32_pair_struct) : slprop
    
    auto *UtilityFunctionHeap = new GenericDecl();
    UtilityFunctionHeap->Ident = "assume ";
    UtilityFunctionHeap->Ident += "val "; 
    UtilityFunctionHeap->Ident += StructName + "_allocated ";
    UtilityFunctionHeap->Ident += "(x: ref ";
    UtilityFunctionHeap->Ident += StructName + ") : slprop";
    UtilityFunctionHeap->Ident += "\n\n";

    // fn u32_pair_struct_alloc ()
    // returns x:ref u32_pair_struct
    // ensures u32_pair_struct_allocated x
    // ensures exists* v. u32_pair_struct_pred x v
    // { admit () }

    UtilityFunctionHeap->Ident += "fn " + StructName + "_alloc ()\n";
    UtilityFunctionHeap->Ident += "returns x:ref " + StructName + "\n";
    UtilityFunctionHeap->Ident += "ensures " + StructName + "_allocated x\n";
    UtilityFunctionHeap->Ident += "ensures exists* v. " + StructName + "_pred x v\n";
    UtilityFunctionHeap->Ident += "{ admit () }\n\n";

    // fn u32_pair_struct_free (x:ref u32_pair_struct)
    // requires u32_pair_struct_allocated x
    // requires exists* v. u32_pair_struct_pred x v
    // { admit () }

    UtilityFunctionHeap->Ident += "fn " + StructName + "_free " + "(x:ref " + StructName + ")\n";
    UtilityFunctionHeap->Ident += "requires " + StructName + "_allocated x\n";
    UtilityFunctionHeap->Ident += "requires exists* v. " + StructName + "_pred x v\n";
    UtilityFunctionHeap->Ident += "{ admit() }\n\n";

    NewModul->Decls.push_back(UtilityFunctionHeap);

    //5. A ghost function that unfolds the predicate for u32_pair_struct_refs
    // ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
    // requires u32_pair_struct_pred x s
    // ensures exists* (v: u32_pair_struct). (x |-> ({first=v.first; second=v.second} <: u32_pair_struct))
    //   ** (v.first |-> s.first) ** (v.second |-> s.second)
    // { unfold u32_pair_struct_pred }

    auto *GhostExplode = new GenericDecl(); 
    GhostExplode->Ident = "ghost fn " + StructName + "_explode (x:ref " + StructName + ") " + "(#s:" + StructName + "_spec)\n";
    GhostExplode->Ident += "requires " + StructName + "_pred x s\n";
    GhostExplode->Ident += "ensures exists* (v: " + StructName + "). " + "(x |-> ({";
    Counter = 0;
    for (auto *Fld : RD->fields()){
      GhostExplode->Ident += Fld->getNameAsString();
      GhostExplode->Ident += "=v." + Fld->getNameAsString();
      if (Counter < NumRecordFields - 1){
        GhostExplode->Ident += "; ";
      }
      Counter++;
    }
    GhostExplode->Ident += "} <: " + StructName + "))\n";
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

    //6. Utility functions that convert a reference to the struct to a reference to its fields

    // // &(x->first)
    // fn u32_pair_struct_to_first (x: ref u32_pair_struct) (#first #second: erased _)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal first |-> 'y
    // returns first': ref UInt32.t
    // ensures (x |-> ({ first=first'; second } <: u32_pair_struct))
    // ensures first' |-> 'y
    // ensures pure (first == first')
    // { let vx' = !x; rewrite each first as vx'.first; vx'.first }

    // // &(x->second)
    // fn u32_pair_struct_to_second (x: ref u32_pair_struct) (#first #second: erased _)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal second |-> 'y
    // returns second': ref UInt32.t
    // ensures (x |-> ({ first; second=second' } <: u32_pair_struct))
    // ensures second' |-> 'y
    // ensures pure (second == second')
    // { let vx' = !x; rewrite each second as vx'.second; vx'.second }

    std::string ErasedBinder = "(";
    for (auto *Fld : RD->fields()){
      ErasedBinder += "#" + Fld->getNameAsString() + " ";
    }
    ErasedBinder += ":erased _)";

    std::string EnsuresHelper = "({ "; 
    Counter = 0;
    for (auto *Fld : RD->fields()){
      EnsuresHelper += Fld->getNameAsString();
      if (Counter < NumRecordFields - 1){
        EnsuresHelper += ";";
      }
      Counter++;
    }
    EnsuresHelper += " }";

    for (auto *Fld : RD->fields()){

      auto Ty = Fld->getType(); 
      auto *PulseTy = getPulseTyFromCTy(Ty);
      
      auto * StructToFieldRef = new GenericDecl();
      StructToFieldRef->Ident = "fn " + StructName + "_to_" + Fld->getNameAsString() + " ";
      StructToFieldRef->Ident += "(x: ref " + StructName + ")" + " ";
      StructToFieldRef->Ident += ErasedBinder + "\n"; 
      StructToFieldRef->Ident += "requires x |-> " + EnsuresHelper + " <: " + StructName + ")\n";
      StructToFieldRef->Ident += "requires reveal " + Fld->getNameAsString() + " |-> 'y\n";
      StructToFieldRef->Ident +=  "returns " + Fld->getNameAsString() + "': ref " + PulseTy->print() + "\n";
      StructToFieldRef->Ident += "ensures (x |-> ({";
      Counter = 0;
      for (auto *FldPr : RD->fields()){
            StructToFieldRef->Ident += FldPr->getNameAsString();
            if (FldPr == Fld){
              StructToFieldRef->Ident += "=" + FldPr->getNameAsString() + "'";
            }
            if (Counter < NumRecordFields - 1){
              StructToFieldRef->Ident += "; ";
            }
            Counter++;
      } 
      StructToFieldRef->Ident += " } <: " + StructName + "))\n";
      StructToFieldRef->Ident += "ensures " + Fld->getNameAsString() + "' |-> 'y\n";
      StructToFieldRef->Ident += "ensures pure (" + Fld->getNameAsString() + " == " + Fld->getNameAsString() + "'" + ")\n";
      StructToFieldRef->Ident += "{ let vx' = !x; rewrite each " + Fld->getNameAsString() + " as vx'." + Fld->getNameAsString() + "; vx'." + Fld->getNameAsString() + " }\n";
      NewModul->Decls.push_back(StructToFieldRef);
    }

    //7. Setters/getters

    // // x->first
    // fn u32_pair_struct_get_first (x: ref u32_pair_struct) (#first #second: erased _)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal first |-> 'y
    // returns first': UInt32.t
    // ensures (x |-> ({ first; second } <: u32_pair_struct))
    // ensures reveal first |-> 'y
    // ensures pure ('y == first')
    // { let vfirst = u32_pair_struct_to_first x; let ret = !vfirst; rewrite each vfirst as first; ret }

    // // x->second
    // fn u32_pair_struct_get_second (x: ref u32_pair_struct) (#first #second: erased _)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal second |-> 'y
    // returns second': UInt32.t
    // ensures (x |-> ({ first; second } <: u32_pair_struct))
    // ensures reveal second |-> 'y
    // ensures pure ('y == second')
    // { let vsecond = u32_pair_struct_to_second x; let ret = !vsecond; rewrite each vsecond as second; ret }

    for (auto *Fld : RD->fields()){

      auto Ty = Fld->getType(); 
      auto *PulseTy = getPulseTyFromCTy(Ty);
      
      auto * StructToFieldRef = new GenericDecl();
      StructToFieldRef->Ident = "fn " + StructName + "_get_" + Fld->getNameAsString() + " ";
      StructToFieldRef->Ident += "(x: ref " + StructName + ")" + " ";
      StructToFieldRef->Ident += ErasedBinder + "\n"; 
      StructToFieldRef->Ident += "requires x |-> " + EnsuresHelper + " <: " + StructName + ")\n";
      StructToFieldRef->Ident += "requires reveal " + Fld->getNameAsString() + " |-> 'y\n";
      StructToFieldRef->Ident +=  "returns " + Fld->getNameAsString() + "': " + PulseTy->print() + "\n";
      StructToFieldRef->Ident += "ensures (x |-> ";
      StructToFieldRef->Ident += EnsuresHelper;
      StructToFieldRef->Ident += " <: " + StructName + "))\n";
      StructToFieldRef->Ident += "ensures reveal " + Fld->getNameAsString() + " |-> 'y\n";
      StructToFieldRef->Ident += "ensures pure ('y == " + Fld->getNameAsString() + "'" + ")\n";
      StructToFieldRef->Ident += "{ let v" + Fld->getNameAsString() + " = " + StructName + "_to_" + Fld->getNameAsString() + " x; let ret = !v" + Fld->getNameAsString() + "; " + "rewrite each " + "v" + Fld->getNameAsString() + " as " + Fld->getNameAsString() + "; ret " + "}\n";
      NewModul->Decls.push_back(StructToFieldRef);
    }

    // // x->first = first'
    // fn u32_pair_struct_set_first (x: ref u32_pair_struct) (#first #second: erased _) (first': UInt32.t)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal first |-> 'y
    // ensures (x |-> ({ first; second } <: u32_pair_struct))
    // ensures reveal first |-> first'
    // { let vfirst = u32_pair_struct_to_first x; vfirst := first'; rewrite each vfirst as first }

    // // x->second = second'
    // fn u32_pair_struct_set_second (x: ref u32_pair_struct) (#first #second: erased _) (second': UInt32.t)
    // requires x |-> ({ first; second } <: u32_pair_struct)
    // requires reveal second |-> 'y
    // ensures (x |-> ({ first; second } <: u32_pair_struct))
    // ensures reveal second |-> second'
    // { let vsecond = u32_pair_struct_to_second x; vsecond := second'; rewrite each vsecond as second }

    for (auto *Fld : RD->fields()){

      auto Ty = Fld->getType(); 
      auto *PulseTy = getPulseTyFromCTy(Ty);
      
      auto * StructToFieldRef = new GenericDecl();
      StructToFieldRef->Ident = "fn " + StructName + "_set_" + Fld->getNameAsString() + " ";
      StructToFieldRef->Ident += "(x: ref " + StructName + ")" + " ";
      StructToFieldRef->Ident += ErasedBinder + " ";
      StructToFieldRef->Ident += "(" + Fld->getNameAsString() + "': "; 
      StructToFieldRef->Ident += PulseTy->print() + ")" + "\n"; 
      StructToFieldRef->Ident += "requires x |-> " + EnsuresHelper + " <: " + StructName + ")\n";
      StructToFieldRef->Ident += "requires reveal " + Fld->getNameAsString() + " |-> 'y\n";
      StructToFieldRef->Ident += "ensures (x |-> ";
      StructToFieldRef->Ident += EnsuresHelper;
      StructToFieldRef->Ident += " <: " + StructName + "))\n";
      StructToFieldRef->Ident += "ensures reveal " + Fld->getNameAsString() + " |-> " + Fld->getNameAsString() + "'" + "\n";
      StructToFieldRef->Ident += "{ let v" + Fld->getNameAsString() + " = " + StructName + "_to_" + Fld->getNameAsString() + " x;" + "v" + Fld->getNameAsString() + ":=" + Fld->getNameAsString() + "'" + "; " + "rewrite each " + "v" + Fld->getNameAsString() + " as " + Fld->getNameAsString() + " }\n";
      NewModul->Decls.push_back(StructToFieldRef);
    }


    //8. A ghost function that folds the predicate for u32_pair_struct_refs
    // ghost
    // fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 #a1 :erased U32.t)
    // requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
    // ensures u32_pair_struct_pred x ({first = a0; second = a1})
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

    NewGhostFunction->Ident += "ensures " + StructName + "_pred x ({";
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
          auto Ref = AnnAttr->getAnnotation();
          if (!Ref.empty()) {
            std::string Match = "";
            PulseAnnKind AnnKind = getPulseAnnKindFromString(
                AnnAttr->getAnnotation().data(), Match);
            switch (AnnKind) {
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

          if (PulseAnnotKind == PulseAnnKind::IsArray){
          assert(PulseAnnotKind == PulseAnnKind::IsArray &&
                 "Only expect is array annotations for param decl atm.!\n");
          // Add type to map.
          // Make a clang Array Type
          // Try to get element type
          if (!Param->getType()->isPointerType() &&
              !Param->getType()->isArrayType()) {
            assert(false && "Expected parameter to be a ref or array!\n");
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
                Ctx, NestedNameSpecifierLoc(), SourceLocation(), SizeVar, false,
                SourceLocation(), Ctx.IntTy,
                clang::Expr::getValueKindForType(ElementType));

            // Step 4: Create the VLA type
            QualType VLAType = Ctx.getVariableArrayType(
                ElementType, SizeExpr, ArraySizeModifier::Normal, 0);
            // llvm::outs() << "Print the element type here!!!\n";
            // llvm::outs() << QualType(VLAType->getPointeeOrArrayElementType(),
            // 0); llvm::outs() << "End of element type!" << "\n"; exit(1);
            DeclTyMap.insert(std::make_pair(Param, VLAType));
          } else {
            clang::QualType ConstArrayTy = Ctx.getConstantArrayType(
                ElementType, llvm::APInt(32, std::stoi(Match)), nullptr,
                ArraySizeModifier::Normal, 0);
            DeclTyMap.insert(std::make_pair(Param, ConstArrayTy));
          }
        }
        else if (PulseAnnotKind == PulseAnnKind::HeapAllocated){
          IsAllocatedOnHeap.insert(Param);
        }
        else {
          assert(false && "Pulse annotation kind not implemented yet!\n");
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
      TrackStructExplodeAndRecover.clear();
      auto *PulseBody = pulseFromCompoundStmt(CS, &Analyzer, Module);

      // Release declarations that are function parameters.
      PulseSequence *NewSeq = nullptr;
      PulseSequence *Head = nullptr;
      for (auto It = TrackStructExplodeAndRecover.begin(); It != TrackStructExplodeAndRecover.end();) {
        auto ItElem = *It;
        auto &Decl = ItElem.first;
        auto &Info = ItElem.second;
        // recover not released.
        if (!Info.second) {
          if (auto *ParamD = dyn_cast<ParmVarDecl>(Decl)) {

            auto StructName = ParamD->getType()->getPointeeType().getAsString();

            auto *RecoverStatememt = new GenericStmt();
            RecoverStatememt->body =
                StructName + "_recover " + ParamD->getNameAsString() + ";";
            if (Head == nullptr) {
              Head = new PulseSequence();
              NewSeq = Head;
              NewSeq->assignS1(PulseBody);

              auto *NextSequence = new PulseSequence();
              NextSequence->assignS1(RecoverStatememt);
              NewSeq->assignS2(NextSequence);
              NewSeq = NextSequence;
               TrackStructExplodeAndRecover.erase(It++);
              continue;
            }

            auto *NextSequence = new PulseSequence();
            NextSequence->assignS1(RecoverStatememt);
            NewSeq->assignS2(NextSequence);
            NewSeq = NextSequence;

            TrackStructExplodeAndRecover.erase(It++);
            continue;
          }
        }
        It++;
      }

      assert(TrackStructExplodeAndRecover.empty() && "Failed to recover all structure types in the function!\n");

      if (Head != nullptr) {
        Head->dumpPretty();
        FDefn->Body = Head;
      } else if (PulseBody != nullptr) {
        PulseBody->dumpPretty();
        FDefn->Body = PulseBody;
      }
    }
  }

  PulseFnDefn *PulseFn = new PulseFnDefn(FDefn);

  llvm::outs() << "=================================================";
  llvm::outs() << "\nPrint the Pulse function Definition:\n\n";
  PulseFn->dumpPretty();
  llvm::outs() << "\nEnd printing the function Definition\n\n";
  llvm::outs() << "=================================================\n";
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
      assert(false && "PulseVisitor: Did not implement array type in clang.\n");
    }

    PulseTy = new FStarPointerType();
    auto *PulsePointerTy = static_cast<FStarPointerType *>(PulseTy);
    auto BaseTy = CType->getPointeeType();
    PulsePointerTy->setName("ref " + BaseTy.getAsString());
    PulsePointerTy->setTag(TermTag::FStarPointerType);
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
  PulseTy->setTag(TermTag::FStarType);
  
  return PulseTy;

}

PulseStmt *PulseVisitor::pulseFromCompoundStmt(Stmt *S,
                                               ExprMutationAnalyzer *Analyzer,
                                               PulseModul *Modul) {

  PulseSequence *Stmt = nullptr;
  PulseSequence *Head = nullptr;
  if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    for (auto *InnerStmt : CS->body()) {

      auto *NextPulseStmt = pulseFromStmt(InnerStmt, Analyzer, nullptr, Modul);
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
  }

  return Head;
}

PulseStmt *PulseVisitor::pulseFromStmt(Stmt *S, ExprMutationAnalyzer *Analyzer,
                                       Stmt *Parent, PulseModul *Module) {

  if (!S)
    return nullptr;

  if (auto *DS = dyn_cast<DeclStmt>(S)) {

    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {

        if (auto *Init = VD->getInit()) {
          auto VarName = VD->getNameAsString();
          // Unsure if we really need the type here.
          // Though it may be usefuel checking invalid casting operations.
          // auto VarType = VD->getType();

          // This gets converted to the pulse let expression.
          // Vidush : It is probably good to make a setter / pass arguments to
          // the constructor.
          auto *PulseLet = new LetBinding();
          PulseLet->VarName = VarName;

          // Don't forget to release these exprs.
          SmallVector<PulseStmt *> NewExprs;

          PulseLet->LetInit =
              getTermFromCExpr(Init, Analyzer, NewExprs, Parent, VD->getType(), Module);
          PulseLet->setTag(PulseStmtTag::LetBinding);
          if (Analyzer->isMutated(D)) {
            PulseLet->Qualifier = MutOrRef::MUT;
          }

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
                  llvm::outs() << "Found Lemma: " << "\n";
                  llvm::outs() << Match << "\n";
                  llvm::outs() << "End.\n";
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
                  assert(false && "Did not expect pulse annotation kind!\n");
                }
              }
            }
          }

          auto *AppendLet = new PulseSequence();
          AppendLet->assignS1(Start);
          AppendLet->assignS2(PulseLet);

          return AppendLet;
        }
      }
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
          PulseAssignment *Assignment = new PulseAssignment();
          Assignment->setTag(PulseStmtTag::Assignment);
          Assignment->Lhs = PulseLhsTerm;
          Assignment->Value = PulseRhsTerm;

          assert(ExprsBef.empty() && "Expected expressions to be released!");

          return Assignment;
        }
      } else if (auto *ArrSub = dyn_cast<ArraySubscriptExpr>(Lhs)) {

        // TODO: Make sure to release these expressions
        SmallVector<PulseStmt *> ExprsBef;

        auto *ArrayAssignExpr = new PulseArrayAssignment();
        ArrayAssignExpr->setTag(PulseStmtTag::ArrayAssignment);
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
        S->dumpPretty(Ctx);
        assert(false && "Not implemented when Lhs is array sub expr");
      } else if (auto *ME = dyn_cast<MemberExpr>(Lhs)) {

        auto *LhsDecl = ME->getMemberDecl();
        auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();
        BaseExpr->dump();

        std::string NameOfDecl;
        QualType TyOfDecl;
        std::string StructName;
        if (const clang::DeclRefExpr *DRE =
                llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
          const clang::ValueDecl *VD = DRE->getDecl();
          // Now you can safely cast VD to a more specific Decl type if needed
          VD->dump();
          llvm::outs() << VD->getDeclName() << "End\n";
          NameOfDecl = VD->getDeclName().getAsString();
          TyOfDecl = VD->getType();
          llvm::outs() << TyOfDecl->getPointeeType().getAsString() << "\n";
          StructName = TyOfDecl->getPointeeType().getAsString();

          auto MemberName = LhsDecl->getDeclName();

          auto *PulseCall = new AppE();
          auto *CallName = new VarTerm();

          CallName->setVarName( StructName + "_set_" + MemberName.getAsString());
          PulseCall->setCallName(CallName);

          auto *InnerTermCallArg = new VarTerm();
            InnerTermCallArg->setVarName(NameOfDecl);

          PulseCall->pushArg(InnerTermCallArg);

          // TODO: Angelica, I don't this releasing expressions before is
          // required. This was done because Pulse was not in ANF before.
          // However, it is in ANF now.
          SmallVector<PulseStmt *> ExprsBef;
          auto *RhsExpr =
              getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent, BO->getType(), Module);
          PulseCall->pushArg(RhsExpr);

          // Perhaps warp this In Pulse Expr
          auto *Expr = new PulseExpr();
          Expr->E = PulseCall;
          assert(ExprsBef.empty() && "Expected ExprsBefore to be empty!\n");

          auto It = TrackStructExplodeAndRecover.find(VD);
          if (It == TrackStructExplodeAndRecover.end()){
            auto *NewSeq = new PulseSequence();
            NewSeq->assignS2(Expr);
            auto *ExplodeStmt = new GenericStmt();
            ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString() + ";";
            NewSeq->assignS1(ExplodeStmt);
            TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
            return NewSeq;
          }

          return Expr;
        }

        ME->dump();
        assert(false && "Did not expect to reach here!\n");

      } else if (auto *ME = dyn_cast<MemberExpr>(Rhs)) {

        auto *LhsDecl = ME->getMemberDecl();
        auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();
        BaseExpr->dump();

        std::string NameOfDecl;
        QualType TyOfDecl;
        std::string StructName;
        if (const clang::DeclRefExpr *DRE =
                llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
          const clang::ValueDecl *VD = DRE->getDecl();
          NameOfDecl = VD->getDeclName().getAsString();
          TyOfDecl = VD->getType();
          StructName = TyOfDecl->getPointeeType().getAsString();

          auto MemberName = LhsDecl->getDeclName();

          auto *PulseCall = new AppE();
          auto *CallName = new VarTerm();
          CallName->setVarName(StructName + "_get_" + MemberName.getAsString());
          PulseCall->setCallName(CallName);
          auto *InnerTermCallArg = new VarTerm();
          InnerTermCallArg->setVarName(NameOfDecl);
          PulseCall->pushArg(InnerTermCallArg);

          // TODO: Angelica, I don't this releasing expressions before is
          // required. This was done because Pulse was not in ANF before.
          // However, it is in ANF now.
          SmallVector<PulseStmt *> ExprsBef;
          auto *RhsExpr =
              getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent, BO->getType(), Module);
          PulseCall->pushArg(RhsExpr);

          // Perhaps warp this In Pulse Expr
          auto *Expr = new PulseExpr();
          Expr->E = PulseCall;
          assert(ExprsBef.empty() && "Expected Exprs before to be empty!\n");
          
          auto It = TrackStructExplodeAndRecover.find(VD);
          if (It == TrackStructExplodeAndRecover.end()) {
            auto *NewSeq = new PulseSequence();
            NewSeq->assignS2(Expr);
            auto *ExplodeStmt = new GenericStmt();
            ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString();
            NewSeq->assignS2(ExplodeStmt);
            TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
            return NewSeq;
          }

          return Expr;
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
        PulseAssignment *Assignment = new PulseAssignment();
        Assignment->Lhs = PulseLhsTerm;
        Assignment->Value = PulseRhsTerm;
        Assignment->setTag(PulseStmtTag::Assignment);

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
      PExpr->setTag(PulseStmtTag::Expr);
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

      assert(false && "Binary Operator not implemented in pulseFromStmt\n");
    }

  } else if (auto *E = dyn_cast<Expr>(S)) {

    SmallVector<PulseStmt *> ExprsBefore;

    auto *PulseExpression = new PulseExpr();
    PulseExpression->setTag(PulseStmtTag::Expr);

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

    auto *Cond = IF->getCond();
    auto *Else = IF->getElse();
    auto *Then = IF->getThen();

    SmallVector<PulseStmt *> ExprsBefore;

    auto PulseCond =
        getTermFromCExpr(Cond, Analyzer, ExprsBefore, Parent, Cond->getType(), Module);
    auto *PulseElse = pulseFromStmt(Else, Analyzer, Parent, Module);
    auto *PulseThen = pulseFromStmt(Then, Analyzer, Parent, Module);

    auto PulseIfStmt = new PulseIf();
    PulseIfStmt->setTag(PulseStmtTag::If);
    PulseIfStmt->Head = PulseCond;
    PulseIfStmt->Else = PulseElse;
    PulseIfStmt->Then = PulseThen;

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return PulseIfStmt;
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {

    if (auto *RetVal = RS->getRetValue()) {
      SmallVector<PulseStmt *> ExprsBefore;
      auto *NewPulseExpr = new PulseExpr();
      NewPulseExpr->setTag(PulseStmtTag::Expr);
      auto *RetTerm = getTermFromCExpr(RetVal, Analyzer, ExprsBefore,
                                       Parent, RetVal->getType(), Module);

      if (auto *DeclRef = dyn_cast<DeclRefExpr>(RetVal->IgnoreParenImpCasts()->IgnoreImpCasts())){
        auto It = TrackStructExplodeAndRecover.find(DeclRef->getDecl());
        if (It != TrackStructExplodeAndRecover.end()){
          auto Info = It->second; 
          if (!Info.second){

            //Get struct name from declration.
              const auto *VD = DeclRef->getDecl();
              auto *PSeq = new PulseSequence();
              auto StructName = VD->getType()->getPointeeType().getAsString(); 
              if (RetTerm){
                 NewPulseExpr->E = RetTerm;
                 PSeq->assignS2(NewPulseExpr);
                 auto *FallBack  = new GenericStmt(); 
                 FallBack->body += StructName + "_recover " +  DeclRef->getDecl()->getNameAsString() + ";";
                 PSeq->assignS1(FallBack);
                 // update element in map.
                 TrackStructExplodeAndRecover.erase(It);
                 return PSeq;
             }

            auto *FallBack  = new GenericStmt(); 
            FallBack->body += StructName + "_recover " +  DeclRef->getDecl()->getNameAsString() + ";";

            // update element in map.
            TrackStructExplodeAndRecover.erase(It);
            return FallBack;
        }
      }
    }

      if (RetTerm == nullptr)
        return nullptr;
      NewPulseExpr->E = RetTerm;

      assert(ExprsBefore.empty() && "Expected expressions to be released!");
      return NewPulseExpr;
    }

    return nullptr;
  } else if (auto *FS = dyn_cast<ForStmt>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt ForStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
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

      PulseWhile->setTag(PulseStmtTag::WhileStmt);
      PulseWhile->Guard = pulseFromStmt(WhileCond, Analyzer, Parent, Module);
      PulseWhile->Body = pulseFromCompoundStmt(CompundBody, Analyzer, Module);

      return PulseWhile;
    } else {

      auto *PulseWhile = new PulseWhileStmt();
      PulseWhile->setTag(PulseStmtTag::WhileStmt);

      PulseWhile->Guard = pulseFromStmt(WhileCond, Analyzer, Parent, Module);
      PulseWhile->Body = pulseFromCompoundStmt(WhileBody, Analyzer, Module);

      return PulseWhile;
    }
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt UnaryOperator\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
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
              assert(false && "Unhandled Attr in Attributed Stmt!\n");
            }
        }
      }
    }
    NewSequence->assignS2(pulseFromStmt(SubStmt, Analyzer, Parent, Module));
    return NewSequence;
  } else {
    llvm::outs() << "\n\nPrint in pulseFromStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
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

Term *
PulseVisitor::getTermFromCExpr(Expr *E, ExprMutationAnalyzer *MutAnalyzer,
                               llvm::SmallVector<PulseStmt *> &ExprsBefore,
                               Stmt *Parent, 
                               QualType ParentType, PulseModul *Module,
                               bool isWrite) {

  if (auto *IL = dyn_cast<IntegerLiteral>(E)) {

    auto NewConstTerm = new ConstTerm();
    NewConstTerm->setTag(TermTag::Const);
    NewConstTerm->ConstantValue = std::to_string(IL->getValue().getSExtValue());

    llvm::outs() << "Found Integer Literal: " << NewConstTerm->ConstantValue
                 << "\n";
    llvm::outs() << ParentType.getAsString() << "\n";

    NewConstTerm->Symbol = getSymbolKeyForCType(ParentType, Ctx);

    return NewConstTerm;

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "IntegerLiteral\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *FL = dyn_cast<FloatingLiteral>(E)) {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "FloatingLiteral\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *SL = dyn_cast<StringLiteral>(E)) {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "StringLiteral\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *CL = dyn_cast<CharacterLiteral>(E)) {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "CharacterLiteral\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {

    auto *Lhs = BO->getLHS();
    auto *Rhs = BO->getRHS();
    auto Op = BO->getOpcode();

    if (Lhs->getType() != Rhs->getType()) {
      E->dumpPretty(Ctx);
      LLVM_DEBUG(llvm::dbgs() << "\n");
      assert(false && "Expected types of Lhs and Rhs to be the same. \
              Unsafe type casting now allowed in Pulse\n");
    }

    SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
    auto OpKey = getSymbolKeyForOperator(TypeKey, Op);

    auto *NewAppENode = new AppE();
    NewAppENode->setTag(TermTag::AppE);
    auto LhsTerm =
        getTermFromCExpr(Lhs, MutAnalyzer, ExprsBefore, Parent, BO->getType(), Module);
    auto RhsTerm =
        getTermFromCExpr(Rhs, MutAnalyzer, ExprsBefore, Parent, BO->getType(), Module);

    auto *CallNameVar = new VarTerm();
    CallNameVar->setVarName(OpKey);
    CallNameVar->setTag(TermTag::Var);
    NewAppENode->setCallName(CallNameVar);
    NewAppENode->pushArg(LhsTerm);
    NewAppENode->pushArg(RhsTerm);

    // Wrap Call Expr into a Paren to be safe.
    auto *NewParen = new Paren();
    NewParen->setInnerExpr(NewAppENode);

    assert(ExprsBefore.empty() && "Unreleased expressions!\n");

    return NewParen;

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "BinaryOperator\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_Deref) {
      auto *DerefAppE = new AppE();
      auto *FuncName = new VarTerm();
      FuncName->setVarName("!");
      FuncName->setTag(TermTag::Var);
      DerefAppE->setTag(TermTag::AppE);
      DerefAppE->setCallName(FuncName);
      auto *TermForBaseExpr = getTermFromCExpr(UO->getSubExpr(), MutAnalyzer,
                                               ExprsBefore, Parent, ParentType, Module);
      DerefAppE->pushArg(TermForBaseExpr);

      // Wrap this deref in a parenthesis.
      auto *Parenthesis = new Paren();
      Parenthesis->setInnerExpr(DerefAppE);
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

          auto *GenStmt = new Name();
          GenStmt->NamedValue =
              "(" + StructName + "_to_" +
              Mem->getMemberDecl()->getDeclName().getAsString() + " " +
              Dec->getDecl()->getNameAsString() + ")";

          auto It = TrackStructExplodeAndRecover.find(VD);
          if (It == TrackStructExplodeAndRecover.end()) {
            auto *ExplodeStmt = new GenericStmt();
            ExplodeStmt->body =
                StructName + "_explode " + VD->getNameAsString() + ";";
            ExprsBefore.push_back(ExplodeStmt);
            TrackStructExplodeAndRecover.insert(
                std::make_pair(VD, std::make_pair(true, false)));
          }

          return GenStmt;
        }
      }

      llvm::outs() << "Print in Addr of" << "\n";
      E->dump();
      assert(false && "Not implemented Member Expression!\n");
    }
    else {
      llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                      "UnaryOperator\n";
      E->dumpPretty(Ctx);
      E->dump();
      llvm::outs() << E->getStmtClassName() << "\n";
      llvm::outs() << "\nEnd printing term.\n\n";
      assert(false && "Expression not implemeted in getTermFromCExpr\n");
    }

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "UnaryOperator\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
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
        assert(false &&
               "Expected pulse while to have arguments as string literals");
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
          assert(false &&
                 "Expected pulse while to have arguments as string literals");
        }
      }

      auto *Next = getNextStatement(E, Ctx);
      if (auto *While = dyn_cast<WhileStmt>(Next)) {
        // Add corresponding while invariant.
        StmtToLemmas.insert(std::make_pair(While, VectorLemmas));
        return nullptr;
      }
      assert(false &&
             "Expected next statement after pulse invariant to be a while!\n");
    }

    auto CallName = CE->getDirectCallee()->getNameAsString();
    auto *CallAppE = new AppE();
    auto *FuncName = new VarTerm();

    FuncName->setTag(TermTag::Var);
    CallAppE->setTag(TermTag::AppE);

    if (CallName != "free") {
      FuncName->setVarName(CallName);
      CallAppE->setCallName(FuncName);

      for (size_t i = 0; i < CE->getNumArgs(); i++) {
        auto *Arg = CE->getArg(i);
        auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                         ParentType, Module);
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
          FuncName->setVarName(NewCallName);
          CallAppE->setCallName(FuncName);
          auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                           ParentType, Module);
          CallAppE->pushArg(ArgTerm);
        } else {
          FuncName->setVarName(CallName);
          CallAppE->setCallName(FuncName);
          auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, CE,
                                           ParentType, Module);
          CallAppE->pushArg(ArgTerm);
        }
      }
    }

    // Wrap Call expr in Paren Node
    auto *NewParen = new Paren();
    NewParen->setInnerExpr(CallAppE);
    return NewParen;

    llvm::outs()
        << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr CallExpr\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *IC = dyn_cast<ImplicitCastExpr>(E)) {

    auto *SubExpr = IC->getSubExpr();
    return getTermFromCExpr(SubExpr, MutAnalyzer, ExprsBefore, Parent, ParentType,
                            Module);

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "ImplicitCastExpr\n";
    SubExpr->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {

    auto *DreDecl = DRE->getDecl();
    PulseDecl *PDef = nullptr;
    CallExpr *Call = nullptr;
    if (Parent){
      if (auto *C = dyn_cast<CallExpr>(Parent)){
          Call = C;
          llvm::outs() << "Print the call Name in DeclRefExpr\n";
          llvm::outs() << C->getDirectCallee()->getNameAsString() << "\n";
          llvm::outs() << "End call name print in DeclRefExpr.\n";
          auto *ClangFunctionDec = C->getDirectCallee();
          auto It = DeclarationsMap.find(ClangFunctionDec);          
          if (It != DeclarationsMap.end()){
            PDef = It->second;
          }
      }
    }
    
    PulseFnDefn *PFDef = nullptr;
    if (PDef){
      if (auto *PFDefTmp = dyn_cast<PulseFnDefn>(PDef)){
        PFDef = PFDefTmp;
      }
    }

    //TODO : Vidush check if this declaration has a attribute attached to it that says something 
    // about if this is head or stack allocated?

    //TODO: FIXME: 
    //If the Variable is mutated and If the variable is a boxed. 
    //We cannot just use ! to retrieve its value.
    if (MutAnalyzer->isMutated(DreDecl) && !isWrite) {
      // Create a new variable to be returned.
      // TODO: Vidush create a gensym for to get variable name.

      auto *InitAppE = new AppE();
      auto *CallName = new VarTerm();
      CallName->setVarName("!");
      InitAppE->setCallName(CallName);

      // The actual variable whose value we want
      VarTerm *VTerm = new VarTerm();
      VTerm->setVarName(DRE->getDecl()->getNameAsString());
      InitAppE->pushArg(VTerm);

      // Wrap this AppE in a Parenthesis.
      auto *PulseParenthesis = new Paren();
      PulseParenthesis->setInnerExpr(InitAppE);
      return PulseParenthesis;
    }

    VarTerm *VTerm = new VarTerm();
    VTerm->setTag(TermTag::Var);
    VTerm->setVarName(DRE->getDecl()->getNameAsString());

    if (!Call)
        return VTerm;

    return VTerm;
  } else if (auto *ArrSubExpr = dyn_cast<ArraySubscriptExpr>(E)) {
    auto *ArrBase = ArrSubExpr->getBase();
    auto *ArrIdx = ArrSubExpr->getIdx();

    auto *PulseCall = new AppE();
    PulseCall->setTag(TermTag::AppE);
    auto *Call = new VarTerm();
    Call->setTag(TermTag::Var);
    Call->setVarName("op_Array_Access");
    PulseCall->setCallName(Call);

    PulseCall->pushArg(getTermFromCExpr(ArrBase, MutAnalyzer, ExprsBefore, Parent,
                                        ParentType, Module));
    PulseCall->pushArg(
        getTermFromCExpr(ArrIdx, MutAnalyzer, ExprsBefore, Parent, ParentType, Module));

    // wrap PulseCall in Paren
    auto *NewParen = new Paren();
    NewParen->setInnerExpr(PulseCall);

    return NewParen;
  } else if (auto *ParenExpr = dyn_cast<clang::ParenExpr>(E)) {

    auto *ClangSubExpr = ParenExpr->getSubExpr();
    auto *PulseParenExpr = new Paren();

    auto *PulseSubExpr = getTermFromCExpr(ClangSubExpr, MutAnalyzer,
                                          ExprsBefore, Parent, ParentType, Module);

    PulseParenExpr->setInnerExpr(PulseSubExpr);
    return PulseParenExpr;
  } else if (auto *CCastExpr = dyn_cast<CStyleCastExpr>(E)) {
    if (const CallExpr *Call =
            dyn_cast<CallExpr>(E->IgnoreParenImpCasts()->IgnoreCasts())) {
      if (const FunctionDecl *FD = Call->getDirectCallee()) {
        if (FD->getName() == "malloc") {
          llvm::outs() << "Found a malloc call inside CStyleCastExpr!\n";
          auto *ElementTy =
              CCastExpr->getType()->getPointeeOrArrayElementType();
          auto *DesugaredElemTy = ElementTy->getUnqualifiedDesugaredType();
          if (const TypedefType *TT = ElementTy->getAs<TypedefType>()) {
            if (auto *RT = dyn_cast<RecordType>(DesugaredElemTy)) {
              // const RecordDecl *RD = RT->getDecl();
              auto RecordName = TT->getDecl()->getDeclName();
              Module->insertModule("module Box = Pulse.Lib.Box");
              auto *NewCall = new AppE();
              auto *NewCallName = new VarTerm();
              NewCallName->setVarName(RecordName.getAsString() + "_alloc");
              NewCall->setCallName(NewCallName);
              return NewCall;
            }
            auto *CastType = CCastExpr->getType()
                                 ->getPointeeOrArrayElementType()
                                 ->getUnqualifiedDesugaredType();
            CastType->dump();
            assert(false &&
                   "Not implemented a non record type in malloc call!\n");
          }
          auto *CastType = CCastExpr->getType()->getPointeeOrArrayElementType();
          CastType->dump();
          assert(false && "malloc not implemented for other than record or typedef types!");
        }
      }
    } else {
      CCastExpr->dumpPretty(Ctx);
      assert(false && "Did not implement CStyle Cast Expression!\n");
    }
  }
  else if (auto *RE = dyn_cast<clang::RecoveryExpr>(E)) {
    if (Expr *SubExpr = RE->getExprStmt()) {

      assert(false && "Should not reach here!");
      return getTermFromCExpr(SubExpr, MutAnalyzer, ExprsBefore, Parent, ParentType,
                              Module);
    }
    RE->dump();
    llvm::errs() << "RecoveryExpr without sub-expression, returning nullptr.\n";
    assert(false && "Should not reach here!");
    return nullptr;
  } else if (auto *ME = dyn_cast<MemberExpr>(E)) {
   
    
    auto *LhsDecl = ME->getMemberDecl();

    auto *BaseExpr = ME->getBase()->IgnoreParens()->IgnoreImpCasts();

    std::string NameOfDecl;
    QualType TyOfDecl;
    std::string StructName;
    if (const clang::DeclRefExpr *DRE =
            llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
      const ValueDecl *VD = DRE->getDecl();
      // Now you can safely cast VD to a more specific Decl type if needed
      VD->dump();
      llvm::outs() << VD->getDeclName() << "End\n";
      NameOfDecl = VD->getDeclName().getAsString();
      TyOfDecl = VD->getType();
      llvm::outs() << TyOfDecl->getPointeeType().getAsString() << "\n";
      StructName = TyOfDecl->getPointeeType().getAsString();

      auto MemberName = LhsDecl->getDeclName();
      auto *PulseCall = new AppE();
      auto *CallName = new VarTerm();
      std::string MethodName = isWrite ? "_set_" : "_get_";
      CallName->setVarName(StructName + MethodName + MemberName.getAsString());
      PulseCall->setCallName(CallName);

      auto *InnerTermCallArg = new VarTerm();
      InnerTermCallArg->setVarName(NameOfDecl);
      PulseCall->pushArg(InnerTermCallArg);

      // Wrap in Paren expression.
      auto *PulseParen = new Paren();
      PulseParen->InnerExpr = PulseCall;
      
      //TODO: Vidush, ensure heuristic is correct.
      //check if we already added an explode for the struct here. 
      auto It = TrackStructExplodeAndRecover.find(VD);
      
      //We did not add a explode expression. 
      if (It == TrackStructExplodeAndRecover.end()){
        auto *ExplodeStmt = new GenericStmt();
        ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString() + ";";
        ExprsBefore.push_back(ExplodeStmt);
        TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
      }

      return PulseParen;
    }

    return nullptr;
  } else {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr\n";
    E->dump();
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  }
  
  E->dump();
  assert(false && "Should not reach here!");
  return nullptr;
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
    //Vidush: Don't remove these since the run.sh script
    //depends on printing the output path of the filename.
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
    llvm::outs() << "Generating code for Module: ";
    llvm::outs() << Pair.first;
    llvm::outs() << "End generating code for Module.\n";
    auto ModuleName = Pair.first;
    auto *Module = Pair.second;
    std::string Extension = ".fst";
    if (Module->isHeader)
      Extension = ".fsti";
    CodeGen.generateCodeFromModule(ModuleName + Extension, Module);
  }
}