#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "MacroCommentTracker.h"
#include <memory>

class MacroFrontendAction : public clang::SyntaxOnlyAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;
    std::unique_ptr<MacroCommentTracker> Tracker;

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

};
