#pragma once

#include "PulseIR.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
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

  PulseDecl *VisitFunctionDecl(FunctionDecl *FD);
  PulseStmt *pulseFromCompoundStmt(Stmt *S);
  PulseStmt *pulseFromStmt(Stmt *S);
  FStarType *getPulseTyFromCTy(QualType CType);

private:
  Rewriter &TheRewriter;
  ASTContext &Ctx;
  SourceManager &SM;
};
} // namespace

class PulseConsumer : public ASTConsumer {
public:
  PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(ASTContext &Ctx) override;

private:
  PulseVisitor Visitor;
};