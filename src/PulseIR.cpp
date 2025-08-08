#include "PulseIR.h"
#include "Globals.h"
#include "clang/AST/ASTContext.h"
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

// SourceInfo &RegionMapping::getCInfo() { return CInfo; }

// SourceInfo &RegionMapping::getPulseInfo() { return PulseInfo; }

PulseSourceLocation::PulseSourceLocation(unsigned L, unsigned C){
  Line = L;
  Column = C;
}

unsigned PulseSourceLocation::getLine(){
  return Line;
}

unsigned PulseSourceLocation::getColumn(){
  return Column;
}

void PulseSourceLocation::setLine(unsigned L){
  Line = L;
}

void PulseSourceLocation::setColumn(unsigned C){
  Column = C;
}

void PulseSourceLocation::dumpPretty(){
  
  llvm::outs() << "-------------------------------\n";
  llvm::outs() << "Line: " << Line << "\n";
  llvm::outs() << "Column: " << Column << "\n";
  llvm::outs() << "-------------------------------\n";

}

bool PulseSourceLocation::isSame(PulseSourceLocation ToCheck){
  if (Line == ToCheck.getLine() && Column == ToCheck.getColumn()){
    return true;
  }
  return false;
}

PulseSourceRange::PulseSourceRange(PulseSourceLocation B, PulseSourceLocation E) 
  : Begin(B), End(E){}


PulseSourceRange::PulseSourceRange(PulseSourceLocation B)
  : Begin(B), End(B){}

bool PulseSourceRange::isSingleLocation(){
  if (Begin.isSame(End)){
    return true;
  }
  return false;
}

void PulseSourceRange::dumpPretty(){

  llvm::outs() << "LocRange: (";
  llvm::outs() << "Start: (";
  llvm::outs() << "R: " << Begin.getLine() << ", ";
  llvm::outs() << "C: " << Begin.getColumn() << ")";
  llvm::outs() << ", ";

  llvm::outs() << "End: (";
  llvm::outs() << "R: " << End.getLine() << ", ";
  llvm::outs() << "C: " << End.getColumn() << ")";

  llvm::outs() << ")";
  llvm::outs() << "\n";

}

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

/// A map from symbol to its pulse specific string.
static const llvm::SmallDenseMap<SymbolTable, const char *> SymbolToStringTable{
    {SymbolTable::Int8, "Int8.t"},
    {SymbolTable::Int16, "Int16.t"},
    {SymbolTable::Int32, "Int32.t"},
    {SymbolTable::Int64, "Int64.t"},
    {SymbolTable::UInt8, "UInt8.t"},
    {SymbolTable::UInt16, "UInt16.t"},
    {SymbolTable::UInt32, "UInt32.t"},
    {SymbolTable::UInt64, "UInt64.t"},
    {SymbolTable::UInt128, "UInt128.t"},
    {SymbolTable::SizeT, "SizeT.t"},
    {SymbolTable::Bool, "bool"},
    {SymbolTable::Int, "int"},

    {SymbolTable::SizeT_Add, "SizeT.add"},
    {SymbolTable::SizeT_Sub, "SizeT.sub"},
    {SymbolTable::SizeT_Div, "SizeT.div"},
    {SymbolTable::SizeT_Rem, "SizeT.rem"},
    {SymbolTable::SizeT_Mul, "SizeT.mul"},
    {SymbolTable::SizeT_Eq, "SizeT.eq"},
    {SymbolTable::SizeT_Eq_Int32, "SizeT.eq_int32"},
    {SymbolTable::SizeT_Lt, "SizeT.lt"},
    {SymbolTable::SizeT_Le, "SizeT.lte"},
    {SymbolTable::SizeT_Gt, "SizeT.gt"},
    {SymbolTable::SizeT_Ge, "SizeT.gte"},

    // int 8
    {SymbolTable::Int8_Add, "Int8.add"},
    {SymbolTable::Int8_Sub, "Int8.sub"},
    {SymbolTable::Int8_Div, "Int8.div"},
    {SymbolTable::Int8_Rem, "Int8.rem"},
    {SymbolTable::Int8_Mul, "Int8.mul"},
    {SymbolTable::Int8_Eq, "Int8.eq"},
    {SymbolTable::Int8_Lt, "Int8.lt"},
    {SymbolTable::Int8_Le, "Int8.lte"},
    {SymbolTable::Int8_Gt, "Int8.gt"},
    {SymbolTable::Int8_Ge, "Int8.gte"},

    // uint 8
    {SymbolTable::UInt8_Add, "UInt8.add"},
    {SymbolTable::UInt8_Sub, "UInt8.sub"},
    {SymbolTable::UInt8_Div, "UInt8.div"},
    {SymbolTable::UInt8_Rem, "UInt8.rem"},
    {SymbolTable::UInt8_Mul, "UInt8.mul"},
    {SymbolTable::UInt8_Eq, "UInt8.eq"},
    {SymbolTable::UInt8_Lt, "UInt8.lt"},
    {SymbolTable::UInt8_Le, "UInt8.lte"},
    {SymbolTable::UInt8_Gt, "UInt8.gt"},
    {SymbolTable::UInt8_Ge, "UInt8.gte"},

    // int 16
    {SymbolTable::Int16_Add, "Int16.add"},
    {SymbolTable::Int16_Sub, "Int16.sub"},
    {SymbolTable::Int16_Div, "Int16.div"},
    {SymbolTable::Int16_Rem, "Int16.rem"},
    {SymbolTable::Int16_Mul, "Int16.mul"},
    {SymbolTable::Int16_Eq, "Int16.eq"},
    {SymbolTable::Int16_Lt, "Int16.lt"},
    {SymbolTable::Int16_Le, "Int16.lte"},
    {SymbolTable::Int16_Gt, "Int16.gt"},
    {SymbolTable::Int16_Ge, "Int16.gte"},

    // uint 16
    {SymbolTable::UInt16_Add, "UInt16.add"},
    {SymbolTable::UInt16_Sub, "UInt16.sub"},
    {SymbolTable::UInt16_Div, "UInt16.div"},
    {SymbolTable::UInt16_Rem, "UInt16.rem"},
    {SymbolTable::UInt16_Mul, "UInt16.mul"},
    {SymbolTable::UInt16_Eq, "UInt16.eq"},
    {SymbolTable::UInt16_Lt, "UInt16.lt"},
    {SymbolTable::UInt16_Le, "UInt16.lte"},
    {SymbolTable::UInt16_Gt, "UInt16.gt"},
    {SymbolTable::UInt16_Ge, "UInt16.gte"},

    // int 32
    {SymbolTable::Int32_Add, "Int32.add"},
    {SymbolTable::Int32_Sub, "Int32.sub"},
    {SymbolTable::Int32_Div, "Int32.div"},
    {SymbolTable::Int32_Rem, "Int32.rem"},
    {SymbolTable::Int32_Mul, "Int32.mul"},
    {SymbolTable::Int32_Eq, "Int32.eq"},
    {SymbolTable::Int32_Lt, "Int32.lt"},
    {SymbolTable::Int32_Le, "Int32.lte"},
    {SymbolTable::Int32_Gt, "Int32.gt"},
    {SymbolTable::Int32_Ge, "Int32.gte"},

    // uint 32
    {SymbolTable::UInt32_Add, "UInt32.add"},
    {SymbolTable::UInt32_Sub, "UInt32.sub"},
    {SymbolTable::UInt32_Div, "UInt32.div"},
    {SymbolTable::UInt32_Rem, "UInt32.rem"},
    {SymbolTable::UInt32_Mul, "UInt32.mul"},
    {SymbolTable::UInt32_Eq, "UInt32.eq"},
    {SymbolTable::UInt32_Eq_Int32, "Int32.eq_int32"},
    {SymbolTable::UInt32_Lt, "UInt32.lt"},
    {SymbolTable::UInt32_Le, "UInt32.lte"},
    {SymbolTable::UInt32_Gt, "UInt32.gt"},
    {SymbolTable::UInt32_Ge, "UInt32.gte"},

    // int 64
    {SymbolTable::Int64_Mul, "Int64.mul"},
    {SymbolTable::Int64_Add, "Int64.add"},
    {SymbolTable::Int64_Sub, "Int64.sub"},
    {SymbolTable::Int64_Div, "Int64.div"},
    {SymbolTable::Int64_Rem, "Int64.rem"},
    {SymbolTable::Int64_Eq, "Int64.eq"},
    {SymbolTable::Int64_Lt, "Int64.lt"},
    {SymbolTable::Int64_Le, "Int64.lte"},
    {SymbolTable::Int64_Gt, "Int64.gt"},
    {SymbolTable::Int64_Ge, "Int64.gte"},

    // U int 64
    {SymbolTable::UInt64_Mul, "UInt64.mul"},
    {SymbolTable::UInt64_Add, "UInt64.add"},
    {SymbolTable::UInt64_Sub, "UInt64.sub"},
    {SymbolTable::UInt64_Div, "UInt64.div"},
    {SymbolTable::UInt64_Rem, "UInt64.rem"},
    {SymbolTable::UInt64_Eq, "UInt64.eq"},
    {SymbolTable::UInt64_Eq_Int32, "UInt64.eq_int32"},
    {SymbolTable::UInt64_Lt, "UInt64.lt"},
    {SymbolTable::UInt64_Le, "UInt64.lte"},
    {SymbolTable::UInt64_Gt, "UInt64.gt"},
    {SymbolTable::UInt64_Ge, "UInt64.gte"},

    // logical operators
    {SymbolTable::OpAmpAmp, "op_AmpAmp"},
    {SymbolTable::OpBarBar, "op_BarBar"},

    {SymbolTable::Array, "array"},
    {SymbolTable::Ref, "ref"},
};


const char* lookupSymbol(SymbolTable Key){
  if (SymbolToStringTable.count(Key)) {
    return SymbolToStringTable.lookup(Key);
  } else {
    emitError("Key not found in SymbolTable!");
  }
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

  std::array<std::string, 11> StartOfAnnotations = {
      "requires:", "ensures:",   "preserves:",    "array:",          "lemma:",
      "returns:",  "erased_arg:",    "heap_allocated:", "assert:",
      "includes:", "expect_failure:"};

  std::array<PulseAnnKind, 11> ArrayAnnotationKinds = {
      PulseAnnKind::Requires,      PulseAnnKind::Ensures,
          PulseAnnKind::Preserves,PulseAnnKind::IsArray,       
          PulseAnnKind::LemmaStatement, PulseAnnKind::Returns,       
          PulseAnnKind::ErasedArg, PulseAnnKind::HeapAllocated, PulseAnnKind::Assert,
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


std::string getCastNameForPulseType(Term *T){

  if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::Int8)){
    return "int8";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::Int16)){
    return "int16";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::Int32)){
    return "int32";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::Int64)){
    return "int64";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::UInt8)){
    return "uint8";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::UInt16)){
    return "uint16";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::UInt32)){
    return "uint32";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::UInt64)){
    return "uint64";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::UInt128)){
    return "uint128";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::Bool)){
    return "bool";
  }
  else if (getPulseTyAsString(T) == lookupSymbol(SymbolTable::SizeT)){
    return "sizet";
  }
  else{
    T->dumpPretty();
    emitError("Not Implemented!");
  }
}

std::string getPulseStringForCType(clang::QualType Ty, clang::ASTContext &Ctx) {

  if (Ty->isSignedIntegerType()) {

    if (Ctx.getTypeSize(Ty) == 8) {
      return "int8";
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return "int16";
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return "int32";
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return "int64";
    } else {
      emitError("(getPulseStringForCType): did not expect Clang type!\n");
    }

  } else if (Ty->isUnsignedIntegerType()) {

    // check explicitly if it is size_t
    // Vidush: 7/25/2025: We decided to treat size_t as its own type.
    if (Ty.getAsString() == "size_t") {
      return "sizet";
    }

    if (Ty.getAsString() == "_Bool") {
      return "bool";
    }

    if (Ctx.getTypeSize(Ty) == 8) {
      return "uint8";
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return "uint16";
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return "uint32";
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return "uint64";
    } else if (Ctx.getTypeSize(Ty) == 128) {
      return "uint128";
    } else {
      emitError("(getPulseStringForCType): did not expect C type!\n");
    }

  } else if (Ty.getAsString() == "size_t") {
    return "sizet";
  } else if (Ty.getAsString() == "_Bool") {
    return "bool";
  } else if (Ty->isArrayType()) {
    return "array";
  }
  // What about structs
  else if (Ty->isStructureType() || Ty->isUnionType()) {
    // We do not handle structs and unions in this function.
    // We return UNKNOWN type from this function.
    Ty.dump();
    emitError("(getPulseStringForCType): did not expect C type!\n");

  } else if (Ty->isPointerType()) {
    Ty.dump();
    emitError("(getPulseStringForCType): did not expect C type!\n");
  }

  Ty->dump();
  emitError("(getPulseStringForCType): Did not expect C type!\n");
}

std::string getPulseModuleNameForCType(clang::QualType Ty, clang::ASTContext &Ctx) {

  if (Ty->isSignedIntegerType()) {

    if (Ctx.getTypeSize(Ty) == 8) {
      return "Int8";
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return "Int16";
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return "Int32";
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return "Int64";
    } else {
      emitError("(getPulseModuleNameForCType): Module name unknown!\n");
    }

  } else if (Ty->isUnsignedIntegerType()) {

    if (Ty.getAsString() == "size_t") {
      return "SizeT";
    }

    //TODO: Vidush check what bool modules are called in pulse?
    if (Ty.getAsString() == "_Bool") {
      return "bool";
    }

    if (Ctx.getTypeSize(Ty) == 8) {
      return "UInt8";
    } else if (Ctx.getTypeSize(Ty) == 16) {
      return "UInt16";
    } else if (Ctx.getTypeSize(Ty) == 32) {
      return "UInt32";
    } else if (Ctx.getTypeSize(Ty) == 64) {
      return "UInt64";
    } else if (Ctx.getTypeSize(Ty) == 128) {
      return "UInt128";
    } else {
      emitError("(getPulseModuleNameForCType): Module name unknown!\n");
    }

  } else if (Ty.getAsString() == "size_t") {
    return "sizet";
  } else if (Ty.getAsString() == "_Bool") {
    return "bool";
  }

  Ty->dump();
  emitError("(getPulseModuleNameForCType): Module name unknown!\n");
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
  
  llvm::outs() << "\n";
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
    if (Val == SymbolTable::Int8) {
      return lookupSymbol(SymbolTable::Int8_Le);
    } else if (Val == SymbolTable::Int16) {
      return lookupSymbol(SymbolTable::Int16_Le);
    } else if (Val == SymbolTable::Int32) {
      return lookupSymbol(SymbolTable::Int32_Le);
    } else if (Val == SymbolTable::Int64) {
      return lookupSymbol(SymbolTable::Int64_Le);
    } else if (Val == SymbolTable::UInt8) {
      return lookupSymbol(SymbolTable::UInt8_Le);
    } else if (Val == SymbolTable::UInt16) {
      return lookupSymbol(SymbolTable::UInt16_Le);
    } else if (Val == SymbolTable::UInt32) {
      return lookupSymbol(SymbolTable::UInt32_Le);
    } else if (Val == SymbolTable::UInt64) {
      return lookupSymbol(SymbolTable::UInt64_Le);
    } else if (Val == SymbolTable::SizeT) {
      return lookupSymbol(SymbolTable::SizeT_Le);
    } else {
      emitError("(getSymbolKeyForOperator): Unknown case in BO_GT!\n");
    }
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
    ///Vidush: 
    ///The parent who calls this should check BO_NE 
    ///and add not around the expr.
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
    return lookupSymbol(SymbolTable::OpAmpAmp);
    break;
  }
  case clang::BO_LOr:{
    return lookupSymbol(SymbolTable::OpBarBar);
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

SourceInfo Binder::getCSourceInfo(){
  return CInfo;
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

Term *Term::getType() { return Type; }

void Term::dumpPretty() { printTag(); }

SourceInfo Term::getCSourceInfo(){
  return CInfo;
}


// RegionMapping &Term::getRegInfoMapping(){
//   return RegInfo;
// }


Project::Project(){
  Tag = TermTag::Project;
}

Project::Project(Term *Ty) {
  Tag = TermTag::Project;
  Type = Ty;
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

Paren::Paren(Term *Expr, Term *Ty) {
  Tag = TermTag::Paren;
  Type = Ty;
  InnerExpr = Expr;
}

void Paren::setInnerExpr(Term *Inner) { InnerExpr = Inner; }

void Paren::dumpPretty() {
  llvm::outs() << "(";
  InnerExpr->dumpPretty();
  llvm::outs() << ")";
}

ConstTerm::ConstTerm(std::string C, Term *Ty) {
  Tag = TermTag::Const;
  ConstantValue = C;
  Type = Ty;
}

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


Preserves::Preserves() {
  Tag = TermTag::Preserves;
  Ann = "";
}

void Preserves::dumpPretty() {
  llvm::outs() << "Preserves ";
  llvm::outs() << Ann;
}

void ConstTerm::dumpPretty() { llvm::outs() << ConstantValue; }

VarTerm::VarTerm() { Tag = TermTag::Var; }

VarTerm::VarTerm(std::string Name, Term *Ty) {
  Tag = TermTag::Var;
  Type = Ty;
  VarName = Name;
}

Name::Name() { Tag = TermTag::Name; }

Name::Name(std::string Name, Term *Ty) {
  Tag = TermTag::Name;
  Type = Ty;
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
void FStarSeqSeqType::setName(std::string Name) {NamedValue = Name; }
void FStarPointerType::setName(std::string Name) { NamedValue = Name; }


void FStarArrType::dumpPretty() {
  llvm::outs() << "array ";
  ElementType->dumpPretty();
}

void FStarSeqSeqType::dumpPretty() {
  llvm::outs() << "Seq.seq "; 
  ElementType->dumpPretty();
}

std::string FStarArrType::print(){
  std::string Out = "";
  Out += "(array ";
  Out += ElementType->print();
  Out += ")";
  return Out;
}

std::string FStarSeqSeqType::print(){
  std::string Out = "";
  Out += "(Seq.seq ";
  Out += ElementType->print();
  Out += ")";
  return Out;
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

SourceInfo PulseStmt::getCSourceInfo(){
  return CInfo;
}

FStarArrType::FStarArrType() { Tag = TermTag::FStarArrType; }

FStarPointerType::FStarPointerType() { Tag = TermTag::FStarPointerType; }
FStarPointerType::FStarPointerType(FStarType *RefTo) {
  Tag = TermTag::FStarPointerType;
  PointerTo = RefTo;
}

FStarSeqSeqType::FStarSeqSeqType() {Tag = TermTag::FStarSeqSeqType; }
FStarSeqSeqType::FStarSeqSeqType(FStarType *BaseTy) {
  Tag = TermTag::FStarSeqSeqType;
  ElementType = BaseTy;
}

// FStarSeqSeqType::FStarSeqSeqType(FStarArrType *FromArrTy){
//   Tag = TermTag::FStarSeqSeqType;
  
//   auto *ElemTy = FromArrTy->ElementType;
//   auto ElemArrTy = clang::dyn_cast<FStarArrType>(ElemTy);
  
//   FStarType *NewElemTy;
//   if (ElemArrTy){
//     NewElemTy = new FStarSeqSeqType(ElemArrTy);
//   }
//   else{
//     NewElemTy = ElemTy;
//   } 
  
//   ElementType = NewElemTy;
// }

void FStarArrType::setElementTy(FStarType *Type) { ElementType = Type; }
void FStarPointerType::setPointerToTy(FStarType *Type) { PointerTo = Type; }

AppE::AppE() {
  Tag = TermTag::AppE;
  Args.clear();
}



AppE::AppE(std::string CallName, Term *Ty) {
  Tag = TermTag::AppE;
  Type = Ty;
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

IfExpr::IfExpr(Term *C, Term *T, Term *F, Term *Ty) {
  Tag = TermTag::IfExpr;
  Type = Ty;
  Cond = C;
  TrueExpr = T;
  FalseExpr = F;
}

void IfExpr::dumpPretty(){
  llvm::outs() << "if (";
  Cond->dumpPretty(); 
  llvm::outs() << "then";
  TrueExpr->dumpPretty();
  llvm::outs() << "else";
  FalseExpr->dumpPretty();
}




void PulseStmt::setTag(PulseStmtTag T) { Tag = T; }

void PulseStmt::printTag() { llvm::outs() << Tag << "\n"; }

void PulseStmt::dumpPretty() { PulseStmt::printTag(); }

void PulseExpr::dumpPretty() {
  if (E)
    E->dumpPretty();
}

PulseExpr::PulseExpr() { Tag = PulseStmtTag::Expr; }

PulseExpr::PulseExpr(Term *Exp){
  Tag = PulseStmtTag::Expr;
  E = Exp;
}

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

//Vidush: As per tiny C doc all let bindings are let mut
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

FStarType *PulseDecl::getDeclType() { return DeclType; }

void PulseDecl::setDeclType(FStarType *Ty) { DeclType = Ty; }

SourceInfo PulseDecl::getCSourceInfo(){
  return CInfo;
}

PulseFnDefn::PulseFnDefn(_PulseFnDefn *Defn) : Defn(Defn) {
  Kind = PulseDeclKind::FnDefn;
}

PulseFnDecl::PulseFnDecl(_PulseFnDefn *Defn) : Defn(Defn) {
  Kind = PulseDeclKind::FnDecl;
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

std::string getPulseTyAsString(Term *Type) {

  if (Type == nullptr) {
    emitError("Encountered a nullptr!");
  }

  if (auto *Ty = clang::dyn_cast<FStarType>(Type)) {
    llvm::outs() << "Found FStarType!\n";
    llvm::outs() << Ty->print() << "\n";
    return Ty->print();
  } else if (auto *Ty = clang::dyn_cast<FStarArrType>(Type)) {
    llvm::outs() << "Found FStarArrType!\n";
    return Ty->print();
  }
  else if (auto *Ty = clang::dyn_cast<FStarSeqSeqType>(Type)) {
    llvm::outs() << "Found FStarSeqSeqType!\n";
    return Ty->print();
  }
  else if (auto *Ty = clang::dyn_cast<FStarPointerType>(Type)) {
    llvm::outs() << "Found FStarPointerType!\n";
    return Ty->print();
  } else {
    emitError("Did not expect type!");
  }
}

Term *CopyPulseTy(Term *ToCopy) {

  if (ToCopy == nullptr) {
    emitError("Encountered a nullptr!");
  }

  if (auto *Ty = clang::dyn_cast<FStarType>(ToCopy)) {

    auto *NewTy = new FStarType(Ty->NamedValue);
    NewTy->CInfo = Ty->CInfo;
    // What about the type of the PulseType?
    // Vidush: TODO: find out the type of the pulse type.
    return NewTy;
  } else if (auto *Ty = clang::dyn_cast<FStarArrType>(ToCopy)) {
    auto *NewTy = new FStarArrType();
    auto *TermElemTy = CopyPulseTy(Ty->ElementType);
    if (auto *ETy = clang::dyn_cast<FStarType>(TermElemTy)) {
      NewTy->ElementType = ETy;
    } else {
      emitError("Could not cast element type!");
    }
    NewTy->CInfo = Ty->CInfo;
    return NewTy;
  } else if (auto *Ty = clang::dyn_cast<FStarPointerType>(ToCopy)) {
    auto *NewTy = new FStarPointerType();
    NewTy->setName(Ty->NamedValue);
    auto *TermPointerTo = CopyPulseTy(Ty->PointerTo);

    if (auto *NewPointerTo = clang::dyn_cast<FStarType>(TermPointerTo)) {
      NewTy->PointerTo = NewPointerTo;
    } else {
      emitError("Could not cast pointer to type!");
    }
    NewTy->isBoxed = Ty->isBoxed;
    NewTy->CInfo = Ty->CInfo;
    NewTy->Type = Ty->Type;
    return NewTy;
  } else {
    emitError("Did not expect type!");
  }
}

FStarType *lookupPulseTyEnv(Term *T, std::map<Term *, FStarType *> Env) {

  auto It = Env.find(T);

  if (It == Env.end()) {
    emitError(
        "Trying to access key that does not exist is the type environment!\n");
  }

  return It->second;
}

void insertPulseTyEnv(Term *T, FStarType *Ty,
                      std::map<Term *, FStarType *> &Env) {

  auto It = Env.find(T);
  if (It == Env.end()) {
    Env.insert(std::make_pair(T, Ty));
    return;
  }

  Env[T] = Ty;
  llvm::outs() << "Warning updating type of pulse term in type environment!\n";
  return;
}

FStarType *findVarTyPulseTyEnv(std::string VarName,
                               std::map<Term *, FStarType *> Env) {

  llvm::outs() << "Trying to access variable: " << VarName << "\n";
  llvm::outs() << "Printing contents of the Env!" << "\n";
  printVEnv(Env);
  for (auto It = Env.begin(); It != Env.end(); It++) {
    auto *T = It->first;
    if (auto *VT = clang::dyn_cast<VarTerm>(T)) {
      if (VT->VarName == VarName) {
        return It->second;
      }
    }
  }

  printVEnv(Env);
  emitError("Variable Name not found in pulse ty environment: " + VarName);
}

bool checkVarNameExistsInEnv(std::string VarName,
                               std::map<Term *, FStarType *> Env) {

  llvm::outs() << "Trying to access variable: " << VarName << "\n";
  llvm::outs() << "Printing contents of the Env!" << "\n";
  printVEnv(Env);
  for (auto It = Env.begin(); It != Env.end(); It++) {
    auto *T = It->first;
    if (auto *VT = clang::dyn_cast<VarTerm>(T)) {
      if (VT->VarName == VarName) {
        return true;
      }
    }
  }

  printVEnv(Env);
  return false;
}

std::set<std::string> toSetVEnv(std::map<Term *, FStarType *> Env) {

  std::set<std::string> SetEnv;
  for (auto It = Env.begin(); It != Env.end(); It++) {
    auto *T = It->first;
    if (auto *VT = clang::dyn_cast<VarTerm>(T)) {
      SetEnv.insert(VT->VarName);
    }
  }

  return SetEnv;
}

PulseDecl *lookupDecl(clang::Decl *D,
                      std::map<clang::Decl *, PulseDecl *> DeclEnv) {

  auto It = DeclEnv.find(D);
  if (It != DeclEnv.end()) {
    return It->second;
  }

  D->dump();
  emitError("Could not find pulse function for requested C declaration!\n");
}

std::string getDeclName(PulseDecl *D){

  if (auto *FuncDef = clang::dyn_cast<PulseFnDefn>(D)){
    return FuncDef->Defn->Name;
  }
  else if (auto *FuncDef = clang::dyn_cast<PulseFnDecl>(D)){
    return FuncDef->Defn->Name;
  }
  else {
    emitError("Did not expect declaration!\n");
  }
}

void printVEnv(std::map<Term *, FStarType *> Env) {

  if (Env.empty()) {
    llvm::outs() << "Variable Env is empty!\n";
  }

  llvm::outs() << "Printing the variable Env!\n";
  for (auto It = Env.begin(); It != Env.end(); It++) {
    auto *T = It->first;
    auto *Ty = It->second;

    llvm::outs() << "\n";
    llvm::outs() << "Entry: " << "\n";
    T->dumpPretty();
    llvm::outs() << " : ";
    Ty->dumpPretty();
    llvm::outs() << "\n\n";
  }
}

// FstarValDecl::FstarValDecl(){
//   Tag = FStarDeclTag::ValDecl;
// }