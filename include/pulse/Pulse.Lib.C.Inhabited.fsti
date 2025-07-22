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