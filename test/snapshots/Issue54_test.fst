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
// FunctionDecl 0x5b42e8266150 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:23:1> line:11:6 compare_elt 'bool (int *, int *, size_t, size_t)'
// |-ParmVarDecl 0x5b42e8265cc8 <col:30, col:35> col:35 used a1 'int *'
// | `-AnnotateAttr 0x5b42e8265d30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5b42e8265e18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:11:51, col:56> col:56 used a2 'int *'
// | `-AnnotateAttr 0x5b42e8265e80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5b42e8265f90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:11:60, col:67> col:67 used i 'size_t':'unsigned long'
// |-ParmVarDecl 0x5b42e8266008 <col:70, col:77> col:77 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x5b42e81af9a8 <line:12:1, line:23:1>
// | `-IfStmt 0x5b42e81af978 <line:13:5, line:22:5> has_else
// |   |-BinaryOperator 0x5b42e81af580 <line:13:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x5b42e81af550 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5b42e81af510 <col:9> 'size_t':'unsigned long' lvalue ParmVar 0x5b42e8265f90 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x5b42e81af568 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5b42e81af530 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x5b42e8266008 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x5b42e81af8f0 <line:14:5, line:18:5>
// |   | |-AttributedStmt 0x5b42e81af650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:15:30>
// |   | | |-AnnotateAttr 0x5b42e81af5e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x5b42e81af5d8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:15:30>
// |   | |-AttributedStmt 0x5b42e81af720 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:16:30>
// |   | | |-AnnotateAttr 0x5b42e81af6a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x5b42e81af6a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test.c:16:30>
// |   | `-ReturnStmt 0x5b42e81af8e0 <line:17:9, col:31>
// |   |   `-ImplicitCastExpr 0x5b42e81af8c8 <col:16, col:31> 'bool' <IntegralToBoolean>
// |   |     `-ParenExpr 0x5b42e81af8a8 <col:16, col:31> 'int'
// |   |       `-BinaryOperator 0x5b42e81af888 <col:17, col:30> 'int' '=='
// |   |         |-ImplicitCastExpr 0x5b42e81af858 <col:17, col:21> 'int' <LValueToRValue>
// |   |         | `-ArraySubscriptExpr 0x5b42e81af7a8 <col:17, col:21> 'int' lvalue
// |   |         |   |-ImplicitCastExpr 0x5b42e81af778 <col:17> 'int *' <LValueToRValue>
// |   |         |   | `-DeclRefExpr 0x5b42e81af738 <col:17> 'int *' lvalue ParmVar 0x5b42e8265cc8 'a1' 'int *'
// |   |         |   `-ImplicitCastExpr 0x5b42e81af790 <col:20> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |     `-DeclRefExpr 0x5b42e81af758 <col:20> 'size_t':'unsigned long' lvalue ParmVar 0x5b42e8265f90 'i' 'size_t':'unsigned long'
// |   |         `-ImplicitCastExpr 0x5b42e81af870 <col:26, col:30> 'int' <LValueToRValue>
// |   |           `-ArraySubscriptExpr 0x5b42e81af838 <col:26, col:30> 'int' lvalue
// |   |             |-ImplicitCastExpr 0x5b42e81af808 <col:26> 'int *' <LValueToRValue>
// |   |             | `-DeclRefExpr 0x5b42e81af7c8 <col:26> 'int *' lvalue ParmVar 0x5b42e8265e18 'a2' 'int *'
// |   |             `-ImplicitCastExpr 0x5b42e81af820 <col:29> 'size_t':'unsigned long' <LValueToRValue>
// |   |               `-DeclRefExpr 0x5b42e81af7e8 <col:29> 'size_t':'unsigned long' lvalue ParmVar 0x5b42e8265f90 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x5b42e81af960 <line:20:5, line:22:5>
// |     `-ReturnStmt 0x5b42e81af950 <line:21:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15>
// |       `-ImplicitCastExpr 0x5b42e81af938 <col:15> 'bool' <IntegralToBoolean>
// |         `-IntegerLiteral 0x5b42e81af918 <col:15> 'int' 0
// |-AnnotateAttr 0x5b42e8266218 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x5b42e82662d0 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x5b42e8266350 <line:5:22, col:58> pulse "returns:b:_Bool|END"
// |-AnnotateAttr 0x5b42e82663c0 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x5b42e8266440 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
