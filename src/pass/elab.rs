use crate::diag::{Diagnostic, DiagnosticLevel, Diagnostics};
use crate::env::{Env, LocalDeclKind};
use crate::ir::*;
use crate::mayberc::MaybeRc;
use std::rc::Rc;

struct Elaborator<'a> {
    diags: &'a mut Diagnostics,
}

fn cast_to(rval: &mut Rc<RValue>, ty: Rc<Type>) {
    *rval = RValueT::Cast(rval.clone(), ty).with_loc(rval.loc.clone())
}

impl<'a> Elaborator<'a> {
    fn report(&mut self, msg: String, loc: &SourceInfo) {
        self.diags.report(Diagnostic {
            loc: loc.location().clone(),
            level: DiagnosticLevel::Error,
            msg: msg,
        });
    }

    fn infer_rvalue(&mut self, env: &Env, rval: &RValue) -> Option<MaybeRc<Type>> {
        env.infer_rvalue(rval).or_else(|| {
            self.report(format!("cannot infer type of {}", rval), &rval.loc);
            None
        })
    }

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

            TypeT::TypeRef(_) => {}

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
            .filter(|p| env.is_slprop(p.clone()))
            .is_none()
        {
            *rval = RValueT::Cast(rval.clone(), TypeT::SLProp.with_loc(rval.loc.clone()))
                .with_loc(rval.loc.clone())
        }
    }

    fn cast_to_bool(&mut self, env: &Env, rval: &mut Rc<RValue>) {
        if env
            .infer_rvalue(rval)
            .filter(|p| env.is_bool(p.clone()))
            .is_none()
        {
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
                let Some(lhs_ty) = self.infer_rvalue(env, lhs) else {
                    return;
                };
                let Some(rhs_ty) = self.infer_rvalue(env, rhs) else {
                    return;
                };
                match bin_op {
                    BinOp::Eq
                    | BinOp::LEq
                    | BinOp::Mul
                    | BinOp::Div
                    | BinOp::Mod
                    | BinOp::Add
                    | BinOp::Sub
                    | BinOp::LogAnd => {
                        if let Some(meet_type) = env.meet_type(lhs_ty.clone(), rhs_ty.clone()) {
                            if !env.vtype_eq(lhs_ty, meet_type.clone()) {
                                cast_to(lhs, meet_type.clone().to_rc())
                            }
                            if !env.vtype_eq(rhs_ty, meet_type.clone()) {
                                cast_to(rhs, meet_type.to_rc())
                            }
                        } else {
                            self.report(
                                format!(
                                    "cannot apply {} to arguments of type {} and {}",
                                    bin_op, lhs_ty, rhs_ty
                                ),
                                &rval.loc,
                            );
                        }
                    }
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
                let Some(x_ty) = env.infer_lvalue(x) else {
                    return;
                };
                let Some(v_ty) = env.infer_rvalue(v) else {
                    return;
                };
                if !env.vtype_eq(x_ty.clone(), v_ty) {
                    cast_to(v, x_ty.to_rc());
                }
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
        // TODO: check double definition
        match &mut decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                self.elab_fn_decl(env, decl);
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                self.elab_stmts(env, body);
            }
            DeclT::FnDecl(fn_decl) => self.elab_fn_decl(env, fn_decl),
            DeclT::Typedef(typedef) => self.elab_type(env, Rc::make_mut(&mut typedef.body)),
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
