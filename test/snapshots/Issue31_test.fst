module Issue31_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn check_null
(r : ref Int32.t)
requires emp
returns i:int32
ensures emp
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
0l;
}
else
{
1l;
};
}

//Dumping the Clang AST.
// FunctionDecl 0x5bb1fe7ff4f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue31_test.c:7:1, line:17:1> line:7:5 check_null 'int (int *)'
// |-ParmVarDecl 0x5bb1fe7ff428 <col:16, col:21> col:21 used r 'int *'
// |-CompoundStmt 0x5bb1fe7ff8d0 <line:8:1, line:17:1>
// | `-IfStmt 0x5bb1fe7ff8a0 <line:9:3, line:16:3> has_else
// |   |-BinaryOperator 0x5bb1fe7ff7f0 <line:9:7, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> 'int' '=='
// |   | |-ImplicitCastExpr 0x5bb1fe7ff7c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue31_test.c:9:7> 'int *' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5bb1fe7ff720 <col:7> 'int *' lvalue ParmVar 0x5bb1fe7ff428 'r' 'int *'
// |   | `-ImplicitCastExpr 0x5bb1fe7ff7d8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <BitCast>
// |   |   `-ParenExpr 0x5bb1fe7ff7a0 <col:14, col:23> 'void *'
// |   |     `-CStyleCastExpr 0x5bb1fe7ff778 <col:15, col:22> 'void *' <NullToPointer>
// |   |       `-IntegerLiteral 0x5bb1fe7ff740 <col:22> 'int' 0
// |   |-CompoundStmt 0x5bb1fe7ff840 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue31_test.c:10:3, line:12:3>
// |   | `-ReturnStmt 0x5bb1fe7ff830 <line:11:5, col:12>
// |   |   `-IntegerLiteral 0x5bb1fe7ff810 <col:12> 'int' 0
// |   `-CompoundStmt 0x5bb1fe7ff888 <line:14:3, line:16:3>
// |     `-ReturnStmt 0x5bb1fe7ff878 <line:15:5, col:12>
// |       `-IntegerLiteral 0x5bb1fe7ff858 <col:12> 'int' 1
// |-AnnotateAttr 0x5bb1fe7ff5a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5bb1fe7ff640 <line:5:22, col:58> pulse "returns:i:int32|END"
// `-AnnotateAttr 0x5bb1fe7ff6b0 <line:4:24, col:71> pulse "ensures:emp|END"
