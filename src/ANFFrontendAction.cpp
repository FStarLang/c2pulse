#include "ANFFrontendAction.h"
#include "llvm/Support/raw_ostream.h"

std::string &ANFFrontendAction::getTransformedCode() {
  return transformedCode;
}

std::unique_ptr<clang::ASTConsumer>
ANFFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI,
                                     llvm::StringRef InFile) {
  RewriterForPlugin.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
  auto Consumer =
      std::make_unique<ANFConsumer>(CI.getASTContext(), RewriterForPlugin);
  ConsumerInstance = Consumer.get(); // Store the instance to retrieve data later
  return std::move(Consumer);
}

bool ANFFrontendAction::ParseArgs(const clang::CompilerInstance &CI,
                                  const std::vector<std::string> &args) {
  return true;
}

void ANFFrontendAction::EndSourceFileAction() {
  if (ConsumerInstance) {
    transformedCode = ConsumerInstance->getTransformedCode();
  }

  RewriterForPlugin
      .getEditBuffer(RewriterForPlugin.getSourceMgr().getMainFileID())
      .write(llvm::outs());
}
