#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <utility>

// // Enum for annotation kinds
// enum class PulseAnnKind {
//     Requires,
//     Ensures
// };

// // Struct for Pulse annotations
// struct PulseAnnotation {
//     PulseAnnKind kind;
//     std::string predicate; //ptrName
//     std::string regionId;
// };

// // Abstract base class for Pulse statements
// struct PulseStmt {
//     virtual std::string print() const = 0;
//     virtual ~PulseStmt() = default;
// };

// // Represents a 'let' binding in Pulse
// struct PulseLet : PulseStmt {
//     std::string var, expr;
//     PulseLet(std::string v, std::string e) : var(std::move(v)), expr(std::move(e)) {}
//     std::string print() const override {
//         return (var == "_") ? "" : "  let " + var + " = " + expr + ";";
//     }
// };

// // Represents an assignment in Pulse
// struct PulseAssign : PulseStmt {
//     std::string ptr, rhs;
//     PulseAssign(std::string p, std::string r) : ptr(std::move(p)), rhs(std::move(r)) {}
//     std::string print() const override {
//         return "  " + ptr + " := " + rhs + ";";
//     }
// };

// // Represents a Pulse function
// struct PulseFn {
//     std::string name;
//     std::vector<std::pair<std::string, std::string>> params;
//     std::vector<PulseAnnotation> ann;
//     std::vector<std::shared_ptr<PulseStmt>> body;

//     std::string print() const {
//         std::ostringstream out;
//         out << "fn " << name << "(";
//         for (size_t i = 0; i < params.size(); ++i) {
//             out << params[i].first << " " << params[i].second;
//             if (i + 1 < params.size())
//                 out << ", ";
//         }
//         out << ")\n";
//         for (const auto& A : ann) {
//             out << (A.kind == PulseAnnKind::Requires ? "requires " : "ensures  ")
//                 << A.predicate << " |-> " << A.regionId << "\n";
//         }
//         out << "{\n";
//         for (const auto& S : body) {
//             if (auto line = S->print(); !line.empty())
//                 out << line << "\n";
//         }
//         out << "}";
//         return out.str();
//     }
// };


// Define F* IR Similar to type term 
//https://github.com/FStarLang/FStar/blob/3ff998c60bb0efe9925fc94e8fb8b785b9485af0/src/parser/FStarC.Parser.AST.fsti#L40
enum class TermTag {
    Const,
    Var, 
    Name,
    AppE
};


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
}; 


class AppE : public Term {

    public:
       VarTerm *CallName; 
       std::vector<Term*> Args;
};


// Define pattern class 


// class Pattern { };

// class PatWild : public Pattern {
// };

// class PatVar


//Define Pulse IR 

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
      Term* E;
};

class PulseAssignment : public PulseStmt {
    public: 
        Term* Lhs; 
        Term* value; 
};

class PulseArrayAssignment : public PulseStmt {
    public: 
        Term* Arr; 
        Term* Index; 
        Term* Value;
};

class LetBinding : public PulseStmt {
    public:
        std::string VarName;
        Term *LetInit;
};

class PulseIf : public PulseStmt{
    public: 
       Term* Head; 
       PulseStmt* Then; 
       PulseStmt* Else = nullptr;
};

class PulseWhileStmt : public PulseStmt {

    public: 
      PulseStmt *Guard;
      std::vector<Slprop*> Invariant;
      PulseStmt *Body; 
};

class PulseSequence : public PulseStmt {
    public: 
      PulseStmt *S1;
      PulseStmt *S2; 
};


//Function declaration in Pulse IR 
struct Binder {
    std::string Ident; 
    Term* Type;
};

struct _PulseFnDecl {
    std::string Name; 
    std::vector<Binder*> Args;
};


struct _PulseFnDefn {
    std::string Name;
    std::vector<Binder*> Args;
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
        _PulseFnDefn *Defn;
}; 

class PulseFnDecl : public PulseDecl {
    public: 
        _PulseFnDecl *Defn;
}; 










