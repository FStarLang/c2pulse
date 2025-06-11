#pragma once

#include "PulseCodeGen.h"
#include "PulseIR.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Analysis/Analyses/ExprMutationAnalyzer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <memory>
#include <vector>

using namespace clang;

namespace {

class PulseVisitor : public RecursiveASTVisitor<PulseVisitor> {

public:
  PulseVisitor(Rewriter &R, ASTContext &Ctx)
      : TheRewriter(R), Ctx(Ctx), SM(Ctx.getSourceManager()) {}

  bool VisitFunctionDecl(FunctionDecl *FD);
  PulseStmt *pulseFromCompoundStmt(Stmt *S, ExprMutationAnalyzer *A);
  PulseStmt *pulseFromStmt(Stmt *S, ExprMutationAnalyzer *A);
  FStarType *getPulseTyFromCTy(QualType CType);
  Term *getTermFromCExpr(Expr *E, ExprMutationAnalyzer *A, llvm::SmallVector<PulseStmt*> &ExprsBef,
                           QualType ParentType, bool isWrite = false);
  std::vector<PulseDecl *> &getFunctionDeclarations();
  void extractPulseAnnotations(const clang::FunctionDecl *FD,
                               const clang::SourceManager &SM,
                               std::vector<PulseExpr*> &Ann);

private:
  std::vector<PulseDecl *> FunctionDeclarations;
  Rewriter &TheRewriter;
  ASTContext &Ctx;
  SourceManager &SM;
};
} // namespace

class PulseConsumer : public ASTConsumer {
public:
  PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(ASTContext &Ctx) override;

  void
  setNewFunctionDeclarations(std::vector<PulseDecl *> &FunctionDeclarations);

  std::vector<PulseDecl *> &getNewFunctionDeclarations();

private:
  std::vector<PulseDecl *> FunctionDeclarations;
  PulseVisitor Visitor;
};

class PulseTransformer {
public:
  PulseTransformer(std::vector<std::unique_ptr<ASTUnit>> &ASTList);
  std::string getTransformedCode();
  void transform();
  std::string writeToFile();

private:
  PulseCodeGen CodeGen;
  clang::Rewriter RewriterForPlugin;
  std::string TransformedCode;
  std::vector<std::unique_ptr<ASTUnit>>
      &InternalAstList; // Store the ASTList for processing
};