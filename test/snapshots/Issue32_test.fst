module Issue32_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn check_null
(r : ref Int32.t)
requires emp
returns i : int32
ensures emp
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (not (is_null (! r))))))
{
1l;
}
else
{
0l;
};
}

//Dumping the Clang AST.
// FunctionDecl 0x5bd2ca22e4f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue32_test.c:7:1, line:13:1> line:7:5 check_null 'int (int *)'
// |-ParmVarDecl 0x5bd2ca22e428 <col:16, col:21> col:21 used r 'int *'
// |-CompoundStmt 0x5bd2ca22e8d0 <col:24, line:13:1>
// | `-IfStmt 0x5bd2ca22e8a0 <line:8:3, line:12:3> has_else
// |   |-BinaryOperator 0x5bd2ca22e7f0 <line:8:7, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:23> 'int' '!='
// |   | |-ImplicitCastExpr 0x5bd2ca22e7c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue32_test.c:8:7> 'int *' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5bd2ca22e720 <col:7> 'int *' lvalue ParmVar 0x5bd2ca22e428 'r' 'int *'
// |   | `-ImplicitCastExpr 0x5bd2ca22e7d8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/__stddef_null.h:26:14, col:23> 'int *' <BitCast>
// |   |   `-ParenExpr 0x5bd2ca22e7a0 <col:14, col:23> 'void *'
// |   |     `-CStyleCastExpr 0x5bd2ca22e778 <col:15, col:22> 'void *' <NullToPointer>
// |   |       `-IntegerLiteral 0x5bd2ca22e740 <col:22> 'int' 0
// |   |-CompoundStmt 0x5bd2ca22e840 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue32_test.c:8:18, line:10:3>
// |   | `-ReturnStmt 0x5bd2ca22e830 <line:9:5, col:12>
// |   |   `-IntegerLiteral 0x5bd2ca22e810 <col:12> 'int' 1
// |   `-CompoundStmt 0x5bd2ca22e888 <line:10:10, line:12:3>
// |     `-ReturnStmt 0x5bd2ca22e878 <line:11:5, col:12>
// |       `-IntegerLiteral 0x5bd2ca22e858 <col:12> 'int' 0
// |-AnnotateAttr 0x5bd2ca22e5a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5bd2ca22e640 <line:5:22, col:58> pulse "returns:i : int32|END"
// `-AnnotateAttr 0x5bd2ca22e6b0 <line:4:24, col:71> pulse "ensures:emp|END"
