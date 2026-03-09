module Pulse.Lib.C.Inhabited

class inhabited (a:Type) = {
  witness : a
}

instance inhabited_pair (a:Type) (b:Type) {| wa : inhabited a |} {| wb : inhabited b |}
: inhabited (a * b) = {
  witness = (witness, witness)
}

instance inhabited_option (a:Type) : inhabited (option a) = {
  witness = None
}

instance inhabited_uint8 : inhabited FStar.UInt8.t = {
  witness = 0uy
}

/// Values of this type have a zero default corresponding to the C zero bitpattern.
class has_zero_default (a:Type) = {
  zero_default : a
}

instance has_zero_default_bool : has_zero_default bool = {
  zero_default = false
}

instance has_zero_default_int32 : has_zero_default FStar.Int32.t = {
  zero_default = FStar.Int32.int_to_t 0
}

instance has_zero_default_uint32 : has_zero_default FStar.UInt32.t = {
  zero_default = FStar.UInt32.uint_to_t 0
}

instance has_zero_default_uint8 : has_zero_default FStar.UInt8.t = {
  zero_default = 0uy
}

instance has_zero_default_size_t : has_zero_default FStar.SizeT.t = {
  zero_default = 0sz
}