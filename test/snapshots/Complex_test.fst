module Complex_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn triple
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * 3))
returns res : Int32.t
ensures pure (Int32.v x * 3 == Int32.v res)
{
let mut x : Int32.t = x;
(Int32.mul (! x) 3l);
}

fn square
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * Int32.v x))
returns res : Int32.t
ensures pure (Int32.v x * Int32.v x == Int32.v res)
{
let mut x : Int32.t = x;
(Int32.mul (! x) (! x));
}

fn doubleValue
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * 2))
returns res : Int32.t
ensures pure (Int32.v x * 2 == Int32.v res)
{
let mut x : Int32.t = x;
(Int32.mul (! x) 2l);
}

fn sum
(x : Int32.t)
(y : Int32.t)
requires pure (Int32.fits (Int32.v x + Int32.v y))
returns res : Int32.t
ensures pure (Int32.v x + Int32.v y == Int32.v res)
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(Int32.add (! x) (! y));
}

fn rec recursiveFunction
(x : Int32.t)
(limit : Int32.t)
requires pure False
returns res : Int32.t
{
let mut x : Int32.t = x;
let mut limit : Int32.t = limit;
if((int32_to_bool (bool_to_int32 (Int32.gte (! x) (! limit)))))
{
(! x);
}
else
{
(sum 12l (recursiveFunction (Int32.add (! x) 1l) (! limit)));
};
}

fn complexComputation
(x : Int32.t)
(y : Int32.t)
requires pure (abs (Int32.v x) < 100 /\ abs (Int32.v y) < 100)
returns res : Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(Int32.add (square (triple (sum (doubleValue (! x)) (! y)))) (doubleValue (square (Int32.sub (! y) (! x)))));
}

fn conditionalProcessing
(x : Int32.t)
(y : Int32.t)
requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
returns res : Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
if((int32_to_bool (bool_to_int32 (Int32.eq (Int32.rem ((Int32.mul (! x) (! y))) 2l) 0l))))
{
(sum (triple (! x)) (square (Int32.sub (! y) (! x))));
}
else
{
(doubleValue (square (sum (! x) (! y))));
};
}

//Dumping the Clang AST.
// FunctionDecl 0x55dc2471b2e8 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:6:1, line:8:1> line:6:5 used triple 'int (int)'
// |-ParmVarDecl 0x55dc2471b218 <col:12, col:16> col:16 used x 'int'
// |-CompoundStmt 0x55dc2471b630 <col:19, line:8:1>
// | `-ReturnStmt 0x55dc2471b620 <line:7:5, col:16>
// |   `-BinaryOperator 0x55dc2471b600 <col:12, col:16> 'int' '*'
// |     |-ImplicitCastExpr 0x55dc2471b5e8 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x55dc2471b5a8 <col:12> 'int' lvalue ParmVar 0x55dc2471b218 'x' 'int'
// |     `-IntegerLiteral 0x55dc2471b5c8 <col:16> 'int' 3
// |-AnnotateAttr 0x55dc2471b398 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (Int32.fits (Int32.v x * 3))|END"
// |-AnnotateAttr 0x55dc2471b450 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// `-AnnotateAttr 0x55dc2471b4d0 <line:4:24, col:71> pulse "ensures:pure (Int32.v x * 3 == Int32.v res)|END"
// FunctionDecl 0x55dc2471b7e8 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:13:1, line:15:1> line:13:5 used square 'int (int)'
// |-ParmVarDecl 0x55dc2471b750 <col:12, col:16> col:16 used x 'int'
// |-CompoundStmt 0x55dc2471bb10 <col:19, line:15:1>
// | `-ReturnStmt 0x55dc2471bb00 <line:14:5, col:16>
// |   `-BinaryOperator 0x55dc2471bae0 <col:12, col:16> 'int' '*'
// |     |-ImplicitCastExpr 0x55dc2471bab0 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x55dc2471ba70 <col:12> 'int' lvalue ParmVar 0x55dc2471b750 'x' 'int'
// |     `-ImplicitCastExpr 0x55dc2471bac8 <col:16> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x55dc2471ba90 <col:16> 'int' lvalue ParmVar 0x55dc2471b750 'x' 'int'
// |-AnnotateAttr 0x55dc2471b898 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (Int32.fits (Int32.v x * Int32.v x))|END"
// |-AnnotateAttr 0x55dc2471b960 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// `-AnnotateAttr 0x55dc2471b9e0 <line:4:24, col:71> pulse "ensures:pure (Int32.v x * Int32.v x == Int32.v res)|END"
// FunctionDecl 0x55dc2471bcb8 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:20:1, line:22:1> line:20:5 used doubleValue 'int (int)'
// |-ParmVarDecl 0x55dc2471bc20 <col:17, col:21> col:21 used x 'int'
// |-CompoundStmt 0x55dc2471bfb8 <col:24, line:22:1>
// | `-ReturnStmt 0x55dc2471bfa8 <line:21:5, col:16>
// |   `-BinaryOperator 0x55dc2471bf88 <col:12, col:16> 'int' '*'
// |     |-ImplicitCastExpr 0x55dc2471bf70 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x55dc2471bf30 <col:12> 'int' lvalue ParmVar 0x55dc2471bc20 'x' 'int'
// |     `-IntegerLiteral 0x55dc2471bf50 <col:16> 'int' 2
// |-AnnotateAttr 0x55dc2471bd68 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (Int32.fits (Int32.v x * 2))|END"
// |-AnnotateAttr 0x55dc2471be20 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// `-AnnotateAttr 0x55dc2471bea0 <line:4:24, col:71> pulse "ensures:pure (Int32.v x * 2 == Int32.v res)|END"
// FunctionDecl 0x55dc2471d898 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:28:1, line:30:1> line:28:5 used sum 'int (int, int)'
// |-ParmVarDecl 0x55dc2471d730 <col:9, col:13> col:13 used x 'int'
// |-ParmVarDecl 0x55dc2471d7b0 <col:16, col:20> col:20 used y 'int'
// |-CompoundStmt 0x55dc2471dbc0 <col:23, line:30:1>
// | `-ReturnStmt 0x55dc2471dbb0 <line:29:5, col:16>
// |   `-BinaryOperator 0x55dc2471db90 <col:12, col:16> 'int' '+'
// |     |-ImplicitCastExpr 0x55dc2471db60 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x55dc2471db20 <col:12> 'int' lvalue ParmVar 0x55dc2471d730 'x' 'int'
// |     `-ImplicitCastExpr 0x55dc2471db78 <col:16> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x55dc2471db40 <col:16> 'int' lvalue ParmVar 0x55dc2471d7b0 'y' 'int'
// |-AnnotateAttr 0x55dc2471d950 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (Int32.fits (Int32.v x + Int32.v y))|END"
// |-AnnotateAttr 0x55dc2471da10 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// `-AnnotateAttr 0x55dc2471da90 <line:4:24, col:71> pulse "ensures:pure (Int32.v x + Int32.v y == Int32.v res)|END"
// FunctionDecl 0x55dc2471dd88 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:34:1, line:39:1> line:34:5 referenced recursiveFunction 'int (int, int)'
// |-ParmVarDecl 0x55dc2471dc68 <col:23, col:27> col:27 used x 'int'
// |-ParmVarDecl 0x55dc2471dce8 <col:30, col:34> col:34 used limit 'int'
// |-CompoundStmt 0x55dc2471e298 <col:41, line:39:1>
// | `-IfStmt 0x55dc2471e240 <line:35:5, line:38:5> has_else
// |   |-BinaryOperator 0x55dc2471dfd0 <line:35:9, col:14> 'int' '>='
// |   | |-ImplicitCastExpr 0x55dc2471dfa0 <col:9> 'int' <LValueToRValue>
// |   | | `-DeclRefExpr 0x55dc2471df60 <col:9> 'int' lvalue ParmVar 0x55dc2471dc68 'x' 'int'
// |   | `-ImplicitCastExpr 0x55dc2471dfb8 <col:14> 'int' <LValueToRValue>
// |   |   `-DeclRefExpr 0x55dc2471df80 <col:14> 'int' lvalue ParmVar 0x55dc2471dce8 'limit' 'int'
// |   |-ReturnStmt 0x55dc2471e028 <col:21, col:28>
// |   | `-ImplicitCastExpr 0x55dc2471e010 <col:28> 'int' <LValueToRValue>
// |   |   `-DeclRefExpr 0x55dc2471dff0 <col:28> 'int' lvalue ParmVar 0x55dc2471dc68 'x' 'int'
// |   `-CompoundStmt 0x55dc2471e228 <line:36:9, line:38:5>
// |     `-ReturnStmt 0x55dc2471e218 <line:37:9, col:56>
// |       `-CallExpr 0x55dc2471e1e0 <col:16, col:56> 'int'
// |         |-ImplicitCastExpr 0x55dc2471e1c8 <col:16> 'int (*)(int, int)' <FunctionToPointerDecay>
// |         | `-DeclRefExpr 0x55dc2471e038 <col:16> 'int (int, int)' Function 0x55dc2471d898 'sum' 'int (int, int)'
// |         |-IntegerLiteral 0x55dc2471e058 <col:20> 'int' 12
// |         `-CallExpr 0x55dc2471e178 <col:25, col:55> 'int'
// |           |-ImplicitCastExpr 0x55dc2471e160 <col:25> 'int (*)(int, int)' <FunctionToPointerDecay>
// |           | `-DeclRefExpr 0x55dc2471e078 <col:25> 'int (int, int)' Function 0x55dc2471dd88 'recursiveFunction' 'int (int, int)'
// |           |-BinaryOperator 0x55dc2471e0f0 <col:43, col:47> 'int' '+'
// |           | |-ImplicitCastExpr 0x55dc2471e0d8 <col:43> 'int' <LValueToRValue>
// |           | | `-DeclRefExpr 0x55dc2471e098 <col:43> 'int' lvalue ParmVar 0x55dc2471dc68 'x' 'int'
// |           | `-IntegerLiteral 0x55dc2471e0b8 <col:47> 'int' 1
// |           `-ImplicitCastExpr 0x55dc2471e1b0 <col:50> 'int' <LValueToRValue>
// |             `-DeclRefExpr 0x55dc2471e110 <col:50> 'int' lvalue ParmVar 0x55dc2471dce8 'limit' 'int'
// |-AnnotateAttr 0x55dc2471de40 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure False|END"
// `-AnnotateAttr 0x55dc2471dee0 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// FunctionDecl 0x55dc2471e490 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:45:1, line:48:1> line:45:5 complexComputation 'int (int, int)'
// |-ParmVarDecl 0x55dc2471e370 <col:24, col:28> col:28 used x 'int'
// |-ParmVarDecl 0x55dc2471e3f0 <col:31, col:35> col:35 used y 'int'
// |-CompoundStmt 0x55dc24720ae8 <col:38, line:48:1>
// | `-ReturnStmt 0x55dc24720ad8 <line:46:5, line:47:37>
// |   `-BinaryOperator 0x55dc24720ab8 <line:46:12, line:47:37> 'int' '+'
// |     |-CallExpr 0x55dc24720928 <line:46:12, col:49> 'int'
// |     | |-ImplicitCastExpr 0x55dc24720910 <col:12> 'int (*)(int)' <FunctionToPointerDecay>
// |     | | `-DeclRefExpr 0x55dc2471e6a0 <col:12> 'int (int)' Function 0x55dc2471b7e8 'square' 'int (int)'
// |     | `-CallExpr 0x55dc247208e0 <col:19, col:48> 'int'
// |     |   |-ImplicitCastExpr 0x55dc247208c8 <col:19> 'int (*)(int)' <FunctionToPointerDecay>
// |     |   | `-DeclRefExpr 0x55dc2471e6c0 <col:19> 'int (int)' Function 0x55dc2471b2e8 'triple' 'int (int)'
// |     |   `-CallExpr 0x55dc24720878 <col:26, col:47> 'int'
// |     |     |-ImplicitCastExpr 0x55dc24720860 <col:26> 'int (*)(int, int)' <FunctionToPointerDecay>
// |     |     | `-DeclRefExpr 0x55dc2471e6e0 <col:26> 'int (int, int)' Function 0x55dc2471d898 'sum' 'int (int, int)'
// |     |     |-CallExpr 0x55dc247207f8 <col:30, col:43> 'int'
// |     |     | |-ImplicitCastExpr 0x55dc247207e0 <col:30> 'int (*)(int)' <FunctionToPointerDecay>
// |     |     | | `-DeclRefExpr 0x55dc2471e700 <col:30> 'int (int)' Function 0x55dc2471bcb8 'doubleValue' 'int (int)'
// |     |     | `-ImplicitCastExpr 0x55dc24720828 <col:42> 'int' <LValueToRValue>
// |     |     |   `-DeclRefExpr 0x55dc24720790 <col:42> 'int' lvalue ParmVar 0x55dc2471e370 'x' 'int'
// |     |     `-ImplicitCastExpr 0x55dc247208b0 <col:46> 'int' <LValueToRValue>
// |     |       `-DeclRefExpr 0x55dc24720840 <col:46> 'int' lvalue ParmVar 0x55dc2471e3f0 'y' 'int'
// |     `-CallExpr 0x55dc24720a88 <line:47:12, col:37> 'int'
// |       |-ImplicitCastExpr 0x55dc24720a70 <col:12> 'int (*)(int)' <FunctionToPointerDecay>
// |       | `-DeclRefExpr 0x55dc24720958 <col:12> 'int (int)' Function 0x55dc2471bcb8 'doubleValue' 'int (int)'
// |       `-CallExpr 0x55dc24720a40 <col:24, col:36> 'int'
// |         |-ImplicitCastExpr 0x55dc24720a28 <col:24> 'int (*)(int)' <FunctionToPointerDecay>
// |         | `-DeclRefExpr 0x55dc24720978 <col:24> 'int (int)' Function 0x55dc2471b7e8 'square' 'int (int)'
// |         `-BinaryOperator 0x55dc24720a08 <col:31, col:35> 'int' '-'
// |           |-ImplicitCastExpr 0x55dc247209d8 <col:31> 'int' <LValueToRValue>
// |           | `-DeclRefExpr 0x55dc24720998 <col:31> 'int' lvalue ParmVar 0x55dc2471e3f0 'y' 'int'
// |           `-ImplicitCastExpr 0x55dc247209f0 <col:35> 'int' <LValueToRValue>
// |             `-DeclRefExpr 0x55dc247209b8 <col:35> 'int' lvalue ParmVar 0x55dc2471e370 'x' 'int'
// |-AnnotateAttr 0x55dc2471e548 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (abs (Int32.v x) < 100 /\ abs (Int32.v y) < 100)|END"
// |-AnnotateAttr 0x55dc2471e620 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
// `-FullComment 0x55dc24721530 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:41:4, line:42:69>
//   `-ParagraphComment 0x55dc24721500 <line:41:4, line:42:69>
//     |-TextComment 0x55dc247214b0 <line:41:4, col:62> Text="Note: The Z3 solver may be flacky at times and Z3 might not"
//     `-TextComment 0x55dc247214d0 <line:42:4, col:69> Text="be able to prove things. Chaning 1000 to 100 makes this test pass."
// FunctionDecl 0x55dc24720ce0 </home/t-visinghal/Applications/src/c2pulse/test/general/complex_test.c:52:1, line:58:1> line:52:5 conditionalProcessing 'int (int, int)'
// |-ParmVarDecl 0x55dc24720bc0 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x55dc24720c40 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x55dc24721458 <col:41, line:58:1>
// | `-IfStmt 0x55dc24721428 <line:53:5, line:57:5> has_else
// |   |-BinaryOperator 0x55dc24721000 <line:53:9, col:24> 'int' '=='
// |   | |-BinaryOperator 0x55dc24720fc0 <col:9, col:19> 'int' '%'
// |   | | |-ParenExpr 0x55dc24720f80 <col:9, col:15> 'int'
// |   | | | `-BinaryOperator 0x55dc24720f60 <col:10, col:14> 'int' '*'
// |   | | |   |-ImplicitCastExpr 0x55dc24720f30 <col:10> 'int' <LValueToRValue>
// |   | | |   | `-DeclRefExpr 0x55dc24720ef0 <col:10> 'int' lvalue ParmVar 0x55dc24720bc0 'x' 'int'
// |   | | |   `-ImplicitCastExpr 0x55dc24720f48 <col:14> 'int' <LValueToRValue>
// |   | | |     `-DeclRefExpr 0x55dc24720f10 <col:14> 'int' lvalue ParmVar 0x55dc24720c40 'y' 'int'
// |   | | `-IntegerLiteral 0x55dc24720fa0 <col:19> 'int' 2
// |   | `-IntegerLiteral 0x55dc24720fe0 <col:24> 'int' 0
// |   |-CompoundStmt 0x55dc24721238 <col:27, line:55:5>
// |   | `-ReturnStmt 0x55dc24721228 <line:54:9, col:44>
// |   |   `-CallExpr 0x55dc247211f0 <col:16, col:44> 'int'
// |   |     |-ImplicitCastExpr 0x55dc247211d8 <col:16> 'int (*)(int, int)' <FunctionToPointerDecay>
// |   |     | `-DeclRefExpr 0x55dc24721020 <col:16> 'int (int, int)' Function 0x55dc2471d898 'sum' 'int (int, int)'
// |   |     |-CallExpr 0x55dc24721098 <col:20, col:28> 'int'
// |   |     | |-ImplicitCastExpr 0x55dc24721080 <col:20> 'int (*)(int)' <FunctionToPointerDecay>
// |   |     | | `-DeclRefExpr 0x55dc24721040 <col:20> 'int (int)' Function 0x55dc2471b2e8 'triple' 'int (int)'
// |   |     | `-ImplicitCastExpr 0x55dc247210c8 <col:27> 'int' <LValueToRValue>
// |   |     |   `-DeclRefExpr 0x55dc24721060 <col:27> 'int' lvalue ParmVar 0x55dc24720bc0 'x' 'int'
// |   |     `-CallExpr 0x55dc247211a8 <col:31, col:43> 'int'
// |   |       |-ImplicitCastExpr 0x55dc24721190 <col:31> 'int (*)(int)' <FunctionToPointerDecay>
// |   |       | `-DeclRefExpr 0x55dc247210e0 <col:31> 'int (int)' Function 0x55dc2471b7e8 'square' 'int (int)'
// |   |       `-BinaryOperator 0x55dc24721170 <col:38, col:42> 'int' '-'
// |   |         |-ImplicitCastExpr 0x55dc24721140 <col:38> 'int' <LValueToRValue>
// |   |         | `-DeclRefExpr 0x55dc24721100 <col:38> 'int' lvalue ParmVar 0x55dc24720c40 'y' 'int'
// |   |         `-ImplicitCastExpr 0x55dc24721158 <col:42> 'int' <LValueToRValue>
// |   |           `-DeclRefExpr 0x55dc24721120 <col:42> 'int' lvalue ParmVar 0x55dc24720bc0 'x' 'int'
// |   `-CompoundStmt 0x55dc24721410 <line:55:12, line:57:5>
// |     `-ReturnStmt 0x55dc24721400 <line:56:9, col:45>
// |       `-CallExpr 0x55dc247213d0 <col:16, col:45> 'int'
// |         |-ImplicitCastExpr 0x55dc247213b8 <col:16> 'int (*)(int)' <FunctionToPointerDecay>
// |         | `-DeclRefExpr 0x55dc24721250 <col:16> 'int (int)' Function 0x55dc2471bcb8 'doubleValue' 'int (int)'
// |         `-CallExpr 0x55dc24721388 <col:28, col:44> 'int'
// |           |-ImplicitCastExpr 0x55dc24721370 <col:28> 'int (*)(int)' <FunctionToPointerDecay>
// |           | `-DeclRefExpr 0x55dc24721270 <col:28> 'int (int)' Function 0x55dc2471b7e8 'square' 'int (int)'
// |           `-CallExpr 0x55dc24721308 <col:35, col:43> 'int'
// |             |-ImplicitCastExpr 0x55dc247212f0 <col:35> 'int (*)(int, int)' <FunctionToPointerDecay>
// |             | `-DeclRefExpr 0x55dc24721290 <col:35> 'int (int, int)' Function 0x55dc2471d898 'sum' 'int (int, int)'
// |             |-ImplicitCastExpr 0x55dc24721340 <col:39> 'int' <LValueToRValue>
// |             | `-DeclRefExpr 0x55dc247212b0 <col:39> 'int' lvalue ParmVar 0x55dc24720bc0 'x' 'int'
// |             `-ImplicitCastExpr 0x55dc24721358 <col:42> 'int' <LValueToRValue>
// |               `-DeclRefExpr 0x55dc247212d0 <col:42> 'int' lvalue ParmVar 0x55dc24720c40 'y' 'int'
// |-AnnotateAttr 0x55dc24720d98 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)|END"
// `-AnnotateAttr 0x55dc24720e70 <line:5:22, col:58> pulse "returns:res : Int32.t|END"
