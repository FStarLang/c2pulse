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
  std::ostringstream m_out;
  size_t m_line, m_col;

public:
  osstream_with_pos() : m_line(1), m_col(1) {}

  size_t line() const { return m_line; }
  size_t col() const { return m_col; }
  std::string str() const { return m_out.str(); }

  osstream_with_pos& operator<<(char const *str) {
    m_out << str;

    // update position information
    char const *nl;
    while (nl = strchr(str, '\n')) {
      m_line++;
      m_col = 1;
      str = nl + 1;
    }
    m_col += strlen(str);

    return *this;
  }

  osstream_with_pos& operator<<(std::string const& str) {
    return *this << str.c_str();
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
  std::string generateCodeFromTerm(osstream_with_pos &OS, Term *T);
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
// class NoPointerTextDumper : public clang::TextNodeDumper {
// public:
//   NoPointerTextDumper(llvm::raw_ostream &OS,
//                       const clang::ASTContext &Ctx)
//       : TextNodeDumper(OS, Ctx, true), OS(OS) {}

//   void dumpPointer(const void *Ptr) {
//     // Suppress pointer output
//     OS << "<ptr>";
//   }

// private:
//   llvm::raw_ostream &OS;
// };



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