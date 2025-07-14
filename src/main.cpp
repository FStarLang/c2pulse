#include "ExprLocationAnalyzer.h"
#include "MacroFrontendActionFactory.h"
#include "PulseASTGenerator.h"
#include "Globals.h"
#include "FileIDHash.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Debug.h"
#include <functional>

#include <unistd.h>
#include <memory>
#include <unordered_map>
#include <map>

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

static TransformModeEnum parseTransformMode(const std::string &modeStr) {
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

    // std::vector<MacroEventInfo> macroEventsVec;
    // auto Factory = std::make_unique<MacroFrontendActionFactory>(macroEventsVec);
    
    // I decided to use a map to store macro events by file ID and source location (range)
    // This allows for better organization and retrieval of macro events across different 
    // files and locations. And also allows for events with same range in different files
    // be stored separately, I am trying to avoid collisions. I believe this is useful for 
    // large projects with many files.
    std::unordered_map<FileID, std::map<unsigned, MacroEventInfo>> macroInfoMap;
    auto Factory = std::make_unique<MacroFrontendActionFactory>(macroInfoMap);
    int Result = Tool->run(Factory.get());
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

    LLVM_DEBUG( llvm::dbgs() << "Number of parsed ASTs: " 
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
        LLVM_DEBUG( llvm::dbgs() << "Processing AST for file: " 
                                 << fileName << "\n");
        
        clang::SourceManager &SM = Ctx.getSourceManager();
        clang::FileID fileID = SM.getMainFileID();
        // this is the map of events that you can pass to the PulseTransformer or ExprLocationAnalyzer
        // it depends where you will decide to collect the source information
        // I was wondering if we could have it modular, so we can have everything
        // organized in such a way that it is easy to maintain and extend.
        const std::map<unsigned, MacroEventInfo> &events = macroInfoMap[fileID]; 
        LLVM_DEBUG( 
        if (!events.empty()) {
        llvm::outs() << "  Number of macro events collected: " << events.size() << "\n";
        llvm::outs() << "----------------------\n";
        for (const auto &e : events) {
            const MacroEventInfo &macroInfo = e.second;
            llvm::outs() << "Kind: " << toString(macroInfo.Kind) << "\n";
            llvm::outs() << "Macro: " << macroInfo.MacroName << "\n";
            llvm::outs() << "Expansion: " << macroInfo.ExpansionText << "\n";
            llvm::outs() << "Location: Line: " << macroInfo.Line << ", Column: " << macroInfo.Column << "\n";
            llvm::outs() << "Filename: " << macroInfo.FileName << "\n";
            llvm::outs() << "Tokens:\n";
            for (const auto &t : macroInfo.Tokens) {
                llvm::outs() << "  " << (t.IsParam ? "[param] " : "[macro] ")
                             << t.TokenText << " at Line " << t.Line
                             << ", Column " << t.Column << "\n";
            }
            llvm::outs() << "----------------------\n";
        }
        llvm::outs() << "\n";
        } else {
            llvm::outs() << "  [No macro events found]\n";
        }
        );

        if (mode == TransformModeEnum::LocOnly || mode == TransformModeEnum::Both) {
            ExprLocationAnalyzer Analyzer(Ctx);
            Analyzer.analyze(Ctx.getTranslationUnitDecl());
        }

        if (mode == TransformModeEnum::PulseOnly || mode == TransformModeEnum::Both) {
            PulseTransformer Transformer(Ctx);
            Transformer.transform();
            Transformer.writeToFile();
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
