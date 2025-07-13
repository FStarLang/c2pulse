#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include <string>

extern int GlobalGenSymCounter; // Declaration of global variable

std::string gensym(const std::string base);

void emitErrorWithLocationInternal(
    llvm::StringRef Message, const char *CallerFile, int CallerLine,
    const char *CallerFunc, const clang::ASTContext *Ctx = nullptr,
    clang::SourceLocation Loc = clang::SourceLocation::getFromRawEncoding(0));

#define emitErrorWithLocation(MSG, CTX, LOC)                                   \
  emitErrorWithLocationInternal(MSG, __FILE__, __LINE__, __func__, CTX, LOC)

#define emitError(MSG)                                                         \
  emitErrorWithLocationInternal(MSG, __FILE__, __LINE__, __func__)

extern const char* pulseProofTermFromC;
extern const char *pulseWhileInvariantFromC;
