#include "Globals.h"
#include <atomic>


extern int GlobalGenSymCounter;

static std::string counterToVar(){
    return std::to_string(GlobalGenSymCounter++);
}


std::string gensym(const std::string base = "var") {
    return base + counterToVar();
}
