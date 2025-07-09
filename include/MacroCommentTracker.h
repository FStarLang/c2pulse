#pragma once

#include "clang/Lex/PPCallbacks.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Lex/Preprocessor.h"
#include <vector>
#include <string>

class MacroCommentTracker : public clang::PPCallbacks, public clang::CommentHandler {
public:
    MacroCommentTracker(clang::Preprocessor &PP, clang::SourceManager &SM, const clang::LangOptions &LangOpts);

    void MacroDefined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD) override;
    void MacroExpands(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD,
                      clang::SourceRange Range, const clang::MacroArgs *Args) override;

    void Comment(clang::SourceLocation Loc, clang::StringRef CommentText);

    bool HandleComment(clang::Preprocessor &PP, clang::SourceRange CommentRange) override;

    void printCollectedInfo() const;

private:
    clang::SourceManager &SM;
    const clang::LangOptions &LangOpts;

    std::vector<std::string> MacroDefs;
    std::vector<std::string> MacroExpansions;
    std::vector<std::string> Comments;
};


