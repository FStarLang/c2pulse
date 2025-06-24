#include "ANFVisitor.h"
#include "ANFConsumer.h"
#include "ANFFrontendAction.h"
#include "ANFTransformer.h"
#include "ExprLocationAnalyzer.h"
#include "PulseGenerator.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Debug.h"
#include <fstream>
#include <unistd.h>
#include <memory>

#include "Globals.h"

using namespace clang;
using namespace clang::tooling;

int GlobalGenSymCounter = 0;

const char* pulseProofTermFromC = "__pulseproofterm";
const char* pulseWhileInvariantFromC = "__pulsewhileinvariant";

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
    const auto &SourceFiles = OptionsParser->getSourcePathList();
    auto Tool = std::make_unique<ClangTool>(
        OptionsParser->getCompilations(), SourceFiles
    );
    
    LLVM_DEBUG({
        for (const auto &file : SourceFiles) {
            llvm::dbgs() << "Parsing: " << file << "\n";
        }
    });

    std::vector<std::unique_ptr<ASTUnit>> ASTList;
    // Syntax and semantic checks are performed internally by Clang during AST construction
    Tool->buildASTs(ASTList); 

    DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                        << "Number of parsed ASTs: " 
                                        << ASTList.size() << "\n"); 

    int fileIndex = 0;
    size_t failedASTs = 0;
    for (const auto &AST : ASTList) {
        const std::string &fileName = SourceFiles[fileIndex++];

        if (!AST) {
            llvm::errs() << "Failed to parse file: " << fileName << "\n";
            ++failedASTs;
            continue;
        }

        clang::ASTContext &Ctx = AST->getASTContext();

        LLVM_DEBUG({
            ExprLocationAnalyzer Analyzer(Ctx);
            Analyzer.analyze(Ctx.getTranslationUnitDecl());
            Analyzer.printNodeInfoMap();
        });

        PulseTransformer Transformer(Ctx);
        Transformer.transform();
        Transformer.writeToFile();
    }

    if (failedASTs > 0) {
        llvm::errs() << failedASTs << " file(s) failed to compile.\n";
        return 1;
    }

    llvm::outs() << "Success: Code transformed and syntax validated.\n";
    return 0;
}
