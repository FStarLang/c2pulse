module Issue51_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn cmp1
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.eq x y));
}

fn cmp2
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.gte x y));
}

fn cmp3
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.gt x y));
}

fn cmp4
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.lte x y));
}

fn cmp5
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.lt x y));
}

fn cmp6
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (not (Int32.eq x y)));
}

fn cmp7
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (Int64.eq x y)));
}

fn cmp8
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (Int64.gte x y)));
}

fn cmp9
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (Int64.gt x y)));
}

fn cmp10
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (Int64.lte x y)));
}

fn cmp11
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (Int64.lt x y)));
}

fn cmp12
(x : Int64.t)
(y : Int64.t)
returns Int64.t
{
(int32_to_int64 (bool_to_int32 (not (Int64.eq x y))));
}

fn cmp13
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.eq x y));
}

fn cmp14
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.gte x y));
}

fn cmp15
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.gt x y));
}

fn cmp16
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.lte x y));
}

fn cmp17
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (Int32.lt x y));
}

fn cmp18
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
(bool_to_int32 (not (Int32.eq x y)));
}

fn cmp19
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (Int32.eq (int16_to_int32 x) (int16_to_int32 y))));
}

fn cmp20
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (Int32.gte (int16_to_int32 x) (int16_to_int32 y))));
}

fn cmp21
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (Int32.gt (int16_to_int32 x) (int16_to_int32 y))));
}

fn cmp22
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (Int32.lte (int16_to_int32 x) (int16_to_int32 y))));
}

fn cmp23
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (Int32.lt (int16_to_int32 x) (int16_to_int32 y))));
}

fn cmp24
(x : Int16.t)
(y : Int16.t)
returns Int16.t
{
(int32_to_int16 (bool_to_int32 (not (Int32.eq (int16_to_int32 x) (int16_to_int32 y)))));
}

fn cmp25
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (Int32.eq (int8_to_int32 x) (int8_to_int32 y))));
}

fn cmp26
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (Int32.gte (int8_to_int32 x) (int8_to_int32 y))));
}

fn cmp27
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (Int32.gt (int8_to_int32 x) (int8_to_int32 y))));
}

fn cmp28
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (Int32.lte (int8_to_int32 x) (int8_to_int32 y))));
}

fn cmp29
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (Int32.lt (int8_to_int32 x) (int8_to_int32 y))));
}

fn cmp30
(x : Int8.t)
(y : Int8.t)
returns Int8.t
{
(int32_to_int8 (bool_to_int32 (not (Int32.eq (int8_to_int32 x) (int8_to_int32 y)))));
}

fn cmp31
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (UInt64.eq x y)));
}

fn cmp32
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (UInt64.gte x y)));
}

fn cmp33
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (UInt64.gt x y)));
}

fn cmp34
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (UInt64.lte x y)));
}

fn cmp35
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (UInt64.lt x y)));
}

fn cmp36
(x : UInt64.t)
(y : UInt64.t)
returns UInt64.t
{
(int32_to_uint64 (bool_to_int32 (not (UInt64.eq x y))));
}

fn cmp37
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (UInt32.eq x y)));
}

fn cmp38
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (UInt32.gte x y)));
}

fn cmp39
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (UInt32.gt x y)));
}

fn cmp40
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (UInt32.lte x y)));
}

fn cmp41
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (UInt32.lt x y)));
}

fn cmp42
(x : UInt32.t)
(y : UInt32.t)
returns UInt32.t
{
(int32_to_uint32 (bool_to_int32 (not (UInt32.eq x y))));
}

fn cmp43
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (Int32.eq (uint16_to_int32 x) (uint16_to_int32 y))));
}

fn cmp44
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (Int32.gte (uint16_to_int32 x) (uint16_to_int32 y))));
}

fn cmp45
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (Int32.gt (uint16_to_int32 x) (uint16_to_int32 y))));
}

fn cmp46
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (Int32.lte (uint16_to_int32 x) (uint16_to_int32 y))));
}

fn cmp47
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (Int32.lt (uint16_to_int32 x) (uint16_to_int32 y))));
}

fn cmp48
(x : UInt16.t)
(y : UInt16.t)
returns UInt16.t
{
(int32_to_uint16 (bool_to_int32 (not (Int32.eq (uint16_to_int32 x) (uint16_to_int32 y)))));
}

fn cmp49
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (Int32.eq (uint8_to_int32 x) (uint8_to_int32 y))));
}

fn cmp50
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (Int32.gte (uint8_to_int32 x) (uint8_to_int32 y))));
}

fn cmp51
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (Int32.gt (uint8_to_int32 x) (uint8_to_int32 y))));
}

fn cmp52
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (Int32.lte (uint8_to_int32 x) (uint8_to_int32 y))));
}

fn cmp53
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (Int32.lt (uint8_to_int32 x) (uint8_to_int32 y))));
}

fn cmp54
(x : UInt8.t)
(y : UInt8.t)
returns UInt8.t
{
(int32_to_uint8 (bool_to_int32 (not (Int32.eq (uint8_to_int32 x) (uint8_to_int32 y)))));
}

fn cmp55
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (Int32.eq (bool_to_int32 x) (bool_to_int32 y))));
}

fn cmp56
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (Int32.gte (bool_to_int32 x) (bool_to_int32 y))));
}

fn cmp57
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (Int32.gt (bool_to_int32 x) (bool_to_int32 y))));
}

fn cmp58
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (Int32.lte (bool_to_int32 x) (bool_to_int32 y))));
}

fn cmp59
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (Int32.lt (bool_to_int32 x) (bool_to_int32 y))));
}

fn cmp60
(x : bool)
(y : bool)
returns _Bool
{
(int32_to_bool (bool_to_int32 (not (Int32.eq (bool_to_int32 x) (bool_to_int32 y)))));
}
