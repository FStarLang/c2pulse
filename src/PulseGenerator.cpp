#include "PulseGenerator.h"
#include "PulseEmitter.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "llvm/Support/raw_ostream.h"
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

PulseStmt *PulseVisitor::pulseFromStmt(Stmt *S) { return nullptr; }

//  std::string rewriteCompound(Stmt *St, std::string AppendBefore = "") {

//     if (auto *CS = dyn_cast<CompoundStmt>(St)) {
//       std::string NewText = "{\n";
//       //Append any instructions if needed before.
//       NewText += AppendBefore;
//       for (Stmt *S : CS->body()) {
//         NewText += rewriteStmt(S);
//       }
//       NewText += "}\n";
//       return NewText;
//     }
//     else {
//       return rewriteStmt(St);
//     }
//   }