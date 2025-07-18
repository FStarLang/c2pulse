module Issue52_test_2

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
let idx0 : Int32.t = 0l;
let mut idx : Int32.t = idx0;
pts_to_len !x;
(op_Array_Access (! x) (int32_to_sizet (! idx)));
}

//Dumping the Clang AST.
// FunctionDecl 0x5bcf22e4af38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test_2.c:6:1, line:10:1> line:6:5 fake_arr 'int (int *)'
// |-ParmVarDecl 0x5bcf22e4adc8 <col:25, col:30> col:30 used x 'int *'
// | `-AnnotateAttr 0x5bcf22e4ae30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:5|END"
// |-CompoundStmt 0x5bcf22e4b410 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test_2.c:6:33, line:10:1>
// | |-DeclStmt 0x5bcf22e4b278 <line:7:9, col:20>
// | | `-VarDecl 0x5bcf22e4b1f0 <col:9, col:19> col:13 used idx 'int' cinit
// | |   `-IntegerLiteral 0x5bcf22e4b258 <col:19> 'int' 0
// | |-AttributedStmt 0x5bcf22e4b340 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test_2.c:8:29>
// | | |-AnnotateAttr 0x5bcf22e4b2d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !x|END"
// | | `-NullStmt 0x5bcf22e4b2c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue52_test_2.c:8:29>
// | `-ReturnStmt 0x5bcf22e4b400 <line:9:9, col:21>
// |   `-ImplicitCastExpr 0x5bcf22e4b3e8 <col:16, col:21> 'int' <LValueToRValue>
// |     `-ArraySubscriptExpr 0x5bcf22e4b3c8 <col:16, col:21> 'int' lvalue
// |       |-ImplicitCastExpr 0x5bcf22e4b398 <col:16> 'int *' <LValueToRValue>
// |       | `-DeclRefExpr 0x5bcf22e4b358 <col:16> 'int *' lvalue ParmVar 0x5bcf22e4adc8 'x' 'int *'
// |       `-ImplicitCastExpr 0x5bcf22e4b3b0 <col:18> 'int' <LValueToRValue>
// |         `-DeclRefExpr 0x5bcf22e4b378 <col:18> 'int' lvalue Var 0x5bcf22e4b1f0 'idx' 'int'
// |-AnnotateAttr 0x5bcf22e4afe8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (x |-> v)|END"
// |-AnnotateAttr 0x5bcf22e4b0a0 <line:5:22, col:58> pulse "returns:Int32.t|END"
// `-AnnotateAttr 0x5bcf22e4b110 <line:4:24, col:71> pulse "ensures:exists* v. (x |-> v)|END"
