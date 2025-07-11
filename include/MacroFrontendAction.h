#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "MacroCommentTracker.h"
#include "MacroMetadata.h"
#include <memory>

class MacroFrontendAction : public clang::SyntaxOnlyAction {
public:
    explicit MacroFrontendAction(std::vector<MacroEventInfo> &macroEvents)
            : macroEventsVec(macroEvents) {}
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;
    MacroCommentTracker *getTracker() const { return TrackerRaw; }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

private:
    std::vector<MacroEventInfo> &macroEventsVec;
    MacroCommentTracker * TrackerRaw = nullptr; // externally managed in main.cpp

};
