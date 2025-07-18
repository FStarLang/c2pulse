module Swap_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn ref_swap
(r1 : ref Int32.t)
(r2 : ref Int32.t)
requires (r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)
ensures (r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)
{
let mut r1 : (ref Int32.t) = r1;
let mut r2 : (ref Int32.t) = r2;
let tmp0 : Int32.t = (! (! r1));
let mut tmp : Int32.t = tmp0;
(! r1) := (! (! r2));
(! r2) := (! tmp);
}

//Dumping the Clang AST.
// FunctionDecl 0x64c428384380 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_test.c:5:1, line:10:1> line:5:6 ref_swap 'void (int *, int *)'
// |-ParmVarDecl 0x64c428384228 <col:15, col:20> col:20 used r1 'int *'
// |-ParmVarDecl 0x64c4283842a8 <col:24, col:29> col:29 used r2 'int *'
// |-CompoundStmt 0x64c428384858 <line:6:1, line:10:1>
// | |-DeclStmt 0x64c4283846c0 <line:7:3, col:16>
// | | `-VarDecl 0x64c4283845f0 <col:3, col:14> col:7 used tmp 'int' cinit
// | |   `-ImplicitCastExpr 0x64c4283846a8 <col:13, col:14> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x64c428384690 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x64c428384678 <col:14> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x64c428384658 <col:14> 'int *' lvalue ParmVar 0x64c428384228 'r1' 'int *'
// | |-BinaryOperator 0x64c428384790 <line:8:3, col:10> 'int' '='
// | | |-UnaryOperator 0x64c428384710 <col:3, col:4> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x64c4283846f8 <col:4> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x64c4283846d8 <col:4> 'int *' lvalue ParmVar 0x64c428384228 'r1' 'int *'
// | | `-ImplicitCastExpr 0x64c428384778 <col:9, col:10> 'int' <LValueToRValue>
// | |   `-UnaryOperator 0x64c428384760 <col:9, col:10> 'int' lvalue prefix '*' cannot overflow
// | |     `-ImplicitCastExpr 0x64c428384748 <col:10> 'int *' <LValueToRValue>
// | |       `-DeclRefExpr 0x64c428384728 <col:10> 'int *' lvalue ParmVar 0x64c4283842a8 'r2' 'int *'
// | `-BinaryOperator 0x64c428384838 <line:9:3, col:9> 'int' '='
// |   |-UnaryOperator 0x64c4283847e8 <col:3, col:4> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x64c4283847d0 <col:4> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x64c4283847b0 <col:4> 'int *' lvalue ParmVar 0x64c4283842a8 'r2' 'int *'
// |   `-ImplicitCastExpr 0x64c428384820 <col:9> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x64c428384800 <col:9> 'int' lvalue Var 0x64c4283845f0 'tmp' 'int'
// |-AnnotateAttr 0x64c428384438 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:(r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)|END"
// `-AnnotateAttr 0x64c428384500 <line:4:24, col:71> pulse "ensures:(r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)|END"
