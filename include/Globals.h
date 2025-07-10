#pragma once

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include <string>

extern int GlobalGenSymCounter; // Declaration of global variable

std::string gensym(const std::string base);

void emitErrorWithLocation(
    llvm::StringRef Message, const clang::SourceManager *SM = nullptr,
    clang::SourceLocation Loc = clang::SourceLocation::getFromRawEncoding(0));

extern const char* pulseProofTermFromC;
extern const char *pulseWhileInvariantFromC;
