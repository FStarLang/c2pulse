#include "PulseIR.h"

// define functions for F* classes.

void Term::setTag(TermTag T) { Tag = T; }

void VarTerm::setVarName(std::string Name) { VarName = Name; }

void Name::setName(std::string Name) { NamedValue = Name; }

void FStarType::setName(std::string Name) { NamedValue = Name; }

void FStarPointerType::setName(std::string Name) { NamedValue = Name; }

void PulseSequence::assignS1(PulseStmt *S) { S1 = S; }

void PulseSequence::assignS2(PulseStmt *S) { S2 = S; }

PulseFnDefn::PulseFnDefn(_PulseFnDefn *Defn) : Defn(Defn) {}

void PulseStmt::setTag(PulseStmtTag T) { Tag = T; }