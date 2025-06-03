#pragma once

#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
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
          std::string StmtToStr;
          llvm::raw_string_ostream os(StmtToStr);
          CS->printPretty(os, nullptr, Ctx.getPrintingPolicy());
          DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                          << "The compound statement is: "
                                          << os.str() << "\n");
        }
        auto NewText = rewriteCompound(CS);
        TheRewriter.ReplaceText(CS->getSourceRange(), NewText);
        TransformedCode = NewText;
      }
    }
    
    return true;
  }

  std::string getTransformedCode() const { return TransformedCode; }

private:
  Rewriter &TheRewriter;
  ASTContext &Ctx;
  SourceManager &SM;
  std::map<Expr*, std::string> MapExprToAssignedTemporary;
  std::set<SourceLocation> LocsSeen;
  std::set<unsigned> LinesSeen;
  int Counter;
  std::string TransformedCode;

  void insertExprAndTemp(Expr *Expr, std::string TempName) {
    // Insert the expression and its temporary name into the map
    MapExprToAssignedTemporary[Expr] = TempName;
  }

  std::string lookupExprTempVal(Expr* Expr){

    //assert that the expression is in the map
    auto It = MapExprToAssignedTemporary.find(Expr);
    //Remove temporary for now
    //assert(it != MapExprToAssignedTemporary.end() && "Expression not found in map");
    if (It == MapExprToAssignedTemporary.end()) {
      // If not found, create a new temporary
      std::string NewTemp = "";
      return NewTemp;
    }

    return It->second;
  }

  std::string lookupFinalExpr(Expr *Expr){

    auto It = MapExprToAssignedTemporary.find(Expr);
    if (It == MapExprToAssignedTemporary.end()) {
      return rewriteStmt(Expr);
    }
    return It->second;
  }

  std::string freshTemp() {
    return "__anf_tmp" + std::to_string(Counter++);
  }

  /// Rewrite a compound statement in-place.
  std::string rewriteCompound(Stmt *St, std::string AppendBefore = "") {

    if (auto *CS = dyn_cast<CompoundStmt>(St)) {
      std::string NewText = "{\n";
      //Append any instructions if needed before.
      NewText += AppendBefore;
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

  std::string getTyOfExprAsString(Expr *E) {
    if (!E)
      return "";

    auto ExprType = E->getType();

    if (ExprType->isVoidType()) {
      return "";
    }

    return ExprType.getAsString();
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

    // define a lambda function to get type from a variable declaration
    auto getExprFromVarDecl = [this](const clang::VarDecl *VD) {
      std::string Expr;
      if (VD->getType()->isConstantArrayType()) {
        auto *ArrTy = cast<clang::ConstantArrayType>(VD->getType());
        Expr += ArrTy->getElementType().getAsString() + " " +
                VD->getNameAsString() + "[" +
                std::to_string(ArrTy->getSize().getZExtValue()) + "]";

      } else if (VD->getType()->isVariableArrayType()) {
        auto *ArrTy = cast<clang::VariableArrayType>(VD->getType());
        Expr += ArrTy->getElementType().getAsString() + " " +
                VD->getNameAsString() + "[" +
                exprToString(ArrTy->getSizeExpr()) + "]";
      } else {
        Expr = VD->getType().getAsString() + " " + VD->getNameAsString();
      }
      return Expr;
    };

    if (auto *DS = dyn_cast<DeclStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) DeclStmt: " << "\n");
      //S->dumpPretty(Ctx);

      std::string StrForDecls;
      for (auto *D : DS->decls()) {
        if (auto *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
          if (clang::Expr *Initializer = VD->getInit()) {
              // Initializer now holds the right-hand side expression
              auto InitExpr = rewriteStmt(Initializer);
              auto TempForInit = lookupExprTempVal(Initializer);
              if (TempForInit == "") {
                TempForInit = exprToString(Initializer);
                StrForDecls +=
                    getExprFromVarDecl(VD) + " = " + TempForInit + ";\n";
              } else {
                StrForDecls += InitExpr;
                StrForDecls +=
                    getExprFromVarDecl(VD) + " = " + TempForInit + ";\n";
              }
          } else {
            Out += getExprFromVarDecl(VD) + ";\n";
            // if (VD->getType()->isConstantArrayType()){
            //   auto *ArrTy = cast<clang::ConstantArrayType>(VD->getType());
            //   Out +=  ArrTy->getElementType().getAsString() + " " +
            //          VD->getNameAsString() + "[" +
            //             std::to_string(ArrTy->getSize().getZExtValue()) +
            //             "];\n";

            // }
            // else if (VD->getType()->isVariableArrayType()) {
            //   auto *ArrTy = cast<clang::VariableArrayType>(VD->getType());
            //   Out += ArrTy->getElementType().getAsString() + " " +
            //          VD->getNameAsString() + "[" +
            //           exprToString(ArrTy->getSizeExpr()) + "];\n";
            // }
            // else{
            //   Out += VD->getType().getAsString() + " " +
            //          VD->getNameAsString() + ";\n";
            // }
          }
        }
        else {
          assert(false && "Unhandled declaration type in DeclStmt");
        }
      }

      Out += StrForDecls;
    }
    //unwrap paren expressions
    else if (auto *PE = llvm::dyn_cast<clang::ParenExpr>(S)){

      auto *InnerExpr = PE->getSubExpr();
      auto InnerString = rewriteStmt(InnerExpr);
      auto TempForInner = lookupExprTempVal(InnerExpr);
      //Same value with or without the parenthesis.
      insertExprAndTemp(PE, TempForInner);
      // if (tempForInner == ""){
      //   tempForInner = freshTemp();
      // }
      // insertExprAndTemp(PE, tempForInner);
      Out += InnerString;
      //Out += "(" + tempForInner + ")";

    }
    else if (auto *RS = dyn_cast<ReturnStmt>(S)) {
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) ReturnStmt: " << "\n");
      //S->dumpPretty(Ctx);
      Out += rewriteReturn(RS);
    }
    // Broken for unary operators. Vidush: TODO: Fix this.
    else if (auto *US = dyn_cast<UnaryOperator>(S)) {

      // Out += stmtToString(US) + ";\n";
      
      //Desugar some unary operators if possible. 
      switch (US->getOpcode()) {
      case UO_PostInc: {
        auto *SubExpr = US->getSubExpr();
        auto SubExprStr =
            rewriteStmt(SubExpr) + " = " + exprToString(SubExpr) + " + 1;\n";
        Out += SubExprStr;
        break;
      }
      case UO_PostDec: {
        auto *SubExpr = US->getSubExpr();
        auto SubExprStr =
            rewriteStmt(SubExpr) + " = " + exprToString(SubExpr) + " + 1;\n";
        Out += SubExprStr;
        break;
      }
      case UO_PreInc:
      case UO_PreDec:
      case UO_AddrOf: {
        Out += stmtToString(US) + ";\n";
        break;
      }
      case UO_Deref: {
        Out += stmtToString(US) + ";\n";
        break;
      }
      case UO_Plus:
      case UO_Minus:
      case UO_Not:
      case UO_LNot:
      case UO_Real:
      case UO_Imag:
      case UO_Extension:
      case UO_Coawait:
        assert(false && "Unary operator not handled in ANF rewriter");
        break;
      }

    } else if (auto *BO = dyn_cast<BinaryOperator>(S)) {
      if (BO->isAssignmentOp()){
        DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) BO assignment: " << "\n");
        Out += rewriteAssignment(BO);
      }
      else{
        DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print in (rewriteStmt) BO not assignment: " << "\n");
        //S->dumpPretty(Ctx);

        auto *LeftExpr = BO->getLHS();
        auto *RightExpr = BO->getRHS();

        auto OperatorAsString = BO->getOpcodeStr();

        auto TyOfExpr = getTyOfExprAsString(BO);

        auto NewLeftExpr = rewriteStmt(LeftExpr);
        auto NewRightExpr = rewriteStmt(RightExpr);

        auto LeftTemp = lookupExprTempVal(LeftExpr);
        auto RightTemp = lookupExprTempVal(RightExpr);

        auto TempForBO = freshTemp();
        if (LeftTemp == "" && RightTemp == "") {
          LeftTemp = NewLeftExpr;
          RightTemp = NewRightExpr;

          if (TyOfExpr == "") {
            Out += LeftTemp + " " + OperatorAsString.str() + " " + RightTemp +
                   ";\n";
          } else {
            Out += TyOfExpr + " " + TempForBO + " = " + LeftTemp + " " +
                   OperatorAsString.str() + " " + RightTemp + ";\n";
          }
        } else if (LeftTemp != "" && RightTemp == "") {
          RightTemp = NewRightExpr;
          Out += NewLeftExpr;

          if (TyOfExpr == "") {
            Out += LeftTemp + " " + OperatorAsString.str() + " " + RightTemp +
                   ";\n";
          } else {
            Out += TyOfExpr + " " + TempForBO + " = " + LeftTemp + " " +
                   OperatorAsString.str() + " " + RightTemp + ";\n";
          }

        } else if (LeftTemp == "" && RightTemp != "") {
          LeftTemp = NewLeftExpr;
          Out += NewRightExpr;

          if (TyOfExpr == "") {
            Out += LeftTemp + " " + OperatorAsString.str() + " " + RightTemp +
                   ";\n";
          } else {
            Out += TyOfExpr + " " + TempForBO + " = " + LeftTemp + " " +
                   OperatorAsString.str() + " " + RightTemp + ";\n";
          }

        } else {
          Out += NewLeftExpr;
          Out += NewRightExpr;

          if (TyOfExpr == "") {
            Out += LeftTemp + " " + OperatorAsString.str() + " " + RightTemp +
                   ";\n";
          } else {
            Out += TyOfExpr + " " + TempForBO + " = " + LeftTemp + " " +
                   OperatorAsString.str() + " " + RightTemp + ";\n";
          }
        }

       //std::string new_bin_inst = TyOfExpr + " " + tempForBO + " = " + lhsTemp + " " + opAsString.str() + " " + rhsTemp + ";\n";
        insertExprAndTemp(BO, TempForBO);
        // std::string newExpr = newLhsExpr + newRhsExpr;

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
    } else if (auto *FS = dyn_cast<ForStmt>(S)) {
      Out += rewriteFor(FS);
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
          std::string FreshTmp = freshTemp();
          insertExprAndTemp(E, FreshTmp);

          if (CallExpr *Call = dyn_cast<CallExpr>(E)) {
            int NumArgs = Call->getNumArgs();
            std::vector<std::string> FinalArgs;
            for (int i = 0; i < NumArgs; i++) {
              Expr *CallArg = Call->getArg(i);

              if (isEffectful(CallArg) || !isLeafNode(CallArg)) {
                DEBUG_WITH_TYPE(DEBUG_TYPE,
                                llvm::dbgs() << "Effectful arg: "
                                             << exprToString(CallArg) << "\n");
                Out += rewriteStmt(CallArg);
                auto ArgToExpr = lookupExprTempVal(CallArg);
                if (ArgToExpr == "") {
                  ArgToExpr = exprToString(CallArg);
                }
                FinalArgs.push_back(ArgToExpr);
              } else {
                auto ArgToExpr = exprToString(CallArg);
                FinalArgs.push_back(ArgToExpr);
              }
            }

            auto CallName = Call->getDirectCallee()->getNameAsString();

            auto Ty = getTyOfExprAsString(E);
            if (Ty == "") {
              Out += CallName + "(" + llvm::join(FinalArgs, ", ") + ");\n";
            } else {
              Out += Ty + " " + FreshTmp + " = " + CallName + "(" +
                     llvm::join(FinalArgs, ", ") + ");\n";
            }
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
      Out += stmtToString(S) + ";\n";
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
            std::string FreshTmp = freshTemp();
            insertExprAndTemp(Init, FreshTmp);
            Out += VD->getType().getAsString() + " " + FreshTmp + " = " +
                   exprToString(Init) + ";\n";
            // rewrite original init to tmp
            Out += VD->getType().getAsString() + " " + VD->getNameAsString() +
                   " = " + FreshTmp + ";\n";
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
        std::string NewExpr = rewriteStmt(E);
        //DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print expr in return: " << newExpr << ", " << exprToString(E) << "\n");
        //return Ty + " " + tmp + " = " + newExpr  + ";\n"
        //     + "return " + tmp + ";\n";

        auto TempForNewExpr = lookupExprTempVal(E);
        if (TempForNewExpr == "") {
          TempForNewExpr = exprToString(E);
          return "return " + TempForNewExpr + ";\n";
        }

        return NewExpr + "return " + TempForNewExpr + ";\n";
      }
      std::string NewExpr = rewriteStmt(E);
      DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << "Print expr in return: "
                                               << NewExpr << "\n");
      auto TempForNewExpr = lookupExprTempVal(E);
      if (TempForNewExpr == "") {
        TempForNewExpr = exprToString(E);
        return "return " + TempForNewExpr + ";\n";
      }

      return NewExpr + "return " + TempForNewExpr + ";\n";
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

    auto NewRight = rewriteStmt(R);
    auto TempForRight = lookupExprTempVal(R);
    if (TempForRight == "") {
      Out += exprToString(L) + " " + BO->getOpcodeStr().str() + " " + exprToString(R) + ";\n";
    } else {
      Out += NewRight;
      Out += exprToString(L) + " = " + TempForRight + ";\n";
    }

    return Out;
  }

/// Rewrite an if statement, lifting its condition.
std::string rewriteIf(IfStmt *IS) {
  Expr *Cond = IS->getCond();
  Stmt *ThenBody = IS->getThen();
  std::string Out;

  // Get the source‐spelled type of the condition
  std::string Ty = getTyOfExprAsString(Cond); // Cond->getType().getAsString();

  if (isEffectful(Cond) || !isLeafNode(Cond)) {
    //std::string tmp = freshTemp();
    //insertExprAndTemp(Cond, tmp);

    auto NewCond = rewriteStmt(Cond);
    auto TempForCond = lookupExprTempVal(Cond);
    if (TempForCond == "") {
      TempForCond = exprToString(Cond);
    }

    Out += NewCond;
    //Out += Ty + " " + tmp + " = " + exprToString(Cond) + ";\n";
    Out += "if (" + TempForCond + ") " + rewriteCompound(IS->getThen()) + "\n";
  } else {
    Out += "if (" + exprToString(Cond) + ") " + rewriteCompound(IS->getThen()) + "\n";
  }

  if (Stmt *E = IS->getElse())
    Out += "else " + rewriteCompound(E) + "\n";

  return Out;
}

std::string rewriteFor(ForStmt *FS) {

  std::string Out;
  Expr *Cond = FS->getCond();
  Stmt *Body = FS->getBody();

  Stmt *Init = FS->getInit();
  Stmt *Inc = FS->getInc();

  auto InitStr = stmtToString(Init);

  // This is just a sad hack.
  //  We need to have a pretty printer that print precise instructions.

  if (InitStr.back() != ';') {
    InitStr += ";";
  }

  auto IncStr = stmtToString(Inc);
  // For now we do not flatten the Condition.
  // If it is complex, we will need to find a way to update the temporary.
  auto CondStr = stmtToString(Cond);

  Out += "for (" + InitStr + " " + CondStr + "; " + IncStr + ")";

  if (auto *CS = dyn_cast<CompoundStmt>(Body)) {
    Out += rewriteCompound(CS);
  } else {
    Out += "{\n";
    Out += rewriteCompound(Body);
    Out += "}\n";
  }

  return Out;
}

/// Rewrite a while statement, lifting its condition.
/// Vidush: TODO: We need more analysis for while loops.
/// Since, we may change the condition to a temporary variable,
/// we need to also ensure that the temporary is updated accordingly.
/// This may need data flow analysis??
std::string rewriteWhile(WhileStmt *WS) {
  Expr *Cond = WS->getCond();

  std::string Ty = getTyOfExprAsString(Cond); // Cond->getType().getAsString();
  std::string Out;

  if (isEffectful(Cond) || !isLeafNode(Cond)) {

    auto NewCond = rewriteStmt(Cond);
    auto TempForCond = lookupExprTempVal(Cond);
    std::string AppendBefore = "";
    //The condition is effectful and we get a temp for it. 
    //We need to ensure that the temp is updated in the loop.
    //We can do this by inserting the temp in the loop body.
    std::string tmp = freshTemp();
    insertExprAndTemp(Cond, tmp);

    if (TempForCond == "") {
      TempForCond = exprToString(Cond);
    } else {
      AppendBefore = NewCond;
      if (Ty == "") {
        AppendBefore += TempForCond + ";\n";
      } else {
        AppendBefore += Ty + " " + tmp + " = " + TempForCond + ";\n";
      }
      // break on the inverse of the condition.
      AppendBefore += "if (!" + tmp + ") { break; }\n";
    }

    //Out += newCond;
    //Out += Ty + tmp + " = " + tempForCond + ";\n"; 

    //Transform the while loop to use the temp variable.
    if (AppendBefore != "") {
      Out += "while (1) " + rewriteCompound(WS->getBody(), AppendBefore) + "\n";
    } else {
      Out += "while (" + TempForCond + ") " +
             rewriteCompound(WS->getBody(), AppendBefore) + "\n";
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
      std::string FreshTemp = freshTemp();
      insertExprAndTemp(E, FreshTemp);
      // E->getType().getAsString()

      auto Ty = getTyOfExprAsString(E);
      if (Ty == "") {
        return exprToString(E) + ";\n";
      } else {
        return Ty + " " + FreshTemp + " = " + exprToString(E) + ";\n";
      }

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
  std::string exprToString(const Expr *E) {
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