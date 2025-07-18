module Issue24_test

#lang-pulse

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn refs_are_scoped ()
returns s:ref int32
ensures s |-> 0l
{
let s0 : Int32.t = 0l;
let mut s : Int32.t = s0;
(s);
}

//Dumping the Clang AST.
// FunctionDecl 0x640635f9fc28 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue24_test.c:6:1, line:10:1> line:6:6 refs_are_scoped 'int *()'
// |-CompoundStmt 0x640635f9ff98 <line:7:1, line:10:1>
// | |-DeclStmt 0x640635f9ff38 <line:8:5, col:14>
// | | `-VarDecl 0x640635f9feb0 <col:5, col:13> col:9 used s 'int' cinit
// | |   `-IntegerLiteral 0x640635f9ff18 <col:13> 'int' 0
// | `-ReturnStmt 0x640635f9ff88 <line:9:5, col:13>
// |   `-UnaryOperator 0x640635f9ff70 <col:12, col:13> 'int *' prefix '&' cannot overflow
// |     `-DeclRefExpr 0x640635f9ff50 <col:13> 'int' lvalue Var 0x640635f9feb0 's' 'int'
// |-AnnotateAttr 0x640635f9fcd0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
// |-AnnotateAttr 0x640635f9fd70 <line:5:22, col:58> pulse "returns:s:ref int32|END"
// `-AnnotateAttr 0x640635f9fde0 <line:4:24, col:71> pulse "ensures:s |-> 0l|END"
