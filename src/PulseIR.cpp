#include "PulseIR.h"
#include "clang/AST/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include <cstddef>

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

const char* lookupSymbol(SymbolTable Key){
  return SymbolToStringTable.lookup(Key);
}

PulseAnnKind getPulseAnnKindFromString(llvm::StringRef Data, std::string &match){

    // std::wregex requires_pattern(L"(requires:(.*))");
    // std::wregex ensures_pattern(L"(ensures:(.*))");
    // std::wregex isarray_pattern(L"(array:(.*):(.*))");
    // std::wregex invariants_pattern(L"(invariants:(.*))");
    // std::wregex lemma_pattern(L"(lemma:(.*))");

    std::regex requires_pattern(R"(requires:([\w\*\.\|\-]+))");
    std::regex ensures_pattern(u8R"(ensures:(.*))");
    std::regex isarray_pattern(u8R"(array:(.*):(.*))");
    std::regex invariants_pattern(u8R"(invariants:(.*))");
    std::regex lemma_pattern(u8R"(lemma:(.*))");

    llvm::outs() << "Print StringRef!!!!!!\n";
    llvm::outs() << Data.str() << "\n";
    llvm::outs() << "End of StringRef.\n";
    std::smatch match2;
    if (!Data.empty()){
          
          std::string cleanedString;
          for (auto c : Data.trim().bytes()){
                if (c != '\r') {  // Keep only printable ASCII characters
                    cleanedString += c;
                }
          }
          // std::wstring DataStr = std::wstring(cleanedString.begin(), cleanedString.end());
           
          //std::string DataStr = Data.trim().str();

    if (std::regex_search(cleanedString, match2, requires_pattern)) {
        // auto NewRequires = new Requires(); 
        // NewRequires->Ann = match[1];
        // Anns.push_back(NewRequires);
        std::string delimiter = "requires:";
        size_t pos = cleanedString.find(delimiter);
        if (pos != std::string::npos) {
          std::string firstPart = cleanedString.substr(0, pos);// Before "requires:"
          match = cleanedString.substr(pos + delimiter.length()); // After "requires:"
          //std::cout << "First Part: " << firstPart << std::endl;
          //std::cout << "Second Part: " << secondPart << std::endl;
        }


        return PulseAnnKind::Requires;
        
    }
    else if (std::regex_search(cleanedString, match2, ensures_pattern)) {
        // auto NewEnsures = new Ensures(); 
        // NewEnsures->Ann = match[1];
        // Anns.push_back(NewEnsures);

        std::string delimiter = "ensures:";
        size_t pos = cleanedString.find(delimiter);
        if (pos != std::string::npos) {
          std::string firstPart = cleanedString.substr(0, pos);// Before "requires:"
          match = cleanedString.substr(pos + delimiter.length()); // After "requires:"
          //std::cout << "First Part: " << firstPart << std::endl;
          //std::cout << "Second Part: " << secondPart << std::endl;
        }
        return PulseAnnKind::Ensures;
    }
    else if (std::regex_search(cleanedString, match2, isarray_pattern)) {
      return PulseAnnKind::IsArray;
    }
    else if (std::regex_search(cleanedString, match2, invariants_pattern)) {
        return PulseAnnKind::Invariants;
    }
    else if (std::regex_search(cleanedString, match2, lemma_pattern)) {

       std::string delimiter = "lemma:";
        size_t pos = cleanedString.find(delimiter);
        if (pos != std::string::npos) {
          std::string firstPart = cleanedString.substr(0, pos);// Before "requires:"
          match = cleanedString.substr(pos + delimiter.length()); // After "requires:"
          //std::cout << "First Part: " << firstPart << std::endl;
          //std::cout << "Second Part: " << secondPart << std::endl;
        }

        return PulseAnnKind::LemmaStatement;
    }
    else{
          llvm::outs() << cleanedString.data() << "\n";
          assert(false && "Unhandeled pulse annotation kind!\n");
    }
  }
}

SymbolTable getSymbolKeyForCType(clang::QualType Ty, clang::ASTContext &Ctx){

  if(Ty->isSignedIntegerType()){

    if (Ctx.getTypeSize(Ty) == 8){
      return SymbolTable::Int8;
    }
    else if (Ctx.getTypeSize(Ty) == 16){
      return SymbolTable::Int16;
    }
    else if (Ctx.getTypeSize(Ty) == 32){
      return SymbolTable::Int32;
    }
    else if (Ctx.getTypeSize(Ty) == 64){
      return SymbolTable::Int64;
    }
    else {
      assert(false && "getSymbolKeyForType: did not expect signed integer size");
    }

  }
  else if (Ty->isUnsignedIntegerType()){
    
    //check explicitly if it is size_t
    if (Ty.getAsString() == "size_t"){
      return SymbolTable::SizeT;
    }
    
    if (Ctx.getTypeSize(Ty) == 8){
      return SymbolTable::UInt8;
    }
    else if (Ctx.getTypeSize(Ty) == 16){
      return SymbolTable::UInt16;
    }
    else if (Ctx.getTypeSize(Ty) == 32){
      return SymbolTable::UInt32;
    }
    else if (Ctx.getTypeSize(Ty) == 64){
      return SymbolTable::UInt64;
    }
    else if (Ctx.getTypeSize(Ty) == 128){
      return SymbolTable::UInt128;
    }
    else {
      assert(false && "getSymbolKeyForType: did not expect signed integer size");
    }

  }
  else if (Ty.getAsString() == "size_t"){
    return SymbolTable::SizeT;
  }

  assert(false && "getSymbolKeyForType: did not expect type.");

}

const char* getSymbolKeyForOperator(SymbolTable Val, clang::BinaryOperatorKind &Op){
  
  switch(Op){
  case clang::BO_PtrMemD:
  case clang::BO_PtrMemI:
  case clang::BO_Mul:{
    if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Mul);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Mul);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Mul);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Mul);
    }
    
    break;
  }
  case clang::BO_Div:{
     if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Div);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Div);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Div);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Div);
    }
    else if (Val == SymbolTable::SizeT){
      return lookupSymbol(SymbolTable::SizeT_Div);
    }
    else if (Val == SymbolTable::UInt64){
      return lookupSymbol(SymbolTable::UInt64_Div);
    }
    else{
      assert(false && "unimplemented case.\n");
    }
    break;
  }
  case clang::BO_Rem:
  case clang::BO_Add:{
    if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Add);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Add);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Add);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Add);
    }
    else if (Val == SymbolTable::UInt64){
      return lookupSymbol(SymbolTable::UInt64_Add);
    }
    else if (Val == SymbolTable::SizeT){
      return lookupSymbol(SymbolTable::SizeT_Add);
    }
    else{
      assert(false && "Did not expect case.");
    }
    
    break;
  }
  case clang::BO_Sub:{
    if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Sub);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Sub);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Sub);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Sub);
    }
    else if (Val == SymbolTable::UInt64){
      return lookupSymbol(SymbolTable::UInt64_Sub);
    }
    else if (Val == SymbolTable::SizeT){
      return lookupSymbol(SymbolTable::SizeT_Sub);
    }
    
    break;
  }
  case clang::BO_Shl:
  case clang::BO_Shr:
  case clang::BO_Cmp:
  case clang::BO_LT:{
     if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Lt);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Lt);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Lt);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Lt);
    }
    else if (Val == SymbolTable::SizeT){
      return lookupSymbol(SymbolTable::SizeT_Lt);
    }
    else if (Val == SymbolTable::UInt64){
      return lookupSymbol(SymbolTable::UInt64_Lt);
    }
    else{
      assert(false && "unimplemented case.\n");
    }
    break;
  }
  case clang::BO_GT:
  case clang::BO_LE:
  case clang::BO_GE:
  case clang::BO_EQ:{
    if (Val == SymbolTable::Int8){
      return lookupSymbol(SymbolTable::Int8_Eq);
    }
    else if (Val == SymbolTable::Int16){
      return lookupSymbol(SymbolTable::Int16_Eq);
    }
    else if (Val == SymbolTable::Int32){
      return lookupSymbol(SymbolTable::Int32_Eq);
    }
    else if (Val == SymbolTable::Int64){
      return lookupSymbol(SymbolTable::Int64_Eq);
    }
    
    break;
  }
  case clang::BO_NE:
  case clang::BO_And:
  case clang::BO_Xor:
  case clang::BO_Or:
  case clang::BO_LAnd:
  case clang::BO_LOr:
  case clang::BO_Assign:
  case clang::BO_MulAssign:
  case clang::BO_DivAssign:
  case clang::BO_RemAssign:
  case clang::BO_AddAssign:
  case clang::BO_SubAssign:
  case clang::BO_ShlAssign:
  case clang::BO_ShrAssign:
  case clang::BO_AndAssign:
  case clang::BO_XorAssign:
  case clang::BO_OrAssign:
  case clang::BO_Comma:
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

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, PulseFnKind T) {
  switch (T) {
  case PulseFnKind::FnDecl:
    os << "FnDecl";
    break;
  case PulseFnKind::FnDefn:
    os << "FnDefn";
    break;
  }
  return os;
}

void Term::setTag(TermTag T) { Tag = T; }

void Term::printTag() { llvm::outs() << Tag << "\n"; }

void Term::dumpPretty() { printTag(); }


void Lemma::dumpPretty() {
  for (auto lemma : lemmas){
    llvm::outs() << lemma << "\n";
  }
}

void LemmaStatement::dumpPretty() {
  llvm::outs() << Lemma << "\n";
}

Lemma::Lemma(){
  Tag = TermTag::Lemma;
}

LemmaStatement::LemmaStatement(){
  Tag = TermTag::LemmaStatement;
}


Paren::Paren(){
  llvm::outs() << "Called Paren Constructor!!" << "\n";
  Tag = TermTag::Paren;
}

void Paren::setInnerExpr(Term *Inner){
  InnerExpr = Inner;
}

void Paren :: dumpPretty() {
  llvm::outs() << "("; 
  InnerExpr->dumpPretty(); 
  llvm::outs() << ")";
}

ConstTerm::ConstTerm(){
  Tag = TermTag::Const;
}

Ensures::Ensures(){
  Tag = TermTag::Ensures;
}

void Ensures::dumpPretty(){
  llvm::outs() << "Ensures ";
  llvm::outs() << Ann;
}

void Requires::dumpPretty(){
  llvm::outs() << "Requires ";
  llvm::outs() << Ann;
}

Requires::Requires(){
  Tag = TermTag::Requires;
}

void ConstTerm::dumpPretty() { llvm::outs() << ConstantValue; }

VarTerm::VarTerm(){
  Tag = TermTag::Var;
}

Name::Name(){
  Tag = TermTag::Name;
}

void VarTerm::setVarName(std::string Name) { VarName = Name; }

void VarTerm::dumpPretty() { llvm::outs() << VarName; }

void Name::setName(std::string Name) { NamedValue = Name; }

void Name::dumpPretty() { llvm::outs() << NamedValue; }

void FStarType::setName(std::string Name) { NamedValue = Name; }

void FStarType::dumpPretty() { llvm::outs() << NamedValue; }

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

FStarArrType::FStarArrType(){
  Tag = TermTag::FStarArrType;
}

void FStarArrType::setElementTy(FStarType *Type) { ElementType = Type; }
void FStarPointerType::setPointerToTy(FStarType *Type) { PointerTo = Type; }

AppE::AppE(){
  Tag = TermTag::AppE;
}

void AppE::dumpPretty() {
  llvm::outs() << CallName->VarName;
  llvm::outs() << " ";
  size_t Idx = 1;
  for (auto *Arg : Args) {
    Arg->dumpPretty();
    if (Idx < Args.size())
      llvm::outs() << " ";
    Idx++;
  }
}

void AppE::setCallName(VarTerm *Call) { CallName = Call; }

void AppE::pushArg(Term *Arg) { Args.push_back(Arg); }

void PulseStmt::setTag(PulseStmtTag T) { Tag = T; }

void PulseStmt::printTag() { llvm::outs() << Tag << "\n"; }

void PulseStmt::dumpPretty() { PulseStmt::printTag(); }

void PulseExpr::dumpPretty() { 
  if (E)
    E->dumpPretty(); 
}

void PulseAssignment::dumpPretty() {
  Lhs->dumpPretty();
  llvm::outs() << " := ";
  Value->dumpPretty();
  llvm::outs() << "\n";
}

void PulseArrayAssignment::dumpPretty(){

  Arr->dumpPretty(); 
  llvm::outs() << ".(";
  Index->dumpPretty(); 
  llvm::outs() << ")";
  llvm::outs() << " <- ";
  Value->dumpPretty();
  llvm::outs() << "\n";

}

LetBinding::LetBinding(){
  Tag = PulseStmtTag::LetBinding;
}

void LetBinding::dumpPretty() {
  llvm::outs() << "let ";
  if (Qualifier == MutOrRef::MUT){
    llvm::outs() << "mut ";
  } 
  llvm::outs() << VarName << " = ";
  LetInit->dumpPretty();
  llvm::outs() << "\n";
}

void PulseIf::dumpPretty(){

  
}

PulseSequence::PulseSequence(){
  Tag = PulseStmtTag::Sequence;
  S1 = nullptr; 
  S2 = nullptr;
}

void PulseSequence::assignS1(PulseStmt *S) { S1 = S; }

void PulseSequence::assignS2(PulseStmt *S) { S2 = S; }

void PulseSequence::dumpPretty() {
  if (S1 != nullptr){
    S1->dumpPretty();
  }

  if (S2 != nullptr){
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

PulseFnKind PulseDecl::getKind() { return Kind; }

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

  llvm::outs() << "\n\nPrint annotations: " << "\n\n";

  for (auto *Ann : Defn->Annotation){
    Ann->dumpPretty();
    llvm::outs() << "\n";
  }

  llvm::outs() << "\n\n";
  llvm::outs() << "Print the function body: " << "\n\n";
  Defn->Body->dumpPretty();
}