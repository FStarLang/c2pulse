#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "ANFAndPulseRewriter.cpp"


using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolCategory("anf-pulse-tool options");

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
