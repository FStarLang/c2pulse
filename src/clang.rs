use crate::ir::*;
use std::{collections::HashSet, rc::Rc};

#[derive(Clone)]
struct MyRc<T>(Rc<T>);
impl<T> MyRc<T> {
    fn new(t: T) -> Self {
        MyRc(Rc::new(t))
    }
    fn get(&self) -> Rc<T> {
        self.0.clone()
    }
}

type RcLocation = MyRc<Location>;
type RcStmt = MyRc<Stmt>;
type RcLValue = MyRc<LValue>;
type RcRValue = MyRc<RValue>;
type RcType = MyRc<Type>;
type RcDecl = MyRc<Decl>;
type AstRcIdent = Ast<Rc<Ident>>;

#[cxx::bridge]
mod ffi {
    struct Arg {
        name: Box<AstRcIdent>,
        ty: Box<RcType>,
    }

    extern "Rust" {
        type Ctx;
        type RcLocation;
        type RcStmt;
        type RcLValue;
        type RcRValue;
        type RcType;
        type RcDecl;
        type AstRcIdent;

        fn mk_location(
            self: &mut Ctx,
            file_name: &str,
            start_line: u32,
            start_char: u32,
            end_line: u32,
            end_char: u32,
        ) -> Box<RcLocation>;

        fn mk_assign(
            self: &mut Ctx,
            loc: &RcLocation,
            lhs: &RcLValue,
            rhs: &RcRValue,
        ) -> Box<RcStmt>;

        fn mk_ident(self: &mut Ctx, name: &str, loc: &RcLocation) -> Box<AstRcIdent>;

        fn add_fn_decl(self: &mut Ctx, name: &AstRcIdent, ret_type: &RcType, args: Vec<Arg>);

        fn clone_ident(t: &AstRcIdent) -> Box<AstRcIdent>;
        fn clone_type(t: &RcType) -> Box<RcType>;
    }

    unsafe extern "C++" {
        include!("c2pulse/src/clang_bridge.h");
        fn parse_file(ctx: &mut Ctx, file_name: &str);
    }
}

struct Ctx {
    file_names: HashSet<Rc<str>>,
    translation_unit: TranslationUnit,
}

impl Ctx {
    fn new() -> Ctx {
        Ctx {
            file_names: HashSet::new(),
            translation_unit: TranslationUnit { decls: vec![] },
        }
    }

    fn mk_file_name(&mut self, file_name: &str) -> Rc<str> {
        match self.file_names.get(file_name) {
            Some(file_name) => file_name.clone(),
            None => {
                let file_name: Rc<str> = Rc::from(file_name);
                self.file_names.insert(file_name.clone());
                file_name
            }
        }
    }

    fn mk_location(
        &mut self,
        file_name: &str,
        start_line: u32,
        start_char: u32,
        end_line: u32,
        end_char: u32,
    ) -> Box<RcLocation> {
        let file_name = self.mk_file_name(file_name);
        Box::new(MyRc::new(Location {
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

    fn mk_ident(&mut self, name: &str, loc: &RcLocation) -> Box<AstRcIdent> {
        Box::new(Ast {
            val: Rc::from(name),
            loc: loc.get(),
        })
    }

    fn add_fn_decl(&mut self, name: &AstRcIdent, ret_type: &RcType, args: Vec<ffi::Arg>) {
        self.translation_unit.decls.push(Decl::FnDecl(FnDecl {
            name: name.clone(),
            ret_type: ret_type.get(),
            args: args.into_iter().map(|arg| (*arg.name, arg.ty.0)).collect(),
        }))
    }

    fn mk_assign(&mut self, loc: &RcLocation, lhs: &RcLValue, rhs: &RcRValue) -> Box<RcStmt> {
        Box::new(MyRc::new(Ast {
            val: StmtT::Assign(lhs.get(), rhs.get()),
            loc: loc.get(),
        }))
    }
}

fn clone_ident(t: &AstRcIdent) -> Box<AstRcIdent> {
    Box::new(t.clone())
}
fn clone_type(t: &RcType) -> Box<RcType> {
    Box::new(t.clone())
}

pub fn parse_file(file_name: &str) -> TranslationUnit {
    let mut ctx = Ctx::new();
    ffi::parse_file(&mut ctx, file_name);
    ctx.translation_unit
}
