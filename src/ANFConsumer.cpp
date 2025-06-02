#include "ANFConsumer.h"

ANFConsumer::ANFConsumer(clang::ASTContext &Ctx, clang::Rewriter &R)
    : Visitor(R, Ctx) {}

void ANFConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
  transformedCode = Visitor.getTransformedCode();
}

std::string ANFConsumer::getTransformedCode() const {
  return transformedCode;
}
