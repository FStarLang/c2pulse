module Issue55_test

#lang-pulse

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn compare
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : SizeT.t)
requires pure (length a2 == SizeT.v l)
requires pure (length a1 == SizeT.v l)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : SizeT.t = l;
let i0 : SizeT.t = (int32_to_sizet 0l);
let mut i : SizeT.t = i0;
while((int32_to_bool (if (int32_to_bool ((bool_to_int32 (SizeT.lt (! i) (! l)))))
then (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i))))
else 0l));
)
invariant c. emp
{
i := (SizeT.add (! i) (int32_to_sizet 1l));
};(int32_to_bool ((bool_to_int32 (SizeT.eq (! i) (! l)))));
}

//Dumping the Clang AST.
// FunctionDecl 0x569363749880 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:15:1> line:6:6 compare 'bool (int *, int *, size_t)'
// |-ParmVarDecl 0x569363749478 <col:25, col:30> col:30 used a1 'int *'
// | `-AnnotateAttr 0x5693637494e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:l|END"
// |-ParmVarDecl 0x5693637495c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:6:45, col:50> col:50 used a2 'int *'
// | `-AnnotateAttr 0x569363749630 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:l|END"
// |-ParmVarDecl 0x569363749740 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:6:54, col:61> col:61 used l 'size_t':'unsigned long'
// |-CompoundStmt 0x56936374e0b0 <line:7:1, line:15:1>
// | |-DeclStmt 0x569363749ad8 <line:8:5, col:17>
// | | `-VarDecl 0x569363749a38 <col:5, col:16> col:12 used i 'size_t':'unsigned long' cinit
// | |   `-ImplicitCastExpr 0x569363749ac0 <col:16> 'size_t':'unsigned long' <IntegralCast>
// | |     `-IntegerLiteral 0x569363749aa0 <col:16> 'int' 0
// | |-WhileStmt 0x56936374dfb8 <line:9:5, line:13:5>
// | | |-ConditionalOperator 0x569363749d30 <line:9:12, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15> 'int'
// | | | |-ParenExpr 0x569363749b80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:9:12, col:18> 'int'
// | | | | `-BinaryOperator 0x569363749b60 <col:13, col:17> 'int' '<'
// | | | |   |-ImplicitCastExpr 0x569363749b30 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// | | | |   | `-DeclRefExpr 0x569363749af0 <col:13> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// | | | |   `-ImplicitCastExpr 0x569363749b48 <col:17> 'size_t':'unsigned long' <LValueToRValue>
// | | | |     `-DeclRefExpr 0x569363749b10 <col:17> 'size_t':'unsigned long' lvalue ParmVar 0x569363749740 'l' 'size_t':'unsigned long'
// | | | |-BinaryOperator 0x569363749cf0 <col:22, col:35> 'int' '=='
// | | | | |-ImplicitCastExpr 0x569363749cc0 <col:22, col:26> 'int' <LValueToRValue>
// | | | | | `-ArraySubscriptExpr 0x569363749c10 <col:22, col:26> 'int' lvalue
// | | | | |   |-ImplicitCastExpr 0x569363749be0 <col:22> 'int *' <LValueToRValue>
// | | | | |   | `-DeclRefExpr 0x569363749ba0 <col:22> 'int *' lvalue ParmVar 0x569363749478 'a1' 'int *'
// | | | | |   `-ImplicitCastExpr 0x569363749bf8 <col:25> 'size_t':'unsigned long' <LValueToRValue>
// | | | | |     `-DeclRefExpr 0x569363749bc0 <col:25> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// | | | | `-ImplicitCastExpr 0x569363749cd8 <col:31, col:35> 'int' <LValueToRValue>
// | | | |   `-ArraySubscriptExpr 0x569363749ca0 <col:31, col:35> 'int' lvalue
// | | | |     |-ImplicitCastExpr 0x569363749c70 <col:31> 'int *' <LValueToRValue>
// | | | |     | `-DeclRefExpr 0x569363749c30 <col:31> 'int *' lvalue ParmVar 0x5693637495c8 'a2' 'int *'
// | | | |     `-ImplicitCastExpr 0x569363749c88 <col:34> 'size_t':'unsigned long' <LValueToRValue>
// | | | |       `-DeclRefExpr 0x569363749c50 <col:34> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// | | | `-IntegerLiteral 0x569363749d10 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15> 'int' 0
// | | `-AttributedStmt 0x56936374dfa0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:7:25, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:13:5>
// | |   |-AnnotateAttr 0x56936374df20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:7:27, col:76> pulse "invariants:invariant c. emp|END"
// | |   `-CompoundStmt 0x569363749e70 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:11:5, line:13:5>
// | |     `-BinaryOperator 0x569363749e50 <line:12:9, col:17> 'size_t':'unsigned long' '='
// | |       |-DeclRefExpr 0x569363749da0 <col:9> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// | |       `-BinaryOperator 0x569363749e30 <col:13, col:17> 'size_t':'unsigned long' '+'
// | |         |-ImplicitCastExpr 0x569363749e00 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// | |         | `-DeclRefExpr 0x569363749dc0 <col:13> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// | |         `-ImplicitCastExpr 0x569363749e18 <col:17> 'size_t':'unsigned long' <IntegralCast>
// | |           `-IntegerLiteral 0x569363749de0 <col:17> 'int' 1
// | `-ReturnStmt 0x56936374e0a0 <line:14:5, col:19>
// |   `-ImplicitCastExpr 0x56936374e088 <col:12, col:19> 'bool' <IntegralToBoolean>
// |     `-ParenExpr 0x56936374e068 <col:12, col:19> 'int'
// |       `-BinaryOperator 0x56936374e048 <col:13, col:18> 'int' '=='
// |         |-ImplicitCastExpr 0x56936374e018 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |         | `-DeclRefExpr 0x56936374dfd8 <col:13> 'size_t':'unsigned long' lvalue Var 0x569363749a38 'i' 'size_t':'unsigned long'
// |         `-ImplicitCastExpr 0x56936374e030 <col:18> 'size_t':'unsigned long' <LValueToRValue>
// |           `-DeclRefExpr 0x56936374dff8 <col:18> 'size_t':'unsigned long' lvalue ParmVar 0x569363749740 'l' 'size_t':'unsigned long'
// `-AnnotateAttr 0x569363749940 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
