module Issue48_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo ()
returns Int32.t
{
let mut x: Int32.t = witness #_ #_;
(! x);
}

//Dumping the Clang AST.
// FunctionDecl 0x6301a3ce2448 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue48_test.c:3:1, line:7:1> line:3:5 foo 'int ()'
// |-CompoundStmt 0x6301a3ce26b8 <line:4:1, line:7:1>
// | |-DeclStmt 0x6301a3ce2658 <line:5:2, col:7>
// | | `-VarDecl 0x6301a3ce25f0 <col:2, col:6> col:6 used x 'int'
// | `-ReturnStmt 0x6301a3ce26a8 <line:6:2, col:9>
// |   `-ImplicitCastExpr 0x6301a3ce2690 <col:9> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x6301a3ce2670 <col:9> 'int' lvalue Var 0x6301a3ce25f0 'x' 'int'
// `-AnnotateAttr 0x6301a3ce24f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
