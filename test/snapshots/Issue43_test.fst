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
// FunctionDecl 0x60da1d3456c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:3:1, line:4:1> line:3:6 used foo 'void (int *)'
// |-ParmVarDecl 0x60da1d3455f8 <col:10, col:15> col:15 p 'int *'
// `-CompoundStmt 0x60da1d345770 <col:18, line:4:1>
// FunctionDecl 0x60da1d3457d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:6:1, line:8:1> line:6:6 bar 'void ()'
// `-CompoundStmt 0x60da1d3459b0 <col:12, line:8:1>
//   `-CallExpr 0x60da1d345968 <line:7:2, col:10> 'void'
//     |-ImplicitCastExpr 0x60da1d345950 <col:2> 'void (*)(int *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x60da1d345878 <col:2> 'void (int *)' Function 0x60da1d3456c0 'foo' 'void (int *)'
//     `-ImplicitCastExpr 0x60da1d345998 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <NullToPointer>
//       `-ParenExpr 0x60da1d3458f8 <col:14, col:23> 'void *'
//         `-CStyleCastExpr 0x60da1d3458d0 <col:15, col:22> 'void *' <NullToPointer>
//           `-IntegerLiteral 0x60da1d345898 <col:22> 'int' 0
// FunctionDecl 0x60da1d3459e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:10:1, line:13:1> line:10:6 baz 'void ()'
// `-CompoundStmt 0x60da1d345c60 <col:12, line:13:1>
//   |-DeclStmt 0x60da1d345ba8 <line:11:2, col:15>
//   | `-VarDecl 0x60da1d345aa8 <col:2, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:11:7 used p 'int *' cinit
//   |   `-ImplicitCastExpr 0x60da1d345b90 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <NullToPointer>
//   |     `-ParenExpr 0x60da1d345b70 <col:14, col:23> 'void *'
//   |       `-CStyleCastExpr 0x60da1d345b48 <col:15, col:22> 'void *' <NullToPointer>
//   |         `-IntegerLiteral 0x60da1d345b10 <col:22> 'int' 0
//   `-CallExpr 0x60da1d345c18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue43_test.c:12:2, col:7> 'void'
//     |-ImplicitCastExpr 0x60da1d345c00 <col:2> 'void (*)(int *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x60da1d345bc0 <col:2> 'void (int *)' Function 0x60da1d3456c0 'foo' 'void (int *)'
//     `-ImplicitCastExpr 0x60da1d345c48 <col:6> 'int *' <LValueToRValue>
//       `-DeclRefExpr 0x60da1d345be0 <col:6> 'int *' lvalue Var 0x60da1d345aa8 'p' 'int *'
