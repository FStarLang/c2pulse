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
// FunctionDecl 0x57042472d600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue33_test.c:8:1, line:11:1> line:8:10 count_down 'uint32_t (uint32_t *)'
// |-ParmVarDecl 0x57042472d4f8 <col:22, col:32> col:32 used x 'uint32_t *'
// |-CompoundStmt 0x57042472d9a0 <line:9:1, line:11:1>
// | `-ReturnStmt 0x57042472d990 <line:10:3, col:18>
// |   `-ImplicitCastExpr 0x57042472d978 <col:10, col:18> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-ParenExpr 0x57042472d958 <col:10, col:18> 'int'
// |       `-BinaryOperator 0x57042472d938 <col:11, col:17> 'int' '=='
// |         |-ImplicitCastExpr 0x57042472d908 <col:11, col:12> 'uint32_t':'unsigned int' <LValueToRValue>
// |         | `-UnaryOperator 0x57042472d8d0 <col:11, col:12> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |         |   `-ImplicitCastExpr 0x57042472d8b8 <col:12> 'uint32_t *' <LValueToRValue>
// |         |     `-DeclRefExpr 0x57042472d898 <col:12> 'uint32_t *' lvalue ParmVar 0x57042472d4f8 'x' 'uint32_t *'
// |         `-ImplicitCastExpr 0x57042472d920 <col:17> 'uint32_t':'unsigned int' <IntegralCast>
// |           `-IntegerLiteral 0x57042472d8e8 <col:17> 'int' 0
// |-AnnotateAttr 0x57042472d6b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. x |-> v|END"
// |-AnnotateAttr 0x57042472d760 <line:5:22, col:58> pulse "returns:UInt32.t|END"
// `-AnnotateAttr 0x57042472d7d0 <line:4:24, col:71> pulse "ensures:exists* v. x |-> v|END"
// FunctionDecl 0x57042472db20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue33_test.c:17:1, line:20:1> line:17:10 decr 'uint32_t (uint32_t *)'
// |-ParmVarDecl 0x57042472da90 <col:16, col:26> col:26 used x 'uint32_t *'
// |-CompoundStmt 0x57042472de70 <line:18:1, line:20:1>
// | `-ReturnStmt 0x57042472de60 <line:19:3, col:17>
// |   `-ParenExpr 0x57042472de40 <col:10, col:17> 'uint32_t':'unsigned int'
// |     `-BinaryOperator 0x57042472de20 <col:11, col:16> 'uint32_t':'unsigned int' '-'
// |       |-ImplicitCastExpr 0x57042472ddf0 <col:11, col:12> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-UnaryOperator 0x57042472ddb8 <col:11, col:12> 'uint32_t':'unsigned int' lvalue prefix '*' cannot overflow
// |       |   `-ImplicitCastExpr 0x57042472dda0 <col:12> 'uint32_t *' <LValueToRValue>
// |       |     `-DeclRefExpr 0x57042472dd80 <col:12> 'uint32_t *' lvalue ParmVar 0x57042472da90 'x' 'uint32_t *'
// |       `-ImplicitCastExpr 0x57042472de08 <col:16> 'uint32_t':'unsigned int' <IntegralCast>
// |         `-IntegerLiteral 0x57042472ddd0 <col:16> 'int' 0
// |-AnnotateAttr 0x57042472dbd0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. x |-> v|END"
// |-AnnotateAttr 0x57042472dc80 <line:5:22, col:58> pulse "returns:b:FStar.UInt32.t|END"
// `-AnnotateAttr 0x57042472dd00 <line:4:24, col:71> pulse "ensures:exists* v. x |-> v|END"
