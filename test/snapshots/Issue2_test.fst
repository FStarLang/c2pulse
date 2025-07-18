module Issue2_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn value_of_explicit
(r : ref Int32.t)
(#w:_)
requires r |-> w
returns v:Int32.t
ensures r |-> w
ensures pure (v == w)
{
let mut r : (ref Int32.t) = r;
(! (! r));
}

//Dumping the Clang AST.
// FunctionDecl 0x5930ced4f128 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue2_test.c:8:1, line:11:1> line:8:5 value_of_explicit 'int (int *)'
// |-ParmVarDecl 0x5930ced4f058 <col:23, col:28> col:28 used r 'int *'
// |-CompoundStmt 0x5930ced4f510 <line:9:1, line:11:1>
// | `-ReturnStmt 0x5930ced4f500 <line:10:5, col:13>
// |   `-ImplicitCastExpr 0x5930ced4f4e8 <col:12, col:13> 'int' <LValueToRValue>
// |     `-UnaryOperator 0x5930ced4f4d0 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |       `-ImplicitCastExpr 0x5930ced4f4b8 <col:13> 'int *' <LValueToRValue>
// |         `-DeclRefExpr 0x5930ced4f498 <col:13> 'int *' lvalue ParmVar 0x5930ced4f058 'r' 'int *'
// |-AnnotateAttr 0x5930ced4f1d8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:_|END"
// |-AnnotateAttr 0x5930ced4f280 <line:3:23, col:61> pulse "requires:r |-> w|END"
// |-AnnotateAttr 0x5930ced4f2f0 <line:5:22, col:58> pulse "returns:v:Int32.t|END"
// |-AnnotateAttr 0x5930ced4f360 <line:4:24, col:71> pulse "ensures:r |-> w|END"
// `-AnnotateAttr 0x5930ced4f3d0 <col:24, col:71> pulse "ensures:pure (v == w)|END"
