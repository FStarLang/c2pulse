#pragma once

#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Token.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/Support/raw_ostream.h"

#include "MacroMetadata.h"

#include <string>
#include <vector>


class MacroCommentTracker : public clang::PPCallbacks, public clang::CommentHandler {
public:
    MacroCommentTracker(clang::Preprocessor &PP,
                        clang::SourceManager &SM,
                        const clang::LangOptions &LangOpts);

    // PPCallbacks overrides
    void MacroDefined(const clang::Token &MacroNameTok,
                      const clang::MacroDirective *MD) override;

    void MacroUndefined(const clang::Token &MacroNameTok,
                        const clang::MacroDefinition &MD,
                        const clang::MacroDirective *Undef) override;

    void MacroExpands(const clang::Token &MacroNameTok,
                      const clang::MacroDefinition &MD,
                      clang::SourceRange Range,
                      const clang::MacroArgs *Args) override;

    void Ifdef(clang::SourceLocation Loc,
               const clang::Token &MacroNameTok,
               const clang::MacroDefinition &MD) override;

    void Ifndef(clang::SourceLocation Loc,
                const clang::Token &MacroNameTok,
                const clang::MacroDefinition &MD) override;

    void Defined(const clang::Token &MacroNameTok,
                 const clang::MacroDefinition &MD,
                 clang::SourceRange Range) override;

    bool HandleComment(clang::Preprocessor &PP,
                       clang::SourceRange Comment) override;

    // void printCollectedInfo() const;
    void printMacroEventMap() const;
    void printMacroCollectedInfo() const;

    const std::vector<MacroEventInfo>& getEvents() const { return macroEventsVec; }
    const std::unordered_map<std::string, std::vector<MacroEventInfo>>& getMacroEventMap() const {
        return macroEventMap;
    }

private:
    clang::SourceManager &SM;
    const clang::LangOptions &LangOpts;

    std::vector<MacroEventInfo> macroEventsVec;
    std::unordered_map<std::string, std::vector<MacroEventInfo>> macroEventMap;

    // std::vector<std::string> MacroDefs;
    // std::vector<std::string> MacroExpansions;
    // std::vector<std::string> Comments;

    void printMacroEvent() const;
    void printMacroInfo(std::string filename, const MacroEventInfo &e) const;
};
