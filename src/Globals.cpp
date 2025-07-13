#include "Globals.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/Twine.h"

using namespace std;

extern int GlobalGenSymCounter;

static string counterToVar(){
    return to_string(GlobalGenSymCounter++);
}


string gensym(const string base = "var") {
    return base + counterToVar();
}

void emitErrorWithLocationInternal(llvm::StringRef Message,
                                   const char *CallerFile, int CallerLine,
                                   const char *CallerFunc,
                                   const clang::ASTContext *Ctx,
                                   clang::SourceLocation Loc) {

  if (Ctx == nullptr) {
    llvm::errs() << "error: " << Message
                 << " (No source location to report!)\n";

    llvm::errs() << "[\n"
                 << "Triggered in File: " << CallerFile << "\n"
                 << "Triggered at line: " << CallerLine << "\n"
                 << "Triggered in function: " << CallerFunc << "\n"
                 << "]\n\n";

    llvm::report_fatal_error("Fatal Error ... Exiting!");
  }

  auto *SM = &Ctx->getSourceManager();

  clang::PresumedLoc PLoc = SM->getPresumedLoc(Loc);

  clang::SourceLocation Start = Loc;
  clang::SourceLocation End =
      clang::Lexer::getLocForEndOfToken(Start, 0, *SM, Ctx->getLangOpts());
  clang::CharSourceRange CharRange =
      clang::CharSourceRange::getCharRange(Start, End);
  clang::StringRef Text =
      clang::Lexer::getSourceText(CharRange, *SM, Ctx->getLangOpts());

  llvm::errs() << "Error: " << Message << "\n"
               << "[\n"
               << "Source Filename: " << PLoc.getFilename() << "\n"
               << "Source Line: " << PLoc.getLine() << "\n"
               << "Source Column: " << PLoc.getColumn() << "\n"
               << "C Expr: " << Text << "\n"
               << "]\n\n";

  llvm::errs() << "[\n"
               << "Triggered in File: " << CallerFile << "\n"
               << "Triggered at line: " << CallerLine << "\n"
               << "Triggered in function: " << CallerFunc << "\n"
               << "]\n\n";

  llvm::report_fatal_error("Fatal Error ... Exiting!");
}
