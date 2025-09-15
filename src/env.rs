use crate::ir::*;
use std::{collections::HashMap, rc::Rc};

#[derive(Clone, Debug, Default)]
struct Globals {
    fns: HashMap<Rc<str>, FnDecl>,
    structs: HashMap<Rc<str>, StructDefn>,
}

#[derive(Clone, Debug, Default)]
pub struct Env {
    globals: Rc<Globals>,
    locals: HashMap<Rc<str>, Rc<Type>>,
}

impl Env {
    pub fn new() -> Env {
        Env::default()
    }

    pub fn push_fn_decl(&mut self, decl: FnDecl) {
        Rc::make_mut(&mut self.globals)
            .fns
            .insert(decl.name.val.clone(), decl);
    }

    pub fn push_struct(&mut self, decl: StructDefn) {
        Rc::make_mut(&mut self.globals)
            .structs
            .insert(decl.name.val.clone(), decl);
    }

    pub fn push_decl(&mut self, decl: &Decl) {
        match &decl.val {
            DeclT::FnDefn(fn_defn) => self.push_fn_decl(fn_defn.decl.clone()),
            DeclT::FnDecl(fn_decl) => self.push_fn_decl(fn_decl.clone()),
            DeclT::StructDefn(struct_defn) => self.push_struct(struct_defn.clone()),
            DeclT::IncludeDecl(_) => {}
        }
    }

    pub fn push_var_decl(&mut self, ident: &Ident, ty: Rc<Type>) {
        self.locals.insert(ident.val.clone(), ty);
    }

    pub fn lookup_var(&self, ident: &Ident) -> Option<&Rc<Type>> {
        self.locals.get(&ident.val)
    }

    pub fn push_stmt(&mut self, stmt: &Stmt) {
        match &stmt.val {
            StmtT::Decl(ident, ty) => self.push_var_decl(ident, ty.clone()),
            _ => {}
        }
    }

    pub fn infer_rvalue(&self, rvalue: &RValue) -> Option<Rc<Type>> {
        match &rvalue.val {
            RValueT::IntLit { val: _, ty } => Some(ty.clone()),
            RValueT::LValue(v) => self.infer_lvalue(v),
            RValueT::Ref(v) => Some(Rc::new(rvalue.reuse_loc(TypeT::Pointer {
                to: self.infer_lvalue(v)?,
                // TODO: put kind on Ref?
                kind: PointerKind::Unknown,
            }))),
            RValueT::Cast { val: _, ty } => Some(ty.clone()),
            RValueT::Error(ty) => Some(ty.clone()),
            RValueT::InlinePulse { val, ty } => Some(ty.clone()),
        }
    }

    pub fn infer_lvalue(&self, lvalue: &LValue) -> Option<Rc<Type>> {
        match &lvalue.val {
            LValueT::Var(ident) => Some(self.lookup_var(ident)?.clone()),
            LValueT::Deref(x) => {
                let x_ty = self.infer_rvalue(x)?;
                match &x_ty.val {
                    TypeT::Pointer { to, kind: _ } => Some(to.clone()),
                    _ => None,
                }
            }
            LValueT::Error(ty) => Some(ty.clone()),
        }
    }
}
