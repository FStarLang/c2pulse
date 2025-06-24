#pragma once

#include "ANFVisitor.h"

#include "clang/AST/ASTContext.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <string>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

/// ASTConsumer that drives the ANFVisitor to perform analysis and transformation.
class ANFConsumer : public ASTConsumer {
public:
  ANFConsumer(clang::ASTContext &Ctx, clang::Rewriter &R);

  void HandleTranslationUnit(ASTContext &Ctx) override;
  std::string getTransformedCode() const;

private:
  ANFVisitor Visitor;
  std::string transformedCode;
};
