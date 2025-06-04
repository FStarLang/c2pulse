#include "PulseGenerator.h"
#include "PulseEmitter.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

using namespace clang;

PulseConsumer::PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R)
    : Visitor(R, Ctx) {}

void PulseConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
}

PulseDecl *PulseVisitor::VisitFunctionDecl(FunctionDecl *FD) {
  if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()))
    return nullptr;

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

  // Always apply ANF rewriting on user functions
  if (Stmt *Body = FD->getBody()) {
    if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
      auto *PulseBody = pulseFromCompoundStmt(CS);
      FDefn->Body = PulseBody;
    }
  }

  PulseFnDefn *PulseFn = new PulseFnDefn(FDefn);

  llvm::outs() << PulseFn->Defn->Name << "\n";

  return PulseFn;
}

FStarType *PulseVisitor::getPulseTyFromCTy(clang::QualType CType) {
  // TODO: Check if Ctype is a pointer type, if so, use FStarPointerType.
  auto *PulseTy = new FStarType();
  PulseTy->setName(CType.getAsString());
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

      auto *PulseLhsTerm = getTermFromCExpr(Lhs);
      auto *PulseRhsTerm = getTermFromCExpr(Rhs);
      PulseAssignment *Assignment = new PulseAssignment();
      Assignment->Lhs = PulseLhsTerm;
      Assignment->Value = PulseRhsTerm;
      return Assignment;
    } else {
      assert(false && "Binary Operator not implemented in pulseFromStmt\n");
    }

  } else if (auto *E = dyn_cast<Expr>(S)) {
  } else if (auto *IF = dyn_cast<IfStmt>(S)) {
  } else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
  } else if (auto *FS = dyn_cast<ForStmt>(S)) {
  } else if (auto *WS = dyn_cast<WhileStmt>(S)) {
  } else if (auto *US = dyn_cast<UnaryOperator>(S)) {
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
  } else if (auto *FL = dyn_cast<FloatingLiteral>(E)) {
  } else if (auto *SL = dyn_cast<StringLiteral>(E)) {
  } else if (auto *CL = dyn_cast<CharacterLiteral>(E)) {
  } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {
  } else if (auto *UO = dyn_cast<UnaryOperator>(E)) {
  } else if (auto *CE = dyn_cast<CallExpr>(E)) {
  } else if (auto *IC = dyn_cast<ImplicitCastExpr>(E)) {

    auto *SubExpr = IC->getSubExpr();
    return getTermFromCExpr(SubExpr);

    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr "
                    "ImplicitCastExpr\n";
    SubExpr->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  } else {
    llvm::outs() << "\n\nPrint Expresion in PulseVisitor::getTermFromCExpr\n";
    E->dumpPretty(Ctx);
    llvm::outs() << "\nEnd printing term.\n\n";
    assert(false && "Expression not implemeted in getTermFromCExpr\n");
  }

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

  // Vidush: Maybe add an assertion here that the extension is supposed to be .c

  std::string TempFilePath =
      TempFilePathWithoutExtension.string() + ".transformed" + ".fst";
  std::ofstream OutFile(TempFilePath);
  if (!OutFile.is_open()) {
    llvm::errs()
        << "Error: Failed to create temporary file for transformed code.\n";
  }

  OutFile << TransformedCode;
  OutFile.close();
  return TempFilePath;
}

void PulseTransformer::transform() {
  for (auto &AstCtx : InternalAstList) {
    PulseConsumer Consumer(AstCtx->getASTContext(), RewriterForPlugin);
    Consumer.HandleTranslationUnit(AstCtx->getASTContext());
  }

  clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
  clang::FileID MainFileID = SM.getMainFileID();

  if (!MainFileID.isValid()) {
    llvm::errs() << "Error: Invalid MainFileID—source file may not be loaded "
                    "correctly.\n";
  }

  // Capture rewritten buffer
  const llvm::RewriteBuffer *Buffer =
      RewriterForPlugin.getRewriteBufferFor(MainFileID);

  if (!Buffer) {
    llvm::errs()
        << "Warning: Rewriter buffer is empty—no modifications detected.\n";
    exit(1);
  }

  // Store transformed code in the class variable
  TransformedCode = std::string(Buffer->begin(), Buffer->end());
}

std::string PulseTransformer::getTransformedCode() { return TransformedCode; }