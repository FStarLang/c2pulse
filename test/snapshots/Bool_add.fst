module Bool_add

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(a : bool)
(b : bool)
returns Int32.t
{
let mut a : bool = a;
let mut b : bool = b;
(Int32.add (bool_to_int32 (! a)) (bool_to_int32 (! b)));
}

//Dumping the Clang AST.
// FunctionDecl 0x60d62d910f78 </home/t-visinghal/Applications/src/c2pulse/test/general/bool_add.c:5:1, line:7:1> line:5:5 foo 'int (bool, bool)'
// |-ParmVarDecl 0x60d62d910e20 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/general/bool_add.c:5:14> col:14 used a 'bool'
// |-ParmVarDecl 0x60d62d910e98 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/general/bool_add.c:5:22> col:22 used b 'bool'
// |-CompoundStmt 0x60d62d9111e8 <col:24, line:7:1>
// | `-ReturnStmt 0x60d62d9111d8 <line:6:2, col:13>
// |   `-BinaryOperator 0x60d62d9111b8 <col:9, col:13> 'int' '+'
// |     |-ImplicitCastExpr 0x60d62d911170 <col:9> 'int' <IntegralCast>
// |     | `-ImplicitCastExpr 0x60d62d911158 <col:9> 'bool' <LValueToRValue>
// |     |   `-DeclRefExpr 0x60d62d911118 <col:9> 'bool' lvalue ParmVar 0x60d62d910e20 'a' 'bool'
// |     `-ImplicitCastExpr 0x60d62d9111a0 <col:13> 'int' <IntegralCast>
// |       `-ImplicitCastExpr 0x60d62d911188 <col:13> 'bool' <LValueToRValue>
// |         `-DeclRefExpr 0x60d62d911138 <col:13> 'bool' lvalue ParmVar 0x60d62d910e98 'b' 'bool'
// `-AnnotateAttr 0x60d62d911030 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
