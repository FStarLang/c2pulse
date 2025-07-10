#include "Globals.h"
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
                                   const clang::SourceManager *SM,
                                   clang::SourceLocation Loc) {

  auto Info = llvm::Twine("[\n") + llvm::Twine("Triggered in File: ") +
              CallerFile +
              "\n"
              "Triggered at line: " +
              llvm::Twine(CallerLine) + "\n" +
              "Triggered in function: " + llvm::Twine(CallerFunc) + "\n" + "]";
  if (SM == nullptr) {
    llvm::errs() << "error: " << Message
                 << " (No source location to report!)\n";
    llvm::report_fatal_error(Info);
  }

  clang::PresumedLoc PLoc = SM->getPresumedLoc(Loc);
  llvm::errs() << "error: " << Message << " at " << PLoc.getFilename() << ":"
               << PLoc.getLine() << ":" << PLoc.getColumn() << "\n";
  llvm::report_fatal_error(Info);
}
