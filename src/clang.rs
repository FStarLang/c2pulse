use num_bigint::BigInt;

use crate::{
    diag::{Diagnostic, DiagnosticLevel},
    ir::*,
};
use core::slice;
use std::{collections::HashSet, rc::Rc, str::FromStr};

mod generated {
    use std::ops::Deref;
    include!(concat!(env!("OUT_DIR"), "/generated.rs"));
}

unsafe fn str_from_parts<'a>(ptr: *const u8, sz: usize) -> &'a str {
    str::from_utf8(unsafe { slice::from_raw_parts(ptr, sz) }).unwrap()
}

pub struct Ctx {
    input_file_name: String,
    interned_strs: HashSet<Rc<str>>,
    translation_unit: TranslationUnit,
    diagnostics: Vec<Diagnostic>,
}

impl Ctx {
    fn new(input_file_name: String) -> Ctx {
        let input_fn: &str = &input_file_name;
        let main_file_name: Rc<str> = Rc::from(input_fn);
        Ctx {
            input_file_name,
            interned_strs: HashSet::new(),
            translation_unit: TranslationUnit {
                main_file_name: main_file_name,
                decls: vec![],
            },
            diagnostics: vec![],
        }
    }

    fn get_input_file_name(&self) -> &str {
        &self.input_file_name
    }

    fn intern_str(&mut self, s: &str) -> Rc<str> {
        match self.interned_strs.get(s) {
            Some(s) => s.clone(),
            None => {
                let s: Rc<str> = Rc::from(s);
                self.interned_strs.insert(s.clone());
                s
            }
        }
    }

    fn mk_ident(&mut self, name: &str, loc: Rc<SourceInfo>) -> Rc<Ident> {
        Rc::new(Ast {
            val: self.intern_str(name),
            loc: loc,
        })
    }

    fn add_fn_decl(&mut self, builder: DeclBuilder) {
        self.translation_unit.decls.push(Ast {
            loc: builder.loc,
            val: Decl::FnDecl(FnDecl {
                name: builder.name,
                ret_type: builder.ret_type.unwrap(),
                args: builder.args,
            }),
        })
    }

    fn add_fn_defn(&mut self, builder: DeclBuilder, body: Vec<Rc<Stmt>>) {
        self.translation_unit.decls.push(Ast {
            loc: builder.loc,
            val: Decl::FnDefn(FnDefn {
                decl: FnDecl {
                    name: builder.name,
                    ret_type: builder.ret_type.unwrap(),
                    args: builder.args,
                },
                body: body,
            }),
        })
    }

    fn add_struct(&mut self, builder: DeclBuilder) {
        self.translation_unit.decls.push(Ast {
            loc: builder.loc,
            val: Decl::StructDefn(StructDefn {
                name: builder.name,
                fields: builder.fields,
            }),
        })
    }

    fn report_diag(&mut self, loc: Rc<SourceInfo>, is_err: bool, msg: &str) {
        self.diagnostics.push(Diagnostic {
            loc: (match &*loc {
                SourceInfo::Original(loc) => loc.clone(),
                _ => panic!(),
            }),
            level: (if is_err {
                DiagnosticLevel::Error
            } else {
                DiagnosticLevel::Warning
            }),
            msg: msg.into(),
        })
    }
}

struct DeclBuilder {
    name: Ident,
    loc: Rc<SourceInfo>,
    ret_type: Option<Rc<Type>>,
    args: Vec<(Option<Ident>, Rc<Type>)>,
    fields: Vec<(Ident, Rc<Type>)>,
}

impl DeclBuilder {
    fn new(loc: Rc<SourceInfo>, name: Rc<Ident>) -> DeclBuilder {
        DeclBuilder {
            name: (*name).clone(),
            loc: loc,
            ret_type: None,
            args: vec![],
            fields: vec![],
        }
    }

    fn return_type(&mut self, ret_type: Rc<Type>) {
        self.ret_type = Some(ret_type);
    }

    fn arg(&mut self, name: Rc<Ident>, ty: Rc<Type>) {
        self.args.push((Some((*name).clone()), ty))
    }
    fn arg_anon(&mut self, ty: Rc<Type>) {
        self.args.push((None, ty))
    }

    fn field(&mut self, name: Rc<Ident>, ty: Rc<Type>) {
        self.fields.push(((*name).clone(), ty))
    }
}

struct InlineCodeBuilder(InlineCode);

impl InlineCodeBuilder {
    fn new() -> InlineCodeBuilder {
        InlineCodeBuilder(InlineCode { tokens: vec![] })
    }
    fn push_token(&mut self, before: &'static str, loc: Rc<SourceInfo>, tok: &str) {
        self.0.tokens.push(CodeToken {
            before: before,
            text: Ast {
                val: Rc::from(tok),
                loc: loc,
            },
        })
    }
}

fn mk_original_location(
    file_name: Rc<str>,
    start_line: u32,
    start_char: u32,
    end_line: u32,
    end_char: u32,
) -> Rc<SourceInfo> {
    Rc::new(SourceInfo::Original(Location {
        file_name: file_name,
        range: Range {
            start: Position {
                line: start_line,
                character: start_char,
            },
            end: Position {
                line: end_line,
                character: end_char,
            },
        },
    }))
}
fn mk_none_sourceinfo() -> Rc<SourceInfo> {
    Rc::new(SourceInfo::None)
}

fn mk_ast<T>(loc: Rc<SourceInfo>, val: T) -> Rc<Ast<T>> {
    Rc::new(Ast { val: val, loc: loc })
}

fn mk_int_type(loc: Rc<SourceInfo>, signed: bool, width: u32) -> Rc<Type> {
    mk_ast(
        loc,
        TypeT::Int {
            signed: signed,
            width: width,
        },
    )
}
fn mk_sizet(loc: Rc<SourceInfo>) -> Rc<Type> {
    mk_ast(loc, TypeT::SizeT)
}
fn mk_pointer_unknown(loc: Rc<SourceInfo>, to: Rc<Type>) -> Rc<Type> {
    mk_ast(
        loc,
        TypeT::Pointer {
            to: to,
            kind: PointerKind::Unknown,
        },
    )
}
fn mk_type_err(loc: Rc<SourceInfo>) -> Rc<Type> {
    mk_ast(loc, TypeT::Error)
}

fn mk_bigint(s: &str) -> Rc<BigInt> {
    Rc::from(BigInt::from_str(s).unwrap())
}

fn mk_int_lit(loc: Rc<SourceInfo>, val: Rc<BigInt>, ty: Rc<Type>) -> Rc<RValue> {
    mk_ast(loc, RValueT::IntLit { val: val, ty: ty })
}
fn mk_rvalue_lvalue(loc: Rc<SourceInfo>, lval: Rc<LValue>) -> Rc<RValue> {
    mk_ast(loc, RValueT::LValue(lval))
}
fn mk_rvalue_ref(loc: Rc<SourceInfo>, lval: Rc<LValue>) -> Rc<RValue> {
    mk_ast(loc, RValueT::Ref(lval))
}
fn mk_cast(loc: Rc<SourceInfo>, val: Rc<RValue>, ty: Rc<Type>) -> Rc<RValue> {
    mk_ast(loc, RValueT::Cast { val: val, ty: ty })
}
fn mk_rvalue_err(loc: Rc<SourceInfo>, ty: Rc<Type>) -> Rc<RValue> {
    mk_ast(loc, RValueT::Error(ty))
}

fn mk_lvalue_var(loc: Rc<SourceInfo>, name: Rc<Ident>) -> Rc<LValue> {
    mk_ast(loc, LValueT::Var(name))
}
fn mk_deref(loc: Rc<SourceInfo>, v: Rc<RValue>) -> Rc<LValue> {
    mk_ast(loc, LValueT::Deref(v))
}
fn mk_lvalue_err(loc: Rc<SourceInfo>, ty: Rc<Type>) -> Rc<LValue> {
    mk_ast(loc, LValueT::Error(ty))
}

fn mk_var_decl(loc: Rc<SourceInfo>, id: Rc<Ident>, ty: Rc<Type>) -> Rc<Stmt> {
    mk_ast(loc, StmtT::Decl(id, ty))
}
fn mk_assign(loc: Rc<SourceInfo>, lhs: Rc<LValue>, rhs: Rc<RValue>) -> Rc<Stmt> {
    mk_ast(loc, StmtT::Assign(lhs, rhs))
}
fn mk_return(loc: Rc<SourceInfo>, v: Rc<RValue>) -> Rc<Stmt> {
    mk_ast(loc, StmtT::Return(v))
}
fn mk_stmt_err(loc: Rc<SourceInfo>) -> Rc<Stmt> {
    mk_ast(loc, StmtT::Error)
}

pub fn parse_file(file_name: &str) -> (TranslationUnit, Vec<Diagnostic>) {
    let mut ctx = Ctx::new(file_name.to_string());
    generated::parse_file(&mut ctx);
    (ctx.translation_unit, ctx.diagnostics)
}
