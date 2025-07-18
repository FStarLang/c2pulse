module Issue54_test_2

#lang-pulse

open Pulse
open Pulse.Lib.C



fn add ()
returns Int32.t
{
let x0 : Int32.t = 1l;
let mut x : Int32.t = x0;
let y1 : Int32.t = 0l;
let mut y : Int32.t = y1;
(Int32.add (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! x))))))) (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! y))))))));
}

fn compare_elt
(a1 : array Int32.t)
(a2 : array Int32.t)
(i : SizeT.t)
(len : SizeT.t)
requires pure (length a2 == SizeT.v len)
requires pure (length a1 == SizeT.v len)
requires exists* v. (a1 |-> v)
requires exists* v. (a2 |-> v)
returns Int32.t
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
((bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i)))));
}
else
{
(int64_to_int32 0L);
};
}

//Dumping the Clang AST.
// FunctionDecl 0x599e1d750c38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:7:1, line:11:1> line:7:5 add 'int ()'
// |-CompoundStmt 0x599e1d751038 <col:10, line:11:1>
// | |-DeclStmt 0x599e1d750e68 <line:8:5, col:14>
// | | `-VarDecl 0x599e1d750de0 <col:5, col:13> col:9 used x 'int' cinit
// | |   `-IntegerLiteral 0x599e1d750e48 <col:13> 'int' 1
// | |-DeclStmt 0x599e1d750f20 <line:9:5, col:14>
// | | `-VarDecl 0x599e1d750e98 <col:5, col:13> col:9 used y 'int' cinit
// | |   `-IntegerLiteral 0x599e1d750f00 <col:13> 'int' 0
// | `-ReturnStmt 0x599e1d751028 <line:10:5, col:20>
// |   `-BinaryOperator 0x599e1d751008 <col:12, col:20> 'int' '+'
// |     |-UnaryOperator 0x599e1d750f88 <col:12, col:14> 'int' prefix '!' cannot overflow
// |     | `-UnaryOperator 0x599e1d750f70 <col:13, col:14> 'int' prefix '!' cannot overflow
// |     |   `-ImplicitCastExpr 0x599e1d750f58 <col:14> 'int' <LValueToRValue>
// |     |     `-DeclRefExpr 0x599e1d750f38 <col:14> 'int' lvalue Var 0x599e1d750de0 'x' 'int'
// |     `-UnaryOperator 0x599e1d750ff0 <col:18, col:20> 'int' prefix '!' cannot overflow
// |       `-UnaryOperator 0x599e1d750fd8 <col:19, col:20> 'int' prefix '!' cannot overflow
// |         `-ImplicitCastExpr 0x599e1d750fc0 <col:20> 'int' <LValueToRValue>
// |           `-DeclRefExpr 0x599e1d750fa0 <col:20> 'int' lvalue Var 0x599e1d750e98 'y' 'int'
// `-AnnotateAttr 0x599e1d750ce0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x599e1d6b4db8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:1, line:30:1> line:18:5 compare_elt 'int (int *, int *, size_t, size_t)'
// |-ParmVarDecl 0x599e1d751238 <col:29, col:34> col:34 used a1 'int *'
// | `-AnnotateAttr 0x599e1d7512a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x599e1d751388 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:50, col:55> col:55 used a2 'int *'
// | `-AnnotateAttr 0x599e1d7513f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x599e1d751500 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:59, col:66> col:66 used i 'size_t':'unsigned long'
// |-ParmVarDecl 0x599e1d6b4c60 <col:69, col:76> col:76 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x599e1d6b55a0 <line:19:1, line:30:1>
// | `-IfStmt 0x599e1d6b5570 <line:20:5, line:29:5> has_else
// |   |-BinaryOperator 0x599e1d6b5190 <line:20:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x599e1d6b5160 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x599e1d6b5120 <col:9> 'size_t':'unsigned long' lvalue ParmVar 0x599e1d751500 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x599e1d6b5178 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x599e1d6b5140 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x599e1d6b4c60 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x599e1d6b54e8 <line:21:5, line:25:5>
// |   | |-AttributedStmt 0x599e1d6b5260 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:22:30>
// |   | | |-AnnotateAttr 0x599e1d6b51f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x599e1d6b51e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:22:30>
// |   | |-AttributedStmt 0x599e1d6b5330 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:23:30>
// |   | | |-AnnotateAttr 0x599e1d6b52b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x599e1d6b52b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:23:30>
// |   | `-ReturnStmt 0x599e1d6b54d8 <line:24:9, col:31>
// |   |   `-ParenExpr 0x599e1d6b54b8 <col:16, col:31> 'int'
// |   |     `-BinaryOperator 0x599e1d6b5498 <col:17, col:30> 'int' '=='
// |   |       |-ImplicitCastExpr 0x599e1d6b5468 <col:17, col:21> 'int' <LValueToRValue>
// |   |       | `-ArraySubscriptExpr 0x599e1d6b53b8 <col:17, col:21> 'int' lvalue
// |   |       |   |-ImplicitCastExpr 0x599e1d6b5388 <col:17> 'int *' <LValueToRValue>
// |   |       |   | `-DeclRefExpr 0x599e1d6b5348 <col:17> 'int *' lvalue ParmVar 0x599e1d751238 'a1' 'int *'
// |   |       |   `-ImplicitCastExpr 0x599e1d6b53a0 <col:20> 'size_t':'unsigned long' <LValueToRValue>
// |   |       |     `-DeclRefExpr 0x599e1d6b5368 <col:20> 'size_t':'unsigned long' lvalue ParmVar 0x599e1d751500 'i' 'size_t':'unsigned long'
// |   |       `-ImplicitCastExpr 0x599e1d6b5480 <col:26, col:30> 'int' <LValueToRValue>
// |   |         `-ArraySubscriptExpr 0x599e1d6b5448 <col:26, col:30> 'int' lvalue
// |   |           |-ImplicitCastExpr 0x599e1d6b5418 <col:26> 'int *' <LValueToRValue>
// |   |           | `-DeclRefExpr 0x599e1d6b53d8 <col:26> 'int *' lvalue ParmVar 0x599e1d751388 'a2' 'int *'
// |   |           `-ImplicitCastExpr 0x599e1d6b5430 <col:29> 'size_t':'unsigned long' <LValueToRValue>
// |   |             `-DeclRefExpr 0x599e1d6b53f8 <col:29> 'size_t':'unsigned long' lvalue ParmVar 0x599e1d751500 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x599e1d6b5558 <line:27:5, line:29:5>
// |     `-ReturnStmt 0x599e1d6b5548 <line:28:9, col:16>
// |       `-ImplicitCastExpr 0x599e1d6b5530 <col:16> 'int' <IntegralCast>
// |         `-IntegerLiteral 0x599e1d6b5510 <col:16> 'long' 0
// |-AnnotateAttr 0x599e1d6b4e80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x599e1d6b4f30 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x599e1d6b4fb0 <line:5:22, col:58> pulse "returns:Int32.t|END"
// |-AnnotateAttr 0x599e1d6b5020 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x599e1d6b50a0 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
