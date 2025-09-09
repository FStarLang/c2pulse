#include "Globals.h"

#include "clang/AST/ASTContext.h"
#include "llvm/ADT/Twine.h"

#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"

namespace c2pulse {

void emitErrorWithLocationInternal(llvm::StringRef Message,
                                   const char *CallerFile, int CallerLine,
                                   const char *CallerFunc,
                                   const clang::ASTContext *Ctx,
                                   clang::SourceLocation Loc) {
  if (Ctx == nullptr) {
    llvm::errs() << "error: " << Message
                 << " (No source location to report!)\n"
                 << "[\n"
                 << "Triggered in File: " << CallerFile << "\n"
                 << "Triggered at line: " << CallerLine << "\n"
                 << "Triggered in function: " << CallerFunc << "\n"
                 << "]\n\n";
    llvm::report_fatal_error("Fatal Error ... Exiting!");
  }

  const auto &SM = Ctx->getSourceManager();
  clang::PresumedLoc PLoc = SM.getPresumedLoc(Loc);

  clang::SourceLocation Start = Loc;
  clang::SourceLocation End =
      clang::Lexer::getLocForEndOfToken(Start, 0, SM, Ctx->getLangOpts());
  clang::CharSourceRange CharRange =
      clang::CharSourceRange::getCharRange(Start, End);
  clang::StringRef Text =
      clang::Lexer::getSourceText(CharRange, SM, Ctx->getLangOpts());

  llvm::errs() << "Error: " << Message << "\n"
               << "[\n"
               << "Source Filename: "
               << (PLoc.isValid() ? PLoc.getFilename() : "<unknown>") << "\n"
               << "Source Line: " << (PLoc.isValid() ? PLoc.getLine() : 0)
               << "\n"
               << "Source Column: " << (PLoc.isValid() ? PLoc.getColumn() : 0)
               << "\n"
               << "C Expr: " << Text << "\n"
               << "]\n\n";

  llvm::errs() << "[\n"
               << "Triggered in File: " << CallerFile << "\n"
               << "Triggered at line: " << CallerLine << "\n"
               << "Triggered in function: " << CallerFunc << "\n"
               << "]\n\n";

  llvm::report_fatal_error("Fatal Error ... Exiting!");
}

} // namespace c2pulse
