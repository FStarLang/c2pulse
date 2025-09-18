use std::rc::Rc;

use num_bigint::BigInt;
use pretty::{RcDoc, Render, RenderAnnotated};

use crate::{env::Env, ir::*};

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
            } => parens(
                Doc::text("array")
                    .append(Doc::line())
                    .append(emit_type(env, to)),
            ),
            TypeT::Pointer {
                to,
                kind: PointerKind::Ref | PointerKind::Unknown,
            } => parens(
                Doc::text("ref")
                    .append(Doc::line())
                    .append(emit_type(env, to)),
            ),
            TypeT::Error => Doc::text("unit"),
            TypeT::SLProp => Doc::text("slprop"),
        }
    })
}

fn emit_lvalue(env: &Env, v: &LValue) -> Doc {
    annotated(v, {
        match &v.val {
            LValueT::Var(x) => Doc::text(x.val.to_string()),
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

fn emit_rvalue(env: &Env, v: &RValue) -> Doc {
    annotated(v, {
        match &v.val {
            RValueT::BoolLit(v) => Doc::text(if *v { "true" } else { "false" }),
            RValueT::IntLit { val, ty } => {
                // TODO
                Doc::text(val.to_string())
            }
            RValueT::LValue(v) => parens(Doc::text("!").append(emit_lvalue(env, v))),
            RValueT::Ref(v) => emit_lvalue(env, v),
            RValueT::Cast { val, ty } => Doc::text("(*TODO cast*)").append(emit_rvalue(env, val)),
            RValueT::Error(_ty) => Doc::text("(admit())"),
            RValueT::InlinePulse { val, ty } => parens(Doc::concat(val.tokens.iter().map(|tok| {
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
                // TODO: this should be == in ghost contexts
                binop(emit_rvalue(env, lhs), Doc::text("="), emit_rvalue(env, rhs))
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
    env: &mut Env,
    FnDecl {
        name,
        ret_type,
        args,
        requires,
        ensures,
    }: &FnDecl,
) -> Doc {
    Doc::group(
        Doc::text("fn")
            .append(Doc::line())
            .append(name.val.to_string()),
    )
    .append(
        Doc::concat(args.iter().map(|arg @ (n, ty)| {
            let doc = Doc::line().append(parens(
                (match n {
                    Some(n) => annotated(n, Doc::text(n.val.to_string())),
                    None => Doc::text("_"),
                })
                .append(":")
                .append(Doc::line())
                .append(emit_type(env, ty)),
            ));
            env.push_arg(arg);
            doc
        }))
        .nest(2),
    )
    .group()
    .append(Doc::hardline())
    .append(Doc::group(
        Doc::text("returns")
            .append(Doc::line())
            .append("return")
            .append(Doc::line())
            .append(":")
            .group()
            .append(Doc::line())
            .append(emit_type(env, ret_type)),
    ))
    .append(Doc::concat(requires.iter().map(|r| {
        Doc::hardline().append(
            Doc::text("requires")
                .append(Doc::line())
                .append(emit_rvalue(env, r))
                .nest(2)
                .group(),
        )
    })))
    .append(Doc::concat(ensures.iter().map(|r| {
        Doc::hardline().append(
            Doc::text("ensures")
                .append(Doc::line())
                .append(emit_rvalue(env, r))
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
                let env = &mut env.clone();
                emit_fn_decl(env, decl)
                    .nest(2)
                    .append(Doc::hardline())
                    .append(
                        block(
                            Doc::concat(decl.args.iter().filter_map(|(n, _)| {
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
                            }))
                            .append(emit_stmts(env, body)),
                        )
                        .group(),
                    )
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
