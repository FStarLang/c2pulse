#include "ExprLocationAnalyzer.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/PrettyPrinter.h"

using namespace clang;

ExprLocationAnalyzer::ExprLocationAnalyzer(ASTContext &Ctx)
    : Context(Ctx), SM(Ctx.getSourceManager()) {}

void ExprLocationAnalyzer::analyze(TranslationUnitDecl *TU) {
  TraverseDecl(TU);
}

bool ExprLocationAnalyzer::VisitFunctionDecl(FunctionDecl *FD) {
  if (!FD->hasBody()) return true;
  CurrentFunctionName = FD->getNameAsString();
  return true;
}

bool ExprLocationAnalyzer::VisitVarDecl(VarDecl *VD) {
  if (!shouldProcess()) return true;
  if (!VD->hasInit()) return true;
  if (SM.isInSystemHeader(VD->getBeginLoc())) return true;

  // prettyPrint(VD);

  auto varLoc = SM.getSpellingLoc(VD->getBeginLoc());
  llvm::outs() << "[VarDecl] " << VD->getNameAsString()
               << " at line " << SM.getSpellingLineNumber(varLoc)
               << ", column " << SM.getSpellingColumnNumber(varLoc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitIntegerLiteral(IntegerLiteral *IL) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(IL->getBeginLoc())) return true;

  prettyPrint(IL);

  auto loc = SM.getSpellingLoc(IL->getBeginLoc());
  llvm::SmallString<32> str;
  IL->getValue().toString(str, 10, true, false);
  llvm::outs() << "[IntegerLiteral] " << str
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitFloatingLiteral(FloatingLiteral *FL) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(FL->getBeginLoc())) return true;

  prettyPrint(FL);

  auto loc = SM.getSpellingLoc(FL->getBeginLoc());
  llvm::outs() << "[FloatingLiteral] " << FL->getValue().convertToDouble()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitCharacterLiteral(CharacterLiteral *CL) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CL->getBeginLoc())) return true;

  prettyPrint(CL);

  auto loc = SM.getSpellingLoc(CL->getBeginLoc());
  llvm::outs() << "[CharacterLiteral] '" << (char)CL->getValue() << "'"
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitStringLiteral(StringLiteral *SL) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(SL->getBeginLoc())) return true;

  prettyPrint(SL);

  auto loc = SM.getSpellingLoc(SL->getBeginLoc());
  llvm::outs() << "[StringLiteral] \"" << SL->getString() << "\""
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitParenExpr(ParenExpr *PE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(PE->getExprLoc())) return true;

  prettyPrint(PE);

  auto loc = SM.getSpellingLoc(PE->getExprLoc());
  llvm::outs() << "[ParenExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[SubExpr]", PE->getSubExpr());
  return true;
}

bool ExprLocationAnalyzer::VisitUnaryOperator(UnaryOperator *UO) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(UO->getOperatorLoc())) return true;

  prettyPrint(UO);

  auto opLoc = SM.getSpellingLoc(UO->getOperatorLoc());
  llvm::outs() << "[UnaryOperator] " << UnaryOperator::getOpcodeStr(UO->getOpcode())
               << " at line " << SM.getSpellingLineNumber(opLoc)
               << ", column " << SM.getSpellingColumnNumber(opLoc) << "\n";

  printExprInfo("[SubExpr]", UO->getSubExpr());
  return true;
}

bool ExprLocationAnalyzer::VisitBinaryOperator(BinaryOperator *BO) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(BO->getOperatorLoc())) return true;

  prettyPrint(BO);

  auto opLoc = SM.getSpellingLoc(BO->getOperatorLoc());
  llvm::outs() << "[BinaryOperator] " << BO->getOpcodeStr()
               << " at line " << SM.getSpellingLineNumber(opLoc)
               << ", column " << SM.getSpellingColumnNumber(opLoc) << "\n";

  printExprInfo("[LHS]", BO->getLHS());
  printExprInfo("[RHS]", BO->getRHS());
  return true;
}

bool ExprLocationAnalyzer::VisitCallExpr(CallExpr *CE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CE->getExprLoc())) return true;

  prettyPrint(CE);

  auto callLoc = SM.getSpellingLoc(CE->getExprLoc());
  llvm::outs() << "[CallExpr] at line " << SM.getSpellingLineNumber(callLoc)
               << ", column " << SM.getSpellingColumnNumber(callLoc) << "\n";

  if (const FunctionDecl *Callee = CE->getDirectCallee()) {
    llvm::outs() << "  Callee: " << Callee->getNameAsString() << "\n";
  }

  for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
    printExprInfo("[Arg " + std::to_string(i) + "]", CE->getArg(i));
  }

  return true;
}

bool ExprLocationAnalyzer::VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(ICE->getExprLoc())) return true;

  prettyPrint(ICE);

  auto loc = SM.getSpellingLoc(ICE->getExprLoc());
  llvm::outs() << "[ImplicitCastExpr] kind=" << ICE->getCastKindName()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[SubExpr]", ICE->getSubExpr());
  return true;
}

bool ExprLocationAnalyzer::VisitCStyleCastExpr(CStyleCastExpr *CCE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CCE->getExprLoc())) return true;

  prettyPrint(CCE);

  auto loc = SM.getSpellingLoc(CCE->getExprLoc());
  llvm::outs() << "[CStyleCastExpr] to type " << CCE->getType().getAsString()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[SubExpr]", CCE->getSubExpr());
  return true;
}

bool ExprLocationAnalyzer::VisitDeclRefExpr(DeclRefExpr *DRE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(DRE->getBeginLoc())) return true;

  prettyPrint(DRE);

  auto loc = SM.getSpellingLoc(DRE->getBeginLoc());
  llvm::outs() << "[DeclRefExpr] " << DRE->getNameInfo().getName().getAsString()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitMemberExpr(MemberExpr *ME) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(ME->getExprLoc())) return true;

  prettyPrint(ME);

  auto loc = SM.getSpellingLoc(ME->getExprLoc());
  llvm::outs() << "[MemberExpr] " << ME->getMemberNameInfo().getName().getAsString()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Base]", ME->getBase());
  return true;
}

bool ExprLocationAnalyzer::VisitConditionalOperator(ConditionalOperator *CO) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CO->getExprLoc())) return true;

  prettyPrint(CO);

  auto loc = SM.getSpellingLoc(CO->getExprLoc());
  llvm::outs() << "[ConditionalOperator] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Cond]", CO->getCond());
  printExprInfo("[TrueExpr]", CO->getTrueExpr());
  printExprInfo("[FalseExpr]", CO->getFalseExpr());

  return true;
}

bool ExprLocationAnalyzer::VisitArraySubscriptExpr(ArraySubscriptExpr *ASE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(ASE->getExprLoc())) return true;

  prettyPrint(ASE);

  auto loc = SM.getSpellingLoc(ASE->getExprLoc());
  llvm::outs() << "[ArraySubscriptExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Base]", ASE->getBase());
  printExprInfo("[Idx]", ASE->getIdx());
  return true;
}

bool ExprLocationAnalyzer::VisitCompoundLiteralExpr(CompoundLiteralExpr *CLE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CLE->getExprLoc())) return true;

  prettyPrint(CLE);

  auto loc = SM.getSpellingLoc(CLE->getExprLoc());
  llvm::outs() << "[CompoundLiteralExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Initializer]", CLE->getInitializer());
  return true;
}

bool ExprLocationAnalyzer::VisitImplicitValueInitExpr(ImplicitValueInitExpr *IVIE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(IVIE->getExprLoc())) return true;

  prettyPrint(IVIE);

  auto loc = SM.getSpellingLoc(IVIE->getExprLoc());
  llvm::outs() << "[ImplicitValueInitExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitInitListExpr(InitListExpr *ILE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(ILE->getExprLoc())) return true;

  prettyPrint(ILE);

  auto loc = SM.getSpellingLoc(ILE->getExprLoc());
  llvm::outs() << "[InitListExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  for (unsigned i = 0; i < ILE->getNumInits(); ++i) {
    printExprInfo("[Init " + std::to_string(i) + "]", ILE->getInit(i));
  }
  return true;
}

bool ExprLocationAnalyzer::VisitLambdaExpr(LambdaExpr *LE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(LE->getBeginLoc())) return true;

  prettyPrint(LE);

  auto loc = SM.getSpellingLoc(LE->getBeginLoc());
  llvm::outs() << "[LambdaExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  // You can extend to print captures, body, etc.
  return true;
}

bool ExprLocationAnalyzer::VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *SOAE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(SOAE->getExprLoc())) return true;

  prettyPrint(SOAE);

  auto loc = SM.getSpellingLoc(SOAE->getExprLoc());
  llvm::outs() << "[UnaryExprOrTypeTraitExpr] " << (SOAE->getKind() == clang::UETT_SizeOf ? "sizeof" : "alignof")
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Argument]", SOAE->getArgumentExpr());
  return true;
}

bool ExprLocationAnalyzer::VisitCXXConstructExpr(CXXConstructExpr *CCE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(CCE->getExprLoc())) return true;

  prettyPrint(CCE);

  auto loc = SM.getSpellingLoc(CCE->getExprLoc());
  llvm::outs() << "[CXXConstructExpr] for type " << CCE->getType().getAsString()
               << " at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  for (unsigned i = 0; i < CCE->getNumArgs(); ++i) {
    printExprInfo("[Arg " + std::to_string(i) + "]", CCE->getArg(i));
  }
  return true;
}

bool ExprLocationAnalyzer::VisitCXXNewExpr(CXXNewExpr *NewE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(NewE->getExprLoc())) return true;

  prettyPrint(NewE);

  auto loc = SM.getSpellingLoc(NewE->getExprLoc());
  llvm::outs() << "[CXXNewExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  const clang::Type *Ty = NewE->getAllocatedTypeSourceInfo()->getTypeLoc().getTypePtr();
  llvm::outs() << "[Allocated Type] " << Ty->getCanonicalTypeInternal().getAsString() << "\n";

  return true;
}

bool ExprLocationAnalyzer::VisitCXXDeleteExpr(CXXDeleteExpr *DelE) {
  if (!shouldProcess()) return true;
  if (SM.isInSystemHeader(DelE->getExprLoc())) return true;

  prettyPrint(DelE);

  auto loc = SM.getSpellingLoc(DelE->getExprLoc());
  llvm::outs() << "[CXXDeleteExpr] at line " << SM.getSpellingLineNumber(loc)
               << ", column " << SM.getSpellingColumnNumber(loc) << "\n";

  printExprInfo("[Deleted Expr]", DelE->getArgument());
  return true;
}

bool ExprLocationAnalyzer::shouldProcess() const {
  return FunctionNameToProcess.empty() || FunctionNameToProcess == CurrentFunctionName;
}

void ExprLocationAnalyzer::prettyPrint(const clang::Expr *E){
  llvm::outs()<<"Dumping content of exp"<<"\n"; 
  clang::PrintingPolicy policy(Context.getLangOpts());
  std::string str;
  llvm::raw_string_ostream rso(str);
  E->printPretty(rso, nullptr, policy);
  llvm::outs() << rso.str() <<"\n";

}

void ExprLocationAnalyzer::printExprInfo(const std::string &label, const Expr *E) {
  if (!E) return;

  const auto *Expr = E->IgnoreParenImpCasts();
  auto loc = SM.getSpellingLoc(Expr->getExprLoc());

  unsigned lineNo = SM.getSpellingLineNumber(loc);
  unsigned colNo = SM.getSpellingColumnNumber(loc);

  llvm::outs() << "  " << label << " at line " << lineNo
               << ", column " << colNo << "\n";

  // printSourceLine(loc);
}

// void ExprLocationAnalyzer::printSourceLine(SourceLocation loc) {
//   bool invalid = false;
//   const char *bufferStart = SM.getCharacterData(loc, &invalid);
//   if (invalid) {
//     llvm::outs() << "  [Could not get source line]\n";
//     return;
//   }

//   FileID FID = SM.getFileID(loc);
//   llvm::Optional<llvm::MemoryBufferRef> BufferOrNone = SM.getBufferOrNone(FID);

//   if (!BufferOrNone) {
//     llvm::outs() << "  [Could not get source buffer]\n";
//     return;
//   }

//   const llvm::MemoryBufferRef &buffer = *BufferOrNone;

//   const char *fileStart = buffer->getBufferStart();
//   const char *fileEnd = buffer->getBufferEnd();

//   // Find start of line
//   const char *lineStart = bufferStart;
//   while (lineStart > fileStart && (*(lineStart - 1) != '\n' && *(lineStart - 1) != '\r')) {
//     --lineStart;
//   }

//   // Find end of line
//   const char *lineEnd = bufferStart;
//   while (lineEnd < fileEnd && *lineEnd != '\n' && *lineEnd != '\r') {
//     ++lineEnd;
//   }

//   std::string lineText(lineStart, lineEnd);
//   llvm::outs() << "    Source line: " << lineText << "\n";
// }

