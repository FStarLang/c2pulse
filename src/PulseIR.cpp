#include "PulseIR.h"

void PulseSequence::assignS1(PulseStmt *S) { S1 = S; }

void PulseSequence::assignS2(PulseStmt *S) { S2 = S; }

void Name::setName(std::string NamedVal) { NamedValue = NamedVal; }

void FStarType::setName(std::string Name) { NamedValue = Name; }

PulseFnDefn::PulseFnDefn(_PulseFnDefn *Defn) : Defn(Defn) {}