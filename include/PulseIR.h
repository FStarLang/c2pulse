#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <utility>

// Enum for annotation kinds
enum class PulseAnnKind {
    Requires,
    Ensures
};

// Struct for Pulse annotations
struct PulseAnnotation {
    PulseAnnKind kind;
    std::string predicate; //ptrName
    std::string regionId;
};

// Abstract base class for Pulse statements
struct PulseStmt {
    virtual std::string print() const = 0;
    virtual ~PulseStmt() = default;
};

// Represents a 'let' binding in Pulse
struct PulseLet : PulseStmt {
    std::string var, expr;
    PulseLet(std::string v, std::string e) : var(std::move(v)), expr(std::move(e)) {}
    std::string print() const override {
        return (var == "_") ? "" : "  let " + var + " = " + expr + ";";
    }
};

// Represents an assignment in Pulse
struct PulseAssign : PulseStmt {
    std::string ptr, rhs;
    PulseAssign(std::string p, std::string r) : ptr(std::move(p)), rhs(std::move(r)) {}
    std::string print() const override {
        return "  " + ptr + " := " + rhs + ";";
    }
};

// Represents a Pulse function
struct PulseFn {
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::vector<PulseAnnotation> ann;
    std::vector<std::shared_ptr<PulseStmt>> body;

    std::string print() const {
        std::ostringstream out;
        out << "fn " << name << "(";
        for (size_t i = 0; i < params.size(); ++i) {
            out << params[i].first << " " << params[i].second;
            if (i + 1 < params.size())
                out << ", ";
        }
        out << ")\n";
        for (const auto& A : ann) {
            out << (A.kind == PulseAnnKind::Requires ? "requires " : "ensures  ")
                << A.predicate << " |-> " << A.regionId << "\n";
        }
        out << "{\n";
        for (const auto& S : body) {
            if (auto line = S->print(); !line.empty())
                out << line << "\n";
        }
        out << "}";
        return out.str();
    }
};
