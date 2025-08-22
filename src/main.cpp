#include "ExprLocationAnalyzer.h"
#include "MacroFrontendActionFactory.h"
#include "PulseASTGenerator.h"
#include "Globals.h"
#include "FileIDHash.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Driver/Driver.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include <functional>

#include <unistd.h>
#include <memory>
#include <unordered_map>
#include <map>

#include <dlfcn.h>

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

llvm::cl::opt<std::string>
    TmpDirectory("tmpdir",
        llvm::cl::desc("Temporary directory for input and output files (used by VS Code extension)"),
        llvm::cl::init(""));

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

static llvm::Expected<std::string> readFileToEOF(std::string const & FN) {
    auto FD = llvm::sys::fs::openNativeFileForRead(FN);
    if (!FD) { return std::move(FD.takeError()); }
    llvm::SmallString<llvm::sys::fs::DefaultReadChunkSize> Buffer;
    if (auto E = llvm::sys::fs::readNativeFileToEOF(*FD, Buffer)) { return std::move(E); }
    return std::string(Buffer.data(), Buffer.size());
}

std::string getBinaryForResourcesPath() {
    Dl_info info;
    if (dladdr((void *) &clang::driver::Driver::GetResourcesPath, &info)) {
        return info.dli_fname;
    } else {
        return "/usr/bin/clang";
    }
}

std::string getResourcesPath() {
    return clang::driver::Driver::GetResourcesPath(getBinaryForResourcesPath());
}

int main(int argc, const char **argv) {

    auto OptionsParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (auto error = OptionsParser.takeError()) {
        llvm::errs() << "Error parsing command line options: "
                     << toString(std::move(error)) << "\n";
        return 1;
    }
    
    // Create a Clang tool instance
    const auto &SourceFiles = OptionsParser->getSourcePathList();
    auto Tool = std::make_unique<ClangTool>(
        OptionsParser->getCompilations(), SourceFiles
    );

    for (auto & SF : SourceFiles) {
        for (auto & CC : OptionsParser->getCompilations().getCompileCommands(SF)) {
            for (auto & C : CC.CommandLine) {
                llvm::outs() << C << " ";
            }
            llvm::outs() << " # " << CC.Heuristic << "\n";
        }
    }

    Tool->appendArgumentsAdjuster(OptionsParser->getArgumentsAdjuster());

    Tool->appendArgumentsAdjuster(getInsertArgumentAdjuster(
        { "-DC2PULSE", "-fno-builtin" },
        ArgumentInsertPosition::BEGIN));

    auto resourceDir = getResourcesPath();
    llvm::outs() << "resource dir is: " << resourceDir << "\n";

    Tool->appendArgumentsAdjuster(getInsertArgumentAdjuster(
        { "-resource-dir", resourceDir },
        ArgumentInsertPosition::BEGIN));

    std::optional<std::string> TmpDir;
    if (TmpDirectory.getValue() != "") {
        TmpDir = TmpDirectory.getValue();
    }

    std::vector<std::string> SFBufs(SourceFiles.size()); // VFS takes references to file contents, we need to hold on to the memory...
    if (TmpDir) {
        for (auto & SF : SourceFiles) {
            auto TmpSF = *TmpDir + "/" + std::filesystem::path(SF).filename().string();
            auto S = readFileToEOF(TmpSF);
            if (!S) {
                llvm::errs() << "cannot read input file " << SF << " from temporary location " << TmpSF << ":\n"
                    << toString(std::move(S.takeError())) << "\n";
                return 1;
            }
            SFBufs.emplace_back(*S);
            Tool->mapVirtualFile(SF, StringRef(SFBufs.back()));
        }
    }

    // std::vector<MacroEventInfo> macroEventsVec;
    // auto Factory = std::make_unique<MacroFrontendActionFactory>(macroEventsVec);
    
    // I decided to use a map to store macro events by file ID and source location (range)
    // This allows for better organization and retrieval of macro events across different 
    // files and locations. And also allows for events with same range in different files
    // be stored separately, I am trying to avoid collisions. I believe this is useful for 
    // large projects with many files.
    std::unordered_map<FileID, std::map<unsigned, MacroEventInfo>, FileIDHash> macroInfoMap;
    std::unordered_map<unsigned, std::vector<TokenInfo>> macroTokens;
    auto Factory = std::make_unique<MacroFrontendActionFactory>(macroInfoMap, macroTokens);
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
        // LLVM_DEBUG( 
        if (!events.empty()) {
        llvm::outs() << "  Number of macro events collected: " << events.size() << "\n";
        llvm::outs() << "----------------------\n";
        for (const auto &e : events) {
            const MacroEventInfo &macroInfo = e.second;
            llvm::outs() << "Kind: " << toString(macroInfo.Kind) << "\n";
            llvm::outs() << "Macro: " << macroInfo.MacroName << "\n";
            llvm::outs() << "Counter value: " << macroInfo.CounterValue << "\n";
            llvm::outs() << "Range: " << macroInfo.range.printToString(SM) << "\n";
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
        // );

        if (mode == TransformModeEnum::LocOnly || mode == TransformModeEnum::Both) {
            ExprLocationAnalyzer Analyzer(Ctx);
            Analyzer.analyze(Ctx.getTranslationUnitDecl());
        }

        if (mode == TransformModeEnum::PulseOnly || mode == TransformModeEnum::Both) {
            PulseTransformer Transformer(Ctx, macroTokens);
            Transformer.transform();
            Transformer.writeToFile(TmpDir);
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
