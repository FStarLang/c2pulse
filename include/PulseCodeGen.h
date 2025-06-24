#pragma once

#include "PulseIR.h"

#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <vector>

#define DEBUG_TYPE "pulse-codegen"

// namespace{

using CodegenStrTy = const char;
class PulseCodeGen {

public:
  std::string getGeneratedCodeForModule(std::string &ModuleName);

  std::map<std::string, std::unique_ptr<llvm::raw_string_ostream>> &
  returnOutPutModules();

  void generateCodeFromModule(std::string ModuleName, PulseModul *Modul);
  void generateCodeFromPulseAst(llvm::raw_string_ostream &S, PulseDecl *FD);
  std::string generateCodeFromTerm(llvm::raw_string_ostream &OS, Term *T);
  void generateCodeFromPulseStmt(llvm::raw_string_ostream &S, PulseStmt *T);
  std::string formatAsComments(PulseDecl *Decl);
  void writeHeaders(PulseModul *Modul, llvm::raw_string_ostream &Stream);

private:
  std::map<std::string, std::unique_ptr<llvm::raw_string_ostream>>
      OutputModules;
  // llvm::raw_string_ostream OS{OutputBuffer};
};

static class PulseSyntax {
public:
  CodegenStrTy *PulseAssignmentOpRef = ":=";
  CodegenStrTy *PulseLetAssignmentOpRef = "=";
  CodegenStrTy *PulseFunctionDeclaration = "fn";
  CodegenStrTy *PulseRecursiveFunctionDeclaration = "fn rec";
  CodegenStrTy *OpeningParenthesis = "(";
  CodegenStrTy *ClosingParenthesis = ")";
  CodegenStrTy *Colon = ":";
  CodegenStrTy *Semicolon = ";";
  CodegenStrTy *Reference = "ref";
  CodegenStrTy *Array = "array";
  CodegenStrTy *OpeningCurlyBrace = "{";
  CodegenStrTy *ClosingCurlyBrace = "}";
  CodegenStrTy *NewLine = "\n";
  CodegenStrTy *LetBind = "let";
  CodegenStrTy *LetMut = "let mut";
  CodegenStrTy *Space = " ";
  CodegenStrTy *LangPulse = "#lang-pulse";
  CodegenStrTy *ModuleSyntax = "module";
  CodegenStrTy *PulseInclude = "open Pulse";
  CodegenStrTy *PulseIf = "if";
  CodegenStrTy *PulseElse = "else";
  CodegenStrTy *PulseWhile = "while";
  CodegenStrTy *Dot = ".";
  CodegenStrTy *ArrAssignment = "<-";
  CodegenStrTy *Requires = "requires";
  CodegenStrTy *Ensures = "ensures";
  CodegenStrTy *Returns = "returns";
  CodegenStrTy *Val = "val";
  CodegenStrTy *Typ = "type";
} PulseSyntax;

// } //namespace