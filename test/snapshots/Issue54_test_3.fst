module Issue54_test_3

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_ampamp ()
returns Int32.t
{
let mut a: Int32.t = witness #_ #_;
let mut b: Int32.t = witness #_ #_;
(bool_to_int32 (op_AmpAmp (int32_to_bool (! a)) (int32_to_bool (! b))));
}

fn test_barbar ()
returns Int32.t
{
let mut a: Int32.t = witness #_ #_;
let mut b: Int32.t = witness #_ #_;
(bool_to_int32 (op_BarBar (int32_to_bool (! a)) (int32_to_bool (! b))));
}

fn compare_elt
(a1 : array Int32.t)
(a2 : array Int32.t)
(len : SizeT.t)
requires pure (length a2 == SizeT.v len)
requires pure (length a1 == SizeT.v len)
requires exists* v. (a1 |-> v)
requires exists* v. (a2 |-> v)
returns _Bool
ensures exists* v. (a1 |-> v)
ensures exists* v. (a2 |-> v)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut len : SizeT.t = len;
let i0 : SizeT.t = (int32_to_sizet 0l);
let mut i : SizeT.t = i0;
if((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (! len)))))
{
pts_to_len !a1;
pts_to_len !a2;
(int32_to_bool ((bool_to_int32 (op_AmpAmp (int32_to_bool ((bool_to_int32 (SizeT.lt (! i) (! len))))) (int32_to_bool (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i)))))))));
}
else
{
(int32_to_bool 0l);
};
}

//Dumping the Clang AST.
// FunctionDecl 0x57d5be17f8a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:7:1, line:11:1> line:7:5 test_ampamp 'int ()'
// |-CompoundStmt 0x57d5be17fc08 <col:18, line:11:1>
// | |-DeclStmt 0x57d5be17fab8 <line:8:5, col:10>
// | | `-VarDecl 0x57d5be17fa50 <col:5, col:9> col:9 used a 'int'
// | |-DeclStmt 0x57d5be17fb50 <line:9:5, col:10>
// | | `-VarDecl 0x57d5be17fae8 <col:5, col:9> col:9 used b 'int'
// | `-ReturnStmt 0x57d5be17fbf8 <line:10:5, col:17>
// |   `-BinaryOperator 0x57d5be17fbd8 <col:12, col:17> 'int' '&&'
// |     |-ImplicitCastExpr 0x57d5be17fba8 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x57d5be17fb68 <col:12> 'int' lvalue Var 0x57d5be17fa50 'a' 'int'
// |     `-ImplicitCastExpr 0x57d5be17fbc0 <col:17> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x57d5be17fb88 <col:17> 'int' lvalue Var 0x57d5be17fae8 'b' 'int'
// `-AnnotateAttr 0x57d5be17f950 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x57d5be17fc90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:14:1, line:19:1> line:14:5 test_barbar 'int ()'
// |-CompoundStmt 0x57d5be17ffb0 <col:18, line:19:1>
// | |-DeclStmt 0x57d5be17fe60 <line:15:5, col:10>
// | | `-VarDecl 0x57d5be17fdf8 <col:5, col:9> col:9 used a 'int'
// | |-DeclStmt 0x57d5be17fef8 <line:16:5, col:10>
// | | `-VarDecl 0x57d5be17fe90 <col:5, col:9> col:9 used b 'int'
// | `-ReturnStmt 0x57d5be17ffa0 <line:17:5, col:17>
// |   `-BinaryOperator 0x57d5be17ff80 <col:12, col:17> 'int' '||'
// |     |-ImplicitCastExpr 0x57d5be17ff50 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x57d5be17ff10 <col:12> 'int' lvalue Var 0x57d5be17fdf8 'a' 'int'
// |     `-ImplicitCastExpr 0x57d5be17ff68 <col:17> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x57d5be17ff30 <col:17> 'int' lvalue Var 0x57d5be17fe90 'b' 'int'
// `-AnnotateAttr 0x57d5be17fd38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x57d5be0e5040 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:39:1> line:26:6 compare_elt 'bool (int *, int *, size_t)'
// |-ParmVarDecl 0x57d5be0e4c40 <col:30, col:35> col:35 used a1 'int *'
// | `-AnnotateAttr 0x57d5be0e4ca8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x57d5be0e4d88 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:26:51, col:56> col:56 used a2 'int *'
// | `-AnnotateAttr 0x57d5be0e4df0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x57d5be0e4f00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:26:60, col:67> col:67 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x57d5be0e59d8 <line:27:1, line:39:1>
// | |-DeclStmt 0x57d5be0e5450 <line:28:5, col:17>
// | | `-VarDecl 0x57d5be0e53b0 <col:5, col:16> col:12 used i 'size_t':'unsigned long' cinit
// | |   `-ImplicitCastExpr 0x57d5be0e5438 <col:16> 'size_t':'unsigned long' <IntegralCast>
// | |     `-IntegerLiteral 0x57d5be0e5418 <col:16> 'int' 0
// | `-IfStmt 0x57d5be0e59a8 <line:29:5, line:38:5> has_else
// |   |-BinaryOperator 0x57d5be0e54d8 <line:29:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x57d5be0e54a8 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x57d5be0e5468 <col:9> 'size_t':'unsigned long' lvalue Var 0x57d5be0e53b0 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x57d5be0e54c0 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x57d5be0e5488 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x57d5be0e4f00 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x57d5be0e5920 <line:30:5, line:34:5>
// |   | |-AttributedStmt 0x57d5be0e55b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:31:30>
// |   | | |-AnnotateAttr 0x57d5be0e5538 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x57d5be0e5530 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:31:30>
// |   | |-AttributedStmt 0x57d5be0e5680 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:32:30>
// |   | | |-AnnotateAttr 0x57d5be0e5608 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x57d5be0e5600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:32:30>
// |   | `-ReturnStmt 0x57d5be0e5910 <line:33:9, col:44>
// |   |   `-ImplicitCastExpr 0x57d5be0e58f8 <col:16, col:44> 'bool' <IntegralToBoolean>
// |   |     `-ParenExpr 0x57d5be0e58d8 <col:16, col:44> 'int'
// |   |       `-BinaryOperator 0x57d5be0e58b8 <col:17, col:43> 'int' '&&'
// |   |         |-ParenExpr 0x57d5be0e5728 <col:17, col:25> 'int'
// |   |         | `-BinaryOperator 0x57d5be0e5708 <col:18, col:22> 'int' '<'
// |   |         |   |-ImplicitCastExpr 0x57d5be0e56d8 <col:18> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |   | `-DeclRefExpr 0x57d5be0e5698 <col:18> 'size_t':'unsigned long' lvalue Var 0x57d5be0e53b0 'i' 'size_t':'unsigned long'
// |   |         |   `-ImplicitCastExpr 0x57d5be0e56f0 <col:22> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |     `-DeclRefExpr 0x57d5be0e56b8 <col:22> 'size_t':'unsigned long' lvalue ParmVar 0x57d5be0e4f00 'len' 'size_t':'unsigned long'
// |   |         `-BinaryOperator 0x57d5be0e5898 <col:30, col:43> 'int' '=='
// |   |           |-ImplicitCastExpr 0x57d5be0e5868 <col:30, col:34> 'int' <LValueToRValue>
// |   |           | `-ArraySubscriptExpr 0x57d5be0e57b8 <col:30, col:34> 'int' lvalue
// |   |           |   |-ImplicitCastExpr 0x57d5be0e5788 <col:30> 'int *' <LValueToRValue>
// |   |           |   | `-DeclRefExpr 0x57d5be0e5748 <col:30> 'int *' lvalue ParmVar 0x57d5be0e4c40 'a1' 'int *'
// |   |           |   `-ImplicitCastExpr 0x57d5be0e57a0 <col:33> 'size_t':'unsigned long' <LValueToRValue>
// |   |           |     `-DeclRefExpr 0x57d5be0e5768 <col:33> 'size_t':'unsigned long' lvalue Var 0x57d5be0e53b0 'i' 'size_t':'unsigned long'
// |   |           `-ImplicitCastExpr 0x57d5be0e5880 <col:39, col:43> 'int' <LValueToRValue>
// |   |             `-ArraySubscriptExpr 0x57d5be0e5848 <col:39, col:43> 'int' lvalue
// |   |               |-ImplicitCastExpr 0x57d5be0e5818 <col:39> 'int *' <LValueToRValue>
// |   |               | `-DeclRefExpr 0x57d5be0e57d8 <col:39> 'int *' lvalue ParmVar 0x57d5be0e4d88 'a2' 'int *'
// |   |               `-ImplicitCastExpr 0x57d5be0e5830 <col:42> 'size_t':'unsigned long' <LValueToRValue>
// |   |                 `-DeclRefExpr 0x57d5be0e57f8 <col:42> 'size_t':'unsigned long' lvalue Var 0x57d5be0e53b0 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x57d5be0e5990 <line:36:5, line:38:5>
// |     `-ReturnStmt 0x57d5be0e5980 <line:37:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15>
// |       `-ImplicitCastExpr 0x57d5be0e5968 <col:15> 'bool' <IntegralToBoolean>
// |         `-IntegerLiteral 0x57d5be0e5948 <col:15> 'int' 0
// |-AnnotateAttr 0x57d5be0e5100 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x57d5be0e51b0 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x57d5be0e5230 <line:5:22, col:58> pulse "returns:_Bool|END"
// |-AnnotateAttr 0x57d5be0e52a0 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x57d5be0e5320 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
