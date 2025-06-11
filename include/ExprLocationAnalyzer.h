#pragma once

#include <string>
#include <optional> 

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"


extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

#define DEBUG_TYPE "anf-pulse-rewriter"

class ExprLocationAnalyzer : public clang::RecursiveASTVisitor<ExprLocationAnalyzer> {
public:
  explicit ExprLocationAnalyzer(clang::ASTContext &Ctx);

  void analyze(clang::TranslationUnitDecl *TU);

  // Needed for extracting function metadata
  bool VisitFunctionDecl(clang::FunctionDecl *FD);

  // Needed for variable declarations like `int tmp = *r1;`
  bool VisitDeclStmt(clang::DeclStmt *DS);

  // Needed for assignments like `*r1 = *r2;`
  bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
  clang::ASTContext &Context;
  const clang::SourceManager &SM;
  std::string CurrentFunctionName = "";

  bool shouldProcess() const;

  void printExprInfo(const std::string &label, const clang::Expr *E);
  void printSourceLine(clang::SourceLocation loc);
};
