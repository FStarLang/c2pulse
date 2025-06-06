#include "PulseGenerator.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/Expr.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Stmt.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
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

void PulseVisitor::extractPulseAnnotations(
    const clang::FunctionDecl *FD, const clang::SourceManager &SM,
    std::vector<PulseAnnotation> &result) {

  // auto *Raw = FD->getASTContext().getRawCommentForAnyRedecl(FD);
  // llvm::outs() << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << "\n";
  // llvm::outs() << Raw;
  if (const auto *C = FD->getASTContext().getRawCommentForAnyRedecl(FD)) {
    std::string cleaned;
    for (char c : C->getRawText(SM))
      if (c != '\r')
        cleaned += c;

    std::istringstream in(cleaned);
    std::string line;
    std::regex reqRegex(R"(@requires\s+(.*))");
    std::regex ensRegex(R"(@ensures\s+(.*))");
    std::smatch match;

    while (std::getline(in, line)) {
      auto trimmed = StringRef(line).trim().str();
      if (std::regex_search(trimmed, match, reqRegex))
        result.push_back({PulseAnnKind::Requires, match[1], ""});
      else if (std::regex_search(trimmed, match, ensRegex))
        result.push_back({PulseAnnKind::Ensures, match[1], ""});
    }

    // llvm::outs() << "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << "\n";
    // llvm::outs() << result.back().predicate << "\n";
    // llvm::outs() << "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << "\n";
    
  }

  // int counter = 1;
  // for (auto& ann : result)
  //     ann.regionId = "'n" + std::to_string(counter++);
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

  // for (unsigned i = 0; i < functionDecl->getNumParams(); ++i) {
  // clang::ParmVarDecl *param = functionDecl->getParamDecl(i);
  // llvm::errs() << "Parameter: " << param->getNameAsString() << "\n";
  // }

  std::vector<Binder *> PulseArgs;
  for (unsigned i = 0; i < FD->getNumParams(); i++) {
    auto *Param = FD->getParamDecl(i);
    auto ParamName = Param->getNameAsString();
    auto CParamType = Param->getType();
    FStarType *ParamTy = getPulseTyFromCTy(CParamType);
    auto *Binder = new struct Binder(ParamName, ParamTy);
    PulseArgs.push_back(Binder);
  }
  FDefn->Args = PulseArgs;
  extractPulseAnnotations(FD, SM, FDefn->Annotation);

  // llvm::outs() << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP" << "\n";
  // for (auto &Ann : FDefn->Annotation) {
  //   llvm::outs() << Ann.predicate << "\n";
  // }
  // llvm::outs() << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP" << "\n";

  // Always apply ANF rewriting on user functions
  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      auto *PulseBody = pulseFromCompoundStmt(CS);
      // PulseBody->printTag();
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
  PulseTy->setName(CType.getAsString());
  PulseTy->setTag(TermTag::FStarType);
  return PulseTy;
}

PulseStmt *PulseVisitor::pulseFromCompoundStmt(Stmt *S) {

  PulseStmt *Stmt = nullptr;
  if (auto *CS = dyn_cast<CompoundStmt>(S)) {

    for (auto *InnerStmt : CS->body()) {
      auto *NextPulseStmt = pulseFromStmt(InnerStmt);
      if (Stmt == nullptr) {
        Stmt = NextPulseStmt;
      } else {
        auto *NewSequence = new PulseSequence();
        NewSequence->setTag(PulseStmtTag::Sequence);
        NewSequence->assignS1(Stmt);
        NewSequence->assignS2(NextPulseStmt);
        Stmt = NewSequence;
      }
    }
  }

  return Stmt;
}

PulseStmt *PulseVisitor::pulseFromStmt(Stmt *S) {

  if (auto *DS = dyn_cast<DeclStmt>(S)) {

    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {

        if (auto *Init = VD->getInit()) {
          auto VarName = VD->getNameAsString();
          // Unsure if we really need the type here.
          // Though it may be usefuel checking invalid casting operations.
          auto VarType = VD->getType();

          // This gets converted to the pulse let expression.
          // Vidush : It is probably good to make a setter / pass arguments to
          // the constructor.
          auto *PulseLet = new LetBinding();
          PulseLet->VarName = VarName;
          PulseLet->LetInit = getTermFromCExpr(Init);
          PulseLet->setTag(PulseStmtTag::LetBinding);
          return PulseLet;
        } else {
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
          auto *PulseLhsTerm = getTermFromCExpr(UO->getSubExpr());
          auto *PulseRhsTerm = getTermFromCExpr(Rhs);
          PulseAssignment *Assignment = new PulseAssignment();
          Assignment->setTag(PulseStmtTag::Assignment);
          Assignment->Lhs = PulseLhsTerm;
          Assignment->Value = PulseRhsTerm;
          return Assignment;
        }
        // assert(false && "Could not dyn cast to a declaration expression.");
      }
      // TODO:
      // We should generate Lets otherwise
      else {
        auto *PulseLhsTerm = getTermFromCExpr(Lhs);
        auto *PulseRhsTerm = getTermFromCExpr(Rhs);
        PulseAssignment *Assignment = new PulseAssignment();
        Assignment->Lhs = PulseLhsTerm;
        Assignment->Value = PulseRhsTerm;
        Assignment->setTag(PulseStmtTag::Assignment);
        return Assignment;
      }
    } else {
      assert(false && "Binary Operator not implemented in pulseFromStmt\n");
    }

  } else if (auto *E = dyn_cast<Expr>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt Expr\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *IF = dyn_cast<IfStmt>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt IfStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
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
    llvm::outs() << "\n\nPrint in pulseFromStmt WhileStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
    llvm::outs() << "\n\nPrint in pulseFromStmt UnaryOperator\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  } else if (auto *NS = dyn_cast<NullStmt>(S)) {
    return nullptr;
  } else {

    llvm::outs() << "\n\nPrint in pulseFromStmt\n";
    S->dumpPretty(Ctx);
    llvm::outs() << "\nEnd in pulseFromStmt.\n";
    assert(false && "Not implemented Clang expr in pulseFromStmt\n");
  }

  return nullptr;
}

Term *PulseVisitor::getTermFromCExpr(Expr *E) {

  if (auto *IL = dyn_cast<IntegerLiteral>(E)) {

    auto NewConstTerm = new ConstTerm(); 
    NewConstTerm->setTag(TermTag::Const);
    NewConstTerm->ConstantValue = IL->getValue().getSExtValue();
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
    auto LhsTerm = getTermFromCExpr(Lhs);
    auto RhsTerm = getTermFromCExpr(Rhs);
    
    auto *CallNameVar = new VarTerm();
    CallNameVar->setVarName(OpKey);
    CallNameVar->setTag(TermTag::Var);
    NewAppENode->setCallName(CallNameVar);
    NewAppENode->pushArg(LhsTerm);
    NewAppENode->pushArg(RhsTerm);

    return NewAppENode;

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
      auto *TermForBaseExpr = getTermFromCExpr(UO->getSubExpr());
      DerefAppE->pushArg(TermForBaseExpr);
      return DerefAppE;
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
    
      auto *CallAppE = new AppE();
      auto *FuncName = new VarTerm();
      FuncName->setVarName(CE->getDirectCallee()->getNameAsString());
      FuncName->setTag(TermTag::Var);
      CallAppE->setTag(TermTag::AppE);
      CallAppE->setCallName(FuncName);

      for (size_t i = 0; i < CE->getNumArgs(); i++){
        auto *Arg = CE->getArg(i);
        auto *ArgTerm = getTermFromCExpr(Arg);
        CallAppE->pushArg(ArgTerm);
      }
      
      return CallAppE;

    llvm::outs()
        << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr CallExpr\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *IC = dyn_cast<ImplicitCastExpr>(E)) {

    auto *SubExpr = IC->getSubExpr();
    return getTermFromCExpr(SubExpr);

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "ImplicitCastExpr\n";
    SubExpr->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {

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