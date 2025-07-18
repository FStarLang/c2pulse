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
// FunctionDecl 0x63e1e925b880 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:15:1> line:6:6 compare 'bool (int *, int *, size_t)'
// |-ParmVarDecl 0x63e1e925b478 <col:25, col:30> col:30 used a1 'int *'
// | `-AnnotateAttr 0x63e1e925b4e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:l|END"
// |-ParmVarDecl 0x63e1e925b5c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:6:45, col:50> col:50 used a2 'int *'
// | `-AnnotateAttr 0x63e1e925b630 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:l|END"
// |-ParmVarDecl 0x63e1e925b740 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:6:54, col:61> col:61 used l 'size_t':'unsigned long'
// |-CompoundStmt 0x63e1e92600b0 <line:7:1, line:15:1>
// | |-DeclStmt 0x63e1e925bad8 <line:8:5, col:17>
// | | `-VarDecl 0x63e1e925ba38 <col:5, col:16> col:12 used i 'size_t':'unsigned long' cinit
// | |   `-ImplicitCastExpr 0x63e1e925bac0 <col:16> 'size_t':'unsigned long' <IntegralCast>
// | |     `-IntegerLiteral 0x63e1e925baa0 <col:16> 'int' 0
// | |-WhileStmt 0x63e1e925ffb8 <line:9:5, line:13:5>
// | | |-ConditionalOperator 0x63e1e925bd30 <line:9:12, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15> 'int'
// | | | |-ParenExpr 0x63e1e925bb80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:9:12, col:18> 'int'
// | | | | `-BinaryOperator 0x63e1e925bb60 <col:13, col:17> 'int' '<'
// | | | |   |-ImplicitCastExpr 0x63e1e925bb30 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// | | | |   | `-DeclRefExpr 0x63e1e925baf0 <col:13> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// | | | |   `-ImplicitCastExpr 0x63e1e925bb48 <col:17> 'size_t':'unsigned long' <LValueToRValue>
// | | | |     `-DeclRefExpr 0x63e1e925bb10 <col:17> 'size_t':'unsigned long' lvalue ParmVar 0x63e1e925b740 'l' 'size_t':'unsigned long'
// | | | |-BinaryOperator 0x63e1e925bcf0 <col:22, col:35> 'int' '=='
// | | | | |-ImplicitCastExpr 0x63e1e925bcc0 <col:22, col:26> 'int' <LValueToRValue>
// | | | | | `-ArraySubscriptExpr 0x63e1e925bc10 <col:22, col:26> 'int' lvalue
// | | | | |   |-ImplicitCastExpr 0x63e1e925bbe0 <col:22> 'int *' <LValueToRValue>
// | | | | |   | `-DeclRefExpr 0x63e1e925bba0 <col:22> 'int *' lvalue ParmVar 0x63e1e925b478 'a1' 'int *'
// | | | | |   `-ImplicitCastExpr 0x63e1e925bbf8 <col:25> 'size_t':'unsigned long' <LValueToRValue>
// | | | | |     `-DeclRefExpr 0x63e1e925bbc0 <col:25> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// | | | | `-ImplicitCastExpr 0x63e1e925bcd8 <col:31, col:35> 'int' <LValueToRValue>
// | | | |   `-ArraySubscriptExpr 0x63e1e925bca0 <col:31, col:35> 'int' lvalue
// | | | |     |-ImplicitCastExpr 0x63e1e925bc70 <col:31> 'int *' <LValueToRValue>
// | | | |     | `-DeclRefExpr 0x63e1e925bc30 <col:31> 'int *' lvalue ParmVar 0x63e1e925b5c8 'a2' 'int *'
// | | | |     `-ImplicitCastExpr 0x63e1e925bc88 <col:34> 'size_t':'unsigned long' <LValueToRValue>
// | | | |       `-DeclRefExpr 0x63e1e925bc50 <col:34> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// | | | `-IntegerLiteral 0x63e1e925bd10 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15> 'int' 0
// | | `-AttributedStmt 0x63e1e925ffa0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:7:25, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:13:5>
// | |   |-AnnotateAttr 0x63e1e925ff20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:7:27, col:76> pulse "invariants:invariant c. emp|END"
// | |   `-CompoundStmt 0x63e1e925be70 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue55_test.c:11:5, line:13:5>
// | |     `-BinaryOperator 0x63e1e925be50 <line:12:9, col:17> 'size_t':'unsigned long' '='
// | |       |-DeclRefExpr 0x63e1e925bda0 <col:9> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// | |       `-BinaryOperator 0x63e1e925be30 <col:13, col:17> 'size_t':'unsigned long' '+'
// | |         |-ImplicitCastExpr 0x63e1e925be00 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// | |         | `-DeclRefExpr 0x63e1e925bdc0 <col:13> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// | |         `-ImplicitCastExpr 0x63e1e925be18 <col:17> 'size_t':'unsigned long' <IntegralCast>
// | |           `-IntegerLiteral 0x63e1e925bde0 <col:17> 'int' 1
// | `-ReturnStmt 0x63e1e92600a0 <line:14:5, col:19>
// |   `-ImplicitCastExpr 0x63e1e9260088 <col:12, col:19> 'bool' <IntegralToBoolean>
// |     `-ParenExpr 0x63e1e9260068 <col:12, col:19> 'int'
// |       `-BinaryOperator 0x63e1e9260048 <col:13, col:18> 'int' '=='
// |         |-ImplicitCastExpr 0x63e1e9260018 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |         | `-DeclRefExpr 0x63e1e925ffd8 <col:13> 'size_t':'unsigned long' lvalue Var 0x63e1e925ba38 'i' 'size_t':'unsigned long'
// |         `-ImplicitCastExpr 0x63e1e9260030 <col:18> 'size_t':'unsigned long' <LValueToRValue>
// |           `-DeclRefExpr 0x63e1e925fff8 <col:18> 'size_t':'unsigned long' lvalue ParmVar 0x63e1e925b740 'l' 'size_t':'unsigned long'
// `-AnnotateAttr 0x63e1e925b940 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:13:5, col:59> pulse "expect_failure:|END"
