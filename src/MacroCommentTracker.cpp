#include "MacroCommentTracker.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

MacroCommentTracker::MacroCommentTracker(Preprocessor &PP, SourceManager &SM, const LangOptions &LangOpts)
    : SM(SM), LangOpts(LangOpts) {}

void MacroCommentTracker::MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) {
    // SourceLocation Loc = MacroNameTok.getLocation();
    // if (!SM.isWrittenInMainFile(Loc))
    //     return;
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;
    std::string Info = "Macro defined: " + MacroNameTok.getIdentifierInfo()->getName().str();
    MacroDefs.push_back(Info);
}

void MacroCommentTracker::MacroExpands(const Token &MacroNameTok,
                                       const MacroDefinition &MD,
                                       SourceRange Range,
                                       const MacroArgs *Args) {
                                        
    SourceLocation Loc = MacroNameTok.getLocation();
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());

    if (!SM.isWrittenInMainFile(ExpansionLoc))
        return;
    
    unsigned Line = SM.getSpellingLineNumber(Loc);
    unsigned Col = SM.getSpellingColumnNumber(Loc);

    std::string macroName = MacroNameTok.getIdentifierInfo()->getName().str();

    // Extract expanded source text
    bool invalid = false;
    const char* startData = SM.getCharacterData(Range.getBegin(), &invalid);
    const char* endData = SM.getCharacterData(Range.getEnd(), &invalid);

    std::string expansionText = "[Unavailable]";
    if (!invalid && endData >= startData) {
        expansionText.assign(startData, endData - startData + 1);
    }

    // Extract macro parameters (with token location)
    std::string paramsText;
    if (Args) {
        unsigned numArgs = Args->getNumMacroArguments();
        for (unsigned i = 0; i < numArgs; ++i) {
            const Token *argTokens = Args->getUnexpArgument(i);
            unsigned numTokens = Args->getArgLength(argTokens);

            std::string paramTokens;
            for (unsigned j = 0; j < numTokens; ++j) {
                const Token &tok = argTokens[j];
                std::string spelling = Lexer::getSpelling(tok, SM, LangOpts);
                SourceLocation tokLoc = SM.getSpellingLoc(tok.getLocation());

                unsigned tokLine = SM.getSpellingLineNumber(tokLoc);
                unsigned tokCol = SM.getSpellingColumnNumber(tokLoc);

                paramTokens += spelling + "@(" + std::to_string(tokLine) + ":" + std::to_string(tokCol) + ") ";
            }

            if (!paramTokens.empty())
                paramsText += "[ " + paramTokens + "] ";
        }

        if (paramsText.empty()) {
            paramsText = "[None]";
        }
    } else {
        paramsText = "[None]";
    }

    // Final formatted info
    std::string info = "Macro expanded: " + macroName +
                       " at Line " + std::to_string(Line) +
                       ", Column " + std::to_string(Col) +
                       "\n  Expansion: " + expansionText +
                       "\n  Parameters: " + paramsText;

    llvm::outs() << info << "\n";

    MacroExpansions.push_back(info);
}


void MacroCommentTracker::Comment(SourceLocation Loc, StringRef CommentText) {

    if (!SM.isWrittenInMainFile(Loc))
        return;

    std::string Info = "Comment: " + CommentText.str();
    Comments.push_back(Info);
}

void MacroCommentTracker::printCollectedInfo() const {
    llvm::outs() << "=== Macro Definitions ===\n";
    for (const auto &Def : MacroDefs)
        llvm::outs() << Def << "\n";

    llvm::outs() << "=== Macro Expansions ===\n";
    for (const auto &Exp : MacroExpansions)
        llvm::outs() << Exp << "\n";

    llvm::outs() << "=== Comments ===\n";
    for (const auto &C : Comments)
        llvm::outs() << C << "\n";
}

bool MacroCommentTracker::HandleComment(Preprocessor &PP, SourceRange CommentRange) {
    SourceLocation Loc = CommentRange.getBegin();
    if (!SM.isWrittenInMainFile(SM.getExpansionLoc(Loc)))
        return false;

    bool invalid = false;
    const char *startData = SM.getCharacterData(CommentRange.getBegin(), &invalid);
    const char *endData = SM.getCharacterData(CommentRange.getEnd(), &invalid);
    if (invalid) return false;

    std::string CommentText(startData, endData - startData + 1);
    Comments.push_back("Comment: " + CommentText);
    return false;  // allowing other handlers to see it too
}
