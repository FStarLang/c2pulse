use crate::{ir::*, mayberc::MaybeRc};
use std::{collections::HashMap, rc::Rc};

#[derive(Clone, Debug, Default)]
struct Globals {
    fns: HashMap<Rc<str>, FnDecl>,
    typedefs: HashMap<Rc<str>, TypeDefn>,
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

macro_rules! either_side {
    ($t:pat) => {
        ($t, _) | (_, $t)
    };
}
macro_rules! both_sides {
    ($t:pat) => {
        ($t, $t)
    };
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

    pub fn push_typedef(&mut self, defn: TypeDefn) {
        Rc::make_mut(&mut self.globals)
            .typedefs
            .insert(defn.name.val.clone(), defn);
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
            DeclT::Typedef(defn) => self.push_typedef(defn.clone()),
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

    pub fn lookup_type(&self, ident: &Ident) -> Option<&TypeDefn> {
        self.globals.typedefs.get(&ident.val)
    }

    pub fn lookup_struct(&self, ident: &Ident) -> Option<&StructDefn> {
        self.globals.structs.get(&ident.val)
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

    pub fn infer_rvalue(&self, rvalue: &RValue) -> Option<MaybeRc<Type>> {
        match &rvalue.val {
            RValueT::IntLit(_, ty) => Some(ty.clone().into()),
            RValueT::LValue(v) => self.infer_lvalue(v),
            RValueT::Ref(v) => Some(
                // TODO: put kind on Ref?
                rvalue
                    .reuse_loc(TypeT::Pointer(
                        self.infer_lvalue(v)?.to_rc(),
                        PointerKind::Unknown,
                    ))
                    .into(),
            ),
            RValueT::FnCall(f, _args) => match self.globals.fns.get(&f.val) {
                Some(f_decl) => Some(f_decl.ret_type.clone().into()),
                None => None,
            },
            RValueT::Cast(_, ty) => Some(ty.clone().into()),
            RValueT::Error(ty) => Some(ty.clone().into()),
            RValueT::InlinePulse(_, ty) => Some(ty.clone().into()),
            RValueT::BinOp(BinOp::Eq | BinOp::LEq, _, _) => {
                Some(TypeT::Bool.with_loc_core(rvalue.loc.clone()).into())
            }
            RValueT::BinOp(
                BinOp::LogAnd | BinOp::Mul | BinOp::Div | BinOp::Mod | BinOp::Add | BinOp::Sub,
                lhs,
                _,
            ) => self.infer_rvalue(lhs),
            RValueT::BoolLit(_) => Some(TypeT::Bool.with_loc_core(rvalue.loc.clone()).into()),
            RValueT::Live(_) => Some(TypeT::SLProp.with_loc_core(rvalue.loc.clone()).into()),
            RValueT::Old(v) => self.infer_rvalue(v),
        }
    }

    pub fn infer_lvalue(&self, lvalue: &LValue) -> Option<MaybeRc<Type>> {
        match &lvalue.val {
            LValueT::Var(ident) => Some(self.lookup_var_type(ident)?.clone().into()),
            LValueT::Deref(x) => {
                let x_ty = self.infer_rvalue(x)?;
                match &x_ty.val {
                    TypeT::Pointer(to, _) => Some(to.clone().into()),
                    _ => None,
                }
            }
            LValueT::Error(ty) => Some(ty.clone().into()),
        }
    }

    pub fn meet_type(&self, a: MaybeRc<Type>, b: MaybeRc<Type>) -> Option<MaybeRc<Type>> {
        let a0 = a.clone();
        let b0 = b.clone();
        let a = self.vtype_whnf(a);
        let b = self.vtype_whnf(b);
        if self.vtype_eq(a.clone(), b.clone()) {
            return Some(a0);
        }
        match (&a.val, &b.val) {
            (_, TypeT::Error) => Some(b0),
            (TypeT::Error, _) => Some(a0),

            (TypeT::SpecInt, TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT) => Some(a0),
            (TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT, TypeT::SpecInt) => Some(b0),

            (TypeT::SizeT, TypeT::Bool | TypeT::Int { .. }) => Some(a0),
            (TypeT::Bool | TypeT::Int { .. }, TypeT::SizeT) => Some(b0),

            (TypeT::Int { .. }, TypeT::Bool) => Some(a0),
            (TypeT::Bool, TypeT::Int { .. }) => Some(b0),

            (TypeT::SLProp, TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT | TypeT::SpecInt) => {
                Some(a0)
            }
            (TypeT::Bool | TypeT::Int { .. } | TypeT::SizeT | TypeT::SpecInt, TypeT::SLProp) => {
                Some(b0)
            }

            both_sides!(TypeT::Bool) => None,
            both_sides!(TypeT::Int { .. }) => None,
            both_sides!(TypeT::SLProp) => None,
            both_sides!(TypeT::SizeT) => None,
            both_sides!(TypeT::SpecInt) => None,

            either_side!(TypeT::Void) => None,
            either_side!(TypeT::Pointer(_, _)) => None,

            either_side!(TypeT::TypeRef(_)) => None,

            either_side!(
                TypeT::Requires(..) | TypeT::Ensures(..) | TypeT::Consumes(..) | TypeT::Plain(..)
            ) => None,
        }
    }

    pub fn vtype_whnf(&self, a: MaybeRc<Type>) -> MaybeRc<Type> {
        let mut a = a;
        while let Some(a_next) = self.vtype_whnf_step(&*a) {
            a = a_next
        }
        a
    }

    pub fn vtype_whnf_step(&self, a: &Type) -> Option<MaybeRc<Type>> {
        match &a.val {
            TypeT::Void
            | TypeT::Bool
            | TypeT::Int { .. }
            | TypeT::SizeT
            | TypeT::Pointer(_, _)
            | TypeT::SpecInt
            | TypeT::SLProp
            | TypeT::Error => None,

            TypeT::TypeRef(TypeRefKind::Typedef(n)) => {
                self.lookup_type(n).map(|defn| defn.body.clone().into())
            }
            TypeT::TypeRef(TypeRefKind::Struct(_)) => None,

            TypeT::Requires(ty, _)
            | TypeT::Ensures(ty, _)
            | TypeT::Consumes(ty)
            | TypeT::Plain(ty) => Some(ty.clone().into()),
        }
    }

    pub fn vtype_eq(&self, a: MaybeRc<Type>, b: MaybeRc<Type>) -> bool {
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
            (TypeT::Pointer(t1, k1), TypeT::Pointer(t2, k2)) => {
                k1 == k2 && self.vtype_eq(t1.clone().into(), t2.clone().into())
            }
            (TypeT::TypeRef(t1), TypeT::TypeRef(t2)) => t1 == t2,
            (TypeT::SpecInt, TypeT::SpecInt) => true,
            (TypeT::SLProp, TypeT::SLProp) => true,
            (TypeT::Error, _) | (_, TypeT::Error) => true,
            _ => false,
        }
    }

    pub fn is_bool(&self, a: MaybeRc<Type>) -> bool {
        match &self.vtype_whnf(a).val {
            TypeT::Bool => true,
            _ => false,
        }
    }
    pub fn is_slprop(&self, a: MaybeRc<Type>) -> bool {
        match &self.vtype_whnf(a).val {
            TypeT::SLProp => true,
            _ => false,
        }
    }
}
