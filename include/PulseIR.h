#pragma once

#include "ExprLocationAnalyzer.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

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
  UInt128,
  SizeT,
  Bool,
  Int,

  // size t
  SizeT_Add,
  SizeT_Sub,
  SizeT_Div,
  SizeT_Rem,
  SizeT_Mul,

  SizeT_Eq,
  SizeT_Eq_Int32, 

  SizeT_Lt,
  SizeT_Le,
  SizeT_Gt,
  SizeT_Ge,
  SizeT_Ne,

  // int 8
  Int8_Add,
  Int8_Sub,
  Int8_Div,
  Int8_Rem,
  Int8_Mul,
  Int8_Eq,
  Int8_Lt,
  Int8_Le,
  Int8_Gt,
  Int8_Ge,
  Int8_Ne,

  // U int 8
  UInt8_Add,
  UInt8_Sub,
  UInt8_Div,
  UInt8_Rem,
  UInt8_Mul,
  UInt8_Eq,
  UInt8_Lt,
  UInt8_Le,
  UInt8_Gt,
  UInt8_Ge,
  UInt8_Ne,

  // int 16
  Int16_Add,
  Int16_Sub,
  Int16_Div,
  Int16_Rem,
  Int16_Mul,
  Int16_Eq,
  Int16_Lt,
  Int16_Le,
  Int16_Gt,
  Int16_Ge,
  Int16_Ne,

  // uint 16
  UInt16_Add,
  UInt16_Sub,
  UInt16_Div,
  UInt16_Rem,
  UInt16_Mul,
  UInt16_Eq,
  UInt16_Lt,
  UInt16_Le,
  UInt16_Gt,
  UInt16_Ge,
  UInt16_Ne,

  // int 32
  Int32_Add,
  Int32_Sub,
  Int32_Div,
  Int32_Rem,
  Int32_Mul,
  Int32_Eq,
  Int32_Lt,
  Int32_Le,
  Int32_Gt,
  Int32_Ge,
  Int32_Ne,

  // uint 32
  UInt32_Add,
  UInt32_Sub,
  UInt32_Div,
  UInt32_Rem,
  UInt32_Mul,
  UInt32_Eq,
  UInt32_Lt,
  UInt32_Le,
  UInt32_Gt,
  UInt32_Ge,
  UInt32_Ne,

  // int 64
  Int64_Add,
  Int64_Sub,
  Int64_Div,
  Int64_Rem,
  Int64_Mul,
  Int64_Eq,
  Int64_Lt,
  Int64_Le,
  Int64_Gt,
  Int64_Ge,
  Int64_Ne,

  // uint 64
  UInt64_Add,
  UInt64_Sub,
  UInt64_Div,
  UInt64_Rem,
  UInt64_Mul,
  UInt64_Eq,
  UInt64_Lt,
  UInt64_Le,
  UInt64_Gt,
  UInt64_Ge,
  UInt64_Ne,

  // int
  Int_Add,
  Int_Sub,
  Int_Div,
  Int_Rem,
  Int_Mul,
  Int_Eq,
  Int_Lt,
  Int_Le,
  Int_Gt,
  Int_Ge,
  Int_Ne,

  Array,
  Ref,
  UNKNOWN,
};

SymbolTable getSymbolKeyForCType(clang::QualType Ty, clang::ASTContext &Ctx);
const char* getSymbolKeyForOperator(SymbolTable Val, SymbolTable RetTy, clang::BinaryOperatorKind &Op);
const char* lookupSymbol(SymbolTable Key);

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

     //int 8
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
        
         //uint 8
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

        //int 16
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
        
         //uint 16
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


         //int 32
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
        
         //uint 32
    {SymbolTable::UInt32_Add, "UInt32.add"},
    {SymbolTable::UInt32_Sub, "UInt32.sub"},
    {SymbolTable::UInt32_Div, "UInt32.div"},
    {SymbolTable::UInt32_Rem, "UInt32.rem"},
    {SymbolTable::UInt32_Mul, "UInt32.mul"},
    {SymbolTable::UInt32_Eq, "UInt32.eq"},
    {SymbolTable::UInt32_Lt, "UInt32.lt"},
    {SymbolTable::UInt32_Le, "UInt32.lte"},
    {SymbolTable::UInt32_Gt, "UInt32.gt"},
    {SymbolTable::UInt32_Ge, "UInt32.gte"},

    //int 64
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

         //U int 64  
    {SymbolTable::UInt64_Mul, "UInt64.mul"},
    {SymbolTable::UInt64_Add, "UInt64.add"},
    {SymbolTable::UInt64_Sub, "UInt64.sub"},
    {SymbolTable::UInt64_Div, "UInt64.div"},
    {SymbolTable::UInt64_Rem, "UInt64.rem"},
    {SymbolTable::UInt64_Eq, "UInt64.eq"},
    {SymbolTable::UInt64_Lt, "UInt64.lt"},
    {SymbolTable::UInt64_Le, "UInt64.lte"},
    {SymbolTable::UInt64_Gt, "UInt64.gt"},
    {SymbolTable::UInt64_Ge, "UInt64.gte"},


    ///VIDUSH:
    /// Right now Int32.t and int are the same 
    /// In case we want these to be different...
    /// adding stuff here for now.
    {SymbolTable::Int_Add, "add"},
    {SymbolTable::Int_Sub, "sub"},
    {SymbolTable::Int_Div, "div"},
    {SymbolTable::Int_Rem, "rem"},
    {SymbolTable::Int_Mul, "mul"},
    {SymbolTable::Int_Eq, "eq"},
    {SymbolTable::Int_Lt, "lt"},
    {SymbolTable::Int_Le, "lte"},
    {SymbolTable::Int_Gt, "gt"},
    {SymbolTable::Int_Ge, "gte"},

    {SymbolTable::Array, "array"},
    {SymbolTable::Ref, "ref"},
};

/// Classes to Store Location Information for AST Nodes.
// class RegionRange {
//   public:
//   //CLASS DYN_CAST TO A RANGE OR A SINGLE LOC
//   SourceInfo Start;
//   //SourceInfo End;
//   void setStartLine(unsigned Line);
//   void setEndLine(unsigned Line);
//   void setStartColumn(unsigned Col);
//   void setEndColumn(unsigned Col);

// };

class RegionMapping {
  public:
  //use SourceInfo.
    SourceInfo CInfo;
    SourceInfo PulseInfo;

    SourceInfo &getCInfo();
    SourceInfo &getPulseInfo();
};

/// Define F* IR Similar to type term
/// https://github.com/FStarLang/FStar/blob/3ff998c60bb0efe9925fc94e8fb8b785b9485af0/src/parser/FStarC.Parser.AST.fsti#L40
enum class TermTag {Const, Paren, Var, Name, AppE, FStarType, FStarPointerType, FStarArrType, 
                    Ensures, 
                    Requires,
                    Returns,
                    Lemma, 
                    Project,
                    LemmaStatement};

/// A base class for term.                    
class Term {
public:
  RegionMapping RegInfo;
  RegionMapping &getRegInfoMapping();
  TermTag Tag;
  void setTag(TermTag T);
  void printTag();
  virtual void dumpPretty() = 0;
  virtual ~Term() = default;
};

/// Slprop in pulse can be a term.
typedef Term Slprop;

/// An Ast node for projections. 
/// This is meant to use for a member acces.
/// for a stack allocated object or a reference.
class Project : public Term {
  public: 
    Project(); 
    Term* BaseTerm; 
    std::string MemberName;
    virtual void dumpPretty() override; 
    static bool classof(const Term *T) { return T->Tag == TermTag::Project; }
};

/// Store Pulse Lemmas.
/// Store a list of pulse Lemmas.
class Lemma : public Term {
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
  Paren(Term *InnerExpr);
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
  VarTerm(std::string Name);
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
  AppE(std::string CallName);
  virtual ~AppE() = default;
  virtual void dumpPretty() override;
  void makeCallName(std::string CallName);
  void setCallName(VarTerm *Call);
  void pushArg(Term *Arg);
  static bool classof(const Term *T) { return T->Tag == TermTag::AppE; }
};

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
  RegionMapping RegInfo;
  RegionMapping &getRegInfoMapping();
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
  PulseAssignment(Term *LhsTerm, Term *RhsTerm);
  Term *Value;
  Term *Lhs;
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
  PulseArrayAssignment();
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) {
    return S->Tag == PulseStmtTag::ArrayAssignment;
  }
};

/// An IR node for representing a pulse let binding.
class LetBinding : public PulseStmt {
public:
  std::string VarName;
  std::string VarTy;
  LetBinding();
  LetBinding(std::string Lhs, Term *Rhs, MutOrRef Qual);
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
  PulseIf();
  Term *Head;
  std::vector<Term *> IfLemmas;
  PulseStmt *Then;
  PulseStmt *Else = nullptr;
  virtual void dumpPretty() override;
  static bool classof(const PulseStmt *S) { return S->Tag == PulseStmtTag::If; }
};

/// An IR node for representing a pulse while statement.
class PulseWhileStmt : public PulseStmt {
public:
  PulseWhileStmt();
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
  RegionMapping RegInfo;
  RegionMapping &getRegInfoMapping();
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
  RegionMapping RegInfo;
  RegionMapping &getRegInfoMapping();
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

/// A function to get the string for the C type.
std::string getPulseStringForCType(clang::QualType Ty, clang::ASTContext &Ctx);