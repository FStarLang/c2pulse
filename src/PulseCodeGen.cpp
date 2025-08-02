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

void PulseCodeGen::writeHeaders(PulseModul *M, osstream_with_pos &OS) {
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
  OS << PulseSyntax::ModuleSyntax; 
  OS << PulseSyntax::Space;
  OS << M->ModuleName;
  OS << PulseSyntax::NewLine;
  OS << PulseSyntax::NewLine;
  OS << PulseSyntax::LangPulse;
  OS << PulseSyntax::NewLine;
  OS << PulseSyntax::NewLine;

  // Now add the Pulse Prelude if specified.
  // Prelude is a common header that includes basic definitions and utilities.
  if (M->includePulsePrelude) {
    OS << PulseSyntax::PulseInclude; 
    OS << PulseSyntax::NewLine;
    OS << PulseSyntax::NewLine;
  }

  // And finally, emit the textual “include” lines for its direct deps.
  for (auto &depName : M->IncludedModules) {
    OS << depName; 
    OS << PulseSyntax::NewLine;
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
  emitError("(getGeneratedCodeForModule): Could not find a output stream for "
            "Module: " +
            ModuleName + "\n");
}

void PulseCodeGen::generateCodeFromModule(const std::string ModuleName,
                                          PulseModul *pulseModule) {
  
  //unsigned RowCounter = 1;
  //unsigned ColCounter = 1;                                          
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

    //std::string *Str = new std::string();
    auto OS = std::make_unique<osstream_with_pos>(); //*Str
    writeHeaders(pulseModule, *OS);
    for (auto *F : pulseModule->Decls) {
    DEBUG_WITH_TYPE(DEBUG_TYPE, {
      if (PulseFnDefn *PF = dyn_cast<PulseFnDefn>(F)) {
        llvm::outs() << "Trying to generate code for Function: " << PF->Defn->Name << "\n";
      }
    });


      generateCodeFromPulseAST(*OS, F);
    }
    emittedModules.emplace(ModuleName, std::move(OS));
  }
}

void PulseCodeGen::generateCodeFromPulseAST(osstream_with_pos &OS,
                                            PulseDecl *FD) {

  OS << PulseSyntax::NewLine;

  if (PulseFnDefn *F = dyn_cast<PulseFnDefn>(FD)) {

    PulseSourceLocation Start = PulseSourceLocation(OS.line(), OS.col());
    auto *FuncDef = F->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;
    auto *FuncBody = FuncDef->Body;

    for (auto *Att : FuncDef->Attr) {
      generateCodeFromTerm(OS, Att);
      OS << PulseSyntax::NewLine;
    }

    if (FuncDef->isRecursive) {
      OS << PulseSyntax::PulseRecursiveFunctionDeclaration;
      OS << PulseSyntax::Space;
    } else {
      OS << PulseSyntax::PulseFunctionDeclaration;
      OS << PulseSyntax::Space;
    }
    OS << FuncName;

    // If there were no args, we still want to write () in the function
    if (Args.empty()) {
      OS << PulseSyntax::Space;
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
    }

    OS << PulseSyntax::NewLine;

    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;

      if (!Arg->useFallBack) {
        OS << PulseSyntax::OpeningParenthesis; 
        OS << Val; 
        OS << PulseSyntax::Space;
        OS << PulseSyntax::Colon;
        OS << PulseSyntax::Space;
        generateCodeFromTerm(OS, Ty);
        OS << PulseSyntax::ClosingParenthesis;
        OS << PulseSyntax::NewLine;
      } else {
        OS << PulseSyntax::OpeningParenthesis;
        OS << Val;
        OS << PulseSyntax::ClosingParenthesis;
        OS << PulseSyntax::NewLine;
      }
    }

    // Print out the Ensures
    for (auto *A : FuncDef->Annotation) {
      generateCodeFromTerm(OS, A);
    }
    
    PulseSourceLocation *End;
    // Codegen Function Body.
    if (FuncBody != nullptr) {
      OS << PulseSyntax::OpeningCurlyBrace; 
      OS << PulseSyntax::NewLine;
      generateCodeFromPulseStmt(OS, FuncBody);
      OS << PulseSyntax::ClosingCurlyBrace;

      End = new PulseSourceLocation(OS.line(), OS.col());

      OS << PulseSyntax::NewLine;
    }
    else {
      OS << PulseSyntax::OpeningCurlyBrace;
      OS << PulseSyntax::NewLine;
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
      OS << PulseSyntax::Semicolon;
      OS << PulseSyntax::NewLine;
      OS << PulseSyntax::ClosingCurlyBrace;

      End = new PulseSourceLocation(OS.line(), OS.col());

      OS << PulseSyntax::NewLine;
    }
    
    PulseSourceRange FuncRange = PulseSourceRange(Start, *End);
    PulseLocsToCLocs.push_back(std::make_pair(FuncRange, F->getCSourceInfo()));

  } else if (auto *ValD = dyn_cast<ValDecl>(FD)) {

    PulseSourceLocation Start = PulseSourceLocation(OS.line(), OS.col());

    OS << PulseSyntax::Val;
    OS << PulseSyntax::Space; 
    OS << ValD->Ident;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::Colon;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, ValD->ValTerm);
    PulseSourceLocation End = PulseSourceLocation(OS.line(), OS.col());
    PulseSourceRange Range = PulseSourceRange(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, ValD->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (auto *TyCDecl = dyn_cast<TyConDecl>(FD)) {


    PulseSourceLocation Start = PulseSourceLocation(OS.line(), OS.col());

    auto TCons = TyCDecl->TyCons;
    for (auto *TCon : TCons) {

      if (auto *TCRecord = dyn_cast<TyConRecord>(TCon)) {
        auto Attrs = TCRecord->Attrs;
        auto Fields = TCRecord->RecordFields;
        for (auto *Attr : Attrs) {
          generateCodeFromTerm(OS, Attr);
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
          generateCodeFromTerm(OS, ElemTerm);
          if (I < FldsSize - 1) {
            OS << PulseSyntax::Semicolon;
          }
          I++;
          OS << PulseSyntax::NewLine;
        }

        OS << PulseSyntax::NewLine;
        OS << PulseSyntax::ClosingCurlyBrace;
        PulseSourceLocation End = PulseSourceLocation(OS.line(), OS.col());
        PulseSourceRange Range = PulseSourceRange(Start, End);
        PulseLocsToCLocs.push_back(std::make_pair(Range, TyCDecl->getCSourceInfo()));

        OS << PulseSyntax::NewLine;
      }
    }
  } else if (auto *PulseTopLevelLet = dyn_cast<TopLevelLet>(FD)) {
    
    PulseSourceLocation Start(OS.line(), OS.col());
    
    OS << PulseSyntax::LetBind;
    OS << PulseSyntax::Space;
    OS << PulseTopLevelLet->Ident;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::PulseLetAssignmentOpRef;
    OS << PulseSyntax::Space;
    OS << PulseTopLevelLet->Lhs;

    PulseSourceLocation End = PulseSourceLocation(OS.line(), OS.col());
    PulseSourceRange Range = PulseSourceRange(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, PulseTopLevelLet->getCSourceInfo())); 

    OS << PulseSyntax::NewLine;
  } else if (auto *FallBackDeclaration = dyn_cast<GenericDecl>(FD)){
    PulseSourceLocation Location(OS.line(), OS.col());    
    OS << FallBackDeclaration->Ident;

    PulseSourceRange Range(Location);
    PulseLocsToCLocs.push_back(std::make_pair(Range, FallBackDeclaration->getCSourceInfo()));
  } 
  else if (auto *PulseFunDecl = dyn_cast<PulseFnDecl>(FD)) {

    PulseSourceLocation Start(OS.line(), OS.col());
     
    auto *FuncDef = PulseFunDecl->Defn;
    auto Args = FuncDef->Args;
    auto FuncName = FuncDef->Name;

    //Write out the annotations attached to the function
    for (auto *Att : FuncDef->Attr) {
      generateCodeFromTerm(OS, Att);
      OS << PulseSyntax::NewLine;
    }
    
    //TODO: maybe for recursive functions these need to be extended 
    // with a isRec field.
    OS << PulseSyntax::PulseFunctionDeclaration; 
    OS << PulseSyntax::Space;
    OS << FuncName;

    // If there were no args, we still want to write () in the function
    if (Args.empty()) {
      OS << PulseSyntax::Space;
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
    }

    OS << PulseSyntax::NewLine;

    for (auto *Arg : Args) {
      auto *Ty = Arg->Type;
      auto Val = Arg->Ident;

      if (!Arg->useFallBack) {
        OS << PulseSyntax::OpeningParenthesis;
        OS << Val;
        OS << PulseSyntax::Space;
        OS << PulseSyntax::Colon;
        OS << PulseSyntax::Space;
        generateCodeFromTerm(OS, Ty);
        OS << PulseSyntax::ClosingParenthesis;
        OS << PulseSyntax::NewLine;
      } else {
        OS << PulseSyntax::OpeningParenthesis;
        OS << Val;
        OS << PulseSyntax::ClosingParenthesis;
        OS << PulseSyntax::NewLine;
      }
    }

    //Codegen the specs;
    for (auto *A : FuncDef->Annotation) {
      generateCodeFromTerm(OS, A);
    }
    
    //Vidush: For now always admit these kinds of function declarations
    OS << PulseSyntax::OpeningCurlyBrace; 
    OS << PulseSyntax::NewLine;
    OS << "admit";
    OS << PulseSyntax::OpeningParenthesis;
    OS << PulseSyntax::ClosingParenthesis;
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;
    OS << PulseSyntax::ClosingCurlyBrace;

    OS << PulseSyntax::NewLine;
    
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, PulseFunDecl->getCSourceInfo()));

  } else {
    emitError("(generateCodeFromPulseAST): Encountered an unknown pulse "
              "declaration type!\n");
  }
}

void PulseCodeGen::generateCodeFromTerm(osstream_with_pos &OS,
                                               Term *T) {

  if (!T) {
    return;
  }

  if (Paren *P = dyn_cast<Paren>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::OpeningParenthesis;
    generateCodeFromTerm(OS, P->InnerExpr);
    OS << PulseSyntax::ClosingParenthesis;

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, P->getCSourceInfo()));

  } else if (ConstTerm *CT = dyn_cast<ConstTerm>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());
    switch (CT->Symbol) {
    case SymbolTable::Int32: {
      OS << CT->ConstantValue + "l";
      break;
    }
    case SymbolTable::Int64: {
      OS << CT->ConstantValue + "L";
      break;
    }
    case SymbolTable::Int8: {
      OS << CT->ConstantValue + "y";
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
      OS << CT->ConstantValue + "ul";
      break;
    }
    case SymbolTable::UInt64: {
      OS << CT->ConstantValue + "UL";
      break;
    }
    case SymbolTable::UInt128: {
      emitError("ConstTerm: did not implement case for UInt128!\n");
    }
    case SymbolTable::SizeT: {
      OS << CT->ConstantValue + "sz";
      break;
    }
    case SymbolTable::Bool: {
      if (CT->ConstantValue == "0") {
        OS << "false";
      } else {
        OS << "true";
      }
      break;
    }
    default: {
      //CT->dumpPretty();
      emitError("Did not expect pulse type!");
      break;
    }
    }

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, CT->getCSourceInfo()));

  } else if (VarTerm *VT = dyn_cast<VarTerm>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());
    OS << VT->VarName;

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, VT->getCSourceInfo()));
  } else if (Name *N = dyn_cast<Name>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());
    OS << N->NamedValue;

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (FStarType *FT = dyn_cast<FStarType>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());
    OS << FT->NamedValue;
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (FStarPointerType *FPT = dyn_cast<FStarPointerType>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());
    
    OS << PulseSyntax::OpeningParenthesis;
    OS << PulseSyntax::Space;
    OS << PulseSyntax::Reference;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, FPT->PointerTo);
    OS << PulseSyntax::ClosingParenthesis;
    OS << PulseSyntax::Space;

    PulseSourceLocation End(OS.line(), OS.col()); 
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (FStarArrType *FAT = dyn_cast<FStarArrType>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::Array;

    OS << PulseSyntax::Space;

    generateCodeFromTerm(OS, FAT->ElementType);

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));
  } else if (AppE *App = dyn_cast<AppE>(T)) {
    
    PulseSourceLocation Start(OS.line(), OS.col());
    generateCodeFromTerm(OS, App->CallName);
    OS << PulseSyntax::Space;

    for (size_t i = 0; i < App->Args.size(); ++i) {
      generateCodeFromTerm(OS, App->Args[i]);
      if (i < (App->Args).size() - 1) {
        OS << PulseSyntax::Space;
      }
    }

    if (App->Args.empty()) {
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
    }

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (Ensures *Ensure = dyn_cast<Ensures>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::Ensures;
    OS << PulseSyntax::Space;
    OS << Ensure->Ann;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (Requires *Require = dyn_cast<Requires>(T)) {
    
    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::Requires;
    OS << PulseSyntax::Space;
    OS << Require->Ann;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (Preserves *Preserve = dyn_cast<Preserves>(T)) {
    
    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::Preserves;

    OS << PulseSyntax::Space;
    OS << Preserve->Ann;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
  }
  else if (Returns *Return = dyn_cast<Returns>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::Returns;
    OS << PulseSyntax::Space;
    OS << Return->Ann;
    //End

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (Lemma *UserTerm = dyn_cast<Lemma>(T)) {
    
    PulseSourceLocation Start(OS.line(), OS.col());
    for (auto Lemma : UserTerm->lemmas){
      OS << Lemma;
    }
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);

    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (LemmaStatement *S = dyn_cast<LemmaStatement>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());

    OS << S->Lemma;
    
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } else if (Project *P = dyn_cast<Project>(T)) {
    PulseSourceLocation Start(OS.line(), OS.col());

    generateCodeFromTerm(OS, P->BaseTerm);
    OS << PulseSyntax::Dot;
    OS << P->MemberName;

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  }
  else if (IfExpr *If = dyn_cast<IfExpr>(T)){

    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::PulseIf;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, If->Cond);
    
    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::IfExprThen; 
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, If->TrueExpr);

    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::IfExprElse;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, If->FalseExpr);

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  }
  else {
    T->dumpPretty();
    emitError("Did not expect Pulse AST Node!");
  }

  return;
}

void PulseCodeGen::generateCodeFromPulseStmt(osstream_with_pos &OS,
                                             PulseStmt *T) {

  if (!T)
    return;

  if (PulseExpr *S = dyn_cast<PulseExpr>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    generateCodeFromTerm(OS, S->E);

    OS << PulseSyntax::Semicolon;
    //End

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;
  } else if (PulseAssignment *A = dyn_cast<PulseAssignment>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    generateCodeFromTerm(OS, A->Lhs);
    OS << PulseSyntax::Space;
    OS << PulseSyntax::PulseAssignmentOpRef;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, A->Value);

    OS << PulseSyntax::Semicolon;
    //End 
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


    OS << PulseSyntax::NewLine;
  } else if (PulseArrayAssignment *AS = dyn_cast<PulseArrayAssignment>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    auto *Base = AS->Arr;
    auto *Idx = AS->Index;
    auto *LVal = AS->Value;

    generateCodeFromTerm(OS, Base);
    
    OS << PulseSyntax::Dot;
    
    OS << PulseSyntax::OpeningParenthesis;

    generateCodeFromTerm(OS, Idx);
    
    OS << PulseSyntax::ClosingParenthesis;

    OS << PulseSyntax::Space;

    OS << PulseSyntax::ArrAssignment;

    OS << PulseSyntax::Space;

    generateCodeFromTerm(OS, LVal);

    OS << PulseSyntax::Semicolon;

    //End 
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (LetBinding *Let = dyn_cast<LetBinding>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    if (Let->Qualifier == MutOrRef::MUT) {
      OS << PulseSyntax::LetMut;
    } else {
      OS << PulseSyntax::LetBind;
    }

    OS << PulseSyntax::Space;
    OS << Let->VarName;

    OS << PulseSyntax::Space;

    //Add type annotation for variable in let binding
    OS << PulseSyntax::Colon; 
    OS << PulseSyntax::Space; 
    OS << Let->VarTy; 
    OS << PulseSyntax::Space;

    OS << PulseSyntax::PulseLetAssignmentOpRef;
    OS << PulseSyntax::Space;
    generateCodeFromTerm(OS, Let->LetInit);
    
    OS << PulseSyntax::Semicolon;
    //End

    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


    OS << PulseSyntax::NewLine;

  } else if (PulseIf *If = dyn_cast<PulseIf>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    OS << PulseSyntax::PulseIf;

    auto *PulseIfCond = If->Head;
    auto *PulseThen = If->Then;
    auto *PulseElse = If->Else;

    OS << PulseSyntax::OpeningParenthesis;
    generateCodeFromTerm(OS, PulseIfCond);
    OS << PulseSyntax::ClosingParenthesis;
    
    //Reset Col Counter at every new line
    OS << PulseSyntax::NewLine;

    // Add all the if lemmas here
    for (auto &Lemma : If->IfLemmas) {
      generateCodeFromTerm(OS, Lemma);
    }

    OS << PulseSyntax::OpeningCurlyBrace;

    OS << PulseSyntax::NewLine;

    if (PulseThen == nullptr){
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;

      OS << PulseSyntax::NewLine;
    }
    generateCodeFromPulseStmt(OS, PulseThen);
    OS << PulseSyntax::ClosingCurlyBrace;

    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::PulseElse;

    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::OpeningCurlyBrace;

    OS << PulseSyntax::NewLine;

    if (PulseElse == nullptr){
      OS << PulseSyntax::OpeningParenthesis;
      OS << PulseSyntax::ClosingParenthesis;
      OS << PulseSyntax::NewLine;
    }
    generateCodeFromPulseStmt(OS, PulseElse);
    OS << PulseSyntax::ClosingCurlyBrace;
    // Seems like Pulse If statements need an semicolon at the end
    //TODO: Vidush ensure this is correct?
    //Maybe only needed when the if has some accompanying lemmas?
    OS << PulseSyntax::Semicolon;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

    OS << PulseSyntax::NewLine;

  } else if (PulseWhileStmt *While = dyn_cast<PulseWhileStmt>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());
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
      generateCodeFromTerm(OS, Lemma);
      if (Idx < Lemmas.size()){
        OS << PulseSyntax::NewLine;
      }
      Idx++;
    }

    OS << PulseSyntax::NewLine;

    OS << PulseSyntax::OpeningCurlyBrace;

    OS << PulseSyntax::NewLine;

    generateCodeFromPulseStmt(OS, WBod);
    OS << PulseSyntax::ClosingCurlyBrace;
    OS << PulseSyntax::Semicolon;
    OS << PulseSyntax::NewLine;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));


  } else if (PulseSequence *Seq = dyn_cast<PulseSequence>(T)) {

    PulseSourceLocation Start(OS.line(), OS.col());

    auto *S1 = Seq->S1;
    auto *S2 = Seq->S2;
    generateCodeFromPulseStmt(OS, S1);
    generateCodeFromPulseStmt(OS, S2);

    //End
    PulseSourceLocation End(OS.line(), OS.col());
    PulseSourceRange Range(Start, End);
    PulseLocsToCLocs.push_back(std::make_pair(Range, T->getCSourceInfo()));

  } 
  else if (auto *FallBack = dyn_cast<GenericStmt>(T)){
    PulseSourceLocation Start(OS.line(), OS.col());

    OS << FallBack->body;

    OS << PulseSyntax::NewLine;

    //End
    PulseSourceLocation End(OS.line(), OS.col());
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
    { "isVerbatim", Info.IsVerbatim },
    {"clangAstNode", Info.ClangAstNode}
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