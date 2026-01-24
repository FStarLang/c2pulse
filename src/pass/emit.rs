use std::rc::Rc;

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

fn emit_type(env: &Env, ty: &Type) -> Doc {
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
                unaryfn(Doc::text("array"), emit_type(env, to))
            }
            TypeT::Pointer(to, PointerKind::Ref | PointerKind::Unknown) => {
                unaryfn(Doc::text("ref"), emit_type(env, to))
            }
            TypeT::Error => Doc::text("unit"),

            TypeT::TypeRef(n) => {
                let (t, _pre, _post) = emit_typeref_name(n);
                Doc::text(t)
            }

            TypeT::SLProp => Doc::text("slprop"),
            TypeT::SpecInt => Doc::text("int"),

            TypeT::Requires(ty, _)
            | TypeT::Ensures(ty, _)
            | TypeT::Consumes(ty)
            | TypeT::Plain(ty) => emit_type(env, ty),
        }
    })
}

fn subst_this_lvalue(lvalue: &mut LValue, this: &Rc<Ident>) {
    match &mut lvalue.val {
        LValueT::Var(x) => {
            if &*x.val == "this" {
                *x = this.clone();
            }
        }
        LValueT::Deref(rv) => subst_this_rvalue(Rc::make_mut(rv), this),
        LValueT::Member(x, _a) => subst_this_lvalue(Rc::make_mut(x), this),
        LValueT::Error(_ty) => {}
    }
}

fn subst_this_rvalue(rvalue: &mut RValue, this: &Rc<Ident>) {
    match &mut rvalue.val {
        RValueT::BoolLit(_) => {}
        RValueT::IntLit(..) => {}
        RValueT::LValue(lv) | RValueT::Ref(lv) => subst_this_lvalue(Rc::make_mut(lv), this),
        RValueT::UnOp(_, arg) => {
            subst_this_rvalue(Rc::make_mut(arg), this);
        }
        RValueT::BinOp(_, lhs, rhs) => {
            subst_this_rvalue(Rc::make_mut(lhs), this);
            subst_this_rvalue(Rc::make_mut(rhs), this);
        }
        RValueT::FnCall(_f, args) => {
            for arg in args {
                subst_this_rvalue(Rc::make_mut(arg), this);
            }
        }
        RValueT::Cast(val, _) => {
            subst_this_rvalue(Rc::make_mut(val), this);
        }
        RValueT::InlinePulse(val, _) => subst_inline_code_this(Rc::make_mut(val), this),
        RValueT::Error(_ty) => {}
        RValueT::Live(val) => subst_this_lvalue(Rc::make_mut(val), this),
        RValueT::Old(val) => subst_this_rvalue(Rc::make_mut(val), this),
    }
}

fn subst_inline_code_this(val: &mut InlineCode, this: &Rc<Ident>) {
    for tok in &mut val.tokens {
        // This is ridiculuously hacky....
        if &*tok.text.val == "this" {
            tok.text = (**this).clone();
        }
    }
}

fn emit_typeref_name(k: &TypeRefKind) -> (String, String, String) {
    let t = match k {
        TypeRefKind::Typedef(n) => n.val.to_string(),
        TypeRefKind::Struct(n) => format!("struct_{}", n.val),
    };
    let pre = format!("{}_pre", t);
    let post = format!("{}_post", t);
    (t, pre, post)
}

fn emit_type_slprop(
    env: &Env,
    ty: &Type,
    req: &mut Vec<Doc>,
    ens: &mut Vec<Doc>,
    this: &Rc<Ident>,
) {
    match &ty.val {
        TypeT::Void
        | TypeT::Bool
        | TypeT::Int { .. }
        | TypeT::SizeT
        | TypeT::SpecInt
        | TypeT::SLProp => {}
        TypeT::Pointer(..) => {
            let live = annotated(
                ty,
                unaryfn(Doc::text("live"), Doc::text(this.val.to_string())),
            );
            req.push(live.clone());
            ens.push(live);
        }
        TypeT::TypeRef(n) => {
            let (_t, pre, post) = emit_typeref_name(n);
            let this = Doc::text(this.val.to_string());
            req.push(unaryfn(Doc::text(pre), this.clone()));
            ens.push(unaryfn(Doc::text(post), this));
        }
        TypeT::Requires(ty, p) => {
            emit_type_slprop(env, ty, req, ens, this);

            let p = &mut p.clone();
            subst_this_rvalue(Rc::make_mut(p), this);
            req.push(emit_rvalue(env, p));
        }
        TypeT::Ensures(ty, p) => {
            emit_type_slprop(env, ty, req, ens, this);

            let p = &mut p.clone();
            subst_this_rvalue(Rc::make_mut(p), this);
            ens.push(emit_rvalue(env, p));
        }
        TypeT::Consumes(ty) => emit_type_slprop(env, ty, req, &mut vec![], this),
        TypeT::Plain(_) => {}
        TypeT::Error => {}
    }
}

fn emit_var(v: &Ident) -> Doc {
    annotated(v, Doc::text(v.val.to_string()))
}

fn emit_lvalue(env: &Env, v: &LValue) -> Doc {
    annotated(v, {
        match &v.val {
            LValueT::Var(x) => {
                if let Some(LocalDecl {
                    kind: LocalDeclKind::RValue,
                    ..
                }) = env.lookup_var(x)
                {
                    Doc::text(format!(
                        "admit() (* illegal lvalue reference to variable {} *)",
                        v
                    ))
                } else {
                    emit_var(x)
                }
            }
            LValueT::Deref(v) => emit_rvalue(env, v),
            LValueT::Member(x, a) => {
                Doc::text(format!("(*TODO field access {}*)", a)).append(emit_lvalue(env, x))
            }
            LValueT::Error(_ty) => Doc::text("(admit())"),
        }
    })
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

fn emit_rvalue(env: &Env, v: &RValue) -> Doc {
    annotated(v, {
        match &v.val {
            RValueT::BoolLit(v) => Doc::text(if *v { "true" } else { "false" }),
            RValueT::IntLit(val, ty) => match ty.val {
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
            RValueT::LValue(v) => {
                if let LValueT::Var(x) = &v.val
                    && let Some(LocalDecl {
                        kind: LocalDeclKind::RValue,
                        ..
                    }) = env.lookup_var(&x)
                {
                    emit_var(x)
                } else {
                    parens(Doc::text("!").append(emit_lvalue(env, v)))
                }
            }
            RValueT::Ref(v) => emit_lvalue(env, v),
            RValueT::Cast(val, to_ty) => {
                let val_doc = emit_rvalue(env, val);
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
            RValueT::Error(_ty) => Doc::text("(admit())"),
            RValueT::InlinePulse(val, _) => parens(Doc::concat(val.tokens.iter().map(|tok| {
                Doc::text(tok.before)
                    .append(annotated(&tok.text, Doc::text(tok.text.val.to_string())))
            }))),
            RValueT::BinOp(BinOp::LogAnd, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(lhs) {
                    if ty.val == TypeT::SLProp {
                        return binop(
                            emit_rvalue(env, lhs),
                            Doc::text("**"),
                            emit_rvalue(env, rhs),
                        );
                    }
                }
                binop(
                    emit_rvalue(env, lhs),
                    Doc::text("&&"),
                    emit_rvalue(env, rhs),
                )
            }
            RValueT::BinOp(BinOp::Eq, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(lhs) {
                    let ty = env.vtype_whnf(ty);
                    match (&ty.val, &rhs.val) {
                        (
                            TypeT::Pointer(
                                _,
                                PointerKind::Ref | PointerKind::Unknown, /* TODO */
                            ),
                            RValueT::IntLit(n, _),
                        ) => {
                            if **n == BigInt::ZERO {
                                return unaryfn(
                                    Doc::text("Pulse.Lib.Reference.is_null"),
                                    emit_rvalue(env, lhs),
                                );
                            }
                        }
                        _ => {}
                    }
                }
                // TODO: this should be == in ghost contexts
                binop(emit_rvalue(env, lhs), Doc::text("="), emit_rvalue(env, rhs))
            }
            RValueT::UnOp(op, arg) => {
                if let Some(ty) = env.infer_rvalue(&arg)
                    && let Some(op) = emit_unop(env, *op, ty)
                {
                    unaryfn(op, emit_rvalue(env, arg))
                } else {
                    // TODO: error
                    Doc::text("(*unsupported binop*)(admit())")
                }
            }
            RValueT::BinOp(op, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(&lhs)
                    && let Some(op) = emit_binop(env, *op, ty)
                {
                    binop(emit_rvalue(env, lhs), op, emit_rvalue(env, rhs))
                } else {
                    // TODO: error
                    Doc::text("(*unsupported binop*)(admit())")
                }
            }
            RValueT::FnCall(f, args) => {
                let args = if args.is_empty() {
                    Doc::text("()")
                } else {
                    Doc::intersperse(args.iter().map(|arg| emit_rvalue(env, arg)), Doc::line())
                };
                parens(
                    Doc::text(f.val.to_string())
                        .append(Doc::line())
                        .append(args),
                )
            }
            RValueT::Live(v) => unaryfn(Doc::text("live"), emit_lvalue(env, v)),
            RValueT::Old(v) => unaryfn(Doc::text("old"), emit_rvalue(env, v)),
        }
    })
}

fn emit_stmt(env: &Env, stmt: &Stmt) -> Doc {
    annotated(stmt, {
        match &stmt.val {
            StmtT::Call(v) => emit_rvalue(env, v).append(";").nest(2).group(),
            StmtT::Decl(x, ty) => (Doc::text("let mut ").append(x.val.to_string()).append(" :"))
                .append(Doc::line())
                .append(emit_type(env, ty))
                .append(";")
                .nest(2)
                .group(),
            StmtT::Assign(x, t) => emit_lvalue(env, x)
                .append(Doc::line())
                .append(":=")
                .group()
                .append(Doc::line())
                .append(emit_rvalue(env, t))
                .append(";")
                .group()
                .nest(2),
            StmtT::If(c, b1, b2) => Doc::text("if ")
                .append(parens(emit_rvalue(env, c)))
                .nest(2)
                .append(" ")
                .append(emit_block(env, b1))
                .append(" else ")
                .append(emit_block(env, b2))
                .append(";")
                .group(),
            StmtT::While(cond, invs, body) => Doc::text("while ")
                .append(parens(emit_rvalue(env, cond)))
                .append(Doc::line())
                .append(Doc::concat(invs.iter().map(|inv| {
                    Doc::text("invariant ")
                        .append(emit_rvalue(env, inv))
                        .group()
                        .nest(2)
                        .append(Doc::line())
                })))
                .nest(2)
                .append(emit_block(env, body))
                .append(";")
                .group(),
            StmtT::Return(t) => emit_rvalue(env, t).append(";").group().nest(2),
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

fn emit_stmts(env: &Env, stmts: &Vec<Rc<Stmt>>) -> Doc {
    let mut env = env.clone();
    Doc::concat(stmts.iter().map(|stmt| {
        let doc = Doc::line().append(emit_stmt(&env, stmt));
        env.push_stmt(stmt);
        doc
    }))
}

fn emit_block(env: &Env, stmts: &Vec<Rc<Stmt>>) -> Doc {
    if stmts.is_empty() {
        return Doc::text("{}");
    }
    block(emit_stmts(env, stmts))
}

fn mk_let(n: Doc, args: &Vec<Doc>, ty: Doc, body: Doc) -> Doc {
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

fn mk_eager_unfold_let(n: Doc, args: &Vec<Doc>, ty: Doc, body: Doc) -> Doc {
    Doc::text("[@@pulse_eager_unfold]")
        .append(Doc::line())
        .append(mk_let(n, args, ty, body))
        .group()
}

fn mk_star<I: Iterator<Item = Doc>>(ps: I) -> Doc {
    ps.reduce(|accum, p| {
        accum
            .append(Doc::space())
            .append("**")
            .append(Doc::line())
            .append(p)
    })
    .unwrap_or_else(|| Doc::text("emp"))
    .group()
}

fn emit_typedef(env: &Env, TypeDefn { name, body }: &TypeDefn) -> Doc {
    let k = TypeRefKind::Typedef(name.clone());
    let (t, pre, post) = emit_typeref_name(&k);
    let t = Doc::text(t);
    let ty_decl = mk_let(t.clone(), &vec![], Doc::text("Type"), emit_type(env, body));
    let env = &mut env.clone();
    env.push_this(TypeT::TypeRef(k).with_loc(name.loc.clone()));
    let this = Rc::<str>::from("this").with_loc(name.loc.clone());
    let this_args = vec![parens(Doc::text("this:").append(Doc::line()).append(t))];
    let mut req = vec![];
    let mut ens = vec![];
    emit_type_slprop(env, body, &mut req, &mut ens, &this);
    let pre_decl = mk_eager_unfold_let(
        Doc::text(pre),
        &this_args,
        Doc::text("slprop"),
        mk_star(req.into_iter()),
    );
    let post_decl = mk_eager_unfold_let(
        Doc::text(post),
        &this_args,
        Doc::text("slprop"),
        mk_star(ens.into_iter()),
    );
    Doc::intersperse(vec![ty_decl, pre_decl, post_decl], Doc::line())
}

fn emit_fn_decl(
    env: &Env,
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
            annotated(&n, Doc::text(n.val.to_string()))
                .append(":")
                .append(Doc::line())
                .append(emit_type(env, ty)),
        ));

        env.push_arg(arg, LocalDeclKind::RValue);
        emit_type_slprop(env, ty, &mut requires_props, &mut ensures_props, &n);
    }

    if params.is_empty() {
        params.push(Doc::text("()"));
    }

    requires_props.extend(requires.iter().map(|r| emit_rvalue(env, r)));

    let return_id = Rc::<str>::from("return").with_loc(ret_type.loc.clone());
    emit_type_slprop(env, &ret_type, &mut ensures_props, &mut vec![], &return_id);
    let ret_type_doc = emit_type(env, ret_type);

    env.push_return(ret_type.clone());
    ensures_props.extend(ensures.iter().map(|r| emit_rvalue(env, r)));

    let hdr = Doc::group(
        Doc::text("fn")
            .append(Doc::line())
            .append(name.val.to_string()),
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
            .append(return_id.val.to_string())
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

fn emit_decl(env: &Env, decl: &Decl) -> Doc {
    let todo = || Doc::text(format!("(*TODO:\n{:#?}\n*)", decl));
    annotated(decl, {
        match &decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                let decl_doc = emit_fn_decl(env, decl).nest(2).append(Doc::hardline());
                let arg_redecl_as_mut = Doc::concat(decl.args.iter().filter_map(|(n, _)| {
                    n.as_ref().map(|n| {
                        Doc::line().append(annotated(
                            n,
                            Doc::group(
                                Doc::text("let mut ")
                                    .append(n.val.to_string())
                                    .append(" = ")
                                    .append(n.val.to_string())
                                    .append(";"),
                            ),
                        ))
                    })
                }));
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                decl_doc.append(block(arg_redecl_as_mut.append(emit_stmts(env, body))).group())
            }
            DeclT::FnDecl(fn_decl) => emit_fn_decl(&mut env.clone(), fn_decl),
            DeclT::Typedef(typedef) => emit_typedef(env, typedef),
            DeclT::StructDefn(_struct_defn) => todo(),
            DeclT::IncludeDecl(include_decl) => emit_inline_code(&include_decl.code),
        }
    })
}

pub fn emit(module_name: &str, tu: &TranslationUnit) -> (String, SourceRangeMap) {
    let mut env = Env::new();
    let mut output: Vec<Doc> = vec![];
    output.push(Doc::text(format!(
        "module {}\nopen Pulse\nopen Pulse.Lib.C\n#lang-pulse",
        module_name
    )));
    for decl in &tu.decls {
        output.push(emit_decl(&env, decl));
        env.push_decl(decl);
    }
    let output = Doc::intersperse(output, Doc::hardline().append(Doc::hardline())).group();
    let mut writer = StrWriter::new();
    output.render_raw(100, &mut writer).unwrap();
    (writer.buffer, writer.source_range_map)
}
