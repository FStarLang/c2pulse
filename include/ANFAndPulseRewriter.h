#pragma once

#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "llvm/Support/Debug.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <memory>
#include <vector>

extern llvm::cl::OptionCategory ToolCategory;
extern llvm::cl::opt<std::string> FunctionNameToProcess;
extern llvm::cl::opt<std::string> TransformMode;
using namespace clang;

#define DEBUG_TYPE "anf-pulse-rewriter"

enum class ANFTransformDebugModeKind
{
    ANFOnly,
    PulseOnly,
    Both
};

static bool isLeafNode(const clang::Expr* E) {
  if (!E) return false;
  if (llvm::isa<clang::IntegerLiteral>(E) ||
        llvm::isa<clang::FloatingLiteral>(E) ||
        llvm::isa<clang::StringLiteral>(E)) {
        return true;
    }

    //Remove implicit casts to show true underlying expression
    if (auto* ICE = llvm::dyn_cast<clang::ImplicitCastExpr>(E)) {
        return isLeafNode(ICE->getSubExpr());
    }

    //We should remove variable declarations and parameters that are variable declarations.
    if (auto* DRE = llvm::dyn_cast<clang::DeclRefExpr>(E)) {
        if (llvm::isa<clang::ParmVarDecl>(DRE->getDecl()) || 
            llvm::isa<clang::VarDecl>(DRE->getDecl())) {
            return true; // Function parameter or variable reference is a leaf
        }
    }
    
    //Ideally we should desugar such expressions because pulse may not support them.
    if (auto* UO = llvm::dyn_cast<clang::UnaryOperator>(E)) {
        return isLeafNode(UO->getSubExpr());
    }

    return false;
}


static ANFTransformDebugModeKind getTransformMode()
{
    if (TransformMode == "anf")
        return ANFTransformDebugModeKind::ANFOnly;
    if (TransformMode == "pulse")
        return ANFTransformDebugModeKind::PulseOnly;
    return ANFTransformDebugModeKind::Both;
}

namespace {
/// \brief Visitor that rewrites effectful expressions into ANF.
class ANFVisitor : public RecursiveASTVisitor<ANFVisitor> {
public:
  ANFVisitor(Rewriter &R, ASTContext &Ctx)
    : TheRewriter(R), Ctx(Ctx), SM(Ctx.getSourceManager()), Counter(0) {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody() || SM.isInSystemHeader(FD->getLocation()))
      return true;

    // Check for function name match if specified
    if (!FunctionNameToProcess.empty() && FD->getNameAsString() != FunctionNameToProcess) {
        return true;
    }

    // Always apply ANF rewriting on user functions
    if (Stmt *Body = FD->getBody()) {
      if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
        if (llvm::DebugFlag && CS) {
          std::string stmtStr;
          llvm::raw_string_ostream os(stmtStr);
          CS->printPretty(os, nullptr, Ctx.getPrintingPolicy());
          DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                          << "The compound statement is: "
                                          << os.str() << "\n");
        }
        auto NewText = rewriteCompound(CS);
        TheRewriter.ReplaceText(CS->getSourceRange(), NewText);
        transformedCode = NewText;
      }
    }
    
    return true;
  }

  std::string getTransformedCode() const { return transformedCode; }

private:
  Rewriter &TheRewriter;
  ASTContext &Ctx;
  SourceManager &SM;
  std::map<Expr*, std::string> MapExprToAssignedTemporary;
  std::set<SourceLocation> LocsSeen;
  std::set<unsigned> LinesSeen;
  int Counter;
  std::string transformedCode;

  void insertExprAndTemp(Expr* expr, std::string tempName) {
    // Insert the expression and its temporary name into the map
    MapExprToAssignedTemporary[expr] = tempName;

  }

  std::string lookupExprTempVal(Expr* expr){

    //assert that the expression is in the map
    auto it = MapExprToAssignedTemporary.find(expr);
    //Remove temporary for now
    //assert(it != MapExprToAssignedTemporary.end() && "Expression not found in map");
    if (it == MapExprToAssignedTemporary.end()) {
      // If not found, create a new temporary
      std::string newTemp = "";
      return newTemp;
    }

    return it->second;

  }

  std::string lookupFinalExpr(Expr *expr){
    
    auto it = MapExprToAssignedTemporary.find(expr);
    if (it == MapExprToAssignedTemporary.end()) {
      return rewriteStmt(expr);
    }
    return it->second;
  }

  std::string freshTemp() {
    return "__anf_tmp" + std::to_string(Counter++);
  }

  /// Rewrite a compound statement in-place.
  std::string rewriteCompound(Stmt *St, std::string appendBefore = "") {

    if (auto *CS = dyn_cast<CompoundStmt>(St)) {
      std::string NewText = "{\n";
      //Append any instructions if needed before.
      NewText += appendBefore;
      for (Stmt *S : CS->body()) {
        NewText += rewriteStmt(S);
      }
      NewText += "}\n";
      return NewText;
    }
    else {
      return rewriteStmt(St);
    }
  }

  /// Rewrite a single statement, returning its text (including lifted temps).
  std::string rewriteStmt(Stmt *S) {
    std::string Out;
    // Preserve leading comments/whitespace

    if (LocsSeen.count(S->getBeginLoc()) == 0) {
      LocsSeen.insert(S->getBeginLoc());
      Out += commentPrefix(S->getBeginLoc());
    }
    // else{
    //   Out += "\n";
    // }
    // SourceLocation B = S->getBeginLoc();
    // Out += commentPrefix(B);

    if (auto *DS = dyn_cast<DeclStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) DeclStmt: " << "\n");
      //S->dumpPretty(Ctx);
      
      std::string strForDecls;
      for (auto *D : DS->decls()) {
        if (auto *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
          if (clang::Expr *Initializer = VD->getInit()) {
              // Initializer now holds the right-hand side expression
              auto initExpr = rewriteStmt(Initializer);
              auto tempForInit = lookupExprTempVal(Initializer);
              if (tempForInit == "") {
                tempForInit = exprToString(Initializer);
                strForDecls += VD->getType().getAsString() + " " + VD->getNameAsString() + " = " + tempForInit + ";\n";
              }
              else{
                strForDecls += initExpr;
                strForDecls += VD->getType().getAsString() + " " + VD->getNameAsString() + " = " + tempForInit + ";\n";
              }
          }
        }
        else {
          assert(true && "Unhandled declaration type in DeclStmt");
        }
      }

      Out += strForDecls;
    }
    //unwrap paren expressions
    else if (auto *PE = llvm::dyn_cast<clang::ParenExpr>(S)){

      auto innerExpr = PE->getSubExpr();
      auto innerString = rewriteStmt(innerExpr);
      auto tempForInner = lookupExprTempVal(innerExpr);
      //Same value with or without the parenthesis.
      insertExprAndTemp(PE, tempForInner);
      // if (tempForInner == ""){
      //   tempForInner = freshTemp();
      // }
      // insertExprAndTemp(PE, tempForInner);
      Out += innerString;
      //Out += "(" + tempForInner + ")";

    }
    else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) ReturnStmt: " << "\n");
      //S->dumpPretty(Ctx);
      Out += rewriteReturn(RS);
    }
    // Broken for unary operators. Vidush: TODO: Fix this.
    else if (auto *US = dyn_cast<UnaryOperator>(S)) {
      Out += stmtToString(US) + ";\n";
    } else if (auto *BO = dyn_cast<BinaryOperator>(S)) {
      if (BO->isAssignmentOp()){
        DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) BO assignment: " << "\n");
        //S->dumpPretty(Ctx);
        Out += rewriteAssignment(BO);
      }
      else{
        DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) BO not assignment: " << "\n");
        //S->dumpPretty(Ctx);

       auto lhsExpr = BO->getLHS();
       auto rhsExpr = BO->getRHS();

       auto opAsString = BO->getOpcodeStr();

       auto TyOfExpr = BO->getType().getAsString();

       auto newLhsExpr = rewriteStmt(lhsExpr);
       auto newRhsExpr = rewriteStmt(rhsExpr);


       auto lhsTemp = lookupExprTempVal(lhsExpr);
       auto rhsTemp = lookupExprTempVal(rhsExpr);
       
       auto tempForBO = freshTemp();
       if (lhsTemp == "" && rhsTemp == ""){
          lhsTemp = newLhsExpr;
          rhsTemp = newRhsExpr; 
          Out += TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
       }
       else if (lhsTemp != "" && rhsTemp == ""){
          rhsTemp  = newRhsExpr; 
          Out += newLhsExpr; 
          Out += TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
       }
       else if (lhsTemp == "" && rhsTemp != ""){
          lhsTemp  = newLhsExpr; 
          Out += newRhsExpr; 
          Out += TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
       }
       else{
          Out += newLhsExpr;
          Out += newRhsExpr; 
          Out += TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
       }
       
       //std::string new_bin_inst = TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
       insertExprAndTemp(BO, tempForBO);
       //std::string newExpr = newLhsExpr + newRhsExpr;

        //Out += newExpr + new_bin_inst ;
      }
    } else if (auto *IS = dyn_cast<IfStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) IfStmt: " << "\n");
      //S->dumpPretty(Ctx);
      Out += rewriteIf(IS);
    } else if (auto *WS = dyn_cast<WhileStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) WhileStmt: " << "\n");
      //S->dumpPretty(Ctx);
      Out += rewriteWhile(WS);
    }
    // else if (CallExpr *Call = dyn_cast<CallExpr>(S)){

    //           std::string tmp = freshTemp();
    //           insertExprAndTemp(Call, tmp);

    //           int numArgs = Call->getNumArgs();
    //           std::vector<std::string> finalArgs;
    //           for (int i =0; i < numArgs; i++){
    //             Expr *arg = Call->getArg(i);
    //             finalArgs.push_back(lookupFinalExpr(arg));
    //           }

    //           auto callName = Call->getDirectCallee()->getNameAsString();
    //           Out += Call->getType().getAsString() + " " + tmp + " = "
    //                + callName + "(" + llvm::join(finalArgs, ", ") + ");\n";
    // }
    else if (auto *E = dyn_cast<Expr>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) Expr: " <<  E->getStmtClassName() << "\n");
      ///S->dumpPretty(Ctx);

      switch(E->getStmtClass()) {
        case Stmt::CallExprClass:
        {
            std::string tmp = freshTemp();
            insertExprAndTemp(E, tmp);

            if (CallExpr *Call = dyn_cast<CallExpr>(E)){
              int numArgs = Call->getNumArgs();
              std::vector<std::string> finalArgs;
              for (int i =0; i < numArgs; i++){
                Expr *arg = Call->getArg(i);
                
                if (isEffectful(arg) || !isLeafNode(arg)) {
                  DEBUG_WITH_TYPE(DEBUG_TYPE , llvm::dbgs() << "Effectful arg: " << exprToString(arg) << "\n");
                  Out += rewriteStmt(arg);
                  auto arg_expr = lookupExprTempVal(arg);
                  if (arg_expr == "") {
                    arg_expr = exprToString(arg);
                  }
                  finalArgs.push_back(arg_expr);
                }
                else{
                  auto arg_expr = exprToString(arg);
                  finalArgs.push_back(arg_expr);
                }
              }

              auto callName = Call->getDirectCallee()->getNameAsString();
              Out += E->getType().getAsString() + " " + tmp + " = "
                   + callName + "(" + llvm::join(finalArgs, ", ") + ");\n";
            }
            // Out += E->getType().getAsString() + " " + tmp + " = "
            //      + exprToString(E) + ";\n";
          break;
        }
        default:
          // Other expressions are printed as-is
          Out += rewriteExprStmt(E);
          break;
      }

    } else {
      // Fallback: print as-is
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in fallback: " << "\n");
      //S->dumpPretty(Ctx);
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
            insertExprAndTemp(Init, tmp);
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
        //std::string tmp = freshTemp();
        //insertExprAndTemp(E, tmp);
        //std::string Ty  = E->getType().getAsString();
        std::string newExpr = rewriteStmt(E);
        //DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print expr in return: " << newExpr << ", " << exprToString(E) << "\n");
        //return Ty + " " + tmp + " = " + newExpr  + ";\n"
        //     + "return " + tmp + ";\n";

        auto tempForExprs = lookupExprTempVal(E);
        if (tempForExprs == ""){
          tempForExprs = exprToString(E);
          return "return " + tempForExprs + ";\n";
        }

        return newExpr  + "return " + tempForExprs + ";\n";
      }
      std::string newExpr = rewriteStmt(E);
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print expr in return: " << newExpr << "\n");
      auto tempForExprs = lookupExprTempVal(E);
      if (tempForExprs == ""){
        tempForExprs = exprToString(E);
        return "return " + tempForExprs + ";\n";
      }

      return newExpr 
             + "return " + tempForExprs + ";\n";
    }
    return "return;\n";
  }

  /// Rewrite an assignment, lifting RHS if needed.
  std::string rewriteAssignment(BinaryOperator *BO) {
    Expr *L = BO->getLHS(), *R = BO->getRHS();
    std::string Out;
    // if (isEffectful(R)) {
    //   std::string tmp = freshTemp();
    //   insertExprAndTemp(R, tmp);
    //   Out += R->getType().getAsString() + " "
    //        + tmp + " = " + exprToString(R) + ";\n";
    //   Out += exprToString(L) + " = " + tmp + ";\n";
    // } else {
    //   Out += exprToString(L) + " = " + exprToString(R) + ";\n";
    // }

    auto newR = rewriteStmt(R);
    auto tempR = lookupExprTempVal(R);
    if (tempR == ""){
      Out += exprToString(L) + " = " + exprToString(R) + ";\n";
    }
    else {
      Out += newR;
      Out += exprToString(L) + " = " + tempR + ";\n";
    }

    return Out;
  }

/// Rewrite an if statement, lifting its condition.
std::string rewriteIf(IfStmt *IS) {
  Expr *Cond = IS->getCond();
  Stmt *thenBody = IS->getThen();
  std::string Out;

  // Get the source‐spelled type of the condition
  std::string Ty = Cond->getType().getAsString();

  if (isEffectful(Cond) || !isLeafNode(Cond)) {
    //std::string tmp = freshTemp();
    //insertExprAndTemp(Cond, tmp);

    auto newCond = rewriteStmt(Cond);
    auto tempForCond = lookupExprTempVal(Cond);
    if (tempForCond == ""){
      tempForCond = exprToString(Cond);
    }

    Out += newCond;
    //Out += Ty + " " + tmp + " = " + exprToString(Cond) + ";\n";
    Out += "if (" + tempForCond + ") " + rewriteCompound(IS->getThen()) + "\n";
  } else {
    Out += "if (" + exprToString(Cond) + ") " + rewriteCompound(IS->getThen()) + "\n";
  }

  if (Stmt *E = IS->getElse())
    Out += "else " + rewriteCompound(E) + "\n";

  return Out;
}

/// Rewrite a while statement, lifting its condition.
/// Vidush: TODO: We need more analysis for while loops.
/// Since, we may change the condition to a temporary variable,
/// we need to also ensure that the temporary is updated accordingly.
/// This may need data flow analysis??
std::string rewriteWhile(WhileStmt *WS) {
  Expr *Cond = WS->getCond();

  std::string Ty = Cond->getType().getAsString();
  std::string Out;

  if (isEffectful(Cond) || !isLeafNode(Cond)) {

    auto newCond = rewriteStmt(Cond);
    auto tempForCond = lookupExprTempVal(Cond);
    std::string appendBefore = "";
    //The condition is effectful and we get a temp for it. 
    //We need to ensure that the temp is updated in the loop.
    //We can do this by inserting the temp in the loop body.
    std::string tmp = freshTemp();
    insertExprAndTemp(Cond, tmp);

    if (tempForCond == "") {
      tempForCond = exprToString(Cond);
    }
    else {
      appendBefore = newCond;
      appendBefore += Ty + " " + tmp + " = " + tempForCond + ";\n";
      appendBefore += "if (" + tmp + ") { break; }\n";
    }
    
    //Out += newCond;
    //Out += Ty + tmp + " = " + tempForCond + ";\n"; 

    //Transform the while loop to use the temp variable.
    if (appendBefore != ""){
      Out += "while (true) " + rewriteCompound(WS->getBody(), appendBefore) + "\n";
    }
    else {
      Out += "while (" + tempForCond + ") " +
             rewriteCompound(WS->getBody(), appendBefore) + "\n";
    }
  } else {
    Out += "while (" + exprToString(Cond) + ") " +
           rewriteCompound(WS->getBody()) + "\n";
  }
  return Out;
}

  /// Rewrite a standalone expr stmt.
  std::string rewriteExprStmt(Expr *E) {
    if (isEffectful(E)) {
      //if (isLeafNode(E)){
      //  return exprToString(E);
      //}
      //else {
        std::string tmp = freshTemp();
        insertExprAndTemp(E, tmp);
        return E->getType().getAsString() + " " + tmp + " = " + exprToString(E) + ";\n";
      //}
    } else {
      if (isLeafNode(E)){
        return exprToString(E);
      }
      else{
        return exprToString(E);
      }
    }
  }

  /// True if E or any subexpr is a Call, Deref, or ArraySubscript.
  bool isEffectful(const Expr *E) {
    if (!E)
      return false;

    E = E->IgnoreParenImpCasts();

    // 1. Check if Clang believes there are side effects
    if (E->HasSideEffects(Ctx, /*IncludePossibleEffects=*/true))
      return true;

    // 2. Handle known effectful expression kinds explicitly
    switch (E->getStmtClass()) {
      case Stmt::CallExprClass:
      case Stmt::CXXConstructExprClass:
      case Stmt::CXXNewExprClass:
      case Stmt::CXXDeleteExprClass:
        return true;

      case Stmt::UnaryOperatorClass: {
        const auto *UO = cast<UnaryOperator>(E);
        if (UO->getOpcode() == UO_Deref)
          return true;
        break;
      }

      case Stmt::ArraySubscriptExprClass:
        return true;

      case Stmt::MemberExprClass: {
        const auto *ME = cast<MemberExpr>(E);
        // s->x is effectful (pointer access), s.x is not
        if (ME->isArrow())
          return true;
        break;
      }

      default:
        break;
    }

    // 3. Recursively check children
    for (const Stmt *Child : E->children()) {
      if (const auto *ChildExpr = dyn_cast_or_null<Expr>(Child)) {
        if (isEffectful(ChildExpr))
          return true;
      }
    }

    return false;
  }

  /// Extract raw source text of an Expr.
  std::string exprToString(Expr *E) {
    auto R = CharSourceRange::getTokenRange(E->getSourceRange());
    return Lexer::getSourceText(R, SM, Ctx.getLangOpts()).str();
  }

  /// Extract raw text of a Stmt.
  std::string stmtToString(Stmt *S) {
    auto R = CharSourceRange::getTokenRange(S->getSourceRange());
    return Lexer::getSourceText(R, SM, Ctx.getLangOpts()).str();
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
  
  if (LinesSeen.count(ThisLine) > 0) {
    // Already processed this line, no need to repeat
    return "";
  }

  LinesSeen.insert(ThisLine);

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

} // namespace