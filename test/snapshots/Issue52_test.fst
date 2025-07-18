module Issue52_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn fake_arr
(x : array Int32.t)
requires pure (length x == SizeT.v 5sz)
requires exists* v. (x |-> v)
returns Int32.t
ensures exists* v. (x |-> v)
{
let mut x : (array Int32.t) = x;
pts_to_len !x;
(op_Array_Access (! x) (int32_to_sizet 0l));
}

//Dumping the Clang AST.
// FunctionDecl 0x56b8bef7d1a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test.c:7:1, line:10:1> line:7:5 fake_arr 'int (int *)'
// |-ParmVarDecl 0x56b8bef7d038 <col:25, col:30> col:30 used x 'int *'
// | `-AnnotateAttr 0x56b8bef7d0a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:5|END"
// |-CompoundStmt 0x56b8bef7d5b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test.c:7:33, line:10:1>
// | |-AttributedStmt 0x56b8bef7d500 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test.c:8:22>
// | | |-AnnotateAttr 0x56b8bef7d488 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !x|END"
// | | `-NullStmt 0x56b8bef7d480 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test.c:8:22>
// | `-ReturnStmt 0x56b8bef7d5a8 <line:9:2, col:12>
// |   `-ImplicitCastExpr 0x56b8bef7d590 <col:9, col:12> 'int' <LValueToRValue>
// |     `-ArraySubscriptExpr 0x56b8bef7d570 <col:9, col:12> 'int' lvalue
// |       |-ImplicitCastExpr 0x56b8bef7d558 <col:9> 'int *' <LValueToRValue>
// |       | `-DeclRefExpr 0x56b8bef7d518 <col:9> 'int *' lvalue ParmVar 0x56b8bef7d038 'x' 'int *'
// |       `-IntegerLiteral 0x56b8bef7d538 <col:11> 'int' 0
// |-AnnotateAttr 0x56b8bef7d258 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (x |-> v)|END"
// |-AnnotateAttr 0x56b8bef7d310 <line:5:22, col:58> pulse "returns:Int32.t|END"
// `-AnnotateAttr 0x56b8bef7d380 <line:4:24, col:71> pulse "ensures:exists* v. (x |-> v)|END"
