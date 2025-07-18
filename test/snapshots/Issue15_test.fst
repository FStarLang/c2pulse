module Issue15_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_empty ()
requires emp
ensures emp
{
();
}

//Dumping the Clang AST.
// FunctionDecl 0x5ae98a5f96c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue15_test.c:5:1, line:6:12> line:5:6 test_empty 'void (void)'
// |-CompoundStmt 0x5ae98a5f98c8 <line:6:1, col:12>
// | `-ReturnStmt 0x5ae98a5f98b8 <col:3>
// |-AnnotateAttr 0x5ae98a5f9768 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// `-AnnotateAttr 0x5ae98a5f9800 <line:4:24, col:71> pulse "ensures:emp|END"
