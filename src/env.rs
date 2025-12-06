use crate::ir::*;
use std::{collections::HashMap, rc::Rc};

#[derive(Clone, Debug, Default)]
struct Globals {
    fns: HashMap<Rc<str>, FnDecl>,
    structs: HashMap<Rc<str>, StructDefn>,
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub enum LocalDeclKind {
    LValue,
    RValue,
}

#[derive(Clone, Debug)]
pub struct LocalDecl {
    pub ty: Rc<Type>,
    pub kind: LocalDeclKind,
}

#[derive(Clone, Debug, Default)]
pub struct Env {
    globals: Rc<Globals>,
    locals: HashMap<Rc<str>, LocalDecl>,
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

    pub fn push_var_decl(&mut self, ident: &Ident, ty: Rc<Type>, is_rvalue: LocalDeclKind) {
        self.locals.insert(
            ident.val.clone(),
            LocalDecl {
                ty,
                kind: is_rvalue,
            },
        );
    }

    pub fn push_arg(&mut self, arg: &FnArg, is_rvalue: LocalDeclKind) {
        match arg {
            (Some(n), ty) => self.push_var_decl(n, ty.clone(), is_rvalue),
            (None, _) => {}
        }
    }

    pub fn push_fn_decl_args_for_body(&mut self, decl: &FnDecl) {
        for arg in &decl.args {
            self.push_arg(arg, LocalDeclKind::LValue);
        }
    }

    pub fn lookup_fn(&self, ident: &Ident) -> Option<&FnDecl> {
        self.globals.fns.get(&ident.val)
    }

    pub fn lookup_var(&self, ident: &Ident) -> Option<&LocalDecl> {
        self.locals.get(&ident.val)
    }

    pub fn lookup_var_type(&self, ident: &Ident) -> Option<&Rc<Type>> {
        self.lookup_var(ident).map(|decl| &decl.ty)
    }

    pub fn push_stmt(&mut self, stmt: &Stmt) {
        match &stmt.val {
            StmtT::Decl(ident, ty) => self.push_var_decl(ident, ty.clone(), LocalDeclKind::LValue),
            _ => {}
        }
    }

    pub fn push_this(&mut self, ty: Rc<Type>) {
        self.locals.insert(
            Rc::<str>::from("this"),
            LocalDecl {
                ty,
                kind: LocalDeclKind::RValue,
            },
        );
    }

    pub fn push_return(&mut self, ty: Rc<Type>) {
        self.locals.insert(
            Rc::<str>::from("return"),
            LocalDecl {
                ty,
                kind: LocalDeclKind::RValue,
            },
        );
    }

    pub fn infer_rvalue(&self, rvalue: &RValue) -> Option<Rc<Type>> {
        match &rvalue.val {
            RValueT::IntLit(_, ty) => Some(ty.clone()),
            RValueT::LValue(v) => self.infer_lvalue(v),
            RValueT::Ref(v) => Some(Rc::new(
                // TODO: put kind on Ref?
                rvalue.reuse_loc(TypeT::Pointer(self.infer_lvalue(v)?, PointerKind::Unknown)),
            )),
            RValueT::FnCall(f, _args) => match self.globals.fns.get(&f.val) {
                Some(f_decl) => Some(f_decl.ret_type.clone()),
                None => None,
            },
            RValueT::Cast(_, ty) => Some(ty.clone()),
            RValueT::Error(ty) => Some(ty.clone()),
            RValueT::InlinePulse(_, ty) => Some(ty.clone()),
            RValueT::BinOp(BinOp::Eq | BinOp::LEq, _, _) => {
                Some(TypeT::Bool.with_loc(rvalue.loc.clone()))
            }
            RValueT::BinOp(
                BinOp::LogAnd | BinOp::Mul | BinOp::Div | BinOp::Mod | BinOp::Add | BinOp::Sub,
                lhs,
                _,
            ) => self.infer_rvalue(lhs),
            RValueT::BoolLit(_) => Some(TypeT::Bool.with_loc(rvalue.loc.clone())),
            RValueT::Live(_) => Some(TypeT::SLProp.with_loc(rvalue.loc.clone())),
            RValueT::Old(v) => self.infer_rvalue(v),
        }
    }

    pub fn infer_lvalue(&self, lvalue: &LValue) -> Option<Rc<Type>> {
        match &lvalue.val {
            LValueT::Var(ident) => Some(self.lookup_var_type(ident)?.clone()),
            LValueT::Deref(x) => {
                let x_ty = self.infer_rvalue(x)?;
                match &x_ty.val {
                    TypeT::Pointer(to, _) => Some(to.clone()),
                    _ => None,
                }
            }
            LValueT::Error(ty) => Some(ty.clone()),
        }
    }

    pub fn implicitly_converts_to(&self, a: &Type, b: &Type) -> bool {
        match (&a.val, &b.val) {
            (TypeT::Error, _) => true,
            (TypeT::Void, TypeT::Void) => true,
            (TypeT::Bool, TypeT::Bool) => true,
            (TypeT::Bool, TypeT::Int { .. }) => true,
            (TypeT::Bool, TypeT::SizeT) => true,
            (TypeT::Bool, TypeT::SLProp) => true,
            (
                TypeT::Int {
                    signed: s1,
                    width: w1,
                },
                TypeT::Int {
                    signed: _,
                    width: w2,
                },
            ) => w2 > w1 || (w1 == w2 && !s1),
            (TypeT::Int { .. }, TypeT::SizeT) => true,
            (TypeT::SizeT, TypeT::SizeT) => true,
            (TypeT::Pointer(..), TypeT::Pointer(..)) => true,
            (TypeT::SpecInt, TypeT::SpecInt) => true,
            _ => false,
        }
    }

    pub fn vtype_whnf(&self, a: &Rc<Type>) -> Rc<Type> {
        match &a.val {
            TypeT::Void
            | TypeT::Bool
            | TypeT::Int { .. }
            | TypeT::SizeT
            | TypeT::Pointer(_, _)
            | TypeT::SpecInt
            | TypeT::SLProp
            | TypeT::Error => a.clone(),

            TypeT::Requires(ty, _)
            | TypeT::Ensures(ty, _)
            | TypeT::Consumes(ty)
            | TypeT::Plain(ty) => self.vtype_whnf(ty),
        }
    }

    pub fn vtype_eq(&self, a: &Rc<Type>, b: &Rc<Type>) -> bool {
        match (&self.vtype_whnf(a).val, &self.vtype_whnf(b).val) {
            (TypeT::Void, TypeT::Void) => true,
            (TypeT::Bool, TypeT::Bool) => true,
            (
                TypeT::Int {
                    signed: s1,
                    width: w1,
                },
                TypeT::Int {
                    signed: s2,
                    width: w2,
                },
            ) => s1 == s2 && w1 == w2,
            (TypeT::SizeT, TypeT::SizeT) => true,
            (TypeT::Pointer(t1, k1), TypeT::Pointer(t2, k2)) => k1 == k2 && self.vtype_eq(t1, t2),
            (TypeT::SpecInt, TypeT::SpecInt) => true,
            (TypeT::SLProp, TypeT::SLProp) => true,
            (TypeT::Error, _) | (_, TypeT::Error) => true,
            _ => false,
        }
    }

    pub fn is_bool(&self, a: &Type) -> bool {
        match &a.val {
            TypeT::Bool => true,
            _ => false,
        }
    }
    pub fn is_slprop(&self, a: &Type) -> bool {
        match &a.val {
            TypeT::SLProp => true,
            _ => false,
        }
    }
}
