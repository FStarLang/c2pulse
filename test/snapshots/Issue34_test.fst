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
// FunctionDecl 0x6256763eab50 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue34_test.c:5:1, line:16:1> line:5:10 integer_promotion 'uint32_t ()'
// |-CompoundStmt 0x6256763eaf30 <line:6:1, line:16:1>
// | |-DeclStmt 0x6256763ead98 <line:7:3, col:17>
// | | `-VarDecl 0x6256763eacf8 <col:3, col:16> col:12 used x 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x6256763ead80 <col:16> 'uint32_t':'unsigned int' <IntegralCast>
// | |     `-IntegerLiteral 0x6256763ead60 <col:16> 'int' 1
// | `-IfStmt 0x6256763eaf00 <line:8:3, line:15:3> has_else
// |   |-BinaryOperator 0x6256763eae20 <line:8:7, col:12> 'int' '=='
// |   | |-ImplicitCastExpr 0x6256763eadf0 <col:7> 'uint32_t':'unsigned int' <LValueToRValue>
// |   | | `-DeclRefExpr 0x6256763eadb0 <col:7> 'uint32_t':'unsigned int' lvalue Var 0x6256763eacf8 'x' 'uint32_t':'unsigned int'
// |   | `-ImplicitCastExpr 0x6256763eae08 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |   |   `-IntegerLiteral 0x6256763eadd0 <col:12> 'int' 1
// |   |-CompoundStmt 0x6256763eae88 <line:9:3, line:11:3>
// |   | `-ReturnStmt 0x6256763eae78 <line:10:5, col:12>
// |   |   `-ImplicitCastExpr 0x6256763eae60 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |   |     `-IntegerLiteral 0x6256763eae40 <col:12> 'int' 0
// |   `-CompoundStmt 0x6256763eaee8 <line:13:3, line:15:3>
// |     `-ReturnStmt 0x6256763eaed8 <line:14:5, col:12>
// |       `-ImplicitCastExpr 0x6256763eaec0 <col:12> 'uint32_t':'unsigned int' <IntegralCast>
// |         `-IntegerLiteral 0x6256763eaea0 <col:12> 'int' 0
// `-AnnotateAttr 0x6256763eabf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:b:FStar.UInt32.t|END"
