module Test_logicals_int

#lang-pulse

open Pulse
open Pulse.Lib.C



fn f1 ()
returns Int32.t
{
let x0 : Int32.t = 11l;
let mut x : Int32.t = x0;
let y1 : Int32.t = 10l;
let mut y : Int32.t = y1;
let z2 : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z2;
(! z);
}

fn f2 ()
returns Int32.t
{
let x3 : Int32.t = 10l;
let mut x : Int32.t = x3;
let y4 : Int32.t = 11l;
let mut y : Int32.t = y4;
let z5 : Int32.t = (bool_to_int32 (op_BarBar (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z5;
(! z);
}

fn f3 ()
returns Int32.t
{
let x6 : Int32.t = 10l;
let mut x : Int32.t = x6;
let y7 : Int32.t = 20l;
let mut y : Int32.t = y7;
(Int32.add (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! x))))))) (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! y))))))));
}

fn f4 ()
returns Int32.t
{
let x8 : Int32.t = (Int32.sub 0l 10l);
let mut x : Int32.t = x8;
let y9 : Int32.t = 10l;
let mut y : Int32.t = y9;
let z10 : Int32.t = (bool_to_int32 (not (int32_to_bool ((bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))))))));
let mut z : Int32.t = z10;
(! z);
}

fn f5 ()
returns Int32.t
{
let x11 : Int32.t = (Int32.sub 0l 100l);
let mut x : Int32.t = x11;
let y12 : Int32.t = 100l;
let mut y : Int32.t = y12;
let z13 : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z13;
(! z);
}

[@@expect_failure]
fn f6 ()
returns Int32.t
{
let x14 : UInt32.t = (int32_to_uint32 100l);
let mut x : UInt32.t = x14;
let y15 : Int32.t = (uint32_to_int32 (UInt32.sub 0ul (! x)));
let mut y : Int32.t = y15;
let z16 : Int32.t = (bool_to_int32 (op_AmpAmp (uint32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z16;
(! z);
}

//Dumping the Clang AST.
// FunctionDecl 0x64e7829de218 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:6:1, line:12:1> line:6:5 f1 'int ()'
// |-CompoundStmt 0x64e7829de640 <col:9, line:12:1>
// | |-DeclStmt 0x64e7829de400 <line:8:3, col:13>
// | | `-VarDecl 0x64e7829de378 <col:3, col:11> col:7 used x 'int' cinit
// | |   `-IntegerLiteral 0x64e7829de3e0 <col:11> 'int' 11
// | |-DeclStmt 0x64e7829de4b8 <line:9:3, col:13>
// | | `-VarDecl 0x64e7829de430 <col:3, col:11> col:7 used y 'int' cinit
// | |   `-IntegerLiteral 0x64e7829de498 <col:11> 'int' 10
// | |-DeclStmt 0x64e7829de5e0 <line:10:3, col:17>
// | | `-VarDecl 0x64e7829de4e8 <col:3, col:16> col:7 used z 'int' cinit
// | |   `-BinaryOperator 0x64e7829de5c0 <col:11, col:16> 'int' '&&'
// | |     |-ImplicitCastExpr 0x64e7829de590 <col:11> 'int' <LValueToRValue>
// | |     | `-DeclRefExpr 0x64e7829de550 <col:11> 'int' lvalue Var 0x64e7829de378 'x' 'int'
// | |     `-ImplicitCastExpr 0x64e7829de5a8 <col:16> 'int' <LValueToRValue>
// | |       `-DeclRefExpr 0x64e7829de570 <col:16> 'int' lvalue Var 0x64e7829de430 'y' 'int'
// | `-ReturnStmt 0x64e7829de630 <line:11:3, col:10>
// |   `-ImplicitCastExpr 0x64e7829de618 <col:10> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64e7829de5f8 <col:10> 'int' lvalue Var 0x64e7829de4e8 'z' 'int'
// `-AnnotateAttr 0x64e7829de2c0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x64e7829de6d0 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:15:1, line:21:1> line:15:5 f2 'int ()'
// |-CompoundStmt 0x64e7829deb00 <col:9, line:21:1>
// | |-DeclStmt 0x64e7829de8c0 <line:17:3, col:13>
// | | `-VarDecl 0x64e7829de838 <col:3, col:11> col:7 used x 'int' cinit
// | |   `-IntegerLiteral 0x64e7829de8a0 <col:11> 'int' 10
// | |-DeclStmt 0x64e7829de978 <line:18:3, col:13>
// | | `-VarDecl 0x64e7829de8f0 <col:3, col:11> col:7 used y 'int' cinit
// | |   `-IntegerLiteral 0x64e7829de958 <col:11> 'int' 11
// | |-DeclStmt 0x64e7829deaa0 <line:19:3, col:17>
// | | `-VarDecl 0x64e7829de9a8 <col:3, col:16> col:7 used z 'int' cinit
// | |   `-BinaryOperator 0x64e7829dea80 <col:11, col:16> 'int' '||'
// | |     |-ImplicitCastExpr 0x64e7829dea50 <col:11> 'int' <LValueToRValue>
// | |     | `-DeclRefExpr 0x64e7829dea10 <col:11> 'int' lvalue Var 0x64e7829de838 'x' 'int'
// | |     `-ImplicitCastExpr 0x64e7829dea68 <col:16> 'int' <LValueToRValue>
// | |       `-DeclRefExpr 0x64e7829dea30 <col:16> 'int' lvalue Var 0x64e7829de8f0 'y' 'int'
// | `-ReturnStmt 0x64e7829deaf0 <line:20:3, col:10>
// |   `-ImplicitCastExpr 0x64e7829dead8 <col:10> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64e7829deab8 <col:10> 'int' lvalue Var 0x64e7829de9a8 'z' 'int'
// `-AnnotateAttr 0x64e7829de778 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x64e7829deb90 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:24:1, line:29:1> line:24:5 f3 'int ()'
// |-CompoundStmt 0x64e7829e03b0 <col:9, line:29:1>
// | |-DeclStmt 0x64e7829e01e0 <line:26:2, col:12>
// | | `-VarDecl 0x64e7829e0158 <col:2, col:10> col:6 used x 'int' cinit
// | |   `-IntegerLiteral 0x64e7829e01c0 <col:10> 'int' 10
// | |-DeclStmt 0x64e7829e0298 <line:27:2, col:12>
// | | `-VarDecl 0x64e7829e0210 <col:2, col:10> col:6 used y 'int' cinit
// | |   `-IntegerLiteral 0x64e7829e0278 <col:10> 'int' 20
// | `-ReturnStmt 0x64e7829e03a0 <line:28:2, col:17>
// |   `-BinaryOperator 0x64e7829e0380 <col:9, col:17> 'int' '+'
// |     |-UnaryOperator 0x64e7829e0300 <col:9, col:11> 'int' prefix '!' cannot overflow
// |     | `-UnaryOperator 0x64e7829e02e8 <col:10, col:11> 'int' prefix '!' cannot overflow
// |     |   `-ImplicitCastExpr 0x64e7829e02d0 <col:11> 'int' <LValueToRValue>
// |     |     `-DeclRefExpr 0x64e7829e02b0 <col:11> 'int' lvalue Var 0x64e7829e0158 'x' 'int'
// |     `-UnaryOperator 0x64e7829e0368 <col:15, col:17> 'int' prefix '!' cannot overflow
// |       `-UnaryOperator 0x64e7829e0350 <col:16, col:17> 'int' prefix '!' cannot overflow
// |         `-ImplicitCastExpr 0x64e7829e0338 <col:17> 'int' <LValueToRValue>
// |           `-DeclRefExpr 0x64e7829e0318 <col:17> 'int' lvalue Var 0x64e7829e0210 'y' 'int'
// `-AnnotateAttr 0x64e7829e00a0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x64e7829e0438 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:32:1, line:37:1> line:32:5 f4 'int ()'
// |-CompoundStmt 0x64e7829e08b0 <col:9, line:37:1>
// | |-DeclStmt 0x64e7829e0638 <line:33:2, col:13>
// | | `-VarDecl 0x64e7829e0598 <col:2, col:11> col:6 used x 'int' cinit
// | |   `-UnaryOperator 0x64e7829e0620 <col:10, col:11> 'int' prefix '-'
// | |     `-IntegerLiteral 0x64e7829e0600 <col:11> 'int' 10
// | |-DeclStmt 0x64e7829e06f0 <line:34:2, col:12>
// | | `-VarDecl 0x64e7829e0668 <col:2, col:10> col:6 used y 'int' cinit
// | |   `-IntegerLiteral 0x64e7829e06d0 <col:10> 'int' 10
// | |-DeclStmt 0x64e7829e0850 <line:35:2, col:19>
// | | `-VarDecl 0x64e7829e0720 <col:2, col:18> col:6 used z 'int' cinit
// | |   `-UnaryOperator 0x64e7829e0838 <col:10, col:18> 'int' prefix '!' cannot overflow
// | |     `-ParenExpr 0x64e7829e0818 <col:11, col:18> 'int'
// | |       `-BinaryOperator 0x64e7829e07f8 <col:12, col:17> 'int' '&&'
// | |         |-ImplicitCastExpr 0x64e7829e07c8 <col:12> 'int' <LValueToRValue>
// | |         | `-DeclRefExpr 0x64e7829e0788 <col:12> 'int' lvalue Var 0x64e7829e0598 'x' 'int'
// | |         `-ImplicitCastExpr 0x64e7829e07e0 <col:17> 'int' <LValueToRValue>
// | |           `-DeclRefExpr 0x64e7829e07a8 <col:17> 'int' lvalue Var 0x64e7829e0668 'y' 'int'
// | `-ReturnStmt 0x64e7829e08a0 <line:36:2, col:9>
// |   `-ImplicitCastExpr 0x64e7829e0888 <col:9> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64e7829e0868 <col:9> 'int' lvalue Var 0x64e7829e0720 'z' 'int'
// `-AnnotateAttr 0x64e7829e04e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x64e7829e0940 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:40:1, line:45:1> line:40:5 f5 'int ()'
// |-CompoundStmt 0x64e7829e0d88 <col:9, line:45:1>
// | |-DeclStmt 0x64e7829e0b48 <line:41:2, col:14>
// | | `-VarDecl 0x64e7829e0aa8 <col:2, col:11> col:6 used x 'int' cinit
// | |   `-UnaryOperator 0x64e7829e0b30 <col:10, col:11> 'int' prefix '-'
// | |     `-IntegerLiteral 0x64e7829e0b10 <col:11> 'int' 100
// | |-DeclStmt 0x64e7829e0c00 <line:42:2, col:13>
// | | `-VarDecl 0x64e7829e0b78 <col:2, col:10> col:6 used y 'int' cinit
// | |   `-IntegerLiteral 0x64e7829e0be0 <col:10> 'int' 100
// | |-DeclStmt 0x64e7829e0d28 <line:43:2, col:16>
// | | `-VarDecl 0x64e7829e0c30 <col:2, col:15> col:6 used z 'int' cinit
// | |   `-BinaryOperator 0x64e7829e0d08 <col:10, col:15> 'int' '&&'
// | |     |-ImplicitCastExpr 0x64e7829e0cd8 <col:10> 'int' <LValueToRValue>
// | |     | `-DeclRefExpr 0x64e7829e0c98 <col:10> 'int' lvalue Var 0x64e7829e0aa8 'x' 'int'
// | |     `-ImplicitCastExpr 0x64e7829e0cf0 <col:15> 'int' <LValueToRValue>
// | |       `-DeclRefExpr 0x64e7829e0cb8 <col:15> 'int' lvalue Var 0x64e7829e0b78 'y' 'int'
// | `-ReturnStmt 0x64e7829e0d78 <line:44:2, col:9>
// |   `-ImplicitCastExpr 0x64e7829e0d60 <col:9> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64e7829e0d40 <col:9> 'int' lvalue Var 0x64e7829e0c30 'z' 'int'
// `-AnnotateAttr 0x64e7829e09e8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x64e7829e0e50 </home/t-visinghal/Applications/src/c2pulse/test/general/test_logicals_int.c:49:1, line:54:1> line:49:5 f6 'int ()'
// |-CompoundStmt 0x64e782ad6388 <col:9, line:54:1>
// | |-DeclStmt 0x64e782ad6100 <line:50:2, col:18>
// | | `-VarDecl 0x64e782ad6060 <col:2, col:15> col:11 used x 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x64e782ad60e8 <col:15> 'uint32_t':'unsigned int' <IntegralCast>
// | |     `-IntegerLiteral 0x64e782ad60c8 <col:15> 'int' 100
// | |-DeclStmt 0x64e782ad6200 <line:51:2, col:12>
// | | `-VarDecl 0x64e782ad6130 <col:2, col:11> col:6 used y 'int' cinit
// | |   `-ImplicitCastExpr 0x64e782ad61e8 <col:10, col:11> 'int' <IntegralCast>
// | |     `-UnaryOperator 0x64e782ad61d0 <col:10, col:11> 'uint32_t':'unsigned int' prefix '-'
// | |       `-ImplicitCastExpr 0x64e782ad61b8 <col:11> 'uint32_t':'unsigned int' <LValueToRValue>
// | |         `-DeclRefExpr 0x64e782ad6198 <col:11> 'uint32_t':'unsigned int' lvalue Var 0x64e782ad6060 'x' 'uint32_t':'unsigned int'
// | |-DeclStmt 0x64e782ad6328 <line:52:2, col:16>
// | | `-VarDecl 0x64e782ad6230 <col:2, col:15> col:6 used z 'int' cinit
// | |   `-BinaryOperator 0x64e782ad6308 <col:10, col:15> 'int' '&&'
// | |     |-ImplicitCastExpr 0x64e782ad62d8 <col:10> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     | `-DeclRefExpr 0x64e782ad6298 <col:10> 'uint32_t':'unsigned int' lvalue Var 0x64e782ad6060 'x' 'uint32_t':'unsigned int'
// | |     `-ImplicitCastExpr 0x64e782ad62f0 <col:15> 'int' <LValueToRValue>
// | |       `-DeclRefExpr 0x64e782ad62b8 <col:15> 'int' lvalue Var 0x64e782ad6130 'y' 'int'
// | `-ReturnStmt 0x64e782ad6378 <line:53:2, col:9>
// |   `-ImplicitCastExpr 0x64e782ad6360 <col:9> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64e782ad6340 <col:9> 'int' lvalue Var 0x64e782ad6230 'z' 'int'
// |-AnnotateAttr 0x64e7829e0ef8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
// `-AnnotateAttr 0x64e7829e0fa0 <line:5:22, col:58> pulse "returns:Int32.t|END"
