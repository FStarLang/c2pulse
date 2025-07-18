module Swap_struct_test

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type u32_pair_struct = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type u32_pair_struct_spec = {
first : UInt32.t;
second : UInt32.t

}

let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
exists* (y: u32_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val u32_pair_struct_spec_default : u32_pair_struct_spec

assume val u32_pair_struct_default (u32_pair_struct_var_spec:u32_pair_struct_spec) : u32_pair_struct

ghost
fn u32_pair_struct_pack (u32_pair_struct_var:ref u32_pair_struct) (#u32_pair_struct_var_spec:u32_pair_struct_spec)
requires u32_pair_struct_var|-> u32_pair_struct_default u32_pair_struct_var_spec
ensures exists* v. u32_pair_struct_pred u32_pair_struct_var v ** pure (v == u32_pair_struct_var_spec)
{ admit() }

ghost
fn u32_pair_struct_unpack (u32_pair_struct_var:ref u32_pair_struct)
requires exists* v. u32_pair_struct_pred u32_pair_struct_var v 
ensures exists* u. u32_pair_struct_var |-> u
{ admit() }

fn u32_pair_struct_alloc ()
returns x:ref u32_pair_struct
ensures freeable x
ensures exists* v. u32_pair_struct_pred x v
{ admit () }

fn u32_pair_struct_free (x:ref u32_pair_struct)
requires freeable x
requires exists* v. u32_pair_struct_pred x v
{ admit() }


ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (v: u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** (v.second |-> s.second)
{unfold u32_pair_struct_pred}


ghost
fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{fold u32_pair_struct_pred x ({first = a0; second = a1}) }

fn new_u32_pair_struct ()
requires emp
returns x:ref u32_pair_struct
ensures freeable x
ensures (u32_pair_struct_pred x { first = 0ul; second = 1ul })
{
let x0 : (ref u32_pair_struct) = u32_pair_struct_alloc ();
let mut x : (ref u32_pair_struct) = x0;
u32_pair_struct_explode !x;
Mku32_pair_struct?.first (! (! x)) := (uint64_to_uint32 0UL);
Mku32_pair_struct?.second (! (! x)) := (uint64_to_uint32 1UL);
u32_pair_struct_recover !x;
(! x);
}

fn swap_fields
(x : ref u32_pair_struct)
(#s : u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
{
let mut x : (ref u32_pair_struct) = x;
u32_pair_struct_explode !x;
let f11 : UInt32.t = (! (! (! x)).first);
let mut f1 : UInt32.t = f11;
Mku32_pair_struct?.first (! (! x)) := (! (! (! x)).second);
Mku32_pair_struct?.second (! (! x)) := (! f1);
u32_pair_struct_recover !x;
}

fn swap_refs
(x : ref UInt32.t)
(y : ref UInt32.t)
requires x |-> 'x
requires y |-> 'y
ensures x |-> 'y
ensures y |-> 'x
{
let mut x : (ref UInt32.t) = x;
let mut y : (ref UInt32.t) = y;
let tmp2 : UInt32.t = (! (! x));
let mut tmp : UInt32.t = tmp2;
(! x) := (! (! y));
(! y) := (! tmp);
}

fn swap_fields_alt
(x : ref u32_pair_struct)
(#s : u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})
{
let mut x : (ref u32_pair_struct) = x;
u32_pair_struct_explode !x;
(swap_refs (! (! x)).first (! (! x)).second);
u32_pair_struct_recover !x;
}

fn main ()
{
let x3 : (ref u32_pair_struct) = (new_u32_pair_struct ());
let mut x : (ref u32_pair_struct) = x3;
(swap_fields (! x));
(swap_fields_alt (! x));
with vx. assert x |-> vx;
assert(u32_pair_struct_pred vx {first = 0ul; second = 1ul});
(u32_pair_struct_free (! x));
}

//Dumping the Clang AST.
// RecordDecl 0x5a9abd7bf4a8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:5:9, line:8:1> line:5:16 struct _u32_pair_struct definition
// |-FieldDecl 0x5a9abd7bf5c0 <line:6:3, col:12> col:12 referenced first 'uint32_t':'unsigned int'
// `-FieldDecl 0x5a9abd7bf620 <line:7:3, col:12> col:12 referenced second 'uint32_t':'unsigned int'
// TypedefDecl 0x5a9abd7bf6c8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:5:1, line:8:3> col:3 referenced u32_pair_struct 'struct _u32_pair_struct'
// `-ElaboratedType 0x5a9abd7bf670 'struct _u32_pair_struct' sugar
//   `-RecordType 0x5a9abd7bf530 'struct _u32_pair_struct'
//     `-Record 0x5a9abd7bf4a8 '_u32_pair_struct'
// FunctionDecl 0x5a9abd7bf998 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:15:1, line:23:1> line:15:18 used new_u32_pair_struct 'u32_pair_struct *()'
// |-CompoundStmt 0x5a9abd7c2220 <line:16:1, line:23:1>
// | |-DeclStmt 0x5a9abd7bfdf8 <line:17:3, col:75>
// | | `-VarDecl 0x5a9abd7bfc88 <col:3, col:74> col:20 used x 'u32_pair_struct *' cinit
// | |   `-CStyleCastExpr 0x5a9abd7bfdd0 <col:24, col:74> 'u32_pair_struct *' <BitCast>
// | |     `-CallExpr 0x5a9abd7bfd88 <col:44, col:74> 'void *'
// | |       |-ImplicitCastExpr 0x5a9abd7bfd70 <col:44> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x5a9abd7bfcf0 <col:44> 'void *(unsigned long)' Function 0x5a9abd685f50 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x5a9abd7bfd20 <col:51, col:73> 'unsigned long' sizeof 'u32_pair_struct':'struct _u32_pair_struct'
// | |-AttributedStmt 0x5a9abd7c1f50 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:18:36>
// | | |-AnnotateAttr 0x5a9abd7c1ed0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x5a9abd7bfe58 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:18:36>
// | |-BinaryOperator 0x5a9abd7c2008 <line:19:3, col:14> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x5a9abd7c1fa0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x5a9abd7bf5c0
// | | | `-ImplicitCastExpr 0x5a9abd7c1f88 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5a9abd7c1f68 <col:3> 'u32_pair_struct *' lvalue Var 0x5a9abd7bfc88 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x5a9abd7c1ff0 <col:14> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x5a9abd7c1fd0 <col:14> 'unsigned long' 0
// | |-BinaryOperator 0x5a9abd7c20c8 <line:20:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x5a9abd7c2060 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x5a9abd7bf620
// | | | `-ImplicitCastExpr 0x5a9abd7c2048 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5a9abd7c2028 <col:3> 'u32_pair_struct *' lvalue Var 0x5a9abd7bfc88 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x5a9abd7c20b0 <col:15> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x5a9abd7c2090 <col:15> 'unsigned long' 1
// | |-AttributedStmt 0x5a9abd7c21c0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:21:36>
// | | |-AnnotateAttr 0x5a9abd7c2138 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// | | `-NullStmt 0x5a9abd7c2130 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:21:36>
// | `-ReturnStmt 0x5a9abd7c2210 <line:22:3, col:10>
// |   `-ImplicitCastExpr 0x5a9abd7c21f8 <col:10> 'u32_pair_struct *' <LValueToRValue>
// |     `-DeclRefExpr 0x5a9abd7c21d8 <col:10> 'u32_pair_struct *' lvalue Var 0x5a9abd7bfc88 'x' 'u32_pair_struct *'
// |-AnnotateAttr 0x5a9abd7bfa40 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5a9abd7bfae0 <line:5:22, col:58> pulse "returns:x:ref u32_pair_struct|END"
// |-AnnotateAttr 0x5a9abd7bfb60 <line:4:24, col:71> pulse "ensures:freeable x|END"
// `-AnnotateAttr 0x5a9abd7bfbd0 <col:24, col:71> pulse "ensures:(u32_pair_struct_pred x { first = 0ul; second = 1ul })|END"
// FunctionDecl 0x5a9abd7c24c0 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:28:1, line:34:1> line:28:6 used swap_fields 'void (u32_pair_struct *)'
// |-ParmVarDecl 0x5a9abd7c23b0 <col:18, col:35> col:35 used x 'u32_pair_struct *'
// |-CompoundStmt 0x5a9abd7c2c28 <col:38, line:34:1>
// | |-AttributedStmt 0x5a9abd7c2850 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:29:36>
// | | |-AnnotateAttr 0x5a9abd7c27d0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x5a9abd7c27c8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:29:36>
// | |-DeclStmt 0x5a9abd7c2960 <line:30:3, col:25>
// | | `-VarDecl 0x5a9abd7c2878 <col:3, col:20> col:12 used f1 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x5a9abd7c2948 <col:17, col:20> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-MemberExpr 0x5a9abd7c2918 <col:17, col:20> 'uint32_t':'unsigned int' lvalue ->first 0x5a9abd7bf5c0
// | |       `-ImplicitCastExpr 0x5a9abd7c2900 <col:17> 'u32_pair_struct *' <LValueToRValue>
// | |         `-DeclRefExpr 0x5a9abd7c28e0 <col:17> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c23b0 'x' 'u32_pair_struct *'
// | |-BinaryOperator 0x5a9abd7c2a60 <line:31:3, col:17> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x5a9abd7c29b0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x5a9abd7bf5c0
// | | | `-ImplicitCastExpr 0x5a9abd7c2998 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5a9abd7c2978 <col:3> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c23b0 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x5a9abd7c2a48 <col:14, col:17> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-MemberExpr 0x5a9abd7c2a18 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->second 0x5a9abd7bf620
// | |     `-ImplicitCastExpr 0x5a9abd7c2a00 <col:14> 'u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x5a9abd7c29e0 <col:14> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c23b0 'x' 'u32_pair_struct *'
// | |-BinaryOperator 0x5a9abd7c2b20 <line:32:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x5a9abd7c2ab8 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x5a9abd7bf620
// | | | `-ImplicitCastExpr 0x5a9abd7c2aa0 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5a9abd7c2a80 <col:3> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c23b0 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x5a9abd7c2b08 <col:15> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5a9abd7c2ae8 <col:15> 'uint32_t':'unsigned int' lvalue Var 0x5a9abd7c2878 'f1' 'uint32_t':'unsigned int'
// | `-AttributedStmt 0x5a9abd7c2c10 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:33:36>
// |   |-AnnotateAttr 0x5a9abd7c2b90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x5a9abd7c2b88 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:33:36>
// |-AnnotateAttr 0x5a9abd7c2570 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : u32_pair_struct_spec|END"
// |-AnnotateAttr 0x5a9abd7c2620 <line:3:23, col:61> pulse "requires:u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x5a9abd7c26a0 <line:4:24, col:71> pulse "ensures:"exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))"|END"
// FunctionDecl 0x5a9abd7c2f50 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:40:1, line:44:1> line:40:6 used swap_refs 'void (uint32_t *, uint32_t *)'
// |-ParmVarDecl 0x5a9abd7c2d88 <col:16, col:26> col:26 used x 'uint32_t *'
// |-ParmVarDecl 0x5a9abd7c2e08 <col:29, col:39> col:39 used y 'uint32_t *'
// |-CompoundStmt 0x5a9abd7c3478 <col:42, line:44:1>
// | |-DeclStmt 0x5a9abd7c32e0 <line:41:3, col:20>
// | | `-VarDecl 0x5a9abd7c3210 <col:3, col:19> col:12 used tmp 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x5a9abd7c32c8 <col:18, col:19> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-UnaryOperator 0x5a9abd7c32b0 <col:18, col:19> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x5a9abd7c3298 <col:19> 'uint32_t *' <LValueToRValue>
// | |         `-DeclRefExpr 0x5a9abd7c3278 <col:19> 'uint32_t *' lvalue ParmVar 0x5a9abd7c2d88 'x' 'uint32_t *'
// | |-BinaryOperator 0x5a9abd7c33b0 <line:42:3, col:9> 'uint32_t':'unsigned int' '='
// | | |-UnaryOperator 0x5a9abd7c3330 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x5a9abd7c3318 <col:4> 'uint32_t *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5a9abd7c32f8 <col:4> 'uint32_t *' lvalue ParmVar 0x5a9abd7c2d88 'x' 'uint32_t *'
// | | `-ImplicitCastExpr 0x5a9abd7c3398 <col:8, col:9> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-UnaryOperator 0x5a9abd7c3380 <col:8, col:9> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |     `-ImplicitCastExpr 0x5a9abd7c3368 <col:9> 'uint32_t *' <LValueToRValue>
// | |       `-DeclRefExpr 0x5a9abd7c3348 <col:9> 'uint32_t *' lvalue ParmVar 0x5a9abd7c2e08 'y' 'uint32_t *'
// | `-BinaryOperator 0x5a9abd7c3458 <line:43:3, col:8> 'uint32_t':'unsigned int' '='
// |   |-UnaryOperator 0x5a9abd7c3408 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x5a9abd7c33f0 <col:4> 'uint32_t *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5a9abd7c33d0 <col:4> 'uint32_t *' lvalue ParmVar 0x5a9abd7c2e08 'y' 'uint32_t *'
// |   `-ImplicitCastExpr 0x5a9abd7c3440 <col:8> 'uint32_t':'unsigned int' <LValueToRValue>
// |     `-DeclRefExpr 0x5a9abd7c3420 <col:8> 'uint32_t':'unsigned int' lvalue Var 0x5a9abd7c3210 'tmp' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x5a9abd7c3008 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'x"|END"
// |-AnnotateAttr 0x5a9abd7c30b0 <col:23, col:61> pulse "requires:"y |-> 'y"|END"
// |-AnnotateAttr 0x5a9abd7c3120 <line:4:24, col:71> pulse "ensures:"x |-> 'y"|END"
// `-AnnotateAttr 0x5a9abd7c3190 <col:24, col:71> pulse "ensures:"y |-> 'x"|END"
// FunctionDecl 0x5a9abd7c3678 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:49:1, line:53:1> line:49:6 used swap_fields_alt 'void (u32_pair_struct *)'
// |-ParmVarDecl 0x5a9abd7c35e8 <col:22, col:39> col:39 used x 'u32_pair_struct *'
// |-CompoundStmt 0x5a9abd7c3ce8 <col:42, line:53:1>
// | |-AttributedStmt 0x5a9abd7c3a10 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:50:36>
// | | |-AnnotateAttr 0x5a9abd7c3990 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x5a9abd7c3988 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:50:36>
// | |-CallExpr 0x5a9abd7c3bc8 <line:51:3, col:34> 'void'
// | | |-ImplicitCastExpr 0x5a9abd7c3bb0 <col:3> 'void (*)(uint32_t *, uint32_t *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x5a9abd7c3a28 <col:3> 'void (uint32_t *, uint32_t *)' Function 0x5a9abd7c2f50 'swap_refs' 'void (uint32_t *, uint32_t *)'
// | | |-UnaryOperator 0x5a9abd7c3ab0 <col:13, col:17> 'uint32_t *' prefix '&' cannot overflow
// | | | `-MemberExpr 0x5a9abd7c3a80 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->first 0x5a9abd7bf5c0
// | | |   `-ImplicitCastExpr 0x5a9abd7c3a68 <col:14> 'u32_pair_struct *' <LValueToRValue>
// | | |     `-DeclRefExpr 0x5a9abd7c3a48 <col:14> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c35e8 'x' 'u32_pair_struct *'
// | | `-UnaryOperator 0x5a9abd7c3b30 <col:24, col:28> 'uint32_t *' prefix '&' cannot overflow
// | |   `-MemberExpr 0x5a9abd7c3b00 <col:25, col:28> 'uint32_t':'unsigned int' lvalue ->second 0x5a9abd7bf620
// | |     `-ImplicitCastExpr 0x5a9abd7c3ae8 <col:25> 'u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x5a9abd7c3ac8 <col:25> 'u32_pair_struct *' lvalue ParmVar 0x5a9abd7c35e8 'x' 'u32_pair_struct *'
// | `-AttributedStmt 0x5a9abd7c3cd0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:52:36>
// |   |-AnnotateAttr 0x5a9abd7c3c50 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x5a9abd7c3c48 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:52:36>
// |-AnnotateAttr 0x5a9abd7c3728 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : u32_pair_struct_spec|END"
// |-AnnotateAttr 0x5a9abd7c37e0 <line:3:23, col:61> pulse "requires:u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x5a9abd7c3860 <line:4:24, col:71> pulse "ensures:"exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})"|END"
// FunctionDecl 0x5a9abd7c3d68 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:55:1, line:62:1> line:55:5 main 'int ()'
// `-CompoundStmt 0x5a9abd7c4488 <col:12, line:62:1>
//   |-DeclStmt 0x5a9abd7c3f60 <line:56:3, col:45>
//   | `-VarDecl 0x5a9abd7c3e28 <col:3, col:44> col:20 used x 'u32_pair_struct *' cinit
//   |   `-CallExpr 0x5a9abd7c3f38 <col:24, col:44> 'u32_pair_struct *'
//   |     `-ImplicitCastExpr 0x5a9abd7c3f20 <col:24> 'u32_pair_struct *(*)()' <FunctionToPointerDecay>
//   |       `-DeclRefExpr 0x5a9abd7c3e90 <col:24> 'u32_pair_struct *()' Function 0x5a9abd7bf998 'new_u32_pair_struct' 'u32_pair_struct *()'
//   |-CallExpr 0x5a9abd7c4038 <line:57:3, col:16> 'void'
//   | |-ImplicitCastExpr 0x5a9abd7c4020 <col:3> 'void (*)(u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x5a9abd7c3f78 <col:3> 'void (u32_pair_struct *)' Function 0x5a9abd7c24c0 'swap_fields' 'void (u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x5a9abd7c4068 <col:15> 'u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x5a9abd7c3f98 <col:15> 'u32_pair_struct *' lvalue Var 0x5a9abd7c3e28 'x' 'u32_pair_struct *'
//   |-CallExpr 0x5a9abd7c4138 <line:58:3, col:20> 'void'
//   | |-ImplicitCastExpr 0x5a9abd7c4120 <col:3> 'void (*)(u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x5a9abd7c40e0 <col:3> 'void (u32_pair_struct *)' Function 0x5a9abd7c3678 'swap_fields_alt' 'void (u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x5a9abd7c4168 <col:19> 'u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x5a9abd7c4100 <col:19> 'u32_pair_struct *' lvalue Var 0x5a9abd7c3e28 'x' 'u32_pair_struct *'
//   |-AttributedStmt 0x5a9abd7c4250 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:59:34>
//   | |-AnnotateAttr 0x5a9abd7c41d0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with vx. assert x |-> vx|END"
//   | `-NullStmt 0x5a9abd7c41c8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:59:34>
//   |-AttributedStmt 0x5a9abd7c4370 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:10:25, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:60:62>
//   | |-AnnotateAttr 0x5a9abd7c42d0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:10:27, col:69> pulse "assert:u32_pair_struct_pred vx {first = 0ul; second = 1ul}|END"
//   | `-NullStmt 0x5a9abd7c42c8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test.c:60:62>
//   `-CallExpr 0x5a9abd7c4428 <line:61:3, col:9> 'void'
//     |-ImplicitCastExpr 0x5a9abd7c4410 <col:3> 'void (*)(void *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x5a9abd7c4388 <col:3> 'void (void *)' Function 0x5a9abd686ec8 'free' 'void (void *)'
//     `-ImplicitCastExpr 0x5a9abd7c4470 <col:8> 'void *' <BitCast>
//       `-ImplicitCastExpr 0x5a9abd7c4458 <col:8> 'u32_pair_struct *' <LValueToRValue>
//         `-DeclRefExpr 0x5a9abd7c43a8 <col:8> 'u32_pair_struct *' lvalue Var 0x5a9abd7c3e28 'x' 'u32_pair_struct *'
