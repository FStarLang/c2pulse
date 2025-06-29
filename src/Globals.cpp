#include "Globals.h"

using namespace std;

extern int GlobalGenSymCounter;

static string counterToVar(){
    return to_string(GlobalGenSymCounter++);
}


string gensym(const string base = "var") {
    return base + counterToVar();
}
