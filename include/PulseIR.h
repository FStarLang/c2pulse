#pragma once
#include "clang/AST/ASTContext.h"
#include "clang/AST/OperationKinds.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include <regex>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <utility>



// // Define What all kinds of Annotations are there in Pulse.
// //TODO: These can be term type in IR, we shoudl refactor these
enum class PulseAnnKind { Requires, Ensures, Returns, IsArray, Invariants, LemmaStatement};

// Struct for Pulse annotations,
struct PulseAnnotation {
  PulseAnnKind kind; 
};


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
};

SymbolTable getSymbolKeyForCType(clang::QualType Ty, clang::ASTContext &Ctx);
const char* getSymbolKeyForOperator(SymbolTable Val, clang::BinaryOperatorKind &Op);
const char* lookupSymbol(SymbolTable Key);

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
};

// Define F* IR Similar to type term
// https://github.com/FStarLang/FStar/blob/3ff998c60bb0efe9925fc94e8fb8b785b9485af0/src/parser/FStarC.Parser.AST.fsti#L40
enum class TermTag {Const, Paren, Var, Name, AppE, FStarType, FStarPointerType, FStarArrType, 
                    Ensures, 
                    Requires,
                    Returns,
                    Lemma, 
                    LemmaStatement};
                    

class Term {
public:
  TermTag Tag;
  void setTag(TermTag T);
  void printTag();
  virtual void dumpPretty() = 0;
  virtual ~Term() = default;
};


class Lemma : public Term{
  public:
    Lemma();
    std::vector<std::string> lemmas;
    virtual ~Lemma() = default;
    virtual void dumpPretty() override; 
    static bool classof(const Term *T) { return T->Tag == TermTag::Lemma; }

};

class LemmaStatement : public Term{
  public:
    LemmaStatement();
    std::string Lemma;
    virtual ~LemmaStatement() = default;
    virtual void dumpPretty() override; 
    static bool classof(const Term *T) { return T->Tag == TermTag::LemmaStatement; }

};


class Paren : public Term {

  public: 
    Paren(); 
    Term *InnerExpr;
    void setInnerExpr(Term *Inner);
    virtual void dumpPretty() override;
    virtual ~Paren() = default;
    static bool classof(const Term *T) { return T->Tag == TermTag::Paren; }

};


class Ensures : public Term {
  public: 
    std::string Ann;
    Ensures();
    virtual void dumpPretty() override;
    static bool classof(const Term *T) { return T->Tag == TermTag::Ensures; }

};

class Requires : public Term {
  public: 
    std::string Ann;
    Requires();
    virtual void dumpPretty() override;
    static bool classof(const Term *T) { return T->Tag == TermTag::Requires; }

};

class Returns : public Term {
  public: 
    std::string Ann; 
    Returns(); 
    virtual void dumpPretty() override;
    static bool classof(const Term *T){ return T->Tag == TermTag::Returns; }
};

class ConstTerm : public Term {
public:
  ConstTerm();
  std::string ConstantValue;
  SymbolTable Symbol;
  virtual ~ConstTerm() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Const; }
};

class VarTerm : public Term {
public:
  std::string VarName;
  VarTerm();
  void setVarName(std::string Name);
  virtual ~VarTerm() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Var; }
};

class Name : public Term {
public:
  Name(); 
  Name(std::string Name);
  std::string NamedValue;
  virtual void setName(std::string Name);
  virtual ~Name() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::Name; }
};

class FStarType : public Name {
public:
  FStarType();
  FStarType(std::string Name);
  virtual void setName(std::string Name) override;
  virtual ~FStarType() = default;
  virtual void dumpPretty() override;
  static bool classof(const Term *T) { return T->Tag == TermTag::FStarType; }
};


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

class FStarPointerType : public FStarType {
public:
  FStarPointerType();
  FStarType *PointerTo;
  virtual void setName(std::string Name) override;
  virtual ~FStarPointerType() = default;
  virtual void dumpPretty() override;
  void setPointerToTy(FStarType *Ty);
  static bool classof(const Term *T) {
    return T->Tag == TermTag::FStarPointerType;
  }
};



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

// Define pattern class

// class Pattern { };

// class PatWild : public Pattern {
// };

// class PatVar

// Define Pulse IR

typedef Term Slprop;

// enum class MutOrRef {
//     Mut,
//     Ref
// };

enum class PulseStmtTag {
  Expr,
  Assignment,
  ArrayAssignment,
  LetBinding,
  If,
  WhileStmt,
  Sequence
};


enum class MutOrRef {
  NOTMUT,
  MUT, 
  REF,
};

class PulseStmt {
public:
  PulseStmtTag Tag;
  void setTag(PulseStmtTag T);
  void printTag();
  virtual void dumpPretty() = 0;
  virtual ~PulseStmt() = default;
};

class PulseExpr : public PulseStmt {
public:
  Term *E;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::Expr;
  }
};

class PulseAssignment : public PulseStmt {
public:
  Term *Lhs;
  Term *Value;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::Assignment;
  }
};

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

class PulseIf : public PulseStmt {
public:
  Term *Head;
  PulseStmt *Then;
  PulseStmt *Else = nullptr;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) { return S->Tag == PulseStmtTag::If; }

  // virtual void dumpPretty() override = 0;
};

class PulseWhileStmt : public PulseStmt {

public:
  PulseStmt *Guard;
  std::vector<Slprop *> Invariant;
  PulseStmt *Body;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::WhileStmt;
  }
  // virtual void dumpPretty() override = 0;
};

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

// Function declaration in Pulse IR
struct Binder {
public:
  Binder(std::string ident, Term *type) : Ident(std::move(ident)), Type(type) {}
  std::string Ident;
  Term *Type;
};

struct _PulseFnDecl {
  std::string Name;
  std::vector<Binder *> Args;
};

typedef std::vector<Term*> Attributes;

struct _PulseFnDefn {
  Attributes Attr;
  std::string Name;
  std::vector<Binder *> Args;
  std::vector<Term*> Annotation;
  bool isRecursive;
  PulseStmt *Body;
};

struct RecordElement {
  std::string Ident; 
  Attributes Attrs;
  Term *ElementTerm;
};

enum class TyConTag {Base, TyConAbstract, TyConAbbrev, TyConRecord, TyConVariant};

class TyCon {
  public: 
    TyCon();
    TyConTag Tag;
    std::string Ident; 
    std::vector<Binder*> Binders;

};

class TyConRecord : public TyCon {
  public:
    TyConRecord();
    Attributes Attrs;
    std::vector<RecordElement*> RecordFields;
    static bool classof(const TyCon *T) {
     return T->Tag == TyConTag::TyConRecord;
    }
};

enum class PulseDeclKind {
  FnDefn, // Function definition
  FnDecl,  // Function declaration
  ValDecl,  // Value declaration
  TyconDecl, //TyCon declaration
  TopLevelLet // Top level let binding
};

class PulseDecl {
public:
  PulseDeclKind Kind;
  PulseDeclKind getKind();
};

class TopLevelLet : public PulseDecl {
public:
  TopLevelLet();
  std::string Ident; 
  std::string Lhs;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::TopLevelLet;
  }
};

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

class ValDecl : public PulseDecl {
  public:
    ValDecl();
    std::string Ident; 
    Term *ValTerm;
    static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::ValDecl;
  }
};

class PulseFnDefn : public PulseDecl {
public:
  PulseFnDefn(_PulseFnDefn *Defn);
  _PulseFnDefn *Defn;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::FnDefn;
  }

  void dumpPretty();
};

class PulseFnDecl : public PulseDecl {
public:
  _PulseFnDecl *Defn;
  static bool classof(const PulseDecl *D) {
    return D->Kind == PulseDeclKind::FnDecl;
  }
};

class PulseModul {
public:
  bool includePulsePrelude;
  std::vector<std::string> IncludedModules;
  std::string ModuleName;
  std::vector<PulseDecl *> Decls;
  //set to true for .fsti files
  bool isHeader = false;
};

// class File : 
//Moduel == file in clang
typedef PulseModul File;




// enum class FStarDeclTag {BaseDecl, ValDecl};

// class FStarDecl {
//   public: 
//     FStarDeclTag Tag; 
// };

// class FstarValDecl : public FStarDecl {
//   public:
//     FstarValDecl();
//     std::string Ident; 
//     Term *ValTerm;
//     static bool classof(const FStarDecl *D) {
//     return D->Tag == FStarDeclTag::ValDecl;
//   }
// };

// PulseAnnKind getPulseAnnKindFromString(llvm::StringRef Data, std::wsmatch &match);

PulseAnnKind getPulseAnnKindFromString(llvm::StringRef Data, std::string &match);