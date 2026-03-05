use std::fmt::Write;
use std::{
    collections::{HashMap, HashSet},
    rc::Rc,
};

use num_bigint::BigInt;
use pretty::{RcDoc, Render, RenderAnnotated};

use crate::{
    diag::{Diagnostic, DiagnosticLevel, Diagnostics},
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

    /// Extract the raw document without rvalue/lvalue conversion.
    fn into_doc(self) -> Doc {
        match self {
            ExprKind::LValue(doc) | ExprKind::RValue(doc) => doc,
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

struct Emitter<'a> {
    nm: NameMangling,
    diags: &'a mut Diagnostics,
}

impl<'a> Emitter<'a> {
    fn report(&mut self, msg: String, loc: &SourceInfo) {
        self.diags.report(Diagnostic {
            loc: loc.location().clone(),
            level: DiagnosticLevel::Error,
            msg,
        });
    }
}

impl<'a> Emitter<'a> {
    fn emit_type(&mut self, env: &Env, ty: &Type) -> Doc {
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
                    unaryfn(Doc::text("array"), self.emit_type(env, to))
                }
                TypeT::Pointer(to, PointerKind::Ref | PointerKind::Unknown) => {
                    unaryfn(Doc::text("ref"), self.emit_type(env, to))
                }
                TypeT::Error => Doc::text("unit"),

                TypeT::TypeRef(n) => self.nm.emit(Name::TypeRef(n.into())),

                TypeT::SLProp => Doc::text("slprop"),
                TypeT::SpecInt => Doc::text("int"),

                TypeT::Requires(ty, _)
                | TypeT::Ensures(ty, _)
                | TypeT::Consumes(ty)
                | TypeT::Plain(ty) => self.emit_type(env, ty),
            }
        })
    }

    fn subst_this_rvalue(&mut self, env: &Env, rvalue: &mut Expr, this: &Rc<Expr>) {
        match &mut rvalue.val {
            ExprT::Var(x) => {
                if &*x.val == "this" {
                    *rvalue = (**this).clone()
                }
            }
            ExprT::Deref(rv) => self.subst_this_rvalue(env, Rc::make_mut(rv), this),
            ExprT::Member(x, _a) => self.subst_this_rvalue(env, Rc::make_mut(x), this),
            ExprT::BoolLit(_) => {}
            ExprT::IntLit(..) => {}
            ExprT::Ref(lv) => self.subst_this_rvalue(env, Rc::make_mut(lv), this),
            ExprT::UnOp(_, arg) => {
                self.subst_this_rvalue(env, Rc::make_mut(arg), this);
            }
            ExprT::BinOp(_, lhs, rhs) => {
                self.subst_this_rvalue(env, Rc::make_mut(lhs), this);
                self.subst_this_rvalue(env, Rc::make_mut(rhs), this);
            }
            ExprT::FnCall(_f, args) => {
                for arg in args {
                    self.subst_this_rvalue(env, Rc::make_mut(arg), this);
                }
            }
            ExprT::Cast(val, _) => {
                self.subst_this_rvalue(env, Rc::make_mut(val), this);
            }
            ExprT::InlinePulse(val, _) => {
                self.subst_inline_pulse_code_this(env, Rc::make_mut(val), this)
            }
            ExprT::Error(_ty) => {}
            ExprT::Live(val) => self.subst_this_rvalue(env, Rc::make_mut(val), this),
            ExprT::Old(val) => self.subst_this_rvalue(env, Rc::make_mut(val), this),
            ExprT::Forall(_, _, body) | ExprT::Exists(_, _, body) => {
                self.subst_this_rvalue(env, Rc::make_mut(body), this);
            }
            ExprT::StructInit(_, fields) => {
                for (_fld, val) in fields {
                    self.subst_this_rvalue(env, Rc::make_mut(val), this);
                }
            }
            ExprT::Malloc(_) => {}
            ExprT::MallocArray(_, count) => {
                self.subst_this_rvalue(env, Rc::make_mut(count), this);
            }
            ExprT::Free(val) => self.subst_this_rvalue(env, Rc::make_mut(val), this),
            ExprT::Index(arr, idx) => {
                self.subst_this_rvalue(env, Rc::make_mut(arr), this);
                self.subst_this_rvalue(env, Rc::make_mut(idx), this);
            }
        }
    }

    fn subst_inline_pulse_code_this(
        &mut self,
        env: &Env,
        val: &mut InlinePulseCode,
        this: &Rc<Expr>,
    ) {
        for tok in &mut val.tokens {
            match tok {
                InlinePulseToken::Verbatim(_) => {}
                InlinePulseToken::RValueAntiquot { expr, .. }
                | InlinePulseToken::LValueAntiquot { expr, .. } => {
                    self.subst_this_rvalue(env, Rc::make_mut(expr), this);
                }
            }
        }
    }

    fn emit_type_slprop(
        &mut self,
        env: &Env,
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
                let live = annotated(ty, unaryfn(Doc::text("live"), self.emit_rvalue(env, this)));
                req.push(live.clone());
                ens.push(live);

                match kind {
                    PointerKind::Ref => {
                        let derefed = ExprT::Deref(this.clone()).with_loc(this.loc.clone());
                        self.emit_type_slprop(env, pointee_ty, req, ens, &derefed);
                    }
                    _ => {} // TODO
                }
            }
            TypeT::TypeRef(n) => {
                let this = self.emit_rvalue(env, this);
                req.push(unaryfn(
                    self.nm.emit(Name::TypeRefPred(n.into())),
                    this.clone(),
                ));
                ens.push(unaryfn(
                    self.nm.emit(Name::TypeRefPredPost(n.into())),
                    this.clone(),
                ));
            }
            TypeT::Requires(ty, p) => {
                self.emit_type_slprop(env, ty, req, ens, this);

                let p = &mut p.clone();
                self.subst_this_rvalue(env, Rc::make_mut(p), this);
                req.push(self.emit_rvalue(env, p));
            }
            TypeT::Ensures(ty, p) => {
                self.emit_type_slprop(env, ty, req, ens, this);

                let p = &mut p.clone();
                self.subst_this_rvalue(env, Rc::make_mut(p), this);
                ens.push(self.emit_rvalue(env, p));
            }
            TypeT::Consumes(ty) => self.emit_type_slprop(env, ty, req, &mut vec![], this),
            TypeT::Plain(_) => {}
            TypeT::Error => {}
        }
    }

    fn emit_var(&mut self, v: &Ident) -> Doc {
        annotated(v, self.nm.emit(Name::Var(v.val.clone())))
    }

    fn emit_lvalue(&mut self, env: &Env, v: &Expr) -> Doc {
        match self.emit_expr(env, v) {
            ExprKind::LValue(doc) => doc,
            ExprKind::RValue(_) => {
                self.report(format!("cannot produce lvalue for {}", v), &v.loc);
                Doc::text("(admit())")
            }
        }
    }

    fn emit_expr(&mut self, env: &Env, v: &Expr) -> ExprKind {
        match &v.val {
            ExprT::Var(x) => {
                if let Some(LocalDecl {
                    kind: LocalDeclKind::RValue,
                    ..
                }) = env.lookup_var(x)
                {
                    ExprKind::RValue(annotated(v, self.emit_var(x)))
                } else {
                    ExprKind::LValue(annotated(v, self.emit_var(x)))
                }
            }
            ExprT::Deref(inner) => {
                ExprKind::LValue(annotated(v, self.emit_expr(env, inner).to_rvalue()))
            }
            ExprT::Member(x, a) => match env.infer_expr(x) {
                Some(ty) => {
                    let ty = env.vtype_whnf(ty);
                    match &ty.val {
                        TypeT::Pointer(_, PointerKind::Array) if &*a.val == "_length" => {
                            ExprKind::RValue(annotated(
                                v,
                                unaryfn(
                                    Doc::text("Seq.length"),
                                    unaryfn(Doc::text("value_of"), self.emit_rvalue(env, x)),
                                ),
                            ))
                        }
                        TypeT::TypeRef(TypeRefKind::Struct(struct_name)) => {
                            match self.emit_expr(env, x) {
                                ExprKind::LValue(x_doc) => ExprKind::LValue(annotated(
                                    v,
                                    unaryfn(
                                        self.nm.emit(Name::StructFieldProj(
                                            struct_name.val.clone(),
                                            a.val.clone(),
                                        )),
                                        x_doc,
                                    ),
                                )),
                                ExprKind::RValue(x_doc) => ExprKind::RValue(annotated(
                                    v,
                                    x_doc.append(Doc::text(".")).append(self.nm.emit(
                                        Name::StructDirectFieldName(
                                            struct_name.val.clone(),
                                            a.val.clone(),
                                        ),
                                    )),
                                )),
                            }
                        }
                        _ => {
                            self.report(
                                format!("unsupported struct field access on {}", ty),
                                &v.loc,
                            );
                            ExprKind::RValue(annotated(v, Doc::text("(admit())")))
                        }
                    }
                }
                None => {
                    self.report(format!("cannot infer type of {}", x), &v.loc);
                    ExprKind::RValue(annotated(v, Doc::text("(admit())")))
                }
            },
            ExprT::Index(arr, idx) => {
                let arr_doc = self.emit_rvalue(env, arr);
                let idx_doc = self.emit_rvalue(env, idx);
                ExprKind::RValue(annotated(
                    v,
                    arr_doc
                        .append(Doc::text(".("))
                        .append(idx_doc)
                        .append(Doc::text(")")),
                ))
            }
            _ => ExprKind::RValue(self.emit_rvalue_inner(env, v)),
        }
    }
} // impl Emitter (group A)

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
        (BinOp::LogOr, TypeT::Bool) => Doc::text("||"),
        (BinOp::Implies, TypeT::Bool) => Doc::text("==>"),
        (BinOp::Div, TypeT::Bool) => todo_binop!(),
        (BinOp::Mod, TypeT::Bool) => todo_binop!(),
        (BinOp::Sub, TypeT::Bool) => todo_binop!(),
        (BinOp::Add, TypeT::Bool) => todo_binop!(),
        (BinOp::Mul, TypeT::Bool) => Doc::text("&&"),

        (BinOp::LogAnd, TypeT::SLProp) => Doc::text("**"),
        (BinOp::LogOr, TypeT::SLProp) => todo_binop!(),
        (BinOp::Implies, TypeT::SLProp) => Doc::text("`Pulse.Lib.Trade.trade`"),

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
        (BinOp::LogOr, TypeT::SpecInt) => todo_binop!(),
        (BinOp::Implies, TypeT::SpecInt) => todo_binop!(),

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
        | (
            BinOp::LogAnd | BinOp::LogOr | BinOp::Implies,
            TypeT::Int { .. } | TypeT::SizeT | TypeT::Pointer(..),
        )
        | (_, TypeT::SLProp)
        | (_, TypeT::Error) => return None,
    })
}

impl<'a> Emitter<'a> {
    fn emit_rvalue(&mut self, env: &Env, v: &Expr) -> Doc {
        self.emit_expr(env, v).to_rvalue()
    }

    fn emit_rvalue_inner(&mut self, env: &Env, v: &Expr) -> Doc {
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
                    TypeT::SpecInt => Doc::text(format!("{}", val)),
                    _ => {
                        self.report(
                            format!("unsupported integer literal type for {}", val),
                            &v.loc,
                        );
                        Doc::text(format!("(admit()) (* {} *)", val))
                    }
                },
                ExprT::Var(_) | ExprT::Deref(_) | ExprT::Member(_, _) | ExprT::Index(_, _) => {
                    // These are lvalue variants; handled by emit_expr
                    unreachable!("lvalue variants should be handled by emit_expr")
                }
                ExprT::Ref(v) => self.emit_lvalue(env, v),
                ExprT::Cast(val, to_ty) => {
                    let val_doc = self.emit_rvalue(env, val);
                    let from_ty = env.infer_rvalue(val).map(|t| env.vtype_whnf(t));
                    let to_ty = env.vtype_whnf(to_ty.clone().into());
                    let from_ty = match &from_ty {
                        Some(ty) => &ty.val,
                        None => &TypeT::Error,
                    };
                    // Special case: integer literal cast to SizeT → emit Nsz
                    if matches!(&to_ty.val, TypeT::SizeT) {
                        if let ExprT::IntLit(n, _) = &val.val {
                            return Doc::text(format!("{}sz", n));
                        }
                    }
                    let default_msg = format!("unsupported cast from {} to {}", from_ty, to_ty);
                    match (from_ty, &to_ty.val) {
                        (
                            TypeT::TypeRef(TypeRefKind::Struct(a)),
                            TypeT::TypeRef(TypeRefKind::Struct(b)),
                        ) if a.val == b.val => val_doc,
                        (TypeT::Void, TypeT::Void) => val_doc,
                        (TypeT::Bool, TypeT::Bool) => val_doc,
                        (TypeT::Bool, TypeT::Int { signed, width }) => {
                            fn abbrev(s: &bool, w: &u32) -> String {
                                format!("{}int{}", if *s { "" } else { "u" }, w)
                            }
                            unaryfn(
                                Doc::text(format!("bool_to_{}", abbrev(signed, width))),
                                val_doc,
                            )
                        }
                        (TypeT::Bool, TypeT::SpecInt) => unaryfn(Doc::text("bool_to_int"), val_doc),
                        (TypeT::SpecInt, TypeT::Bool) => parens(
                            val_doc
                                .append(Doc::line())
                                .append("<>")
                                .append(Doc::line())
                                .append("0"),
                        ),
                        // (TypeT::Bool, TypeT::SizeT) => todo!(),
                        (TypeT::Bool, TypeT::SLProp) => unaryfn(Doc::text("with_pure"), val_doc),
                        (TypeT::Int { signed, width }, TypeT::Bool) => {
                            fn abbrev(s: &bool, w: &u32) -> String {
                                format!("{}int{}", if *s { "" } else { "u" }, w)
                            }
                            unaryfn(
                                Doc::text(format!("{}_to_bool", abbrev(signed, width))),
                                val_doc,
                            )
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
                                self.report(default_msg.clone(), &v.loc);
                                Doc::text("(admit())")
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
                        (TypeT::Int { signed, width }, TypeT::SizeT) => {
                            if let Some(m) = get_int_mod(signed, width) {
                                unaryfn(
                                    Doc::text(format!("SizeT.uint_to_t ({}.v", m)),
                                    val_doc.append(Doc::text(")")),
                                )
                            } else {
                                self.report(default_msg.clone(), &v.loc);
                                Doc::text("(admit())")
                            }
                        }
                        (TypeT::SpecInt, TypeT::SizeT) => {
                            unaryfn(Doc::text("SizeT.uint_to_t"), val_doc)
                        }
                        (TypeT::SpecInt, TypeT::Int { signed, width }) => {
                            if let Some(m) = get_int_mod(signed, width) {
                                unaryfn(
                                    Doc::text(format!(
                                        "{}.{}",
                                        m,
                                        if *signed { "int_to_t" } else { "uint_to_t" }
                                    )),
                                    val_doc,
                                )
                            } else {
                                self.report(default_msg.clone(), &v.loc);
                                Doc::text("(admit())")
                            }
                        }
                        // (TypeT::Int { signed:s1, width:w1 }, TypeT::Int { signed:s2, width:w2 }) => todo!(),
                        // (TypeT::Int { signed, width }, TypeT::SizeT) => todo!(),
                        // (TypeT::Int { signed, width }, TypeT::SLProp) => todo!(),
                        // (TypeT::SizeT, TypeT::Bool) => todo!(),
                        // (TypeT::SizeT, TypeT::Int { signed, width }) => todo!(),
                        (TypeT::SizeT, TypeT::SizeT) => val_doc,
                        (TypeT::SpecInt, TypeT::SpecInt) => val_doc,
                        // (TypeT::SizeT, TypeT::SLProp) => todo!(),
                        // (TypeT::Pointer { to, kind }, TypeT::Bool) => todo!(),
                        // (TypeT::Pointer { to, kind }, TypeT::SizeT) => todo!(),
                        (TypeT::Pointer(t1, k1), TypeT::Pointer(t2, k2))
                            if t1 == t2 && k1 == k2 =>
                        {
                            val_doc
                        }
                        // (TypeT::Pointer { to:t1, kind:k1 }, TypeT::Pointer { to:t2, kind:k2 }) if t1 == t2 => todo!(),
                        // (TypeT::Pointer { to, kind }, TypeT::SLProp) => todo!(),
                        (TypeT::Error, _) | (_, TypeT::Error) => val_doc,
                        _ => {
                            self.report(default_msg.clone(), &v.loc);
                            Doc::text("(admit())")
                        }
                    }
                }
                ExprT::Error(_ty) => Doc::text("(admit())"),
                ExprT::InlinePulse(val, _) => parens(Doc::concat(val.tokens.iter().map(|tok| {
                    match tok {
                        InlinePulseToken::Verbatim(ct) => Doc::text(ct.before)
                            .append(annotated(&ct.text, Doc::text(ct.text.val.to_string()))),
                        InlinePulseToken::RValueAntiquot { before, expr } => {
                            Doc::text(*before).append(self.emit_rvalue(env, expr))
                        }
                        InlinePulseToken::LValueAntiquot { before, expr } => {
                            Doc::text(*before).append(self.emit_expr(env, expr).into_doc())
                        }
                    }
                }))),
                ExprT::BinOp(BinOp::LogAnd, lhs, rhs) => {
                    if let Some(ty) = env.infer_rvalue(lhs) {
                        if ty.val == TypeT::SLProp {
                            return binop(
                                self.emit_rvalue(env, lhs),
                                Doc::text("**"),
                                self.emit_rvalue(env, rhs),
                            );
                        }
                    }
                    binop(
                        self.emit_rvalue(env, lhs),
                        Doc::text("&&"),
                        self.emit_rvalue(env, rhs),
                    )
                }
                ExprT::BinOp(BinOp::LogOr, lhs, rhs) => binop(
                    self.emit_rvalue(env, lhs),
                    Doc::text("||"),
                    self.emit_rvalue(env, rhs),
                ),
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
                                        self.emit_rvalue(env, lhs),
                                    );
                                }
                            }
                            _ => {}
                        }
                    }
                    // TODO: this should be == in ghost contexts
                    binop(
                        self.emit_rvalue(env, lhs),
                        Doc::text("="),
                        self.emit_rvalue(env, rhs),
                    )
                }
                ExprT::UnOp(op, arg) => {
                    if let Some(ty) = env.infer_rvalue(&arg)
                        && let Some(op) = emit_unop(env, *op, ty)
                    {
                        unaryfn(op, self.emit_rvalue(env, arg))
                    } else {
                        self.report(format!("unsupported unary operator on {}", arg), &v.loc);
                        Doc::text("(admit())")
                    }
                }
                ExprT::BinOp(op, lhs, rhs) => {
                    if let Some(ty) = env.infer_rvalue(&lhs)
                        && let Some(op) = emit_binop(env, *op, ty)
                    {
                        binop(self.emit_rvalue(env, lhs), op, self.emit_rvalue(env, rhs))
                    } else {
                        self.report(format!("unsupported binary operator on {}", lhs), &v.loc);
                        Doc::text("(admit())")
                    }
                }
                ExprT::FnCall(f, args) => {
                    let args = if args.is_empty() {
                        Doc::text("()")
                    } else {
                        Doc::intersperse(
                            args.iter().map(|arg| self.emit_rvalue(env, arg)),
                            Doc::line(),
                        )
                    };
                    parens(
                        self.nm
                            .emit(Name::Fn(f.val.clone()))
                            .append(Doc::line())
                            .append(args),
                    )
                }
                ExprT::Live(v) => unaryfn(Doc::text("live"), self.emit_lvalue(env, v)),
                ExprT::Old(v) => unaryfn(Doc::text("old"), self.emit_rvalue(env, v)),
                ExprT::Forall(var, ty, body) | ExprT::Exists(var, ty, body) => {
                    let mut env = env.clone();
                    env.push_var_decl(var, ty.clone(), LocalDeclKind::RValue);
                    let keyword = match &v.val {
                        ExprT::Forall(..) => {
                            if let Some(body_ty) = env.infer_rvalue(body)
                                && matches!(env.vtype_whnf(body_ty).val, TypeT::SLProp)
                            {
                                "forall*"
                            } else {
                                "forall"
                            }
                        }
                        _ => {
                            if let Some(body_ty) = env.infer_rvalue(body)
                                && matches!(env.vtype_whnf(body_ty).val, TypeT::SLProp)
                            {
                                "exists*"
                            } else {
                                "exists"
                            }
                        }
                    };
                    parens(
                        Doc::text(keyword)
                            .append(Doc::line())
                            .append(parens(
                                self.nm
                                    .emit(Name::Var(var.val.clone()))
                                    .append(":")
                                    .append(Doc::space())
                                    .append(self.emit_type(&env, ty)),
                            ))
                            .append(".")
                            .append(Doc::line())
                            .append(self.emit_rvalue(&env, body)),
                    )
                }
                ExprT::StructInit(name, fields) => Doc::text("{")
                    .append(Doc::concat(fields.iter().map(|(fld, val)| {
                        Doc::line()
                            .append(self.nm.emit(Name::StructDirectFieldName(
                                name.val.clone(),
                                fld.val.clone(),
                            )))
                            .append("=")
                            .append(self.emit_rvalue(env, val))
                            .append(";")
                    })))
                    .nest(2)
                    .append(Doc::line())
                    .append("}")
                    .group(),
                ExprT::Malloc(ty) => parens(
                    Doc::text("Pulse.Lib.C.Ref.alloc_ref")
                        .append(Doc::line())
                        .append(Doc::text("#"))
                        .append(self.emit_type(env, ty))
                        .append(Doc::line())
                        .append("()"),
                ),
                ExprT::MallocArray(ty, count) => parens(
                    Doc::text("Pulse.Lib.C.Array.alloc_array")
                        .append(Doc::line())
                        .append(Doc::text("#"))
                        .append(self.emit_type(env, ty))
                        .append(Doc::line())
                        .append(self.emit_rvalue(env, count)),
                ),
                ExprT::Free(val) => {
                    let is_array = env
                        .infer_expr(val)
                        .map(|ty| {
                            matches!(
                                env.vtype_whnf(ty).val,
                                TypeT::Pointer(_, PointerKind::Array)
                            )
                        })
                        .unwrap_or(false);
                    let func = if is_array {
                        "Pulse.Lib.C.Array.free_array"
                    } else {
                        "Pulse.Lib.C.Ref.free_ref"
                    };
                    parens(
                        Doc::text(func)
                            .append(Doc::line())
                            .append(self.emit_rvalue(env, val)),
                    )
                }
            }
        })
    }

    fn emit_stmt(&mut self, env: &Env, stmt: &Stmt) -> Doc {
        annotated(stmt, {
            match &stmt.val {
                StmtT::Call(v) => self.emit_rvalue(env, v).append(";").nest(2).group(),
                StmtT::Decl(x, ty) => {
                    let x = self.nm.emit(Name::Var(x.val.clone()));
                    (Doc::text("let mut ").append(x).append(" :"))
                        .append(Doc::line())
                        .append(self.emit_type(env, ty))
                        .append(";")
                        .nest(2)
                        .group()
                }
                StmtT::Assign(x, t) => {
                    if let ExprT::Index(arr, idx) = &x.val {
                        // Array write: arr.(idx) <- val;
                        self.emit_rvalue(env, arr)
                            .append(Doc::text(".("))
                            .append(self.emit_rvalue(env, idx))
                            .append(Doc::text(")"))
                            .append(Doc::line())
                            .append("<-")
                            .group()
                            .append(Doc::line())
                            .append(self.emit_rvalue(env, t))
                            .append(";")
                            .group()
                            .nest(2)
                    } else {
                        self.emit_lvalue(env, x)
                            .append(Doc::line())
                            .append(":=")
                            .group()
                            .append(Doc::line())
                            .append(self.emit_rvalue(env, t))
                            .append(";")
                            .group()
                            .nest(2)
                    }
                }
                StmtT::If(c, b1, b2) => Doc::text("if ")
                    .append(parens(self.emit_rvalue(env, c)))
                    .nest(2)
                    .append(" ")
                    .append(self.emit_block(env, b1))
                    .append(" else ")
                    .append(self.emit_block(env, b2))
                    .append(";")
                    .group(),
                StmtT::While {
                    cond,
                    inv,
                    requires,
                    ensures,
                    body,
                } => Doc::text("while ")
                    .append(parens(self.emit_rvalue(env, cond)))
                    .append(Doc::line())
                    .append(Doc::concat(inv.iter().map(|inv| {
                        Doc::text("invariant ")
                            .append(self.emit_rvalue(env, inv))
                            .group()
                            .nest(2)
                            .append(Doc::line())
                    })))
                    .append(Doc::concat(requires.iter().map(|r| {
                        Doc::text("requires ")
                            .append(self.emit_rvalue(env, r))
                            .group()
                            .nest(2)
                            .append(Doc::line())
                    })))
                    .append(Doc::concat(ensures.iter().map(|e| {
                        Doc::text("ensures ")
                            .append(self.emit_rvalue(env, e))
                            .group()
                            .nest(2)
                            .append(Doc::line())
                    })))
                    .nest(2)
                    .append(self.emit_block(env, body))
                    .append(";")
                    .group(),
                StmtT::Break => Doc::text("break;"),
                StmtT::Continue => Doc::text("continue;"),
                StmtT::Return(Some(t)) => Doc::text("return")
                    .append(Doc::line())
                    .append(self.emit_rvalue(env, t))
                    .append(";")
                    .group()
                    .nest(2),
                StmtT::Return(None) => Doc::text("return;"),
                StmtT::Assert(v) => Doc::text("assert")
                    .append(Doc::line())
                    .append(self.emit_rvalue(env, v))
                    .append(";")
                    .group()
                    .nest(2),
                StmtT::GhostStmt(code) => Doc::concat(code.tokens.iter().map(|tok| {
                    match tok {
                        InlinePulseToken::Verbatim(ct) => Doc::text(ct.before)
                            .append(annotated(&ct.text, Doc::text(ct.text.val.to_string()))),
                        InlinePulseToken::RValueAntiquot { before, expr } => {
                            Doc::text(*before).append(self.emit_rvalue(env, expr))
                        }
                        InlinePulseToken::LValueAntiquot { before, expr } => {
                            Doc::text(*before).append(self.emit_expr(env, expr).into_doc())
                        }
                    }
                }))
                .append(";"),
                StmtT::Goto(label) => Doc::text("goto ")
                    .append(self.nm.emit(Name::Var(label.val.clone())))
                    .append(";"),
                StmtT::Label { .. } => Doc::text("(* unrestructured label *)"),
                StmtT::GotoBlock {
                    body,
                    label,
                    ensures,
                } => {
                    let mut doc = block(self.emit_stmts(env, body));
                    for e in ensures.iter() {
                        doc = doc
                            .append(Doc::hardline())
                            .append("ensures ")
                            .append(self.emit_rvalue(env, e));
                    }
                    doc.append(Doc::hardline())
                        .append("label ")
                        .append(self.nm.emit(Name::Var(label.val.clone())))
                        .append(":;")
                }
                StmtT::Error => Doc::text("(admit());"),
            }
        })
    }
} // impl Emitter (group B)

fn block(stmts: Doc) -> Doc {
    Doc::text("{")
        .append(stmts.nest(2))
        .append(Doc::hardline())
        .append(Doc::text("}"))
        .group()
}

impl<'a> Emitter<'a> {
    fn emit_stmts(&mut self, env: &Env, stmts: &Vec<Rc<Stmt>>) -> Doc {
        let mut env = env.clone();
        Doc::concat(stmts.iter().map(|stmt| {
            let doc = Doc::line().append(self.emit_stmt(&env, stmt));
            env.push_stmt(stmt);
            doc
        }))
    }

    fn emit_block(&mut self, env: &Env, stmts: &Vec<Rc<Stmt>>) -> Doc {
        if stmts.is_empty() {
            return Doc::text("{}");
        }
        block(self.emit_stmts(env, stmts))
    }
} // impl Emitter (group C)

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

fn mk_eager_unfold_slprop(n: Doc, args: &[Doc], body: Doc) -> Doc {
    (Doc::text("[@@pulse_eager_unfold]")
        .append(Doc::line())
        .append("let")
        .append(Doc::line())
        .append("predicate")
        .group()
        .append(Doc::line())
        .append(n))
    .group()
    .append(
        Doc::concat(args.iter().map(|arg| Doc::line().append(arg.clone())))
            .nest(2)
            .append(Doc::line().append("=").group()),
    )
    .group()
    .nest(2)
    .group()
    .append(Doc::line().append(body))
    .group()
    .nest(2)
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

impl<'a> Emitter<'a> {
    fn emit_typedef(&mut self, env: &Env, TypeDefn { name, body }: &TypeDefn) -> Doc {
        let k = &TypeRefKind::Typedef(name.clone());
        let t = self.nm.emit(Name::TypeRef(k.into()));
        let ty_decl = mk_let(t.clone(), &[], Doc::text("Type"), self.emit_type(env, body));
        let env = &mut env.clone();
        let this = env
            .push_this(TypeT::TypeRef(k.clone()).with_loc(name.loc.clone()))
            .with_loc(name.loc.clone());
        let this_doc = self.nm.emit(Name::Var(this.val.clone()));
        let this_args = vec![parens(this_doc.append(":").append(Doc::line()).append(t))];
        let mut req = vec![];
        let mut ens = vec![];
        self.emit_type_slprop(env, body, &mut req, &mut ens, &mk_rvar(&this));
        let pre_decl = mk_eager_unfold_slprop(
            self.nm.emit(Name::TypeRefPred(k.into())),
            &this_args,
            mk_star(req),
        );
        let post_decl = mk_eager_unfold_slprop(
            self.nm.emit(Name::TypeRefPredPost(k.into())),
            &this_args,
            mk_star(ens),
        );
        Doc::intersperse(vec![ty_decl, pre_decl, post_decl], Doc::line())
    }
} // impl Emitter (group D)

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

impl<'a> Emitter<'a> {
    fn emit_structdefn(&mut self, env: &Env, StructDefn { name, fields }: &StructDefn) -> Doc {
        let k = &TypeRefKind::Struct(name.clone());
        let struct_type_name = self.nm.emit(Name::TypeRef(k.into()));
        let pts_to_name = self.nm.emit(Name::TypeRefPred(k.into()));
        let pts_to_name_post = self.nm.emit(Name::TypeRefPredPost(k.into()));
        let ref_struct_type = unaryfn(Doc::text("ref"), struct_type_name.clone());

        let direct_fld =
            |fld: &Ident| Name::StructDirectFieldName(name.val.clone(), fld.val.clone());

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
                        self.nm
                            .emit(direct_fld(fld))
                            .append(":")
                            .append(Doc::line())
                            .append(self.emit_type(env, fld_ty))
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

        // Generate struct pred/post by gathering slprops from fields
        let env = &mut env.clone();
        let this = env
            .push_this(TypeT::TypeRef(k.clone()).with_loc(name.loc.clone()))
            .with_loc(name.loc.clone());
        let this_args = vec![parens(
            Doc::text("[@@@mkey] ")
                .append(self.nm.emit(Name::Var(this.val.clone())))
                .append(":")
                .append(Doc::line())
                .append(struct_type_name.clone()),
        )];
        let mut req = vec![];
        let mut ens = vec![];
        for (fld, fld_ty) in fields {
            let field_expr =
                ExprT::Member(mk_rvar(&this), fld.clone().into()).with_loc(fld.loc.clone());
            self.emit_type_slprop(env, fld_ty, &mut req, &mut ens, &field_expr);
        }
        ses.push(mk_eager_unfold_slprop(
            pts_to_name.clone(),
            &this_args,
            mk_star(req),
        ));
        ses.push(mk_eager_unfold_slprop(
            pts_to_name_post.clone(),
            &this_args,
            mk_star(ens),
        ));

        let unfolded_tok = self
            .nm
            .emit(Name::StructAuxFn(name.val.clone(), "raw_unfolded".into()));
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
            let ll_type = self.emit_type(env, fld_ty);

            ses.push(mk_assume_val(
                self.nm.emit(ghost_fld(fld)),
                &[parens(
                    Doc::text("x:")
                        .append(Doc::line())
                        .append(ref_struct_type.clone()),
                )],
                Doc::text("GTot")
                    .append(Doc::line())
                    .append(unaryfn(Doc::text("ref"), ll_type))
                    .group(),
            ));
        }

        ses.push(
            Doc::text("[@@pulse_intro]")
                .append(Doc::line())
                .append(mk_assume_val(
                    self.nm
                        .emit(Name::StructAuxFn(name.val.clone(), "raw_unfold".into())),
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
                                    unaryfn(self.nm.emit(ghost_fld(fld)), Doc::text("x")),
                                    Doc::text("vx.").append(self.nm.emit(direct_fld(fld))),
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
                    self.nm
                        .emit(Name::StructAuxFn(name.val.clone(), "raw_fold".into())),
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
                                    unaryfn(self.nm.emit(ghost_fld(fld)), Doc::text("x")),
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
                                        .append(self.nm.emit(direct_fld(fld)))
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
                    self.nm.emit(Name::StructAuxFn(
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
                                    unaryfn(self.nm.emit(ghost_fld(fld)), Doc::text("x")),
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
            let ll_ty = self.emit_type(env, fld_ty);
            let fld_pts_to = naryfn([
                Doc::text("pts_to"),
                unaryfn(self.nm.emit(ghost_fld(fld)), Doc::text("x")),
                Doc::text("vx"),
            ]);
            ses.push(mk_assume_val(
                self.nm
                    .emit(Name::StructFieldProj(name.val.clone(), fld.val.clone())),
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
                                unaryfn(self.nm.emit(ghost_fld(fld)), Doc::text("x")),
                            ]),
                        ]),
                    ),
                ]),
            ))
        }

        Doc::intersperse(ses.into_iter().map(|se| se.group()), Doc::hardline())
    }

    fn emit_fn_decl(
        &mut self,
        env: &Env,
        FnDecl {
            name,
            ret_type,
            args,
            requires,
            ensures,
            is_pure: _,
        }: &FnDecl,
    ) -> Doc {
        let env = &mut env.clone();

        let mut requires_props = vec![];
        let mut ensures_props = vec![];
        let mut params = vec![];
        for (i, arg @ (n0, ty)) in args.iter().enumerate() {
            let n: Rc<Ident> = n0.clone().unwrap_or_else(|| {
                Rc::<str>::from(format!("_unnamed{}", i)).with_loc(ty.loc.clone())
            });

            params.push(parens(
                annotated(&n, self.nm.emit(Name::Var(n.val.clone())))
                    .append(":")
                    .append(Doc::line())
                    .append(self.emit_type(env, ty)),
            ));

            env.push_arg(arg, LocalDeclKind::RValue);
            self.emit_type_slprop(
                env,
                ty,
                &mut requires_props,
                &mut ensures_props,
                &mk_rvar(&n),
            );
        }

        if params.is_empty() {
            params.push(Doc::text("()"));
        }

        requires_props.extend(requires.iter().map(|r| self.emit_rvalue(env, r)));

        let return_id = env
            .push_return(ret_type.clone())
            .with_loc(ret_type.loc.clone());
        self.emit_type_slprop(
            env,
            &ret_type,
            &mut ensures_props,
            &mut vec![],
            &mk_rvar(&return_id),
        );
        let ret_type_doc = self.emit_type(env, ret_type);

        ensures_props.extend(ensures.iter().map(|r| self.emit_rvalue(env, r)));

        let hdr = Doc::group(
            Doc::text("fn")
                .append(Doc::line())
                .append(self.nm.emit(Name::Fn(name.val.clone()))),
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
                .append(self.nm.emit(Name::Var(return_id.val.clone())))
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
} // impl Emitter (group E)

fn emit_inline_code(code: &InlineCode) -> Doc {
    Doc::concat(code.tokens.iter().map(|tok| {
        Doc::text(tok.before).append(annotated(&tok.text, Doc::text(tok.text.val.to_string())))
    }))
}

/// Append remaining statements to a block (for if/else continuation in pure functions).
fn append_rest(block_stmts: &[Rc<Stmt>], rest: &[Rc<Stmt>]) -> Vec<Rc<Stmt>> {
    block_stmts.iter().chain(rest.iter()).cloned().collect()
}

impl<'a> Emitter<'a> {
    /// Emit a pure function spec prop: strip the outer Cast(_, SLProp) wrapper
    /// and emit the inner boolean expression directly.
    fn emit_pure_prop(&mut self, env: &Env, expr: &Expr) -> Doc {
        match &expr.val {
            ExprT::Cast(inner, ty) if matches!(ty.val, TypeT::SLProp) => {
                self.emit_rvalue(env, inner)
            }
            _ => {
                self.report(
                    format!("pure function spec must be a boolean expression"),
                    &expr.loc,
                );
                Doc::text("True")
            }
        }
    }

    /// Check that a parameter type is valid for a pure function (no pointers, arrays, etc.)
    fn check_pure_type(&mut self, ty: &Type) {
        match &ty.val {
            TypeT::Void
            | TypeT::Bool
            | TypeT::Int { .. }
            | TypeT::SizeT
            | TypeT::SpecInt
            | TypeT::SLProp
            | TypeT::Error
            | TypeT::TypeRef(_) => {}
            TypeT::Pointer(_, _) => {
                self.report(
                    format!("pointer parameters are not supported in pure functions"),
                    &ty.loc,
                );
            }
            TypeT::Requires(inner, _)
            | TypeT::Ensures(inner, _)
            | TypeT::Consumes(inner)
            | TypeT::Plain(inner) => self.check_pure_type(inner),
        }
    }

    fn emit_pure_body(&mut self, env: &Env, stmts: &[Rc<Stmt>]) -> Doc {
        if stmts.is_empty() {
            return Doc::text("()");
        }

        match &stmts[0].val {
            StmtT::Return(Some(e)) if stmts.len() == 1 => self.emit_rvalue(env, e),
            StmtT::Return(None) if stmts.len() == 1 => Doc::text("()"),

            StmtT::If(cond, then_body, else_body) => {
                let rest = &stmts[1..];
                let then_stmts = append_rest(then_body, rest);
                let else_stmts = append_rest(else_body, rest);
                let mut env_then = env.clone();
                for s in &**then_body {
                    env_then.push_stmt(s);
                }
                let mut env_else = env.clone();
                for s in &**else_body {
                    env_else.push_stmt(s);
                }
                parens(
                    Doc::text("if")
                        .append(Doc::line())
                        .append(self.emit_rvalue(env, cond))
                        .group()
                        .append(Doc::line())
                        .append("then")
                        .append(
                            Doc::line()
                                .append(self.emit_pure_body(&env_then, &then_stmts))
                                .nest(2),
                        )
                        .append(Doc::line())
                        .append("else")
                        .append(
                            Doc::line()
                                .append(self.emit_pure_body(&env_else, &else_stmts))
                                .nest(2),
                        ),
                )
            }

            StmtT::Decl(x, ty) => {
                // Look for the assignment to this variable in the next statement
                if stmts.len() >= 3 {
                    if let StmtT::Assign(lhs, rhs) = &stmts[1].val {
                        if let ExprT::Var(v) = &lhs.val {
                            if v.val == x.val {
                                let mut env = env.clone();
                                env.push_var_decl(x, ty.clone(), LocalDeclKind::RValue);
                                let rest_expr = self.emit_pure_body(&env, &stmts[2..]);
                                return parens(
                                    Doc::text("let")
                                        .append(Doc::line())
                                        .append(self.nm.emit(Name::Var(x.val.clone())))
                                        .append(Doc::line())
                                        .append(":")
                                        .append(Doc::line())
                                        .append(self.emit_type(&env, ty))
                                        .append(Doc::line())
                                        .append("=")
                                        .group()
                                        .nest(2)
                                        .append(
                                            Doc::line().append(self.emit_rvalue(&env, rhs)).nest(2),
                                        )
                                        .append(Doc::line())
                                        .append("in")
                                        .append(Doc::line().append(rest_expr).nest(2)),
                                );
                            }
                        }
                    }
                }
                self.report(
                    format!("unsupported declaration without assignment in pure function"),
                    &stmts[0].loc,
                );
                Doc::text("(admit())")
            }

            _ => {
                self.report(
                    format!("unsupported statement in pure function: {}", stmts[0]),
                    &stmts[0].loc,
                );
                Doc::text("(admit())")
            }
        }
    }

    fn emit_pure_fn(&mut self, env: &Env, decl: &FnDecl, body: &Stmts) -> Doc {
        let env = &mut env.clone();

        let mut params = vec![];
        for (i, arg @ (n0, ty)) in decl.args.iter().enumerate() {
            let n: Rc<Ident> = n0.clone().unwrap_or_else(|| {
                Rc::<str>::from(format!("_unnamed{}", i)).with_loc(ty.loc.clone())
            });

            params.push(parens(
                annotated(&n, self.nm.emit(Name::Var(n.val.clone())))
                    .append(":")
                    .append(Doc::line())
                    .append(self.emit_type(env, ty)),
            ));

            env.push_arg(arg, LocalDeclKind::RValue);
        }

        if params.is_empty() {
            params.push(Doc::text("()"));
        }

        let requires_props: Vec<Doc> = decl
            .requires
            .iter()
            .map(|r| self.emit_pure_prop(env, r))
            .collect();
        // Reject non-bool type-level specs on parameters
        for (_n, ty) in &decl.args {
            self.check_pure_type(ty);
        }

        let ret_type_doc = self.emit_type(env, &decl.ret_type);

        let return_id = env
            .push_return(decl.ret_type.clone())
            .with_loc(decl.ret_type.loc.clone());
        let ensures_props: Vec<Doc> = decl
            .ensures
            .iter()
            .map(|e| self.emit_pure_prop(env, e))
            .collect();

        let body_doc = self.emit_pure_body(env, body);

        let has_specs = !requires_props.is_empty() || !ensures_props.is_empty();

        let ty_doc = if has_specs {
            let req_doc = if requires_props.is_empty() {
                Doc::text("True")
            } else {
                Doc::intersperse(requires_props, Doc::text(" /\\ "))
            };
            let ens_doc = if ensures_props.is_empty() {
                Doc::text("True")
            } else {
                Doc::intersperse(ensures_props, Doc::text(" /\\ "))
            };
            naryfn([
                Doc::text("Pure"),
                ret_type_doc,
                parens(Doc::text("requires").append(Doc::line()).append(req_doc)),
                parens(
                    Doc::text("ensures").append(Doc::line()).append(parens(
                        Doc::text("fun")
                            .append(Doc::line())
                            .append(self.nm.emit(Name::Var(return_id.val.clone())))
                            .append(Doc::line())
                            .append("->")
                            .group()
                            .nest(2)
                            .append(Doc::line())
                            .append(ens_doc),
                    )),
                ),
            ])
        } else {
            ret_type_doc
        };

        mk_let(
            self.nm.emit(Name::Fn(decl.name.val.clone())),
            &params,
            ty_doc,
            body_doc,
        )
    }

    fn emit_decl(&mut self, env: &Env, decl: &Decl) -> Doc {
        annotated(decl, {
            match &decl.val {
                DeclT::FnDefn(FnDefn { decl, body }) => {
                    if decl.is_pure {
                        return self.emit_pure_fn(env, decl, body);
                    }
                    let decl_doc = self.emit_fn_decl(env, decl).nest(2).append(Doc::hardline());
                    let arg_redecl_as_mut = Doc::concat(decl.args.iter().filter_map(|(n, _)| {
                        n.as_ref().map(|n| {
                            Doc::line().append(annotated(
                                n,
                                Doc::group({
                                    let n = self.nm.emit(Name::Var(n.val.clone()));
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
                    decl_doc
                        .append(block(arg_redecl_as_mut.append(self.emit_stmts(env, body))).group())
                }
                DeclT::FnDecl(fn_decl) => self.emit_fn_decl(&mut env.clone(), fn_decl),
                DeclT::Typedef(typedef) => self.emit_typedef(env, typedef),
                DeclT::StructDefn(struct_defn) => self.emit_structdefn(env, struct_defn),
                DeclT::IncludeDecl(include_decl) => emit_inline_code(&include_decl.code),
            }
        })
    }
} // impl Emitter (group F)

pub fn emit(
    diags: &mut Diagnostics,
    module_name: &str,
    tu: &TranslationUnit,
) -> (String, SourceRangeMap) {
    let mut env = Env::new();
    let emitter = &mut Emitter {
        nm: NameMangling::new(),
        diags,
    };
    let mut output: Vec<Doc> = vec![];
    output.push(Doc::text(format!(
        "module {}\nopen Pulse\nopen Pulse.Lib.C\n#lang-pulse",
        module_name
    )));
    for decl in &tu.decls {
        output.push(emitter.emit_decl(&env, decl));
        env.push_decl(decl);
    }
    let output = Doc::intersperse(output, Doc::hardline().append(Doc::hardline())).group();
    let mut writer = StrWriter::new();
    output.render_raw(100, &mut writer).unwrap();
    (writer.buffer, writer.source_range_map)
}
