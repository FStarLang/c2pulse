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
    pub loop_depth: usize,
    pub return_type: Option<Rc<Type>>,
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

    pub fn in_loop(&self) -> bool {
        self.loop_depth > 0
    }

    pub fn enter_loop(&mut self) {
        self.loop_depth += 1;
    }

    pub fn set_return_type(&mut self, ty: Rc<Type>) {
        self.return_type = Some(ty);
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
        self.set_return_type(decl.ret_type.clone());
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

    pub fn push_this(&mut self, ty: Rc<Type>) -> Rc<IdentT> {
        let id: Rc<IdentT> = Rc::from("this");
        self.locals.insert(
            id.clone(),
            LocalDecl {
                ty,
                kind: LocalDeclKind::RValue,
            },
        );
        id
    }

    pub fn push_return(&mut self, ty: Rc<Type>) -> Rc<IdentT> {
        let id: Rc<IdentT> = Rc::from("return");
        self.locals.insert(
            id.clone(),
            LocalDecl {
                ty,
                kind: LocalDeclKind::RValue,
            },
        );
        id
    }

    pub fn infer_rvalue(&self, rvalue: &Expr) -> Option<MaybeRc<Type>> {
        self.infer_expr(rvalue)
    }

    pub fn infer_lvalue(&self, lvalue: &Expr) -> Option<MaybeRc<Type>> {
        self.infer_expr(lvalue)
    }

    pub fn infer_expr(&self, expr: &Expr) -> Option<MaybeRc<Type>> {
        match &expr.val {
            ExprT::Var(ident) => Some(self.lookup_var_type(ident)?.clone().into()),
            ExprT::Deref(x) => {
                let x_ty = self.vtype_whnf(self.infer_expr(x)?);
                match &x_ty.val {
                    TypeT::Pointer(to, _) => Some(to.clone().into()),
                    _ => None,
                }
            }
            ExprT::Member(x, a) => {
                let x_ty = self.vtype_whnf(self.infer_expr(x)?);
                match &x_ty.val {
                    TypeT::Pointer(_, PointerKind::Array) if &*a.val == "_length" => {
                        Some(TypeT::SpecInt.with_loc_core(expr.loc.clone()).into())
                    }
                    TypeT::TypeRef(TypeRefKind::Struct(s)) => {
                        Some(self.lookup_struct(s)?.get_field(a)?.clone().into())
                    }
                    _ => None,
                }
            }
            ExprT::Index(arr, _idx) => {
                let arr_ty = self.vtype_whnf(self.infer_expr(arr)?);
                match &arr_ty.val {
                    TypeT::Pointer(elem, PointerKind::Array) => Some(elem.clone().into()),
                    _ => None,
                }
            }
            ExprT::IntLit(_, ty) => Some(ty.clone().into()),
            ExprT::Ref(v) => Some(
                expr.reuse_loc(TypeT::Pointer(
                    self.infer_expr(v)?.to_rc(),
                    PointerKind::Ref,
                ))
                .into(),
            ),
            ExprT::FnCall(f, _args) => match self.globals.fns.get(&f.val) {
                Some(f_decl) => Some(f_decl.ret_type.clone().into()),
                None => None,
            },
            ExprT::Cast(_, ty) => Some(ty.clone().into()),
            ExprT::Error(ty) => Some(ty.clone().into()),
            ExprT::Malloc(ty) => Some(
                expr.reuse_loc(TypeT::Pointer(ty.clone(), PointerKind::Ref))
                    .into(),
            ),
            ExprT::MallocArray(ty, _) => Some(
                expr.reuse_loc(TypeT::Pointer(ty.clone(), PointerKind::Array))
                    .into(),
            ),
            ExprT::Free(_) => Some(TypeT::Void.with_loc_core(expr.loc.clone()).into()),
            ExprT::InlinePulse(_, ty) => Some(ty.clone().into()),
            ExprT::UnOp(UnOp::Not, _)
            | ExprT::BinOp(
                BinOp::Eq | BinOp::LEq | BinOp::Lt | BinOp::LogOr | BinOp::Implies,
                _,
                _,
            ) => Some(TypeT::Bool.with_loc_core(expr.loc.clone()).into()),
            ExprT::UnOp(UnOp::Neg, lhs)
            | ExprT::BinOp(
                BinOp::LogAnd | BinOp::Mul | BinOp::Div | BinOp::Mod | BinOp::Add | BinOp::Sub,
                lhs,
                _,
            ) => self.infer_expr(lhs),
            ExprT::BoolLit(_) => Some(TypeT::Bool.with_loc_core(expr.loc.clone()).into()),
            ExprT::Live(_) => Some(TypeT::SLProp.with_loc_core(expr.loc.clone()).into()),
            ExprT::Old(v) => self.infer_expr(v),
            ExprT::Forall(_, _, body) | ExprT::Exists(_, _, body) => self.infer_expr(body),
            ExprT::StructInit(name, _) => Some(
                expr.reuse_loc(TypeT::TypeRef(TypeRefKind::Struct(name.clone())))
                    .into(),
            ),
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
            // Different Int types: use C-style usual arithmetic conversion
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
                // Pick the wider type; if same width, pick unsigned
                let (signed, width) = if w1 > w2 {
                    (*s1, *w1)
                } else if w2 > w1 {
                    (*s2, *w2)
                } else {
                    // Same width: unsigned wins per C rules
                    (*s1 && *s2, *w1)
                };
                Some(TypeT::Int { signed, width }.with_loc(a.loc.clone()).into())
            }
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

    pub fn is_lvalue(&self, expr: &Expr) -> bool {
        match &expr.val {
            ExprT::Var(x) => match self.lookup_var(x) {
                Some(decl) => decl.kind == LocalDeclKind::LValue,
                None => false,
            },
            ExprT::Deref(_) => true,
            ExprT::Member(x, _) => self.is_lvalue(x),
            ExprT::Index(_, _) => true,
            ExprT::Error(_) => true,
            _ => false,
        }
    }
}
