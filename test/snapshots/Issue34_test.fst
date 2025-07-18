module Issue34_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn integer_promotion ()
returns b:FStar.UInt32.t
{
let x0 : UInt32.t = (int32_to_uint32 1l);
let mut x : UInt32.t = x0;
if((int32_to_bool (bool_to_int32 (UInt32.eq (! x) (int32_to_uint32 1l)))))
{
(int32_to_uint32 0l);
}
else
{
(int32_to_uint32 0l);
};
}

//Dumping the Clang AST.
// FunctionDecl 0x5d1df41cbb50 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue34_test.c:5:1, line:16:1> line:5:10 integer_promotion 'uint32_t ()'
// |-CompoundStmt 0x5d1df41cbf30 <line:6:1, line:16:1>
// | |-DeclStmt 0x5d1df41cbd98 <line:7:3, col:17>
// | | `-VarDecl 0x5d1df41cbcf8 <col:3, col:16> col:12 used x 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x5d1df41cbd80 <col:16> 'uint32_t':'unsigned int' <IntegralCast>
// | |     `-IntegerLiteral 0x5d1df41cbd60 <col:16> 'int' 1
// | `-IfStmt 0x5d1df41cbf00 <line:8:3, line:15:3> has_else
// |   |-BinaryOperator 0x5d1df41cbe20 <line:8:7, col:12> 'int' '=='
// |   | |-ImplicitCastExpr 0x5d1df41cbdf0 <col:7> 'uint32_t':'unsigned int' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5d1df41cbdb0 <col:7> 'uint32_t':'unsigned int' lvalue Var 0x5d1df41cbcf8 'x' 'uint32_t':'unsigned int'
// |   | `-ImplicitCastExpr 0x5d1df41cbe08 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |   |   `-IntegerLiteral 0x5d1df41cbdd0 <col:12> 'int' 1
// |   |-CompoundStmt 0x5d1df41cbe88 <line:9:3, line:11:3>
// |   | `-ReturnStmt 0x5d1df41cbe78 <line:10:5, col:12>
// |   |   `-ImplicitCastExpr 0x5d1df41cbe60 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |   |     `-IntegerLiteral 0x5d1df41cbe40 <col:12> 'int' 0
// |   `-CompoundStmt 0x5d1df41cbee8 <line:13:3, line:15:3>
// |     `-ReturnStmt 0x5d1df41cbed8 <line:14:5, col:12>
// |       `-ImplicitCastExpr 0x5d1df41cbec0 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |         `-IntegerLiteral 0x5d1df41cbea0 <col:12> 'int' 0
// `-AnnotateAttr 0x5d1df41cbbf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:b:FStar.UInt32.t|END"
