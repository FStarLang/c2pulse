#include "ANFFrontendAction.h"
#include "llvm/Support/raw_ostream.h"

std::string ANFFrontendAction::getTransformedCode() {
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
  // if (ConsumerInstance) {
  //   transformedCode = ConsumerInstance->getTransformedCode();
  // }

  // RewriterForPlugin
  //     .getEditBuffer(RewriterForPlugin.getSourceMgr().getMainFileID())
  //     .write(llvm::outs());

 RewriterForPlugin.setSourceMgr(getCompilerInstance().getSourceManager(),
                                   getCompilerInstance().getLangOpts());

    auto &SM = getCompilerInstance().getSourceManager();
    FileID MainFileID = SM.getMainFileID();
    
    if (!MainFileID.isValid()) {
        llvm::errs() << "Error: Invalid MainFileID—source file may not be loaded correctly.\n";
        return;
    }

    // Capture rewritten buffer
    const llvm::RewriteBuffer *Buffer = RewriterForPlugin.getRewriteBufferFor(MainFileID);
    //llvm::outs() << "Print buffer \n"; 
    //Buffer->write(llvm::outs());
    //llvm::outs() << "End buffer \n";
    if (!Buffer) {
        llvm::errs() << "Warning: Rewriter buffer is empty—no modifications detected.\n";
        return;
    }

    // Store transformed code in the class variable
    transformedCode = std::string(Buffer->begin(), Buffer->end());

    llvm::outs() << "Print buffer \n"; 
    llvm::outs() << transformedCode << "\n";
    llvm::outs() << "End buffer transFormedCode \n";
    
    llvm::outs() << "Transformation completed. Transformed code is now stored in";




}
