module Issue43_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(p : ref Int32.t)
{
let mut p : (ref Int32.t) = p;
();
}

fn bar ()
{
(foo null);
}

fn baz ()
{
let p0 : (ref Int32.t) = null;
let mut p : (ref Int32.t) = p0;
(foo (! p));
}

//Dumping the Clang AST.
// FunctionDecl 0x5a0ceb58c870 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:3:1, line:4:1> line:3:6 used foo 'void (int *)'
// |-ParmVarDecl 0x5a0ceb58c7a8 <col:10, col:15> col:15 p 'int *'
// `-CompoundStmt 0x5a0ceb58c920 <col:18, line:4:1>
// FunctionDecl 0x5a0ceb58c980 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:6:1, line:8:1> line:6:6 bar 'void ()'
// `-CompoundStmt 0x5a0ceb58cb60 <col:12, line:8:1>
//   `-CallExpr 0x5a0ceb58cb18 <line:7:2, col:10> 'void'
//     |-ImplicitCastExpr 0x5a0ceb58cb00 <col:2> 'void (*)(int *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x5a0ceb58ca28 <col:2> 'void (int *)' Function 0x5a0ceb58c870 'foo' 'void (int *)'
//     `-ImplicitCastExpr 0x5a0ceb58cb48 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <NullToPointer>
//       `-ParenExpr 0x5a0ceb58caa8 <col:14, col:23> 'void *'
//         `-CStyleCastExpr 0x5a0ceb58ca80 <col:15, col:22> 'void *' <NullToPointer>
//           `-IntegerLiteral 0x5a0ceb58ca48 <col:22> 'int' 0
// FunctionDecl 0x5a0ceb58cb98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:10:1, line:13:1> line:10:6 baz 'void ()'
// `-CompoundStmt 0x5a0ceb58ce10 <col:12, line:13:1>
//   |-DeclStmt 0x5a0ceb58cd58 <line:11:2, col:15>
//   | `-VarDecl 0x5a0ceb58cc58 <col:2, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:11:7 used p 'int *' cinit
//   |   `-ImplicitCastExpr 0x5a0ceb58cd40 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <NullToPointer>
//   |     `-ParenExpr 0x5a0ceb58cd20 <col:14, col:23> 'void *'
//   |       `-CStyleCastExpr 0x5a0ceb58ccf8 <col:15, col:22> 'void *' <NullToPointer>
//   |         `-IntegerLiteral 0x5a0ceb58ccc0 <col:22> 'int' 0
//   `-CallExpr 0x5a0ceb58cdc8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:12:2, col:7> 'void'
//     |-ImplicitCastExpr 0x5a0ceb58cdb0 <col:2> 'void (*)(int *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x5a0ceb58cd70 <col:2> 'void (int *)' Function 0x5a0ceb58c870 'foo' 'void (int *)'
//     `-ImplicitCastExpr 0x5a0ceb58cdf8 <col:6> 'int *' <LValueToRValue>
//       `-DeclRefExpr 0x5a0ceb58cd90 <col:6> 'int *' lvalue Var 0x5a0ceb58cc58 'p' 'int *'
