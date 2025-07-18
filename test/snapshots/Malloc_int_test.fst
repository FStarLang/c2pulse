module Malloc_int_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn new_heap_ref
(v : Int32.t)
returns r:ref int32
ensures r |-> v
ensures freeable r
{
let mut v : Int32.t = v;
let r0 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut r : (ref Int32.t) = r0;
(! r) := (! v);
(! r);
}

//Dumping the Clang AST.
// FunctionDecl 0x57057c819418 </home/t-visinghal/Applications/src/c2pulse/test/general/malloc_int_test.c:7:1, line:12:1> line:7:6 new_heap_ref 'int *(int)'
// |-ParmVarDecl 0x57057c819348 <col:19, col:23> col:23 used v 'int'
// |-CompoundStmt 0x57057c8198f0 <line:8:1, line:12:1>
// | |-DeclStmt 0x57057c8197e8 <line:9:5, col:39>
// | | `-VarDecl 0x57057c819668 <col:5, col:38> col:10 used r 'int *' cinit
// | |   `-CStyleCastExpr 0x57057c8197c0 <col:14, col:38> 'int *' <BitCast>
// | |     `-CallExpr 0x57057c819778 <col:20, col:38> 'void *'
// | |       |-ImplicitCastExpr 0x57057c819760 <col:20> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x57057c8196d0 <col:20> 'void *(unsigned long)' Function 0x57057c801e80 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x57057c819708 <col:27, col:37> 'unsigned long' sizeof 'int'
// | |-BinaryOperator 0x57057c819888 <line:10:5, col:10> 'int' '='
// | | |-UnaryOperator 0x57057c819838 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x57057c819820 <col:6> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x57057c819800 <col:6> 'int *' lvalue Var 0x57057c819668 'r' 'int *'
// | | `-ImplicitCastExpr 0x57057c819870 <col:10> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x57057c819850 <col:10> 'int' lvalue ParmVar 0x57057c819348 'v' 'int'
// | `-ReturnStmt 0x57057c8198e0 <line:11:5, col:12>
// |   `-ImplicitCastExpr 0x57057c8198c8 <col:12> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x57057c8198a8 <col:12> 'int *' lvalue Var 0x57057c819668 'r' 'int *'
// |-AnnotateAttr 0x57057c8194c8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:r:ref int32|END"
// |-AnnotateAttr 0x57057c819570 <line:4:24, col:71> pulse "ensures:r |-> v|END"
// `-AnnotateAttr 0x57057c8195e0 <col:24, col:71> pulse "ensures:freeable r|END"
