module Pulse_tutorial_existentials

#lang-pulse

open Pulse
open Pulse.Lib.C



fn assign
(r : ref Int32.t)
(v : Int32.t)
requires exists* s. r |-> s
ensures r |-> v
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
(! r) := (! v);
}

fn make_even
(x : ref Int32.t)
requires exists* s. (x |-> s) ** pure (fits ( * ) 2 (as_int s))
ensures exists* s. (x |-> s) ** pure (as_int s % 2 == 0)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) (! (! x)));
}

[@@expect_failure]
fn make_even_explicit
(x : ref Int32.t)
requires exists* w0. (x |-> w0) ** pure (fits ( * ) 2 (as_int w0))
ensures exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)
{
let mut x : (ref Int32.t) = x;
with w0. assert (x |-> w0);
(! x) := (Int32.add (! (! x)) (! (! x)));
introduce exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0) with (w0 +^ w0);
}

[@@expect_failure]
fn make_even_explicit_alt
(x : ref Int32.t)
(y : ref Int32.t)
requires exists* wx wy. (x |-> wx) ** (y |-> wy) ** pure (as_int wx % 2 == as_int wy % 2) ** pure (fits (+) (as_int wx) (as_int wy))
ensures exists* wx wy. (x |-> wx) ** (y |-> wy)
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
with wx wy. assert ((x |-> wx) ** (y |-> wy));
(! x) := (Int32.add (! (! x)) (! (! y)));
introduce exists* nx ny. (x |-> nx) ** (y |-> ny) ** pure (as_int nx % 2 == 0) with (wx +^ wy) wy;
}

fn call_make_even
(x : ref Int32.t)
(#v:erased _)
requires x |-> v
requires pure (fits (+) (as_int v) (as_int v))
ensures exists* w. (x |-> w) ** pure (as_int w % 2 == 0)
{
let mut x : (ref Int32.t) = x;
(make_even (! x));
}

//Dumping the Clang AST.
// FunctionDecl 0x58bfc35e8d40 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:7:1, line:10:1> line:7:6 assign 'void (int *, int)'
// |-ParmVarDecl 0x58bfc35e8be8 <col:14, col:19> col:19 used r 'int *'
// |-ParmVarDecl 0x58bfc35e8c68 <col:22, col:26> col:26 used v 'int'
// |-CompoundStmt 0x58bfc35e8fb8 <line:8:1, line:10:1>
// | `-BinaryOperator 0x58bfc35e8f98 <line:9:5, col:10> 'int' '='
// |   |-UnaryOperator 0x58bfc35e8f48 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x58bfc35e8f30 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x58bfc35e8f10 <col:6> 'int *' lvalue ParmVar 0x58bfc35e8be8 'r' 'int *'
// |   `-ImplicitCastExpr 0x58bfc35e8f80 <col:10> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x58bfc35e8f60 <col:10> 'int' lvalue ParmVar 0x58bfc35e8c68 'v' 'int'
// |-AnnotateAttr 0x58bfc35e8df8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* s. r |-> s|END"
// `-AnnotateAttr 0x58bfc35e8ea0 <line:4:24, col:71> pulse "ensures:r |-> v|END"
// FunctionDecl 0x58bfc35e9180 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:14:1, line:17:1> line:14:6 used make_even 'void (int *)'
// |-ParmVarDecl 0x58bfc35e90b0 <col:16, col:21> col:21 used x 'int *'
// |-CompoundStmt 0x58bfc35e9500 <line:15:1, line:17:1>
// | `-BinaryOperator 0x58bfc35e94e0 <line:16:5, col:16> 'int' '='
// |   |-UnaryOperator 0x58bfc35e93d8 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x58bfc35e93c0 <col:6> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x58bfc35e93a0 <col:6> 'int *' lvalue ParmVar 0x58bfc35e90b0 'x' 'int *'
// |   `-BinaryOperator 0x58bfc35e94c0 <col:10, col:16> 'int' '+'
// |     |-ImplicitCastExpr 0x58bfc35e9490 <col:10, col:11> 'int' <LValueToRValue>
// |     | `-UnaryOperator 0x58bfc35e9428 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// |     |   `-ImplicitCastExpr 0x58bfc35e9410 <col:11> 'int *' <LValueToRValue>
// |     |     `-DeclRefExpr 0x58bfc35e93f0 <col:11> 'int *' lvalue ParmVar 0x58bfc35e90b0 'x' 'int *'
// |     `-ImplicitCastExpr 0x58bfc35e94a8 <col:15, col:16> 'int' <LValueToRValue>
// |       `-UnaryOperator 0x58bfc35e9478 <col:15, col:16> 'int' lvalue prefix '*' cannot overflow
// |         `-ImplicitCastExpr 0x58bfc35e9460 <col:16> 'int *' <LValueToRValue>
// |           `-DeclRefExpr 0x58bfc35e9440 <col:16> 'int *' lvalue ParmVar 0x58bfc35e90b0 'x' 'int *'
// |-AnnotateAttr 0x58bfc35e9230 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* s. (x |-> s) ** pure (fits ( * ) 2 (as_int s))|END"
// `-AnnotateAttr 0x58bfc35e9300 <line:4:24, col:71> pulse "ensures:exists* s. (x |-> s) ** pure (as_int s % 2 == 0)|END"
// FunctionDecl 0x58bfc35ebb48 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:23:1, line:32:1> line:23:6 make_even_explicit 'void (int *)'
// |-ParmVarDecl 0x58bfc35ebab8 <col:26, col:31> col:31 used x 'int *'
// |-CompoundStmt 0x58bfc35ec178 <line:24:1, line:32:1>
// | |-AttributedStmt 0x58bfc35ebeb0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:25:36>
// | | |-AnnotateAttr 0x58bfc35ebe30 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with w0. assert (x |-> w0)|END"
// | | `-NullStmt 0x58bfc35ebe28 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:25:36>
// | |-BinaryOperator 0x58bfc35ec008 <line:26:3, col:14> 'int' '='
// | | |-UnaryOperator 0x58bfc35ebf00 <col:3, col:4> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x58bfc35ebee8 <col:4> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x58bfc35ebec8 <col:4> 'int *' lvalue ParmVar 0x58bfc35ebab8 'x' 'int *'
// | | `-BinaryOperator 0x58bfc35ebfe8 <col:8, col:14> 'int' '+'
// | |   |-ImplicitCastExpr 0x58bfc35ebfb8 <col:8, col:9> 'int' <LValueToRValue>
// | |   | `-UnaryOperator 0x58bfc35ebf50 <col:8, col:9> 'int' lvalue prefix '*' cannot overflow
// | |   |   `-ImplicitCastExpr 0x58bfc35ebf38 <col:9> 'int *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x58bfc35ebf18 <col:9> 'int *' lvalue ParmVar 0x58bfc35ebab8 'x' 'int *'
// | |   `-ImplicitCastExpr 0x58bfc35ebfd0 <col:13, col:14> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x58bfc35ebfa0 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x58bfc35ebf88 <col:14> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x58bfc35ebf68 <col:14> 'int *' lvalue ParmVar 0x58bfc35ebab8 'x' 'int *'
// | `-AttributedStmt 0x58bfc35ec160 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:31:4>
// |   |-AnnotateAttr 0x58bfc35ec0a8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:introduce exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0) with (w0 +^ w0)|END"
// |   `-NullStmt 0x58bfc35ec0a0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:31:4>
// |-AnnotateAttr 0x58bfc35ebbf8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* w0. (x |-> w0) ** pure (fits ( * ) 2 (as_int w0))|END"
// |-AnnotateAttr 0x58bfc35ebcd0 <line:4:24, col:71> pulse "ensures:exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)|END"
// `-AnnotateAttr 0x58bfc35ebd70 <line:13:5, col:59> pulse "expect_failure:|END"
// FunctionDecl 0x58bfc35ec450 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:37:1, line:44:1> line:37:6 make_even_explicit_alt 'void (int *, int *)'
// |-ParmVarDecl 0x58bfc35ec2f0 <col:30, col:35> col:35 used x 'int *'
// |-ParmVarDecl 0x58bfc35ec370 <col:38, col:43> col:43 used y 'int *'
// |-CompoundStmt 0x58bfc35ed528 <line:38:1, line:44:1>
// | |-AttributedStmt 0x58bfc35ec810 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:39:55>
// | | |-AnnotateAttr 0x58bfc35ec780 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with wx wy. assert ((x |-> wx) ** (y |-> wy))|END"
// | | `-NullStmt 0x58bfc35ec778 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:39:55>
// | |-BinaryOperator 0x58bfc35ec968 <line:40:3, col:14> 'int' '='
// | | |-UnaryOperator 0x58bfc35ec860 <col:3, col:4> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x58bfc35ec848 <col:4> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x58bfc35ec828 <col:4> 'int *' lvalue ParmVar 0x58bfc35ec2f0 'x' 'int *'
// | | `-BinaryOperator 0x58bfc35ec948 <col:8, col:14> 'int' '+'
// | |   |-ImplicitCastExpr 0x58bfc35ec918 <col:8, col:9> 'int' <LValueToRValue>
// | |   | `-UnaryOperator 0x58bfc35ec8b0 <col:8, col:9> 'int' lvalue prefix '*' cannot overflow
// | |   |   `-ImplicitCastExpr 0x58bfc35ec898 <col:9> 'int *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x58bfc35ec878 <col:9> 'int *' lvalue ParmVar 0x58bfc35ec2f0 'x' 'int *'
// | |   `-ImplicitCastExpr 0x58bfc35ec930 <col:13, col:14> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x58bfc35ec900 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x58bfc35ec8e8 <col:14> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x58bfc35ec8c8 <col:14> 'int *' lvalue ParmVar 0x58bfc35ec370 'y' 'int *'
// | `-AttributedStmt 0x58bfc35ed510 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:43:28>
// |   |-AnnotateAttr 0x58bfc35ed448 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:introduce exists* nx ny. (x |-> nx) ** (y |-> ny) ** pure (as_int nx % 2 == 0) with (wx +^ wy) wy|END"
// |   `-NullStmt 0x58bfc35ed440 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:43:28>
// |-AnnotateAttr 0x58bfc35ec508 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
// |-AnnotateAttr 0x58bfc35ec5b0 <line:3:23, col:61> pulse "requires:exists* wx wy. (x |-> wx) ** (y |-> wy) ** pure (as_int wx % 2 == as_int wy % 2) ** pure (fits (+) (as_int wx) (as_int wy))|END"
// `-AnnotateAttr 0x58bfc35ec690 <line:4:24, col:71> pulse "ensures:exists* wx wy. (x |-> wx) ** (y |-> wy)|END"
// FunctionDecl 0x58bfc35ed728 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_existentials.c:50:1, line:53:1> line:50:6 call_make_even 'void (int *)'
// |-ParmVarDecl 0x58bfc35ed698 <col:22, col:27> col:27 used x 'int *'
// |-CompoundStmt 0x58bfc35edaf0 <line:51:1, line:53:1>
// | `-CallExpr 0x58bfc35edaa8 <line:52:3, col:14> 'void'
// |   |-ImplicitCastExpr 0x58bfc35eda90 <col:3> 'void (*)(int *)' <FunctionToPointerDecay>
// |   | `-DeclRefExpr 0x58bfc35eda20 <col:3> 'void (int *)' Function 0x58bfc35e9180 'make_even' 'void (int *)'
// |   `-ImplicitCastExpr 0x58bfc35edad8 <col:13> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x58bfc35eda40 <col:13> 'int *' lvalue ParmVar 0x58bfc35ed698 'x' 'int *'
// |-AnnotateAttr 0x58bfc35ed7d8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#v:erased _|END"
// |-AnnotateAttr 0x58bfc35ed880 <line:3:23, col:61> pulse "requires:x |-> v|END"
// |-AnnotateAttr 0x58bfc35ed8f0 <col:23, col:61> pulse "requires:pure (fits (+) (as_int v) (as_int v))|END"
// `-AnnotateAttr 0x58bfc35ed980 <line:4:24, col:71> pulse "ensures:exists* w. (x |-> w) ** pure (as_int w % 2 == 0)|END"
