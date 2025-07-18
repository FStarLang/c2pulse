module Issue16_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn incr
(r : ref Int32.t)
(#vr:erased _)
requires r |-> vr
requires pure Pulse.Lib.C.Int32.(fits (+) (as_int vr) 1)
ensures exists* w. (r |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == as_int vr + 1)
{
let mut r : (ref Int32.t) = r;
(! r) := (Int32.add (! (! r)) 1l);
}

fn one ()
requires emp
returns i:Pulse.Lib.C.Int32.int32
ensures pure Pulse.Lib.C.Int32.(as_int i == 1)
{
let i0 : Int32.t = 0l;
let mut i : Int32.t = i0;
(incr (i));
(! i);
}

//Dumping the Clang AST.
// FunctionDecl 0x5c00ef7c1638 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue16_test.c:7:1, line:10:1> line:7:5 used incr 'int (int *)'
// |-ParmVarDecl 0x5c00ef7c1568 <col:11, col:16> col:16 used r 'int *'
// |-CompoundStmt 0x5c00ef7c1ab0 <line:8:1, line:10:1>
// | `-BinaryOperator 0x5c00ef7c1a90 <line:9:5, col:15> 'int' '='
// |   |-UnaryOperator 0x5c00ef7c19d0 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x5c00ef7c19b8 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5c00ef7c1998 <col:6> 'int *' lvalue ParmVar 0x5c00ef7c1568 'r' 'int *'
// |   `-BinaryOperator 0x5c00ef7c1a70 <col:10, col:15> 'int' '+'
// |     |-ImplicitCastExpr 0x5c00ef7c1a58 <col:10, col:11> 'int' <LValueToRValue>
// |     | `-UnaryOperator 0x5c00ef7c1a20 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// |     |   `-ImplicitCastExpr 0x5c00ef7c1a08 <col:11> 'int *' <LValueToRValue>
// |     |     `-DeclRefExpr 0x5c00ef7c19e8 <col:11> 'int *' lvalue ParmVar 0x5c00ef7c1568 'r' 'int *'
// |     `-IntegerLiteral 0x5c00ef7c1a38 <col:15> 'int' 1
// |-AnnotateAttr 0x5c00ef7c16e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#vr:erased _|END"
// |-AnnotateAttr 0x5c00ef7c1790 <line:3:23, col:61> pulse "requires:r |-> vr|END"
// |-AnnotateAttr 0x5c00ef7c1800 <col:23, col:61> pulse "requires:pure Pulse.Lib.C.Int32.(fits (+) (as_int vr) 1)|END"
// `-AnnotateAttr 0x5c00ef7c18a0 <line:4:24, col:71> pulse "ensures:exists* w. (r |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == as_int vr + 1)|END"
// FunctionDecl 0x5c00ef7c1bf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue16_test.c:15:1, line:20:1> line:15:5 one 'int ()'
// |-CompoundStmt 0x5c00ef7c2020 <line:16:1, line:20:1>
// | |-DeclStmt 0x5c00ef7c1ef0 <line:17:5, col:14>
// | | `-VarDecl 0x5c00ef7c1e68 <col:5, col:13> col:9 used i 'int' cinit
// | |   `-IntegerLiteral 0x5c00ef7c1ed0 <col:13> 'int' 0
// | |-CallExpr 0x5c00ef7c1fa8 <line:18:5, col:12> 'int'
// | | |-ImplicitCastExpr 0x5c00ef7c1f90 <col:5> 'int (*)(int *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x5c00ef7c1f08 <col:5> 'int (int *)' Function 0x5c00ef7c1638 'incr' 'int (int *)'
// | | `-UnaryOperator 0x5c00ef7c1f48 <col:10, col:11> 'int *' prefix '&' cannot overflow
// | |   `-DeclRefExpr 0x5c00ef7c1f28 <col:11> 'int' lvalue Var 0x5c00ef7c1e68 'i' 'int'
// | `-ReturnStmt 0x5c00ef7c2010 <line:19:5, col:12>
// |   `-ImplicitCastExpr 0x5c00ef7c1ff8 <col:12> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x5c00ef7c1fd8 <col:12> 'int' lvalue Var 0x5c00ef7c1e68 'i' 'int'
// |-AnnotateAttr 0x5c00ef7c1ca0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5c00ef7c1d40 <line:5:22, col:58> pulse "returns:i:Pulse.Lib.C.Int32.int32|END"
// `-AnnotateAttr 0x5c00ef7c1dc0 <line:4:24, col:71> pulse "ensures:pure Pulse.Lib.C.Int32.(as_int i == 1)|END"
