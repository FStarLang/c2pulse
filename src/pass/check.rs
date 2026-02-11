use std::rc::Rc;

use crate::{
    diag::{Diagnostic, DiagnosticLevel, Diagnostics},
    env::{Env, LocalDeclKind},
    ir::*,
    mayberc::MaybeRc,
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

    fn infer_rvalue(&mut self, env: &Env, rval: &RValue) -> Option<MaybeRc<Type>> {
        env.infer_rvalue(rval).or_else(|| {
            self.report(format!("cannot infer type of {}", rval), &rval.loc);
            None
        })
    }

    fn infer_lvalue(&mut self, env: &Env, lval: &LValue) -> Option<MaybeRc<Type>> {
        env.infer_lvalue(lval).or_else(|| {
            self.report(format!("cannot infer type of {}", lval), &lval.loc);
            None
        })
    }

    fn check_type_eq(&mut self, env: &Env, actual: MaybeRc<Type>, expected: MaybeRc<Type>) {
        if self.check_types && !env.vtype_eq(actual.clone(), expected.clone()) {
            self.report(
                format!("expected type {} got {}", expected, actual),
                &actual.loc,
            )
        }
    }

    fn check_has_type(&mut self, env: &Env, rval: &RValue, expected: MaybeRc<Type>) {
        if self.check_types
            && let Some(ty) = self.infer_rvalue(env, rval)
            && !env.vtype_eq(ty.clone().into(), expected.clone())
        {
            self.report(
                format!(
                    "expected type {}, but got {} with type {}",
                    expected, rval, ty
                ),
                &rval.loc,
            )
        }
    }

    fn check_slprop(&mut self, env: &Env, p: &RValue) {
        self.check_rvalue(env, p);
        self.check_has_type(env, p, TypeT::SLProp.with_loc_core(p.loc.clone()).into());
    }

    fn check_bool(&mut self, env: &Env, p: &RValue) {
        self.check_rvalue(env, p);
        self.check_has_type(env, p, TypeT::Bool.with_loc_core(p.loc.clone()).into());
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
            TypeT::TypeRef(TypeRefKind::Typedef(n)) => {
                if let None = env.lookup_type(n) {
                    self.report(format!("unknown type {}", n), &ty.loc)
                }
            }
            TypeT::TypeRef(TypeRefKind::Struct(n)) => {
                if let None = env.lookup_struct(n) {
                    self.report(format!("unknown struct {}", n), &ty.loc)
                }
            }
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

    fn is_valid_int_type(&self, env: &Env, ty: MaybeRc<Type>) -> bool {
        match &env.vtype_whnf(ty).val {
            TypeT::Void => false, // ?
            TypeT::Bool => true,
            TypeT::Int { .. } => true,
            TypeT::SizeT => true,
            TypeT::Pointer(_, _) => true, // == 0 ?
            TypeT::SpecInt => true,
            TypeT::SLProp => true, // true/false
            TypeT::TypeRef(_) => false,
            TypeT::Requires(..) | TypeT::Ensures(..) | TypeT::Consumes(..) | TypeT::Plain(..) => {
                false
            }
            TypeT::Error => true,
        }
    }

    fn check_rvalue(&mut self, env: &Env, rval: &RValue) {
        match &rval.val {
            RValueT::BoolLit(_) => {}
            RValueT::IntLit(_n, ty) => {
                self.check_type(env, ty);
                if self.check_types && !self.is_valid_int_type(env, ty.clone().into()) {
                    self.report(format!("invalid integer type: {}", ty), &rval.loc);
                }
            }
            RValueT::LValue(lval) => match &lval.val {
                LValueT::Var(n) => self.check_var(env, n, false),
                _ => self.check_lvalue(env, lval),
            },
            RValueT::Ref(lval) => self.check_lvalue(env, lval),
            RValueT::UnOp(un_op, arg) => {
                self.check_rvalue(env, arg);
                if self.check_types
                    && let Some(arg_ty) = self.infer_rvalue(env, arg)
                {
                    match un_op {
                        UnOp::Not => match &env.vtype_whnf(arg_ty.clone().into()).val {
                            TypeT::Bool | TypeT::Error => {}
                            _ => self.report(
                                format!("! needs to be applied to bool, not {}", arg_ty),
                                &rval.loc,
                            ),
                        },
                        UnOp::Neg => {}
                    }
                }
            }
            RValueT::BinOp(bin_op, lhs, rhs) => {
                self.check_rvalue(env, lhs);
                self.check_rvalue(env, rhs);
                if self.check_types
                    && let (Some(lhs_ty), Some(rhs_ty)) =
                        (self.infer_rvalue(env, lhs), self.infer_rvalue(env, rhs))
                {
                    let check_eq = {
                        let lhs_ty = lhs_ty.clone();
                        let rhs_ty = rhs_ty.clone();
                        move |this: &mut Checker| {
                            if !env.vtype_eq(lhs_ty.clone().into(), rhs_ty.clone().into()) {
                                this.report(
                                    format!(
                                        "binop applied to mismatching types {} and {}",
                                        lhs_ty, rhs_ty
                                    ),
                                    &rval.loc,
                                )
                            }
                        }
                    };
                    match bin_op {
                        BinOp::Eq => check_eq(self),
                        BinOp::LogAnd => {
                            check_eq(self);
                            match &env.vtype_whnf(lhs_ty.clone().into()).val {
                                TypeT::Bool | TypeT::SLProp | TypeT::Error => {}
                                _ => self.report(
                                    format!(
                                        "&& needs to be applied to bool/slprop, not {}",
                                        lhs_ty
                                    ),
                                    &rval.loc,
                                ),
                            }
                        }
                        BinOp::LEq
                        | BinOp::Lt
                        | BinOp::Mul
                        | BinOp::Div
                        | BinOp::Mod
                        | BinOp::Add
                        | BinOp::Sub => check_eq(self),
                    }
                }
            }
            RValueT::FnCall(f, args) => {
                for arg in args {
                    self.check_rvalue(env, arg)
                }
                let Some(fn_decl) = env.lookup_fn(f) else {
                    self.report(format!("unknown function {}", f.val), &rval.loc);
                    return;
                };
                if fn_decl.args.len() != args.len() {
                    self.report(
                        format!(
                            "incorrect number of arguments, expected {}, got {}",
                            fn_decl.args.len(),
                            args.len()
                        ),
                        &rval.loc,
                    );
                }
                for (decl_arg, arg) in fn_decl.args.iter().zip(args.iter()) {
                    self.check_has_type(env, arg, decl_arg.1.clone().into());
                }
            }
            RValueT::Cast(rval, ty) => {
                self.check_rvalue(env, rval);
                self.check_type(env, ty);
            }
            RValueT::InlinePulse(_inline_code, ty) => self.check_type(env, ty),
            RValueT::Live(lval) => self.check_lvalue(env, lval),
            RValueT::Old(rval) => self.check_rvalue(env, rval),
            RValueT::StructInit(name, fields) => {
                let Some(s) = env.lookup_struct(name) else {
                    self.report(format!("unknown struct {}", name), &rval.loc);
                    return;
                };
                if self.check_types {
                    if fields.len() != s.fields.len() {
                        self.report(
                            format!(
                                "struct {} has {} fields, but {} were given",
                                name,
                                s.fields.len(),
                                fields.len()
                            ),
                            &rval.loc,
                        );
                    }
                    for (fld_name, fld_val) in fields {
                        self.check_rvalue(env, fld_val);
                        let Some(fld_ty) = s.get_field(fld_name) else {
                            self.report(
                                format!("no field {} in struct {}", fld_name, name),
                                &rval.loc,
                            );
                            continue;
                        };
                        self.check_has_type(env, fld_val, fld_ty.clone().into());
                    }
                }
            }
            RValueT::Error(ty) => self.check_type(env, ty),
        }
    }

    fn is_pointer_type(&mut self, env: &Env, ty: MaybeRc<Type>) -> bool {
        match &env.vtype_whnf(ty).val {
            TypeT::Pointer(_ty, _kind) => true,
            TypeT::Error => true,
            _ => false,
        }
    }

    fn check_lvalue(&mut self, env: &Env, lval: &LValue) {
        match &lval.val {
            LValueT::Var(n) => self.check_var(env, n, true),
            LValueT::Deref(rval) => {
                self.check_rvalue(env, rval);
                if self.check_types
                    && let Some(rval_ty) = self.infer_rvalue(env, rval)
                    && !self.is_pointer_type(env, rval_ty.clone())
                {
                    self.report(format!("not a pointer type: {}", rval_ty), &rval.loc)
                }
            }
            LValueT::Member(x, a) => {
                self.check_lvalue(env, x);
                if self.check_types
                    && let Some(t) = self.infer_lvalue(env, x)
                {
                    let t = env.vtype_whnf(t);
                    let TypeT::TypeRef(TypeRefKind::Struct(n)) = &t.val else {
                        return self.report(format!("not a structure type: {}", t), &lval.loc);
                    };
                    let Some(s) = env.lookup_struct(n) else {
                        return self.report(format!("unknown structure {}", n), &lval.loc);
                    };
                    let Some(_f) = s.get_field(a) else {
                        return self
                            .report(format!("no field {} in structure {}", a, n), &lval.loc);
                    };
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
                    self.check_type_eq(env, v_ty.into(), x_ty.into());
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
            StmtT::Assert(v) => self.check_slprop(env, v),
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
        // TODO: check double definition
        match &decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                self.check_fn_decl(env, decl);
                let env = &mut env.clone();
                env.push_fn_decl_args_for_body(decl);
                self.check_stmts(env, body);
            }
            DeclT::FnDecl(fn_decl) => self.check_fn_decl(env, fn_decl),
            DeclT::Typedef(TypeDefn { name: _, body }) => self.check_type(env, body),
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
