use std::{fmt::Pointer, rc::Rc};

use pretty::{RcDoc, Render, RenderAnnotated};

use crate::{
    env::{Env, LocalDecl, LocalDeclKind},
    ir::*,
};

pub type SourceRangeMap = Vec<(Location, Range)>;

type Annotation = Location;
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
            loc,
            Range {
                start,
                end: self.cur_pos(),
            },
        ));
        Ok(())
    }
}

fn annotated<T>(ast: &Ast<T>, doc: Doc) -> Doc {
    match &*ast.loc {
        SourceInfo::None => doc,
        SourceInfo::Original(location) => doc.annotate(location.clone()),
    }
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

            TypeT::Pointer {
                to,
                kind: PointerKind::Array,
            } => unaryfn(Doc::text("array"), emit_type(env, to)),
            TypeT::Pointer {
                to,
                kind: PointerKind::Ref | PointerKind::Unknown,
            } => unaryfn(Doc::text("ref"), emit_type(env, to)),
            TypeT::Error => Doc::text("unit"),
            TypeT::SLProp => Doc::text("slprop"),
        }
    })
}

fn emit_type_slprop(env: &Env, ty: &Type, req: &mut Vec<Doc>, ens: &mut Vec<Doc>, this: &Ident) {
    match &ty.val {
        TypeT::Void | TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT | TypeT::SLProp => {}
        TypeT::Pointer { to: _, kind: _ } => {
            let live = annotated(
                ty,
                unaryfn(Doc::text("live"), Doc::text(this.val.to_string())),
            );
            req.push(live.clone());
            ens.push(live);
        }
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
                    panic!("illegal lvalue reference to variable {:?}", v)
                } else {
                    emit_var(x)
                }
            }
            LValueT::Deref(v) => emit_rvalue(env, v),
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

fn emit_binop(op: BinOp, ty: &Type) -> Option<Doc> {
    Some(match (op, &ty.val) {
        (BinOp::Eq, TypeT::SLProp | TypeT::Void) => Doc::text("=="),
        (BinOp::Eq, TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT | TypeT::Pointer { .. }) => {
            Doc::text("=")
        }

        (BinOp::LogAnd, TypeT::Bool) => Doc::text("&&"),
        (BinOp::Div, TypeT::Bool) => todo!(),
        (BinOp::Mod, TypeT::Bool) => todo!(),
        (BinOp::Sub, TypeT::Bool) => todo!(),
        (BinOp::Add, TypeT::Bool) => todo!(),
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
            Doc::text(format!("`{}.mod`", get_int_mod(signed, width)?))
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

        (BinOp::Mul | BinOp::Div | BinOp::Mod | BinOp::Add | BinOp::Sub, TypeT::Pointer { .. })
        | (_, TypeT::Void)
        | (BinOp::LogAnd, TypeT::Int { .. } | TypeT::SizeT | TypeT::Pointer { .. })
        | (_, TypeT::SLProp)
        | (_, TypeT::Error) => return None,
    })
}

fn emit_rvalue(env: &Env, v: &RValue) -> Doc {
    annotated(v, {
        match &v.val {
            RValueT::BoolLit(v) => Doc::text(if *v { "true" } else { "false" }),
            RValueT::IntLit { val, ty } => {
                // TODO
                Doc::text(val.to_string())
            }
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
            RValueT::Cast { val, ty } => {
                let val_doc = emit_rvalue(env, val);
                let val_ty = env.infer_rvalue(val).unwrap();
                match (&val_ty.val, &ty.val) {
                    (TypeT::Void, TypeT::Void) => val_doc,
                    (TypeT::Bool, TypeT::Bool) => val_doc,
                    // (TypeT::Bool, TypeT::Int { signed, width }) => todo!(),
                    // (TypeT::Bool, TypeT::SizeT) => todo!(),
                    (TypeT::Bool, TypeT::SLProp) => unaryfn(Doc::text("pure"), val_doc),
                    // (TypeT::Int { signed, width }, TypeT::Bool) => todo!(),
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
                    // (TypeT::Int { signed:s1, width:w1 }, TypeT::Int { signed:s2, width:w2 }) => todo!(),
                    // (TypeT::Int { signed, width }, TypeT::SizeT) => todo!(),
                    // (TypeT::Int { signed, width }, TypeT::SLProp) => todo!(),
                    // (TypeT::SizeT, TypeT::Bool) => todo!(),
                    // (TypeT::SizeT, TypeT::Int { signed, width }) => todo!(),
                    (TypeT::SizeT, TypeT::SizeT) => val_doc,
                    // (TypeT::SizeT, TypeT::SLProp) => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::Bool) => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::SizeT) => todo!(),
                    (TypeT::Pointer { to: t1, kind: k1 }, TypeT::Pointer { to: t2, kind: k2 })
                        if t1 == t2 && k1 == k2 =>
                    {
                        val_doc
                    }
                    // (TypeT::Pointer { to:t1, kind:k1 }, TypeT::Pointer { to:t2, kind:k2 }) if t1 == t2 => todo!(),
                    // (TypeT::Pointer { to, kind }, TypeT::SLProp) => todo!(),
                    (TypeT::Error, _) | (_, TypeT::Error) => val_doc,
                    _ => Doc::text("(*TODO unsupported cast*)").append(val_doc),
                }
            }
            RValueT::Error(_ty) => Doc::text("(admit())"),
            RValueT::InlinePulse { val, ty: _ } => {
                parens(Doc::concat(val.tokens.iter().map(|tok| {
                    Doc::text(tok.before)
                        .append(annotated(&tok.text, Doc::text(tok.text.val.to_string())))
                })))
            }
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
                // TODO: this should be == in ghost contexts
                binop(emit_rvalue(env, lhs), Doc::text("="), emit_rvalue(env, rhs))
            }
            RValueT::BinOp(op, lhs, rhs) => {
                if let Some(ty) = env.infer_rvalue(&lhs)
                    && let Some(op) = emit_binop(*op, &ty)
                {
                    binop(emit_rvalue(env, lhs), op, emit_rvalue(env, rhs))
                } else {
                    // TODO: error
                    Doc::text("(*unsupported binop*)(admit())")
                }
            }
            RValueT::FnCall(f, args) => parens(
                Doc::text(f.val.to_string())
                    .append(Doc::line())
                    .append(Doc::intersperse(
                        args.iter().map(|arg| emit_rvalue(env, arg)),
                        Doc::line(),
                    )),
            ),
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
                .nest(2)
                .append(Doc::line())
                .append("=")
                .group()
                .append(Doc::line())
                .append("witness #_ #_;")
                .nest(2)
                .append(Doc::line())
                .append("assert live ")
                .append(x.val.to_string())
                .append(";")
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
                .append(" ")
                .append(emit_block(env, b1))
                .append(" else ")
                .append(emit_block(env, b2))
                .append(";")
                .group()
                .nest(2),
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

        emit_type_slprop(env, ty, &mut requires_props, &mut ensures_props, &n);

        env.push_arg(arg, LocalDeclKind::RValue);
    }

    let return_id = Rc::<str>::from("return").with_loc(ret_type.loc.clone());
    emit_type_slprop(env, &ret_type, &mut ensures_props, &mut vec![], &return_id);

    requires_props.extend(requires.iter().map(|r| emit_rvalue(env, r)));
    ensures_props.extend(ensures.iter().map(|r| emit_rvalue(env, r)));

    let hdr = Doc::group(
        Doc::text("fn")
            .append(Doc::line())
            .append(name.val.to_string()),
    )
    .append(Doc::concat(params.into_iter().map(|p| Doc::line().append(p))).nest(2))
    .group()
    .append(Doc::hardline());

    hdr.append(Doc::group(
        Doc::text("returns")
            .append(Doc::line())
            .append(return_id.val.to_string())
            .append(Doc::line())
            .append(":")
            .group()
            .append(Doc::line())
            .append(emit_type(env, ret_type)),
    ))
    .append(Doc::concat(requires_props.into_iter().map(|r| {
        Doc::hardline().append(
            Doc::text("requires")
                .append(Doc::line())
                .append(r)
                .nest(2)
                .group(),
        )
    })))
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
            DeclT::StructDefn(struct_defn) => todo(),
            DeclT::IncludeDecl(include_decl) => emit_inline_code(&include_decl.code),
        }
    })
}

pub fn emit(tu: &TranslationUnit) -> (String, SourceRangeMap) {
    let mut env = Env::new();
    let mut output: Vec<Doc> = vec![];
    output.push(Doc::text("module TODO\nopen Pulse\n#lang-pulse"));
    for decl in &tu.decls {
        output.push(emit_decl(&env, decl));
        env.push_decl(decl);
    }
    let output = Doc::intersperse(output, Doc::hardline().append(Doc::hardline())).group();
    let mut writer = StrWriter::new();
    output.render_raw(100, &mut writer).unwrap();
    (writer.buffer, writer.source_range_map)
}
