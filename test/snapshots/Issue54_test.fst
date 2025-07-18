module Issue54_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn compare_elt
(a1 : array Int32.t)
(a2 : array Int32.t)
(i : SizeT.t)
(len : SizeT.t)
requires pure (length a2 == SizeT.v len)
requires pure (length a1 == SizeT.v len)
requires exists* v. (a1 |-> v)
requires exists* v. (a2 |-> v)
returns b:_Bool
ensures exists* v. (a1 |-> v)
ensures exists* v. (a2 |-> v)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut i : SizeT.t = i;
let mut len : SizeT.t = len;
if((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (! len)))))
{
pts_to_len !a1;
pts_to_len !a2;
(int32_to_bool ((bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i))))));
}
else
{
(int32_to_bool 0l);
};
}

//Dumping the Clang AST.
// FunctionDecl 0x61ac2834d150 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:23:1> line:11:6 compare_elt 'bool (int *, int *, size_t, size_t)'
// |-ParmVarDecl 0x61ac2834ccc8 <col:30, col:35> col:35 used a1 'int *'
// | `-AnnotateAttr 0x61ac2834cd30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x61ac2834ce18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:11:51, col:56> col:56 used a2 'int *'
// | `-AnnotateAttr 0x61ac2834ce80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x61ac2834cf90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:11:60, col:67> col:67 used i 'size_t':'unsigned long'
// |-ParmVarDecl 0x61ac2834d008 <col:70, col:77> col:77 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x61ac282969a8 <line:12:1, line:23:1>
// | `-IfStmt 0x61ac28296978 <line:13:5, line:22:5> has_else
// |   |-BinaryOperator 0x61ac28296580 <line:13:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x61ac28296550 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x61ac28296510 <col:9> 'size_t':'unsigned long' lvalue ParmVar 0x61ac2834cf90 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x61ac28296568 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x61ac28296530 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x61ac2834d008 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x61ac282968f0 <line:14:5, line:18:5>
// |   | |-AttributedStmt 0x61ac28296650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:15:30>
// |   | | |-AnnotateAttr 0x61ac282965e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x61ac282965d8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:15:30>
// |   | |-AttributedStmt 0x61ac28296720 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:16:30>
// |   | | |-AnnotateAttr 0x61ac282966a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x61ac282966a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:16:30>
// |   | `-ReturnStmt 0x61ac282968e0 <line:17:9, col:31>
// |   |   `-ImplicitCastExpr 0x61ac282968c8 <col:16, col:31> 'bool' <IntegralToBoolean>
// |   |     `-ParenExpr 0x61ac282968a8 <col:16, col:31> 'int'
// |   |       `-BinaryOperator 0x61ac28296888 <col:17, col:30> 'int' '=='
// |   |         |-ImplicitCastExpr 0x61ac28296858 <col:17, col:21> 'int' <LValueToRValue>
// |   |         | `-ArraySubscriptExpr 0x61ac282967a8 <col:17, col:21> 'int' lvalue
// |   |         |   |-ImplicitCastExpr 0x61ac28296778 <col:17> 'int *' <LValueToRValue>
// |   |         |   | `-DeclRefExpr 0x61ac28296738 <col:17> 'int *' lvalue ParmVar 0x61ac2834ccc8 'a1' 'int *'
// |   |         |   `-ImplicitCastExpr 0x61ac28296790 <col:20> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |     `-DeclRefExpr 0x61ac28296758 <col:20> 'size_t':'unsigned long' lvalue ParmVar 0x61ac2834cf90 'i' 'size_t':'unsigned long'
// |   |         `-ImplicitCastExpr 0x61ac28296870 <col:26, col:30> 'int' <LValueToRValue>
// |   |           `-ArraySubscriptExpr 0x61ac28296838 <col:26, col:30> 'int' lvalue
// |   |             |-ImplicitCastExpr 0x61ac28296808 <col:26> 'int *' <LValueToRValue>
// |   |             | `-DeclRefExpr 0x61ac282967c8 <col:26> 'int *' lvalue ParmVar 0x61ac2834ce18 'a2' 'int *'
// |   |             `-ImplicitCastExpr 0x61ac28296820 <col:29> 'size_t':'unsigned long' <LValueToRValue>
// |   |               `-DeclRefExpr 0x61ac282967e8 <col:29> 'size_t':'unsigned long' lvalue ParmVar 0x61ac2834cf90 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x61ac28296960 <line:20:5, line:22:5>
// |     `-ReturnStmt 0x61ac28296950 <line:21:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15>
// |       `-ImplicitCastExpr 0x61ac28296938 <col:15> 'bool' <IntegralToBoolean>
// |         `-IntegerLiteral 0x61ac28296918 <col:15> 'int' 0
// |-AnnotateAttr 0x61ac2834d218 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x61ac2834d2d0 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x61ac2834d350 <line:5:22, col:58> pulse "returns:b:_Bool|END"
// |-AnnotateAttr 0x61ac2834d3c0 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x61ac2834d440 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
