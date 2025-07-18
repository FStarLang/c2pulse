module Issue35_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn empty_branch ()
{
let x0 : Int32.t = 1l;
let mut x : Int32.t = x0;
if((int32_to_bool (bool_to_int32 (Int32.eq (! x) 1l))))
{
();
}
else
{
();
};
}

//Dumping the Clang AST.
// FunctionDecl 0x64af911141f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue35_test.c:1:1, line:12:1> line:1:6 empty_branch 'void ()'
// `-CompoundStmt 0x64af91114490 <line:2:1, line:12:1>
//   |-DeclStmt 0x64af91114380 <line:3:3, col:12>
//   | `-VarDecl 0x64af911142f8 <col:3, col:11> col:7 used x 'int' cinit
//   |   `-IntegerLiteral 0x64af91114360 <col:11> 'int' 1
//   `-IfStmt 0x64af91114460 <line:4:3, line:11:3> has_else
//     |-BinaryOperator 0x64af911143f0 <line:4:7, col:12> 'int' '=='
//     | |-ImplicitCastExpr 0x64af911143d8 <col:7> 'int' <LValueToRValue>
//     | | `-DeclRefExpr 0x64af91114398 <col:7> 'int' lvalue Var 0x64af911142f8 'x' 'int'
//     | `-IntegerLiteral 0x64af911143b8 <col:12> 'int' 1
//     |-CompoundStmt 0x64af91114420 <line:5:3, line:7:3>
//     | `-ReturnStmt 0x64af91114410 <line:6:5>
//     `-CompoundStmt 0x64af91114448 <line:9:3, line:11:3>
//       `-ReturnStmt 0x64af91114438 <line:10:5>
