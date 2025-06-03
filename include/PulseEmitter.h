// #pragma once

// #include "clang/AST/AST.h"
// #include "clang/AST/ASTContext.h"
// #include "clang/Basic/SourceManager.h"

// #include "PulseIR.h"

// class PulseEmitter {
// public:
//     // Entry point to extract a Pulse spec from a function declaration
//     static std::unique_ptr<PulseFn>  emitPulseSpecFor(const clang::FunctionDecl* FD);

//     // Pretty-prints Pulse annotations as comments
//     static std::string formatAsComments(const PulseFn& spec);

//     // Constructs a PulseEmitter for a given AST context and source manager
//     PulseEmitter(clang::ASTContext& ctx, clang::SourceManager& sm);

//     // Emits Pulse IR from the function body and stores it in `pulse`
//     void emitFunction(const clang::FunctionDecl* FD);

//     // Output spec generated from function body
//     PulseFn pulse;

// private:
//     clang::ASTContext& Context;
//     clang::SourceManager& SM;
//     int tempVarCount;

//     // Extracts @requires and @ensures annotations from comments
//     std::vector<PulseAnnotation> extractPulseAnnotations(const clang::FunctionDecl* FD, const clang::SourceManager& SM);

//     // Converts a Clang expression into a Pulse-compatible string
//     std::string printPulseExpr(const clang::Expr* E, clang::PrintingPolicy& Policy);

//     // Transforms the function body into Pulse statements
//     void transformStmt(clang::Stmt* S, PulseFn& pulse);
// };
