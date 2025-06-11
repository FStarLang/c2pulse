#include <string>
#define DEBUG_TYPE "anf-pulse-rewriter"

extern int GlobalGenSymCounter; // Declaration of global variable

std::string gensym(const std::string base);

extern const char* pulseProofTermFromC;
extern const char* pulseWhileInvariantFromC;