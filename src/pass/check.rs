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

    fn infer_expr(&mut self, env: &Env, rval: &Expr) -> Option<MaybeRc<Type>> {
        match env.infer_expr(rval) {
            Ok(ty) => Some(ty),
            Err(error) => {
                self.report(
                    format!("cannot infer type of {}: {}\n{}", rval, error, env),
                    &rval.loc,
                );
                None
            }
        }
    }

    fn check_type_eq(&mut self, env: &Env, actual: MaybeRc<Type>, expected: MaybeRc<Type>) {
        if self.check_types && !env.vtype_eq(actual.clone(), expected.clone()) {
            self.report(
                format!("expected type {} got {}", expected, actual),
                &actual.loc,
            )
        }
    }

    fn check_has_type(&mut self, env: &Env, rval: &Expr, expected: MaybeRc<Type>) {
        if self.check_types
            && let Some(ty) = self.infer_expr(env, rval)
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

    fn check_slprop(&mut self, env: &Env, p: &Expr) {
        self.check_rvalue(env, p);
        self.check_has_type(env, p, TypeT::SLProp.with_loc_core(p.loc.clone()).into());
    }

    fn check_bool(&mut self, env: &Env, p: &Expr) {
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

    fn check_inline_pulse_code(&mut self, env: &Env, code: &InlinePulseCode) {
        let env = &mut env.clone();
        for tok in &code.tokens {
            match tok {
                InlinePulseToken::RValueAntiquot { expr, .. }
                | InlinePulseToken::LValueAntiquot { expr, .. } => self.check_rvalue(env, expr),
                InlinePulseToken::TypeAntiquot { ty, .. } => self.check_type(env, ty),
                InlinePulseToken::Declare { ident, ty, .. } => {
                    self.check_type(env, ty);
                    env.push_var_decl(ident, ty.clone(), LocalDeclKind::RValue);
                }
                InlinePulseToken::Verbatim(_) => {}
                InlinePulseToken::FieldAntiquot { .. } => {
                    // TODO: check that field exists
                }
            }
        }
    }

    fn check_rvalue(&mut self, env: &Env, rval: &Expr) {
        match &rval.val {
            ExprT::BoolLit(_) => {}
            ExprT::IntLit(_n, ty) => {
                self.check_type(env, ty);
                if self.check_types && !self.is_valid_int_type(env, ty.clone().into()) {
                    self.report(format!("invalid integer type: {}", ty), &rval.loc);
                }
            }
            ExprT::Var(n) => self.check_var(env, n, false),
            ExprT::Deref(inner) => {
                self.check_rvalue(env, inner);
                if self.check_types
                    && let Some(rval_ty) = self.infer_expr(env, inner)
                    && !self.is_pointer_type(env, rval_ty.clone())
                {
                    self.report(format!("not a pointer type: {}", rval_ty), &inner.loc)
                }
            }
            ExprT::Member(x, a) => {
                self.check_rvalue(env, x);
                if self.check_types
                    && let Some(t) = self.infer_expr(env, x)
                {
                    let t = env.vtype_whnf(t);
                    match &t.val {
                        TypeT::Pointer(_, PointerKind::Array) if &*a.val == "_length" => {}
                        TypeT::TypeRef(TypeRefKind::Struct(n)) => {
                            let Some(s) = env.lookup_struct(n) else {
                                return self.report(format!("unknown structure {}", n), &rval.loc);
                            };
                            let Some(_f) = s.get_field(a) else {
                                return self.report(
                                    format!("no field {} in structure {}", a, n),
                                    &rval.loc,
                                );
                            };
                        }
                        _ => {
                            return self.report(format!("not a structure type: {}", t), &rval.loc);
                        }
                    }
                }
            }
            ExprT::Index(arr, idx) => {
                self.check_rvalue(env, arr);
                self.check_rvalue(env, idx);
                if self.check_types {
                    if let Some(arr_ty) = self.infer_expr(env, arr) {
                        let arr_ty = env.vtype_whnf(arr_ty);
                        match &arr_ty.val {
                            TypeT::Pointer(_, PointerKind::Array) => {}
                            TypeT::Error => {}
                            _ => self.report(format!("not an array type: {}", arr_ty), &rval.loc),
                        }
                    }
                    if let Some(idx_ty) = self.infer_expr(env, idx) {
                        let idx_ty = env.vtype_whnf(idx_ty);
                        match &idx_ty.val {
                            TypeT::SizeT => {}
                            TypeT::Error => {}
                            _ => self.report(
                                format!("array index must have type SizeT, got: {}", idx_ty),
                                &idx.loc,
                            ),
                        }
                    }
                }
            }
            ExprT::Ref(lval) => {
                self.check_rvalue(env, lval);
                if !env.is_lvalue(lval) {
                    self.report(format!("expected lvalue for &, got {}", lval), &rval.loc);
                }
            }
            ExprT::UnOp(un_op, arg) => {
                self.check_rvalue(env, arg);
                if self.check_types
                    && let Some(arg_ty) = self.infer_expr(env, arg)
                {
                    match un_op {
                        UnOp::Not => match &env.vtype_whnf(arg_ty.clone().into()).val {
                            TypeT::Bool | TypeT::Error => {}
                            _ => self.report(
                                format!("! needs to be applied to bool, not {}", arg_ty),
                                &rval.loc,
                            ),
                        },
                        UnOp::Neg | UnOp::BitNot => {}
                    }
                }
            }
            ExprT::BinOp(bin_op, lhs, rhs) => {
                self.check_rvalue(env, lhs);
                self.check_rvalue(env, rhs);
                if self.check_types
                    && let (Some(lhs_ty), Some(rhs_ty)) =
                        (self.infer_expr(env, lhs), self.infer_expr(env, rhs))
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
                        BinOp::LogAnd | BinOp::LogOr | BinOp::Implies => {
                            check_eq(self);
                            match &env.vtype_whnf(lhs_ty.clone().into()).val {
                                TypeT::Bool | TypeT::SLProp | TypeT::Error => {}
                                _ => self.report(
                                    format!(
                                        "{} needs to be applied to bool/slprop, not {}",
                                        bin_op, lhs_ty
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
                        | BinOp::Sub
                        | BinOp::BitAnd
                        | BinOp::BitOr
                        | BinOp::BitXor
                        | BinOp::Shl
                        | BinOp::Shr => check_eq(self),
                    }
                }
            }
            ExprT::FnCall(f, args) => {
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
            ExprT::Cast(rval, ty) => {
                self.check_rvalue(env, rval);
                self.check_type(env, ty);
            }
            ExprT::InlinePulse(code, ty) => {
                self.check_type(env, ty);
                self.check_inline_pulse_code(env, code);
            }
            ExprT::Live(lval) => self.check_rvalue(env, lval),
            ExprT::Old(rval) => self.check_rvalue(env, rval),
            ExprT::Forall(var, ty, body) | ExprT::Exists(var, ty, body) => {
                self.check_type(env, ty);
                let mut env = env.clone();
                env.push_var_decl(var, ty.clone(), LocalDeclKind::RValue);
                self.check_rvalue(&env, body);
            }
            ExprT::StructInit(name, fields) => {
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
            ExprT::Malloc(ty) => self.check_type(env, ty),
            ExprT::MallocArray(ty, count) => {
                self.check_type(env, ty);
                self.check_rvalue(env, count);
            }
            ExprT::Free(val) => self.check_rvalue(env, val),
            ExprT::Error(ty) => self.check_type(env, ty),
        }
    }

    fn is_pointer_type(&mut self, env: &Env, ty: MaybeRc<Type>) -> bool {
        match &env.vtype_whnf(ty).val {
            TypeT::Pointer(_ty, _kind) => true,
            TypeT::Error => true,
            _ => false,
        }
    }

    fn check_lvalue(&mut self, env: &Env, lval: &Expr) {
        self.check_rvalue(env, lval);
        if !env.is_lvalue(lval) {
            self.report(format!("expected lvalue, got {}", lval), &lval.loc);
        }
    }

    fn check_var(&mut self, env: &Env, n: &Ident, needs_lvalue: bool) {
        match env.lookup_var(n) {
            None => match env.lookup_global_var(n) {
                Some(_) => {
                    if needs_lvalue {
                        self.report(
                            format!("need lvalue, but global {} is rvalue", n.val),
                            &n.loc,
                        );
                    }
                }
                None => self.report(format!("unknown local variable {}", n.val), &n.loc),
            },
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
                        (self.infer_expr(env, x), self.infer_expr(env, v))
                {
                    self.check_type_eq(env, v_ty.into(), x_ty.into());
                }
            }
            StmtT::If(c, b1, b2) => {
                self.check_bool(env, c);
                self.check_stmts(env, b1);
                self.check_stmts(env, b2);
            }
            StmtT::While {
                cond,
                inv,
                requires,
                ensures,
                body,
            } => {
                self.check_bool(env, cond);
                for inv in &**inv {
                    self.check_slprop(env, inv)
                }
                for r in &**requires {
                    self.check_bool(env, r)
                }
                for e in &**ensures {
                    self.check_bool(env, e)
                }
                let mut loop_env = env.clone();
                loop_env.enter_loop();
                self.check_stmts(&loop_env, body);
            }
            StmtT::Break | StmtT::Continue => {
                if !env.in_loop() {
                    self.report(
                        format!(
                            "{} outside of loop",
                            if matches!(&stmt.val, StmtT::Break) {
                                "break"
                            } else {
                                "continue"
                            }
                        ),
                        &stmt.loc,
                    );
                }
            }
            StmtT::Return(v) => {
                if let Some(v) = v {
                    self.check_rvalue(env, v);
                    if self.check_types
                        && let Some(ret_ty) = &env.return_type
                    {
                        self.check_has_type(env, v, ret_ty.clone().into());
                    }
                }
            }
            StmtT::Assert(v) => self.check_slprop(env, v),
            StmtT::GhostStmt(code) => self.check_inline_pulse_code(env, code),
            StmtT::Goto(_) => {}
            StmtT::Label { ensures, .. } => {
                for e in &**ensures {
                    self.check_slprop(env, e)
                }
            }
            StmtT::GotoBlock {
                body,
                label: _,
                ensures,
            } => {
                self.check_stmts(env, body);
                for e in &**ensures {
                    self.check_slprop(env, e)
                }
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

    fn check_slprops(&mut self, env: &Env, slprops: &Vec<Rc<Expr>>) {
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
            is_pure: _,
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
            DeclT::GlobalVar(GlobalVar {
                name: _,
                ty,
                init,
                is_pure: _,
            }) => {
                self.check_type(env, ty);
                if let Some(init) = init {
                    self.check_rvalue(env, init);
                }
            }
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
