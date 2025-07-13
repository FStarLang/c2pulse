#pragma once

#include "MacroFrontendAction.h"
#include "MacroMetadata.h"
#include "FileIDHash.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"

// #include <vector>
#include <unordered_map>
#include <map>

class MacroFrontendActionFactory : public clang::tooling::FrontendActionFactory {
public:
    // MacroFrontendActionFactory(std::vector<MacroEventInfo> &events)
    //         : macroEventsVec(events) {}
    MacroFrontendActionFactory(std::unordered_map<clang::FileID, std::map<std::string, MacroEventInfo>> &events)
        : macroInfoMap(events) {}

    MacroFrontendAction *getAction() const { return ActionPtr; }

    std::unique_ptr<clang::FrontendAction> create() override {
        // auto Action = std::make_unique<MacroFrontendAction>(macroEventsVec);
        auto Action = std::make_unique<MacroFrontendAction>(macroInfoMap);
        ActionPtr = Action.get(); // Keep raw pointer for later access in main.cpp [I need to revisit this decision]
        return Action;
    }

private:
    mutable MacroFrontendAction *ActionPtr = nullptr; 
    // std::vector<MacroEventInfo> &macroEventsVec;
    std::unordered_map<clang::FileID, std::map<std::string, MacroEventInfo>> &macroInfoMap;
};
