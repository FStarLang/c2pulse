module Issue51_test_2

#lang-pulse

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn neg
(x : UInt32.t)
returns Int32.t
{
let mut x : UInt32.t = x;
(uint32_to_int32 (UInt32.sub 0ul (! x)));
}

[@@expect_failure]
fn neg_2
(x : UInt64.t)
returns Int64.t
{
let mut x : UInt64.t = x;
(uint64_to_int64 (UInt64.sub 0UL (! x)));
}

fn not
(x : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
(bool_to_int32 (not (int32_to_bool (! x))));
}

//Dumping the Clang AST.
// FunctionDecl 0x614c3593e5f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:6:1, line:8:1> line:6:5 neg 'int (uint32_t)'
// |-ParmVarDecl 0x614c3593e4e0 <col:9, col:18> col:18 used x 'uint32_t':'unsigned int'
// |-CompoundStmt 0x614c3593e880 <col:21, line:8:1>
// | `-ReturnStmt 0x614c3593e870 <line:7:5, col:13>
// |   `-ImplicitCastExpr 0x614c3593e858 <col:12, col:13> 'int' <IntegralCast>
// |     `-UnaryOperator 0x614c3593e840 <col:12, col:13> 'uint32_t':'unsigned int' prefix '-'
// |       `-ImplicitCastExpr 0x614c3593e828 <col:13> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x614c3593e808 <col:13> 'uint32_t':'unsigned int' lvalue ParmVar 0x614c3593e4e0 'x' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x614c3593e6a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// `-AnnotateAttr 0x614c3593e750 <line:13:5, col:59> pulse "expect_failure:|END"
// FunctionDecl 0x614c3593ea98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:12:1, line:14:1> line:12:6 neg_2 'long (uint64_t)'
// |-ParmVarDecl 0x614c3593e980 <col:12, col:21> col:21 used x 'uint64_t':'unsigned long'
// |-CompoundStmt 0x614c3593ecd8 <col:24, line:14:1>
// | `-ReturnStmt 0x614c3593ecc8 <line:13:5, col:13>
// |   `-ImplicitCastExpr 0x614c3593ecb0 <col:12, col:13> 'long' <IntegralCast>
// |     `-UnaryOperator 0x614c3593ec98 <col:12, col:13> 'uint64_t':'unsigned long' prefix '-'
// |       `-ImplicitCastExpr 0x614c3593ec80 <col:13> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x614c3593ec60 <col:13> 'uint64_t':'unsigned long' lvalue ParmVar 0x614c3593e980 'x' 'uint64_t':'unsigned long'
// |-AnnotateAttr 0x614c3593eb48 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// `-AnnotateAttr 0x614c3593ebf0 <line:13:5, col:59> pulse "expect_failure:|END"
// FunctionDecl 0x614c3593ee18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:23:1, line:25:1> line:23:5 not 'int (int)'
// |-ParmVarDecl 0x614c3593ed40 <col:9, col:13> col:13 used x 'int'
// |-CompoundStmt 0x614c3593efd0 <col:16, line:25:1>
// | `-ReturnStmt 0x614c3593efc0 <line:24:5, col:13>
// |   `-UnaryOperator 0x614c3593efa8 <col:12, col:13> 'int' prefix '!' cannot overflow
// |     `-ImplicitCastExpr 0x614c3593ef90 <col:13> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x614c3593ef70 <col:13> 'int' lvalue ParmVar 0x614c3593ed40 'x' 'int'
// `-AnnotateAttr 0x614c3593eec8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
