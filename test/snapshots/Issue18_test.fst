module Issue18_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test ()
requires emp
returns i:int32
ensures emp
{
0l;
}

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

let is_diag_point (p:ref point) (v:int32) : slprop = point_pred p {px=v; py=v} let is_point (p:ref point) (xy : (int & int)) : slprop = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy) ghost fn fold_is_point (p:ref point) (#s:point_spec) requires point_pred p s ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py)) { fold (is_point p (as_int s.px, as_int s.py)); }
fn move
(p : ref point)
(dx : Int32.t)
(dy : Int32.t)
(#v:erased _)
requires is_point p v
requires pure <| fits (+) (fst v) (as_int dx)
requires pure <| fits (+) (snd v) (as_int dy)
ensures is_point p (fst v + as_int dx, snd v + as_int dy)
{
let mut p : (ref point) = p;
let mut dx : Int32.t = dx;
let mut dy : Int32.t = dy;
unfold(is_point); point_explode !p;
Mkpoint?.px (! (! p)) := (Int32.add (! (! (! p)).px) (! dx));
Mkpoint?.py (! (! p)) := (Int32.add (! (! (! p)).py) (! dy));
point_recover !p; fold_is_point !p;
}

//Dumping the Clang AST.
// FunctionDecl 0x5e875195b780 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:8:1, line:11:1> line:8:5 test 'int (void)'
// |-CompoundStmt 0x5e875195b9d0 <line:9:1, line:11:1>
// | `-ReturnStmt 0x5e875195b9c0 <line:10:3, col:10>
// |   `-IntegerLiteral 0x5e875195b9a0 <col:10> 'int' 0
// |-AnnotateAttr 0x5e875195b828 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:3:23, col:61> pulse "requires:emp|END"
// |-AnnotateAttr 0x5e875195b8c0 <line:5:22, col:58> pulse "returns:i:int32|END"
// `-AnnotateAttr 0x5e875195b930 <line:4:24, col:71> pulse "ensures:emp|END"
// RecordDecl 0x5e875195b9e8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:13:9, line:16:1> line:13:16 struct _point definition
// |-FieldDecl 0x5e875195baa8 <line:14:3, col:7> col:7 referenced px 'int'
// `-FieldDecl 0x5e875195bb10 <line:15:3, col:7> col:7 referenced py 'int'
// TypedefDecl 0x5e875195bbb8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:13:1, line:16:3> col:3 referenced point 'struct _point'
// `-ElaboratedType 0x5e875195bb60 'struct _point' sugar
//   `-RecordType 0x5e875195ba70 'struct _point'
//     `-Record 0x5e875195b9e8 '_point'
// FunctionDecl 0x5e875195e710 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:41:1, line:47:1> line:41:6 move 'void (point *, int, int)'
// |-ParmVarDecl 0x5e875195e4d8 <col:11, col:18> col:18 used p 'point *'
// |-ParmVarDecl 0x5e875195e558 <col:21, col:25> col:25 used dx 'int'
// |-ParmVarDecl 0x5e875195e5d8 <col:29, col:33> col:33 used dy 'int'
// |-CompoundStmt 0x5e875195ef78 <line:42:1, line:47:1>
// | |-AttributedStmt 0x5e875195eba0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:43:44>
// | | |-AnnotateAttr 0x5e875195eb18 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:unfold(is_point); point_explode !p|END"
// | | `-NullStmt 0x5e875195eb10 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:43:44>
// | |-BinaryOperator 0x5e875195ecf8 <line:44:3, col:19> 'int' '='
// | | |-MemberExpr 0x5e875195ebf0 <col:3, col:6> 'int' lvalue ->px 0x5e875195baa8
// | | | `-ImplicitCastExpr 0x5e875195ebd8 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5e875195ebb8 <col:3> 'point *' lvalue ParmVar 0x5e875195e4d8 'p' 'point *'
// | | `-BinaryOperator 0x5e875195ecd8 <col:11, col:19> 'int' '+'
// | |   |-ImplicitCastExpr 0x5e875195eca8 <col:11, col:14> 'int' <LValueToRValue>
// | |   | `-MemberExpr 0x5e875195ec58 <col:11, col:14> 'int' lvalue ->px 0x5e875195baa8
// | |   |   `-ImplicitCastExpr 0x5e875195ec40 <col:11> 'point *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x5e875195ec20 <col:11> 'point *' lvalue ParmVar 0x5e875195e4d8 'p' 'point *'
// | |   `-ImplicitCastExpr 0x5e875195ecc0 <col:19> 'int' <LValueToRValue>
// | |     `-DeclRefExpr 0x5e875195ec88 <col:19> 'int' lvalue ParmVar 0x5e875195e558 'dx' 'int'
// | |-BinaryOperator 0x5e875195ee58 <line:45:3, col:19> 'int' '='
// | | |-MemberExpr 0x5e875195ed50 <col:3, col:6> 'int' lvalue ->py 0x5e875195bb10
// | | | `-ImplicitCastExpr 0x5e875195ed38 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5e875195ed18 <col:3> 'point *' lvalue ParmVar 0x5e875195e4d8 'p' 'point *'
// | | `-BinaryOperator 0x5e875195ee38 <col:11, col:19> 'int' '+'
// | |   |-ImplicitCastExpr 0x5e875195ee08 <col:11, col:14> 'int' <LValueToRValue>
// | |   | `-MemberExpr 0x5e875195edb8 <col:11, col:14> 'int' lvalue ->py 0x5e875195bb10
// | |   |   `-ImplicitCastExpr 0x5e875195eda0 <col:11> 'point *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x5e875195ed80 <col:11> 'point *' lvalue ParmVar 0x5e875195e4d8 'p' 'point *'
// | |   `-ImplicitCastExpr 0x5e875195ee20 <col:19> 'int' <LValueToRValue>
// | |     `-DeclRefExpr 0x5e875195ede8 <col:19> 'int' lvalue ParmVar 0x5e875195e5d8 'dy' 'int'
// | `-AttributedStmt 0x5e875195ef60 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:46:44>
// |   |-AnnotateAttr 0x5e875195eed0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_recover !p; fold_is_point !p|END"
// |   `-NullStmt 0x5e875195eec8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue18_test.c:46:44>
// |-AnnotateAttr 0x5e875195e7d0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#v:erased _|END"
// |-AnnotateAttr 0x5e875195e880 <line:3:23, col:61> pulse "requires:is_point p v|END"
// |-AnnotateAttr 0x5e875195e900 <col:23, col:61> pulse "requires:pure <| fits (+) (fst v) (as_int dx)|END"
// |-AnnotateAttr 0x5e875195e990 <col:23, col:61> pulse "requires:pure <| fits (+) (snd v) (as_int dy)|END"
// `-AnnotateAttr 0x5e875195ea20 <line:4:24, col:71> pulse "ensures:is_point p (fst v + as_int dx, snd v + as_int dy)|END"
