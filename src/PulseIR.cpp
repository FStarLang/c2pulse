#include "PulseIR.h"
#include "Globals.h"
#include "clang/AST/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include <cstddef>
#include <regex.h>

// define functions for F* classes.

// void RegionRange::setStartLine(unsigned Line){
//   Start.Line = Line;
// }

// void RegionRange::setEndLine(unsigned Line){
//   End.Line = Line;
// }

// void RegionRange::setStartColumn(unsigned Col){
//   Start.Column = Col;
// }

// void RegionRange::setEndColumn(unsigned Col){
//   End.Column = Col;
// }

SourceInfo &RegionMapping::getCInfo() { return CInfo; }

SourceInfo &RegionMapping::getPulseInfo() { return PulseInfo; }

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

const char* lookupSymbol(SymbolTable Key){
  return SymbolToStringTable.lookup(Key);
}

static bool matchAnnotation(std::string Start, std::string End,
                            std::string CleanedString, std::string &Match) {

  size_t StartPos = CleanedString.find(Start);
  size_t EndPos = CleanedString.find(End);
  if (StartPos != std::string::npos) {
    std::string FirstPart = CleanedString.substr(0, StartPos);
    Match = CleanedString.substr(StartPos + Start.length(),
                                 EndPos - (StartPos + Start.length()));
    if (!Match.empty() && Match.front() == '"' && Match.back() == '"') {
      Match = Match.substr(1, Match.size() - 2);
      return true;
    }
    return true;
  }

  return false;
}

PulseAnnKind getPulseAnnKindFromString(llvm::StringRef Data,
                                       std::string &Match) {

  if (Data.empty()) {
    emitError("(getPulseAnnKindFromString) Expected data to be non empty!\n");
  }

  std::string CleanedString;
  for (auto c : Data.trim().bytes()) {
    if (c != '\r') {
      CleanedString += c;
    }
  }

  std::array<std::string, 10> StartOfAnnotations = {
      "requires:", "ensures:",       "array:",          "lemma:",
      "returns:",  "erased_arg:",    "heap_allocated:", "assert:",
      "includes:", "expect_failure:"};

  std::array<PulseAnnKind, 10> ArrayAnnotationKinds = {
      PulseAnnKind::Requires,      PulseAnnKind::Ensures,
      PulseAnnKind::IsArray,       PulseAnnKind::LemmaStatement,
      PulseAnnKind::Returns,       PulseAnnKind::ErasedArg,
      PulseAnnKind::HeapAllocated, PulseAnnKind::Assert,
      PulseAnnKind::Includes,      PulseAnnKind::ExpectFailure};

  std::string EndDelimiter = "|END";
  size_t NumAnnotations = StartOfAnnotations.size();

  for (size_t I = 0; I < NumAnnotations; I++) {

    auto AnnonKind = StartOfAnnotations[I];
    auto ArrKind = ArrayAnnotationKinds[I];

    if (matchAnnotation(AnnonKind, EndDelimiter, CleanedString, Match)) {
      return ArrKind;
    }
  }

  emitError("getPulseAnnKindFromString: Encountered an unknown annotation!\n");
  return PulseAnnKind::Unknown;
}

SymbolTable getSymbolKeyForCType(clang::QualType Ty, clang::ASTContext &Ctx) {

  if (Ty->isSignedIntegerType()) {

    if (Ctx.getTypeSize(Ty) == 8) {
      return SymbolTable::Int8;
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return SymbolTable::Int16;
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return SymbolTable::Int32;
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return SymbolTable::Int64;
    } else {
      emitError("(getSymbolKeyForType): did not expect Clang type!\n");
    }

  } else if (Ty->isUnsignedIntegerType()) {

    // check explicitly if it is size_t
    if (Ty.getAsString() == "size_t") {
      return SymbolTable::SizeT;
    }

    if (Ty.getAsString() == "_Bool") {
      return SymbolTable::Bool;
    }

    if (Ctx.getTypeSize(Ty) == 8) {
      return SymbolTable::UInt8;
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return SymbolTable::UInt16;
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return SymbolTable::UInt32;
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return SymbolTable::UInt64;
    } else if (Ctx.getTypeSize(Ty) == 128) {
      return SymbolTable::UInt128;
    } else {
      emitError("(getSymbolKeyForType): did not expect C type!\n");
    }

  } else if (Ty.getAsString() == "size_t") {
    return SymbolTable::SizeT;
  } else if (Ty.getAsString() == "_Bool") {
    return SymbolTable::Bool;
  } else if (Ty->isArrayType()) {
    return SymbolTable::Array;
  }
  // What about structs
  else if (Ty->isStructureType() || Ty->isUnionType()) {
    // We do not handle structs and unions in this function.
    // We return UNKNOWN type from this function.
    return SymbolTable::UNKNOWN;

  } else if (Ty->isPointerType()) {
    return SymbolTable::Ref;
  }

  Ty->dump();
  emitError("(getSymbolKeyForCType): Did not expect C type!\n");
}

const char *getSymbolKeyForOperator(SymbolTable Val,
                                    clang::BinaryOperatorKind &Op) {

  switch (Op) {
  case clang::BO_PtrMemD:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_PtrMemD!\n");
    break;
  }
  case clang::BO_PtrMemI:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_PtrMemI!\n");
    break;
  }
  case clang::BO_Mul: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Mul);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Mul);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Mul);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Mul);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Mul);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Mul);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Mul);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Mul);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Mul);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_Mul!\n");
    }
    break;
  }
  case clang::BO_Div: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Div);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Div);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Div);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Div);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Div);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Div);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Div);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Div);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Div);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_Div!\n");
    }
    break;
  }
  case clang::BO_Rem:{
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Rem);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Rem);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Rem);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Rem);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Rem);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Rem);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Rem);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Rem);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Rem);
    } else {
      emitError("(getSymbolKeyForOperator): Not implemented BO_Rem!\n");
    }
    break;
  }
  case clang::BO_Add: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Add);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Add);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Add);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Add);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Add);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Add);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Add);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Add);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Add);
    } else {
      emitError("(getSymbolKeyForOperator): Not implemented BO_Add!\n");
    }

    break;
  }
  case clang::BO_Sub: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Sub);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Sub);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Sub);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Sub);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Sub);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Sub);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Sub);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Sub);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Sub);
    }

    break;
  }
  case clang::BO_Shl:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Shl!\n");
    break;
  }
  case clang::BO_Shr:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Shr!\n");
    break;
  }
  case clang::BO_Cmp:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Cmp!\n");
    break;
  }
  case clang::BO_LT: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Lt);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Lt);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Lt);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Lt);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Lt);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Lt);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Lt);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Lt);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Lt);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_LT!\n");
    }
    break;
  }
  case clang::BO_GT: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Gt);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Gt);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Gt);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Gt);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Gt);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Gt);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Gt);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Gt);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Gt);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_GT!\n");
    }
    break;
  }
  case clang::BO_LE:{
    emitError("(getSymbolKeyForOperator): Unknown case in BO_LE!\n");
    break;
  }
  case clang::BO_GE:{
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Ge);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Ge);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Ge);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Ge);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Ge);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Ge);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Ge);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Ge);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Ge);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_GE!\n");
      ;
    }
    break;
  }
  case clang::BO_EQ: {
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Eq);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Eq);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Eq);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Eq);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Eq);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Eq);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Eq);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Eq);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Eq);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_EQ!\n");
    }
    break;
  }
  case clang::BO_NE:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_NE!\n");
    break;
  }
  case clang::BO_And:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_And!\n");
    break;
  }
  case clang::BO_Xor:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Xor!\n");
    break;
  }
  case clang::BO_Or:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Or!\n");
    break;
  }
  case clang::BO_LAnd:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_LAnd!\n");
    break;
  }
  case clang::BO_LOr:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_LOr!\n");
    break;
  }
  case clang::BO_Assign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Assign!\n");
    break;
  }
  case clang::BO_MulAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_MulAssign!\n");
    break;
  }
  case clang::BO_DivAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_DivAssign!\n");
    break;
  }
  case clang::BO_RemAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_RemAssign!\n");
    break;
  }
  case clang::BO_AddAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_AddAssign!\n");
    break;
  }
  case clang::BO_SubAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_SubAssign!\n");
    break;
  }
  case clang::BO_ShlAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_ShlAssign!\n");
    break;
  }
  case clang::BO_ShrAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_ShrAssign!\n");
    break;
  }
  case clang::BO_AndAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_AndAssign!\n");
    break;
  }
  case clang::BO_XorAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_XorAssign!\n");
    break;
  }
  case clang::BO_OrAssign:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_OrAssign!\n");
    break;
  }
  case clang::BO_Comma:{
    emitError("(getSymbolKeyForOperator): Not implemented BO_Comma!\n");
    break;
  }
  default:
    break;
  }
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

Binder::Binder(std::string FallBack) { Ident = FallBack; }

RegionMapping &Binder::getRegInfoMapping(){
  return RegInfo;
}

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

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, PulseDeclKind T) {
  switch (T) {
  case PulseDeclKind::FnDecl:
    os << "FnDecl";
    break;
  case PulseDeclKind::FnDefn:
    os << "FnDefn";
    break;
  case PulseDeclKind::TyconDecl:
    os << "TyconDecl";
    break;
  case PulseDeclKind::ValDecl:
    os << "ValDecl";
    break;
  }
  return os;
}

void Term::setTag(TermTag T) { Tag = T; }

void Term::printTag() { llvm::outs() << Tag << "\n"; }

void Term::dumpPretty() { printTag(); }


RegionMapping &Term::getRegInfoMapping(){
  return RegInfo;
}


Project::Project(){
  Tag = TermTag::Project;
}

void Project::dumpPretty() {
  BaseTerm->dumpPretty(); 
  llvm::outs() << "." + MemberName;
}

void Lemma::dumpPretty() {
  for (auto lemma : lemmas) {
    llvm::outs() << lemma << "\n";
  }
}

GenericStmt::GenericStmt() { Tag = PulseStmtTag::GenericStmt; }

void GenericStmt::dumpPretty() { llvm::outs() << body << "\n"; }

void LemmaStatement::dumpPretty() { llvm::outs() << Lemma << "\n"; }

Lemma::Lemma() { Tag = TermTag::Lemma; }

LemmaStatement::LemmaStatement() { Tag = TermTag::LemmaStatement; }

Paren::Paren() {
  llvm::outs() << "Called Paren Constructor!!" << "\n";
  Tag = TermTag::Paren;
}

Paren::Paren(Term *Expr) {
  Tag = TermTag::Paren;
  InnerExpr = Expr;
}

void Paren::setInnerExpr(Term *Inner) { InnerExpr = Inner; }

void Paren::dumpPretty() {
  llvm::outs() << "(";
  InnerExpr->dumpPretty();
  llvm::outs() << ")";
}

ConstTerm::ConstTerm() { Tag = TermTag::Const; }

Ensures::Ensures() { Tag = TermTag::Ensures; }

Returns::Returns() { Tag = TermTag::Returns; }

void Ensures::dumpPretty() {
  llvm::outs() << "Ensures ";
  llvm::outs() << Ann;
}

void Requires::dumpPretty() {
  llvm::outs() << "Requires ";
  llvm::outs() << Ann;
}

void Returns::dumpPretty() {
  llvm::outs() << "Requires ";
  llvm::outs() << Ann;
}

Requires::Requires() {
  Tag = TermTag::Requires;
  Ann = "";
}

void ConstTerm::dumpPretty() { llvm::outs() << ConstantValue; }

VarTerm::VarTerm() { Tag = TermTag::Var; }

VarTerm::VarTerm(std::string Name) {
  Tag = TermTag::Var;
  VarName = Name;
}

Name::Name() { Tag = TermTag::Name; }

Name::Name(std::string Name) {
  Tag = TermTag::Name;
  NamedValue = Name;
}

std::string Name::print() { return NamedValue; }

void VarTerm::setVarName(std::string Name) { VarName = Name; }

void VarTerm::dumpPretty() { llvm::outs() << VarName; }

void Name::setName(std::string Name) { NamedValue = Name; }

void Name::dumpPretty() { llvm::outs() << NamedValue; }

void FStarType::setName(std::string Name) { NamedValue = Name; }

void FStarType::dumpPretty() { llvm::outs() << NamedValue; }

FStarType::FStarType() { Tag = TermTag::FStarType; }

std::string FStarType::print() { return NamedValue; }

FStarType::FStarType(std::string Name) {
  Tag = TermTag::FStarType;
  NamedValue = Name;
}

void FStarArrType::setName(std::string Name) { NamedValue = Name; }
void FStarPointerType::setName(std::string Name) { NamedValue = Name; }

void FStarArrType::dumpPretty() {
  llvm::outs() << "array ";
  ElementType->dumpPretty();
}

void FStarPointerType::dumpPretty() {
  llvm::outs() << "ref ";
  PointerTo->dumpPretty();
}

std::string FStarPointerType::print() {
  std::string Out = "";
  Out += "(ref ";
  Out += PointerTo->print();
  Out += ")";
  return Out;
}

RegionMapping &PulseStmt::getRegInfoMapping(){
  return RegInfo;
}

FStarArrType::FStarArrType() { Tag = TermTag::FStarArrType; }

void FStarArrType::setElementTy(FStarType *Type) { ElementType = Type; }
void FStarPointerType::setPointerToTy(FStarType *Type) { PointerTo = Type; }

FStarPointerType::FStarPointerType() { Tag = TermTag::FStarPointerType; }

AppE::AppE() {
  Tag = TermTag::AppE;
  Args.clear();
}

AppE::AppE(std::string CallName) {
  Tag = TermTag::AppE;
  makeCallName(CallName);
}

void AppE::dumpPretty() {
  llvm::outs() << CallName->VarName;
  llvm::outs() << " ";
  size_t Idx = 1;
  for (auto *Arg : Args) {
    if (Arg == nullptr)
      continue;
    Arg->dumpPretty();
    if (Idx < Args.size())
      llvm::outs() << " ";
    Idx++;
  }
}

void AppE::setCallName(VarTerm *Call) { CallName = Call; }

void AppE::makeCallName(std::string CallName) {
  VarTerm *Call = new VarTerm();
  Call->setVarName(CallName);
  this->setCallName(Call);
}

void AppE::pushArg(Term *Arg) { Args.push_back(Arg); }

void PulseStmt::setTag(PulseStmtTag T) { Tag = T; }

void PulseStmt::printTag() { llvm::outs() << Tag << "\n"; }

void PulseStmt::dumpPretty() { PulseStmt::printTag(); }

void PulseExpr::dumpPretty() {
  if (E)
    E->dumpPretty();
}

PulseExpr::PulseExpr() { Tag = PulseStmtTag::Expr; }

void PulseAssignment::dumpPretty() {
  Lhs->dumpPretty();
  llvm::outs() << " := ";
  Value->dumpPretty();
  llvm::outs() << "\n";
}

PulseAssignment::PulseAssignment(){
  Tag = PulseStmtTag::Assignment;
}

PulseAssignment::PulseAssignment(Term *LhsTerm, Term *RhsTerm) {
  Tag = PulseStmtTag::Assignment;
  Lhs = LhsTerm;
  Value = RhsTerm;
}

void PulseArrayAssignment::dumpPretty() {

  Arr->dumpPretty();
  llvm::outs() << ".(";
  Index->dumpPretty();
  llvm::outs() << ")";
  llvm::outs() << " <- ";
  Value->dumpPretty();
  llvm::outs() << "\n";
}

PulseArrayAssignment::PulseArrayAssignment() {
  Tag = PulseStmtTag::ArrayAssignment;
}

LetBinding::LetBinding() { Tag = PulseStmtTag::LetBinding; }

LetBinding::LetBinding(std::string Lhs, Term *Rhs, MutOrRef Qual) {
  Tag = PulseStmtTag::LetBinding;
  VarName = Lhs;
  LetInit = Rhs;
  Qualifier = Qual;
}

void LetBinding::dumpPretty() {
  llvm::outs() << "let ";
  if (Qualifier == MutOrRef::MUT) {
    llvm::outs() << "mut ";
  }
  llvm::outs() << VarName << " = ";
  if (LetInit != nullptr)
    LetInit->dumpPretty();
  llvm::outs() << "\n";
}

void PulseIf::dumpPretty() {}

PulseIf::PulseIf() { Tag = PulseStmtTag::If; }

PulseSequence::PulseSequence() {
  Tag = PulseStmtTag::Sequence;
  S1 = nullptr;
  S2 = nullptr;
}

void PulseSequence::assignS1(PulseStmt *S) { S1 = S; }

void PulseSequence::assignS2(PulseStmt *S) { S2 = S; }

void PulseSequence::dumpPretty() {
  if (S1 != nullptr) {
    S1->dumpPretty();
  }

  if (S2 != nullptr) {
    S2->dumpPretty();
  }
}

void PulseWhileStmt::dumpPretty() {

  llvm::outs() << "while(";
  Guard->dumpPretty();
  llvm::outs() << ")";
  llvm::outs() << "{";
  llvm::outs() << "\n";
  Body->dumpPretty();
  llvm::outs() << "}";
}

PulseWhileStmt::PulseWhileStmt() { Tag = PulseStmtTag::WhileStmt; }

PulseDeclKind PulseDecl::getKind() { return Kind; }

RegionMapping &PulseDecl::getRegInfoMapping(){
  return RegInfo;
}

PulseFnDefn::PulseFnDefn(_PulseFnDefn *Defn) : Defn(Defn) {
  Kind = PulseDeclKind::FnDefn;
}

void PulseFnDefn::dumpPretty() {

  llvm::outs() << "The pulse function Name is: ";
  llvm::outs() << Defn->Name << "\n";

  llvm::outs() << "\n";
  llvm::outs() << "Print function arguments: ";
  for (auto *Arg : (Defn->Args)) {
    if (Arg->useFallBack) {
      llvm::outs() << "(" << Arg->Ident << ")";
      llvm::outs() << ",";
      continue;
    }

    llvm::outs() << "(" << Arg->Ident << ",";
    // Arg->Type->printTag();
    if (auto *Ty = static_cast<FStarType *>((Arg->Type))) {
      Ty->dumpPretty();
      llvm::outs() << ")";
    }
    llvm::outs() << ",";
  }

  llvm::outs() << "\n\nPrint annotations: " << "\n\n";

  for (auto *Ann : Defn->Annotation) {
    Ann->dumpPretty();
    llvm::outs() << "\n";
  }

  llvm::outs() << "\n\n";
  llvm::outs() << "Print the function body: " << "\n\n";
  if (Defn->Body)
    Defn->Body->dumpPretty();
}

TyConDecl::TyConDecl() { Kind = PulseDeclKind::TyconDecl; }

ValDecl::ValDecl() { Kind = PulseDeclKind::ValDecl; }

TyCon::TyCon() { Tag = TyConTag::Base; }

TyConRecord::TyConRecord() { Tag = TyConTag::TyConRecord; }

TopLevelLet::TopLevelLet() { Kind = PulseDeclKind::TopLevelLet; }

GenericDecl::GenericDecl() { Kind = PulseDeclKind::GenericDecl; }


void PulseModul::insertModule(std::string IncModule){
  //Module does not exists
  if (std::find(IncludedModules.begin(), IncludedModules.end(), IncModule) == IncludedModules.end()){
    IncludedModules.push_back(IncModule);
  }
}

// FstarValDecl::FstarValDecl(){
//   Tag = FStarDeclTag::ValDecl;
// }