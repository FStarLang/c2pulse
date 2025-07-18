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
// FunctionDecl 0x5c19a3f138a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:7:1, line:11:1> line:7:5 test_ampamp 'int ()'
// |-CompoundStmt 0x5c19a3f13c08 <col:18, line:11:1>
// | |-DeclStmt 0x5c19a3f13ab8 <line:8:5, col:10>
// | | `-VarDecl 0x5c19a3f13a50 <col:5, col:9> col:9 used a 'int'
// | |-DeclStmt 0x5c19a3f13b50 <line:9:5, col:10>
// | | `-VarDecl 0x5c19a3f13ae8 <col:5, col:9> col:9 used b 'int'
// | `-ReturnStmt 0x5c19a3f13bf8 <line:10:5, col:17>
// |   `-BinaryOperator 0x5c19a3f13bd8 <col:12, col:17> 'int' '&&'
// |     |-ImplicitCastExpr 0x5c19a3f13ba8 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x5c19a3f13b68 <col:12> 'int' lvalue Var 0x5c19a3f13a50 'a' 'int'
// |     `-ImplicitCastExpr 0x5c19a3f13bc0 <col:17> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x5c19a3f13b88 <col:17> 'int' lvalue Var 0x5c19a3f13ae8 'b' 'int'
// `-AnnotateAttr 0x5c19a3f13950 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x5c19a3f13c90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:14:1, line:19:1> line:14:5 test_barbar 'int ()'
// |-CompoundStmt 0x5c19a3f13fb0 <col:18, line:19:1>
// | |-DeclStmt 0x5c19a3f13e60 <line:15:5, col:10>
// | | `-VarDecl 0x5c19a3f13df8 <col:5, col:9> col:9 used a 'int'
// | |-DeclStmt 0x5c19a3f13ef8 <line:16:5, col:10>
// | | `-VarDecl 0x5c19a3f13e90 <col:5, col:9> col:9 used b 'int'
// | `-ReturnStmt 0x5c19a3f13fa0 <line:17:5, col:17>
// |   `-BinaryOperator 0x5c19a3f13f80 <col:12, col:17> 'int' '||'
// |     |-ImplicitCastExpr 0x5c19a3f13f50 <col:12> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x5c19a3f13f10 <col:12> 'int' lvalue Var 0x5c19a3f13df8 'a' 'int'
// |     `-ImplicitCastExpr 0x5c19a3f13f68 <col:17> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x5c19a3f13f30 <col:17> 'int' lvalue Var 0x5c19a3f13e90 'b' 'int'
// `-AnnotateAttr 0x5c19a3f13d38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x5c19a3e79040 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:39:1> line:26:6 compare_elt 'bool (int *, int *, size_t)'
// |-ParmVarDecl 0x5c19a3e78c40 <col:30, col:35> col:35 used a1 'int *'
// | `-AnnotateAttr 0x5c19a3e78ca8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5c19a3e78d88 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:26:51, col:56> col:56 used a2 'int *'
// | `-AnnotateAttr 0x5c19a3e78df0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5c19a3e78f00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:26:60, col:67> col:67 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x5c19a3e799d8 <line:27:1, line:39:1>
// | |-DeclStmt 0x5c19a3e79450 <line:28:5, col:17>
// | | `-VarDecl 0x5c19a3e793b0 <col:5, col:16> col:12 used i 'size_t':'unsigned long' cinit
// | |   `-ImplicitCastExpr 0x5c19a3e79438 <col:16> 'size_t':'unsigned long' <IntegralCast>
// | |     `-IntegerLiteral 0x5c19a3e79418 <col:16> 'int' 0
// | `-IfStmt 0x5c19a3e799a8 <line:29:5, line:38:5> has_else
// |   |-BinaryOperator 0x5c19a3e794d8 <line:29:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x5c19a3e794a8 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5c19a3e79468 <col:9> 'size_t':'unsigned long' lvalue Var 0x5c19a3e793b0 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x5c19a3e794c0 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5c19a3e79488 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x5c19a3e78f00 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x5c19a3e79920 <line:30:5, line:34:5>
// |   | |-AttributedStmt 0x5c19a3e795b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:31:30>
// |   | | |-AnnotateAttr 0x5c19a3e79538 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x5c19a3e79530 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:31:30>
// |   | |-AttributedStmt 0x5c19a3e79680 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:32:30>
// |   | | |-AnnotateAttr 0x5c19a3e79608 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x5c19a3e79600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_3.c:32:30>
// |   | `-ReturnStmt 0x5c19a3e79910 <line:33:9, col:44>
// |   |   `-ImplicitCastExpr 0x5c19a3e798f8 <col:16, col:44> 'bool' <IntegralToBoolean>
// |   |     `-ParenExpr 0x5c19a3e798d8 <col:16, col:44> 'int'
// |   |       `-BinaryOperator 0x5c19a3e798b8 <col:17, col:43> 'int' '&&'
// |   |         |-ParenExpr 0x5c19a3e79728 <col:17, col:25> 'int'
// |   |         | `-BinaryOperator 0x5c19a3e79708 <col:18, col:22> 'int' '<'
// |   |         |   |-ImplicitCastExpr 0x5c19a3e796d8 <col:18> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |   | `-DeclRefExpr 0x5c19a3e79698 <col:18> 'size_t':'unsigned long' lvalue Var 0x5c19a3e793b0 'i' 'size_t':'unsigned long'
// |   |         |   `-ImplicitCastExpr 0x5c19a3e796f0 <col:22> 'size_t':'unsigned long' <LValueToRValue>
// |   |         |     `-DeclRefExpr 0x5c19a3e796b8 <col:22> 'size_t':'unsigned long' lvalue ParmVar 0x5c19a3e78f00 'len' 'size_t':'unsigned long'
// |   |         `-BinaryOperator 0x5c19a3e79898 <col:30, col:43> 'int' '=='
// |   |           |-ImplicitCastExpr 0x5c19a3e79868 <col:30, col:34> 'int' <LValueToRValue>
// |   |           | `-ArraySubscriptExpr 0x5c19a3e797b8 <col:30, col:34> 'int' lvalue
// |   |           |   |-ImplicitCastExpr 0x5c19a3e79788 <col:30> 'int *' <LValueToRValue>
// |   |           |   | `-DeclRefExpr 0x5c19a3e79748 <col:30> 'int *' lvalue ParmVar 0x5c19a3e78c40 'a1' 'int *'
// |   |           |   `-ImplicitCastExpr 0x5c19a3e797a0 <col:33> 'size_t':'unsigned long' <LValueToRValue>
// |   |           |     `-DeclRefExpr 0x5c19a3e79768 <col:33> 'size_t':'unsigned long' lvalue Var 0x5c19a3e793b0 'i' 'size_t':'unsigned long'
// |   |           `-ImplicitCastExpr 0x5c19a3e79880 <col:39, col:43> 'int' <LValueToRValue>
// |   |             `-ArraySubscriptExpr 0x5c19a3e79848 <col:39, col:43> 'int' lvalue
// |   |               |-ImplicitCastExpr 0x5c19a3e79818 <col:39> 'int *' <LValueToRValue>
// |   |               | `-DeclRefExpr 0x5c19a3e797d8 <col:39> 'int *' lvalue ParmVar 0x5c19a3e78d88 'a2' 'int *'
// |   |               `-ImplicitCastExpr 0x5c19a3e79830 <col:42> 'size_t':'unsigned long' <LValueToRValue>
// |   |                 `-DeclRefExpr 0x5c19a3e797f8 <col:42> 'size_t':'unsigned long' lvalue Var 0x5c19a3e793b0 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x5c19a3e79990 <line:36:5, line:38:5>
// |     `-ReturnStmt 0x5c19a3e79980 <line:37:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15>
// |       `-ImplicitCastExpr 0x5c19a3e79968 <col:15> 'bool' <IntegralToBoolean>
// |         `-IntegerLiteral 0x5c19a3e79948 <col:15> 'int' 0
// |-AnnotateAttr 0x5c19a3e79100 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x5c19a3e791b0 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x5c19a3e79230 <line:5:22, col:58> pulse "returns:_Bool|END"
// |-AnnotateAttr 0x5c19a3e792a0 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x5c19a3e79320 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
