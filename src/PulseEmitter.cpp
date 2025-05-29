#include "PulseEmitter.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "llvm/Support/raw_ostream.h"
#include <regex>
#include <sstream>

using namespace clang;

// Entry point for plugin to emit Pulse specs for a FunctionDecl
std::unique_ptr<PulseFn> PulseEmitter::emitPulseSpecFor(const FunctionDecl* FD) {
    PulseEmitter emitter(FD->getASTContext(), FD->getASTContext().getSourceManager());
    emitter.emitFunction(FD);
    return std::make_unique<PulseFn>(std::move(emitter.pulse)); 
}

std::string PulseEmitter::formatAsComments(const PulseFn& spec) {
    std::string out;
    for (const PulseAnnotation& ann : spec.ann) {
        out += "// @";
        out += (ann.kind == PulseAnnKind::Requires ? "requires " : "ensures ");
        out += ann.predicate + "\n";
    }
    return out;
}

// Constructs a PulseEmitter
PulseEmitter::PulseEmitter(ASTContext& ctx, SourceManager& sm)
  : Context(ctx), SM(sm), tempVarCount(0) {}

// Emits Pulse code for a given function declaration
void PulseEmitter::emitFunction(const FunctionDecl* FD) {
    if (!FD || !FD->hasBody())
        return;

    pulse.name = FD->getNameAsString();

    for (const auto* P : FD->parameters()) {
        std::string ty = P->getType().getAsString();
        if (StringRef(ty).ends_with(" *"))
            ty = ty.substr(0, ty.size() - 2);
        pulse.params.emplace_back(P->getNameAsString(), "ref " + ty);
    }

    pulse.ann = extractPulseAnnotations(FD, SM);
    transformStmt(FD->getBody(), pulse);
}

// Extracts Pulse annotations from function comments
std::vector<PulseAnnotation> PulseEmitter::extractPulseAnnotations(const clang::FunctionDecl* FD, const clang::SourceManager& SM){
    std::vector<PulseAnnotation> result;

    if (const auto* C = FD->getASTContext().getRawCommentForAnyRedecl(FD)) {
        std::string cleaned;
        for (char c : C->getRawText(SM))
            if (c != '\r')
                cleaned += c;

        std::istringstream in(cleaned);
        std::string line;
        std::regex reqRegex(R"(@requires\s+(\S+))");
        std::regex ensRegex(R"(@ensures\s+(\S+))");
        std::smatch match;

        while (std::getline(in, line)) {
            auto trimmed = StringRef(line).trim().str();
            if (std::regex_search(trimmed, match, reqRegex))
                result.push_back({PulseAnnKind::Requires, match[1], ""});
            else if (std::regex_search(trimmed, match, ensRegex))
                result.push_back({PulseAnnKind::Ensures, match[1], ""});
        }
    }

    int counter = 1;
    for (auto& ann : result)
        ann.regionId = "'n" + std::to_string(counter++);
    return result;
}

// Converts a Clang expression to a Pulse-compatible string
std::string PulseEmitter::printPulseExpr(const Expr* E, PrintingPolicy& Policy) {
    E = E->IgnoreImpCasts();

    if (auto* UO = dyn_cast<UnaryOperator>(E)) {
        if (UO->getOpcode() == UO_Deref) {
            std::string inner;
            llvm::raw_string_ostream os(inner);
            auto* sub = UO->getSubExpr()->IgnoreImpCasts();
            sub->printPretty(os, nullptr, Policy);
            os.flush();
            return "!" + inner;
        }
    }

    std::string result;
    llvm::raw_string_ostream os(result);
    E->printPretty(os, nullptr, Policy);
    os.flush();
    return result;
}

// Transforms statements into Pulse IR
void PulseEmitter::transformStmt(Stmt* S, PulseFn& pulse) {
    PrintingPolicy Policy(Context.getLangOpts());
    Policy.SuppressUnwrittenScope = true;
    Policy.SuppressTagKeyword = true;

    if (auto* CS = dyn_cast<CompoundStmt>(S)) {
        for (auto* Inner : CS->body())
            transformStmt(Inner, pulse);
    } else if (auto* DS = dyn_cast<DeclStmt>(S)) {
        for (auto* DI : DS->decls()) {
            if (auto* VD = dyn_cast<VarDecl>(DI)) {
                if (VD->hasInit()) {
                    std::string varName = VD->getNameAsString();
                    std::string initStr = printPulseExpr(VD->getInit(), Policy);
                    pulse.body.push_back(std::make_unique<PulseLet>(PulseLet{varName, initStr}));
                }
            }
        }
    } else if (auto* BO = dyn_cast<BinaryOperator>(S)) {
        if (BO->isAssignmentOp()) {
            Expr* LHS = BO->getLHS()->IgnoreImpCasts();
            Expr* RHS = BO->getRHS()->IgnoreImpCasts();

            std::string lhsStr = printPulseExpr(LHS, Policy);
            std::string rhsStr = printPulseExpr(RHS, Policy);

            if (auto* UO = dyn_cast<UnaryOperator>(LHS); UO && UO->getOpcode() == UO_Deref) {
                std::string ptrName = printPulseExpr(UO->getSubExpr()->IgnoreImpCasts(), Policy);
                std::string tmp = "tmp" + std::to_string(++tempVarCount);
                pulse.body.push_back(std::make_unique<PulseLet>(PulseLet{tmp, rhsStr}));
                pulse.body.push_back(std::make_unique<PulseAssign>(PulseAssign{ptrName, tmp}));
            } else {
                pulse.body.push_back(std::make_unique<PulseLet>(PulseLet{lhsStr, rhsStr}));
            }
        }
    } else if (auto* E = dyn_cast<Expr>(S)) {
        if (auto* CE = dyn_cast<CallExpr>(E)) {
            std::string callStr = printPulseExpr(CE, Policy);
            pulse.body.push_back(std::make_unique<PulseLet>(PulseLet{"_", callStr}));
        } else {
            std::string exprStr = printPulseExpr(E, Policy);
            pulse.body.push_back(std::make_unique<PulseLet>(PulseLet{"_", exprStr}));
        }
    }
}
