#include "MacroFrontendAction.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/SourceManager.h"

#define DEBUG_TYPE "macro-frontend-action"

using namespace clang;

bool MacroFrontendAction::BeginSourceFileAction(CompilerInstance &CI) {
    // Enable full comment parsing
    CI.getLangOpts().CommentOpts.ParseAllComments = true;

    Preprocessor &PP = CI.getPreprocessor();
    SourceManager &SM = CI.getSourceManager();
    const LangOptions &LangOpts = CI.getLangOpts();

    // std::unique_ptr<MacroCommentTracker> Tracker = std::make_unique<MacroCommentTracker>(PP, SM, LangOpts, macroEventsVec);
    std::unique_ptr<MacroCommentTracker> Tracker = std::make_unique<MacroCommentTracker>(PP, SM, LangOpts, macroInfoMap);
    TrackerRaw = Tracker.get();

    PP.addPPCallbacks(std::move(Tracker));
    PP.addCommentHandler(TrackerRaw);
    
    return SyntaxOnlyAction::BeginSourceFileAction(CI);
}

void MacroFrontendAction::EndSourceFileAction() {
    SyntaxOnlyAction::EndSourceFileAction();
}

std::unique_ptr<ASTConsumer> MacroFrontendAction::CreateASTConsumer(
    CompilerInstance &CI, llvm::StringRef InFile) {

    return SyntaxOnlyAction::CreateASTConsumer(CI, InFile);
}

