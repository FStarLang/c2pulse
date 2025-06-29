#include "ExprLocationAnalyzer.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "source-loc-map"

using namespace clang;

ExprLocationAnalyzer::ExprLocationAnalyzer(ASTContext &Ctx)
    : Context(Ctx), SM(Ctx.getSourceManager()) {}

void ExprLocationAnalyzer::analyze(TranslationUnitDecl *TU) {
  TraverseDecl(TU);
}

bool ExprLocationAnalyzer::TraverseFunctionDecl(FunctionDecl *FD){

  // Skip functions whithout a body, system headers, implicit, or macro functions not in main file
  if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()) ||
      FD->isImplicit() ||
      (FD->getLocation().isMacroID() &&
       !SM.isWrittenInMainFile(SM.getExpansionLoc(FD->getLocation())))) {
    return true; 
  }

  CurrentFunctionName = FD->getNameAsString();    

  if (!shouldProcess()) return true;

  return RecursiveASTVisitor::TraverseFunctionDecl(FD);
}

bool ExprLocationAnalyzer::VisitFunctionDecl(FunctionDecl *FD) {
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

  DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                        << "String: " << rso.str() 
                                        << " Line: " << line << "\n"); 

  for (auto *D : DS->decls()) {
    if (auto *VD = dyn_cast<VarDecl>(D)) {
      if (Expr *Init = VD->getInit()) {
        recordSourceInfo("Init of " + VD->getNameAsString(), Init, "");
        printExprInfo("  " + VD->getNameAsString(), Init);
      }
    }
  }

  return true;
}

const FunctionDecl *ExprLocationAnalyzer::getContainingFunction(const Stmt *S) const {
  const Stmt *Current = S;
  while (true) {
    auto Parents = Context.getParents(*Current);
    if (Parents.empty())
      return nullptr;

    // Check if any parent is FunctionDecl
    for (const auto &P : Parents) {
      if (const FunctionDecl *FD = P.get<FunctionDecl>()) {
        return FD;
      }
    }

    // If no FunctionDecl parent, go one level up the first Stmt parent
    const Stmt *Next = nullptr;
    for (const auto &P : Parents) {
      if (const Stmt *PS = P.get<Stmt>()) {
        Next = PS;
        break;
      }
    }
    if (!Next)
      return nullptr; // no further parent stmt found
    Current = Next;
  }
}


bool ExprLocationAnalyzer::VisitBinaryOperator(BinaryOperator *BO) {
  if (!shouldProcess()) return true;

  const FunctionDecl *FD = getContainingFunction(BO);
  if (!FD) {
    // Not inside any function — skip or handle accordingly
    return true;
  }
  std::string FuncName = FD->getNameAsString();

  if (!FunctionNameToProcess.empty() && FuncName != FunctionNameToProcess)
    return true; // skipping if function name doesn't match

  LLVM_DEBUG(llvm::outs() << "Function: " << FuncName << "\n";);

  unsigned line = SM.getSpellingLineNumber(BO->getBeginLoc());

  // Get the full expression string
  std::string str;
  llvm::raw_string_ostream rso(str);
  BO->printPretty(rso, nullptr, Context.getPrintingPolicy());

  // Get the binary operator string (e.g., "+", "=")
  StringRef OpStr = BinaryOperator::getOpcodeStr(BO->getOpcode());

  DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                          << "String: " << rso.str() 
                                          << " Line: " << line << "\n"
                                          << "  Operation: " << OpStr << "\n");                                       

  // Get string representation of LHS and RHS for labels
  std::string lhsStr;
  llvm::raw_string_ostream lhsOS(lhsStr);
  BO->getLHS()->printPretty(lhsOS, nullptr, Context.getPrintingPolicy());

  std::string rhsStr;
  llvm::raw_string_ostream rhsOS(rhsStr);
  BO->getRHS()->printPretty(rhsOS, nullptr, Context.getPrintingPolicy());


  // Print labeled LHS and RHS info
  printExprInfo("  LHS (" + lhsOS.str() + ")", BO->getLHS());
  printExprInfo("  RHS (" + rhsOS.str() + ")", BO->getRHS());
  

  recordSourceInfo("BinaryOperator", BO, OpStr.str());
  recordSourceInfo("LHS (" + lhsOS.str() + ")", BO->getLHS(), OpStr.str());
  recordSourceInfo("RHS (" + rhsOS.str() + ")", BO->getRHS(), OpStr.str());


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

  std::optional<std::string> lineText = getSourceLine(loc);
  if (lineText) {
    llvm::outs() << "    Source line: " << *lineText << "\n";
  } else {
    llvm::outs() << "    [Could not get source line]\n";
  }
}

std::optional<std::string> ExprLocationAnalyzer::getSourceLine(SourceLocation loc) {
  bool invalid = false;
  const char *bufferStart = SM.getCharacterData(loc, &invalid);
  if (invalid) {
    llvm::errs() << "  [Could not get source line]\n";
    return "";
  }

  FileID FID = SM.getFileID(loc);
  std::optional<llvm::MemoryBufferRef> BufRef = SM.getBufferOrNone(FID);
  if (!BufRef) {
    llvm::errs() << "Unable to get buffer\n";
    return "";
  }

  const char *fileStart = BufRef->getBufferStart();
  const char *fileEnd = BufRef->getBufferEnd();

  const char *lineStart = bufferStart;
  while (lineStart > fileStart && (*(lineStart - 1) != '\n' && *(lineStart - 1) != '\r'))
    --lineStart;

  const char *lineEnd = bufferStart;
  while (lineEnd < fileEnd && *lineEnd != '\n' && *lineEnd != '\r')
    ++lineEnd;

  std::string lineText(lineStart, lineEnd);
  return lineText;
}

bool ExprLocationAnalyzer::shouldProcess() const {
  return FunctionNameToProcess.empty() || FunctionNameToProcess == CurrentFunctionName;
}

void ExprLocationAnalyzer::recordSourceInfo(const std::string &context, const clang::Expr *E, const std::string &op) {
  if (!E) return;

  const Expr *ExprNode = E->IgnoreParenImpCasts();
  SourceLocation loc = SM.getSpellingLoc(ExprNode->getExprLoc());

  unsigned line = SM.getSpellingLineNumber(loc);
  unsigned column = SM.getSpellingColumnNumber(loc);
  QualType QT = ExprNode->getType();

  std::string pretty;
  llvm::raw_string_ostream rso(pretty);
  ExprNode->printPretty(rso, nullptr, Context.getPrintingPolicy());

  std::optional<std::string> srcLine = getSourceLine(loc);

  SourceInfo info;
  info.PrettyString = rso.str();
  info.Line = line;
  info.Column = column;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = context;
  info.Operation = op;

  NodeInfoMap[E] = std::move(info);
}

const  std::map<const clang::Stmt*, SourceInfo>  &ExprLocationAnalyzer::getNodeInfoMap() const {
  return NodeInfoMap;
}

void ExprLocationAnalyzer::printNodeInfoMap() const {
  for (const auto &entry : NodeInfoMap) {
    const SourceInfo &info = entry.second;
    llvm::outs() << "-------------------------------\n";
    llvm::outs() << "Pretty:    " << info.PrettyString << "\n";
    llvm::outs() << "Location:  Line " << info.Line << ", Column " << info.Column << "\n";
    llvm::outs() << "Type:      " << info.Type << "\n";
    llvm::outs() << "Source:    " << info.SourceLine << "\n";
    llvm::outs() << "Context:   " << info.Context << "\n";
    llvm::outs() << "Operation: " << info.Operation << "\n";
  }
}

