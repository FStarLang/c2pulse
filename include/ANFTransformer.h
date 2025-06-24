#pragma once

#include "llvm/Support/CommandLine.h"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <memory>
#include <string>
#include <vector>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;

class ANFTranformer {

    public: 
      ANFTranformer(std::vector<std::unique_ptr<clang::ASTUnit>> &ASTList); 
      std::string getTransformedCode();
      void transform();
      std::string writeToFile();

    private:
       clang::Rewriter RewriterForPlugin;
       std::string TransformedCode;
       std::vector<std::unique_ptr<clang::ASTUnit>> &InternalAstList; // Store the ASTList for processing

};