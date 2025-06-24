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
  getOutPutModules();

  void generateCodeFromModule(const std::string ModuleName, PulseModul *Modul);
  void generateCodeFromPulseAst(llvm::raw_string_ostream &S, PulseDecl *FD);
  std::string generateCodeFromTerm(llvm::raw_string_ostream &OS, Term *T);
  void generateCodeFromPulseStmt(llvm::raw_string_ostream &S, PulseStmt *T);
  std::string formatAsComments(PulseDecl *Decl);
  void writeHeaders(PulseModul *Modul, llvm::raw_string_ostream &Stream);

private:
  std::map<const std::string, std::unique_ptr<llvm::raw_string_ostream>>
      outputModules;
};

namespace PulseSyntax {
  inline constexpr CodegenStrTy PulseAssignmentOpRef = ":=";
  inline constexpr CodegenStrTy PulseLetAssignmentOpRef = "=";
  inline constexpr CodegenStrTy PulseFunctionDeclaration = "fn";
  inline constexpr CodegenStrTy PulseRecursiveFunctionDeclaration = "fn rec";
  inline constexpr CodegenStrTy OpeningParenthesis = "(";
  inline constexpr CodegenStrTy ClosingParenthesis = ")";
  inline constexpr CodegenStrTy Colon = ":";
  inline constexpr CodegenStrTy Semicolon = ";";
  inline constexpr CodegenStrTy Reference = "ref";
  inline constexpr CodegenStrTy Array = "array";
  inline constexpr CodegenStrTy OpeningCurlyBrace = "{";
  inline constexpr CodegenStrTy ClosingCurlyBrace = "}";
  inline constexpr CodegenStrTy NewLine = "\n";
  inline constexpr CodegenStrTy LetBind = "let";
  inline constexpr CodegenStrTy LetMut = "let mut";
  inline constexpr CodegenStrTy Space = " ";
  inline constexpr CodegenStrTy LangPulse = "#lang-pulse";
  inline constexpr CodegenStrTy ModuleSyntax = "module";
  inline constexpr CodegenStrTy PulseInclude = "open Pulse";
  inline constexpr CodegenStrTy PulseIf = "if";
  inline constexpr CodegenStrTy PulseElse = "else";
  inline constexpr CodegenStrTy PulseWhile = "while";
  inline constexpr CodegenStrTy Dot = ".";
  inline constexpr CodegenStrTy ArrAssignment = "<-";
  inline constexpr CodegenStrTy Requires = "requires";
  inline constexpr CodegenStrTy Ensures = "ensures";
  inline constexpr CodegenStrTy Returns = "returns";
  inline constexpr CodegenStrTy Val = "val";
  inline constexpr CodegenStrTy Typ = "type";
} // namespace PulseSyntax