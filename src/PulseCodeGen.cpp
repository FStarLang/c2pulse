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
#include <cstddef>
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
    
    if (FuncDef->isRecursive){
      OS << PulseSyntax.PulseRecursiveFunctionDeclaration << " ";
    }
    else {
      OS << PulseSyntax.PulseFunctionDeclaration << " ";
    }
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
    for (auto *A : FuncDef->Annotation) {
      generateCodeFromTerm(A);
    }

    // Codegen Function Body.
    OS << PulseSyntax.OpeningCurlyBrace << PulseSyntax.NewLine;
    generateCodeFromPulseStmt(FuncBody);
    OS << PulseSyntax.ClosingCurlyBrace << PulseSyntax.NewLine;

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
  if (Paren *P = dyn_cast<Paren>(T)){
    TermString += PulseSyntax.OpeningParenthesis; 
    TermString += generateCodeFromTerm(P->InnerExpr);
    TermString += PulseSyntax.ClosingParenthesis;
  }
  else if (ConstTerm *CT = dyn_cast<ConstTerm>(T)) {
    switch(CT->Symbol){
    case SymbolTable::Int32:{
         TermString += CT->ConstantValue + "l";
         break;
    }
    case SymbolTable::Int64:{
         TermString += CT->ConstantValue + "L";
         break;
    }
    case SymbolTable::Int8:
    case SymbolTable::Int16:
    case SymbolTable::UInt8:
    case SymbolTable::UInt16:
    case SymbolTable::UInt32:
    case SymbolTable::UInt64:{
         TermString += CT->ConstantValue + "UL";
         break;
    }
    case SymbolTable::UInt128:
    case SymbolTable::SizeT:{
         TermString += CT->ConstantValue + "sz";
         break;
    }
    default:{
        llvm::outs() << lookupSymbol(CT->Symbol) << "\n\n";
        CT->dumpPretty();
        llvm::outs() << "\n\n";
        assert(false && "Did not expect type in switch!");
      break;
    }
    }
  } else if (VarTerm *VT = dyn_cast<VarTerm>(T)) {
    TermString += VT->VarName;
  } else if (Name *N = dyn_cast<Name>(T)) {
    TermString += N->NamedValue;
  } else if (FStarType *FT = dyn_cast<FStarType>(T)) {
    TermString += FT->NamedValue;
  } else if (FStarPointerType *FPT = dyn_cast<FStarPointerType>(T)) {
    auto StrBase = generateCodeFromTerm(FPT->PointerTo);
    TermString += PulseSyntax.Reference;
    TermString += " " + StrBase;
  } else if (FStarArrType *FAT = dyn_cast<FStarArrType>(T)) {
    auto StrBase = generateCodeFromTerm(FAT->ElementType);
    TermString += PulseSyntax.Array;
    TermString += " " + StrBase;
  }
  else if (AppE *App = dyn_cast<AppE>(T)) {
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
  } 
  else if (Ensures *Ensure = dyn_cast<Ensures>(T)){
    OS << PulseSyntax.Ensures;
    OS << PulseSyntax.Space;
    OS << Ensure->Ann;
    OS << PulseSyntax.NewLine;
  }
  else if (Requires *Require = dyn_cast<Requires>(T)){
    OS << PulseSyntax.Requires;
    OS << PulseSyntax.Space; 
    OS << Require->Ann;
    OS << PulseSyntax.NewLine;
  }
  else if (Returns *Return = dyn_cast<Returns>(T)){
    OS << PulseSyntax.Returns;
    OS << PulseSyntax.Space; 
    OS << Return->Ann; 
    OS << PulseSyntax.NewLine;
  }
  else if (Lemma *UserTerm = dyn_cast<Lemma>(T)){
    
    for (auto Lemma : UserTerm->lemmas)
          OS << Lemma; 
  }
  else if (LemmaStatement *S = dyn_cast<LemmaStatement>(T)){
    OS << S->Lemma;
  }
  else {
    T->dumpPretty();
    assert(false && "Did not expect Term node in generateCodeFromTerm");
  }

  return TermString;
}

void PulseCodeGen::generateCodeFromPulseStmt(PulseStmt *T) {

  if (!T)
    return;

  if (PulseExpr *S = dyn_cast<PulseExpr>(T)) {
    OS << generateCodeFromTerm(S->E);
      OS << PulseSyntax.Semicolon;
      OS << PulseSyntax.NewLine;
  } else if (PulseAssignment *A = dyn_cast<PulseAssignment>(T)) {
    OS << generateCodeFromTerm(A->Lhs);
    OS << PulseSyntax.Space;
    OS << PulseSyntax.PulseAssignmentOpRef;
    OS << PulseSyntax.Space;
    OS << generateCodeFromTerm(A->Value);
    OS << PulseSyntax.Semicolon;
    OS << PulseSyntax.NewLine;

  } else if (PulseArrayAssignment *AS = dyn_cast<PulseArrayAssignment>(T)) {

    auto *Base = AS->Arr;
    auto *Idx = AS->Index;
    auto *LVal = AS->Value;

    OS << generateCodeFromTerm(Base);
    OS << PulseSyntax.Dot; 
    OS << PulseSyntax.OpeningParenthesis; 
    OS << generateCodeFromTerm(Idx); 
    OS << PulseSyntax.ClosingParenthesis;
    OS << PulseSyntax.Space; 
    OS << PulseSyntax.ArrAssignment;
    OS << PulseSyntax.Space;
    OS << generateCodeFromTerm(LVal);
    OS << PulseSyntax.Semicolon;
    OS << PulseSyntax.NewLine;

    //assert(false && "Did not expect pulse array statement type");
  } else if (LetBinding *Let = dyn_cast<LetBinding>(T)) {
    
    if (Let->Qualifier == MutOrRef::MUT){
      OS << PulseSyntax.LetMut;
    }
    else {
      OS << PulseSyntax.LetBind;
    }
    OS << PulseSyntax.Space;
    OS << Let->VarName;
    OS << PulseSyntax.Space;
    OS << PulseSyntax.PulseLetAssignmentOpRef;
    OS << PulseSyntax.Space;
    OS << generateCodeFromTerm(Let->LetInit);
    OS << PulseSyntax.Semicolon;
    OS << PulseSyntax.NewLine;
  } else if (PulseIf *If = dyn_cast<PulseIf>(T)) {

    OS << PulseSyntax.PulseIf;

    auto *PulseIfCond = If->Head; 
    auto *PulseThen = If->Then;
    auto *PulseElse = If->Else; 

    OS << PulseSyntax.OpeningParenthesis;
    OS << generateCodeFromTerm(PulseIfCond);
    OS << PulseSyntax.ClosingParenthesis;
    OS << PulseSyntax.NewLine; 

    OS << PulseSyntax.OpeningCurlyBrace;
    OS << PulseSyntax.NewLine; 
    generateCodeFromPulseStmt(PulseThen);
    OS << PulseSyntax.ClosingCurlyBrace; 
    OS << PulseSyntax.NewLine;

    OS << PulseSyntax.PulseElse;
    OS << PulseSyntax.NewLine; 
    OS << PulseSyntax.OpeningCurlyBrace; 
    OS << PulseSyntax.NewLine; 
    generateCodeFromPulseStmt(PulseElse);
    OS << PulseSyntax.ClosingCurlyBrace;
    OS << PulseSyntax.NewLine;
    //assert(false && "Did not expect pulse if statement type");

  } else if (PulseWhileStmt *While = dyn_cast<PulseWhileStmt>(T)) {

    auto *WCond = While->Guard; 
    auto *WBod = While->Body; 

    auto Lemmas = While->Invariant;

    OS << PulseSyntax.PulseWhile;
    OS << PulseSyntax.OpeningParenthesis; 
    generateCodeFromPulseStmt(WCond);
    OS << PulseSyntax.ClosingParenthesis; 
    OS << PulseSyntax.NewLine;
    size_t Idx = 1;
    for (auto *Lemma : Lemmas){
      OS << generateCodeFromTerm(Lemma);
      if (Idx < Lemmas.size())
        OS << "\n";
      Idx++; 
    } 
    OS << PulseSyntax.NewLine; 
    OS << PulseSyntax.OpeningCurlyBrace; 
    OS << PulseSyntax.NewLine; 
    generateCodeFromPulseStmt(WBod);
    OS << PulseSyntax.ClosingCurlyBrace; 

    //assert(false && "Did not expect pulse while statement type");
  } else if (PulseSequence *Seq = dyn_cast<PulseSequence>(T)) {
    auto *S1 = Seq->S1;
    auto *S2 = Seq->S2;
    generateCodeFromPulseStmt(S1);
    generateCodeFromPulseStmt(S2);
  } else {
    assert(false && "Did not expect pulse statement type");
  }
}