module Issue51_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn cmp1
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.eq (! x) (! y)));
}

fn cmp2
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.gte (! x) (! y)));
}

fn cmp3
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.gt (! x) (! y)));
}

fn cmp4
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.lte (! x) (! y)));
}

fn cmp5
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.lt (! x) (! y)));
}

fn cmp6
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (not (Int32.eq (! x) (! y))));
}

fn cmp7
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (Int64.eq (! x) (! y))));
}

fn cmp8
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (Int64.gte (! x) (! y))));
}

fn cmp9
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (Int64.gt (! x) (! y))));
}

fn cmp10
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (Int64.lte (! x) (! y))));
}

fn cmp11
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (Int64.lt (! x) (! y))));
}

fn cmp12
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
let mut x : Int64.t = x;
let mut y : Int64.t = y;
(int32_to_int64 (bool_to_int32 (not (Int64.eq (! x) (! y)))));
}

fn cmp13
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.eq (! x) (! y)));
}

fn cmp14
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.gte (! x) (! y)));
}

fn cmp15
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.gt (! x) (! y)));
}

fn cmp16
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.lte (! x) (! y)));
}

fn cmp17
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (Int32.lt (! x) (! y)));
}

fn cmp18
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
(bool_to_int32 (not (Int32.eq (! x) (! y))));
}

fn cmp19
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (Int32.eq (int16_to_int32 (! x)) (int16_to_int32 (! y)))));
}

fn cmp20
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (Int32.gte (int16_to_int32 (! x)) (int16_to_int32 (! y)))));
}

fn cmp21
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (Int32.gt (int16_to_int32 (! x)) (int16_to_int32 (! y)))));
}

fn cmp22
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (Int32.lte (int16_to_int32 (! x)) (int16_to_int32 (! y)))));
}

fn cmp23
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (Int32.lt (int16_to_int32 (! x)) (int16_to_int32 (! y)))));
}

fn cmp24
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
let mut x : Int16.t = x;
let mut y : Int16.t = y;
(int32_to_int16 (bool_to_int32 (not (Int32.eq (int16_to_int32 (! x)) (int16_to_int32 (! y))))));
}

fn cmp25
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (Int32.eq (int8_to_int32 (! x)) (int8_to_int32 (! y)))));
}

fn cmp26
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (Int32.gte (int8_to_int32 (! x)) (int8_to_int32 (! y)))));
}

fn cmp27
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (Int32.gt (int8_to_int32 (! x)) (int8_to_int32 (! y)))));
}

fn cmp28
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (Int32.lte (int8_to_int32 (! x)) (int8_to_int32 (! y)))));
}

fn cmp29
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (Int32.lt (int8_to_int32 (! x)) (int8_to_int32 (! y)))));
}

fn cmp30
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
let mut x : Int8.t = x;
let mut y : Int8.t = y;
(int32_to_int8 (bool_to_int32 (not (Int32.eq (int8_to_int32 (! x)) (int8_to_int32 (! y))))));
}

fn cmp31
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (UInt64.eq (! x) (! y))));
}

fn cmp32
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (UInt64.gte (! x) (! y))));
}

fn cmp33
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (UInt64.gt (! x) (! y))));
}

fn cmp34
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (UInt64.lte (! x) (! y))));
}

fn cmp35
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (UInt64.lt (! x) (! y))));
}

fn cmp36
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
let mut x : UInt64.t = x;
let mut y : UInt64.t = y;
(int32_to_uint64 (bool_to_int32 (not (UInt64.eq (! x) (! y)))));
}

fn cmp37
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (UInt32.eq (! x) (! y))));
}

fn cmp38
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (UInt32.gte (! x) (! y))));
}

fn cmp39
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (UInt32.gt (! x) (! y))));
}

fn cmp40
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (UInt32.lte (! x) (! y))));
}

fn cmp41
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (UInt32.lt (! x) (! y))));
}

fn cmp42
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
let mut x : UInt32.t = x;
let mut y : UInt32.t = y;
(int32_to_uint32 (bool_to_int32 (not (UInt32.eq (! x) (! y)))));
}

fn cmp43
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (Int32.eq (uint16_to_int32 (! x)) (uint16_to_int32 (! y)))));
}

fn cmp44
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (Int32.gte (uint16_to_int32 (! x)) (uint16_to_int32 (! y)))));
}

fn cmp45
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (Int32.gt (uint16_to_int32 (! x)) (uint16_to_int32 (! y)))));
}

fn cmp46
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (Int32.lte (uint16_to_int32 (! x)) (uint16_to_int32 (! y)))));
}

fn cmp47
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (Int32.lt (uint16_to_int32 (! x)) (uint16_to_int32 (! y)))));
}

fn cmp48
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
let mut x : UInt16.t = x;
let mut y : UInt16.t = y;
(int32_to_uint16 (bool_to_int32 (not (Int32.eq (uint16_to_int32 (! x)) (uint16_to_int32 (! y))))));
}

fn cmp49
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (Int32.eq (uint8_to_int32 (! x)) (uint8_to_int32 (! y)))));
}

fn cmp50
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (Int32.gte (uint8_to_int32 (! x)) (uint8_to_int32 (! y)))));
}

fn cmp51
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (Int32.gt (uint8_to_int32 (! x)) (uint8_to_int32 (! y)))));
}

fn cmp52
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (Int32.lte (uint8_to_int32 (! x)) (uint8_to_int32 (! y)))));
}

fn cmp53
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (Int32.lt (uint8_to_int32 (! x)) (uint8_to_int32 (! y)))));
}

fn cmp54
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
let mut x : UInt8.t = x;
let mut y : UInt8.t = y;
(int32_to_uint8 (bool_to_int32 (not (Int32.eq (uint8_to_int32 (! x)) (uint8_to_int32 (! y))))));
}

fn cmp55
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (Int32.eq (bool_to_int32 (! x)) (bool_to_int32 (! y)))));
}

fn cmp56
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (Int32.gte (bool_to_int32 (! x)) (bool_to_int32 (! y)))));
}

fn cmp57
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (Int32.gt (bool_to_int32 (! x)) (bool_to_int32 (! y)))));
}

fn cmp58
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (Int32.lte (bool_to_int32 (! x)) (bool_to_int32 (! y)))));
}

fn cmp59
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (Int32.lt (bool_to_int32 (! x)) (bool_to_int32 (! y)))));
}

fn cmp60
(x : bool)
(y : bool)
returns _Bool
{
let mut x : bool = x;
let mut y : bool = y;
(int32_to_bool (bool_to_int32 (not (Int32.eq (bool_to_int32 (! x)) (bool_to_int32 (! y))))));
}

//Dumping the Clang AST.
// FunctionDecl 0x63e625ccd9d8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:5:18, col:58> col:22 cmp1 'int (int, int)'
// |-ParmVarDecl 0x63e625ccd878 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625ccd8f8 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625ccdc18 <col:41, col:58>
// | `-ReturnStmt 0x63e625ccdc08 <col:43, col:55>
// |   `-BinaryOperator 0x63e625ccdbe8 <col:50, col:55> 'int' '=='
// |     |-ImplicitCastExpr 0x63e625ccdbb8 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625ccdb78 <col:50> 'int' lvalue ParmVar 0x63e625ccd878 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625ccdbd0 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625ccdb98 <col:55> 'int' lvalue ParmVar 0x63e625ccd8f8 'y' 'int'
// `-AnnotateAttr 0x63e625ccda90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625ccdda0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:6:18, col:58> col:22 cmp2 'int (int, int)'
// |-ParmVarDecl 0x63e625ccdc80 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625ccdd00 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625ccdfa0 <col:41, col:58>
// | `-ReturnStmt 0x63e625ccdf90 <col:43, col:55>
// |   `-BinaryOperator 0x63e625ccdf70 <col:50, col:55> 'int' '>='
// |     |-ImplicitCastExpr 0x63e625ccdf40 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625ccdf00 <col:50> 'int' lvalue ParmVar 0x63e625ccdc80 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625ccdf58 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625ccdf20 <col:55> 'int' lvalue ParmVar 0x63e625ccdd00 'y' 'int'
// `-AnnotateAttr 0x63e625ccde58 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cce128 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:7:18, col:57> col:22 cmp3 'int (int, int)'
// |-ParmVarDecl 0x63e625cce008 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625cce088 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625cce320 <col:41, col:57>
// | `-ReturnStmt 0x63e625cce310 <col:43, col:54>
// |   `-BinaryOperator 0x63e625cce2f0 <col:50, col:54> 'int' '>'
// |     |-ImplicitCastExpr 0x63e625cce2c0 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cce280 <col:50> 'int' lvalue ParmVar 0x63e625cce008 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625cce2d8 <col:54> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cce2a0 <col:54> 'int' lvalue ParmVar 0x63e625cce088 'y' 'int'
// `-AnnotateAttr 0x63e625cce1e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cce4a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:8:18, col:58> col:22 cmp4 'int (int, int)'
// |-ParmVarDecl 0x63e625cce388 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625cce408 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625cce6a0 <col:41, col:58>
// | `-ReturnStmt 0x63e625cce690 <col:43, col:55>
// |   `-BinaryOperator 0x63e625cce670 <col:50, col:55> 'int' '<='
// |     |-ImplicitCastExpr 0x63e625cce640 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cce600 <col:50> 'int' lvalue ParmVar 0x63e625cce388 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625cce658 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cce620 <col:55> 'int' lvalue ParmVar 0x63e625cce408 'y' 'int'
// `-AnnotateAttr 0x63e625cce560 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cd5f38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:9:18, col:57> col:22 cmp5 'int (int, int)'
// |-ParmVarDecl 0x63e625cd5e18 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625cd5e98 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625cd6130 <col:41, col:57>
// | `-ReturnStmt 0x63e625cd6120 <col:43, col:54>
// |   `-BinaryOperator 0x63e625cd6100 <col:50, col:54> 'int' '<'
// |     |-ImplicitCastExpr 0x63e625cd60d0 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cd6090 <col:50> 'int' lvalue ParmVar 0x63e625cd5e18 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625cd60e8 <col:54> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cd60b0 <col:54> 'int' lvalue ParmVar 0x63e625cd5e98 'y' 'int'
// `-AnnotateAttr 0x63e625cd5ff0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cd62b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:10:18, col:58> col:22 cmp6 'int (int, int)'
// |-ParmVarDecl 0x63e625cd6198 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x63e625cd6218 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x63e625cd64b0 <col:41, col:58>
// | `-ReturnStmt 0x63e625cd64a0 <col:43, col:55>
// |   `-BinaryOperator 0x63e625cd6480 <col:50, col:55> 'int' '!='
// |     |-ImplicitCastExpr 0x63e625cd6450 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cd6410 <col:50> 'int' lvalue ParmVar 0x63e625cd6198 'x' 'int'
// |     `-ImplicitCastExpr 0x63e625cd6468 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cd6430 <col:55> 'int' lvalue ParmVar 0x63e625cd6218 'y' 'int'
// `-AnnotateAttr 0x63e625cd6370 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cd6700 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:12:18, col:70> col:26 cmp7 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cd6570 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cd65e8 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cd6918 <col:53, col:70>
// | `-ReturnStmt 0x63e625cd6908 <col:55, col:67>
// |   `-ImplicitCastExpr 0x63e625cd68f0 <col:62, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd68d0 <col:62, col:67> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625cd68a0 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cd6860 <col:62> 'int64_t':'long' lvalue ParmVar 0x63e625cd6570 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cd68b8 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cd6880 <col:67> 'int64_t':'long' lvalue ParmVar 0x63e625cd65e8 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cd67b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cd6a88 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:13:18, col:70> col:26 cmp8 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cd6978 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cd69f0 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cd6c98 <col:53, col:70>
// | `-ReturnStmt 0x63e625cd6c88 <col:55, col:67>
// |   `-ImplicitCastExpr 0x63e625cd6c70 <col:62, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd6c50 <col:62, col:67> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625cd6c20 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cd6be0 <col:62> 'int64_t':'long' lvalue ParmVar 0x63e625cd6978 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cd6c38 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cd6c00 <col:67> 'int64_t':'long' lvalue ParmVar 0x63e625cd69f0 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cd6b40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cbca40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:14:18, col:69> col:26 cmp9 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cd6cf8 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cd6d70 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cbcc58 <col:53, col:69>
// | `-ReturnStmt 0x63e625cbcc48 <col:55, col:66>
// |   `-ImplicitCastExpr 0x63e625cbcc30 <col:62, col:66> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cbcc10 <col:62, col:66> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625cbcbe0 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cbcba0 <col:62> 'int64_t':'long' lvalue ParmVar 0x63e625cd6cf8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cbcbf8 <col:66> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cbcbc0 <col:66> 'int64_t':'long' lvalue ParmVar 0x63e625cd6d70 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cbcaf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cbcdc8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:15:18, col:71> col:26 cmp10 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cbccb8 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cbcd30 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cbcfd8 <col:54, col:71>
// | `-ReturnStmt 0x63e625cbcfc8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cbcfb0 <col:63, col:68> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cbcf90 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625cbcf60 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cbcf20 <col:63> 'int64_t':'long' lvalue ParmVar 0x63e625cbccb8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cbcf78 <col:68> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cbcf40 <col:68> 'int64_t':'long' lvalue ParmVar 0x63e625cbcd30 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cbce80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cbd148 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:16:18, col:70> col:26 cmp11 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cbd038 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cbd0b0 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cbd358 <col:54, col:70>
// | `-ReturnStmt 0x63e625cbd348 <col:56, col:67>
// |   `-ImplicitCastExpr 0x63e625cbd330 <col:63, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cbd310 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625cbd2e0 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cbd2a0 <col:63> 'int64_t':'long' lvalue ParmVar 0x63e625cbd038 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cbd2f8 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cbd2c0 <col:67> 'int64_t':'long' lvalue ParmVar 0x63e625cbd0b0 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cbd200 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cbd4c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:17:18, col:71> col:26 cmp12 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x63e625cbd3b8 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x63e625cbd430 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x63e625cbd6d8 <col:54, col:71>
// | `-ReturnStmt 0x63e625cbd6c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cbd6b0 <col:63, col:68> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cbd690 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625cbd660 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cbd620 <col:63> 'int64_t':'long' lvalue ParmVar 0x63e625cbd3b8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x63e625cbd678 <col:68> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cbd640 <col:68> 'int64_t':'long' lvalue ParmVar 0x63e625cbd430 'y' 'int64_t':'long'
// `-AnnotateAttr 0x63e625cbd580 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x63e625cbd8f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:19:18, col:71> col:26 cmp13 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbd7a0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbd818 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cbdb10 <col:54, col:71>
// | `-ReturnStmt 0x63e625cbdb00 <col:56, col:68>
// |   `-BinaryOperator 0x63e625cbdae0 <col:63, col:68> 'int' '=='
// |     |-ImplicitCastExpr 0x63e625cbdab0 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cbda70 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbd7a0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cbdac8 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cbda90 <col:68> 'int32_t':'int' lvalue ParmVar 0x63e625cbd818 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cbd9a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cbdc80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:20:18, col:71> col:26 cmp14 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbdb70 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbdbe8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cbde80 <col:54, col:71>
// | `-ReturnStmt 0x63e625cbde70 <col:56, col:68>
// |   `-BinaryOperator 0x63e625cbde50 <col:63, col:68> 'int' '>='
// |     |-ImplicitCastExpr 0x63e625cbde20 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cbdde0 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbdb70 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cbde38 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cbde00 <col:68> 'int32_t':'int' lvalue ParmVar 0x63e625cbdbe8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cbdd38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cbdff0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:21:18, col:70> col:26 cmp15 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbdee0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbdf58 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cbe1f0 <col:54, col:70>
// | `-ReturnStmt 0x63e625cbe1e0 <col:56, col:67>
// |   `-BinaryOperator 0x63e625cbe1c0 <col:63, col:67> 'int' '>'
// |     |-ImplicitCastExpr 0x63e625cbe190 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cbe150 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbdee0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cbe1a8 <col:67> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cbe170 <col:67> 'int32_t':'int' lvalue ParmVar 0x63e625cbdf58 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cbe0a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cbe360 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:22:18, col:71> col:26 cmp16 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbe250 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbe2c8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cbe560 <col:54, col:71>
// | `-ReturnStmt 0x63e625cbe550 <col:56, col:68>
// |   `-BinaryOperator 0x63e625cbe530 <col:63, col:68> 'int' '<='
// |     |-ImplicitCastExpr 0x63e625cbe500 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cbe4c0 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbe250 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cbe518 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cbe4e0 <col:68> 'int32_t':'int' lvalue ParmVar 0x63e625cbe2c8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cbe418 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cbe6d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:23:18, col:70> col:26 cmp17 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbe5c0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbe638 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cbe8d0 <col:54, col:70>
// | `-ReturnStmt 0x63e625cbe8c0 <col:56, col:67>
// |   `-BinaryOperator 0x63e625cbe8a0 <col:63, col:67> 'int' '<'
// |     |-ImplicitCastExpr 0x63e625cbe870 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cbe830 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbe5c0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cbe888 <col:67> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cbe850 <col:67> 'int32_t':'int' lvalue ParmVar 0x63e625cbe638 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cbe788 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cd6e40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:24:18, col:71> col:26 cmp18 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x63e625cbe930 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x63e625cbe9a8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x63e625cd7040 <col:54, col:71>
// | `-ReturnStmt 0x63e625cd7030 <col:56, col:68>
// |   `-BinaryOperator 0x63e625cd7010 <col:63, col:68> 'int' '!='
// |     |-ImplicitCastExpr 0x63e625cd6fe0 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x63e625cd6fa0 <col:63> 'int32_t':'int' lvalue ParmVar 0x63e625cbe930 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x63e625cd6ff8 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x63e625cd6fc0 <col:68> 'int32_t':'int' lvalue ParmVar 0x63e625cbe9a8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x63e625cd6ef8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x63e625cd7290 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:26:18, col:71> col:26 cmp19 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd7100 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd7178 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd74d8 <col:54, col:71>
// | `-ReturnStmt 0x63e625cd74c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cd74b0 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd7490 <col:63, col:68> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625cd7460 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd7430 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd73f0 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd7100 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd7478 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd7448 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd7410 <col:68> 'int16_t':'short' lvalue ParmVar 0x63e625cd7178 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd7348 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd7648 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:27:18, col:71> col:26 cmp20 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd7538 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd75b0 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd7888 <col:54, col:71>
// | `-ReturnStmt 0x63e625cd7878 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cd7860 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd7840 <col:63, col:68> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625cd7810 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd77e0 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd77a0 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd7538 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd7828 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd77f8 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd77c0 <col:68> 'int16_t':'short' lvalue ParmVar 0x63e625cd75b0 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd7700 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd79f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:28:18, col:70> col:26 cmp21 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd78e8 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd7960 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd7c38 <col:54, col:70>
// | `-ReturnStmt 0x63e625cd7c28 <col:56, col:67>
// |   `-ImplicitCastExpr 0x63e625cd7c10 <col:63, col:67> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd7bf0 <col:63, col:67> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625cd7bc0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd7b90 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd7b50 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd78e8 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd7bd8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd7ba8 <col:67> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd7b70 <col:67> 'int16_t':'short' lvalue ParmVar 0x63e625cd7960 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd7ab0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd8e30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:29:18, col:71> col:26 cmp22 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd7c98 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd7d10 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd9078 <col:54, col:71>
// | `-ReturnStmt 0x63e625cd9068 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cd9050 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd9030 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625cd9000 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd8fd0 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd8f90 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd7c98 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd9018 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd8fe8 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd8fb0 <col:68> 'int16_t':'short' lvalue ParmVar 0x63e625cd7d10 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd8ee8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd91e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:30:18, col:70> col:26 cmp23 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd90d8 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd9150 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd9428 <col:54, col:70>
// | `-ReturnStmt 0x63e625cd9418 <col:56, col:67>
// |   `-ImplicitCastExpr 0x63e625cd9400 <col:63, col:67> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd93e0 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625cd93b0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd9380 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd9340 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd90d8 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd93c8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd9398 <col:67> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd9360 <col:67> 'int16_t':'short' lvalue ParmVar 0x63e625cd9150 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd92a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd9598 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:31:18, col:71> col:26 cmp24 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x63e625cd9488 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x63e625cd9500 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x63e625cd97d8 <col:54, col:71>
// | `-ReturnStmt 0x63e625cd97c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625cd97b0 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd9790 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625cd9760 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd9730 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd96f0 <col:63> 'int16_t':'short' lvalue ParmVar 0x63e625cd9488 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x63e625cd9778 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd9748 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd9710 <col:68> 'int16_t':'short' lvalue ParmVar 0x63e625cd9500 'y' 'int16_t':'short'
// `-AnnotateAttr 0x63e625cd9650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x63e625cd9a30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:33:17, col:67> col:24 cmp25 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cd98a0 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cd9918 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cd9c78 <col:50, col:67>
// | `-ReturnStmt 0x63e625cd9c68 <col:52, col:64>
// |   `-ImplicitCastExpr 0x63e625cd9c50 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cd9c30 <col:59, col:64> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625cd9c00 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd9bd0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd9b90 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cd98a0 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cd9c18 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd9be8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd9bb0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x63e625cd9918 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cd9ae8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cd9e40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:34:17, col:67> col:24 cmp26 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cd9cd8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cd9d50 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cda088 <col:50, col:67>
// | `-ReturnStmt 0x63e625cda078 <col:52, col:64>
// |   `-ImplicitCastExpr 0x63e625cda060 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cda040 <col:59, col:64> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625cda010 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cd9fe0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cd9fa0 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cd9cd8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cda028 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cd9ff8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cd9fc0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x63e625cd9d50 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cd9ef8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cda1f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:35:17, col:66> col:24 cmp27 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cda0e8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cda160 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cda438 <col:50, col:66>
// | `-ReturnStmt 0x63e625cda428 <col:52, col:63>
// |   `-ImplicitCastExpr 0x63e625cda410 <col:59, col:63> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cda3f0 <col:59, col:63> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625cda3c0 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cda390 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cda350 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda0e8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cda3d8 <col:63> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cda3a8 <col:63> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cda370 <col:63> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda160 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cda2b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cda5a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:36:17, col:67> col:24 cmp28 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cda498 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cda510 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cda7e8 <col:50, col:67>
// | `-ReturnStmt 0x63e625cda7d8 <col:52, col:64>
// |   `-ImplicitCastExpr 0x63e625cda7c0 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cda7a0 <col:59, col:64> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625cda770 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cda740 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cda700 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda498 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cda788 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cda758 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cda720 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda510 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cda660 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cda958 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:37:17, col:66> col:24 cmp29 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cda848 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cda8c0 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cdab98 <col:50, col:66>
// | `-ReturnStmt 0x63e625cdab88 <col:52, col:63>
// |   `-ImplicitCastExpr 0x63e625cdab70 <col:59, col:63> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdab50 <col:59, col:63> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625cdab20 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cdaaf0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cdaab0 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda848 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cdab38 <col:63> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cdab08 <col:63> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cdaad0 <col:63> 'int8_t':'signed char' lvalue ParmVar 0x63e625cda8c0 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cdaa10 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cdad08 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:38:17, col:67> col:24 cmp30 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x63e625cdabf8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x63e625cdac70 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x63e625cdaf68 <col:50, col:67>
// | `-ReturnStmt 0x63e625cdaf58 <col:52, col:64>
// |   `-ImplicitCastExpr 0x63e625cdaf40 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdaf20 <col:59, col:64> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625cdaef0 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625cdaec0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625cdae80 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x63e625cdabf8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x63e625cdaf08 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625cdaed8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625cdaea0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x63e625cdac70 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x63e625cdadc0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x63e625cdb1c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:41:19, col:75> col:28 cmp31 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625cdb030 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625cdb0a8 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625cdb3d8 <col:58, col:75>
// | `-ReturnStmt 0x63e625cdb3c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625cdb3b0 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdb390 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625cdb360 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cdb320 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb030 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625cdb378 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cdb340 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb0a8 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625cdb278 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625cdb548 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:42:19, col:75> col:28 cmp32 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625cdb438 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625cdb4b0 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625cdb758 <col:58, col:75>
// | `-ReturnStmt 0x63e625cdb748 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625cdb730 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdb710 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625cdb6e0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cdb6a0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb438 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625cdb6f8 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cdb6c0 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb4b0 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625cdb600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625cdb8c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:43:19, col:74> col:28 cmp33 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625cdb7b8 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625cdb830 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625cdbad8 <col:58, col:74>
// | `-ReturnStmt 0x63e625cdbac8 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625cdbab0 <col:67, col:71> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdba90 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625cdba60 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cdba20 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb7b8 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625cdba78 <col:71> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cdba40 <col:71> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdb830 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625cdb980 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625cdbc48 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:44:19, col:75> col:28 cmp34 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625cdbb38 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625cdbbb0 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625da01d0 <col:58, col:75>
// | `-ReturnStmt 0x63e625da01c0 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625cdbe30 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625cdbe10 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625cdbde0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625cdbda0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdbb38 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625cdbdf8 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625cdbdc0 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625cdbbb0 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625cdbd00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625da0340 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:45:19, col:74> col:28 cmp35 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625da0230 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625da02a8 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625da0558 <col:58, col:74>
// | `-ReturnStmt 0x63e625da0548 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625da0530 <col:67, col:71> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625da0510 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625da04e0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da04a0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625da0230 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625da04f8 <col:71> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da04c0 <col:71> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625da02a8 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625da03f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625da06c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:46:19, col:75> col:28 cmp36 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x63e625da05b8 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x63e625da0630 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x63e625da08d8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da08c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da08b0 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x63e625da0890 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625da0860 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da0820 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625da05b8 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x63e625da0878 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da0840 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x63e625da0630 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x63e625da0780 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x63e625da0b30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:48:19, col:75> col:28 cmp37 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da09a0 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da0a18 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da0d48 <col:58, col:75>
// | `-ReturnStmt 0x63e625da0d38 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da0d20 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da0d00 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625da0cd0 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da0c90 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da09a0 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da0ce8 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da0cb0 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da0a18 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da0be8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da0eb8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:49:19, col:75> col:28 cmp38 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da0da8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da0e20 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da10c8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da10b8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da10a0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da1080 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625da1050 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da1010 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da0da8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da1068 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da1030 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da0e20 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da0f70 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da1268 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:50:19, col:74> col:28 cmp39 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da1128 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da11d0 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da1478 <col:58, col:74>
// | `-ReturnStmt 0x63e625da1468 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625da1450 <col:67, col:71> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da1430 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625da1400 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da13c0 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da1128 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da1418 <col:71> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da13e0 <col:71> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da11d0 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da1320 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da15e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:51:19, col:75> col:28 cmp40 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da14d8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da1550 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da17f8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da17e8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da17d0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da17b0 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625da1780 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da1740 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da14d8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da1798 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da1760 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da1550 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da16a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da1968 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:52:19, col:74> col:28 cmp41 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da1858 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da18d0 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da1b78 <col:58, col:74>
// | `-ReturnStmt 0x63e625da1b68 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625da1b50 <col:67, col:71> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da1b30 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625da1b00 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da1ac0 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da1858 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da1b18 <col:71> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da1ae0 <col:71> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da18d0 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da1a20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da1ce8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:53:19, col:75> col:28 cmp42 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x63e625da1bd8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x63e625da1c50 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x63e625da1ef8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da1ee8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da1ed0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x63e625da1eb0 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625da1e80 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x63e625da1e40 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da1bd8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x63e625da1e98 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x63e625da1e60 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x63e625da1c50 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x63e625da1da0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x63e625da21e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:55:19, col:75> col:28 cmp43 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da1fc0 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da2038 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da2428 <col:58, col:75>
// | `-ReturnStmt 0x63e625da2418 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da2400 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da23e0 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625da23b0 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da2380 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da2340 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da1fc0 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da23c8 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da2398 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da2360 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2038 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da2298 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da2598 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:56:19, col:75> col:28 cmp44 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da2488 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da2500 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da27d8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da27c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da27b0 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da2790 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625da2760 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da2730 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da26f0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2488 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da2778 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da2748 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da2710 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2500 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da2650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da2948 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:57:19, col:74> col:28 cmp45 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da2838 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da28b0 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da2b88 <col:58, col:74>
// | `-ReturnStmt 0x63e625da2b78 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625da2b60 <col:67, col:71> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da2b40 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625da2b10 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da2ae0 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da2aa0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2838 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da2b28 <col:71> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da2af8 <col:71> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da2ac0 <col:71> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da28b0 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da2a00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da2cf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:58:19, col:75> col:28 cmp46 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da2be8 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da2c60 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da2f38 <col:58, col:75>
// | `-ReturnStmt 0x63e625da2f28 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da2f10 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da2ef0 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625da2ec0 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da2e90 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da2e50 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2be8 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da2ed8 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da2ea8 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da2e70 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2c60 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da2db0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da30a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:59:19, col:74> col:28 cmp47 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da2f98 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da3010 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da3308 <col:58, col:74>
// | `-ReturnStmt 0x63e625da32f8 <col:60, col:71>
// |   `-ImplicitCastExpr 0x63e625da32e0 <col:67, col:71> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da32c0 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625da3290 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da3260 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da3220 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da2f98 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da32a8 <col:71> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da3278 <col:71> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da3240 <col:71> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da3010 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da3160 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da3478 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:60:19, col:75> col:28 cmp48 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x63e625da3368 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x63e625da33e0 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x63e625da36b8 <col:58, col:75>
// | `-ReturnStmt 0x63e625da36a8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x63e625da3690 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x63e625da3670 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625da3640 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da3610 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da35d0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da3368 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x63e625da3658 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da3628 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da35f0 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x63e625da33e0 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x63e625da3530 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x63e625da3910 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:62:18, col:71> col:26 cmp49 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da3780 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da37f8 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da3b58 <col:54, col:71>
// | `-ReturnStmt 0x63e625da3b48 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625da3b30 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da3b10 <col:63, col:68> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625da3ae0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da3ab0 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da3a70 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da3780 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da3af8 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da3ac8 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da3a90 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da37f8 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da39c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da3cc8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:63:18, col:71> col:26 cmp50 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da3bb8 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da3c30 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da3f08 <col:54, col:71>
// | `-ReturnStmt 0x63e625da3ef8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625da3ee0 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da3ec0 <col:63, col:68> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625da3e90 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da3e60 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da3e20 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da3bb8 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da3ea8 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da3e78 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da3e40 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da3c30 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da3d80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da4078 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:64:18, col:70> col:26 cmp51 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da3f68 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da3fe0 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da42c8 <col:54, col:70>
// | `-ReturnStmt 0x63e625da42b8 <col:56, col:67>
// |   `-ImplicitCastExpr 0x63e625da42a0 <col:63, col:67> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da4280 <col:63, col:67> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625da4250 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da4220 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da41d0 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da3f68 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da4268 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da4238 <col:67> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da4200 <col:67> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da3fe0 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da4130 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da4438 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:65:18, col:71> col:26 cmp52 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da4328 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da43a0 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da4678 <col:54, col:71>
// | `-ReturnStmt 0x63e625da4668 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625da4650 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da4630 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625da4600 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da45d0 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da4590 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da4328 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da4618 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da45e8 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da45b0 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da43a0 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da44f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da47e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:66:18, col:70> col:26 cmp53 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da46d8 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da4750 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da4a28 <col:54, col:70>
// | `-ReturnStmt 0x63e625da4a18 <col:56, col:67>
// |   `-ImplicitCastExpr 0x63e625da4a00 <col:63, col:67> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da49e0 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625da49b0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da4980 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da4940 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da46d8 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da49c8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da4998 <col:67> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da4960 <col:67> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da4750 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da48a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da4b98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:67:18, col:71> col:26 cmp54 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x63e625da4a88 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x63e625da4b00 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x63e625da4dd8 <col:54, col:71>
// | `-ReturnStmt 0x63e625da4dc8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x63e625da4db0 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x63e625da4d90 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625da4d60 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da4d30 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da4cf0 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da4a88 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x63e625da4d78 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da4d48 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da4d10 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x63e625da4b00 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x63e625da4c50 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x63e625da4f90 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:59> col:20 cmp55 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da4e38 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da4eb0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da51d8 <col:42, col:59>
// | `-ReturnStmt 0x63e625da51c8 <col:44, col:56>
// |   `-ImplicitCastExpr 0x63e625da51b0 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da5190 <col:51, col:56> 'int' '=='
// |       |-ImplicitCastExpr 0x63e625da5160 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da5130 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da50f0 <col:51> 'bool' lvalue ParmVar 0x63e625da4e38 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da5178 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da5148 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da5110 <col:56> 'bool' lvalue ParmVar 0x63e625da4eb0 'y' 'bool'
// `-AnnotateAttr 0x63e625da5048 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x63e625da5368 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:59> col:20 cmp56 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da5258 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da52d0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da55a8 <col:42, col:59>
// | `-ReturnStmt 0x63e625da5598 <col:44, col:56>
// |   `-ImplicitCastExpr 0x63e625da5580 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da5560 <col:51, col:56> 'int' '>='
// |       |-ImplicitCastExpr 0x63e625da5530 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da5500 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da54c0 <col:51> 'bool' lvalue ParmVar 0x63e625da5258 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da5548 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da5518 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da54e0 <col:56> 'bool' lvalue ParmVar 0x63e625da52d0 'y' 'bool'
// `-AnnotateAttr 0x63e625da5420 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x63e625da5718 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:58> col:20 cmp57 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da5608 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da5680 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da5958 <col:42, col:58>
// | `-ReturnStmt 0x63e625da5948 <col:44, col:55>
// |   `-ImplicitCastExpr 0x63e625da5930 <col:51, col:55> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da5910 <col:51, col:55> 'int' '>'
// |       |-ImplicitCastExpr 0x63e625da58e0 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da58b0 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da5870 <col:51> 'bool' lvalue ParmVar 0x63e625da5608 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da58f8 <col:55> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da58c8 <col:55> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da5890 <col:55> 'bool' lvalue ParmVar 0x63e625da5680 'y' 'bool'
// `-AnnotateAttr 0x63e625da57d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x63e625da5ac8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:59> col:20 cmp58 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da59b8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da5a30 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da5d08 <col:42, col:59>
// | `-ReturnStmt 0x63e625da5cf8 <col:44, col:56>
// |   `-ImplicitCastExpr 0x63e625da5ce0 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da5cc0 <col:51, col:56> 'int' '<='
// |       |-ImplicitCastExpr 0x63e625da5c90 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da5c60 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da5c20 <col:51> 'bool' lvalue ParmVar 0x63e625da59b8 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da5ca8 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da5c78 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da5c40 <col:56> 'bool' lvalue ParmVar 0x63e625da5a30 'y' 'bool'
// `-AnnotateAttr 0x63e625da5b80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x63e625da5e78 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:58> col:20 cmp59 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da5d68 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da5de0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da60b8 <col:42, col:58>
// | `-ReturnStmt 0x63e625da60a8 <col:44, col:55>
// |   `-ImplicitCastExpr 0x63e625da6090 <col:51, col:55> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da6070 <col:51, col:55> 'int' '<'
// |       |-ImplicitCastExpr 0x63e625da6040 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da6010 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da5fd0 <col:51> 'bool' lvalue ParmVar 0x63e625da5d68 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da6058 <col:55> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da6028 <col:55> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da5ff0 <col:55> 'bool' lvalue ParmVar 0x63e625da5de0 'y' 'bool'
// `-AnnotateAttr 0x63e625da5f30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x63e625da7260 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:59> col:20 cmp60 'bool (bool, bool)'
// |-ParmVarDecl 0x63e625da6118 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:31> col:31 used x 'bool'
// |-ParmVarDecl 0x63e625da6190 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:39> col:39 used y 'bool'
// |-CompoundStmt 0x63e625da74a8 <col:42, col:59>
// | `-ReturnStmt 0x63e625da7498 <col:44, col:56>
// |   `-ImplicitCastExpr 0x63e625da7480 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x63e625da7460 <col:51, col:56> 'int' '!='
// |       |-ImplicitCastExpr 0x63e625da7430 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x63e625da7400 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x63e625da73c0 <col:51> 'bool' lvalue ParmVar 0x63e625da6118 'x' 'bool'
// |       `-ImplicitCastExpr 0x63e625da7448 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x63e625da7418 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x63e625da73e0 <col:56> 'bool' lvalue ParmVar 0x63e625da6190 'y' 'bool'
// `-AnnotateAttr 0x63e625da7318 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
