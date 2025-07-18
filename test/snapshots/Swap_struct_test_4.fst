module Swap_struct_test_4

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
with vx. assert ((x |-> vx) ** u32_pair_struct_pred vx {first = 0ul; second = 1ul});
(u32_pair_struct_free (! x));
}

//Dumping the Clang AST.
// RecordDecl 0x6054c38d1dc8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:5:9, line:8:1> line:5:9 struct definition
// |-FieldDecl 0x6054c38d1ee0 <line:6:3, col:12> col:12 referenced first 'uint32_t':'unsigned int'
// `-FieldDecl 0x6054c38d1f40 <line:7:3, col:12> col:12 referenced second 'uint32_t':'unsigned int'
// TypedefDecl 0x6054c38d1fe8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:5:1, line:8:3> col:3 referenced u32_pair_struct 'struct u32_pair_struct':'u32_pair_struct'
// `-ElaboratedType 0x6054c38d1f90 'struct u32_pair_struct' sugar
//   `-RecordType 0x6054c38d1e50 'u32_pair_struct'
//     `-Record 0x6054c38d1dc8 <line:5:9, line:8:1>
// FunctionDecl 0x6054c38d22b8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:15:1, line:23:1> line:15:18 used new_u32_pair_struct 'u32_pair_struct *()'
// |-CompoundStmt 0x6054c38d4b40 <line:16:1, line:23:1>
// | |-DeclStmt 0x6054c38d2718 <line:17:3, col:75>
// | | `-VarDecl 0x6054c38d25a8 <col:3, col:74> col:20 used x 'u32_pair_struct *' cinit
// | |   `-CStyleCastExpr 0x6054c38d26f0 <col:24, col:74> 'u32_pair_struct *' <BitCast>
// | |     `-CallExpr 0x6054c38d26a8 <col:44, col:74> 'void *'
// | |       |-ImplicitCastExpr 0x6054c38d2690 <col:44> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x6054c38d2610 <col:44> 'void *(unsigned long)' Function 0x6054c3798f40 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x6054c38d2640 <col:51, col:73> 'unsigned long' sizeof 'u32_pair_struct'
// | |-AttributedStmt 0x6054c38d4870 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:18:36>
// | | |-AnnotateAttr 0x6054c38d47f0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x6054c38d2778 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:18:36>
// | |-BinaryOperator 0x6054c38d4928 <line:19:3, col:14> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x6054c38d48c0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x6054c38d1ee0
// | | | `-ImplicitCastExpr 0x6054c38d48a8 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x6054c38d4888 <col:3> 'u32_pair_struct *' lvalue Var 0x6054c38d25a8 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x6054c38d4910 <col:14> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x6054c38d48f0 <col:14> 'unsigned long' 0
// | |-BinaryOperator 0x6054c38d49e8 <line:20:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x6054c38d4980 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x6054c38d1f40
// | | | `-ImplicitCastExpr 0x6054c38d4968 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x6054c38d4948 <col:3> 'u32_pair_struct *' lvalue Var 0x6054c38d25a8 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x6054c38d49d0 <col:15> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x6054c38d49b0 <col:15> 'unsigned long' 1
// | |-AttributedStmt 0x6054c38d4ae0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:21:36>
// | | |-AnnotateAttr 0x6054c38d4a58 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// | | `-NullStmt 0x6054c38d4a50 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:21:36>
// | `-ReturnStmt 0x6054c38d4b30 <line:22:3, col:10>
// |   `-ImplicitCastExpr 0x6054c38d4b18 <col:10> 'u32_pair_struct *' <LValueToRValue>
// |     `-DeclRefExpr 0x6054c38d4af8 <col:10> 'u32_pair_struct *' lvalue Var 0x6054c38d25a8 'x' 'u32_pair_struct *'
// |-AnnotateAttr 0x6054c38d2360 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x6054c38d2400 <line:5:22, col:58> pulse "returns:x:ref u32_pair_struct|END"
// |-AnnotateAttr 0x6054c38d2480 <line:4:24, col:71> pulse "ensures:freeable x|END"
// `-AnnotateAttr 0x6054c38d24f0 <col:24, col:71> pulse "ensures:(u32_pair_struct_pred x { first = 0ul; second = 1ul })|END"
// FunctionDecl 0x6054c38d4de0 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:28:1, line:34:1> line:28:6 used swap_fields 'void (u32_pair_struct *)'
// |-ParmVarDecl 0x6054c38d4cd0 <col:18, col:35> col:35 used x 'u32_pair_struct *'
// |-CompoundStmt 0x6054c38d5548 <col:38, line:34:1>
// | |-AttributedStmt 0x6054c38d5170 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:29:36>
// | | |-AnnotateAttr 0x6054c38d50f0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x6054c38d50e8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:29:36>
// | |-DeclStmt 0x6054c38d5280 <line:30:3, col:25>
// | | `-VarDecl 0x6054c38d5198 <col:3, col:20> col:12 used f1 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x6054c38d5268 <col:17, col:20> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-MemberExpr 0x6054c38d5238 <col:17, col:20> 'uint32_t':'unsigned int' lvalue ->first 0x6054c38d1ee0
// | |       `-ImplicitCastExpr 0x6054c38d5220 <col:17> 'u32_pair_struct *' <LValueToRValue>
// | |         `-DeclRefExpr 0x6054c38d5200 <col:17> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d4cd0 'x' 'u32_pair_struct *'
// | |-BinaryOperator 0x6054c38d5380 <line:31:3, col:17> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x6054c38d52d0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x6054c38d1ee0
// | | | `-ImplicitCastExpr 0x6054c38d52b8 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x6054c38d5298 <col:3> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d4cd0 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x6054c38d5368 <col:14, col:17> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-MemberExpr 0x6054c38d5338 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->second 0x6054c38d1f40
// | |     `-ImplicitCastExpr 0x6054c38d5320 <col:14> 'u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x6054c38d5300 <col:14> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d4cd0 'x' 'u32_pair_struct *'
// | |-BinaryOperator 0x6054c38d5440 <line:32:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x6054c38d53d8 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x6054c38d1f40
// | | | `-ImplicitCastExpr 0x6054c38d53c0 <col:3> 'u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x6054c38d53a0 <col:3> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d4cd0 'x' 'u32_pair_struct *'
// | | `-ImplicitCastExpr 0x6054c38d5428 <col:15> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-DeclRefExpr 0x6054c38d5408 <col:15> 'uint32_t':'unsigned int' lvalue Var 0x6054c38d5198 'f1' 'uint32_t':'unsigned int'
// | `-AttributedStmt 0x6054c38d5530 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:33:36>
// |   |-AnnotateAttr 0x6054c38d54b0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x6054c38d54a8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:33:36>
// |-AnnotateAttr 0x6054c38d4e90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : u32_pair_struct_spec|END"
// |-AnnotateAttr 0x6054c38d4f40 <line:3:23, col:61> pulse "requires:u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x6054c38d4fc0 <line:4:24, col:71> pulse "ensures:"exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))"|END"
// FunctionDecl 0x6054c38d5870 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:40:1, line:44:1> line:40:6 used swap_refs 'void (uint32_t *, uint32_t *)'
// |-ParmVarDecl 0x6054c38d56a8 <col:16, col:26> col:26 used x 'uint32_t *'
// |-ParmVarDecl 0x6054c38d5728 <col:29, col:39> col:39 used y 'uint32_t *'
// |-CompoundStmt 0x6054c38d5d98 <col:42, line:44:1>
// | |-DeclStmt 0x6054c38d5c00 <line:41:3, col:20>
// | | `-VarDecl 0x6054c38d5b30 <col:3, col:19> col:12 used tmp 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x6054c38d5be8 <col:18, col:19> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-UnaryOperator 0x6054c38d5bd0 <col:18, col:19> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x6054c38d5bb8 <col:19> 'uint32_t *' <LValueToRValue>
// | |         `-DeclRefExpr 0x6054c38d5b98 <col:19> 'uint32_t *' lvalue ParmVar 0x6054c38d56a8 'x' 'uint32_t *'
// | |-BinaryOperator 0x6054c38d5cd0 <line:42:3, col:9> 'uint32_t':'unsigned int' '='
// | | |-UnaryOperator 0x6054c38d5c50 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x6054c38d5c38 <col:4> 'uint32_t *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x6054c38d5c18 <col:4> 'uint32_t *' lvalue ParmVar 0x6054c38d56a8 'x' 'uint32_t *'
// | | `-ImplicitCastExpr 0x6054c38d5cb8 <col:8, col:9> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-UnaryOperator 0x6054c38d5ca0 <col:8, col:9> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |     `-ImplicitCastExpr 0x6054c38d5c88 <col:9> 'uint32_t *' <LValueToRValue>
// | |       `-DeclRefExpr 0x6054c38d5c68 <col:9> 'uint32_t *' lvalue ParmVar 0x6054c38d5728 'y' 'uint32_t *'
// | `-BinaryOperator 0x6054c38d5d78 <line:43:3, col:8> 'uint32_t':'unsigned int' '='
// |   |-UnaryOperator 0x6054c38d5d28 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x6054c38d5d10 <col:4> 'uint32_t *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x6054c38d5cf0 <col:4> 'uint32_t *' lvalue ParmVar 0x6054c38d5728 'y' 'uint32_t *'
// |   `-ImplicitCastExpr 0x6054c38d5d60 <col:8> 'uint32_t':'unsigned int' <LValueToRValue>
// |     `-DeclRefExpr 0x6054c38d5d40 <col:8> 'uint32_t':'unsigned int' lvalue Var 0x6054c38d5b30 'tmp' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x6054c38d5928 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'x"|END"
// |-AnnotateAttr 0x6054c38d59d0 <col:23, col:61> pulse "requires:"y |-> 'y"|END"
// |-AnnotateAttr 0x6054c38d5a40 <line:4:24, col:71> pulse "ensures:"x |-> 'y"|END"
// `-AnnotateAttr 0x6054c38d5ab0 <col:24, col:71> pulse "ensures:"y |-> 'x"|END"
// FunctionDecl 0x6054c38d5f98 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:49:1, line:53:1> line:49:6 used swap_fields_alt 'void (u32_pair_struct *)'
// |-ParmVarDecl 0x6054c38d5f08 <col:22, col:39> col:39 used x 'u32_pair_struct *'
// |-CompoundStmt 0x6054c38d6608 <col:42, line:53:1>
// | |-AttributedStmt 0x6054c38d6330 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:50:36>
// | | |-AnnotateAttr 0x6054c38d62b0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x6054c38d62a8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:50:36>
// | |-CallExpr 0x6054c38d64e8 <line:51:3, col:34> 'void'
// | | |-ImplicitCastExpr 0x6054c38d64d0 <col:3> 'void (*)(uint32_t *, uint32_t *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x6054c38d6348 <col:3> 'void (uint32_t *, uint32_t *)' Function 0x6054c38d5870 'swap_refs' 'void (uint32_t *, uint32_t *)'
// | | |-UnaryOperator 0x6054c38d63d0 <col:13, col:17> 'uint32_t *' prefix '&' cannot overflow
// | | | `-MemberExpr 0x6054c38d63a0 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->first 0x6054c38d1ee0
// | | |   `-ImplicitCastExpr 0x6054c38d6388 <col:14> 'u32_pair_struct *' <LValueToRValue>
// | | |     `-DeclRefExpr 0x6054c38d6368 <col:14> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d5f08 'x' 'u32_pair_struct *'
// | | `-UnaryOperator 0x6054c38d6450 <col:24, col:28> 'uint32_t *' prefix '&' cannot overflow
// | |   `-MemberExpr 0x6054c38d6420 <col:25, col:28> 'uint32_t':'unsigned int' lvalue ->second 0x6054c38d1f40
// | |     `-ImplicitCastExpr 0x6054c38d6408 <col:25> 'u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x6054c38d63e8 <col:25> 'u32_pair_struct *' lvalue ParmVar 0x6054c38d5f08 'x' 'u32_pair_struct *'
// | `-AttributedStmt 0x6054c38d65f0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:52:36>
// |   |-AnnotateAttr 0x6054c38d6570 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x6054c38d6568 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:52:36>
// |-AnnotateAttr 0x6054c38d6048 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : u32_pair_struct_spec|END"
// |-AnnotateAttr 0x6054c38d6100 <line:3:23, col:61> pulse "requires:u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x6054c38d6180 <line:4:24, col:71> pulse "ensures:"exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})"|END"
// FunctionDecl 0x6054c38d6688 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:55:1, line:61:1> line:55:5 main 'int ()'
// `-CompoundStmt 0x6054c38d6cf8 <col:12, line:61:1>
//   |-DeclStmt 0x6054c38d6880 <line:56:3, col:45>
//   | `-VarDecl 0x6054c38d6748 <col:3, col:44> col:20 used x 'u32_pair_struct *' cinit
//   |   `-CallExpr 0x6054c38d6858 <col:24, col:44> 'u32_pair_struct *'
//   |     `-ImplicitCastExpr 0x6054c38d6840 <col:24> 'u32_pair_struct *(*)()' <FunctionToPointerDecay>
//   |       `-DeclRefExpr 0x6054c38d67b0 <col:24> 'u32_pair_struct *()' Function 0x6054c38d22b8 'new_u32_pair_struct' 'u32_pair_struct *()'
//   |-CallExpr 0x6054c38d6958 <line:57:3, col:16> 'void'
//   | |-ImplicitCastExpr 0x6054c38d6940 <col:3> 'void (*)(u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x6054c38d6898 <col:3> 'void (u32_pair_struct *)' Function 0x6054c38d4de0 'swap_fields' 'void (u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x6054c38d6988 <col:15> 'u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x6054c38d68b8 <col:15> 'u32_pair_struct *' lvalue Var 0x6054c38d6748 'x' 'u32_pair_struct *'
//   |-CallExpr 0x6054c38d6a58 <line:58:3, col:20> 'void'
//   | |-ImplicitCastExpr 0x6054c38d6a40 <col:3> 'void (*)(u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x6054c38d6a00 <col:3> 'void (u32_pair_struct *)' Function 0x6054c38d5f98 'swap_fields_alt' 'void (u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x6054c38d6a88 <col:19> 'u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x6054c38d6a20 <col:19> 'u32_pair_struct *' lvalue Var 0x6054c38d6748 'x' 'u32_pair_struct *'
//   |-AttributedStmt 0x6054c38d6be0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:59:93>
//   | |-AnnotateAttr 0x6054c38d6b28 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with vx. assert ((x |-> vx) ** u32_pair_struct_pred vx {first = 0ul; second = 1ul})|END"
//   | `-NullStmt 0x6054c38d6b20 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_4.c:59:93>
//   `-CallExpr 0x6054c38d6c98 <line:60:3, col:9> 'void'
//     |-ImplicitCastExpr 0x6054c38d6c80 <col:3> 'void (*)(void *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x6054c38d6bf8 <col:3> 'void (void *)' Function 0x6054c3799eb8 'free' 'void (void *)'
//     `-ImplicitCastExpr 0x6054c38d6ce0 <col:8> 'void *' <BitCast>
//       `-ImplicitCastExpr 0x6054c38d6cc8 <col:8> 'u32_pair_struct *' <LValueToRValue>
//         `-DeclRefExpr 0x6054c38d6c18 <col:8> 'u32_pair_struct *' lvalue Var 0x6054c38d6748 'x' 'u32_pair_struct *'
