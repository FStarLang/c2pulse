#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "ANFAndPulseRewriter.h"

#include <string>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

/// ASTConsumer that drives ANFVisitor
class ANFConsumer : public ASTConsumer {
public:
  ANFConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(ASTContext &Ctx) override;
  std::string getTransformedCode() const;

private:
  ANFVisitor Visitor;
  std::string transformedCode;
};
