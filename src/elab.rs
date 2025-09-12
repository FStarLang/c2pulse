use crate::env::Env;
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
    }
}

fn elab_lvalue(env: &Env, lval: &mut LValue, expected_type: Option<Rc<Type>>) {
    match &mut lval.val {
        LValueT::Var(_) => {}
        LValueT::Deref(v) => elab_rvalue(
            env,
            Rc::make_mut(v),
            expected_type.map(|ety| {
                Rc::new(Ast {
                    loc: lval.loc.clone(),
                    val: TypeT::Pointer {
                        to: ety,
                        kind: PointerKind::Unknown,
                    },
                })
            }),
        ),
        LValueT::Error(ty) => elab_type(env, Rc::make_mut(ty)),
    }
}

fn elab_rvalue(env: &Env, rval: &mut RValue, expected_type: Option<Rc<Type>>) {
    match &mut rval.val {
        RValueT::IntLit { val: _, ty } => elab_type(env, Rc::make_mut(ty)),
        RValueT::LValue(v) => elab_lvalue(env, Rc::make_mut(v), expected_type),
        RValueT::Ref(v) => elab_lvalue(
            env,
            Rc::make_mut(v),
            expected_type.and_then(|ety| match &ety.val {
                TypeT::Pointer { to, kind: _ } => Some(to.clone()),
                _ => None,
            }),
        ),
        RValueT::Cast { val, ty } => {
            let val = Rc::make_mut(val);
            elab_type(env, Rc::make_mut(ty));
            elab_rvalue(env, val, Some(ty.clone()));
            let _actual_ty = env.infer_rvalue(val);
            // TODO: check that actual_ty can be casted to ty
        }
        RValueT::Error(ty) => elab_type(env, Rc::make_mut(ty)),
    }
}

fn elab_stmt(env: &Env, stmt: &mut Stmt) {
    match &mut stmt.val {
        StmtT::Call(rval) => elab_rvalue(env, Rc::make_mut(rval), None),
        StmtT::Decl(_, ty) => elab_type(env, Rc::make_mut(ty)),
        StmtT::Assign(x, v) => {
            elab_lvalue(env, Rc::make_mut(x), None);
            let x_ty = env.infer_lvalue(x);
            elab_rvalue(env, Rc::make_mut(v), x_ty);
            let _y_ty = env.infer_rvalue(v);
            // TODO: check that x_ty and y_ty are equal
        }
        StmtT::If(c, b1, b2) => {
            let c_bool_type = None; // TODO
            elab_rvalue(env, Rc::make_mut(c), c_bool_type);
            elab_stmts(env, Rc::make_mut(b1));
            elab_stmts(env, Rc::make_mut(b2));
        }
        StmtT::Return(x) => {
            let ret_ty = None; // TODO
            elab_rvalue(env, Rc::make_mut(x), ret_ty);
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

fn elab_fn_decl(
    env: &Env,
    FnDecl {
        name: _,
        ret_type,
        args,
    }: &mut FnDecl,
) {
    for (_n, ty) in args {
        elab_type(env, Rc::make_mut(ty))
    }
    elab_type(env, Rc::make_mut(ret_type));
}

fn elab_decl(env: &Env, decl: &mut Decl) {
    // TODO: check double definition of functions
    match &mut decl.val {
        DeclT::FnDefn(FnDefn { decl, body }) => {
            elab_fn_decl(env, decl);
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
