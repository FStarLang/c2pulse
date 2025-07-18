module Issue1_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn incr
(x : ref Int32.t)
requires x |-> 'i
requires pure FStar.Int32.(fits (v 'i + 1))
ensures exists* j. (x |-> j) ** pure FStar.Int32.((v j <: int) == v 'i + 1)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) 1l);
}

fn incr_frame
(x : ref Int32.t)
(y : ref Int32.t)
(#i:_)
(#j:_)
requires x |-> i
requires y |-> j
requires pure FStar.Int32.(fits (v i + 1))
ensures exists* k. (x |-> k) ** pure FStar.Int32.(v i + 1 == v k)
ensures y |-> j
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
(incr (! x));
}

//Dumping the Clang AST.
// FunctionDecl 0x55e750d509d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue1_test.c:8:1, line:11:1> line:8:6 used incr 'void (int *)'
// |-ParmVarDecl 0x55e750d50908 <col:12, col:17> col:17 used x 'int *'
// |-CompoundStmt 0x55e750d50d78 <line:9:1, line:11:1>
// | `-BinaryOperator 0x55e750d50d58 <line:10:5, col:15> 'int' '='
// |   |-UnaryOperator 0x55e750d50c98 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x55e750d50c80 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x55e750d50c60 <col:6> 'int *' lvalue ParmVar 0x55e750d50908 'x' 'int *'
// |   `-BinaryOperator 0x55e750d50d38 <col:10, col:15> 'int' '+'
// |     |-ImplicitCastExpr 0x55e750d50d20 <col:10, col:11> 'int' <LValueToRValue>
// |     | `-UnaryOperator 0x55e750d50ce8 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// |     |   `-ImplicitCastExpr 0x55e750d50cd0 <col:11> 'int *' <LValueToRValue>
// |     |     `-DeclRefExpr 0x55e750d50cb0 <col:11> 'int *' lvalue ParmVar 0x55e750d50908 'x' 'int *'
// |     `-IntegerLiteral 0x55e750d50d00 <col:15> 'int' 1
// |-AnnotateAttr 0x55e750d50a80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'i"|END"
// |-AnnotateAttr 0x55e750d50b20 <col:23, col:61> pulse "requires:"pure FStar.Int32.(fits (v 'i + 1))"|END"
// `-AnnotateAttr 0x55e750d50bb0 <line:4:24, col:71> pulse "ensures:"exists* j. (x |-> j) ** pure FStar.Int32.((v j <: int) == v 'i + 1)"|END"
// FunctionDecl 0x55e750d510d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue1_test.c:20:1, line:23:1> line:20:6 incr_frame 'void (int *, int *)'
// |-ParmVarDecl 0x55e750d50f78 <col:17, col:22> col:22 used x 'int *'
// |-ParmVarDecl 0x55e750d50ff8 <col:25, col:30> col:30 y 'int *'
// |-CompoundStmt 0x55e750d53620 <line:21:1, line:23:1>
// | `-CallExpr 0x55e750d535d8 <line:22:5, col:11> 'void'
// |   |-ImplicitCastExpr 0x55e750d535c0 <col:5> 'void (*)(int *)' <FunctionToPointerDecay>
// |   | `-DeclRefExpr 0x55e750d53550 <col:5> 'void (int *)' Function 0x55e750d509d0 'incr' 'void (int *)'
// |   `-ImplicitCastExpr 0x55e750d53608 <col:10> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x55e750d53570 <col:10> 'int *' lvalue ParmVar 0x55e750d50f78 'x' 'int *'
// |-AnnotateAttr 0x55e750d51188 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#i:_|END"
// |-AnnotateAttr 0x55e750d53260 <col:27, col:68> pulse "erased_arg:#j:_|END"
// |-AnnotateAttr 0x55e750d532d0 <line:3:23, col:61> pulse "requires:x |-> i|END"
// |-AnnotateAttr 0x55e750d53340 <col:23, col:61> pulse "requires:y |-> j|END"
// |-AnnotateAttr 0x55e750d533b0 <col:23, col:61> pulse "requires:pure FStar.Int32.(fits (v i + 1))|END"
// |-AnnotateAttr 0x55e750d53440 <line:4:24, col:71> pulse "ensures:exists* k. (x |-> k) ** pure FStar.Int32.(v i + 1 == v k)|END"
// `-AnnotateAttr 0x55e750d534e0 <col:24, col:71> pulse "ensures:y |-> j|END"
