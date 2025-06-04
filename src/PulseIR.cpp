#include "PulseIR.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

// define functions for F* classes.

// enum class TermTag { Const, Var, Name, AppE, FStarType, FStarPointerType };
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, TermTag T) {
  switch (T) {
  case TermTag::Const:
    os << "Const";
    break;
  case TermTag::Var:
    os << "Var";
    break;
  case TermTag::Name:
    os << "Name";
    break;
  case TermTag::AppE:
    os << "AppE";
    break;
  case TermTag::FStarType:
    os << "FStarType";
    break;
  case TermTag::FStarPointerType:
    os << "FStarPointerType";
    break;
  default:
    os << "Unknown TermTag case";
    break;
  }
  return os;
}

// enum class PulseStmtTag {
//   Expr,
//   Assignment,
//   ArrayAssignment,
//   LetBinding,
//   If,
//   WhileStmt,
//   Sequence
// };

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, PulseStmtTag T) {
  switch (T) {
  case PulseStmtTag::Expr:
    os << "Expr";
    break;
  case PulseStmtTag::Assignment:
    os << "Assignment";
    break;
  case PulseStmtTag::ArrayAssignment:
    os << "ArrayAssignment";
    break;
  case PulseStmtTag::LetBinding:
    os << "LetBinding";
    break;
  case PulseStmtTag::If:
    os << "If";
    break;
  case PulseStmtTag::WhileStmt:
    os << "WhileStmt";
    break;
  case PulseStmtTag::Sequence:
    os << "Sequence";
    break;
  default:
    os << "Unknown PulseStmtTag case";
    break;
  }
  return os;
}

void Term::setTag(TermTag T) { Tag = T; }

void Term::printTag() { llvm::outs() << Tag << "\n"; }

void Term::dumpPretty() { printTag(); }

void ConstTerm::dumpPretty() { llvm::outs() << ConstantValue; }

void VarTerm::setVarName(std::string Name) { VarName = Name; }

void VarTerm::dumpPretty() { llvm::outs() << VarName; }

void Name::setName(std::string Name) { NamedValue = Name; }

void Name::dumpPretty() { llvm::outs() << NamedValue; }

void FStarType::setName(std::string Name) { NamedValue = Name; }

void FStarType::dumpPretty() { llvm::outs() << NamedValue; }

void FStarPointerType::setName(std::string Name) { NamedValue = Name; }

void FStarPointerType::dumpPretty() {
  llvm::outs() << "ref ";
  PointerTo->dumpPretty();
}

void FStarPointerType::setPointerToTy(FStarType *Type) { PointerTo = Type; }

void AppE::dumpPretty() {
  llvm::outs() << CallName->VarName;
  for (auto *Arg : Args) {
    Arg->dumpPretty();
  }
}

void AppE::setCallName(VarTerm *Call) { CallName = Call; }

void AppE::pushArg(Term *Arg) { Args.push_back(Arg); }

void PulseStmt::setTag(PulseStmtTag T) { Tag = T; }

void PulseStmt::printTag() { llvm::outs() << Tag << "\n"; }

void PulseStmt::dumpPretty() { PulseStmt::printTag(); }

void PulseExpr::dumpPretty() { E->dumpPretty(); }

void PulseAssignment::dumpPretty() {
  Lhs->dumpPretty();
  llvm::outs() << " := ";
  Value->dumpPretty();
  llvm::outs() << "\n";
}

void LetBinding::dumpPretty() {
  llvm::outs() << "let " << VarName << " = ";
  LetInit->dumpPretty();
  llvm::outs() << "\n";
}

void PulseSequence::assignS1(PulseStmt *S) { S1 = S; }

void PulseSequence::assignS2(PulseStmt *S) { S2 = S; }

void PulseSequence::dumpPretty() {
  S1->dumpPretty();
  // llvm::outs() << "\n";
  S2->dumpPretty();
}

PulseFnDefn::PulseFnDefn(_PulseFnDefn *Defn) : Defn(Defn) {}

void PulseFnDefn::dumpPretty() {

  llvm::outs() << "The pulse function Name is: ";
  llvm::outs() << Defn->Name << "\n";

  llvm::outs() << "\n";
  llvm::outs() << "Print function arguments: ";
  for (auto *Arg : (Defn->Args)) {
    llvm::outs() << "(" << Arg->Ident << ",";
    // Arg->Type->printTag();
    if (auto *Ty = static_cast<FStarType *>((Arg->Type))) {
      Ty->dumpPretty();
      llvm::outs() << ")";
    }
    llvm::outs() << ",";
  }

  llvm::outs() << "\n\n";
  llvm::outs() << "Print the function body: " << "\n\n";
  Defn->Body->dumpPretty();
}