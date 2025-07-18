module Pulse_tutorial_conditionals

#lang-pulse

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max
(x : ref Int32.t)
(y : ref Int32.t)
(#vx #vy:_)
(#px #py:_)
requires x |-> Frac px vx
requires y |-> Frac py vy
returns n:int32
ensures x |-> Frac px vx
ensures y |-> Frac py vy
ensures pure (as_int n == max_spec (as_int vx) (as_int vy))
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

[@@expect_failure]
fn max_alt
(x : ref Int32.t)
(y : ref Int32.t)
(#vx #vy:_)
(#px #py:_)
requires x |-> Frac px vx
requires y |-> Frac py vy
returns n:int32
ensures x |-> Frac px vx
ensures y |-> Frac py vy
ensures pure (as_int n == max_spec (as_int vx) (as_int vy))
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
let result0 : Int32.t = 0l;
let mut result : Int32.t = result0;
let vx1 : Int32.t = (! (! x));
let mut vx : Int32.t = vx1;
let vy2 : Int32.t = (! (! y));
let mut vy : Int32.t = vy2;
if((int32_to_bool (bool_to_int32 (Int32.gt (! vx) (! vy)))))
ensures exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (result |-> r) ** pure (as_int r == max_spec (as_int vx) (as_int vy))
{
result := (! vx);
}
else
{
result := (! vy);
};
(! result);
}

fn read_nullable
(r : ref Int32.t)
(#w:option int32)
(#p:_)
requires r |->? Frac p w
returns i:int32
ensures r |->? Frac p w
ensures pure (Some? w ==> Some?.v w == i)
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
elim_intro_null !r;
0l;
}
else
{
elim_non_null !r;
let v3 : Int32.t = (! (! r));
let mut v : Int32.t = v3;
intro_non_null !r;
(! v);
};
}

fn write_nullable
(r : ref Int32.t)
(v : Int32.t)
(#w:option int32)
requires r |->? w
ensures exists* x. (r |->? x) ** pure (if Some? w then x == Some v else x == w)
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
elim_intro_null !r;
}
else
{
elim_non_null !r;
(! r) := (! v);
intro_non_null !r;
};
}

//Dumping the Clang AST.
// FunctionDecl 0x561dd2ea68b8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:17:1, line:27:1> line:17:5 max 'int (int *, int *)'
// |-ParmVarDecl 0x561dd2ea6750 <col:10, col:15> col:15 used x 'int *'
// |-ParmVarDecl 0x561dd2ea67d0 <col:18, col:23> col:23 used y 'int *'
// |-CompoundStmt 0x561dd2ea9050 <line:18:1, line:27:1>
// | `-IfStmt 0x561dd2ea9020 <line:19:5, line:26:5> has_else
// |   |-BinaryOperator 0x561dd2ea8ee0 <line:19:9, col:15> 'int' '>'
// |   | |-ImplicitCastExpr 0x561dd2ea8eb0 <col:9, col:10> 'int' <LValueToRValue>
// |   | | `-UnaryOperator 0x561dd2ea8e48 <col:9, col:10> 'int' lvalue prefix '*' cannot overflow
// |   | |   `-ImplicitCastExpr 0x561dd2ea8e30 <col:10> 'int *' <LValueToRValue>
// |   | |     `-DeclRefExpr 0x561dd2ea8e10 <col:10> 'int *' lvalue ParmVar 0x561dd2ea6750 'x' 'int *'
// |   | `-ImplicitCastExpr 0x561dd2ea8ec8 <col:14, col:15> 'int' <LValueToRValue>
// |   |   `-UnaryOperator 0x561dd2ea8e98 <col:14, col:15> 'int' lvalue prefix '*' cannot overflow
// |   |     `-ImplicitCastExpr 0x561dd2ea8e80 <col:15> 'int *' <LValueToRValue>
// |   |       `-DeclRefExpr 0x561dd2ea8e60 <col:15> 'int *' lvalue ParmVar 0x561dd2ea67d0 'y' 'int *'
// |   |-CompoundStmt 0x561dd2ea8f78 <line:20:5, line:22:5>
// |   | `-ReturnStmt 0x561dd2ea8f68 <line:21:9, col:17>
// |   |   `-ImplicitCastExpr 0x561dd2ea8f50 <col:16, col:17> 'int' <LValueToRValue>
// |   |     `-UnaryOperator 0x561dd2ea8f38 <col:16, col:17> 'int' lvalue prefix '*' cannot overflow
// |   |       `-ImplicitCastExpr 0x561dd2ea8f20 <col:17> 'int *' <LValueToRValue>
// |   |         `-DeclRefExpr 0x561dd2ea8f00 <col:17> 'int *' lvalue ParmVar 0x561dd2ea6750 'x' 'int *'
// |   `-CompoundStmt 0x561dd2ea9008 <line:24:5, line:26:5>
// |     `-ReturnStmt 0x561dd2ea8ff8 <line:25:9, col:17>
// |       `-ImplicitCastExpr 0x561dd2ea8fe0 <col:16, col:17> 'int' <LValueToRValue>
// |         `-UnaryOperator 0x561dd2ea8fc8 <col:16, col:17> 'int' lvalue prefix '*' cannot overflow
// |           `-ImplicitCastExpr 0x561dd2ea8fb0 <col:17> 'int *' <LValueToRValue>
// |             `-DeclRefExpr 0x561dd2ea8f90 <col:17> 'int *' lvalue ParmVar 0x561dd2ea67d0 'y' 'int *'
// |-AnnotateAttr 0x561dd2ea6970 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#vx #vy:_|END"
// |-AnnotateAttr 0x561dd2ea6a10 <col:27, col:68> pulse "erased_arg:#px #py:_|END"
// |-AnnotateAttr 0x561dd2ea6a80 <line:3:23, col:61> pulse "requires:x |-> Frac px vx|END"
// |-AnnotateAttr 0x561dd2ea6b00 <col:23, col:61> pulse "requires:y |-> Frac py vy|END"
// |-AnnotateAttr 0x561dd2ea6b80 <line:5:22, col:58> pulse "returns:n:int32|END"
// |-AnnotateAttr 0x561dd2ea6bf0 <line:4:24, col:71> pulse "ensures:x |-> Frac px vx|END"
// |-AnnotateAttr 0x561dd2ea8cf0 <col:24, col:71> pulse "ensures:y |-> Frac py vy|END"
// `-AnnotateAttr 0x561dd2ea8d70 <col:24, col:71> pulse "ensures:pure (as_int n == max_spec (as_int vx) (as_int vy))|END"
// FunctionDecl 0x561dd2ea93e0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:38:1, line:59:1> line:38:5 max_alt 'int (int *, int *)'
// |-ParmVarDecl 0x561dd2ea92c0 <col:13, col:18> col:18 used x 'int *'
// |-ParmVarDecl 0x561dd2ea9340 <col:21, col:26> col:26 used y 'int *'
// |-CompoundStmt 0x561dd2eaa048 <line:39:1, line:59:1>
// | |-DeclStmt 0x561dd2ea99d0 <line:40:3, col:17>
// | | `-VarDecl 0x561dd2ea9948 <col:3, col:16> col:7 used result 'int' cinit
// | |   `-IntegerLiteral 0x561dd2ea99b0 <col:16> 'int' 0
// | |-DeclStmt 0x561dd2ea9ad0 <line:41:3, col:14>
// | | `-VarDecl 0x561dd2ea9a00 <col:3, col:13> col:7 used vx 'int' cinit
// | |   `-ImplicitCastExpr 0x561dd2ea9ab8 <col:12, col:13> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x561dd2ea9aa0 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x561dd2ea9a88 <col:13> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x561dd2ea9a68 <col:13> 'int *' lvalue ParmVar 0x561dd2ea92c0 'x' 'int *'
// | |-DeclStmt 0x561dd2ea9bd0 <line:42:3, col:14>
// | | `-VarDecl 0x561dd2ea9b00 <col:3, col:13> col:7 used vy 'int' cinit
// | |   `-ImplicitCastExpr 0x561dd2ea9bb8 <col:12, col:13> 'int' <LValueToRValue>
// | |     `-UnaryOperator 0x561dd2ea9ba0 <col:12, col:13> 'int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x561dd2ea9b88 <col:13> 'int *' <LValueToRValue>
// | |         `-DeclRefExpr 0x561dd2ea9b68 <col:13> 'int *' lvalue ParmVar 0x561dd2ea9340 'y' 'int *'
// | |-IfStmt 0x561dd2ea9fc8 <line:43:3, line:57:3> has_else
// | | |-BinaryOperator 0x561dd2ea9c58 <line:43:7, col:12> 'int' '>'
// | | | |-ImplicitCastExpr 0x561dd2ea9c28 <col:7> 'int' <LValueToRValue>
// | | | | `-DeclRefExpr 0x561dd2ea9be8 <col:7> 'int' lvalue Var 0x561dd2ea9a00 'vx' 'int'
// | | | `-ImplicitCastExpr 0x561dd2ea9c40 <col:12> 'int' <LValueToRValue>
// | | |   `-DeclRefExpr 0x561dd2ea9c08 <col:12> 'int' lvalue Var 0x561dd2ea9b00 'vy' 'int'
// | | |-AttributedStmt 0x561dd2ea9f20 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:4:22, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:53:3>
// | | | |-AnnotateAttr 0x561dd2ea9e38 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:4:24, col:71> pulse "ensures:exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (result |-> r) ** pure (as_int r == max_spec (as_int vx) (as_int vy))|END"
// | | | `-CompoundStmt 0x561dd2ea9e20 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:51:3, line:53:3>
// | | |   `-BinaryOperator 0x561dd2ea9e00 <line:52:5, col:14> 'int' '='
// | | |     |-DeclRefExpr 0x561dd2ea9da8 <col:5> 'int' lvalue Var 0x561dd2ea9948 'result' 'int'
// | | |     `-ImplicitCastExpr 0x561dd2ea9de8 <col:14> 'int' <LValueToRValue>
// | | |       `-DeclRefExpr 0x561dd2ea9dc8 <col:14> 'int' lvalue Var 0x561dd2ea9a00 'vx' 'int'
// | | `-CompoundStmt 0x561dd2ea9fb0 <line:55:3, line:57:3>
// | |   `-BinaryOperator 0x561dd2ea9f90 <line:56:5, col:14> 'int' '='
// | |     |-DeclRefExpr 0x561dd2ea9f38 <col:5> 'int' lvalue Var 0x561dd2ea9948 'result' 'int'
// | |     `-ImplicitCastExpr 0x561dd2ea9f78 <col:14> 'int' <LValueToRValue>
// | |       `-DeclRefExpr 0x561dd2ea9f58 <col:14> 'int' lvalue Var 0x561dd2ea9b00 'vy' 'int'
// | |-NullStmt 0x561dd2ea9ff8 <line:57:4>
// | `-ReturnStmt 0x561dd2eaa038 <line:58:3, col:10>
// |   `-ImplicitCastExpr 0x561dd2eaa020 <col:10> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x561dd2eaa000 <col:10> 'int' lvalue Var 0x561dd2ea9948 'result' 'int'
// |-AnnotateAttr 0x561dd2ea9498 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
// |-AnnotateAttr 0x561dd2ea9540 <line:9:27, col:68> pulse "erased_arg:#vx #vy:_|END"
// |-AnnotateAttr 0x561dd2ea95b0 <col:27, col:68> pulse "erased_arg:#px #py:_|END"
// |-AnnotateAttr 0x561dd2ea9620 <line:3:23, col:61> pulse "requires:x |-> Frac px vx|END"
// |-AnnotateAttr 0x561dd2ea96a0 <col:23, col:61> pulse "requires:y |-> Frac py vy|END"
// |-AnnotateAttr 0x561dd2ea9720 <line:5:22, col:58> pulse "returns:n:int32|END"
// |-AnnotateAttr 0x561dd2ea9790 <line:4:24, col:71> pulse "ensures:x |-> Frac px vx|END"
// |-AnnotateAttr 0x561dd2ea9810 <col:24, col:71> pulse "ensures:y |-> Frac py vy|END"
// `-AnnotateAttr 0x561dd2ea9890 <col:24, col:71> pulse "ensures:pure (as_int n == max_spec (as_int vx) (as_int vy))|END"
// FunctionDecl 0x561dd2eaa2f8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:67:1, line:81:1> line:67:5 read_nullable 'int (int *)'
// |-ParmVarDecl 0x561dd2eaa220 <col:19, col:24> col:24 used r 'int *'
// |-CompoundStmt 0x561dd2eaac40 <line:68:1, line:81:1>
// | `-IfStmt 0x561dd2eaac10 <line:69:3, line:80:3> has_else
// |   |-BinaryOperator 0x561dd2eaa790 <line:69:7, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> 'int' '=='
// |   | |-ImplicitCastExpr 0x561dd2eaa760 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:69:7> 'int *' <LValueToRValue>
// |   | | `-DeclRefExpr 0x561dd2eaa6c0 <col:7> 'int *' lvalue ParmVar 0x561dd2eaa220 'r' 'int *'
// |   | `-ImplicitCastExpr 0x561dd2eaa778 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <BitCast>
// |   |   `-ParenExpr 0x561dd2eaa740 <col:14, col:23> 'void *'
// |   |     `-CStyleCastExpr 0x561dd2eaa718 <col:15, col:22> 'void *' <NullToPointer>
// |   |       `-IntegerLiteral 0x561dd2eaa6e0 <col:22> 'int' 0
// |   |-CompoundStmt 0x561dd2eaa8b8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:70:3, line:73:3>
// |   | |-AttributedStmt 0x561dd2eaa870 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:71:30>
// |   | | |-AnnotateAttr 0x561dd2eaa7f8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:elim_intro_null !r|END"
// |   | | `-NullStmt 0x561dd2eaa7f0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:71:30>
// |   | `-ReturnStmt 0x561dd2eaa8a8 <line:72:5, col:12>
// |   |   `-IntegerLiteral 0x561dd2eaa888 <col:12> 'int' 0
// |   `-CompoundStmt 0x561dd2eaabe0 <line:75:3, line:80:3>
// |     |-AttributedStmt 0x561dd2eaa9a0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:76:28>
// |     | |-AnnotateAttr 0x561dd2eaa920 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:elim_non_null !r|END"
// |     | `-NullStmt 0x561dd2eaa918 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:76:28>
// |     |-DeclStmt 0x561dd2eaaaa0 <line:77:5, col:15>
// |     | `-VarDecl 0x561dd2eaa9d0 <col:5, col:14> col:9 used v 'int' cinit
// |     |   `-ImplicitCastExpr 0x561dd2eaaa88 <col:13, col:14> 'int' <LValueToRValue>
// |     |     `-UnaryOperator 0x561dd2eaaa70 <col:13, col:14> 'int' lvalue prefix '*' cannot overflow
// |     |       `-ImplicitCastExpr 0x561dd2eaaa58 <col:14> 'int *' <LValueToRValue>
// |     |         `-DeclRefExpr 0x561dd2eaaa38 <col:14> 'int *' lvalue ParmVar 0x561dd2eaa220 'r' 'int *'
// |     |-AttributedStmt 0x561dd2eaab80 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:78:29>
// |     | |-AnnotateAttr 0x561dd2eaab00 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:intro_non_null !r|END"
// |     | `-NullStmt 0x561dd2eaaaf8 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:78:29>
// |     `-ReturnStmt 0x561dd2eaabd0 <line:79:5, col:12>
// |       `-ImplicitCastExpr 0x561dd2eaabb8 <col:12> 'int' <LValueToRValue>
// |         `-DeclRefExpr 0x561dd2eaab98 <col:12> 'int' lvalue Var 0x561dd2eaa9d0 'v' 'int'
// |-AnnotateAttr 0x561dd2eaa3a8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:option int32|END"
// |-AnnotateAttr 0x561dd2eaa450 <col:27, col:68> pulse "erased_arg:#p:_|END"
// |-AnnotateAttr 0x561dd2eaa4c0 <line:3:23, col:61> pulse "requires:r |->? Frac p w|END"
// |-AnnotateAttr 0x561dd2eaa540 <line:5:22, col:58> pulse "returns:i:int32|END"
// |-AnnotateAttr 0x561dd2eaa5b0 <line:4:24, col:71> pulse "ensures:r |->? Frac p w|END"
// `-AnnotateAttr 0x561dd2eaa630 <col:24, col:71> pulse "ensures:pure (Some? w ==> Some?.v w == i)|END"
// FunctionDecl 0x561dd2eaaf00 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:86:1, line:98:1> line:86:6 write_nullable 'void (int *, int)'
// |-ParmVarDecl 0x561dd2eaada0 <col:21, col:26> col:26 used r 'int *'
// |-ParmVarDecl 0x561dd2eaae20 <col:29, col:33> col:33 used v 'int'
// |-CompoundStmt 0x561dd2eab610 <line:87:1, line:98:1>
// | `-IfStmt 0x561dd2eab5e0 <line:88:3, line:97:3> has_else
// |   |-BinaryOperator 0x561dd2eab250 <line:88:7, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> 'int' '=='
// |   | |-ImplicitCastExpr 0x561dd2eab220 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:88:7> 'int *' <LValueToRValue>
// |   | | `-DeclRefExpr 0x561dd2eab180 <col:7> 'int *' lvalue ParmVar 0x561dd2eaada0 'r' 'int *'
// |   | `-ImplicitCastExpr 0x561dd2eab238 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <BitCast>
// |   |   `-ParenExpr 0x561dd2eab200 <col:14, col:23> 'void *'
// |   |     `-CStyleCastExpr 0x561dd2eab1d8 <col:15, col:22> 'void *' <NullToPointer>
// |   |       `-IntegerLiteral 0x561dd2eab1a0 <col:22> 'int' 0
// |   |-CompoundStmt 0x561dd2eab348 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:89:3, line:91:3>
// |   | `-AttributedStmt 0x561dd2eab330 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:90:30>
// |   |   |-AnnotateAttr 0x561dd2eab2b8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:elim_intro_null !r|END"
// |   |   `-NullStmt 0x561dd2eab2b0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:90:30>
// |   `-CompoundStmt 0x561dd2eab5b8 <line:93:3, line:97:3>
// |     |-AttributedStmt 0x561dd2eab420 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:94:28>
// |     | |-AnnotateAttr 0x561dd2eab3a8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:elim_non_null !r|END"
// |     | `-NullStmt 0x561dd2eab3a0 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:94:28>
// |     |-BinaryOperator 0x561dd2eab4c0 <line:95:5, col:10> 'int' '='
// |     | |-UnaryOperator 0x561dd2eab470 <col:5, col:6> 'int' lvalue prefix '*' cannot overflow
// |     | | `-ImplicitCastExpr 0x561dd2eab458 <col:6> 'int *' <LValueToRValue>
// |     | |   `-DeclRefExpr 0x561dd2eab438 <col:6> 'int *' lvalue ParmVar 0x561dd2eaada0 'r' 'int *'
// |     | `-ImplicitCastExpr 0x561dd2eab4a8 <col:10> 'int' <LValueToRValue>
// |     |   `-DeclRefExpr 0x561dd2eab488 <col:10> 'int' lvalue ParmVar 0x561dd2eaae20 'v' 'int'
// |     `-AttributedStmt 0x561dd2eab5a0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:96:29>
// |       |-AnnotateAttr 0x561dd2eab528 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:intro_non_null !r|END"
// |       `-NullStmt 0x561dd2eab520 </home/t-visinghal/Applications/src/c2pulse/test/general/pulse_tutorial_conditionals.c:96:29>
// |-AnnotateAttr 0x561dd2eaafb8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#w:option int32|END"
// |-AnnotateAttr 0x561dd2eab060 <line:3:23, col:61> pulse "requires:r |->? w|END"
// `-AnnotateAttr 0x561dd2eab0d0 <line:4:24, col:71> pulse "ensures:exists* x. (r |->? x) ** pure (if Some? w then x == Some v else x == w)|END"
