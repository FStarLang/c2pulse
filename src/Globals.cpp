#include "Globals.h"

using namespace std;

extern int GlobalGenSymCounter;

static string counterToVar(){
    return to_string(GlobalGenSymCounter++);
}


string gensym(const string base = "var") {
    return base + counterToVar();
}

void emitErrorWithLocation(llvm::StringRef Message,
                           const clang::SourceManager *SM,
                           clang::SourceLocation Loc) {
  if (SM == nullptr) {
    llvm::errs() << "error: " << Message
                 << " (No source location to report!)\n";
    llvm::report_fatal_error("Aborting due to fatal error");
  }

  clang::PresumedLoc PLoc = SM->getPresumedLoc(Loc);
  llvm::errs() << "error: " << Message << " at " << PLoc.getFilename() << ":"
               << PLoc.getLine() << ":" << PLoc.getColumn() << "\n";
  llvm::report_fatal_error("Aborting due to fatal error");
}
