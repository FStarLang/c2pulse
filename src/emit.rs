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
            TypeT::Int {
                signed: false,
                width: 1,
            } => Doc::text("bool"),
            TypeT::Int { signed, width } => {
                // TODO
                Doc::text("Int.t")
            }
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

fn emit_rvalue(env: &Env, v: &RValue) -> Doc {
    annotated(v, {
        match &v.val {
            RValueT::IntLit { val, ty } => {
                match &ty.val {
                    TypeT::Int {
                        signed: false,
                        width: 1,
                    } => Doc::text(if **val == BigInt::ZERO {
                        "false"
                    } else {
                        "true"
                    }),
                    _ => {
                        // TODO
                        Doc::text(val.to_string())
                    }
                }
            }
            RValueT::LValue(v) => parens(Doc::text("!").append(emit_lvalue(env, v))),
            RValueT::Ref(v) => emit_lvalue(env, v),
            RValueT::Cast { val, ty } => Doc::text("(*TODO cast*)").append(emit_rvalue(env, val)),
            RValueT::Error(_ty) => Doc::text("(admit())"),
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
    }: &FnDecl,
) -> Doc {
    Doc::group(
        Doc::text("fn")
            .append(Doc::line())
            .append(name.val.to_string()),
    )
    .append(
        Doc::concat(args.iter().map(|(n, ty)| {
            Doc::line().append(parens(
                (match n {
                    Some(n) => annotated(n, Doc::text(n.val.to_string())),
                    None => Doc::text("_"),
                })
                .append(":")
                .append(Doc::line())
                .append(emit_type(env, ty)),
            ))
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
            DeclT::FnDefn(FnDefn { decl, body }) => emit_fn_decl(env, decl)
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
                ),
            DeclT::FnDecl(fn_decl) => emit_fn_decl(env, fn_decl),
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
