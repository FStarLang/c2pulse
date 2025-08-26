#include "generated.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CompilationDatabase.h"

using namespace clang::tooling;
using rust::RefMut;
using namespace rust::crate::clang;

// static llvm::StringRef toStringRef(rust::Str const & str) {
//     return llvm::StringRef(str.data(), str.size());
// }

std::string toString(rust::Ref<rust::Str> str) {
    return std::string((char const *) str.as_ptr(), str.len());
}

void parse_file(RefMut<Ctx> ctx) {
    std::string fileName = toString(ctx.get_input_file_name());
    std::vector<std::string> sourcePathList { fileName };
    std::string compDBErrMsg;
    auto compDB = CompilationDatabase::autoDetectFromSource(fileName, compDBErrMsg);
    if (!compDB) {
        // TODO
        abort();
    }
    auto Tool = std::make_unique<ClangTool>(*compDB, sourcePathList);
    abort();
}

namespace rust::exported_functions {
Unit parse_file(RefMut<Ctx> ctx) {
    ::parse_file(ctx);
    return {};
}
}