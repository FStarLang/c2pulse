#pragma once

#include <string>

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

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitBinaryOperator(clang::BinaryOperator *BO);
  bool VisitUnaryOperator(clang::UnaryOperator *UO);
  bool VisitCallExpr(clang::CallExpr *CE);
  bool VisitIntegerLiteral(clang::IntegerLiteral *IL);
  bool VisitFloatingLiteral(clang::FloatingLiteral *FL);
  bool VisitCharacterLiteral(clang::CharacterLiteral *CL);
  bool VisitStringLiteral(clang::StringLiteral *SL);
  bool VisitParenExpr(clang::ParenExpr *PE);
  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *ICE);
  bool VisitCStyleCastExpr(clang::CStyleCastExpr *CCE);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool VisitMemberExpr(clang::MemberExpr *ME);
  bool VisitConditionalOperator(clang::ConditionalOperator *CO);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  bool VisitCompoundLiteralExpr(clang::CompoundLiteralExpr *CLE);
  bool VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr *IVIE);
  bool VisitInitListExpr(clang::InitListExpr *ILE);
  bool VisitLambdaExpr(clang::LambdaExpr *LE);
  bool VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *SOAE);
  bool VisitCXXConstructExpr(clang::CXXConstructExpr *CCE);
  bool VisitCXXNewExpr(clang::CXXNewExpr *NewE);
  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr *DelE);

private:
  clang::ASTContext &Context;
  const clang::SourceManager &SM;
  std::string CurrentFunctionName = ""; 

  bool shouldProcess() const;
  void printExprInfo(const std::string &label, const clang::Expr *E);
  void printSourceLine(clang::SourceLocation loc);
  void prettyPrint(const clang::Expr *E);
};
