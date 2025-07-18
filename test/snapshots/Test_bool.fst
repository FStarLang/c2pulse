module Test_bool

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_bool
(a : Int32.t)
(b : Int32.t)
returns b:_Bool
{
let mut a : Int32.t = a;
let mut b : Int32.t = b;
if((int32_to_bool (bool_to_int32 (Int32.lt (! a) (! b)))))
{
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}

//Dumping the Clang AST.
// FunctionDecl 0x5972a10e4b70 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/general/test_bool.c:17:1> line:7:6 test_bool 'bool (int, int)'
// |-ParmVarDecl 0x5972a10e4a18 <col:16, col:20> col:20 used a 'int'
// |-ParmVarDecl 0x5972a10e4a98 <col:23, col:27> col:27 used b 'int'
// |-CompoundStmt 0x5972a10e4e98 <line:8:1, line:17:1>
// | `-IfStmt 0x5972a10e4e68 <line:9:5, line:16:5> has_else
// |   |-BinaryOperator 0x5972a10e4d88 <line:9:9, col:13> 'int' '<'
// |   | |-ImplicitCastExpr 0x5972a10e4d58 <col:9> 'int' <LValueToRValue>
// |   | | `-DeclRefExpr 0x5972a10e4d18 <col:9> 'int' lvalue ParmVar 0x5972a10e4a18 'a' 'int'
// |   | `-ImplicitCastExpr 0x5972a10e4d70 <col:13> 'int' <LValueToRValue>
// |   |   `-DeclRefExpr 0x5972a10e4d38 <col:13> 'int' lvalue ParmVar 0x5972a10e4a98 'b' 'int'
// |   |-CompoundStmt 0x5972a10e4df0 <line:10:5, line:12:5>
// |   | `-ReturnStmt 0x5972a10e4de0 <line:11:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:25:14>
// |   |   `-ImplicitCastExpr 0x5972a10e4dc8 <col:14> 'bool' <IntegralToBoolean>
// |   |     `-IntegerLiteral 0x5972a10e4da8 <col:14> 'int' 1
// |   `-CompoundStmt 0x5972a10e4e50 </home/t-visinghal/Applications/src/c2pulse/test/general/test_bool.c:14:5, line:16:5>
// |     `-ReturnStmt 0x5972a10e4e40 <line:15:9, /home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:26:15>
// |       `-ImplicitCastExpr 0x5972a10e4e28 <col:15> 'bool' <IntegralToBoolean>
// |         `-IntegerLiteral 0x5972a10e4e08 <col:15> 'int' 0
// `-AnnotateAttr 0x5972a10e4c28 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:5:22, col:58> pulse "returns:b:_Bool|END"
