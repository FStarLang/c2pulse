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
// FunctionDecl 0x595c125af9d8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:5:18, col:58> col:22 cmp1 'int (int, int)'
// |-ParmVarDecl 0x595c125af878 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125af8f8 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125afc18 <col:41, col:58>
// | `-ReturnStmt 0x595c125afc08 <col:43, col:55>
// |   `-BinaryOperator 0x595c125afbe8 <col:50, col:55> 'int' '=='
// |     |-ImplicitCastExpr 0x595c125afbb8 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125afb78 <col:50> 'int' lvalue ParmVar 0x595c125af878 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125afbd0 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125afb98 <col:55> 'int' lvalue ParmVar 0x595c125af8f8 'y' 'int'
// `-AnnotateAttr 0x595c125afa90 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125afda0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:6:18, col:58> col:22 cmp2 'int (int, int)'
// |-ParmVarDecl 0x595c125afc80 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125afd00 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125affa0 <col:41, col:58>
// | `-ReturnStmt 0x595c125aff90 <col:43, col:55>
// |   `-BinaryOperator 0x595c125aff70 <col:50, col:55> 'int' '>='
// |     |-ImplicitCastExpr 0x595c125aff40 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125aff00 <col:50> 'int' lvalue ParmVar 0x595c125afc80 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125aff58 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125aff20 <col:55> 'int' lvalue ParmVar 0x595c125afd00 'y' 'int'
// `-AnnotateAttr 0x595c125afe58 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b0128 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:7:18, col:57> col:22 cmp3 'int (int, int)'
// |-ParmVarDecl 0x595c125b0008 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125b0088 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125b0320 <col:41, col:57>
// | `-ReturnStmt 0x595c125b0310 <col:43, col:54>
// |   `-BinaryOperator 0x595c125b02f0 <col:50, col:54> 'int' '>'
// |     |-ImplicitCastExpr 0x595c125b02c0 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125b0280 <col:50> 'int' lvalue ParmVar 0x595c125b0008 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125b02d8 <col:54> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125b02a0 <col:54> 'int' lvalue ParmVar 0x595c125b0088 'y' 'int'
// `-AnnotateAttr 0x595c125b01e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b04a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:8:18, col:58> col:22 cmp4 'int (int, int)'
// |-ParmVarDecl 0x595c125b0388 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125b0408 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125b06a0 <col:41, col:58>
// | `-ReturnStmt 0x595c125b0690 <col:43, col:55>
// |   `-BinaryOperator 0x595c125b0670 <col:50, col:55> 'int' '<='
// |     |-ImplicitCastExpr 0x595c125b0640 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125b0600 <col:50> 'int' lvalue ParmVar 0x595c125b0388 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125b0658 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125b0620 <col:55> 'int' lvalue ParmVar 0x595c125b0408 'y' 'int'
// `-AnnotateAttr 0x595c125b0560 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b7f38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:9:18, col:57> col:22 cmp5 'int (int, int)'
// |-ParmVarDecl 0x595c125b7e18 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125b7e98 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125b8130 <col:41, col:57>
// | `-ReturnStmt 0x595c125b8120 <col:43, col:54>
// |   `-BinaryOperator 0x595c125b8100 <col:50, col:54> 'int' '<'
// |     |-ImplicitCastExpr 0x595c125b80d0 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125b8090 <col:50> 'int' lvalue ParmVar 0x595c125b7e18 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125b80e8 <col:54> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125b80b0 <col:54> 'int' lvalue ParmVar 0x595c125b7e98 'y' 'int'
// `-AnnotateAttr 0x595c125b7ff0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b82b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:10:18, col:58> col:22 cmp6 'int (int, int)'
// |-ParmVarDecl 0x595c125b8198 <col:27, col:31> col:31 used x 'int'
// |-ParmVarDecl 0x595c125b8218 <col:34, col:38> col:38 used y 'int'
// |-CompoundStmt 0x595c125b84b0 <col:41, col:58>
// | `-ReturnStmt 0x595c125b84a0 <col:43, col:55>
// |   `-BinaryOperator 0x595c125b8480 <col:50, col:55> 'int' '!='
// |     |-ImplicitCastExpr 0x595c125b8450 <col:50> 'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125b8410 <col:50> 'int' lvalue ParmVar 0x595c125b8198 'x' 'int'
// |     `-ImplicitCastExpr 0x595c125b8468 <col:55> 'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125b8430 <col:55> 'int' lvalue ParmVar 0x595c125b8218 'y' 'int'
// `-AnnotateAttr 0x595c125b8370 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b8700 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:12:18, col:70> col:26 cmp7 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c125b8570 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c125b85e8 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x595c125b8918 <col:53, col:70>
// | `-ReturnStmt 0x595c125b8908 <col:55, col:67>
// |   `-ImplicitCastExpr 0x595c125b88f0 <col:62, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c125b88d0 <col:62, col:67> 'int' '=='
// |       |-ImplicitCastExpr 0x595c125b88a0 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125b8860 <col:62> 'int64_t':'long' lvalue ParmVar 0x595c125b8570 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c125b88b8 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125b8880 <col:67> 'int64_t':'long' lvalue ParmVar 0x595c125b85e8 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c125b87b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c125b8a88 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:13:18, col:70> col:26 cmp8 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c125b8978 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c125b89f0 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x595c125b8c98 <col:53, col:70>
// | `-ReturnStmt 0x595c125b8c88 <col:55, col:67>
// |   `-ImplicitCastExpr 0x595c125b8c70 <col:62, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c125b8c50 <col:62, col:67> 'int' '>='
// |       |-ImplicitCastExpr 0x595c125b8c20 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125b8be0 <col:62> 'int64_t':'long' lvalue ParmVar 0x595c125b8978 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c125b8c38 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125b8c00 <col:67> 'int64_t':'long' lvalue ParmVar 0x595c125b89f0 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c125b8b40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c1259ea40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:14:18, col:69> col:26 cmp9 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c125b8cf8 <col:31, col:39> col:39 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c125b8d70 <col:42, col:50> col:50 used y 'int64_t':'long'
// |-CompoundStmt 0x595c1259ec58 <col:53, col:69>
// | `-ReturnStmt 0x595c1259ec48 <col:55, col:66>
// |   `-ImplicitCastExpr 0x595c1259ec30 <col:62, col:66> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c1259ec10 <col:62, col:66> 'int' '>'
// |       |-ImplicitCastExpr 0x595c1259ebe0 <col:62> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c1259eba0 <col:62> 'int64_t':'long' lvalue ParmVar 0x595c125b8cf8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c1259ebf8 <col:66> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c1259ebc0 <col:66> 'int64_t':'long' lvalue ParmVar 0x595c125b8d70 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c1259eaf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c1259edc8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:15:18, col:71> col:26 cmp10 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c1259ecb8 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c1259ed30 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x595c1259efd8 <col:54, col:71>
// | `-ReturnStmt 0x595c1259efc8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c1259efb0 <col:63, col:68> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c1259ef90 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x595c1259ef60 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c1259ef20 <col:63> 'int64_t':'long' lvalue ParmVar 0x595c1259ecb8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c1259ef78 <col:68> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c1259ef40 <col:68> 'int64_t':'long' lvalue ParmVar 0x595c1259ed30 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c1259ee80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c1259f148 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:16:18, col:70> col:26 cmp11 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c1259f038 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c1259f0b0 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x595c1259f358 <col:54, col:70>
// | `-ReturnStmt 0x595c1259f348 <col:56, col:67>
// |   `-ImplicitCastExpr 0x595c1259f330 <col:63, col:67> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c1259f310 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x595c1259f2e0 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c1259f2a0 <col:63> 'int64_t':'long' lvalue ParmVar 0x595c1259f038 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c1259f2f8 <col:67> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c1259f2c0 <col:67> 'int64_t':'long' lvalue ParmVar 0x595c1259f0b0 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c1259f200 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c1259f4c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:17:18, col:71> col:26 cmp12 'int64_t (int64_t, int64_t)'
// |-ParmVarDecl 0x595c1259f3b8 <col:32, col:40> col:40 used x 'int64_t':'long'
// |-ParmVarDecl 0x595c1259f430 <col:43, col:51> col:51 used y 'int64_t':'long'
// |-CompoundStmt 0x595c1259f6d8 <col:54, col:71>
// | `-ReturnStmt 0x595c1259f6c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c1259f6b0 <col:63, col:68> 'int64_t':'long' <IntegralCast>
// |     `-BinaryOperator 0x595c1259f690 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x595c1259f660 <col:63> 'int64_t':'long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c1259f620 <col:63> 'int64_t':'long' lvalue ParmVar 0x595c1259f3b8 'x' 'int64_t':'long'
// |       `-ImplicitCastExpr 0x595c1259f678 <col:68> 'int64_t':'long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c1259f640 <col:68> 'int64_t':'long' lvalue ParmVar 0x595c1259f430 'y' 'int64_t':'long'
// `-AnnotateAttr 0x595c1259f580 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int64.t|END"
// FunctionDecl 0x595c1259f8f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:19:18, col:71> col:26 cmp13 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c1259f7a0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c1259f818 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c1259fb10 <col:54, col:71>
// | `-ReturnStmt 0x595c1259fb00 <col:56, col:68>
// |   `-BinaryOperator 0x595c1259fae0 <col:63, col:68> 'int' '=='
// |     |-ImplicitCastExpr 0x595c1259fab0 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c1259fa70 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c1259f7a0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c1259fac8 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c1259fa90 <col:68> 'int32_t':'int' lvalue ParmVar 0x595c1259f818 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c1259f9a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c1259fc80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:20:18, col:71> col:26 cmp14 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c1259fb70 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c1259fbe8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c1259fe80 <col:54, col:71>
// | `-ReturnStmt 0x595c1259fe70 <col:56, col:68>
// |   `-BinaryOperator 0x595c1259fe50 <col:63, col:68> 'int' '>='
// |     |-ImplicitCastExpr 0x595c1259fe20 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c1259fde0 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c1259fb70 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c1259fe38 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c1259fe00 <col:68> 'int32_t':'int' lvalue ParmVar 0x595c1259fbe8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c1259fd38 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c1259fff0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:21:18, col:70> col:26 cmp15 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c1259fee0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c1259ff58 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c125a01f0 <col:54, col:70>
// | `-ReturnStmt 0x595c125a01e0 <col:56, col:67>
// |   `-BinaryOperator 0x595c125a01c0 <col:63, col:67> 'int' '>'
// |     |-ImplicitCastExpr 0x595c125a0190 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125a0150 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c1259fee0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c125a01a8 <col:67> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125a0170 <col:67> 'int32_t':'int' lvalue ParmVar 0x595c1259ff58 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c125a00a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125a0360 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:22:18, col:71> col:26 cmp16 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c125a0250 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c125a02c8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c125a0560 <col:54, col:71>
// | `-ReturnStmt 0x595c125a0550 <col:56, col:68>
// |   `-BinaryOperator 0x595c125a0530 <col:63, col:68> 'int' '<='
// |     |-ImplicitCastExpr 0x595c125a0500 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125a04c0 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c125a0250 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c125a0518 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125a04e0 <col:68> 'int32_t':'int' lvalue ParmVar 0x595c125a02c8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c125a0418 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125a06d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:23:18, col:70> col:26 cmp17 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c125a05c0 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c125a0638 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c125a08d0 <col:54, col:70>
// | `-ReturnStmt 0x595c125a08c0 <col:56, col:67>
// |   `-BinaryOperator 0x595c125a08a0 <col:63, col:67> 'int' '<'
// |     |-ImplicitCastExpr 0x595c125a0870 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125a0830 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c125a05c0 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c125a0888 <col:67> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125a0850 <col:67> 'int32_t':'int' lvalue ParmVar 0x595c125a0638 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c125a0788 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b8e40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:24:18, col:71> col:26 cmp18 'int32_t (int32_t, int32_t)'
// |-ParmVarDecl 0x595c125a0930 <col:32, col:40> col:40 used x 'int32_t':'int'
// |-ParmVarDecl 0x595c125a09a8 <col:43, col:51> col:51 used y 'int32_t':'int'
// |-CompoundStmt 0x595c125b9040 <col:54, col:71>
// | `-ReturnStmt 0x595c125b9030 <col:56, col:68>
// |   `-BinaryOperator 0x595c125b9010 <col:63, col:68> 'int' '!='
// |     |-ImplicitCastExpr 0x595c125b8fe0 <col:63> 'int32_t':'int' <LValueToRValue>
// |     | `-DeclRefExpr 0x595c125b8fa0 <col:63> 'int32_t':'int' lvalue ParmVar 0x595c125a0930 'x' 'int32_t':'int'
// |     `-ImplicitCastExpr 0x595c125b8ff8 <col:68> 'int32_t':'int' <LValueToRValue>
// |       `-DeclRefExpr 0x595c125b8fc0 <col:68> 'int32_t':'int' lvalue ParmVar 0x595c125a09a8 'y' 'int32_t':'int'
// `-AnnotateAttr 0x595c125b8ef8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int32.t|END"
// FunctionDecl 0x595c125b9290 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:26:18, col:71> col:26 cmp19 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125b9100 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125b9178 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125b94d8 <col:54, col:71>
// | `-ReturnStmt 0x595c125b94c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c125b94b0 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125b9490 <col:63, col:68> 'int' '=='
// |       |-ImplicitCastExpr 0x595c125b9460 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125b9430 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125b93f0 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125b9100 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125b9478 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125b9448 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125b9410 <col:68> 'int16_t':'short' lvalue ParmVar 0x595c125b9178 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125b9348 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125b9648 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:27:18, col:71> col:26 cmp20 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125b9538 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125b95b0 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125b9888 <col:54, col:71>
// | `-ReturnStmt 0x595c125b9878 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c125b9860 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125b9840 <col:63, col:68> 'int' '>='
// |       |-ImplicitCastExpr 0x595c125b9810 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125b97e0 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125b97a0 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125b9538 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125b9828 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125b97f8 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125b97c0 <col:68> 'int16_t':'short' lvalue ParmVar 0x595c125b95b0 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125b9700 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125b99f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:28:18, col:70> col:26 cmp21 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125b98e8 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125b9960 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125b9c38 <col:54, col:70>
// | `-ReturnStmt 0x595c125b9c28 <col:56, col:67>
// |   `-ImplicitCastExpr 0x595c125b9c10 <col:63, col:67> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125b9bf0 <col:63, col:67> 'int' '>'
// |       |-ImplicitCastExpr 0x595c125b9bc0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125b9b90 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125b9b50 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125b98e8 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125b9bd8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125b9ba8 <col:67> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125b9b70 <col:67> 'int16_t':'short' lvalue ParmVar 0x595c125b9960 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125b9ab0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125bae30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:29:18, col:71> col:26 cmp22 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125b9c98 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125b9d10 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125bb078 <col:54, col:71>
// | `-ReturnStmt 0x595c125bb068 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c125bb050 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125bb030 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x595c125bb000 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bafd0 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125baf90 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125b9c98 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125bb018 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bafe8 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bafb0 <col:68> 'int16_t':'short' lvalue ParmVar 0x595c125b9d10 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125baee8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125bb1e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:30:18, col:70> col:26 cmp23 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125bb0d8 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125bb150 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125bb428 <col:54, col:70>
// | `-ReturnStmt 0x595c125bb418 <col:56, col:67>
// |   `-ImplicitCastExpr 0x595c125bb400 <col:63, col:67> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125bb3e0 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x595c125bb3b0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bb380 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bb340 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125bb0d8 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125bb3c8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bb398 <col:67> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bb360 <col:67> 'int16_t':'short' lvalue ParmVar 0x595c125bb150 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125bb2a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125bb598 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:31:18, col:71> col:26 cmp24 'int16_t (int16_t, int16_t)'
// |-ParmVarDecl 0x595c125bb488 <col:32, col:40> col:40 used x 'int16_t':'short'
// |-ParmVarDecl 0x595c125bb500 <col:43, col:51> col:51 used y 'int16_t':'short'
// |-CompoundStmt 0x595c125bb7d8 <col:54, col:71>
// | `-ReturnStmt 0x595c125bb7c8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c125bb7b0 <col:63, col:68> 'int16_t':'short' <IntegralCast>
// |     `-BinaryOperator 0x595c125bb790 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x595c125bb760 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bb730 <col:63> 'int16_t':'short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bb6f0 <col:63> 'int16_t':'short' lvalue ParmVar 0x595c125bb488 'x' 'int16_t':'short'
// |       `-ImplicitCastExpr 0x595c125bb778 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bb748 <col:68> 'int16_t':'short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bb710 <col:68> 'int16_t':'short' lvalue ParmVar 0x595c125bb500 'y' 'int16_t':'short'
// `-AnnotateAttr 0x595c125bb650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int16.t|END"
// FunctionDecl 0x595c125bba30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:33:17, col:67> col:24 cmp25 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bb8a0 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bb918 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bbc78 <col:50, col:67>
// | `-ReturnStmt 0x595c125bbc68 <col:52, col:64>
// |   `-ImplicitCastExpr 0x595c125bbc50 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bbc30 <col:59, col:64> 'int' '=='
// |       |-ImplicitCastExpr 0x595c125bbc00 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bbbd0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bbb90 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bb8a0 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bbc18 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bbbe8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bbbb0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x595c125bb918 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bbae8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bbe40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:34:17, col:67> col:24 cmp26 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bbcd8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bbd50 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bc088 <col:50, col:67>
// | `-ReturnStmt 0x595c125bc078 <col:52, col:64>
// |   `-ImplicitCastExpr 0x595c125bc060 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bc040 <col:59, col:64> 'int' '>='
// |       |-ImplicitCastExpr 0x595c125bc010 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bbfe0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bbfa0 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bbcd8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bc028 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bbff8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bbfc0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x595c125bbd50 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bbef8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bc1f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:35:17, col:66> col:24 cmp27 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bc0e8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bc160 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bc438 <col:50, col:66>
// | `-ReturnStmt 0x595c125bc428 <col:52, col:63>
// |   `-ImplicitCastExpr 0x595c125bc410 <col:59, col:63> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bc3f0 <col:59, col:63> 'int' '>'
// |       |-ImplicitCastExpr 0x595c125bc3c0 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bc390 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bc350 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc0e8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bc3d8 <col:63> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bc3a8 <col:63> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bc370 <col:63> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc160 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bc2b0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bc5a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:36:17, col:67> col:24 cmp28 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bc498 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bc510 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bc7e8 <col:50, col:67>
// | `-ReturnStmt 0x595c125bc7d8 <col:52, col:64>
// |   `-ImplicitCastExpr 0x595c125bc7c0 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bc7a0 <col:59, col:64> 'int' '<='
// |       |-ImplicitCastExpr 0x595c125bc770 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bc740 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bc700 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc498 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bc788 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bc758 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bc720 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc510 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bc660 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bc958 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:37:17, col:66> col:24 cmp29 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bc848 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bc8c0 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bcb98 <col:50, col:66>
// | `-ReturnStmt 0x595c125bcb88 <col:52, col:63>
// |   `-ImplicitCastExpr 0x595c125bcb70 <col:59, col:63> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bcb50 <col:59, col:63> 'int' '<'
// |       |-ImplicitCastExpr 0x595c125bcb20 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bcaf0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bcab0 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc848 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bcb38 <col:63> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bcb08 <col:63> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bcad0 <col:63> 'int8_t':'signed char' lvalue ParmVar 0x595c125bc8c0 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bca10 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bcd08 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:38:17, col:67> col:24 cmp30 'int8_t (int8_t, int8_t)'
// |-ParmVarDecl 0x595c125bcbf8 <col:30, col:37> col:37 used x 'int8_t':'signed char'
// |-ParmVarDecl 0x595c125bcc70 <col:40, col:47> col:47 used y 'int8_t':'signed char'
// |-CompoundStmt 0x595c125bcf68 <col:50, col:67>
// | `-ReturnStmt 0x595c125bcf58 <col:52, col:64>
// |   `-ImplicitCastExpr 0x595c125bcf40 <col:59, col:64> 'int8_t':'signed char' <IntegralCast>
// |     `-BinaryOperator 0x595c125bcf20 <col:59, col:64> 'int' '!='
// |       |-ImplicitCastExpr 0x595c125bcef0 <col:59> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c125bcec0 <col:59> 'int8_t':'signed char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c125bce80 <col:59> 'int8_t':'signed char' lvalue ParmVar 0x595c125bcbf8 'x' 'int8_t':'signed char'
// |       `-ImplicitCastExpr 0x595c125bcf08 <col:64> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c125bced8 <col:64> 'int8_t':'signed char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c125bcea0 <col:64> 'int8_t':'signed char' lvalue ParmVar 0x595c125bcc70 'y' 'int8_t':'signed char'
// `-AnnotateAttr 0x595c125bcdc0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:Int8.t|END"
// FunctionDecl 0x595c125bd1c0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:41:19, col:75> col:28 cmp31 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c125bd030 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c125bd0a8 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c125bd3d8 <col:58, col:75>
// | `-ReturnStmt 0x595c125bd3c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c125bd3b0 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c125bd390 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x595c125bd360 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125bd320 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd030 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c125bd378 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125bd340 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd0a8 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c125bd278 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c125bd548 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:42:19, col:75> col:28 cmp32 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c125bd438 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c125bd4b0 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c125bd758 <col:58, col:75>
// | `-ReturnStmt 0x595c125bd748 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c125bd730 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c125bd710 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x595c125bd6e0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125bd6a0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd438 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c125bd6f8 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125bd6c0 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd4b0 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c125bd600 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c125bd8c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:43:19, col:74> col:28 cmp33 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c125bd7b8 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c125bd830 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c125bdad8 <col:58, col:74>
// | `-ReturnStmt 0x595c125bdac8 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c125bdab0 <col:67, col:71> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c125bda90 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x595c125bda60 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125bda20 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd7b8 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c125bda78 <col:71> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125bda40 <col:71> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bd830 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c125bd980 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c125bdc48 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:44:19, col:75> col:28 cmp34 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c125bdb38 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c125bdbb0 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c126821d0 <col:58, col:75>
// | `-ReturnStmt 0x595c126821c0 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c125bde30 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c125bde10 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x595c125bdde0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c125bdda0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bdb38 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c125bddf8 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c125bddc0 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c125bdbb0 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c125bdd00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c12682340 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:45:19, col:74> col:28 cmp35 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c12682230 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c126822a8 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c12682558 <col:58, col:74>
// | `-ReturnStmt 0x595c12682548 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c12682530 <col:67, col:71> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c12682510 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x595c126824e0 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c126824a0 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c12682230 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c126824f8 <col:71> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c126824c0 <col:71> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c126822a8 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c126823f8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c126826c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:46:19, col:75> col:28 cmp36 'uint64_t (uint64_t, uint64_t)'
// |-ParmVarDecl 0x595c126825b8 <col:34, col:43> col:43 used x 'uint64_t':'unsigned long'
// |-ParmVarDecl 0x595c12682630 <col:46, col:55> col:55 used y 'uint64_t':'unsigned long'
// |-CompoundStmt 0x595c126828d8 <col:58, col:75>
// | `-ReturnStmt 0x595c126828c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c126828b0 <col:67, col:72> 'uint64_t':'unsigned long' <IntegralCast>
// |     `-BinaryOperator 0x595c12682890 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x595c12682860 <col:67> 'uint64_t':'unsigned long' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12682820 <col:67> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c126825b8 'x' 'uint64_t':'unsigned long'
// |       `-ImplicitCastExpr 0x595c12682878 <col:72> 'uint64_t':'unsigned long' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12682840 <col:72> 'uint64_t':'unsigned long' lvalue ParmVar 0x595c12682630 'y' 'uint64_t':'unsigned long'
// `-AnnotateAttr 0x595c12682780 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt64.t|END"
// FunctionDecl 0x595c12682b30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:48:19, col:75> col:28 cmp37 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c126829a0 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c12682a18 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c12682d48 <col:58, col:75>
// | `-ReturnStmt 0x595c12682d38 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c12682d20 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c12682d00 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x595c12682cd0 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12682c90 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c126829a0 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12682ce8 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12682cb0 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12682a18 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c12682be8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c12682eb8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:49:19, col:75> col:28 cmp38 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c12682da8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c12682e20 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c126830c8 <col:58, col:75>
// | `-ReturnStmt 0x595c126830b8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c126830a0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c12683080 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x595c12683050 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12683010 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12682da8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12683068 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12683030 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12682e20 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c12682f70 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c12683268 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:50:19, col:74> col:28 cmp39 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c12683128 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c126831d0 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c12683478 <col:58, col:74>
// | `-ReturnStmt 0x595c12683468 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c12683450 <col:67, col:71> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c12683430 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x595c12683400 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c126833c0 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12683128 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12683418 <col:71> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c126833e0 <col:71> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c126831d0 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c12683320 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c126835e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:51:19, col:75> col:28 cmp40 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c126834d8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c12683550 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c126837f8 <col:58, col:75>
// | `-ReturnStmt 0x595c126837e8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c126837d0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c126837b0 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x595c12683780 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12683740 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c126834d8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12683798 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12683760 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12683550 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c126836a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c12683968 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:52:19, col:74> col:28 cmp41 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c12683858 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c126838d0 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c12683b78 <col:58, col:74>
// | `-ReturnStmt 0x595c12683b68 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c12683b50 <col:67, col:71> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c12683b30 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x595c12683b00 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12683ac0 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12683858 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12683b18 <col:71> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12683ae0 <col:71> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c126838d0 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c12683a20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c12683ce8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:53:19, col:75> col:28 cmp42 'uint32_t (uint32_t, uint32_t)'
// |-ParmVarDecl 0x595c12683bd8 <col:34, col:43> col:43 used x 'uint32_t':'unsigned int'
// |-ParmVarDecl 0x595c12683c50 <col:46, col:55> col:55 used y 'uint32_t':'unsigned int'
// |-CompoundStmt 0x595c12683ef8 <col:58, col:75>
// | `-ReturnStmt 0x595c12683ee8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c12683ed0 <col:67, col:72> 'uint32_t':'unsigned int' <IntegralCast>
// |     `-BinaryOperator 0x595c12683eb0 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x595c12683e80 <col:67> 'uint32_t':'unsigned int' <LValueToRValue>
// |       | `-DeclRefExpr 0x595c12683e40 <col:67> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12683bd8 'x' 'uint32_t':'unsigned int'
// |       `-ImplicitCastExpr 0x595c12683e98 <col:72> 'uint32_t':'unsigned int' <LValueToRValue>
// |         `-DeclRefExpr 0x595c12683e60 <col:72> 'uint32_t':'unsigned int' lvalue ParmVar 0x595c12683c50 'y' 'uint32_t':'unsigned int'
// `-AnnotateAttr 0x595c12683da0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt32.t|END"
// FunctionDecl 0x595c126841e0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:55:19, col:75> col:28 cmp43 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12683fc0 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c12684038 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c12684428 <col:58, col:75>
// | `-ReturnStmt 0x595c12684418 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c12684400 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c126843e0 <col:67, col:72> 'int' '=='
// |       |-ImplicitCastExpr 0x595c126843b0 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12684380 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12684340 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12683fc0 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c126843c8 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12684398 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12684360 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684038 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12684298 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c12684598 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:56:19, col:75> col:28 cmp44 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12684488 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c12684500 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c126847d8 <col:58, col:75>
// | `-ReturnStmt 0x595c126847c8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c126847b0 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c12684790 <col:67, col:72> 'int' '>='
// |       |-ImplicitCastExpr 0x595c12684760 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12684730 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126846f0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684488 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c12684778 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12684748 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12684710 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684500 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12684650 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c12684948 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:57:19, col:74> col:28 cmp45 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12684838 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c126848b0 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c12684b88 <col:58, col:74>
// | `-ReturnStmt 0x595c12684b78 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c12684b60 <col:67, col:71> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c12684b40 <col:67, col:71> 'int' '>'
// |       |-ImplicitCastExpr 0x595c12684b10 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12684ae0 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12684aa0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684838 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c12684b28 <col:71> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12684af8 <col:71> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12684ac0 <col:71> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c126848b0 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12684a00 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c12684cf8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:58:19, col:75> col:28 cmp46 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12684be8 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c12684c60 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c12684f38 <col:58, col:75>
// | `-ReturnStmt 0x595c12684f28 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c12684f10 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c12684ef0 <col:67, col:72> 'int' '<='
// |       |-ImplicitCastExpr 0x595c12684ec0 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12684e90 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12684e50 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684be8 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c12684ed8 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12684ea8 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12684e70 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684c60 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12684db0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c126850a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:59:19, col:74> col:28 cmp47 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12684f98 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c12685010 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c12685308 <col:58, col:74>
// | `-ReturnStmt 0x595c126852f8 <col:60, col:71>
// |   `-ImplicitCastExpr 0x595c126852e0 <col:67, col:71> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c126852c0 <col:67, col:71> 'int' '<'
// |       |-ImplicitCastExpr 0x595c12685290 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12685260 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12685220 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12684f98 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c126852a8 <col:71> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12685278 <col:71> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12685240 <col:71> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12685010 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12685160 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c12685478 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:60:19, col:75> col:28 cmp48 'uint16_t (uint16_t, uint16_t)'
// |-ParmVarDecl 0x595c12685368 <col:34, col:43> col:43 used x 'uint16_t':'unsigned short'
// |-ParmVarDecl 0x595c126853e0 <col:46, col:55> col:55 used y 'uint16_t':'unsigned short'
// |-CompoundStmt 0x595c126856b8 <col:58, col:75>
// | `-ReturnStmt 0x595c126856a8 <col:60, col:72>
// |   `-ImplicitCastExpr 0x595c12685690 <col:67, col:72> 'uint16_t':'unsigned short' <IntegralCast>
// |     `-BinaryOperator 0x595c12685670 <col:67, col:72> 'int' '!='
// |       |-ImplicitCastExpr 0x595c12685640 <col:67> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12685610 <col:67> 'uint16_t':'unsigned short' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126855d0 <col:67> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c12685368 'x' 'uint16_t':'unsigned short'
// |       `-ImplicitCastExpr 0x595c12685658 <col:72> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12685628 <col:72> 'uint16_t':'unsigned short' <LValueToRValue>
// |           `-DeclRefExpr 0x595c126855f0 <col:72> 'uint16_t':'unsigned short' lvalue ParmVar 0x595c126853e0 'y' 'uint16_t':'unsigned short'
// `-AnnotateAttr 0x595c12685530 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt16.t|END"
// FunctionDecl 0x595c12685910 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:62:18, col:71> col:26 cmp49 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c12685780 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c126857f8 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c12685b58 <col:54, col:71>
// | `-ReturnStmt 0x595c12685b48 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c12685b30 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c12685b10 <col:63, col:68> 'int' '=='
// |       |-ImplicitCastExpr 0x595c12685ae0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12685ab0 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12685a70 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12685780 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c12685af8 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12685ac8 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12685a90 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c126857f8 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c126859c8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c12685cc8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:63:18, col:71> col:26 cmp50 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c12685bb8 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c12685c30 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c12685f08 <col:54, col:71>
// | `-ReturnStmt 0x595c12685ef8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c12685ee0 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c12685ec0 <col:63, col:68> 'int' '>='
// |       |-ImplicitCastExpr 0x595c12685e90 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12685e60 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12685e20 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12685bb8 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c12685ea8 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12685e78 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12685e40 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12685c30 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c12685d80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c12686078 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:64:18, col:70> col:26 cmp51 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c12685f68 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c12685fe0 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c126862c8 <col:54, col:70>
// | `-ReturnStmt 0x595c126862b8 <col:56, col:67>
// |   `-ImplicitCastExpr 0x595c126862a0 <col:63, col:67> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c12686280 <col:63, col:67> 'int' '>'
// |       |-ImplicitCastExpr 0x595c12686250 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12686220 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126861d0 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12685f68 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c12686268 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12686238 <col:67> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12686200 <col:67> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12685fe0 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c12686130 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c12686438 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:65:18, col:71> col:26 cmp52 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c12686328 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c126863a0 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c12686678 <col:54, col:71>
// | `-ReturnStmt 0x595c12686668 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c12686650 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c12686630 <col:63, col:68> 'int' '<='
// |       |-ImplicitCastExpr 0x595c12686600 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c126865d0 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12686590 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12686328 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c12686618 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c126865e8 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c126865b0 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c126863a0 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c126864f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c126867e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:66:18, col:70> col:26 cmp53 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c126866d8 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c12686750 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c12686a28 <col:54, col:70>
// | `-ReturnStmt 0x595c12686a18 <col:56, col:67>
// |   `-ImplicitCastExpr 0x595c12686a00 <col:63, col:67> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c126869e0 <col:63, col:67> 'int' '<'
// |       |-ImplicitCastExpr 0x595c126869b0 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12686980 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12686940 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c126866d8 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c126869c8 <col:67> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12686998 <col:67> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12686960 <col:67> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12686750 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c126868a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c12686b98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:67:18, col:71> col:26 cmp54 'uint8_t (uint8_t, uint8_t)'
// |-ParmVarDecl 0x595c12686a88 <col:32, col:40> col:40 used x 'uint8_t':'unsigned char'
// |-ParmVarDecl 0x595c12686b00 <col:43, col:51> col:51 used y 'uint8_t':'unsigned char'
// |-CompoundStmt 0x595c12686dd8 <col:54, col:71>
// | `-ReturnStmt 0x595c12686dc8 <col:56, col:68>
// |   `-ImplicitCastExpr 0x595c12686db0 <col:63, col:68> 'uint8_t':'unsigned char' <IntegralCast>
// |     `-BinaryOperator 0x595c12686d90 <col:63, col:68> 'int' '!='
// |       |-ImplicitCastExpr 0x595c12686d60 <col:63> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12686d30 <col:63> 'uint8_t':'unsigned char' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12686cf0 <col:63> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12686a88 'x' 'uint8_t':'unsigned char'
// |       `-ImplicitCastExpr 0x595c12686d78 <col:68> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12686d48 <col:68> 'uint8_t':'unsigned char' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12686d10 <col:68> 'uint8_t':'unsigned char' lvalue ParmVar 0x595c12686b00 'y' 'uint8_t':'unsigned char'
// `-AnnotateAttr 0x595c12686c50 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:UInt8.t|END"
// FunctionDecl 0x595c12686f90 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:59> col:20 cmp55 'bool (bool, bool)'
// |-ParmVarDecl 0x595c12686e38 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c12686eb0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:70:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c126871d8 <col:42, col:59>
// | `-ReturnStmt 0x595c126871c8 <col:44, col:56>
// |   `-ImplicitCastExpr 0x595c126871b0 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12687190 <col:51, col:56> 'int' '=='
// |       |-ImplicitCastExpr 0x595c12687160 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12687130 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126870f0 <col:51> 'bool' lvalue ParmVar 0x595c12686e38 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c12687178 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12687148 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12687110 <col:56> 'bool' lvalue ParmVar 0x595c12686eb0 'y' 'bool'
// `-AnnotateAttr 0x595c12687048 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x595c12687368 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:59> col:20 cmp56 'bool (bool, bool)'
// |-ParmVarDecl 0x595c12687258 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c126872d0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:71:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c126875a8 <col:42, col:59>
// | `-ReturnStmt 0x595c12687598 <col:44, col:56>
// |   `-ImplicitCastExpr 0x595c12687580 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12687560 <col:51, col:56> 'int' '>='
// |       |-ImplicitCastExpr 0x595c12687530 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12687500 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126874c0 <col:51> 'bool' lvalue ParmVar 0x595c12687258 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c12687548 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12687518 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c126874e0 <col:56> 'bool' lvalue ParmVar 0x595c126872d0 'y' 'bool'
// `-AnnotateAttr 0x595c12687420 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x595c12687718 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:58> col:20 cmp57 'bool (bool, bool)'
// |-ParmVarDecl 0x595c12687608 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c12687680 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:72:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c12687958 <col:42, col:58>
// | `-ReturnStmt 0x595c12687948 <col:44, col:55>
// |   `-ImplicitCastExpr 0x595c12687930 <col:51, col:55> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12687910 <col:51, col:55> 'int' '>'
// |       |-ImplicitCastExpr 0x595c126878e0 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c126878b0 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12687870 <col:51> 'bool' lvalue ParmVar 0x595c12687608 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c126878f8 <col:55> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c126878c8 <col:55> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12687890 <col:55> 'bool' lvalue ParmVar 0x595c12687680 'y' 'bool'
// `-AnnotateAttr 0x595c126877d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x595c12687ac8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:59> col:20 cmp58 'bool (bool, bool)'
// |-ParmVarDecl 0x595c126879b8 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c12687a30 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:73:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c12687d08 <col:42, col:59>
// | `-ReturnStmt 0x595c12687cf8 <col:44, col:56>
// |   `-ImplicitCastExpr 0x595c12687ce0 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12687cc0 <col:51, col:56> 'int' '<='
// |       |-ImplicitCastExpr 0x595c12687c90 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12687c60 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12687c20 <col:51> 'bool' lvalue ParmVar 0x595c126879b8 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c12687ca8 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12687c78 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12687c40 <col:56> 'bool' lvalue ParmVar 0x595c12687a30 'y' 'bool'
// `-AnnotateAttr 0x595c12687b80 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x595c12687e78 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:58> col:20 cmp59 'bool (bool, bool)'
// |-ParmVarDecl 0x595c12687d68 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c12687de0 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:74:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c126880b8 <col:42, col:58>
// | `-ReturnStmt 0x595c126880a8 <col:44, col:55>
// |   `-ImplicitCastExpr 0x595c12688090 <col:51, col:55> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12688070 <col:51, col:55> 'int' '<'
// |       |-ImplicitCastExpr 0x595c12688040 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12688010 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c12687fd0 <col:51> 'bool' lvalue ParmVar 0x595c12687d68 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c12688058 <col:55> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12688028 <col:55> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c12687ff0 <col:55> 'bool' lvalue ParmVar 0x595c12687de0 'y' 'bool'
// `-AnnotateAttr 0x595c12687f30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
// FunctionDecl 0x595c12689260 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:59> col:20 cmp60 'bool (bool, bool)'
// |-ParmVarDecl 0x595c12688118 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:31> col:31 used x 'bool'
// |-ParmVarDecl 0x595c12688190 </home/t-visinghal/Applications/src/c2pulse/external/llvm-project/build/lib/clang/21/include/stdbool.h:24:14, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue51_test.c:75:39> col:39 used y 'bool'
// |-CompoundStmt 0x595c126894a8 <col:42, col:59>
// | `-ReturnStmt 0x595c12689498 <col:44, col:56>
// |   `-ImplicitCastExpr 0x595c12689480 <col:51, col:56> 'bool' <IntegralToBoolean>
// |     `-BinaryOperator 0x595c12689460 <col:51, col:56> 'int' '!='
// |       |-ImplicitCastExpr 0x595c12689430 <col:51> 'int' <IntegralCast>
// |       | `-ImplicitCastExpr 0x595c12689400 <col:51> 'bool' <LValueToRValue>
// |       |   `-DeclRefExpr 0x595c126893c0 <col:51> 'bool' lvalue ParmVar 0x595c12688118 'x' 'bool'
// |       `-ImplicitCastExpr 0x595c12689448 <col:56> 'int' <IntegralCast>
// |         `-ImplicitCastExpr 0x595c12689418 <col:56> 'bool' <LValueToRValue>
// |           `-DeclRefExpr 0x595c126893e0 <col:56> 'bool' lvalue ParmVar 0x595c12688190 'y' 'bool'
// `-AnnotateAttr 0x595c12689318 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:_Bool|END"
