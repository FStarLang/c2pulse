#include "ExprLocationAnalyzer.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/SourceManager.h"

using namespace clang;

ExprLocationAnalyzer::ExprLocationAnalyzer(ASTContext &Ctx)
    : Context(Ctx), SM(Ctx.getSourceManager()) {}

void ExprLocationAnalyzer::analyze(TranslationUnitDecl *TU) {
  TraverseDecl(TU);
}

bool ExprLocationAnalyzer::VisitFunctionDecl(FunctionDecl *FD) {
  if (!FD->hasBody()) return true;
  if (!shouldProcess()) return true;

  CurrentFunctionName = FD->getNameAsString();

  unsigned startLine = SM.getSpellingLineNumber(FD->getBeginLoc());
  unsigned endLine = SM.getSpellingLineNumber(FD->getEndLoc());

  llvm::outs() << "Function: " << CurrentFunctionName
               << " StartLine: " << startLine
               << " EndLine: " << endLine << "\n";

  llvm::outs() << "Params:";
  for (unsigned i = 0; i < FD->getNumParams(); ++i) {
    ParmVarDecl *Param = FD->getParamDecl(i);
    QualType QT = Param->getType();
    llvm::outs() << " " << QT.getAsString() << " " << Param->getNameAsString();
    if (i != FD->getNumParams() - 1)
      llvm::outs() << ",";
  }
  llvm::outs() << "\n";

  llvm::outs() << "Body:\n";

  return true;
}

bool ExprLocationAnalyzer::VisitDeclStmt(DeclStmt *DS) {
  if (!shouldProcess()) return true;

  std::string str;
  llvm::raw_string_ostream rso(str);
  DS->printPretty(rso, nullptr, Context.getPrintingPolicy());

  unsigned line = SM.getSpellingLineNumber(DS->getBeginLoc());
  llvm::outs() << "String: " << rso.str() << " Line: " << line << "\n";

  for (auto *D : DS->decls()) {
    if (auto *VD = dyn_cast<VarDecl>(D)) {
      if (Expr *Init = VD->getInit()) {
        printExprInfo("  " + VD->getNameAsString(), Init);
      }
    }
  }

  return true;
}

bool ExprLocationAnalyzer::VisitBinaryOperator(BinaryOperator *BO) {
  if (!shouldProcess()) return true;

  unsigned line = SM.getSpellingLineNumber(BO->getBeginLoc());
  std::string str;
  llvm::raw_string_ostream rso(str);
  BO->printPretty(rso, nullptr, Context.getPrintingPolicy());
  llvm::outs() << "String: " << rso.str() << " Line: " << line << "\n";

  printExprInfo("  LHS", BO->getLHS());
  printExprInfo("  RHS", BO->getRHS());

  return true;
}

// Shared logic
void ExprLocationAnalyzer::printExprInfo(const std::string &label, const Expr *E) {
  if (!E) return;

  const Expr *Expr = E->IgnoreParenImpCasts();
  auto loc = SM.getSpellingLoc(Expr->getExprLoc());

  unsigned lineNo = SM.getSpellingLineNumber(loc);
  unsigned colNo = SM.getSpellingColumnNumber(loc);

  QualType QT = Expr->getType();
  llvm::outs() << label << ": line " << lineNo
               << ", column " << colNo
               << ", type: " << QT.getAsString() << "\n";

  printSourceLine(loc);
}

void ExprLocationAnalyzer::printSourceLine(SourceLocation loc) {
  bool invalid = false;
  const char *bufferStart = SM.getCharacterData(loc, &invalid);
  if (invalid) {
    llvm::outs() << "  [Could not get source line]\n";
    return;
  }

  FileID FID = SM.getFileID(loc);
  llvm::Optional<llvm::MemoryBufferRef> BufferOrNone = SM.getBufferOrNone(FID);

  if (!BufferOrNone) {
    llvm::outs() << "  [Could not get source buffer]\n";
    return;
  }

  const llvm::MemoryBufferRef &buffer = *BufferOrNone;
  const char *fileStart = buffer.getBufferStart();
  const char *fileEnd = buffer.getBufferEnd();

  const char *lineStart = bufferStart;
  while (lineStart > fileStart && (*(lineStart - 1) != '\n' && *(lineStart - 1) != '\r'))
    --lineStart;

  const char *lineEnd = bufferStart;
  while (lineEnd < fileEnd && *lineEnd != '\n' && *lineEnd != '\r')
    ++lineEnd;

  std::string lineText(lineStart, lineEnd);
  llvm::outs() << "    Source line: " << lineText << "\n";
}

bool ExprLocationAnalyzer::shouldProcess() const {
  return FunctionNameToProcess.empty() || FunctionNameToProcess == CurrentFunctionName;
}
