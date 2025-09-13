#include "PulseASTGenerator.h"
#include "Globals.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTDumperUtils.h"
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
#include "clang/Basic/Version.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <climits>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#undef DEBUG_TYPE
#define DEBUG_TYPE "pulse-ast-gen"

using namespace clang;

void PulseConsumer::setNewModules(
    std::map<std::string, PulseModul *> &PulseModules) {
  Modules = PulseModules;
}

std::map<std::string, PulseModul *> &PulseConsumer::getNewModules() {
  return Modules;
}

PulseConsumer::PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R,
  std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens
)
    : Visitor(R, Ctx, macroTokens) {}

void PulseConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  setNewModules(Visitor.getPulseModules());
}

std::map<std::string, PulseModul *> &PulseVisitor::getPulseModules() {
  return Modules;
}

void PulseVisitor::InferDeclType(const Decl *Dec, FunctionDecl *FD) {

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

void PulseVisitor::inferDeclType(const Decl *Dec, Stmt *InnerStmt) {

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
  } else if (auto *IfStm = dyn_cast<IfStmt>(InnerStmt)) {
    auto *Cond = IfStm->getCond();
    auto *Then = IfStm->getThen();
    auto *Else = IfStm->getElse();
    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Then);
    inferArrayTypesStmt(Else);
  } else if (auto *RetStmt = dyn_cast<ReturnStmt>(InnerStmt)) {
    if (auto *RetVal = RetStmt->getRetValue()) {
      inferArrayTypesExpr(RetVal);
    }
  } else {
    // TODO: Vidush see if we want to handle any other statement.
    return;
  }
}

void PulseVisitor::inferArrayTypesStmt(Stmt *InnerStmt) {

  if (!InnerStmt)
     return;

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
  } else if (auto *IfStm = dyn_cast<IfStmt>(InnerStmt)) {
    auto *Cond = IfStm->getCond();
    auto *Then = IfStm->getThen();
    auto *Else = IfStm->getElse();
    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Then);
    inferArrayTypesStmt(Else);
  } else if (auto *RetStmt = dyn_cast<ReturnStmt>(InnerStmt)) {
    if (auto *RetVal = RetStmt->getRetValue()) {
      inferArrayTypesExpr(RetVal);
    }
  } else {
    // TODO: Vidush see if we want to handle any other statement.
    InnerStmt->dump();
    // emitErrorWithLocation("uimplemented case!", &Ctx,
    // InnerStmt->getBeginLoc());
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
        llvm::outs() << "Found array type!\n";
      } else {
        BaseDecl->dump();
        emitErrorWithLocation("could not cast Base to a variable declaration!", &Ctx,
                              ExprPtr->getExprLoc());
      }
    }
  } else if (auto *Paren = dyn_cast<clang::ParenExpr>(ExprPtr)) {
    inferArrayTypesExpr(Paren->getSubExpr());
  } else if (auto *IC = dyn_cast<clang::ImplicitCastExpr>(ExprPtr)) {
    inferArrayTypesExpr(IC->getSubExpr());
  } else if (auto *DRE = dyn_cast<clang::DeclRefExpr>(ExprPtr)) {
    return;
  } else if (auto *IL = dyn_cast<IntegerLiteral>(ExprPtr)) {
    return;
  } else {
    ExprPtr->dump();
    // emitErrorWithLocation("uimplemented case!", &Ctx, ExprPtr->getExprLoc());
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
    emitErrorWithLocation("Encountered an unhandled case!", &Ctx,
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

///TODO: Vidush change name of function to getTypeNameForRecordDecl.
std::string PulseVisitor::getNameForRecordDecl(const RecordDecl *RD){

  std::string StructName = RD->getDeclName().getAsString();
  const clang::DeclContext *context = RD->getDeclContext();

  auto It = RecordToRecordName.find(RD);
  if (It != RecordToRecordName.end()){
    return It->second;
  }

  if (context == nullptr) {
    llvm::outs() << "Cannot get typedefs for record, using record name for "
                    "generating pulse code\n";
  }

  if (context->decls_empty()) {
    llvm::outs() << "Declarations empty for record\n";
  }

  if (RD->isAnonymousStructOrUnion()){
    auto NewName = c2pulse::gensym("anon_struct"); 
    RecordToRecordName.insert(std::make_pair(RD, NewName));
    return NewName;
  }

  if (RD->isInAnonymousNamespace()){
    auto NewName = c2pulse::gensym("anon_struct"); 
    RecordToRecordName.insert(std::make_pair(RD, NewName));
    return NewName;
  }

  for (auto *Decl : context->decls()) {
    if (const auto *TypedefDecl = llvm::dyn_cast<clang::TypedefDecl>(Decl)) {
      clang::QualType TypedefQT = TypedefDecl->getUnderlyingType();
      if (const auto *RecordType = TypedefQT->getAs<clang::RecordType>()) {
        if (RecordType->getDecl() == RD) {

          if (TypedefDecl->getIdentifier() == NULL) {
            // No typedef name was given
            llvm::outs() << "No typedef name — fallback to struct tag\n";
            StructName = RD->getNameAsString();
          } else {
            llvm::outs() << "Typedef found: " << TypedefDecl->getNameAsString()
                         << "\n";
            llvm::outs() << TypedefDecl->getIdentifier() << "\n";
            /// The record type has an Associated Typedef so we should use that!
            StructName = TypedefDecl->getNameAsString();
          }
        }
      }
    }
  }


  //Could not find a struct Name, hence we just gensym a struct name.
  if (StructName == ""){
    auto NewName = c2pulse::gensym("anon_name");
    RecordToRecordName.insert(std::make_pair(RD, NewName));
    return NewName;
  }


  RecordToRecordName.insert(std::make_pair(RD, StructName));
  return StructName;

}


QualType PulseVisitor::getTypeFromDecl(const Decl *D){

  if (auto *FD = dyn_cast<FieldDecl>(D)){
    return FD->getType();
  }
  else if (auto *RD = dyn_cast<RecordDecl>(D)){
    return QualType(RD->getTypeForDecl(), 0);
  }
  else if (auto *VD = dyn_cast<VarDecl>(D)){
    return VD->getType();
  }
  else{
    emitErrorWithLocation("Declaration Type not handled!", &Ctx, D->getLocation());
  }
}

bool PulseVisitor::VisitRecordDecl(const RecordDecl *RD) {

  auto SourceLoc = RD->getLocation();
  if (SM.isInSystemHeader(SourceLoc))
    return true;
   
  ///TODO: Vidush, we only handle record declaration and definitions together.
  ///Not just declarations.  
  if (!RD->isThisDeclarationADefinition())
    return true;
  
  //Vidush: 
  //TODO: Expand this map to also include globals. 
  //Since pulse does not support globals right now, 
  //This map does not require to store globals.  
  std::map<std::string, const Decl *> VarNamesInScope;  
  
  std::string StructName;
  auto ItRecName = RecordToRecordName.find(RD);
  //Record type name already exists.
  if (ItRecName != RecordToRecordName.end()){
    //Record must already be visited and we have generated code for it.
    return true;
  }

  StructName = getNameForRecordDecl(RD);
  VarNamesInScope.insert(std::make_pair(StructName, RD));

  // auto *Def = TypeDefDec->getUnderlyingDecl();

  /// Check if we have a record declaration associated with the typedefDecl.
  ///  For every record type we create a new Module.
  /// if (const auto *RT = TypeDefDec->getUnderlyingType()->getAs<RecordType>())
  /// { RecordDecl *RD = RecordDecl;

  // Map record declaration to typedef declaration.
  /// MapRecordDeclsToTypedefDecls.insert(std::make_pair(RD, TypeDefDec));

  ////auto StructName = Def->getNameAsString();
  auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
  if (!FileEnt) {
    llvm::errs() << "Error: Main file entry not found in source manager.\n";
    exit(1);
  }

  /// Parse the file name. CopyPulseTy
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

    //Get Unique Field Names in scope.
    bool HasArrayTypeFields = false;
    size_t NumArrayFields = 0;
    llvm::SmallDenseMap<const Decl*, std::string> FieldToUniqueNames;
    for (const FieldDecl *FD : RD->fields()){

      auto FName = FD->getNameAsString();

      //Visit field record to make sure its definition is released.
      clang::QualType fieldType = FD->getType();
      if (const clang::RecordType *RecordType = fieldType->getAs<clang::RecordType>()) {
          clang::RecordDecl *CastRecTy = RecordType->getDecl();
          VisitRecordDecl(CastRecTy);
      }

      if (VarNamesInScope.count(FName) > 0){
        FName = c2pulse::gensym(FName);
      }
      //Make sure name of the field is made unique.
      FieldToUniqueNames.insert(std::make_pair(FD, FName));
      VarNamesInScope.insert(std::make_pair(FName, FD));

      
      std::set<std::string> VarNamesInScopeSet;
      for (auto &KPair : VarNamesInScope){
        VarNamesInScopeSet.insert(KPair.first);
      }
      //Check for Attrs attached to Field declarations. 
      if (FD->hasAttrs()){
      for (auto Attr : FD->getAttrs()){
        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)){
          auto AttrName = AnnAttr->getAttrName();
          auto Annotation = AnnAttr->getAnnotation();
          //Match has the length
          std::string Match;
          auto AnnKind = getPulseAnnKindFromString(Annotation, Match);
          if (AnnKind != PulseAnnKind::IsArray){
            emitError("Unimplemented! Did not expect an annotation other that ISARRAY!\n");
          }
          HasArrayTypeFields = true;
          NumArrayFields++;
          addArrayTy(Match, FD, VarNamesInScopeSet);
        }
      }
    }
    }

    auto *AbstractType = new GenericDecl();
    AbstractType->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    AbstractType->Ident = "noeq\n";
    AbstractType->Ident += "type ";
    AbstractType->Ident += StructName + " = {\n";
    for (const FieldDecl *FD : RD->fields()) {
      auto *PulseTy = pulseTyFromDecl(FD);
      AbstractType->Ident += FieldToUniqueNames[FD]; 

      //Check if the field is not a lvalue, for instance arrays
      //Discussed with Guido, that arrays in structs are not lvalues.
      //They should not be refs 
      if (FD->getType()->isArrayType()){
        AbstractType->Ident += ": ";
      }
      else {
        AbstractType->Ident += ": ref ";
      }
      
      AbstractType->Ident += PulseTy->print() + ";";
      AbstractType->Ident += "\n";

    }
    AbstractType->Ident += "}\n";
    NewModul->Decls.push_back(AbstractType);



    //If the Record is a Struct/Typedef/Anon struct
    if (RD->isStruct()){

    //2. A purely functional specification type for the struct
    // [@@erasable]
    // noeq
    // type u32_pair_struct_spec = {
    //   first: FStar.UInt32.t;
    //   second: FStar.UInt32.t;
    // }

    auto *Tycon = new TyConDecl();
    Tycon->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    auto *TyconRec = new TyConRecord();
    auto *ErasableAttr = new Name("[@@erasable]", nullptr);
    auto *NoEqTerm = new Name("noeq", nullptr);
    TyconRec->Ident = StructName + "_spec";
    TyconRec->Attrs.push_back(ErasableAttr);
    TyconRec->Attrs.push_back(NoEqTerm);

    std::vector<RecordElement *> Fields;
    for (const FieldDecl *FD : RD->fields()) {
      auto *Element = new RecordElement();
      auto FieldTy = pulseTyFromDecl(FD);
      if (FD->getType()->isRecordType()){
        FieldTy->NamedValue += "_spec";
      }

      if (FD->getType()->isArrayType()){
        auto *FieldArrTy = dyn_cast<FStarArrType>(FieldTy);
        if (!FieldArrTy){
          emitErrorWithLocation("Expected member to be array type!", &Ctx, FD->getLocation());
        }
        auto *UnderlyingType = FieldArrTy->ElementType;
        //should be Seq.seq
        auto *NewSeqTy = new FStarSeqSeqType(UnderlyingType);
        Element->ElementTerm = NewSeqTy;
      }
      else{
        Element->ElementTerm = FieldTy;
      }
      Element->Ident = FieldToUniqueNames[FD];
      Fields.push_back(Element);
    }
    auto NumRecordFields = Fields.size();
    TyconRec->RecordFields = Fields;
    Tycon->TyCons.push_back(TyconRec);
    NewModul->Decls.push_back(Tycon);


    //Issue: https://github.com/FStarLang/c2pulse/issues/58
    // let engine_record_t_relations (s:engine_record_t_spec) : slprop =
    //   pure (
    //     Pulse.Lib.Array.length s.l0_image_header == Pulse.Lib.C.SizeT.as_int s.l0_image_header_size /\
    //     Pulse.Lib.Array.length s.l0_image_header_sig == 64 /\
    //     Pulse.Lib.Array.length s.l0_binary == Pulse.Lib.C.SizeT.as_int s.l0_binary_size /\
    //     Pulse.Lib.Array.length s.l0_binary_hash == 64 /\ //<MACRO EXPANSION OF DICE_DIGEST_LEN> /\
    //     Pulse.Lib.Array.length s.l0_image_auth_pubkey == 32  
    //   )
    size_t Counter = 0; 
    if (HasArrayTypeFields){
      auto *GenericRelations = new GenericDecl(); 
      GenericRelations->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
      GenericRelations->Ident = "let ";
      GenericRelations->Ident += StructName + "_relations (s:" + StructName + "_spec) : prop = \n";
      //Only for fields that are array.
      for (auto *Fld : RD->fields()){

        auto It = DeclTyMap.find(Fld);
        if (It != DeclTyMap.end()){
          auto Len = It->second;
          auto *ArrType = Len->getAsArrayTypeUnsafe();
          if (!ArrType){
            emitErrorWithLocation("Could not get Type of Array", &Ctx, Fld->getLocation());
          }
          if (const auto *constArray = dyn_cast<ConstantArrayType>(ArrType)) {
            llvm::APInt size = constArray->getSize();
            uint64_t ArraySize = size.getLimitedValue();
            GenericRelations->Ident += "Pulse.Lib.Array.length s." + Fld->getNameAsString() + " == " + std::to_string(ArraySize);
            if (Counter < NumArrayFields - 1){
              GenericRelations->Ident += " /\\\n";
            }
          }
          else if (const auto *VarArray = dyn_cast<VariableArrayType>(ArrType)){
            auto ArrSize = VarArray->getSizeExpr();

            //Only implemented for single variables, no complex expressions.
            if (auto *Decl = dyn_cast<DeclRefExpr>(ArrSize)){
              //std::string ArrSizeStr;
              //llvm::raw_string_ostream ArrSizeStream(ArrSizeStr);
              //PrintingPolicy policy(Ctx.getLangOpts());
              //ArrSize->printPretty(ArrSizeStream, nullptr, policy);
              
              auto VarName = Decl->getDecl()->getNameAsString();
              auto It = VarNamesInScope.find(VarName);
              std::string LengthExpr;
              if (It != VarNamesInScope.end()){
                //Check if is a field. 
                auto *Def = It->second;
                if (!Def){
                  emitErrorWithLocation("Could not find definition for variable: " + VarName, &Ctx, RD->getLocation());
                }

                QualType DefTy = getTypeFromDecl(Def);
                auto ModuleName = getPulseModuleNameForCType(DefTy, Ctx);
                auto It2 = FieldToUniqueNames.find(Def); 
                //Length is part of the Struct
                if (It2 != FieldToUniqueNames.end()){
                  LengthExpr = ModuleName + ".v " + "s." + VarName;
                }
                else {
                  LengthExpr = ModuleName + ".v " + VarName;
                }
              }
              else {
                emitErrorWithLocation("Could not find variable in env: " + VarName, &Ctx, RD->getLocation());
              }

              GenericRelations->Ident += "Pulse.Lib.Array.length s." + Fld->getNameAsString() + " == " + LengthExpr;
              if (Counter < NumArrayFields - 1){
                GenericRelations->Ident += " /\\\n";
              }
            }
            else {
              emitErrorWithLocation("Cannot handle complex size expressions for array!", &Ctx, RD->getLocation());
            }

          }
          Counter++;
        }
    }
    GenericRelations->Ident += "\n";
    NewModul->Decls.push_back(GenericRelations);
  }



    // Generate predicate
    // 3. A predicate that relates a u32_pair_struct to its specification
    // let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
    //     exists* (y: u32_pair_struct). (x |-> y) **
    //     (y.first |-> s.first) **
    //     (y.second |-> s.second)

    auto *GenericPredicate = new GenericDecl();
    GenericPredicate->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    GenericPredicate->Ident = "[@@pulse_unfold]\nlet ";
    //Vidush:
    //Discussed with Guido.
    //We should add [@@@mkey] so that the SMT solver
    //can match resources with different matching keys
    GenericPredicate->Ident += StructName + "_pred ([@@@mkey]x:ref " +  StructName + ") (s:" + StructName + "_spec) : slprop =\n";
    GenericPredicate->Ident += "exists* (y: " + StructName + "). (x |-> y) **\n";
    Counter = 0;
    for (auto *Fld : RD->fields()){

        if (Fld->getType()->isRecordType()){
          auto FieldTy = pulseTyFromDecl(Fld);
          GenericPredicate->Ident += FieldTy->print() + "_pred ";
          GenericPredicate->Ident += "y.";
          GenericPredicate->Ident += FieldToUniqueNames[Fld]; 
          GenericPredicate->Ident += " ";
          GenericPredicate->Ident += "s." + FieldToUniqueNames[Fld];
        }
        else{
          GenericPredicate->Ident += "(y.";
          GenericPredicate->Ident += FieldToUniqueNames[Fld]; 
          GenericPredicate->Ident += " |-> ";
          GenericPredicate->Ident += "s." + FieldToUniqueNames[Fld] + ")";
        }
        
        if (Counter < NumRecordFields - 1){
          GenericPredicate->Ident += " **";
        }
        Counter++;
        GenericPredicate->Ident += "\n";
    }

    if (HasArrayTypeFields){
      GenericPredicate->Ident += "** pure (" + StructName + "_relations s)\n";
    }


    NewModul->Decls.push_back(GenericPredicate);

    // Auto Generated functions for Stack Allocated Structs.
    // assume val point_spec_default : point_spec

    auto StructPrefix = StructName + "_var";
    auto StructPrefixSpec = StructPrefix + "_spec";

    auto *StructSpecDefault = new GenericDecl();
    StructSpecDefault->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    StructSpecDefault->Ident = "assume val " + StructName +
                               "_spec_default : " + StructName + "_spec\n";
    NewModul->Decls.push_back(StructSpecDefault);

    // assume val point_default (ps:point_spec) : point

    auto *StructDefault = new GenericDecl();
    StructDefault->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
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
    GhostPack->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
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
    GhostUnpack->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    GhostUnpack->Ident = "ghost\n";
    GhostUnpack->Ident += "fn " + StructName + "_unpack " + "(" + StructPrefix +
                          ":ref " + StructName + ")\n";
    GhostUnpack->Ident +=
        "requires exists* v. " + StructName + "_pred " + StructPrefix + " v \n";
    GhostUnpack->Ident += "ensures exists* u. " + StructPrefix + " |-> u\n";
    GhostUnpack->Ident += "{ admit() }\n";
    NewModul->Decls.push_back(GhostUnpack);

    auto *UtilityFunctionHeap = new GenericDecl();
    UtilityFunctionHeap->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
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
    GhostExplode->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    GhostExplode->Ident = "ghost fn " + StructName + "_explode (x:ref " + StructName + ") " + "(#s:" + StructName + "_spec)\n";
    GhostExplode->Ident += "requires " + StructName + "_pred x s\n";
    GhostExplode->Ident += "ensures exists* (v: " + StructName + "). " + "(x |-> v)";
    GhostExplode->Ident += " ** ";
    Counter = 0;
    for (auto *Fld : RD->fields()){
      GhostExplode->Ident += "(v." + FieldToUniqueNames[Fld];
      if (Fld->getType()->isRecordType()){
        auto *FieldTy = pulseTyFromDecl(Fld);
        GhostExplode->Ident += " `" + FieldTy->print() + "_pred` ";
      }
      else {
        GhostExplode->Ident += " |-> ";
      }
      GhostExplode->Ident += "s." + FieldToUniqueNames[Fld] + ")";
      if (Counter < NumRecordFields - 1){
        GhostExplode->Ident += " ** ";
      }
      GhostExplode->Ident += "\n";
      Counter++;
    }
    GhostExplode->Ident += "\n";
    if (HasArrayTypeFields){
      GhostExplode->Ident += "** pure (" + StructName + "_relations s)\n";
    }
    GhostExplode->Ident += "{unfold " + StructName + "_pred" + "}\n\n";
    NewModul->Decls.push_back(GhostExplode);

    //8. A ghost function that folds the predicate for u32_pair_struct_refs
    // ghost
    // fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 #a1 :erased U32.t)
    // requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
    // ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
    // { fold u32_pair_struct_pred x ({first = a0; second = a1}) }

    std::string FieldPrefix = "a";
    auto *NewGhostFunctionRecover = new GenericDecl();
    NewGhostFunctionRecover->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    NewGhostFunctionRecover->Ident = "ghost\n";
    NewGhostFunctionRecover->Ident += "fn " + StructName + "_recover ";
    NewGhostFunctionRecover->Ident += "(x:ref " + StructName + ") ";
    
    Counter = 0;
    for (auto *Fld : RD->fields()) {
      auto Ty = Fld->getType(); 
      auto *PulseTy = pulseTyFromDecl(Fld);
      NewGhostFunctionRecover->Ident += "(";
      NewGhostFunctionRecover->Ident += "#" + FieldPrefix + std::to_string(Counter) + " : ";
      if (Fld->getType()->isRecordType()){
        NewGhostFunctionRecover->Ident += PulseTy->print() + "_spec";
      }
      else if (Fld->getType()->isArrayType()){
        auto *PulseArrTy = dyn_cast<FStarArrType>(PulseTy);
        if (!PulseArrTy){
          emitErrorWithLocation("Expected field type to be array!", &Ctx, Fld->getLocation());
        }

        auto *ElemTy = PulseArrTy->ElementType;
        auto *SeqTy = new FStarSeqSeqType(ElemTy);
        NewGhostFunctionRecover->Ident += SeqTy->print();
      }
      else {
        NewGhostFunctionRecover->Ident += PulseTy->print();
      }
      NewGhostFunctionRecover->Ident += ")\n";
      Counter++;
    }
    NewGhostFunctionRecover->Ident += "\n";

    NewGhostFunctionRecover->Ident += "requires exists* (y: " + StructName + "). (x |-> y) ** \n";
    Counter = 0;
    for (auto *Fld : RD->fields()){
      NewGhostFunctionRecover->Ident += "(";
      NewGhostFunctionRecover->Ident += "y.";
      NewGhostFunctionRecover->Ident += FieldToUniqueNames[Fld] + " ";
      if (Fld->getType()->isRecordType()){
        auto *FieldTy = pulseTyFromDecl(Fld);
        NewGhostFunctionRecover->Ident += " `" + FieldTy->print() + "_pred` ";
      }
      else{
        NewGhostFunctionRecover->Ident += "|-> ";
      }
      NewGhostFunctionRecover->Ident += FieldPrefix + std::to_string(Counter);
      NewGhostFunctionRecover->Ident += ")";

      if (Counter < NumRecordFields - 1){
        NewGhostFunctionRecover->Ident += " **\n";
      }
      // NewGhostFunction->Ident += "\n";
      Counter++;
    }

  //   (engine_record_t_relations {
  //   l0_image_header_size = a0;
  //   l0_image_header = a1;
  //   l0_image_header_sig = a2; 
  //   l0_binary_size = a3; 
  //   l0_binary = a4; 
  //   l0_binary_hash = a5;
  //   l0_image_auth_pubkey = a6
  // })

    if (HasArrayTypeFields){
  
    Counter = 0;
    NewGhostFunctionRecover->Ident += "** pure (" + StructName + "_relations {\n";
    for (auto *Fld : RD->fields()){
      NewGhostFunctionRecover->Ident += FieldToUniqueNames[Fld] + " = " + FieldPrefix + std::to_string(Counter);
      if (Counter < NumRecordFields - 1){
        NewGhostFunctionRecover->Ident += ";\n";
      }
      Counter++;
    }
    NewGhostFunctionRecover->Ident += "})\n";
  }


    NewGhostFunctionRecover->Ident += "\n";

    NewGhostFunctionRecover->Ident += "ensures exists* w. " + StructName + "_pred x w ** pure (w == {";
    Counter = 0;
    std::string TempStr = "";
    for (auto *Fld : RD->fields()){
      TempStr += FieldToUniqueNames[Fld];
      TempStr += " = ";
      TempStr += FieldPrefix;
      TempStr += std::to_string(Counter);

      if (Counter < NumRecordFields - 1){
        TempStr += ";\n";
      }
      Counter++;
    }
    NewGhostFunctionRecover->Ident += TempStr;
    NewGhostFunctionRecover->Ident += "})\n";

    NewGhostFunctionRecover->Ident += "{fold " + StructName + "_pred x ({";
    NewGhostFunctionRecover->Ident += TempStr + "}) }\n";

    NewModul->Decls.push_back(NewGhostFunctionRecover);
  }
  else if (RD->isUnion()){

    //2. A purely functional specification type for the struct
    //   type ab_spec =
    // | Case_a of UInt32.t
    // | Case_b of bool

    auto *UnionSpec = new GenericDecl(); 
    UnionSpec->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    UnionSpec->Ident += "noeq\n";
    UnionSpec->Ident += "type " + StructName + "_spec = \n";
    for (auto Fld : RD->fields()){
      auto FldName = FieldToUniqueNames[Fld];
      auto FldTy = pulseTyFromDecl(Fld);
      UnionSpec->Ident += " | Case_" + StructName + "_" + FldName + " of " + FldTy->print() + "\n";
    }

    NewModul->Decls.push_back(UnionSpec);


    // let ab_union_relations (s:ab_union_spec) : prop = 
    // match s with
    //   | Case_ab_union_a a -> length a == 24
    //   | Case_ab_union_b b -> length b == 48

    size_t Counter = 0; 
    if (HasArrayTypeFields){
      auto *GenericRelations = new GenericDecl(); 
      GenericRelations->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
      GenericRelations->Ident = "let ";
      GenericRelations->Ident += StructName + "_relations (s:" + StructName + "_spec) : prop = \n";
      GenericRelations->Ident += "match s with\n";
      //Only for fields that are array.
      for (auto *Fld : RD->fields()){

        auto It = DeclTyMap.find(Fld);
        if (It != DeclTyMap.end()){
          auto Len = It->second;
          auto *ArrType = Len->getAsArrayTypeUnsafe();
          if (!ArrType){
            emitErrorWithLocation("Could not get Type of Array", &Ctx, Fld->getLocation());
          }
          if (const auto *constArray = dyn_cast<ConstantArrayType>(ArrType)) {
            llvm::APInt size = constArray->getSize();
            uint64_t ArraySize = size.getLimitedValue();

            GenericRelations->Ident += "| Case_" + StructName + "_" + Fld->getNameAsString() + " " + Fld->getNameAsString(); 
            GenericRelations->Ident += " -> length " + Fld->getNameAsString() + " == " + std::to_string(ArraySize);
            GenericRelations->Ident += "\n";
          }
          else if (const auto *VarArray = dyn_cast<VariableArrayType>(ArrType)){
            auto ArrSize = VarArray->getSizeExpr();

            //Only implemented for single variables, no complex expressions.
            if (auto *Decl = dyn_cast<DeclRefExpr>(ArrSize)){
              //std::string ArrSizeStr;
              //llvm::raw_string_ostream ArrSizeStream(ArrSizeStr);
              //PrintingPolicy policy(Ctx.getLangOpts());
              //ArrSize->printPretty(ArrSizeStream, nullptr, policy);
              
              auto VarName = Decl->getDecl()->getNameAsString();
              auto It = VarNamesInScope.find(VarName);
              std::string LengthExpr;
              if (It != VarNamesInScope.end()){
                //Check if is a field. 
                auto *Def = It->second;
                if (!Def){
                  emitErrorWithLocation("Could not find definition for variable: " + VarName, &Ctx, RD->getLocation());
                }

                QualType DefTy = getTypeFromDecl(Def);
                auto ModuleName = getPulseModuleNameForCType(DefTy, Ctx);
                auto It2 = FieldToUniqueNames.find(Def); 
                //Length is part of the Struct
                if (It2 != FieldToUniqueNames.end()){
                  LengthExpr = ModuleName + ".v " + "s." + VarName;
                }
                else {
                  LengthExpr = ModuleName + ".v " + VarName;
                }
              }
              else {
                emitErrorWithLocation("Could not find variable in env: " + VarName, &Ctx, RD->getLocation());
              }

              GenericRelations->Ident += "| Case_" + StructName + "_" + Fld->getNameAsString() + " " + Fld->getNameAsString(); 
              GenericRelations->Ident += " -> length " + Fld->getNameAsString() + " == " + LengthExpr;
              GenericRelations->Ident += "\n";

            }
            else {
              emitErrorWithLocation("Cannot handle complex size expressions for array!", &Ctx, RD->getLocation());
            }

          }
          Counter++;
        }
    }
    GenericRelations->Ident += "| _ -> True \n";
    NewModul->Decls.push_back(GenericRelations);
  }


    // (* Just like a struct, this should the predicate used by the translation
    // whenever there are fields of type `union ab`. *)
    // let ab_pred (u : ref ab) (s : ab_spec) : slprop =
    //   exists* uv. (u |-> uv) **
    //     begin match s with
    //       | Case_a v -> uv.a |-> v
    //       | Case_b v -> uv.b |-> v
    //     end

    auto *UnionPred = new GenericDecl();
    UnionPred->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    UnionPred->Ident = "let " + StructName + "_pred" + "(u : ref " + StructName + ") " + "(s : " + StructName + "_spec) : slprop =\n";
    UnionPred->Ident += "exists* uv. (u |-> uv) **\n";
    UnionPred->Ident += "begin match s with\n";
    for (auto *Fld : RD->fields()){
      auto FldName = FieldToUniqueNames[Fld];
      UnionPred->Ident += " | Case_" + StructName + "_" + FldName + " v -> " + "uv." + FldName + " |-> v\n";
    } 
    UnionPred->Ident += "end\n";

    if (HasArrayTypeFields){
      UnionPred->Ident += "** pure (" + StructName + "_relations s)\n";
    }

    NewModul->Decls.push_back(UnionPred);

    // ghost
    // fn ab_explode (x : ref ab) (#s : ab_spec)
    // requires ab_pred x s
    // ensures exists* (v : ab). (x |-> v) **
    //   begin match s with
    //     | Case_a w -> v.a |-> w
    //     | Case_b w -> v.b |-> w
    //   end
    // {
    //   unfold ab_pred;
    // }

    auto *UnionExplode = new GenericDecl(); 
    UnionExplode->Ident = "ghost fn " + StructName + "_explode " + "(x : ref " + StructName + ") " + "(#s : " + StructName + "_spec)\n";
    UnionExplode->Ident += "requires " + StructName + "_pred x s\n";
    UnionExplode->Ident += "ensures exists* (v : " + StructName + "). (x |-> v) **\n";
    UnionExplode->Ident += "begin match s with\n";
    for (auto Fld : RD->fields()){
      auto FldName = FieldToUniqueNames[Fld];
      UnionExplode->Ident += " | Case_" + StructName + "_" + FldName + " w -> v." + FldName + " |-> w\n";
    }
    UnionExplode->Ident += "end\n";
    if (HasArrayTypeFields){
      UnionExplode->Ident += "** pure (" + StructName + "_relations s)\n";
    }
    UnionExplode->Ident += "{\n";
    UnionExplode->Ident += "unfold " + StructName + "_pred;\n";
    UnionExplode->Ident += "}\n";
    NewModul->Decls.push_back(UnionExplode);

    // ghost
    // fn ab_recover (x : ref ab) (#s : ab_spec)
    // requires exists* (v : ab). (x |-> v) **
    //     begin match s with
    //       | Case_a w -> v.a |-> w
    //       | Case_b w -> v.b |-> w
    //     end
    // ensures  ab_pred x s
    // {
    //   fold (ab_pred x s);
    // }
    auto *GhostRecover = new GenericDecl(); 
    GhostRecover->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
    GhostRecover->Ident = "ghost\n";
    GhostRecover->Ident += "fn " + StructName + "_recover " + "(x : ref " + StructName + ") " + "(#s : " + StructName + "_spec)\n";
    GhostRecover->Ident += "requires exists* (v : " + StructName + "). " + "(x |-> v) **\n";
    GhostRecover->Ident += "begin match s with\n";
    for (auto Fld : RD->fields()){
      auto FldName = FieldToUniqueNames[Fld];
      GhostRecover->Ident += " | Case_" + StructName + "_" + FldName + " w -> v." + FldName + "|-> w\n";
    }
    GhostRecover->Ident += "end\n";
    if (HasArrayTypeFields){
      GhostRecover->Ident += "** pure (" + StructName + "_relations s)\n";
    }
    GhostRecover->Ident += "ensures " + StructName + "_pred x s\n";
    GhostRecover->Ident += "{\n";
    GhostRecover->Ident += "fold (" + StructName + "_pred x s);\n";
    GhostRecover->Ident += "}\n";

    NewModul->Decls.push_back(GhostRecover);

    for (auto Fld : RD->fields()) {
      auto FldName = FieldToUniqueNames[Fld];
      auto *GhostChange = new GenericDecl(); 
      GhostChange->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
      GhostChange->Ident = "ghost\n";
      GhostChange->Ident += "fn " + StructName + "_change_" + FldName + "(x : ref " + StructName + ")\n";
      GhostChange->Ident += "requires exists* s. " + StructName + "_pred x s\n";
      GhostChange->Ident += "ensures exists* s. " + StructName + "_pred x (Case_" + StructName + "_" +FldName+ " s)\n";
      GhostChange->Ident += "{ admit() }\n";
      NewModul->Decls.push_back(GhostChange);    
    }

    for (auto Fld : RD->fields()) {
      auto FldName = FieldToUniqueNames[Fld];
      auto *GhostIs = new GenericDecl(); 
      GhostIs->CInfo = getSourceInfoFromDecl(RD, Ctx, "");
      GhostIs->Ident = "ghost\n";
      GhostIs->Ident += "fn " + StructName + "_is_" + FldName + "(x : ref " + StructName + ") (#s:_{Case_"+StructName+"_"+FldName+"? s})\n";
      GhostIs->Ident += "requires " + StructName + "_pred x s\n";
      GhostIs->Ident += "ensures  " + StructName + "_pred x (Case_" + StructName + "_" +FldName+ "(Case_"+StructName+"_"+FldName+"?._0 s))\n";
      GhostIs->Ident += "{ admit() }\n";
      NewModul->Decls.push_back(GhostIs);    
    }
  }
 

    return true;
}

// bool PulseVisitor::VisitTypedefDecl(TypedefDecl *TypeDefDec) {

//   /// TODO: Angelica: This might fail for analyzing programs that use
//   /// struct definitions from system libraries or C standard libraries.
//   auto SourceLoc = TypeDefDec->getLocation();
//   if (SM.isInSystemHeader(SourceLoc))
//     return true;

//   auto *Def = TypeDefDec->getUnderlyingDecl();

//   /// Check if we have a record declaration associated with the typedefDecl.
//   ///  For every record type we create a new Module.
//   if (const auto *RT = TypeDefDec->getUnderlyingType()->getAs<RecordType>())
//   {
//     const RecordDecl *RD = RT->getDecl();

//     //Map record declaration to typedef declaration.
//     MapRecordDeclsToTypedefDecls.insert(std::make_pair(RD, TypeDefDec));

//     auto StructName = Def->getNameAsString();
//     auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
//     if (!FileEnt) {
//       llvm::errs() << "Error: Main file entry not found in source
//       manager.\n"; exit(1);
//     }

//     /// Parse the file name.
//     /// The filename is same as module.
//     auto FilePath = FileEnt->tryGetRealPathName();
//     std::filesystem::path FilePathSys = FilePath.str();
//     auto Extension = FilePathSys.extension().string();
//     auto TempFilePathWithoutExtension = FilePathSys.replace_extension("");
//     auto FileName = TempFilePathWithoutExtension.filename();
//     auto FileNameStr = FileName.string();
//     if (!FileNameStr.empty()) {
//       FileNameStr[0] = std::toupper(FileNameStr[0]);
//     }

//     // change dots to _ since . is reserved for nested modules.
//     std::replace(FileNameStr.begin(), FileNameStr.end(), '.', '_');

//     auto It = Modules.find(FileNameStr);
//     PulseModul *NewModul = nullptr;
//     if (It != Modules.end()) {
//       NewModul = It->second;
//     }
//     else {
//       NewModul = new PulseModul();
//       NewModul->includePulsePrelude = true;
//       NewModul->ModuleName = FileNameStr;
//       Modules.insert(std::make_pair(NewModul->ModuleName, NewModul));
//     }

//     auto *AbstractType = new GenericDecl();
//     AbstractType->Ident = "noeq\n";
//     AbstractType->Ident += "type ";
//     AbstractType->Ident += StructName + " = {\n";
//     for (const FieldDecl *FD : RD->fields()) {
//       auto *PulseTy = getPulseTyFromCTy(FD->getType());
//       AbstractType->Ident += FD->getNameAsString() + ": ref ";
//       AbstractType->Ident += PulseTy->print() + ";";
//       AbstractType->Ident += "\n";

//     }
//     AbstractType->Ident += "}\n";
//     NewModul->Decls.push_back(AbstractType);

//     //2. A purely functional specification type for the struct
//     // [@@erasable]
//     // noeq
//     // type u32_pair_struct_spec = {
//     //   first: FStar.UInt32.t;
//     //   second: FStar.UInt32.t;
//     // }

//     auto *Tycon = new TyConDecl();
//     auto *TyconRec = new TyConRecord();
//     auto *ErasableAttr = new Name("[@@erasable]");
//     auto *NoEqTerm = new Name("noeq");
//     TyconRec->Ident = Def->getNameAsString() + "_spec";
//     TyconRec->Attrs.push_back(ErasableAttr);
//     TyconRec->Attrs.push_back(NoEqTerm);

//     std::vector<RecordElement *> Fields;
//     for (const FieldDecl *FD : RD->fields()) {
//       auto *Element = new RecordElement();
//       Element->ElementTerm = getPulseTyFromCTy(FD->getType());
//       Element->Ident = FD->getNameAsString();
//       Fields.push_back(Element);
//     }
//     auto NumRecordFields = Fields.size();
//     TyconRec->RecordFields = Fields;
//     Tycon->TyCons.push_back(TyconRec);
//     NewModul->Decls.push_back(Tycon);

//     // Generate predicate
//     // 3. A predicate that relates a u32_pair_struct to its specification
//     // let u32_pair_struct_pred (x:ref u32_pair_struct)
//     (s:u32_pair_struct_spec) : slprop =
//     //     exists* (y: u32_pair_struct). (x |-> y) **
//     //     (y.first |-> s.first) **
//     //     (y.second |-> s.second)

//     auto *GenericPredicate = new GenericDecl();
//     GenericPredicate->Ident = "let ";
//     GenericPredicate->Ident += StructName + "_pred (x:ref " +  StructName +
//     ") (s:" + StructName + "_spec) : slprop =\n"; GenericPredicate->Ident +=
//     "exists* (y: " + StructName + "). (x |-> y) **\n"; size_t Counter = 0;
//     for (auto *Fld : RD->fields()){
//         GenericPredicate->Ident += "(y.";
//         GenericPredicate->Ident += Fld->getNameAsString();
//         GenericPredicate->Ident += " |-> ";
//         GenericPredicate->Ident += "s." + Fld->getNameAsString() + ")";
//         if (Counter < NumRecordFields - 1){
//           GenericPredicate->Ident += " **";
//         }
//         Counter++;
//         GenericPredicate->Ident += "\n";
//     }
//     NewModul->Decls.push_back(GenericPredicate);

//     // Auto Generated functions for Stack Allocated Structs.
//     // assume val point_spec_default : point_spec

//     auto StructPrefix = StructName + "_var";
//     auto StructPrefixSpec = StructPrefix + "_spec";

//     auto *StructSpecDefault = new GenericDecl();
//     StructSpecDefault->Ident = "assume val " + StructName +
//                                "_spec_default : " + StructName + "_spec\n";
//     NewModul->Decls.push_back(StructSpecDefault);

//     // assume val point_default (ps:point_spec) : point

//     auto *StructDefault = new GenericDecl();
//     StructDefault->Ident = "assume val " + StructName + "_default ";
//     StructDefault->Ident += "(" + StructPrefixSpec + ":" + StructName +
//                             "_spec)" + " : " + StructName + "\n";
//     NewModul->Decls.push_back(StructDefault);

//     // ghost
//     // fn point_pack (p:ref point) (#ps:point_spec)
//     // requires p |-> point_default ps
//     // ensures exists* v. point_pred p v ** pure (v == ps)
//     // { admit() }

//     auto *GhostPack = new GenericDecl();
//     GhostPack->Ident = "ghost\n";
//     GhostPack->Ident += "fn " + StructName + "_pack (" + StructPrefix +
//                         ":ref " + StructName + ") " + "(#" + StructPrefixSpec
//                         +
//                         ":" + StructName + "_spec)\n";
//     GhostPack->Ident += "requires " + StructPrefix + "|-> " + StructName +
//                         "_default " + StructPrefixSpec + "\n";
//     GhostPack->Ident += "ensures exists* v. " + StructName + "_pred " +
//                         StructPrefix + " v ** pure (v == " + StructPrefixSpec
//                         +
//                         ")\n";
//     GhostPack->Ident += "{ admit() }\n";
//     NewModul->Decls.push_back(GhostPack);

//     // ghost
//     // fn point_unpack (p:ref point)
//     // requires exists* v. point_pred p v
//     // ensures exists* u. p |-> u
//     // { admit() }

//     auto *GhostUnpack = new GenericDecl();
//     GhostUnpack->Ident = "ghost\n";
//     GhostUnpack->Ident += "fn " + StructName + "_unpack " + "(" +
//     StructPrefix +
//                           ":ref " + StructName + ")\n";
//     GhostUnpack->Ident +=
//         "requires exists* v. " + StructName + "_pred " + StructPrefix + " v
//         \n";
//     GhostUnpack->Ident += "ensures exists* u. " + StructPrefix + " |-> u\n";
//     GhostUnpack->Ident += "{ admit() }\n";
//     NewModul->Decls.push_back(GhostUnpack);

//     auto *UtilityFunctionHeap = new GenericDecl();
//     UtilityFunctionHeap->Ident += "fn " + StructName + "_alloc ()\n";
//     UtilityFunctionHeap->Ident += "returns x:ref " + StructName + "\n";
//     UtilityFunctionHeap->Ident += "ensures freeable x\n";
//     UtilityFunctionHeap->Ident += "ensures exists* v. " + StructName + "_pred
//     x v\n"; UtilityFunctionHeap->Ident += "{ admit () }\n\n";

//     // fn u32_pair_struct_free (x:ref u32_pair_struct)
//     // requires freeable x
//     // requires exists* v. u32_pair_struct_pred x v
//     // { admit () }

//     UtilityFunctionHeap->Ident += "fn " + StructName + "_free " + "(x:ref " +
//     StructName + ")\n"; UtilityFunctionHeap->Ident += "requires freeable
//     x\n"; UtilityFunctionHeap->Ident += "requires exists* v. " + StructName +
//     "_pred x v\n"; UtilityFunctionHeap->Ident += "{ admit() }\n\n";

//     NewModul->Decls.push_back(UtilityFunctionHeap);

//     //5. A ghost function that unfolds the predicate for u32_pair_struct_refs
//     // ghost fn u32_pair_struct_explode (x:ref u32_pair_struct)
//     (#s:u32_pair_struct_spec)
//     // requires u32_pair_struct_pred x s
//     // ensures exists* (v: u32_pair_struct). (x |-> v)
//     //   ** (v.first |-> s.first) ** (v.second |-> s.second)
//     // { unfold u32_pair_struct_pred }

//     auto *GhostExplode = new GenericDecl();
//     GhostExplode->Ident = "ghost fn " + StructName + "_explode (x:ref " +
//     StructName + ") " + "(#s:" + StructName + "_spec)\n"; GhostExplode->Ident
//     += "requires " + StructName + "_pred x s\n"; GhostExplode->Ident +=
//     "ensures exists* (v: " + StructName + "). " + "(x |-> v)";
//     GhostExplode->Ident += " ** ";
//     Counter = 0;
//     for (auto *Fld : RD->fields()){
//       GhostExplode->Ident += "(v." + Fld->getNameAsString() + " |-> " + "s."
//       + Fld->getNameAsString() + ")"; if (Counter < NumRecordFields - 1){
//         GhostExplode->Ident += " ** ";
//       }
//       Counter++;
//     }
//     GhostExplode->Ident += "\n";
//     GhostExplode->Ident += "{unfold " + StructName + "_pred" + "}\n\n";
//     NewModul->Decls.push_back(GhostExplode);

//     //8. A ghost function that folds the predicate for u32_pair_struct_refs
//     // ghost
//     // fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 #a1 :erased
//     U32.t)
//     // requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0)
//     ** (y.second |-> a1)
//     // ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0;
//     second = a1})
//     // { fold u32_pair_struct_pred x ({first = a0; second = a1}) }

//     std::string FieldPrefix = "a";
//     auto *NewGhostFunction = new GenericDecl();
//     NewGhostFunction->Ident = "ghost\n";
//     NewGhostFunction->Ident += "fn " + StructName + "_recover ";
//     NewGhostFunction->Ident += "(x:ref " + StructName + ") ";

//     Counter = 0;
//     for (auto *Fld : RD->fields()) {
//       auto Ty = Fld->getType();
//       auto *PulseTy = getPulseTyFromCTy(Ty);
//       NewGhostFunction->Ident += "(";
//       NewGhostFunction->Ident += "#" + FieldPrefix + std::to_string(Counter)
//       + " : "; NewGhostFunction->Ident += PulseTy->print();
//       NewGhostFunction->Ident += ") ";
//       Counter++;
//     }
//     NewGhostFunction->Ident += "\n";

//     NewGhostFunction->Ident += "requires exists* (y: " + StructName + "). (x
//     |-> y) ** "; Counter = 0; for (auto *Fld : RD->fields()){
//       NewGhostFunction->Ident += "(";
//       NewGhostFunction->Ident += "y.";
//       NewGhostFunction->Ident += Fld->getNameAsString() + " ";
//       NewGhostFunction->Ident += "|-> ";
//       NewGhostFunction->Ident += FieldPrefix + std::to_string(Counter);
//       NewGhostFunction->Ident += ")";

//       if (Counter < NumRecordFields - 1){
//         NewGhostFunction->Ident += " ** ";
//       }
//       Counter++;
//     }
//     NewGhostFunction->Ident += "\n";

//     NewGhostFunction->Ident += "ensures exists* w. " + StructName + "_pred x
//     w ** pure (w == {"; Counter = 0; std::string TempStr = ""; for (auto *Fld
//     : RD->fields()){
//       TempStr += Fld->getNameAsString();
//       TempStr += " = ";
//       TempStr += FieldPrefix;
//       TempStr += std::to_string(Counter);

//       if (Counter < NumRecordFields - 1){
//         TempStr += "; ";
//       }
//       Counter++;
//     }
//     NewGhostFunction->Ident += TempStr;
//     NewGhostFunction->Ident += "})\n";

//     NewGhostFunction->Ident += "{fold " + StructName + "_pred x ({";
//     NewGhostFunction->Ident += TempStr + "}) }\n";

//     NewModul->Decls.push_back(NewGhostFunction);
//   }

//   return true;
// }

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

bool PulseVisitor::VisitVarDecl(VarDecl *VD) {

  auto SourceLoc = VD->getLocation();
  if (SM.isInSystemHeader(SourceLoc))
    return true;

  DEBUG_WITH_TYPE(DEBUG_TYPE, {
  llvm::outs() << "\n===============================================" << "\n";
  llvm::outs() << "Print Var: ";
  llvm::outs() << VD->getNameAsString() << "\n";
  llvm::outs() << "===============================================" << "\n";
  });

  if (VD->hasGlobalStorage()) {
    emitErrorWithLocation("Globals are not implemented yet in Pulse!", &Ctx,
                          VD->getLocation());
  }

  ////Vidush:
  /// Assuming that all other local variables are part of a function body.
  /// In which case they should be handled when handling the function body.
  /// They don't need handling here

  return true;
}

static inline clang::QualType makeVLA(clang::ASTContext &Ctx,
                                      clang::QualType EltTy,
                                      clang::Expr *NumElts) {
#if defined(CLANG_VERSION_MAJOR) && (CLANG_VERSION_MAJOR >= 21)
  return Ctx.getVariableArrayType(EltTy, NumElts,
                                  clang::ArraySizeModifier::Normal, 0);
#else
  return Ctx.getVariableArrayType(EltTy, NumElts,
                                  clang::ArraySizeModifier::Normal, 0,
                                  clang::SourceRange());
#endif
}

void PulseVisitor::addArrayTy(std::string Match, const Decl *ArrDecl, std::set<std::string> VEnv) {

  QualType ArrTy;
  QualType ArrElementType;
  SourceLocation Loc;
  if (auto *ParamDecl = dyn_cast<ParmVarDecl>(ArrDecl)) {
    ArrTy = ParamDecl->getType();
    ArrElementType = ArrTy->getPointeeType();
    Loc = ParamDecl->getLocation();
  } else if (auto *Field = dyn_cast<FieldDecl>(ArrDecl)) {
    ArrTy = Field->getType();
    ArrElementType = ArrTy->getPointeeType();
    Loc = Field->getLocation();
  }
  else if (auto *VD = dyn_cast<VarDecl>(ArrDecl)){
    ArrTy = VD->getType();
    ArrElementType = ArrTy->getPointeeType();
    Loc = VD->getLocation();
  }
  else if (auto *FD = dyn_cast<FunctionDecl>(ArrDecl)){
    ArrTy = FD->getReturnType();
    ArrElementType = ArrTy->getPointeeType();
    Loc = FD->getLocation();
  }
  else {
    emitErrorWithLocation("Not implemented for declaration type!", &Ctx, ArrDecl->getLocation());
  }

  if (!ArrTy->isPointerType() && !ArrTy->isArrayType()) {
    emitErrorWithLocation("Expected type to be a ref or an array!", &Ctx,
                          ArrDecl->getLocation());
  }

  if (!std::regex_match(Match, std::regex("[-+]?[0-9]+"))) {
    // Step 2: Create a VarDecl for the size variable 'n'
    // We should check here is the length is a constant or of variable
    // array type.
    
    //If the variable is not scope, we should just error out.
    if (Match != "" && VEnv.count(Match) == 0){
      emitErrorWithLocation("The variable: " + Match + " is not in scope! Please use a valid variable name!", &Ctx, Loc);
    }

    IdentifierInfo &Id = Ctx.Idents.get(Match);
    VarDecl *SizeVar =
        VarDecl::Create(Ctx, Ctx.getTranslationUnitDecl(), SourceLocation(),
                        SourceLocation(), &Id, Ctx.IntTy, nullptr, SC_Auto);

    // Step 3: Create a DeclRefExpr to refer to 'n'
    DeclRefExpr *SizeExpr = DeclRefExpr::Create(
        Ctx, NestedNameSpecifierLoc(), SourceLocation(), SizeVar, false,
        SourceLocation(), Ctx.IntTy,
        clang::Expr::getValueKindForType(ArrElementType));

    // Step 4: Create the VLA type
    // QualType VLAType = Ctx.getVariableArrayType(ArrElementType, SizeExpr,
    //                                             ArraySizeModifier::Normal, 0,
    //                                             SourceRange());
    QualType VLAType = makeVLA(Ctx, ArrElementType, SizeExpr);
    DeclTyMap.insert(std::make_pair(ArrDecl, VLAType));

  } else {
    clang::QualType ConstArrayTy = Ctx.getConstantArrayType(
        ArrElementType, llvm::APInt(32, std::stoi(Match)), nullptr,
        ArraySizeModifier::Normal, 0);
    DeclTyMap.insert(std::make_pair(ArrDecl, ConstArrayTy));
  }
}

FStarType *PulseVisitor::pulseTyFromDecl(const Decl* D){

   QualType DeclTy; 
   if (auto *P = dyn_cast<ParmVarDecl>(D)){
    DeclTy = P->getType();
   }
   else if (auto *FD = dyn_cast<FieldDecl>(D)){
    //llvm::outs() << "Print the name of the Field Decl!\n" << FD->getName() << "\n\n";
    //FD->dump();
    //llvm::outs() << "\n\n";
    DeclTy = FD->getType();
   }
   else if (auto *VD = dyn_cast<VarDecl>(D)){
    DeclTy = VD->getType();
   } else if (auto *FD = dyn_cast<FunctionDecl>(D)) {
     // For a function declaration, the return type is the declaration type
     DeclTy = FD->getReturnType();
   } else {
     emitErrorWithLocation("Did not expect declaration type!", &Ctx,
                           D->getLocation());
   }

    FStarType *PulseTy;
    auto It = DeclTyMap.find(D);
    if (It != DeclTyMap.end()) {
      // Get the qualification
      auto Ty = It->second;
      if (Ty->isArrayType() || Ty->isConstantArrayType() ||
          Ty->isVariableArrayType()) {
        auto *FArrTy = new FStarArrType();
        FArrTy->CInfo = getSourceInfoFromDecl(D, Ctx, "");
        FArrTy->ElementType =
            getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        PulseTy = FArrTy;
      } else {
        PulseTy = getPulseTyFromCTy(DeclTy);
      }
    }
    else {
      PulseTy = getPulseTyFromCTy(DeclTy);
    }
    return PulseTy;
}


static bool containsTerm(std::vector<Term*> &Vec, TermTag TagCheck){
  
  for (auto *T : Vec){
    if (T->Tag == TagCheck){
      return true;
    }
  }

  return false;
}

static bool isPulseAnnotAttr(AnnotateAttr const *AnnAttr) {
  auto name = AnnAttr->getAttrName()->getName();
  return name == "annotate" || name == "annotate_type";
}

GenericDecl2 *PulseVisitor::getAnnotationCode(AnnotateAttr const *AnnAttr, SourceLocation Loc) {
  if (AnnAttr->args_size() != 1){
    emitErrorWithLocation("pulse attr does not have exactly one arg", &Ctx, Loc);
  }
  Expr *ctrArg = AnnAttr->args_begin()[0];
  if (auto ctrVal = ctrArg->getIntegerConstantExpr(Ctx)) {
    unsigned ctr = ctrVal->getZExtValue();
    auto & toks = macroTokens.at(ctr);
    auto *Inc = new GenericDecl2();
    for (auto & tok : toks) {
      Inc->Tokens.push_back({ tok.Pre, getSourceInfoForToken(tok.Range, tok.Text.length(), Ctx, "", true), tok.Text });
    }
    return Inc;
  } else {
    emitErrorWithLocation("pulse attr arg is not an int lit", &Ctx, Loc);
  }
}

void PulseVisitor::handleFunctionAttributes(FunctionDecl *FD,
                                            _PulseFnDefn *FDefn,
                                            std::string FuncName,
                                            PulseModul *Module,
                                            bool *Terminate,
                                            bool HasAssociatedDefinition,
                                            std::vector<Binder *> &PulseArgs, 
                                            std::vector<Binder *> &ErasedArgs, 
                                            std::set<std::string> VEnv){

  llvm::outs() << "HandleFunctionAttributes " << FuncName << "\n";
  llvm::outs() <<  "Has Attrs: " << FD->hasAttrs() << "\n";
  llvm::outs() << "HasAssociatedDef " << HasAssociatedDefinition << "\n";                                   

  if (HasAssociatedDefinition){
    if (FD->hasAttrs()){
      auto AnnotationsAttachedToFD = FD->getAttrs();
      for (auto *Attr : AnnotationsAttachedToFD) {
        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
          if (!AnnAttr->isInherited() && isPulseAnnotAttr(AnnAttr)){
            emitErrorWithLocation("This function has an associated declaration, please annotate pulse specs on the declaration instead of the definition!", &Ctx, FD->getLocation());
          }
        }
      }
      //Vidush: If for all the attributes, none of them are pulse attributes, 
      //don't error out, copy attribues from function declaration
      //goto copy_attrs_from_definition;
      goto add_annotations;
    }
    
    //copy_attrs_from_definition:
    // if (auto *FuncDecl = FD->getCanonicalDecl()){
    //   auto It = DeclEnv.find(FuncDecl);
    //   //Error out since we should have generated the corresponding definition
    //   if (It == DeclEnv.end()){
    //      emitErrorWithLocation("Expected pulse declaration to be generated!", &Ctx, FuncDecl->getLocation());
    //   }

    //   auto *PulseDecl = It->second;
    //   //Copy over attributes from declaration to definition
    //   if (auto *PFnDecl = dyn_cast<PulseFnDecl>(PulseDecl)){
    //     for (auto *T : PFnDecl->Defn->Attr){
    //       FDefn->Attr.push_back(T);
    //     }
    //   }
    //   else {
    //     emitErrorWithLocation("Could not dyn cast to a function declaration!", &Ctx, FuncDecl->getLocation());
    //   }
    // }
    // else {
    //   emitErrorWithLocation("Expected a function declaration!", &Ctx, FD->getLocation());
    // }

  }
  else if (FD->hasAttrs()) {
    add_annotations:
    auto AnnotationsAttachedToFD = FD->getAttrs();
    for (auto *Attr : AnnotationsAttachedToFD) {

      if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (isPulseAnnotAttr(AnnAttr)) {
          
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
          llvm::outs() << "Print Ann in handleAnnotations " << Ref << "\n";
          if (!Ref.empty() && Ref.find(':') != std::string::npos) {
            llvm::outs() << "Print Ann in handleAnnotations in if check " << Ref << "\n";
            std::string Match = "";
            PulseAnnKind AnnKind = getPulseAnnKindFromString(
                AnnAttr->getAnnotation().data(), Match);
            switch (AnnKind) {
            case PulseAnnKind::ExpectFailure: {
              StringRef MatchRef(Match);
              llvm::SmallVector<StringRef, 4> CommaSeperatedItems;
              MatchRef.split(CommaSeperatedItems, ",");
              auto *NewAttr = new Name();
              NewAttr->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");

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
            case PulseAnnKind::Returns: {
              auto *ReturnSpec = new Returns();
              ReturnSpec->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
              ReturnSpec->Ann = Match;
              FDefn->Annotation.push_back(ReturnSpec);
              break;
            }
            case PulseAnnKind::ErasedArg: {
              auto *NewErasedArgBinder = new Binder(Match);
              NewErasedArgBinder->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
              NewErasedArgBinder->useFallBack = true;
              ErasedArgs.push_back(NewErasedArgBinder);
              break;
            }
            case PulseAnnKind::IsArray:{
              //We parse this as: The function returns an array type.
              //So we could say that the function is of type array type.
              addArrayTy(Match, FD, VEnv);
              break;
            }
            case PulseAnnKind::Invariants:
              llvm::report_fatal_error(
                  "Unhandled PulseAnnKind::Invariants in switch statement");
              break;
            case PulseAnnKind::LemmaStatement:
              llvm::report_fatal_error(
                  "Unhandled PulseAnnKind::LemmaStatement in switch statement");
              break;
            }
          } else if (Ref == "includes") {
            Module->Decls.push_back(getAnnotationCode(AnnAttr, FD->getLocation()));
            //If function name is the anchor dummy function. 
            //Skip generating code for it entirely.
            if ((FuncName.find("__pulse_include_anchor") != std::string::npos)){
              *Terminate = true;
            }
          } else if (Ref == "requires") {
            auto *NewRequires = new Requires();
            NewRequires->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
            NewRequires->Ann2 = getAnnotationCode(AnnAttr, FD->getLocation());
            FDefn->Annotation.push_back(NewRequires);
          } else if (Ref == "preserves") {
            auto *NewRequires = new Preserves();
            NewRequires->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
            NewRequires->Ann2 = getAnnotationCode(AnnAttr, FD->getLocation());
            FDefn->Annotation.push_back(NewRequires);
          } else if (Ref == "ensures") {
            auto *NewRequires = new Ensures();
            NewRequires->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
            NewRequires->Ann2 = getAnnotationCode(AnnAttr, FD->getLocation());
            FDefn->Annotation.push_back(NewRequires);
          }
        }
      }
    }
    llvm::outs() << "Print Size handle Annotation: " << FDefn->Attr.size() << "\n";
  }
}

//Vidush
//TODO: We could just coalese VEnv and VEnvSet maps as one.
//Maybe we can do this by makeing VEnv's key as a std::string rather than a term
PulseSequence *PulseVisitor::handleFunctionParameters(FunctionDecl *FD,
                                            _PulseFnDefn *FDefn,
                                            std::vector<Binder*> &PulseArgs,
                                            std::map<Term *, FStarType *> &VEnv, 
                                            std::set<std::string> VEnvSet){

    ///Vidush: Also construct let mut for all function parameters
  PulseSequence *ParamLetMutSequence = nullptr;
  PulseSequence *ParamLetMutSequenceHead = nullptr;
  
  auto *FuncDecl = FD->getCanonicalDecl();

  for (unsigned i = 0; i < FD->getNumParams(); i++) {
    auto *Param = FD->getParamDecl(i);
    auto ParamName = Param->getNameAsString();
    
    //Vidush: 
    //If this is function definition which has an 
    //existing definition, we should make sure 
    //that parameter names are the same.
    if (FuncDecl && FD->isThisDeclarationADefinition()){
      if (ParamName != FuncDecl->getParamDecl(i)->getNameAsString()){
        FuncDecl->dump(); 
        FD->dump();
        emitErrorWithLocation("Function parameter names differ from declaration, please use same parameter names!\n", &Ctx, FD->getLocation());
      }
    }

    /// See if this array parameter has any annotations arrached to it.
    auto Attrs = Param->attrs();
    for (auto *Attr : Attrs) {
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (isPulseAnnotAttr(AnnotAttr)) {
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
                  "Expected parameter to be a ref or an array!", &Ctx,
                  Param->getLocation());
            }

            QualType ElementType = Param->getType()->getPointeeType();

            if (!std::regex_match(Match, std::regex("[-+]?[0-9]+"))) {

              // Step 2: Create a VarDecl for the size variable 'n'
              // We should check here is the length is a constant or of variable
              // array type.


              //If the variable name is not in scope error out. 
              if (Match != "" && VEnvSet.count(Match) == 0){
                emitErrorWithLocation("got Array length: " + Match + " is not in scope! Please use a variable in scope!", &Ctx, Attr->getLocation());
              }

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
              // QualType VLAType = Ctx.getVariableArrayType(
              //     ElementType, SizeExpr, ArraySizeModifier::Normal, 0,
              //     SourceRange());
              QualType VLAType = makeVLA(Ctx, ElementType, SizeExpr);
              // llvm::outs() << "Print the element type here!!!\n";
              // llvm::outs() <<
              // QualType(VLAType->getPointeeOrArrayElementType(), 0);
              // llvm::outs() << "End of element type!" << "\n"; exit(1);
              DeclTyMap.insert(std::make_pair(Param, VLAType));
              
              //Removing these since SizeT.v is assumed but length can be any type in C
              if (!Match.empty()) {
                auto *NewRequires = new Requires();
                NewRequires->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
                NewRequires->Ann = "pure (length " + Param->getNameAsString() +
                                   " == SizeT.v " + Match + ")";
                auto &Arr = FDefn->Annotation;
                Arr.insert(Arr.begin(), NewRequires);
              }

            } else {
              clang::QualType ConstArrayTy = Ctx.getConstantArrayType(
                  ElementType, llvm::APInt(32, std::stoi(Match)), nullptr,
                  ArraySizeModifier::Normal, 0);
              DeclTyMap.insert(std::make_pair(Param, ConstArrayTy));
              //Removing these since SizeT.v is assumed but length can be any type in C
              if (!Match.empty()) {
                auto *NewRequires = new Requires();
                NewRequires->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
                NewRequires->Ann = "pure (length " + Param->getNameAsString() +
                                   " == SizeT.v " + Match + "sz)";
                auto &Arr = FDefn->Annotation;
                Arr.insert(Arr.begin(), NewRequires);
              }
            }
          } else if (PulseAnnotKind == PulseAnnKind::HeapAllocated) {
            IsAllocatedOnHeap.insert(Param);
          } else {
            emitErrorWithLocation("Pulse annotation kind not implemented yet!",
                                  &Ctx, FD->getLocation());
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
        FArrTy->CInfo = getSourceInfoFromDecl(Param, Ctx, "");
        FArrTy->ElementType =
            getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        ParamTy = FArrTy;
      } else {
        ParamTy = getPulseTyFromCTy(Param->getType());
      }
    } else {
      //Try to infer automatically but this is commented out for now.
      //InferDeclType(Param, FD);
      auto It = DeclTyMap.find(Param);
      if (It != DeclTyMap.end()) {
        auto Ty = It->second;
        auto *FArrTy = new FStarArrType();
        FArrTy->CInfo = getSourceInfoFromDecl(Param, Ctx, "");
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
    Binder->CInfo = getSourceInfoFromDecl(Param, Ctx, "");

    auto *MutLetBindInit = new VarTerm(ParamName, ParamTy);
    MutLetBindInit->CInfo = getSourceInfoFromDecl(Param, Ctx, "");
    LetBinding *MutLetBindForParam = new LetBinding(ParamName,  MutLetBindInit , MutOrRef::MUT);
    MutLetBindForParam->CInfo = getSourceInfoFromDecl(Param, Ctx, "");
    MutLetBindForParam->VarTy = ParamTy->print();

    auto *LetMutParamTy = new FStarPointerType();
    LetMutParamTy->setPointerToTy(ParamTy);
    llvm::outs() << "Print the parameter type: " << LetMutParamTy->print()
                 << "\n";
    VEnv.insert(std::make_pair(MutLetBindInit, LetMutParamTy));

    if (ParamLetMutSequence == nullptr){
      ParamLetMutSequence = new PulseSequence();
      ParamLetMutSequenceHead = ParamLetMutSequence;
      ParamLetMutSequence->assignS1(MutLetBindForParam);
    }
    else{
      auto *NextSequence = new PulseSequence();
      NextSequence->assignS1(MutLetBindForParam);
      ParamLetMutSequence->assignS2(NextSequence);
      ParamLetMutSequence = NextSequence;
    }
    PulseArgs.push_back(Binder);
  }

  return ParamLetMutSequenceHead;

}


bool PulseVisitor::VisitFunctionDeclMain(FunctionDecl *FD, bool OverrideGen){

  if (OverrideGen){
    ForceVisitFunction = true;
    bool res = VisitFunctionDecl(FD);
    ForceVisitFunction = false;
    return res;
  }

  ForceVisitFunction = false;
  bool res = VisitFunctionDecl(FD);
  ForceVisitFunction = false;
  return res;
}

bool PulseVisitor::checkDeclNameExists(std::string DeclName){

  for (auto &P : DeclEnv){
    auto PulseDecl = P.second;
    if (auto *PF = dyn_cast<PulseFnDefn>(PulseDecl)){
      if (PF->Defn->Name == DeclName)
        return true;
    }
    else if (auto *PD = dyn_cast<PulseFnDecl>(PulseDecl)){
      if (PD->Defn->Name == DeclName)
        return true;
    }
    else{
      emitError("Unimplemented case for pulse declaration!");
    }
  }

  return false;
}


bool PulseVisitor::VisitFunctionDecl(FunctionDecl *FD) {
  
  //We may want to store the type of the function declarations regardless.
  //TODO: Limited to just one TU. Function declarations may be spread across TUs.
  // //If this is a declaration and it does have a function body, we ignore it.
  // //When we visit the actual declaration, we will generate code for it.
  // if (!FD->isThisDeclarationADefinition()){
  //   if (const FunctionDecl *Def = FD->getDefinition()){
  //     return true;
  //   }
  // }

  if (ForceVisitFunction)
    goto override;
  
  //Vidush: Revisit, I am removing this check since it prevents macro functions that are in included headers.
  //|| (FD->getLocation().isMacroID() &&
  //     !SM.isWrittenInMainFile(SM.getExpansionLoc(FD->getLocation())))

  if (SM.isInSystemHeader(FD->getLocation()) ||
      FD->isImplicit())
    return true;


  override:

  auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
  if (!FileEnt) {
    emitErrorWithLocation("Error: Main file entry not found in source manager.", &Ctx, FD->getLocation());
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
  auto It = Modules.find(ClangModuleName);
  PulseModul *Module = nullptr;
  if (It != Modules.end()) {
    Module = It->second;
  }
  // Create new pulse module.
  else {
    Module = new PulseModul();
    Module->includePulsePrelude = true;
    Module->ModuleName = ClangModuleName;
  }
  Modules.insert(std::make_pair(ClangModuleName, Module));

  std::string FuncName;
  auto *FDefn = new _PulseFnDefn();
  if (!FD->isThisDeclarationADefinition()){
    FuncName = FD->getNameAsString() + "_decl";
    //check if the function exists in the decl environment.
    //Another option is to error out if the user has duplicate definitions.
    if (checkDeclNameExists(FuncName)){
      return true;
    }
    
    FDefn->Name = FuncName;
    FDefn->isDeclaration = true;

    std::vector<Binder*> PulseArgs;
    std::vector<Binder*> ErasedArgs;
    bool Terminate = false;
    
    //TODO: The first map's key could just be made a string
    std::map<Term *, FStarType *> VEnv;
    std::set<std::string> VEnvSet;
   
    //Add variable names in scope
    //TODO This could also include globals later on.
    for (uint PI = 0; PI < FD->getNumParams(); PI++){
      auto *Param = FD->getParamDecl(PI);
      VEnvSet.insert(Param->getNameAsString());
    }

    handleFunctionAttributes(FD, FDefn, FuncName, Module, &Terminate,
                                            false,
                                            PulseArgs, 
                                            ErasedArgs, VEnvSet);
    
    //We automatically try to infer the RETURNS attribute if the user
    //did not annotate the function with any returns macro                                        
    if (!containsTerm(FDefn->Annotation, TermTag::Returns)){
      auto FDReturnTy = FD->getReturnType();
      if (!FDReturnTy->isVoidPointerType() && !FDReturnTy->isVoidType()){
        auto *FDPulseReturnTy = pulseTyFromDecl(FD);
        auto *NewReturns = new Returns();
        NewReturns->CInfo = getSourceInfoFromDecl(FD, Ctx, "");
        NewReturns->Ann = FDPulseReturnTy->print();

        //Make sure to insert this just before the ensures
        bool FoundEnsures = false;
        for (auto It = FDefn->Annotation.begin(); It != FDefn->Annotation.end(); It++){
          auto *Term = *It;
          if (Term->Tag == TermTag::Ensures){
            FDefn->Annotation.insert(It, NewReturns);
            FoundEnsures = true;
            break;
          }
        }
        
        //If no ensures are found, append returns at the end
        if (!FoundEnsures){
          FDefn->Annotation.push_back(NewReturns);
        }

      }
    }

    if (Terminate == true){
      return true;
    }

    //TODO: The let mut for function parameters is just unnecessary here. 
    //We should pass a flag to this function to not generate it in case of function declaration!
    //TODO: Vidush simplify the code.
    PulseSequence *ParamSequence = handleFunctionParameters(FD, FDefn, PulseArgs, VEnv, VEnvSet);
    
    std::copy(ErasedArgs.begin(), ErasedArgs.end(),
            std::back_inserter(PulseArgs));
    
            
    FDefn->Args = PulseArgs;

    auto *PulseDecl = new PulseFnDecl(FDefn);
    PulseDecl->CInfo = getSourceInfoFromDecl(FD, Ctx, "");

    // We don't handle functions that return a void type or a void pointer type!
    if (!FD->getReturnType()->isVoidType() &&
        !FD->getReturnType()->isVoidPointerType())
      PulseDecl->setDeclType(pulseTyFromDecl(FD));

    Module->Decls.push_back(PulseDecl);
    DeclEnv.insert(std::make_pair(FD, PulseDecl));
    return true;

  }

    bool HasAssociatedDefinition = false;
    auto *FunDecl = FD->getCanonicalDecl();
    //If a declaration does not exist for this function
    //We use its actual name not the declaration name!
    if (!FunDecl){
      FuncName = FD->getNameAsString(); 
    }
    else if (FunDecl != FD){
      //Assert that we already generated the function declaration!
      auto It = DeclEnv.find(FunDecl);
      // FunDecl->dump();
      // exit(0);
      if (It == DeclEnv.end()){
        //If not, please generate the declaration first.
        VisitFunctionDeclMain(FunDecl, true);
      }
      FuncName = FD->getNameAsString() + "_impl";
      HasAssociatedDefinition = true;
    }
    else {
      FuncName = FD->getNameAsString();
    }

    //check if the function exists in the decl environment.
    //Another option is to error out if the user has duplicate definitions.
    if (checkDeclNameExists(FuncName)){
      return true;
    }

    
    
    std::vector<Binder*> PulseArgs;
    std::vector<Binder*> ErasedArgs;
    bool Terminate = false;
    
    //TODO: The first maps key could just be make a string tbh.
    std::map<Term *, FStarType *> VEnv;
    std::set<std::string> VEnvSet;
   
    //Add variable names in scope
    //TODO This could also include globals later on.
    for (uint PI = 0; PI < FD->getNumParams(); PI++){
      auto *Param = FD->getParamDecl(PI);
      VEnvSet.insert(Param->getNameAsString());
    }

    handleFunctionAttributes(FD, FDefn, FuncName, Module, &Terminate,
                                            HasAssociatedDefinition,
                                            PulseArgs, 
                                            ErasedArgs, VEnvSet);
    
    //We automatically try to infer the RETURNS attribute if the user
    //did not annotate the function with any returns macro                                        
    if (!containsTerm(FDefn->Annotation, TermTag::Returns)){
      auto FDReturnTy = FD->getReturnType();
      if (!FDReturnTy->isVoidPointerType() && !FDReturnTy->isVoidType()){
        auto *FDPulseReturnTy = pulseTyFromDecl(FD);
        auto *NewReturns = new Returns();
        NewReturns->CInfo = getSourceInfoFromDecl(FD, Ctx, "");
        NewReturns->Ann = FDPulseReturnTy->print();

        //Make sure to insert this just before the ensures
        bool FoundEnsures = false;
        for (auto It = FDefn->Annotation.begin(); It != FDefn->Annotation.end(); It++){
          auto *Term = *It;
          if (Term->Tag == TermTag::Ensures){
            FDefn->Annotation.insert(It, NewReturns);
            FoundEnsures = true;
            break;
          }
        }
        
        //If no ensures are found, append returns at the end
        if (!FoundEnsures){
          FDefn->Annotation.push_back(NewReturns);
        }
        
      }
    }

    if (Terminate == true){
      return true;
    }

    //Vidush: TODO: We should check if the function name
    //is not shadowed. It is it is shadowed, we should change it.
    FDefn->Name = FuncName;
    FDefn->isDeclaration = false;

    FDefn->isRecursive = true;
    if (!checkIsRecursiveFunction(FD)){
      FDefn->isRecursive = false;
    }

    PulseSequence *ParamSequence = handleFunctionParameters(FD, FDefn, PulseArgs, VEnv, VEnvSet);
    
    std::copy(ErasedArgs.begin(), ErasedArgs.end(),
            std::back_inserter(PulseArgs));
            
    FDefn->Args = PulseArgs;

    PulseFnDefn *PulseFn = new PulseFnDefn(FDefn);
    PulseFn->CInfo = getSourceInfoFromDecl(FD, Ctx, "");
    DeclEnv.insert(std::make_pair(FD, PulseFn));

    if (Stmt *Body = FD->getBody()) {
      if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
        ExprMutationAnalyzer Analyzer(*CS, Ctx);

        // Track struct variables per function.
        //TrackStructExplodeAndRecover.clear();
        auto *PulseBody = pulseFromCompoundStmt(CS, VEnv, &Analyzer, Module);

        //Make a void return type ()
        if (PulseBody == nullptr){
          auto *ReturnExpr = new PulseExpr();
          auto *VoidRetTerm = new Name("()", nullptr);
          ReturnExpr->E = VoidRetTerm;
          PulseBody = ReturnExpr;
        }
        
        if (ParamSequence != nullptr){
          auto *NewBodySeq = new PulseSequence(); 
          NewBodySeq->assignS1(ParamSequence);
          NewBodySeq->assignS2(PulseBody);
          FDefn->Body = NewBodySeq;
        }
        else {
          FDefn->Body = PulseBody;
        }
      }
    }
    
    // We don't handle functions that return a void type or a void pointer type!
    if (!FD->getReturnType()->isVoidType() && !FD->getReturnType()->isVoidPointerType())
        PulseFn->setDeclType(pulseTyFromDecl(FD));

    DEBUG_WITH_TYPE(DEBUG_TYPE, {
      llvm::outs() << "=================================================";
      llvm::outs() << "\nPrint the Pulse function Definition:\n\n";
      PulseFn->dumpPretty();
      llvm::outs() << "\nEnd printing the function Definition\n\n";
      llvm::outs() << "=================================================\n";
    });

    PulseFn->Kind = PulseDeclKind::FnDefn;
    Module->Decls.push_back(PulseFn);
    return true;

  //Don't infer array types automatically but get that from ISARRAY
  //inferArrayTypes(FD);


  


      // Release declarations that are function parameters.
      //TODO: Vidush Eventually we should get rid of these.
      //This is just in case there are release expressions left and we need to release them.
      //PulseSequence *NewSeq = nullptr;
      //PulseSequence *Head = nullptr;
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

      //if (Head != nullptr) {
      //  DEBUG_WITH_TYPE(DEBUG_TYPE, {
      //  Head->dumpPretty();
      //  });
      //  FDefn->Body = Head;
      //} else if (PulseBody != nullptr) {
      //  DEBUG_WITH_TYPE(DEBUG_TYPE, {
      //  PulseBody->dumpPretty();
      //  });
      //}
}

bool PulseVisitor::isKnownArrayType(const Decl *D){

  auto It = DeclTyMap.find(D);
  if (It != DeclTyMap.end()){
    auto Ty = It->second; 
    if (Ty->isArrayType()){
      return true;
    }
  }
  return false;
}


FStarType *PulseVisitor::getPulseTyFromCTy(clang::QualType CType) {
  // TODO: Check if Ctype is a pointer type, if so, use FStarPointerType.

  FStarType *PulseTy;

  //TODO: Vidush
  //We need to implement function pointers.
  if (CType->isFunctionProtoType()){
    
  }

  if (CType->isPointerType()) {
    
    if (CType->isArrayType()) {
      emitError("PulseVisitor: Did not implement array type in clang");
    }

    if (CType->getPointeeType()->isRecordType()) {
      auto PointeeTy = CType->getPointeeType();
      auto *RecordTy = PointeeTy->getAs<RecordType>();
      auto *RD = RecordTy->getDecl();
      auto It = RecordToRecordName.find(RD);
      
      std::string StructName;
      if (It == RecordToRecordName.end()) {
        StructName = getNameForRecordDecl(RD);
      }
      else {
        StructName = It->second;
      }

      PulseTy = new FStarPointerType();
      auto *PulsePointerTy = static_cast<FStarPointerType *>(PulseTy);
      auto BaseTy = StructName;
      PulsePointerTy->setName("ref " + StructName);
      auto UnderLyingType = CType->getPointeeType();
      auto *FStartUnderLyingType = getPulseTyFromCTy(UnderLyingType);
      PulsePointerTy->setPointerToTy(FStartUnderLyingType);
      return PulsePointerTy;
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
  if (const auto *AT = CType->getAsArrayTypeUnsafe()) {
    
    QualType BaseTy;
    if (const auto *CAT = dyn_cast<clang::ConstantArrayType>(AT)) {
        BaseTy = CAT->getElementType();
    } else if (const auto *IAT = dyn_cast<clang::IncompleteArrayType>(AT)) {
        BaseTy = IAT->getElementType();  // For flexible array members
    } else if (const auto *VAT = dyn_cast<clang::VariableArrayType>(AT)) {
        BaseTy = VAT->getElementType();  // For VLAs
    }

    auto *ArrTy = new FStarArrType(); 
    //auto BaseTy = CType->getPointeeOrArrayElementType();
    //auto BaseTyQual = QualType(BaseTy, 0);
    ArrTy->ElementType = getPulseTyFromCTy(BaseTy);
    return ArrTy;
  }

  PulseTy = new FStarType();
  auto CTyKey = getSymbolKeyForCType(CType, Ctx);
  std::string CTyKeyStr;
  if (CTyKey != SymbolTable::UNKNOWN)
    CTyKeyStr = lookupSymbol(CTyKey);
  else{
    if (CType->isRecordType()){

      auto RT = CType->getAs<RecordType>();
      auto RD = RT->getDecl();
      auto It = RecordToRecordName.find(RD);
      std::string StructName;
      if (It == RecordToRecordName.end()) {
        StructName = getNameForRecordDecl(RD);
      }
      else {
        StructName = It->second;
      }
      CTyKeyStr = StructName;

    }
    else{
      CTyKeyStr = CType.getAsString();
    }
  }
  PulseTy->setName(CTyKeyStr);

  return PulseTy;

}

PulseStmt *PulseVisitor::pulseFromCompoundStmt(
    Stmt *S, std::map<Term *, FStarType *> TermTyEnv,
    ExprMutationAnalyzer *Analyzer, PulseModul *Modul) {

  PulseSequence *Stmt = nullptr;
  PulseSequence *Head = nullptr;
  if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    // //This only works on Structs for now.
    // //For any struct declaration,
    // TrackScopeOfStackAllocatedStructs.clear();

    for (auto *InnerStmt : CS->body()) {

      auto NextRet =
          pulseFromStmt(InnerStmt, TermTyEnv, Analyzer, nullptr, Modul, CS);
      auto *NextPulseStmt = NextRet.first;
      // Update Env to use the returned Env.
      TermTyEnv = NextRet.second;

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

bool PulseVisitor::checkAndAddIsArrayTy(const AttrVec &Attrs, const Decl* D, std::map<Term*, FStarType*> VEnv){
  
  bool IsArray = false;
  auto VEnvToSet = toSetVEnv(VEnv);
  for (auto *Att : Attrs) {
    if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)) {
      if (isPulseAnnotAttr(AnnonAttr)) {
        std::string Match;
        auto AnnKind = getPulseAnnKindFromString(AnnonAttr->getAnnotation(), Match);
        if (AnnKind != PulseAnnKind::IsArray){
          continue;
        }
        IsArray = true;
        addArrayTy(Match, D, VEnvToSet);
      }
    }
  }

  return IsArray;
}

std::pair<Term *, PulseVisitor::VarTyEnv>
PulseVisitor::getPulseTermForMallocSize(
    Expr *SizeExpr, std::map<Term *, FStarType *> VEnv, QualType ArrayElemType,
    clang::ExprMutationAnalyzer *A, llvm::SmallVector<PulseStmt *> &ExprsBef,
    clang::Stmt *Parent, clang::QualType ParentType, PulseModul *Module) {

  //(sizeof(int) * length)
  //(sizeof(int) * 100)
  //(length * sizeof(int))
  //(100 * sizeof(int))
  //(sizeof(..)) == length 1 array
  //Everything else errors out.
  //llvm::outs() << "Dump size expr for malloc \n";
  //SizeExpr->dumpPretty(Ctx);
  if (auto *BO = dyn_cast<BinaryOperator>(SizeExpr)){
    auto *Lhs = BO->getLHS();
    auto *Rhs = BO->getRHS();
    
    ///(sizeof(..) * _)
    if (auto *SizeOfExpr = dyn_cast<UnaryExprOrTypeTraitExpr>(Lhs)){
      if (SizeOfExpr->getKind() == UETT_SizeOf){
        auto Ty = SizeOfExpr->getArgumentType();
        if (Ty != ArrayElemType){
          emitErrorWithLocation("Size of malloc not supported!", &Ctx, SizeExpr->getExprLoc());
        }
      }
    
    
    Expr::EvalResult RhsResultVal;
    if (Rhs->EvaluateAsInt(RhsResultVal, Ctx)) {
      llvm::APSInt Value = RhsResultVal.Val.getInt();
      int64_t ValSigned = Value.getZExtValue();
      if (ValSigned <= 0) {
        emitErrorWithLocation("The length of the array cannot be 0 or negative!", &Ctx, Rhs->getExprLoc());
      }
    }

    //Generate term for Rhs and return it as the length of the array.
    auto Ret =
        getTermFromCExpr(Rhs, VEnv, A, ExprsBef, Parent, ParentType, Module);
    return Ret;
  }
  
  ///(_ * sizeof(..))
  if (auto *SizeOfExpr = dyn_cast<UnaryExprOrTypeTraitExpr>(Rhs)){
      if (SizeOfExpr->getKind() == UETT_SizeOf){
        auto Ty = SizeOfExpr->getArgumentType();
        if (Ty != ArrayElemType){
          emitErrorWithLocation("Size of malloc not supported!", &Ctx, SizeExpr->getExprLoc());
        }
      }
    
    
    Expr::EvalResult LhsResultVal;
    if (Lhs->EvaluateAsInt(LhsResultVal, Ctx)) {
      llvm::APSInt Value = LhsResultVal.Val.getInt();
      int64_t ValSigned = Value.getZExtValue();
      if (ValSigned <= 0) {
        emitErrorWithLocation("The length of the array cannot be 0 or negative!", &Ctx, Lhs->getExprLoc());
      }
    }

    //Generate term for Rhs and return it as the length of the array.
    auto Ret =
        getTermFromCExpr(Lhs, VEnv, A, ExprsBef, Parent, ParentType, Module);
    return Ret;
  }

  emitErrorWithLocation("Size of malloc not supported!", &Ctx, SizeExpr->getExprLoc());

  }
  else if (auto *SizeOfExpr = dyn_cast<UnaryExprOrTypeTraitExpr>(SizeExpr)){
    if (SizeOfExpr->getKind() == UETT_SizeOf){
        auto Ty = SizeOfExpr->getArgumentType();
        if (Ty != ArrayElemType){
          emitErrorWithLocation("Size of malloc not supported!", &Ctx, SizeExpr->getExprLoc());
        }
    }

    auto *ConstTy = getPulseTyFromCTy(SizeOfExpr->getType());
    auto *CT = new ConstTerm("1", ConstTy);
    CT->CInfo = getSourceInfoFromExpr(SizeExpr, Ctx, "", "");
    CT->ConstantValue = "1";
    CT->Symbol = getSymbolKeyForCType(SizeOfExpr->getType(), Ctx);
    return std::make_pair(CT, VEnv);
  }
  emitErrorWithLocation("Shape of malloc not supported!", &Ctx, SizeExpr->getExprLoc());
}

//Vidush: 
//Only handles mallocs of specified shape.
//Valid shape
// int *a = (int*) malloc(sizeof(int) * length);
// int *a = (int*) malloc(sizeof(int) * 10);
// int *a = malloc(sizeof(int) * length);
//Invalid
// int *a = (uint64_t*) malloc(sizeof(int)...);
// int *a = malloc(1000);
// int *a = malloc(sizeof(float) * 1000);
// int *a = malloc(length);
std::pair<PulseStmt *, PulseVisitor::VarTyEnv> PulseVisitor::handleMallocs(
    Expr *E, std::map<Term *, FStarType *> VEnv, const VarDecl *VD,
    clang::DeclStmt *DS, clang::Decl *D, clang::ExprMutationAnalyzer *A,
    llvm::SmallVector<PulseStmt *> &ExprsBef, clang::Stmt *Parent,
    clang::QualType ParentType, PulseModul *Module) {
  auto handleMallocHelper =
      [this](CallExpr *Call, std::map<Term *, FStarType *> VEnv,
             clang::ASTContext &Ctx, const clang::VarDecl *VD,
             const clang::Decl *D, clang::DeclStmt *DS,
             clang::ExprMutationAnalyzer *A,
             llvm::SmallVector<PulseStmt *> &ExprsBef, PulseModul *Module) {
        Expr::EvalResult Result;
        auto *SizeExpr = Call->getArg(0);
        auto *PulseTy = pulseTyFromDecl(VD);
        auto ArrayElemType = VD->getType()->getPointeeType();
        if (auto *PulseArrTy = dyn_cast<FStarArrType>(PulseTy)) {
          auto Ret =
              getPulseTermForMallocSize(SizeExpr, VEnv, ArrayElemType, A,
                                        ExprsBef, DS, VD->getType(), Module);
          auto *TermForSizeExpr = Ret.first;
          auto *PulseSizeTTy = new FStarType(lookupSymbol(SymbolTable::SizeT));

          llvm::outs() << "Print Size Expr: ";
          TermForSizeExpr->dumpPretty();
          TermForSizeExpr->getType()->dumpPretty();
          llvm::outs() << "Print: "
                       << getPulseTyAsString(TermForSizeExpr->getType())
                       << "\n";
          llvm::outs() << "\n";

          if (getPulseTyAsString(TermForSizeExpr->getType()) !=
              lookupSymbol(SymbolTable::SizeT)) {
            auto *CastCall = new AppE(
                getPulseStringForCType(SizeExpr->getType(), Ctx) + "_to_sizet",
                PulseSizeTTy);
            CastCall->CInfo = getSourceInfoFromExpr(SizeExpr, Ctx, "", "");
            CastCall->pushArg(TermForSizeExpr);
            auto *NewParen = new Paren(CastCall, PulseSizeTTy);
            NewParen->CInfo = getSourceInfoFromExpr(SizeExpr, Ctx, "", "");
            TermForSizeExpr = NewParen;
          }

          auto SizeName = c2pulse::gensym("size_expr");
          auto *LetBindSize = new LetBinding(SizeName, TermForSizeExpr, MutOrRef::MUT);
          LetBindSize->CInfo = getSourceInfoFromExpr(SizeExpr, Ctx, "", "");
          //Since we added a cast this is fine to hardcode.
          LetBindSize->VarTy = "SizeT.t";

          auto *AllocationCall = new AppE("alloc_array", PulseArrTy);
          AllocationCall->CInfo = getSourceInfoFromExpr(Call, Ctx, "", "");
          auto *ElemTy = PulseArrTy->ElementType;
          auto SymbolElemTy = ElemTy->print();
          auto *Arg1 = new Name("#" + SymbolElemTy, ElemTy);
          Arg1->CInfo = getSourceInfoFromExpr(Call, Ctx, "", "");
          AllocationCall->pushArg(Arg1);
      
          //Since this is a LetMut add a !.
          auto *Arg2 = new Name("!" + SizeName, PulseSizeTTy);
          Arg2->CInfo = getSourceInfoFromExpr(Call, Ctx, "", "");
          AllocationCall->pushArg(Arg2);
          auto *NewLet = new LetBinding(VD->getNameAsString(), AllocationCall, MutOrRef::MUT);
          NewLet->CInfo = getSourceInfoFromExpr(Call, Ctx, "", "");
          NewLet->VarTy = PulseArrTy->print();

          auto *NewSeq = new PulseSequence(); 
          NewSeq->assignS1(LetBindSize);
          NewSeq->assignS2(NewLet);
          return std::make_pair(NewSeq, Ret.second);
        }
        emitErrorWithLocation("Expected Array type!", &Ctx,
                              Call->getBeginLoc());
      };

  //int *a = malloc(sizeof(int) * length);
  if (auto *IC = dyn_cast<ImplicitCastExpr>(E)){
    if (auto *Call = dyn_cast<CallExpr>(IC->getSubExpr())){
      if (Call->getDirectCallee()->getNameAsString() == "malloc"){
        auto RetStmt =
            handleMallocHelper(Call, VEnv, Ctx, VD, D, DS, A, ExprsBef, Module);
        return RetStmt;
      }
    }
  }

  //int *a = (int*) malloc(sizeof(int) * 10);
  //int *a = (int*) malloc(sizeof(int) * length);
  if (auto *C = dyn_cast<CStyleCastExpr>(E)){
    auto *SubExpr = C->getSubExpr();
    //LHS and RHS should have the same Type! 
    if (C->getType() == VD->getType()){
      if (auto *Call = dyn_cast<CallExpr>(SubExpr)){
        if (Call->getDirectCallee()->getNameAsString() == "malloc"){
          auto RetStmt = handleMallocHelper(Call, VEnv, Ctx, VD, D, DS, A,
                                            ExprsBef, Module);
          return RetStmt;
        }
      }
    }
  }
  
  E->dump();
  emitErrorWithLocation("Encountered a Malloc call whose shape is not supported!", &Ctx, E->getExprLoc());
}



Term *PulseVisitor::checkAndAddCast(Term *SrcTerm, Term *DstType){
  
  auto *SrcType = SrcTerm->getType();
  if (SrcType == nullptr){
    return SrcTerm;
  }
  
  llvm::outs() << "Print the source type!\n\n";
  SrcType->dumpPretty();
  llvm::outs() << "\n\nPrint the destination type!\n";
  DstType->dumpPretty();
  llvm::outs() << "\n\nEnd printing Dst type\n";

  if (getPulseTyAsString(SrcType) != getPulseTyAsString(DstType)){
    
    auto *CastCall = new AppE(getCastNameForPulseType(SrcType) + "_to_" + getCastNameForPulseType(DstType), DstType);
    CastCall->CInfo = SrcTerm->CInfo;
    CastCall->pushArg(SrcTerm);
    
    auto *ParenAroundCall = new Paren(CastCall, DstType);
    return ParenAroundCall;
  }

  return SrcTerm;
}

std::pair<PulseStmt *, PulseVisitor::VarTyEnv>
PulseVisitor::pulseFromStmt(Stmt *S, std::map<Term *, FStarType *> VEnv,
                            ExprMutationAnalyzer *Analyzer, Stmt *Parent,
                            PulseModul *Module, CompoundStmt *CS) {

  if (!S)
    return std::make_pair(nullptr, VEnv);

  if (auto *DS = dyn_cast<DeclStmt>(S)) {
    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {

        if (VD->hasInit()) {
          auto *Init = VD->getInit();
          if (auto *Call = dyn_cast<CallExpr>(Init->IgnoreCasts()->IgnoreParens()->IgnoreCasts())){
            if (Call->getDirectCallee()->getNameAsString() == "malloc"){
              //Check if the user wanted this allocation to be an array
              if (VD->hasAttrs() && checkAndAddIsArrayTy(VD->getAttrs(), D, VEnv)){
                auto PulseTy = pulseTyFromDecl(VD);
                auto *ArrayRefTy = new FStarPointerType(PulseTy);
                auto *ArrNameTerm =
                    new VarTerm(VD->getNameAsString(), ArrayRefTy);
                insertPulseTyEnv(ArrNameTerm, ArrayRefTy, VEnv);

                llvm::SmallVector<PulseStmt *> ExprsBef;
                return handleMallocs(Init, VEnv, VD, DS, D, Analyzer, ExprsBef,
                                     DS, VD->getType(), Module);
              }
              //translate it as a ref
              goto init_standard;
            }
          }
          
          init_standard:
            auto VarName = VD->getNameAsString();
            // Unsure if we really need the type here.
            // Though it may be usefuel checking invalid casting operations.
            // auto VarType = VD->getType();

            // Don't forget to release these exprs.
            SmallVector<PulseStmt *> NewExprs;
            auto LetInitRet = getTermFromCExpr(Init, VEnv, Analyzer, NewExprs,
                                               Parent, VD->getType(), Module);
            Term *LetInit = LetInitRet.first;

            // Overwrite old env.
            VEnv = LetInitRet.second;

            FStarType *VDPulseTy;
            if (VD->hasAttrs() && checkAndAddIsArrayTy(VD->getAttrs(), D, VEnv)){
                VDPulseTy = pulseTyFromDecl(VD);
            }
            else {
              VDPulseTy = getPulseTyFromCTy(VD->getType());
            }

            
            // if (Analyzer->isMutated(D)) {
            // auto TempVarName = c2pulse::gensym(VarName);
            // auto *PulseLetTmp = new LetBinding(TempVarName, LetInit,
            // MutOrRef::NOTMUT); PulseLetTmp->CInfo = getSourceInfoFromStmt(S,
            // Ctx, "", ""); PulseLetTmp->VarTy = VDPulseTy->print();

            // Make the call to assert only for refs.
            // Better to have this in pulse side.
            // auto *PulseLetSeq = new PulseSequence();
            //  if (auto *VDPulseTyToRef = dyn_cast<FStarPointerType>(VDPulseTy)){
            //    auto *RewritesTo = new AppE("rewrites_to");
            //    auto *Arg1 = new VarTerm(TempVarName);
            //    RewritesTo->pushArg(Arg1);
            //    RewritesTo->pushArg(LetInit);
            //    auto *AssertCall = new AppE("assert");
            //    AssertCall->pushArg(RewritesTo);
            //    auto *AssertCallExpr = new PulseExpr();
            //    AssertCallExpr->E = AssertCall;

            //   auto *Seq1 = new PulseSequence(); 
            //   Seq1->assignS1(PulseLetTmp);
            //   Seq1->assignS2(AssertCallExpr);

            //   LetBinding *PulseLet = new LetBinding(VarName, new VarTerm(TempVarName), MutOrRef::MUT);
            //   PulseLet->VarTy = VDPulseTy->print();

            //   PulseLetSeq->assignS1(Seq1);
            //   PulseLetSeq->assignS2(PulseLet);
            // }
            // else {
            // LetBinding *PulseLet = new LetBinding(VarName, new
            // VarTerm(TempVarName), MutOrRef::MUT);
            //LetInit->dumpPretty();
            auto *CastLetInit = checkAndAddCast(LetInit, VDPulseTy);
            LetBinding *PulseLet = new LetBinding(VarName, CastLetInit, MutOrRef::MUT);
            PulseLet->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");
            PulseLet->VarTy = VDPulseTy->print();

            auto *RefTy = new FStarPointerType(VDPulseTy);
            auto *VDNameTerm = new VarTerm(VD->getNameAsString(), RefTy);
            insertPulseTyEnv(VDNameTerm, RefTy, VEnv);

            // auto *LetMutVarTy = new FStarPointerType();
            // LetMutVarTy->PointerTo = lookupPulseTyEnv(LetInit, VEnv);
            // TODO: Set the CInfo LetMutVarTy->CInfo ....
            // Perhaps we should change LetBinding() To store this as Lhs rather
            // than just a string.
            // auto *NewTermForVarName = new VarTerm(VarName, LetMutVarTy);
            // insertPulseTyEnv(NewTermForVarName, LetMutVarTy, VEnv);

            // PulseLetSeq->assignS1(PulseLetTmp);
            // PulseLetSeq->assignS2(PulseLet);
            //}

            //} else {
            //  PulseLet = new LetBinding(VarName, LetInit, MutOrRef::NOTMUT);
            //}

            //PulseLet->CInfo = getSourceInfoFromExpr(VD, Ctx, "", "");

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
              // Start->assignS2(PulseLetSeq);
              Start->assignS2(PulseLet);

              // check for any lemmas to be released.
              auto Attrs = VD->attrs();
              for (auto *Att : Attrs) {
                if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)) {
                  if (isPulseAnnotAttr(AnnonAttr)) {
                    std::string Match;
                    auto AnnKind = getPulseAnnKindFromString(
                      AnnonAttr->getAnnotation(), Match);
                    if (AnnKind == PulseAnnKind::LemmaStatement){
                  
                      auto *LS = new LemmaStatement();
                      LS->CInfo = getSourceInfoFromAttr(Att, Ctx, "");
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
              }
              return std::make_pair(Start, LetInitRet.second);
            }

          // check for any lemmas to be released.
          auto Attrs = VD->attrs();
          for (auto *Att : Attrs) {
            if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)) {
              if (isPulseAnnotAttr(AnnonAttr)) {
                std::string Match;
                auto AnnKind = getPulseAnnKindFromString(
                    AnnonAttr->getAnnotation(), Match);

                if (AnnKind == PulseAnnKind::LemmaStatement) {
                  auto *LS = new LemmaStatement();
                  LS->CInfo = getSourceInfoFromAttr(Att, Ctx, "");
                  LS->Lemma = Match;
                  DEBUG_WITH_TYPE(DEBUG_TYPE , {
                  llvm::outs() << "Found Lemma: " << "\n";
                  llvm::outs() << Match << "\n";
                  llvm::outs() << "End.\n";
                  });

                  auto *LSE = new PulseExpr();
                  LSE->CInfo = getSourceInfoFromAttr(Att, Ctx, "");
                  LSE->E = LS;
                  if (Start == nullptr) {
                    auto *NewS = new PulseSequence();
                    NewS->assignS1(LSE);
                    Start = NewS;
                  }
                  Start->assignS2(LSE);
                } else if (AnnKind == PulseAnnKind::HeapAllocated) {
                  IsAllocatedOnHeap.insert(VD);
                }
                else if (AnnKind == PulseAnnKind::IsArray){
                  auto VEnvToSet = toSetVEnv(VEnv);
                  addArrayTy(Match, VD, VEnvToSet);
                }
                else {
                  emitErrorWithLocation("Did not expect pulse annotation kind!",
                                        &Ctx, VD->getLocation());
                }
              }
            }
          }

          auto *AppendLet = new PulseSequence();
          AppendLet->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");

          AppendLet->assignS1(Start);
          // AppendLet->assignS2(PulseLetSeq);
          AppendLet->assignS2(PulseLet);
          return std::make_pair(AppendLet, VEnv);
        }

        // TODO: Vidush: Handle if for record without typedefs
        // Also handle when typedef don't have a name defined.
        if (const TypedefType *TT = VD->getType()->getAs<TypedefType>()) {

            auto *TypedefDecl = TT->getDecl();
            auto StructName = TypedefDecl->getDeclName();
            DEBUG_WITH_TYPE(DEBUG_TYPE , {
            llvm::outs() << "Typedef name: " << StructName << "\n";
            });
            //if (Analyzer->isMutated(VD)){

            auto *Rhs = new Name(StructName.getAsString() + "_default " +
                                     StructName.getAsString() + "_spec_default",
                                 nullptr);
            Rhs->CInfo = getSourceInfoFromDecl(VD, Ctx, "");
            
            auto *PulseTy = getPulseTyFromCTy(VD->getType());
            auto *NewMutLet =
                new LetBinding(VD->getNameAsString(), Rhs, MutOrRef::MUT);
           
            NewMutLet->VarTy = PulseTy->print();
            NewMutLet->CInfo = getSourceInfoFromDecl(VD, Ctx, "");

            auto *StructRefTy = new FStarPointerType(PulseTy);
            auto *StructNameTerm =
                new VarTerm(VD->getNameAsString(), StructRefTy);
            insertPulseTyEnv(StructNameTerm, StructRefTy, VEnv);

            return std::make_pair(NewMutLet, VEnv);
            //}

            // Implement case when the allocation is not mutated.
            //  A normal let bind
            // emitErrorWithLocation("Did not implement case when struct "
            //                       "allocation is not mutated!",
            //                       &Ctx, VD->getLocation());
        }
        
        //Handle non initalized arrays.
        if (const auto *ArrTy = VD->getType()->getAsArrayTypeUnsafe()) {
          if (const auto *ConstArrTy = llvm::dyn_cast<ConstantArrayType>(ArrTy)) {
            // Constant-sized array
            // Stack Allocated
            if (VD->hasLocalStorage()){
              auto *PulseTy = pulseTyFromDecl(VD);
              auto ArrSize = ConstArrTy->getSize();
              //[| witness #_ #_; 10sz |]
              auto *LetInit = new Name(
                  "[| witness #_ #_; " +
                      std::to_string(ArrSize.getSExtValue()) + "sz" + " |]",
                  PulseTy);
              auto Lhs = VD->getNameAsString(); 
              auto *ConstantArrLet = new LetBinding(Lhs, LetInit, MutOrRef::MUT);
              ConstantArrLet->VarTy = PulseTy->print();

              // TODO: Add VarTerm as part of the LetBinding() ?
              auto *ArrayRefTy = new FStarPointerType(PulseTy);
              auto *ArrNameTerm =
                  new VarTerm(VD->getNameAsString(), ArrayRefTy);
              insertPulseTyEnv(ArrNameTerm, ArrayRefTy, VEnv);

              return std::make_pair(ConstantArrLet, VEnv);
            }
            else {
              emitErrorWithLocation("Not implemented non local storage constant array!", &Ctx, VD->getLocation());
            }
          } else if (const auto *VarArrTy = llvm::dyn_cast<VariableArrayType>(ArrTy)) {
            if (VD->hasLocalStorage()){
              auto *PulseTy = pulseTyFromDecl(VD);
              auto *SizeExpr = VarArrTy->getSizeExpr(); 
              SmallVector<PulseStmt*> ExprsBef;
              auto PulseSizeExprRet =
                  getTermFromCExpr(SizeExpr, VEnv, Analyzer, ExprsBef, Parent,
                                   VD->getType(), Module);
              auto *PulseSizeExpr = PulseSizeExprRet.first;
              VEnv = PulseSizeExprRet.second;

              auto NewNameForSizeExpr = c2pulse::gensym("size_expr");
              //This should ideally be casted to a sizeTy. 
              //Add a cast here if the expression type is not sizet. 
              //Vidush: TODO check if this is correct.
              auto PulseSizeTTy =
                  new FStarType(lookupSymbol(SymbolTable::SizeT));

              // TODO: Use the pulse ty environment to get the type of the size
              // expression auto *SizeExprPulseTy =
              // lookupPulseTyEnv(PulseSizeExpr, VEnv);
              PulseSizeExpr->dumpPretty();
              llvm::outs() << "Print Type: "
                           << getPulseTyAsString(PulseSizeExpr->getType())
                           << "\n";
              if (getPulseTyAsString(PulseSizeExpr->getType()) !=
                  lookupSymbol(SymbolTable::SizeT)) {
                PulseSizeExpr->dumpPretty();
                llvm::outs()
                    << "Print Type: "
                    << getPulseTyAsString(PulseSizeExpr->getType()) << "\n";
                auto *Cast =
                    new AppE(getPulseStringForCType(SizeExpr->getType(), Ctx) +
                                 "_to_sizet",
                             PulseSizeTTy);
                Cast->pushArg(PulseSizeExpr);
                PulseSizeExpr = Cast;
              }

              auto *LetSizeExpr = new LetBinding(NewNameForSizeExpr, PulseSizeExpr, MutOrRef::MUT);
              //hardcoding it because of the cast.
              LetSizeExpr->VarTy = "SizeT.t";
              auto *LetInit = new Name(
                  "[| witness #_ #_; !" + NewNameForSizeExpr + " |]", PulseTy);
              auto *NewLetBind = new LetBinding(VD->getNameAsString(), LetInit, MutOrRef::MUT);
              NewLetBind->VarTy = PulseTy->print();

              // TODO: Add VarTerm as part of the LetBinding() ?
              auto *ArrayRefTy = new FStarPointerType(PulseTy);
              auto *ArrNameTerm =
                  new VarTerm(VD->getNameAsString(), ArrayRefTy);
              insertPulseTyEnv(ArrNameTerm, ArrayRefTy, VEnv);

              auto *RetSeq = new PulseSequence(); 
              RetSeq->assignS1(LetSizeExpr);
              RetSeq->assignS2(NewLetBind);
              return std::make_pair(RetSeq, VEnv);

            }
            else {
              emitErrorWithLocation("Not implemented non local storage constant array!", &Ctx, VD->getLocation());
            }
          }
          else {
            emitErrorWithLocation("Not implemented array type!", &Ctx, VD->getLocation());
          }
        }
        
        //Uninitialized array that is a pointer.
        //Check if it has been marked as an array by the user. 
        if (VD->hasAttrs() && checkAndAddIsArrayTy(VD->getAttrs(), D, VEnv)){
          auto *PulseTy = pulseTyFromDecl(VD);
          auto VarDeclName = VD->getNameAsString(); 

          auto *GenericDecl = new GenericStmt();
          GenericDecl->CInfo = getSourceInfoFromDecl(VD, Ctx, "");

          GenericDecl->body =
            "let mut " + VarDeclName + ": " + PulseTy->print() + " = witness #_ #_;";

          // TODO: Add VarTerm as part of the LetBinding() ?
          auto *RefTy = new FStarPointerType(PulseTy);
          auto *NameTerm = new VarTerm(VarDeclName, RefTy);
          insertPulseTyEnv(NameTerm, RefTy, VEnv);

          return std::make_pair(GenericDecl, VEnv);
        }

        // Any uninitialized declaration that is not a struct
        auto CType = VD->getType();
        auto *PulseTy = getPulseTyFromCTy(CType);
        auto ClangVarName = VD->getNameAsString();

        auto *GenericDecl = new GenericStmt();
        GenericDecl->CInfo = getSourceInfoFromDecl(VD, Ctx, "");
        //if (Analyzer->isMutated(VD)){
        GenericDecl->body =
            "let mut " + ClangVarName + ": " + PulseTy->print() + " = witness #_ #_;";

        // TODO: Add VarTerm as part of the LetBinding() ?
        auto *RefTy = new FStarPointerType(PulseTy);
        auto *NameTerm = new VarTerm(ClangVarName, RefTy);
        insertPulseTyEnv(NameTerm, RefTy, VEnv);

        //}
        //else {
        //  GenericDecl->body =
        //    "let " + ClangVarName + ": " + PulseTyStr + " = witness #_ #_;";
        //}
        return std::make_pair(GenericDecl, VEnv);
      }
      emitErrorWithLocation(
          "Declarations other than variable declarations not implemented!",
          &Ctx, D->getLocation());
    }

  } else if (auto *BO = dyn_cast<BinaryOperator>(S)) {

    if (BO->isAssignmentOp()) {


      auto handleCompoundOpsHelper = [](clang::Expr* Lhs, 
                                                  clang::BinaryOperator *BO,
                                                  Term *PulseLhsTerm,
                                                  Term *PulseRhsTerm,
                                                  ASTContext &Ctx){

        if (!BO->isCompoundAssignmentOp()){
          return PulseRhsTerm;
        }
        
        auto *RhsForPulseLhsTerm = PulseLhsTerm;
        if (Lhs->isLValue()){
          auto *PulseTy = PulseLhsTerm->getType();
          if (auto *PulseRefTy = dyn_cast<FStarPointerType>(PulseTy)){
            auto PointeeTy = PulseRefTy->PointerTo;
            auto *CallForLhs = new AppE("!", PointeeTy);
            CallForLhs->pushArg(PulseLhsTerm);
            RhsForPulseLhsTerm = new Paren(CallForLhs, PointeeTy);
          }
          else {
            emitErrorWithLocation("Expected lhs to be a ref!", &Ctx, Lhs->getExprLoc());
          }
        }
        
        //Check the kind of assignment
        //TODO factor into a lambda function.
        if (BO->getOpcode() == clang::BO_AddAssign){
            SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
            auto Op = clang::BO_Add;
            auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);
            auto *Call = new AppE(OpKey, RhsForPulseLhsTerm->getType());
            Call->pushArg(RhsForPulseLhsTerm);
            Call->pushArg(PulseRhsTerm);
            Term *Ret = Call;
            return Ret;
          }
          else if (BO->getOpcode() == clang::BO_SubAssign){
            SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
            auto Op = clang::BO_Sub;
            auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);
            auto *Call = new AppE(OpKey, RhsForPulseLhsTerm->getType());
            Call->pushArg(RhsForPulseLhsTerm);
            Call->pushArg(PulseRhsTerm);
            Term *Ret = Call;
            return Ret;
          }
          else if (BO->getOpcode() == clang::BO_MulAssign){
            SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
            auto Op = clang::BO_Mul;
            auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);
            auto *Call = new AppE(OpKey, RhsForPulseLhsTerm->getType());
            Call->pushArg(RhsForPulseLhsTerm);
            Call->pushArg(PulseRhsTerm);
            Term *Ret = Call;
            return Ret;
          }
          else if (BO->getOpcode() == clang::BO_DivAssign){
            SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
            auto Op = clang::BO_Div;
            auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);
            auto *Call = new AppE(OpKey, RhsForPulseLhsTerm->getType());
            Call->pushArg(RhsForPulseLhsTerm);
            Call->pushArg(PulseRhsTerm);
            Term *Ret = Call;
            return Ret;
          }
          else {
            emitErrorWithLocation("Unimplemented compound assignment!", &Ctx, BO->getExprLoc());
          }
      };


      auto *Lhs = BO->getLHS();
      auto *Rhs = BO->getRHS();

      if (UnaryOperator *UO = dyn_cast<UnaryOperator>(Lhs)) {

        if (UO->getOpcode() == UO_Deref) {

          // TODO: Make sure to release these expressions
          SmallVector<PulseStmt *> ExprsBef;

          auto *SubExpr = UO->getSubExpr();
          
          //TODO: Vidush
          //check if the dereference is to a ref or an array.
          //We can do some shape analysis here to determine if 
          // the dereference is of the following types:
          // *x where x is a known array
          // *(x + constant) where x is a known array
          // *(var + constant + x + var + constant)
          // We should be able to generalize these as 
          // A base array + some constant or variable offset.

          auto PulseLhsTermRet = getTermFromCExpr(
              SubExpr, VEnv, Analyzer, ExprsBef, Parent, BO->getType(), Module);
          auto *PulseLhsTerm = PulseLhsTermRet.first;

          auto PulseRhsTermRet =
              getTermFromCExpr(Rhs, PulseLhsTermRet.second, Analyzer, ExprsBef,
                               Parent, BO->getType(), Module);
          auto *PulseRhsTerm = PulseRhsTermRet.first;
          PulseRhsTerm = handleCompoundOpsHelper(Lhs, BO, PulseLhsTerm, PulseRhsTerm, Ctx);
          
          PulseAssignment *Assignment =
              new PulseAssignment(PulseLhsTerm, PulseRhsTerm);

          Assignment->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");

          assert(ExprsBef.empty() && "Expected expressions to be released!");

          return std::make_pair(Assignment, PulseRhsTermRet.second);
        }
      //TODO: We should also handle pointer arithmetic that are to arrays. 
      //Vidush: For starters, we can start with expressions such as: 
      //*x -> x[0];
      //*(x + constant) -> x[constant];
      //*(x + i) -> x[i]
      } else if (auto *ArrSub = dyn_cast<ArraySubscriptExpr>(Lhs)) {

        // TODO: Make sure to release these expressions
        SmallVector<PulseStmt *> ExprsBef;

        auto *ArrayAssignExpr = new PulseArrayAssignment();
        ArrayAssignExpr->CInfo = getSourceInfoFromExpr(ArrSub, Ctx, "", "");

        auto ArrRet = getTermFromCExpr(ArrSub->getBase(), VEnv, Analyzer,
                                       ExprsBef, Parent, BO->getType(), Module);

        ArrayAssignExpr->Arr = ArrRet.first;

        auto IdxExpr = ArrSub->getIdx();
        auto ArrIdxExprRet =
            getTermFromCExpr(IdxExpr, ArrRet.second, Analyzer, ExprsBef, Parent,
                             BO->getType(), Module);
        auto *ArrIdxExpr = ArrIdxExprRet.first;

        // We use the Pulse type to check if the expression returns sizet.
        auto *NewSizeT = new FStarType(lookupSymbol(SymbolTable::SizeT));
        if (getPulseTyAsString(ArrIdxExpr->getType()) !=
            lookupSymbol(SymbolTable::SizeT)) {
          auto *CastCall = new AppE(
              getPulseStringForCType(IdxExpr->getType(), Ctx) + "_to_sizet",
              NewSizeT);
          CastCall->CInfo = getSourceInfoFromExpr(IdxExpr, Ctx, "", "");
          CastCall->pushArg(ArrIdxExpr);
          ArrIdxExpr = CastCall;
        }

        ArrayAssignExpr->Index = ArrIdxExpr;
        auto ValueRet =
            getTermFromCExpr(Rhs, ArrIdxExprRet.second, Analyzer, ExprsBef,
                             Parent, BO->getType(), Module);
        ArrayAssignExpr->Value = ValueRet.first;

        ArrayAssignExpr->CInfo = getSourceInfoFromStmt(Lhs, Ctx, "", "");

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
          return std::make_pair(Start, ValueRet.second);
        }

        return std::make_pair(ArrayAssignExpr, ValueRet.second);
      } else if (auto *ME = dyn_cast<MemberExpr>(Lhs)) {

        auto *LhsDecl = ME->getMemberDecl();
        // TODO: Vidush
        // We are ignoring implicit casts here
        // However, we should check if that' the right approach.
        //  casts may need to be handled and translated into !
        auto *BaseExpr = ME->getBase();
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        ME->dump();
        BaseExpr->dump();
        });

        //std::string NameOfDecl;
        //std::string StructName;
        // if (const clang::DeclRefExpr *DRE =
        //         llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
        //   const clang::ValueDecl *VD = DRE->getDecl();
        //   // Now you can safely cast VD to a more specific Decl type if needed
        //   DEBUG_WITH_TYPE(DEBUG_TYPE , {
        //   VD->dump();
        //   llvm::outs() << VD->getDeclName() << "End\n";
        //   });
        //   NameOfDecl = VD->getDeclName().getAsString();

        //   auto TyOfDecl = VD->getType();
        //   QualType StructTy;
        //   // Step 1: If it's a pointer, unwrap it
        //   if (TyOfDecl->isPointerType()) {
        //     StructTy = TyOfDecl->getPointeeType();
        //   } else {
        //     StructTy = TyOfDecl;
        //   }

        //   DEBUG_WITH_TYPE(DEBUG_TYPE , {
        //   llvm::outs() << StructTy.getAsString() << "\n";
        //   });

        //   // Step 2: Desugar the type (handles typedefs, elaborated types, etc.)
        //   clang::QualType StrucTyDesugared = StructTy.getDesugaredType(Ctx);

        //   // Step 3: Try to get the RecordType
        //   const auto *recordType = StrucTyDesugared->getAs<clang::RecordType>();

        //   const clang::RecordDecl *RecordDec = recordType->getDecl();
        //   llvm::outs() << "Struct name: " << RecordDec->getNameAsString()
        //                << "\n";

        //   if (!RecordDec) {
        //     emitErrorWithLocation("Could not find record declaration!", &Ctx,
        //                           ME->getBeginLoc());
        //   }

        //   // auto It = MapRecordDeclsToTypedefDecls.find(RecordDec);
        //   // if (It == MapRecordDeclsToTypedefDecls)
        //   auto It = RecordToRecordName.find(RecordDec);
        //   if (It == RecordToRecordName.end()) {
        //     emitErrorWithLocation("Could not find name of record!", &Ctx,
        //                           BO->getBeginLoc());
        //   }
        //   StructName = It->second;
        //   auto MemberName = LhsDecl->getDeclName();

        //   // UPDATE: Vidush: Releasing expresssions may be required in certain cases. 
        //   //Especially when you want to add calls to explode struct.
        //   SmallVector<PulseStmt *> ExprsBef;
        //   auto *PulseRhsTerm =
        //       getTermFromCExpr(Rhs, Analyzer, ExprsBef, Parent,BO->getType(), Module);
        //   PulseAssignment *Assignment;

        //   auto *DerefAppE = new AppE("!");

        //   auto *InnerTermCallArg = new VarTerm(NameOfDecl);
        //   DerefAppE->pushArg(InnerTermCallArg);

        //   // Wrap this deref in a parenthesis.
        //   auto *ParenthesisDeref = new Paren(DerefAppE);

        //   //Hack to check if the base type is a pointer. 
        //   //Leveraging the C syntax.
        //   bool BaseIsPointer = ME->isArrow() ? true : false;

        //   if (BaseIsPointer){
        //     auto *PulseCall =
        //         new AppE("Mk" + StructName + "?." + MemberName.getAsString());
        //     PulseCall->pushArg(ParenthesisDeref);
        //     Assignment = new PulseAssignment(PulseCall, PulseRhsTerm);
        //   }
        //   else {
        //     auto *NewProjection = new Project();
        //     NewProjection->BaseTerm = ParenthesisDeref;
        //     NewProjection->MemberName = MemberName.getAsString();
        //     Assignment = new PulseAssignment(NewProjection, PulseRhsTerm);
        //   }

        //   Assignment->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");

        //   // auto It = TrackStructExplodeAndRecover.find(VD);
        //   // if (It == TrackStructExplodeAndRecover.end()){
        //   //   auto *NewSeq = new PulseSequence();
        //   //   NewSeq->assignS2(Assignment);
        //   //   auto *ExplodeStmt = new GenericStmt();
        //   //   ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString() + ";";
        //   //   NewSeq->assignS1(ExplodeStmt);
        //   //   TrackStructExplodeAndRecover.insert(std::make_pair(VD, std::make_pair(true, false)));
        //   //   return NewSeq;
        //   // }

        //   auto *RetSeq = releaseExprs(ExprsBef);

        //   assert(ExprsBef.empty() && "Expected ExprsBefore to be empty!\n");

        //   // if (checkIfLastStructAccess(ME, CS, Ctx)) {

        //   //   auto It = TrackStructExplodeAndRecover.find(VD);
        //   //   auto ItElem = *It;
        //   //   auto &Decl = ItElem.first;
        //   //   auto &Info = ItElem.second;
        //   //   // recover not released.
        //   //   // In fact assert that a recover should not be released before.
        //   //   assert(!Info.second && "A recover was released for the struct when "
        //   //                          "there are accesses remaining!\n");
        //   //   if (auto *ParamD = dyn_cast<ParmVarDecl>(Decl)) {

        //   //     auto StructName =
        //   //         ParamD->getType()->getPointeeType().getAsString();

        //   //     auto *RecoverStatememt = new GenericStmt();
        //   //     RecoverStatememt->body =
        //   //         StructName + "_recover " + ParamD->getNameAsString() + ";";
        //   //     TrackStructExplodeAndRecover.erase(It);

        //   //     auto *NewSeq = new PulseSequence();
        //   //     NewSeq->assignS1(Assignment);
        //   //     NewSeq->assignS2(RecoverStatememt);
        //   //     if (RetSeq) {
        //   //       RetSeq->assignS2(NewSeq);
        //   //       return RetSeq;
        //   //     }

        //   //     return NewSeq;
        //   //   }
        //   // }

        //   if (RetSeq) {
        //     RetSeq->assignS2(Assignment);
        //     return RetSeq;
        //   }

        //   return Assignment;
        // }
        //else {

          //assert base expression to typedef type.
          ValueDecl *MemberDecl = ME->getMemberDecl();
          auto MemberCTy = MemberDecl->getType();
          auto *MemberTy = pulseTyFromDecl(MemberDecl);
          if (!MemberCTy->isArrayType()){
            auto *ToPulseRef = new FStarPointerType(MemberTy);
            MemberTy = ToPulseRef;
          }
          std::string StructName;
          if (FieldDecl *FieldDecl = llvm::dyn_cast<clang::FieldDecl>(MemberDecl)) {
            const RecordDecl *RecordDecl = FieldDecl->getParent();
            // Check if there is a typedef declaration existsing from the record
            // decl.
            auto It = RecordToRecordName.find(RecordDecl);
            if (It == RecordToRecordName.end()) {
              emitErrorWithLocation(
                  "Did not find a Name for Struct Declaration!", &Ctx,
                  ME->getBeginLoc());
            }

            StructName = It->second;
            llvm::outs() << "Print the structname: " << StructName << "\n";
            // make a map here to retieve the typedef if any avaiable.
          }

          SmallVector<PulseStmt*> ExprsBef;
          auto PulseBaseExprRet =
              getTermFromCExpr(BaseExpr, VEnv, Analyzer, ExprsBef, Parent,
                               BaseExpr->getType(), Module, false);
          auto *PulseBaseExpr = PulseBaseExprRet.first;

          auto PulseBaseExprTy = PulseBaseExpr->getType();
          if (!PulseBaseExprTy) {
            BaseExpr->dump();
            PulseBaseExpr->dumpPretty();

            if (auto *P = dyn_cast<Paren>(PulseBaseExpr)) {
              emitError("BaseTy was Project and its Type was nullptr!");
            }
            emitError("BaseTy was nullptr!");
          }
          if (ME->isLValue()){
            // TODO: Change type from nullptr to the correct type
            auto *PulseBaseExprPointerTy =
                dyn_cast<FStarPointerType>(PulseBaseExprTy);
            if (!PulseBaseExprPointerTy) {
              emitError("Expected type to be a ref but got: " +
                        getPulseTyAsString(PulseBaseExprTy));
            }
            auto *PulseBaseExprPointsTo = PulseBaseExprPointerTy->PointerTo;
            auto *App = new AppE("!", PulseBaseExprPointsTo);
            App->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");
            App->pushArg(PulseBaseExpr);
            // The paren should be the same type as App.
            auto *NewParen = new Paren(App, PulseBaseExprPointsTo);
            NewParen->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");
            PulseBaseExpr = NewParen;
          }

          auto MemberName = LhsDecl->getDeclName();

          auto PulseRhsTermRet =
              getTermFromCExpr(Rhs, PulseBaseExprRet.second, Analyzer, ExprsBef,
                               Parent, BO->getType(), Module);
          auto *PulseRhsTerm = PulseRhsTermRet.first;

          PulseAssignment *Assignment;

          //auto *DerefAppE = new AppE("!");

          //auto *InnerTermCallArg = new Paren(PulseBaseExpr);
          //DerefAppE->pushArg(InnerTermCallArg);

          // Wrap this deref in a parenthesis.
          //auto *ParenthesisDeref = new Paren(DerefAppE);

          //Hack to check if the base type is a pointer. 
          //Leveraging the C syntax.
          bool BaseIsPointer = ME->isArrow() ? true : false;
          if (BaseIsPointer){
            // TODO: Base expression should be a struct here.
            // Get the Pulse type of the struct.
            // auto *BaseExprTy = PulseBaseExpr->getType();
            auto *PulseCall = new AppE(
                "Mk" + StructName + "?." + MemberName.getAsString(), MemberTy);
            PulseCall->pushArg(PulseBaseExpr);
            //Vidush: This call handles compound ops like += etc.
            PulseRhsTerm = handleCompoundOpsHelper(Lhs, BO, PulseCall, PulseRhsTerm, Ctx);
            Assignment = new PulseAssignment(PulseCall, PulseRhsTerm);
            Assignment->CInfo = getSourceInfoFromExpr(BO, Ctx, "", "");
          }
          else {
            auto *NewProjection = new Project(MemberTy);
            NewProjection->BaseTerm = PulseBaseExpr;
            NewProjection->MemberName = MemberName.getAsString();
            PulseRhsTerm = handleCompoundOpsHelper(Lhs, BO, NewProjection, PulseRhsTerm, Ctx);
            Assignment = new PulseAssignment(NewProjection, PulseRhsTerm);
            Assignment->CInfo = getSourceInfoFromExpr(BO, Ctx, "", "");
          }

          Assignment->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");

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
            return std::make_pair(RetSeq, PulseRhsTermRet.second);
          }

          return std::make_pair(Assignment, PulseRhsTermRet.second);

          //}

          DEBUG_WITH_TYPE(DEBUG_TYPE, { ME->dump(); });
          emitErrorWithLocation(
              "Could not cast member base expression to its declaration!", &Ctx,
              ME->getBeginLoc());

      } else if (auto *ME = dyn_cast<MemberExpr>(Rhs)) {

        auto *RhsDecl = ME->getMemberDecl();
        /// TODO: Vidush, handle casts and check if it is safe to ignore them
        auto *BaseExpr = ME->getBase();
        DEBUG_WITH_TYPE(DEBUG_TYPE, {
        BaseExpr->dump();
        });

        auto *MemberDecl = ME->getMemberDecl();

        //std::string NameOfDecl;
        //QualType TyOfDecl;
        //std::string StructName;
        //if (const clang::DeclRefExpr *DRE =
        //        llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
          //const clang::ValueDecl *VD = DRE->getDecl();
          //NameOfDecl = VD->getDeclName().getAsString();
          //TyOfDecl = VD->getType();
          //StructName = TyOfDecl->getPointeeType().getAsString();

          auto MemberName = RhsDecl->getDeclName();
          auto MemberCTy = MemberDecl->getType();
          auto MemberTy = pulseTyFromDecl(MemberDecl);
          // We make the member's type a ref since we assume it in out struct
          // declarations
          if (!MemberCTy->isArrayType()){
            auto *ToPulseRef = new FStarPointerType(MemberTy);
            MemberTy = ToPulseRef;
          }

          //x->f translates to (!(!x).f)
          // auto *GenStmt = new Name("(!(!" + NameOfDecl + ")." +
          //                          MemberName.getAsString() + ")");

          auto *ProjectRhs = new Project(MemberTy);
          ProjectRhs->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");
          SmallVector<PulseStmt *> ExprsBef;
          ProjectRhs->MemberName = MemberName.getAsString();
          std::pair<Term *, PulseVisitor::VarTyEnv> PulseBaseTermRet;
          if (ME->isLValue()){
            PulseBaseTermRet =
                getTermFromCExpr(BaseExpr, VEnv, Analyzer, ExprsBef, Parent,
                                 BO->getType(), Module, true);
            auto *PulseBaseTerm = PulseBaseTermRet.first;
            auto *PulseBaseTermTy = PulseBaseTerm->getType();

            // TODO: get the underlying type of the call.
            // auto *BangCallTy =
            auto *BangCall = new AppE("!", PulseBaseTermTy);
            BangCall->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");
            BangCall->pushArg(PulseBaseTerm);
            // This should be the same as the type of the !
            auto *NewParen = new Paren(BangCall, PulseBaseTermTy);
            NewParen->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");
            ProjectRhs->BaseTerm = NewParen;
          }
          else{
            PulseBaseTermRet =
                getTermFromCExpr(BaseExpr, VEnv, Analyzer, ExprsBef, Parent,
                                 BO->getType(), Module, true);
            ProjectRhs->BaseTerm = PulseBaseTermRet.first;
          }

          auto PulseLhsTermRet =
              getTermFromCExpr(Lhs, PulseBaseTermRet.second, Analyzer, ExprsBef,
                               Parent, BO->getType(), Module, true);

          auto *PulseLhsTerm = PulseLhsTermRet.first;
          
          Term *RhsTerm = ProjectRhs;
          RhsTerm = handleCompoundOpsHelper(Lhs, BO, PulseLhsTerm, RhsTerm, Ctx);
          PulseAssignment *Assignment =
              new PulseAssignment(PulseLhsTerm, RhsTerm);

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

          Assignment->CInfo = getSourceInfoFromExpr(ME, Ctx, "", "");

          assert(ExprsBef.empty() && "Expected ExprsBefore to be empty!\n");

          return std::make_pair(Assignment, PulseLhsTermRet.second);
          //}

      }
      // TODO:
      // We should generate Lets otherwise
      else {

        // TODO: Make sure to release these expressions
        SmallVector<PulseStmt *> ExprsBef;
        auto PulseLhsTermRet = getTermFromCExpr(
            Lhs, VEnv, Analyzer, ExprsBef, Parent, BO->getType(), Module, true);
        auto *PulseLhsTerm = PulseLhsTermRet.first;

        auto PulseRhsTermRet =
            getTermFromCExpr(Rhs, PulseLhsTermRet.second, Analyzer, ExprsBef,
                             Parent, BO->getType(), Module);
        auto *PulseRhsTerm = PulseRhsTermRet.first;
        
        PulseRhsTerm = handleCompoundOpsHelper(Lhs, BO, PulseLhsTerm, PulseRhsTerm, Ctx);
        
        PulseAssignment *Assignment =
            new PulseAssignment(PulseLhsTerm, PulseRhsTerm);

        Assignment->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");

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
          return std::make_pair(Start, PulseRhsTermRet.second);
        }

        return std::make_pair(Assignment, PulseRhsTermRet.second);
      }
    } else {

      SmallVector<PulseStmt *> ExprsBefore;

      auto *PExpr = new PulseExpr();
      PExpr->CInfo = getSourceInfoFromExpr(BO, Ctx, "", "");

      auto ExprTermRet = getTermFromCExpr(BO, VEnv, Analyzer, ExprsBefore,
                                          Parent, BO->getType(), Module);
      auto ExprTerm = ExprTermRet.first;

      if (!ExprTerm)
        return std::make_pair(nullptr, ExprTermRet.second);

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
        return std::make_pair(Start, ExprTermRet.second);
      }

      assert(ExprsBefore.empty() && "Expected expressions to be released!");
      return std::make_pair(PExpr, ExprTermRet.second);
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
    PulseExpression->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    auto PExprTermRet = getTermFromCExpr(E, VEnv, Analyzer, ExprsBefore, Parent,
                                         E->getType(), Module);
    auto *PExprTerm = PExprTermRet.first;

    if (PExprTerm == nullptr)
      return std::make_pair(nullptr, PExprTermRet.second);

    PulseExpression->E = PExprTerm;

    auto *NewSeq = releaseExprs(ExprsBefore);

    if (NewSeq) {
      NewSeq->assignS2(PulseExpression);
      return std::make_pair(NewSeq, PExprTermRet.second);
    }

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return std::make_pair(PulseExpression, PExprTermRet.second);
  } else if (auto *IF = dyn_cast<IfStmt>(S)) {

    auto *PulseIfStmt = new PulseIf();
    PulseIfStmt->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");

    auto *Cond = IF->getCond();
    auto *Then = IF->getThen();
    auto *Else = IF->getElse();

    SmallVector<PulseStmt *> ExprsBefore;

    auto PulseCondRet = getTermFromCExpr(Cond, VEnv, Analyzer, ExprsBefore,
                                         Parent, Cond->getType(), Module);
    auto *PulseCond = PulseCondRet.first;

    if (Cond->getType().getAsString() != "_Bool" && Cond->getType().getAsString() != "bool"){

      auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
      auto *CastCall =
          new AppE(getPulseStringForCType(Cond->getType(), Ctx) + "_to_bool",
                   PulseBoolTy);
      CastCall->CInfo = getSourceInfoFromExpr(Cond, Ctx, "", "");
      CastCall->pushArg(PulseCond);
      auto *NewParen = new Paren(CastCall, PulseBoolTy);
      NewParen->CInfo = getSourceInfoFromExpr(Cond, Ctx, "", "");
      PulseCond = NewParen;
    }                                   

    PulseStmt *PulseThen;
    std::pair<PulseStmt *, PulseVisitor::VarTyEnv> PulseThenRet;
    if (auto *AttrStmt = dyn_cast<AttributedStmt>(Then)) {
      auto *ThenBody = AttrStmt->getSubStmt();

      auto Attributes = AttrStmt->getAttrs();
      // auto *PulseWhile = new PulseWhileStmt();
      for (auto *Attr : Attributes) {

        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
          if (AnnAttr->getAnnotation() == "ensures") {
            auto *NewEnsures = new Ensures();
            NewEnsures->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
            NewEnsures->Ann2 = getAnnotationCode(AnnAttr, Attr->getLocation());
            PulseIfStmt->IfLemmas.push_back(NewEnsures);
          }
        }
      }

      PulseThenRet = pulseFromStmt(ThenBody, PulseCondRet.second, Analyzer,
                                   Parent, Module, CS);
      PulseThen = PulseThenRet.first;
    } else {
      PulseThenRet = pulseFromStmt(Then, VEnv, Analyzer, Parent, Module, CS);
      PulseThen = PulseThenRet.first;
    }

    auto PulseElseCond =
        pulseFromStmt(Else, VEnv, Analyzer, Parent, Module, CS);
    auto *PulseElse = PulseElseCond.first;

    PulseIfStmt->Head = PulseCond;
    PulseIfStmt->Else = PulseElse;
    PulseIfStmt->Then = PulseThen;

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    // Whatever is defined inside the If, the scope stays local.
    return std::make_pair(PulseIfStmt, VEnv);
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {

    if (auto *RetVal = RS->getRetValue()) {

      // if (checkIfExprIsNullPtr(RetVal)){


      // }

       SmallVector<PulseStmt *> ExprsBefore;
      // if (auto *CastToStmt = dyn_cast<Stmt>(RS->getRetValue())){
       auto RetStmtCond = getTermFromCExpr(RetVal, VEnv, Analyzer, ExprsBefore,
                                           Parent, RetVal->getType(), Module);
       auto *RetStmt = RetStmtCond.first;
       auto *RetExpr = new PulseExpr();
       RetExpr->CInfo = getSourceInfoFromStmt(RS, Ctx, "", "");
       RetExpr->E = RetStmt;

       // Overide location of RetStmt
       // Vidush: removed the override based on meeting from 7.25.2025
       // RetStmt->CInfo = RetExpr->CInfo;

       return std::make_pair(RetExpr, RetStmtCond.second);
       // return RetStmt;
       //}
       // else{

       // // Key settings to preserve non-canonical names
       // PrintingPolicy Policy(Ctx.getLangOpts());
       // Policy.SuppressTagKeyword = false;
       // Policy.SuppressUnwrittenScope = false;
       // Policy.PrintAsCanonical = false;
       // Policy.FullyQualifiedName = false;

       // llvm::outs() << "Print the Return Stmt!" <<
       // RetVal->getType().getAsString(Policy) << "\n"; RS->dump();
       // llvm::outs() << "End of printing return stmt!\n";
       // RetVal->dump();
       // llvm::outs() << "End of printing return value!\n";

       //   if (auto *IC = dyn_cast<CastExpr>(RetVal)){
       //     auto* SubExpr = IC->getSubExpr()->IgnoreParens();
       //     QualType sourceType = IC->getSubExpr()->getType();
       //     QualType destType = IC->getType();
       //     if (sourceType.getAsString() == destType.getAsString()) {
       //             goto ret_val_bo_general;
       //     }
       //     //Handle BO differently
       //     if (checkIfExprIsBoolTy(SubExpr)){

       //       //No need for a bool to bool cast.
       //       if (IC->getType().getAsString() == "_Bool" ||
       //       IC->getType().getAsString() == "bool"){
       //           goto ret_val_bo_general;
       //       }
       //       ///make a call expr
       //       auto *CastCall = new AppE("bool_to_" +
       //       getPulseStringForCType(IC->getType(), Ctx));

       //       SmallVector<PulseStmt *> ExprsBefore;
       //       auto *RetTerm = getTermFromCExpr(BO, Analyzer, ExprsBefore,
       //       Parent,
       //                                    RetVal->getType(), Module);
       //       CastCall->pushArg(RetTerm);
       //       auto *NewParen = new Paren(CastCall);
       //       auto *NewPulseExpr = new PulseExpr();
       //       NewPulseExpr->E = NewParen;
       //       return NewPulseExpr;

       //     }
       //     else if (auto *IL = dyn_cast<IntegerLiteral>(SubExpr)){

       //     auto *CastCall = new AppE(getPulseStringForCType(IL->getType(),
       //     Ctx) + "_to_" + getPulseStringForCType(IC->getType(), Ctx));

       //     SmallVector<PulseStmt *> ExprsBefore;
       //     auto *RetTerm = new ConstTerm();
       //     RetTerm->ConstantValue =
       //     std::to_string(IL->getValue().getSExtValue()); RetTerm->Symbol =
       //     getSymbolKeyForCType(IL->getType(), Ctx);
       //     CastCall->pushArg(RetTerm);
       //     auto *NewParen = new Paren(CastCall);
       //     auto *NewPulseExpr = new PulseExpr();
       //     NewPulseExpr->E = NewParen;
       //     return NewPulseExpr;

       //     }
       //     RS->dump();
       //     ///Vidush: TODO We should handle these other cases where implicit
       //     casts exist.
       //     ////TODO: These might need library implementation in Pulse
       //     auto *CastCall = new
       //     AppE(getPulseStringForCType(SubExpr->getType(), Ctx) + "_to_" +
       //     getPulseStringForCType(IC->getType(), Ctx));

       //     SmallVector<PulseStmt *> ExprsBefore;
       //     auto *RetTerm = getTermFromCExpr(SubExpr, Analyzer, ExprsBefore,
       //     Parent,
       //                                    RetVal->getType(), Module);
       //     CastCall->pushArg(RetTerm);
       //     auto *NewParen = new Paren(CastCall);
       //     auto *NewPulseExpr = new PulseExpr();
       //     NewPulseExpr->E = NewParen;
       //     return NewPulseExpr;
       //   }
       //   else{
       //     ret_val_bo_general:
       //     SmallVector<PulseStmt *> ExprsBefore;
       //     auto *RetTerm = getTermFromCExpr(RetVal, Analyzer, ExprsBefore,
       //     Parent,
       //                                    RetVal->getType(), Module);

       //     if (RetTerm == nullptr)
       //       return nullptr;

       //     auto *NewPulseExpr = new PulseExpr();
       //     NewPulseExpr->E = RetTerm;

       //     assert(ExprsBefore.empty() && "Expected expressions to be
       //     released!"); return NewPulseExpr;
       //   }
       //   //}
       //   //   if (auto *DeclRef =
       //   //
       //   dyn_cast<DeclRefExpr>(RetVal->IgnoreParenImpCasts()->IgnoreImpCasts())){
       //   //     auto It =
       //   TrackStructExplodeAndRecover.find(DeclRef->getDecl());
       //   //     if (It != TrackStructExplodeAndRecover.end()){
       //   //       auto Info = It->second;
       //   //       if (!Info.second){

       //   //         //Get struct name from declration.
       //   //           const auto *VD = DeclRef->getDecl();
       //   //           auto *PSeq = new PulseSequence();
       //   //           auto StructName =
       //   //           VD->getType()->getPointeeType().getAsString(); if
       //   (RetTerm){
       //   //              NewPulseExpr->E = RetTerm;
       //   //              PSeq->assignS2(NewPulseExpr);
       //   //              auto *FallBack  = new GenericStmt();
       //   //              FallBack->body += StructName + "_recover " +
       //   //              DeclRef->getDecl()->getNameAsString() + ";";
       //   //              PSeq->assignS1(FallBack);
       //   //              // update element in map.
       //   //              TrackStructExplodeAndRecover.erase(It);
       //   //              return PSeq;
       //   //          }

       //   //         auto *FallBack  = new GenericStmt();
       //   //         FallBack->body += StructName + "_recover " +
       //   //         DeclRef->getDecl()->getNameAsString() + ";";

       //   //         // update element in map.
       //   //         TrackStructExplodeAndRecover.erase(It);
       //   //         return FallBack;
       //   //     }
       //   //   }
       //   // }
   }

    //Return a unit / void type
    auto *RetExpr = new PulseExpr();
    RetExpr->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");
    // TODO: Return type is nullptr for now but could be better for unit type.
    auto *Unit = new Name("()", nullptr);
    Unit->CInfo = getSourceInfoFromStmt(RS, Ctx, "", "");
    RetExpr->E = Unit;
    return std::make_pair(RetExpr, VEnv);
  } else if (auto *FS = dyn_cast<ForStmt>(S)) {
    S->dumpPretty(Ctx);
    emitErrorWithLocation("For loops not implemented since pulse does not "
                          "support for expressions",
                          &Ctx, FS->getBeginLoc());
  } else if (auto *WS = dyn_cast<WhileStmt>(S)) {

    auto *WhileCond = WS->getCond();
    auto *WhileBody = WS->getBody();

    if (auto *AttrStmt = dyn_cast<AttributedStmt>(WhileBody)) {
      auto *CompundBody = AttrStmt->getSubStmt();

      auto Attributes = AttrStmt->getAttrs();
      auto *PulseWhile = new PulseWhileStmt();
      PulseWhile->CInfo = getSourceInfoFromStmt(S, Ctx, "", "");
      for (auto *Attr : Attributes) {

        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
          if (AnnAttr->getAnnotation() == "invariant") {
            PulseWhile->Invariant = getAnnotationCode(AnnAttr, AttrStmt->getAttrLoc());
          }
        }
      }
      
      SmallVector<PulseStmt*> ExprsBef;
      auto PulseWhileGuardRet =
          getTermFromCExpr(WhileCond, VEnv, Analyzer, ExprsBef, Parent,
                           WhileCond->getType(), Module, CS);
      auto *PulseWhileGuard = PulseWhileGuardRet.first;

      auto *NewExprGuard = new PulseExpr();
      NewExprGuard->CInfo = getSourceInfoFromExpr(WhileCond, Ctx, "", "");
      if (WhileCond->getType().getAsString() != "_Bool" && WhileCond->getType().getAsString() != "bool"){
        auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
        auto *CastCall = new AppE(
            getPulseStringForCType(WhileCond->getType(), Ctx) + "_to_bool",
            PulseBoolTy);
        CastCall->CInfo = getSourceInfoFromExpr(WhileCond, Ctx, "", "");
        CastCall->pushArg(PulseWhileGuard);
        auto *NewParenCastCall = new Paren(CastCall, PulseBoolTy);
        NewExprGuard->E = NewParenCastCall; 
      }
      else {
        NewExprGuard->E = PulseWhileGuard;
      }
      PulseWhile->Guard = NewExprGuard;
      auto NewVEnv = PulseWhileGuardRet.second;
      PulseWhile->Body =
          pulseFromCompoundStmt(CompundBody, NewVEnv, Analyzer, Module);

      // Whatever, is inside the While loop is in local scope.
      // So we can return the VEnv directly.
      return std::make_pair(PulseWhile, VEnv);
    } else {

      auto *PulseWhile = new PulseWhileStmt();
      PulseWhile->CInfo = getSourceInfoFromStmt(WS, Ctx, "", "");

      auto PulseWhileGuardRet =
          pulseFromStmt(WhileCond, VEnv, Analyzer, Parent, Module, CS);
      PulseWhile->Guard = PulseWhileGuardRet.first;

      PulseWhile->Body = pulseFromCompoundStmt(
          WhileBody, PulseWhileGuardRet.second, Analyzer, Module);

      // Can just return the current vars in the env.
      return std::make_pair(PulseWhile, VEnv);
    }
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
    S->dumpPretty(Ctx);
    emitErrorWithLocation(
        "Did not implement translation from C unary expression to PulseStmt!",
        &Ctx, US->getBeginLoc());
  } else if (auto *NS = dyn_cast<NullStmt>(S)) {
    return std::make_pair(nullptr, VEnv);
  } else if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    // We assume that whatever is defined in the compound statement is in local
    // scope.
    auto *CompoundSeq = pulseFromCompoundStmt(CS, VEnv, Analyzer, Module);
    return std::make_pair(CompoundSeq, VEnv);
  } else if (auto *AttrStmt = dyn_cast<AttributedStmt>(S)) {

    auto *SubStmt = AttrStmt->getSubStmt();
    PulseSequence *NewSequence = nullptr;

    auto Attrs = AttrStmt->getAttrs();
    assert(Attrs.size() == 1 && "Did not handle multiple attrs in an attributed stmt.\n");
    for (auto *Attr : Attrs) {
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)) {
        if (isPulseAnnotAttr(AnnotAttr)) {
          std::string Match;
          auto AttrKind =
              getPulseAnnKindFromString(AnnotAttr->getAnnotation(), Match);
            if (AttrKind == PulseAnnKind::LemmaStatement){
              auto *LS = new LemmaStatement();
              LS->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
              LS->Lemma = Match;
              auto *PE = new PulseExpr();
              PE->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
              PE->E = LS;
              return std::make_pair(PE, VEnv);
            }
            else if (AttrKind == PulseAnnKind::Assert){
              auto *GenStmt = new GenericStmt();
              GenStmt->CInfo = getSourceInfoFromAttr(Attr, Ctx, "");
              GenStmt->body = "assert(" + Match + ");";
              return std::make_pair(GenStmt, VEnv);
            }
            else {
              emitErrorWithLocation("Unhandled Attr in Attributed Stmt!", &Ctx,
                                    AttrStmt->getAttrLoc());
            }
        }
      }
    }

    auto PulseSubStmtRet =
        pulseFromStmt(SubStmt, VEnv, Analyzer, Parent, Module, CS);
    auto *PulseSubStmt = PulseSubStmtRet.first;
    NewSequence->assignS2(PulseSubStmt);
    return std::make_pair(NewSequence, PulseSubStmtRet.second);
  } else {
    S->dumpPretty(Ctx);
    emitErrorWithLocation("Not implemented Clang expr!", &Ctx,
                          S->getBeginLoc());
  }

  return std::make_pair(nullptr, VEnv);
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

std::pair<Term *, PulseVisitor::VarTyEnv> PulseVisitor::getTermFromCExpr(
    Expr *E, PulseVisitor::VarTyEnv VEnv, ExprMutationAnalyzer *MutAnalyzer,
    llvm::SmallVector<PulseStmt *> &ExprsBefore, Stmt *Parent,
    QualType ParentType, PulseModul *Module, bool isWrite) {

  if (auto *IL = dyn_cast<IntegerLiteral>(E)) {

    auto *PulseTy = getPulseTyFromCTy(IL->getType());
    auto ConstantValue = std::to_string(IL->getValue().getSExtValue());
    auto *NewConstTerm = new ConstTerm(ConstantValue, PulseTy);
    NewConstTerm->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    DEBUG_WITH_TYPE(DEBUG_TYPE , {
    llvm::outs() << "Found Integer Literal: " << NewConstTerm->ConstantValue
                 << "\n";
    llvm::outs() << ParentType.getAsString() << "\n";
    });

    NewConstTerm->Symbol = getSymbolKeyForCType(IL->getType(), Ctx);

    return std::make_pair(NewConstTerm, VEnv);
  } else if (auto *FL = dyn_cast<FloatingLiteral>(E)) {
    E->dumpPretty(Ctx);
    emitErrorWithLocation("No support for floats in Pulse!", &Ctx,
                          E->getExprLoc());
  } else if (auto *SL = dyn_cast<StringLiteral>(E)) {
    E->dumpPretty(Ctx);
    emitErrorWithLocation("String Literal not implemented!", &Ctx,
                          E->getExprLoc());
  } else if (auto *CL = dyn_cast<CharacterLiteral>(E)) {
    
    // CL->dump();
    // llvm::outs() << "break\n";
    // CL->getType().dump();
    // exit(0);
    // auto *NewCharConstant = new ConstTerm();
    // NewCharConstant->CInfo = getSourceInfoFromExpr(CL, Ctx, "", "");
    // auto CharVal = CL->getValue();
    // char CVal = static_cast<char>(CharVal);
    // NewCharConstant->ConstantValue = "'" + std::string(1, CVal) + "'";
    // NewCharConstant->Symbol = getSymbolKeyForCType(CL->getType(), Ctx);
    // return NewCharConstant;

    E->dumpPretty(Ctx);
    emitErrorWithLocation("Char Literal not implemented!", &Ctx,
                          E->getExprLoc());

  } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {

    auto *Lhs = BO->getLHS();
    auto *Rhs = BO->getRHS();
    auto Op = BO->getOpcode();
    
    //Remove this check for now since we have cast operations!
    // if (Lhs->getType() != Rhs->getType()) {
    //   E->dumpPretty(Ctx);
    //   LLVM_DEBUG(llvm::dbgs() << "\n");
    //   emitErrorWithLocation("Expected types of Lhs and Rhs to be the same, "
    //                         "unsafe type casting now allowed in pulse!",
    //                         &Ctx, E->getExprLoc());
    // }

    switch (Op) {
    case clang::BO_EQ: {

      // Check if either Lhs or Rhs is NULL.
      // In case it is NULL we would like to generate is_null checks for pulse.
      if (checkIfExprIsNullPtr(Lhs)) {

        // Return Type of is_null should be a Bool expression.
        auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
        auto *IsNullCall = new AppE("is_null", PulseBoolTy);
        IsNullCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        auto RhsTermRet = getTermFromCExpr(Rhs, VEnv, MutAnalyzer, ExprsBefore,
                                           Parent, BO->getType(), Module);
        auto *RhsTerm = RhsTermRet.first;
        IsNullCall->pushArg(RhsTerm);
        if (BO->isKnownToHaveBooleanValue()){
          // Here we are adding bool_to_return_ty_of_binary expression casts
          auto *PulseTy = getPulseTyFromCTy(BO->getType());
          auto *CastCall = new AppE(
              "bool_to_" + getPulseStringForCType(BO->getType(), Ctx), PulseTy);
          CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          auto *ParenAroundNullCall = new Paren(IsNullCall, PulseBoolTy);
          CastCall->pushArg(ParenAroundNullCall);
          auto *NewParen = new Paren(CastCall, PulseTy);
          NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          return std::make_pair(NewParen, RhsTermRet.second);
        }

        return std::make_pair(IsNullCall, RhsTermRet.second);
      }

      if (checkIfExprIsNullPtr(Rhs)) {

        // Vidush: Return type of is_null should be a Boolean.
        auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
        auto *IsNullCall = new AppE("is_null", PulseBoolTy);
        IsNullCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        auto LhsRetTerm = getTermFromCExpr(Lhs, VEnv, MutAnalyzer, ExprsBefore,
                                           Parent, BO->getType(), Module);
        auto *LhsTerm = LhsRetTerm.first;
        IsNullCall->pushArg(LhsTerm);
        if (BO->isKnownToHaveBooleanValue()){
          auto *CastCallTy = getPulseTyFromCTy(BO->getType());
          auto *CastCall =
              new AppE("bool_to_" + getPulseStringForCType(BO->getType(), Ctx),
                       CastCallTy);
          CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          auto *ParenAroundNullCall = new Paren(IsNullCall, PulseBoolTy);
          CastCall->pushArg(ParenAroundNullCall);
          auto *NewParen = new Paren(CastCall, CastCallTy);
          NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          return std::make_pair(NewParen, LhsRetTerm.second);
        }

        return std::make_pair(IsNullCall, LhsRetTerm.second);
      }

      goto default_bin_op_case;
    }
    case clang::BO_NE: {

      // Check if either Lhs or Rhs is NULL.
      // In case it is NULL we would like to generate is_null checks for pulse.
      if (checkIfExprIsNullPtr(Lhs)) {
       
        //Generate null call
        auto *BooleanTy = new FStarType(lookupSymbol(SymbolTable::Bool));
        auto *IsNullCall = new AppE("is_null", BooleanTy);
        IsNullCall->CInfo = getSourceInfoFromExpr(Lhs, Ctx, "", "");
        auto RhsTermRet = getTermFromCExpr(Rhs, VEnv, MutAnalyzer, ExprsBefore,
                                           Parent, BO->getType(), Module);
        auto *RhsTerm = RhsTermRet.first;
        IsNullCall->pushArg(RhsTerm);

        // Generate not call
        // Vidush: Should I copy the type instead of assigning the same
        // pointer?? It might be better to assign a new pointer
        auto *NotCall = new AppE("not", BooleanTy);
        NotCall->CInfo = getSourceInfoFromExpr(Lhs, Ctx, "", "");

        //Wrap null call around parenthesis
        auto *ParenNullCall = new Paren(IsNullCall, BooleanTy);
        ParenNullCall->CInfo = getSourceInfoFromExpr(Lhs, Ctx, "", "");
        NotCall->pushArg(ParenNullCall);

        if (BO->isKnownToHaveBooleanValue()){
          auto *CastCallTy = getPulseTyFromCTy(BO->getType());
          auto *CastCall =
              new AppE("bool_to_" + getPulseStringForCType(BO->getType(), Ctx),
                       CastCallTy);
          CastCall->CInfo = getSourceInfoFromExpr(Lhs, Ctx, "", "");
          auto *ParenAroundNotCall = new Paren(NotCall, BooleanTy);
          CastCall->pushArg(ParenAroundNotCall);
          auto *NewParen = new Paren(CastCall, CastCallTy);
          NewParen->CInfo = getSourceInfoFromExpr(Lhs, Ctx, "", "");
          return std::make_pair(NewParen, RhsTermRet.second);
        }

        return std::make_pair(NotCall, RhsTermRet.second);
      }

      if (checkIfExprIsNullPtr(Rhs)) {

        // Generate null call
        // is_null has return type bool
        auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
        auto *IsNullCall = new AppE("is_null", PulseBoolTy);
        IsNullCall->CInfo = getSourceInfoFromExpr(Rhs, Ctx, "", "");
        auto LhsTermRet = getTermFromCExpr(Lhs, VEnv, MutAnalyzer, ExprsBefore,
                                           Parent, BO->getType(), Module);
        auto *LhsTerm = LhsTermRet.first;
        IsNullCall->pushArg(LhsTerm);
        
        //Generate not call
        auto *NotCall = new AppE("not", PulseBoolTy);
        NotCall->CInfo = getSourceInfoFromExpr(Rhs, Ctx, "", "");

        //Wrap null call around parenthesis
        auto *ParenNullCall = new Paren(IsNullCall, PulseBoolTy);
        ParenNullCall->CInfo = getSourceInfoFromExpr(Rhs, Ctx, "", "");
        NotCall->pushArg(ParenNullCall);

        if (BO->isKnownToHaveBooleanValue()){
          auto *CastCallTy = getPulseTyFromCTy(BO->getType());
          auto *CastCall =
              new AppE("bool_to_" + getPulseStringForCType(BO->getType(), Ctx),
                       CastCallTy);
          CastCall->CInfo = getSourceInfoFromExpr(Rhs, Ctx, "", "");
          auto *ParenAroundNotCall = new Paren(NotCall, PulseBoolTy);
          CastCall->pushArg(ParenAroundNotCall);
          auto *NewParen = new Paren(CastCall, CastCallTy);
          NewParen->CInfo = getSourceInfoFromExpr(Rhs, Ctx, "", "");
          return std::make_pair(NewParen, LhsTermRet.second);
        }
        NotCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        return std::make_pair(NotCall, LhsTermRet.second);
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
                              &Ctx, BO->getExprLoc());
      }
      
      //Vidush: This is a HACK, there should be a better approach rather than ignoring 
      //casts to get the type of the operator
      //Ignore casts to get underlying type!

     // auto *RemoveCastsLhs = Lhs->IgnoreParens()->IgnoreCasts()->IgnoreImpCasts();
      SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
      //SymbolTable RetTy = getSymbolKeyForCType(BO->getType(), Ctx);
      auto *OpKey = getSymbolKeyForOperator(TypeKey, Op);

      switch (Op){
        case BO_NE:{
          // In Pulse we just blindly assume that all binary operators have a
          // return type of the whole expression as a bool
          auto *BoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
          auto *NewAppENode = new AppE(OpKey, BoolTy);
          NewAppENode->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

          auto LhsTermRet =
              getTermFromCExpr(Lhs, VEnv, MutAnalyzer, ExprsBefore, Parent,
                               Lhs->getType(), Module);
          auto *LhsTerm = LhsTermRet.first;

          auto RhsTermRet =
              getTermFromCExpr(Rhs, LhsTermRet.second, MutAnalyzer, ExprsBefore,
                               Parent, Rhs->getType(), Module);
          auto *RhsTerm = RhsTermRet.first;
          NewAppENode->pushArg(LhsTerm);
          NewAppENode->pushArg(RhsTerm);
          
          // Wrap Call Expr into a Paren to be safe.
          auto *NewParen = new Paren(NewAppENode, BoolTy);
          NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

          // Add not
          auto *NotAppE = new AppE("not", BoolTy);
          NotAppE->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          NotAppE->pushArg(NewParen);

          auto *NotAppEParen = new Paren(NotAppE, BoolTy);
          NotAppEParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

          if (BO->isKnownToHaveBooleanValue()){
            auto *PulseBOTy = getPulseTyFromCTy(BO->getType());
            auto *CastCall = new AppE(
                "bool_to_" + getPulseStringForCType(BO->getType(), Ctx),
                PulseBOTy);
            CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            CastCall->pushArg(NotAppEParen);
            auto *NewParen = new Paren(CastCall, PulseBOTy);
            NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewParen, RhsTermRet.second);
          }

          return std::make_pair(NotAppEParen, RhsTermRet.second);
          break;
        }
        default: {
          // Assuming that binary operators in Pulse have a boolean return
          // value.
          auto *PulseTyOfBOExpr = getPulseTyFromCTy(Lhs->getType());
          auto *NewAppENode = new AppE(OpKey, PulseTyOfBOExpr);
          NewAppENode->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

          auto LhsTermRet =
              getTermFromCExpr(Lhs, VEnv, MutAnalyzer, ExprsBefore, Parent,
                               Lhs->getType(), Module);
          auto *LhsTerm = LhsTermRet.first;
          auto RhsTermRet =
              getTermFromCExpr(Rhs, LhsTermRet.second, MutAnalyzer, ExprsBefore,
                               Parent, Rhs->getType(), Module);

          auto *RhsTerm = RhsTermRet.first;

          // For logical ops make sure type of argument is bool if not cast them
          if (BO->isLogicalOp()) {
            auto *BoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
            PulseTyOfBOExpr = BoolTy;
            if (Lhs->getType().getAsString() != "_Bool" &&
                Lhs->getType().getAsString() != "bool") {
              auto *CastCall = new AppE(
                  getPulseStringForCType(Lhs->getType(), Ctx) + "_to_bool",
                  BoolTy);
              CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
              CastCall->pushArg(LhsTerm);
              auto *ParenAroundCastCall = new Paren(CastCall, BoolTy);
              NewAppENode->pushArg(ParenAroundCastCall);
            }
            if (Rhs->getType().getAsString() != "_Bool" &&
                Rhs->getType().getAsString() != "bool") {
              auto *CastCall = new AppE(
                  getPulseStringForCType(Rhs->getType(), Ctx) + "_to_bool",
                  BoolTy);
              CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
              CastCall->pushArg(RhsTerm);
              auto *ParenAroundCastCall = new Paren(CastCall, BoolTy);
              NewAppENode->pushArg(ParenAroundCastCall);
            }
          } else {
            NewAppENode->pushArg(LhsTerm);

            //If rhs is not same type as Lhs, we need to cast it.
            if (Lhs->getType().getAsString() != Rhs->getType().getAsString()){
              auto *LhsType = getPulseTyFromCTy(Lhs->getType());
              auto *CastCall = new AppE(
                  getPulseStringForCType(Rhs->getType(), Ctx) + "_to_" +
                      getPulseStringForCType(Lhs->getType(), Ctx),
                  LhsType);
              CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
              CastCall->pushArg(RhsTerm);
              auto *NewParen = new Paren(CastCall, LhsType);
              NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
              RhsTerm = NewParen;

            }
            NewAppENode->pushArg(RhsTerm);
          }

          // Wrap Call Expr into a Paren to be safe.
          auto *NewParen = new Paren(NewAppENode, PulseTyOfBOExpr);
          NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          //In case its know to have a boolean return value generate
          //the right kind of op here.
          if (BO->isKnownToHaveBooleanValue()){
            auto *BOPulseTy = getPulseTyFromCTy(BO->getType());
            auto *CastCall = new AppE(
                "bool_to_" + getPulseStringForCType(BO->getType(), Ctx),
                BOPulseTy);
            CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            CastCall->pushArg(NewParen);
            auto *NewParenRet = new Paren(CastCall, BOPulseTy);
            NewParenRet->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewParenRet, RhsTermRet.second);
          }

          return std::make_pair(NewParen, RhsTermRet.second);
          break;
        };
      };
      break;
    }
    }

  } else if (auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_Deref) {
      // auto *DerefAppE = new AppE("!");

      auto TermForBaseExprRet =
          getTermFromCExpr(UO->getSubExpr(), VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      auto *TermForBaseExpr = TermForBaseExprRet.first;
      // DerefAppE->pushArg(TermForBaseExpr);

      // // Wrap this deref in a parenthesis.
      // auto *Parenthesis = new Paren(DerefAppE);
      // Parenthesis->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
      // return Parenthesis;

      return std::make_pair(TermForBaseExpr, TermForBaseExprRet.second);

    } 
    else if (UO->getOpcode() == UO_AddrOf){

      auto SubExpr = UO->getSubExpr();
      if (auto *Mem = dyn_cast<MemberExpr>(SubExpr)) {

        auto *BaseExpr = Mem->getBase();
        auto isLVal = Mem->isLValue();
        // TODO: Vidush, I am ignoring casts here.
        // But the ! is added in GenStmt which comes because of the imp cast.
        //if (auto *Dec = dyn_cast<DeclRefExpr>(
        //        BaseExpr->IgnoreParenImpCasts()->IgnoreImpCasts())) {
        //  auto *VD = Dec->getDecl();
        //  auto VDTy = VD->getType();
        //  auto StructName = VDTy->getPointeeType().getAsString();

          //auto *GenStmt =
          //    new Name("(!" + Dec->getDecl()->getNameAsString() + ")." +
          //             Mem->getMemberDecl()->getDeclName().getAsString());
        auto MemberDecl = Mem->getMemberDecl();
        auto MemberCTy = MemberDecl->getType();
        auto MemberTy = pulseTyFromDecl(MemberDecl);

        if (!MemberCTy->isArrayType()){
          auto *ToPulseRef = new FStarPointerType(MemberTy);
          MemberTy = ToPulseRef;
        }

        auto *NewProject = new Project(MemberTy);
        NewProject->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        NewProject->MemberName =
            Mem->getMemberDecl()->getDeclName().getAsString();

        auto BaseTermRet =
            getTermFromCExpr(BaseExpr, VEnv, MutAnalyzer, ExprsBefore, Parent,
                             ParentType, Module);
        auto *BaseTerm = BaseTermRet.first;

        auto *BaseTermTy = BaseTerm->getType();

        if (isLVal) {

          auto *BaseTermTyPointer = dyn_cast<FStarPointerType>(BaseTermTy);
          if (!BaseTermTyPointer) {
            emitError("Expected base type to be a pulse ref!\n");
          }
          auto *BaseTermTyPointsTo = BaseTermTyPointer->PointerTo;
          // TODO: We should probably have copies of the type being assigned
          // rather than a pointer?? In the pulse AST we can change to a Value
          // rather than a pointer.
          auto *App = new AppE("!", BaseTermTyPointsTo);
          App->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          App->pushArg(BaseTerm);
          auto *NewParen = new Paren(App, BaseTermTyPointsTo);
          NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
          NewProject->BaseTerm = NewParen;
        } else {
          NewProject->BaseTerm = BaseTerm;
        }

          // auto It = TrackStructExplodeAndRecover.find(VD);
          // if (It == TrackStructExplodeAndRecover.end()) {
          //   auto *ExplodeStmt = new GenericStmt();
          //   ExplodeStmt->body =
          //       StructName + "_explode " + VD->getNameAsString() + ";";
          //   ExprsBefore.push_back(ExplodeStmt);
          //   TrackStructExplodeAndRecover.insert(
          //       std::make_pair(VD, std::make_pair(true, false)));
          // }

        return std::make_pair(NewProject, BaseTermRet.second);
        //}
      }
      else {

        /// Expecting Decl ref to add ! since it will consider addof as yes to
        /// the variable being mutated.
        auto TermForBaseExprRet =
            getTermFromCExpr(UO->getSubExpr(), VEnv, MutAnalyzer, ExprsBefore,
                             Parent, ParentType, Module);

        auto *TermForBaseExpr = TermForBaseExprRet.first;
        // Wrap address of in a parenthesis.
        auto *Parenthesis =
            new Paren(TermForBaseExpr, TermForBaseExpr->getType());
        Parenthesis->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        return std::make_pair(Parenthesis, TermForBaseExprRet.second);
      }
    } 
    else if (UO->getOpcode() == clang::UO_LNot){

      auto *PulseBoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
      auto *NotCall = new AppE("not", PulseBoolTy);
      NotCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

      auto *SubExpr = UO->getSubExpr();

      auto PulseSubExprRet = getTermFromCExpr(
          SubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent, ParentType, Module);
      auto *PulseSubExpr = PulseSubExprRet.first;

      if (SubExpr->getType().getAsString() != "_Bool" && SubExpr->getType().getAsString() != "bool"){
        auto *CastCall = new AppE(
            getPulseStringForCType(SubExpr->getType(), Ctx) + "_to_bool",
            PulseBoolTy);
        CastCall->CInfo = getSourceInfoFromExpr(SubExpr, Ctx, "", "");
        CastCall->pushArg(PulseSubExpr);
        auto *NewParen = new Paren(CastCall, PulseBoolTy);
        NewParen->CInfo = getSourceInfoFromExpr(SubExpr, Ctx, "", "");
        PulseSubExpr = NewParen;
      }

      NotCall->pushArg(PulseSubExpr);

      if (UO->isKnownToHaveBooleanValue()){
        auto UOTy = getPulseTyFromCTy(UO->getType());
        auto *CastCall =
            new AppE("bool_to_" + getPulseStringForCType(UO->getType(), Ctx),
                     UOTy);
        CastCall->CInfo = getSourceInfoFromExpr(SubExpr, Ctx, "", "");
        auto *ParenAroundNotCall = new Paren(NotCall, UOTy);
        CastCall->pushArg(ParenAroundNotCall);
        auto *NewParenRet = new Paren(CastCall, UOTy);
        NewParenRet->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
        return std::make_pair(NewParenRet, PulseSubExprRet.second);
      }

      return std::make_pair(NotCall, PulseSubExprRet.second);
    }
    else if (UO->getOpcode() == clang::UO_Minus) {
      // UO_MINUS should be compiled as 0 - x.
      // See:
      // https://github.com/FStarLang/c2pulse/issues/51#issuecomment-3076469809
      // TODO: Special handling for Uints.

      auto SymbolForBase = getSymbolKeyForCType(UO->getSubExpr()->getType(), Ctx);
      auto BinOp = clang::BO_Sub;

      auto TermForBaseExprRet =
          getTermFromCExpr(UO->getSubExpr(), VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      auto *TermForBaseExpr = TermForBaseExprRet.first;

      std::string OpStr;
      AppE *UOMinus;
      switch (SymbolForBase) {
      case SymbolTable::UInt32: {
        OpStr = "UInt32.minus";
        UOMinus = new AppE(OpStr, TermForBaseExpr->getType());
        UOMinus->pushArg(TermForBaseExpr);
        break;
      };
      case SymbolTable::UInt64: {
        OpStr = "UInt64.minus";
        UOMinus = new AppE(OpStr, TermForBaseExpr->getType());
        UOMinus->pushArg(TermForBaseExpr);
        break;
      };
      default: {
        OpStr = getSymbolKeyForOperator(SymbolForBase, BinOp);
        UOMinus = new AppE(OpStr, TermForBaseExpr->getType());
        auto *ConstrZeroTerm = new ConstTerm("0", TermForBaseExpr->getType());
        ConstrZeroTerm->CInfo = getSourceInfoFromExpr(UO, Ctx, "", "");
        // This is probably not needed anymore, since we have types in the AST,
        // Vidush TODO: remove
        ConstrZeroTerm->Symbol = SymbolForBase;
        UOMinus->pushArg(ConstrZeroTerm);
        UOMinus->pushArg(TermForBaseExpr);
      }
      };

      UOMinus->CInfo = getSourceInfoFromExpr(UO, Ctx, "", "");

      // Wrap this deref in a parenthesis.
      auto *Parenthesis = new Paren(UOMinus, UOMinus->getType());
      Parenthesis->CInfo = getSourceInfoFromExpr(UO, Ctx, "", "");
      return std::make_pair(Parenthesis, TermForBaseExprRet.second);
    }
    else if (UO->getOpcode() == clang::UO_PostDec){
      auto *SubExpr = UO->getSubExpr();

      auto PulseSubExprRet =
          getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      
      auto PulseSubExpr = PulseSubExprRet.first;
      auto PulseSubExprTy = PulseSubExpr->getType();

      //Vidush: The post and pre op functions take ref as arguments. 
      //We should assert this here 
      std::string StringForPulseTy;
      if (auto *PulseRefTy = dyn_cast<FStarPointerType>(PulseSubExprTy)){
        StringForPulseTy = getCastNameForPulseType(PulseRefTy->PointerTo);
      }
      else {
        emitErrorWithLocation("Expected a pulse ref type!", &Ctx, UO->getExprLoc());
      }

      auto *PostDecPulseCall = new AppE("minusminuspost_" + StringForPulseTy, PulseSubExprTy);
      PostDecPulseCall->pushArg(PulseSubExpr);
      return std::make_pair(PostDecPulseCall, PulseSubExprRet.second);
    }
    else if (UO->getOpcode() == clang::UO_PostInc){
      auto *SubExpr = UO->getSubExpr();
      auto CTy = SubExpr->getType();

      auto PulseSubExprRet =
          getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      
      auto PulseSubExpr = PulseSubExprRet.first;
      auto PulseSubExprTy = PulseSubExpr->getType();

      //Vidush: The post and pre op functions take ref as arguments. 
      //We should assert this here 
      std::string StringForPulseTy;
      if (auto *PulseRefTy = dyn_cast<FStarPointerType>(PulseSubExprTy)){
        StringForPulseTy = getCastNameForPulseType(PulseRefTy->PointerTo);
      }
      else {
        emitErrorWithLocation("Expected a pulse ref type!", &Ctx, UO->getExprLoc());
      }

      auto *PostIncPulseCall = new AppE("pluspluspost_" + StringForPulseTy, PulseSubExprTy);
      PostIncPulseCall->pushArg(PulseSubExpr);
      return std::make_pair(PostIncPulseCall, PulseSubExprRet.second);
    }
    else if (UO->getOpcode() == clang::UO_PreDec){
      auto *SubExpr = UO->getSubExpr();

      auto PulseSubExprRet =
          getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      
      auto PulseSubExpr = PulseSubExprRet.first;
      auto PulseSubExprTy = PulseSubExpr->getType();

      //Vidush: The post and pre op functions take ref as arguments. 
      //We should assert this here 
      std::string StringForPulseTy;
      if (auto *PulseRefTy = dyn_cast<FStarPointerType>(PulseSubExprTy)){
        StringForPulseTy = getCastNameForPulseType(PulseRefTy->PointerTo);
      }
      else {
        emitErrorWithLocation("Expected a pulse ref type!", &Ctx, UO->getExprLoc());
      }

      auto *PreDecPulseCall = new AppE("minusminuspre_" + StringForPulseTy, PulseSubExprTy);
      PreDecPulseCall->pushArg(PulseSubExpr);
      return std::make_pair(PreDecPulseCall, PulseSubExprRet.second);
    }
    else if (UO->getOpcode() == clang::UO_PreInc){
      auto *SubExpr = UO->getSubExpr();

      auto PulseSubExprRet =
          getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                           Parent, ParentType, Module);
      
      auto PulseSubExpr = PulseSubExprRet.first;
      auto PulseSubExprTy = PulseSubExpr->getType();

      //Vidush: The post and pre op functions take ref as arguments. 
      //We should assert this here 
      std::string StringForPulseTy;
      if (auto *PulseRefTy = dyn_cast<FStarPointerType>(PulseSubExprTy)){
        StringForPulseTy = getCastNameForPulseType(PulseRefTy->PointerTo);
      }
      else {
        emitErrorWithLocation("Expected a pulse ref type!", &Ctx, UO->getExprLoc());
      }

      auto *PreIncPulseCall = new AppE("pluspluspre_" + StringForPulseTy, PulseSubExprTy);
      PreIncPulseCall->pushArg(PulseSubExpr);
      return std::make_pair(PreIncPulseCall, PulseSubExprRet.second);
    }
    
    else {
      DEBUG_WITH_TYPE(DEBUG_TYPE, {
      UO->dump();
      UO->getStmtClassName();  
      E->dumpPretty(Ctx);
      E->dump();
      });
      emitErrorWithLocation(
          "Unhandeled case in UnaryOperator getTermFromCExpr!", &Ctx,
          E->getExprLoc());
    }
  } else if (auto *CE = dyn_cast<CallExpr>(E)) {

    // if (CE->getDirectCallee()->getNameAsString() == pulseProofTermFromC) {
    //   auto NumArgs = CE->getNumArgs();
    //   assert(NumArgs == 1 &&
    //          "Expected number of arguments for Pulse Proof Term to be 1!");
    //   auto *UserLemma = new Lemma();
    //   if (auto *ArgToString =
    //           dyn_cast<StringLiteral>(CE->getArg(0)->IgnoreCasts())) {
    //     auto ArgString = ArgToString->getString();
    //     UserLemma->lemmas.push_back(ArgString.str());
    //   } else {
    //     emitErrorWithLocation(
    //         "Expected pulse while to have arguments as string literals", &SM,
    //         CE->getBeginLoc());
    //   }

    //   return UserLemma;
    // } else if (CE->getDirectCallee()->getNameAsString() ==
    //            pulseWhileInvariantFromC) {

    //   auto NumArgs = CE->getNumArgs();

    //   std::vector<Slprop *> VectorLemmas;
    //   for (size_t Idx = 0; Idx < NumArgs; Idx++) {
    //     auto *UserLemma = new Lemma();
    //     auto *Arg = CE->getArg(Idx);
    //     // assert that each argument is actually a string literal
    //     if (auto *ArgToString = dyn_cast<StringLiteral>(Arg->IgnoreCasts()))
    //     {
    //       auto ArgString = ArgToString->getString();
    //       UserLemma->lemmas.push_back(ArgString.str());
    //       Slprop *Prop = UserLemma;
    //       VectorLemmas.push_back(Prop);
    //       // assert that next statement is a while loop
    //     } else {
    //       emitErrorWithLocation(
    //           "Expected pulse while to have arguments as string literals",
    //           &SM, E->getExprLoc());
    //     }
    //   }

    //   auto *Next = getNextStatement(E, Ctx);
    //   if (auto *While = dyn_cast<WhileStmt>(Next)) {
    //     // Add corresponding while invariant.
    //     StmtToLemmas.insert(std::make_pair(While, VectorLemmas));
    //     return nullptr;
    //   }
    //   emitErrorWithLocation(
    //       "Expected next statement after pulse invariant to be a while!",
    //       &SM, CE->getBeginLoc());
    // }

    //auto CallName = CE->getDirectCallee()->getNameAsString();
    std::string CallName;
    auto *Callee = CE->getDirectCallee();
    PulseDecl *PulseDecl = nullptr;

    auto *CallAppE = new AppE();
    CallAppE->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    
    //We handle calls to free as a special case.
    if (Callee->getNameAsString() == "free"){
      CallName = "free";
      goto bypass_function_type_inference;
    }

    //Check if a declaration exists. 
    if (auto *CalleeDecl = Callee->getCanonicalDecl()){
      
      auto PulseDefIt = DeclEnv.find(Callee);
      auto PulseDeclIt = DeclEnv.find(CalleeDecl);
      
      //We need to generate a declaration or error out!
      if (PulseDeclIt == DeclEnv.end() && PulseDefIt == DeclEnv.end()){
        auto *ClangDecl = Callee->getCanonicalDecl();
        VisitFunctionDeclMain(ClangDecl, true);
        PulseDeclIt = DeclEnv.find(ClangDecl);
        if (PulseDeclIt == DeclEnv.end()){
          emitErrorWithLocation("Could not find pulse function!", &Ctx, Callee->getLocation());
        }
        PulseDecl = PulseDeclIt->second;
        CallName = getDeclName(PulseDecl);        
      }
      //use the declaration name
      else if (PulseDeclIt != DeclEnv.end()){
        PulseDecl = PulseDeclIt->second;
        CallName = getDeclName(PulseDecl);
      }
      //use the definition name
      else if (PulseDefIt != DeclEnv.end()) {
        PulseDecl = PulseDefIt->second;
        CallName = getDeclName(PulseDecl);
      }
      //either error out or generate the definition!
      else {
        emitErrorWithLocation("Could not find pulse function!", &Ctx, Callee->getLocation());
      }
    }
    else {
      auto PulseDefIt = DeclEnv.find(Callee);
      //generate definition or error out
      if (PulseDefIt == DeclEnv.end()){
        emitErrorWithLocation("Could not find pulse function!", &Ctx, Callee->getLocation());
      }
      PulseDecl = PulseDefIt->second;
      CallName = getDeclName(PulseDecl);
    }


    if (Callee->getType()->isVoidPointerType() || Callee->getType()->isVoidType()){
        goto bypass_function_type_inference;
    }

    CallAppE->Type = PulseDecl->DeclType;
  bypass_function_type_inference:

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
        // if (auto *UO_Arg = dyn_cast<UnaryOperator>(Arg)){
        //   if (UO_Arg->getOpcode() == UO_AddrOf){
        //     assert(Param->getType()->isPointerType() && "Expect to pass a reference since function param expects it!");
        //     auto *BaseExpr = UO_Arg->getSubExpr();
        //     if (auto *DeclSub = dyn_cast<DeclRefExpr>(BaseExpr)){
        //       auto *NewVar = new VarTerm(DeclSub->getDecl()->getNameAsString());
        //       CallAppE->pushArg(NewVar);
        //       continue;
        //     }
        //   }
        // }

        auto ArgTermRet = getTermFromCExpr(Arg, VEnv, MutAnalyzer, ExprsBefore,
                                           CE, Arg->getType(), Module);
        auto *ArgTerm = ArgTermRet.first;
        CallAppE->pushArg(ArgTerm);
      }
    } else {

      // check argument type.
      assert(CE->getNumArgs() == 1 &&
             "Did not expect free to have more than one argument!");
      ///Vidush: Ignore implicit casts that are bitcasts (for instance to void*). 
      ///For free this is a special case.      
      auto *Arg = CE->getArg(0);
      if (auto *ICArg = dyn_cast<ImplicitCastExpr>(Arg)){
        if (ICArg->getCastKind() == clang::CK_BitCast){
          Arg = ICArg->getSubExpr();
        }
      }
      // Vidush: TODO, check if we should ignore implicit casts here!
      if (auto *ArgDeclR =
              dyn_cast<DeclRefExpr>(Arg->IgnoreParens()->IgnoreImpCasts())) {
        auto *ArgDecl = ArgDeclR->getDecl();
        auto Ty = ArgDecl->getType();
        auto PointeeType = Ty->getPointeeType();
        if (Ty->isPointerType() && PointeeType->isRecordType()) {
          //auto *RecordTy = PointeeType->getAsStructureType();
          //auto *RecDec = RecordTy->getDecl();
          auto *RecDec = PointeeType->getAsRecordDecl();
          auto It = RecordToRecordName.find(RecDec);
          if (It == RecordToRecordName.end()) {
            emitError("Could not find type name for Record!");
          }
          auto StructName = It->second;
          auto NewCallName = StructName + "_free";
          CallAppE->makeCallName(NewCallName);
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(ArgTerm);
        } 
        else if (auto *ArrTy = dyn_cast<FStarArrType>(pulseTyFromDecl(ArgDecl))){
          auto *ElemTy = ArrTy->ElementType; 
          CallAppE->makeCallName("free_array");
          auto *Arg1 = new Name("#" + ElemTy->print(), ElemTy);
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(Arg1);
          CallAppE->pushArg(ArgTerm);
          
        }
        else {
          CallAppE->makeCallName(CallName + "_ref");
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(ArgTerm);
        }
      }
      else {
        auto Ty = Arg->getType();
        auto PointeeType = Ty->getPointeeType();
        if (Ty->isPointerType() && PointeeType->isRecordType()) {
          //auto *RecordTy = PointeeType->getAsStructureType();
          //auto *RecDec = RecordTy->getDecl();
          auto *RecDec = PointeeType->getAsRecordDecl();
          auto It = RecordToRecordName.find(RecDec);
          if (It == RecordToRecordName.end()) {
            emitError("Could not find type name for Record!");
          }
          auto StructName = It->second;
          auto NewCallName = StructName + "_free";
          CallAppE->makeCallName(NewCallName);
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(ArgTerm);
        }
        //TODO: This needs more examples to test.
        else if (auto *ArrTy = dyn_cast<FStarArrType>(getPulseTyFromCTy(Ty))){
          auto *ElemTy = ArrTy->ElementType; 
          CallAppE->makeCallName("free_array");
          auto *Arg1 = new Name("#" + ElemTy->print(), ElemTy);
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(Arg1);
          CallAppE->pushArg(ArgTerm);
          
        }
        else {
          CallAppE->makeCallName(CallName + "_ref");
          auto ArgTermRet = getTermFromCExpr(
              Arg, VEnv, MutAnalyzer, ExprsBefore, CE, ParentType, Module);
          auto *ArgTerm = ArgTermRet.first;
          CallAppE->pushArg(ArgTerm);
        }
      }
    }

    // Wrap Call expr in Paren Node
    // Vidush : TODO: Find a way to get the return type of the pulse call.
    // Bug: This should not be nullptr!
    // Update: 7/28/25 at 3:00 PM
    // I have all the pulse types, next thing, we should not copy pointers for
    // type but value. Right now, the type in a Pulse Term is a pointer, but we
    // can just make it a value instead. I don't think we should allow sharing
    // between types of different AST nodes. This might break in case of
    // optimizations.
    auto *NewParen = new Paren(CallAppE, CallAppE->getType());
    NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    // Vidush: fine to return VEnv here as I don't think pulse allows
    // declaration new variables in call args.
    return std::make_pair(NewParen, VEnv);
  } else if (auto *IC = dyn_cast<ImplicitCastExpr>(E)) {
    
    //Check if this is a call to malloc
    if (const CallExpr *Call =
            dyn_cast<CallExpr>(E->IgnoreParenImpCasts()->IgnoreCasts())) {
      if (const FunctionDecl *FD = Call->getDirectCallee()) {
        if (FD->getName() == "malloc") {
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << "Found a malloc call inside CStyleCastExpr!\n";
          });
          auto *ElementTy =
              IC->getType()->getPointeeOrArrayElementType();
          auto *DesugaredElemTy = ElementTy->getUnqualifiedDesugaredType();
          auto CastType = IC->getType();
          
          auto *SizeExpr = Call->getArg(0);
          Expr::EvalResult Result;
          if (SizeExpr->EvaluateAsInt(Result, Ctx)) {
            llvm::APSInt val = Result.Val.getInt();
            int64_t SizeVal = val.getZExtValue();
            if (SizeVal != Ctx.getTypeSizeInChars(CastType->getPointeeType()).getQuantity()){
              emitErrorWithLocation("Allocating incorrect size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            }
          }
          else{
            emitErrorWithLocation("Could not deduce constant size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
          }


          if (const RecordType *TT = ElementTy->getAs<RecordType>()) {
            const RecordDecl *RD = TT->getDecl();
            auto It = RecordToRecordName.find(RD);
            if (!RD || It == RecordToRecordName.end()) {
              auto *CastType = IC->getType()
                                   ->getPointeeOrArrayElementType()
                                   ->getUnqualifiedDesugaredType();
              CastType->dump();
              emitErrorWithLocation(
                  "Could not find Record Declaration or Corresponding Name!",
                  &Ctx, FD->getLocation());
            }
            
            auto RecordName = It->second;
            auto *PulseRecordTy = new FStarType(RecordName);
            auto *PulseRecordRefTy = new FStarPointerType(PulseRecordTy);
            auto *NewCall = new AppE(RecordName + "_alloc", PulseRecordRefTy);
            NewCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewCall, VEnv);
          }
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << "Print the type of cast!" << "\n";
          }); 
          
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          CastType->dump();
          llvm::outs() << "The corresponding pulse type is: ";
          });
          auto *PulseTy = getPulseTyFromCTy(CastType);
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << PulseTy->print() << "\n";
          });
          if (auto *PulsePointerTy = dyn_cast<FStarPointerType>(PulseTy)){

            // //Check size. 
            // //Vidush: For now, if a ref asks for a larger size than the type of the cast,
            // //We error out. 
            // //Also if we can't tell the size staically we error out. 
            // auto *SizeExpr = Call->getArg(0);
            // Expr::EvalResult Result;
            // if (SizeExpr->EvaluateAsInt(Result, Ctx)) {
            //   llvm::APSInt val = Result.Val.getInt();
            //   int64_t SizeVal = val.getZExtValue();
            //   if (SizeVal != Ctx.getTypeSizeInChars(CastType->getPointeeType()).getQuantity()){
            //     emitErrorWithLocation("Allocating incorrect size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            //   }
            // }
            // else{
            //   emitErrorWithLocation("Could not deduce constant size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            // }

            auto *NewCall =
                new AppE("alloc_ref #" + PulsePointerTy->PointerTo->print(),
                         PulsePointerTy);
            NewCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewCall, VEnv);
          }

          emitErrorWithLocation(
              "Expected allocated type for malloc to be a reference but found "
              "a pulse type that's not a reference!",
              &Ctx, FD->getBeginLoc());
        }
      }
    }
    
    //Skip ArrayToPointer Decay casts for now.
    if (IC->getCastKind() == clang::CK_ArrayToPointerDecay){
      auto *SubExpr = IC->getSubExpr();
      auto PulseSubExprRet = getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent,
                              ParentType, Module);
      return PulseSubExprRet;
      // auto *PulseSubExpr = PulseSubExprRet.first;
      // auto NewVEnv = PulseSubExprRet.second;

      // //Assert it is a ref and change type!
      // //Or Add a band here?? Vidush: Not sure about adding a bang.
      // auto *PulseSubExprTy = PulseSubExpr->getType();
      // if (auto *PulseArrPointerTy = dyn_cast<FStarPointerType>(PulseSubExprTy)){
      //   auto *ArrTy = PulseArrPointerTy->PointerTo;
      //   //Assert this is an array?
      //   auto *PulseArrTy = dyn_cast<FStarArrType>(ArrTy);
      //   if (!PulseArrTy){
      //     emitErrorWithLocation("Expected underlying type to be array type!", &Ctx, IC->getExprLoc());
      //   }
      //   //We need to add a bang here and return the pulse array type as the eventual value.
      //   auto *BangCall = new AppE("!", PulseArrTy);
      //   auto *ParenAroundSubExpr = new Paren(PulseSubExpr, PulseSubExprTy);
      //   BangCall->pushArg(ParenAroundSubExpr);
      //   //Return!
      //   return std::make_pair(BangCall, NewVEnv);
      // }
      
      // emitErrorWithLocation("Expected type to be a ref!", &Ctx, IC->getExprLoc());
    }

    if (checkIfExprIsNullPtr(IC)) {
      // TODO: Vidush: What is the pulse return type of a null value?
      auto *NullValue = new Name("(null #_)", nullptr);
      NullValue->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
      return std::make_pair(NullValue, VEnv);
    }
    
    //TODO: These might require additional casts.
    if (IC->getCastKind() == clang::CK_LValueToRValue){      
      auto *SubExpr = IC->getSubExpr(); 
      //TODO: Vidush: These should be changed to Arr_At later on.
      //Check for Array subscript expression. 
      if (auto *ArrSub = dyn_cast<ArraySubscriptExpr>(SubExpr->IgnoreParens())){
        // We ignore the cast.
        auto PulseSubExprRet =
            getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent,
                             ParentType, Module);
        return PulseSubExprRet;
      }

      auto PulseSubExprRet = getTermFromCExpr(
          SubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent, ParentType, Module);

      auto *PulseSubExpr = PulseSubExprRet.first;

      auto *PulseSubExprTy = PulseSubExpr->getType();
      auto *PulseSubExprRefTy = dyn_cast<FStarPointerType>(PulseSubExprTy);
      if (!PulseSubExprRefTy) {
        SubExpr->dump();
        llvm::outs() << "\n";
        PulseSubExpr->dumpPretty();
        llvm::outs() << "\n";
        llvm::outs() << getPulseTyAsString(PulseSubExprTy) << "\n";
        emitError("Expected underlying type to be a ref!");
      }

      auto PulseSubExprPointsTo = PulseSubExprRefTy->PointerTo;

      auto *BangNode = new AppE("!", PulseSubExprPointsTo);
      BangNode->CInfo = getSourceInfoFromExpr(IC, Ctx, "", "");
      BangNode->pushArg(PulseSubExpr);
      auto *NewParen = new Paren(BangNode, PulseSubExprPointsTo);
      NewParen->CInfo = getSourceInfoFromExpr(IC, Ctx, "", "");

      return std::make_pair(NewParen, PulseSubExprRet.second);
    }
    
    IC->dump();
    // TODO: Check : Vidush
    // Right now we basically ignore implicit cast expressions.
    // However, since pulse is pure this may not be expected.
    auto *SubExpr = IC->getSubExpr();
    QualType sourceType = SubExpr->getType();
    QualType destType = IC->getType();
    if (getPulseStringForCType(sourceType, Ctx) == getPulseStringForCType(destType, Ctx)) {
      SmallVector<PulseStmt *> ExprsBefore;
      auto RetTerm = getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                                      Parent, ParentType, Module);
      auto *NewSubExpr = RetTerm.first;
      NewSubExpr->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
      return RetTerm;
    }
    else if (isa<CStyleCastExpr>(SubExpr)){
      emitErrorWithLocation("Unsafe casting not not handled!", &Ctx, SubExpr->getExprLoc());
    }
    
    ///make a call expr
    auto *PulseDestType = getPulseTyFromCTy(destType);
    auto *CastCall = new AppE(getPulseStringForCType(sourceType, Ctx) + "_to_" +
                                  getPulseStringForCType(destType, Ctx),
                              PulseDestType);
    CastCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    SmallVector<PulseStmt *> ExprsBefore;
    auto RetTerm = getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore,
                                    Parent, IC->getType(), Module);
    auto *NewSubExpr = RetTerm.first;
    CastCall->pushArg(NewSubExpr);
    auto *NewParen = new Paren(CastCall, PulseDestType);
    NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    return std::make_pair(NewParen, RetTerm.second);

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
    // if (MutAnalyzer->isMutated(DreDecl) && !isWrite) {
    //   // Create a new variable to be returned.
    //   // TODO: Vidush create a gensym for to get variable name.

    //   auto *InitAppE = new AppE("!");

    //   // The actual variable whose value we want
    //   VarTerm *VTerm = new VarTerm(DRE->getDecl()->getNameAsString());
    //   InitAppE->pushArg(VTerm);

    //   // Wrap this AppE in a Parenthesis.
    //   auto *PulseParenthesis = new Paren(InitAppE);
    //   PulseParenthesis->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    //   return PulseParenthesis;
    // }

    // Get the the type of the variable from VEnv.
    auto *PulseTyOfVTerm =
        findVarTyPulseTyEnv(DRE->getDecl()->getNameAsString(), VEnv);
    VarTerm *VTerm =
        new VarTerm(DRE->getDecl()->getNameAsString(), PulseTyOfVTerm);
    VTerm->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    return std::make_pair(VTerm, VEnv);

  } else if (auto *ArrSubExpr = dyn_cast<ArraySubscriptExpr>(E)) {
    auto *ArrBase = ArrSubExpr->getBase();
    auto *ArrIdx = ArrSubExpr->getIdx();

    llvm::outs() << "Dump arr sub expression!\n";
    ArrSubExpr->dump();
    llvm::outs() << "End dump arr sub expression.\n";

    auto ArrBaseRet = getTermFromCExpr(ArrBase, VEnv, MutAnalyzer, ExprsBefore,
                                       Parent, ParentType, Module);

    auto *PulseArrBase = ArrBaseRet.first;

    // Use The IR to get the type not the C function.
    auto *PulseTy =
        PulseArrBase->getType(); // getPulseTyFromCTy(ArrBase->getType());
    auto *PulseArrTy = dyn_cast<FStarArrType>(PulseTy);
    if (!PulseArrTy) {
      llvm::outs() << "\n";
      ArrBase->dump(); 
      llvm::outs() << "\n";
      ArrIdx->dump();
      llvm::outs() << "\n";
      emitErrorWithLocation("Expected a pulse array type but got: " +
                                getPulseTyAsString(PulseTy),
                            &Ctx, ArrSubExpr->getExprLoc());
    }

    auto *PulseCall = new AppE("op_Array_Access", PulseArrTy->ElementType);
    PulseCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    PulseCall->pushArg(PulseArrBase);

    auto PulseArrIdxRet =
        getTermFromCExpr(ArrIdx, ArrBaseRet.second, MutAnalyzer, ExprsBefore,
                         Parent, ArrIdx->getType(), Module);

    auto *PulseArrIdx = PulseArrIdxRet.first;

    // Add a cast to size_t if it is needed
    if (getPulseTyAsString(PulseArrIdx->getType()) !=
        lookupSymbol(SymbolTable::SizeT)) {
      auto *PulseSizeTy = new FStarType(lookupSymbol(SymbolTable::SizeT));
      auto *CastCall =
          new AppE(getPulseStringForCType(ArrIdx->getType(), Ctx) + "_to_sizet",
                   PulseSizeTy);
      CastCall->CInfo = getSourceInfoFromExpr(ArrIdx, Ctx, "", "");
      CastCall->pushArg(PulseArrIdx);
      auto *NewParen = new Paren(CastCall, PulseSizeTy);
      NewParen->CInfo = getSourceInfoFromExpr(ArrIdx, Ctx, "", "");
      PulseArrIdx = NewParen;
    }

    PulseCall->pushArg(PulseArrIdx);

    // wrap PulseCall in Paren
    auto *NewParen = new Paren(PulseCall, PulseArrTy->ElementType);
    NewParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    return std::make_pair(NewParen, PulseArrIdxRet.second);

  } else if (auto *ParenExpr = dyn_cast<clang::ParenExpr>(E)) {

    auto *ClangSubExpr = ParenExpr->getSubExpr();

    auto PulseSubExprRet =
        getTermFromCExpr(ClangSubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent,
                         ParentType, Module);
    auto *PulseSubExpr = PulseSubExprRet.first;

    auto *PulseParenExpr = new Paren(PulseSubExpr, PulseSubExpr->getType());
    PulseParenExpr->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
    return std::make_pair(PulseParenExpr, PulseSubExprRet.second);
  } else if (auto *CCastExpr = dyn_cast<CStyleCastExpr>(E)) {
    // Vidush::TODO We're ignore casts here, Check if we should?
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
          auto CastType = CCastExpr->getType();
          
          auto *SizeExpr = Call->getArg(0);
          Expr::EvalResult Result;
          if (SizeExpr->EvaluateAsInt(Result, Ctx)) {
            llvm::APSInt val = Result.Val.getInt();
            int64_t SizeVal = val.getZExtValue();
            if (SizeVal != Ctx.getTypeSizeInChars(CastType->getPointeeType()).getQuantity()){
              emitErrorWithLocation("Allocating incorrect size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            }
          }
          else{
            emitErrorWithLocation("Could not deduce constant size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
          }


          if (const RecordType *TT = ElementTy->getAs<RecordType>()) {
            const RecordDecl *RD = TT->getDecl();
            auto It = RecordToRecordName.find(RD);
            if (!RD || It == RecordToRecordName.end()) {
              auto *CastType = CCastExpr->getType()
                                   ->getPointeeOrArrayElementType()
                                   ->getUnqualifiedDesugaredType();
              CastType->dump();
              emitErrorWithLocation(
                  "Could not find Record Declaration or Corresponding Name!",
                  &Ctx, FD->getLocation());
            }
            
            auto RecordName = It->second;
            auto *PulseRecordTy = new FStarType(RecordName);
            auto *PulseRecordRefTy = new FStarPointerType(PulseRecordTy);
            auto *NewCall = new AppE(RecordName + "_alloc", PulseRecordRefTy);
            NewCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewCall, VEnv);
          }
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << "Print the type of cast!" << "\n";
          }); 
          
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          CastType->dump();
          llvm::outs() << "The corresponding pulse type is: ";
          });
          auto *PulseTy = getPulseTyFromCTy(CastType);
          DEBUG_WITH_TYPE(DEBUG_TYPE , {
          llvm::outs() << PulseTy->print() << "\n";
          });
          if (auto *PulsePointerTy = dyn_cast<FStarPointerType>(PulseTy)){

            // //Check size. 
            // //Vidush: For now, if a ref asks for a larger size than the type of the cast,
            // //We error out. 
            // //Also if we can't tell the size staically we error out. 
            // auto *SizeExpr = Call->getArg(0);
            // Expr::EvalResult Result;
            // if (SizeExpr->EvaluateAsInt(Result, Ctx)) {
            //   llvm::APSInt val = Result.Val.getInt();
            //   int64_t SizeVal = val.getZExtValue();
            //   if (SizeVal != Ctx.getTypeSizeInChars(CastType->getPointeeType()).getQuantity()){
            //     emitErrorWithLocation("Allocating incorrect size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            //   }
            // }
            // else{
            //   emitErrorWithLocation("Could not deduce constant size for reference. Did you want an array instead, if so, annotate with ISARRAY.\n", &Ctx, E->getExprLoc());
            // }

            auto *NewCall =
                new AppE("alloc_ref #" + PulsePointerTy->PointerTo->print(),
                         PulsePointerTy);
            NewCall->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
            return std::make_pair(NewCall, VEnv);
          }

          emitErrorWithLocation(
              "Expected allocated type for malloc to be a reference but found "
              "a pulse type that's not a reference!",
              &Ctx, FD->getBeginLoc());
        }
      }
    } else if (checkIfExprIsNullPtr(CCastExpr)) {
      // TODO: Don't put nullptr here
      auto NullValue = new Name("(null #_)", nullptr);
      NullValue->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");
      return std::make_pair(NullValue, VEnv);
    } else {
      CCastExpr->dumpPretty(Ctx);
      emitErrorWithLocation("Unimplemented case in CStyle Cast Expression!",
                            &Ctx, CCastExpr->getExprLoc());
    }
  } else if (auto *RE = dyn_cast<clang::RecoveryExpr>(E)) {
    if (Expr *SubExpr = RE->getExprStmt()) {
      return getTermFromCExpr(SubExpr, VEnv, MutAnalyzer, ExprsBefore, Parent,
                              ParentType, Module);
    }
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    RE->dump();
    });
    llvm::errs() << "RecoveryExpr without sub-expression, returning nullptr.\n";
    return std::make_pair(nullptr, VEnv);
  } else if (auto *ME = dyn_cast<MemberExpr>(E)) {

    auto *MemberExprDecl = ME->getMemberDecl();
    // TODO: check if we should ignore cases here
    // Casts may need to be translated to !.
    // But we need a principled approach for this.
    auto *BaseExpr = ME->getBase();
    auto IsLVal = ME->isLValue();

    //std::string NameOfDecl;
    //QualType TyOfDecl;
    //std::string StructName;
    //if (const clang::DeclRefExpr *DRE =
    //        llvm::dyn_cast<clang::DeclRefExpr>(BaseExpr)) {
    //  const ValueDecl *VD = DRE->getDecl();
    //  // Now you can safely cast VD to a more specific Decl type if needed
    //  DEBUG_WITH_TYPE(DEBUG_TYPE , {
    //  VD->dump();
    //  llvm::outs() << VD->getDeclName() << "End\n";
    //  });

    //  NameOfDecl = VD->getDeclName().getAsString();
    //  TyOfDecl = VD->getType();
      
    //  DEBUG_WITH_TYPE(DEBUG_TYPE , {
    //  llvm::outs() << TyOfDecl->getPointeeType().getAsString() << "\n";
    //  });
    //  StructName = TyOfDecl->getPointeeType().getAsString();

      auto MemberName = MemberExprDecl->getDeclName();
      ValueDecl *MemberDecl = ME->getMemberDecl();
      auto MemberCTy = MemberDecl->getType();
      auto *MemberTy = pulseTyFromDecl(MemberDecl);
      // We always assume member fields of a struct are a ref.
      if (!MemberCTy->isArrayType()){
        auto *ToPulseRef = new FStarPointerType(MemberTy);
        MemberTy = ToPulseRef;
      }
      
      //auto *GenStmt =
      //    new Name("(!(!" + NameOfDecl + ")." + MemberName.getAsString() + ")");

      llvm::outs() << "Print the type of the projection!\n\n";
      MemberTy->dumpPretty();
      llvm::outs() << "\n\nEnd printing member type!\n\n";


      auto *NewProject = new Project(MemberTy);
      NewProject->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

      NewProject->MemberName = MemberName.getAsString();

      auto BaseTermRet = getTermFromCExpr(
          BaseExpr, VEnv, MutAnalyzer, ExprsBefore, Parent, ParentType, Module);

      auto *BaseTerm = BaseTermRet.first;

      // auto *MemberTy = findVarTyPulseTyEnv(MemberName.getAsString(),
      // BaseTermRet.second);
      auto *BaseTermTy = BaseTerm->getType();

      llvm::outs() << "Print the type of the base term!\n\n";
      BaseTermTy->dumpPretty();
      llvm::outs() << "\n\nEnd printing base term!\n";

      if (IsLVal){
        // TODO: change from nullptr
        auto *BaseTermTyRef = dyn_cast<FStarPointerType>(BaseTermTy);
        if (!BaseTermTyRef) {
          emitError("Expected a reference!");
        }
        auto BaseTermTyRefPointsTo = BaseTermTyRef->PointerTo;
        // auto MemberTermPointsTo = MemberTermTyRef->PointerTo;
        auto *App = new AppE("!", BaseTermTyRefPointsTo);
        App->CInfo = getSourceInfoFromExpr(BaseExpr, Ctx, "", "");
        App->pushArg(BaseTerm);

        auto *NewParen = new Paren(App, BaseTermTyRefPointsTo);
        NewParen->CInfo = getSourceInfoFromExpr(BaseExpr, Ctx, "", "");
        NewProject->BaseTerm = NewParen;
      } 
      else {
        NewProject->BaseTerm = BaseTerm;
      }                       
      //TODO: Vidush, ensure heuristic is correct.
      //check if we already added an explode for the struct here. 
      // auto It = TrackStructExplodeAndRecover.find(VD);

      // //We did not add a explode expression.
      // if (It == TrackStructExplodeAndRecover.end()){
      //   auto *ExplodeStmt = new GenericStmt();
      //   ExplodeStmt->body = StructName + "_explode " + VD->getNameAsString()
      //   + ";"; ExprsBefore.push_back(ExplodeStmt);
      //   TrackStructExplodeAndRecover.insert(std::make_pair(VD,
      //   std::make_pair(true, false)));
      // }

      return std::make_pair(NewProject, BaseTermRet.second);
      //}

      // return nullptr;
  }
  else if (auto *CondOperator = dyn_cast<ConditionalOperator>(E)){
    auto *Cond = CondOperator->getCond();
    auto *Then = CondOperator->getTrueExpr(); 
    auto *Else = CondOperator->getFalseExpr();

    SmallVector<PulseStmt *> ExprsBefore;

    auto PulseCondRet = getTermFromCExpr(Cond, VEnv, MutAnalyzer, ExprsBefore,
                                         Parent, Cond->getType(), Module);
    auto *PulseCond = PulseCondRet.first;

    if (Cond->getType().getAsString() != "_Bool" && Cond->getType().getAsString() != "bool"){

      auto *BoolTy = new FStarType(lookupSymbol(SymbolTable::Bool));
      auto *CastCall = new AppE(
          getPulseStringForCType(Cond->getType(), Ctx) + "_to_bool", BoolTy);
      CastCall->CInfo = getSourceInfoFromExpr(Cond, Ctx, "", "");
      CastCall->pushArg(PulseCond);

      auto *NewParen = new Paren(CastCall, BoolTy);
      NewParen->CInfo = getSourceInfoFromExpr(Cond, Ctx, "", "");
      PulseCond = NewParen;
    }                                   

    //PulseStmt *PulseThen;
    //Conditional Expressions don't have any supported annotations at the moment.
    // if (auto *AttrStmt = dyn_cast<AttributedStmt>(Then)) {
    //   auto *ThenBody = AttrStmt->getSubStmt();

    //   auto Attributes = AttrStmt->getAttrs();
    //   // auto *PulseWhile = new PulseWhileStmt();
    //   for (auto *Attr : Attributes) {

    //     if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)) {
    //       if (isPulseAnnotAttr(AnnAttr)) {

    //         auto AnnotationData = AnnAttr->getAnnotation().str();

    //         std::string Match;
    //         auto AnnKind = getPulseAnnKindFromString(AnnotationData, Match);
    //         switch (AnnKind) {
    //         case PulseAnnKind::Ensures: {
    //           auto *NewEnsures = new Ensures();
    //           NewEnsures->Ann = Match;
    //           PulseIfStmt->IfLemmas.push_back(NewEnsures);
    //           break;
    //         };
    //         default:
    //           emitErrorWithLocation("Annotation not expected for IfStmt",
    //           &Ctx,
    //                                 IF->getBeginLoc());
    //         };
    //       }
    //     }
    //   }
    //   PulseThen = pulseFromStmt(ThenBody, Analyzer, Parent, Module, CS);
    // } else {

    auto PulseThenRet = getTermFromCExpr(Then, VEnv, MutAnalyzer, ExprsBefore,
                                         Parent, Then->getType(), Module);
    auto *PulseThen = PulseThenRet.first;
    //}

    auto PulseElseRet = getTermFromCExpr(Else, VEnv, MutAnalyzer, ExprsBefore,
                                         Parent, Else->getType(), Module);
    auto *PulseElse = PulseElseRet.first;

    auto *PulseIfExpr =
        new IfExpr(PulseCond, PulseThen, PulseElse, PulseThen->getType());
    PulseIfExpr->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    auto *AddParen = new Paren(PulseIfExpr, PulseThen->getType());
    AddParen->CInfo = getSourceInfoFromExpr(E, Ctx, "", "");

    // Return VEnv since we don't want to return variables that are inside if
    // scope.
    return std::make_pair(AddParen, VEnv);

  }
  else if (auto *SizeOfExpr = dyn_cast<UnaryExprOrTypeTraitExpr>(E)){
    if (SizeOfExpr->getKind() == UETT_SizeOf){
       llvm::APSInt SizeValue = SizeOfExpr->EvaluateKnownConstInt(Ctx);
       uint64_t Size = SizeValue.getZExtValue();

       auto ConstantValue = std::to_string(Size);
       auto Ty = SizeOfExpr->getType();
       auto PulseTy = getPulseTyFromCTy(Ty);

       auto *Const = new ConstTerm(ConstantValue, PulseTy);
       Const->Symbol = getSymbolKeyForCType(Ty, Ctx);

       //  //add a cast
       // Don't add a cast here but handle at the array site.
       //  if (Ty.getAsString() != "size_t"){
       //   auto *CastCall = new AppE(getPulseStringForCType(Ty, Ctx) +
       //   "_to_size_t"); CastCall->pushArg(Const); auto *NewParen = new
       //   Paren(CastCall); return NewParen;
       //  }

       return std::make_pair(Const, VEnv);
    }
    else {
      emitErrorWithLocation("Not implemented!", &Ctx, E->getExprLoc());
    }

  }
  else {
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    E->dump();
    });
    emitErrorWithLocation("Expression not implemented in getTermFromCExpr!",
                          &Ctx, E->getExprLoc());
  }
}

PulseTransformer::PulseTransformer(ASTContext &Ctx,
  std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens
) : AstCtx(Ctx), CodeGen(Ctx), macroTokens(macroTokens) {
  RewriterForPlugin.setSourceMgr(Ctx.getSourceManager(), Ctx.getLangOpts());
}

// TODO: Make this return a bool instead of a string.
// Bool says if the function passed or failed. instead of exiting.
std::string PulseTransformer::writeToFile(std::optional<std::string> const& TmpDir) {
  
  
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
  if (TmpDir) NewPath = *TmpDir;
  NewPath += "/";

  auto &ModulesToBeOutputted = CodeGen.getEmittedModules();
  assert(ModulesToBeOutputted.size() == 1 && "Expected one Moudle per AST unit!");
  auto M = (ModulesToBeOutputted.begin());
  //for (auto &M : ModulesToBeOutputted) {

    // ASSUME: The module name if the file name atm.
    auto ModuleName = M->first;
    auto &OutputString = M->second;

    // Calculate path and then add NewFileName
    auto FilePathN = NewPath.string() + ModuleName;
    // Don't remove these since the run.sh script
    // depends on printing the output path of the filename.
    llvm::outs() << "Print the filename!\n";
    llvm::outs() << FilePathN << "\n";
    llvm::outs() << "End printing the filename!\n";
    std::error_code EC;
    llvm::raw_fd_ostream OutFile(FilePathN, EC);
    if (EC) {
      llvm::errs()
          << "Error: Failed to create temporary file for transformed code.\n";
    }

    OutFile << OutputString->str();
    

    // //Write the Clang C AST at the very end for debugging!
    // std::string Buffer;
    // llvm::raw_string_ostream TempStream(Buffer);
    // const clang::Decl *TUDecl = AstCtx.getTranslationUnitDecl();

    // NoPointerTextDumper NodeDumper(TempStream, AstCtx);


    // if (const auto *DC = llvm::dyn_cast<clang::DeclContext>(TUDecl)) {
    //   for (const auto *D : DC->decls()) {
    //       clang::SourceLocation Loc = D->getLocation();
    //       if (Loc.isValid()) {
    //           std::string fileName = SM.getFilename(Loc).str();
    //           if (fileName == FilePath) {
    //              //NodeDumper.dumpBareDeclRef(D);
    //              D->dump(TempStream);
    //           }
    //       }
    //   }
    // }
    
    // OutFile << "\n";
    // OutFile << "//Dumping the Clang AST.\n"; 
    // std::istringstream Input(Buffer);
    // std::string Line;
    // while (std::getline(Input, Line)) {
    //     OutFile << "// " << Line << "\n";
    // }
   
  if (false)
  CodeGen.printSourceLocations();

  CodeGen.JsonifySourceRangeMap(NewPath.string() + "/" + FileNameStr +  "_source_range_info.json");


  // TODO what should we return for writeToFile
  return "";
}

void PulseTransformer::transform() {

  PulseConsumer Consumer(AstCtx, RewriterForPlugin, macroTokens);
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
