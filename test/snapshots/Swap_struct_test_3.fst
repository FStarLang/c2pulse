module Swap_struct_test_3

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type _u32_pair_struct = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type _u32_pair_struct_spec = {
first : UInt32.t;
second : UInt32.t

}

let _u32_pair_struct_pred (x:ref _u32_pair_struct) (s:_u32_pair_struct_spec) : slprop =
exists* (y: _u32_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val _u32_pair_struct_spec_default : _u32_pair_struct_spec

assume val _u32_pair_struct_default (_u32_pair_struct_var_spec:_u32_pair_struct_spec) : _u32_pair_struct

ghost
fn _u32_pair_struct_pack (_u32_pair_struct_var:ref _u32_pair_struct) (#_u32_pair_struct_var_spec:_u32_pair_struct_spec)
requires _u32_pair_struct_var|-> _u32_pair_struct_default _u32_pair_struct_var_spec
ensures exists* v. _u32_pair_struct_pred _u32_pair_struct_var v ** pure (v == _u32_pair_struct_var_spec)
{ admit() }

ghost
fn _u32_pair_struct_unpack (_u32_pair_struct_var:ref _u32_pair_struct)
requires exists* v. _u32_pair_struct_pred _u32_pair_struct_var v 
ensures exists* u. _u32_pair_struct_var |-> u
{ admit() }

fn _u32_pair_struct_alloc ()
returns x:ref _u32_pair_struct
ensures freeable x
ensures exists* v. _u32_pair_struct_pred x v
{ admit () }

fn _u32_pair_struct_free (x:ref _u32_pair_struct)
requires freeable x
requires exists* v. _u32_pair_struct_pred x v
{ admit() }


ghost fn _u32_pair_struct_explode (x:ref _u32_pair_struct) (#s:_u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (v: _u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** (v.second |-> s.second)
{unfold _u32_pair_struct_pred}


ghost
fn _u32_pair_struct_recover (x:ref _u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
requires exists* (y: _u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. _u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{fold _u32_pair_struct_pred x ({first = a0; second = a1}) }

fn new_u32_pair_struct ()
requires emp
returns x:ref _u32_pair_struct
ensures freeable x
ensures (_u32_pair_struct_pred x { first = 0ul; second = 1ul })
{
let x0 : (ref _u32_pair_struct) = _u32_pair_struct_alloc ();
let mut x : (ref _u32_pair_struct) = x0;
_u32_pair_struct_explode !x;
Mk_u32_pair_struct?.first (! (! x)) := (uint64_to_uint32 0UL);
Mk_u32_pair_struct?.second (! (! x)) := (uint64_to_uint32 1UL);
_u32_pair_struct_recover !x;
(! x);
}

fn swap_fields
(x : ref _u32_pair_struct)
(#s : _u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
{
let mut x : (ref _u32_pair_struct) = x;
_u32_pair_struct_explode !x;
let f11 : UInt32.t = (! (! (! x)).first);
let mut f1 : UInt32.t = f11;
Mk_u32_pair_struct?.first (! (! x)) := (! (! (! x)).second);
Mk_u32_pair_struct?.second (! (! x)) := (! f1);
_u32_pair_struct_recover !x;
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
(x : ref _u32_pair_struct)
(#s : _u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})
{
let mut x : (ref _u32_pair_struct) = x;
_u32_pair_struct_explode !x;
(swap_refs (! (! x)).first (! (! x)).second);
_u32_pair_struct_recover !x;
}

fn main ()
{
let x3 : (ref _u32_pair_struct) = (new_u32_pair_struct ());
let mut x : (ref _u32_pair_struct) = x3;
(swap_fields (! x));
(swap_fields_alt (! x));
with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul});
(_u32_pair_struct_free (! x));
}

//Dumping the Clang AST.
// RecordDecl 0x641809a607d8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:5:1, line:8:1> line:5:8 struct _u32_pair_struct definition
// |-FieldDecl 0x641809a608f0 <line:6:3, col:12> col:12 referenced first 'uint32_t':'unsigned int'
// `-FieldDecl 0x641809a60950 <line:7:3, col:12> col:12 referenced second 'uint32_t':'unsigned int'
// FunctionDecl 0x641809a60be8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:15:1, line:23:1> line:15:26 used new_u32_pair_struct 'struct _u32_pair_struct *()'
// |-CompoundStmt 0x641809a63480 <line:16:1, line:23:1>
// | |-DeclStmt 0x641809a61078 <line:17:3, col:99>
// | | `-VarDecl 0x641809a60ee8 <col:3, col:98> col:28 used x 'struct _u32_pair_struct *' cinit
// | |   `-CStyleCastExpr 0x641809a61050 <col:32, col:98> 'struct _u32_pair_struct *' <BitCast>
// | |     `-CallExpr 0x641809a60ff8 <col:60, col:98> 'void *'
// | |       |-ImplicitCastExpr 0x641809a60fe0 <col:60> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x641809a60f50 <col:60> 'void *(unsigned long)' Function 0x641809926f80 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x641809a60f90 <col:67, col:97> 'unsigned long' sizeof 'struct _u32_pair_struct'
// | |-AttributedStmt 0x641809a61160 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:18:37>
// | | |-AnnotateAttr 0x641809a610e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x641809a610d8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:18:37>
// | |-BinaryOperator 0x641809a63268 <line:19:3, col:14> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x641809a63200 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x641809a608f0
// | | | `-ImplicitCastExpr 0x641809a61198 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x641809a61178 <col:3> 'struct _u32_pair_struct *' lvalue Var 0x641809a60ee8 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x641809a63250 <col:14> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x641809a63230 <col:14> 'unsigned long' 0
// | |-BinaryOperator 0x641809a63328 <line:20:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x641809a632c0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x641809a60950
// | | | `-ImplicitCastExpr 0x641809a632a8 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x641809a63288 <col:3> 'struct _u32_pair_struct *' lvalue Var 0x641809a60ee8 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x641809a63310 <col:15> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x641809a632f0 <col:15> 'unsigned long' 1
// | |-AttributedStmt 0x641809a63420 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:21:37>
// | | |-AnnotateAttr 0x641809a63398 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// | | `-NullStmt 0x641809a63390 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:21:37>
// | `-ReturnStmt 0x641809a63470 <line:22:3, col:10>
// |   `-ImplicitCastExpr 0x641809a63458 <col:10> 'struct _u32_pair_struct *' <LValueToRValue>
// |     `-DeclRefExpr 0x641809a63438 <col:10> 'struct _u32_pair_struct *' lvalue Var 0x641809a60ee8 'x' 'struct _u32_pair_struct *'
// |-AnnotateAttr 0x641809a60c90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x641809a60d30 <line:5:22, col:58> pulse "returns:x:ref _u32_pair_struct|END"
// |-AnnotateAttr 0x641809a60db0 <line:4:24, col:71> pulse "ensures:freeable x|END"
// `-AnnotateAttr 0x641809a60e20 <col:24, col:71> pulse "ensures:(_u32_pair_struct_pred x { first = 0ul; second = 1ul })|END"
// FunctionDecl 0x641809a63730 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:28:1, line:34:1> line:28:6 used swap_fields 'void (struct _u32_pair_struct *)'
// |-ParmVarDecl 0x641809a63628 <col:18, col:43> col:43 used x 'struct _u32_pair_struct *'
// |-CompoundStmt 0x641809a63ea8 <col:46, line:34:1>
// | |-AttributedStmt 0x641809a63ad0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:29:37>
// | | |-AnnotateAttr 0x641809a63a50 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x641809a63a48 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:29:37>
// | |-DeclStmt 0x641809a63be0 <line:30:3, col:25>
// | | `-VarDecl 0x641809a63af8 <col:3, col:20> col:12 used f1 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x641809a63bc8 <col:17, col:20> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-MemberExpr 0x641809a63b98 <col:17, col:20> 'uint32_t':'unsigned int' lvalue ->first 0x641809a608f0
// | |       `-ImplicitCastExpr 0x641809a63b80 <col:17> 'struct _u32_pair_struct *' <LValueToRValue>
// | |         `-DeclRefExpr 0x641809a63b60 <col:17> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a63628 'x' 'struct _u32_pair_struct *'
// | |-BinaryOperator 0x641809a63ce0 <line:31:3, col:17> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x641809a63c30 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x641809a608f0
// | | | `-ImplicitCastExpr 0x641809a63c18 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x641809a63bf8 <col:3> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a63628 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x641809a63cc8 <col:14, col:17> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-MemberExpr 0x641809a63c98 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->second 0x641809a60950
// | |     `-ImplicitCastExpr 0x641809a63c80 <col:14> 'struct _u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x641809a63c60 <col:14> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a63628 'x' 'struct _u32_pair_struct *'
// | |-BinaryOperator 0x641809a63da0 <line:32:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x641809a63d38 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x641809a60950
// | | | `-ImplicitCastExpr 0x641809a63d20 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x641809a63d00 <col:3> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a63628 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x641809a63d88 <col:15> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-DeclRefExpr 0x641809a63d68 <col:15> 'uint32_t':'unsigned int' lvalue Var 0x641809a63af8 'f1' 'uint32_t':'unsigned int'
// | `-AttributedStmt 0x641809a63e90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:33:37>
// |   |-AnnotateAttr 0x641809a63e10 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x641809a63e08 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:33:37>
// |-AnnotateAttr 0x641809a637e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : _u32_pair_struct_spec|END"
// |-AnnotateAttr 0x641809a638a0 <line:3:23, col:61> pulse "requires:_u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x641809a63920 <line:4:24, col:71> pulse "ensures:"exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))"|END"
// FunctionDecl 0x641809a64210 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:40:1, line:44:1> line:40:6 used swap_refs 'void (uint32_t *, uint32_t *)'
// |-ParmVarDecl 0x641809a64008 <col:16, col:26> col:26 used x 'uint32_t *'
// |-ParmVarDecl 0x641809a64088 <col:29, col:39> col:39 used y 'uint32_t *'
// |-CompoundStmt 0x641809a64738 <col:42, line:44:1>
// | |-DeclStmt 0x641809a645a0 <line:41:3, col:20>
// | | `-VarDecl 0x641809a644d0 <col:3, col:19> col:12 used tmp 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x641809a64588 <col:18, col:19> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-UnaryOperator 0x641809a64570 <col:18, col:19> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x641809a64558 <col:19> 'uint32_t *' <LValueToRValue>
// | |         `-DeclRefExpr 0x641809a64538 <col:19> 'uint32_t *' lvalue ParmVar 0x641809a64008 'x' 'uint32_t *'
// | |-BinaryOperator 0x641809a64670 <line:42:3, col:9> 'uint32_t':'unsigned int' '='
// | | |-UnaryOperator 0x641809a645f0 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x641809a645d8 <col:4> 'uint32_t *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x641809a645b8 <col:4> 'uint32_t *' lvalue ParmVar 0x641809a64008 'x' 'uint32_t *'
// | | `-ImplicitCastExpr 0x641809a64658 <col:8, col:9> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-UnaryOperator 0x641809a64640 <col:8, col:9> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |     `-ImplicitCastExpr 0x641809a64628 <col:9> 'uint32_t *' <LValueToRValue>
// | |       `-DeclRefExpr 0x641809a64608 <col:9> 'uint32_t *' lvalue ParmVar 0x641809a64088 'y' 'uint32_t *'
// | `-BinaryOperator 0x641809a64718 <line:43:3, col:8> 'uint32_t':'unsigned int' '='
// |   |-UnaryOperator 0x641809a646c8 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x641809a646b0 <col:4> 'uint32_t *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x641809a64690 <col:4> 'uint32_t *' lvalue ParmVar 0x641809a64088 'y' 'uint32_t *'
// |   `-ImplicitCastExpr 0x641809a64700 <col:8> 'uint32_t':'unsigned int' <LValueToRValue>
// |     `-DeclRefExpr 0x641809a646e0 <col:8> 'uint32_t':'unsigned int' lvalue Var 0x641809a644d0 'tmp' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x641809a642c8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'x"|END"
// |-AnnotateAttr 0x641809a64370 <col:23, col:61> pulse "requires:"y |-> 'y"|END"
// |-AnnotateAttr 0x641809a643e0 <line:4:24, col:71> pulse "ensures:"x |-> 'y"|END"
// `-AnnotateAttr 0x641809a64450 <col:24, col:71> pulse "ensures:"y |-> 'x"|END"
// FunctionDecl 0x641809a64958 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:49:1, line:53:1> line:49:6 used swap_fields_alt 'void (struct _u32_pair_struct *)'
// |-ParmVarDecl 0x641809a648c8 <col:22, col:47> col:47 used x 'struct _u32_pair_struct *'
// |-CompoundStmt 0x641809a64fc8 <col:50, line:53:1>
// | |-AttributedStmt 0x641809a64cf0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:50:37>
// | | |-AnnotateAttr 0x641809a64c70 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x641809a64c68 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:50:37>
// | |-CallExpr 0x641809a64ea8 <line:51:3, col:34> 'void'
// | | |-ImplicitCastExpr 0x641809a64e90 <col:3> 'void (*)(uint32_t *, uint32_t *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x641809a64d08 <col:3> 'void (uint32_t *, uint32_t *)' Function 0x641809a64210 'swap_refs' 'void (uint32_t *, uint32_t *)'
// | | |-UnaryOperator 0x641809a64d90 <col:13, col:17> 'uint32_t *' prefix '&' cannot overflow
// | | | `-MemberExpr 0x641809a64d60 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->first 0x641809a608f0
// | | |   `-ImplicitCastExpr 0x641809a64d48 <col:14> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |     `-DeclRefExpr 0x641809a64d28 <col:14> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a648c8 'x' 'struct _u32_pair_struct *'
// | | `-UnaryOperator 0x641809a64e10 <col:24, col:28> 'uint32_t *' prefix '&' cannot overflow
// | |   `-MemberExpr 0x641809a64de0 <col:25, col:28> 'uint32_t':'unsigned int' lvalue ->second 0x641809a60950
// | |     `-ImplicitCastExpr 0x641809a64dc8 <col:25> 'struct _u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x641809a64da8 <col:25> 'struct _u32_pair_struct *' lvalue ParmVar 0x641809a648c8 'x' 'struct _u32_pair_struct *'
// | `-AttributedStmt 0x641809a64fb0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:52:37>
// |   |-AnnotateAttr 0x641809a64f30 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x641809a64f28 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:52:37>
// |-AnnotateAttr 0x641809a64a08 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : _u32_pair_struct_spec|END"
// |-AnnotateAttr 0x641809a64ac0 <line:3:23, col:61> pulse "requires:_u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x641809a64b40 <line:4:24, col:71> pulse "ensures:"exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})"|END"
// FunctionDecl 0x641809a65048 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:55:1, line:61:1> line:55:5 main 'int ()'
// `-CompoundStmt 0x641809a656d8 <col:12, line:61:1>
//   |-DeclStmt 0x641809a65260 <line:56:3, col:53>
//   | `-VarDecl 0x641809a65118 <col:3, col:52> col:28 used x 'struct _u32_pair_struct *' cinit
//   |   `-CallExpr 0x641809a65238 <col:32, col:52> 'struct _u32_pair_struct *'
//   |     `-ImplicitCastExpr 0x641809a65220 <col:32> 'struct _u32_pair_struct *(*)()' <FunctionToPointerDecay>
//   |       `-DeclRefExpr 0x641809a65180 <col:32> 'struct _u32_pair_struct *()' Function 0x641809a60be8 'new_u32_pair_struct' 'struct _u32_pair_struct *()'
//   |-CallExpr 0x641809a65338 <line:57:3, col:16> 'void'
//   | |-ImplicitCastExpr 0x641809a65320 <col:3> 'void (*)(struct _u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x641809a65278 <col:3> 'void (struct _u32_pair_struct *)' Function 0x641809a63730 'swap_fields' 'void (struct _u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x641809a65368 <col:15> 'struct _u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x641809a65298 <col:15> 'struct _u32_pair_struct *' lvalue Var 0x641809a65118 'x' 'struct _u32_pair_struct *'
//   |-CallExpr 0x641809a65438 <line:58:3, col:20> 'void'
//   | |-ImplicitCastExpr 0x641809a65420 <col:3> 'void (*)(struct _u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x641809a653e0 <col:3> 'void (struct _u32_pair_struct *)' Function 0x641809a64958 'swap_fields_alt' 'void (struct _u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x641809a65468 <col:19> 'struct _u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x641809a65400 <col:19> 'struct _u32_pair_struct *' lvalue Var 0x641809a65118 'x' 'struct _u32_pair_struct *'
//   |-AttributedStmt 0x641809a655c0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:59:94>
//   | |-AnnotateAttr 0x641809a65508 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul})|END"
//   | `-NullStmt 0x641809a65500 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_3.c:59:94>
//   `-CallExpr 0x641809a65678 <line:60:3, col:10> 'void'
//     |-ImplicitCastExpr 0x641809a65660 <col:3> 'void (*)(void *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x641809a655d8 <col:3> 'void (void *)' Function 0x641809927ef8 'free' 'void (void *)'
//     `-ImplicitCastExpr 0x641809a656c0 <col:9> 'void *' <BitCast>
//       `-ImplicitCastExpr 0x641809a656a8 <col:9> 'struct _u32_pair_struct *' <LValueToRValue>
//         `-DeclRefExpr 0x641809a655f8 <col:9> 'struct _u32_pair_struct *' lvalue Var 0x641809a65118 'x' 'struct _u32_pair_struct *'
