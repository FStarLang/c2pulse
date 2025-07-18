module Test_bool_2

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(p : Int64.t)
{
let mut p : Int64.t = p;
if((int32_to_bool (bool_to_int32 (op_AmpAmp (int64_to_bool (! p)) (int64_to_bool (! p))))))
{
()
}
else
{
()
};
}

//Dumping the Clang AST.
// FunctionDecl 0x589d253c0d48 </home/t-visinghal/Applications/src/c2pulse/test/general/test_bool_2.c:4:1, line:6:1> line:4:5 foo 'int (long)'
// |-ParmVarDecl 0x589d253c0c78 <col:9, col:14> col:14 used p 'long'
// `-CompoundStmt 0x589d253c0eb0 <col:17, line:6:1>
//   `-IfStmt 0x589d253c0e90 <line:5:9, col:20>
//     |-BinaryOperator 0x589d253c0e68 <col:13, col:18> 'int' '&&'
//     | |-ImplicitCastExpr 0x589d253c0e38 <col:13> 'long' <LValueToRValue>
//     | | `-DeclRefExpr 0x589d253c0df8 <col:13> 'long' lvalue ParmVar 0x589d253c0c78 'p' 'long'
//     | `-ImplicitCastExpr 0x589d253c0e50 <col:18> 'long' <LValueToRValue>
//     |   `-DeclRefExpr 0x589d253c0e18 <col:18> 'long' lvalue ParmVar 0x589d253c0c78 'p' 'long'
//     `-NullStmt 0x589d253c0e88 <col:20>
