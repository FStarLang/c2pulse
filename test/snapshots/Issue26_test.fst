module Issue26_test

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

let is_point (p:ref point) (xy : (int & int)) : slprop = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
ghost fn fold_is_point (p:ref point) (#s:point_spec) requires point_pred p s ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py)) { fold (is_point p (as_int s.px, as_int s.py)); }
fn move_alt
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

fn create_point
(x : Int32.t)
(y : Int32.t)
returns p:ref point
ensures is_point p (as_int x, as_int y)
ensures freeable p
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
let p0 : (ref point) = point_alloc ();
let mut p : (ref point) = p0;
point_explode !p;
Mkpoint?.px (! (! p)) := (! x);
Mkpoint?.py (! (! p)) := (! y);
point_recover !p; fold_is_point !p;
(! p);
}

fn create_and_move ()
{
let p1 : (ref point) = (create_point 0l 0l);
let mut p : (ref point) = p1;
(move_alt (! p) 1l 1l);
unfold(is_point); point_explode !p; point_recover !p;
(point_free (! p));
}

//Dumping the Clang AST.
// RecordDecl 0x5f2251031de8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:5:9, line:8:1> line:5:16 struct _point definition
// |-FieldDecl 0x5f2251031ea8 <line:6:3, col:7> col:7 referenced px 'int'
// `-FieldDecl 0x5f2251031f10 <line:7:3, col:7> col:7 referenced py 'int'
// TypedefDecl 0x5f2251031fb8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:5:1, line:8:3> col:3 referenced point 'struct _point'
// `-ElaboratedType 0x5f2251031f60 'struct _point' sugar
//   `-RecordType 0x5f2251031e70 'struct _point'
//     `-Record 0x5f2251031de8 '_point'
// FunctionDecl 0x5f2251035ea0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:32:1, line:38:1> line:32:6 used move_alt 'void (point *, int, int)'
// |-ParmVarDecl 0x5f2251035c68 <col:15, col:22> col:22 used p 'point *'
// |-ParmVarDecl 0x5f2251035ce8 <col:25, col:29> col:29 used dx 'int'
// |-ParmVarDecl 0x5f2251035d68 <col:33, col:37> col:37 used dy 'int'
// |-CompoundStmt 0x5f2251036708 <line:33:1, line:38:1>
// | |-AttributedStmt 0x5f2251036330 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:34:44>
// | | |-AnnotateAttr 0x5f22510362a8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:unfold(is_point); point_explode !p|END"
// | | `-NullStmt 0x5f22510362a0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:34:44>
// | |-BinaryOperator 0x5f2251036488 <line:35:3, col:19> 'int' '='
// | | |-MemberExpr 0x5f2251036380 <col:3, col:6> 'int' lvalue ->px 0x5f2251031ea8
// | | | `-ImplicitCastExpr 0x5f2251036368 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5f2251036348 <col:3> 'point *' lvalue ParmVar 0x5f2251035c68 'p' 'point *'
// | | `-BinaryOperator 0x5f2251036468 <col:11, col:19> 'int' '+'
// | |   |-ImplicitCastExpr 0x5f2251036438 <col:11, col:14> 'int' <LValueToRValue>
// | |   | `-MemberExpr 0x5f22510363e8 <col:11, col:14> 'int' lvalue ->px 0x5f2251031ea8
// | |   |   `-ImplicitCastExpr 0x5f22510363d0 <col:11> 'point *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x5f22510363b0 <col:11> 'point *' lvalue ParmVar 0x5f2251035c68 'p' 'point *'
// | |   `-ImplicitCastExpr 0x5f2251036450 <col:19> 'int' <LValueToRValue>
// | |     `-DeclRefExpr 0x5f2251036418 <col:19> 'int' lvalue ParmVar 0x5f2251035ce8 'dx' 'int'
// | |-BinaryOperator 0x5f22510365e8 <line:36:3, col:19> 'int' '='
// | | |-MemberExpr 0x5f22510364e0 <col:3, col:6> 'int' lvalue ->py 0x5f2251031f10
// | | | `-ImplicitCastExpr 0x5f22510364c8 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5f22510364a8 <col:3> 'point *' lvalue ParmVar 0x5f2251035c68 'p' 'point *'
// | | `-BinaryOperator 0x5f22510365c8 <col:11, col:19> 'int' '+'
// | |   |-ImplicitCastExpr 0x5f2251036598 <col:11, col:14> 'int' <LValueToRValue>
// | |   | `-MemberExpr 0x5f2251036548 <col:11, col:14> 'int' lvalue ->py 0x5f2251031f10
// | |   |   `-ImplicitCastExpr 0x5f2251036530 <col:11> 'point *' <LValueToRValue>
// | |   |     `-DeclRefExpr 0x5f2251036510 <col:11> 'point *' lvalue ParmVar 0x5f2251035c68 'p' 'point *'
// | |   `-ImplicitCastExpr 0x5f22510365b0 <col:19> 'int' <LValueToRValue>
// | |     `-DeclRefExpr 0x5f2251036578 <col:19> 'int' lvalue ParmVar 0x5f2251035d68 'dy' 'int'
// | `-AttributedStmt 0x5f22510366f0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:37:44>
// |   |-AnnotateAttr 0x5f2251036660 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_recover !p; fold_is_point !p|END"
// |   `-NullStmt 0x5f2251036658 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:37:44>
// |-AnnotateAttr 0x5f2251035f60 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:9:27, col:68> pulse "erased_arg:#v:erased _|END"
// |-AnnotateAttr 0x5f2251036010 <line:3:23, col:61> pulse "requires:is_point p v|END"
// |-AnnotateAttr 0x5f2251036090 <col:23, col:61> pulse "requires:pure <| fits (+) (fst v) (as_int dx)|END"
// |-AnnotateAttr 0x5f2251036120 <col:23, col:61> pulse "requires:pure <| fits (+) (snd v) (as_int dy)|END"
// `-AnnotateAttr 0x5f22510361b0 <line:4:24, col:71> pulse "ensures:is_point p (fst v + as_int dx, snd v + as_int dy)|END"
// FunctionDecl 0x5f22510369b8 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:44:1, line:52:1> line:44:8 used create_point 'point *(int, int)'
// |-ParmVarDecl 0x5f2251036810 <col:21, col:25> col:25 used x 'int'
// |-ParmVarDecl 0x5f2251036890 <col:28, col:32> col:32 used y 'int'
// |-CompoundStmt 0x5f2251038180 <line:45:1, line:52:1>
// | |-DeclStmt 0x5f2251037dc8 <line:46:3, col:43>
// | | `-VarDecl 0x5f2251037c58 <col:3, col:42> col:10 used p 'point *' cinit
// | |   `-CStyleCastExpr 0x5f2251037da0 <col:14, col:42> 'point *' <BitCast>
// | |     `-CallExpr 0x5f2251037d58 <col:22, col:42> 'void *'
// | |       |-ImplicitCastExpr 0x5f2251037d40 <col:22> 'void *(*)(unsigned long)' <FunctionToPointerDecay>
// | |       | `-DeclRefExpr 0x5f2251037cc0 <col:22> 'void *(unsigned long)' Function 0x5f2250ef5c20 'malloc' 'void *(unsigned long)'
// | |       `-UnaryExprOrTypeTraitExpr 0x5f2251037cf0 <col:29, col:41> 'unsigned long' sizeof 'point':'struct _point'
// | |-AttributedStmt 0x5f2251037ea0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:47:26>
// | | |-AnnotateAttr 0x5f2251037e28 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_explode !p|END"
// | | `-NullStmt 0x5f2251037e20 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:47:26>
// | |-BinaryOperator 0x5f2251037f58 <line:48:3, col:11> 'int' '='
// | | |-MemberExpr 0x5f2251037ef0 <col:3, col:6> 'int' lvalue ->px 0x5f2251031ea8
// | | | `-ImplicitCastExpr 0x5f2251037ed8 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5f2251037eb8 <col:3> 'point *' lvalue Var 0x5f2251037c58 'p' 'point *'
// | | `-ImplicitCastExpr 0x5f2251037f40 <col:11> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5f2251037f20 <col:11> 'int' lvalue ParmVar 0x5f2251036810 'x' 'int'
// | |-BinaryOperator 0x5f2251038018 <line:49:3, col:11> 'int' '='
// | | |-MemberExpr 0x5f2251037fb0 <col:3, col:6> 'int' lvalue ->py 0x5f2251031f10
// | | | `-ImplicitCastExpr 0x5f2251037f98 <col:3> 'point *' <LValueToRValue>
// | | |   `-DeclRefExpr 0x5f2251037f78 <col:3> 'point *' lvalue Var 0x5f2251037c58 'p' 'point *'
// | | `-ImplicitCastExpr 0x5f2251038000 <col:11> 'int' <LValueToRValue>
// | |   `-DeclRefExpr 0x5f2251037fe0 <col:11> 'int' lvalue ParmVar 0x5f2251036890 'y' 'int'
// | |-AttributedStmt 0x5f2251038120 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:50:44>
// | | |-AnnotateAttr 0x5f2251038090 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:point_recover !p; fold_is_point !p|END"
// | | `-NullStmt 0x5f2251038088 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:50:44>
// | `-ReturnStmt 0x5f2251038170 <line:51:3, col:10>
// |   `-ImplicitCastExpr 0x5f2251038158 <col:10> 'point *' <LValueToRValue>
// |     `-DeclRefExpr 0x5f2251038138 <col:10> 'point *' lvalue Var 0x5f2251037c58 'p' 'point *'
// |-AnnotateAttr 0x5f2251037aa0 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:5:22, col:58> pulse "returns:p:ref point|END"
// |-AnnotateAttr 0x5f2251037b40 <line:4:24, col:71> pulse "ensures:is_point p (as_int x, as_int y)|END"
// `-AnnotateAttr 0x5f2251037bd0 <col:24, col:71> pulse "ensures:freeable p|END"
// FunctionDecl 0x5f2251038210 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:55:1, line:61:1> line:55:6 create_and_move 'void ()'
// `-CompoundStmt 0x5f2251038838 <line:56:1, line:61:1>
//   |-DeclStmt 0x5f2251038450 <line:57:3, col:32>
//   | `-VarDecl 0x5f22510382d0 <col:3, col:31> col:10 used p 'point *' cinit
//   |   `-CallExpr 0x5f2251038418 <col:14, col:31> 'point *'
//   |     |-ImplicitCastExpr 0x5f2251038400 <col:14> 'point *(*)(int, int)' <FunctionToPointerDecay>
//   |     | `-DeclRefExpr 0x5f2251038338 <col:14> 'point *(int, int)' Function 0x5f22510369b8 'create_point' 'point *(int, int)'
//   |     |-IntegerLiteral 0x5f2251038358 <col:27> 'int' 0
//   |     `-IntegerLiteral 0x5f2251038378 <col:30> 'int' 0
//   |-CallExpr 0x5f2251038568 <line:58:3, col:19> 'void'
//   | |-ImplicitCastExpr 0x5f2251038550 <col:3> 'void (*)(point *, int, int)' <FunctionToPointerDecay>
//   | | `-DeclRefExpr 0x5f2251038468 <col:3> 'void (point *, int, int)' Function 0x5f2251035ea0 'move_alt' 'void (point *, int, int)'
//   | |-ImplicitCastExpr 0x5f22510385a8 <col:12> 'point *' <LValueToRValue>
//   | | `-DeclRefExpr 0x5f2251038488 <col:12> 'point *' lvalue Var 0x5f22510382d0 'p' 'point *'
//   | |-IntegerLiteral 0x5f22510384a8 <col:15> 'int' 1
//   | `-IntegerLiteral 0x5f22510384c8 <col:18> 'int' 1
//   |-AttributedStmt 0x5f2251038720 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:59:62>
//   | |-AnnotateAttr 0x5f2251038688 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/../include/PulseMacros.h:8:20, col:54> pulse "lemma:unfold(is_point); point_explode !p; point_recover !p|END"
//   | `-NullStmt 0x5f2251038680 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue26_test.c:59:62>
//   `-CallExpr 0x5f22510387d8 <line:60:3, col:9> 'void'
//     |-ImplicitCastExpr 0x5f22510387c0 <col:3> 'void (*)(void *)' <FunctionToPointerDecay>
//     | `-DeclRefExpr 0x5f2251038738 <col:3> 'void (void *)' Function 0x5f2250ef6b98 'free' 'void (void *)'
//     `-ImplicitCastExpr 0x5f2251038820 <col:8> 'void *' <BitCast>
//       `-ImplicitCastExpr 0x5f2251038808 <col:8> 'point *' <LValueToRValue>
//         `-DeclRefExpr 0x5f2251038758 <col:8> 'point *' lvalue Var 0x5f22510382d0 'p' 'point *'
