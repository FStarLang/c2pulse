module Stack_allocated_point

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type point = {
px: ref Int32.t;
py: ref Int32.t;
}

[@@erasable]
noeq
type point_spec = {
px : Int32.t;
py : Int32.t

}

let point_pred (x:ref point) (s:point_spec) : slprop =
exists* (y: point). (x |-> y) **
(y.px |-> s.px) **
(y.py |-> s.py)

assume val point_spec_default : point_spec

assume val point_default (point_var_spec:point_spec) : point

ghost
fn point_pack (point_var:ref point) (#point_var_spec:point_spec)
requires point_var|-> point_default point_var_spec
ensures exists* v. point_pred point_var v ** pure (v == point_var_spec)
{ admit() }

ghost
fn point_unpack (point_var:ref point)
requires exists* v. point_pred point_var v 
ensures exists* u. point_var |-> u
{ admit() }

fn point_alloc ()
returns x:ref point
ensures freeable x
ensures exists* v. point_pred x v
{ admit () }

fn point_free (x:ref point)
requires freeable x
requires exists* v. point_pred x v
{ admit() }


ghost fn point_explode (x:ref point) (#s:point_spec)
requires point_pred x s
ensures exists* (v: point). (x |-> v) ** (v.px |-> s.px) ** (v.py |-> s.py)
{unfold point_pred}


ghost
fn point_recover (x:ref point) (#a0 : Int32.t) (#a1 : Int32.t) 
requires exists* (y: point). (x |-> y) ** (y.px |-> a0) ** (y.py |-> a1)
ensures exists* w. point_pred x w ** pure (w == {px = a0; py = a1})
{fold point_pred x ({px = a0; py = a1}) }

fn test_stack_allocator ()
requires emp
ensures emp
{
let mut p : point = point_default point_spec_default;
point_pack p;
point_explode p;
(! p).px := 1l;
(! p).py := 2l;
point_recover p;
point_unpack p;
}

//Dumping the Clang AST.
// RecordDecl 0x5ab2c7408bc8 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:5:9, line:8:1> line:5:16 struct _point definition
// |-FieldDecl 0x5ab2c7408c88 <line:6:3, col:7> col:7 referenced px 'int'
// `-FieldDecl 0x5ab2c7408cf0 <line:7:3, col:7> col:7 referenced py 'int'
// TypedefDecl 0x5ab2c7408d98 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:5:1, line:8:3> col:3 referenced point 'struct _point'
// `-ElaboratedType 0x5ab2c7408d40 'struct _point' sugar
//   `-RecordType 0x5ab2c7408c50 'struct _point'
//     `-Record 0x5ab2c7408bc8 '_point'
// FunctionDecl 0x5ab2c7408ec0 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:13:1, line:24:1> line:13:6 test_stack_allocator 'void ()'
// |-CompoundStmt 0x5ab2c740c4c8 <line:14:1, line:24:1>
// | |-DeclStmt 0x5ab2c7409178 <line:15:3, col:10>
// | | `-VarDecl 0x5ab2c74090e0 <col:3, col:9> col:9 used p 'point':'struct _point'
// | |-AttributedStmt 0x5ab2c7409240 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:18:22>
// | | |-AnnotateAttr 0x5ab2c74091d0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_pack p|END"
// | | `-NullStmt 0x5ab2c74091c8 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:18:22>
// | |-AttributedStmt 0x5ab2c7409320 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:19:25>
// | | |-AnnotateAttr 0x5ab2c74092a0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_explode p|END"
// | | `-NullStmt 0x5ab2c7409298 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:19:25>
// | |-BinaryOperator 0x5ab2c74093a8 <line:20:3, col:10> 'int' '='
// | | |-MemberExpr 0x5ab2c7409358 <col:3, col:5> 'int' lvalue .px 0x5ab2c7408c88
// | | | `-DeclRefExpr 0x5ab2c7409338 <col:3> 'point':'struct _point' lvalue Var 0x5ab2c74090e0 'p' 'point':'struct _point'
// | | `-IntegerLiteral 0x5ab2c7409388 <col:10> 'int' 1
// | |-BinaryOperator 0x5ab2c7409498 <line:21:3, col:10> 'int' '='
// | | |-MemberExpr 0x5ab2c7409448 <col:3, col:5> 'int' lvalue .py 0x5ab2c7408cf0
// | | | `-DeclRefExpr 0x5ab2c7409428 <col:3> 'point':'struct _point' lvalue Var 0x5ab2c74090e0 'p' 'point':'struct _point'
// | | `-IntegerLiteral 0x5ab2c7409478 <col:10> 'int' 2
// | |-AttributedStmt 0x5ab2c7409580 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:22:25>
// | | |-AnnotateAttr 0x5ab2c7409500 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_recover p|END"
// | | `-NullStmt 0x5ab2c74094f8 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:22:25>
// | `-AttributedStmt 0x5ab2c740c4b0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:23:24>
// |   |-AnnotateAttr 0x5ab2c740c440 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_unpack p|END"
// |   `-NullStmt 0x5ab2c740c438 </home/t-visinghal/Applications/src/c2pulse/test/general/stack_allocated_point.c:23:24>
// |-AnnotateAttr 0x5ab2c7408f68 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// `-AnnotateAttr 0x5ab2c7409000 <line:4:24, col:71> pulse "ensures:emp|END"
