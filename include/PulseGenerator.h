#pragma once

#include "PulseCodeGen.h"
#include "PulseIR.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Analysis/Analyses/ExprMutationAnalyzer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include <memory>
#include <vector>

#define DEBUG_TYPE "pulse-gen"

using namespace clang;

namespace {

class PulseVisitor : public RecursiveASTVisitor<PulseVisitor> {

public:
  PulseVisitor(Rewriter &R, ASTContext &Ctx)
      : TheRewriter(R), Ctx(Ctx), SM(Ctx.getSourceManager()) {}

  bool VisitFunctionDecl(FunctionDecl *FD);
  bool VisitRecordDecl(RecordDecl *RD);
  bool VisitTypedefDecl(TypedefDecl *TypeDefDec);

  PulseStmt *pulseFromCompoundStmt(Stmt *S, ExprMutationAnalyzer *A);
  PulseStmt *pulseFromStmt(Stmt *S, ExprMutationAnalyzer *A);
  FStarType *getPulseTyFromCTy(QualType CType);
  Term *getTermFromCExpr(Expr *E, ExprMutationAnalyzer *A, llvm::SmallVector<PulseStmt*> &ExprsBef,
                           QualType ParentType, bool isWrite = false);
  std::vector<PulseDecl *> &getFunctionDeclarations();
  std::map<std::string, PulseModul *> &getPulseModules();
  void extractPulseAnnotations(const clang::FunctionDecl *FD,
                               const clang::SourceManager &SM,
                               std::vector<PulseExpr*> &Ann);

  void inferArrayTypesStmt(Stmt *InnerStmt);
  void inferArrayTypesExpr(Expr *ExprPtr);
  void inferDeclType(Decl *Dec, Stmt *InnerStmt);
  std::map<Decl*, QualType> inferArrayTypes(FunctionDecl *FD);
  void InferDeclType(Decl* Dec, FunctionDecl *FD);
  bool checkIsRecursiveExpr(Expr *ExprPtr, FunctionDecl *CurrFunction);
  bool checkIsRecursiveStmt(Stmt *InnerStmt, FunctionDecl *CurrFunction);
  bool checkIsRecursiveFunction(FunctionDecl *FD);

private:
  std::map<std::string, PulseModul *> Modules;
  // std::vector<PulseDecl *> FunctionDeclarations;
  Rewriter &TheRewriter;
  ASTContext &Ctx;
  SourceManager &SM;
  std::map<Decl*, QualType> DeclTyMap;
  std::map<const Stmt*, std::vector<Slprop*>> StmtToLemmas;
};
} // namespace

class PulseConsumer : public ASTConsumer {
public:
  PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(ASTContext &Ctx) override;

  // void
  // setNewFunctionDeclarations(std::vector<PulseDecl *> &FunctionDeclarations);

  void setNewModules(std::map<std::string, PulseModul *> &PulseModules);

  std::map<std::string, PulseModul *> &getNewModules();

  // std::vector<PulseDecl *> &getNewFunctionDeclarations();

private:
  // std::vector<PulseDecl *> FunctionDeclarations;
  std::map<std::string, PulseModul *> Modules;
  PulseVisitor Visitor;
};

class PulseTransformer {
public:
  PulseTransformer(ASTContext &Ctx); //std::vector<std::unique_ptr<ASTUnit>> &ASTList
  // std::string getTransformedCode();
  void transform();
  std::string writeToFile();

private:
  ASTContext &AstCtx;
  PulseCodeGen CodeGen;
  clang::Rewriter RewriterForPlugin;
  std::string TransformedCode;
  //std::vector<std::unique_ptr<ASTUnit>>
  //    &InternalAstList; // Store the ASTList for processing
};