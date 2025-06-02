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

class ANFTranformer {

    public: 
      ANFTranformer(std::vector<std::unique_ptr<ASTUnit>> &ASTList); 
      std::string getTransformedCode();
      void transform();
      std::string writeToFile();

    private:
       clang::Rewriter RewriterForPlugin;
       std::string TransformedCode;
       std::vector<std::unique_ptr<ASTUnit>> &InternalAstList; // Store the ASTList for processing

};