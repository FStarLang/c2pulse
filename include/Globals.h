#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/ADT/StringRef.h"

#include <string>

// Forward declaration to avoid heavy includes.
namespace clang {
class ASTContext;
}

namespace c2pulse {

// Thread-local, deterministic gensym counter (no global extern).
inline thread_local int g_GenSymCounter = 0;

inline std::string gensym(llvm::StringRef base = "var") {
  return std::string(base) + std::to_string(g_GenSymCounter++);
}

// Keeping the implementation symbol in our namespace; macros (below) call this.
void emitErrorWithLocationInternal(
    llvm::StringRef Message,
    const char* CallerFile,
    int CallerLine,
    const char* CallerFunc,
    const clang::ASTContext* Ctx = nullptr,
    clang::SourceLocation Loc = clang::SourceLocation::getFromRawEncoding(0));

// Token constants as constexpr (no extern globals).
inline constexpr char kPulseProofTermFromC[] = "__pulseproofterm";
inline constexpr char kPulseWhileInvariantFromC[] = "__pulsewhileinvariant";

} // namespace c2pulse

// Macros remain global but dispatch to the namespaced function.
#define emitErrorWithLocation(MSG, CTX, LOC)                                    \
  ::c2pulse::emitErrorWithLocationInternal((MSG), __FILE__, __LINE__, __func__, \
                                           (CTX), (LOC))

#define emitError(MSG)                                                          \
  ::c2pulse::emitErrorWithLocationInternal((MSG), __FILE__, __LINE__, __func__)
