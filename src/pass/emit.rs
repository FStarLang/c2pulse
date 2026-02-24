use std::fmt::Write;
use std::{
    collections::{HashMap, HashSet},
    rc::Rc,
};

use num_bigint::BigInt;
use pretty::{RcDoc, Render, RenderAnnotated};

use crate::{
    env::{Env, LocalDecl, LocalDeclKind},
    ir::*,
    mayberc::MaybeRc,
};

pub type SourceRangeMap = Vec<(Location, Range)>;

type Annotation = Rc<SourceInfo>;
type Doc = RcDoc<'static, Annotation>;

/// Tracks whether an emitted expression is an F* lvalue (ref a) or rvalue (a).
enum ExprKind {
    LValue(Doc),
    RValue(Doc),
}

impl ExprKind {
    /// Convert to an rvalue (F* type `a`). Dereferences lvalues with `!`.
    fn to_rvalue(self) -> Doc {
        match self {
            ExprKind::LValue(doc) => parens(Doc::text("!").append(doc)),
            ExprKind::RValue(doc) => doc,
        }
    }
}

struct StrWriter {
    buffer: String,
    line: usize,
    character: usize,

    source_range_map: SourceRangeMap,
    annotation_stack: Vec<(Annotation, Position)>,
}
impl StrWriter {
    fn new() -> StrWriter {
        StrWriter {
            buffer: String::default(),
            line: 0,
            character: 0,
            source_range_map: vec![],
            annotation_stack: vec![],
        }
    }
    fn cur_pos(&self) -> Position {
        Position {
            line: self.line as u32 + 1,
            character: self.character as u32 + 1,
        }
    }
}
impl Render for StrWriter {
    type Error = ();

    fn write_str(&mut self, s: &str) -> Result<usize, Self::Error> {
        self.buffer.push_str(s);

        let mut first = true;
        for line in s.split('\n') {
            if first {
                first = false
            } else {
                self.line += 1;
                self.character = 0;
            }
            self.character += line.len();
        }

        Ok(s.len())
    }

    fn fail_doc(&self) -> Self::Error {
        ()
    }
}
impl<'a> RenderAnnotated<'a, Annotation> for StrWriter {
    fn push_annotation(&mut self, annotation: &'a Annotation) -> Result<(), Self::Error> {
        self.annotation_stack
            .push((annotation.clone(), self.cur_pos()));
        Ok(())
    }

    fn pop_annotation(&mut self) -> Result<(), Self::Error> {
        let (loc, start) = self.annotation_stack.pop().unwrap();
        self.source_range_map.push((
            loc.location().clone(),
            Range {
                start,
                end: self.cur_pos(),
            },
        ));
        Ok(())
    }
}

fn annotated<T>(ast: &Ast<T>, doc: Doc) -> Doc {
    doc.annotate(ast.loc.clone())
}

fn parens(doc: Doc) -> Doc {
    Doc::text("(")
        .append(doc)
        .append(Doc::text(")"))
        .nest(2)
        .group()
}

fn unaryfn(f: Doc, arg: Doc) -> Doc {
    parens(f.append(Doc::line()).append(arg))
}

fn naryfn<T: IntoIterator<Item = Doc>>(args: T) -> Doc {
    parens(Doc::intersperse(args.into_iter(), Doc::line()))
}

fn unaryfn_with_type(f: Doc, arg: Doc, ty: Doc) -> Doc {
    parens(
        f.append(Doc::line()).append(arg).append(
            Doc::line()
                .append("<:")
                .append(Doc::space())
                .append(ty)
                .nest(2)
                .group(),
        ),
    )
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
enum TypeRef {
    Typedef(Rc<IdentT>),
    Struct(Rc<IdentT>),
}
impl From<&TypeRefKind> for TypeRef {
    fn from(tr: &TypeRefKind) -> Self {
        match tr {
            TypeRefKind::Typedef(t) => TypeRef::Typedef(t.val.clone()),
            TypeRefKind::Struct(s) => TypeRef::Struct(s.val.clone()),
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
enum Name {
    Var(Rc<IdentT>),
    Fn(Rc<IdentT>),
    TypeRef(TypeRef),
    TypeRefPred(TypeRef),
    TypeRefPredPost(TypeRef),

    StructFieldProj(Rc<IdentT>, Rc<IdentT>),
    StructDirectFieldName(Rc<IdentT>, Rc<IdentT>),
    StructGhostFieldProj(Rc<IdentT>, Rc<IdentT>),
    StructAuxFn(Rc<IdentT>, String),
}
impl Name {
    fn to_string(&self) -> String {
        fn struct_to_string(ident: &Rc<IdentT>) -> String {
            Name::TypeRef(TypeRef::Struct(ident.clone())).to_string()
        }
        fn typeref_to_string(typeref: &TypeRef) -> String {
            Name::TypeRef(typeref.clone()).to_string()
        }

        match self {
            Name::Var(v) => {
                let v: &str = v;
                match v {
                    "this" | "return" => v.into(),
                    _ => format!("var_{}", v),
                }
            }
            Name::Fn(v) => format!("func_{}", v),
            Name::TypeRef(TypeRef::Struct(str)) => format!("struct_{}", str),
            Name::TypeRef(TypeRef::Typedef(ty)) => format!("ty_{}", ty),
            Name::TypeRefPred(type_ref) => format!("{}__pred", typeref_to_string(type_ref)),
            Name::TypeRefPredPost(type_ref) => format!("{}__post", typeref_to_string(type_ref)),
            Name::StructFieldProj(str, fld) => format!("{}__get_{}", struct_to_string(str), fld),
            Name::StructDirectFieldName(str, fld) => format!("{}__{}", struct_to_string(str), fld),
            Name::StructGhostFieldProj(str, fld) => format!("{}__{}", struct_to_string(str), fld),
            Name::StructAuxFn(str, f) => format!("{}__aux_{}", struct_to_string(str), f),
        }
    }
}

const RESERVED: &[&str] = &[
    "fn", // keywords
    "assume",
    "requires",
    "ensures",
    "preserves",
    "continue",
    "break",
    "label",
    "goto",
    "return",
    "stt", // library names
    "stt_ghost",
    "stt_atomic",
    "ref",
    "array",
    "admit",
    "bool",
    "unit",
    "slprop",
    "emp",
    "emp_inames",
    "int",
    "nat",
    "not",
    "pulse_eager_unfold",
    "pulse_intro",
];

struct NameMangling {
    map: HashMap<Name, Rc<str>>,
    used: HashSet<Rc<str>>,
}
impl NameMangling {
    fn new() -> Self {
        NameMangling {
            map: HashMap::new(),
            used: RESERVED.iter().map(|r| Rc::from(*r)).collect(),
        }
    }

    fn pick_new(&mut self, mut base: String) -> Rc<str> {
        if !self.used.contains(base.as_str()) {
            return Rc::from(base);
        }
        let base_init_len = base.len();
        for i in 1.. {
            base.truncate(base_init_len);
            write!(base, "_{}", i).unwrap();
            if !self.used.contains(base.as_str()) {
                return Rc::from(base);
            }
        }
        unreachable!()
    }

    fn mangle(&mut self, name: &Name) -> Rc<str> {
        if let Some(mangled) = self.map.get(name) {
            return mangled.clone();
        }

        let mangled = self.pick_new(name.to_string().to_lowercase());
        self.used.insert(mangled.clone());
        self.map.insert(name.clone(), mangled.clone());
        mangled
    }

    fn emit(&mut self, name: Name) -> Doc {
        Doc::text(self.mangle(&name).to_string())
    }
}

fn emit_type(env: &Env, nm: &mut NameMangling, ty: &Type) -> Doc {
    annotated(ty, {
        match &ty.val {
            TypeT::Void => Doc::text("unit"),

            // TODO: support all widths
            TypeT::Int {
                signed: false,
                width,
            } => Doc::text(format!("UInt{}.t", width)),
            TypeT::Int {
                signed: true,
                width,
            } => Doc::text(format!("Int{}.t", width)),

            TypeT::Bool => Doc::text("bool"),
            TypeT::SizeT => Doc::text("SizeT.t"),

            TypeT::Pointer(to, PointerKind::Array) => {
                unaryfn(Doc::text("array"), emit_type(env, nm, to))
            }
            TypeT::Pointer(to, PointerKind::Ref | PointerKind::Unknown) => {
                unaryfn(Doc::text("ref"), emit_type(env, nm, to))
            }
            TypeT::Error => Doc::text("unit"),

            TypeT::TypeRef(n) => nm.emit(Name::TypeRef(n.into())),

            TypeT::SLProp => Doc::text("slprop"),
            TypeT::SpecInt => Doc::text("int"),

            TypeT::Requires(ty, _)
            | TypeT::Ensures(ty, _)
            | TypeT::Consumes(ty)
            | TypeT::Plain(ty) => emit_type(env, nm, ty),
        }
    })
}

fn subst_this_rvalue(env: &Env, nm: &mut NameMangling, rvalue: &mut Expr, this: &Rc<Expr>) {
    match &mut rvalue.val {
        ExprT::Var(x) => {
            if &*x.val == "this" {
                *rvalue = (**this).clone()
            }
        }
        ExprT::Deref(rv) => subst_this_rvalue(env, nm, Rc::make_mut(rv), this),
        ExprT::Member(x, _a) => subst_this_rvalue(env, nm, Rc::make_mut(x), this),
        ExprT::BoolLit(_) => {}
        ExprT::IntLit(..) => {}
        ExprT::Ref(lv) => subst_this_rvalue(env, nm, Rc::make_mut(lv), this),
        ExprT::UnOp(_, arg) => {
            subst_this_rvalue(env, nm, Rc::make_mut(arg), this);
        }
        ExprT::BinOp(_, lhs, rhs) => {
            subst_this_rvalue(env, nm, Rc::make_mut(lhs), this);
            subst_this_rvalue(env, nm, Rc::make_mut(rhs), this);
        }
        ExprT::FnCall(_f, args) => {
            for arg in args {
                subst_this_rvalue(env, nm, Rc::make_mut(arg), this);
            }
        }
        ExprT::Cast(val, _) => {
            subst_this_rvalue(env, nm, Rc::make_mut(val), this);
        }
        ExprT::InlinePulse(val, _) => subst_inline_code_this(env, nm, Rc::make_mut(val), this),
        ExprT::Error(_ty) => {}
        ExprT::Live(val) => subst_this_rvalue(env, nm, Rc::make_mut(val), this),
        ExprT::Old(val) => subst_this_rvalue(env, nm, Rc::make_mut(val), this),
        ExprT::StructInit(_, fields) => {
            for (_fld, val) in fields {
                subst_this_rvalue(env, nm, Rc::make_mut(val), this);
            }
        }
    }
}

fn subst_inline_code_this(env: &Env, nm: &mut NameMangling, val: &mut InlineCode, this: &Rc<Expr>) {
    for tok in &mut val.tokens {
        // This is ridiculuously hacky....
        if &*tok.text.val == "this" {
            tok.text.val = Rc::from(emit_rvalue(env, nm, this).pretty(100).to_string());
        }
    }
}

fn emit_type_slprop(
    env: &Env,
    nm: &mut NameMangling,
    ty: &Type,
    req: &mut Vec<Doc>,
    ens: &mut Vec<Doc>,
    this: &Rc<Expr>,
) {
    match &ty.val {
        TypeT::Void
        | TypeT::Bool
        | TypeT::Int { .. }
        | TypeT::SizeT
        | TypeT::SpecInt
        | TypeT::SLProp => {}
        TypeT::Pointer(pointee_ty, kind) => {
            let live = annotated(ty, unaryfn(Doc::text("live"), emit_rvalue(env, nm, this)));
            req.push(live.clone());
            ens.push(live);

            match kind {
                PointerKind::Ref => {
                    let derefed = ExprT::Deref(this.clone()).with_loc(this.loc.clone());
                    emit_type_slprop(env, nm, pointee_ty, req, ens, &derefed);
                }
                _ => {} // TODO
            }
        }
        TypeT::TypeRef(n) => {
            let this = emit_rvalue(env, nm, this);
            req.push(unaryfn(nm.emit(Name::TypeRefPred(n.into())), this.clone()));
            ens.push(unaryfn(
                nm.emit(Name::TypeRefPredPost(n.into())),
                this.clone(),
            ));
        }
        TypeT::Requires(ty, p) => {
            emit_type_slprop(env, nm, ty, req, ens, this);

            let p = &mut p.clone();
            subst_this_rvalue(env, nm, Rc::make_mut(p), this);
            req.push(emit_rvalue(env, nm, p));
        }
        TypeT::Ensures(ty, p) => {
            emit_type_slprop(env, nm, ty, req, ens, this);

            let p = &mut p.clone();
            subst_this_rvalue(env, nm, Rc::make_mut(p), this);
            ens.push(emit_rvalue(env, nm, p));
        }
        TypeT::Consumes(ty) => emit_type_slprop(env, nm, ty, req, &mut vec![], this),
        TypeT::Plain(_) => {}
        TypeT::Error => {}
    }
}

fn emit_var(nm: &mut NameMangling, v: &Ident) -> Doc {
    annotated(v, nm.emit(Name::Var(v.val.clone())))
}

fn emit_lvalue(env: &Env, nm: &mut NameMangling, v: &Expr) -> Doc {
    match emit_expr(env, nm, v) {
        ExprKind::LValue(doc) => doc,
        ExprKind::RValue(_) => Doc::text(format!(
            "(*TODO: cannot produce lvalue for {}*) (admit())",
            v
        )),
    }
}

fn emit_expr(env: &Env, nm: &mut NameMangling, v: &Expr) -> ExprKind {
    match &v.val {
        ExprT::Var(x) => {
            if let Some(LocalDecl {
                kind: LocalDeclKind::RValue,
                ..
            }) = env.lookup_var(x)
            {
                ExprKind::RValue(annotated(v, emit_var(nm, x)))
            } else {
                ExprKind::LValue(annotated(v, emit_var(nm, x)))
            }
        }
        ExprT::Deref(inner) => {
            ExprKind::LValue(annotated(v, emit_expr(env, nm, inner).to_rvalue()))
        }
        ExprT::Member(x, a) => match env.infer_expr(x) {
            Some(ty) => {
                let ty = env.vtype_whnf(ty);
                match &ty.val {
                    TypeT::TypeRef(TypeRefKind::Struct(struct_name)) => match emit_expr(env, nm, x)
                    {
                        ExprKind::LValue(x_doc) => ExprKind::LValue(annotated(
                            v,
                            unaryfn(
                                nm.emit(Name::StructFieldProj(
                                    struct_name.val.clone(),
                                    a.val.clone(),
                                )),
                                x_doc,
                            ),
                        )),
                        ExprKind::RValue(x_doc) => ExprKind::RValue(annotated(
                            v,
                            x_doc.append(Doc::text(".")).append(nm.emit(
                                Name::StructDirectFieldName(struct_name.val.clone(), a.val.clone()),
                            )),
                        )),
                    },
                    _ => ExprKind::RValue(annotated(
                        v,
                        Doc::text(format!("((*TODO struct field access on {}*) admit())", ty)),
                    )),
                }
            }
            None => ExprKind::RValue(annotated(
                v,
                Doc::text(format!("((*TODO cannot infer type of {}*) admit())", x)),
            )),
        },
        _ => ExprKind::RValue(emit_rvalue_inner(env, nm, v)),
    }
}

fn binop(a: Doc, op: Doc, b: Doc) -> Doc {
    parens(
        a.append(Doc::line())
            .append(op)
            .group()
            .append(Doc::line())
            .append(b),
    )
}

fn get_int_mod(signed: &bool, width: &u32) -> Option<&'static str> {
    Some(match (signed, width) {
        (false, 8) => "UInt8",
        (false, 16) => "UInt16",
        (false, 32) => "UInt32",
        (false, 64) => "UInt64",

        (true, 8) => "Int8",
        (true, 16) => "Int16",
        (true, 32) => "Int32",
        (true, 64) => "Int64",

        _ => return None,
    })
}

macro_rules! todo_binop {
    () => {
        return None
    };
}

fn emit_unop(env: &Env, op: UnOp, ty: MaybeRc<Type>) -> Option<Doc> {
    Some(match (op, &env.vtype_whnf(ty).val) {
        (UnOp::Not, _) => Doc::text("not"),
        (UnOp::Neg, TypeT::Int { signed, width }) => {
            let modu = get_int_mod(signed, width)?;
            Doc::text(format!("{}.sub {}.zero", modu, modu))
        }
        (UnOp::Neg, _) => return None,
    })
}

fn emit_binop(env: &Env, op: BinOp, ty: MaybeRc<Type>) -> Option<Doc> {
    Some(match (op, &env.vtype_whnf(ty).val) {
        (BinOp::Eq, TypeT::SLProp | TypeT::Void) => Doc::text("=="),
        (
            BinOp::Eq,
            TypeT::SpecInt | TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT | TypeT::Pointer(_, _),
        ) => Doc::text("="),

        (BinOp::LEq, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.lte`", get_int_mod(signed, width)?))
        }
        (BinOp::Lt, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.lt`", get_int_mod(signed, width)?))
        }
        (BinOp::LEq, TypeT::SizeT) => Doc::text("`SizeT.lte`"),
        (BinOp::Lt, TypeT::SizeT) => Doc::text("`SizeT.lt`"),

        (BinOp::LEq, TypeT::Bool) => todo_binop!(),
        (BinOp::Lt, TypeT::Bool) => todo_binop!(),
        (BinOp::LogAnd, TypeT::Bool) => Doc::text("&&"),
        (BinOp::Div, TypeT::Bool) => todo_binop!(),
        (BinOp::Mod, TypeT::Bool) => todo_binop!(),
        (BinOp::Sub, TypeT::Bool) => todo_binop!(),
        (BinOp::Add, TypeT::Bool) => todo_binop!(),
        (BinOp::Mul, TypeT::Bool) => Doc::text("&&"),

        (BinOp::LogAnd, TypeT::SLProp) => Doc::text("**"),

        (BinOp::Mul, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.mul`", get_int_mod(signed, width)?))
        }
        (BinOp::Mul, TypeT::SizeT) => Doc::text("`SizeT.mul`"),
        (BinOp::Div, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.div`", get_int_mod(signed, width)?))
        }
        (BinOp::Div, TypeT::SizeT) => Doc::text("`SizeT.div`"),
        (BinOp::Mod, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.rem`", get_int_mod(signed, width)?))
        }
        (BinOp::Mod, TypeT::SizeT) => Doc::text("`SizeT.mod`"),
        (BinOp::Add, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.add`", get_int_mod(signed, width)?))
        }
        (BinOp::Add, TypeT::SizeT) => Doc::text("`SizeT.add`"),
        (BinOp::Sub, TypeT::Int { signed, width }) => {
            Doc::text(format!("`{}.sub`", get_int_mod(signed, width)?))
        }
        (BinOp::Sub, TypeT::SizeT) => Doc::text("`SizeT.sub`"),

        (BinOp::LEq, TypeT::SpecInt) => Doc::text("<="),
        (BinOp::Lt, TypeT::SpecInt) => Doc::text("<"),
        (BinOp::Mul, TypeT::SpecInt) => Doc::text("`op_Multiply`"),
        (BinOp::Div, TypeT::SpecInt) => Doc::text("/"),
        (BinOp::Mod, TypeT::SpecInt) => Doc::text("%"),
        (BinOp::Add, TypeT::SpecInt) => Doc::text("+"),
        (BinOp::Sub, TypeT::SpecInt) => Doc::text("-"),
        (BinOp::LogAnd, TypeT::SpecInt) => todo_binop!(),

        (
            op,
            TypeT::Requires(ty, _) | TypeT::Ensures(ty, _) | TypeT::Consumes(ty) | TypeT::Plain(ty),
        ) => emit_binop(env, op, ty.clone().into())?,

        (_, TypeT::TypeRef(_)) => return None,
        (
            BinOp::LEq | BinOp::Lt | BinOp::Mul | BinOp::Div | BinOp::Mod | BinOp::Add | BinOp::Sub,
            TypeT::Pointer(..),
        )
        | (_, TypeT::Void)
        | (BinOp::LogAnd, TypeT::Int { .. } | TypeT::SizeT | TypeT::Pointer(..))
        | (_, TypeT::SLProp)
        | (_, TypeT::Error) => return None,
    })
}

fn emit_rvalue(env: &Env, nm: &mut NameMangling, v: &Expr) -> Doc {
    emit_expr(env, nm, v).to_rvalue()
}

fn emit_rvalue_inner(env: &Env, nm: &mut NameMangling, v: &Expr) -> Doc {
    annotated(v, {
        match &v.val {
            ExprT::BoolLit(v) => Doc::text(if *v { "true" } else { "false" }),
            ExprT::IntLit(val, ty) => match ty.val {
                TypeT::Int {
                    signed: true,
                    width,
                } => Doc::text(format!("(Int{}.int_to_t {})", width, val)),
                TypeT::Int {
                    signed: false,
                    width,
                } => Doc::text(format!("(UInt{}.uint_to_t {})", width, val)),
                TypeT::SizeT => Doc::text(format!("{}sz", val)),
                _ => Doc::text(format!("(*unsupported integer literal*){}", val)),
            },
            ExprT::Var(_) | ExprT::Deref(_) | ExprT::Member(_, _) => {
                // These are lvalue variants; handled by emit_expr
                unreachable!("lvalue variants should be handled by emit_expr")
            }
            ExprT::Ref(v) => emit_lvalue(env, nm, v),
            ExprT::Cast(val, to_ty) => {
                let val_doc = emit_rvalue(env, nm, val);
                let from_ty = env.infer_rvalue(val).map(|t| env.vtype_whnf(t));
                let to_ty = env.vtype_whnf(to_ty.clone().into());
                let from_ty = match &from_ty {
                    Some(ty) => &ty.val,
                    None => &TypeT::Error,
                };
                let default = {
                    let val_doc = val_doc.clone();
                    || {
                        Doc::text(format!(
                            "(*TODO unsupported cast from {} to {} *)",
                            from_ty, to_ty
                        ))
                        .append(val_doc)
                    }
                };
                match (from_ty, &to_ty.val) {
                    (
                        TypeT::TypeRef(TypeRefKind::Struct(a)),
                        TypeT::TypeRef(TypeRefKind::Struct(b)),
                    ) if a.val == b.val => val_doc,
                    (TypeT::Void, TypeT::Void) => val_doc,
                    (TypeT::Bool, TypeT::Bool) => val_doc,
                    (TypeT::Bool, TypeT::Int { signed, width }) => {
                        if let Some(m) = get_int_mod(signed, width) {
                            parens(
                                Doc::text("if")
                                    .append(Doc::line())
                                    .append(val_doc)
                                    .append(Doc::line())
                                    .append("then")
                                    .append(Doc::line())
                                    .append(format!("{}.one", m))
                                    .append(Doc::line())
                                    .append("else")
                                    .append(Doc::line())
                                    .append(format!("{}.zero", m)),
                            )
                        } else {
                            default()
                        }
                    }
                    // (TypeT::Bool, TypeT::SizeT) => todo!(),
                    (TypeT::Bool, TypeT::SLProp) => unaryfn(Doc::text("pure"), val_doc),
                    (TypeT::Int { signed, width }, TypeT::Bool) => {
                        if let Some(m) = get_int_mod(signed, width) {
                            binop(
                                unaryfn_with_type(
                                    Doc::text(format!("{}.v", m)),
                                    val_doc,
                                    Doc::text("int"),
                                ),
                                Doc::text("<>"),
                                Doc::text("0"),
                            )
                        } else {
                            default()
                        }
                    }
                    (
                        TypeT::Int {
                            signed: s1,
                            width: w1,
                        },
                        TypeT::Int {
                            signed: s2,
                            width: w2,
                        },
                    ) if s1 == s2 && w1 == w2 => val_doc,
                    (TypeT::Int { signed, width }, TypeT::SpecInt) => {
                        if let Some(m) = get_int_mod(signed, width) {
                            unaryfn_with_type(
                                Doc::text(format!("{}.v", m)),
                                val_doc,
                                Doc::text("int"),
                            )
                        } else {
                            default()
                        }
                    }
                    (
                        TypeT::Int {
                            signed: s1,
                            width: w1,
                        },
                        TypeT::Int {
                            signed: s2,
                            width: w2,
                        },
                    ) => {
                        fn abbrev(s: bool, w: u32) -> String {
                            format!("{}int{}", if s { "" } else { "u" }, w)
                        }
                        unaryfn(
                            Doc::text(format!(
                                "Int.Cast.{}_to_{}",
                                abbrev(*s1, *w1),
                                abbrev(*s2, *w2)
                            )),
                            val_doc,
                        )
                    }
                    (TypeT::SizeT, TypeT::SpecInt) => unaryfn(Doc::text("SizeT.v"), val_doc),
                    // (TypeT::Int { signed:s1, width:w1 }, TypeT::Int { signed:s2, width:w2 }) => todo!(),
                    // (TypeT::Int { signed, width }, TypeT::SizeT) => todo!(),
                    // (TypeT::Int { signed, width }, TypeT::SLProp) => todo!(),
                    // (TypeT::SizeT, TypeT::Bool) => todo!(),
                    // (TypeT::SizeT, TypeT::Int { signed, width }) => todo!(),
                    (TypeT::SizeT, TypeT::SizeT) => val_doc,
                    // (TypeT::SizeT, TypeT::SLProp) => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::Bool) => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::SizeT) => todo!(),
                    (TypeT::Pointer(t1, k1), TypeT::Pointer(t2, k2)) if t1 == t2 && k1 == k2 => {
                        val_doc
                    }
                    // (TypeT::Pointer { to:t1, kind:k1 }, TypeT::Pointer { to:t2, kind:k2 }) if t1 == t2 => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::SLProp) => todo!(),
                    (TypeT::Error, _) | (_, TypeT::Error) => val_doc,
                    _ => default(),
                }
            }
            ExprT::Error(_ty) => Doc::text("(admit())"),
            ExprT::InlinePulse(val, _) => parens(Doc::concat(val.tokens.iter().map(|tok| {
                Doc::text(tok.before)
                    .append(annotated(&tok.text, Doc::text(tok.text.val.to_string())))
            }))),
            ExprT::BinOp(BinOp::LogAnd, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(lhs) {
                    if ty.val == TypeT::SLProp {
                        return binop(
                            emit_rvalue(env, nm, lhs),
                            Doc::text("**"),
                            emit_rvalue(env, nm, rhs),
                        );
                    }
                }
                binop(
                    emit_rvalue(env, nm, lhs),
                    Doc::text("&&"),
                    emit_rvalue(env, nm, rhs),
                )
            }
            ExprT::BinOp(BinOp::Eq, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(lhs) {
                    let ty = env.vtype_whnf(ty);
                    match (&ty.val, &rhs.val) {
                        (
                            TypeT::Pointer(
                                _,
                                PointerKind::Ref | PointerKind::Unknown, /* TODO */
                            ),
                            ExprT::IntLit(n, _),
                        ) => {
                            if **n == BigInt::ZERO {
                                return unaryfn(
                                    Doc::text("Pulse.Lib.Reference.is_null"),
                                    emit_rvalue(env, nm, lhs),
                                );
                            }
                        }
                        _ => {}
                    }
                }
                // TODO: this should be == in ghost contexts
                binop(
                    emit_rvalue(env, nm, lhs),
                    Doc::text("="),
                    emit_rvalue(env, nm, rhs),
                )
            }
            ExprT::UnOp(op, arg) => {
                if let Some(ty) = env.infer_rvalue(&arg)
                    && let Some(op) = emit_unop(env, *op, ty)
                {
                    unaryfn(op, emit_rvalue(env, nm, arg))
                } else {
                    // TODO: error
                    Doc::text("(*unsupported binop*)(admit())")
                }
            }
            ExprT::BinOp(op, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(&lhs)
                    && let Some(op) = emit_binop(env, *op, ty)
                {
                    binop(emit_rvalue(env, nm, lhs), op, emit_rvalue(env, nm, rhs))
                } else {
                    // TODO: error
                    Doc::text("(*unsupported binop*)(admit())")
                }
            }
            ExprT::FnCall(f, args) => {
                let args = if args.is_empty() {
                    Doc::text("()")
                } else {
                    Doc::intersperse(
                        args.iter().map(|arg| emit_rvalue(env, nm, arg)),
                        Doc::line(),
                    )
                };
                parens(
                    nm.emit(Name::Fn(f.val.clone()))
                        .append(Doc::line())
                        .append(args),
                )
            }
            ExprT::Live(v) => unaryfn(Doc::text("live"), emit_lvalue(env, nm, v)),
            ExprT::Old(v) => unaryfn(Doc::text("old"), emit_rvalue(env, nm, v)),
            ExprT::StructInit(name, fields) => Doc::text("{")
                .append(Doc::concat(fields.iter().map(|(fld, val)| {
                    Doc::line()
                        .append(nm.emit(Name::StructDirectFieldName(
                            name.val.clone(),
                            fld.val.clone(),
                        )))
                        .append("=")
                        .append(emit_rvalue(env, nm, val))
                        .append(";")
                })))
                .nest(2)
                .append(Doc::line())
                .append("}")
                .group(),
        }
    })
}

fn emit_stmt(env: &Env, nm: &mut NameMangling, stmt: &Stmt) -> Doc {
    annotated(stmt, {
        match &stmt.val {
            StmtT::Call(v) => emit_rvalue(env, nm, v).append(";").nest(2).group(),
            StmtT::Decl(x, ty) => {
                let x = nm.emit(Name::Var(x.val.clone()));
                (Doc::text("let mut ").append(x).append(" :"))
                    .append(Doc::line())
                    .append(emit_type(env, nm, ty))
                    .append(";")
                    .nest(2)
                    .group()
            }
            StmtT::Assign(x, t) => emit_lvalue(env, nm, x)
                .append(Doc::line())
                .append(":=")
                .group()
                .append(Doc::line())
                .append(emit_rvalue(env, nm, t))
                .append(";")
                .group()
                .nest(2),
            StmtT::If(c, b1, b2) => Doc::text("if ")
                .append(parens(emit_rvalue(env, nm, c)))
                .nest(2)
                .append(" ")
                .append(emit_block(env, nm, b1))
                .append(" else ")
                .append(emit_block(env, nm, b2))
                .append(";")
                .group(),
            StmtT::While(cond, invs, body) => Doc::text("while ")
                .append(parens(emit_rvalue(env, nm, cond)))
                .append(Doc::line())
                .append(Doc::concat(invs.iter().map(|inv| {
                    Doc::text("invariant ")
                        .append(emit_rvalue(env, nm, inv))
                        .group()
                        .nest(2)
                        .append(Doc::line())
                })))
                .nest(2)
                .append(emit_block(env, nm, body))
                .append(";")
                .group(),
            StmtT::Break => Doc::text("break;"),
            StmtT::Continue => Doc::text("continue;"),
            StmtT::Return(t) => emit_rvalue(env, nm, t).append(";").group().nest(2),
            StmtT::Assert(v) => Doc::text("assert")
                .append(Doc::line())
                .append(emit_rvalue(env, nm, v))
                .append(";")
                .group()
                .nest(2),
            StmtT::Error => Doc::text("(admit());"),
        }
    })
}

fn block(stmts: Doc) -> Doc {
    Doc::text("{")
        .append(stmts.nest(2))
        .append(Doc::hardline())
        .append(Doc::text("}"))
        .group()
}

fn emit_stmts(env: &Env, nm: &mut NameMangling, stmts: &Vec<Rc<Stmt>>) -> Doc {
    let mut env = env.clone();
    Doc::concat(stmts.iter().map(|stmt| {
        let doc = Doc::line().append(emit_stmt(&env, nm, stmt));
        env.push_stmt(stmt);
        doc
    }))
}

fn emit_block(env: &Env, nm: &mut NameMangling, stmts: &Vec<Rc<Stmt>>) -> Doc {
    if stmts.is_empty() {
        return Doc::text("{}");
    }
    block(emit_stmts(env, nm, stmts))
}

fn mk_let(n: Doc, args: &[Doc], ty: Doc, body: Doc) -> Doc {
    (Doc::text("let").append(Doc::line()).append(n))
        .append(
            Doc::concat(args.iter().map(|arg| Doc::line().append(arg.clone())))
                .append(Doc::line().append(":"))
                .nest(2),
        )
        .group()
        .append(Doc::line().append(ty))
        .append(Doc::line().append("="))
        .nest(2)
        .group()
        .append(Doc::line().append(body))
        .group()
        .nest(2)
}

fn mk_eager_unfold_let(n: Doc, args: &[Doc], ty: Doc, body: Doc) -> Doc {
    Doc::text("[@@pulse_eager_unfold]")
        .append(Doc::line())
        .append(mk_let(n, args, ty, body))
        .group()
}

fn mk_star<I: IntoIterator<Item = Doc>>(ps: I) -> Doc {
    match ps.into_iter().reduce(|accum, p| {
        accum
            .append(Doc::space())
            .append("**")
            .append(Doc::line())
            .append(p)
    }) {
        Some(star) => parens(star),
        None => Doc::text("emp"),
    }
}

fn mk_rvar(n: &Rc<Ident>) -> Rc<Expr> {
    ExprT::Var(n.clone()).with_loc(n.loc.clone())
}

fn emit_typedef(env: &Env, nm: &mut NameMangling, TypeDefn { name, body }: &TypeDefn) -> Doc {
    let k = &TypeRefKind::Typedef(name.clone());
    let t = nm.emit(Name::TypeRef(k.into()));
    let ty_decl = mk_let(t.clone(), &[], Doc::text("Type"), emit_type(env, nm, body));
    let env = &mut env.clone();
    env.push_this(TypeT::TypeRef(k.clone()).with_loc(name.loc.clone()));
    let this: Rc<Ident> = Rc::<str>::from("this").with_loc(name.loc.clone());
    let this_doc = nm.emit(Name::Var(this.val.clone()));
    let this_args = vec![parens(this_doc.append(":").append(Doc::line()).append(t))];
    let mut req = vec![];
    let mut ens = vec![];
    emit_type_slprop(env, nm, body, &mut req, &mut ens, &mk_rvar(&this));
    let pre_decl = mk_eager_unfold_let(
        nm.emit(Name::TypeRefPred(k.into())),
        &this_args,
        Doc::text("slprop"),
        mk_star(req),
    );
    let post_decl = mk_eager_unfold_let(
        nm.emit(Name::TypeRefPredPost(k.into())),
        &this_args,
        Doc::text("slprop"),
        mk_star(ens),
    );
    Doc::intersperse(vec![ty_decl, pre_decl, post_decl], Doc::line())
}

fn mk_assume_val(n: Doc, args: &[Doc], ty: Doc) -> Doc {
    (Doc::text("assume val").append(Doc::line()).append(n))
        .append(
            Doc::concat(args.iter().map(|arg| Doc::line().append(arg.clone())))
                .append(Doc::line().append(":"))
                .nest(2),
        )
        .group()
        .append(Doc::line().append(ty))
        .nest(2)
        .group()
}

fn mk_fun(arg: Doc, body: Doc) -> Doc {
    parens(
        Doc::text("fun")
            .append(Doc::line())
            .append(arg)
            .append(Doc::line())
            .append("->")
            .group()
            .nest(2)
            .append(Doc::line())
            .append(body),
    )
}

fn mk_thunk(body: Doc) -> Doc {
    mk_fun(Doc::text("_"), body)
}

fn emit_structdefn(
    env: &Env,
    nm: &mut NameMangling,
    StructDefn { name, fields }: &StructDefn,
) -> Doc {
    let k = &TypeRefKind::Struct(name.clone());
    let struct_type_name = nm.emit(Name::TypeRef(k.into()));
    let pts_to_name = nm.emit(Name::TypeRefPred(k.into()));
    let pts_to_name_post = nm.emit(Name::TypeRefPredPost(k.into()));
    let ref_struct_type = unaryfn(Doc::text("ref"), struct_type_name.clone());

    let direct_fld = |fld: &Ident| Name::StructDirectFieldName(name.val.clone(), fld.val.clone());

    let mut ses = vec![];

    ses.push(
        Doc::text("noeq type")
            .append(Doc::line())
            .append(struct_type_name.clone())
            .append(Doc::line())
            .append("=")
            .append(Doc::line())
            .append("{")
            .group()
            .append(Doc::concat(fields.iter().map(|(fld, fld_ty)| {
                Doc::hardline().append(
                    nm.emit(direct_fld(fld))
                        .append(":")
                        .append(Doc::line())
                        .append(emit_type(env, nm, fld_ty))
                        .append(";")
                        .group()
                        .nest(2),
                )
            })))
            .nest(2)
            .append(Doc::line())
            .append("}")
            .group(),
    );

    for n in [&pts_to_name, &pts_to_name_post] {
        ses.push(mk_eager_unfold_let(
            n.clone(),
            &[parens(
                Doc::text("[@@@mkey] x:")
                    .append(Doc::line())
                    .append(struct_type_name.clone()),
            )],
            Doc::text("slprop"),
            Doc::text("emp"),
        ));
    }

    let unfolded_tok = nm.emit(Name::StructAuxFn(name.val.clone(), "raw_unfolded".into()));
    ses.push(mk_assume_val(
        unfolded_tok.clone(),
        &[parens(
            Doc::text("x:")
                .append(Doc::line())
                .append(ref_struct_type.clone()),
        )],
        Doc::text("slprop"),
    ));

    let ghost_fld = |fld: &Ident| Name::StructGhostFieldProj(name.val.clone(), fld.val.clone());

    for (fld, fld_ty) in fields {
        let ll_type = emit_type(env, nm, fld_ty);

        ses.push(mk_assume_val(
            nm.emit(ghost_fld(fld)),
            &[parens(
                Doc::text("x:")
                    .append(Doc::line())
                    .append(ref_struct_type.clone()),
            )],
            Doc::text("GTot")
                .append(Doc::line())
                .append(unaryfn(Doc::text("ref"), ll_type)),
        ));
    }

    ses.push(
        Doc::text("[@@pulse_intro]")
            .append(Doc::line())
            .append(mk_assume_val(
                nm.emit(Name::StructAuxFn(name.val.clone(), "raw_unfold".into())),
                &[
                    parens(
                        Doc::text("x:")
                            .append(Doc::line())
                            .append(ref_struct_type.clone()),
                    ),
                    parens(
                        Doc::text("vx:")
                            .append(Doc::line())
                            .append(struct_type_name.clone()),
                    ),
                ],
                naryfn([
                    Doc::text("stt_ghost"),
                    Doc::text("unit"),
                    Doc::text("emp_inames"),
                    // pre
                    naryfn([
                        Doc::text("Pulse.Lib.Reference.pts_to"),
                        Doc::text("x"),
                        Doc::text("vx"),
                    ]),
                    {
                        let mut post = vec![naryfn([unfolded_tok.clone(), Doc::text("x")])];
                        for (fld, _) in fields {
                            post.push(naryfn([
                                Doc::text("Pulse.Lib.Reference.pts_to"),
                                unaryfn(nm.emit(ghost_fld(fld)), Doc::text("x")),
                                Doc::text("vx.").append(nm.emit(direct_fld(fld))),
                            ]));
                        }
                        mk_thunk(mk_star(post))
                    },
                ]),
            )),
    );

    let fold_arg_name = |fld: &Ident| Doc::text(format!("v_{}", fld));
    ses.push(
        Doc::text("[@@pulse_intro]")
            .append(Doc::line())
            .append(mk_assume_val(
                nm.emit(Name::StructAuxFn(name.val.clone(), "raw_fold".into())),
                &{
                    let mut args = vec![parens(
                        Doc::text("x:")
                            .append(Doc::line())
                            .append(ref_struct_type.clone()),
                    )];
                    for (fld, _) in fields {
                        args.push(fold_arg_name(fld))
                    }
                    args
                },
                naryfn([
                    Doc::text("stt_ghost"),
                    Doc::text("unit"),
                    Doc::text("emp_inames"),
                    {
                        let mut pre = vec![naryfn([unfolded_tok.clone(), Doc::text("x")])];
                        for (fld, _) in fields {
                            pre.push(naryfn([
                                Doc::text("Pulse.Lib.Reference.pts_to"),
                                unaryfn(nm.emit(ghost_fld(fld)), Doc::text("x")),
                                fold_arg_name(fld),
                            ]));
                        }
                        mk_star(pre)
                    },
                    mk_thunk(naryfn([
                        Doc::text("Pulse.Lib.Reference.pts_to"),
                        Doc::text("x"),
                        Doc::text("{")
                            .append(Doc::concat(fields.iter().map(|(fld, _)| {
                                Doc::line()
                                    .append(nm.emit(direct_fld(fld)))
                                    .append("=")
                                    .append(fold_arg_name(fld))
                                    .append(";")
                            })))
                            .nest(2)
                            .append(Doc::line())
                            .append("}")
                            .group(),
                    ])),
                ]),
            )),
    );
    ses.push(
        Doc::text("[@@pulse_intro]")
            .append(Doc::line())
            .append(mk_assume_val(
                nm.emit(Name::StructAuxFn(
                    name.val.clone(),
                    "raw_fold_uninit".into(),
                )),
                &[parens(
                    Doc::text("x:")
                        .append(Doc::line())
                        .append(ref_struct_type.clone()),
                )],
                naryfn([
                    Doc::text("stt_ghost"),
                    Doc::text("unit"),
                    Doc::text("emp_inames"),
                    {
                        let mut pre = vec![naryfn([unfolded_tok.clone(), Doc::text("x")])];
                        for (fld, _) in fields {
                            pre.push(naryfn([
                                Doc::text("Pulse.Lib.Reference.pts_to_uninit"),
                                unaryfn(nm.emit(ghost_fld(fld)), Doc::text("x")),
                            ]));
                        }
                        mk_star(pre)
                    },
                    mk_thunk(naryfn([
                        Doc::text("Pulse.Lib.Reference.pts_to_uninit"),
                        Doc::text("x"),
                    ])),
                ]),
            )),
    );

    for (fld, fld_ty) in fields {
        let ll_ty = emit_type(env, nm, fld_ty);
        let fld_pts_to = naryfn([
            Doc::text("pts_to"),
            unaryfn(nm.emit(ghost_fld(fld)), Doc::text("x")),
            Doc::text("vx"),
        ]);
        ses.push(mk_assume_val(
            nm.emit(Name::StructFieldProj(name.val.clone(), fld.val.clone())),
            &[
                parens(
                    Doc::text("x:")
                        .append(Doc::line())
                        .append(ref_struct_type.clone()),
                ),
                parens(
                    Doc::text("#vx:")
                        .append(Doc::line())
                        .append(unaryfn(Doc::text("erased"), ll_ty.clone())),
                ),
            ],
            naryfn([
                Doc::text("stt_atomic"),
                unaryfn(Doc::text("ref"), ll_ty),
                Doc::text("#PulseCore.Observability.Neutral"),
                Doc::text("emp_inames"),
                fld_pts_to.clone(),
                mk_fun(
                    Doc::text("vx'"),
                    mk_star([
                        fld_pts_to,
                        naryfn([
                            Doc::text("rewrites_to"),
                            Doc::text("vx'"),
                            unaryfn(nm.emit(ghost_fld(fld)), Doc::text("x")),
                        ]),
                    ]),
                ),
            ]),
        ))
    }

    Doc::intersperse(ses.into_iter().map(|se| se.group()), Doc::hardline())
}

fn emit_fn_decl(
    env: &Env,
    nm: &mut NameMangling,
    FnDecl {
        name,
        ret_type,
        args,
        requires,
        ensures,
    }: &FnDecl,
) -> Doc {
    let env = &mut env.clone();

    let mut requires_props = vec![];
    let mut ensures_props = vec![];
    let mut params = vec![];
    for (i, arg @ (n0, ty)) in args.iter().enumerate() {
        let n: Rc<Ident> = n0
            .clone()
            .unwrap_or_else(|| Rc::<str>::from(format!("_unnamed{}", i)).with_loc(ty.loc.clone()));

        params.push(parens(
            annotated(&n, nm.emit(Name::Var(n.val.clone())))
                .append(":")
                .append(Doc::line())
                .append(emit_type(env, nm, ty)),
        ));

        env.push_arg(arg, LocalDeclKind::RValue);
        emit_type_slprop(
            env,
            nm,
            ty,
            &mut requires_props,
            &mut ensures_props,
            &mk_rvar(&n),
        );
    }

    if params.is_empty() {
        params.push(Doc::text("()"));
    }

    requires_props.extend(requires.iter().map(|r| emit_rvalue(env, nm, r)));

    let return_id = Rc::<str>::from("return").with_loc(ret_type.loc.clone());
    env.push_return(ret_type.clone());
    emit_type_slprop(
        env,
        nm,
        &ret_type,
        &mut ensures_props,
        &mut vec![],
        &mk_rvar(&return_id),
    );
    let ret_type_doc = emit_type(env, nm, ret_type);

    env.push_return(ret_type.clone());
    ensures_props.extend(ensures.iter().map(|r| emit_rvalue(env, nm, r)));

    let hdr = Doc::group(
        Doc::text("fn")
            .append(Doc::line())
            .append(nm.emit(Name::Fn(name.val.clone()))),
    )
    .append(Doc::concat(params.into_iter().map(|p| Doc::line().append(p))).nest(2))
    .group();

    hdr.append(Doc::concat(requires_props.into_iter().map(|r| {
        Doc::hardline().append(
            Doc::text("requires")
                .append(Doc::line())
                .append(r)
                .nest(2)
                .group(),
        )
    })))
    .append(Doc::hardline())
    .append(Doc::group(
        Doc::text("returns")
            .append(Doc::line())
            .append(nm.emit(Name::Var(return_id.val.clone())))
            .append(Doc::line())
            .append(":")
            .group()
            .append(Doc::line())
            .append(ret_type_doc),
    ))
    .append(Doc::concat(ensures_props.into_iter().map(|r| {
        Doc::hardline().append(
            Doc::text("ensures")
                .append(Doc::line())
                .append(r)
                .nest(2)
                .group(),
        )
    })))
    .group()
}

fn emit_inline_code(code: &InlineCode) -> Doc {
    Doc::concat(code.tokens.iter().map(|tok| {
        Doc::text(tok.before).append(annotated(&tok.text, Doc::text(tok.text.val.to_string())))
    }))
}

fn emit_decl(env: &Env, nm: &mut NameMangling, decl: &Decl) -> Doc {
    annotated(decl, {
        match &decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                let decl_doc = emit_fn_decl(env, nm, decl).nest(2).append(Doc::hardline());
                let arg_redecl_as_mut = Doc::concat(decl.args.iter().filter_map(|(n, _)| {
                    n.as_ref().map(|n| {
                        Doc::line().append(annotated(
                            n,
                            Doc::group({
                                let n = nm.emit(Name::Var(n.val.clone()));
                                Doc::text("let mut ")
                                    .append(n.clone())
                                    .append(" = ")
                                    .append(n)
                                    .append(";")
                            }),
                        ))
                    })
                }));
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                decl_doc.append(block(arg_redecl_as_mut.append(emit_stmts(env, nm, body))).group())
            }
            DeclT::FnDecl(fn_decl) => emit_fn_decl(&mut env.clone(), nm, fn_decl),
            DeclT::Typedef(typedef) => emit_typedef(env, nm, typedef),
            DeclT::StructDefn(struct_defn) => emit_structdefn(env, nm, struct_defn),
            DeclT::IncludeDecl(include_decl) => emit_inline_code(&include_decl.code),
        }
    })
}

pub fn emit(module_name: &str, tu: &TranslationUnit) -> (String, SourceRangeMap) {
    let mut env = Env::new();
    let nm = &mut NameMangling::new();
    let mut output: Vec<Doc> = vec![];
    output.push(Doc::text(format!(
        "module {}\nopen Pulse\nopen Pulse.Lib.C\n#lang-pulse",
        module_name
    )));
    for decl in &tu.decls {
        output.push(emit_decl(&env, nm, decl));
        env.push_decl(decl);
    }
    let output = Doc::intersperse(output, Doc::hardline().append(Doc::hardline())).group();
    let mut writer = StrWriter::new();
    output.render_raw(100, &mut writer).unwrap();
    (writer.buffer, writer.source_range_map)
}
