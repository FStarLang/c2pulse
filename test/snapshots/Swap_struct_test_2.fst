module Swap_struct_test_2

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
// RecordDecl 0x555d3998f7d8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:5:9, line:8:1> line:5:16 struct _u32_pair_struct definition
// |-FieldDecl 0x555d3998f8f0 <line:6:3, col:12> col:12 referenced first 'uint32_t':'unsigned int'
// `-FieldDecl 0x555d3998f950 <line:7:3, col:12> col:12 referenced second 'uint32_t':'unsigned int'
// FunctionDecl 0x555d3998fbe8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:15:1, line:23:1> line:15:26 used new_u32_pair_struct 'struct _u32_pair_struct *()'
// |-CompoundStmt 0x555d39992480 <line:16:1, line:23:1>
// | |-DeclStmt 0x555d39990078 <line:17:3, col:99>
// | | `-VarDecl 0x555d3998fee8 <col:3, col:98> col:28 used x 'struct _u32_pair_struct *' cinit
// | |   `-CStyleCastExpr 0x555d39990050 <col:32, col:98> 'struct _u32_pair_struct *' <BitCast>
// | |     `-CallExpr 0x555d3998fff8 <col:60, col:98> 'void *'
// | |       |-ImplicitCastExpr 0x555d3998ffe0 <col:60> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x555d3998ff50 <col:60> 'void *(unsigned long)' Function 0x555d39855f80 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x555d3998ff90 <col:67, col:97> 'unsigned long' sizeof 'struct _u32_pair_struct'
// | |-AttributedStmt 0x555d39990160 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:18:37>
// | | |-AnnotateAttr 0x555d399900e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x555d399900d8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:18:37>
// | |-BinaryOperator 0x555d39992268 <line:19:3, col:14> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x555d39992200 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x555d3998f8f0
// | | | `-ImplicitCastExpr 0x555d39990198 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x555d39990178 <col:3> 'struct _u32_pair_struct *' lvalue Var 0x555d3998fee8 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x555d39992250 <col:14> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x555d39992230 <col:14> 'unsigned long' 0
// | |-BinaryOperator 0x555d39992328 <line:20:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x555d399922c0 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x555d3998f950
// | | | `-ImplicitCastExpr 0x555d399922a8 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x555d39992288 <col:3> 'struct _u32_pair_struct *' lvalue Var 0x555d3998fee8 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x555d39992310 <col:15> 'uint32_t':'unsigned int' <IntegralCast>
// | |   `-IntegerLiteral 0x555d399922f0 <col:15> 'unsigned long' 1
// | |-AttributedStmt 0x555d39992420 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:21:37>
// | | |-AnnotateAttr 0x555d39992398 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// | | `-NullStmt 0x555d39992390 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:21:37>
// | `-ReturnStmt 0x555d39992470 <line:22:3, col:10>
// |   `-ImplicitCastExpr 0x555d39992458 <col:10> 'struct _u32_pair_struct *' <LValueToRValue>
// |     `-DeclRefExpr 0x555d39992438 <col:10> 'struct _u32_pair_struct *' lvalue Var 0x555d3998fee8 'x' 'struct _u32_pair_struct *'
// |-AnnotateAttr 0x555d3998fc90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x555d3998fd30 <line:5:22, col:58> pulse "returns:x:ref _u32_pair_struct|END"
// |-AnnotateAttr 0x555d3998fdb0 <line:4:24, col:71> pulse "ensures:freeable x|END"
// `-AnnotateAttr 0x555d3998fe20 <col:24, col:71> pulse "ensures:(_u32_pair_struct_pred x { first = 0ul; second = 1ul })|END"
// FunctionDecl 0x555d39992730 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:28:1, line:34:1> line:28:6 used swap_fields 'void (struct _u32_pair_struct *)'
// |-ParmVarDecl 0x555d39992628 <col:18, col:43> col:43 used x 'struct _u32_pair_struct *'
// |-CompoundStmt 0x555d39992ea8 <col:46, line:34:1>
// | |-AttributedStmt 0x555d39992ad0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:29:37>
// | | |-AnnotateAttr 0x555d39992a50 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x555d39992a48 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:29:37>
// | |-DeclStmt 0x555d39992be0 <line:30:3, col:25>
// | | `-VarDecl 0x555d39992af8 <col:3, col:20> col:12 used f1 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x555d39992bc8 <col:17, col:20> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-MemberExpr 0x555d39992b98 <col:17, col:20> 'uint32_t':'unsigned int' lvalue ->first 0x555d3998f8f0
// | |       `-ImplicitCastExpr 0x555d39992b80 <col:17> 'struct _u32_pair_struct *' <LValueToRValue>
// | |         `-DeclRefExpr 0x555d39992b60 <col:17> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d39992628 'x' 'struct _u32_pair_struct *'
// | |-BinaryOperator 0x555d39992ce0 <line:31:3, col:17> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x555d39992c30 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->first 0x555d3998f8f0
// | | | `-ImplicitCastExpr 0x555d39992c18 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x555d39992bf8 <col:3> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d39992628 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x555d39992cc8 <col:14, col:17> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-MemberExpr 0x555d39992c98 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->second 0x555d3998f950
// | |     `-ImplicitCastExpr 0x555d39992c80 <col:14> 'struct _u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x555d39992c60 <col:14> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d39992628 'x' 'struct _u32_pair_struct *'
// | |-BinaryOperator 0x555d39992da0 <line:32:3, col:15> 'uint32_t':'unsigned int' '='
// | | |-MemberExpr 0x555d39992d38 <col:3, col:6> 'uint32_t':'unsigned int' lvalue ->second 0x555d3998f950
// | | | `-ImplicitCastExpr 0x555d39992d20 <col:3> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x555d39992d00 <col:3> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d39992628 'x' 'struct _u32_pair_struct *'
// | | `-ImplicitCastExpr 0x555d39992d88 <col:15> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-DeclRefExpr 0x555d39992d68 <col:15> 'uint32_t':'unsigned int' lvalue Var 0x555d39992af8 'f1' 'uint32_t':'unsigned int'
// | `-AttributedStmt 0x555d39992e90 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:33:37>
// |   |-AnnotateAttr 0x555d39992e10 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x555d39992e08 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:33:37>
// |-AnnotateAttr 0x555d399927e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : _u32_pair_struct_spec|END"
// |-AnnotateAttr 0x555d399928a0 <line:3:23, col:61> pulse "requires:_u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x555d39992920 <line:4:24, col:71> pulse "ensures:"exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))"|END"
// FunctionDecl 0x555d39993210 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:40:1, line:44:1> line:40:6 used swap_refs 'void (uint32_t *, uint32_t *)'
// |-ParmVarDecl 0x555d39993008 <col:16, col:26> col:26 used x 'uint32_t *'
// |-ParmVarDecl 0x555d39993088 <col:29, col:39> col:39 used y 'uint32_t *'
// |-CompoundStmt 0x555d39993738 <col:42, line:44:1>
// | |-DeclStmt 0x555d399935a0 <line:41:3, col:20>
// | | `-VarDecl 0x555d399934d0 <col:3, col:19> col:12 used tmp 'uint32_t':'unsigned int' cinit
// | |   `-ImplicitCastExpr 0x555d39993588 <col:18, col:19> 'uint32_t':'unsigned int' <LValueToRValue>
// | |     `-UnaryOperator 0x555d39993570 <col:18, col:19> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |       `-ImplicitCastExpr 0x555d39993558 <col:19> 'uint32_t *' <LValueToRValue>
// | |         `-DeclRefExpr 0x555d39993538 <col:19> 'uint32_t *' lvalue ParmVar 0x555d39993008 'x' 'uint32_t *'
// | |-BinaryOperator 0x555d39993670 <line:42:3, col:9> 'uint32_t':'unsigned int' '='
// | | |-UnaryOperator 0x555d399935f0 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | | | `-ImplicitCastExpr 0x555d399935d8 <col:4> 'uint32_t *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x555d399935b8 <col:4> 'uint32_t *' lvalue ParmVar 0x555d39993008 'x' 'uint32_t *'
// | | `-ImplicitCastExpr 0x555d39993658 <col:8, col:9> 'uint32_t':'unsigned int' <LValueToRValue>
// | |   `-UnaryOperator 0x555d39993640 <col:8, col:9> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// | |     `-ImplicitCastExpr 0x555d39993628 <col:9> 'uint32_t *' <LValueToRValue>
// | |       `-DeclRefExpr 0x555d39993608 <col:9> 'uint32_t *' lvalue ParmVar 0x555d39993088 'y' 'uint32_t *'
// | `-BinaryOperator 0x555d39993718 <line:43:3, col:8> 'uint32_t':'unsigned int' '='
// |   |-UnaryOperator 0x555d399936c8 <col:3, col:4> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x555d399936b0 <col:4> 'uint32_t *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x555d39993690 <col:4> 'uint32_t *' lvalue ParmVar 0x555d39993088 'y' 'uint32_t *'
// |   `-ImplicitCastExpr 0x555d39993700 <col:8> 'uint32_t':'unsigned int' <LValueToRValue>
// |     `-DeclRefExpr 0x555d399936e0 <col:8> 'uint32_t':'unsigned int' lvalue Var 0x555d399934d0 'tmp' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x555d399932c8 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:"x |-> 'x"|END"
// |-AnnotateAttr 0x555d39993370 <col:23, col:61> pulse "requires:"y |-> 'y"|END"
// |-AnnotateAttr 0x555d399933e0 <line:4:24, col:71> pulse "ensures:"x |-> 'y"|END"
// `-AnnotateAttr 0x555d39993450 <col:24, col:71> pulse "ensures:"y |-> 'x"|END"
// FunctionDecl 0x555d39993958 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:49:1, line:53:1> line:49:6 used swap_fields_alt 'void (struct _u32_pair_struct *)'
// |-ParmVarDecl 0x555d399938c8 <col:22, col:47> col:47 used x 'struct _u32_pair_struct *'
// |-CompoundStmt 0x555d39993fc8 <col:50, line:53:1>
// | |-AttributedStmt 0x555d39993cf0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:50:37>
// | | |-AnnotateAttr 0x555d39993c70 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_explode !x|END"
// | | `-NullStmt 0x555d39993c68 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:50:37>
// | |-CallExpr 0x555d39993ea8 <line:51:3, col:34> 'void'
// | | |-ImplicitCastExpr 0x555d39993e90 <col:3> 'void (*)(uint32_t *, uint32_t *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x555d39993d08 <col:3> 'void (uint32_t *, uint32_t *)' Function 0x555d39993210 'swap_refs' 'void (uint32_t *, uint32_t *)'
// | | |-UnaryOperator 0x555d39993d90 <col:13, col:17> 'uint32_t *' prefix '&' cannot overflow
// | | | `-MemberExpr 0x555d39993d60 <col:14, col:17> 'uint32_t':'unsigned int' lvalue ->first 0x555d3998f8f0
// | | |   `-ImplicitCastExpr 0x555d39993d48 <col:14> 'struct _u32_pair_struct *' <LValueToRValue>
// | | |     `-DeclRefExpr 0x555d39993d28 <col:14> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d399938c8 'x' 'struct _u32_pair_struct *'
// | | `-UnaryOperator 0x555d39993e10 <col:24, col:28> 'uint32_t *' prefix '&' cannot overflow
// | |   `-MemberExpr 0x555d39993de0 <col:25, col:28> 'uint32_t':'unsigned int' lvalue ->second 0x555d3998f950
// | |     `-ImplicitCastExpr 0x555d39993dc8 <col:25> 'struct _u32_pair_struct *' <LValueToRValue>
// | |       `-DeclRefExpr 0x555d39993da8 <col:25> 'struct _u32_pair_struct *' lvalue ParmVar 0x555d399938c8 'x' 'struct _u32_pair_struct *'
// | `-AttributedStmt 0x555d39993fb0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:52:37>
// |   |-AnnotateAttr 0x555d39993f30 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:_u32_pair_struct_recover !x|END"
// |   `-NullStmt 0x555d39993f28 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:52:37>
// |-AnnotateAttr 0x555d39993a08 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#s : _u32_pair_struct_spec|END"
// |-AnnotateAttr 0x555d39993ac0 <line:3:23, col:61> pulse "requires:_u32_pair_struct_pred x s|END"
// `-AnnotateAttr 0x555d39993b40 <line:4:24, col:71> pulse "ensures:"exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})"|END"
// FunctionDecl 0x555d39994048 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:55:1, line:61:1> line:55:5 main 'int ()'
// `-CompoundStmt 0x555d399946d8 <col:12, line:61:1>
//   |-DeclStmt 0x555d39994260 <line:56:3, col:53>
//   | `-VarDecl 0x555d39994118 <col:3, col:52> col:28 used x 'struct _u32_pair_struct *' cinit
//   |   `-CallExpr 0x555d39994238 <col:32, col:52> 'struct _u32_pair_struct *'
//   |     `-ImplicitCastExpr 0x555d39994220 <col:32> 'struct _u32_pair_struct *(*)()' <FunctionToPointerDecay>
//   |       `-DeclRefExpr 0x555d39994180 <col:32> 'struct _u32_pair_struct *()' Function 0x555d3998fbe8 'new_u32_pair_struct' 'struct _u32_pair_struct *()'
//   |-CallExpr 0x555d39994338 <line:57:3, col:16> 'void'
//   | |-ImplicitCastExpr 0x555d39994320 <col:3> 'void (*)(struct _u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x555d39994278 <col:3> 'void (struct _u32_pair_struct *)' Function 0x555d39992730 'swap_fields' 'void (struct _u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x555d39994368 <col:15> 'struct _u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x555d39994298 <col:15> 'struct _u32_pair_struct *' lvalue Var 0x555d39994118 'x' 'struct _u32_pair_struct *'
//   |-CallExpr 0x555d39994438 <line:58:3, col:20> 'void'
//   | |-ImplicitCastExpr 0x555d39994420 <col:3> 'void (*)(struct _u32_pair_struct *)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x555d399943e0 <col:3> 'void (struct _u32_pair_struct *)' Function 0x555d39993958 'swap_fields_alt' 'void (struct _u32_pair_struct *)'
//   | `-ImplicitCastExpr 0x555d39994468 <col:19> 'struct _u32_pair_struct *' <LValueToRValue>
//   |   `-DeclRefExpr 0x555d39994400 <col:19> 'struct _u32_pair_struct *' lvalue Var 0x555d39994118 'x' 'struct _u32_pair_struct *'
//   |-AttributedStmt 0x555d399945c0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:59:94>
//   | |-AnnotateAttr 0x555d39994508 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul})|END"
//   | `-NullStmt 0x555d39994500 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_test_2.c:59:94>
//   `-CallExpr 0x555d39994678 <line:60:3, col:10> 'void'
//     |-ImplicitCastExpr 0x555d39994660 <col:3> 'void (*)(void *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x555d399945d8 <col:3> 'void (void *)' Function 0x555d39856ef8 'free' 'void (void *)'
//     `-ImplicitCastExpr 0x555d399946c0 <col:9> 'void *' <BitCast>
//       `-ImplicitCastExpr 0x555d399946a8 <col:9> 'struct _u32_pair_struct *' <LValueToRValue>
//         `-DeclRefExpr 0x555d399945f8 <col:9> 'struct _u32_pair_struct *' lvalue Var 0x555d39994118 'x' 'struct _u32_pair_struct *'
