#include "MacroFrontendAction.h"
#include "MacroMetadata.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"

class MacroFrontendActionFactory : public clang::tooling::FrontendActionFactory {
public:
    MacroFrontendActionFactory(std::vector<MacroEventInfo> &events)
            : macroEventsVec(events) {}

    MacroFrontendAction *getAction() const { return ActionPtr; }

    std::unique_ptr<clang::FrontendAction> create() override {
        auto Action = std::make_unique<MacroFrontendAction>(macroEventsVec);
        ActionPtr = Action.get(); // Keep raw pointer for later access
        return Action;
    }

private:
    mutable MacroFrontendAction *ActionPtr = nullptr; 
    std::vector<MacroEventInfo> &macroEventsVec;
};
