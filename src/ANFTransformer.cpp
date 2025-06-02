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
#include "llvm/Support/Error.h"

#include "ANFConsumer.h"
#include "ANFTransformer.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

ANFTranformer::ANFTranformer(std::vector<std::unique_ptr<ASTUnit>> &ASTList)
        : InternalAstList(ASTList) {
            // Initialize the rewriter with the first AST unit's context
            if (!ASTList.empty()) {
                RewriterForPlugin.setSourceMgr(ASTList[0]->getSourceManager(), ASTList[0]->getLangOpts());
            } else {
                llvm::errs() << "Error: No AST units provided for transformation.\n";
                exit(1);
            }
}

std::string ANFTranformer::writeToFile() {

  clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
  auto *FileEnt = SM.getFileEntryForID(SM.getMainFileID());
  if (!FileEnt) {
    llvm::errs() << "Error: Main file entry not found in source manager.\n";
    exit(1);
  }

  auto FilePath = FileEnt->tryGetRealPathName();

  std::filesystem::path FilePathSys = FilePath.str();
  auto Extension = FilePathSys.extension().string();
  auto TempFilePathWithoutExtension = FilePathSys.replace_extension("");

  std::string TempFilePath =
      TempFilePathWithoutExtension.string() + ".transformed" + Extension;
  std::ofstream OutFile(TempFilePath);
  if (!OutFile.is_open()) {
    llvm::errs()
        << "Error: Failed to create temporary file for transformed code.\n";
  }

  OutFile << TransformedCode;
  OutFile.close();
  return TempFilePath;
}

void ANFTranformer::transform() {
            for (auto &AstCtx : InternalAstList) {
                ANFConsumer Consumer(AstCtx->getASTContext(), RewriterForPlugin);
                Consumer.HandleTranslationUnit(AstCtx->getASTContext());
            }
            
            clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
            clang::FileID MainFileID = SM.getMainFileID();
            
            if (!MainFileID.isValid()) {
                llvm::errs() << "Error: Invalid MainFileID—source file may not be loaded correctly.\n";
            }

            // Capture rewritten buffer
            const llvm::RewriteBuffer *Buffer = RewriterForPlugin.getRewriteBufferFor(MainFileID);

            if (!Buffer) {
                llvm::errs() << "Warning: Rewriter buffer is empty—no modifications detected.\n";
                exit(1);
            }
            
            // Store transformed code in the class variable
            TransformedCode = std::string(Buffer->begin(), Buffer->end());
}

std::string ANFTranformer::getTransformedCode() { return TransformedCode; }