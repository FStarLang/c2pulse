#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "MacroCommentTracker.h"
#include <memory>

class MacroFrontendAction : public clang::SyntaxOnlyAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;
    MacroCommentTracker *getTracker() const { return TrackerRaw; }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

private:
    // std::unique_ptr<MacroCommentTracker> Tracker;
    MacroCommentTracker * TrackerRaw = nullptr;

};
