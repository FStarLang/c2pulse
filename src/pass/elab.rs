use crate::diag::Diagnostics;
use crate::env::{Env, LocalDeclKind};
use crate::ir::*;
use std::rc::Rc;

struct Elaborator<'a> {
    diags: &'a mut Diagnostics,
}

fn cast_to(rval: &mut Rc<RValue>, ty: Rc<Type>) {
    *rval = RValueT::Cast(rval.clone(), ty).with_loc(rval.loc.clone())
}

impl<'a> Elaborator<'a> {
    fn elab_type(&mut self, env: &Env, ty: &mut Type) {
        match &mut ty.val {
            TypeT::Int {
                signed: _,
                width: _,
            } => {}
            TypeT::SizeT => {}
            TypeT::Pointer(to, kind) => {
                self.elab_type(env, Rc::make_mut(to));
                match kind {
                    PointerKind::Unknown => *kind = PointerKind::Ref,
                    PointerKind::Ref => {}
                    PointerKind::Array => {}
                }
            }
            TypeT::Error => {}
            TypeT::Void => {}
            TypeT::SLProp => {}
            TypeT::SpecInt => {}
            TypeT::Bool => {}

            TypeT::Requires(ty, p) | TypeT::Ensures(ty, p) => {
                self.elab_type(env, Rc::make_mut(ty));

                let env = &mut env.clone();
                env.push_this(ty.clone());
                self.elab_rvalue(env, Rc::make_mut(p));
                self.cast_to_slprop(env, p);
            }
            TypeT::Consumes(ty) | TypeT::Plain(ty) => self.elab_type(env, Rc::make_mut(ty)),
        }
    }

    fn elab_lvalue(&mut self, env: &Env, lval: &mut LValue) {
        match &mut lval.val {
            LValueT::Var(_) => {}
            LValueT::Deref(v) => self.elab_rvalue(env, Rc::make_mut(v)),
            LValueT::Error(ty) => self.elab_type(env, Rc::make_mut(ty)),
        }
    }

    fn cast_to_slprop(&mut self, env: &Env, rval: &mut Rc<RValue>) {
        if env
            .infer_rvalue(rval)
            .filter(|p| env.is_slprop(p))
            .is_none()
        {
            *rval = RValueT::Cast(rval.clone(), TypeT::SLProp.with_loc(rval.loc.clone()))
                .with_loc(rval.loc.clone())
        }
    }

    fn cast_to_bool(&mut self, env: &Env, rval: &mut Rc<RValue>) {
        if env.infer_rvalue(rval).filter(|p| env.is_bool(p)).is_none() {
            *rval = RValueT::Cast(rval.clone(), TypeT::Bool.with_loc(rval.loc.clone()))
                .with_loc(rval.loc.clone())
        }
    }

    fn elab_rvalue(&mut self, env: &Env, rval: &mut RValue) {
        match &mut rval.val {
            RValueT::IntLit(_, ty) => self.elab_type(env, Rc::make_mut(ty)),
            RValueT::LValue(v) => self.elab_lvalue(env, Rc::make_mut(v)),
            RValueT::Ref(v) => self.elab_lvalue(env, Rc::make_mut(v)),
            RValueT::FnCall(_f, args) => {
                // TODO: check type for f
                for arg in args {
                    self.elab_rvalue(env, Rc::make_mut(arg));
                }
            }
            RValueT::Cast(val, ty) => {
                let val = Rc::make_mut(val);
                self.elab_type(env, Rc::make_mut(ty));
                self.elab_rvalue(env, val);
                let _actual_ty = env.infer_rvalue(val);
                // TODO: check that actual_ty can be casted to ty
            }
            RValueT::Error(ty) => self.elab_type(env, Rc::make_mut(ty)),
            RValueT::InlinePulse(_, ty) => self.elab_type(env, Rc::make_mut(ty)),
            RValueT::BinOp(bin_op, lhs, rhs) => {
                self.elab_rvalue(env, Rc::make_mut(lhs));
                self.elab_rvalue(env, Rc::make_mut(rhs));
                match (env.infer_rvalue(lhs), env.infer_rvalue(rhs)) {
                    (Some(lhs_ty), Some(rhs_ty)) => {
                        let lhs_to_rhs = env.implicitly_converts_to(&lhs_ty, &rhs_ty);
                        let rhs_to_lhs = env.implicitly_converts_to(&rhs_ty, &lhs_ty);
                        match bin_op {
                            BinOp::Eq
                            | BinOp::LEq
                            | BinOp::Mul
                            | BinOp::Div
                            | BinOp::Mod
                            | BinOp::Add
                            | BinOp::Sub => {
                                match (lhs_to_rhs, rhs_to_lhs) {
                                    (true, true) => {}
                                    (true, false) => cast_to(lhs, rhs_ty.to_rc()),
                                    (false, true) => cast_to(rhs, lhs_ty.to_rc()),
                                    (false, false) => {
                                        // TODO: produce error
                                    }
                                }
                            }
                            BinOp::LogAnd => match (env.is_slprop(&lhs_ty), env.is_slprop(&rhs_ty))
                            {
                                (true, true) => {}
                                (true, false) => cast_to(rhs, lhs_ty.to_rc()),
                                (false, true) => cast_to(lhs, rhs_ty.to_rc()),
                                (false, false) => {
                                    if !env.is_bool(&lhs_ty) {
                                        cast_to(lhs, TypeT::Bool.with_loc(rval.loc.clone()))
                                    }
                                    if !env.is_bool(&rhs_ty) {
                                        cast_to(rhs, TypeT::Bool.with_loc(rval.loc.clone()))
                                    }
                                }
                            },
                        }
                    }
                    (_, _) => {} // TODO: produce error
                }
            }
            RValueT::BoolLit(_) => {}
            RValueT::Live(val) => self.elab_lvalue(env, Rc::make_mut(val)),
            RValueT::Old(val) => self.elab_rvalue(env, Rc::make_mut(val)),
        }
    }

    fn elab_stmt(&mut self, env: &Env, stmt: &mut Stmt) {
        match &mut stmt.val {
            StmtT::Call(rval) => self.elab_rvalue(env, Rc::make_mut(rval)),
            StmtT::Decl(_, ty) => self.elab_type(env, Rc::make_mut(ty)),
            StmtT::Assign(x, v) => {
                self.elab_lvalue(env, Rc::make_mut(x));
                self.elab_rvalue(env, Rc::make_mut(v));
                let _x_ty = env.infer_lvalue(x);
                let _y_ty = env.infer_rvalue(v);
                // TODO: check that x_ty and y_ty are equal
            }
            StmtT::If(c, b1, b2) => {
                self.elab_rvalue(env, Rc::make_mut(c));
                self.cast_to_bool(env, c);
                self.elab_stmts(env, Rc::make_mut(b1));
                self.elab_stmts(env, Rc::make_mut(b2));
            }
            StmtT::While(cond, invs, body) => {
                self.elab_rvalue(env, Rc::make_mut(cond));
                self.cast_to_bool(env, cond);
                self.elab_slprops(env, Rc::make_mut(invs));
                self.elab_stmts(env, Rc::make_mut(body));
            }
            StmtT::Return(x) => {
                self.elab_rvalue(env, Rc::make_mut(x));
            }
            StmtT::Error => {}
        }
    }

    fn elab_stmts(&mut self, env: &Env, stmts: &mut Vec<Rc<Stmt>>) {
        let mut env = env.clone();
        for stmt in stmts {
            self.elab_stmt(&env, Rc::make_mut(stmt));
            env.push_stmt(stmt);
        }
    }

    fn elab_slprops(&mut self, env: &Env, slprops: &mut Vec<Rc<RValue>>) {
        for p in slprops {
            self.elab_rvalue(env, Rc::make_mut(p));
            self.cast_to_slprop(env, p);
        }
    }

    fn elab_fn_decl(
        &mut self,
        env: &Env,
        FnDecl {
            name: _,
            ret_type,
            args,
            requires,
            ensures,
        }: &mut FnDecl,
    ) {
        let env = &mut env.clone();
        for arg in args {
            let (_, ty) = arg;
            self.elab_type(env, Rc::make_mut(ty));
            env.push_arg(arg, LocalDeclKind::RValue);
        }
        self.elab_slprops(env, requires);
        self.elab_type(env, Rc::make_mut(ret_type));
        env.push_return(ret_type.clone());
        self.elab_slprops(env, ensures);
    }

    fn elab_decl(&mut self, env: &Env, decl: &mut Decl) {
        // TODO: check double definition of functions
        match &mut decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                self.elab_fn_decl(env, decl);
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                self.elab_stmts(env, body);
            }
            DeclT::FnDecl(fn_decl) => self.elab_fn_decl(env, fn_decl),
            DeclT::StructDefn(StructDefn { name: _, fields }) => {
                for (_n, ty) in fields {
                    self.elab_type(env, Rc::make_mut(ty))
                }
            }
            DeclT::IncludeDecl(_) => {}
        }
    }
}

pub fn elab(diags: &mut Diagnostics, tu: &mut TranslationUnit) {
    let mut env = Env::new();
    let mut elab = Elaborator { diags };
    for decl in &mut tu.decls {
        elab.elab_decl(&env, decl);
        env.push_decl(decl);
    }
}
