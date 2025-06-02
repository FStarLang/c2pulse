#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "ANFAndPulseRewriter.h"
#include "ANFFrontendAction.h"
#include "ANFConsumer.h"
#include <fstream>
#include "ANFTransformer.h"

using namespace clang;
using namespace clang::tooling;

llvm::cl::OptionCategory ToolCategory("anf-pulse-tool options");

llvm::cl::opt<std::string>
    FunctionNameToProcess("func",
      llvm::cl::desc("Only transform the function named <func>"),
      llvm::cl::value_desc("function name"),
      llvm::cl::cat(ToolCategory));

llvm::cl::opt<std::string>
    TransformMode("mode",
      llvm::cl::desc("Select transformation mode: anf, pulse, or both"),
      llvm::cl::value_desc("mode"),
      llvm::cl::init("both"),
      llvm::cl::cat(ToolCategory));


int main(int argc, const char **argv) {
    
    auto OptionsParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!OptionsParser) {
        llvm::errs() << "Error parsing command line options: "
                     << OptionsParser.takeError() << "\n";
        return 1;
    }
    
    // Create a Clang tool instance
    ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());

    std::vector<std::unique_ptr<ASTUnit>> ASTList;
    Tool.buildASTs(ASTList);

    if (Tool.run(newFrontendActionFactory<clang::SyntaxOnlyAction>().get())){
	    llvm::errs() << "Error: the provided C program has a compilation error";
    	    return 0;
    }


    ANFTranformer transformer(ASTList);
    transformer.transform();
    std::string transformedCode = transformer.getTransformedCode();
    llvm::outs() << "Transformed code:\n";
    llvm::outs() << transformedCode << "\n";
    llvm::outs() << "End of transformed code.\n";

    // ANFFrontendAction TransformationAction;

    // clang::Rewriter RewriterForPlugin;
    // RewriterForPlugin.setSourceMgr(ASTList[0]->getSourceManager(), ASTList[0]->getLangOpts());

    // for (auto &AstCtx : ASTList) {
    //     // Set the AST context for the transformation action
    //     // auto Consumer = TransformationAction.CreateASTConsumer(&(AstCtx.get()->getASTContext()));
    //     // Consumer->HandleTranslationUnit(AstCtx.get()->getASTContext());
    //     ANFConsumer Consumer(AstCtx->getASTContext(), RewriterForPlugin);
    //     Consumer.HandleTranslationUnit(AstCtx->getASTContext());
    // }
    
    // clang::SourceManager &SM = RewriterForPlugin.getSourceMgr();
    // clang::FileID MainFileID = SM.getMainFileID();

    //     if (!MainFileID.isValid()) {
    //       llvm::errs() << "Error: Invalid MainFileID—source file may not be loaded correctly.\n";
    //       return 1;
    //     }

    // // Capture rewritten buffer
    // const llvm::RewriteBuffer *Buffer = RewriterForPlugin.getRewriteBufferFor(MainFileID);
    // //llvm::outs() << "Print buffer \n"; 
    // //Buffer->write(llvm::outs());
    // //llvm::outs() << "End buffer \n";
    // if (!Buffer) {
    //     llvm::errs() << "Warning: Rewriter buffer is empty—no modifications detected.\n";
    //     return 1;
    // }

    // // Store transformed code in the class variable
    // auto transformedCode = std::string(Buffer->begin(), Buffer->end());

    // llvm::outs() << "Print buffer \n"; 
    // llvm::outs() << transformedCode << "\n";
    // llvm::outs() << "End buffer transFormedCode \n";

    // //  llvm::outs() << "Print transformed code:\n";
    // // llvm:: outs() << transformedCode << "\n";
    // // llvm::outs() << "End of transformed code.\n";

    // // Step 3: Write transformed code to a temporary file
    // std::string TempFilePath = "transformed_output.c";
    // std::ofstream OutFile(TempFilePath);
    // if (!OutFile.is_open()) {
    //     llvm::errs() << "Error: Failed to create temporary file for transformed code.\n";
    //     return 1;
    // }

    // OutFile << transformedCode;
    // OutFile.close();

    // // Step 4: Run Syntax-Only Action again on transformed output
    // ClangTool TransformedTool(OptionsParser->getCompilations(), {TempFilePath});
    // if (TransformedTool.run(newFrontendActionFactory<SyntaxOnlyAction>().get())) {
    //     llvm::errs() << "Error: Transformed code has syntax errors.\n";
    //     return 1;
    // }

    // llvm::outs() << "Success: Code transformed and syntax validated.\n";
    // return 0;



    // if (Tool.run(clang::tooling::newFrontendActionFactory<ANFFrontendAction>()
    //                  .get())) {
    //   llvm::errs() << "Error: the tranformation did not run correctly.\n";
    //   return 0;
    // }

    // std::string transformedCode = TransformationAction.getTransformedCode();
    
   



}
