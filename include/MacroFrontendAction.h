#pragma once

#include "MacroCommentTracker.h"
#include "MacroMetadata.h"
#include "FileIDHash.h"

#include "clang/Frontend/FrontendActions.h"

#include <memory>
#include <unordered_map>
#include <map>

class MacroFrontendAction : public clang::SyntaxOnlyAction {
public:
    explicit MacroFrontendAction(std::unordered_map<clang::FileID, std::map<unsigned, MacroEventInfo>, FileIDHash> &macroEvents,
        std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens)
        : macroInfoMap(macroEvents), macroTokens(macroTokens) {}
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;
    MacroCommentTracker *getTracker() const { return TrackerRaw; }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

private:
    std::unordered_map<clang::FileID, std::map<unsigned, MacroEventInfo>, FileIDHash> &macroInfoMap;
    std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens;
    MacroCommentTracker * TrackerRaw = nullptr; // externally managed in main.cpp

};
