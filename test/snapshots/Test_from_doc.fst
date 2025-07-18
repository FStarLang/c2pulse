module Test_from_doc

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(a : Int32.t)
(b : bool)
requires pure (Int32.v a < 100)
returns Int32.t
{
let mut a : Int32.t = a;
let mut b : bool = b;
a := (Int32.add (! a) (bool_to_int32 (! b)));
let mut p: (ref Int32.t) = witness #_ #_;
p := (a);
(! p) := (Int32.add (! (! p)) 123l);
(! (! p));
}

//Dumping the Clang AST.
// FunctionDecl 0x5864bc534448 </home/t-visinghal/Applications/src/c2pulse/test/general/test_from_doc.c:7:1, line:13:1> line:7:5 foo 'int (int, bool)'
// |-ParmVarDecl 0x5864bc5342e8 <col:9, col:13> col:13 used a 'int'
// |-ParmVarDecl 0x5864bc534360 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/general/test_from_doc.c:7:21> col:21 used b 'bool'
// |-CompoundStmt 0x5864bc5349e0 <col:24, line:13:1>
// | |-BinaryOperator 0x5864bc5346e8 <line:8:5, col:13> 'int' '='
// | | |-DeclRefExpr 0x5864bc534620 <col:5> 'int' lvalue ParmVar 0x5864bc5342e8 'a' 'int'
// | | `-BinaryOperator 0x5864bc5346c8 <col:9, col:13> 'int' '+'
// | |   |-ImplicitCastExpr 0x5864bc534680 <col:9> 'int' <LValueToRValue>
// | |   | `-DeclRefExpr 0x5864bc534640 <col:9> 'int' lvalue ParmVar 0x5864bc5342e8 'a' 'int'
// | |   `-ImplicitCastExpr 0x5864bc5346b0 <col:13> 'int' <IntegralCast>
// | |     `-ImplicitCastExpr 0x5864bc534698 <col:13> 'bool' <LValueToRValue>
// | |       `-DeclRefExpr 0x5864bc534660 <col:13> 'bool' lvalue ParmVar 0x5864bc534360 'b' 'bool'
// | |-DeclStmt 0x5864bc5347c0 <line:9:5, col:11>
// | | `-VarDecl 0x5864bc534758 <col:5, col:10> col:10 used p 'int *'
// | |-BinaryOperator 0x5864bc534830 <line:10:5, col:10> 'int *' '='
// | | |-DeclRefExpr 0x5864bc5347d8 <col:5> 'int *' lvalue Var 0x5864bc534758 'p' 'int *'
// | | `-UnaryOperator 0x5864bc534818 <col:9, col:10> 'int *' prefix '&' cannot overflow
// | |   `-DeclRefExpr 0x5864bc5347f8 <col:10> 'int' lvalue ParmVar 0x5864bc5342e8 'a' 'int'
// | |-BinaryOperator 0x5864bc534948 <line:11:5, col:15> 'int' '='
// | | |-UnaryOperator 0x5864bc534888 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x5864bc534870 <col:6> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5864bc534850 <col:6> 'int *' lvalue Var 0x5864bc534758 'p' 'int *'
// | | `-BinaryOperator 0x5864bc534928 <col:10, col:15> 'int' '+'
// | |   |-ImplicitCastExpr 0x5864bc534910 <col:10, col:11> 'int' <LValueToRValue>
// | |   | `-UnaryOperator 0x5864bc5348d8 <col:10, col:11> 'int' lvalue prefix '*' cannot overflow
// | |   |   `-ImplicitCastExpr 0x5864bc5348c0 <col:11> 'int *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x5864bc5348a0 <col:11> 'int *' lvalue Var 0x5864bc534758 'p' 'int *'
// | |   `-IntegerLiteral 0x5864bc5348f0 <col:15> 'int' 123
// | `-ReturnStmt 0x5864bc5349d0 <line:12:5, col:13>
// |   `-ImplicitCastExpr 0x5864bc5349b8 <col:12, col:13> 'int' <LValueToRValue>
// |     `-UnaryOperator 0x5864bc5349a0 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |       `-ImplicitCastExpr 0x5864bc534988 <col:13> 'int *' <LValueToRValue>
// |         `-DeclRefExpr 0x5864bc534968 <col:13> 'int *' lvalue Var 0x5864bc534758 'p' 'int *'
// |-AnnotateAttr 0x5864bc534500 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:pure (Int32.v a < 100)|END"
// `-AnnotateAttr 0x5864bc5345b0 <line:5:22, col:58> pulse "returns:Int32.t|END"
