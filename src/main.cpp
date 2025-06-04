#include "ANFAndPulseRewriter.h"
#include "ANFConsumer.h"
#include "ANFFrontendAction.h"
#include "ANFTransformer.h"
#include "PulseGenerator.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include <fstream>

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
    auto NewFilePath = transformer.writeToFile();

    llvm::outs() << "Print the new file path: \n";
    llvm::outs() << NewFilePath;
    llvm::outs() << "\n\n";

    // Run Syntax-Only Action again on transformed output
    ClangTool TransformedTool(OptionsParser->getCompilations(), {NewFilePath});
    if (TransformedTool.run(
            newFrontendActionFactory<SyntaxOnlyAction>().get())) {
      llvm::errs() << "Error: Transformed code has syntax errors.\n";
      return 1;
    }

    std::vector<std::unique_ptr<ASTUnit>> TransformedASTList;
    TransformedTool.buildASTs(TransformedASTList);

    PulseTransformer _PulseTransformer(TransformedASTList);
    _PulseTransformer.transform();
    _PulseTransformer.writeToFile();
    // auto &FunctionDeclarations = _PulseTransformer
    auto Code = _PulseTransformer.getTransformedCode();

    llvm::outs() << "Print Program: \n\n";
    llvm::outs() << Code << "\n";

    llvm::outs() << "Success: Code transformed and syntax validated.\n";
    return 0;
}
