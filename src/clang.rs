use crate::ir::*;
use std::{collections::HashSet, rc::Rc};

mod generated {
    use std::ops::Deref;
    include!(concat!(env!("OUT_DIR"), "/generated.rs"));
}

pub struct Ctx {
    input_file_name: String,
    file_names: HashSet<Rc<str>>,
    translation_unit: TranslationUnit,
}

impl Ctx {
    fn new(input_file_name: String) -> Ctx {
        Ctx {
            input_file_name,
            file_names: HashSet::new(),
            translation_unit: TranslationUnit { decls: vec![] },
        }
    }

    fn get_input_file_name(&self) -> &str {
        &self.input_file_name
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
    ) -> Rc<Location> {
        let file_name = self.mk_file_name(file_name);
        Rc::new(Location {
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
        })
    }

    fn mk_ident(&mut self, name: &str, loc: Rc<Location>) -> Rc<Ast<Rc<Ident>>> {
        Rc::new(Ast {
            val: Rc::from(name),
            loc: loc,
        })
    }

    fn add_fn_decl(
        &mut self,
        name: Rc<Ast<Rc<Ident>>>,
        ret_type: Rc<Type>,
        args: Vec<(Rc<Ast<Rc<Ident>>>, Rc<Type>)>,
    ) {
        self.translation_unit.decls.push(Decl::FnDecl(FnDecl {
            name: (*name).clone(),
            ret_type: ret_type,
            args: args
                .into_iter()
                .map(|(name, ty)| ((*name).clone(), ty))
                .collect(),
        }))
    }
}

fn mk_assign(loc: Rc<Location>, lhs: Rc<LValue>, rhs: Rc<RValue>) -> Rc<Stmt> {
    Rc::new(Ast {
        val: StmtT::Assign(lhs, rhs),
        loc: loc,
    })
}

pub fn parse_file(file_name: &str) -> TranslationUnit {
    let mut ctx = Ctx::new(file_name.to_string());
    generated::parse_file(&mut ctx);
    ctx.translation_unit
}
