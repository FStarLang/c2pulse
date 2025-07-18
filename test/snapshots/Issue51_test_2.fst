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
// FunctionDecl 0x5c657d1165f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:6:1, line:8:1> line:6:5 neg 'int (uint32_t)'
// |-ParmVarDecl 0x5c657d1164e0 <col:9, col:18> col:18 used x 'uint32_t':'unsigned int'
// |-CompoundStmt 0x5c657d116880 <col:21, line:8:1>
// | `-ReturnStmt 0x5c657d116870 <line:7:5, col:13>
// |   `-ImplicitCastExpr 0x5c657d116858 <col:12, col:13> 'int' <IntegralCast>
// |     `-UnaryOperator 0x5c657d116840 <col:12, col:13> 'uint32_t':'unsigned int' prefix '-'
// |       `-ImplicitCastExpr 0x5c657d116828 <col:13> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x5c657d116808 <col:13> 'uint32_t':'unsigned int' lvalue ParmVar 0x5c657d1164e0 'x' 'uint32_t':'unsigned int'
// |-AnnotateAttr 0x5c657d1166a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// `-AnnotateAttr 0x5c657d116750 <line:13:5, col:59> pulse "expect_failure:|END"
// FunctionDecl 0x5c657d116a98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:12:1, line:14:1> line:12:6 neg_2 'long (uint64_t)'
// |-ParmVarDecl 0x5c657d116980 <col:12, col:21> col:21 used x 'uint64_t':'unsigned long'
// |-CompoundStmt 0x5c657d116cd8 <col:24, line:14:1>
// | `-ReturnStmt 0x5c657d116cc8 <line:13:5, col:13>
// |   `-ImplicitCastExpr 0x5c657d116cb0 <col:12, col:13> 'long' <IntegralCast>
// |     `-UnaryOperator 0x5c657d116c98 <col:12, col:13> 'uint64_t':'unsigned long' prefix '-'
// |       `-ImplicitCastExpr 0x5c657d116c80 <col:13> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x5c657d116c60 <col:13> 'uint64_t':'unsigned long' lvalue ParmVar 0x5c657d116980 'x' 'uint64_t':'unsigned long'
// |-AnnotateAttr 0x5c657d116b48 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// `-AnnotateAttr 0x5c657d116bf0 <line:13:5, col:59> pulse "expect_failure:|END"
// FunctionDecl 0x5c657d116e18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test_2.c:23:1, line:25:1> line:23:5 not 'int (int)'
// |-ParmVarDecl 0x5c657d116d40 <col:9, col:13> col:13 used x 'int'
// |-CompoundStmt 0x5c657d116fd0 <col:16, line:25:1>
// | `-ReturnStmt 0x5c657d116fc0 <line:24:5, col:13>
// |   `-UnaryOperator 0x5c657d116fa8 <col:12, col:13> 'int' prefix '!' cannot overflow
// |     `-ImplicitCastExpr 0x5c657d116f90 <col:13> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x5c657d116f70 <col:13> 'int' lvalue ParmVar 0x5c657d116d40 'x' 'int'
// `-AnnotateAttr 0x5c657d116ec8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
