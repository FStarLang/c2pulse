#include "ExprLocationAnalyzer.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Interpreter/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "ast-loc-info"

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

SourceInfo::SourceInfo() { isValid = false; }

void SourceInfo::setLine(unsigned LineNum) { Line = LineNum; }

void SourceInfo::setColumn(unsigned ColumnNum) { Column = ColumnNum; }

clang::SourceLocation SourceInfo::getBeginLoc(){
  return range.getBegin();
}

clang::SourceLocation SourceInfo::getEndLoc(){
  return range.getEnd();
}

void SourceInfo::setBeginLoc(clang::SourceLocation b){
  range.setBegin(b);
}

void SourceInfo::setEndLoc(clang::SourceLocation e){
  range.setEnd(e);
}

bool SourceInfo::isSingleLocation(){

  if (range.getBegin() == range.getEnd()){
    return true;
  }

  return false;
}

void ExprLocationAnalyzer::dumpTokens(SourceRange Range) {
  SourceLocation B = Range.getBegin();
  SourceLocation E = Range.getEnd();
  if (B.isMacroID()) B = SM.getExpansionLoc(B);
  if (E.isMacroID()) E = SM.getExpansionLoc(E);

  bool invalid = false;
  StringRef Buffer = Lexer::getSourceText(CharSourceRange::getTokenRange(B, E), SM, Context.getLangOpts(), &invalid);
  if (invalid) {
    llvm::errs() << "Failed to get source text.\n";
    return;
  }

  // Ensure null-terminated buffer for Lexer constructor
  std::string NullTerminatedBuffer = Buffer.str(); 

  Lexer lexer(SM.getSpellingLoc(B), Context.getLangOpts(),
              NullTerminatedBuffer.c_str(), NullTerminatedBuffer.c_str(),
              NullTerminatedBuffer.c_str() + NullTerminatedBuffer.size());

  Token tok;
  std::map<unsigned, std::vector<std::string>> tokensByLine;

  while (!lexer.LexFromRawLexer(tok)) {
    if (tok.is(tok::eof)) break;

    SourceLocation loc = tok.getLocation();
    if (loc.isMacroID()) loc = SM.getExpansionLoc(loc);
    unsigned line = SM.getSpellingLineNumber(loc);
    unsigned col = SM.getSpellingColumnNumber(loc);
    std::string spelling = Lexer::getSpelling(tok, SM, Context.getLangOpts());
    std::string tokenInfo = "Token: " + std::string(tok.getName()) + " (" + spelling + ") at line " + std::to_string(line) + ", col " + std::to_string(col);
    // llvm::outs() << tokenInfo << "\n";
    tokensByLine[line].emplace_back(std::move(tokenInfo));
  }

  for (const auto &[line, tokens] : tokensByLine) {
    // Create a SourceLocation for the beginning of the line
    SourceLocation lineLoc = SM.translateLineCol(SM.getMainFileID(), line, 1);

    // Get the whole source line text
    bool invalidLine = false;
    StringRef lineText = Lexer::getSourceText(CharSourceRange::getCharRange(lineLoc, SM.getLocForEndOfFile(SM.getMainFileID())), SM, Context.getLangOpts(), &invalidLine);
    if (!invalidLine) {
      // Extract only the line content until newline
      std::string::size_type pos = lineText.find_first_of("\n\r");
      std::string singleLine = (pos == std::string::npos) ? std::string(lineText) : std::string(lineText.substr(0, pos));

      llvm::outs() << "Line " << line << ": " << singleLine << "\n";
    } else {
      llvm::outs() << "Line " << line << ": [Could not retrieve source line]\n";
    }

    for (const std::string &tok : tokens) {
      llvm::outs() << "  " << tok << "\n";
    }
  }
}


bool ExprLocationAnalyzer::VisitFunctionDecl(FunctionDecl *FD) {
  unsigned startLine = SM.getSpellingLineNumber(FD->getBeginLoc());
  unsigned endLine = SM.getSpellingLineNumber(FD->getEndLoc());

  DEBUG_WITH_TYPE(DEBUG_TYPE, { 
  llvm::outs() << "Function: " << CurrentFunctionName
               << " StartLine: " << startLine
               << " EndLine: " << endLine << "\n";

  llvm::outs() << "Params:";
  });
  
  for (unsigned i = 0; i < FD->getNumParams(); ++i) {
    ParmVarDecl *Param = FD->getParamDecl(i);
    QualType QT = Param->getType();
    
    DEBUG_WITH_TYPE(DEBUG_TYPE, { 
    llvm::outs() << " " << QT.getAsString() << " " << Param->getNameAsString();
    if (i != FD->getNumParams() - 1)
      llvm::outs() << ",";
    });
  }
  DEBUG_WITH_TYPE(DEBUG_TYPE, { 
  llvm::outs() << "\n";

  llvm::outs() << "Body:\n";
  

  if (Stmt *Body = FD->getBody()) {
    // SourceRange Range = Body->getSourceRange();
    SourceLocation startLoc = Body->getBeginLoc();
    SourceLocation endLoc = Body->getEndLoc();

    endLoc = Lexer::getLocForEndOfToken(endLoc, 0, SM, Context.getLangOpts());

    SourceRange fullRange(startLoc, endLoc);
    dumpTokens(fullRange);
  }
  });

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


    DEBUG_WITH_TYPE(DEBUG_TYPE, {   
    llvm::outs() << "Function: " << FuncName << "\n";
    });

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
  DEBUG_WITH_TYPE(DEBUG_TYPE, { 
  llvm::outs() << label << ": line " << lineNo
               << ", column " << colNo
               << ", type: " << QT.getAsString() << "\n";

  std::optional<std::string> lineText = getSourceLine(loc);
  if (lineText) {
    llvm::outs() << "    Source line: " << *lineText << "\n";
  } else {
    llvm::outs() << "    [Could not get source line]\n";
  }
 });
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

void SourceInfo::dumpPretty(clang::ASTContext &Ctx) {
  llvm::outs() << "------------------------------------------------------\n";
  llvm::outs() << "Pretty:    " << PrettyString << "\n";
  llvm::outs() << "Location:  Line " << Line << ", Column " << Column << "\n";
  llvm::outs() << "Type:      " << Type << "\n";
  llvm::outs() << "Source:    " << SourceLine << "\n";
  llvm::outs() << "Context:   " << Context << "\n";
  if (!Operation.empty())
      llvm::outs() << "Operation: " << Operation << "\n";
  if (range.isValid()) {
    llvm::outs() << "RangeInfo (Valid): ";
    range.print(llvm::outs(), Ctx.getSourceManager());
    llvm::outs() << "\n";
  }
  llvm::outs() << "------------------------------------------------------\n";
}

void ExprLocationAnalyzer::printNodeInfoMap() const {
  
  llvm::outs() << "-------------------------------\n";
  llvm::outs() << "Total nodes: " << NodeInfoMap.size() << "\n";
  llvm::outs() << "-------------------------------\n";
  for (const auto &entry : NodeInfoMap) {
    const SourceInfo &info = entry.second;
    llvm::outs() << "Pretty:    " << info.PrettyString << "\n";
    llvm::outs() << "Location:  Line " << info.Line << ", Column " << info.Column << "\n";
    llvm::outs() << "Type:      " << info.Type << "\n";
    llvm::outs() << "Source:    " << info.SourceLine << "\n";
    llvm::outs() << "Context:   " << info.Context << "\n";
    if (!info.Operation.empty())
      llvm::outs() << "Operation: " << info.Operation << "\n";
    llvm::outs() << "----------------------\n";
  }
}



static std::optional<std::string> getSourceLine(SourceLocation loc, SourceManager &SM) {
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


SourceInfo getSourceInfoFromExpr(clang::Expr *ExprNode, clang::ASTContext &Context, 
    std::string CtxString, std::string Op){
  
  auto &SM = Context.getSourceManager();
  
  auto SourceRange = ExprNode->getSourceRange();
  SourceLocation BeginLoc = SM.getExpansionLoc(SourceRange.getBegin());
  SourceLocation EndLoc = SM.getExpansionLoc(SourceRange.getEnd());

  unsigned BeginLine = SM.getExpansionLineNumber(BeginLoc);
  unsigned BeginCol = SM.getExpansionColumnNumber(BeginLoc);
  
  unsigned EndLine = SM.getExpansionLineNumber(EndLoc);
  unsigned EndCol = SM.getExpansionColumnNumber(EndLoc);

  unsigned line = SM.getExpansionLineNumber(BeginLoc);
  unsigned column = SM.getExpansionColumnNumber(BeginLoc);

  QualType QT = ExprNode->getType();

  auto PresumedLoc = SM.getPresumedLoc(BeginLoc);

  std::string pretty;
  llvm::raw_string_ostream rso(pretty);
  ExprNode->printPretty(rso, nullptr, Context.getPrintingPolicy());

  std::optional<std::string> srcLine = getSourceLine(BeginLoc, SM);

  SourceInfo info;
  info.isValid = true;
  info.PrettyString = rso.str();
  info.Line = line;
  info.Column = column;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = CtxString;
  info.Operation = Op;
  info.range = ExprNode->getSourceRange();
  info.StartLine = BeginLine;
  info.StartColumn = BeginCol;
  info.EndLine = EndLine;
  info.EndColumn = EndCol;
  if (PresumedLoc.isValid())
    info.FileName = PresumedLoc.getFilename();
  info.IsVerbatim = false;
  llvm::raw_string_ostream S(info.ClangAstNode);
  ExprNode->printPretty(S, nullptr, Context.getPrintingPolicy());
  return info;
}

SourceInfo getSourceInfoForToken(clang::SourceRange Start, unsigned Len,
                                 clang::ASTContext &Context,
                                 std::string CtxString,
                                 bool verbatim) {

  auto &SM = Context.getSourceManager();
  SourceLocation BeginLoc = Start.getBegin();
  SourceLocation EndLoc = Start.getEnd();

  unsigned BeginLine = SM.getSpellingLineNumber(BeginLoc);
  unsigned BeginCol = SM.getSpellingColumnNumber(BeginLoc);
  
  unsigned EndLine = SM.getSpellingLineNumber(EndLoc);
  unsigned EndCol = SM.getSpellingColumnNumber(EndLoc);

  unsigned line = SM.getSpellingLineNumber(BeginLoc);
  unsigned column = SM.getSpellingColumnNumber(BeginLoc);

  auto PresumedLoc = SM.getPresumedLoc(BeginLoc);

  QualType QT;

  std::optional<std::string> srcLine = getSourceLine(BeginLoc, SM);

  SourceInfo info;
  info.isValid = true;
  info.Line = line;
  info.Column = column;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = CtxString;
  info.Operation = "Attribute";
  info.range = Start;
  info.StartLine = BeginLine;
  info.StartColumn = BeginCol;
  info.EndLine = EndLine;
  info.EndColumn = EndCol;
  if (PresumedLoc.isValid())
    info.FileName = PresumedLoc.getFilename();
  info.IsVerbatim = verbatim;
  
  return info;
}

SourceInfo getSourceInfoFromAttr(const clang::Attr *AttrNode,
                                 clang::ASTContext &Context,
                                 std::string CtxString) {

  auto &SM = Context.getSourceManager();
  SourceRange Range = AttrNode->getRange();
  SourceLocation BeginLoc = SM.getExpansionLoc(Range.getBegin());
  SourceLocation EndLoc = SM.getExpansionLoc(Range.getEnd());

  unsigned BeginLine = SM.getExpansionLineNumber(BeginLoc);
  unsigned BeginCol = SM.getExpansionColumnNumber(BeginLoc);
  
  unsigned EndLine = SM.getExpansionLineNumber(EndLoc);
  unsigned EndCol = SM.getExpansionColumnNumber(EndLoc);

  unsigned line = SM.getExpansionLineNumber(BeginLoc);
  unsigned column = SM.getExpansionColumnNumber(BeginLoc);

  auto PresumedLoc = SM.getPresumedLoc(BeginLoc);

  QualType QT;

  std::string pretty;
  llvm::raw_string_ostream rso(pretty);
  AttrNode->printPretty(rso, Context.getPrintingPolicy());

  std::optional<std::string> srcLine = getSourceLine(BeginLoc, SM);

  SourceInfo info;
  info.isValid = true;
  info.PrettyString = rso.str();
  info.Line = line;
  info.Column = column;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = CtxString;
  info.Operation = "Attribute";
  info.range = AttrNode->getRange();
  info.StartLine = BeginLine;
  info.StartColumn = BeginCol;
  info.EndLine = EndLine;
  info.EndColumn = EndCol;
  if (PresumedLoc.isValid())
    info.FileName = PresumedLoc.getFilename();
  info.IsVerbatim = true;
  llvm::raw_string_ostream S(info.ClangAstNode);
  AttrNode->printPretty(S, Context.getPrintingPolicy());
  
  return info;
}

SourceInfo getSourceInfoFromStmt(clang::Stmt *StmtNode, clang::ASTContext &Context, 
    std::string CtxString, std::string Op){

  auto &SM = Context.getSourceManager();
  SourceRange Range = StmtNode->getSourceRange();
  SourceLocation BeginLoc = SM.getExpansionLoc(Range.getBegin());
  SourceLocation EndLoc = SM.getExpansionLoc(Range.getEnd());

  unsigned BeginLine = SM.getExpansionLineNumber(BeginLoc);
  unsigned BeginCol = SM.getExpansionColumnNumber(BeginLoc);
  
  unsigned EndLine = SM.getExpansionLineNumber(EndLoc);
  unsigned EndCol = SM.getExpansionColumnNumber(EndLoc);

  unsigned line = SM.getExpansionLineNumber(BeginLoc);
  unsigned column = SM.getExpansionColumnNumber(BeginLoc);

  auto PresumedLoc = SM.getPresumedLoc(BeginLoc);
  
  QualType QT;
  if (auto *Expr = llvm::dyn_cast<clang::Expr>(StmtNode)) {
    QT = Expr->getType();
  }

  std::string pretty;
  llvm::raw_string_ostream rso(pretty);
  StmtNode->printPretty(rso, nullptr, Context.getPrintingPolicy());

  std::optional<std::string> srcLine = getSourceLine(BeginLoc, SM);

  SourceInfo info;
  info.isValid = true;
  info.PrettyString = rso.str();
  info.Line = line;
  info.Column = column;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = CtxString;
  info.Operation = Op;
  info.range = StmtNode->getSourceRange();
  info.StartLine = BeginLine;
  info.StartColumn = BeginCol;
  info.EndLine = EndLine;
  info.EndColumn = EndCol;
  if (PresumedLoc.isValid())
    info.FileName = PresumedLoc.getFilename();
  info.IsVerbatim = false;
  llvm::raw_string_ostream S(info.ClangAstNode);
  StmtNode->printPretty(S, nullptr, Context.getPrintingPolicy());

  return info;
}

SourceInfo getSourceInfoFromDecl(const clang::Decl *Decl,
                                 clang::ASTContext &Context,
                                 std::string CtxString) {

  // Vidush: maybe we should change these to pointers.
  if (!Decl)
    return SourceInfo();

  auto &SM = Context.getSourceManager();
  SourceRange Range = Decl->getSourceRange();
  SourceLocation BeginLoc = SM.getSpellingLoc(Range.getBegin());
  SourceLocation EndLoc = SM.getSpellingLoc(Range.getEnd());

  unsigned BeginLine = SM.getSpellingLineNumber(BeginLoc);
  unsigned BeginCol = SM.getSpellingColumnNumber(BeginLoc);
  
  unsigned EndLine = SM.getSpellingLineNumber(EndLoc);
  unsigned EndCol = SM.getSpellingColumnNumber(EndLoc);

  auto PresumedLoc = SM.getPresumedLoc(BeginLoc);

  QualType QT;
  std::string pretty;
  llvm::raw_string_ostream rso(pretty);

  if (auto *VD = dyn_cast<VarDecl>(Decl)) {
    QT = VD->getType();
  } else if (auto *FD = dyn_cast<FieldDecl>(Decl)) {
    QT = FD->getType();
  } else if (auto *PD = dyn_cast<ParmVarDecl>(Decl)) {
    QT = PD->getType();
  } else if (auto *FunD = dyn_cast<FunctionDecl>(Decl)) {
    QT = FunD->getReturnType();
  } else if (auto *TD = dyn_cast<TypedefNameDecl>(Decl)) {
    QT = TD->getUnderlyingType();
  } else if (auto *ED = dyn_cast<EnumDecl>(Decl)) {
    QT = ED->getIntegerType();
  } else if (auto *TTP = dyn_cast<TemplateTypeParmDecl>(Decl)) {
    QT = TTP->getTypeForDecl()->getCanonicalTypeUnqualified();
  }

  Decl->print(rso, Context.getPrintingPolicy());

  std::optional<std::string> srcLine = getSourceLine(BeginLoc, SM);

  SourceInfo info;
  info.isValid = true;
  info.PrettyString = rso.str();
  info.Line = BeginLine;
  info.Column = BeginCol;
  info.Type = QT.getAsString();
  info.SourceLine = srcLine.value_or("[Unavailable]");
  info.Context = CtxString;
  info.Operation = "";
  info.range = Decl->getSourceRange();
  info.StartLine = BeginLine;
  info.StartColumn = BeginCol;
  info.EndLine = EndLine;
  info.EndColumn = EndCol;
  if (PresumedLoc.isValid())
    info.FileName = PresumedLoc.getFilename();
  info.IsVerbatim = false;
  llvm::raw_string_ostream S(info.ClangAstNode);
  Decl->print(S, Context.getPrintingPolicy());

  return info;
}
