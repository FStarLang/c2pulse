#pragma once

#include "PulseIR.h"
#include "PulseCodeGen.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTDumper.h"
#include "clang/AST/TextNodeDumper.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <tuple>

class osstream_with_pos {
private:
  std::ostringstream Str;
  size_t Line, Col;

public:
  osstream_with_pos() : Line(1), Col(1) {}

  size_t line() const { return Line; }
  size_t col() const { return Col; }
  std::string str() const { return Str.str(); }

  osstream_with_pos& operator<<(char const *InStr) {
    Str << InStr;

    // update position information
    char const *Nl;
    while (Nl = strchr(InStr, '\n')) {
      Line++;
      Col = 1;
      InStr = Nl + 1;
    }
    Col += strlen(InStr);

    return *this;
  }

  osstream_with_pos& operator<<(std::string const& InStr) {
    return *this << InStr.c_str();
  }
};

using CodegenStrTy = const char*;
using CodegenPyTy = std::unique_ptr<osstream_with_pos>;
using json = nlohmann::json;


class PulseCodeGen {

public:
  PulseCodeGen(clang::ASTContext &Ctx);
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
  void generateCodeFromPulseAST(osstream_with_pos &S, PulseDecl *FD);
  void generateCodeFromTerm(osstream_with_pos &OS, Term *T);
  void generateCodeFromPulseStmt(osstream_with_pos &S, PulseStmt *T);
  std::string formatAsComments(PulseDecl *Decl);
  void writeHeaders(PulseModul *pulseModule, osstream_with_pos &Stream);
  void printSourceLocations();
  void JsonifySourceRangeMap(std::string JsonOutputFile);
  void PulseSourceRangeToJson(json &J, const PulseSourceRange &Range);
  void SourceInfoToJson(json &J, const SourceInfo &Info);

private:
  std::map<std::string, CodegenPyTy> emittedModules;
  std::set<std::string> alreadyEmittedModules;
  std::map<std::string, PulseModul *> allModulesByName;
  std::vector<std::pair<PulseSourceRange, SourceInfo>> PulseLocsToCLocs;
  clang::ASTContext &ClangCtx;
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
inline constexpr CodegenStrTy SeqSeq = "Seq.seq";
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
inline constexpr CodegenStrTy IfExprThen = "then";
inline constexpr CodegenStrTy IfExprElse = "else";
inline constexpr CodegenStrTy PulseElse = "else";
inline constexpr CodegenStrTy PulseWhile = "while";
inline constexpr CodegenStrTy Dot = ".";
inline constexpr CodegenStrTy ArrAssignment = "<-";
inline constexpr CodegenStrTy Requires = "requires";
inline constexpr CodegenStrTy Preserves = "preserves";
inline constexpr CodegenStrTy Ensures = "ensures";
inline constexpr CodegenStrTy Returns = "returns";
inline constexpr CodegenStrTy Val = "val";
inline constexpr CodegenStrTy Typ = "type";
} // namespace PulseSyntax