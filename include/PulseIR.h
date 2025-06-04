#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <utility>

// Define F* IR Similar to type term
// https://github.com/FStarLang/FStar/blob/3ff998c60bb0efe9925fc94e8fb8b785b9485af0/src/parser/FStarC.Parser.AST.fsti#L40
enum class TermTag { Const, Var, Name, AppE, FStarType, FStarPointerType };

class Term {
public:
  TermTag Tag;
};

class ConstTerm : public Term {
public:
  int ConstantValue;
};

class VarTerm : public Term {
public:
  std::string VarName;
};

class Name : public Term {
public:
  std::string NamedValue;
  virtual void setName(std::string Name) = 0;
  virtual ~Name() = default;
};

class FStarType : public Name {

public:
  virtual void setName(std::string Name) override;
  virtual ~FStarType() = default;
};

class FStarPointerType : public FStarType {
public:
  FStarPointerType *Pointer;
  virtual void setName(std::string Name) override;
  virtual ~FStarPointerType() = default;
};

class AppE : public Term {
public:
  VarTerm *CallName;
  std::vector<Term *> Args;
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

class PulseStmt {
public:
  PulseStmtTag Tag;
};

class PulseExpr : public PulseStmt {
public:
  Term *E;
};

class PulseAssignment : public PulseStmt {
public:
  Term *Lhs;
  Term *value;
};

class PulseArrayAssignment : public PulseStmt {
public:
  Term *Arr;
  Term *Index;
  Term *Value;
};

class LetBinding : public PulseStmt {
public:
  std::string VarName;
  Term *LetInit;
};

class PulseIf : public PulseStmt {
public:
  Term *Head;
  PulseStmt *Then;
  PulseStmt *Else = nullptr;
};

class PulseWhileStmt : public PulseStmt {

public:
  PulseStmt *Guard;
  std::vector<Slprop *> Invariant;
  PulseStmt *Body;
};

class PulseSequence : public PulseStmt {
public:
  PulseStmt *S1;
  PulseStmt *S2;
  void assignS1(PulseStmt *S);
  void assignS2(PulseStmt *S);
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

struct _PulseFnDefn {
  std::string Name;
  std::vector<Binder *> Args;
  bool isRecursive;
  PulseStmt *Body;
};

enum class PulseFnKind {
  FnDefn, // Function definition
  FnDecl  // Function declaration
};

class PulseDecl {
public:
  PulseFnKind Kind;
};

class PulseFnDefn : public PulseDecl {
public:
  PulseFnDefn(_PulseFnDefn *Defn);
  _PulseFnDefn *Defn;
};

class PulseFnDecl : public PulseDecl {
public:
  _PulseFnDecl *Defn;
};
