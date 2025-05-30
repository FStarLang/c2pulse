//===- ANFAndPulseRewriter.cpp - ANF & Pulse Preparation Plugin ---------===//
//
// Transforms C functions into A-Normal Form (ANF), isolating each effectful
// operation into its own temporary, and preserves comments for Pulse-style
// annotations.
//
// Usage (after building as libANFPlugin.so):
//   clang -cc1 -load ./libANFPlugin.so -plugin anf-pulse <file.c>
//
//===----------------------------------------------------------------------===//

/*
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "PulseEmitter.h"
*/

#include "ANFAndPulseRewriter.h"
