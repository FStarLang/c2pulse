#include "PulseCodeGen.h"
#include "Globals.h"
#include "PulseIR.h"
#include "clang/AST/ASTContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <cstddef>
#include <cstdio>
#include <fstream>

#define DEBUG_TYPE "pulse-code-gen"

using namespace clang;
using namespace llvm;

PulseCodeGen::PulseCodeGen(clang::ASTContext &Ctx) : ClangCtx(Ctx){}

void PulseCodeGen::writeHeaders(PulseModul *M, llvm::raw_string_ostream &OS, 
  unsigned *RowIdx) {
  // If the module is already emitted, skip it.
  if (!M || alreadyEmittedModules.count(M->ModuleName))
    return;

  // First, recursively emit all dependencies.
  for (auto &depName : M->IncludedModules) {
    auto it = allModulesByName.find(depName);
    if (it != allModulesByName.end()) {
      writeHeaders(it->second, OS, RowIdx);
    } else {
      llvm::errs()
        << "Warning: Included module '" << depName
        << "' not found; check your codegen pipeline.\n";
    }
  }

  // Now emit this module’s own header.
  OS << PulseSyntax::ModuleSyntax; 
  OS << PulseSyntax::Space;
  OS << M->ModuleName;
  OS << PulseSyntax::NewLine;
  *RowIdx += 1;
  OS << PulseSyntax::NewLine;
  *RowIdx += 1;
  OS << PulseSyntax::LangPulse;
  OS << PulseSyntax::NewLine;
  *RowIdx += 1;
  OS << PulseSyntax::NewLine;
  *RowIdx += 1;

  // Now add the Pulse Prelude if specified.
  // Prelude is a common header that includes basic definitions and utilities.
  *RowIdx += 1;
  if (M->includePulsePrelude) {
    OS << PulseSyntax::PulseInclude; 
    OS << PulseSyntax::NewLine;
    *RowIdx += 1;
    OS << PulseSyntax::NewLine;
    *RowIdx += 1;
  }

  // And finally, emit the textual “include” lines for its direct deps.
  for (auto &depName : M->IncludedModules) {
    *RowIdx += 1;
    OS << depName; 
    OS << PulseSyntax::NewLine;
    *RowIdx += 1;
  }
  OS << PulseSyntax::NewLine;
  *RowIdx += 1;

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
  emitError("(getGeneratedCodeForModule): Could not find a output stream for "
            "Module: " +
            ModuleName + "\n");
}

void PulseCodeGen::generateCodeFromModule(const std::string ModuleName,
                                          PulseModul *pulseModule) {
  
  unsigned RowCounter = 1;
  unsigned ColCounter = 1;                                          
  // check if a stream exists for the module already.
  auto It = emittedModules.find(ModuleName);
  // Found a stream for module
  if (It != emittedModules.end()) {

    auto &OutputStream = It->second;

    writeHeaders(pulseModule, *OutputStream, &RowCounter);
    for (auto *F : pulseModule->Decls) {
      generateCodeFromPulseAST(*OutputStream, F, &RowCounter, &ColCounter);
    }

  } else {

    std::string *Str = new std::string();
    auto OS = std::make_unique<llvm::raw_string_ostream>(*Str);
    writeHeaders(pulseModule, *OS, &RowCounter);
    for (auto *F : pulseModule->Decls) {
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
      if (PulseFnDefn *PF = dyn_cast<PulseFnDefn>(F)) {
        llvm::outs() << "Trying to generate code for Function: " << PF->Defn->Name << "\n";
      }
    });


      generateCodeFromPulseAST(*OS, F, &RowCounter, &ColCounter);
    }
    emittedModules.emplace(ModuleName, std::move(OS));
  }
}

void PulseCodeGen::generateCodeFromPulseAST(llvm::raw_string_ostream &OS,
                                            PulseDecl *FD, 
                                            unsigned *RowCounter, 
                                            unsigned *ColCounter) {

  OS << PulseSyntax::NewLine;
  *RowCounter += 1;
  *ColCounter = 1;

  if (PulseFnDefn *F = dyn_cast<PulseFnDefn>(FD)) {

    PulseSourceLocation Start = PulseSourceLocation(*RowCounter, *ColCounter);
    auto *FuncDef = F->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;
    auto *FuncBody = FuncDef->Body;

    for (auto *Att : FuncDef->Attr) {
      OS << generateCodeFromTerm(OS, Att, RowCounter, ColCounter);
      OS << PulseSyntax::NewLine;
      *RowCounter += 1;
      *ColCounter = 1;
    }

    if (FuncDef->isRecursive) {
      OS << PulseSyntax::PulseRecursiveFunctionDeclaration;
      *ColCounter += strlen(PulseSyntax::PulseRecursiveFunctionDeclaration);
      OS << PulseSyntax::Space;
      *ColCounter += strlen(PulseSyntax::PulseRecursiveFunctionDeclaration);
    } else {
      OS << PulseSyntax::PulseFunctionDeclaration;
      *ColCounter += strlen(PulseSyntax::PulseFunctionDeclaration);
      OS << PulseSyntax::Space;
      *ColCounter += strlen(PulseSyntax::Space);
    }
    OS << FuncName;
    *ColCounter += FuncName.length();

    // If there were no args, we still want to write () in the function
    if (Args.empty()) {
      OS << PulseSyntax::Space;
      *ColCounter += strlen(PulseSyntax::Space);
      OS << PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      OS << PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
    }

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;

      if (!Arg->useFallBack) {
        OS << PulseSyntax::OpeningParenthesis; 
        *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
        OS << Val; 
        *ColCounter += Val.length();
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << PulseSyntax::Colon;
        *ColCounter += strlen(PulseSyntax::Colon);
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << generateCodeFromTerm(OS, Ty, RowCounter, ColCounter);
        OS << PulseSyntax::ClosingParenthesis;
        *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      } else {
        OS << PulseSyntax::OpeningParenthesis;
        *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
        OS << Val;
        *ColCounter += Val.length();
        OS << PulseSyntax::ClosingParenthesis;
        *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      }
    }

    // Print out the Ensures
    for (auto *A : FuncDef->Annotation) {
      generateCodeFromTerm(OS, A, RowCounter, ColCounter);
    }
    
    PulseSourceLocation *End;
    // Codegen Function Body.
    if (FuncBody != nullptr) {
      OS << PulseSyntax::OpeningCurlyBrace; 
      *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);
      OS << PulseSyntax::NewLine;
      *RowCounter += 1; 
      *ColCounter = 1;
      generateCodeFromPulseStmt(OS, FuncBody, RowCounter, ColCounter);
      OS << PulseSyntax::ClosingCurlyBrace;
      *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);

      End = new PulseSourceLocation(*RowCounter, *ColCounter);

      OS << PulseSyntax::NewLine;
      *RowCounter += 1; 
      *ColCounter = 1;
    }
    else {
      OS << PulseSyntax::OpeningCurlyBrace;
      *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);
      OS << PulseSyntax::NewLine;
      *RowCounter += 1; 
      *ColCounter = 1;
      OS << PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      OS << PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
      OS << PulseSyntax::Semicolon;
      *ColCounter += strlen(PulseSyntax::Semicolon);
      OS << PulseSyntax::NewLine;
      *RowCounter += 1; 
      *ColCounter = 1;
      OS << PulseSyntax::ClosingCurlyBrace;
      *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);

      End = new PulseSourceLocation(*RowCounter, *ColCounter);

      OS << PulseSyntax::NewLine;
      *RowCounter += 1;
      *ColCounter = 1;
    }
    
    PulseSourceRange FuncRange = PulseSourceRange(Start, *End);
    PulseLocsToCLocs.push_back(std::make_pair(FuncRange, F->getCSourceInfo()));

  } else if (auto *ValD = dyn_cast<ValDecl>(FD)) {

    PulseSourceLocation Start = PulseSourceLocation(*RowCounter, *ColCounter);

    OS << PulseSyntax::Val;
    *ColCounter += strlen(PulseSyntax::Val);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space); 
    OS << ValD->Ident;
    *ColCounter += ValD->Ident.length();
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << PulseSyntax::Colon;
    *ColCounter += strlen(PulseSyntax::Colon);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << generateCodeFromTerm(OS, ValD->ValTerm, RowCounter, ColCounter);
    PulseSourceLocation End = PulseSourceLocation(*RowCounter, *ColCounter);
    PulseSourceRange Range = PulseSourceRange(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, ValD->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

  } else if (auto *TyCDecl = dyn_cast<TyConDecl>(FD)) {


    PulseSourceLocation Start = PulseSourceLocation(*RowCounter, *ColCounter);

    auto TCons = TyCDecl->TyCons;
    for (auto *TCon : TCons) {

      if (auto *TCRecord = dyn_cast<TyConRecord>(TCon)) {
        auto Attrs = TCRecord->Attrs;
        auto Fields = TCRecord->RecordFields;
        for (auto *Attr : Attrs) {
          OS << generateCodeFromTerm(OS, Attr, RowCounter, ColCounter);
          OS << PulseSyntax::NewLine;
          *RowCounter += 1; 
          *ColCounter = 1;
        }

        OS << PulseSyntax::Typ;
        *ColCounter += strlen(PulseSyntax::Typ);
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << TCRecord->Ident;
        *ColCounter += TCRecord->Ident.length();
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << PulseSyntax::PulseLetAssignmentOpRef;
        *ColCounter += strlen(PulseSyntax::PulseLetAssignmentOpRef);
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << PulseSyntax::OpeningCurlyBrace;
        *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;

        auto Flds = TCRecord->RecordFields;
        auto FldsSize = Flds.size();
        size_t I = 0;
        for (auto *Elem : Flds) {
          auto *ElemTerm = Elem->ElementTerm;
          OS << Elem->Ident;
          *ColCounter += Elem->Ident.length();
          OS << PulseSyntax::Space;
          *ColCounter += strlen(PulseSyntax::Space);
          OS << PulseSyntax::Colon;
          *ColCounter += strlen(PulseSyntax::Colon);
          OS << PulseSyntax::Space;
          *ColCounter += strlen(PulseSyntax::Space);
          OS << generateCodeFromTerm(OS, ElemTerm, RowCounter, ColCounter);
          if (I < FldsSize - 1) {
            OS << PulseSyntax::Semicolon;
            *ColCounter += strlen(PulseSyntax::Semicolon);
          }
          I++;
          OS << PulseSyntax::NewLine;
          *RowCounter += 1;
          *ColCounter = 1;
        }

        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
        OS << PulseSyntax::ClosingCurlyBrace;
        *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);
        PulseSourceLocation End = PulseSourceLocation(*RowCounter, *ColCounter);
        PulseSourceRange Range = PulseSourceRange(Start, End);
        PulseLocsToCLocs.push_back(std::make_pair(Range, TyCDecl->getCSourceInfo()));

        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      }
    }
  } else if (auto *PulseTopLevelLet = dyn_cast<TopLevelLet>(FD)) {
    
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    
    OS << PulseSyntax::LetBind;
    *ColCounter += strlen(PulseSyntax::LetBind);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << PulseTopLevelLet->Ident;
    *ColCounter += PulseTopLevelLet->Ident.length();
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << PulseSyntax::PulseLetAssignmentOpRef;
    *ColCounter += strlen(PulseSyntax::PulseLetAssignmentOpRef);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << PulseTopLevelLet->Lhs;
    *ColCounter += PulseTopLevelLet->Lhs.length();

    PulseSourceLocation End = PulseSourceLocation(*RowCounter, *ColCounter);
    PulseSourceRange Range = PulseSourceRange(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, PulseTopLevelLet->getCSourceInfo())); 

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter += 0;
  } else if (auto *FallBackDeclaration = dyn_cast<GenericDecl>(FD)){
    PulseSourceLocation Location(*RowCounter, *ColCounter);    
    OS << FallBackDeclaration->Ident;
    *ColCounter += FallBackDeclaration->Ident.length();
    PulseSourceRange Range(Location);
    PulseLocsToCLocs.push_back(std::make_pair(Range, FallBackDeclaration->getCSourceInfo()));
  } 
  else if (auto *PulseFunDecl = dyn_cast<PulseFnDecl>(FD)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);
     
    auto *FuncDef = PulseFunDecl->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;
    
    //TODO: maybe for recursive functions these need to be extended 
    // with a isRec field.
    OS << PulseSyntax::PulseFunctionDeclaration;
    *ColCounter += strlen(PulseSyntax::PulseFunctionDeclaration); 
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << FuncName;
    *ColCounter += FuncName.length();

    // If there were no args, we still want to write () in the function
    if (Args.empty()) {
      OS << PulseSyntax::Space;
      *ColCounter += strlen(PulseSyntax::Space);
      OS << PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      OS << PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
    }

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;

      if (!Arg->useFallBack) {
        OS << PulseSyntax::OpeningParenthesis; 
        *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
        OS << Val;
        *ColCounter += Val.length();
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << PulseSyntax::Colon; 
        *ColCounter += strlen(PulseSyntax::Colon);
        OS << PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
        OS << generateCodeFromTerm(OS, Ty, RowCounter, ColCounter);
        OS << PulseSyntax::ClosingParenthesis;
        *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      } else {
        OS << PulseSyntax::OpeningParenthesis;
        *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
        OS << Val;
        *ColCounter += Val.length();
        OS << PulseSyntax::ClosingParenthesis;
        *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      }
    }

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, PulseFunDecl->getCSourceInfo()));

  } else {
    emitError("(generateCodeFromPulseAST): Encountered an unknown pulse "
              "declaration type!\n");
  }
}

std::string PulseCodeGen::generateCodeFromTerm(llvm::raw_string_ostream &OS,
                                               Term *T, 
                                               unsigned *RowCounter, 
                                               unsigned *ColCounter) {

  std::string TermString = "";

  if (!T) {
    return TermString;
  }

  if (Paren *P = dyn_cast<Paren>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    TermString += PulseSyntax::OpeningParenthesis;
    *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
    TermString += generateCodeFromTerm(OS, P->InnerExpr, RowCounter, ColCounter);
    TermString += PulseSyntax::ClosingParenthesis;
    *ColCounter += strlen(PulseSyntax::ClosingParenthesis);

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, P->getCSourceInfo()));

  } else if (ConstTerm *CT = dyn_cast<ConstTerm>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);
    switch (CT->Symbol) {
    case SymbolTable::Int32: {
      TermString += CT->ConstantValue + "l";
      *ColCounter += CT->ConstantValue.length() + strlen("l");
      break;
    }
    case SymbolTable::Int64: {
      TermString += CT->ConstantValue + "L";
      *ColCounter += CT->ConstantValue.length() + strlen("L");
      break;
    }
    case SymbolTable::Int8: {
      TermString += CT->ConstantValue + "y";
      *ColCounter += CT->ConstantValue.length() + strlen("y");
      break;
    }
    case SymbolTable::Int16: {
      emitError("ConstTerm: did not implement case for Int16!\n");
    }
    case SymbolTable::UInt8: {
      emitError("ConstTerm: did not implement case for UInt8!\n");
    }
    case SymbolTable::UInt16: {
      emitError("ConstTerm: did not implement case for UInt16!\n");
    }
    case SymbolTable::UInt32: {
      TermString += CT->ConstantValue + "ul";
      *ColCounter += CT->ConstantValue.length() + strlen("ul");
      break;
    }
    case SymbolTable::UInt64: {
      TermString += CT->ConstantValue + "UL";
      *ColCounter += CT->ConstantValue.length() + strlen("UL");
      break;
    }
    case SymbolTable::UInt128: {
      emitError("ConstTerm: did not implement case for UInt128!\n");
    }
    case SymbolTable::SizeT: {
      TermString += CT->ConstantValue + "sz";
      *ColCounter += CT->ConstantValue.length() + strlen("sz");
      break;
    }
    case SymbolTable::Bool: {
      if (CT->ConstantValue == "0") {
        TermString += "false";
        *ColCounter += strlen("false");
      } else {
        TermString += "true";
        *ColCounter += strlen("true");
      }
      break;
    }
    default: {
      //CT->dumpPretty();
      emitError("Did not expect pulse type!");
      break;
    }
    }

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, CT->getCSourceInfo()));

  } else if (VarTerm *VT = dyn_cast<VarTerm>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    TermString += VT->VarName;
    *ColCounter += VT->VarName.length();
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, VT->getCSourceInfo()));
  } else if (Name *N = dyn_cast<Name>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    TermString += N->NamedValue;
    *ColCounter +=  N->NamedValue.length();
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (FStarType *FT = dyn_cast<FStarType>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    TermString += FT->NamedValue;
    *ColCounter += FT->NamedValue.length();
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (FStarPointerType *FPT = dyn_cast<FStarPointerType>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    auto StrBase = generateCodeFromTerm(OS, FPT->PointerTo, RowCounter, ColCounter);
    TermString += PulseSyntax::Reference;
    *ColCounter += strlen(PulseSyntax::Reference);
    // TODO: Angelica to be revisited: this is a hack to add space after the reference
    std::string refWithSpace = std::string(PulseSyntax::Reference);
    *ColCounter += strlen(PulseSyntax::Reference); 
    refWithSpace += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);

    if (StrBase.compare(0, refWithSpace.size(), refWithSpace) == 0) {
      TermString += PulseSyntax::Space;
      *ColCounter += strlen(PulseSyntax::Space); 
      TermString += PulseSyntax::OpeningParenthesis;
       *ColCounter += strlen(PulseSyntax::OpeningParenthesis);  
      TermString += StrBase; 
       *ColCounter += StrBase.length(); 
      TermString += PulseSyntax::ClosingParenthesis;
       *ColCounter += strlen(PulseSyntax::ClosingParenthesis); 
    } else {
      TermString += PulseSyntax::Space;
       *ColCounter += strlen(PulseSyntax::Space);  
      TermString += StrBase;
       *ColCounter += StrBase.length(); 
    }

    PulseSourceLocation End(*RowCounter, *ColCounter); 
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (FStarArrType *FAT = dyn_cast<FStarArrType>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    auto StrBase = generateCodeFromTerm(OS, FAT->ElementType, RowCounter, ColCounter);
    TermString += PulseSyntax::Array;
    *ColCounter += strlen(PulseSyntax::Array);
    TermString += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    TermString += StrBase;
    *ColCounter += StrBase.length();
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (AppE *App = dyn_cast<AppE>(T)) {
    
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    TermString += generateCodeFromTerm(OS, App->CallName, RowCounter, ColCounter);
    TermString += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    for (size_t i = 0; i < App->Args.size(); ++i) {
      TermString += generateCodeFromTerm(OS, App->Args[i], RowCounter, ColCounter);
      if (i < (App->Args).size() - 1) {
        TermString += PulseSyntax::Space;
        *ColCounter += strlen(PulseSyntax::Space);
      }
    }

    if (App->Args.empty()) {
      TermString += PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      TermString += PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
    }

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (Ensures *Ensure = dyn_cast<Ensures>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << PulseSyntax::Ensures;
    *ColCounter += strlen(PulseSyntax::Ensures);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << Ensure->Ann;
    *ColCounter += Ensure->Ann.length();
    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

  } else if (Requires *Require = dyn_cast<Requires>(T)) {
    
    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << PulseSyntax::Requires;
    *ColCounter += strlen(PulseSyntax::Requires);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << Require->Ann;
    *ColCounter += Require->Ann.length();
    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

  } else if (Preserves *Preserve = dyn_cast<Preserves>(T)) {
    
    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << PulseSyntax::Preserves;
    *ColCounter += strlen(PulseSyntax::Preserves);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << Preserve->Ann;
    *ColCounter += Preserve->Ann.length();
    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

  }
  else if (Returns *Return = dyn_cast<Returns>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << PulseSyntax::Returns;
    *ColCounter += strlen(PulseSyntax::Returns);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << Return->Ann;
    *ColCounter += Return->Ann.length();
    //End

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;
  } else if (Lemma *UserTerm = dyn_cast<Lemma>(T)) {
    
    PulseSourceLocation Start(*RowCounter, *ColCounter);
    for (auto Lemma : UserTerm->lemmas){
      OS << Lemma;
      *ColCounter += Lemma.length();
    }
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);

    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (LemmaStatement *S = dyn_cast<LemmaStatement>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << S->Lemma;
    *ColCounter += S->Lemma.length();
    
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (Project *P = dyn_cast<Project>(T)) {
    PulseSourceLocation Start(*RowCounter, *ColCounter);

    TermString += generateCodeFromTerm(OS, P->BaseTerm, RowCounter, ColCounter);
    TermString += PulseSyntax::Dot;
    *ColCounter += strlen(PulseSyntax::Dot);
    TermString += P->MemberName;
    *ColCounter += P->MemberName.length();

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  }
  else if (IfExpr *If = dyn_cast<IfExpr>(T)){

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    TermString += PulseSyntax::PulseIf;
    *ColCounter += strlen(PulseSyntax::PulseIf);
    TermString += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    TermString += generateCodeFromTerm(OS, If->Cond, RowCounter, ColCounter);
    
    TermString += PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    TermString += PulseSyntax::IfExprThen; 
    *ColCounter += strlen(PulseSyntax::IfExprThen);
    TermString += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    TermString += generateCodeFromTerm(OS, If->TrueExpr, RowCounter, ColCounter);

    TermString += PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    TermString += PulseSyntax::IfExprElse;
    *ColCounter += strlen(PulseSyntax::IfExprElse);
    TermString += PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    TermString += generateCodeFromTerm(OS, If->FalseExpr, RowCounter, ColCounter);

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  }
  else {
    T->dumpPretty();
    emitError("Did not expect Pulse AST Node!");
  }

  return TermString;
}

void PulseCodeGen::generateCodeFromPulseStmt(llvm::raw_string_ostream &OS,
                                             PulseStmt *T, 
                                            unsigned *RowCounter,
                                            unsigned *ColCounter) {

  if (!T)
    return;

  if (PulseExpr *S = dyn_cast<PulseExpr>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << generateCodeFromTerm(OS, S->E, RowCounter, ColCounter);

    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);
    //End

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;
  } else if (PulseAssignment *A = dyn_cast<PulseAssignment>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << generateCodeFromTerm(OS, A->Lhs, RowCounter, ColCounter);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << PulseSyntax::PulseAssignmentOpRef;
    *ColCounter += strlen(PulseSyntax::PulseAssignmentOpRef);
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);
    OS << generateCodeFromTerm(OS, A->Value, RowCounter, ColCounter);

    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);
    //End 
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;
  } else if (PulseArrayAssignment *AS = dyn_cast<PulseArrayAssignment>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    auto *Base = AS->Arr;
    auto *Idx = AS->Index;
    auto *LVal = AS->Value;

    OS << generateCodeFromTerm(OS, Base, RowCounter, ColCounter);
    
    OS << PulseSyntax::Dot;
    *ColCounter += strlen(PulseSyntax::Dot);
    
    OS << PulseSyntax::OpeningParenthesis;
    *ColCounter += strlen(PulseSyntax::OpeningParenthesis);

    OS << generateCodeFromTerm(OS, Idx, RowCounter, ColCounter);
    
    OS << PulseSyntax::ClosingParenthesis;
    *ColCounter += strlen(PulseSyntax::ClosingParenthesis);

    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);

    OS << PulseSyntax::ArrAssignment;
    *ColCounter += strlen(PulseSyntax::ArrAssignment);

    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);

    OS << generateCodeFromTerm(OS, LVal, RowCounter, ColCounter);

    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);
    //End 
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

  } else if (LetBinding *Let = dyn_cast<LetBinding>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    if (Let->Qualifier == MutOrRef::MUT) {
      OS << PulseSyntax::LetMut;
      *ColCounter = *ColCounter + strlen(PulseSyntax::LetMut);
    } else {
      OS << PulseSyntax::LetBind;
      *ColCounter = *ColCounter + strlen(PulseSyntax::LetBind);
    }

    OS << PulseSyntax::Space;
    *ColCounter = *ColCounter + strlen(PulseSyntax::Space);
    OS << Let->VarName;
    *ColCounter = *ColCounter + (Let->VarName).length();

    OS << PulseSyntax::Space;
    *ColCounter = *ColCounter + strlen(PulseSyntax::Space);

    //Add type annotation for variable in let binding
    OS << PulseSyntax::Colon; 
    *ColCounter += strlen(PulseSyntax::Colon);
    OS << PulseSyntax::Space; 
    *ColCounter += strlen(PulseSyntax::Space);
    OS << Let->VarTy; 
    *ColCounter += Let->VarTy.length();
    OS << PulseSyntax::Space;
    *ColCounter += strlen(PulseSyntax::Space);

    OS << PulseSyntax::PulseLetAssignmentOpRef;
    *ColCounter = *ColCounter + strlen(PulseSyntax::PulseLetAssignmentOpRef);
    OS << PulseSyntax::Space;
    *ColCounter = *ColCounter + strlen(PulseSyntax::Space);
    OS << generateCodeFromTerm(OS, Let->LetInit, RowCounter, ColCounter);
    
    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);
    //End

    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

  } else if (PulseIf *If = dyn_cast<PulseIf>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << PulseSyntax::PulseIf;
    *ColCounter += strlen(PulseSyntax::PulseIf);

    auto *PulseIfCond = If->Head;
    auto *PulseThen = If->Then;
    auto *PulseElse = If->Else;

    OS << PulseSyntax::OpeningParenthesis;
    *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
    OS << generateCodeFromTerm(OS, PulseIfCond, RowCounter, ColCounter);
    OS << PulseSyntax::ClosingParenthesis;
    *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
    
    //Reset Col Counter at every new line
    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

    // Add all the if lemmas here
    for (auto &Lemma : If->IfLemmas) {
      OS << generateCodeFromTerm(OS, Lemma, RowCounter, ColCounter);
    }

    OS << PulseSyntax::OpeningCurlyBrace;
    *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

    if (PulseThen == nullptr){
      OS << PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      OS << PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);

      OS << PulseSyntax::NewLine;
      *RowCounter += 1;
      *ColCounter = 1;
    }
    generateCodeFromPulseStmt(OS, PulseThen, RowCounter, ColCounter);
    OS << PulseSyntax::ClosingCurlyBrace;
    *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

    OS << PulseSyntax::PulseElse;
    *ColCounter += strlen(PulseSyntax::PulseElse);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

    OS << PulseSyntax::OpeningCurlyBrace;
    *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

    if (PulseElse == nullptr){
      OS << PulseSyntax::OpeningParenthesis;
      *ColCounter += strlen(PulseSyntax::OpeningParenthesis);
      OS << PulseSyntax::ClosingParenthesis;
      *ColCounter += strlen(PulseSyntax::ClosingParenthesis);
      OS << PulseSyntax::NewLine;
      *RowCounter += 1;
      *ColCounter = 1;
    }
    generateCodeFromPulseStmt(OS, PulseElse, RowCounter, ColCounter);
    OS << PulseSyntax::ClosingCurlyBrace;
    *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);
    // Seems like Pulse If statements need an semicolon at the end
    //TODO: Vidush ensure this is correct?
    //Maybe only needed when the if has some accompanying lemmas?
    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);

    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
    *RowCounter += 1;
    *ColCounter = 1;

  } else if (PulseWhileStmt *While = dyn_cast<PulseWhileStmt>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    auto *WCond = While->Guard;
    auto *WBod = While->Body;

    auto Lemmas = While->Invariant;

    OS << PulseSyntax::PulseWhile;
    *ColCounter += strlen(PulseSyntax::PulseWhile);
    OS << PulseSyntax::OpeningParenthesis;
    *ColCounter += strlen(PulseSyntax::OpeningParenthesis);

    generateCodeFromPulseStmt(OS, WCond, RowCounter, ColCounter);
    OS << PulseSyntax::ClosingParenthesis;
    *ColCounter += strlen(PulseSyntax::ClosingParenthesis);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    size_t Idx = 1;
    for (auto *Lemma : Lemmas) {
      OS << generateCodeFromTerm(OS, Lemma, RowCounter, ColCounter);
      if (Idx < Lemmas.size()){
        OS << PulseSyntax::NewLine;
        *RowCounter += 1; 
        *ColCounter = 1;
      }
      Idx++;
    }

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    OS << PulseSyntax::OpeningCurlyBrace;
    *ColCounter += strlen(PulseSyntax::OpeningCurlyBrace);

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    generateCodeFromPulseStmt(OS, WBod, RowCounter, ColCounter);
    OS << PulseSyntax::ClosingCurlyBrace;
    *ColCounter += strlen(PulseSyntax::ClosingCurlyBrace);
    OS << PulseSyntax::Semicolon;
    *ColCounter += strlen(PulseSyntax::Semicolon);
    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


  } else if (PulseSequence *Seq = dyn_cast<PulseSequence>(T)) {

    PulseSourceLocation Start(*RowCounter, *ColCounter);

    auto *S1 = Seq->S1;
    auto *S2 = Seq->S2;
    generateCodeFromPulseStmt(OS, S1, RowCounter, ColCounter);
    generateCodeFromPulseStmt(OS, S2, RowCounter, ColCounter);

    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } 
  else if (auto *FallBack = dyn_cast<GenericStmt>(T)){
    PulseSourceLocation Start(*RowCounter, *ColCounter);

    OS << FallBack->body;
    *ColCounter += FallBack->body.length();

    OS << PulseSyntax::NewLine;
    *RowCounter += 1; 
    *ColCounter = 1;

    //End
    PulseSourceLocation End(*RowCounter, *ColCounter);
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  }
  
  else {
    emitError("Did not Expect Pulse AST Node!");
  }
}

// Define how to convert PulseSourceRange to JSON
void PulseCodeGen::PulseSourceRangeToJson(json &J,
                                          const PulseSourceRange &Range) {
  J = json{
      { "start", 
        {
        { "line", Range.Begin.Line },
        { "column", Range.Begin.Column }
            }
          },
      { "end", 
        {
        { "line", Range.End.Line },
        { "column", Range.End.Column }
            }
          }
  };
}

void PulseCodeGen::SourceInfoToJson(json &J, const SourceInfo &Info) {

  std::string Buffer;
  llvm::raw_string_ostream RangInfoStream(Buffer);

  Info.range.print(RangInfoStream, ClangCtx.getSourceManager());
  J = json{
    { "fileName", Info.FileName },
    { "start", {
        { "line", Info.StartLine },
        { "column", Info.StartColumn }
    }},
    { "end", {
        { "line", Info.EndLine },
        { "column", Info.EndColumn }
    }},
    { "isVerbatim", Info.IsVerbatim }
};

}

void PulseCodeGen::JsonifySourceRangeMap(std::string JsonOutputFile){

    nlohmann::ordered_json JSonArray = nlohmann::ordered_json::array();
    for (const auto &[Key, Value] : PulseLocsToCLocs) {

      // Skip generating entry for this.
      if (!Value.isValid) {
        continue;
      }

      nlohmann::ordered_json Entry;
      json JsonKey;
      json JsonVal;

      PulseSourceRangeToJson(JsonKey, Key);
      Entry["pulseRange"] = JsonKey;

      SourceInfoToJson(JsonVal, Value);
      Entry["cRange"] = JsonVal;

      JSonArray.push_back(Entry);
    }

    std::ofstream JsonStream(JsonOutputFile);
    if (JsonStream.is_open()) {
        JsonStream << JSonArray.dump(2); // Pretty-print with indent = 2
        JsonStream.close();
    } else {
        llvm::errs() << "Unable to open file: " << JsonOutputFile << "\n";
    }

}


void PulseCodeGen::printSourceLocations(){

  for (auto &LocationPair : PulseLocsToCLocs){

    llvm::outs() << "Print Pulse Info and Corresponding C info: " << "\n";
    llvm::outs() << "Print Pulse Info: " << "\n";
    llvm::outs() << "==================================================\n";
    LocationPair.first.dumpPretty();
    llvm::outs() << "==================================================\n\n";
    llvm::outs() << "Print C Info:\n";
    llvm::outs() << "\n";
    LocationPair.second.dumpPretty(ClangCtx);
    llvm::outs() << "==================================================\n\n";
  }

}