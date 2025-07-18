module Pulse_tutorial_intro

#lang-pulse

open Pulse
open Pulse.Lib.C



fn incr
(x : ref Int32.t)
requires x |-> 'i
requires pure (fits (+) (as_int 'i) 1)
ensures exists* j. (x |-> j) ** pure (as_int 'i + 1 == as_int j)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) 1l);
}

fn incr_explicit_i
(x : ref Int32.t)
(#i:_)
requires x |-> i
requires pure (fits (+) (as_int i) 1)
ensures exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
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
requires pure (fits (+) (as_int i) 1)
ensures exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
ensures y |-> j
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
(incr (! x));
}

//Dumping the Clang AST.
// FunctionDecl 0x5da53695b9d0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_intro.c:8:1, line:11:1> line:8:6 used incr 'void (int *)'
// |-ParmVarDecl 0x5da53695b900 <col:12, col:17> col:17 used x 'int *'
// |-CompoundStmt 0x5da53695bd68 <line:9:1, line:11:1>
// | `-BinaryOperator 0x5da53695bd48 <line:10:5, col:15> 'int' '='
// |   |-UnaryOperator 0x5da53695bc88 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x5da53695bc70 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5da53695bc50 <col:6> 'int *' lvalue ParmVar 0x5da53695b900 'x' 'int *'
// |   `-BinaryOperator 0x5da53695bd28 <col:10, col:15> 'int' '+'
// |     |-ImplicitCastExpr 0x5da53695bd10 <col:10, col:11> 'int' <LValueToRValue>
// |     | `-UnaryOperator 0x5da53695bcd8 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// |     |   `-ImplicitCastExpr 0x5da53695bcc0 <col:11> 'int *' <LValueToRValue>
// |     |     `-DeclRefExpr 0x5da53695bca0 <col:11> 'int *' lvalue ParmVar 0x5da53695b900 'x' 'int *'
// |     `-IntegerLiteral 0x5da53695bcf0 <col:15> 'int' 1
// |-AnnotateAttr 0x5da53695ba80 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'i"|END"
// |-AnnotateAttr 0x5da53695bb20 <col:23, col:61> pulse "requires:"pure (fits (+) (as_int 'i) 1)"|END"
// `-AnnotateAttr 0x5da53695bbb0 <line:4:24, col:71> pulse "ensures:"exists* j. (x |-> j) ** pure (as_int 'i + 1 == as_int j)"|END"
// FunctionDecl 0x5da53695bf50 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_intro.c:17:1, line:20:1> line:17:6 incr_explicit_i 'void (int *)'
// |-ParmVarDecl 0x5da53695bec0 <col:23, col:28> col:28 used x 'int *'
// |-CompoundStmt 0x5da53695e3d8 <line:18:1, line:20:1>
// | `-BinaryOperator 0x5da53695e3b8 <line:19:5, col:15> 'int' '='
// |   |-UnaryOperator 0x5da53695e2f8 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x5da53695e2e0 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5da53695e2c0 <col:6> 'int *' lvalue ParmVar 0x5da53695bec0 'x' 'int *'
// |   `-BinaryOperator 0x5da53695e398 <col:10, col:15> 'int' '+'
// |     |-ImplicitCastExpr 0x5da53695e380 <col:10, col:11> 'int' <LValueToRValue>
// |     | `-UnaryOperator 0x5da53695e348 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// |     |   `-ImplicitCastExpr 0x5da53695e330 <col:11> 'int *' <LValueToRValue>
// |     |     `-DeclRefExpr 0x5da53695e310 <col:11> 'int *' lvalue ParmVar 0x5da53695bec0 'x' 'int *'
// |     `-IntegerLiteral 0x5da53695e360 <col:15> 'int' 1
// |-AnnotateAttr 0x5da53695c000 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#i:_|END"
// |-AnnotateAttr 0x5da53695c0a0 <line:3:23, col:61> pulse "requires:x |-> i|END"
// |-AnnotateAttr 0x5da53695c110 <col:23, col:61> pulse "requires:pure (fits (+) (as_int i) 1)|END"
// `-AnnotateAttr 0x5da53695e220 <line:4:24, col:71> pulse "ensures:"exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)"|END"
// FunctionDecl 0x5da53695e730 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_intro.c:29:1, line:32:1> line:29:6 incr_frame 'void (int *, int *)'
// |-ParmVarDecl 0x5da53695e5d8 <col:17, col:22> col:22 used x 'int *'
// |-ParmVarDecl 0x5da53695e658 <col:25, col:30> col:30 y 'int *'
// |-CompoundStmt 0x5da53695ec50 <line:30:1, line:32:1>
// | `-CallExpr 0x5da53695ec08 <line:31:5, col:11> 'void'
// |   |-ImplicitCastExpr 0x5da53695ebf0 <col:5> 'void (*)(int *)' <FunctionToPointerDecay>
// |   | `-DeclRefExpr 0x5da53695eb80 <col:5> 'void (int *)' Function 0x5da53695b9d0 'incr' 'void (int *)'
// |   `-ImplicitCastExpr 0x5da53695ec38 <col:10> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x5da53695eba0 <col:10> 'int *' lvalue ParmVar 0x5da53695e5d8 'x' 'int *'
// |-AnnotateAttr 0x5da53695e7e8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#i:_|END"
// |-AnnotateAttr 0x5da53695e890 <col:27, col:68> pulse "erased_arg:#j:_|END"
// |-AnnotateAttr 0x5da53695e900 <line:3:23, col:61> pulse "requires:x |-> i|END"
// |-AnnotateAttr 0x5da53695e970 <col:23, col:61> pulse "requires:y |-> j|END"
// |-AnnotateAttr 0x5da53695e9e0 <col:23, col:61> pulse "requires:pure (fits (+) (as_int i) 1)|END"
// |-AnnotateAttr 0x5da53695ea70 <line:4:24, col:71> pulse "ensures:"exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)"|END"
// `-AnnotateAttr 0x5da53695eb10 <col:24, col:71> pulse "ensures:y |-> j|END"
