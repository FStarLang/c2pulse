use crate::env::{Env, LocalDeclKind};
use crate::ir::*;
use std::rc::Rc;

fn elab_type(env: &Env, ty: &mut Type) {
    match &mut ty.val {
        TypeT::Int {
            signed: _,
            width: _,
        } => {}
        TypeT::SizeT => {}
        TypeT::Pointer { to, kind } => {
            elab_type(env, Rc::make_mut(to));
            match kind {
                PointerKind::Unknown => *kind = PointerKind::Ref,
                PointerKind::Ref => {}
                PointerKind::Array => {}
            }
        }
        TypeT::Error => {}
        TypeT::Void => {}
        TypeT::SLProp => {}
        TypeT::Bool => {}
    }
}

fn elab_lvalue(env: &Env, lval: &mut LValue) {
    match &mut lval.val {
        LValueT::Var(_) => {}
        LValueT::Deref(v) => elab_rvalue(env, Rc::make_mut(v)),
        LValueT::Error(ty) => elab_type(env, Rc::make_mut(ty)),
    }
}

fn cast_to(rval: &mut Rc<RValue>, ty: Rc<Type>) {
    *rval = RValueT::Cast {
        val: rval.clone(),
        ty,
    }
    .with_loc(rval.loc.clone())
}

fn cast_to_slprop(env: &Env, rval: &mut Rc<RValue>) {
    if !env.is_slprop(&env.infer_rvalue(rval).unwrap()) {
        *rval = RValueT::Cast {
            val: rval.clone(),
            ty: TypeT::SLProp.with_loc(rval.loc.clone()),
        }
        .with_loc(rval.loc.clone())
    }
}

fn cast_to_bool(env: &Env, rval: &mut Rc<RValue>) {
    if !env.is_bool(&env.infer_rvalue(rval).unwrap()) {
        *rval = RValueT::Cast {
            val: rval.clone(),
            ty: TypeT::Bool.with_loc(rval.loc.clone()),
        }
        .with_loc(rval.loc.clone())
    }
}

fn elab_rvalue(env: &Env, rval: &mut RValue) {
    match &mut rval.val {
        RValueT::IntLit { val: _, ty } => elab_type(env, Rc::make_mut(ty)),
        RValueT::LValue(v) => elab_lvalue(env, Rc::make_mut(v)),
        RValueT::Ref(v) => elab_lvalue(env, Rc::make_mut(v)),
        RValueT::FnCall(_f, args) => {
            // TODO: check type for f
            for arg in args {
                elab_rvalue(env, Rc::make_mut(arg));
            }
        }
        RValueT::Cast { val, ty } => {
            let val = Rc::make_mut(val);
            elab_type(env, Rc::make_mut(ty));
            elab_rvalue(env, val);
            let _actual_ty = env.infer_rvalue(val);
            // TODO: check that actual_ty can be casted to ty
        }
        RValueT::Error(ty) => elab_type(env, Rc::make_mut(ty)),
        RValueT::InlinePulse { val: _, ty } => elab_type(env, Rc::make_mut(ty)),
        RValueT::BinOp(bin_op, lhs, rhs) => {
            elab_rvalue(env, Rc::make_mut(lhs));
            elab_rvalue(env, Rc::make_mut(rhs));
            match (env.infer_rvalue(lhs), env.infer_rvalue(rhs)) {
                (Some(lhs_ty), Some(rhs_ty)) => {
                    let lhs_to_rhs = env.implicitly_converts_to(&lhs_ty, &rhs_ty);
                    let rhs_to_lhs = env.implicitly_converts_to(&rhs_ty, &lhs_ty);
                    match bin_op {
                        BinOp::Eq
                        | BinOp::Mul
                        | BinOp::Div
                        | BinOp::Mod
                        | BinOp::Add
                        | BinOp::Sub => {
                            match (lhs_to_rhs, rhs_to_lhs) {
                                (true, true) => {}
                                (true, false) => cast_to(lhs, rhs_ty),
                                (false, true) => cast_to(rhs, lhs_ty),
                                (false, false) => {
                                    // TODO: produce error
                                }
                            }
                        }
                        BinOp::LogAnd => match (env.is_slprop(&lhs_ty), env.is_slprop(&rhs_ty)) {
                            (true, true) => {}
                            (true, false) => cast_to(rhs, lhs_ty),
                            (false, true) => cast_to(lhs, rhs_ty),
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
    }
}

fn elab_stmt(env: &Env, stmt: &mut Stmt) {
    match &mut stmt.val {
        StmtT::Call(rval) => elab_rvalue(env, Rc::make_mut(rval)),
        StmtT::Decl(_, ty) => elab_type(env, Rc::make_mut(ty)),
        StmtT::Assign(x, v) => {
            elab_lvalue(env, Rc::make_mut(x));
            elab_rvalue(env, Rc::make_mut(v));
            let _x_ty = env.infer_lvalue(x);
            let _y_ty = env.infer_rvalue(v);
            // TODO: check that x_ty and y_ty are equal
        }
        StmtT::If(c, b1, b2) => {
            elab_rvalue(env, Rc::make_mut(c));
            cast_to_bool(env, c);
            elab_stmts(env, Rc::make_mut(b1));
            elab_stmts(env, Rc::make_mut(b2));
        }
        StmtT::Return(x) => {
            elab_rvalue(env, Rc::make_mut(x));
        }
        StmtT::Error => {}
    }
}

fn elab_stmts(env: &Env, stmts: &mut Vec<Rc<Stmt>>) {
    let mut env = env.clone();
    for stmt in stmts {
        elab_stmt(&env, Rc::make_mut(stmt));
        env.push_stmt(stmt);
    }
}

fn elab_slprops(env: &Env, slprops: &mut Vec<Rc<RValue>>) {
    for p in slprops {
        elab_rvalue(env, Rc::make_mut(p));
        cast_to_slprop(env, p);
    }
}

fn elab_fn_decl(
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
        elab_type(env, Rc::make_mut(ty));
        env.push_arg(arg, LocalDeclKind::RValue);
    }
    elab_type(env, Rc::make_mut(ret_type));
    elab_slprops(env, requires);
    elab_slprops(env, ensures);
}

fn elab_decl(env: &Env, decl: &mut Decl) {
    // TODO: check double definition of functions
    match &mut decl.val {
        DeclT::FnDefn(FnDefn { decl, body }) => {
            elab_fn_decl(env, decl);
            let env = &mut env.clone();
            env.push_fn_decl_args_for_body(decl);
            elab_stmts(env, body);
        }
        DeclT::FnDecl(fn_decl) => elab_fn_decl(env, fn_decl),
        DeclT::StructDefn(StructDefn { name: _, fields }) => {
            for (_n, ty) in fields {
                elab_type(env, Rc::make_mut(ty))
            }
        }
        DeclT::IncludeDecl(_) => {}
    }
}

pub fn elab(tu: &mut TranslationUnit) {
    let mut env = Env::new();
    for decl in &mut tu.decls {
        elab_decl(&env, decl);
        env.push_decl(decl);
    }
}
