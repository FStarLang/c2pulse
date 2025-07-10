#include "MacroCommentTracker.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "macro-comment-tracker"

using namespace clang;

MacroCommentTracker::MacroCommentTracker(Preprocessor &PP, SourceManager &SM, const LangOptions &LangOpts)
    : SM(SM), LangOpts(LangOpts) {}

static std::string locToStr(const SourceManager &SM, SourceLocation Loc) {
    SourceLocation ExpansionLoc = SM.getExpansionLoc(Loc);
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return "[!MainFile]";
    unsigned Line = SM.getSpellingLineNumber(ExpansionLoc);
    unsigned Col = SM.getSpellingColumnNumber(ExpansionLoc);
    return "Line " + std::to_string(Line) + ", Column " + std::to_string(Col);
}

void MacroCommentTracker::MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) {
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string info = "Macro defined: " + MacroNameTok.getIdentifierInfo()->getName().str() +
                       " at " + locToStr(SM, MacroNameTok.getLocation());
    
    DEBUG_WITH_TYPE(DEBUG_TYPE, {                   
    llvm::outs() << info << "\n";
    });
    MacroDefs.push_back(info);
}

void MacroCommentTracker::MacroUndefined(const Token &MacroNameTok,
                                         const MacroDefinition &MD,
                                         const MacroDirective *Undef) {

    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;       

    std::string info = "Macro undefined: " + MacroNameTok.getIdentifierInfo()->getName().str() +
                       " at " + locToStr(SM, MacroNameTok.getLocation());
    DEBUG_WITH_TYPE(DEBUG_TYPE, {                   
    llvm::outs() << info << "\n";
    });
    MacroDefs.push_back(info);
}

void MacroCommentTracker::MacroExpands(const Token &MacroNameTok,
                                       const MacroDefinition &MD,
                                       SourceRange Range,
                                       const MacroArgs *Args) {

    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    SourceLocation Loc = MacroNameTok.getLocation();
    std::string macroName = MacroNameTok.getIdentifierInfo()->getName().str();

    // Extract expanded source text
    bool invalid = false;
    const char *startData = SM.getCharacterData(Range.getBegin(), &invalid);
    const char *endData = SM.getCharacterData(Range.getEnd(), &invalid);

    std::string expansionText = "[Unavailable]";
    if (!invalid && endData >= startData) {
        expansionText.assign(startData, endData - startData + 1);
    }

    // Extract macro parameters (with token locations)
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

                paramTokens += spelling + "@(" + locToStr(SM, tokLoc) + ") ";
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
                       " at " + locToStr(SM, Loc) +
                       "\n  Expansion: " + expansionText +
                       "\n  Parameters: " + paramsText;

    DEBUG_WITH_TYPE(DEBUG_TYPE, {                   
    llvm::outs() << info << "\n";
    });
    MacroExpansions.push_back(info);
}

void MacroCommentTracker::Ifdef(SourceLocation Loc,
                                const Token &MacroNameTok,
                                const MacroDefinition &MD) {
    
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string info = "#ifdef: " + macro + " at " + locToStr(SM, Loc);

    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    llvm::outs() << info << "\n";
    });

    MacroDefs.push_back(info);
}

void MacroCommentTracker::Ifndef(SourceLocation Loc,
                                 const Token &MacroNameTok,
                                 const MacroDefinition &MD) {
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string ifndefInfo = "#ifndef: " + macro + " at " + locToStr(SM, Loc);

    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    llvm::outs() << ifndefInfo << "\n";
    });

    MacroDefs.push_back(ifndefInfo);
}

void MacroCommentTracker::Defined(const clang::Token &MacroNameTok,
                                  const clang::MacroDefinition &MD,
                                  clang::SourceRange Range) {
    
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string locInfo = locToStr(SM, Range.getBegin());
    std::string macroDefinition = "defined(" + macro + ") at " + locInfo;
    
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    llvm::outs() << macroDefinition << "\n";
    });

    MacroDefs.push_back(macroDefinition);
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
    std::string info = "Comment at " + locToStr(SM, Loc) + ": " + CommentText;

    DEBUG_WITH_TYPE(DEBUG_TYPE, {                   
    llvm::outs() << info << "\n";
    });

    Comments.push_back(info);
    return false;  // allow others to see it
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
