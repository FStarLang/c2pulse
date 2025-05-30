#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "ANFAndPulseRewriter.h"


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

    return Tool.run(clang::tooling::newFrontendActionFactory<ANFFrontendAction>().get());

}
