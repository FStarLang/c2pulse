#pragma once

#include "PulseIR.h"

#include "llvm/Support/raw_ostream.h"

#include <map>
#include <memory>

using CodegenStrTy = const char *;
class PulseCodeGen {

public:
  std::string getGeneratedCodeForModule(std::string &ModuleName);

  std::map<const std::string, std::unique_ptr<llvm::raw_string_ostream>> &
  returnOutPutModules();

  void generateCodeFromModule(const std::string ModuleName, PulseModul *Modul);
  void generateCodeFromPulseAst(llvm::raw_string_ostream &S, PulseDecl *FD);
  std::string generateCodeFromTerm(llvm::raw_string_ostream &OS, Term *T);
  void generateCodeFromPulseStmt(llvm::raw_string_ostream &S, PulseStmt *T);
  std::string formatAsComments(PulseDecl *Decl);
  void writeHeaders(PulseModul *Modul, llvm::raw_string_ostream &Stream);

private:
  std::map<const std::string, std::unique_ptr<llvm::raw_string_ostream>>
      OutputModules;
};

static class PulseSyntax {
public:
  CodegenStrTy PulseAssignmentOpRef = ":=";
  CodegenStrTy PulseLetAssignmentOpRef = "=";
  CodegenStrTy PulseFunctionDeclaration = "fn";
  CodegenStrTy PulseRecursiveFunctionDeclaration = "fn rec";
  CodegenStrTy OpeningParenthesis = "(";
  CodegenStrTy ClosingParenthesis = ")";
  CodegenStrTy Colon = ":";
  CodegenStrTy Semicolon = ";";
  CodegenStrTy Reference = "ref";
  CodegenStrTy Array = "array";
  CodegenStrTy OpeningCurlyBrace = "{";
  CodegenStrTy ClosingCurlyBrace = "}";
  CodegenStrTy NewLine = "\n";
  CodegenStrTy LetBind = "let";
  CodegenStrTy LetMut = "let mut";
  CodegenStrTy Space = " ";
  CodegenStrTy LangPulse = "#lang-pulse";
  CodegenStrTy ModuleSyntax = "module";
  CodegenStrTy PulseInclude = "open Pulse";
  CodegenStrTy PulseIf = "if";
  CodegenStrTy PulseElse = "else";
  CodegenStrTy PulseWhile = "while";
  CodegenStrTy Dot = ".";
  CodegenStrTy ArrAssignment = "<-";
  CodegenStrTy Requires = "requires";
  CodegenStrTy Ensures = "ensures";
  CodegenStrTy Returns = "returns";
  CodegenStrTy Val = "val";
  CodegenStrTy Typ = "type";
} PulseSyntax;