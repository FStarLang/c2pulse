module Issue33_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn count_down
(x : ref UInt32.t)
requires exists* v. x |-> v
returns UInt32.t
ensures exists* v. x |-> v
{
let mut x : (ref UInt32.t) = x;
(int32_to_uint32 ((bool_to_int32 (UInt32.eq (! (! x)) (int32_to_uint32 0l)))));
}

fn decr
(x : ref UInt32.t)
requires exists* v. x |-> v
returns b:FStar.UInt32.t
ensures exists* v. x |-> v
{
let mut x : (ref UInt32.t) = x;
((UInt32.sub (! (! x)) (int32_to_uint32 0l)));
}

//Dumping the Clang AST.
// FunctionDecl 0x62def4317600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue33_test.c:8:1, line:11:1> line:8:10 count_down 'uint32_t (uint32_t *)'
// |-ParmVarDecl 0x62def43174f8 <col:22, col:32> col:32 used x 'uint32_t *'
// |-CompoundStmt 0x62def43179a0 <line:9:1, line:11:1>
// | `-ReturnStmt 0x62def4317990 <line:10:3, col:18>
// |   `-ImplicitCastExpr 0x62def4317978 <col:10, col:18> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-ParenExpr 0x62def4317958 <col:10, col:18> 'int'
// |       `-BinaryOperator 0x62def4317938 <col:11, col:17> 'int' '=='
// |         |-ImplicitCastExpr 0x62def4317908 <col:11, col:12> 'uint32_t':'unsigned int' <LValueToRValue>
// |         | `-UnaryOperator 0x62def43178d0 <col:11, col:12> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |         |   `-ImplicitCastExpr 0x62def43178b8 <col:12> 'uint32_t *' <LValueToRValue>
// |         |     `-DeclRefExpr 0x62def4317898 <col:12> 'uint32_t *' lvalue ParmVar 0x62def43174f8 'x' 'uint32_t *'
// |         `-ImplicitCastExpr 0x62def4317920 <col:17> 'uint32_t':'unsigned int' <IntegralCast>
// |           `-IntegerLiteral 0x62def43178e8 <col:17> 'int' 0
// |-AnnotateAttr 0x62def43176b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. x |-> v|END"
// |-AnnotateAttr 0x62def4317760 <line:5:22, col:58> pulse "returns:UInt32.t|END"
// `-AnnotateAttr 0x62def43177d0 <line:4:24, col:71> pulse "ensures:exists* v. x |-> v|END"
// FunctionDecl 0x62def4317b20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue33_test.c:17:1, line:20:1> line:17:10 decr 'uint32_t (uint32_t *)'
// |-ParmVarDecl 0x62def4317a90 <col:16, col:26> col:26 used x 'uint32_t *'
// |-CompoundStmt 0x62def4317e70 <line:18:1, line:20:1>
// | `-ReturnStmt 0x62def4317e60 <line:19:3, col:17>
// |   `-ParenExpr 0x62def4317e40 <col:10, col:17> 'uint32_t':'unsigned int'
// |     `-BinaryOperator 0x62def4317e20 <col:11, col:16> 'uint32_t':'unsigned int' '-'
// |       |-ImplicitCastExpr 0x62def4317df0 <col:11, col:12> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-UnaryOperator 0x62def4317db8 <col:11, col:12> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |       |   `-ImplicitCastExpr 0x62def4317da0 <col:12> 'uint32_t *' <LValueToRValue>
// |       |     `-DeclRefExpr 0x62def4317d80 <col:12> 'uint32_t *' lvalue ParmVar 0x62def4317a90 'x' 'uint32_t *'
// |       `-ImplicitCastExpr 0x62def4317e08 <col:16> 'uint32_t':'unsigned int' <IntegralCast>
// |         `-IntegerLiteral 0x62def4317dd0 <col:16> 'int' 0
// |-AnnotateAttr 0x62def4317bd0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. x |-> v|END"
// |-AnnotateAttr 0x62def4317c80 <line:5:22, col:58> pulse "returns:b:FStar.UInt32.t|END"
// `-AnnotateAttr 0x62def4317d00 <line:4:24, col:71> pulse "ensures:exists* v. x |-> v|END"
