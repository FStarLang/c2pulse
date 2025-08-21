#pragma once

#include "PulseCodeGen.h"
#include "PulseIR.h"
#include "MacroCommentTracker.h"

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
  PulseVisitor(clang::Rewriter &R, clang::ASTContext &Ctx,
    std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens)
      : TheRewriter(R), Ctx(Ctx), SM(Ctx.getSourceManager()), macroTokens(macroTokens) {}

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitFunctionDeclMain(clang::FunctionDecl *FD, bool OverrideGen);
  // bool VisitTypedefDecl(clang::TypedefDecl *TypeDefDec);
  bool VisitRecordDecl(const clang::RecordDecl *RecordDecl);
  bool VisitVarDecl(clang::VarDecl *VD);

  using VarTyEnv = std::map<Term *, FStarType *>;

  PulseStmt *pulseFromCompoundStmt(clang::Stmt *S, VarTyEnv VEnv,
                                   clang::ExprMutationAnalyzer *A,
                                   PulseModul *Module);

  std::pair<PulseStmt *, VarTyEnv> pulseFromStmt(clang::Stmt *S, VarTyEnv VEnv,
                                                 clang::ExprMutationAnalyzer *A,
                                                 clang::Stmt *Parent,
                                                 PulseModul *Module,
                                                 clang::CompoundStmt *CS);

  FStarType *getPulseTyFromCTy(clang::QualType CType);

  std::pair<Term *, VarTyEnv> getTermFromCExpr(
      clang::Expr *E, VarTyEnv VEnv, clang::ExprMutationAnalyzer *A,
      llvm::SmallVector<PulseStmt *> &ExprsBef, clang::Stmt *Parent,
      clang::QualType ParentType, PulseModul *Module, bool isWrite = false);

  std::vector<PulseDecl *> &getFunctionDeclarations();
  std::map<std::string, PulseModul *> &getPulseModules();
  void extractPulseAnnotations(const clang::FunctionDecl *FD,
                               const clang::SourceManager &SM,
                               std::vector<PulseExpr *> &Ann);

  void inferArrayTypesStmt(clang::Stmt *InnerStmt);
  void inferArrayTypesExpr(clang::Expr *ExprPtr);
  void inferDeclType(const clang::Decl *Dec, clang::Stmt *InnerStmt);
  std::map<clang::Decl *, clang::QualType>
  inferArrayTypes(clang::FunctionDecl *FD);
  void InferDeclType(const clang::Decl *Dec, clang::FunctionDecl *FD);
  bool checkIsRecursiveExpr(clang::Expr *ExprPtr,
                            clang::FunctionDecl *CurrFunction);
  bool checkIsRecursiveStmt(clang::Stmt *InnerStmt,
                            clang::FunctionDecl *CurrFunction);
  bool checkIsRecursiveFunction(clang::FunctionDecl *FD);
  std::string getNameForRecordDecl(const clang::RecordDecl *RD);
  void addArrayTy(std::string Match, const clang::Decl *ArrDecl, std::set<std::string> VEnv);
  FStarType *pulseTyFromDecl(const clang::Decl* D);
  bool checkAndAddIsArrayTy(const clang::AttrVec &Attrs, const clang::Decl* D, std::map<Term*, FStarType*> Env);
  bool isKnownArrayType(const clang::Decl *D);
  Term *checkAndAddCast(Term *Src, Term *Dst);
  clang::QualType getTypeFromDecl(const clang::Decl *D);
  bool checkDeclNameExists(std::string DeclName);

  void handleFunctionAttributes(clang::FunctionDecl *FD,
                                _PulseFnDefn *FDefn,
                                std::string FuncName,
                                PulseModul *Modul,
                                bool *Terminate,
                                bool HasAssociatedDefinition,
                                std::vector<Binder *> &PulseArgs, 
                                std::vector<Binder *> &ErasedArgs,
                                std::set<std::string> VEnv);

  PulseSequence * handleFunctionParameters(clang::FunctionDecl *FD, _PulseFnDefn *Defn, std::vector<Binder*> &PulseArgs, 
                                std::map<Term *, FStarType *> &TermToPulseTy, 
                                std::set<std::string> VEnvSet);

  std::pair<Term *, VarTyEnv> getPulseTermForMallocSize(
      clang::Expr *SizeExpr, VarTyEnv VEnv, clang::QualType ArrayElemType,
      clang::ExprMutationAnalyzer *A, llvm::SmallVector<PulseStmt *> &ExprsBef,
      clang::Stmt *Parent, clang::QualType ParentType, PulseModul *Module);

  std::pair<PulseStmt *, VarTyEnv>
  handleMallocs(clang::Expr *E, VarTyEnv VEnv, const clang::VarDecl *VD,
                clang::DeclStmt *DS, clang::Decl *D,
                clang::ExprMutationAnalyzer *A,
                llvm::SmallVector<PulseStmt *> &ExprsBef, clang::Stmt *Parent,
                clang::QualType ParentType, PulseModul *Module);

private:
  std::map<std::string, PulseModul *> Modules;
  clang::Rewriter &TheRewriter;
  clang::ASTContext &Ctx;
  clang::SourceManager &SM;
  bool ForceVisitFunction = false;
  std::map<const clang::Decl*, clang::QualType> DeclTyMap;
  std::map<const clang::Stmt*, std::vector<Slprop*>> StmtToLemmas;
  std::set<const clang::Decl*> IsAllocatedOnHeap;
  std::map<clang::Decl *, PulseDecl *> DeclEnv;
  std::set<const clang::Decl*> TrackScopeOfStackAllocatedStructs;
  std::map<const clang::Decl*, std::pair<bool, bool>> TrackStructExplodeAndRecover;
  std::map<const clang::Decl *, std::string> RecordToRecordName;
  std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens;
};

} // end of anonymous namespace

class PulseConsumer : public clang::ASTConsumer {
public:
  PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R,
    std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens);

  void HandleTranslationUnit(clang::ASTContext &Ctx) override;
  void setNewModules(std::map<std::string, PulseModul *> &PulseModules);

  std::map<std::string, PulseModul *> &getNewModules();

private:
  std::map<std::string, PulseModul *> Modules;
  PulseVisitor Visitor;
};

class PulseTransformer {
public:
  PulseTransformer(clang::ASTContext &Ctx,
        std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens);
  void transform();
  std::string writeToFile(std::optional<std::string> const& TmpDir);

private:
  clang::ASTContext &AstCtx;
  PulseCodeGen CodeGen;
  clang::Rewriter RewriterForPlugin;
  std::string TransformedCode;
  std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens;
};