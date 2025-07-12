#pragma once

#include "PulseIR.h"

#include "llvm/Support/raw_ostream.h"

#include <map>
#include <memory>
#include <set>
#include <string>

using CodegenStrTy = const char*;
using CodegenPyTy = std::unique_ptr<llvm::raw_string_ostream>;
class PulseCodeGen {

public:
  PulseCodeGen() = default;
  ~PulseCodeGen() {
    // emittedModules uses unique_ptr, auto-cleanup—no manual deletion needed
    emittedModules.clear();
    // alreadyEmittedModules is a value container—no manual deletion needed
    alreadyEmittedModules.clear();

    // raw pointers in allModulesByName must be deleted since we own them
    for (auto &pair : allModulesByName) {
      delete pair.second;
    }
    allModulesByName.clear();
  }

  std::map<std::string, CodegenPyTy> &getEmittedModules();
  std::string getGeneratedCodeForModule(std::string ModuleName);
  void generateCodeFromModule(const std::string ModuleName, PulseModul *Modul);
  void generateCodeFromPulseAST(llvm::raw_string_ostream &S, PulseDecl *FD, unsigned *RowCounter, unsigned *ColCounter);
  std::string generateCodeFromTerm(llvm::raw_string_ostream &OS, Term *T, unsigned *RowCounter, unsigned *ColCounter);
  void generateCodeFromPulseStmt(llvm::raw_string_ostream &S, PulseStmt *T, unsigned *RowCounter, unsigned *ColCounter);
  std::string formatAsComments(PulseDecl *Decl);
  void writeHeaders(PulseModul *pulseModule, llvm::raw_string_ostream &Stream, 
    unsigned *RowIdx);

private:
  std::map<std::string, CodegenPyTy> emittedModules;
  std::set<std::string> alreadyEmittedModules;
  std::map<std::string, PulseModul *> allModulesByName;
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
inline constexpr CodegenStrTy PulseInclude = "open Pulse\nopen Pulse.Lib.C";
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