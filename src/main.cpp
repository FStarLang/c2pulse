#include "ExprLocationAnalyzer.h"
#include "PulseGenerator.h"
// #include "clang/Frontend/FrontendActions.h"
#include "MacroFrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Debug.h"
#include <fstream>
#include <unistd.h>
#include <memory>

#include "Globals.h"

#define DEBUG_TYPE "expr-analyzer"

using namespace clang;
using namespace clang::tooling;

int GlobalGenSymCounter = 0;

const char* pulseProofTermFromC = "__pulseproofterm";
const char* pulseWhileInvariantFromC = "__pulsewhileinvariant";

llvm::cl::OptionCategory ToolCategory("c2pulse options");

llvm::cl::opt<std::string>
    FunctionNameToProcess("func",
      llvm::cl::desc("Only transform the function named <func>"),
      llvm::cl::value_desc("function name"),
      llvm::cl::cat(ToolCategory));

llvm::cl::opt<std::string>
    TransformMode("mode",
      llvm::cl::desc("Select transformation mode: loc, pulse, or both"),
      llvm::cl::value_desc("mode"),
      llvm::cl::init("both"),
      llvm::cl::cat(ToolCategory));


enum class TransformModeEnum {
    LocOnly,
    PulseOnly,
    Both,
};

TransformModeEnum parseTransformMode(const std::string &modeStr) {
    if (modeStr.compare("loc") == 0) return TransformModeEnum::LocOnly;
    if (modeStr.compare("pulse") == 0) return TransformModeEnum::PulseOnly;
    return TransformModeEnum::Both;
}

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

    // int Result = Tool->run(newFrontendActionFactory<SyntaxOnlyAction>().get());
    int Result =  Tool->run(newFrontendActionFactory<MacroFrontendAction>().get());
    if (Result != 0) {
        llvm::errs() << "c2pulse cannot compile the C files due to a syntax error!\n";
        llvm::errs() << "Exiting..." << "\n";
        return Result;
    }
    llvm::outs() << "Success: Syntax validated.\n";

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


    TransformModeEnum mode = parseTransformMode(TransformMode);

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
        DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs()
                                        << "Processing AST for file: " 
                                        << fileName << "\n");

        if (mode == TransformModeEnum::LocOnly || mode == TransformModeEnum::Both) {
            ExprLocationAnalyzer Analyzer(Ctx);
            Analyzer.analyze(Ctx.getTranslationUnitDecl());
            Analyzer.printNodeInfoMap();
        }

        if (mode == TransformModeEnum::PulseOnly || mode == TransformModeEnum::Both) {
            PulseTransformer Transformer(Ctx);
            Transformer.transform();
            // Transformer.writeToFile();
        }
    }

    if (failedASTs > 0) {
        llvm::errs() << failedASTs << " file(s) failed to compile.\n";
        llvm::outs() << "Exiting...\n";
        return 1;
    }

    if(mode == TransformModeEnum::LocOnly || mode == TransformModeEnum::Both) {
        llvm::outs() << "Success: Location analysis completed.\n";
    } 
    
    if (mode == TransformModeEnum::PulseOnly || mode == TransformModeEnum::Both) {
        llvm::outs() << "Success: Pulse transformation completed.\n";
        llvm::outs() << "Generated Pulse code for " << ASTList.size() << " file(s).\n";
    }
    
    llvm::outs() << "Exiting...\n";
    return 0;
}
