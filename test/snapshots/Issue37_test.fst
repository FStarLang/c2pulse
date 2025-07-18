module Issue37_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn set
(n : ref Int32.t)
requires n |-> 'v
ensures n |-> 0l
{
let mut n : (ref Int32.t) = n;
(! n) := 0l;
}

fn call_fib_rec
(n : Int32.t)
returns res:int32
{
let mut n : Int32.t = n;
let mut cur: Int32.t = witness #_ #_;
(set (cur));
(! cur);
}

//Dumping the Clang AST.
// FunctionDecl 0x59ed3a795b80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue37_test.c:5:1, line:6:11> line:5:6 used set 'void (int *)'
// |-ParmVarDecl 0x59ed3a795ab8 <col:10, col:15> col:15 used n 'int *'
// |-CompoundStmt 0x59ed3a795e18 <line:6:1, col:11>
// | `-BinaryOperator 0x59ed3a795df8 <col:3, col:8> 'int' '='
// |   |-UnaryOperator 0x59ed3a795dc0 <col:3, col:4> 'int' lvalue prefix '*' cannot overflow
// |   | `-ImplicitCastExpr 0x59ed3a795da8 <col:4> 'int *' <LValueToRValue>
// |   |   `-DeclRefExpr 0x59ed3a795d88 <col:4> 'int *' lvalue ParmVar 0x59ed3a795ab8 'n' 'int *'
// |   `-IntegerLiteral 0x59ed3a795dd8 <col:8> 'int' 0
// |-AnnotateAttr 0x59ed3a795c30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:"n |-> 'v"|END"
// `-AnnotateAttr 0x59ed3a795cd0 <line:4:24, col:71> pulse "ensures:n |-> 0l|END"
// FunctionDecl 0x59ed3a795f58 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue37_test.c:9:1, line:14:1> line:9:5 call_fib_rec 'int (int)'
// |-ParmVarDecl 0x59ed3a795e80 <col:19, col:23> col:23 n 'int'
// |-CompoundStmt 0x59ed3a796260 <line:10:1, line:14:1>
// | |-DeclStmt 0x59ed3a796130 <line:11:3, col:10>
// | | `-VarDecl 0x59ed3a7960c8 <col:3, col:7> col:7 used cur 'int'
// | |-CallExpr 0x59ed3a7961e8 <line:12:3, col:11> 'void'
// | | |-ImplicitCastExpr 0x59ed3a7961d0 <col:3> 'void (*)(int *)' <FunctionToPointerDecay>
// | | | `-DeclRefExpr 0x59ed3a796148 <col:3> 'void (int *)' Function 0x59ed3a795b80 'set' 'void (int *)'
// | | `-UnaryOperator 0x59ed3a796188 <col:7, col:8> 'int *' prefix '&' cannot overflow
// | |   `-DeclRefExpr 0x59ed3a796168 <col:8> 'int' lvalue Var 0x59ed3a7960c8 'cur' 'int'
// | `-ReturnStmt 0x59ed3a796250 <line:13:3, col:10>
// |   `-ImplicitCastExpr 0x59ed3a796238 <col:10> 'int' <LValueToRValue>
// |     `-DeclRefExpr 0x59ed3a796218 <col:10> 'int' lvalue Var 0x59ed3a7960c8 'cur' 'int'
// `-AnnotateAttr 0x59ed3a796008 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:res:int32|END"
