#pragma once

#include "clang/Basic/SourceLocation.h"
#include <functional>

// Define hash function for FileID
namespace std {
    template<> 
    struct hash<clang::FileID> {
        size_t operator()(const clang::FileID& id) const {
            return id.getHashValue();
        }
    };
}
