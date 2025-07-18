module Issue29_test

#lang-pulse

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max
(x : ref Int32.t)
(y : ref Int32.t)
(#vx #vy : _)
(#px #py : _)
requires x |->Frac px vx
requires y |->Frac py vy
returns n : int32
ensures x |->Frac px vx
ensures y |->Frac py vy
ensures pure(as_int n == max_spec(as_int vx)(as_int vy))
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
if((int32_to_bool (bool_to_int32 (Int32.gt (! (! x)) (! (! y))))))
{
(! (! x));
}
else
{
(! (! y));
};
}

//Dumping the Clang AST.
// FunctionDecl 0x6077ed93f5c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue29_test.c:15:1, line:21:1> line:15:5 max 'int (int *, int *)'
// |-ParmVarDecl 0x6077ed93f460 <col:9, col:14> col:14 used x 'int *'
// |-ParmVarDecl 0x6077ed93f4e0 <col:17, col:22> col:22 used y 'int *'
// |-CompoundStmt 0x6077ed93fd30 <col:25, line:21:1>
// | `-IfStmt 0x6077ed93fd00 <line:16:3, line:20:3> has_else
// |   |-BinaryOperator 0x6077ed93fbc0 <line:16:7, col:13> 'int' '>'
// |   | |-ImplicitCastExpr 0x6077ed93fb90 <col:7, col:8> 'int' <LValueToRValue>
// |   | | `-UnaryOperator 0x6077ed93fb28 <col:7, col:8> 'int' lvalue prefix '*' cannot overflow
// |   | |   `-ImplicitCastExpr 0x6077ed93fb10 <col:8> 'int *' <LValueToRValue>
// |   | |     `-DeclRefExpr 0x6077ed93faf0 <col:8> 'int *' lvalue ParmVar 0x6077ed93f460 'x' 'int *'
// |   | `-ImplicitCastExpr 0x6077ed93fba8 <col:12, col:13> 'int' <LValueToRValue>
// |   |   `-UnaryOperator 0x6077ed93fb78 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |   |     `-ImplicitCastExpr 0x6077ed93fb60 <col:13> 'int *' <LValueToRValue>
// |   |       `-DeclRefExpr 0x6077ed93fb40 <col:13> 'int *' lvalue ParmVar 0x6077ed93f4e0 'y' 'int *'
// |   |-CompoundStmt 0x6077ed93fc58 <col:16, line:18:3>
// |   | `-ReturnStmt 0x6077ed93fc48 <line:17:5, col:13>
// |   |   `-ImplicitCastExpr 0x6077ed93fc30 <col:12, col:13> 'int' <LValueToRValue>
// |   |     `-UnaryOperator 0x6077ed93fc18 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |   |       `-ImplicitCastExpr 0x6077ed93fc00 <col:13> 'int *' <LValueToRValue>
// |   |         `-DeclRefExpr 0x6077ed93fbe0 <col:13> 'int *' lvalue ParmVar 0x6077ed93f460 'x' 'int *'
// |   `-CompoundStmt 0x6077ed93fce8 <line:18:10, line:20:3>
// |     `-ReturnStmt 0x6077ed93fcd8 <line:19:5, col:13>
// |       `-ImplicitCastExpr 0x6077ed93fcc0 <col:12, col:13> 'int' <LValueToRValue>
// |         `-UnaryOperator 0x6077ed93fca8 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |           `-ImplicitCastExpr 0x6077ed93fc90 <col:13> 'int *' <LValueToRValue>
// |             `-DeclRefExpr 0x6077ed93fc70 <col:13> 'int *' lvalue ParmVar 0x6077ed93f4e0 'y' 'int *'
// |-AnnotateAttr 0x6077ed93f680 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#vx #vy : _|END"
// |-AnnotateAttr 0x6077ed93f730 <col:27, col:68> pulse "erased_arg:#px #py : _|END"
// |-AnnotateAttr 0x6077ed93f7b0 <line:3:23, col:61> pulse "requires:x |->Frac px vx|END"
// |-AnnotateAttr 0x6077ed93f830 <col:23, col:61> pulse "requires:y |->Frac py vy|END"
// |-AnnotateAttr 0x6077ed93f8b0 <line:5:22, col:58> pulse "returns:n : int32|END"
// |-AnnotateAttr 0x6077ed93f920 <line:4:24, col:71> pulse "ensures:x |->Frac px vx|END"
// |-AnnotateAttr 0x6077ed93f9d0 <col:24, col:71> pulse "ensures:y |->Frac py vy|END"
// `-AnnotateAttr 0x6077ed93fa50 <col:24, col:71> pulse "ensures:pure(as_int n == max_spec(as_int vx)(as_int vy))|END"
