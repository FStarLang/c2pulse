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
// FunctionDecl 0x5bebcefa0c38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:7:1, line:11:1> line:7:5 add 'int ()'
// |-CompoundStmt 0x5bebcefa1038 <col:10, line:11:1>
// | |-DeclStmt 0x5bebcefa0e68 <line:8:5, col:14>
// | | `-VarDecl 0x5bebcefa0de0 <col:5, col:13> col:9 used x 'int' cinit
// | |   `-IntegerLiteral 0x5bebcefa0e48 <col:13> 'int' 1
// | |-DeclStmt 0x5bebcefa0f20 <line:9:5, col:14>
// | | `-VarDecl 0x5bebcefa0e98 <col:5, col:13> col:9 used y 'int' cinit
// | |   `-IntegerLiteral 0x5bebcefa0f00 <col:13> 'int' 0
// | `-ReturnStmt 0x5bebcefa1028 <line:10:5, col:20>
// |   `-BinaryOperator 0x5bebcefa1008 <col:12, col:20> 'int' '+'
// |     |-UnaryOperator 0x5bebcefa0f88 <col:12, col:14> 'int' prefix '!' cannot overflow
// |     | `-UnaryOperator 0x5bebcefa0f70 <col:13, col:14> 'int' prefix '!' cannot overflow
// |     |   `-ImplicitCastExpr 0x5bebcefa0f58 <col:14> 'int' <LValueToRValue>
// |     |     `-DeclRefExpr 0x5bebcefa0f38 <col:14> 'int' lvalue Var 0x5bebcefa0de0 'x' 'int'
// |     `-UnaryOperator 0x5bebcefa0ff0 <col:18, col:20> 'int' prefix '!' cannot overflow
// |       `-UnaryOperator 0x5bebcefa0fd8 <col:19, col:20> 'int' prefix '!' cannot overflow
// |         `-ImplicitCastExpr 0x5bebcefa0fc0 <col:20> 'int' <LValueToRValue>
// |           `-DeclRefExpr 0x5bebcefa0fa0 <col:20> 'int' lvalue Var 0x5bebcefa0e98 'y' 'int'
// `-AnnotateAttr 0x5bebcefa0ce0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x5bebcef04db8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:1, line:30:1> line:18:5 compare_elt 'int (int *, int *, size_t, size_t)'
// |-ParmVarDecl 0x5bebcefa1238 <col:29, col:34> col:34 used a1 'int *'
// | `-AnnotateAttr 0x5bebcefa12a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5bebcefa1388 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:50, col:55> col:55 used a2 'int *'
// | `-AnnotateAttr 0x5bebcefa13f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:6:24, col:60> pulse "array:len|END"
// |-ParmVarDecl 0x5bebcefa1500 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:18:59, col:66> col:66 used i 'size_t':'unsigned long'
// |-ParmVarDecl 0x5bebcef04c60 <col:69, col:76> col:76 used len 'size_t':'unsigned long'
// |-CompoundStmt 0x5bebcef055a0 <line:19:1, line:30:1>
// | `-IfStmt 0x5bebcef05570 <line:20:5, line:29:5> has_else
// |   |-BinaryOperator 0x5bebcef05190 <line:20:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x5bebcef05160 <col:9> 'size_t':'unsigned long' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5bebcef05120 <col:9> 'size_t':'unsigned long' lvalue ParmVar 0x5bebcefa1500 'i' 'size_t':'unsigned long'
// |   | `-ImplicitCastExpr 0x5bebcef05178 <col:13> 'size_t':'unsigned long' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5bebcef05140 <col:13> 'size_t':'unsigned long' lvalue ParmVar 0x5bebcef04c60 'len' 'size_t':'unsigned long'
// |   |-CompoundStmt 0x5bebcef054e8 <line:21:5, line:25:5>
// |   | |-AttributedStmt 0x5bebcef05260 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:22:30>
// |   | | |-AnnotateAttr 0x5bebcef051f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a1|END"
// |   | | `-NullStmt 0x5bebcef051e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:22:30>
// |   | |-AttributedStmt 0x5bebcef05330 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:23:30>
// |   | | |-AnnotateAttr 0x5bebcef052b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:pts_to_len !a2|END"
// |   | | `-NullStmt 0x5bebcef052b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue54_test_2.c:23:30>
// |   | `-ReturnStmt 0x5bebcef054d8 <line:24:9, col:31>
// |   |   `-ParenExpr 0x5bebcef054b8 <col:16, col:31> 'int'
// |   |     `-BinaryOperator 0x5bebcef05498 <col:17, col:30> 'int' '=='
// |   |       |-ImplicitCastExpr 0x5bebcef05468 <col:17, col:21> 'int' <LValueToRValue>
// |   |       | `-ArraySubscriptExpr 0x5bebcef053b8 <col:17, col:21> 'int' lvalue
// |   |       |   |-ImplicitCastExpr 0x5bebcef05388 <col:17> 'int *' <LValueToRValue>
// |   |       |   | `-DeclRefExpr 0x5bebcef05348 <col:17> 'int *' lvalue ParmVar 0x5bebcefa1238 'a1' 'int *'
// |   |       |   `-ImplicitCastExpr 0x5bebcef053a0 <col:20> 'size_t':'unsigned long' <LValueToRValue>
// |   |       |     `-DeclRefExpr 0x5bebcef05368 <col:20> 'size_t':'unsigned long' lvalue ParmVar 0x5bebcefa1500 'i' 'size_t':'unsigned long'
// |   |       `-ImplicitCastExpr 0x5bebcef05480 <col:26, col:30> 'int' <LValueToRValue>
// |   |         `-ArraySubscriptExpr 0x5bebcef05448 <col:26, col:30> 'int' lvalue
// |   |           |-ImplicitCastExpr 0x5bebcef05418 <col:26> 'int *' <LValueToRValue>
// |   |           | `-DeclRefExpr 0x5bebcef053d8 <col:26> 'int *' lvalue ParmVar 0x5bebcefa1388 'a2' 'int *'
// |   |           `-ImplicitCastExpr 0x5bebcef05430 <col:29> 'size_t':'unsigned long' <LValueToRValue>
// |   |             `-DeclRefExpr 0x5bebcef053f8 <col:29> 'size_t':'unsigned long' lvalue ParmVar 0x5bebcefa1500 'i' 'size_t':'unsigned long'
// |   `-CompoundStmt 0x5bebcef05558 <line:27:5, line:29:5>
// |     `-ReturnStmt 0x5bebcef05548 <line:28:9, col:16>
// |       `-ImplicitCastExpr 0x5bebcef05530 <col:16> 'int' <IntegralCast>
// |         `-IntegerLiteral 0x5bebcef05510 <col:16> 'long' 0
// |-AnnotateAttr 0x5bebcef04e80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:exists* v. (a1 |-> v)|END"
// |-AnnotateAttr 0x5bebcef04f30 <col:23, col:61> pulse "requires:exists* v. (a2 |-> v)|END"
// |-AnnotateAttr 0x5bebcef04fb0 <line:5:22, col:58> pulse "returns:Int32.t|END"
// |-AnnotateAttr 0x5bebcef05020 <line:4:24, col:71> pulse "ensures:exists* v. (a1 |-> v)|END"
// `-AnnotateAttr 0x5bebcef050a0 <col:24, col:71> pulse "ensures:exists* v. (a2 |-> v)|END"
