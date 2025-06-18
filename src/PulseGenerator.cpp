#include "PulseGenerator.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attrs.inc"
#include "clang/AST/Comment.h"
#include "clang/AST/CurrentSourceLocExprScope.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Analysis/Analyses/ExprMutationAnalyzer.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <utility>
// #include "PulseCodeGen.h"
#include "Globals.h"

using namespace clang;

void PulseConsumer::setNewFunctionDeclarations(std::vector<PulseDecl *> &FVec) {
  FunctionDeclarations = FVec;
}

std::vector<PulseDecl *> &PulseConsumer::getNewFunctionDeclarations() {
  return FunctionDeclarations;
}

PulseConsumer::PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R)
    : Visitor(R, Ctx) {}

void PulseConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  setNewFunctionDeclarations(Visitor.getFunctionDeclarations());
}

std::vector<PulseDecl *> &PulseVisitor::getFunctionDeclarations() {
  return FunctionDeclarations;
}


// void PulseVisitor::extractPulseAnnotations(
//     const clang::FunctionDecl *FD, const clang::SourceManager &SM,
//     std::vector<PulseExpr *> &result) {

//   // auto *Raw = FD->getASTContext().getRawCommentForAnyRedecl(FD);
//   // llvm::outs() << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << "\n";
//   // llvm::outs() << Raw->getRawText(Ctx.getSourceManager());

//   if (const auto *C = FD->getASTContext().getRawCommentForAnyRedecl(FD)) {
//     std::string cleaned;
//     for (char c : C->getRawText(SM))
//       if (c != '\r')
//         cleaned += c;
    

//     std::istringstream in(cleaned);
//     std::string line;
//     std::regex reqRegex(R"(@requires\s+(.*))");
//     std::regex ensRegex(R"(@ensures\s+(.*))");
//     std::smatch match;

//     while (std::getline(in, line)) {
//       auto trimmed = StringRef(line).trim().str();
//       if (std::regex_search(trimmed, match, reqRegex)){

//         auto *NewPulseExprNode = new PulseExpr(); 
//         auto *RequiresTerm = new Requires();
//         RequiresTerm->Ann = match[1];
//         NewPulseExprNode->E = RequiresTerm;
//         result.push_back(NewPulseExprNode);
//       }
//       else if (std::regex_search(trimmed, match, ensRegex)){
//         auto *NewPulseExprNode = new PulseExpr(); 
//         auto *EnsuresTerm = new Ensures();
//         EnsuresTerm->Ann = match[1];
//         NewPulseExprNode->E = EnsuresTerm;
//         result.push_back(NewPulseExprNode);
//       }
//     }

//     // llvm::outs() << "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << "\n";
//     // llvm::outs() << result.back().predicate << "\n";
//     // llvm::outs() << "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << "\n";
    
//   }

//   // int counter = 1;
//   // for (auto& ann : result)
//   //     ann.regionId = "'n" + std::to_string(counter++);
// }

void PulseVisitor::InferDeclType(Decl* Dec, FunctionDecl *FD){

     if (Stmt *Body = FD->getBody()) {
      if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
        for (auto *InnerStmt : CS->body()) {
          inferDeclType(Dec, InnerStmt);
        }
      }
    }

}

std::map<Decl*, QualType> PulseVisitor::inferArrayTypes(FunctionDecl *FD){
  
  //std::vector<Decl*> VariablesInScope;
  std::map<Decl*, QualType> DeclToPulseSymbol;
  //For now do this for all function arguments.
  // size_t NumDeclarations = FD->getNumParams(); 
  // for (size_t I = 0; I < NumDeclarations; I++){
  //   VariablesInScope.push_back(FD->getParamDecl(I));
  // } 

    if (Stmt *Body = FD->getBody()) {
      if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
        for (auto *InnerStmt : CS->body()) {
          inferArrayTypesStmt(InnerStmt);
        }
      }
    }

    return DeclToPulseSymbol;
}

bool PulseVisitor::checkIsRecursiveFunction(FunctionDecl *FD){
    
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

void PulseVisitor::inferDeclType(Decl *Dec, Stmt *InnerStmt){
  
  //Check the types of the statements here. 
  if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    for (auto *D : DS->decls()) {
      if (D != Dec)
         continue;
      if (auto *VD = dyn_cast<VarDecl>(D)) {
        //if we can tell it is an array type from the declaration we just store it in a map. 
        //Otherwise we use array subscript operations to check.
        if (VD->getType()->isArrayType()){
          DeclTyMap.insert(std::make_pair(VD, VD->getType()));
        }
        if (auto *Annotation = VD->getAttr<AnnotateAttr>()){
          llvm::outs() << Annotation->getAnnotation() << "\n";
        }
      }
    }
  }
  else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)){
    inferArrayTypesExpr(CExpr);
  }
  else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)){

    auto *Cond = While->getCond();
    auto *Body = While->getBody(); 

    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Body);

  }
  else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
        for (auto *InnerStmt : CS->body()) {
          inferArrayTypesStmt(InnerStmt);
        }
  }
  else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)){
    auto *SubExpr = AttrStmt->getSubStmt();
    inferArrayTypesStmt(SubExpr);
    //TODO: Vidush see if we want to handle any other statement.
    //InnerStmt->dump();
    //assert(false && "Did not handle statement in inferArrayTypesStmt\n");
  }
}


void PulseVisitor::inferArrayTypesStmt(Stmt *InnerStmt){
  
  //Check the types of the statements here. 
  if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {
        //if we can tell it is an array type from the declaration we just store it in a map. 
        //Otherwise we use array subscript operations to check.
        if (VD->getType()->isArrayType()){
          DeclTyMap.insert(std::make_pair(VD, VD->getType()));
        }
        if (auto *Annotation = VD->getAttr<AnnotateAttr>()){
          llvm::outs() << Annotation->getAnnotation() << "\n";
        }
      }
    }
  }
  else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)){
    inferArrayTypesExpr(CExpr);
  }
  else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)){

    auto *Cond = While->getCond();
    auto *Body = While->getBody(); 

    inferArrayTypesExpr(Cond);
    inferArrayTypesStmt(Body);

  }
  else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
        for (auto *InnerStmt : CS->body()) {
          inferArrayTypesStmt(InnerStmt);
        }
  }
  else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)){
    auto *SubExpr = AttrStmt->getSubStmt();
    inferArrayTypesStmt(SubExpr);
    //TODO: Vidush see if we want to handle any other statement.
    //InnerStmt->dump();
    //assert(false && "Did not handle statement in inferArrayTypesStmt\n");
  }
}

void PulseVisitor::inferArrayTypesExpr(Expr *ExprPtr){
                                  
    if (auto *BinOp = dyn_cast<clang::BinaryOperator>(ExprPtr)) {
      
      //TODO: Vidush: 
      //If this BinOp is of the shape: *Arr + 8 etc, we may conclude it is of an array type.
      auto *Lhs = BinOp->getLHS(); 
      auto *Rhs = BinOp->getRHS(); 

      inferArrayTypesExpr(Lhs);
      inferArrayTypesExpr(Rhs);


    }
    else if (auto *UOp = dyn_cast<clang::UnaryOperator>(ExprPtr)) { 
      inferArrayTypesExpr(UOp->getSubExpr());

    }
    else if (auto *Call = dyn_cast<clang::CallExpr>(ExprPtr)) { 
      auto NumArgs = Call->getNumArgs(); 
      for (size_t Idx = 0; Idx < NumArgs; Idx++){
        auto *Arg = Call->getArg(Idx);
        inferArrayTypesExpr(Arg);
      }
    }
    else if (auto *ASub = dyn_cast<clang::ArraySubscriptExpr>(ExprPtr)) {
      if (auto *BaseDecl = ASub->getBase()->getReferencedDeclOfCallee()){
        //Find if this BaseDecl
        if (VarDecl *VD = dyn_cast<VarDecl>(BaseDecl)){
          clang::QualType IncompleteArrayTy = Ctx.getIncompleteArrayType(
            VD->getType()->getPointeeType(),
            clang::ArraySizeModifier::Normal,
            0
          );

          DeclTyMap.insert(std::make_pair(BaseDecl, IncompleteArrayTy));
        }
      }
    }
    else {return;}
}

bool PulseVisitor::checkIsRecursiveStmt(Stmt *InnerStmt, FunctionDecl *CurrFunction){

  if (!InnerStmt){
    return false;
  }
  
  if (auto *N = dyn_cast<NullStmt>(InnerStmt)){
    return false;
  }
  //Check the types of the statements here. 
  else if (auto *DS = dyn_cast<DeclStmt>(InnerStmt)) {
    return false;
  }
  else if (auto *CExpr = dyn_cast<Expr>(InnerStmt)){
    return checkIsRecursiveExpr(CExpr, CurrFunction);
  }
  else if (auto *While = dyn_cast<WhileStmt>(InnerStmt)){

    auto *Cond = While->getCond();
    auto *Body = While->getBody(); 

    auto isRecCond = checkIsRecursiveExpr(Cond, CurrFunction);
    auto isRecBody = checkIsRecursiveStmt(Body, CurrFunction);

    return isRecCond || isRecBody;

  }
  else if (auto *CS = dyn_cast<CompoundStmt>(InnerStmt)) {
      bool result = false;  
      for (auto *InnerStmt : CS->body()) {
          result = result || checkIsRecursiveStmt(InnerStmt, CurrFunction);
        }
      return result;
  }
  else if (auto *AttrStmt = dyn_cast<AttributedStmt>(InnerStmt)){
    auto *SubExpr = AttrStmt->getSubStmt();
    return checkIsRecursiveStmt(SubExpr, CurrFunction);
  }
  else if (auto *RetStmt = dyn_cast<ReturnStmt>(InnerStmt)){
    auto *RetExpr = RetStmt->getRetValue();

    return checkIsRecursiveExpr(RetExpr, CurrFunction);
  }
  else if (auto *ClangIfStmt = dyn_cast<IfStmt>(InnerStmt)){
    auto *Cond = ClangIfStmt->getCond(); 
    auto *Then = ClangIfStmt->getThen(); 
    auto *Else = ClangIfStmt->getElse();

    auto CondIsRec = checkIsRecursiveExpr(Cond, CurrFunction);
    auto ThenIsRec = checkIsRecursiveStmt(Then, CurrFunction);
    auto ElseIsRec = checkIsRecursiveStmt(Else, CurrFunction);

    return CondIsRec || ThenIsRec || ElseIsRec;

  }
  else{
        //TODO: Vidush see if we want to handle any other statement.
    InnerStmt->dump();
    assert(false && "Did not handle statement in inferArrayTypesStmt\n");

  }
}

//Recurse all expressions.
bool PulseVisitor::checkIsRecursiveExpr(Expr *ExprPtr, FunctionDecl *CurrFunction){
                                  
    if (auto *BinOp = dyn_cast<clang::BinaryOperator>(ExprPtr)) {
      
      //TODO: Vidush: 
      //If this BinOp is of the shape: *Arr + 8 etc, we may conclude it is of an array type.
      auto *Lhs = BinOp->getLHS(); 
      auto *Rhs = BinOp->getRHS(); 

      return checkIsRecursiveExpr(Lhs, CurrFunction) || checkIsRecursiveExpr(Rhs, CurrFunction);


    }
    else if (auto *UOp = dyn_cast<clang::UnaryOperator>(ExprPtr)) { 
      return checkIsRecursiveExpr(UOp->getSubExpr(), CurrFunction);

    }
    else if (auto *Call = dyn_cast<clang::CallExpr>(ExprPtr)) { 
      auto NumArgs = Call->getNumArgs();
      bool isRec = false; 
      for (size_t Idx = 0; Idx < NumArgs; Idx++){
        auto *Arg = Call->getArg(Idx);
        isRec = isRec ||  checkIsRecursiveExpr(Arg, CurrFunction);
      }


      llvm::outs() << "Found a Call Expression!!!" << "\n";
      llvm::outs() << Call->getDirectCallee()->getNameAsString() << "\n";
      llvm::outs() << "End found a recursive call!" << "\n";
      
      const FunctionDecl *callee = Call->getDirectCallee();
      if (callee && callee == CurrFunction) {
        llvm::outs() << "Found a recursive Call!!" << "\n";
        llvm::outs() << callee << "\n";
        llvm::outs() << "End found a recursive call!" << "\n";
        isRec = isRec || true;
      }

      return isRec;
    }
    else if (auto *ASub = dyn_cast<clang::ArraySubscriptExpr>(ExprPtr)) {
      return false;
    }
    else {return false;}
}

bool PulseVisitor::VisitFunctionDecl(FunctionDecl *FD) {
  
  if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()))
    return true;
  // llvm::outs() << "Processing Function: " << FD->getNameAsString() << "\n";
  auto FuncName = FD->getNameAsString();
  // struct _PulseFnDefn {
  // std::string Name;
  // std::vector<Binder*> Args;
  // bool isRecursive;
  // PulseStmt *Body;
  //  };
  auto *FDefn = new _PulseFnDefn();
  FDefn->Name = FuncName;
  FDefn->isRecursive = true;

  if (!checkIsRecursiveFunction(FD)){
    llvm::outs() << "Found Function to be non recursive!!" << "\n";
    llvm::outs() << FD->getNameAsString() << "\n";
    llvm::outs() << "End function name\n";
    
    FDefn->isRecursive = false;
  }
  
  if (FD->hasAttrs()){
  auto AnnotationsAttachedToFD = FD->getAttrs();
  for (auto *Attr : AnnotationsAttachedToFD){

    if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)){

    if (AnnAttr->getAttrName()->getName() == "pulse"){
      llvm::outs() << "Attribute Name: ";
      llvm::outs() << Attr->getAttrName()->getName();
      llvm::outs() << "\n";
      llvm::outs() << AnnAttr->getAnnotation() << "\n";
      auto Ref = AnnAttr->getAnnotation();
      llvm::outs() << Ref << "\n";
      if (!Ref.empty()) {
        //  std::wsmatch Match;
         std::string Match = "";
         PulseAnnKind AnnKind = getPulseAnnKindFromString(AnnAttr->getAnnotation().data(), Match);
         switch(AnnKind){
         case PulseAnnKind::Requires:{
          auto *NewRequires = new Requires(); 
          // std::wstring matched_wstring = Match[1].str();
          // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
          // std::string matched_string = converter.to_bytes(matched_wstring);
          //auto matched_string = Match[1].str();
          //llvm::outs() << matched_string << "\n";
          //NewRequires->Ann = matched_string;
          NewRequires->Ann = Match;
          FDefn->Annotation.push_back(NewRequires);
          break;
         }
         case PulseAnnKind::Ensures:{
          auto *NewEnsures = new Ensures();
          // std::wstring matched_wstring = Match[1].str();
          // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
          // std::string matched_string = converter.to_bytes(matched_wstring);
          //auto matched_string = Match[1].str();
          //llvm::outs() << matched_string << "\n"; 
          //NewEnsures->Ann = matched_string;
          NewEnsures->Ann = Match;
          FDefn->Annotation.push_back(NewEnsures);
          break;
         }
         case PulseAnnKind::Returns:{
          Returns * ReturnSpec = new Returns(); 
          ReturnSpec->Ann = Match;
          FDefn->Annotation.push_back(ReturnSpec);
          break;
         }
         case PulseAnnKind::IsArray:
         case PulseAnnKind::Invariants:
         case PulseAnnKind::LemmaStatement:
         default:
           assert(false && "Did not expect specs other than requires and ensures attached \
                             to function declaration!\n");
           break;
         }
      }
         }
      }
    }
  }
  
  // exit(0);

  // for (unsigned i = 0; i < functionDecl->getNumParams(); ++i) {
  // clang::ParmVarDecl *param = functionDecl->getParamDecl(i);
  // llvm::errs() << "Parameter: " << param->getNameAsString() << "\n";
  // }

  std::vector<Binder *> PulseArgs;
  for (unsigned i = 0; i < FD->getNumParams(); i++) {
    auto *Param = FD->getParamDecl(i);
    auto ParamName = Param->getNameAsString();

    //See if this array parameter has any annotations arrached to it.
    auto Attrs = Param->attrs();
    for (auto *Attr : Attrs){
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)){
        if (AnnotAttr->getAttrName()->getName() == "pulse"){
          auto AnnotationData = AnnotAttr->getAnnotation();
          std::string Match;
          auto PulseAnnotKind = getPulseAnnKindFromString(AnnotationData, Match);
          assert(PulseAnnotKind == PulseAnnKind::IsArray && "Only expect is array annotations for param decl atm.!\n");
          //Add type to mape.
          //Make a clang Array Type
          //Try to get element type
          if (!Param->getType()->isPointerType() && !Param->getType()->isArrayType()){
            assert(false && "Expected parameter to be a ref or array!\n");
          }
          
          QualType ElementType = Param->getType()->getPointeeType();

          if(!std::regex_match(Match, std::regex("[-+]?[0-9]+"))){
          
          // Step 2: Create a VarDecl for the size variable 'n'
          //We should check here is the length is a constant or of variable array type.
          IdentifierInfo &Id = Ctx.Idents.get(Match);
          VarDecl *SizeVar = VarDecl::Create(
            Ctx,
            Ctx.getTranslationUnitDecl(),
            SourceLocation(),
            SourceLocation(),
            &Id,
            Ctx.IntTy,
            nullptr,
            SC_Auto
            );
            
            // Step 3: Create a DeclRefExpr to refer to 'n'
            DeclRefExpr *SizeExpr = DeclRefExpr::Create(
              Ctx,
              NestedNameSpecifierLoc(),
              SourceLocation(),
              SizeVar,
              false,
              SourceLocation(),
              Ctx.IntTy,
              clang::Expr::getValueKindForType(ElementType)
            );
            
            // Step 4: Create the VLA type
            QualType VLAType = Ctx.getVariableArrayType(
              ElementType,
              SizeExpr,
              ArraySizeModifier::Normal,
              0
            );
            // llvm::outs() << "Print the element type here!!!\n"; 
            // llvm::outs() << QualType(VLAType->getPointeeOrArrayElementType(), 0);
            // llvm::outs() << "End of element type!" << "\n";
            // exit(1);
            DeclTyMap.insert(std::make_pair(Param, VLAType));
          }
          else {
            clang::QualType ConstArrayTy = Ctx.getConstantArrayType(
              ElementType,
              llvm::APInt(32, std::stoi(Match)),
              nullptr,
              ArraySizeModifier::Normal,
              0
            );
            DeclTyMap.insert(std::make_pair(Param, ConstArrayTy));

          }
          
        }

      }
    }
    
    FStarType *ParamTy;
    auto It = DeclTyMap.find(Param);
    if (It != DeclTyMap.end()) {
      //Get the qualification
      auto Ty = It->second;
      if (Ty->isArrayType() || Ty->isConstantArrayType() || Ty->isVariableArrayType()){
        auto *FArrTy = new FStarArrType(); 
        FArrTy->ElementType = getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        ParamTy = FArrTy;
      }
      else {
        ParamTy = getPulseTyFromCTy(Param->getType()); 
      }
    } else {
       InferDeclType(Param, FD);
       auto It = DeclTyMap.find(Param);
       if (It != DeclTyMap.end()) {
        auto Ty = It->second;
        auto *FArrTy = new FStarArrType(); 
        FArrTy->ElementType = getPulseTyFromCTy(QualType(Ty->getPointeeOrArrayElementType(), 0));
        auto *CTyKeyStr = lookupSymbol(SymbolTable::Array);
        FArrTy->setName(CTyKeyStr);
        ParamTy = FArrTy;

       }
       else{
        ParamTy = getPulseTyFromCTy(Param->getType());
       }
    }
    
    //auto CParamType = Param->getType();
    //FStarType *ParamTy = getPulseTyFromCTy(CParamType);

    auto *Binder = new struct Binder(ParamName, ParamTy);
    PulseArgs.push_back(Binder);
  }
  FDefn->Args = PulseArgs;
  //extractPulseAnnotations(FD, SM, FDefn->Annotation);

  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      ExprMutationAnalyzer Analyzer(*CS, Ctx);
      auto *PulseBody = pulseFromCompoundStmt(CS, &Analyzer);
      // PulseBody->printTag();
      if (PulseBody != nullptr) 
        PulseBody->dumpPretty();
      FDefn->Body = PulseBody;
    }
  }

  PulseFnDefn *PulseFn = new PulseFnDefn(FDefn);

  // llvm::outs() << PulseFn->Defn->Name << "\n";
  llvm::outs() << "=================================================";
  llvm::outs() << "\nPrint the Pulse function Definition:\n\n";
  PulseFn->dumpPretty();
  llvm::outs() << "\nEnd printing the function Definition\n\n";
  llvm::outs() << "=================================================\n";
  PulseFn->Kind = PulseFnKind::FnDefn;
  FunctionDeclarations.push_back(PulseFn);

  return true;
}

FStarType *PulseVisitor::getPulseTyFromCTy(clang::QualType CType) {
  // TODO: Check if Ctype is a pointer type, if so, use FStarPointerType.

  FStarType *PulseTy;
  if (CType->isPointerType()) {

    if (CType->isArrayType()){
      assert(false && "PulseVisitor: Did not implement array type in clang.\n");
    }

    PulseTy = new FStarPointerType();
    auto *PulsePointerTy = static_cast<FStarPointerType *>(PulseTy);
    PulsePointerTy->setName(CType.getAsString());
    PulsePointerTy->setTag(TermTag::FStarPointerType);
    auto UnderLyingType = CType->getPointeeType();
    auto *FStartUnderLyingType = getPulseTyFromCTy(UnderLyingType);
    PulsePointerTy->setPointerToTy(FStartUnderLyingType);
    return PulsePointerTy;
  }

  PulseTy = new FStarType();
  auto CTyKey = getSymbolKeyForCType(CType, Ctx);
  auto *CTyKeyStr = lookupSymbol(CTyKey);
  PulseTy->setName(CTyKeyStr);
  PulseTy->setTag(TermTag::FStarType);
  return PulseTy;
} 

PulseStmt *PulseVisitor::pulseFromCompoundStmt(Stmt *S, ExprMutationAnalyzer *Analyzer) {

  PulseSequence *Stmt = nullptr;
  PulseSequence *Head = nullptr;
  if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    for (auto *InnerStmt : CS->body()) {
      // auto *NextPulseStmt = pulseFromStmt(InnerStmt, Analyzer);
      // if (Stmt == nullptr) {
      //   Stmt = NextPulseStmt;
      // } else {
      //   auto *NewSequence = new PulseSequence();
      //   NewSequence->setTag(PulseStmtTag::Sequence);
      //   NewSequence->assignS1(Stmt);
      //   NewSequence->assignS2(NextPulseStmt);
      //   Stmt = NewSequence;
      // }

      auto *NextPulseStmt = pulseFromStmt(InnerStmt, Analyzer);
      if (NextPulseStmt == nullptr)
        continue;

      if (Stmt == nullptr){
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

PulseStmt *PulseVisitor::pulseFromStmt(Stmt *S, ExprMutationAnalyzer *Analyzer) {

  if (!S)
      return nullptr;

  if (auto *DS = dyn_cast<DeclStmt>(S)) {

    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {

        if (auto *Init = VD->getInit()) {
          auto VarName = VD->getNameAsString();
          // Unsure if we really need the type here.
          // Though it may be usefuel checking invalid casting operations.
          auto VarType = VD->getType();
          
          //const Stmt *ConstS = S;
          //ExprMutationAnalyzer Eval(*ConstS, Ctx);

          // This gets converted to the pulse let expression.
          // Vidush : It is probably good to make a setter / pass arguments to
          // the constructor.
          auto *PulseLet = new LetBinding();
          PulseLet->VarName = VarName;
          
          //Don't forget to release these exprs.
          SmallVector<PulseStmt*> NewExprs;
          
          PulseLet->LetInit = getTermFromCExpr(Init, Analyzer, NewExprs, VD->getType());
          PulseLet->setTag(PulseStmtTag::LetBinding);
          if (Analyzer->isMutated(D)){
            PulseLet->Qualifier = MutOrRef::MUT;
          }

          //We need to make a sequence of pulse statements.
          PulseSequence *Start = nullptr;
          if (!NewExprs.empty()){
            
            for (size_t Idx = 0; Idx < NewExprs.size(); Idx++){    
              if (Start == nullptr){
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

          if (Start != nullptr){
              Start->assignS2(PulseLet);

              //check for any lemmas to be released. 
              auto Attrs = VD->attrs();
              for (auto *Att : Attrs){
                if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)){
                  if (AnnonAttr->getAttrName()->getName() == "pulse"){
                    std::string Match; 
                    auto AnnKind = getPulseAnnKindFromString(AnnonAttr->getAnnotation(), Match);
                    assert((AnnKind == PulseAnnKind::LemmaStatement) && "Expected a Lemma statement!\n");
                    auto *LS = new LemmaStatement(); 
                    LS->Lemma = Match; 
                    auto *LSE = new PulseExpr();
                    LSE->E = LS;
                    llvm::outs() << "Found Lemma: " << "\n";
                    llvm::outs() << Match << "\n";
                    llvm::outs() << "End.\n";
                    auto *NewS = new PulseSequence(); 
                    NewS->assignS1(LSE);
                    NewS->assignS2(Start);
                    Start = NewS;
                  }
                }
              }
              return Start;
          }

          //check for any lemmas to be released. 
              auto Attrs = VD->attrs();
              for (auto *Att : Attrs){
                if (AnnotateAttr *AnnonAttr = dyn_cast<AnnotateAttr>(Att)){
                  if (AnnonAttr->getAttrName()->getName() == "pulse"){
                    std::string Match; 
                    auto AnnKind = getPulseAnnKindFromString(AnnonAttr->getAnnotation(), Match);
                    assert((AnnKind == PulseAnnKind::LemmaStatement) && "Expected a Lemma statement!\n");
                    auto *LS = new LemmaStatement(); 
                    LS->Lemma = Match;
                    llvm::outs() << "Found Lemma: " << "\n";
                    llvm::outs() << Match << "\n";
                    llvm::outs() << "End.\n"; 
                    auto *LSE = new PulseExpr();
                    LSE->E = LS;
                    if (Start == nullptr){
                      auto *NewS = new PulseSequence(); 
                      NewS->assignS1(LSE);
                      Start = NewS;
                    }
                    Start->assignS2(LSE);
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
      auto BinaryOp = BO->getOpcode();

      // We use := in pulse to update references directly.
      if (UnaryOperator *UO = dyn_cast<UnaryOperator>(Lhs)) {

        // llvm::outs() << "Print in Assignment Unary: " << "\n";
        // UO->dumpPretty(Ctx);
        // UO->getSubExpr()->dumpPretty(Ctx);
        if (UO->getOpcode() == UO_Deref) {
          
          //TODO: Make sure to release these expressions
          SmallVector<PulseStmt*> ExprsBef;

          auto *PulseLhsTerm = getTermFromCExpr(UO->getSubExpr(), Analyzer, ExprsBef, BO->getType());
          auto *PulseRhsTerm = getTermFromCExpr(Rhs, Analyzer, ExprsBef, BO->getType());
          PulseAssignment *Assignment = new PulseAssignment();
          Assignment->setTag(PulseStmtTag::Assignment);
          Assignment->Lhs = PulseLhsTerm;
          Assignment->Value = PulseRhsTerm;

          assert(ExprsBef.empty() && "Expected expressions to be released!");

          return Assignment;
        }
        // assert(false && "Could not dyn cast to a declaration expression.");
      }
      else if (auto *ArrSub = dyn_cast<ArraySubscriptExpr>(Lhs)){
        
        //TODO: Make sure to release these expressions
        SmallVector<PulseStmt*> ExprsBef;

        auto *ArrayAssignExpr = new PulseArrayAssignment();
        ArrayAssignExpr->setTag(PulseStmtTag::ArrayAssignment);
        ArrayAssignExpr->Arr = getTermFromCExpr(ArrSub->getBase(), Analyzer, ExprsBef, BO->getType());
        ArrayAssignExpr->Index = getTermFromCExpr(ArrSub->getIdx(), Analyzer, ExprsBef, BO->getType());
        ArrayAssignExpr->Value = getTermFromCExpr(Rhs, Analyzer, ExprsBef, BO->getType());

        //We need to make a sequence of pulse statements.
        PulseSequence *Start = nullptr;
        if (!ExprsBef.empty()){
            
            for (size_t Idx = 0; Idx < ExprsBef.size(); Idx++){    
              if (Start == nullptr){
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


        if (Start != nullptr){
          Start->assignS2(ArrayAssignExpr);
          return Start;
        }


        return ArrayAssignExpr;
        S->dumpPretty(Ctx);
        assert(false && "Not implemented when Lhs is array sub expr");
      }
      // TODO:
      // We should generate Lets otherwise
      else {

        //TODO: Make sure to release these expressions
        SmallVector<PulseStmt*> ExprsBef;
      
        auto *PulseLhsTerm = getTermFromCExpr(Lhs, Analyzer, ExprsBef,BO->getType(), true);
        auto *PulseRhsTerm = getTermFromCExpr(Rhs, Analyzer, ExprsBef, BO->getType());
        PulseAssignment *Assignment = new PulseAssignment();
        Assignment->Lhs = PulseLhsTerm;
        Assignment->Value = PulseRhsTerm;
        Assignment->setTag(PulseStmtTag::Assignment);

        //We need to make a sequence of pulse statements.
        PulseSequence *Start = nullptr;
        if (!ExprsBef.empty()){
            
            for (size_t Idx = 0; Idx < ExprsBef.size(); Idx++){    
              if (Start == nullptr){
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


        if (Start != nullptr){
          Start->assignS2(Assignment);
          return Start;
        }

        return Assignment;
      }
    } else {
      
      SmallVector<PulseStmt *> ExprsBefore;
      
      auto *PExpr = new PulseExpr(); 
      PExpr->setTag(PulseStmtTag::Expr);
      auto ExprTerm = getTermFromCExpr(BO, Analyzer, ExprsBefore, BO->getType());
      if (!ExprTerm)
          return nullptr; 
        
      PExpr->E = ExprTerm;      
      
      //We need to make a sequence of pulse statements.
      PulseSequence *Start = nullptr;
      if (!ExprsBefore.empty()){

        for (size_t Idx = 0; Idx < ExprsBefore.size(); Idx++){    
          if (Start == nullptr){
            auto *Seq = new PulseSequence();
            Seq->assignS1(ExprsBefore[Idx]);
            Start = Seq; 
          }

          auto *NextSeq = new PulseSequence(); 
          NextSeq->assignS1(ExprsBefore[Idx]);
          Start->assignS2(NextSeq);
          Start = NextSeq;
        }

        //remove all released expressions.
        ExprsBefore.clear();
      }
      
      if (Start != nullptr){
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

    auto *PExprTerm = getTermFromCExpr(E, Analyzer, ExprsBefore, E->getType());

    if (PExprTerm == nullptr)
      return nullptr;
    
    PulseExpression->E = PExprTerm;

    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return PulseExpression;

    llvm::outs() << "\n\nPrint in pulseFromStmt Expr\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *IF = dyn_cast<IfStmt>(S)) {

    auto *Cond = IF->getCond();
    auto *Else = IF->getElse(); 
    auto *Then = IF->getThen();
    
    SmallVector<PulseStmt *> ExprsBefore;

    auto PulseCond = getTermFromCExpr(Cond, Analyzer, ExprsBefore, Cond->getType());
    auto *PulseElse = pulseFromStmt(Else, Analyzer);
    auto *PulseThen = pulseFromStmt(Then, Analyzer);

    auto PulseIfStmt = new PulseIf();
    PulseIfStmt->setTag(PulseStmtTag::If);
    PulseIfStmt->Head = PulseCond;
    PulseIfStmt->Else = PulseElse; 
    PulseIfStmt->Then = PulseThen;
    
    assert(ExprsBefore.empty() && "Expected expressions to be released!");

    return PulseIfStmt;

    llvm::outs() << "\n\nPrint in pulseFromStmt IfStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
    
    if (auto *RetVal = RS->getRetValue()){
      SmallVector<PulseStmt *> ExprsBefore;
      auto NewPulseExpr = new PulseExpr();
      NewPulseExpr->setTag(PulseStmtTag::Expr);
      auto *RetTerm = getTermFromCExpr(RetVal, Analyzer, ExprsBefore, RetVal->getType());
      if (RetTerm == nullptr)
        return nullptr;
      NewPulseExpr->E = RetTerm; 

      assert(ExprsBefore.empty() && "Expected expressions to be released!");

      return NewPulseExpr; 
    }

    return nullptr;

    llvm::outs() << "\n\nPrint in pulseFromStmt ReturnStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *FS = dyn_cast<ForStmt>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt ForStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *WS = dyn_cast<WhileStmt>(S)) {

    auto *WhileCond = WS->getCond(); 
    llvm::outs () << "Dump while condition" << "\n";
    WhileCond->dump();
    llvm::outs () << "End while condition dump" << "\n";
    auto *WhileBody = WS->getBody();

    llvm::outs () << "Dump while Body" << "\n";
    WhileBody->dump();
    llvm::outs () << "End while Body dump" << "\n";


    if (auto *AttrStmt = dyn_cast<AttributedStmt>(WhileBody)){
      llvm::outs() << "Found while invariant" << "\n";
      AttrStmt->getSubStmt()->dump();
      auto *CompundBody = AttrStmt->getSubStmt();

      auto Attributes = AttrStmt->getAttrs();
      auto *PulseWhile = new PulseWhileStmt(); 
      for (auto *Attr : Attributes){

        if (auto *AnnAttr = dyn_cast<AnnotateAttr>(Attr)){
          if (AnnAttr->getAttrName()->getName() == "pulse"){

            auto AnnotationData = AnnAttr->getAnnotation().str();
            
            std::string StartDelimiter = "invariants:";
            size_t pos = AnnotationData.find(StartDelimiter);
            std::string EndDelimiter = "|END";

            size_t end = AnnotationData.find(EndDelimiter);
            std::string match;
            if (pos != std::string::npos) {
            std::string firstPart = AnnotationData.substr(0, pos);
            match = AnnotationData.substr(pos +  StartDelimiter.length() , end - (pos + StartDelimiter.length()));            
            // if (pos != std::string::npos) {
            //   std::string firstPart = AnnotationData.substr(0, pos);
            //   match = AnnotationData.substr(pos + delimiter.length());
            //}
            }

            std::vector<std::string> tokens;
            std::stringstream ss(match);
            std::string token;
            while (std::getline(ss, token, ',')) {
              tokens.push_back(token.c_str());
            }

            llvm::outs() << "PRINT OUT the invarinat tokens\n";
            for (auto token : tokens){
              llvm::outs() << token << "\n";
              auto *NewLemmaTerm = new LemmaStatement();
              NewLemmaTerm->Lemma.assign(token.c_str());
              PulseWhile->Invariant.push_back(NewLemmaTerm);
            }
            llvm::outs() << "\n";

          }
        }

      }

      PulseWhile->setTag(PulseStmtTag::WhileStmt);
      PulseWhile->Guard = pulseFromStmt(WhileCond, Analyzer);
      PulseWhile->Body = pulseFromCompoundStmt(CompundBody, Analyzer);

      return PulseWhile;

      //exit(1);
    }
    else{
     
    auto *PulseWhile = new PulseWhileStmt(); 
    PulseWhile->setTag(PulseStmtTag::WhileStmt);

    // auto It = StmtToLemmas.find(WS);
    //Remove temporary for now
    //assert(it != MapExprToAssignedTemporary.end() && "Expression not found in map");
    // if (It == StmtToLemmas.end()) {
    //   // If not found, create a new temporary
    //   assert(false && "Key not found in map!\n");
    // }

    // PulseWhile->Invariant = It->second;

    PulseWhile->Guard = pulseFromStmt(WhileCond, Analyzer);
    PulseWhile->Body = pulseFromCompoundStmt(WhileBody, Analyzer);

    return PulseWhile;

    llvm::outs() << "\n\nPrint in pulseFromStmt WhileStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
    }
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt UnaryOperator\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *NS = dyn_cast<NullStmt>(S)) {
    return nullptr;
  } 
  else if (auto *CS = dyn_cast<CompoundStmt>(S)){
    return pulseFromCompoundStmt(CS, Analyzer);
  }
  else if (auto *AttrStmt = dyn_cast<AttributedStmt>(S)){

    auto *SubStmt = AttrStmt->getSubStmt();
    PulseSequence *NewSequence = nullptr;
    
    auto Attrs = AttrStmt->getAttrs();
    for (auto *Attr : Attrs){
      if (auto *AnnotAttr = dyn_cast<AnnotateAttr>(Attr)){
        if (AnnotAttr->getAttrName()->getName() == "pulse"){
          std::string Match;
          auto AttrKind = getPulseAnnKindFromString(AnnotAttr->getAnnotation(), Match);
          assert(AttrKind == PulseAnnKind::LemmaStatement && "Only Lemmas allowed in the middle of the body!");
          if (NewSequence == nullptr){
            auto *LS = new LemmaStatement(); 
            LS->Lemma = Match;
            auto *PE = new PulseExpr(); 
            PE->E = LS; 
            auto *NewSeq = new PulseSequence(); 
            NewSeq->assignS1(PE);
            NewSequence = NewSeq;
            continue;
          }
          auto *LS = new LemmaStatement(); 
          LS->Lemma = Match;
          auto *PE = new PulseExpr(); 
          PE->E = LS; 
          auto *NewSeq = new PulseSequence();
          NewSequence->assignS2(NewSeq);
          NewSequence = NewSeq;
        }
      }
    }
    NewSequence->assignS2(pulseFromStmt(SubStmt, Analyzer));
    return NewSequence;
  }
  else {
    llvm::outs() << "\n\nPrint in pulseFromStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  }

  return nullptr;
}

const clang::Stmt *getNextStatement(const clang::Expr *expr, clang::ASTContext &Context) {
    if (!expr) return nullptr;

    const clang::Stmt *parent = Context.getParents(*expr).begin()->get<clang::Stmt>();  // Get parent node
    if (!parent) return nullptr;

    bool foundExpr = false;
    for (const clang::Stmt *child : parent->children()) {
        if (foundExpr) return child;  // Return next statement after expr
        if (child == expr) foundExpr = true;
    }

    return nullptr;  // No next statement found
}

Term *PulseVisitor::getTermFromCExpr(Expr *E, ExprMutationAnalyzer *MutAnalyzer, 
  llvm::SmallVector<PulseStmt*> &ExprsBefore, QualType ParentType, bool isWrite) {

  if (auto *IL = dyn_cast<IntegerLiteral>(E)) {

    auto NewConstTerm = new ConstTerm(); 
    NewConstTerm->setTag(TermTag::Const);
    NewConstTerm->ConstantValue = std::to_string(IL->getValue().getSExtValue());
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

    if (Lhs->getType() != Rhs->getType()){
      E->dumpPretty(Ctx);
      LLVM_DEBUG(llvm::dbgs() << "\n");
      assert(false && "Expected types of Lhs and Rhs to be the same. \
              Unsafe type casting now allowed in Pulse\n");
    }
    
    SymbolTable TypeKey = getSymbolKeyForCType(Lhs->getType(), Ctx);
    auto OpKey = getSymbolKeyForOperator(TypeKey, Op);

    auto *NewAppENode = new AppE();
    NewAppENode->setTag(TermTag::AppE);
    auto LhsTerm = getTermFromCExpr(Lhs, MutAnalyzer, ExprsBefore, BO->getType());
    auto RhsTerm = getTermFromCExpr(Rhs, MutAnalyzer, ExprsBefore, BO->getType());
    
    auto *CallNameVar = new VarTerm();
    CallNameVar->setVarName(OpKey);
    CallNameVar->setTag(TermTag::Var);
    NewAppENode->setCallName(CallNameVar);
    NewAppENode->pushArg(LhsTerm);
    NewAppENode->pushArg(RhsTerm);

    //Wrap Call Expr into a Paren to be safe. 
    auto *NewParen = new Paren();
    NewParen->setInnerExpr(NewAppENode);

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
      auto *TermForBaseExpr = getTermFromCExpr(UO->getSubExpr(), MutAnalyzer, ExprsBefore, ParentType);
      DerefAppE->pushArg(TermForBaseExpr);

      //Wrap this deref in a parenthesis. 
      auto *Parenthesis = new Paren(); 
      Parenthesis->setInnerExpr(DerefAppE);
      return Parenthesis;

    } else {
      llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                      "UnaryOperator\n";
      E->dumpPretty(Ctx);
      llvm::outs() << "\nEnd printing term.\n\n";
      assert(false && "Expression not implemeted in getTermFromCExpr\n");
    }

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "UnaryOperator\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *CE = dyn_cast<CallExpr>(E)) {

      if (CE->getDirectCallee()->getNameAsString() == pulseProofTermFromC){
              llvm::outs() << "Encountered function: "; 
              llvm::outs() << CE->getDirectCallee()->getNameAsString() << "\n";
              llvm::outs() << "End encountered function.\n\n";

              auto NumArgs = CE->getNumArgs(); 
              assert(NumArgs == 1 && "Expected number of arguments for Pulse Proof Term to be 1!");
              auto *UserLemma = new Lemma(); 
              if (auto *ArgToString = dyn_cast<StringLiteral>(CE->getArg(0)->IgnoreCasts())){
                auto ArgString = ArgToString->getString();
                UserLemma->lemmas.push_back(ArgString.str());
              }
              else {
                 assert (false && "Expected pulse while to have arguments as string literals");
              }

              // Term *LemmaExpr = new PulseExpr(); 
              // LemmaExpr->E = UserLemma; 
              return UserLemma;
      }
      else if (CE->getDirectCallee()->getNameAsString() == pulseWhileInvariantFromC){
        
        llvm::outs() << "Encountered function: "; 
        llvm::outs() << CE->getDirectCallee()->getNameAsString() << "\n";
        llvm::outs() << "End encountered function.\n\n";

        auto NumArgs = CE->getNumArgs();
        
        std::vector<Slprop*> VectorLemmas;
        for (size_t Idx = 0; Idx < NumArgs; Idx++){
          auto *UserLemma = new Lemma(); 
          auto *Arg = CE->getArg(Idx);
          //assert that each argument is actually a string literal 
          if (auto *ArgToString = dyn_cast<StringLiteral>(Arg->IgnoreCasts())){
            auto ArgString = ArgToString->getString();
            UserLemma->lemmas.push_back(ArgString.str());
            Slprop *Prop = UserLemma;
            VectorLemmas.push_back(Prop);
            //assert that next statement is a while loop
          }
          else {
            assert (false && "Expected pulse while to have arguments as string literals");
          }
        }

        //Slprop *Lemma = UserLemmas;
        auto *Next = getNextStatement(E, Ctx);
        if (auto *While = dyn_cast<WhileStmt>(Next)){
                 //Add corresponding while invariant.
                 StmtToLemmas.insert(std::make_pair(While, VectorLemmas));
                 return nullptr;
        }
        assert(false && "Expected next statement after pulse invariant to be a while!\n");
      }

      auto *CallAppE = new AppE();
      auto *FuncName = new VarTerm();
      FuncName->setVarName(CE->getDirectCallee()->getNameAsString());
      FuncName->setTag(TermTag::Var);
      CallAppE->setTag(TermTag::AppE);
      CallAppE->setCallName(FuncName);

      for (size_t i = 0; i < CE->getNumArgs(); i++){
        auto *Arg = CE->getArg(i);
        auto *ArgTerm = getTermFromCExpr(Arg, MutAnalyzer, ExprsBefore, ParentType);
        CallAppE->pushArg(ArgTerm);
      }

      //Wrap Call expr in Paren Node

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
    return getTermFromCExpr(SubExpr, MutAnalyzer, ExprsBefore, ParentType);

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "ImplicitCastExpr\n";
    SubExpr->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {
    
    auto *DreDecl = DRE->getDecl();

    if (MutAnalyzer->isMutated(DreDecl) && !isWrite){

      //Create a new variable to be returned.
      //TODO: Vidush create a gensym for to get variable name.

      //std::string LetVar = DRE->getDecl()->getNameAsString(); //gensym("new");
      //VarTerm *VTermRet = new VarTerm();
      //VTermRet->setVarName(LetVar);

      //Need a let to extract its value
      //auto *LetForVariable = new LetBinding();
      //LetForVariable->VarName = LetVar;
      
      //Generate an AppE for the left hand side. 
      auto *InitAppE = new AppE();
      auto *CallName = new VarTerm(); 
      CallName->setVarName("!");
      InitAppE->setCallName(CallName);
      
      //The actual variable whose value we want 
      VarTerm *VTerm = new VarTerm();
      VTerm->setVarName(DRE->getDecl()->getNameAsString());

      InitAppE->pushArg(VTerm);

      //Wrap this AppE in a Parenthesis. 

      auto *PulseParenthesis = new Paren(); 
      PulseParenthesis->setInnerExpr(InitAppE);

      //LetForVariable->LetInit = LetInitAppE;

      //ExprsBefore.push_back(LetForVariable);

      //return VTermRet;
      return PulseParenthesis;
  
    }

    VarTerm *VTerm = new VarTerm();
    VTerm->setTag(TermTag::Var);
    VTerm->setVarName(DRE->getDecl()->getNameAsString());

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "DeclRefExpr\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    // assert(false && "Expression not implemeted in getTermFromCExpr\n");
    return VTerm;
  }
  else if (auto *ArrSubExpr = dyn_cast<ArraySubscriptExpr>(E)){
    auto *ArrBase = ArrSubExpr->getBase();
    auto ArrIdx = ArrSubExpr->getIdx();

    auto *PulseCall = new AppE(); 
    PulseCall->setTag(TermTag::AppE);
    auto *Call = new VarTerm(); 
    Call->setTag(TermTag::Var);
    Call->setVarName("op_Array_Access");
    PulseCall->setCallName(Call);
     
    PulseCall->pushArg(getTermFromCExpr(ArrBase, MutAnalyzer, ExprsBefore, ParentType));
    PulseCall->pushArg(getTermFromCExpr(ArrIdx, MutAnalyzer, ExprsBefore, ParentType));

    //wrap PulseCall in Paren
    auto *NewParen = new Paren(); 
    NewParen->setInnerExpr(PulseCall);

    return NewParen;
  }
  else if (auto *ParenExpr = dyn_cast<clang::ParenExpr>(E)){

    auto *ClangSubExpr = ParenExpr->getSubExpr();
    auto *PulseParenExpr = new Paren();

    auto *PulseSubExpr = getTermFromCExpr(ClangSubExpr, MutAnalyzer, ExprsBefore, ParentType);
    
    PulseParenExpr->setInnerExpr(PulseSubExpr);
    return PulseParenExpr;
  }
  else {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  }

  assert(false && "Should not reach here!");
  return nullptr;
}

PulseTransformer::PulseTransformer(
    std::vector<std::unique_ptr<ASTUnit>> &ASTList)
    : InternalAstList(ASTList) {
      
  // Initialize the rewriter with the first AST unit's context
  if (!ASTList.empty()) {
    RewriterForPlugin.setSourceMgr(ASTList[0]->getSourceManager(),
                                   ASTList[0]->getLangOpts());
  } else {
    llvm::errs() << "Error: No AST units provided for transformation.\n";
    exit(1);
  }
}

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
  NewPath += "/" + FileNameStr;

  // Vidush: Maybe add an assertion here that the extension is supposed to be .c

  std::string TempFilePath = NewPath.string() + ".fst";
  std::ofstream OutFile(TempFilePath);
  if (!OutFile.is_open()) {
    llvm::errs()
        << "Error: Failed to create temporary file for transformed code.\n";
  }

  CodeGen.writeHeaders(FileNameStr, OutFile);
  OutFile << getTransformedCode();
  OutFile.close();
  return TempFilePath;
}

void PulseTransformer::transform() {
  for (auto &AstCtx : InternalAstList) {
    PulseConsumer Consumer(AstCtx->getASTContext(), RewriterForPlugin);
    Consumer.HandleTranslationUnit(AstCtx->getASTContext());

    auto &FuncDecls = Consumer.getNewFunctionDeclarations();
    for (auto *FD : FuncDecls) {
      CodeGen.generateCodeFromPulseAst(FD);
    }
  }

  // clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
  // clang::FileID MainFileID = SM.getMainFileID();

  // if (!MainFileID.isValid()) {
  //   llvm::errs() << "Error: Invalid MainFileID—source file may not be loaded
  //   "
  //                   "correctly.\n";
  // }

  // // Capture rewritten buffer
  // const llvm::RewriteBuffer *Buffer =
  //     RewriterForPlugin.getRewriteBufferFor(MainFileID);

  // if (!Buffer) {
  //   llvm::errs()
  //       << "Warning: Rewriter buffer is empty—no modifications detected.\n";
  //   exit(1);
  // }

  // // Store transformed code in the class variable
  // TransformedCode = std::string(Buffer->begin(), Buffer->end());
}

std::string PulseTransformer::getTransformedCode() {
  return CodeGen.getGeneratedCode();
}