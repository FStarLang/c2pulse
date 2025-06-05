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

// namespace{

using CodegenStrTy = const char;
class PulseCodeGen {

public:
  std::string getGeneratedCode() { return OS.str(); }

  void generateCodeFromPulseAst(PulseDecl *FD);
  std::string generateCodeFromTerm(Term *T);
  void generateCodeFromPulseStmt(PulseStmt *T);
  std::string formatAsComments(PulseDecl *Decl);

private:
  std::string OutputBuffer;
  llvm::raw_string_ostream OS{OutputBuffer};
};

static class PulseSyntax {
public:
  CodegenStrTy *PulseAssignmentOpRef = ":=";
  CodegenStrTy *PulseLetAssignmentOpRef = "=";
  CodegenStrTy *PulseFunctionDeclaration = "fn";
  CodegenStrTy *OpeningParenthesis = "(";
  CodegenStrTy *ClosingParenthesis = ")";
  CodegenStrTy *Colon = ":";
  CodegenStrTy *Semicolon = ";";
  CodegenStrTy *Reference = "ref";
  CodegenStrTy *OpeningCurlyBrace = "{";
  CodegenStrTy *ClosingCurlyBrace = "}";
  CodegenStrTy *NewLine = "\n";
  CodegenStrTy *LetBind = "let";
  CodegenStrTy *Space = " ";
} PulseSyntax;

// } //namespace