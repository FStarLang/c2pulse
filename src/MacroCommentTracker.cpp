#include "MacroCommentTracker.h"

#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "macro-comment-tracker"

using namespace clang;

MacroCommentTracker::MacroCommentTracker(clang::Preprocessor &PP,
                        clang::SourceManager &SM,
                        const clang::LangOptions &LangOpts,
                        std::unordered_map<clang::FileID, std::map<unsigned, MacroEventInfo>> &macroInfoMap)
    : SM(SM), LangOpts(LangOpts), macroInfoMap(macroInfoMap) {}

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

    SourceLocation Loc = MacroNameTok.getLocation();
    std::string macroName = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string filename = SM.getFilename(Loc).str();

    const MacroInfo *MI = MD->getMacroInfo();
    if (!MI) return;

    SourceLocation startLoc = SM.getSpellingLoc(MI->getDefinitionLoc());
    SourceLocation endLoc = !MI->tokens().empty()
                              ? SM.getSpellingLoc(MI->tokens().back().getLocation())
                              : startLoc;
    SourceRange Range(startLoc,endLoc);

    MacroEventInfo event;
    event.FileName = filename;
    event.Kind = MacroEventKind::Define;
    event.MacroName = macroName;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = "";  // No expansion text for #define event here

    MacroTokenInfo tokInfo;
    tokInfo.TokenText = macroName;
    tokInfo.Line = SM.getSpellingLineNumber(Loc);
    tokInfo.Column = SM.getSpellingColumnNumber(Loc);
    tokInfo.IsParam = false;

    event.Tokens.push_back(tokInfo);

    //macroEventsVec.push_back(event);
    // I am not adding line as a key here because I am afraid it will colide with other events.
    // I have not checked if I could do that, because for example we might have multiple macros 
    // defined on the same line.
    //macroInfoMap[SM.getFileID(Loc)][event.Line] = event;
    
    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(startLoc);
    // unsigned endOffset = SM.getFileOffset(endLoc);
    // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);
    int key = startOffset; // this is a simple key based on start offset if we observe collisions 
                           // we will need to change it to a more complex one the string based on that I have commented here
    macroInfoMap[fileID][key] = event;
    
    DEBUG_WITH_TYPE(DEBUG_TYPE, { 
    std::string info = "Macro defined: " + MacroNameTok.getIdentifierInfo()->getName().str() +
                       " at " + locToStr(SM, MacroNameTok.getLocation());                  
    llvm::outs() << info << "\n";
    });
}

void MacroCommentTracker::MacroUndefined(const Token &MacroNameTok,
                                         const MacroDefinition &MD,
                                         const MacroDirective *Undef) {

    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return; 

    SourceLocation Loc = MacroNameTok.getLocation();
    std::string macroName = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string filename = SM.getFilename(Loc).str();

    const MacroInfo *MI = MD.getMacroInfo();
    if (!MI) return;

    SourceLocation startLoc = SM.getSpellingLoc(MI->getDefinitionLoc());
    SourceLocation endLoc = !MI->tokens().empty()
                              ? SM.getSpellingLoc(MI->tokens().back().getLocation())
                              : startLoc;
    SourceRange Range(startLoc,endLoc);

    MacroEventInfo event;
    event.FileName = filename;
    event.Kind = MacroEventKind::Undefine;
    event.MacroName = macroName;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = ""; // Not applicable for undef

    MacroTokenInfo tokInfo;
    tokInfo.TokenText = macroName;
    tokInfo.Line = SM.getSpellingLineNumber(Loc);
    tokInfo.Column = SM.getSpellingColumnNumber(Loc);
    tokInfo.IsParam = false;

    event.Tokens.push_back(tokInfo);

    //macroEventsVec.push_back(event);

    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(startLoc);
    // unsigned endOffset = SM.getFileOffset(endLoc);
    // // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);

    int key = startOffset;                
    macroInfoMap[fileID][key] = event;

    DEBUG_WITH_TYPE(DEBUG_TYPE, { 
    std::string info = "Macro undefined: " + MacroNameTok.getIdentifierInfo()->getName().str() +
                       " at " + locToStr(SM, MacroNameTok.getLocation());                  
    llvm::outs() << info << "\n";
    });
}

void MacroCommentTracker::MacroExpands(const Token &MacroNameTok,
                                       const MacroDefinition &MD,
                                       SourceRange Range,
                                       const MacroArgs *Args) {

    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    SourceLocation Loc = MacroNameTok.getLocation();
    std::string macroName = MacroNameTok.getIdentifierInfo()->getName().str();

    // NOTE: I need to skip STR macro as it is used to mark what will be replaced 
    // by a string literal. This is a workaround for the fact that STR macro is used
    // in many places and it does not provide useful information for our tracking purposes.
    // This is a temporary solution and should be revisited if we need to track string literals.
    // I need to check if macroName == "STRINGIFY" || macroName == "TO_STRING" will hapen too.
    if (macroName == "STR") {
        return; 
    }

    std::string filename = SM.getFilename(Loc).str();
    if (filename.empty()) {
        filename = "[Unknown Source File]";
    }

    // Extract expanded source text
    bool invalid = false;
    const char *startData = SM.getCharacterData(Range.getBegin(), &invalid);
    const char *endData = SM.getCharacterData(Range.getEnd(), &invalid);

    std::string expansionText = "[Unavailable]";
    if (!invalid && endData >= startData) {
        expansionText.assign(startData, endData - startData + 1);
    }

    MacroEventInfo event;
    event.Kind = MacroEventKind::Expand;
    event.MacroName = macroName;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = std::string(expansionText);
    event.FileName = filename;

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

                MacroTokenInfo tokInfo;
                tokInfo.TokenText = spelling;
                tokInfo.Line = SM.getSpellingLineNumber(tokLoc);
                tokInfo.Column = SM.getSpellingColumnNumber(tokLoc);
                tokInfo.IsParam = true;

                event.Tokens.push_back(tokInfo);
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

    //macroEventsVec.push_back(event);
    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(Range.getBegin());
    // unsigned endOffset = SM.getFileOffset(Range.getEnd());
    // // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);

    int key = startOffset;
    macroInfoMap[fileID][key] = event;

    DEBUG_WITH_TYPE(DEBUG_TYPE, {  
    std::string info = "Macro expanded: " + macroName +
                       " at " + locToStr(SM, Loc) +
                       "\n  Expansion: " + expansionText +
                       "\n  Parameters: " + paramsText;                
    llvm::outs() << info << "\n";
    });
}

void MacroCommentTracker::Ifdef(SourceLocation Loc,
                                const Token &MacroNameTok,
                                const MacroDefinition &MD) {
    
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string filename = SM.getFilename(Loc).str();
    
    const MacroInfo *MI = MD.getMacroInfo();
    if (!MI) return;

    SourceLocation startLoc = SM.getSpellingLoc(MI->getDefinitionLoc());
    SourceLocation endLoc = !MI->tokens().empty()
                              ? SM.getSpellingLoc(MI->tokens().back().getLocation())
                              : startLoc;
    SourceRange Range(startLoc,endLoc);

    MacroEventInfo event;
    event.FileName = filename;
    event.Kind = MacroEventKind::Ifdef;
    event.MacroName = macro;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = ""; // No expansion in #ifdef
    
    //WE MIGHT NEED TO TOKENIZE THE MACRO LINE
    MacroTokenInfo tokInfo;
    tokInfo.TokenText = macro;
    tokInfo.Line = SM.getSpellingLineNumber(Loc);
    tokInfo.Column = SM.getSpellingColumnNumber(Loc);
    tokInfo.IsParam = false;

    event.Tokens.push_back(tokInfo);
    
    //macroEventsVec.push_back(event);

    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(startLoc);
    // unsigned endOffset = SM.getFileOffset(endLoc);
    // // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);

    int key = startOffset;               
    macroInfoMap[fileID][key] = event;

    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    std::string info = "#ifdef: " + macro + " at " + locToStr(SM, Loc);
    llvm::outs() << info << "\n";
    });
}

void MacroCommentTracker::Ifndef(SourceLocation Loc,
                                 const Token &MacroNameTok,
                                 const MacroDefinition &MD) {
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    std::string filename = SM.getFilename(Loc).str();

    const MacroInfo *MI = MD.getMacroInfo();
    if (!MI) return;

    SourceLocation startLoc = SM.getSpellingLoc(MI->getDefinitionLoc());
    SourceLocation endLoc = !MI->tokens().empty()
                              ? SM.getSpellingLoc(MI->tokens().back().getLocation())
                              : startLoc; // TODO: maybe I need to be defensive here and check if endLoc is valid too
    SourceRange Range(startLoc,endLoc);

    MacroEventInfo event;
    event.FileName = filename;
    event.Kind = MacroEventKind::Ifndef;
    event.MacroName = macro;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = ""; // No expansion in #ifndef

    //WE MIGHT NEED TO TOKENIZE THE MACRO LINE
    MacroTokenInfo tokInfo;
    tokInfo.TokenText = macro;
    tokInfo.Line = SM.getSpellingLineNumber(Loc);
    tokInfo.Column = SM.getSpellingColumnNumber(Loc);
    tokInfo.IsParam = false;

    event.Tokens.push_back(tokInfo);

    //macroEventsVec.push_back(event);
  
    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(startLoc);
    // unsigned endOffset = SM.getFileOffset(endLoc);
    // // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);

    int key = startOffset;
    macroInfoMap[fileID][key] = event;

    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    std::string ifndefInfo = "#ifndef: " + macro + " at " + locToStr(SM, Loc);
    llvm::outs() << ifndefInfo << "\n";
    });
}

void MacroCommentTracker::Defined(const clang::Token &MacroNameTok,
                                  const clang::MacroDefinition &MD,
                                  clang::SourceRange Range) {
    
    SourceLocation ExpansionLoc = SM.getExpansionLoc(MacroNameTok.getLocation());
    if (!SM.isWrittenInMainFile(ExpansionLoc)) return;

    std::string macro = MacroNameTok.getIdentifierInfo()->getName().str();
    SourceLocation Loc = Range.getBegin();
    std::string filename = SM.getFilename(Loc).str();

    MacroEventInfo event;
    event.FileName = filename;
    event.Kind = MacroEventKind::Defined;
    event.MacroName = macro;
    event.Line = SM.getSpellingLineNumber(Loc);
    event.Column = SM.getSpellingColumnNumber(Loc);
    event.range = Range;
    event.ExpansionText = ""; // No expansion text for `defined()` usage

    //WE MIGHT NEED TO TOKENIZE THE MACRO LINE
    MacroTokenInfo tokInfo;
    tokInfo.TokenText = macro;
    tokInfo.Line = SM.getSpellingLineNumber(Loc);
    tokInfo.Column = SM.getSpellingColumnNumber(Loc);
    tokInfo.IsParam = false;

    event.Tokens.push_back(tokInfo);
  
    // Get byte offsets within the source file
    FileID fileID = SM.getFileID(Loc);
    unsigned startOffset = SM.getFileOffset(Range.getBegin());
    // unsigned endOffset = SM.getFileOffset(Range.getEnd());
    // // Create a unique key for the macro event
    // std::string key = std::to_string(fileID.getHashValue()) + ":" +
    //                 std::to_string(startOffset) + "-" + std::to_string(endOffset);

    int key = startOffset;
    macroInfoMap[fileID][key] = event;
    
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
    std::string locInfo = locToStr(SM, Range.getBegin());
    std::string macroDefinition = "defined(" + macro + ") at " + locInfo;
    llvm::outs() << macroDefinition << "\n";
    });
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

    DEBUG_WITH_TYPE(DEBUG_TYPE, {    
    std::string info = "Comment at " + locToStr(SM, Loc) + ": " + CommentText;               
    llvm::outs() << info << "\n";
    });

    // commentsVec.push_back(info);
    return false;  // allow others to see it
}

void MacroCommentTracker::printMacroEventMap() const {
    for (const auto &pair : macroInfoMap) {
        const std::map<unsigned, MacroEventInfo> &events = pair.second;
        if (events.empty()) {
            llvm::outs() << "  [No macro events found]\n";
            continue;
        }
        llvm::outs() << "  Number of macro events collected: " << events.size() << "\n";
        llvm::outs() << "----------------------\n";
        for (const auto &e : events) {
            const MacroEventInfo &macroInfo = e.second;
            llvm::outs() << "Kind: " << toString(macroInfo.Kind) << "\n";
            llvm::outs() << "Macro: " << macroInfo.MacroName << "\n";
            llvm::outs() << "Expansion: " << macroInfo.ExpansionText << "\n";
            llvm::outs() << "Location: Line: " << macroInfo.Line << ", Column: " << macroInfo.Column << "\n";
            llvm::outs() << "Filename: " << macroInfo.FileName << "\n";
            llvm::outs() << "Tokens:\n";
            for (const auto &t : macroInfo.Tokens) {
                llvm::outs() << "  " << (t.IsParam ? "[param] " : "[macro] ")
                             << t.TokenText << " at Line " << t.Line
                             << ", Column " << t.Column << "\n";
            }
            llvm::outs() << "----------------------\n";
        }
        llvm::outs() << "\n";
    }
}

