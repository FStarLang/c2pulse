module Issue44_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn bar ()
{
let x0 : Int32.t = 1l;
let mut x : Int32.t = x0;
();
}

//Dumping the Clang AST.
// FunctionDecl 0x5670c4f70350 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue44_test.c:1:1, col:14> col:6 bar 'void (void)'
// FunctionDecl 0x5670c4f70490 prev 0x5670c4f70350 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue44_test.c:3:1, line:6:1> line:3:6 bar 'void (void)'
// `-CompoundStmt 0x5670c4f70600 <col:12, line:6:1>
//   |-DeclStmt 0x5670c4f705d8 <line:4:2, col:11>
//   | `-VarDecl 0x5670c4f70550 <col:2, col:10> col:6 x 'int' cinit
//   |   `-IntegerLiteral 0x5670c4f705b8 <col:10> 'int' 1
//   `-ReturnStmt 0x5670c4f705f0 <line:5:2>
