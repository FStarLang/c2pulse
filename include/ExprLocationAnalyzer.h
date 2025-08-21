#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Lexer.h"

#include "llvm/Support/CommandLine.h"

#include <string>
#include <map>
#include <optional>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

/// Struct that captures detailed source information for an AST node.
class SourceInfo {
  public:
    SourceInfo();
    bool isValid;
    std::string PrettyString; // Formatted string representation of the AST node
                              // or expression
    unsigned Line;   // Line number in the source code where the node/expression
                     // appears
    unsigned Column; // Column number in the source code for precise location
    std::string
        Type; // The data type of the expression or node (e.g., int, float)
    std::string SourceLine; // The full source code line text where the
                            // node/expression is located
    std::string Context; // Semantic or syntactic context describing the node's
                         // role (e.g., "RHS")
    std::string Operation; // Operation or operator associated with the node
                           // (e.g., "+", "=")

    clang::SourceRange
        range; // please refer to MacroCommentTracker.h
               // for details how I envision persisting the source location.
               // I think it is easier to get the begin and end, we know the
               // nodes that do not have a range, so you will get the
               // range.start() only I agree with other design decision but I
               // think it is easier to persist only the range.
    
    unsigned StartLine;   // Line number in the source code where the node/expression
                          // appears
    unsigned StartColumn; // Column number in the source code for precise location

    unsigned EndLine;   // Line number in the source code where the node/expression
                          // appears
    unsigned EndColumn; // Column number in the source code for precise location

    std::string FileName; 

    bool IsVerbatim;

    std::string ClangAstNode;

    void setLine(unsigned Line);
    void setColumn(unsigned Column);
    clang::SourceLocation getBeginLoc();
    clang::SourceLocation getEndLoc();
    void setBeginLoc(clang::SourceLocation b);
    void setEndLoc(clang::SourceLocation e);
    bool isSingleLocation();
    void dumpPretty(clang::ASTContext &Ctx);
};


SourceInfo getSourceInfoFromExpr(clang::Expr *ExprNode, clang::ASTContext &Context, 
    std::string CtxString, std::string Op);

SourceInfo getSourceInfoFromStmt(clang::Stmt *StmtNode,
                                 clang::ASTContext &Context,
                                 std::string CtxString, std::string Op);

SourceInfo getSourceInfoFromDecl(const clang::Decl *Decl,
                                 clang::ASTContext &Context,
                                 std::string CtxString);

SourceInfo getSourceInfoFromAttr(const clang::Attr *AttrNode,
                                 clang::ASTContext &Context,
                                 std::string CtxString);

SourceInfo getSourceInfoForToken(clang::SourceRange Start, unsigned Len,
                                 clang::ASTContext &Context,
                                 std::string CtxString,
                                 bool verbatim);

SourceInfo getSourceInfoFromFuncDecl(clang::FunctionDecl *S);
SourceInfo getSourceInfoFromRecordDecl(clang::RecordDecl *S);


class ExprLocationAnalyzer : public clang::RecursiveASTVisitor<ExprLocationAnalyzer> {
public:
  explicit ExprLocationAnalyzer(clang::ASTContext &Ctx);

  void analyze(clang::TranslationUnitDecl *TU);

  const clang::FunctionDecl *getContainingFunction(const clang::Stmt *S) const;

  // Dumps the tokens in the specified source range to the output
  void dumpTokens(clang::SourceRange Range);
  void dumpTokens2(clang::SourceRange Range);

  // Needed for controling whether we descend into the body
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);

  // Needed for extracting function metadata
  bool VisitFunctionDecl(clang::FunctionDecl *FD);

  // Needed for variable declarations like `int tmp = *r1;`
  bool VisitDeclStmt(clang::DeclStmt *DS);

  // Needed for assignments like `*r1 = *r2;`
  bool VisitBinaryOperator(clang::BinaryOperator *BO);
  
  const std::map<const clang::Stmt *, SourceInfo> &getNodeInfoMap() const;
  void printNodeInfoMap() const;

private:
  clang::ASTContext &Context;
  const clang::SourceManager &SM;
  std::string CurrentFunctionName = "";

  std::map<const clang::Stmt*, SourceInfo> NodeInfoMap;

  bool shouldProcess() const;
  void printExprInfo(const std::string &label, const clang::Expr *E);
  std::optional<std::string>  getSourceLine(clang::SourceLocation loc);
  void recordSourceInfo(const std::string &role, const clang::Expr *E, const std::string &op);
};