#pragma once

#include "clang/Basic/SourceLocation.h"
#include <functional>

struct FileIDHash {
    size_t operator()(const clang::FileID& id) const {
        return id.getHashValue();
    }
};
