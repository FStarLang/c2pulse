use std::rc::Rc;

use crate::{
    diag::{Diagnostic, DiagnosticLevel, Diagnostics},
    env::{Env, LocalDeclKind},
    ir::*,
};

struct Checker<'a> {
    pass: &'a str,
    check_types: bool,
    diags: &'a mut Diagnostics,
}

impl<'a> Checker<'a> {
    fn report(&mut self, msg: String, loc: &SourceInfo) {
        self.diags.report(Diagnostic {
            loc: loc.location().clone(),
            level: DiagnosticLevel::Error,
            msg: format!("(internal, after {}) {}", self.pass, msg),
        });
    }

    fn infer_rvalue(&mut self, env: &Env, rval: &RValue) -> Option<Rc<Type>> {
        env.infer_rvalue(rval).or_else(|| {
            self.report(format!("cannot infer type of\n{:?}", rval), &rval.loc);
            None
        })
    }

    fn infer_lvalue(&mut self, env: &Env, lval: &LValue) -> Option<Rc<Type>> {
        env.infer_lvalue(lval).or_else(|| {
            self.report(format!("cannot infer type of\n{:?}", lval), &lval.loc);
            None
        })
    }

    fn check_type_eq(&mut self, env: &Env, actual: &Type, expected: &Type) {
        if self.check_types {
            // TODO
        }
    }

    fn check_has_type(&mut self, env: &Env, rval: &RValue, expected: &Type) {
        if self.check_types
            && let Some(ty) = self.infer_rvalue(env, rval)
        {
            // TODO
        }
    }

    fn check_slprop(&mut self, env: &Env, p: &RValue) {
        self.check_rvalue(env, p);
        self.check_has_type(
            env,
            p,
            &Ast {
                val: TypeT::SLProp,
                loc: p.loc.clone(),
            },
        );
    }

    fn check_bool(&mut self, env: &Env, p: &RValue) {
        self.check_rvalue(env, p);
        self.check_has_type(
            env,
            p,
            &Ast {
                val: TypeT::Bool,
                loc: p.loc.clone(),
            },
        );
    }

    fn check_type(&mut self, env: &Env, ty: &Type) {
        match &ty.val {
            TypeT::Void => {}
            TypeT::Bool => {}
            TypeT::Int {
                signed: _,
                width: _,
            } => {}
            TypeT::SizeT => {}
            TypeT::Pointer(ty, _kind) => {
                self.check_type(env, ty);
            }
            TypeT::SpecInt => {}
            TypeT::SLProp => {}
            TypeT::Requires(ty, p) | TypeT::Ensures(ty, p) => {
                self.check_type(env, ty);
                let env = &mut env.clone();
                env.push_this(ty.clone());
                self.check_slprop(env, p);
            }
            TypeT::Consumes(ty) | TypeT::Plain(ty) => self.check_type(env, ty),
            TypeT::Error => {}
        }
    }

    fn is_valid_int_type(&self, ty: &Type) -> bool {
        match &ty.val {
            TypeT::Void => false, // ?
            TypeT::Bool => true,
            TypeT::Int { .. } => true,
            TypeT::SizeT => true,
            TypeT::Pointer(_, _) => true, // == 0 ?
            TypeT::SpecInt => true,
            TypeT::SLProp => true, // true/false
            TypeT::Requires(ty, _p) | TypeT::Ensures(ty, _p) => self.is_valid_int_type(ty),
            TypeT::Consumes(ty) | TypeT::Plain(ty) => self.is_valid_int_type(ty),
            TypeT::Error => true,
        }
    }

    fn check_rvalue(&mut self, env: &Env, rval: &RValue) {
        match &rval.val {
            RValueT::BoolLit(_) => {}
            RValueT::IntLit(_n, ty) => {
                self.check_type(env, ty);
                if self.check_types && !self.is_valid_int_type(ty) {
                    self.report(format!("invalid integer type: {:?}", ty), &rval.loc);
                }
            }
            RValueT::LValue(lval) => match &lval.val {
                LValueT::Var(n) => self.check_var(env, n, false),
                _ => self.check_lvalue(env, lval),
            },
            RValueT::Ref(lval) => self.check_lvalue(env, lval),
            RValueT::BinOp(bin_op, lhs, rhs) => {
                self.check_rvalue(env, lhs);
                self.check_rvalue(env, rhs);
                if self.check_types
                    && let (Some(lhs_ty), Some(rhs_ty)) =
                        (self.infer_rvalue(env, lhs), self.infer_rvalue(env, rhs))
                {
                    match bin_op {
                        BinOp::Eq | BinOp::LEq => {} // TODO
                        BinOp::LogAnd
                        | BinOp::Mul
                        | BinOp::Div
                        | BinOp::Mod
                        | BinOp::Add
                        | BinOp::Sub => {} // TODO
                    }
                }
            }
            RValueT::FnCall(f, args) => {
                for arg in args {
                    self.check_rvalue(env, arg)
                }
                // TODO: check function call
            }
            RValueT::Cast(rval, ty) => {
                self.check_rvalue(env, rval);
                self.check_type(env, ty);
            }
            RValueT::InlinePulse(_inline_code, ty) => self.check_type(env, ty),
            RValueT::Live(lval) => self.check_lvalue(env, lval),
            RValueT::Old(rval) => self.check_rvalue(env, rval),
            RValueT::Error(ty) => self.check_type(env, ty),
        }
    }

    fn is_pointer_type(&mut self, env: &Env, ty: &Type) -> bool {
        match &ty.val {
            TypeT::Void => false,
            TypeT::Bool => false,
            TypeT::Int { .. } => false,
            TypeT::SizeT => false,
            TypeT::Pointer(_ty, _kind) => true,
            TypeT::SpecInt => false,
            TypeT::SLProp => false,
            TypeT::Requires(ty, _p) | TypeT::Ensures(ty, _p) => self.is_pointer_type(env, ty),
            TypeT::Consumes(ty) | TypeT::Plain(ty) => self.is_pointer_type(env, ty),
            TypeT::Error => true,
        }
    }

    fn check_lvalue(&mut self, env: &Env, lval: &LValue) {
        match &lval.val {
            LValueT::Var(n) => self.check_var(env, n, true),
            LValueT::Deref(rval) => {
                self.check_rvalue(env, rval);
                if self.check_types
                    && let Some(rval_ty) = self.infer_rvalue(env, rval)
                    && !self.is_pointer_type(env, &rval_ty)
                {
                    self.report(format!("not a pointer type: {:?}", rval_ty), &rval.loc)
                }
            }
            LValueT::Error(ty) => self.check_type(env, ty),
        }
    }

    fn check_var(&mut self, env: &Env, n: &Ident, needs_lvalue: bool) {
        match env.lookup_var(n) {
            None => self.report(format!("unknown local variable {}", n.val), &n.loc),
            Some(ldecl) => {
                if ldecl.kind == LocalDeclKind::RValue && needs_lvalue {
                    self.report(format!("need lvalue, but {} is rvalue", n.val), &n.loc);
                }
            }
        }
    }

    fn check_stmt(&mut self, env: &Env, stmt: &Stmt) {
        match &stmt.val {
            StmtT::Call(rval) => self.check_rvalue(env, rval),
            StmtT::Decl(_, ty) => self.check_type(env, ty),
            StmtT::Assign(x, v) => {
                self.check_lvalue(env, x);
                self.check_rvalue(env, v);
                if self.check_types
                    && let (Some(x_ty), Some(v_ty)) =
                        (self.infer_lvalue(env, x), self.infer_rvalue(env, v))
                {
                    self.check_type_eq(env, &v_ty, &x_ty);
                }
            }
            StmtT::If(c, b1, b2) => {
                self.check_bool(env, c);
                self.check_stmts(env, b1);
                self.check_stmts(env, b2);
            }
            StmtT::While(cond, invs, body) => {
                self.check_bool(env, cond);
                for inv in &**invs {
                    self.check_slprop(env, inv)
                }
                self.check_stmts(env, body);
            }
            StmtT::Return(v) => {
                self.check_rvalue(env, v);
                // TODO: check that v has return type of fn
            }
            StmtT::Error => {}
        }
    }

    fn check_stmts(&mut self, env: &Env, stmts: &Vec<Rc<Stmt>>) {
        let env = &mut env.clone();
        for stmt in stmts {
            self.check_stmt(env, stmt);
            env.push_stmt(stmt);
        }
    }

    fn check_slprops(&mut self, env: &Env, slprops: &Vec<Rc<RValue>>) {
        for p in slprops {
            self.check_slprop(env, p)
        }
    }

    fn check_fn_decl(
        &mut self,
        env: &Env,
        FnDecl {
            name: _,
            ret_type,
            args,
            requires,
            ensures,
        }: &FnDecl,
    ) {
        let env = &mut env.clone();
        for arg in args {
            let (_, ty) = arg;
            self.check_type(env, ty);
            env.push_arg(arg, LocalDeclKind::RValue);
        }
        self.check_slprops(env, requires);
        self.check_type(env, ret_type);
        env.push_return(ret_type.clone());
        self.check_slprops(env, ensures);
    }

    fn check_decl(&mut self, env: &Env, decl: &Decl) {
        // TODO: check double definition of functions
        match &decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                self.check_fn_decl(env, decl);
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                self.check_stmts(env, body);
            }
            DeclT::FnDecl(fn_decl) => self.check_fn_decl(env, fn_decl),
            DeclT::StructDefn(StructDefn { name: _, fields }) => {
                for (_n, ty) in fields {
                    self.check_type(env, ty)
                }
            }
            DeclT::IncludeDecl(_) => {}
        }
    }

    fn check_translation_unit(&mut self, tu: &TranslationUnit) {
        let mut env = Env::new();
        for decl in &tu.decls {
            self.check_decl(&env, decl);
            env.push_decl(decl);
        }
    }
}

pub fn check(diags: &mut Diagnostics, tu: &mut TranslationUnit, pass: &str, check_types: bool) {
    Checker {
        pass,
        check_types,
        diags,
    }
    .check_translation_unit(tu);
}
