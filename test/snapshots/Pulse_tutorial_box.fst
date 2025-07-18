module Pulse_tutorial_box

#lang-pulse

open Pulse
open Pulse.Lib.C



ghost fn freebie () requires emp ensures pure False { admit(); }
fn test_empty ()
requires emp
returns i:int32
ensures emp
{
0l;
}

fn test_freebie ()
returns i:int32
ensures pure False
{
freebie();
0l;
}

fn new_heap_ref
(v : Int32.t)
returns i:ref int32
ensures i |-> v
ensures freeable i
{
let mut v : Int32.t = v;
let r0 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut r : (ref Int32.t) = r0;
(! r) := (! v);
(! r);
}

fn last_value_of
(r : ref Int32.t)
(#w:_)
requires (r |-> w) ** freeable r
returns i:int32
ensures pure (i == w)
{
let mut r : (ref Int32.t) = r;
let v1 : Int32.t = (! (! r));
let mut v : Int32.t = v1;
(free_ref (! r));
(! v);
}

fn copy_free_box
(r : ref Int32.t)
(#w:_)
requires r |-> w
requires freeable r
returns s:ref int32
ensures s |-> w
ensures freeable s
{
let mut r : (ref Int32.t) = r;
let v2 : Int32.t = (! (! r));
let mut v : Int32.t = v2;
(free_ref (! r));
let s3 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut s : (ref Int32.t) = s3;
(! s) := (! v);
(! s);
}

fn copy_box
(r : ref Int32.t)
(#w:_)
requires r |-> w
returns s:ref int32
ensures (r |-> w) ** (s |-> w) ** freeable s
{
let mut r : (ref Int32.t) = r;
let v4 : Int32.t = (! (! r));
let mut v : Int32.t = v4;
let s5 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut s : (ref Int32.t) = s5;
(! s) := (! v);
(! s);
}

//Dumping the Clang AST.
// FunctionDecl 0x5698b3360248 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:18:1, line:19:13> line:18:5 test_empty 'int (void)'
// |-CompoundStmt 0x5698b33604a0 <line:19:1, col:13>
// | `-ReturnStmt 0x5698b3360490 <col:3, col:10>
// |   `-IntegerLiteral 0x5698b3360470 <col:10> 'int' 0
// |-AnnotateAttr 0x5698b33602f0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5698b3360390 <line:5:22, col:58> pulse "returns:i:int32|END"
// `-AnnotateAttr 0x5698b3360400 <line:4:24, col:71> pulse "ensures:emp|END"
// FunctionDecl 0x5698b33605c8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:23:1, line:27:1> line:23:5 test_freebie 'int (void)'
// |-CompoundStmt 0x5698b33628b0 <line:24:1, line:27:1>
// | |-AttributedStmt 0x5698b3360830 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:25:21>
// | | |-AnnotateAttr 0x5698b33607c0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:freebie()|END"
// | | `-NullStmt 0x5698b33607b8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:25:21>
// | `-ReturnStmt 0x5698b33628a0 <line:26:5, col:12>
// |   `-IntegerLiteral 0x5698b3360848 <col:12> 'int' 0
// |-AnnotateAttr 0x5698b3360670 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:i:int32|END"
// `-AnnotateAttr 0x5698b3360710 <line:4:24, col:71> pulse "ensures:pure False|END"
// FunctionDecl 0x5698b3362a68 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:32:1, line:37:1> line:32:6 new_heap_ref 'int *(int)'
// |-ParmVarDecl 0x5698b3362990 <col:19, col:23> col:23 used v 'int'
// |-CompoundStmt 0x5698b3362f40 <line:33:1, line:37:1>
// | |-DeclStmt 0x5698b3362e38 <line:34:5, col:39>
// | | `-VarDecl 0x5698b3362cb8 <col:5, col:38> col:10 used r 'int *' cinit
// | |   `-CStyleCastExpr 0x5698b3362e10 <col:14, col:38> 'int *' <BitCast>
// | |     `-CallExpr 0x5698b3362dc8 <col:20, col:38> 'void *'
// | |       |-ImplicitCastExpr 0x5698b3362db0 <col:20> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x5698b3362d20 <col:20> 'void *(unsigned long)' Function 0x5698b3227790 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x5698b3362d58 <col:27, col:37> 'unsigned long' sizeof 'int'
// | |-BinaryOperator 0x5698b3362ed8 <line:35:5, col:10> 'int' '='
// | | |-UnaryOperator 0x5698b3362e88 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x5698b3362e70 <col:6> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5698b3362e50 <col:6> 'int *' lvalue Var 0x5698b3362cb8 'r' 'int *'
// | | `-ImplicitCastExpr 0x5698b3362ec0 <col:10> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5698b3362ea0 <col:10> 'int' lvalue ParmVar 0x5698b3362990 'v' 'int'
// | `-ReturnStmt 0x5698b3362f30 <line:36:5, col:12>
// |   `-ImplicitCastExpr 0x5698b3362f18 <col:12> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x5698b3362ef8 <col:12> 'int *' lvalue Var 0x5698b3362cb8 'r' 'int *'
// |-AnnotateAttr 0x5698b3362b18 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:i:ref int32|END"
// |-AnnotateAttr 0x5698b3362bc0 <line:4:24, col:71> pulse "ensures:i |-> v|END"
// `-AnnotateAttr 0x5698b3362c30 <col:24, col:71> pulse "ensures:freeable i|END"
// FunctionDecl 0x5698b3363148 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:44:1, line:49:1> line:44:5 last_value_of 'int (int *)'
// |-ParmVarDecl 0x5698b3363078 <col:19, col:24> col:24 used r 'int *'
// |-CompoundStmt 0x5698b3363660 <line:45:1, line:49:1>
// | |-DeclStmt 0x5698b33634f8 <line:46:4, col:14>
// | | `-VarDecl 0x5698b3363428 <col:4, col:13> col:8 used v 'int' cinit
// | |   `-ImplicitCastExpr 0x5698b33634e0 <col:12, col:13> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x5698b33634c8 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x5698b33634b0 <col:13> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x5698b3363490 <col:13> 'int *' lvalue ParmVar 0x5698b3363078 'r' 'int *'
// | |-CallExpr 0x5698b33635b8 <line:47:4, col:10> 'void'
// | | |-ImplicitCastExpr 0x5698b33635a0 <col:4> 'void (*)(void *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x5698b3363510 <col:4> 'void (void *)' Function 0x5698b3228708 'free' 'void (void *)'
// | | `-ImplicitCastExpr 0x5698b3363600 <col:9> 'void *' <BitCast>
// | |   `-ImplicitCastExpr 0x5698b33635e8 <col:9> 'int *' <LValueToRValue>
// | |     `-DeclRefExpr 0x5698b3363530 <col:9> 'int *' lvalue ParmVar 0x5698b3363078 'r' 'int *'
// | `-ReturnStmt 0x5698b3363650 <line:48:4, col:11>
// |   `-ImplicitCastExpr 0x5698b3363638 <col:11> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x5698b3363618 <col:11> 'int' lvalue Var 0x5698b3363428 'v' 'int'
// |-AnnotateAttr 0x5698b33631f8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:_|END"
// |-AnnotateAttr 0x5698b33632a0 <line:3:23, col:61> pulse "requires:(r |-> w) ** freeable r|END"
// |-AnnotateAttr 0x5698b3363320 <line:5:22, col:58> pulse "returns:i:int32|END"
// `-AnnotateAttr 0x5698b3363390 <line:4:24, col:71> pulse "ensures:pure (i == w)|END"
// FunctionDecl 0x5698b33638e0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:57:1, line:64:1> line:57:6 copy_free_box 'int *(int *)'
// |-ParmVarDecl 0x5698b33637f0 <col:21, col:26> col:26 used r 'int *'
// |-CompoundStmt 0x5698b3364098 <line:58:1, line:64:1>
// | |-DeclStmt 0x5698b3363d48 <line:59:5, col:15>
// | | `-VarDecl 0x5698b3363c78 <col:5, col:14> col:9 used v 'int' cinit
// | |   `-ImplicitCastExpr 0x5698b3363d30 <col:13, col:14> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x5698b3363d18 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x5698b3363d00 <col:14> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x5698b3363ce0 <col:14> 'int *' lvalue ParmVar 0x5698b33637f0 'r' 'int *'
// | |-CallExpr 0x5698b3363dd0 <line:60:5, col:11> 'void'
// | | |-ImplicitCastExpr 0x5698b3363db8 <col:5> 'void (*)(void *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x5698b3363d60 <col:5> 'void (void *)' Function 0x5698b3228708 'free' 'void (void *)'
// | | `-ImplicitCastExpr 0x5698b3363e18 <col:10> 'void *' <BitCast>
// | |   `-ImplicitCastExpr 0x5698b3363e00 <col:10> 'int *' <LValueToRValue>
// | |     `-DeclRefExpr 0x5698b3363d80 <col:10> 'int *' lvalue ParmVar 0x5698b33637f0 'r' 'int *'
// | |-DeclStmt 0x5698b3363f90 <line:61:5, col:39>
// | | `-VarDecl 0x5698b3363e48 <col:5, col:38> col:10 used s 'int *' cinit
// | |   `-CStyleCastExpr 0x5698b3363f68 <col:14, col:38> 'int *' <BitCast>
// | |     `-CallExpr 0x5698b3363f20 <col:20, col:38> 'void *'
// | |       |-ImplicitCastExpr 0x5698b3363f08 <col:20> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x5698b3363eb0 <col:20> 'void *(unsigned long)' Function 0x5698b3227790 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x5698b3363ee8 <col:27, col:37> 'unsigned long' sizeof 'int'
// | |-BinaryOperator 0x5698b3364030 <line:62:5, col:10> 'int' '='
// | | |-UnaryOperator 0x5698b3363fe0 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x5698b3363fc8 <col:6> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5698b3363fa8 <col:6> 'int *' lvalue Var 0x5698b3363e48 's' 'int *'
// | | `-ImplicitCastExpr 0x5698b3364018 <col:10> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5698b3363ff8 <col:10> 'int' lvalue Var 0x5698b3363c78 'v' 'int'
// | `-ReturnStmt 0x5698b3364088 <line:63:5, col:12>
// |   `-ImplicitCastExpr 0x5698b3364070 <col:12> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x5698b3364050 <col:12> 'int *' lvalue Var 0x5698b3363e48 's' 'int *'
// |-AnnotateAttr 0x5698b3363990 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:_|END"
// |-AnnotateAttr 0x5698b3363a30 <line:3:23, col:61> pulse "requires:r |-> w|END"
// |-AnnotateAttr 0x5698b3363aa0 <col:23, col:61> pulse "requires:freeable r|END"
// |-AnnotateAttr 0x5698b3363b10 <line:5:22, col:58> pulse "returns:s:ref int32|END"
// |-AnnotateAttr 0x5698b3363b80 <line:4:24, col:71> pulse "ensures:s |-> w|END"
// `-AnnotateAttr 0x5698b3363bf0 <col:24, col:71> pulse "ensures:freeable s|END"
// FunctionDecl 0x5698b3364278 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_box.c:70:1, line:76:1> line:70:6 copy_box 'int *(int *)'
// |-ParmVarDecl 0x5698b33641e0 <col:15, col:20> col:20 used r 'int *'
// |-CompoundStmt 0x5698b33648c0 <line:71:1, line:76:1>
// | |-DeclStmt 0x5698b3364628 <line:72:5, col:15>
// | | `-VarDecl 0x5698b3364558 <col:5, col:14> col:9 used v 'int' cinit
// | |   `-ImplicitCastExpr 0x5698b3364610 <col:13, col:14> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x5698b33645f8 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x5698b33645e0 <col:14> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x5698b33645c0 <col:14> 'int *' lvalue ParmVar 0x5698b33641e0 'r' 'int *'
// | |-DeclStmt 0x5698b33647a0 <line:73:5, col:39>
// | | `-VarDecl 0x5698b3364658 <col:5, col:38> col:10 used s 'int *' cinit
// | |   `-CStyleCastExpr 0x5698b3364778 <col:14, col:38> 'int *' <BitCast>
// | |     `-CallExpr 0x5698b3364730 <col:20, col:38> 'void *'
// | |       |-ImplicitCastExpr 0x5698b3364718 <col:20> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x5698b33646c0 <col:20> 'void *(unsigned long)' Function 0x5698b3227790 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x5698b33646f8 <col:27, col:37> 'unsigned long' sizeof 'int'
// | |-BinaryOperator 0x5698b3364840 <line:74:5, col:10> 'int' '='
// | | |-UnaryOperator 0x5698b33647f0 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x5698b33647d8 <col:6> 'int *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5698b33647b8 <col:6> 'int *' lvalue Var 0x5698b3364658 's' 'int *'
// | | `-ImplicitCastExpr 0x5698b3364828 <col:10> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5698b3364808 <col:10> 'int' lvalue Var 0x5698b3364558 'v' 'int'
// | `-ReturnStmt 0x5698b3364898 <line:75:5, col:12>
// |   `-ImplicitCastExpr 0x5698b3364880 <col:12> 'int *' <LValueToRValue>
// |     `-DeclRefExpr 0x5698b3364860 <col:12> 'int *' lvalue Var 0x5698b3364658 's' 'int *'
// |-AnnotateAttr 0x5698b3364328 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:_|END"
// |-AnnotateAttr 0x5698b33643d0 <line:3:23, col:61> pulse "requires:r |-> w|END"
// |-AnnotateAttr 0x5698b3364440 <line:5:22, col:58> pulse "returns:s:ref int32|END"
// `-AnnotateAttr 0x5698b33644b0 <line:4:24, col:71> pulse "ensures:(r |-> w) ** (s |-> w) ** freeable s|END"
