#pragma once

#include <string>
#include <vector>
#include "clang/Basic/SourceLocation.h"
#include <vector>
// #include <unordered_map>
// #include <map>

enum class MacroEventKind {
    Define,
    Undefine,
    Expand,
    Ifdef,
    Ifndef,
    Defined,
    Comment
};

inline std::string toString(MacroEventKind kind) {
    switch (kind) {
        case MacroEventKind::Define:   return "Define";
        case MacroEventKind::Undefine: return "Undefine";
        case MacroEventKind::Expand:   return "Expand";
        case MacroEventKind::Ifdef:    return "Ifdef";
        case MacroEventKind::Ifndef:   return "Ifndef";
        case MacroEventKind::Defined:  return "Defined";
        case MacroEventKind::Comment:  return "Comment";
        default: return "Unknown";
    }
}

struct MacroTokenInfo {
    std::string TokenText;
    unsigned Line;
    unsigned Column;
    bool IsParam;
};

struct MacroEventInfo {
    std::string FileName; 
    MacroEventKind Kind;
    std::string MacroName;
    std::string ExpansionText; // if any, I am planning  to use StringRef here
    clang::SourceRange range; 
    unsigned Line;
    unsigned Column;
    std::vector<MacroTokenInfo> Tokens;
};
