#pragma once

#include "MacroCommentTracker.h"
#include "MacroMetadata.h"
#include "FileIDHash.h"

#include "clang/Frontend/FrontendActions.h"

#include <memory>
// #include <vector>
#include <unordered_map>
#include <map>

class MacroFrontendAction : public clang::SyntaxOnlyAction {
public:
    // explicit MacroFrontendAction(std::vector<MacroEventInfo> &macroEvents)
    //         : macroEventsVec(macroEvents) {}
    explicit MacroFrontendAction(std::unordered_map<clang::FileID, std::map<std::string, MacroEventInfo>> &macroEvents)
        : macroInfoMap(macroEvents) {}
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) override;
    MacroCommentTracker *getTracker() const { return TrackerRaw; }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;
    void EndSourceFileAction() override;

private:
    // std::vector<MacroEventInfo> &macroEventsVec;
    std::unordered_map<clang::FileID, std::map<std::string, MacroEventInfo>> &macroInfoMap;
    MacroCommentTracker * TrackerRaw = nullptr; // externally managed in main.cpp

};
