#include "PulseEmitter.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "llvm/Support/raw_ostream.h"
#include <regex>
#include <sstream>
#include "PulseGenerator.h"

using namespace clang;

PulseConsumer::PulseConsumer(clang::ASTContext &Ctx, clang::Rewriter &R)
    : Visitor(R, Ctx) {}

void PulseConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
}

  bool PulseVisitor::VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()))
      return true;

    // Always apply ANF rewriting on user functions
    if (Stmt *Body = FD->getBody()) {
      if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
        // if (llvm::DebugFlag && CS) {
        //   std::string StmtToStr;
        //   llvm::raw_string_ostream os(StmtToStr);
        //   CS->printPretty(os, nullptr, Ctx.getPrintingPolicy());
        //   DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
        //                                   << "The compound statement is: "
        //                                   << os.str() << "\n");
        // }
        auto NewText = rewriteCompound(CS);
        TheRewriter.ReplaceText(CS->getSourceRange(), NewText);
        TransformedCode = NewText;
      }
    }
    
    return true;
  }

  