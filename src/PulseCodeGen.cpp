#include "PulseCodeGen.h"
#include "PulseIR.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <fstream>
#include <memory>
#include <vector>

using namespace clang;

void PulseCodeGen::writeHeaders(std::string ModuleName, std::ofstream &Stream) {

  Stream << PulseSyntax.ModuleSyntax << PulseSyntax.Space << ModuleName
         << PulseSyntax.NewLine;
  Stream << PulseSyntax.NewLine;
  Stream << PulseSyntax.LangPulse << PulseSyntax.NewLine;
  Stream << PulseSyntax.NewLine;
  Stream << PulseSyntax.PulseInclude << PulseSyntax.NewLine;
  Stream << PulseSyntax.NewLine;
}

void PulseCodeGen::generateCodeFromPulseAst(PulseDecl *FD) {

  if (PulseFnDefn *F = dyn_cast<PulseFnDefn>(FD)) {
    auto *FuncDef = F->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;
    auto *FuncBody = FuncDef->Body;

    OS << PulseSyntax.PulseFunctionDeclaration << " ";
    OS << FuncName << "\n";
    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;
      OS << PulseSyntax.OpeningParenthesis << Val << PulseSyntax.Space
         << PulseSyntax.Colon << PulseSyntax.Space << generateCodeFromTerm(Ty)
         << PulseSyntax.ClosingParenthesis;
      OS << "\n";
    }

    // Print out the Ensures
    for (const auto &A : FuncDef->Annotation) {
      OS << (A.kind == PulseAnnKind::Requires ? "requires " : "ensures  ")
         << A.predicate << "\n";
      //<< " |-> " << A.regionId << "\n";
    }

    // Codegen Function Body.
    OS << PulseSyntax.OpeningCurlyBrace << "\n";
    generateCodeFromPulseStmt(FuncBody);
    OS << PulseSyntax.ClosingCurlyBrace << "\n";

  } else {
    assert(false && "Not implemented function kind");
  }
}

// std::string PulseCodeGen::formatAsComments(PulseDecl *Decl)
// {

//     if (PulseFnDefn *FDefn = dyn_cast<PulseFnDefn>(Decl)){
//         //We should have a getter for this.
//         auto &Annotations = FDefn->Defn->Annotation;
//         std::string Out;
//         for (const PulseAnnotation& Ann : Annotations) {
//             Out += "// @";
//             Out += (Ann.kind == PulseAnnKind::Requires ? "requires " :
//             "ensures "); Out += Ann.predicate + "\n";
//         }
//         return Out;
//     }
//     assert(false && "Format As Comments not implemented yet!");

// }

std::string PulseCodeGen::generateCodeFromTerm(Term *T) {

  std::string TermString = "";
  if (ConstTerm *CT = dyn_cast<ConstTerm>(T)) {

  } else if (VarTerm *VT = dyn_cast<VarTerm>(T)) {
    TermString += VT->VarName;
  } else if (Name *N = dyn_cast<Name>(T)) {
    TermString += N->NamedValue;
  } else if (FStarType *FT = dyn_cast<FStarType>(T)) {

    // else {
    TermString += FT->NamedValue;
    //        }
  } else if (FStarPointerType *FPT = dyn_cast<FStarPointerType>(T)) {
    auto StrBase = generateCodeFromTerm(FPT->PointerTo);
    TermString += PulseSyntax.Reference;
    TermString += " " + StrBase;
  } else if (AppE *App = dyn_cast<AppE>(T)) {
    TermString += generateCodeFromTerm(App->CallName);
    TermString += " ";
    int i = 0;
    for (auto *Arg : App->Args) {
      TermString += generateCodeFromTerm(Arg);
      if (i < (App->Args).size() - 1) {
        TermString += " ";
      }
      i++;
    }
  } else {
    T->dumpPretty();
    assert(false && "Did not expect Term node in generateCodeFromTerm");
  }

  return TermString;
}

void PulseCodeGen::generateCodeFromPulseStmt(PulseStmt *T) {

  if (PulseExpr *S = dyn_cast<PulseExpr>(T)) {
    OS << generateCodeFromTerm(S->E);
  } else if (PulseAssignment *A = dyn_cast<PulseAssignment>(T)) {
    OS << generateCodeFromTerm(A->Lhs);
    OS << PulseSyntax.Space;
    OS << PulseSyntax.PulseAssignmentOpRef;
    OS << PulseSyntax.Space;
    OS << generateCodeFromTerm(A->Value);
    OS << PulseSyntax.Semicolon;
    OS << PulseSyntax.NewLine;

  } else if (PulseArrayAssignment *AS = dyn_cast<PulseArrayAssignment>(T)) {
    assert(false && "Did not expect pulse statement type");
  } else if (LetBinding *Let = dyn_cast<LetBinding>(T)) {
    OS << PulseSyntax.LetBind;
    OS << PulseSyntax.Space;
    OS << Let->VarName;
    OS << PulseSyntax.Space;
    OS << PulseSyntax.PulseLetAssignmentOpRef;
    OS << PulseSyntax.Space;
    OS << generateCodeFromTerm(Let->LetInit);
    OS << PulseSyntax.Semicolon;
    OS << PulseSyntax.NewLine;
  } else if (PulseIf *If = dyn_cast<PulseIf>(T)) {
    assert(false && "Did not expect pulse statement type");
  } else if (PulseWhileStmt *While = dyn_cast<PulseWhileStmt>(T)) {
    assert(false && "Did not expect pulse statement type");
  } else if (PulseSequence *Seq = dyn_cast<PulseSequence>(T)) {
    auto *S1 = Seq->S1;
    auto *S2 = Seq->S2;
    generateCodeFromPulseStmt(S1);
    generateCodeFromPulseStmt(S2);
  } else {
    assert(false && "Did not expect pulse statement type");
  }
}