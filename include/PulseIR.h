#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/OperationKinds.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

#include <set>
#include <string>
#include <vector>
#include <utility>

/// An enum class for all the pulse annotation kinds.
enum class PulseAnnKind {
  HeapAllocated,
  ErasedArg,
  Requires,
  Ensures,
  Returns,
  IsArray,
  Invariants,
  LemmaStatement,
  Assert,
  Unknown,
  Includes,
  ExpectFailure
};

/// Struct for Pulse annotations.
struct PulseAnnotation {
  PulseAnnKind kind; 
};

/// Symbol Table to capture pulse specific types
enum class SymbolTable {
  Int8, 
  Int16, 
  Int32, 
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  UInt64_Lt,
  UInt64_Div,
  UInt64_Sub,
  UInt64_Add,
  UInt128, 
  SizeT,
  SizeT_Add,
  SizeT_Sub, 
  SizeT_Div,
  SizeT_Mul,
  SizeT_Eq,
  SizeT_Lt,
  Int8_Add,
  Int8_Sub,
  Int8_Div,
  Int8_Mul,
  Int8_Eq,
  Int8_Lt,
  Int16_Add,
  Int16_Sub,
  Int16_Div,
  Int16_Mul,
  Int16_Eq,
  Int16_Lt,
  Int32_Add,
  Int32_Sub,
  Int32_Div,
  Int32_Mul,
  Int32_Eq,
  Int32_Lt,
  Int64_Add,
  Int64_Sub,
  Int64_Div,
  Int64_Mul,
  Int64_Eq,
  Int64_Lt,
  Array,
  Ref,
  UNKNOWN,
};

SymbolTable getSymbolKeyForCType(clang::QualType Ty, clang::ASTContext &Ctx);
const char* getSymbolKeyForOperator(SymbolTable Val, clang::BinaryOperatorKind &Op);
const char* lookupSymbol(SymbolTable Key);

/// A map from symbol to its pulse specific string.
static const llvm::SmallDenseMap<SymbolTable, const char*> SymbolToStringTable {
  {SymbolTable::Int8, "Int8.t"},
  {SymbolTable::Int16, "Int16.t"},
  {SymbolTable::Int32, "Int32.t"},
  {SymbolTable::Int64, "Int64.t"},
  {SymbolTable::UInt8, "UInt8.t"},
  {SymbolTable::UInt16, "UInt16.t"},
  {SymbolTable::UInt32, "UInt32.t"},
  {SymbolTable::UInt64, "UInt64.t"},
  {SymbolTable::UInt64_Lt, "UInt64.lt"},
  {SymbolTable::UInt64_Div, "UInt64.div"},
 {SymbolTable::UInt64_Sub, "UInt64.sub"},
 {SymbolTable::UInt64_Add, "UInt64.add"},
 {SymbolTable::UInt128, "UInt128.t"},
 {SymbolTable::SizeT, "SizeT.t"},
 {SymbolTable::SizeT_Add, "SizeT.add"},
 {SymbolTable::SizeT_Sub, "SizeT.sub"},
 {SymbolTable::SizeT_Div, "SizeT.div"},
 {SymbolTable::SizeT_Mul, "SizeT.mul"},
 {SymbolTable::SizeT_Eq, "SizeT.eq"},
 {SymbolTable::SizeT_Lt, "SizeT.lt"},
 {SymbolTable::Int32_Add, "Int32.add"},
 {SymbolTable::Int32_Sub, "Int32.sub"},
 {SymbolTable::Int32_Div, "Int32.div"},
 {SymbolTable::Int32_Mul, "Int32.mul"},
 {SymbolTable::Int32_Eq, "Int32.eq"},
 {SymbolTable::Int32_Lt, "Int32.lt"},
 {SymbolTable::Int64_Mul, "Int64.mul"},
 {SymbolTable::Int64_Add, "Int64.add"},
 {SymbolTable::Int64_Sub, "Int64.sub"},
 {SymbolTable::Int64_Div, "Int64.div"},
 {SymbolTable::Int64_Eq, "Int64.eq"},
 {SymbolTable::Int64_Lt, "Int64.lt"},
 {SymbolTable::Array, "array"},
 {SymbolTable::Ref, "ref"},
};

/// Define F* IR Similar to type term
/// https://github.com/FStarLang/FStar/blob/3ff998c60bb0efe9925fc94e8fb8b785b9485af0/src/parser/FStarC.Parser.AST.fsti#L40
enum class TermTag {Const, Paren, Var, Name, AppE, FStarType, FStarPointerType, FStarArrType, 
                    Ensures, 
                    Requires,
                    Returns,
                    Lemma, 
                    LemmaStatement};

/// A base class for term.                    
class Term {
public:
  TermTag Tag;
  void setTag(TermTag T);
  void printTag();
  virtual void dumpPretty() = 0;
  virtual ~Term() = default;
};

/// Store Pulse Lemmas.
/// Store a list of pulse Lemmas.
class Lemma : public Term{
  public:
    Lemma();
    std::vector<std::string> lemmas;
    virtual ~Lemma() = default;
    virtual void dumpPretty() override; 
    static bool classof(const Term *T) { return T->Tag == TermTag::Lemma; }

};

/// Lemma statement is any proof term in the middle of a function body.
class LemmaStatement : public Term{
  public:
    LemmaStatement();
    std::string Lemma;
    virtual ~LemmaStatement() = default;
    virtual void dumpPretty() override; 
    static bool classof(const Term *T) { return T->Tag == TermTag::LemmaStatement; }

};

/// An IR node for representing a pulse parenthesis.
class Paren : public Term {
  public: 
    Paren(); 
    Term *InnerExpr;
    void setInnerExpr(Term *Inner);
    virtual void dumpPretty() override;
    virtual ~Paren() = default;
    static bool classof(const Term *T) { return T->Tag == TermTag::Paren; }
};

/// An IR node for representing a pulse ensures.
class Ensures : public Term {
  public: 
    std::string Ann;
    Ensures();
    virtual void dumpPretty() override;
    static bool classof(const Term *T) { return T->Tag == TermTag::Ensures; }
};

/// An IR node for representing a pulse requires.
class Requires : public Term {
  public: 
    std::string Ann;
    Requires();
    virtual void dumpPretty() override;
    static bool classof(const Term *T) { return T->Tag == TermTag::Requires; }

};

/// An IR node for representing a pulse return.
class Returns : public Term {
  public: 
    std::string Ann; 
    Returns(); 
    virtual void dumpPretty() override;
    static bool classof(const Term *T){ return T->Tag == TermTag::Returns; }
};

/// An IR node for representing a pulse constant term.
class ConstTerm : public Term {
public:
  ConstTerm();
  std::string ConstantValue;
  SymbolTable Symbol;
  virtual ~ConstTerm() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Const; }
};

/// An IR node for representing a pulse variable term.
class VarTerm : public Term {
public:
  std::string VarName;
  VarTerm();
  void setVarName(std::string Name);
  virtual ~VarTerm() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Var; }
};

/// An IR node for represeting a pulse name term. 
/// NOTE: This can be used as a fallback node for 
/// creating pulse terms. 
/// These can be accumulated in the NamedValue.
class Name : public Term {
public:
  Name(); 
  Name(std::string Name);
  std::string NamedValue;
  virtual void setName(std::string Name);
  virtual std::string print();
  virtual ~Name() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Name; }
};

/// An IR node for representing a type in FStar.
class FStarType : public Name {
public:
  FStarType();
  FStarType(std::string Name);
  virtual void setName(std::string Name) override;
  virtual std::string print() override;
  virtual ~FStarType() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::FStarType; }
};

/// An IR node for representing an array type in FStar.
class FStarArrType : public FStarType {

  public:
    FStarArrType();
    FStarType *ElementType;
    virtual void setName(std::string Name) override;
    virtual ~FStarArrType() = default;
    virtual void dumpPretty() override;
    void setElementTy(FStarType *Ty);
    static bool classof(const Term *T) {
      return T->Tag == TermTag::FStarArrType;
    }

};

/// An IR node for representing a pointer type in FStar.
class FStarPointerType : public FStarType {
public:
  FStarPointerType();
  //Boxed == Heap Allocated. 
  //Otherwise Stack Allocated.
  //If not box, it is ref.
  bool isBoxed = false;
  FStarType *PointerTo;
  virtual void setName(std::string Name) override;
  virtual std::string print() override;
  virtual ~FStarPointerType() = default;
  virtual void dumpPretty() override;
  void setPointerToTy(FStarType *Ty);
  static bool classof(const Term *T) {
    return T->Tag == TermTag::FStarPointerType;
  }
};

/// An IR node for representing a call in pulse.
class AppE : public Term {
public:
  VarTerm *CallName;
  std::vector<Term *> Args;
  AppE();
  virtual ~AppE() = default;
  virtual void dumpPretty() override;
  void setCallName(VarTerm *Call);
  void pushArg(Term *Arg);
  static bool classof(const Term *T) { return T->Tag == TermTag::AppE; }
};

/// Slprop in pulse can be a term.
typedef Term Slprop;

/// An enum class for a pulse statements types.
enum class PulseStmtTag {
  Expr,
  Assignment,
  ArrayAssignment,
  LetBinding,
  If,
  WhileStmt,
  Sequence, 
  GenericStmt,
};

/// An enum class for specifying if something is mutable or not.
enum class MutOrRef {
  NOTMUT,
  MUT, 
  REF,
};

/// The base class for a pulse statement.
class PulseStmt {
public:
  PulseStmtTag Tag;
  void setTag(PulseStmtTag T);
  void printTag();
  virtual void dumpPretty() = 0;
  virtual ~PulseStmt() = default;
};

/// A class for making general pulse statements. 
/// The pulse code can be accumulated as strings in the body. 
/// Use this if you don't want to create an in-memory AST but just need
/// Some fallback node.
class GenericStmt : public PulseStmt {
  public:
    GenericStmt(); 
    std::string body;
    virtual void dumpPretty() override;
    static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::GenericStmt;
  }
};

/// A class for making a pulse expression.
/// A Pulse expression stores a term inside.
class PulseExpr : public PulseStmt {
public:
  Term *E;
  PulseExpr();
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::Expr;
  }
};

/// An IR node for representing a pulse assignment statement.
class PulseAssignment : public PulseStmt {
public:
  PulseAssignment();
  Term *Lhs;
  Term *Value;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::Assignment;
  }
};

/// An IR node for representing a pulse array assignment statement.
class PulseArrayAssignment : public PulseStmt {
public:
  Term *Arr;
  Term *Index;
  Term *Value;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::ArrayAssignment;
  }
};

/// An IR node for representing a pulse let binding.
class LetBinding : public PulseStmt {
public:
  std::string VarName;
  LetBinding();
  Term *LetInit;
  MutOrRef Qualifier;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::LetBinding;
  }
};

/// An IR node for representing a pulse if statement.
class PulseIf : public PulseStmt {
public:
  Term *Head;
  PulseStmt *Then;
  PulseStmt *Else = nullptr;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) { return S->Tag == PulseStmtTag::If; }
};

/// An IR node for representing a pulse while statement.
class PulseWhileStmt : public PulseStmt {
public:
  PulseStmt *Guard;
  std::vector<Slprop *> Invariant;
  PulseStmt *Body;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::WhileStmt;
  }
};

/// An IR node for representing a pulse Sequence statment.
class PulseSequence : public PulseStmt {
public:
  PulseStmt *S1;
  PulseStmt *S2;
  PulseSequence();
  void assignS1(PulseStmt *S);
  void assignS2(PulseStmt *S);
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::Sequence;
  }
};

/// An IR node for representing a Function Argument.
struct Binder {
public:
  Binder(std::string ident, Term *type) : Ident(std::move(ident)), Type(type) {}
  Binder(std::string fallback);
  std::string Ident;
  Term *Type;
  bool useFallBack = false;
};

/// An IR node for representing a pulse function declaration.
struct _PulseFnDecl {
  std::string Name;
  std::vector<Binder *> Args;
};

/// Attributes a function may have.
typedef std::vector<Term*> Attributes;

/// An IR node for representing a pulse function definition.
struct _PulseFnDefn {
  Attributes Attr;
  std::string Name;
  std::vector<Binder *> Args;
  std::vector<Term*> Annotation;
  bool isRecursive;
  PulseStmt *Body;
  bool useFallback = false;
  std::string FallBackBody;
};

/// An IR node for representing a pulse record element type.
struct RecordElement {
  std::string Ident; 
  Attributes Attrs;
  Term *ElementTerm;
};

/// An enum class for the type of tycon.
enum class TyConTag {Base, TyConAbstract, TyConAbbrev, TyConRecord, TyConVariant};

/// A class for representing a pulse tycon.
class TyCon {
  public: 
    TyCon();
    TyConTag Tag;
    std::string Ident; 
    std::vector<Binder*> Binders;

};

/// A class for representing a pulse tycon record type.
class TyConRecord : public TyCon {
  public:
    TyConRecord();
    Attributes Attrs;
    std::vector<RecordElement*> RecordFields;
    static bool classof(const TyCon *T) {
     return T->Tag == TyConTag::TyConRecord;
    }
};

/// A pulse declaration kind.
enum class PulseDeclKind {
  FnDefn, // Function definition
  FnDecl,  // Function declaration
  ValDecl,  // Value declaration
  TyconDecl, //TyCon declaration
  TopLevelLet, // Top level let binding
  GenericDecl, //A Super Generic declaration. A fallback AST Node in the compiler.
};

/// An enum class to represent what kind of a pulse declaration it is.
class PulseDecl {
public:
  PulseDeclKind Kind;
  PulseDeclKind getKind();
};

/// An IR node to represent a generic pulse declaration. 
/// NOTE: Use this node as a fallback. 
/// We can accumulate strings in this declaration in Ident.
/// This is an escape hatch in case the programmer does not 
/// want to use the defined pulse IR.
class GenericDecl : public PulseDecl {
  public:
    GenericDecl(); 
    std::string Ident;
    static bool classof(const PulseDecl *D) {
      return D->Kind == PulseDeclKind::GenericDecl;
    }

};

/// An IR node to represent a top level let bind.
class TopLevelLet : public PulseDecl {
public:
  TopLevelLet();
  std::string Ident; 
  std::string Lhs;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::TopLevelLet;
  }
};

/// An IR node to represent a top level declaration.
class TyConDecl : public PulseDecl {
  public: 
    TyConDecl(); 
    bool Effect; 
    bool TypeClass; 
    std::vector<TyCon*> TyCons;
    static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::TyconDecl;
  }
};

/// An IR node to represent a value declaration 
class ValDecl : public PulseDecl {
  public:
    ValDecl();
    std::string Ident; 
    Term *ValTerm;
    static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::ValDecl;
  }
};

/// An IR node to represent a pulse function definition.
class PulseFnDefn : public PulseDecl {
public:
  PulseFnDefn(_PulseFnDefn *Defn);
  _PulseFnDefn *Defn;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::FnDefn;
  }

  void dumpPretty();
};

/// An IR node to represent a pulse function declaration.
class PulseFnDecl : public PulseDecl {
public:
  _PulseFnDecl *Defn;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::FnDecl;
  }
};

/// An IR node to represent a pulse module.
class PulseModul {
public:
  bool includePulsePrelude;
  std::vector<std::string> IncludedModules;
  std::string ModuleName;
  std::vector<PulseDecl *> Decls;
  /// set to true for .fsti files
  bool isHeader = false;
  void insertModule(std::string IncModule);
};

/// Class File:
/// We treat a Module as equivalent to a source file.
/// In Clang, a Translation Unit represents a single compilable file—
/// that is, the source file along with all its includes and other necessary
/// context required to compile it.
typedef PulseModul File;

/// A function to get the kind of pulse annotation from a string.
PulseAnnKind getPulseAnnKindFromString(llvm::StringRef Data, std::string &match);