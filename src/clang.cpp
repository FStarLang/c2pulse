#include "clang_bridge.h"
#include "c2pulse/src/clang.rs.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CompilationDatabase.h"

using namespace clang::tooling;

static llvm::StringRef toStringRef(rust::Str const & str) {
    return llvm::StringRef(str.data(), str.size());
}

void parse_file(Ctx & ctx, rust::Str fileName) {
    std::vector<std::string> sourcePathList { std::string(fileName) };
    std::string compDBErrMsg;
    auto compDB = CompilationDatabase::autoDetectFromSource(toStringRef(fileName), compDBErrMsg);
    if (!compDB) {
        // TODO
        abort();
    }
    auto Tool = std::make_unique<ClangTool>(*compDB, sourcePathList);
    abort();
}