use num_bigint::BigInt;

use crate::diag::{Diagnostic, DiagnosticLevel, Diagnostics};
use crate::env::{Env, LocalDeclKind};
use crate::ir::*;
use crate::mayberc::MaybeRc;
use std::rc::Rc;

struct Elaborator<'a> {
    diags: &'a mut Diagnostics,
}

fn cast_to(rval: &mut Rc<Expr>, ty: Rc<Type>) {
    *rval = ExprT::Cast(rval.clone(), ty).with_loc(rval.loc.clone())
}

impl<'a> Elaborator<'a> {
    fn report(&mut self, msg: String, loc: &SourceInfo) {
        self.diags.report(Diagnostic {
            loc: loc.location().clone(),
            level: DiagnosticLevel::Error,
            msg: msg,
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

            TypeT::Refine(ty, p) => {
                self.elab_type(env, Rc::make_mut(ty));

                let env = &mut env.clone();
                env.push_this(ty.clone());
                self.elab_rvalue(env, Rc::make_mut(p));
                self.cast_to_slprop(env, p);
            }
            TypeT::Plain(ty) => self.elab_type(env, Rc::make_mut(ty)),
        }
    }

    fn elab_lvalue(&mut self, env: &Env, lval: &mut Expr) {
        self.elab_rvalue(env, lval);
        if !env.is_lvalue(lval) {
            self.report(format!("expected lvalue, got {}", lval), &lval.loc);
        }
    }

    fn cast_to_slprop(&mut self, env: &Env, rval: &mut Rc<Expr>) {
        if env
            .infer_expr(rval)
            .ok()
            .filter(|p| env.is_slprop(p.clone()))
            .is_none()
        {
            *rval = ExprT::Cast(rval.clone(), TypeT::SLProp.with_loc(rval.loc.clone()))
                .with_loc(rval.loc.clone())
        }
    }

    fn cast_to_bool(&mut self, env: &Env, rval: &mut Rc<Expr>) {
        if env
            .infer_expr(rval)
            .ok()
            .filter(|p| env.is_bool(p.clone()))
            .is_none()
        {
            *rval = ExprT::Cast(rval.clone(), TypeT::Bool.with_loc(rval.loc.clone()))
                .with_loc(rval.loc.clone())
        }
    }

    fn elab_inline_pulse_code(&mut self, env: &Env, code: &mut InlinePulseCode) {
        let env = &mut env.clone();
        for tok in &mut code.tokens {
            match tok {
                InlinePulseToken::RValueAntiquot { expr, .. }
                | InlinePulseToken::LValueAntiquot { expr, .. } => {
                    self.elab_rvalue(env, Rc::make_mut(expr))
                }
                InlinePulseToken::TypeAntiquot { ty, .. } => self.elab_type(env, Rc::make_mut(ty)),
                InlinePulseToken::Declare { ident, ty, .. } => {
                    self.elab_type(env, Rc::make_mut(ty));
                    env.push_var_decl(ident, ty.clone(), LocalDeclKind::RValue);
                }
                InlinePulseToken::Verbatim(_) | InlinePulseToken::FieldAntiquot { .. } => {}
            }
        }
    }

    fn elab_rvalue(&mut self, env: &Env, rval: &mut Expr) {
        match &mut rval.val {
            ExprT::Var(_) => {}
            ExprT::Deref(v) => self.elab_rvalue(env, Rc::make_mut(v)),
            ExprT::Member(x, a) => {
                self.elab_rvalue(env, Rc::make_mut(x));
                if let Ok(t) = env.infer_expr(x) {
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
                        TypeT::TypeRef(TypeRefKind::Union(n)) => {
                            let Some(u) = env.lookup_union(n) else {
                                return self.report(format!("unknown union {}", n), &rval.loc);
                            };
                            let Some(_f) = u.get_field(a) else {
                                return self
                                    .report(format!("no field {} in union {}", a, n), &rval.loc);
                            };
                        }
                        _ => {
                            return self.report(format!("not a structure type: {}", t), &rval.loc);
                        }
                    }
                }
            }
            ExprT::Index(arr, idx) => {
                self.elab_rvalue(env, Rc::make_mut(arr));
                self.elab_rvalue(env, Rc::make_mut(idx));
                // Cast index to SizeT for Pulse array operations
                if let Ok(idx_ty) = env.infer_expr(idx) {
                    let idx_ty_whnf = env.vtype_whnf(idx_ty);
                    if !matches!(idx_ty_whnf.val, TypeT::SizeT) {
                        cast_to(idx, TypeT::SizeT.with_loc(idx.loc.clone()));
                    }
                }
            }
            ExprT::IntLit(_, ty) => self.elab_type(env, Rc::make_mut(ty)),
            ExprT::Ref(v) => {
                self.elab_rvalue(env, Rc::make_mut(v));
                if !env.is_lvalue(v) {
                    self.report(format!("expected lvalue for &, got {}", v), &rval.loc);
                }
            }
            ExprT::FnCall(f, args) => {
                for arg in args.iter_mut() {
                    self.elab_rvalue(env, Rc::make_mut(arg));
                }
                if let Some(fn_decl) = env.lookup_fn(f) {
                    let param_types: Vec<_> =
                        fn_decl.args.iter().map(|arg| arg.ty.clone()).collect();
                    for (arg, param_ty) in args.iter_mut().zip(param_types.iter()) {
                        let expected_ty = env.vtype_whnf(param_ty.clone().into());
                        if let Ok(actual_ty) = env.infer_expr(arg) {
                            if !env.vtype_eq(actual_ty, expected_ty.clone()) {
                                cast_to(arg, (*expected_ty).clone().into());
                            }
                        }
                    }
                }
            }
            ExprT::Cast(val, ty) => {
                let val = Rc::make_mut(val);
                self.elab_type(env, Rc::make_mut(ty));
                self.elab_rvalue(env, val);
                let _actual_ty = env.infer_expr(val);
                // TODO: check that actual_ty can be casted to ty
            }
            ExprT::Error(ty) => self.elab_type(env, Rc::make_mut(ty)),
            ExprT::Malloc(ty) | ExprT::Calloc(ty) => self.elab_type(env, Rc::make_mut(ty)),
            ExprT::MallocArray(ty, count) | ExprT::CallocArray(ty, count) => {
                self.elab_type(env, Rc::make_mut(ty));
                self.elab_rvalue(env, Rc::make_mut(count));
                if let Ok(count_ty) = env.infer_expr(count) {
                    if !matches!(&env.vtype_whnf(count_ty).val, TypeT::SizeT) {
                        cast_to(count, TypeT::SizeT.with_loc(count.loc.clone()));
                    }
                }
            }
            ExprT::Free(val) => self.elab_rvalue(env, Rc::make_mut(val)),
            ExprT::InlinePulse(code, ty) => {
                self.elab_type(env, Rc::make_mut(ty));
                self.elab_inline_pulse_code(env, Rc::make_mut(code));
            }
            ExprT::UnOp(un_op, arg) => {
                self.elab_rvalue(env, Rc::make_mut(arg));
                match un_op {
                    UnOp::Not => self.cast_to_bool(env, arg),
                    UnOp::Neg | UnOp::BitNot => {}
                }
            }
            ExprT::BinOp(bin_op, lhs, rhs) => {
                self.elab_rvalue(env, Rc::make_mut(lhs));
                self.elab_rvalue(env, Rc::make_mut(rhs));
                let Some(lhs_ty) = self.infer_expr(env, lhs) else {
                    return;
                };
                let Some(rhs_ty) = self.infer_expr(env, rhs) else {
                    return;
                };
                if *bin_op == BinOp::Eq {
                    let lhs_ty = env.vtype_whnf(lhs_ty.clone());
                    if let TypeT::Pointer(_, _) = &lhs_ty.val {
                        if let ExprT::IntLit(n, rhs_ty) = &mut Rc::make_mut(rhs).val {
                            if **n == BigInt::ZERO {
                                *rhs_ty = lhs_ty.to_rc();
                                return;
                            }
                        }
                    }
                }
                match bin_op {
                    BinOp::Eq
                    | BinOp::LEq
                    | BinOp::Lt
                    | BinOp::Mul
                    | BinOp::Div
                    | BinOp::Mod
                    | BinOp::Add
                    | BinOp::Sub
                    | BinOp::LogAnd
                    | BinOp::LogOr
                    | BinOp::Implies
                    | BinOp::BitAnd
                    | BinOp::BitOr
                    | BinOp::BitXor => {
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
                    BinOp::Shl | BinOp::Shr => {
                        let u32_ty: MaybeRc<Type> = TypeT::Int {
                            signed: false,
                            width: 32,
                        }
                        .with_loc(rhs.loc.clone())
                        .into();
                        if !env.vtype_eq(rhs_ty, u32_ty.clone()) {
                            cast_to(rhs, u32_ty.to_rc())
                        }
                    }
                }
            }
            ExprT::BoolLit(_) => {}
            ExprT::Live(val) => self.elab_rvalue(env, Rc::make_mut(val)),
            ExprT::Old(val) => self.elab_rvalue(env, Rc::make_mut(val)),
            ExprT::Forall(var, ty, body) | ExprT::Exists(var, ty, body) => {
                let mut env = env.clone();
                env.push_var_decl(var, ty.clone(), LocalDeclKind::RValue);
                self.elab_rvalue(&env, Rc::make_mut(body));
            }
            ExprT::StructInit(_, fields) => {
                for (_fld_name, fld_val) in fields {
                    self.elab_rvalue(env, Rc::make_mut(fld_val));
                }
            }
            ExprT::UnionInit(_, _, fld_val) => {
                self.elab_rvalue(env, Rc::make_mut(fld_val));
            }
        }
    }

    fn elab_stmt(&mut self, env: &Env, stmt: &mut Stmt) {
        match &mut stmt.val {
            StmtT::Call(rval) => self.elab_rvalue(env, Rc::make_mut(rval)),
            StmtT::Decl(_, ty) => self.elab_type(env, Rc::make_mut(ty)),
            StmtT::Assign(x, v) => {
                self.elab_lvalue(env, Rc::make_mut(x));
                self.elab_rvalue(env, Rc::make_mut(v));
                let Ok(x_ty) = env.infer_expr(x) else {
                    return;
                };
                let Ok(v_ty) = env.infer_expr(v) else {
                    return;
                };
                if !env.vtype_eq(x_ty.clone(), v_ty.clone()) {
                    // Don't cast if the only difference is pointer kind refinement
                    let x_whnf = env.vtype_whnf(x_ty.clone());
                    let v_whnf = env.vtype_whnf(v_ty.clone());
                    let is_kind_refinement = matches!(
                        (&x_whnf.val, &v_whnf.val),
                        (
                            TypeT::Pointer(_, PointerKind::Unknown | PointerKind::Ref),
                            TypeT::Pointer(_, PointerKind::Array)
                        )
                    );
                    if !is_kind_refinement {
                        cast_to(v, x_ty.to_rc());
                    }
                }
            }
            StmtT::If(c, b1, b2) => {
                self.elab_rvalue(env, Rc::make_mut(c));
                self.cast_to_bool(env, c);
                self.elab_stmts(env, Rc::make_mut(b1));
                self.elab_stmts(env, Rc::make_mut(b2));
            }
            StmtT::While {
                cond,
                inv,
                requires,
                ensures,
                body,
            } => {
                self.elab_rvalue(env, Rc::make_mut(cond));
                self.cast_to_bool(env, cond);
                self.elab_slprops(env, Rc::make_mut(inv));
                for r in Rc::make_mut(requires) {
                    self.elab_rvalue(env, Rc::make_mut(r));
                    self.cast_to_bool(env, r);
                }
                for e in Rc::make_mut(ensures) {
                    self.elab_rvalue(env, Rc::make_mut(e));
                    self.cast_to_bool(env, e);
                }
                self.elab_stmts(env, Rc::make_mut(body));
            }
            StmtT::Break | StmtT::Continue => {}
            StmtT::Return(x) => {
                if let Some(x) = x {
                    self.elab_rvalue(env, Rc::make_mut(x));
                    if let Some(ret_ty) = &env.return_type {
                        if let Ok(v_ty) = env.infer_expr(x) {
                            if !env.vtype_eq(v_ty, ret_ty.clone().into()) {
                                cast_to(x, ret_ty.clone());
                            }
                        }
                    }
                }
            }
            StmtT::Assert(v) => {
                self.elab_rvalue(env, Rc::make_mut(v));
                self.cast_to_slprop(env, v);
            }
            StmtT::GhostStmt(code) => self.elab_inline_pulse_code(env, Rc::make_mut(code)),
            StmtT::Goto(_) => {}
            StmtT::Label { ensures, .. } => {
                self.elab_slprops(env, Rc::make_mut(ensures));
            }
            StmtT::GotoBlock {
                body,
                label: _,
                ensures,
            } => {
                self.elab_stmts(env, Rc::make_mut(body));
                self.elab_slprops(env, Rc::make_mut(ensures));
            }
            StmtT::Error => {}
        }
    }

    /// Look ahead from a Decl to find a following assignment that refines the
    /// pointer kind (e.g. `int *a = malloc(sizeof(int) * 10)` → Array), and
    /// update the Decl's type before it is pushed to the environment.
    fn refine_decl_pointer_kind(env: &Env, stmts: &mut Vec<Rc<Stmt>>, decl_idx: usize) {
        let StmtT::Decl(decl_name, _) = &stmts[decl_idx].val else {
            return;
        };
        let var_name = &decl_name.val;
        for j in (decl_idx + 1)..stmts.len() {
            let StmtT::Assign(x, v) = &stmts[j].val else {
                continue;
            };
            let ExprT::Var(assign_name) = &x.val else {
                continue;
            };
            if assign_name.val != *var_name {
                continue;
            }
            let Ok(v_ty) = env.infer_expr(v) else {
                break;
            };
            let TypeT::Pointer(_, rhs_kind) = &env.vtype_whnf(v_ty).val else {
                break;
            };
            if *rhs_kind == PointerKind::Unknown {
                break;
            }
            if let StmtT::Decl(_, decl_ty) = &mut Rc::make_mut(&mut stmts[decl_idx]).val {
                if let TypeT::Pointer(_, kind) = &mut Rc::make_mut(decl_ty).val {
                    if *kind == PointerKind::Unknown || *kind == PointerKind::Ref {
                        *kind = rhs_kind.clone();
                    }
                }
            }
            break;
        }
    }

    fn elab_stmts(&mut self, env: &Env, stmts: &mut Vec<Rc<Stmt>>) {
        let mut env = env.clone();
        for i in 0..stmts.len() {
            Self::refine_decl_pointer_kind(&env, stmts, i);

            self.elab_stmt(&env, Rc::make_mut(&mut stmts[i]));
            env.push_stmt(&stmts[i]);
        }
    }

    fn elab_slprops(&mut self, env: &Env, slprops: &mut Vec<Rc<Expr>>) {
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
            is_pure: _,
        }: &mut FnDecl,
    ) {
        let env = &mut env.clone();
        for arg in args {
            self.elab_type(env, Rc::make_mut(&mut arg.ty));
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
            DeclT::UnionDefn(UnionDefn { name: _, fields }) => {
                for (_n, ty) in fields {
                    self.elab_type(env, Rc::make_mut(ty))
                }
            }
            DeclT::IncludeDecl(_) => {}
            DeclT::GlobalVar(GlobalVar {
                name: _,
                ty,
                init,
                is_pure: _,
            }) => {
                self.elab_type(env, Rc::make_mut(ty));
                if let Some(init) = init {
                    self.elab_rvalue(env, Rc::make_mut(init));
                }
            }
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
