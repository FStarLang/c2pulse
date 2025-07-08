#include "PulseCodeGen.h"
#include "PulseIR.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>

using namespace clang;
using namespace llvm;

void PulseCodeGen::writeHeaders(PulseModul *M, llvm::raw_string_ostream &OS) {
  // If the module is already emitted, skip it.
  if (!M || alreadyEmittedModules.count(M->ModuleName))
    return;

  // First, recursively emit all dependencies.
  for (auto &depName : M->IncludedModules) {
    auto it = allModulesByName.find(depName);
    if (it != allModulesByName.end()) {
      writeHeaders(it->second, OS);
    } else {
      llvm::errs()
        << "Warning: Included module '" << depName
        << "' not found; check your codegen pipeline.\n";
    }
  }

  // Now emit this module’s own header.
  OS << PulseSyntax::ModuleSyntax << PulseSyntax::Space
     << M->ModuleName << PulseSyntax::NewLine << PulseSyntax::NewLine
     << PulseSyntax::LangPulse  << PulseSyntax::NewLine << PulseSyntax::NewLine;

  // Now add the Pulse Prelude if specified.
  // Prelude is a common header that includes basic definitions and utilities.
  if (M->includePulsePrelude) {
    OS << PulseSyntax::PulseInclude << PulseSyntax::NewLine
       << PulseSyntax::NewLine;
  }

  // And finally, emit the textual “include” lines for its direct deps.
  for (auto &depName : M->IncludedModules) {
    OS << depName << PulseSyntax::NewLine;
  }
  OS << PulseSyntax::NewLine;

  alreadyEmittedModules.insert(M->ModuleName);
}

std::map<std::string, CodegenPyTy> &PulseCodeGen::getEmittedModules() {
  return emittedModules;
}

std::string PulseCodeGen::getGeneratedCodeForModule(std::string ModuleName) {

  auto It = emittedModules.find(ModuleName);
  // found a stream for module
  if (It != emittedModules.end()) {
    return It->second->str();
  }
  assert(false && "Could not find a output stream for Module!\n");
}

void PulseCodeGen::generateCodeFromModule(const std::string ModuleName,
                                          PulseModul *pulseModule) {

  // check if a stream exists for the module already.
  auto It = emittedModules.find(ModuleName);
  // Found a stream for module
  if (It != emittedModules.end()) {

    auto &OutputStream = It->second;

    writeHeaders(pulseModule, *OutputStream);
    for (auto *F : pulseModule->Decls) {
      generateCodeFromPulseAST(*OutputStream, F);
    }

  } else {

    std::string *Str = new std::string();
    auto OS = std::make_unique<llvm::raw_string_ostream>(*Str);
    writeHeaders(pulseModule, *OS);
    for (auto *F : pulseModule->Decls) {
      llvm::outs() << "Trying to generate code for Function.\n";

      generateCodeFromPulseAST(*OS, F);
    }
    emittedModules.emplace(ModuleName, std::move(OS));
  }
}

void PulseCodeGen::generateCodeFromPulseAST(llvm::raw_string_ostream &OS,
                                            PulseDecl *FD) {

  OS << PulseSyntax::NewLine;

  if (PulseFnDefn *F = dyn_cast<PulseFnDefn>(FD)) {
    auto *FuncDef = F->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;
    auto *FuncBody = FuncDef->Body;

    for (auto *Att : FuncDef->Attr) {
      OS << generateCodeFromTerm(OS, Att);
      OS << PulseSyntax::NewLine;
    }

    if (FuncDef->isRecursive) {
      OS << PulseSyntax::PulseRecursiveFunctionDeclaration << " ";
    } else {
      OS << PulseSyntax::PulseFunctionDeclaration << " ";
    }
    OS << FuncName;

    // If there were no args, we still want to write () in the function
    if (Args.empty()) {
      OS << PulseSyntax::Space;
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
    }

    OS << "\n";

    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;

      if (!Arg->useFallBack) {
        OS << PulseSyntax::OpeningParenthesis << Val << PulseSyntax::Space
           << PulseSyntax::Colon << PulseSyntax::Space
           << generateCodeFromTerm(OS, Ty) << PulseSyntax::ClosingParenthesis;
        OS << "\n";
      } else {
        OS << PulseSyntax::OpeningParenthesis << Val
           << PulseSyntax::ClosingParenthesis;
        OS << "\n";
      }
    }

    // Print out the Ensures
    for (auto *A : FuncDef->Annotation) {
      generateCodeFromTerm(OS, A);
    }

    // Codegen Function Body.
    if (FuncBody != nullptr) {
      OS << PulseSyntax::OpeningCurlyBrace << PulseSyntax::NewLine;
      generateCodeFromPulseStmt(OS, FuncBody);
      OS << PulseSyntax::ClosingCurlyBrace << PulseSyntax::NewLine;
    }
    else {
      OS << PulseSyntax::OpeningCurlyBrace << PulseSyntax::NewLine;
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
      OS << PulseSyntax::Semicolon;
      OS << PulseSyntax::NewLine;
      OS << PulseSyntax::ClosingCurlyBrace << PulseSyntax::NewLine;
    }

  } else if (auto *ValD = dyn_cast<ValDecl>(FD)) {
    OS << PulseSyntax::Val;
    OS << PulseSyntax::Space;
    OS << ValD->Ident;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::Colon;
    OS << PulseSyntax::Space;
    OS << generateCodeFromTerm(OS, ValD->ValTerm);
    OS << PulseSyntax::NewLine;
  } else if (auto *TyCDecl = dyn_cast<TyConDecl>(FD)) {

    auto TCons = TyCDecl->TyCons;
    for (auto TCon : TCons) {

      if (auto *TCRecord = dyn_cast<TyConRecord>(TCon)) {
        auto Attrs = TCRecord->Attrs;
        auto Fields = TCRecord->RecordFields;
        for (auto Attr : Attrs) {
          OS << generateCodeFromTerm(OS, Attr);
          OS << PulseSyntax::NewLine;
        }

        OS << PulseSyntax::Typ;
        OS << PulseSyntax::Space;
        OS << TCRecord->Ident;
        OS << PulseSyntax::Space;
        OS << PulseSyntax::PulseLetAssignmentOpRef;
        OS << PulseSyntax::Space;
        OS << PulseSyntax::OpeningCurlyBrace;
        OS << PulseSyntax::NewLine;

        auto Flds = TCRecord->RecordFields;
        auto FldsSize = Flds.size();
        size_t I = 0;
        for (auto *Elem : Flds) {
          auto *ElemTerm = Elem->ElementTerm;
          OS << Elem->Ident;
          OS << PulseSyntax::Space;
          OS << PulseSyntax::Colon;
          OS << PulseSyntax::Space;
          OS << generateCodeFromTerm(OS, ElemTerm);
          if (I < FldsSize - 1) {
            OS << PulseSyntax::Semicolon;
          }
          I++;
          OS << PulseSyntax::NewLine;
        }

        OS << PulseSyntax::NewLine;
        OS << PulseSyntax::ClosingCurlyBrace;
        OS << PulseSyntax::NewLine;
      }
    }
  } else if (auto *PulseTopLevelLet = dyn_cast<TopLevelLet>(FD)) {
    OS << PulseSyntax::LetBind;
    OS << PulseSyntax::Space;
    OS << PulseTopLevelLet->Ident;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::PulseLetAssignmentOpRef;
    OS << PulseSyntax::Space;
    OS << PulseTopLevelLet->Lhs;
    OS << PulseSyntax::NewLine;
  } else if (auto *FallBackDeclaration = dyn_cast<GenericDecl>(FD)){
    OS << FallBackDeclaration->Ident;
  } 
  else if (auto PulseFunDecl = dyn_cast<PulseFnDecl>(FD)) {
    assert(false && "Not implemented Pulse Fun Decl\n");
  } else {
    assert(false && "Not implemented function kind");
  }
}

std::string PulseCodeGen::generateCodeFromTerm(llvm::raw_string_ostream &OS,
                                               Term *T) {

  std::string TermString = "";

  if (!T) {
    return TermString;
  }

  if (Paren *P = dyn_cast<Paren>(T)) {
    TermString += PulseSyntax::OpeningParenthesis;
    TermString += generateCodeFromTerm(OS, P->InnerExpr);
    TermString += PulseSyntax::ClosingParenthesis;
  } else if (ConstTerm *CT = dyn_cast<ConstTerm>(T)) {
    switch (CT->Symbol) {
    case SymbolTable::Int32: {
      TermString += CT->ConstantValue + "l";
      break;
    }
    case SymbolTable::Int64: {
      TermString += CT->ConstantValue + "L";
      break;
    }
    case SymbolTable::Int8:
    case SymbolTable::Int16:
    case SymbolTable::UInt8:
    case SymbolTable::UInt16:
    case SymbolTable::UInt32: {
      TermString += CT->ConstantValue + "ul";
      break;
    }
    case SymbolTable::UInt64: {
      TermString += CT->ConstantValue + "UL";
      break;
    }
    case SymbolTable::UInt128:
    case SymbolTable::SizeT: {
      TermString += CT->ConstantValue + "sz";
      break;
    }
    default: {
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
    auto StrBase = generateCodeFromTerm(OS, FPT->PointerTo);
    TermString += PulseSyntax::Reference;
    // TODO: Angelica to be revisited: this is a hack to add space after the reference
    std::string refWithSpace = std::string(PulseSyntax::Reference) + " ";
    if (StrBase.compare(0, refWithSpace.size(), refWithSpace) == 0) {
      TermString += " (" + StrBase + ")";
    } else {
      TermString += " " + StrBase;
    }
  } else if (FStarArrType *FAT = dyn_cast<FStarArrType>(T)) {
    auto StrBase = generateCodeFromTerm(OS, FAT->ElementType);
    TermString += PulseSyntax::Array;
    TermString += " " + StrBase;
  } else if (AppE *App = dyn_cast<AppE>(T)) {
    TermString += generateCodeFromTerm(OS, App->CallName);
    TermString += " ";
    for (size_t i = 0; i < App->Args.size(); ++i) {
      TermString += generateCodeFromTerm(OS, App->Args[i]);
      if (i < (App->Args).size() - 1) {
        TermString += " ";
      }
    }

    if (App->Args.empty()) {
      TermString += PulseSyntax::OpeningParenthesis;
      TermString += PulseSyntax::ClosingParenthesis;
    }

  } else if (Ensures *Ensure = dyn_cast<Ensures>(T)) {
    OS << PulseSyntax::Ensures;
    OS << PulseSyntax::Space;
    OS << Ensure->Ann;
    OS << PulseSyntax::NewLine;
  } else if (Requires *Require = dyn_cast<Requires>(T)) {
    OS << PulseSyntax::Requires;
    OS << PulseSyntax::Space;
    OS << Require->Ann;
    OS << PulseSyntax::NewLine;
  } else if (Returns *Return = dyn_cast<Returns>(T)) {
    OS << PulseSyntax::Returns;
    OS << PulseSyntax::Space;
    OS << Return->Ann;
    OS << PulseSyntax::NewLine;
  } else if (Lemma *UserTerm = dyn_cast<Lemma>(T)) {

    for (auto Lemma : UserTerm->lemmas)
      OS << Lemma;
  } else if (LemmaStatement *S = dyn_cast<LemmaStatement>(T)) {
    OS << S->Lemma;
  }
  else if (Project *P = dyn_cast<Project>(T)){
    OS << generateCodeFromTerm(OS, P->BaseTerm);
    OS << PulseSyntax::Dot;
    OS << P->MemberName;
  } 
  
  else {
    T->dumpPretty();
    llvm::outs() << "\n\n";
    assert(false && "Did not expect Term node in generateCodeFromTerm");
  }

  return TermString;
}

void PulseCodeGen::generateCodeFromPulseStmt(llvm::raw_string_ostream &OS,
                                             PulseStmt *T) {

  if (!T)
    return;

  if (PulseExpr *S = dyn_cast<PulseExpr>(T)) {
    OS << generateCodeFromTerm(OS, S->E);
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;
  } else if (PulseAssignment *A = dyn_cast<PulseAssignment>(T)) {
    OS << generateCodeFromTerm(OS, A->Lhs);
    OS << PulseSyntax::Space;
    OS << PulseSyntax::PulseAssignmentOpRef;
    OS << PulseSyntax::Space;
    OS << generateCodeFromTerm(OS, A->Value);
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;
  } else if (PulseArrayAssignment *AS = dyn_cast<PulseArrayAssignment>(T)) {

    auto *Base = AS->Arr;
    auto *Idx = AS->Index;
    auto *LVal = AS->Value;

    OS << generateCodeFromTerm(OS, Base);
    OS << PulseSyntax::Dot;
    OS << PulseSyntax::OpeningParenthesis;
    OS << generateCodeFromTerm(OS, Idx);
    OS << PulseSyntax::ClosingParenthesis;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::ArrAssignment;
    OS << PulseSyntax::Space;
    OS << generateCodeFromTerm(OS, LVal);
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;

  } else if (LetBinding *Let = dyn_cast<LetBinding>(T)) {

    if (Let->Qualifier == MutOrRef::MUT) {
      OS << PulseSyntax::LetMut;
    } else {
      OS << PulseSyntax::LetBind;
    }
    OS << PulseSyntax::Space;
    OS << Let->VarName;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::PulseLetAssignmentOpRef;
    OS << PulseSyntax::Space;
    OS << generateCodeFromTerm(OS, Let->LetInit);
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;
  } else if (PulseIf *If = dyn_cast<PulseIf>(T)) {

    OS << PulseSyntax::PulseIf;

    auto *PulseIfCond = If->Head;
    auto *PulseThen = If->Then;
    auto *PulseElse = If->Else;

    OS << PulseSyntax::OpeningParenthesis;
    OS << generateCodeFromTerm(OS, PulseIfCond);
    OS << PulseSyntax::ClosingParenthesis;
    OS << PulseSyntax::NewLine;

    // Add all the if lemmas here
    for (auto &Lemma : If->IfLemmas) {
      OS << generateCodeFromTerm(OS, Lemma);
    }

    OS << PulseSyntax::OpeningCurlyBrace;
    OS << PulseSyntax::NewLine;
    generateCodeFromPulseStmt(OS, PulseThen);
    OS << PulseSyntax::ClosingCurlyBrace;
    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::PulseElse;
    OS << PulseSyntax::NewLine;
    OS << PulseSyntax::OpeningCurlyBrace;
    OS << PulseSyntax::NewLine;
    generateCodeFromPulseStmt(OS, PulseElse);
    OS << PulseSyntax::ClosingCurlyBrace;
    // Seems like Pulse If statements need an semicolon at the end
    //TODO: Vidush ensure this is correct?
    //Maybe only needed when the if has some accompanying lemmas?
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;

  } else if (PulseWhileStmt *While = dyn_cast<PulseWhileStmt>(T)) {

    auto *WCond = While->Guard;
    auto *WBod = While->Body;

    auto Lemmas = While->Invariant;

    OS << PulseSyntax::PulseWhile;
    OS << PulseSyntax::OpeningParenthesis;
    generateCodeFromPulseStmt(OS, WCond);
    OS << PulseSyntax::ClosingParenthesis;
    OS << PulseSyntax::NewLine;
    size_t Idx = 1;
    for (auto *Lemma : Lemmas) {
      OS << generateCodeFromTerm(OS, Lemma);
      if (Idx < Lemmas.size())
        OS << "\n";
      Idx++;
    }
    OS << PulseSyntax::NewLine;
    OS << PulseSyntax::OpeningCurlyBrace;
    OS << PulseSyntax::NewLine;
    generateCodeFromPulseStmt(OS, WBod);
    OS << PulseSyntax::ClosingCurlyBrace;

  } else if (PulseSequence *Seq = dyn_cast<PulseSequence>(T)) {
    auto *S1 = Seq->S1;
    auto *S2 = Seq->S2;
    generateCodeFromPulseStmt(OS, S1);
    generateCodeFromPulseStmt(OS, S2);
  } 
  else if (auto *FallBack = dyn_cast<GenericStmt>(T)){
    OS << FallBack->body;
    OS << PulseSyntax::NewLine;
  }
  
  else {
    assert(false && "Did not expect pulse statement type");
  }
}