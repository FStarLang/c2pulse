#pragma once

#include "PulseCodeGen.h"
#include "PulseIR.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Analysis/Analyses/ExprMutationAnalyzer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <vector>

#undef DEBUG_TYPE
#define DEBUG_TYPE "pulse-codegen"

namespace {

/// Visitor class to traverse AST nodes and generate Pulse IR.
class PulseVisitor : public clang::RecursiveASTVisitor<PulseVisitor> {
public:
  PulseVisitor(clang::Rewriter &R, clang::ASTContext &Ctx)
      : TheRewriter(R), Ctx(Ctx), SM(Ctx.getSourceManager()) {}

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  // bool VisitTypedefDecl(clang::TypedefDecl *TypeDefDec);
  bool VisitRecordDecl(const clang::RecordDecl *RecordDecl);
  bool VisitVarDecl(clang::VarDecl *VD);

  PulseStmt *pulseFromCompoundStmt(clang::Stmt *S, clang::ExprMutationAnalyzer *A, PulseModul *Module);
  PulseStmt *pulseFromStmt(clang::Stmt *S, clang::ExprMutationAnalyzer *A,
                           clang::Stmt *Parent, PulseModul *Module,
                           clang::CompoundStmt *CS);
  FStarType *getPulseTyFromCTy(clang::QualType CType);
  Term *getTermFromCExpr(clang::Expr *E, clang::ExprMutationAnalyzer *A, llvm::SmallVector<PulseStmt*> &ExprsBef,
                           clang::Stmt *Parent, clang::QualType ParentType, PulseModul *Module, bool isWrite = false);
  std::vector<PulseDecl *> &getFunctionDeclarations();
  std::map<std::string, PulseModul *> &getPulseModules();
  void extractPulseAnnotations(const clang::FunctionDecl *FD,
                               const clang::SourceManager &SM,
                               std::vector<PulseExpr *> &Ann);

  void inferArrayTypesStmt(clang::Stmt *InnerStmt);
  void inferArrayTypesExpr(clang::Expr *ExprPtr);
  void inferDeclType(clang::Decl *Dec, clang::Stmt *InnerStmt);
  std::map<clang::Decl *, clang::QualType>
  inferArrayTypes(clang::FunctionDecl *FD);
  void InferDeclType(clang::Decl *Dec, clang::FunctionDecl *FD);
  bool checkIsRecursiveExpr(clang::Expr *ExprPtr,
                            clang::FunctionDecl *CurrFunction);
  bool checkIsRecursiveStmt(clang::Stmt *InnerStmt,
                            clang::FunctionDecl *CurrFunction);
  bool checkIsRecursiveFunction(clang::FunctionDecl *FD);
  std::string getNameForRecordDecl(const clang::RecordDecl *RD);

private:
  std::map<std::string, PulseModul *> Modules;
  clang::Rewriter &TheRewriter;
  clang::ASTContext &Ctx;
  clang::SourceManager &SM;
  std::map<clang::Decl*, clang::QualType> DeclTyMap;
  std::map<const clang::Stmt*, std::vector<Slprop*>> StmtToLemmas;
  std::set<const clang::Decl*> IsAllocatedOnHeap;
  std::map<clang::FunctionDecl*, PulseDecl*> DeclarationsMap;
  std::set<const clang::Decl*> TrackScopeOfStackAllocatedStructs;
  std::map<const clang::Decl*, std::pair<bool, bool>> TrackStructExplodeAndRecover;
  std::map<const clang::Decl *, std::string> RecordToRecordName;
};

} // end of anonymous namespace

class PulseConsumer : public clang::ASTConsumer {
public:
  PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(clang::ASTContext &Ctx) override;
  void setNewModules(std::map<std::string, PulseModul *> &PulseModules);

  std::map<std::string, PulseModul *> &getNewModules();

private:
  std::map<std::string, PulseModul *> Modules;
  PulseVisitor Visitor;
};

class PulseTransformer {
public:
  PulseTransformer(clang::ASTContext &Ctx);
  void transform();
  std::string writeToFile();

private:
  clang::ASTContext &AstCtx;
  PulseCodeGen CodeGen;
  clang::Rewriter RewriterForPlugin;
  std::string TransformedCode;
};