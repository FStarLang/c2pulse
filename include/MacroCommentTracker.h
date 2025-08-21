#pragma once

#include "MacroMetadata.h"
#include "FileIDHash.h"

#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Token.h"
#include "clang/Basic/LangOptions.h"

#include <unordered_map>
#include <map>

struct TokenInfo {
    std::string Pre; // ' ' or '\n' or ''
    clang::SourceRange Range;
    std::string Text;
};

class MacroCommentTracker : public clang::PPCallbacks, public clang::CommentHandler {
public:

    MacroCommentTracker(clang::Preprocessor &PP,
                        clang::SourceManager &SM,
                        const clang::LangOptions &LangOpts,
                        std::unordered_map<clang::FileID, std::map<unsigned, MacroEventInfo>, FileIDHash> &macroInfoMap,
                        std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens);

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

    // TO-DO: I need to add #endif and #else

    bool HandleComment(clang::Preprocessor &PP,
                       clang::SourceRange Comment) override;

    void printMacroEventMap() const;
    void printMacroCollectedInfo() const;

private:
    clang::Preprocessor &PP;
    clang::SourceManager &SM;
    const clang::LangOptions &LangOpts;

    std::unordered_map<clang::FileID, std::map<unsigned, MacroEventInfo>, FileIDHash> &macroInfoMap;
    std::unordered_map<unsigned, std::vector<TokenInfo>>& macroTokens;
};
