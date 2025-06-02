#pragma once

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "ANFConsumer.h"

#include <memory>
#include <vector>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

class ANFFrontendAction : public clang::PluginASTAction {

protected:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override;

  void EndSourceFileAction() override;

private:
  clang::Rewriter RewriterForPlugin;
  ANFConsumer *ConsumerInstance = nullptr;
};
