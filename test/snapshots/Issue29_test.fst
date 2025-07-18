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
// FunctionDecl 0x55e258daa5c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue29_test.c:15:1, line:21:1> line:15:5 max 'int (int *, int *)'
// |-ParmVarDecl 0x55e258daa460 <col:9, col:14> col:14 used x 'int *'
// |-ParmVarDecl 0x55e258daa4e0 <col:17, col:22> col:22 used y 'int *'
// |-CompoundStmt 0x55e258daad30 <col:25, line:21:1>
// | `-IfStmt 0x55e258daad00 <line:16:3, line:20:3> has_else
// |   |-BinaryOperator 0x55e258daabc0 <line:16:7, col:13> 'int' '>'
// |   | |-ImplicitCastExpr 0x55e258daab90 <col:7, col:8> 'int' <LValueToRValue>
// |   | | `-UnaryOperator 0x55e258daab28 <col:7, col:8> 'int' lvalue prefix '*' cannot overflow
// |   | |   `-ImplicitCastExpr 0x55e258daab10 <col:8> 'int *' <LValueToRValue>
// |   | |     `-DeclRefExpr 0x55e258daaaf0 <col:8> 'int *' lvalue ParmVar 0x55e258daa460 'x' 'int *'
// |   | `-ImplicitCastExpr 0x55e258daaba8 <col:12, col:13> 'int' <LValueToRValue>
// |   |   `-UnaryOperator 0x55e258daab78 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |   |     `-ImplicitCastExpr 0x55e258daab60 <col:13> 'int *' <LValueToRValue>
// |   |       `-DeclRefExpr 0x55e258daab40 <col:13> 'int *' lvalue ParmVar 0x55e258daa4e0 'y' 'int *'
// |   |-CompoundStmt 0x55e258daac58 <col:16, line:18:3>
// |   | `-ReturnStmt 0x55e258daac48 <line:17:5, col:13>
// |   |   `-ImplicitCastExpr 0x55e258daac30 <col:12, col:13> 'int' <LValueToRValue>
// |   |     `-UnaryOperator 0x55e258daac18 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |   |       `-ImplicitCastExpr 0x55e258daac00 <col:13> 'int *' <LValueToRValue>
// |   |         `-DeclRefExpr 0x55e258daabe0 <col:13> 'int *' lvalue ParmVar 0x55e258daa460 'x' 'int *'
// |   `-CompoundStmt 0x55e258daace8 <line:18:10, line:20:3>
// |     `-ReturnStmt 0x55e258daacd8 <line:19:5, col:13>
// |       `-ImplicitCastExpr 0x55e258daacc0 <col:12, col:13> 'int' <LValueToRValue>
// |         `-UnaryOperator 0x55e258daaca8 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// |           `-ImplicitCastExpr 0x55e258daac90 <col:13> 'int *' <LValueToRValue>
// |             `-DeclRefExpr 0x55e258daac70 <col:13> 'int *' lvalue ParmVar 0x55e258daa4e0 'y' 'int *'
// |-AnnotateAttr 0x55e258daa680 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#vx #vy : _|END"
// |-AnnotateAttr 0x55e258daa730 <col:27, col:68> pulse "erased_arg:#px #py : _|END"
// |-AnnotateAttr 0x55e258daa7b0 <line:3:23, col:61> pulse "requires:x |->Frac px vx|END"
// |-AnnotateAttr 0x55e258daa830 <col:23, col:61> pulse "requires:y |->Frac py vy|END"
// |-AnnotateAttr 0x55e258daa8b0 <line:5:22, col:58> pulse "returns:n : int32|END"
// |-AnnotateAttr 0x55e258daa920 <line:4:24, col:71> pulse "ensures:x |->Frac px vx|END"
// |-AnnotateAttr 0x55e258daa9d0 <col:24, col:71> pulse "ensures:y |->Frac py vy|END"
// `-AnnotateAttr 0x55e258daaa50 <col:24, col:71> pulse "ensures:pure(as_int n == max_spec(as_int vx)(as_int vy))|END"
