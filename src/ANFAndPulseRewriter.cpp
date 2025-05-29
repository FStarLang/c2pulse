//===- ANFAndPulseRewriter.cpp - ANF & Pulse Preparation Plugin ---------===//
//
// Transforms C functions into A-Normal Form (ANF), isolating each effectful
// operation into its own temporary, and preserves comments for Pulse-style
// annotations.
//
// Usage (after building as libANFPlugin.so):
//   clang -cc1 -load ./libANFPlugin.so -plugin anf-pulse <file.c>
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"


using namespace clang;

static llvm::cl::OptionCategory ToolCategory("anf-pulse-tool options");

static llvm::cl::opt<std::string>
    FunctionNameToProcess("func",
      llvm::cl::desc("Only transform the function named <func>"),
      llvm::cl::value_desc("function name"),
      llvm::cl::cat(ToolCategory));

static llvm::cl::opt<std::string>
    TransformMode("mode",
      llvm::cl::desc("Select transformation mode: anf, pulse, or both"),
      llvm::cl::value_desc("mode"),
      llvm::cl::init("both"),
      llvm::cl::cat(ToolCategory));

enum class ANFTransformModeKind
{
    ANFOnly,
    PulseOnly,
    Both
};

static ANFTransformModeKind getTransformMode()
{
    if (TransformMode == "anf")
        return ANFTransformModeKind::ANFOnly;
    if (TransformMode == "pulse")
        return ANFTransformModeKind::PulseOnly;
    return ANFTransformModeKind::Both;
}

namespace {
/// \brief Visitor that rewrites effectful expressions into ANF.
class ANFVisitor : public RecursiveASTVisitor<ANFVisitor> {
public:
  ANFVisitor(Rewriter &R, ASTContext &Ctx)
    : TheRewriter(R), Context(Ctx), SM(Ctx.getSourceManager()), Counter(0) {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()))
      return true;

    // Check for function name match if specified
    if (!FunctionNameToProcess.empty() && FD->getNameAsString() != FunctionNameToProcess) {
        return true; // Skip this function
    }

    // Only process user functions
    Stmt *Body = FD->getBody();
    if (auto *CS = dyn_cast<CompoundStmt>(Body))
      rewriteCompound(CS);
    return true;
  }

private:
  Rewriter &TheRewriter;
  ASTContext &Context;
  SourceManager &SM;
  int Counter;

  /// Produce a fresh temp name.
  std::string freshTemp() {
    return "__anf_tmp" + std::to_string(Counter++);
  }

  /// Rewrite a compound statement in-place.
  void rewriteCompound(CompoundStmt *CS) {
    std::string NewText = "{\n";
    for (Stmt *S : CS->body()) {
      NewText += rewriteStmt(S);
    }
    NewText += "}\n";
    TheRewriter.ReplaceText(CS->getSourceRange(), NewText);
  }

  /// Rewrite a single statement, returning its text (including lifted temps).
  std::string rewriteStmt(Stmt *S) {
    std::string Out;
    // Preserve leading comments/whitespace
    SourceLocation B = S->getBeginLoc();
    Out += commentPrefix(B);

    if (auto *DS = dyn_cast<DeclStmt>(S)) {
      Out += rewriteDeclStmt(DS);
    }
    else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
      Out += rewriteReturn(RS);
    }
    else if (auto *BO = dyn_cast<BinaryOperator>(S)) {
      if (BO->isAssignmentOp())
        Out += rewriteAssignment(BO);
      else
        Out += rewriteExprStmt(BO);
    }
    else if (auto *IS = dyn_cast<IfStmt>(S)) {
      Out += rewriteIf(IS);
    }
    else if (auto *WS = dyn_cast<WhileStmt>(S)) {
      Out += rewriteWhile(WS);
    }
    else if (auto *E = dyn_cast<Expr>(S)) {
      Out += rewriteExprStmt(E);
    }
    else {
      // Fallback: print as-is
      Out += stmtToString(S) + "\n";
    }
    return Out;
  }

  /// Lift effectful subexpressions in a DeclStmt.
  std::string rewriteDeclStmt(DeclStmt *DS) {
    std::string Out;
    for (auto *D : DS->decls()) {
      if (auto *VD = dyn_cast<VarDecl>(D)) {
        if (Expr *Init = VD->getInit()) {
          // Lift if effectful
          if (isEffectful(Init)) {
            std::string tmp = freshTemp();
            Out += VD->getType().getAsString() + " " + tmp
                 + " = " + exprToString(Init) + ";\n";
            // rewrite original init to tmp
            Out += VD->getType().getAsString() + " "
                 + VD->getNameAsString() + " = " + tmp + ";\n";
            continue;
          }
        }
      }
      // default: print decl
      Out += stmtToString(DS) + "\n";
      break;
    }
    return Out;
  }

  /// Rewrite a return stmt by lifting its expr if needed.
  std::string rewriteReturn(ReturnStmt *RS) {
    if (Expr *E = RS->getRetValue()) {
      if (isEffectful(E)) {
        std::string tmp = freshTemp();
        std::string Ty  = E->getType().getAsString();
        return Ty + " " + tmp + " = " + exprToString(E) + ";\n"
             + "return " + tmp + ";\n";
      }
      return "return " + exprToString(E) + ";\n";
    }
    return "return;\n";
  }

  /// Rewrite an assignment, lifting RHS if needed.
  std::string rewriteAssignment(BinaryOperator *BO) {
    Expr *L = BO->getLHS(), *R = BO->getRHS();
    std::string Out;
    if (isEffectful(R)) {
      std::string tmp = freshTemp();
      Out += R->getType().getAsString() + " "
           + tmp + " = " + exprToString(R) + ";\n";
      Out += exprToString(L) + " = " + tmp + ";\n";
    } else {
      Out += exprToString(L) + " = " + exprToString(R) + ";\n";
    }
    return Out;
  }

/// Rewrite an if statement, lifting its condition.
std::string rewriteIf(IfStmt *IS) {
  Expr *Cond = IS->getCond();
  std::string Out;

  // Get the source‐spelled type of the condition
  std::string Ty = Cond->getType().getAsString();

  if (isEffectful(Cond)) {
    std::string tmp = freshTemp();
    // Use the actual type, not 'int'
    Out += Ty + " " + tmp + " = " + exprToString(Cond) + ";\n";
    Out += "if (" + tmp + ") " + blockText(IS->getThen()) + "\n";
  } else {
    Out += "if (" + exprToString(Cond) + ") " + blockText(IS->getThen()) + "\n";
  }

  if (Stmt *E = IS->getElse())
    Out += "else " + blockText(E) + "\n";

  return Out;
}

  /// Rewrite a while statement, lifting its condition.
  std::string rewriteWhile(WhileStmt *WS) {
    Expr *Cond = WS->getCond();
    
    std::string Ty = Cond->getType().getAsString();
    std::string Out;
    if (isEffectful(Cond)) {
      std::string tmp = freshTemp();
      Out += Ty + tmp + " = " + exprToString(Cond) + ";\n";
      Out += "while (" + tmp + ") "
           + blockText(WS->getBody()) + "\n";
    } else {
      Out += "while (" + exprToString(Cond) + ") "
           + blockText(WS->getBody()) + "\n";
    }
    return Out;
  }

  /// Rewrite a standalone expr stmt.
  std::string rewriteExprStmt(Expr *E) {
    if (isEffectful(E)) {
      std::string tmp = freshTemp();
      Out:
      return E->getType().getAsString() + " " + tmp + " = " + exprToString(E) + ";\n";
    } else {
      return exprToString(E) + ";\n";
    }
  }

  /// True if E or any subexpr is a Call, Deref, or ArraySubscript.
  bool isEffectful(Expr *E) {
    E = E->IgnoreParenImpCasts();
    if (isa<CallExpr>(E) ||
        isa<UnaryOperator>(E) && cast<UnaryOperator>(E)->getOpcode() == UO_Deref ||
        isa<ArraySubscriptExpr>(E))
      return true;
    for (auto *C : E->children())
      if (Expr *CE = dyn_cast_or_null<Expr>(C))
        if (isEffectful(CE)) return true;
    return false;
  }

  /// Extract raw source text of an Expr.
  std::string exprToString(Expr *E) {
    auto R = CharSourceRange::getTokenRange(E->getSourceRange());
    return Lexer::getSourceText(R, SM, Context.getLangOpts()).str();
  }

  /// Extract raw text of a Stmt.
  std::string stmtToString(Stmt *S) {
    auto R = CharSourceRange::getTokenRange(S->getSourceRange());
    return Lexer::getSourceText(R, SM, Context.getLangOpts()).str();
  }

  /// Wrap a Stmt* into a `{ ... }` block text.
  std::string blockText(Stmt *S) {
    if (auto *CS = dyn_cast<CompoundStmt>(S))
      return stmtToString(CS);
    return "{ " + stmtToString(S) + " }";
  }

/// Preserve the single comment line immediately before Loc (if any).
std::string commentPrefix(SourceLocation Loc) {
  FileID FID = SM.getFileID(Loc);
  unsigned ThisLine = SM.getSpellingLineNumber(Loc);
  if (ThisLine <= 1) return "";

  // Get the start of the previous line
  SourceLocation PrevLineStart = SM.translateLineCol(FID, ThisLine - 1, 1);
  if (PrevLineStart.isInvalid()) return "";

  // Read that line
  bool Invalid = false;
  StringRef Buffer = SM.getBufferData(FID, &Invalid);
  if (Invalid) return "";
  // Extract the text of the previous line
  unsigned Offset = SM.getFileOffset(PrevLineStart);
  StringRef Remaining = Buffer.substr(Offset);
  StringRef PrevLine = Remaining.split('\n').first.trim();

  // Only keep if it’s a single-line comment
  if (PrevLine.starts_with("//"))
    return PrevLine.str() + "\n";
  return "";
}

};

/// ASTConsumer that drives ANFVisitor
class ANFConsumer : public ASTConsumer {
public:
  ANFConsumer(ASTContext &Ctx, Rewriter &R)
    : Visitor(R, Ctx) {}

  void HandleTranslationUnit(ASTContext &Ctx) override {
    Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  }

private:
  ANFVisitor Visitor;
};

/// Plugin entry point
class ANFFrontendAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
  RewriterForPlugin.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
  return std::make_unique<ANFConsumer>(CI.getASTContext(),
                                       RewriterForPlugin);
}

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    RewriterForPlugin
      .getEditBuffer(RewriterForPlugin.getSourceMgr().getMainFileID())
      .write(llvm::outs());
  }

private:
  Rewriter RewriterForPlugin;
};

} // namespace

static FrontendPluginRegistry::Add<ANFFrontendAction>
    X("anf-pulse", "Rewrite C to ANF and prepare for Pulse");